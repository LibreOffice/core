/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: reader.h,v $
 * $Revision: 1.7 $
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

   @since UDK 3.2.0
 */
sal_Bool SAL_CALL typereg_reader_create(
    void const * buffer, sal_uInt32 length, sal_Bool copy,
    enum typereg_Version maxVersion, void ** result)
    SAL_THROW_EXTERN_C();

/**
   Increments the reference count of a type reader.

   @param handle a handle on a type reader; may be null

   @since UDK 3.2.0
 */
void SAL_CALL typereg_reader_acquire(void * handle) SAL_THROW_EXTERN_C();

/**
   Decrements the reference count of a type reader.

   <p>If the reference count drops to zero, the type reader is destroyed.</p>

   @param handle a handle on a type reader; may be null

   @since UDK 3.2.0
 */
void SAL_CALL typereg_reader_release(void * handle) SAL_THROW_EXTERN_C();

/**
   Returns the binary blob version of a type reader.

   @param handle a handle on a type reader; may be null

   @return the version of the binary blob from which the type reader was
   constructed; if <code>handle</code> is null, <code>TYPEREG_VERSION_0</code>
   is returned

   @since UDK 3.2.0
 */
enum typereg_Version SAL_CALL typereg_reader_getVersion(void * handle)
    SAL_THROW_EXTERN_C();

/**
   Returns the documentation of a type reader.

   @param handle a handle on a type reader; may be null

   @param result an out-parameter obtaining the documentation string; must not
   be null; if <code>handle</code> is null, an empty string is returned; if an
   out-of-memory condition occurs, a pointer to a null pointer is returned

   @since UDK 3.2.0
 */
void SAL_CALL typereg_reader_getDocumentation(
    void * handle, rtl_uString ** result) SAL_THROW_EXTERN_C();

/**
   Returns the file name of a type reader.

   @param handle a handle on a type reader; may be null

   @param result an out-parameter obtaining the file name string; must not be
   null; if <code>handle</code> is null, an empty string is returned; if an
   out-of-memory condition occurs, a pointer to a null pointer is returned

   @since UDK 3.2.0
   @deprecated
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

   @since UDK 3.2.0
 */
enum RTTypeClass SAL_CALL typereg_reader_getTypeClass(void * handle)
    SAL_THROW_EXTERN_C();

/**
   Returns whether a type reader is published.

   @param handle a handle on a type reader; may be null

   @return whether the type reader is published; if <code>handle</code> is null,
   <code>sal_False</code> is returned

   @since UDK 3.2.0
 */
sal_Bool SAL_CALL typereg_reader_isPublished(void * handle)
    SAL_THROW_EXTERN_C();

/**
   Returns the type name of a type reader.

   @param handle a handle on a type reader; may be null

   @param result an out-parameter obtaining the type name string; must not be
   null; if <code>handle</code> is null, an empty string is returned; if an
   out-of-memory condition occurs, a pointer to a null pointer is returned

   @since UDK 3.2.0
 */
void SAL_CALL typereg_reader_getTypeName(void * handle, rtl_uString ** result)
    SAL_THROW_EXTERN_C();

/**
   Returns the number of super types of a type reader.

   @param handle a handle on a type reader; may be null

   @return the number of super types of the type reader; if <code>handle</code>
   is null, zero is returned

   @since UDK 3.2.0
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

   @since UDK 3.2.0
 */
void SAL_CALL typereg_reader_getSuperTypeName(
    void * handle, rtl_uString ** result, sal_uInt16 index)
    SAL_THROW_EXTERN_C();

/**
   Returns the number of fields of a type reader.

   @param handle a handle on a type reader; may be null

   @return the number of fields of the type reader; if <code>handle</code> is
   null, zero is returned

   @since UDK 3.2.0
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

   @since UDK 3.2.0
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

   @since UDK 3.2.0
   @deprecated
 */
void SAL_CALL typereg_reader_getFieldFileName(
    void * handle, rtl_uString ** result, sal_uInt16 index)
    SAL_THROW_EXTERN_C();

/**
   Returns the flags of a field of a type reader.

   @param handle a handle on a type reader; must not be null

   @param index a valid index into the range of fields of the given type reader

   @return the flags of the given field of the type reader

   @since UDK 3.2.0
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

   @since UDK 3.2.0
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

   @since UDK 3.2.0
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

   @since UDK 3.2.0
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

   @since UDK 3.2.0
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

   @since UDK 3.2.0
 */
void SAL_CALL typereg_reader_getMethodDocumentation(
    void * handle, rtl_uString ** result, sal_uInt16 index)
    SAL_THROW_EXTERN_C();

/**
   Returns the flags of a method of a type reader.

   @param handle a handle on a type reader; must not be null

   @param index a valid index into the range of methods of the given type reader

   @return the flags of the given method of the type reader

   @since UDK 3.2.0
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

   @since UDK 3.2.0
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

   @since UDK 3.2.0
 */
void SAL_CALL typereg_reader_getMethodReturnTypeName(
    void * handle, rtl_uString ** result, sal_uInt16 index)
    SAL_THROW_EXTERN_C();

/**
   Returns the number of parameters of a method of a type reader.

   @param handle a handle on a type reader; must not be null

   @param index a valid index into the range of methods of the given type reader

   @return the number of parameters of the given method of the type reader

   @since UDK 3.2.0
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

   @since UDK 3.2.0
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

   @since UDK 3.2.0
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

   @since UDK 3.2.0
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

   @since UDK 3.2.0
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

   @since UDK 3.2.0
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

   @since UDK 3.2.0
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

   @since UDK 3.2.0
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

   @since UDK 3.2.0
 */
RTFieldAccess SAL_CALL typereg_reader_getReferenceFlags(
    void * handle, sal_uInt16 index) SAL_THROW_EXTERN_C();

/**
   Returns the sort of a reference of a type reader.

   @param handle a handle on a type reader; must not be null

   @param index a valid index into the range of references of the given type
   reader

   @return the sort of the given reference of the type reader

   @since UDK 3.2.0
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

   @since UDK 3.2.0
 */
void SAL_CALL typereg_reader_getReferenceTypeName(
    void * handle, rtl_uString ** result, sal_uInt16 index)
    SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif

#endif
