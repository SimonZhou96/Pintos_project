#include <stdio.h>
#include <syscall-nr.h>
#include <user/syscall.h>
#include <string.h>
#include <ctype.h>
#include <devices/shutdown.h>
#include <devices/input.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "threads/malloc.h"
#include "userprog/pagedir.h"
#include "userprog/syscall.h"
#include "userprog/process.h"
#include <kernel/console.h>
#include <filesys/filesys.h>
#include <filesys/file.h>


struct lock filesys_lock;

static void syscall_handler (struct intr_frame *);
void validate_pointer (void *ptr);
void extract_arguments (int *esp);
int args[3];
void *pagedir_get_page_now(int one_arg);
void validate_arguments (int *esp, int num);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{	
	// lock_init(&filesys_lock);
	int *f_esp = (int *)f->esp;
	validate_pointer(f_esp);
	switch(*f_esp){
		case SYS_HALT:
      		shutdown_power_off ();
      		
   		case SYS_EXIT:
   			validate_arguments(f_esp, 1);
   			extract_arguments(f_esp);
   			exit ((int)args[0]);
      		break;

   		case SYS_EXEC:
   			validate_arguments(f_esp, 1);
   			extract_arguments (f_esp);
   			char* temp_file = pagedir_get_page_now (args[0]);
  		    if(temp_file == NULL){
  		    	exit(-1);
  		    }
  		    f->eax = exec (temp_file);
      		break;

   		case SYS_WAIT:
   			validate_arguments(f_esp, 1);
   			extract_arguments (f_esp);
  		    f->eax = wait ((pid_t)args[0]);
      		break;
    
   		case SYS_WRITE:
   			validate_arguments(f_esp, 3);

   			//int *next = ((f_esp + 1) + 1);
    		validate_pointer (*(f_esp+2));

   			extract_arguments (f_esp);
  		    args[1] = pagedir_get_page_now (args[1]);
  		    f->eax = write ((int)args[0], 
  		    				(void *)args[1], 
  		    				(unsigned)args[2]);
      		break;
    
   		case SYS_READ:
   			validate_arguments(f_esp, 3);
   			validate_pointer (*(f_esp+2));

   			extract_arguments (f_esp);
  		    f->eax = read ((int)args[0], 
  		    			   (void *)args[1], 
  		    			   (unsigned)args[2]);
      		break;

   		case SYS_CREATE:
   			validate_arguments(f_esp, 2);
   		 	extract_arguments (f_esp);
  			args[0] = pagedir_get_page_now (args[0]);
  			f->eax = create ((char *)args[0], (unsigned) args[1]);
      		break;

    	case SYS_REMOVE:
    		validate_arguments(f_esp, 1);
    		extract_arguments (f_esp);
		    char *file_to_close = pagedir_get_page_now (args[0]);
		    //lock_acquire (&filesys_lock);
		    f->eax = filesys_remove (file_to_close);
		    //if (lock_held_by_current_thread (&filesys_lock))
		    //    lock_release (&filesys_lock);
       		break;

    	case SYS_OPEN:
    		validate_arguments(f_esp, 1);
 	        extract_arguments (f_esp);
            f->eax = open ((char *)args[0]);
       		break; 
  
    	case SYS_FILESIZE:
    		validate_arguments(f_esp, 1);
    		extract_arguments (f_esp);
    		f->eax = filesize ((int)args[0]);
       		break;

    	case SYS_CLOSE:
    		validate_arguments(f_esp, 1);
    		extract_arguments (f_esp);
  	        args[0] = pagedir_get_page_now (args[0]);
  	        close ((int)args[0]);
       		break;       

    	case SYS_TELL:
    		validate_arguments(f_esp, 1);
    		extract_arguments (f_esp);
  			f->eax = tell ((int)args[0]);
       		break;

    	case SYS_SEEK:
    		validate_arguments(f_esp, 1);
    		extract_arguments (f_esp);
  	        seek ((int)args[0], (unsigned)args[1]);
       		break; 
  	}
}

void *pagedir_get_page_now(int one_arg){
	const void * arg2 = (const void *)one_arg;
	const void * return_value = pagedir_get_page(thread_current()->pagedir, arg2);
	return return_value;
}


  // int args[MAX_ARGS];
  // lock_init (&filesys_lock);
  // validate_pointer (f->esp);
  // int *sp = (int *)f->esp;
  // struct thread *cur = thread_current ();

  // switch (*sp)
  // {
  //  case SYS_HALT:
  //     shutdown_power_off ();

  //  case SYS_EXIT:
  //     get_arguments (sp, &args[0], 1); 
  //     exit ((int)args[0]);
  //     break;

  //  case SYS_EXEC:
  //     get_arguments (sp, &args[0], 1);
  //     f->eax = exec ((const char *)pagedir_get_page (cur->pagedir,
		// 			(const void *) args[0]));
  //     break;

  //  case SYS_WAIT:
  //     get_arguments (sp, &args[0], 1);
  //     f->eax = wait ((pid_t)args[0]);
  //     break;
    
  //  case SYS_WRITE:
  //     get_arguments (sp, &args[0], 3);
  //     args[1] = (int)pagedir_get_page (cur->pagedir, (const void *)args[1]);
  //     f->eax = write ((int)args[0], (void *)args[1], (unsigned)args[2]);
  //     break;
    
  //  case SYS_READ:
  //     get_arguments (sp, &args[0], 3);
  //     f->eax = read ((int)args[0], (void *)args[1], (unsigned)args[2]);
  //     break;

  //  case SYS_CREATE:
  //     get_arguments (sp, &args[0], 2);
  //     args[0] =(int) pagedir_get_page (cur->pagedir, (const void *)args[0]);
  //     f->eax = create ((char *)args[0], (unsigned) args[1]);
  //     break;

  //   case SYS_REMOVE:
  //      get_arguments (sp, &args[0], 1);
  //      char *file_to_close = (char *)pagedir_get_page (cur->pagedir,
		// 			(const void *)args[0]);
  //      lock_acquire (&filesys_lock);
  //      f->eax = filesys_remove (file_to_close);
  //      if (lock_held_by_current_thread (&filesys_lock))
  //        lock_release (&filesys_lock);
  //      break;

  //   case SYS_OPEN:
  //      get_arguments (sp, &args[0], 1);
  //      f->eax = open ((char *)args[0]);
  //      break; 
  
  //   case SYS_FILESIZE:
  //      get_arguments (sp, &args[0], 1);
  //      f->eax = filesize ((int)args[0]);
  //      break;

  //   case SYS_CLOSE:
  //      get_arguments (sp, &args[0], 1);
  //      args[0] = (int)pagedir_get_page (cur->pagedir, (const void *)args[0]);
  //      close ((int)args[0]);
  //      break;       

  //   case SYS_TELL:
  //      get_arguments (sp, &args[0], 1);
  //      f->eax = tell ((int)args[0]);
  //      break;

  //   case SYS_SEEK:
  //      get_arguments (sp, &args[0], 2);
  //      seek ((int)args[0], (unsigned)args[1]);
  //      break; 
  // }
//}

void
extract_arguments (int *esp)
{
  for (int i = 0; i < 3; i++)
  {
    int *next = ((esp + i) + 1);
    validate_pointer (next);
    args[i] = *next;
    //printf("%% esp = %d; i = %d; *next = %d\n",*esp ,i, *next);
  }
}

void
validate_arguments (int *esp, int num)
{
  for (int i = 0; i < num; i++)
  {
    int *next = ((esp + i) + 1);
    //validate_pointer (next);
  }
}

void
validate_pointer (void *ptr)												// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! save
{
  if (!is_user_vaddr (ptr)) 
    exit (-1);
  if  ((pagedir_get_page (thread_current ()->pagedir, ptr) == NULL))
    exit (-1);
}


void exit (int status){														// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! save
 /* XXX: TODO
    If the current thread exits, then it should be removed from its
    parent's child list. */
  //cur->md->exit_status = status;
  //sema_up (&cur->md->completed);
  sema_up(&thread_current()->my_semas->terminated);							//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  thread_current()->my_semas->exit_status = status;
  printf ("%s: exit(%d)\n", thread_current ()->name, status);
  thread_exit ();
}
void exit_temp (int status){                           // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! save
 /* XXX: TODO
    If the current thread exits, then it should be removed from its
    parent's child list. */
  //cur->md->exit_status = status;
  //sema_up (&cur->md->completed);
  sema_up(&thread_current()->my_semas->terminated);             //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  thread_current()->my_semas->exit_status = status;
  printf ("%s: exit(%d)\n", thread_current ()->name, status);
  sema_down(&thread_current()->my_semas->terminated);  
}



// bool
// create (const char *file_name, unsigned size)
// {
//   int return_value;
//   if (file_name == NULL)
//     exit (-1);    
//   lock_acquire (&filesys_lock);
//   return_value = filesys_create (file_name, size);
//   lock_release (&filesys_lock);
//   return return_value;
// }

bool
create (const char *file_name, unsigned size)
{
  if (file_name == NULL)
    exit (-1);    
  //lock_acquire (&filesys_lock);
  bool success = filesys_create (file_name, size);
  //lock_release (&filesys_lock);
  return success;
}


// int
// open (const char *file)
// {
//   struct thread *cur = thread_current ();
//   validate_pointer ((void *)file);
//   if (file == NULL)
//     exit (-1);
//   if (strcmp (file, "") == 0)
//     return -1;
//   lock_acquire (&filesys_lock);
//   struct file *open_file = filesys_open (file); 
//   if (lock_held_by_current_thread (&filesys_lock))
//      lock_release (&filesys_lock);
//   if (open_file == NULL)
//     return -1;
//   //if (file_get_inode (open_file) == file_get_inode(cur->md->exec_file))
//   //    file_deny_write (open_file);
//   struct file **fd_array = cur->fd;
//   int k;
//   for (k = 2; k < MAX_FD; k++)
//   { 
//     if (fd_array[k] == NULL)
//     {
//      fd_array[k] = open_file;
//      break;
//     }
//   }
//    return k;
// }  




int
open (const char *file)
{
  struct thread *cur = thread_current ();
  validate_pointer ((void *)file);
  if (file == NULL)
    exit (-1);
  //if (strcmp (file, "") == 0)
  //  return -1;
  //lock_acquire (&filesys_lock);
  struct file *open_file = filesys_open (file); 
  //if (lock_held_by_current_thread (&filesys_lock))
     //lock_release (&filesys_lock);
  if (open_file == NULL)
    return -1;
  if (file_get_inode (open_file) == file_get_inode(cur->my_semas->executable)){
	file_deny_write (open_file);
  }
  struct file **fd_temp = cur->fd;
  int i;
  for (i = 2; i < MAX_FD; i++){ 
    if (fd_temp[i] == NULL){
     fd_temp[i] = open_file;
     break;
    }
  }
   return i;
}  



// int
// read (int fd, void *_buffer, unsigned size)
// {
//   struct thread *cur = thread_current ();
//   char *buffer = (char *)_buffer;
//   validate_pointer (buffer);
//   int retval = -1;
//   if (fd == 1 || fd < 0 || fd > MAX_FD)
//     exit (-1); 
//   if (fd == 0)
//   {
//     char c;
//     unsigned i = 0;
//     while ((c = input_getc ())!= '\n')
//     {
//       buffer[i] = c; 
//       i++;
//       if (i == size-1) break;
//     }

//   }
//   else {
//     //lock_acquire (&filesys_lock);
//     struct file *file = cur->fd[fd];
//     if (file != NULL) {
//       //if (file_get_inode (file) == file_get_inode(cur->md->exec_file))
//       //  file_deny_write (file);
//       retval = file_read (file, buffer, size);
//       //cur->fd[fd] = file;
//     }
//     else retval = -1;
//     //if (lock_held_by_current_thread (&filesys_lock))
//       //lock_release (&filesys_lock);
//   }
//   return retval;
// }

int
read (int fd, void *_buffer, unsigned size)
{
  struct thread *cur = thread_current ();
  char *buffer = (char *)_buffer;
  validate_pointer (buffer);
  int actual_number = size;
  if (size ==0 ){
    actual_number = 0;
    // free(buffer);
    return 0;
  }
  if (fd == 1 || fd < 0 || fd > MAX_FD)
    exit (-1);

  struct file * ff = cur->fd[fd];
  if(ff == NULL){
  	return actual_number;
  }
  else{
  	if (fd == 0){
	    if(ff != NULL){
	    	file_read(ff, buffer, size);
	    }
  	}
  	else {
	    //lock_acquire (&filesys_lock);
	    if (ff != NULL) {
      		if (file_get_inode (ff) == file_get_inode(cur->my_semas->executable)){
       			file_deny_write (ff);
      		}
	      actual_number = file_read (ff, buffer, size);
	      //cur->fd[fd] = file;
	    }
    	else actual_number = -1;
    //if (lock_held_by_current_thread (&filesys_lock))
      //lock_release (&filesys_lock);
  	}	
  } 
  // printf("********888now the actual_number is :%d\n",actual_number);
  return actual_number;
}


// int
// write (int fd, const void *_buffer, unsigned size)
// {
//   struct thread *cur = thread_current ();
//   struct file *file_to_write;

//   if (_buffer == NULL)
//     exit (-1);
//   int actual_number;
//   if (fd == 1) {
//     if(size < 200){
//       putbuf (_buffer, size);
//       actual_number = size;  
//     }
//     else{
//       int temp_size = 200;
//       int put_count = size/200;
//       for(int i = 0; i <= put_count; i++){
//         putbuf(_buffer,200);
//         _buffer = _buffer + 200*sizeof(char *);
//       }
//       int last_size = size%200;
//       putbuf(_buffer,last_size);
//     }  
//   }

//   else if (fd < 1 || fd > MAX_FD){
//     return -1;
//   }

//   else
//   {
//     lock_acquire (&filesys_lock);
//     file_to_write = cur->fd[fd];
//     if (file_to_write != NULL) {
//       actual_number = file_write (file_to_write, _buffer, size);
//       file_allow_write (file_to_write);
//     }
//     else actual_number = -1;
    
//     if (lock_held_by_current_thread (&filesys_lock))
//       lock_release (&filesys_lock);
//   }
//   return actual_number;
// }

int
write (int fd, const void *buffer, unsigned size)
{
  struct thread *cur = thread_current ();
  struct file *file_to_write;
  //validate_pointer(buffer);
  if (buffer == NULL)
    exit (-1);
  if (size == 0) return 0;
  int actual_number;
  if (fd == 1) {
    if(size < 200){
      putbuf (buffer, size);
      actual_number = size;  
    }
    else{
      int temp_size = 200;
      int put_count = size/200;
      for(int i = 0; i <= put_count; i++){
        putbuf(buffer,200);
        buffer = buffer + 200*sizeof(char *);
      }
      int last_size = size%200;
      putbuf(buffer,last_size);
    }  
  }

  else if (fd < 1 || fd > MAX_FD){
    return -1;
  }

  else
  {
    //lock_acquire (&filesys_lock);
    file_to_write = cur->fd[fd];
    if (file_to_write != NULL) {
      actual_number = file_write (file_to_write, buffer, size);
      file_allow_write (file_to_write);
    }
    else actual_number = -1;

    //if (lock_held_by_current_thread (&filesys_lock))
    //  lock_release (&filesys_lock);
  }
  return actual_number;
}




void
close (int fd)
{
  struct thread *cur = thread_current ();
  //lock_acquire (&filesys_lock);
  file_close (cur->fd[fd]);
  cur->fd[fd] = NULL;
  //if (lock_held_by_current_thread (&filesys_lock))
  //  lock_release (&filesys_lock);
}

int
filesize (int fd)
{
  struct file *file = thread_current ()->fd[fd];
  if (file == NULL)
    exit (-1);
  return file_length (file);
}

unsigned
tell (int fd)
{
  struct file *file = thread_current ()->fd[fd];
  if (file == NULL)
   exit (-1);
  return file_tell (file);
} 

void
seek (int fd, unsigned position)
{
  struct file *file = thread_current ()->fd[fd];
  if (file == NULL)
   exit (-1);
  file_seek (file, position);
}

pid_t exec (const char *file)													// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! save
{
  // if (file == NULL)
  //  exit (-1);
  tid_t child_tid = process_execute (file);
  return (pid_t)child_tid;
}

int wait (pid_t pid){													        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! save
  return process_wait((tid_t)pid);
}


