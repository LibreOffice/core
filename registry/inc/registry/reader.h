/*************************************************************************
 *
 *  $RCSfile: reader.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 02:42:23 $
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

#ifndef INCLUDED_registry_reader_h
#define INCLUDED_registry_reader_h

#include "registry/types.h"
#include "registry/version.h"

#include "rtl/ustring.h"
#include "sal/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/// @HTML

/**
   Creates a type reader working on a binary blob that represents a UNOIDL type.

   <p>If a non-null handle is returned through <code>result</code>, its
   reference count will be one.  Operations on a non-null handle are not
   multi-thread&ndash;safe.</p>

   @param buffer the binary blob representing the type; must point to at least
   <code>length</code> bytes, and need only be byte-aligned

   @param length the size in bytes of the binary blob representing the type

   @param copy if true, the type reader creates an internal copy of the given
   buffer, and the given buffer is not accessed after this function returns; if
   false, the type reader works directly on the given buffer, which must remain
   available unmodified until the type reader is destroyed

   @param maxVersion the maximum binary blob version the client is prepared to
   handle; must not be negative

   @param result an out-parameter obtaining a handle on the type reader; must
   not be null; if the given binary blob is malformed, or of a version larger
   than <code>maxVersion</code>, null is returned

   @return false iff an out-of-memory condition occured, in which case
   <code>result</code> is left unchanged, and no type reader is created

   @since #i21150#
 */
sal_Bool SAL_CALL typereg_reader_create(
    void const * buffer, sal_uInt32 length, sal_Bool copy,
    enum typereg_Version maxVersion, void ** result)
    SAL_THROW_EXTERN_C();

/**
   Increments the reference count of a type reader.

   @param handle a handle on a type reader; may be null

   @since #i21150#
 */
void SAL_CALL typereg_reader_acquire(void * handle) SAL_THROW_EXTERN_C();

/**
   Decrements the reference count of a type reader.

   <p>If the reference count drops to zero, the type reader is destroyed.</p>

   @param handle a handle on a type reader; may be null

   @since #i21150#
 */
void SAL_CALL typereg_reader_release(void * handle) SAL_THROW_EXTERN_C();

/**
   Returns the binary blob version of a type reader.

   @param handle a handle on a type reader; may be null

   @return the version of the binary blob from which the type reader was
   constructed; if <code>handle</code> is null, <code>TYPEREG_VERSION_0</code>
   is returned

   @since #i21150#
 */
enum typereg_Version SAL_CALL typereg_reader_getVersion(void * handle)
    SAL_THROW_EXTERN_C();

/**
   Returns the documentation of a type reader.

   @param handle a handle on a type reader; may be null

   @param result an out-parameter obtaining the documentation string; must not
   be null; if <code>handle</code> is null, an empty string is returned; if an
   out-of-memory condition occurs, a pointer to a null pointer is returned

   @since #i21150#
 */
void SAL_CALL typereg_reader_getDocumentation(
    void * handle, rtl_uString ** result) SAL_THROW_EXTERN_C();

/**
   Returns the file name of a type reader.

   @param handle a handle on a type reader; may be null

   @param result an out-parameter obtaining the file name string; must not be
   null; if <code>handle</code> is null, an empty string is returned; if an
   out-of-memory condition occurs, a pointer to a null pointer is returned

   @since #i21150#
 */
void SAL_CALL typereg_reader_getFileName(void * handle, rtl_uString ** result)
    SAL_THROW_EXTERN_C();

/**
   Returns the type class of a type reader.

   <p>This function will always return the type class without the internal
   <code>RT_TYPE_PUBLISHED</code> flag set.  Use
   <code>typereg_reader_isPublished</code> to determine whether a type reader is
   published.</p>

   @param handle a handle on a type reader; may be null

   @return the type class of the type reader; if <code>handle</code> is null,
   <code>RT_TYPE_INVALID</code> is returned

   @since #i21150#
 */
enum RTTypeClass SAL_CALL typereg_reader_getTypeClass(void * handle)
    SAL_THROW_EXTERN_C();

/**
   Returns whether a type reader is published.

   @param handle a handle on a type reader; may be null

   @return whether the type reader is published; if <code>handle</code> is null,
   <code>sal_False</code> is returned

   @since #i21150#
 */
sal_Bool SAL_CALL typereg_reader_isPublished(void * handle)
    SAL_THROW_EXTERN_C();

/**
   Returns the type name of a type reader.

   @param handle a handle on a type reader; may be null

   @param result an out-parameter obtaining the type name string; must not be
   null; if <code>handle</code> is null, an empty string is returned; if an
   out-of-memory condition occurs, a pointer to a null pointer is returned

   @since #i21150#
 */
void SAL_CALL typereg_reader_getTypeName(void * handle, rtl_uString ** result)
    SAL_THROW_EXTERN_C();

/**
   Returns the number of super types of a type reader.

   @param handle a handle on a type reader; may be null

   @return the number of super types of the type reader; if <code>handle</code>
   is null, zero is returned

   @since #i21150#
 */
sal_uInt16 SAL_CALL typereg_reader_getSuperTypeCount(void * handle)
    SAL_THROW_EXTERN_C();

/**
   Returns the type name of a super type of a type reader.

   @param handle a handle on a type reader; must not be null

   @param result an out-parameter obtaining the super type's type name string;
   must not be null; if an out-of-memory condition occurs, a pointer to a null
   pointer is returned

   @param index a valid index into the range of super types of the given type
   reader

   @since #i21150#
 */
void SAL_CALL typereg_reader_getSuperTypeName(
    void * handle, rtl_uString ** result, sal_uInt16 index)
    SAL_THROW_EXTERN_C();

/**
   Returns the number of fields of a type reader.

   @param handle a handle on a type reader; may be null

   @return the number of fields of the type reader; if <code>handle</code> is
   null, zero is returned

   @since #i21150#
 */
sal_uInt16 SAL_CALL typereg_reader_getFieldCount(void * handle)
    SAL_THROW_EXTERN_C();

/**
   Returns the documentation of a field of a type reader.

   @param handle a handle on a type reader; must not be null

   @param result an out-parameter obtaining the field's documentation string;
   must not be null; if an out-of-memory condition occurs, a pointer to a null
   pointer is returned

   @param index a valid index into the range of fields of the given type reader

   @since #i21150#
 */
void SAL_CALL typereg_reader_getFieldDocumentation(
    void * handle, rtl_uString ** result, sal_uInt16 index)
    SAL_THROW_EXTERN_C();

/**
   Returns the file name of a field of a type reader.

   @param handle a handle on a type reader; must not be null

   @param result an out-parameter obtaining the field's file name string; must
   not be null; if an out-of-memory condition occurs, a pointer to a null
   pointer is returned

   @param index a valid index into the range of fields of the given type reader

   @since #i21150#
 */
void SAL_CALL typereg_reader_getFieldFileName(
    void * handle, rtl_uString ** result, sal_uInt16 index)
    SAL_THROW_EXTERN_C();

/**
   Returns the flags of a field of a type reader.

   @param handle a handle on a type reader; must not be null

   @param index a valid index into the range of fields of the given type reader

   @return the flags of the given field of the type reader

   @since #i21150#
 */
RTFieldAccess SAL_CALL typereg_reader_getFieldFlags(
    void * handle, sal_uInt16 index) SAL_THROW_EXTERN_C();

/**
   Returns the name of a field of a type reader.

   @param handle a handle on a type reader; must not be null

   @param result an out-parameter obtaining the field's name string; must not be
   null; if an out-of-memory condition occurs, a pointer to a null pointer is
   returned

   @param index a valid index into the range of fields of the given type reader

   @since #i21150#
 */
void SAL_CALL typereg_reader_getFieldName(
    void * handle, rtl_uString ** result, sal_uInt16 index)
    SAL_THROW_EXTERN_C();

/**
   Returns the type name of a field of a type reader.

   @param handle a handle on a type reader; must not be null

   @param result an out-parameter obtaining the field's type name string; must
   not be null; if an out-of-memory condition occurs, a pointer to a null
   pointer is returned

   @param index a valid index into the range of fields of the given type reader

   @since #i21150#
 */
void SAL_CALL typereg_reader_getFieldTypeName(
    void * handle, rtl_uString ** result, sal_uInt16 index)
    SAL_THROW_EXTERN_C();

/**
   Returns the value of a field of a type reader.

   @param handle a handle on a type reader; must not be null

   @param index a valid index into the range of fields of the given type reader

   @param type an out-parameter obtaining the field value's type; must not be
   null

   @param result an out-parameter obtaining the field value's value; must not be
   null

   @return false iff an out-of-memory condition occured, in which case
   <code>type</code> and <code>value</code> are left unchanged

   @since #i21150#
 */
sal_Bool SAL_CALL typereg_reader_getFieldValue(
    void * handle, sal_uInt16 index, enum RTValueType * type,
    union RTConstValueUnion * value)
    SAL_THROW_EXTERN_C();

/**
   Returns the number of methods of a type reader.

   @param handle a handle on a type reader; may be null

   @return the number of methods of the type reader; if <code>handle</code> is
   null, zero is returned

   @since #i21150#
 */
sal_uInt16 SAL_CALL typereg_reader_getMethodCount(void * handle)
    SAL_THROW_EXTERN_C();

/**
   Returns the documentation of a method of a type reader.

   @param handle a handle on a type reader; must not be null

   @param result an out-parameter obtaining the methods's documentation string;
   must not be null; if an out-of-memory condition occurs, a pointer to a null
   pointer is returned

   @param index a valid index into the range of methods of the given type reader

   @since #i21150#
 */
void SAL_CALL typereg_reader_getMethodDocumentation(
    void * handle, rtl_uString ** result, sal_uInt16 index)
    SAL_THROW_EXTERN_C();

/**
   Returns the flags of a method of a type reader.

   @param handle a handle on a type reader; must not be null

   @param index a valid index into the range of methods of the given type reader

   @return the flags of the given method of the type reader

   @since #i21150#
 */
enum RTMethodMode SAL_CALL typereg_reader_getMethodFlags(
    void * handle, sal_uInt16 index) SAL_THROW_EXTERN_C();

/**
   Returns the name of a method of a type reader.

   @param handle a handle on a type reader; must not be null

   @param result an out-parameter obtaining the methods's name string; must not
   be null; if an out-of-memory condition occurs, a pointer to a null pointer is
   returned

   @param index a valid index into the range of methods of the given type reader

   @since #i21150#
 */
void SAL_CALL typereg_reader_getMethodName(
    void * handle, rtl_uString ** result, sal_uInt16 index)
    SAL_THROW_EXTERN_C();

/**
   Returns the return type name of a method of a type reader.

   @param handle a handle on a type reader; must not be null

   @param result an out-parameter obtaining the methods's return type name
   string; must not be null; if an out-of-memory condition occurs, a pointer to
   a null pointer is returned

   @param index a valid index into the range of methods of the given type reader

   @since #i21150#
 */
void SAL_CALL typereg_reader_getMethodReturnTypeName(
    void * handle, rtl_uString ** result, sal_uInt16 index)
    SAL_THROW_EXTERN_C();

/**
   Returns the number of parameters of a method of a type reader.

   @param handle a handle on a type reader; must not be null

   @param index a valid index into the range of methods of the given type reader

   @return the number of parameters of the given method of the type reader

   @since #i21150#
 */
sal_uInt16 SAL_CALL typereg_reader_getMethodParameterCount(
    void * handle, sal_uInt16 index) SAL_THROW_EXTERN_C();

/**
   Returns the flags of a parameter of a method of a type reader.

   @param handle a handle on a type reader; must not be null

   @param methodIndex a valid index into the range of methods of the given type
   reader

   @param parameterIndex a valid index into the range of parameters of the given
   method

   @return the flags of the given parameter of the given method of the type
   reader

   @since #i21150#
 */
enum RTParamMode SAL_CALL typereg_reader_getMethodParameterFlags(
    void * handle, sal_uInt16 methodIndex, sal_uInt16 parameterIndex)
    SAL_THROW_EXTERN_C();

/**
   Returns the name of a parameter of a method of a type reader.

   @param handle a handle on a type reader; must not be null

   @param result an out-parameter obtaining the parameter's name string; must
   not be null; if an out-of-memory condition occurs, a pointer to a null
   pointer is returned

   @param methodIndex a valid index into the range of methods of the given type
   reader

   @param parameterIndex a valid index into the range of parameters of the given
   method

   @since #i21150#
 */
void SAL_CALL typereg_reader_getMethodParameterName(
    void * handle, rtl_uString ** result, sal_uInt16 methodIndex,
    sal_uInt16 parameterIndex)
    SAL_THROW_EXTERN_C();

/**
   Returns the type name of a parameter of a method of a type reader.

   @param handle a handle on a type reader; must not be null

   @param result an out-parameter obtaining the parameter's type name string;
   must not be null; if an out-of-memory condition occurs, a pointer to a null
   pointer is returned

   @param methodIndex a valid index into the range of methods of the given type
   reader

   @param parameterIndex a valid index into the range of parameters of the given
   method

   @since #i21150#
 */
void SAL_CALL typereg_reader_getMethodParameterTypeName(
    void * handle, rtl_uString ** result, sal_uInt16 methodIndex,
    sal_uInt16 parameterIndex)
    SAL_THROW_EXTERN_C();

/**
   Returns the number of exceptions of a method of a type reader.

   @param handle a handle on a type reader; must not be null

   @param index a valid index into the range of methods of the given type reader

   @return the number of exceptions of the given method of the type reader

   @since #i21150#
 */
sal_uInt16 SAL_CALL typereg_reader_getMethodExceptionCount(
    void * handle, sal_uInt16 index) SAL_THROW_EXTERN_C();

/**
   Returns the type name of an exception of a method of a type reader.

   @param handle a handle on a type reader; must not be null

   @param result an out-parameter obtaining the exception's type name string;
   must not be null; if an out-of-memory condition occurs, a pointer to a null
   pointer is returned

   @param methodIndex a valid index into the range of methods of the given type
   reader

   @param exceptionIndex a valid index into the range of exceptions of the given
   method

   @since #i21150#
 */
void SAL_CALL typereg_reader_getMethodExceptionTypeName(
    void * handle, rtl_uString ** result, sal_uInt16 methodIndex,
    sal_uInt16 exceptionIndex)
    SAL_THROW_EXTERN_C();

/**
   Returns the number of references of a type reader.

   @param handle a handle on a type reader; may be null

   @return the number of references of the type reader; if <code>handle</code>
   is null, zero is returned

   @since #i21150#
 */
sal_uInt16 SAL_CALL typereg_reader_getReferenceCount(void * handle)
    SAL_THROW_EXTERN_C();

/**
   Returns the documentation of a reference of a type reader.

   @param handle a handle on a type reader; must not be null

   @param result an out-parameter obtaining the reference's documentation
   string; must not be null; if an out-of-memory condition occurs, a pointer to
   a null pointer is returned

   @param index a valid index into the range of references of the given type
   reader

   @since #i21150#
 */
void SAL_CALL typereg_reader_getReferenceDocumentation(
    void * handle, rtl_uString ** result, sal_uInt16 index)
    SAL_THROW_EXTERN_C();

/**
   Returns the flags of a reference of a type reader.

   @param handle a handle on a type reader; must not be null

   @param index a valid index into the range of references of the given type
   reader

   @return the flags of the given reference of the type reader

   @since #i21150#
 */
RTFieldAccess SAL_CALL typereg_reader_getReferenceFlags(
    void * handle, sal_uInt16 index) SAL_THROW_EXTERN_C();

/**
   Returns the sort of a reference of a type reader.

   @param handle a handle on a type reader; must not be null

   @param index a valid index into the range of references of the given type
   reader

   @return the sort of the given reference of the type reader

   @since #i21150#
 */
enum RTReferenceType SAL_CALL typereg_reader_getReferenceSort(
    void * handle, sal_uInt16 index) SAL_THROW_EXTERN_C();

/**
   Returns the type name of a reference of a type reader.

   @param handle a handle on a type reader; must not be null

   @param result an out-parameter obtaining the reference's type name string;
   must not be null; if an out-of-memory condition occurs, a pointer to a null
   pointer is returned

   @param index a valid index into the range of references of the given type
   reader

   @since #i21150#
 */
void SAL_CALL typereg_reader_getReferenceTypeName(
    void * handle, rtl_uString ** result, sal_uInt16 index)
    SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif

#endif
