/*************************************************************************
 *
 *  $RCSfile: writer.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 02:43:31 $
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

#ifndef INCLUDED_registry_writer_h
#define INCLUDED_registry_writer_h

#include "registry/types.h"
#include "registry/version.h"

#include "rtl/ustring.h"
#include "sal/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/// @HTML

/**
   Creates a type writer working on a binary blob that represents a UNOIDL type.

   <p>Operations on a returned handle are not multi-thread&ndash;safe.</p>

   @param version the version of the created type writer; must not be negative

   @param documentation the documentation; must not be null

   @param fileName the file name; must not be null

   @param typeClass the type class of the created type writer

   @param published whether the created type writer is published; for a type
   class that cannot be published, this should be false

   @param typeName the type name of the created type writer; must not be null

   @param superTypeCount the number of super types of the created type writer

   @param fieldCount the number of fields of the created type writer

   @param methodCount the number of methods of the created type writer

   @param referenceCount the number of references of the created type writer

   @return a handle on the type writer; if an out-of-memory condition occurs,
   null is returned, and no type writer is created

   @since #i21150#
 */
void * SAL_CALL typereg_writer_create(
    typereg_Version version, rtl_uString const * documentation,
    rtl_uString const * fileName, RTTypeClass typeClass, sal_Bool published,
    rtl_uString const * typeName, sal_uInt16 superTypeCount,
    sal_uInt16 fieldCount, sal_uInt16 methodCount, sal_uInt16 referenceCount)
    SAL_THROW_EXTERN_C();

/**
   Destroys a type writer.

   @param handle a handle on a type writer obtained from a call to
   <code>typereg_writer_create</code>; must not be null

   @since #i21150#
 */
void SAL_CALL typereg_writer_destroy(void * handle) SAL_THROW_EXTERN_C();

/**
   Sets the type name of a super type of a type writer.

   @param handle a handle on a type writer; must not be null

   @param index a valid index into the range of super types of the given type
   writer

   @param typeName the super type name; must not be null

   @return false iff an out-of-memory condition ocurred, in which case the type
   writer is not modified

   @since #i21150#
 */
sal_Bool SAL_CALL typereg_writer_setSuperTypeName(
    void * handle, sal_uInt16 index, rtl_uString const * typeName)
        SAL_THROW_EXTERN_C();

/**
   Sets the data of a field of a type writer.

   @param handle a handle on a type writer; must not be null

   @param index a valid index into the range of fields of the given type writer

   @param documentation the documentation of the field; must not be null

   @param fileName the file name of the field; must not be null

   @param flags the flags of the field

   @param name the name of the field; must not be null

   @param typeName the type name of the field; must not be null

   @param valueType the type of the value of the field

   @param valueValue the value of the value of the field

   @return false iff an out-of-memory condition ocurred, in which case the type
   writer is not modified

   @since #i21150#
 */
sal_Bool SAL_CALL typereg_writer_setFieldData(
    void * handle, sal_uInt16 index, rtl_uString const * documentation,
    rtl_uString const * fileName, RTFieldAccess flags, rtl_uString const * name,
    rtl_uString const * typeName, RTValueType valueType,
    RTConstValueUnion valueValue)
    SAL_THROW_EXTERN_C();

/**
   Sets the data of a method of a type writer.

   @param handle a handle on a type writer; must not be null

   @param index a valid index into the range of methods of the given type writer

   @param documentation the documentation of the method; must not be null

   @param flags the flags of the method

   @param name the name of the method; must not be null

   @param returnTypeName the return type name of the method; must not be null

   @param parameterCount the number of parameters of the method

   @param exceptionCount the number of exceptions of the method

   @return false iff an out-of-memory condition ocurred, in which case the type
   writer is not modified

   @since #i21150#
 */
sal_Bool SAL_CALL typereg_writer_setMethodData(
    void * handle, sal_uInt16 index, rtl_uString const * documentation,
    RTMethodMode flags, rtl_uString const * name,
    rtl_uString const * returnTypeName, sal_uInt16 parameterCount,
    sal_uInt16 exceptionCount)
    SAL_THROW_EXTERN_C();

/**
   Sets the data of a parameter of a method of a type writer.

   @param handle a handle on a type writer; must not be null

   @param methodIndex a valid index into the range of methods of the given type
   writer

   @param parameterIndex a valid index into the range of parameters of the given
   method

   @param flags the flags of the parameter

   @param name the name of the parameter; must not be null

   @param typeName the type name of the parameter; must not be null

   @return false iff an out-of-memory condition ocurred, in which case the type
   writer is not modified

   @since #i21150#
 */
sal_Bool SAL_CALL typereg_writer_setMethodParameterData(
    void * handle, sal_uInt16 methodIndex, sal_uInt16 parameterIndex,
    RTParamMode flags, rtl_uString const * name, rtl_uString const * typeName)
    SAL_THROW_EXTERN_C();

/**
   Sets an exception type name of a method of a type writer.

   @param handle a handle on a type writer; must not be null

   @param methodIndex a valid index into the range of methods of the given type
   writer

   @param exceptionIndex a valid index into the range of exceptions of the given
   method

   @param typeName the exception type name; must not be null

   @return false iff an out-of-memory condition ocurred, in which case the type
   writer is not modified

   @since #i21150#
 */
sal_Bool SAL_CALL typereg_writer_setMethodExceptionTypeName(
    void * handle, sal_uInt16 methodIndex, sal_uInt16 exceptionIndex,
    rtl_uString const * typeName)
    SAL_THROW_EXTERN_C();

/**
   Sets the data of a reference of a type writer.

   @param handle a handle on a type writer; must not be null

   @param index a valid index into the range of references of the given type
   writer

   @param documentation the documentation of the reference; must not be null

   @param sort the sort of the reference

   @param flags the flags of the reference

   @param typeName the type name of the reference; must not be null

   @return false iff an out-of-memory condition ocurred, in which case the type
   writer is not modified

   @since #i21150#
 */
sal_Bool SAL_CALL typereg_writer_setReferenceData(
    void * handle, sal_uInt16 index, rtl_uString const * documentation,
    RTReferenceType sort, RTFieldAccess flags, rtl_uString const * typeName)
    SAL_THROW_EXTERN_C();

/**
   Returns the blob of a type writer.

   @param handle a handle on a type writer; must not be null

   @param size an out-parameter obtaining the size of the blob; must not be null

   @return a (byte-aligned) pointer to the blob; the returned pointer and the
   returned <code>size</code> remain valid until the next function is called on
   the given type writer; if an out-of-memory condition occurs, null is returned
   and <code>size</code> is not modified
 */
void const * SAL_CALL typereg_writer_getBlob(void * handle, sal_uInt32 * size)
    SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif

#endif
