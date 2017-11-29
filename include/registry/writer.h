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

#ifndef INCLUDED_REGISTRY_WRITER_H
#define INCLUDED_REGISTRY_WRITER_H

#include <registry/regdllapi.h>
#include <registry/types.hxx>
#include <registry/version.h>

#include <rtl/ustring.h>
#include <sal/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Creates a type writer working on a binary blob that represents a UNOIDL type.

   <p>Operations on a returned handle are not multi-thread&ndash;safe.</p>

   @param version the version of the created type writer; must not be negative

   @param documentation the documentation; must not be null

   @param fileName the file name; must not be null (deprecated, use an empty string)

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

   @since UDK 3.2.0
 */
REG_DLLPUBLIC void * typereg_writer_create(
    typereg_Version version, rtl_uString const * documentation,
    rtl_uString const * fileName, RTTypeClass typeClass, sal_Bool published,
    rtl_uString const * typeName, sal_uInt16 superTypeCount,
    sal_uInt16 fieldCount, sal_uInt16 methodCount, sal_uInt16 referenceCount)
    SAL_THROW_EXTERN_C();

/**
   Destroys a type writer.

   @param handle a handle on a type writer obtained from a call to
   <code>typereg_writer_create</code>; must not be null

   @since UDK 3.2.0
 */
REG_DLLPUBLIC void typereg_writer_destroy(void * handle) SAL_THROW_EXTERN_C();

/**
   Sets the type name of a super type of a type writer.

   @param handle a handle on a type writer; must not be null

   @param index a valid index into the range of super types of the given type
   writer

   @param typeName the super type name; must not be null

   @return false iff an out-of-memory condition occurred, in which case the type
   writer is not modified

   @since UDK 3.2.0
 */
REG_DLLPUBLIC sal_Bool typereg_writer_setSuperTypeName(
    void const * handle, sal_uInt16 index, rtl_uString const * typeName)
        SAL_THROW_EXTERN_C();

/**
   Sets the data of a field of a type writer.

   @param handle a handle on a type writer; must not be null

   @param index a valid index into the range of fields of the given type writer

   @param documentation the documentation of the field; must not be null

   @param fileName the file name of the field; must not be null (deprecated, use an empty string)

   @param flags the flags of the field

   @param name the name of the field; must not be null

   @param typeName the type name of the field; must not be null

   @param valueType the type of the value of the field

   @param valueValue the value of the value of the field

   @return false iff an out-of-memory condition occurred, in which case the type
   writer is not modified

   @since UDK 3.2.0
 */
REG_DLLPUBLIC sal_Bool typereg_writer_setFieldData(
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

   @return false iff an out-of-memory condition occurred, in which case the type
   writer is not modified

   @since UDK 3.2.0
 */
REG_DLLPUBLIC sal_Bool typereg_writer_setMethodData(
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

   @return false iff an out-of-memory condition occurred, in which case the type
   writer is not modified

   @since UDK 3.2.0
 */
REG_DLLPUBLIC sal_Bool typereg_writer_setMethodParameterData(
    void const * handle, sal_uInt16 methodIndex, sal_uInt16 parameterIndex,
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

   @return false iff an out-of-memory condition occurred, in which case the type
   writer is not modified

   @since UDK 3.2.0
 */
REG_DLLPUBLIC sal_Bool typereg_writer_setMethodExceptionTypeName(
    void const * handle, sal_uInt16 methodIndex, sal_uInt16 exceptionIndex,
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

   @return false iff an out-of-memory condition occurred, in which case the type
   writer is not modified

   @since UDK 3.2.0
 */
REG_DLLPUBLIC sal_Bool typereg_writer_setReferenceData(
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
REG_DLLPUBLIC void const * typereg_writer_getBlob(void * handle, sal_uInt32 * size)
    SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
