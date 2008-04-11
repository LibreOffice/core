/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: any2.h,v $
 * $Revision: 1.12 $
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
#ifndef _UNO_ANY2_H_
#define _UNO_ANY2_H_

#include <sal/types.h>
#include <uno/data.h>

#ifdef __cplusplus
extern "C"
{
#endif

#if defined( SAL_W32)
#pragma pack(push, 8)
#elif defined(SAL_OS2)
#pragma pack(push, 8)
#endif

struct _typelib_TypeDescriptionReference;
struct _typelib_TypeDescription;
struct _uno_Mapping;

/** This is the binary specification of an UNO any.
*/
typedef struct _uno_Any
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

#if defined( SAL_W32) ||  defined(SAL_OS2)
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
void SAL_CALL uno_any_assign(
    uno_Any * pDest, void * pSource,
    struct _typelib_TypeDescription * pTypeDescr,
    uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();
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
void SAL_CALL uno_type_any_assign(
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
void SAL_CALL uno_any_construct(
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
void SAL_CALL uno_type_any_construct(
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
void SAL_CALL uno_any_constructAndConvert(
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
void SAL_CALL uno_type_any_constructAndConvert(
    uno_Any * pDest, void * pSource,
    struct _typelib_TypeDescriptionReference * pType,
    struct _uno_Mapping * mapping )
    SAL_THROW_EXTERN_C();

/** Destructs an any.

    @param pValue           pointer to any
    @param release          function called each time an interface needs to be released;
                            defaults (0) to uno
*/
void SAL_CALL uno_any_destruct(
    uno_Any * pValue, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();

/** Sets value to void.

    @param pValue           pointer to any
    @param release          function called each time an interface needs to be released;
                            defaults (0) to uno
*/
void SAL_CALL uno_any_clear(
    uno_Any * pValue, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif

#endif
