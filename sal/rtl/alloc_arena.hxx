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

#ifndef INCLUDED_SAL_RTL_ALLOC_ARENA_HXX
#define INCLUDED_SAL_RTL_ALLOC_ARENA_HXX

#include <sal/types.h>
#include <rtl/alloc.h>
#include "alloc_impl.hxx"

/** rtl_arena_stat_type
 *  @internal
 */
struct rtl_arena_stat_type
{
    sal_uInt64 m_alloc;
    sal_uInt64 m_free;

    sal_Size   m_mem_total;
    sal_Size   m_mem_alloc;
};

/** rtl_arena_segment_type
 *  @internal
 */
constexpr sal_Size RTL_ARENA_SEGMENT_TYPE_HEAD = 0x01;
constexpr sal_Size RTL_ARENA_SEGMENT_TYPE_SPAN = 0x02;
constexpr sal_Size RTL_ARENA_SEGMENT_TYPE_FREE = 0x04;
constexpr sal_Size RTL_ARENA_SEGMENT_TYPE_USED = 0x08;

struct rtl_arena_segment_type
{
    /* segment list linkage */
    rtl_arena_segment_type * m_snext;
    rtl_arena_segment_type * m_sprev;

    /* free/used list linkage */
    rtl_arena_segment_type * m_fnext;
    rtl_arena_segment_type * m_fprev;

    /* segment description */
    sal_uIntPtr         m_addr;
    sal_Size            m_size;
    sal_Size            m_type;
};

/** rtl_arena_type
 *  @internal
 */
constexpr auto RTL_ARENA_FREELIST_SIZE = sizeof(void*) * 8;
constexpr auto RTL_ARENA_HASH_SIZE = 64;

constexpr auto RTL_ARENA_FLAG_RESCALE = 1; /* within hash rescale operation */

struct rtl_arena_st
{
    /* linkage */
    rtl_arena_type *          m_arena_next;
    rtl_arena_type *          m_arena_prev;

    /* properties */
    char                      m_name[RTL_ARENA_NAME_LENGTH + 1];
    long                      m_flags;

    rtl_memory_lock_type      m_lock;
    rtl_arena_stat_type       m_stats;

    rtl_arena_type *          m_source_arena;
    void * (SAL_CALL * m_source_alloc)(rtl_arena_type *, sal_Size *);
    void   (SAL_CALL * m_source_free) (rtl_arena_type *, void *, sal_Size);

    sal_Size                  m_quantum;
    sal_Size                  m_quantum_shift; /* log2(m_quantum) */

    rtl_arena_segment_type    m_segment_reserve_span_head;
    rtl_arena_segment_type    m_segment_reserve_head;

    rtl_arena_segment_type    m_segment_head;

    rtl_arena_segment_type    m_freelist_head[RTL_ARENA_FREELIST_SIZE];
    sal_Size                  m_freelist_bitmap;

    rtl_arena_segment_type ** m_hash_table;
    rtl_arena_segment_type *  m_hash_table_0[RTL_ARENA_HASH_SIZE];
    sal_Size                  m_hash_size;  /* m_hash_mask + 1   */
    sal_Size                  m_hash_shift; /* log2(m_hash_size) */
};

/** gp_default_arena
 *  default arena with pagesize quantum
 *
 *  @internal
 */
extern rtl_arena_type * gp_default_arena;

typedef void (*ArenaForeachFn)(void *addr, sal_Size size);

void rtl_arena_foreach(rtl_arena_type *arena, ArenaForeachFn fn);

#endif // INCLUDED_SAL_RTL_ALLOC_ARENA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
