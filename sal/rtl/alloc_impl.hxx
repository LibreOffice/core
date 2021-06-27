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

#pragma once

#include <sal/types.h>

#if defined _WIN32
#include <intrin.h>
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
#define RTL_MEMORY_P2ALIGN(value, align) ((value) & -static_cast<sal_IntPtr>(align))

#define RTL_MEMORY_P2ROUNDUP(value, align) \
    (-(-static_cast<sal_IntPtr>(value) & -static_cast<sal_IntPtr>(align)))
#define RTL_MEMORY_P2END(value, align) \
    (-(~static_cast<sal_IntPtr>(value) & -static_cast<sal_IntPtr>(align)))

/** Find highest bit 1-based: 01101010100
  *                            ^
  *                            |
  *                            10
  */
inline unsigned int highbit(sal_Size n)
{
    if (n == 0)
        return 0;

#if defined _WIN32
    unsigned long pos;
#if defined _WIN64
    return _BitScanReverse64(&pos, n) == 0 ? 0 : pos + 1;
#else
    return _BitScanReverse(&pos, n) == 0 ? 0 : pos + 1;
#endif
#else
    if constexpr (sizeof(sal_Size) == sizeof(long long int))
        return sizeof(sal_Size) * 8 - __builtin_clzll(n);
    if constexpr (sizeof(sal_Size) == sizeof(long int))
        return sizeof(sal_Size) * 8 - __builtin_clzl(n);
    if constexpr (sizeof(sal_Size) <= sizeof(int))
        return sizeof(sal_Size) * 8 - __builtin_clz(n);
#endif
}

/** Find lowest bit 1-based: 01101010100
  *                                  ^
  *                                  |
  *                                  3
  */
inline unsigned int lowbit(sal_Size n)
{
#if defined _WIN32
    if (n == 0)
        return 0;
    unsigned long pos;
#if defined _WIN64
    return _BitScanReverse(&pos, n) == 0 ? 0 : pos + 1;
#else
    return _BitScanReverse64(&pos, n) == 0 ? 0 : pos + 1;
#endif
#else
    if constexpr (sizeof(sal_Size) == sizeof(long long int))
        return __builtin_ffsll(n);
    if constexpr (sizeof(sal_Size) == sizeof(long int))
        return __builtin_ffsl(n);
    if constexpr (sizeof(sal_Size) <= sizeof(int))
        return __builtin_ffs(n);
#endif
}

/** Queue manipulation macros
    (doubly linked circular list)
    (complexity O(1))
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

#if defined(SAL_UNX)

#include <unistd.h>
#include <pthread.h>

typedef pthread_mutex_t rtl_memory_lock_type;

#define RTL_MEMORY_LOCK_INIT(lock)    pthread_mutex_init((lock), nullptr)
#define RTL_MEMORY_LOCK_DESTROY(lock) pthread_mutex_destroy((lock))

#define RTL_MEMORY_LOCK_ACQUIRE(lock) pthread_mutex_lock((lock))
#define RTL_MEMORY_LOCK_RELEASE(lock) pthread_mutex_unlock((lock))

#elif defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef CRITICAL_SECTION rtl_memory_lock_type;

#define RTL_MEMORY_LOCK_INIT(lock)    InitializeCriticalSection((lock))
#define RTL_MEMORY_LOCK_DESTROY(lock) DeleteCriticalSection((lock))

#define RTL_MEMORY_LOCK_ACQUIRE(lock) EnterCriticalSection((lock))
#define RTL_MEMORY_LOCK_RELEASE(lock) LeaveCriticalSection((lock))

#else
#error Unknown platform
#endif /* SAL_UNX | _WIN32 */

/** Cache creation flags.
    @internal
*/
#define RTL_CACHE_FLAG_NOMAGAZINE   (1 << 13) /* w/o magazine layer */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
