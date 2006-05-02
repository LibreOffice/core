/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: alloc_arena.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2006-05-02 12:11:03 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_RTL_ALLOC_ARENA_H
#define INCLUDED_RTL_ALLOC_ARENA_H

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#ifndef _RTL_ALLOC_H_
#include "rtl/alloc.h"
#endif

#ifndef INCLUDED_RTL_ALLOC_IMPL_H
#include "alloc_impl.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** rtl_arena_stat_type
 *  @internal
 */
typedef struct rtl_arena_stat_st rtl_arena_stat_type;
struct rtl_arena_stat_st
{
    sal_uInt64 m_alloc;
    sal_uInt64 m_free;

    sal_Size   m_mem_total;
    sal_Size   m_mem_alloc;
};


/** rtl_arena_segment_type
 *  @internal
 */
#define RTL_ARENA_SEGMENT_TYPE_HEAD ((sal_Size)(0x01))
#define RTL_ARENA_SEGMENT_TYPE_SPAN ((sal_Size)(0x02))
#define RTL_ARENA_SEGMENT_TYPE_FREE ((sal_Size)(0x04))
#define RTL_ARENA_SEGMENT_TYPE_USED ((sal_Size)(0x08))

typedef struct rtl_arena_segment_st rtl_arena_segment_type;
struct rtl_arena_segment_st
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
#define RTL_ARENA_FREELIST_SIZE (sizeof(void*) * 8)
#define RTL_ARENA_HASH_SIZE     64

#define RTL_ARENA_FLAG_RESCALE  1 /* within hash rescale operation */

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

    sal_Size                  m_qcache_max;
    rtl_cache_type **         m_qcache_ptr;
};


/** gp_default_arena
 *  default arena with pagesize quantum
 *
 *  @internal
 */
extern rtl_arena_type * gp_default_arena;


#ifdef __cplusplus
}
#endif

#endif /* INCLUDED_RTL_ALLOC_ARENA_H */
