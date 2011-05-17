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

#ifndef INCLUDED_RTL_ALLOC_IMPL_H
#define INCLUDED_RTL_ALLOC_IMPL_H

#include "sal/types.h"

#ifdef __cplusplus
extern "C" {
#endif


/** Alignment macros
 */
#if SAL_TYPES_ALIGNMENT4 > 1
#define RTL_MEMORY_ALIGNMENT_4 SAL_TYPES_ALIGNMENT4
#else
#define RTL_MEMORY_ALIGNMENT_4 sizeof(int)
#endif /* SAL_TYPES_ALIGNMENT4 */

#if SAL_TYPES_ALIGNMENT8 > 1
#define RTL_MEMORY_ALIGNMENT_8 SAL_TYPES_ALIGNMENT8
#else
#define RTL_MEMORY_ALIGNMENT_8 sizeof(void*)
#endif /* SAL_TYPES_ALIGNMENT8 */

#if 0  /* @@@ */
#define RTL_MEMORY_ALIGNMENT_1 8
#define RTL_MEMORY_ALIGNMENT_2 (sizeof(void*) * 2)
#endif /* @@@ */

#define RTL_MEMORY_ALIGN(value, align) (((value) + ((align) - 1)) & ~((align) - 1))

#define RTL_MEMORY_ISP2(value) (((value) & ((value) - 1)) == 0)
#define RTL_MEMORY_P2ALIGN(value, align) ((value) & -(sal_IntPtr)(align))

#define RTL_MEMORY_P2ROUNDUP(value, align) \
    (-(-(sal_IntPtr)(value) & -(sal_IntPtr)(align)))
#define RTL_MEMORY_P2END(value, align) \
    (-(~(sal_IntPtr)(value) & -(sal_IntPtr)(align)))


/** Function inlining macros
 *  (compiler dependent)
 */
#ifndef RTL_MEMORY_INLINE
#if defined(__GNUC__)
#define RTL_MEMORY_INLINE __inline__
#elif defined(_MSC_VER)
#define RTL_MEMORY_INLINE __inline
#else
#define RTL_MEMORY_INLINE
#endif /* __GNUC__ || _MSC_VER */
#endif /* RTL_MEMORY_INLINE */


/** printf() format specifier(s)
 *  (from C90 <sys/int_fmtio.h>)
 */
#ifndef PRIu64
#if defined(_MSC_VER)
#define PRIu64 "I64u"
#else  /* !_MSC_VER */
#define PRIu64 "llu"
#endif /* !_MSC_VER */
#endif /* PRIu64 */


/** highbit(): log2() + 1
 *  (complexity O(1))
 */
static RTL_MEMORY_INLINE int
highbit(sal_Size n)
{
  register int k = 1;

  if (n == 0)
    return (0);
#if SAL_TYPES_SIZEOFLONG == 8
  if (n & 0xffffffff00000000ul)
    k |= 32, n >>= 32;
#endif
  if (n & 0xffff0000)
    k |= 16, n >>= 16;
  if (n & 0xff00)
    k |= 8, n >>= 8;
  if (n & 0xf0)
    k |= 4, n >>= 4;
  if (n & 0x0c)
    k |= 2, n >>= 2;
  if (n & 0x02)
    k++;

  return (k);
}

#if defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#pragma inline(highbit)
#endif /* __SUNPRO_C */


/** lowbit(): find first bit set
 *  (complexity O(1))
 */
static RTL_MEMORY_INLINE int
lowbit(sal_Size n)
{
  register int k = 1;

  if (n == 0)
    return (0);
#if SAL_TYPES_SIZEOFLONG == 8
  if (!(n & 0xffffffff))
    k |= 32, n >>= 32;
#endif
  if (!(n & 0xffff))
    k |= 16, n >>= 16;
  if (!(n & 0xff))
    k |= 8, n >>= 8;
  if (!(n & 0xf))
    k |= 4, n >>= 4;
  if (!(n & 0x3))
    k |= 2, n >>= 2;
  if (!(n & 0x1))
    k++;
  return (k);
}

#if defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#pragma inline(lowbit)
#endif /* __SUNPRO_C */


/** Queue manipulation macros
 *  (doubly linked circular list)
 *  (complexity O(1))
 */
#define QUEUE_STARTED_NAMED(entry, name) \
  (((entry)->m_##name##next == (entry)) && ((entry)->m_##name##prev == (entry)))

#define QUEUE_START_NAMED(entry, name) \
{ \
  (entry)->m_##name##next = (entry); \
  (entry)->m_##name##prev = (entry); \
}

#define QUEUE_REMOVE_NAMED(entry, name) \
{ \
  (entry)->m_##name##prev->m_##name##next = (entry)->m_##name##next; \
  (entry)->m_##name##next->m_##name##prev = (entry)->m_##name##prev; \
  QUEUE_START_NAMED(entry, name); \
}

#define QUEUE_INSERT_HEAD_NAMED(head, entry, name) \
{ \
  (entry)->m_##name##prev = (head); \
  (entry)->m_##name##next = (head)->m_##name##next; \
  (head)->m_##name##next = (entry); \
  (entry)->m_##name##next->m_##name##prev = (entry); \
}

#define QUEUE_INSERT_TAIL_NAMED(head, entry, name) \
{ \
  (entry)->m_##name##next = (head); \
  (entry)->m_##name##prev = (head)->m_##name##prev; \
  (head)->m_##name##prev = (entry); \
  (entry)->m_##name##prev->m_##name##next = (entry); \
}


/** rtl_memory_lock_type
 *  (platform dependent)
 */
#if defined(SAL_UNX) || defined(SAL_OS2)

#include <unistd.h>
#include <pthread.h>

typedef pthread_mutex_t rtl_memory_lock_type;

#define RTL_MEMORY_LOCK_INIT(lock)    pthread_mutex_init((lock), NULL)
#define RTL_MEMORY_LOCK_DESTROY(lock) pthread_mutex_destroy((lock))

#define RTL_MEMORY_LOCK_ACQUIRE(lock) pthread_mutex_lock((lock))
#define RTL_MEMORY_LOCK_RELEASE(lock) pthread_mutex_unlock((lock))

#elif defined(SAL_W32)

#define WIN32_LEAN_AND_MEAN
#ifdef _MSC_VER
#pragma warning(push,1) /* disable warnings within system headers */
#endif
#include <windows.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

typedef CRITICAL_SECTION rtl_memory_lock_type;

#define RTL_MEMORY_LOCK_INIT(lock)    InitializeCriticalSection((lock))
#define RTL_MEMORY_LOCK_DESTROY(lock) DeleteCriticalSection((lock))

#define RTL_MEMORY_LOCK_ACQUIRE(lock) EnterCriticalSection((lock))
#define RTL_MEMORY_LOCK_RELEASE(lock) LeaveCriticalSection((lock))

#else
#error Unknown platform
#endif /* SAL_UNX | SAL_W32 */


/** Cache creation flags.
 *  @internal
 */
#define RTL_CACHE_FLAG_NOMAGAZINE   (1 << 13) /* w/o magazine layer */
#define RTL_CACHE_FLAG_QUANTUMCACHE (2 << 13) /* used as arena quantum cache */


/** Valgrind support macros.
 */
#if !defined(HAVE_MEMCHECK_H) || (OSL_DEBUG_LEVEL == 0)
#if !defined(NVALGRIND)
#define NVALGRIND 1
#endif /* ! NVALGRIND */
#endif /* ! HAVE_MEMCHECK_H || (OSL_DEBUG_LEVEL == 0) */

#if defined(NVALGRIND)
#define VALGRIND_MAKE_MEM_UNDEFINED(addr, size)
#define VALGRIND_MAKE_MEM_DEFINED(addr, size)
#define VALGRIND_MALLOCLIKE_BLOCK(addr, sizeB, rzB, is_zeroed)
#define VALGRIND_FREELIKE_BLOCK(addr, rzB)
#define VALGRIND_CREATE_MEMPOOL(pool, rzB, is_zeroed)
#define VALGRIND_DESTROY_MEMPOOL(pool)
#define VALGRIND_MEMPOOL_ALLOC(pool, addr, size)
#define VALGRIND_MEMPOOL_FREE(pool, addr)
#elif defined(HAVE_MEMCHECK_H)
#include <memcheck.h>
#if !defined(FORCE_SYSALLOC)
#define FORCE_SYSALLOC 1
#endif /* !FORCE_SYSALLOC */
#endif /* NVALGRIND || HAVE_MEMCHECK_H */

#ifdef __cplusplus
}
#endif

#endif /* INCLUDED_RTL_ALLOC_IMPL_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
