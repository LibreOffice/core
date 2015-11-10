/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SAL_RTL_ALLOC_IMPL_HXX
#define INCLUDED_SAL_RTL_ALLOC_IMPL_HXX

#include "sal/types.h"

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
static inline int
highbit(sal_Size n)
{
  int k = 1;

  if (n == 0)
    return 0;
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

  return k;
}

/** lowbit(): find first bit set
 *  (complexity O(1))
 */
static inline int
lowbit(sal_Size n)
{
  int k = 1;

  if (n == 0)
    return 0;
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
  return k;
}

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
#if defined(SAL_UNX)

#include <unistd.h>
#include <pthread.h>

typedef pthread_mutex_t rtl_memory_lock_type;

#define RTL_MEMORY_LOCK_INIT(lock)    pthread_mutex_init((lock), nullptr)
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

typedef enum { AMode_CUSTOM, AMode_SYSTEM, AMode_UNSET } AllocMode;

extern AllocMode alloc_mode;

#endif // INCLUDED_SAL_RTL_ALLOC_IMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
