/*************************************************************************
 *
 *  $RCSfile: alloc.c,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: mhu $ $Date: 2001-12-04 15:10:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Matthias Huetsch <matthias.huetsch@sun.com>
 *
 *
 ************************************************************************/

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif

#ifndef INCLUDED_STDDEF_H
#include <stddef.h>
#define INCLUDED_STDDEF_H
#endif

#ifndef INCLUDED_STDLIB_H
#include <stdlib.h>
#define INCLUDED_STDLIB_H
#endif

#ifndef INCLUDED_STRING_H
#include <string.h>
#define INCLUDED_STRING_H
#endif

/*===========================================================================
 *
 * rtl_memory (UNX) internals.
 *
 *=========================================================================*/
#ifdef SAL_UNX

#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <fcntl.h>

typedef pthread_mutex_t mutex_type;

#define RTL_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER
#define RTL_MUTEX_ACQUIRE(a)  pthread_mutex_lock((a))
#define RTL_MUTEX_RELEASE(a)  pthread_mutex_unlock((a))

#if   defined(FREEBSD) || defined(NETBSD)
static sal_uInt32 __rtl_memory_vmpagesize (void)
{
    /* xBSD */
    return (sal_uInt32)(getpagesize());
}
#elif defined(IRIX) || defined(LINUX) || defined(SOLARIS)
static sal_uInt32 __rtl_memory_vmpagesize (void)
{
    /* POSIX */
    return (sal_uInt32)(sysconf(_SC_PAGESIZE));
}
#else
static sal_uInt32 __rtl_memory_vmpagesize (void)
{
    /* other */
    return (sal_uInt32)(0x2000);
}
#endif /* FREEBSD || NETBSD || IRIX || LINUX || SOLARIS */

#ifndef PROT_HEAP
#define PROT_HEAP (PROT_READ | PROT_WRITE | PROT_EXEC)
#endif

#ifndef MAP_ANON
static void* __rtl_memory_vmalloc (size_t n)
{
    /* SYSV */
    int fd = open("/dev/zero", O_RDWR);
    if (!(fd < 0))
    {
        void * p = mmap(NULL, n, PROT_HEAP, MAP_PRIVATE, fd, 0);
        close(fd);
        return ((p == MAP_FAILED) ? NULL : p);
    }
    return (NULL);
}
#else  /* MAP_ANON */
static void* __rtl_memory_vmalloc (size_t n)
{
    /* xBSD */
    void * p = mmap(NULL, n, PROT_HEAP, MAP_PRIVATE | MAP_ANON, -1, 0);
    return ((p == MAP_FAILED) ? NULL : p);
}
#endif /* MAP_ANON */

#define RTL_MEMORY_ALLOC(n) __rtl_memory_vmalloc((size_t)(n))
#define RTL_MEMORY_FREE(p, n) munmap((void*)(p), (size_t)(n))

#endif /* SAL_UNX */

/*===========================================================================
 *
 * rtl_memory (W32) internals.
 *
 *=========================================================================*/
#ifdef SAL_W32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef CRITICAL_SECTION mutex_type;

/* Static initializer (struct declared in WINNT.H). */
#define RTL_MUTEX_INITIALIZER { NULL, -1, 0, NULL, NULL, 0 }

/*
 * __rtl_mutex_init (dynamic initialization).
 *
 * Static initialization (with DebugInfo == NULL)
 * leads to Access Violation upon first contention.
 */
static void __rtl_mutex_init (LPCRITICAL_SECTION lpCriticalSection)
{
    static LONG g_spinlock = 0;

    while (InterlockedExchange (&g_spinlock, 1) == 1)
    {
        /* Already locked, spin */
        Sleep (0);
    }
    if (!(lpCriticalSection->DebugInfo))
    {
        /* Dynamic initialization */
        InitializeCriticalSection (lpCriticalSection);
    }
    InterlockedExchange (&g_spinlock, 0);
}

#define RTL_MUTEX_INIT(a)  __rtl_mutex_init((LPCRITICAL_SECTION)(a))
#define RTL_MUTEX_ACQUIRE(a)  EnterCriticalSection((a))
#define RTL_MUTEX_RELEASE(a)  LeaveCriticalSection((a))

static sal_uInt32 __rtl_memory_vmpagesize (void)
{
    SYSTEM_INFO info;
    GetSystemInfo (&info);
    return ((sal_uInt32)(info.dwPageSize));
}

#define RTL_MEMORY_ALLOC(n) \
(void*)(VirtualAlloc (NULL, (SIZE_T)(n), MEM_COMMIT, PAGE_READWRITE))

#define RTL_MEMORY_FREE(p, n) \
(void)(VirtualFree ((LPVOID)(p), (SIZE_T)(0), MEM_RELEASE))

#endif /* SAL_W32 */

/*===========================================================================
 *
 * rtl_memory (queue) internals.
 *
 *=========================================================================*/
typedef struct queue_link queue_link;

struct queue_link
{
    queue_link * m_flink;
    queue_link * m_blink;
};

#define queue_start(entry) \
{ \
    (entry)->m_flink = (entry); \
    (entry)->m_blink = (entry); \
}

#define queue_remove(entry) \
{ \
    (entry)->m_blink->m_flink = (entry)->m_flink; \
    (entry)->m_flink->m_blink = (entry)->m_blink; \
    queue_start(entry); \
}

#define queue_insert_tail(head, entry) \
{ \
    (entry)->m_flink = (head); \
    (entry)->m_blink = (head)->m_blink; \
    (head)->m_blink = (entry); \
    (entry)->m_blink->m_flink = (entry); \
}

/*===========================================================================
 *
 * rtl_memory (global) internals.
 *
 *=========================================================================*/
#define __L__ 32
#define __P__ 24
#define __N__ (__L__ + __P__)

static const int __C__ = 2 * sizeof(size_t);
static const int __Q__ = 2 * sizeof(void*);

#if defined(DEBUG) || defined(_DEBUG)
static size_t queue (size_t n)
{
    /* k = n div 8 = n div __C__ */
    register size_t k = (n >> 3), m = __L__;
    if (k > m)
    {
        /* k = k div 32 = k div __L__ */
        k >>= 5;
        while ((k >>= 1) > 0) m++;
        k = m;
    }

    OSL_POSTCOND((0 < k) && (k < __N__),
                 "__rtl_memory_queue()[1]: "
                 "internal error: index out of bounds");
    return (k);
}
#else  /* PRODUCT */
#define queue(k, n) \
{ \
    (k) = ((n) >> 3); \
    if ((k) > __L__) \
    { \
        register size_t m = __L__; \
        (k) >>= 5; \
        while (((k) >>= 1) > 0) m++; \
        (k) = m; \
    } \
}
#endif /* DEBUG || PRODUCT */

typedef struct __rtl_memory_desc_st memory_type;

struct __rtl_memory_desc_st
{
    size_t       m_length;
    size_t       m_offset;
    memory_type *m_flink;
    memory_type *m_blink;
};

typedef struct __rtl_memory_stat_st
{
    size_t m_dequeue;
    size_t m_enqueue;
} memory_stat;

#define RTL_MEMORY_ALIGN(n, m) (((n) + ((m) - 1)) & ~((m) - 1))
#define RTL_MEMORY_SIZEOF(a) RTL_MEMORY_ALIGN(sizeof(a), sizeof(memory_type))

struct __rtl_memory_global_st
{
    sal_uInt32  m_magic;
    sal_uInt32  m_align;

    union {
        mutex_type m_lock;
        char       m_data[RTL_MEMORY_SIZEOF(mutex_type)];
    } m_mutex;

    memory_type m_alloc_head;
    memory_type m_queue_head[__N__];
    memory_stat m_queue_stat[__N__];
};

static struct __rtl_memory_global_st g_memory =
{
    0, 0, { RTL_MUTEX_INITIALIZER }
};

void SAL_CALL ___rtl_memory_init (void);
void SAL_CALL ___rtl_memory_fini (void);

#define RTL_MEMORY_ENTER() \
{ \
    if (!(g_memory.m_align)) ___rtl_memory_init(); \
    RTL_MUTEX_ACQUIRE(&(g_memory.m_mutex.m_lock)); \
}

#define RTL_MEMORY_LEAVE() \
{ \
    RTL_MUTEX_RELEASE(&(g_memory.m_mutex.m_lock)); \
}

/*===========================================================================
 *
 * rtl_memory (debug) internals.
 *
 *=========================================================================*/
#define DBG_MEMORY_K_DEQFILL 0x77777777
#define DBG_MEMORY_K_ENQFILL 0x33333333

#if defined(DEBUG) || defined(_DEBUG)

#define __dbg_memory_succ(entry, length) \
(memory_type*)((char*)((entry)) + ((length) & 0x7fffffff))

#define __dbg_memory_pred(entry, offset) \
(memory_type*)((char*)((entry)) - ((offset) & 0x7fffffff))

#define __dbg_memory_ensure(entry) (!((size_t)(entry) & 0x7))

/*
 * __dbg_memory_dequeue.
 */
static void __dbg_memory_dequeue (size_t n)
{
    register size_t *p, *q, k = queue(n);

    p = &(g_memory.m_queue_stat[k].m_dequeue); *p += 1;
    q = &(g_memory.m_queue_stat[k].m_enqueue);

    g_memory.m_queue_head[k].m_offset = ((*p) - (*q));
}

/*
 * __dbg_memory_enqueue.
 */
static void __dbg_memory_enqueue (size_t n)
{
    register size_t *p, *q, k = queue(n);

    p = &(g_memory.m_queue_stat[k].m_dequeue);
    q = &(g_memory.m_queue_stat[k].m_enqueue); *q += 1;

    g_memory.m_queue_head[k].m_offset = ((*p) - (*q));
}

/*
 * __dbg_memory_insert.
 */
static void __dbg_memory_insert (memory_type **ppMemory)
{
    register memory_type * succ;
    succ = __dbg_memory_succ (*ppMemory, sizeof(memory_type));

    succ->m_length = (*ppMemory)->m_length - sizeof(memory_type);
    succ->m_offset = (*ppMemory)->m_offset;

    queue_insert_tail (&(g_memory.m_alloc_head), (*ppMemory));
    (*ppMemory) = succ;
}

/*
 * __dbg_memory_remove.
 */
static void __dbg_memory_remove (memory_type **ppMemory)
{
    (*ppMemory) = __dbg_memory_pred (*ppMemory, sizeof(memory_type));
    queue_remove (*ppMemory);
}

/*
 * __dbg_memory_verify_chain.
 */
static int __dbg_memory_verify_chain (memory_type * x)
{
    if (!__dbg_memory_ensure(x))
    {
        OSL_ENSURE(0, "__rtl_memory_verify(): invalid pointer alignment.");
        return (0);
    }
    if (!__dbg_memory_ensure(x->m_length))
    {
        OSL_ENSURE(0, "__rtl_memory_verify(): dynamic memory corruption");
        return (0);
    }
    if (!__dbg_memory_ensure(x->m_offset))
    {
        OSL_ENSURE(0, "__rtl_memory_verify(): dynamic memory corruption");
        return (0);
    }
    if (!(x->m_length & 0x7fffffff))
    {
        OSL_ENSURE(0, "__rtl_memory_verify(): dynamic memory corruption");
        return (0);
    }
    return (1);
}

/*
 * __dbg_memory_verify_queue.
 */
static int __dbg_memory_verify_queue (memory_type * x)
{
    if (!__dbg_memory_ensure(x))
    {
        OSL_ENSURE(0, "__rtl_memory_verify(): invalid pointer alignment.");
        return (0);
    }
    if (!__dbg_memory_ensure(x->m_flink))
    {
        OSL_ENSURE(0, "__rtl_memory_verify(): free memory corruption");
        return (0);
    }
    if (!__dbg_memory_ensure(x->m_blink))
    {
        OSL_ENSURE(0, "__rtl_memory_verify(): free memory corruption");
        return (0);
    }
    if ((x == x->m_flink) || (x == x->m_blink))
    {
        OSL_ENSURE(0, "__rtl_memory_verify(): internal logic error");
        return (0);
    }
    return (1);
}

/*
 * __dbg_memory_verify.
 */
static int __dbg_memory_verify (memory_type * x, int debug)
{
    register memory_type *head, *entry;

    /* verify 'chain' fields */
    if (!__dbg_memory_verify_chain (x))
        return (0);
    if (!debug)
        goto verify_leave;

    /* verify allocation */
    head = entry = &(g_memory.m_alloc_head);
    while (!((entry = entry->m_flink) == head))
    {
        if ((entry < (x)) && ((x) < __dbg_memory_succ(entry, entry->m_length)))
        {
            head = entry = __dbg_memory_succ(entry, sizeof(memory_type));
            while (!((x == entry) || (entry->m_offset & 0x80000000)))
            {
                /* no match, not last */
                if (!__dbg_memory_verify_chain (entry))
                    return (0);
                entry = __dbg_memory_succ(entry, entry->m_length);
            }

            /* match, or last */
            if (!__dbg_memory_verify_chain (entry))
                return (0);
            break;
        }
    }
    if (!(x == entry))
    {
        OSL_ENSURE(0, "__rtl_memory_verify(): memory not allocated.");
        return (0);
    }

verify_leave:
    /* verify 'used' bit */
    if (!(x->m_length & 0x80000000))
    {
        OSL_ENSURE(0, "__rtl_memory_verify(): memory not used.");
        return (0);
    }
    return (1);
}

/*
 * __dbg_memory_usage.
 */
static void __dbg_memory_usage (memory_stat * total)
{
    register memory_type *head, *entry, *memory;
    memory_stat           stat[__N__];
    size_t                i;

    total->m_dequeue = total->m_enqueue = 0;
    for (i = 0; i < __N__; i++)
        stat[i].m_dequeue = stat[i].m_enqueue = 0;

    head = entry = &(g_memory.m_alloc_head);
    while (!((entry = entry->m_flink) == head))
    {
        register size_t k = 0, n = entry->m_length - sizeof(memory_type);

        memory = __dbg_memory_succ(entry, sizeof(memory_type));
        while (!(memory->m_offset & 0x80000000))
        {
            /* not last */
            if (!(memory->m_length & 0x80000000))
            {
                /* not used */
                k += memory->m_length;
            }
            memory = __dbg_memory_succ(memory, memory->m_length);
        }
        if (!(memory->m_length & 0x80000000))
        {
            /* not used */
            k += memory->m_length;
        }

        i = queue(n);
        stat[i].m_dequeue += n;
        stat[i].m_enqueue += k;

        OSL_TRACE("%08x %10d %10d", (size_t)(entry), n, k);
        total->m_dequeue += n;
        total->m_enqueue += k;
    }
}

#endif /* DEBUG */
#if defined(DEBUG) || defined(_DEBUG)

#define DBG_MEMORY_ALIGN(n, m) RTL_MEMORY_ALIGN((n) + sizeof(memory_type), (m))

#define DBG_MEMORY_DEQUEUE(n) __dbg_memory_dequeue((size_t)(n) & 0x7fffffff)
#define DBG_MEMORY_ENQUEUE(n) __dbg_memory_enqueue((size_t)(n) & 0x7fffffff)

#define DBG_MEMORY_INSERT(entry) __dbg_memory_insert((entry))
#define DBG_MEMORY_REMOVE(entry) __dbg_memory_remove((entry))

#if defined(DEBUG)
#define DBG_MEMORY_VERIFY(entry) __dbg_memory_verify((entry), 1)
#else  /* _DEBUG */
#define DBG_MEMORY_VERIFY(entry) __dbg_memory_verify((entry), 0)
#endif /* _DEBUG */

#define DBG_MEMORY_VERIFY_CHAIN(entry) __dbg_memory_verify_chain((entry))
#define DBG_MEMORY_VERIFY_QUEUE(entry) __dbg_memory_verify_queue((entry))

#else  /* PRODUCT */

#define DBG_MEMORY_ALIGN(n, m) RTL_MEMORY_ALIGN((n), (m))
#define DBG_MEMORY_DEQUEUE(n)
#define DBG_MEMORY_ENQUEUE(n)
#define DBG_MEMORY_INSERT(entry)
#define DBG_MEMORY_REMOVE(entry)
#define DBG_MEMORY_VERIFY(entry)
#define DBG_MEMORY_VERIFY_CHAIN(entry)
#define DBG_MEMORY_VERIFY_QUEUE(entry)

#endif /* DEBUG || PRODUCT */

/*===========================================================================
 *
 * rtl_memory (manager) internals.
 *
 *=========================================================================*/
#define queue_cast(entry, offset) \
((memory_type*)((char*)(entry) + (ptrdiff_t)(offset)))

#define __rtl_memory_used(entry) ((entry)->m_length & 0x80000000)
#define __rtl_memory_last(entry) ((entry)->m_offset & 0x80000000)

/*
 * ___rtl_memory_init.
 */
void SAL_CALL ___rtl_memory_init (void)
{
#if defined(RTL_MUTEX_INIT)
    RTL_MUTEX_INIT (&(g_memory.m_mutex.m_lock));
#endif /* RTL_MUTEX_INIT */

    RTL_MUTEX_ACQUIRE(&(g_memory.m_mutex.m_lock));
    if (!(g_memory.m_align))
    {
        int i;

        queue_start (&(g_memory.m_alloc_head));
        for (i = 0; i < __N__; i++)
            queue_start (&(g_memory.m_queue_head[i]));
        for (i = 1; i <= __L__; i++)
            g_memory.m_queue_head[i].m_length = (i << 3);
        for (i = 1; i < __P__; i++)
            g_memory.m_queue_head[__L__ + i].m_length = ((1 << (i+5)) << 3);

        g_memory.m_align = __rtl_memory_vmpagesize();
    }
    RTL_MUTEX_RELEASE(&(g_memory.m_mutex.m_lock));
}

/*
 * ___rtl_memory_fini (NYI).
 */
void SAL_CALL ___rtl_memory_fini (void)
{
#if defined(DEBUG)

    memory_stat total;
    size_t      usage;

    __dbg_memory_usage (&total);
    if ((usage = total.m_dequeue - total.m_enqueue) > 0)
    {
        OSL_TRACE("___rtl_memory_fini(): "
                  "Leak: %10d (Alloc: %10d, Free: %10d)",
                  usage, total.m_dequeue, total.m_enqueue);
    }

#endif /* DEBUG */
}

/*
 * __rtl_memory_merge.
 */
#if defined(DEBUG) || defined(_DEBUG)
static void __rtl_memory_merge (memory_type * prev, memory_type * next)
{
    /* adjust length */
    prev->m_length += next->m_length;
    if (!__rtl_memory_last(next))
    {
        /* not last, adjust offset */
        register memory_type * succ = queue_cast(prev, prev->m_length);
        __dbg_memory_verify_chain (succ);
        succ->m_offset = prev->m_length | __rtl_memory_last(succ);
    }

    /* propagate 'last' bit */
    prev->m_offset |= __rtl_memory_last(next);
}
#else  /* PRODUCT */
#define __rtl_memory_merge(prev, next) \
{ \
    (prev)->m_length += (next)->m_length; \
    if (!__rtl_memory_last((next))) \
    { \
        register memory_type * succ = queue_cast((prev), (prev)->m_length); \
        succ->m_offset = (prev)->m_length | __rtl_memory_last(succ); \
    } \
    (prev)->m_offset |= __rtl_memory_last((next)); \
}
#endif /* DEBUG || PRODUCT */

/*
 * __rtl_memory_split.
 */
#if defined(DEBUG) || defined(_DEBUG)
static void __rtl_memory_split (memory_type * prev, memory_type * next)
{
    /* adjust length */
    prev->m_length -= next->m_length;
    if (!__rtl_memory_last(prev))
    {
        /* not last, adjust offset */
        register memory_type * succ = queue_cast(next, next->m_length);
        __dbg_memory_verify_chain (succ);
        succ->m_offset = next->m_length | __rtl_memory_last(succ);
    }

    /* propagate 'last' bit */
    next->m_offset |= __rtl_memory_last(prev);
    prev->m_offset &= ~0x80000000;
}
#else  /* PRODUCT */
#define __rtl_memory_split(prev, next) \
{ \
    (prev)->m_length -= (next)->m_length; \
    if (!__rtl_memory_last((prev))) \
    { \
        register memory_type * succ = queue_cast((next), (next)->m_length); \
        succ->m_offset = (next)->m_length | __rtl_memory_last(succ); \
    } \
\
    (next)->m_offset |= __rtl_memory_last((prev)); \
    (prev)->m_offset &= ~0x80000000; \
}
#endif /* DEBUG || PRODUCT */

/*
 * __rtl_memory_queue.
 */
#if defined(DEBUG) || defined(_DEBUG)
static memory_type* __rtl_memory_queue (size_t n)
{
    /* k = n div 8 = n div __C__ */
    register size_t k = (n >> 3), m = __L__;
    if (k > m)
    {
        /* k = k div 32 = k div __L__ */
        k >>= 5;
        while ((k >>= 1) > 0) m++;
        k = m;
    }

    OSL_POSTCOND((0 < k) && (k < __N__),
                 "__rtl_memory_queue()[2]: "
                 "internal error: index out of bounds");
    return (&(g_memory.m_queue_head[k]));
}
#else  /* PRODUCT */
#define __rtl_memory_queue(h, n) \
{ \
    register size_t k = ((n) >> 3), m = __L__; \
    if (k > m) \
    { \
        k >>= 5; \
        while ((k >>= 1) > 0) m++; \
        k = m; \
    } \
    (h) = &(g_memory.m_queue_head[k]); \
}
#endif /* DEBUG || PRODUCT */

/*
 * __rtl_memory_insert.
 */
#if defined(DEBUG) || defined(_DEBUG)
static void __rtl_memory_insert (memory_type * memory, size_t n)
{
    register memory_type *head, *entry;
    head = __rtl_memory_queue(n);

    OSL_PRECOND(__dbg_memory_ensure(head->m_flink),
                "__rtl_memory_insert(): "
                "static memory corruption.");
    OSL_PRECOND(__dbg_memory_ensure(head->m_blink),
                "__rtl_memory_insert(): "
                "static memory corruption.");

    for (entry = head->m_flink; entry != head; entry = entry->m_flink)
    {
        __dbg_memory_verify_chain (entry);
        __dbg_memory_verify_queue (entry);

        if (entry->m_length >= n)
            break;
    }

    queue_insert_tail (entry, memory);
}
#else  /* PRODUCT */
#define __rtl_memory_insert(memory, n) \
{ \
    register memory_type *head, *entry; \
\
    __rtl_memory_queue (head, (n)); \
    for (entry = head->m_flink; entry != head; entry = entry->m_flink) \
        if (entry->m_length >= (n)) \
            break; \
\
    queue_insert_tail (entry, (memory)); \
}
#endif /* DEBUG || PRODUCT */

/*
 * __rtl_memory_resize.
 */
#if defined(DEBUG) || defined(_DEBUG)
static void __rtl_memory_resize (memory_type * memory, size_t n)
{
    register size_t k = (memory->m_length - n);

    OSL_ENSURE(!(memory->m_length & 0x80000000),
               "__rtl_memory_resize(): "
               "internal logic error.");

    if (k >= sizeof(memory_type))
    {
        /* split */
        register memory_type * remain = queue_cast(memory, n);

        remain->m_length = k; remain->m_offset = n;
        __rtl_memory_split (memory, remain);

        /* check postcond */
        if (!(remain->m_offset & 0x80000000))
        {
            /* not last, verify used next entry */
            register memory_type *next;

            next = __dbg_memory_succ(remain, remain->m_length);
            __dbg_memory_verify_chain (next);

            OSL_POSTCOND(next->m_length & 0x80000000,
                         "__rtl_memory_resize(): "
                         "internal logic error.");
        }

        /* fill w/ 'deinitialized' pattern */
        memset (
            ((char*)(remain)  + __C__),
            DBG_MEMORY_K_ENQFILL,
            (remain->m_length - __C__));

        /* enqueue */
        __rtl_memory_insert (remain, k);
        __dbg_memory_verify_queue (remain);
    }
}
#else  /* PRODUCT */
#define __rtl_memory_resize(memory, n) \
{ \
    register size_t kn = ((memory)->m_length - (n)); \
    if (kn >= sizeof(memory_type)) \
    { \
        register memory_type * remain = queue_cast((memory), (n)); \
\
        remain->m_length = kn; remain->m_offset = (n); \
        __rtl_memory_split ((memory), remain); \
\
        __rtl_memory_insert (remain, kn); \
    } \
}
#endif /* DEBUG || PRODUCT */

/*
 * __rtl_memory_dequeue.
 */
#if defined(DEBUG) || defined(_DEBUG)
static void __rtl_memory_dequeue (memory_type **ppMemory, size_t n)
{
    register memory_type *head, *entry;
    register size_t       k, m = n;

    OSL_PRECOND(!*ppMemory, "__rtl_memory_dequeue(): internal logic error.");
    for (k = queue(m); k < __N__; k++)
    {
        head = &(g_memory.m_queue_head[k]);
        for (entry = head->m_flink; entry != head; entry = entry->m_flink)
        {
            OSL_ENSURE(!(entry->m_length & 0x80000000),
                       "__rtl_memory_dequeue(): "
                       "dynamic memory corruption.");
            if (entry->m_length >= m)
            {
                /* remove entry */
                __dbg_memory_verify_queue (entry);
                queue_remove (entry);

                /* assign result */
                *ppMemory = entry;
                goto dequeue_leave;
            }
        }
    }

    k = DBG_MEMORY_ALIGN(m, g_memory.m_align);
    if (!((entry = RTL_MEMORY_ALLOC(k)) == 0))
    {
        entry->m_length = k;
        entry->m_offset = 0x80000000; /* set 'last' bit */

        *ppMemory = entry;
        DBG_MEMORY_INSERT(ppMemory);
    }

dequeue_leave:
    OSL_POSTCOND(*ppMemory, "__rtl_memory_dequeue(): out of memory.");
    if (*ppMemory)
    {
        /* fill w/ 'uninitialized' pattern */
        memset (
            ((char*)(*ppMemory)    + __C__),
            DBG_MEMORY_K_DEQFILL,
            ((*ppMemory)->m_length - __C__));
    }
}
#else  /* PRODUCT */
#define __rtl_memory_dequeue(ppMemory, n, label) \
{ \
    register memory_type *head, *entry; \
    register size_t       h, m = (n); \
\
    queue (h, m); \
    for (; h < __N__; h++) \
    { \
        head = &(g_memory.m_queue_head[h]); \
        for (entry = head->m_flink; entry != head; entry = entry->m_flink) \
        { \
            if (entry->m_length >= m) \
            { \
                queue_remove (entry); \
                goto label; \
            } \
        } \
    } \
\
    h = RTL_MEMORY_ALIGN(m, g_memory.m_align); \
    if (!((entry = RTL_MEMORY_ALLOC(h)) == 0)) \
    { \
        entry->m_length = h; \
        entry->m_offset = 0x80000000; \
    } \
\
label: \
    (*(ppMemory)) = entry; \
}
#endif /* DEBUG || PRODUCT */

#if defined(DEBUG) || defined(_DEBUG)
#define RTL_MEMORY_DEQUEUE(m, n, l) __rtl_memory_dequeue((m), (n))
#else  /* PRODUCT */
#define RTL_MEMORY_DEQUEUE(m, n, l) __rtl_memory_dequeue(m, n, l)
#endif /* DEBUG || PRODUCT */

/*
 * __rtl_memory_enqueue.
 */
#if defined(DEBUG) || defined(_DEBUG)
static void __rtl_memory_enqueue (memory_type **ppMemory)
{
    register memory_type *head = *ppMemory;

    OSL_ENSURE(!(head->m_length & 0x80000000),
               "__rtl_memory_enqueue(): "
               "internal logic error.");

    DBG_MEMORY_ENQUEUE(head->m_length);
    if (!(head->m_offset & 0x80000000))
    {
        /* not last, try merge w/ next entry */
        register memory_type * next;

        next = (memory_type*)((char*)head + head->m_length);
        __dbg_memory_verify_chain (next);

        if (!(next->m_length & 0x80000000))
        {
            /* next not used */
            __dbg_memory_verify_queue (next);
            queue_remove (next);

            /* merge w/ next */
            __rtl_memory_merge (head, next);
        }
    }

    if ((head->m_offset & 0x7fffffff) > 0)
    {
        /* not first, try merge w/ prev entry */
        register memory_type * prev;

        prev = (memory_type*)((char*)head - (head->m_offset & 0x7fffffff));
        __dbg_memory_verify_chain (prev);

        if (!(prev->m_length & 0x80000000))
        {
            /* prev not used */
            __dbg_memory_verify_queue (prev);
            queue_remove (prev);

            /* merge w/ prev */
            __rtl_memory_merge (prev, head);
            head = prev;
        }
    }

    if (!(head->m_offset == 0x80000000))
    {
        /* fill w/ 'deinitialized' pattern */
        memset (
            ((char*)(head)  + __C__),
            DBG_MEMORY_K_ENQFILL,
            (head->m_length - __C__));

        /* not free, enqueue */
        __rtl_memory_insert (head, head->m_length);
        (*ppMemory) = 0;
    }
    else
    {
        /* free, remove */
        (*ppMemory) = head;
        DBG_MEMORY_REMOVE(ppMemory);
    }
}
#else  /* PRODUCT */
#define __rtl_memory_enqueue(ppMemory) \
{ \
    register memory_type *head = *(ppMemory); \
\
    if (!(head->m_offset & 0x80000000)) \
    { \
        register memory_type * next; \
        next = (memory_type*)((char*)head + head->m_length); \
        if (!(next->m_length & 0x80000000)) \
        { \
            queue_remove (next); \
            __rtl_memory_merge (head, next); \
        } \
    } \
\
    if ((head->m_offset & 0x7fffffff) > 0) \
    { \
        register memory_type * prev; \
        prev = (memory_type*)((char*)head - (head->m_offset & 0x7fffffff)); \
        if (!(prev->m_length & 0x80000000)) \
        { \
            queue_remove (prev); \
            __rtl_memory_merge (prev, head); \
            head = prev; \
        } \
    } \
\
    if (!(head->m_offset == 0x80000000)) \
    { \
        register memory_type * used = head; \
        __rtl_memory_insert (used, used->m_length); \
        head = 0; \
    } \
\
    *(ppMemory) = head; \
}
#endif /* DEBUG || PRODUCT */

#define RTL_MEMORY_ENQUEUE(m) __rtl_memory_enqueue((m))

/*===========================================================================
 *
 * rtl_memory (manager) implementation.
 *
 *=========================================================================*/
/*
 * rtl_reallocateMemory.
 */
#ifndef FORCE_SYSALLOC
void* SAL_CALL rtl_reallocateMemory (void * p, sal_uInt32 n) SAL_THROW_EXTERN_C()
{
    memory_type * memory;
    if (!(!p || !n))
    {
        /* reallocate */
        register int d;

        memory = queue_cast(p, -(__C__)); p = 0;
        n = RTL_MEMORY_ALIGN(n, __Q__) + __C__;

        RTL_MEMORY_ENTER();
        DBG_MEMORY_VERIFY(memory);

        /* clear 'used' bit */
        memory->m_length &= 0x7fffffff;

        /* dispatch */
        d = n - memory->m_length;
        if (d > 0)
        {
            /* extend */
            if (!(memory->m_offset & 0x80000000))
            {
                /* not last, try merge w/ next entry */
                register memory_type * next;

                next = queue_cast(memory, memory->m_length);
                DBG_MEMORY_VERIFY_CHAIN(next);

                if (!(next->m_length & 0x80000000))
                {
                    /* next not used */
                    if ((memory->m_length + next->m_length) >= n)
                    {
                        /* next does fit */
                        DBG_MEMORY_VERIFY_QUEUE(next);
                        queue_remove (next);

                        /* merge w/ next */
                        DBG_MEMORY_ENQUEUE(memory->m_length);
                        __rtl_memory_merge (memory, next);

                        /* adjust, set 'used' bit */
                        __rtl_memory_resize (memory, n);
                        memory->m_length |= 0x80000000;

                        /* assign result */
                        DBG_MEMORY_DEQUEUE(memory->m_length);
                        p = queue_cast(memory, __C__);
                    }
                }
            }
            if (!p)
            {
                /* set 'used' bit, allocate */
                memory_type * next = 0;
                memory->m_length |= 0x80000000;

                RTL_MEMORY_DEQUEUE (&next, n, realloc_label_1);
                if (next)
                {
                    /* adjust, set 'used' bit */
                    __rtl_memory_resize (next, n);
                    next->m_length |= 0x80000000;

                    /* copy */
                    memcpy (
                        ((char*)next   + __C__),
                        ((char*)memory + __C__),
                        ((memory->m_length & 0x7fffffff) - __C__));

                    /* assign result */
                    DBG_MEMORY_DEQUEUE(next->m_length);
                    p = queue_cast(next, __C__);
                }

                /* clear 'used' bit, enqueue */
                memory->m_length &= 0x7fffffff;

                RTL_MEMORY_ENQUEUE (&memory);
                if (memory)
                {
                    /* free memory page */
                    RTL_MEMORY_FREE(memory, memory->m_length);
                }
            }
        }
        else if (d < 0)
        {
            /* shrink */
            DBG_MEMORY_ENQUEUE(memory->m_length);
            if (!(memory->m_offset & 0x80000000))
            {
                /* not last, try merge w/ next entry */
                register memory_type * next;

                next = queue_cast(memory, memory->m_length);
                DBG_MEMORY_VERIFY_CHAIN(next);

                if (!(next->m_length & 0x80000000))
                {
                    /* next not used */
                    DBG_MEMORY_VERIFY_QUEUE (next);
                    queue_remove (next);

                    /* merge w/ next */
                    __rtl_memory_merge (memory, next);
                }
            }

            /* adjust, set 'used' bit */
            __rtl_memory_resize (memory, n);
            memory->m_length |= 0x80000000;

            /* assign result */
            DBG_MEMORY_DEQUEUE(memory->m_length);
            p = queue_cast(memory, __C__);
        }
        else
        {
            /* nop, set 'used' bit */
            memory->m_length |= 0x80000000;

            /* assign result */
            p = queue_cast(memory, __C__);
        }
        RTL_MEMORY_LEAVE();
    }
    else if (!p)
    {
        /* allocate */
        memory = 0;
        n = RTL_MEMORY_ALIGN(n, __Q__) + __C__;

        RTL_MEMORY_ENTER();
        RTL_MEMORY_DEQUEUE (&memory, n, realloc_label_2);
        if (memory)
        {
            /* adjust, set 'used' bit */
            __rtl_memory_resize (memory, n);
            memory->m_length |= 0x80000000;

            /* assign result */
            DBG_MEMORY_DEQUEUE(memory->m_length);
            p = queue_cast(memory, __C__);
        }
        RTL_MEMORY_LEAVE();
    }
    else if (!n)
    {
        /* free */
        memory = queue_cast(p, -(__C__)); p = 0;

        RTL_MEMORY_ENTER();
        DBG_MEMORY_VERIFY(memory);

        /* clear 'used' bit, enqueue */
        memory->m_length &= 0x7fffffff;

        RTL_MEMORY_ENQUEUE (&memory);
        if (memory)
        {
            /* free memory page */
            RTL_MEMORY_FREE(memory, memory->m_length);
        }
        RTL_MEMORY_LEAVE();
    }
    return (p);
}
#else  /* FORCE_SYSALLOC */
void* SAL_CALL rtl_reallocateMemory (void * p, sal_uInt32 n) SAL_THROW_EXTERN_C()
{
    return realloc(p, (size_t)(n));
}
#endif /* FORCE_SYSALLOC */

/*
 * rtl_allocateMemory.
 */
#ifndef FORCE_SYSALLOC
void* SAL_CALL rtl_allocateMemory (sal_uInt32 n) SAL_THROW_EXTERN_C()
{
    void * p = 0;
    if (n > 0)
    {
        memory_type * memory = 0;
        n = RTL_MEMORY_ALIGN(n, __Q__) + __C__;

        RTL_MEMORY_ENTER();
        RTL_MEMORY_DEQUEUE (&memory, n, alloc_label);
        if (memory)
        {
            /* adjust, set 'used' bit */
            __rtl_memory_resize (memory, n);
            memory->m_length |= 0x80000000;

            /* assign result */
            DBG_MEMORY_DEQUEUE(memory->m_length);
            p = queue_cast(memory, __C__);
        }
        RTL_MEMORY_LEAVE();
    }
    return (p);
}
#else  /* FORCE_SYSALLOC */
void* SAL_CALL rtl_allocateMemory (sal_uInt32 n) SAL_THROW_EXTERN_C()
{
    return malloc((size_t)(n));
}
#endif /* FORCE_SYSALLOC */

/*
 * rtl_freeMemory.
 */
#ifndef FORCE_SYSALLOC
void SAL_CALL rtl_freeMemory (void * p) SAL_THROW_EXTERN_C()
{
    if (p)
    {
        memory_type * memory = queue_cast(p, -(__C__));

        RTL_MEMORY_ENTER();
        DBG_MEMORY_VERIFY(memory);

        /* clear 'used' bit, enqueue */
        memory->m_length &= 0x7fffffff;

        RTL_MEMORY_ENQUEUE (&memory);
        if (memory)
        {
            /* free memory page */
            RTL_MEMORY_FREE(memory, memory->m_length);
        }
        RTL_MEMORY_LEAVE();
    }
}
#else  /* FORCE_SYSALLOC */
void SAL_CALL rtl_freeMemory (void * p) SAL_THROW_EXTERN_C()
{
    free(p);
}
#endif /* FORCE_SYSALLOC */

/*
 * rtl_allocateZeroMemory.
 */
#ifndef FORCE_SYSALLOC
void* SAL_CALL rtl_allocateZeroMemory (sal_uInt32 n) SAL_THROW_EXTERN_C()
{
    void * p = 0;
    if (n > 0)
    {
        memory_type * memory = 0;
        n = RTL_MEMORY_ALIGN(n, __Q__) + __C__;

        RTL_MEMORY_ENTER();
        RTL_MEMORY_DEQUEUE (&memory, n, alloc_label); /* NYI: demand zero */
        if (memory)
        {
            /* adjust, zero, set 'used' bit */
            __rtl_memory_resize (memory, n);
            memset ((char*)memory + __C__, 0, memory->m_length - __C__);
            memory->m_length |= 0x80000000;

            /* assign result */
            DBG_MEMORY_DEQUEUE(memory->m_length);
            p = queue_cast(memory, __C__);
        }
        RTL_MEMORY_LEAVE();
    }
    return (p);
}
#else  /* FORCE_SYSALLOC */
void* SAL_CALL rtl_allocateZeroMemory (sal_uInt32 n) SAL_THROW_EXTERN_C()
{
    return calloc((size_t)(n), 1);
}
#endif /* FORCE_SYSALLOC */

/*
 * rtl_freeZeroMemory.
 */
#ifndef FORCE_SYSALLOC
void SAL_CALL rtl_freeZeroMemory (void * p, sal_uInt32 n) SAL_THROW_EXTERN_C()
{
    if (p)
    {
        memory_type * memory = queue_cast(p, -(__C__));

        RTL_MEMORY_ENTER();
        DBG_MEMORY_VERIFY(memory);

        /* clear 'used' bit, zero, enqueue */
        memory->m_length &= 0x7fffffff;
        memset ((char*)memory + __C__, 0, memory->m_length - __C__);

        RTL_MEMORY_ENQUEUE (&memory); /* NYI: demand zero */
        if (memory)
        {
            /* free memory page */
            RTL_MEMORY_FREE(memory, memory->m_length);
        }
        RTL_MEMORY_LEAVE();
    }
}
#else  /* FORCE_SYSALLOC */
void SAL_CALL rtl_freeZeroMemory (void * p, sal_uInt32 n) SAL_THROW_EXTERN_C()
{
    if (p)
    {
        memset(p, 0, n);
        free(p);
    }
}
#endif /* FORCE_SYSALLOC */

/*===========================================================================
 *
 * The End.
 *
 *=========================================================================*/
