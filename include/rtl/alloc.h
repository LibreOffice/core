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

#ifndef INCLUDED_RTL_ALLOC_H
#define INCLUDED_RTL_ALLOC_H

#include "sal/config.h"

#include "sal/saldllapi.h"
#include "sal/types.h"

#ifdef __cplusplus
extern "C" {
#endif


/** Allocate memory.

    A call to this function will return NULL upon the requested
    memory size being either zero or larger than currently allocatable.

    @param[in] Bytes memory size.
    @return pointer to the allocated memory.
 */
SAL_DLLPUBLIC void * SAL_CALL rtl_allocateMemory (
    sal_Size Bytes
) SAL_THROW_EXTERN_C();


/** Reallocate memory.

    A call to this function with parameter 'Ptr' being NULL
    is equivalent to a rtl_allocateMemory() call.
    A call to this function with parameter 'Bytes' being 0
    is equivalent to a rtl_freeMemory() call.

    @see rtl_allocateMemory()
    @see rtl_freeMemory()

    @param[in] Ptr   pointer to the previously allocated memory.
    @param[in] Bytes new memory size.
    @return pointer to the reallocated memory. May differ from Ptr.
 */
SAL_DLLPUBLIC void * SAL_CALL rtl_reallocateMemory (
    void *   Ptr,
    sal_Size Bytes
) SAL_THROW_EXTERN_C();


/** Free memory.
    @param[in] Ptr pointer to the previously allocated memory.
    @return none. Memory is released. Ptr is invalid.
 */
SAL_DLLPUBLIC void SAL_CALL rtl_freeMemory (
    void * Ptr
) SAL_THROW_EXTERN_C();

/** Allocate and zero memory.

    A call to this function will return NULL upon the requested
    memory size being either zero or larger than currently allocatable.

    @param[in] Bytes memory size.
    @return pointer to the allocated and zero'ed memory.
 */
SAL_DLLPUBLIC void * SAL_CALL rtl_allocateZeroMemory (
    sal_Size Bytes
) SAL_THROW_EXTERN_C();

/** Zero memory

    Fills a block of memory with zeros in a way that is guaranteed to be secure

    @param[in] Ptr   pointer to the previously allocated memory.
    @param[in] Bytes memory size.

    @since LibreOffice 5.0
 */
SAL_DLLPUBLIC void SAL_CALL rtl_secureZeroMemory (
    void *   Ptr,
    sal_Size Bytes
) SAL_THROW_EXTERN_C();


/** Zero and free memory.
    @param[in] Ptr   pointer to the previously allocated memory.
    @param[in] Bytes memory size.
    @return none. Memory is zero'ed with rtl_secureZeroMemory() and released.
                  Ptr is invalid.
 */
SAL_DLLPUBLIC void SAL_CALL rtl_freeZeroMemory (
    void *   Ptr,
    sal_Size Bytes
) SAL_THROW_EXTERN_C();

/** Allocate aligned memory.

    A call to this function will return NULL upon the requested
    memory size being either zero or larger than currently allocatable.

    Memory obtained through this function must be freed with
    rtl_freeAlignedMemory().

    @param[in] Alignment alignment in bytes, must be a power of two multiple of
        sizeof(void*).
    @param[in] Bytes     memory size.
    @return pointer to the allocated memory.

    @since LibreOffice 4.3
 */
SAL_DLLPUBLIC void* SAL_CALL rtl_allocateAlignedMemory (
    sal_Size Alignment,
    sal_Size Bytes
) SAL_THROW_EXTERN_C();


/** Free memory allocated with rtl_allocateAlignedMemory().

    @param[in] Ptr pointer to the previously allocated memory.
    @return none. Memory is released. Ptr is invalid.

    @since LibreOffice 4.3
 */
SAL_DLLPUBLIC void SAL_CALL rtl_freeAlignedMemory (
    void *   Ptr
) SAL_THROW_EXTERN_C();


/** Opaque rtl_arena_type.
 */
typedef struct SAL_DLLPUBLIC_RTTI rtl_arena_st rtl_arena_type;

#define RTL_ARENA_NAME_LENGTH 31


/**
 *  @param[in] pName             descriptive name; for debugging purposes.
 *  @param[in] quantum           resource allocation unit / granularity; rounded up to next power of 2.
 *  @param[in] quantum_cache_max no longer used, should be 0.
 *  @param[in] source_arena      passed as argument to source_alloc, source_free; usually NULL.
 *  @param[in] source_alloc      function to allocate resources; usually rtl_arena_alloc.
 *  @param[in] source_free       function to free resources; usually rtl_arena_free.
 *  @param[in] nFlags            flags; usually 0.
 *
 *  @return pointer to rtl_arena_type, or NULL upon failure.
 *
 *  @see rtl_arena_destroy()
 */
SAL_DLLPUBLIC rtl_arena_type * SAL_CALL rtl_arena_create (
    const char *       pName,
    sal_Size           quantum,
    sal_Size           quantum_cache_max,
    rtl_arena_type *   source_arena,
    void * (SAL_CALL * source_alloc)(rtl_arena_type *, sal_Size *),
    void   (SAL_CALL * source_free) (rtl_arena_type *, void *, sal_Size),
    int                nFlags
) SAL_THROW_EXTERN_C();


/**
 *  @param[in] pArena the arena to destroy.
 *  @return None
 *
 *  @see rtl_arena_create()
 */
SAL_DLLPUBLIC void SAL_CALL rtl_arena_destroy (
    rtl_arena_type * pArena
) SAL_THROW_EXTERN_C();


/**
 *  @param[in]     pArena arena from which resource is allocated.
 *  @param[in,out] pBytes size of resource to allocate.
 *
 *  @return allocated resource, or NULL upon failure.
 *
 *  @see rtl_arena_free()
 */
SAL_DLLPUBLIC void * SAL_CALL rtl_arena_alloc (
    rtl_arena_type * pArena,
    sal_Size *       pBytes
) SAL_THROW_EXTERN_C();


/**
 *  @param[in] pArena arena from which resource was allocated.
 *  @param[in] pAddr  resource to free.
 *  @param[in] nBytes size of resource.
 *
 *  @return None.
 *
 *  @see rtl_arena_alloc()
 */
SAL_DLLPUBLIC void SAL_CALL rtl_arena_free (
    rtl_arena_type * pArena,
    void *           pAddr,
    sal_Size         nBytes
) SAL_THROW_EXTERN_C();


/** Opaque rtl_cache_type.
 */
typedef struct rtl_cache_st rtl_cache_type;

#define RTL_CACHE_NAME_LENGTH 31

#define RTL_CACHE_FLAG_BULKDESTROY 1 /* obsolete */

/**
 *  @param[in] pName       descriptive name; for debugging purposes.
 *  @param[in] nObjSize    object size.
 *  @param[in] nObjAlign   object alignment; usually 0 for suitable default.
 *  @param[in] constructor object constructor callback function; returning 1 for success or 0 for failure.
 *  @param[in] destructor  object destructor callback function.
 *  @param[in] reclaim     reclaim callback function.
 *  @param[in] pUserArg    opaque argument passed to callback functions.
 *  @param[in] pSource     unused argument (should be null).
 *  @param[in] nFlags      flags (unused).
 *
 *  @return pointer to rtl_cache_type, or NULL upon failure.
 *
 *  @see rtl_cache_destroy()
 */
SAL_DLLPUBLIC rtl_cache_type * SAL_CALL rtl_cache_create (
    const char *     pName,
    sal_Size         nObjSize,
    sal_Size         nObjAlign,
    int  (SAL_CALL * constructor)(void * pObj, void * pUserArg),
    void (SAL_CALL * destructor) (void * pObj, void * pUserArg),
    void (SAL_CALL * reclaim)    (void * pUserArg),
    void *           pUserArg,
    rtl_arena_type * pSource,
    int              nFlags
) SAL_THROW_EXTERN_C();


/**
 *  @param[in] pCache the cache to destroy.
 *
 *  @return None.
 *
 *  @see rtl_cache_create()
 */
SAL_DLLPUBLIC void SAL_CALL rtl_cache_destroy (
    rtl_cache_type * pCache
) SAL_THROW_EXTERN_C();


/**
 *  @param[in] pCache cache from which object is allocated.
 *
 *  @return pointer to the allocated object, or NULL upon failure.
 */
SAL_DLLPUBLIC void * SAL_CALL rtl_cache_alloc (
    rtl_cache_type * pCache
) SAL_THROW_EXTERN_C();


/**
 *  @param[in] pCache cache from which object was allocated.
 *  @param[in] pObj   object to free.
 *
 *  @return None.
 *
 *  @see rtl_cache_alloc()
 */
SAL_DLLPUBLIC void SAL_CALL rtl_cache_free (
    rtl_cache_type * pCache,
    void *           pObj
) SAL_THROW_EXTERN_C();


#ifdef LIBO_INTERNAL_ONLY

/** @cond INTERNAL */
/** rtl_alloc_preInit
 *
 * This function, is called at the beginning and again
 * at the end of LibreOfficeKit pre-initialization to enable
 * various optimizations.
 *
 * Its function is to annotate a section @start = true
 * to end (@start = false) via. two calls. Inside this
 * section string allocators are replaced with ones which cause the
 * strings to be staticized at the end of the section.
 *
 * This brings a number of constraints - in particular no
 * string allocated outside the section should be freed
 * inside it, practically this means starting the section
 * as early as possible. No string allocated inside the
 * section will be freed subsequently as they are
 * staticized.
 *
 * This method is not thread-safe, nor intended for use in
 * a threaded context, cf. previous constraints.
 *
 * It is almost certainly not the method that you want,
 * use with extraordinary care referring to the
 * implementation.
 *
 * @since LibreOffice 6.1
 */
SAL_DLLPUBLIC void SAL_CALL rtl_alloc_preInit (
    sal_Bool start
) SAL_THROW_EXTERN_C();
/** @endcond */

#endif

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_RTL_ALLOC_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
