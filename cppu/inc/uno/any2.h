/*************************************************************************
 *
 *  $RCSfile: any2.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-09 12:10:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _UNO_ANY2_H_
#define _UNO_ANY2_H_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _UNO_DATA_H_
#include <uno/data.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef SAL_W32
#pragma pack(push, 8)
#elif defined(SAL_OS2)
#pragma pack(8)
#endif

typedef struct _typelib_TypeDescription typelib_TypeDescription;

/** This is the binary specification of an UNO any.
    <br>
*/
typedef struct _uno_Any
{
    /** type of value<br>
    */
    typelib_TypeDescriptionReference *  pType;
    /** pointer to value<br>
    */
    void *                              pData;
} uno_Any;

#ifdef SAL_W32
#pragma pack(pop)
#elif defined(SAL_OS2)
#pragma pack()
#endif

/** Assign an any with a given value.
    Interfaces are acquired or released by the given callback functions.
    <br>
    @param pDest            pointer memory of destination any
    @param pSource          pointer to source value; defaults (0) to default constructed value
    @param pTypeDescr       type description of value; defaults (0) to void
    @param acquire          function called each time an interface needs to be acquired; defaults (0) to uno
    @param release          function called each time an interface needs to be released; defaults (0) to uno
*/
SAL_DLLEXPORT void SAL_CALL uno_any_assign(
    uno_Any * pDest, void * pSource,
    typelib_TypeDescription * pTypeDescr,
    uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();
/** Assign an any with a given value.
    Interfaces are acquired or released by the given callback functions.
    <br>
    @param pDest            pointer memory of destination any
    @param pSource          pointer to source value; defaults (0) to default constructed value
    @param pTypeDescr       type description of value; defaults (0) to void
    @param acquire          function called each time an interface needs to be acquired; defaults (0) to uno
    @param release          function called each time an interface needs to be released; defaults (0) to uno
*/
SAL_DLLEXPORT void SAL_CALL uno_type_any_assign(
    uno_Any * pDest, void * pSource,
    typelib_TypeDescriptionReference * pType,
    uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();

/** Constructs an any with a given value.
    Interfaces are acquired by the given callback function.
    <br>
    @param pDest            pointer memory of destination any
    @param pSource          pointer to source value; defaults (0) to default constructed value
    @param pTypeDescr       type description of value; defaults (0) to void
    @param acquire          function called each time an interface needs to be acquired; defaults (0) to uno
*/
SAL_DLLEXPORT void SAL_CALL uno_any_construct(
    uno_Any * pDest, void * pSource,
    typelib_TypeDescription * pTypeDescr,
    uno_AcquireFunc acquire )
    SAL_THROW_EXTERN_C();
/** Constructs an any with a given value.
    Interfaces are acquired by the given callback function.
    <br>
    @param pDest            pointer memory of destination any
    @param pSource          pointer to source value; defaults (0) to default constructed value
    @param pType            type of value; defaults (0) to void
    @param acquire          function called each time an interface needs to be acquired; defaults (0) to uno
*/
SAL_DLLEXPORT void SAL_CALL uno_type_any_construct(
    uno_Any * pDest, void * pSource,
    typelib_TypeDescriptionReference * pType,
    uno_AcquireFunc acquire )
    SAL_THROW_EXTERN_C();

/** Constructs an any with a given value and converts/ maps interfaces.
    <br>
    @param pDest            pointer memory of destination any
    @param pSource          pointer to source value; defaults (0) to default constructed value
    @param pTypeDescr       type description of value; defaults (0) to void
    @param mapping          mapping to convert/ map interfaces
*/
SAL_DLLEXPORT void SAL_CALL uno_any_constructAndConvert(
    uno_Any * pDest, void * pSource,
    typelib_TypeDescription * pTypeDescr,
    uno_Mapping * mapping )
    SAL_THROW_EXTERN_C();
/** Constructs an any with a given value and converts/ maps interfaces.
    <br>
    @param pDest            pointer memory of destination any
    @param pSource          pointer to source value; defaults (0) to default constructed value
    @param pType            type of value; defaults (0) to void
    @param mapping          mapping to convert/ map interfaces
*/
SAL_DLLEXPORT void SAL_CALL uno_type_any_constructAndConvert(
    uno_Any * pDest, void * pSource,
    typelib_TypeDescriptionReference * pType,
    uno_Mapping * mapping )
    SAL_THROW_EXTERN_C();

/** Destructs an any.
    <br>
    @param pValue           pointer to any
    @param release          function called each time an interface needs to be released. defaults (0) to uno
*/
SAL_DLLEXPORT void SAL_CALL uno_any_destruct(
    uno_Any * pValue, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif

#endif
