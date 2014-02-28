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

#include <sal/config.h>

#include <sal/saldllapi.h>
#include <sal/types.h>

#ifdef __cplusplus
extern "C" {
#endif


/** Allocate memory.

    A call to this function will return NULL upon the requested
    memory size being either zero or larger than currently allocatable.

    @param  Bytes [in] memory size.
    @return pointer to allocated memory.
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

    @param  Ptr   [in] pointer to previously allocated memory.
    @param  Bytes [in] new memory size.
    @return pointer to reallocated memory. May differ from Ptr.
 */
SAL_DLLPUBLIC void * SAL_CALL rtl_reallocateMemory (
    void *   Ptr,
    sal_Size Bytes
) SAL_THROW_EXTERN_C();


/** Free memory.
    @param  Ptr   [in] pointer to previously allocated memory.
    @return none. Memory is released. Ptr is invalid.
 */
SAL_DLLPUBLIC void SAL_CALL rtl_freeMemory (
    void * Ptr
) SAL_THROW_EXTERN_C();


/** Allocate and zero memory.

    A call to this function will return NULL upon the requested
    memory size being either zero or larger than currently allocatable.

    @param  Bytes [in] memory size.
    @return pointer to allocated and zero'ed memory.
 */
SAL_DLLPUBLIC void * SAL_CALL rtl_allocateZeroMemory (
    sal_Size Bytes
) SAL_THROW_EXTERN_C();


/** Zero and free memory.
    @param  Ptr   [in] pointer to previously allocated memory.
    @param  Bytes [in] memory size.
    @return none. Memory is zero'ed and released. Ptr is invalid.
 */
SAL_DLLPUBLIC void SAL_CALL rtl_freeZeroMemory (
    void *   Ptr,
    sal_Size Bytes
) SAL_THROW_EXTERN_C();


/** Allocate memory.

    A call to this function will return NULL upon the requested
    memory size being either zero or larger than currently allocatable.

    @param Alignment alignment in bytes.
    @param  Bytes [in] memory size.
    @return pointer to allocated memory.
 */
SAL_DLLPUBLIC void* SAL_CALL rtl_allocateAlignedMemory (
    sal_Size Alignment,
    sal_Size Bytes
) SAL_THROW_EXTERN_C();


/** Free memory allocated with rtl_allocateAlignedMemory.
    @param  Ptr   [in] pointer to previously allocated memory.
    @return none. Memory is released. Ptr is invalid.
 */
SAL_DLLPUBLIC void SAL_CALL rtl_freeAlignedMemory (
    void *   Ptr
) SAL_THROW_EXTERN_C();


/** Opaque rtl_arena_type.
 */
typedef struct rtl_arena_st rtl_arena_type;

#define RTL_ARENA_NAME_LENGTH 31


/** rtl_arena_create()
 *
 *  @param  pName             [in] descriptive name; for debugging purposes.
 *  @param  quantum           [in] resource allocation unit / granularity; rounded up to next power of 2.
 *  @param  quantum_cache_max [in] max resources to cache; rounded up to next multiple of quantum; usually 0.
 *  @param  source_arena      [in] passed as argument to source_alloc, source_free; usually NULL.
 *  @param  source_alloc      [in] function to allocate resources; usually rtl_arena_alloc.
 *  @param  source_free       [in] function to free resources; usually rtl_arena_free.
 *  @param  nFlags            [in] flags; usually 0.
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


/** rtl_arena_destroy()
 *
 *  @param  pArena [in] the arena to destroy.
 *  @return None
 *
 *  @see rtl_arena_create()
 */
SAL_DLLPUBLIC void SAL_CALL rtl_arena_destroy (
    rtl_arena_type * pArena
) SAL_THROW_EXTERN_C();


/** rtl_arena_alloc()
 *
 *  @param  pArena [in]    arena from which resource is allocated.
 *  @param  pBytes [inout] size of resource to allocate.
 *
 *  @return allocated resource, or NULL upon failure.
 *
 *  @see rtl_arena_free()
 */
SAL_DLLPUBLIC void * SAL_CALL rtl_arena_alloc (
    rtl_arena_type * pArena,
    sal_Size *       pBytes
) SAL_THROW_EXTERN_C();


/** rtl_arena_free()
 *
 *  @param  pArena [in] arena from which resource was allocated.
 *  @param  pAddr  [in] resource to free.
 *  @param  nBytes [in] size of resource.
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

#define RTL_CACHE_FLAG_BULKDESTROY 1

/** rtl_cache_create()
 *
 *  @param  pName       [in] descriptive name; for debugging purposes.
 *  @param  nObjSize    [in] object size.
 *  @param  nObjAlign   [in] object alignment; usually 0 for suitable default.
 *  @param  constructor [in] object constructor callback function; returning 1 for success or 0 for failure.
 *  @param  destructor  [in] object destructor callback function.
 *  @param  reclaim     [in] reclaim callback function.
 *  @param  pUserArg    [in] opaque argument passed to callback functions.
 *  @param  pSource     [in] opaque argument passed to callback functions.
 *  @param  nFlags      [in] flags.
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


/** rtl_cache_destroy()
 *
 *  @param  pCache [in] the cache to destroy.
 *
 *  @return None.
 *
 *  @see rtl_cache_create()
 */
SAL_DLLPUBLIC void SAL_CALL rtl_cache_destroy (
    rtl_cache_type * pCache
) SAL_THROW_EXTERN_C();


/** rtl_cache_alloc()
 *
 *  @param  pCache [in] cache from which object is allocated.
 *
 *  @return pointer to allocated object, or NULL upon failure.
 */
SAL_DLLPUBLIC void * SAL_CALL rtl_cache_alloc (
    rtl_cache_type * pCache
) SAL_THROW_EXTERN_C();


/** rtl_cache_free()
 *
 *  @param  pCache [in] cache from which object was allocated.
 *  @param  pObj   [in] object to free.
 *
 *  @return None.
 *
 *  @see rtl_cache_alloc()
 */
SAL_DLLPUBLIC void SAL_CALL rtl_cache_free (
    rtl_cache_type * pCache,
    void *           pObj
) SAL_THROW_EXTERN_C();


#ifdef __cplusplus
}
#endif

#endif // INCLUDED_RTL_ALLOC_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
