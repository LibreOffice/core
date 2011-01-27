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

#ifndef INCLUDED_RTL_ALLOC_CACHE_H
#define INCLUDED_RTL_ALLOC_CACHE_H

#include "sal/types.h"
#include "rtl/alloc.h"
#include "alloc_impl.h"

#ifdef __cplusplus
extern "C" {
#endif

/** rtl_cache_stat_type
 *  @internal
 */
typedef struct rtl_cache_stat_st rtl_cache_stat_type;
struct rtl_cache_stat_st
{
    sal_uInt64 m_alloc;
    sal_uInt64 m_free;

    sal_Size   m_mem_total;
    sal_Size   m_mem_alloc;
};


/** rtl_cache_bufctl_type
 *  @internal
 */
typedef struct rtl_cache_bufctl_st rtl_cache_bufctl_type;
struct rtl_cache_bufctl_st
{
    rtl_cache_bufctl_type * m_next; /* linkage */

    sal_uIntPtr             m_addr; /* buffer address  */
    sal_uIntPtr             m_slab; /* parent slab address */
};


/** rtl_cache_slab_type
 *  @internal
 */
typedef struct rtl_cache_slab_st rtl_cache_slab_type;
struct rtl_cache_slab_st
{
    rtl_cache_slab_type *   m_slab_next; /* slab linkage */
    rtl_cache_slab_type *   m_slab_prev; /* slab linkage */

    sal_Size                m_ntypes;    /* number of buffers used */
    sal_uIntPtr             m_data;      /* buffer start addr */

    sal_uIntPtr             m_bp;        /* free buffer linkage 'base pointer'  */
    rtl_cache_bufctl_type * m_sp;        /* free buffer linkage 'stack pointer' */
};


/** rtl_cache_magazine_type
 *  @internal
 */
#define RTL_CACHE_MAGAZINE_SIZE 61

typedef struct rtl_cache_magazine_st rtl_cache_magazine_type;
struct rtl_cache_magazine_st
{
    rtl_cache_magazine_type * m_mag_next; /* depot linkage */

    sal_Size                  m_mag_size;
    sal_Size                  m_mag_used;

    void *                    m_objects[RTL_CACHE_MAGAZINE_SIZE];
};


/** rtl_cache_depot_type
 *  @internal
 */
typedef struct rtl_cache_depot_st rtl_cache_depot_type;
struct rtl_cache_depot_st
{
    /* magazine list */
    rtl_cache_magazine_type * m_mag_next;  /* linkage */
    sal_Size                  m_mag_count; /* count */

    /* working set parameters */
    sal_Size                  m_curr_min;
    sal_Size                  m_prev_min;
};


/** rtl_cache_type
 *  @internal
 */
#define RTL_CACHE_HASH_SIZE        8

#define RTL_CACHE_FEATURE_HASH        1
#define RTL_CACHE_FEATURE_BULKDESTROY 2
#define RTL_CACHE_FEATURE_RESCALE     4 /* within hash rescale operation */

struct rtl_cache_st
{
    /* linkage */
    rtl_cache_type *          m_cache_next;
    rtl_cache_type *          m_cache_prev;

    /* properties */
    char                      m_name[RTL_CACHE_NAME_LENGTH + 1];
    long                      m_features;

    sal_Size                  m_type_size;   /* const */
    sal_Size                  m_type_align;  /* const */
    sal_Size                  m_type_shift;  /* log2(m_type_size); const */

    int  (SAL_CALL * m_constructor)(void * obj, void * userarg); /* const */
    void (SAL_CALL * m_destructor) (void * obj, void * userarg); /* const */
    void (SAL_CALL * m_reclaim)    (void * userarg);             /* const */
    void *                    m_userarg;

    /* slab layer */
    rtl_memory_lock_type      m_slab_lock;
    rtl_cache_stat_type       m_slab_stats;

    rtl_arena_type *          m_source;     /* slab supplier; const */
    sal_Size                  m_slab_size;  /* const */
    sal_Size                  m_ntypes;     /* number of buffers per slab; const */
    sal_Size                  m_ncolor;     /* next slab color */
    sal_Size                  m_ncolor_max; /* max. slab color */

    rtl_cache_slab_type       m_free_head;
    rtl_cache_slab_type       m_used_head;

    rtl_cache_bufctl_type **  m_hash_table;
    rtl_cache_bufctl_type *   m_hash_table_0[RTL_CACHE_HASH_SIZE];
    sal_Size                  m_hash_size;  /* m_hash_mask + 1   */
    sal_Size                  m_hash_shift; /* log2(m_hash_size) */

    /* depot layer */
    rtl_memory_lock_type      m_depot_lock;

    rtl_cache_depot_type      m_depot_empty;
    rtl_cache_depot_type      m_depot_full;

    rtl_cache_type *          m_magazine_cache; /* magazine supplier; const */

    /* cpu layer */
    rtl_cache_magazine_type * m_cpu_curr;
    rtl_cache_magazine_type * m_cpu_prev;

    rtl_cache_stat_type       m_cpu_stats;
};


#ifdef __cplusplus
}
#endif

#endif /* INCLUDED_RTL_ALLOC_CACHE_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
