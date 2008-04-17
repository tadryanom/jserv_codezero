/*
 * Helper functions that wrap raw l4 syscalls.
 *
 * Copyright (C) 2007 Bahadir Balban
 */

#ifndef __L4LIB_SYSLIB_H__
#define __L4LIB_SYSLIB_H__

#include <stdio.h>
#include <l4lib/arch/syscalls.h>
#include <l4/macros.h>

/*
 * NOTE:
 * Its best to use these wrappers because they generalise the way
 * common ipc data like sender id, error, ipc tag are passed
 * between ipc parties.
 *
 * The arguments to l4_ipc() are used by the microkernel to initiate
 * the ipc. Any data passed in message registers may or may not be
 * a duplicate of this data, but the distinction is that anything
 * that is passed via the mrs are meant to be used by the other party
 * participating in the ipc.
 */

/* For system call arguments */
#define L4SYS_ARG0	(MR_UNUSED_START)
#define	L4SYS_ARG1	(MR_UNUSED_START + 1)
#define L4SYS_ARG2	(MR_UNUSED_START + 2)
#define L4SYS_ARG3	(MR_UNUSED_START + 3)

/*
 * Servers get sender.
 */
static inline l4id_t l4_get_sender(void)
{
	return (l4id_t)read_mr(MR_SENDER);
}

static inline unsigned int l4_get_tag(void)
{
	return read_mr(MR_TAG);
}

static inline void l4_set_tag(unsigned int tag)
{
	write_mr(MR_TAG, tag);
}

static inline l4id_t self_tid(void)
{
	struct task_ids ids;

	l4_getid(&ids);
	return ids.tid;
}

static inline int l4_send(l4id_t to, unsigned int tag)
{
	l4_set_tag(tag);

	return l4_ipc(to, L4_NILTHREAD);
}

static inline int l4_sendrecv(l4id_t to, l4id_t from, unsigned int tag)
{
	int err;

	BUG_ON(to == L4_NILTHREAD || from == L4_NILTHREAD);
	l4_set_tag(tag);

	err = l4_ipc(to, from);

	return err;
}

static inline int l4_receive(l4id_t from)
{
	return l4_ipc(L4_NILTHREAD, from);
}

/* Servers:
 * Sets the message register for returning errors back to client task.
 * These are usually posix error codes.
 */
static inline void l4_set_retval(int retval)
{
	write_mr(MR_RETURN, retval);
}

/* Clients:
 * Learn result of request.
 */
static inline int l4_get_retval(void)
{
	return read_mr(MR_RETURN);
}

static inline void l4_print_mrs()
{
	printf("Message registers: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
	       read_mr(0), read_mr(1), read_mr(2), read_mr(3),
	       read_mr(4), read_mr(5));
}

/* Servers:
 * Return the ipc result back to requesting task.
 */
static inline int l4_ipc_return(int retval)
{
	// unsigned int tag = l4_get_tag();
	l4id_t sender = l4_get_sender();

	l4_set_retval(retval);

	/* Setting the tag may overwrite retval so we l4_send without tagging */
	return l4_ipc(sender, L4_NILTHREAD);
}

/* A helper that translates and maps a physical address to virtual */
static inline void *l4_map_helper(void *phys, int npages)
{
	struct task_ids ids;

	l4_getid(&ids);
	l4_map(phys, phys_to_virt(phys), npages, MAP_USR_RW_FLAGS, ids.tid);
	return phys_to_virt(phys);
}


/* A helper that translates and maps a physical address to virtual */
static inline void *l4_unmap_helper(void *virt, int npages)
{
	struct task_ids ids;

	l4_getid(&ids);
	l4_unmap(virt, npages, ids.tid);
	return virt_to_phys(virt);
}

#endif /* __L4LIB_SYSLIB_H__ */
