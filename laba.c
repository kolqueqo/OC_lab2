include <linux/kernel.h>
#include <linux/module.h>
#include <linux/blkdev.h>
#include <linux/types.h>
#include <linux/mm_types.h>
#include <linux/mutex.h>

#include <linux/proc_fs.h>
#include <linux/pid.h>
#include <linux/device.h>

#define BUF_SIZE 1024

static struct mutex mtx;

static int pid = 1;

static struct proc_dir_entry *parent;

static size_t write_vm_area(char __user *buffer, struct task_struct *ts) {
size_t len = 0;
struct vm_area_struct *vm;
char buf[BUF_SIZE];
struct mm_struct *mm = ts->mm;
len += sprintf(buf + len, "vm_area_struct:\n");
if (mm == NULL){
len += sprintf(buf + len, "  mm is NULL\n");
if (copy_to_user(buffer, buf, len)) {
return -EFAULT;
}
return len;
}

vm = mm->mmap;
if (vm == NULL){
len += sprintf(buf + len, "  vm is NULL\n");
if (copy_to_user(buffer, buf, len)) {
return -EFAULT;
}
return len;
}

len += sprintf(buf + len, "  vm_start: %ld\n", vm->vm_start); // start
len += sprintf(buf + len, "  vm_end: %ld\n", vm->vm_end); //end
len += sprintf(buf + len, "  vm_subtree_gap: %ld\n", vm->rb_subtree_gap); // largest gap
len += sprintf(buf + len, "  vm_flags: %ld\n", vm->vm_flags); // flags

if (copy_to_user(buffer, buf, len)) {
return -EFAULT;
}

return len;
}

static size_t write_inode(char __user *buffer, struct task_struct *ts) {
size_t len = 0;
struct vm_area_struct *vm;
struct file *fl;
struct inode *inode;
char buf[BUF_SIZE];
struct mm_struct *mm = ts->mm;
len += sprintf(buf + len, "inode:\n");
if (mm == NULL){
len += sprintf(buf + len, " mm is NULL");
if (copy_to_user(buffer, buf, len)) {
return -EFAULT;
}
return len;
}

vm = mm->mmap;
if (vm == NULL){
len += sprintf(buf + len, "  vm is NULL");
if (copy_to_user(buffer, buf, len)) {
return -EFAULT;
}
return len;
}

fl = vm->vm_file;
if (fl == NULL){
len += sprintf(buf + len, "  vm_file is NULL");
if (copy_to_user(buffer, buf, len)) {
return -EFAULT;
}
return len;
}

inode = fl->f_inode;
if (inode == NULL){
len += sprintf(buf + len, "  inode is NULL");
if (copy_to_user(buffer, buf, len)) {
return -EFAULT;
}
return len;
}

len += sprintf(buf + len, "  i_ino: %ld\n", inode->i_ino); // inode number
len += sprintf(buf + len, "  i_mode: %d\n", inode->i_mode); //flags
len += sprintf(buf + len, "  i_nlink: %d\n", inode->i_nlink); // number of hard links to the file
len += sprintf(buf + len, "  i_rdev: %d\n", inode->i_rdev); // if a device contains a ID of that device else 0
len += sprintf(buf + len, "  i_size: %lld\n", inode->i_size); // file size
len += sprintf(buf + len, "  i_blocks: %lld\n", inode->i_blocks); // number of blocks allocated to the file

if (copy_to_user(buffer, buf, len)) {
return -EFAULT;
}

return len;
}

/*       */

static int open_proc(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "proc file opened\t");
    return 0;
}

/*       */

static int release_proc(struct inode *inode, struct file *file)
{
    mutex_unlock(&mtx);

    printk(KERN_INFO "proc file released\n");
    return 0;
}

/*       */

static ssize_t read_proc(struct file *filp, char __user *ubuf, size_t count, loff_t *offset) {
mutex_lock(&mtx);

char buf[BUF_SIZE];
int len = 0;
struct task_struct *task_struct_ref = pid_task(find_vpid(pid), PIDTYPE_PID);
printk(KERN_INFO "proc file read\n");
if (*offset > 0 || count < BUF_SIZE){
return 0;
}


if (task_struct_ref == NULL){

len += sprintf(buf,"task_struct for pid %d is NULL.\n",pid);

if (copy_to_user(ubuf, buf, len)){
return -EFAULT;
}

*offset = len;
return len;
}
len += write_vm_area(ubuf + len, task_struct_ref);
len += write_inode(ubuf + len, task_struct_ref);

*offset = len;

mutex_unlock(&mtx);
return len;
}

/*       */

static ssize_t write_proc(struct file *filp, const char __user *ubuf, size_t count, loff_t *off) {
mutex_lock(&mtx);

int num_of_args, c, b;
char buf[BUF_SIZE];

printk(KERN_INFO "proc file wrote\n");

if (*off > 0 || count > BUF_SIZE){
return -EFAULT;
}

if( copy_from_user(buf, ubuf, count) ) {
return -EFAULT;
}

num_of_args = sscanf(buf, "%d",  &b);
if (num_of_args != 1){
return -EFAULT;
}


pid = b;

c = strlen(buf);
*off = c;

mutex_unlock(&mtx);
return c;
}

/*       */

static struct proc_ops proc_fops = {
        .proc_open = open_proc,
        .proc_read = read_proc,
        .proc_write = write_proc,
        .proc_release = release_proc
};

static int __init lab_driver_init(void) {
    mutex_init(&mtx);

    parent = proc_mkdir("lab2",NULL);

    if( parent == NULL )
    {
        pr_info("Error creating proc entry");
        return -1;
    }

    proc_create("struct_info", 0666, parent, &proc_fops);

    pr_info("init done\n");
    return 0;
}

static void __exit lab_driver_exit(void)
{
    //remove_proc_entry("lab/struct_info", parent);

    proc_remove(parent);

    pr_info("exit done\n");
}

module_init(lab_driver_init);
module_exit(lab_driver_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("lab2");
MODULE_VERSION("1.0");