/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifdef PROFILE
#undef OSL_DEBUG_LEVEL
#define OSL_DEBUG_LEVEL 0
#endif /* PROFILE */

#include <sal/types.h>
#include <osl/diagnose.h>
#include <rtl/alloc.h>

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

#ifndef FORCE_SYSALLOC

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

#if defined(FREEBSD) || defined(NETBSD) || defined(MACOSX) || \
    defined(OPENBSD)
static sal_Size __rtl_memory_vmpagesize (void)
{
    /* xBSD */
    return (sal_Size)(getpagesize());
}
#elif defined(LINUX) || defined(SOLARIS) || defined(AIX)
static sal_Size __rtl_memory_vmpagesize (void)
{
    /* POSIX */
    return (sal_Size)(sysconf(_SC_PAGESIZE));
}
#else
static sal_Size __rtl_memory_vmpagesize (void)
{
    /* other */
    return (sal_Size)(0x2000);
}
#endif /* FREEBSD || NETBSD || MACOSX || LINUX || SOLARIS || AIX || OPENBSD*/

#ifndef PROT_HEAP
#define PROT_HEAP (PROT_READ | PROT_WRITE | PROT_EXEC)
#endif

/* #95880# building on Solaris 8 provides MAP_ANON, but it
   is not available on Solaris 7 */
#if defined (SOLARIS)
#ifdef MAP_ANON
#undef MAP_ANON
#endif
#endif

#ifndef MAP_ANON
static void* __rtl_memory_vmalloc (sal_Size n)
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
static void* __rtl_memory_vmalloc (sal_Size n)
{
    /* xBSD */
    void * p = mmap(NULL, n, PROT_HEAP, MAP_PRIVATE | MAP_ANON, -1, 0);
    return ((p == MAP_FAILED) ? NULL : p);
}
#endif /* MAP_ANON */

#define RTL_MEMORY_ALLOC(n) __rtl_memory_vmalloc((sal_Size)(n))
#define RTL_MEMORY_FREE(p, n) munmap((void*)(p), (sal_Size)(n))

#endif /* SAL_UNX */

/*===========================================================================
 *
 * rtl_memory (W32) internals.
 *
 *=========================================================================*/
#ifdef SAL_W32

#define WIN32_LEAN_AND_MEAN
#ifdef _MSC_VER
#pragma warning(push,1) /* disable warnings within system headers */
#endif
#include <windows.h>
#include <wchar.h>

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

static sal_Size __rtl_memory_vmpagesize (void)
{
    SYSTEM_INFO info;
    GetSystemInfo (&info);
    return ((sal_Size)(info.dwPageSize));
}

#define RTL_MEMORY_ALLOC(n) \
(void*)(VirtualAlloc (NULL, (SIZE_T)(n), MEM_COMMIT, PAGE_READWRITE))

#define RTL_MEMORY_FREE(p, n) \
(void)(VirtualFree ((LPVOID)(p), (SIZE_T)(0), MEM_RELEASE))

#endif /* SAL_W32 */

/*===========================================================================
 *
 * rtl_memory (OS2) internals.
 *
 *=========================================================================*/
#ifdef SAL_OS2

#define INCL_DOS
#include <os2.h>

typedef HMTX mutex_type;

/* Static initializer */
#define RTL_MUTEX_INITIALIZER -1

/*
 * __rtl_mutex_init (dynamic initialization).
 *
 * Static initialization (with DebugInfo == NULL)
 * leads to Access Violation upon first contention.
 */
static void __rtl_mutex_init (mutex_type* mutex)
{
    APIRET      rc = 0;

    rc = DosCreateMutexSem(NULL,mutex,0,0);

}

static int __rtl_mutex_destroy (mutex_type* mutex)
{
    APIRET      rc = 0;


    do {
        rc = DosCloseMutexSem(*mutex);
        if (rc == 301) DosReleaseMutexSem(*mutex);
    } while (rc == 301);

    *mutex = 0;

    /* Return the completion status: */
    return (0);
}


static int __rtl_mutex_acquire(mutex_type* mutex)
{
    int     ret = 0;
    int     status = 0;
    APIRET      rc = 0;

    // initialize static semaphores created with PTHREAD_MUTEX_INITIALIZER state.
    if (*mutex == -1)
        __rtl_mutex_init( mutex);

    rc = DosRequestMutexSem(*mutex,SEM_INDEFINITE_WAIT);
    if (rc)
        return(1);

    /* Return the completion status: */
    return (0);
}

static int __rtl_mutex_release(mutex_type* mutex)
{
    int     ret = 0;
    APIRET      rc = 0;
    int     status;


    // initialize static semaphores created with PTHREAD_MUTEX_INITIALIZER state.
    if (*mutex == -1)
       __rtl_mutex_init( mutex);

    rc = DosReleaseMutexSem(*mutex);

    /* Return the completion status: */
    return (0);
}

#define RTL_MUTEX_INIT(a)  __rtl_mutex_init((mutex_type*)(a))
#define RTL_MUTEX_ACQUIRE(a)  __rtl_mutex_acquire((mutex_type*)(a))
#define RTL_MUTEX_RELEASE(a)  __rtl_mutex_release((mutex_type*)(a))

static sal_Size __rtl_memory_vmpagesize (void)
{
    return (sal_Size)(getpagesize());
}

#define RTL_MEMORY_ALLOC(n) (void*)(malloc(n))

#define RTL_MEMORY_FREE(p, n) (void)(free(p))

#endif /* SAL_OS2 */

/*===========================================================================
 *
 * Determine allocation mode (debug/release) by examining unix
 * environment variable "G_SLICE"
 *
 *=========================================================================*/

#include <stdlib.h>   /* getenv */
#include <stdio.h>    /* stderr */

typedef
   enum { AMode_CUSTOM, AMode_SYSTEM, AMode_UNSET }
   AllocMode;

static AllocMode alloc_mode = AMode_UNSET;

static void determine_alloc_mode ( void )
{
   /* This shouldn't happen, but still ... */
   if (alloc_mode != AMode_UNSET)
      return;

   if (getenv("G_SLICE") != NULL) {
      alloc_mode = AMode_SYSTEM;
      fprintf(stderr, "OOo: Using system memory allocator.\n");
      fprintf(stderr, "OOo: This is for debugging only.  To disable,\n");
      fprintf(stderr, "OOo: unset the environment variable G_SLICE.\n");
   } else {
      alloc_mode = AMode_CUSTOM;
   }
}

/*===========================================================================
 *
 * rtl_memory (global) internals.
 *
 *=========================================================================*/
#define __L__ 32
#define __P__ 24
#define __N__ ((__L__) + (__P__))
#define __M__ 0x10000

static const sal_Size __T__ = (__M__) * 2 / 3;

typedef struct __rtl_memory_desc_st memory_type;
struct __rtl_memory_desc_st
{
    sal_Size       m_length;
    sal_Size       m_offset;
    memory_type *m_flink;
    memory_type *m_blink;
};

static const int __C__ = 2 * sizeof(sal_Size);
static const int __Q__ = 2 * sizeof(memory_type*);

typedef struct __rtl_memory_stat_st memory_stat;
struct __rtl_memory_stat_st
{
    sal_uInt64 m_dequeue;
    sal_uInt64 m_enqueue;
    sal_Int32  m_delta_q;

    sal_uInt64 m_deqsize;
    sal_uInt64 m_enqsize;
    sal_Int32  m_delta_n;
};

#define RTL_MEMORY_ALIGN(n, m) (((n) + ((m) - 1)) & ~((m) - 1))
#define RTL_MEMORY_SIZEOF(a) RTL_MEMORY_ALIGN(sizeof(a), sizeof(memory_type))

struct __rtl_memory_global_st
{
    sal_Size  m_magic;
    sal_Size  m_align;

    union {
        mutex_type m_lock;
        char       m_data[RTL_MEMORY_SIZEOF(mutex_type)];
    } m_mutex;

    memory_type    m_alloc_head;
    memory_type    m_spare_head;
    memory_type    m_queue_head[__N__];

#if OSL_DEBUG_LEVEL > 0
    memory_stat    m_queue_stat[__N__];
#endif /* OSL_DEBUG_LEVEL */
};

static struct __rtl_memory_global_st g_memory =
{
    0, 0, { RTL_MUTEX_INITIALIZER },
    { 0, 0, NULL, NULL }, { 0, 0, NULL, NULL }, { { 0, 0, NULL, NULL } },
#if OSL_DEBUG_LEVEL > 0
    { { 0, 0, 0, 0, 0, 0 } }
#endif /* OSL_DEBUG_LEVEL */
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
 * rtl_memory (queue) internals.
 *
 *=========================================================================*/
#if defined(PROFILE) || (OSL_DEBUG_LEVEL > 0)
static sal_Size queue (sal_Size n)
{
    /* k = n div __C__ */
    register sal_Size k = n / __C__, m = __L__;

    OSL_PRECOND((__L__ == 32),
                "__rtl_memory_queue(): internal logic error");
    if (k > m)
    {
        /* k = k div __L__ = k div 32 */
        k >>= 5;
        while ((k >>= 1) > 0) m++;
        k = m;
    }

    OSL_POSTCOND((0 < k) && (k < __N__),
                 "__rtl_memory_queue(): "
                 "internal error: index out of bounds");
    return (k);
}
#else  /* PRODUCT */
#define queue(k, n) \
{ \
    (k) = ((n) / __C__); \
    if ((k) > __L__) \
    { \
        register sal_Size m = __L__; \
        (k) >>= 5; \
        while (((k) >>= 1) > 0) m++; \
        (k) = m; \
    } \
}
#endif /* OSL_DEBUG_LEVEL || PRODUCT */

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

#define queue_insert_head(head, entry) \
{ \
    (entry)->m_blink = (head); \
    (entry)->m_flink = (head)->m_flink; \
    (head)->m_flink = (entry); \
    (entry)->m_flink->m_blink = (entry); \
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
 * rtl_memory (debug) internals.
 *
 *=========================================================================*/
#if OSL_DEBUG_LEVEL > 0

#define __dbg_memory_succ(entry, length) \
(memory_type*)((char*)((entry)) + ((length) & ~0x1))

#define __dbg_memory_pred(entry, offset) \
(memory_type*)((char*)((entry)) - ((offset) & ~0x1))

#define __dbg_memory_ensure(entry) (!((sal_Size)(entry) & 0x7))

/*
 * __dbg_memory_dequeue.
 */
static void __dbg_memory_dequeue (sal_Size n)
{
    register sal_Size k = queue(n);

    g_memory.m_queue_stat[k].m_dequeue += 1;
    g_memory.m_queue_stat[k].m_delta_q += 1;

    g_memory.m_queue_stat[k].m_deqsize += n;
    g_memory.m_queue_stat[k].m_delta_n += n;
}

/*
 * __dbg_memory_enqueue.
 */
static void __dbg_memory_enqueue (sal_Size n)
{
    register sal_Size k = queue(n);

    g_memory.m_queue_stat[k].m_enqueue += 1;
    g_memory.m_queue_stat[k].m_delta_q -= 1;

    g_memory.m_queue_stat[k].m_enqsize += n;
    g_memory.m_queue_stat[k].m_delta_n -= n;
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
    if (!__dbg_memory_ensure(x->m_length & ~0x1))
    {
        OSL_ENSURE(0, "__rtl_memory_verify(): dynamic memory corruption");
        return (0);
    }
    if (!__dbg_memory_ensure(x->m_offset & ~0x1))
    {
        OSL_ENSURE(0, "__rtl_memory_verify(): dynamic memory corruption");
        return (0);
    }
    if (!(x->m_length & ~0x1))
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
 * __dbg_memory_verify_alloc.
 */
static int __dbg_memory_verify_alloc (memory_type * x)
{
    register memory_type *head, *entry;
    head = entry = &(g_memory.m_alloc_head);

    if (!__dbg_memory_ensure(x))
    {
        OSL_ENSURE(0, "__rtl_memory_verify(): invalid pointer alignment.");
        return (0);
    }
    while (!((entry = entry->m_flink) == head))
    {
        if ((entry < x) && (x < __dbg_memory_succ(entry, entry->m_length)))
        {
            head = entry = __dbg_memory_succ(entry, sizeof(memory_type));
            while (!((x == entry) || (entry->m_offset & 0x1)))
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
    return (1);
}

/*
 * __dbg_memory_verify.
 */
static int __dbg_memory_verify (memory_type * x, int debug)
{
    /* dispatch upon 'debug' level */
    if (debug)
    {
        /* verify allocation */
        if (!__dbg_memory_verify_alloc (x))
            return (0);
    }
    else
    {
        /* verify 'chain' fields */
        if (!__dbg_memory_verify_chain (x))
            return (0);
    }

    /* verify 'used' bit */
    if (!(x->m_length & 0x1))
    {
        OSL_ENSURE(0, "__rtl_memory_verify(): memory not used.");
        return (0);
    }
    return (1);
}

#if OSL_DEBUG_LEVEL > 1
/*
 * __dbg_memory_usage_update.
 */
static sal_Size __dbg_memory_usage_update (memory_stat * stat, sal_Size length)
{
    register sal_Size n = (length & ~0x1), k = queue(n);

    stat[k].m_dequeue += 1;
    stat[k].m_deqsize += n;

    if (!(length & 0x1))
    {
        /* not used */
        stat[k].m_enqueue += 1;
        stat[k].m_enqsize += n;
        return (n);
    }
    else
    {
        /* used */
        stat[k].m_delta_q += 1;
        stat[k].m_delta_n += n;
        return (0);
    }
}

/*
 * __dbg_memory_usage.
 */
static void __dbg_memory_usage (memory_stat * total)
{
    register memory_type *head, *entry, *memory;
    memory_stat           stat[__N__];

    memset (stat, 0, __N__ * sizeof(memory_stat));

    head = entry = &(g_memory.m_alloc_head);
    while (!((entry = entry->m_flink) == head))
    {
        register sal_Size k = 0, n = entry->m_length - sizeof(memory_type);

        memory = __dbg_memory_succ(entry, sizeof(memory_type));
        while (!(memory->m_offset & 0x1))
        {
            /* not last */
            k += __dbg_memory_usage_update (stat, memory->m_length);
            memory = __dbg_memory_succ(memory, memory->m_length);
        }

        k += __dbg_memory_usage_update (stat, memory->m_length);
        OSL_TRACE("%x %10d %10d", (sal_Size)(entry), n, k);
    }

    if (total)
    {
        sal_Size i;

        memset (total, 0, sizeof(memory_stat));
        for (i = 0; i < __N__; i++)
        {
            total->m_dequeue += stat[i].m_dequeue;
            total->m_enqueue += stat[i].m_enqueue;
            total->m_delta_q += stat[i].m_delta_q;

            total->m_deqsize += stat[i].m_deqsize;
            total->m_enqsize += stat[i].m_enqsize;
            total->m_delta_n += stat[i].m_delta_n;
        }
    }
}
#endif /* OSL_DEBUG_LEVEL */

#endif /* OSL_DEBUG_LEVEL */
#if OSL_DEBUG_LEVEL > 0

#define DBG_MEMORY_DEQUEUE(n) __dbg_memory_dequeue((sal_Size)(n) & ~0x1)
#define DBG_MEMORY_ENQUEUE(n) __dbg_memory_enqueue((sal_Size)(n) & ~0x1)

#define DBG_MEMORY_DEQFILL(entry, offset, length) \
    memset(((char*)(entry) + (offset)), 0x77777777, (length))
#define DBG_MEMORY_ENQFILL(entry, offset, length) \
    memset(((char*)(entry) + (offset)), 0x33333333, (length))

#define DBG_MEMORY_INSERT(entry) __dbg_memory_insert((entry))
#define DBG_MEMORY_REMOVE(entry) __dbg_memory_remove((entry))

#if OSL_DEBUG_LEVEL > 1
#define DBG_MEMORY_VERIFY(entry) __dbg_memory_verify((entry), 1)
#else  /* OSL_DEBUG_LEVEL > 0 */
#define DBG_MEMORY_VERIFY(entry) __dbg_memory_verify((entry), 0)
#endif /* OSL_DEBUG_LEVEL */

#define DBG_MEMORY_VERIFY_CHAIN(entry) __dbg_memory_verify_chain((entry))
#define DBG_MEMORY_VERIFY_QUEUE(entry) __dbg_memory_verify_queue((entry))

#else  /* PRODUCT */

#define DBG_MEMORY_DEQUEUE(n)
#define DBG_MEMORY_ENQUEUE(n)

#define DBG_MEMORY_DEQFILL(entry, offset, length)
#define DBG_MEMORY_ENQFILL(entry, offset, length)

#define DBG_MEMORY_INSERT(entry)
#define DBG_MEMORY_REMOVE(entry)

#define DBG_MEMORY_VERIFY(entry)
#define DBG_MEMORY_VERIFY_CHAIN(entry)
#define DBG_MEMORY_VERIFY_QUEUE(entry)

#endif /* OSL_DEBUG_LEVEL || PRODUCT */

/*===========================================================================
 *
 * rtl_memory (manager) internals.
 *
 *=========================================================================*/
#define queue_cast(entry, offset) \
((memory_type*)((char*)(entry) + (ptrdiff_t)(offset)))

#define __rtl_memory_used(entry) ((entry)->m_length & 0x1)
#define __rtl_memory_last(entry) ((entry)->m_offset & 0x1)
#define __rtl_memory_offset(entry) \
    ((ptrdiff_t)((entry)->m_offset & ~0x1))

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
        sal_Size pagesize;
        int    i;

        queue_start (&(g_memory.m_alloc_head));
        queue_start (&(g_memory.m_spare_head));

        for (i = 0; i < __N__; i++)
            queue_start (&(g_memory.m_queue_head[i]));
        for (i = 1; i <= __L__; i++)
            g_memory.m_queue_head[i].m_length = i * __C__;
        for (i = __L__ + 1; i < __N__; i++)
            g_memory.m_queue_head[i].m_length =
                2 * g_memory.m_queue_head[i - 1].m_length;

        pagesize = __rtl_memory_vmpagesize();
        g_memory.m_align = RTL_MEMORY_ALIGN(__M__, pagesize);
    }
    RTL_MUTEX_RELEASE(&(g_memory.m_mutex.m_lock));
}

/*
 * ___rtl_memory_fini.
 */
void SAL_CALL ___rtl_memory_fini (void)
{
#if OSL_DEBUG_LEVEL > 1

    memory_stat total;

    __dbg_memory_usage (&total);
    if (total.m_delta_n > 0)
    {
        OSL_TRACE("___rtl_memory_fini(): "
                  "Leak: %10d (Alloc: %10d, Free: %10d)",
                  total.m_delta_n,
                  (sal_uInt32)(total.m_deqsize & 0xffffffff),
                  (sal_uInt32)(total.m_enqsize & 0xffffffff));
    }

#endif /* OSL_DEBUG_LEVEL */
}

/*
 * __rtl_memory_merge.
 */
#if defined(PROFILE) || (OSL_DEBUG_LEVEL > 0)
static void __rtl_memory_merge (memory_type * prev, memory_type * next)
{
    /* adjust length */
    prev->m_length += next->m_length;
    if (!__rtl_memory_last(next))
    {
        /* not last, adjust offset */
        register memory_type * succ = queue_cast(prev, prev->m_length);
        DBG_MEMORY_VERIFY_CHAIN (succ);
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
#endif /* OSL_DEBUG_LEVEL || PRODUCT */

/*
 * __rtl_memory_split.
 */
#if defined(PROFILE) || (OSL_DEBUG_LEVEL > 0)
static void __rtl_memory_split (memory_type * prev, memory_type * next)
{
    /* adjust length */
    prev->m_length -= next->m_length;
    if (!__rtl_memory_last(prev))
    {
        /* not last, adjust offset */
        register memory_type * succ = queue_cast(next, next->m_length);
        DBG_MEMORY_VERIFY_CHAIN (succ);
        succ->m_offset = next->m_length | __rtl_memory_last(succ);
    }

    /* propagate 'last' bit */
    next->m_offset |= __rtl_memory_last(prev);
    prev->m_offset &= ~0x1;
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
    (prev)->m_offset &= ~0x1; \
}
#endif /* OSL_DEBUG_LEVEL || PRODUCT */

/*
 * __rtl_memory_insert.
 */
#if defined(PROFILE) || (OSL_DEBUG_LEVEL > 0)
static void __rtl_memory_insert (memory_type * memory, sal_Size n)
{
    /* obtain queue head */
    register memory_type *head;

    head = &(g_memory.m_queue_head[queue(n)]);
    DBG_MEMORY_VERIFY_CHAIN (head);

    /* insert at queue tail (first-in first-out) */
    queue_insert_tail (head, memory);
}
#else  /* PRODUCT */
#define __rtl_memory_insert(memory, n) \
{ \
    register sal_Size h; \
\
    queue(h, (n)); \
    queue_insert_tail (&(g_memory.m_queue_head[h]), (memory)); \
}
#endif /* OSL_DEBUG_LEVEL || PRODUCT */

/*
 * __rtl_memory_resize.
 */
#if defined(PROFILE) || (OSL_DEBUG_LEVEL > 0)
static void __rtl_memory_resize (memory_type * memory, sal_Size n)
{
    register sal_Size k = (memory->m_length - n);

    OSL_ENSURE(!(memory->m_length & 0x1),
               "__rtl_memory_resize(): "
               "internal logic error.");

    if ((k >= sizeof(memory_type)) && (n <= __T__))
    {
        /* split */
        register memory_type * remain = queue_cast(memory, n);

        remain->m_length = k; remain->m_offset = n;
        __rtl_memory_split (memory, remain);

        /* check postcond */
        if (!__rtl_memory_last(remain))
        {
            /* not last, verify used next entry */
            register memory_type *next;

            next = queue_cast(remain, remain->m_length);
            DBG_MEMORY_VERIFY_CHAIN (next);

            OSL_POSTCOND(__rtl_memory_used(next),
                         "__rtl_memory_resize(): "
                         "internal logic error.");
        }

        /* enqueue */
        __rtl_memory_insert (remain, k);
        DBG_MEMORY_VERIFY_QUEUE (remain);
    }

    DBG_MEMORY_DEQUEUE(memory->m_length);
}
#else  /* PRODUCT */
#define __rtl_memory_resize(memory, n) \
{ \
    register sal_Size kn = ((memory)->m_length - (n)); \
    if ((kn >= sizeof(memory_type)) && (n <= __T__)) \
    { \
        register memory_type * remain = queue_cast((memory), (n)); \
\
        remain->m_length = kn; remain->m_offset = (n); \
        __rtl_memory_split ((memory), remain); \
\
        __rtl_memory_insert (remain, kn); \
    } \
}
#endif /* OSL_DEBUG_LEVEL || PRODUCT */

/*
 * __rtl_memory_dequeue.
 */
#if defined(PROFILE) || (OSL_DEBUG_LEVEL > 0)
static void __rtl_memory_dequeue (memory_type **ppMemory, sal_Size n)
{
    register memory_type *head, *entry;
    register sal_Size       k, m = n;

    OSL_PRECOND(!*ppMemory, "__rtl_memory_dequeue(): internal logic error.");
    for (k = queue(m); k < __N__; k++)
    {
        /* first fit (equals best fit w/ ascending insert) */
        head = &(g_memory.m_queue_head[k]);
        for (entry = head->m_flink; entry != head; entry = entry->m_flink)
        {
            /* queue not empty */
            DBG_MEMORY_VERIFY_CHAIN (entry);
            if (entry->m_length >= m)
            {
                /* remove entry */
                DBG_MEMORY_VERIFY_QUEUE (entry);
                queue_remove (entry);

                /* assign result */
                *ppMemory = entry;
                goto dequeue_leave;
            }
        }
    }

    head = &(g_memory.m_spare_head);
    for (entry = head->m_flink; entry != head; entry = entry->m_flink)
    {
        /* queue not empty */
        DBG_MEMORY_VERIFY_CHAIN (entry);
        if (entry->m_length >= m)
        {
            /* remove entry */
            DBG_MEMORY_VERIFY_QUEUE (entry);
            queue_remove (entry);

            /* assign result */
            *ppMemory = entry;
            goto dequeue_leave;
        }
    }

#if OSL_DEBUG_LEVEL > 0
    /* adjust for DBG_MEMORY_INSERT() overhead */
    m += sizeof(memory_type);
#endif /* OSL_DEBUG_LEVEL */

    k = RTL_MEMORY_ALIGN((m > __M__) ? m : __M__, g_memory.m_align);
    if (!((entry = RTL_MEMORY_ALLOC(k)) == 0))
    {
        entry->m_length = k;
        entry->m_offset = 0x1; /* set 'last' bit */

        *ppMemory = entry;
        DBG_MEMORY_INSERT(ppMemory);
    }

dequeue_leave:
    OSL_POSTCOND(*ppMemory, "__rtl_memory_dequeue(): out of memory.");
    if ((entry = *ppMemory) != 0)
    {
        /* adjust length */
        __rtl_memory_resize (entry, n);

        /* fill w/ 'uninitialized' pattern */
        DBG_MEMORY_DEQFILL (entry, __C__, entry->m_length - __C__);
    }
#if OSL_DEBUG_LEVEL > 1
    if (!entry)
    {
        memory_stat total;
        __dbg_memory_usage (&total);
    }
#endif /* OSL_DEBUG_LEVEL */
}
#else  /* PRODUCT */
#define __rtl_memory_dequeue(ppMemory, n, label) \
{ \
    register memory_type *head, *entry; \
    register sal_Size       h, m = (n); \
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
    head = &(g_memory.m_spare_head); \
    for (entry = head->m_flink; entry != head; entry = entry->m_flink) \
    { \
        if (entry->m_length >= m) \
        { \
            queue_remove (entry); \
            goto label; \
        } \
    } \
\
    h = RTL_MEMORY_ALIGN((m > __M__) ? m : __M__, g_memory.m_align); \
    if (!((entry = RTL_MEMORY_ALLOC(h)) == 0)) \
    { \
        entry->m_length = h; \
        entry->m_offset = 0x1; \
    } \
\
label: \
    if (entry) \
    { \
        __rtl_memory_resize (entry, (n)); \
        *(ppMemory) = entry; \
    } \
}
#endif /* OSL_DEBUG_LEVEL || PRODUCT */

#if defined(PROFILE) || (OSL_DEBUG_LEVEL > 0)
#define RTL_MEMORY_DEQUEUE(m, n, l) __rtl_memory_dequeue((m), (n))
#else  /* PRODUCT */
#define RTL_MEMORY_DEQUEUE(m, n, l) __rtl_memory_dequeue(m, n, l)
#endif /* OSL_DEBUG_LEVEL || PRODUCT */

/*
 * __rtl_memory_enqueue.
 */
#if defined(PROFILE) || (OSL_DEBUG_LEVEL > 0)
static void __rtl_memory_enqueue (memory_type **ppMemory)
{
    register memory_type *head = *ppMemory;

    OSL_ENSURE(!__rtl_memory_used(head),
               "__rtl_memory_enqueue(): "
               "internal logic error.");
    DBG_MEMORY_ENQUEUE (head->m_length);

    /* fill w/ 'deinitialized' pattern */
    DBG_MEMORY_ENQFILL (head, __C__, head->m_length - __C__);

    /* try merge w/ next entry */
    if (!__rtl_memory_last(head))
    {
        /* not last, check next in chain */
        register memory_type * next;

        next = queue_cast(head, head->m_length);
        DBG_MEMORY_VERIFY_CHAIN (next);

        if (!__rtl_memory_used(next))
        {
            /* next not used */
            DBG_MEMORY_VERIFY_QUEUE (next);
            queue_remove (next);

            /* merge w/ next */
            __rtl_memory_merge (head, next);
            DBG_MEMORY_ENQFILL (next, 0, sizeof(memory_type));
        }
    }

    /* try merge w/ prev entry */
    if (__rtl_memory_offset(head) > 0)
    {
        /* not first, check prev in chain */
        register memory_type * prev;

        prev = queue_cast(head, -(__rtl_memory_offset(head)));
        DBG_MEMORY_VERIFY_CHAIN (prev);

        if (!__rtl_memory_used(prev))
        {
            /* prev not used */
            DBG_MEMORY_VERIFY_QUEUE (prev);
            queue_remove (prev);

            /* merge w/ prev */
            __rtl_memory_merge (prev, head);
            DBG_MEMORY_ENQFILL (head, 0, sizeof(memory_type));
            head = prev;
        }
    }

    if (!(head->m_offset == 0x1))
    {
        /* page still used, enqueue */
        __rtl_memory_insert (head, head->m_length);
        head = 0;
    }
    else if (head->m_length <= g_memory.m_align)
    {
        /* small page unused, check spare page */
        register memory_type * spare;

        spare = &(g_memory.m_spare_head);
        if (spare->m_flink == spare)
        {
            /* keep as spare page */
            queue_insert_tail (spare, head);
            head = 0;
        }
    }
    if ((*ppMemory = head) != 0)
    {
        /* page unused, remove */
        DBG_MEMORY_REMOVE(ppMemory);
    }
}
#else  /* PRODUCT */
#define __rtl_memory_enqueue(ppMemory) \
{ \
    register memory_type *head = *(ppMemory); \
\
    if (!__rtl_memory_last(head)) \
    { \
        register memory_type * next; \
        next = queue_cast(head, head->m_length); \
        if (!__rtl_memory_used(next)) \
        { \
            queue_remove (next); \
            __rtl_memory_merge (head, next); \
        } \
    } \
\
    if (__rtl_memory_offset(head) > 0) \
    { \
        register memory_type * prev; \
        prev = queue_cast(head, -(__rtl_memory_offset(head))); \
        if (!__rtl_memory_used(prev)) \
        { \
            queue_remove (prev); \
            __rtl_memory_merge (prev, head); \
            head = prev; \
        } \
    } \
\
    if (!(head->m_offset == 0x1)) \
    { \
        register memory_type * used = head; \
        __rtl_memory_insert (used, used->m_length); \
        head = 0; \
    } \
    else if (head->m_length <= g_memory.m_align) \
    { \
        register memory_type * spare; \
        spare = &(g_memory.m_spare_head); \
        if (spare->m_flink == spare) \
        { \
            queue_insert_tail (spare, head); \
            head = 0; \
        } \
    } \
\
    *(ppMemory) = head; \
}
#endif /* OSL_DEBUG_LEVEL || PRODUCT */

#define RTL_MEMORY_ENQUEUE(m) __rtl_memory_enqueue((m))

#endif /* FORCE_SYSALLOC */

/*===========================================================================
 *
 * rtl_memory (manager) implementation.
 *
 *=========================================================================*/
/*
 * rtl_reallocateMemory.
 */
static
void* SAL_CALL rtl_reallocateMemory_CUSTOM (void * p, sal_Size n) SAL_THROW_EXTERN_C()
{
    memory_type * memory;
    if (!(!p || !n))
    {
        /* reallocate */
        register sal_Size datlen;

        memory = queue_cast(p, -(__C__)); p = 0;
        n = RTL_MEMORY_ALIGN(n, __Q__) + __C__;

        RTL_MEMORY_ENTER();
        DBG_MEMORY_VERIFY(memory);

        /* clear 'used' bit */
        DBG_MEMORY_ENQUEUE (memory->m_length);
        memory->m_length &= ~0x1;

        /* amount of data to be moved or copied */
        datlen = ((memory->m_length < n) ? memory->m_length : n);

        /* try merge w/ next entry */
        if (!__rtl_memory_last(memory))
        {
            /* not last, check next in chain */
            register memory_type * next;

            next = queue_cast(memory, memory->m_length);
            DBG_MEMORY_VERIFY_CHAIN(next);

            if (!__rtl_memory_used(next))
            {
                /* next not used */
                DBG_MEMORY_VERIFY_QUEUE(next);
                queue_remove (next);

                /* merge w/ next */
                __rtl_memory_merge (memory, next);
            }
        }

        /* try merge w/ prev entry */
        if (__rtl_memory_offset(memory) > 0)
        {
            /* not first, check prev in chain */
            register memory_type * prev;

            prev = queue_cast(memory, -(__rtl_memory_offset(memory)));
            DBG_MEMORY_VERIFY_CHAIN (prev);

            if (!__rtl_memory_used(prev))
            {
                /* prev not used, try merge, move */
                if ((memory->m_length + prev->m_length) >= n)
                {
                    /* prev does fit */
                    DBG_MEMORY_VERIFY_QUEUE (prev);
                    queue_remove (prev);

                    /* merge w/ prev */
                    __rtl_memory_merge (prev, memory);

                    /* move to prev */
                    memmove (
                        queue_cast(prev, __C__),
                        queue_cast(memory, __C__),
                        datlen - __C__);
                    memory = prev;
                }
            }
        }

        if (memory->m_length >= n)
        {
            /* adjust, set 'used' bit */
            __rtl_memory_resize (memory, n);
            memory->m_length |= 0x1;

            /* assign result */
            p = queue_cast(memory, __C__);
        }
        else
        {
            /* allocate */
            memory_type * result = 0;

            /* restore 'used' bit */
            DBG_MEMORY_DEQUEUE (memory->m_length);
            memory->m_length |= 0x80000000;

            RTL_MEMORY_DEQUEUE (&result, n, realloc_label_1);
            if (result)
            {
                /* set 'used' bit */
                result->m_length |= 0x1;

                /* copy */
                memcpy (
                    queue_cast(result, __C__),
                    queue_cast(memory, __C__),
                    datlen - __C__);

                /* clear 'used' bit, enqueue */
                memory->m_length &= 0x7fffffff;
                RTL_MEMORY_ENQUEUE (&memory);
                if (memory)
                {
                    /* free memory page */
                    RTL_MEMORY_FREE(memory, memory->m_length);
                }

                /* assign result */
                p = queue_cast(result, __C__);
            }
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
            /* set 'used' bit */
            memory->m_length |= 0x1;

            /* assign result */
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
        memory->m_length &= ~0x1;

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

static
void* SAL_CALL rtl_reallocateMemory_SYSTEM (void * p, sal_Size n) SAL_THROW_EXTERN_C()
{
    return realloc(p, (sal_Size)(n));
}

void* SAL_CALL rtl_reallocateMemory (void * p, sal_Size n) SAL_THROW_EXTERN_C()
{
   while (1) {
      if (alloc_mode == AMode_CUSTOM) {
         return rtl_reallocateMemory_CUSTOM(p,n);
      }
      if (alloc_mode == AMode_SYSTEM) {
         return rtl_reallocateMemory_SYSTEM(p,n);
      }
      determine_alloc_mode();
   }
}



/*
 * rtl_allocateMemory.
 */
static
void* SAL_CALL rtl_allocateMemory_CUSTOM (sal_Size n) SAL_THROW_EXTERN_C()
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
            /* set 'used' bit */
            memory->m_length |= 0x1;

            /* assign result */
            p = queue_cast(memory, __C__);
        }
        RTL_MEMORY_LEAVE();
    }
    return (p);
}

static
void* SAL_CALL rtl_allocateMemory_SYSTEM (sal_Size n) SAL_THROW_EXTERN_C()
{
    return malloc((sal_Size)(n));
}

void* SAL_CALL rtl_allocateMemory (sal_Size n) SAL_THROW_EXTERN_C()
{
   while (1) {
      if (alloc_mode == AMode_CUSTOM) {
         return rtl_allocateMemory_CUSTOM(n);
      }
      if (alloc_mode == AMode_SYSTEM) {
         return rtl_allocateMemory_SYSTEM(n);
      }
      determine_alloc_mode();
   }
}



/*
 * rtl_freeMemory.
 */
static
void SAL_CALL rtl_freeMemory_CUSTOM (void * p) SAL_THROW_EXTERN_C()
{
    if (p)
    {
        memory_type * memory = queue_cast(p, -(__C__));

        RTL_MEMORY_ENTER();
        DBG_MEMORY_VERIFY(memory);

        /* clear 'used' bit, enqueue */
        memory->m_length &= ~0x1;

        RTL_MEMORY_ENQUEUE (&memory);
        if (memory)
        {
            /* free memory page */
            RTL_MEMORY_FREE(memory, memory->m_length);
        }
        RTL_MEMORY_LEAVE();
    }
}

static
void SAL_CALL rtl_freeMemory_SYSTEM (void * p) SAL_THROW_EXTERN_C()
{
    free(p);
}

void SAL_CALL rtl_freeMemory (void * p) SAL_THROW_EXTERN_C()
{
   while (1) {
      if (alloc_mode == AMode_CUSTOM) {
         rtl_freeMemory_CUSTOM(p);
     return;
      }
      if (alloc_mode == AMode_SYSTEM) {
         rtl_freeMemory_SYSTEM(p);
     return;
      }
      determine_alloc_mode();
   }
}


/*
 * rtl_allocateZeroMemory.
 */
static
void* SAL_CALL rtl_allocateZeroMemory_CUSTOM (sal_Size n) SAL_THROW_EXTERN_C()
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
            /* zero, set 'used' bit */
            memset ((char*)memory + __C__, 0, memory->m_length - __C__);
            memory->m_length |= 0x1;

            /* assign result */
            p = queue_cast(memory, __C__);
        }
        RTL_MEMORY_LEAVE();
    }
    return (p);
}

static
void* SAL_CALL rtl_allocateZeroMemory_SYSTEM (sal_Size n) SAL_THROW_EXTERN_C()
{
    return calloc((sal_Size)(n), 1);
}

void* SAL_CALL rtl_allocateZeroMemory (sal_Size n) SAL_THROW_EXTERN_C()
{
   while (1) {
      if (alloc_mode == AMode_CUSTOM) {
         return rtl_allocateZeroMemory_CUSTOM(n);
      }
      if (alloc_mode == AMode_SYSTEM) {
         return rtl_allocateZeroMemory_SYSTEM(n);
      }
      determine_alloc_mode();
   }
}


/*
 * rtl_freeZeroMemory.
 */
static
void SAL_CALL rtl_freeZeroMemory_CUSTOM (void * p, sal_Size n) SAL_THROW_EXTERN_C()
{
    (void) n; /* unused */
    if (p)
    {
        memory_type * memory = queue_cast(p, -(__C__));

        RTL_MEMORY_ENTER();
        DBG_MEMORY_VERIFY(memory);

        /* clear 'used' bit, zero, enqueue */
        memory->m_length &= ~0x1;
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

static
void SAL_CALL rtl_freeZeroMemory_SYSTEM (void * p, sal_Size n) SAL_THROW_EXTERN_C()
{
    if (p)
    {
        memset(p, 0, n);
        free(p);
    }
}

void SAL_CALL rtl_freeZeroMemory (void * p, sal_Size n) SAL_THROW_EXTERN_C()
{
   while (1) {
      if (alloc_mode == AMode_CUSTOM) {
         rtl_freeZeroMemory_CUSTOM(p,n);
     return;
      }
      if (alloc_mode == AMode_SYSTEM) {
         rtl_freeZeroMemory_SYSTEM(p,n);
     return;
      }
      determine_alloc_mode();
   }
}

/*===========================================================================
 *
 * The End.
 *
 *=========================================================================*/

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
