/*************************************************************************
 *
 *  $RCSfile: data.h,v $
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
#ifndef _UNO_DATA_H_
#define _UNO_DATA_H_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _uno_Mapping uno_Mapping;
typedef struct _typelib_TypeDescription typelib_TypeDescription;
typedef struct _typelib_TypeDescriptionReference typelib_TypeDescriptionReference;
typedef struct _typelib_InterfaceTypeDescription typelib_InterfaceTypeDescription;

/** Generic function pointer declaration to query for an interface.
    <br>
    @param pInterface interface
    @param pTypedemanded interface type
    @return interface pointer
*/
typedef void * (SAL_CALL * uno_QueryInterfaceFunc)(
    void * pInterface, typelib_TypeDescriptionReference * pType );
/** Generic function pointer declaration to acquire an interface.
    <br>
    @param pInterface interface to be acquired
*/
typedef void (SAL_CALL * uno_AcquireFunc)(
    void * pInterface );
/** Generic function pointer declaration to release an interface.
    <br>
    @param pInterface interface to be release
*/
typedef void (SAL_CALL * uno_ReleaseFunc)(
    void * pInterface );

/** Tests if two values are equal. May compare different types (e.g., short to long).
    <br>
    @param pVal1            pointer to a value
    @param pVal1TypeDescr   type description of pVal1
    @param pVal2            pointer to another value
    @param pVal2TypeDescr   type description of pVal2
    @param queryInterface   function called each time two interfaces are tested whether they belong
                            to the same object; defaults (0) to uno
    @param release          function to release queried interfaces; defaults (0) to uno
    @return true if values are equal
*/
SAL_DLLEXPORT sal_Bool SAL_CALL uno_equalData(
    void * pVal1, typelib_TypeDescription * pVal1TypeDescr,
    void * pVal2, typelib_TypeDescription * pVal2TypeDescr,
    uno_QueryInterfaceFunc queryInterface, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();
/** Tests if two values are equal. May compare different types (e.g., short to long).
    <br>
    @param pVal1            pointer to a value
    @param pVal1Type        type of pVal1
    @param pVal2            pointer to another value
    @param pVal2Type        type of pVal2
    @param queryInterface   function called each time two interfaces are tested whether they belong
                            to the same object; defaults (0) to uno
    @param release          function to release queried interfaces; defaults (0) to uno
    @return true if values are equal
*/
SAL_DLLEXPORT sal_Bool SAL_CALL uno_type_equalData(
    void * pVal1, typelib_TypeDescriptionReference * pVal1Type,
    void * pVal2, typelib_TypeDescriptionReference * pVal2Type,
    uno_QueryInterfaceFunc queryInterface, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();

/** Copy construct memory with given value.
    The size of the destination value must be larger or equal to the size of the source value.
    <br>
    @param pDest            pointer to destination value memory
    @param pSource          pointer to source value
    @param pTypeDescr       type description of source
    @param acquire          function called each time an interface needs to be acquired; defaults (0) to uno
*/
SAL_DLLEXPORT void SAL_CALL uno_copyData(
    void * pDest, void * pSource,
    typelib_TypeDescription * pTypeDescr, uno_AcquireFunc acquire )
    SAL_THROW_EXTERN_C();
/** Copy construct memory with given value.
    The size of the destination value must be larger or equal to the size of the source value.
    <br>
    @param pDest            pointer to destination value memory
    @param pSource          pointer to source value
    @param pType            type of source
    @param acquire          function called each time an interface needs to be acquired; defaults (0) to uno
*/
SAL_DLLEXPORT void SAL_CALL uno_type_copyData(
    void * pDest, void * pSource,
    typelib_TypeDescriptionReference * pType, uno_AcquireFunc acquire )
    SAL_THROW_EXTERN_C();

/** Copy construct memory with given value.
    The size of the destination value must be larger or equal to the size of the source value.<br>
    Interfaces are converted/ mapped by mapping parameter.
    <br>
    @param pDest            pointer to destination value memory
    @param pSource          pointer to source value
    @param pTypeDescr       type description of source
    @param mapping          mapping to convert/ map interfaces
*/
SAL_DLLEXPORT void SAL_CALL uno_copyAndConvertData(
    void * pDest, void * pSource,
    typelib_TypeDescription * pTypeDescr, uno_Mapping * mapping )
    SAL_THROW_EXTERN_C();
/** Copy construct memory with given value.
    The size of the destination value must be larger or equal to the size of the source value.<br>
    Interfaces are converted/ mapped by mapping parameter.
    <br>
    @param pDest            pointer to destination value memory
    @param pSource          pointer to source value
    @param pType            type of source
    @param mapping          mapping to convert/ map interfaces
*/
SAL_DLLEXPORT void SAL_CALL uno_type_copyAndConvertData(
    void * pDest, void * pSource,
    typelib_TypeDescriptionReference * pType, uno_Mapping * mapping )
    SAL_THROW_EXTERN_C();

/** Destructs a given value; does <b>not</b> free its memory!
    <br>
    @param pValue           value to be destructed
    @param pTypeDescr       type description of value
    @param release          function called each time an interface pointer needs to be released; defaults (0) to uno
*/
SAL_DLLEXPORT void SAL_CALL uno_destructData(
    void * pValue, typelib_TypeDescription * pTypeDescr, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();
/** Destructs a given value; does <b>not</b> free its memory!
    <br>
    @param pValue           value to be destructed
    @param pType            type of value
    @param release          function called each time an interface pointer needs to be released; defaults (0) to uno
*/
SAL_DLLEXPORT void SAL_CALL uno_type_destructData(
    void * pValue, typelib_TypeDescriptionReference * pType, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();

/** Default constructs a value. All simple types are set to 0, enums are set to their default
    value.
    <br>
    @param pMem             pointer to memory of value to be constructed
    @param pTypeDescr       type description of value to be constructed
*/
SAL_DLLEXPORT void SAL_CALL uno_constructData(
    void * pMem, typelib_TypeDescription * pTypeDescr )
    SAL_THROW_EXTERN_C();
/** Default constructs a value. All simple types are set to 0, enums are set to their default
    value.
    <br>
    @param pMem             pointer to memory of value to be constructed
    @param pType            type of value to be constructed
*/
SAL_DLLEXPORT void SAL_CALL uno_type_constructData(
    void * pMem, typelib_TypeDescriptionReference * pType )
    SAL_THROW_EXTERN_C();

/** Assigns a destination value with a source value. Widening conversion
    <b>without</b> data loss is allowed (e.g., assigning a long with a short).
    Assignment from any value to a value of type Any and vice versa is allowed.
    <br>
    @param pDest            pointer to destination value
    @param pDestTypeDescr   type description of destination value
    @param pSource          pointer to source value; if 0, then destination value will be assigned to default value
    @param pSourceTypeDescr type destination of source value
    @param queryInterface   function called each time an interface needs to be queried; defaults (0) to uno
    @param acquire          function called each time an interface needs to be acquired; defaults (0) to uno
    @param release          function called each time an interface needs to be released; defaults (0) to uno
    @return true if destination has been successfully assigned
*/
SAL_DLLEXPORT sal_Bool SAL_CALL uno_assignData(
    void * pDest, typelib_TypeDescription * pDestTypeDescr,
    void * pSource, typelib_TypeDescription * pSourceTypeDescr,
    uno_QueryInterfaceFunc queryInterface, uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();
/** Assigns a destination value with a source value. Widening conversion
    <b>without</b> data loss is allowed (e.g., assigning a long with a short).
    Assignment from any value to a value of type Any and vice versa is allowed.
    <br>
    @param pDest            pointer to destination value
    @param pDestType        type of destination value
    @param pSource          pointer to source value; if 0, then destination value will be assigned to default value
    @param pSourceType      type of source value
    @param queryInterface   function called each time an interface needs to be queried; defaults (0) to uno
    @param acquire          function called each time an interface needs to be acquired; defaults (0) to uno
    @param release          function called each time an interface needs to be released; defaults (0) to uno
    @return true if destination has been successfully assigned
*/
SAL_DLLEXPORT sal_Bool SAL_CALL uno_type_assignData(
    void * pDest, typelib_TypeDescriptionReference * pDestType,
    void * pSource, typelib_TypeDescriptionReference * pSourceType,
    uno_QueryInterfaceFunc queryInterface, uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif

#endif
