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
#ifndef INCLUDED_UNO_ANY2_H
#define INCLUDED_UNO_ANY2_H

#include "cppu/cppudllapi.h"
#include "uno/data.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if defined( _WIN32)
#pragma pack(push, 8)
#endif

struct _typelib_TypeDescriptionReference;
struct _typelib_TypeDescription;
struct _uno_Mapping;

/** This is the binary specification of a UNO any.
*/
typedef struct SAL_DLLPUBLIC_RTTI _uno_Any
{
    /** type of value
    */
    struct _typelib_TypeDescriptionReference * pType;
    /** pointer to value; this may point to pReserved and thus the uno_Any is not anytime
        mem-copyable! You may have to correct the pData pointer to pReserved. Otherwise you need
        not, because the data is stored in heap space.
    */
    void * pData;
    /** reserved space for storing value
    */
    void * pReserved;
} uno_Any;

#if defined( _WIN32)
#pragma pack(pop)
#endif

/** Assign an any with a given value. Interfaces are acquired or released by the given callback
    functions.

    @param pDest            pointer memory of destination any
    @param pSource          pointer to source value; defaults (0) to default constructed value
    @param pTypeDescr       type description of value; defaults (0) to void
    @param acquire          function called each time an interface needs to be acquired;
                            defaults (0) to uno
    @param release          function called each time an interface needs to be released;
                            defaults (0) to uno
*/
CPPU_DLLPUBLIC void SAL_CALL uno_any_assign(
    uno_Any * pDest, void * pSource,
    struct _typelib_TypeDescription * pTypeDescr,
    uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();
/** Assign an any with a given value. Interfaces are acquired or released by the given callback
    functions.

    @param pDest            pointer memory of destination any
    @param pSource          pointer to source value; defaults (0) to default constructed value
    @param pType            type description of value; defaults (0) to void
    @param acquire          function called each time an interface needs to be acquired;
                            defaults (0) to uno
    @param release          function called each time an interface needs to be released;
                            defaults (0) to uno
*/
CPPU_DLLPUBLIC void SAL_CALL uno_type_any_assign(
    uno_Any * pDest, void * pSource,
    struct _typelib_TypeDescriptionReference * pType,
    uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();

/** Constructs an any with a given value. Interfaces are acquired by the given callback function.

    @param pDest            pointer memory of destination any
    @param pSource          pointer to source value; defaults (0) to default constructed value
    @param pTypeDescr       type description of value; defaults (0) to void
    @param acquire          function called each time an interface needs to be acquired;
                            defaults (0) to uno
*/
CPPU_DLLPUBLIC void SAL_CALL uno_any_construct(
    uno_Any * pDest, void * pSource,
    struct _typelib_TypeDescription * pTypeDescr,
    uno_AcquireFunc acquire )
    SAL_THROW_EXTERN_C();
/** Constructs an any with a given value. Interfaces are acquired by the given callback function.

    @param pDest            pointer memory of destination any
    @param pSource          pointer to source value; defaults (0) to default constructed value
    @param pType            type of value; defaults (0) to void
    @param acquire          function called each time an interface needs to be acquired;
                            defaults (0) to uno
*/
CPPU_DLLPUBLIC void SAL_CALL uno_type_any_construct(
    uno_Any * pDest, void * pSource,
    struct _typelib_TypeDescriptionReference * pType,
    uno_AcquireFunc acquire )
    SAL_THROW_EXTERN_C();

/** Constructs an any with a given value and converts/ maps interfaces.

    @param pDest            pointer memory of destination any
    @param pSource          pointer to source value; defaults (0) to default constructed value
    @param pTypeDescr       type description of value; defaults (0) to void
    @param mapping          mapping to convert/ map interfaces
*/
CPPU_DLLPUBLIC void SAL_CALL uno_any_constructAndConvert(
    uno_Any * pDest, void * pSource,
    struct _typelib_TypeDescription * pTypeDescr,
    struct _uno_Mapping * mapping )
    SAL_THROW_EXTERN_C();
/** Constructs an any with a given value and converts/ maps interfaces.

    @param pDest            pointer memory of destination any
    @param pSource          pointer to source value; defaults (0) to default constructed value
    @param pType            type of value; defaults (0) to void
    @param mapping          mapping to convert/ map interfaces
*/
CPPU_DLLPUBLIC void SAL_CALL uno_type_any_constructAndConvert(
    uno_Any * pDest, void * pSource,
    struct _typelib_TypeDescriptionReference * pType,
    struct _uno_Mapping * mapping )
    SAL_THROW_EXTERN_C();

/** Destructs an any.

    @param pValue           pointer to any
    @param release          function called each time an interface needs to be released;
                            defaults (0) to uno
*/
CPPU_DLLPUBLIC void SAL_CALL uno_any_destruct(
    uno_Any * pValue, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();

/** Sets value to void.

    @param pValue           pointer to any
    @param release          function called each time an interface needs to be released;
                            defaults (0) to uno
*/
CPPU_DLLPUBLIC void SAL_CALL uno_any_clear(
    uno_Any * pValue, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
