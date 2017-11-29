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

#ifndef INCLUDED_REGISTRY_READER_H
#define INCLUDED_REGISTRY_READER_H

#include <registry/regdllapi.h>
#include <registry/types.hxx>
#include <registry/version.h>

#include <rtl/ustring.h>
#include <sal/types.h>

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

   @return false iff an out-of-memory condition occurred, in which case
   <code>result</code> is left unchanged, and no type reader is created

   @since UDK 3.2.0
 */
REG_DLLPUBLIC bool typereg_reader_create(
    void const * buffer, sal_uInt32 length, bool copy,
    typereg_Version maxVersion, void ** result);

/**
   Increments the reference count of a type reader.

   @param handle a handle on a type reader; may be null

   @since UDK 3.2.0
 */
REG_DLLPUBLIC void typereg_reader_acquire(void * handle);

/**
   Decrements the reference count of a type reader.

   <p>If the reference count drops to zero, the type reader is destroyed.</p>

   @param handle a handle on a type reader; may be null

   @since UDK 3.2.0
 */
REG_DLLPUBLIC void typereg_reader_release(void * handle);

/**
   Returns the binary blob version of a type reader.

   @param handle a handle on a type reader; may be null

   @return the version of the binary blob from which the type reader was
   constructed; if <code>handle</code> is null, <code>TYPEREG_VERSION_0</code>
   is returned

   @since UDK 3.2.0
 */
REG_DLLPUBLIC typereg_Version typereg_reader_getVersion(void const * handle);

/**
   Returns the documentation of a type reader.

   @param handle a handle on a type reader; may be null

   @param result an out-parameter obtaining the documentation string; must not
   be null; if <code>handle</code> is null, an empty string is returned; if an
   out-of-memory condition occurs, a pointer to a null pointer is returned

   @since UDK 3.2.0
 */
REG_DLLPUBLIC void typereg_reader_getDocumentation(
    void * handle, rtl_uString ** result);

/**
   Returns the file name of a type reader.

   @param handle a handle on a type reader; may be null

   @param result an out-parameter obtaining the file name string; must not be
   null; if <code>handle</code> is null, an empty string is returned; if an
   out-of-memory condition occurs, a pointer to a null pointer is returned

   @since UDK 3.2.0
   @deprecated
 */
REG_DLLPUBLIC void typereg_reader_getFileName(void * handle, rtl_uString ** result);

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
REG_DLLPUBLIC RTTypeClass typereg_reader_getTypeClass(void * handle);

/**
   Returns whether a type reader is published.

   @param handle a handle on a type reader; may be null

   @return whether the type reader is published; if <code>handle</code> is null,
   <code>sal_False</code> is returned

   @since UDK 3.2.0
 */
REG_DLLPUBLIC bool typereg_reader_isPublished(void * handle);

/**
   Returns the type name of a type reader.

   @param handle a handle on a type reader; may be null

   @param result an out-parameter obtaining the type name string; must not be
   null; if <code>handle</code> is null, an empty string is returned; if an
   out-of-memory condition occurs, a pointer to a null pointer is returned

   @since UDK 3.2.0
 */
REG_DLLPUBLIC void typereg_reader_getTypeName(void * handle, rtl_uString ** result);

/**
   Returns the number of super types of a type reader.

   @param handle a handle on a type reader; may be null

   @return the number of super types of the type reader; if <code>handle</code>
   is null, zero is returned

   @since UDK 3.2.0
 */
REG_DLLPUBLIC sal_uInt16 typereg_reader_getSuperTypeCount(void * handle);

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
REG_DLLPUBLIC void typereg_reader_getSuperTypeName(
    void * handle, rtl_uString ** result, sal_uInt16 index);

/**
   Returns the number of fields of a type reader.

   @param handle a handle on a type reader; may be null

   @return the number of fields of the type reader; if <code>handle</code> is
   null, zero is returned

   @since UDK 3.2.0
 */
REG_DLLPUBLIC sal_uInt16 typereg_reader_getFieldCount(void * handle);

/**
   Returns the documentation of a field of a type reader.

   @param handle a handle on a type reader; must not be null

   @param result an out-parameter obtaining the field's documentation string;
   must not be null; if an out-of-memory condition occurs, a pointer to a null
   pointer is returned

   @param index a valid index into the range of fields of the given type reader

   @since UDK 3.2.0
 */
REG_DLLPUBLIC void typereg_reader_getFieldDocumentation(
    void * handle, rtl_uString ** result, sal_uInt16 index);

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
REG_DLLPUBLIC void typereg_reader_getFieldFileName(
    void * handle, rtl_uString ** result, sal_uInt16 index);

/**
   Returns the flags of a field of a type reader.

   @param handle a handle on a type reader; must not be null

   @param index a valid index into the range of fields of the given type reader

   @return the flags of the given field of the type reader

   @since UDK 3.2.0
 */
REG_DLLPUBLIC RTFieldAccess typereg_reader_getFieldFlags(
    void * handle, sal_uInt16 index);

/**
   Returns the name of a field of a type reader.

   @param handle a handle on a type reader; must not be null

   @param result an out-parameter obtaining the field's name string; must not be
   null; if an out-of-memory condition occurs, a pointer to a null pointer is
   returned

   @param index a valid index into the range of fields of the given type reader

   @since UDK 3.2.0
 */
REG_DLLPUBLIC void typereg_reader_getFieldName(
    void * handle, rtl_uString ** result, sal_uInt16 index);

/**
   Returns the type name of a field of a type reader.

   @param handle a handle on a type reader; must not be null

   @param result an out-parameter obtaining the field's type name string; must
   not be null; if an out-of-memory condition occurs, a pointer to a null
   pointer is returned

   @param index a valid index into the range of fields of the given type reader

   @since UDK 3.2.0
 */
REG_DLLPUBLIC void typereg_reader_getFieldTypeName(
    void * handle, rtl_uString ** result, sal_uInt16 index);

/**
   Returns the value of a field of a type reader.

   @param handle a handle on a type reader; must not be null

   @param index a valid index into the range of fields of the given type reader

   @param type an out-parameter obtaining the field value's type; must not be
   null

   @param value an out-parameter obtaining the field value's value; must not be
   null

   @return false iff an out-of-memory condition occurred, in which case
   <code>type</code> and <code>value</code> are left unchanged

   @since UDK 3.2.0
 */
REG_DLLPUBLIC bool typereg_reader_getFieldValue(
    void * handle, sal_uInt16 index, RTValueType * type,
    union RTConstValueUnion * value);

/**
   Returns the number of methods of a type reader.

   @param handle a handle on a type reader; may be null

   @return the number of methods of the type reader; if <code>handle</code> is
   null, zero is returned

   @since UDK 3.2.0
 */
REG_DLLPUBLIC sal_uInt16 typereg_reader_getMethodCount(void * handle);

/**
   Returns the documentation of a method of a type reader.

   @param handle a handle on a type reader; must not be null

   @param result an out-parameter obtaining the methods's documentation string;
   must not be null; if an out-of-memory condition occurs, a pointer to a null
   pointer is returned

   @param index a valid index into the range of methods of the given type reader

   @since UDK 3.2.0
 */
REG_DLLPUBLIC void typereg_reader_getMethodDocumentation(
    void * handle, rtl_uString ** result, sal_uInt16 index);

/**
   Returns the flags of a method of a type reader.

   @param handle a handle on a type reader; must not be null

   @param index a valid index into the range of methods of the given type reader

   @return the flags of the given method of the type reader

   @since UDK 3.2.0
 */
REG_DLLPUBLIC RTMethodMode typereg_reader_getMethodFlags(
    void * handle, sal_uInt16 index);

/**
   Returns the name of a method of a type reader.

   @param handle a handle on a type reader; must not be null

   @param result an out-parameter obtaining the methods's name string; must not
   be null; if an out-of-memory condition occurs, a pointer to a null pointer is
   returned

   @param index a valid index into the range of methods of the given type reader

   @since UDK 3.2.0
 */
REG_DLLPUBLIC void typereg_reader_getMethodName(
    void * handle, rtl_uString ** result, sal_uInt16 index);

/**
   Returns the return type name of a method of a type reader.

   @param handle a handle on a type reader; must not be null

   @param result an out-parameter obtaining the methods's return type name
   string; must not be null; if an out-of-memory condition occurs, a pointer to
   a null pointer is returned

   @param index a valid index into the range of methods of the given type reader

   @since UDK 3.2.0
 */
REG_DLLPUBLIC void typereg_reader_getMethodReturnTypeName(
    void * handle, rtl_uString ** result, sal_uInt16 index);

/**
   Returns the number of parameters of a method of a type reader.

   @param handle a handle on a type reader; must not be null

   @param index a valid index into the range of methods of the given type reader

   @return the number of parameters of the given method of the type reader

   @since UDK 3.2.0
 */
REG_DLLPUBLIC sal_uInt16 typereg_reader_getMethodParameterCount(
    void * handle, sal_uInt16 index);

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
REG_DLLPUBLIC RTParamMode typereg_reader_getMethodParameterFlags(
    void * handle, sal_uInt16 methodIndex, sal_uInt16 parameterIndex);

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
REG_DLLPUBLIC void typereg_reader_getMethodParameterName(
    void * handle, rtl_uString ** result, sal_uInt16 methodIndex,
    sal_uInt16 parameterIndex);

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
REG_DLLPUBLIC void typereg_reader_getMethodParameterTypeName(
    void * handle, rtl_uString ** result, sal_uInt16 methodIndex,
    sal_uInt16 parameterIndex);

/**
   Returns the number of exceptions of a method of a type reader.

   @param handle a handle on a type reader; must not be null

   @param index a valid index into the range of methods of the given type reader

   @return the number of exceptions of the given method of the type reader

   @since UDK 3.2.0
 */
REG_DLLPUBLIC sal_uInt16 typereg_reader_getMethodExceptionCount(
    void * handle, sal_uInt16 index);

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
REG_DLLPUBLIC void typereg_reader_getMethodExceptionTypeName(
    void * handle, rtl_uString ** result, sal_uInt16 methodIndex,
    sal_uInt16 exceptionIndex);

/**
   Returns the number of references of a type reader.

   @param handle a handle on a type reader; may be null

   @return the number of references of the type reader; if <code>handle</code>
   is null, zero is returned

   @since UDK 3.2.0
 */
REG_DLLPUBLIC sal_uInt16 typereg_reader_getReferenceCount(void * handle);

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
REG_DLLPUBLIC void typereg_reader_getReferenceDocumentation(
    void * handle, rtl_uString ** result, sal_uInt16 index);

/**
   Returns the flags of a reference of a type reader.

   @param handle a handle on a type reader; must not be null

   @param index a valid index into the range of references of the given type
   reader

   @return the flags of the given reference of the type reader

   @since UDK 3.2.0
 */
REG_DLLPUBLIC RTFieldAccess typereg_reader_getReferenceFlags(
    void * handle, sal_uInt16 index);

/**
   Returns the sort of a reference of a type reader.

   @param handle a handle on a type reader; must not be null

   @param index a valid index into the range of references of the given type
   reader

   @return the sort of the given reference of the type reader

   @since UDK 3.2.0
 */
REG_DLLPUBLIC RTReferenceType typereg_reader_getReferenceSort(
    void * handle, sal_uInt16 index);

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
REG_DLLPUBLIC void typereg_reader_getReferenceTypeName(
    void * handle, rtl_uString ** result, sal_uInt16 index);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
