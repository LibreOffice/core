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

#ifndef INCLUDED_REGISTRY_WRITER_HXX
#define INCLUDED_REGISTRY_WRITER_HXX

#include <registry/writer.h>
#include <registry/refltype.hxx>
#include <registry/types.hxx>
#include <registry/version.h>

#include <rtl/ustring.hxx>
#include <sal/types.h>

#include <new>

namespace typereg {

/**
   A type writer working on a binary blob that represents a UNOIDL type.

   <p>Instances of this class are not multi-thread&ndash;safe.</p>

   @since UDK 3.2.0
 */
class Writer {
public:
    /**
       Creates a type writer.

       @param version the version of the created type writer; must not be
       negative

       @param documentation the documentation

       @param fileName the file name (deprecated, use an empty string)

       @param typeClass the type class of the created type writer

       @param published whether the created type writer is published; for a type
       class that cannot be published, this should be false

       @param typeName the type name of the created type writer

       @param superTypeCount the number of super types of the created type
       writer

       @param fieldCount the number of fields of the created type writer

       @param methodCount the number of methods of the created type writer

       @param referenceCount the number of references of the created type writer

       @exception std::bad_alloc is raised if an out-of-memory condition occurs
     */
    Writer(
        typereg_Version version, rtl::OUString const & documentation,
        rtl::OUString const & fileName, RTTypeClass typeClass, bool published,
        rtl::OUString const & typeName, sal_uInt16 superTypeCount,
        sal_uInt16 fieldCount, sal_uInt16 methodCount,
        sal_uInt16 referenceCount):
        m_handle(
            typereg_writer_create(
                version, documentation.pData, fileName.pData, typeClass,
                published, typeName.pData, superTypeCount, fieldCount,
                methodCount, referenceCount))
    {
        if (m_handle == nullptr) {
            throw std::bad_alloc();
        }
    }

    /**
       Destroys this <code>Writer</code> instance.
     */
    ~Writer() {
        typereg_writer_destroy(m_handle);
    }

    /**
       Sets the type name of a super type of this type writer.

       @param index a valid index into the range of super types of this type
       writer

       @param typeName the super type name

       @exception std::bad_alloc is raised if an out-of-memory condition occurs
     */
    void setSuperTypeName(sal_uInt16 index, rtl::OUString const & typeName) {
        if (!typereg_writer_setSuperTypeName(m_handle, index, typeName.pData)) {
            throw std::bad_alloc();
        }
    }

    /**
       Sets the data of a field of this type writer.

       @param index a valid index into the range of fields of this type writer

       @param documentation the documentation of the field

       @param fileName the file name of the field (deprecated, use an empty string)

       @param flags the flags of the field

       @param name the name of the field

       @param typeName the type name of the field

       @param value the value of the field

       @exception std::bad_alloc is raised if an out-of-memory condition occurs
     */
    void setFieldData(
        sal_uInt16 index, rtl::OUString const & documentation,
        rtl::OUString const & fileName, RTFieldAccess flags, rtl::OUString const & name,
        rtl::OUString const & typeName, RTConstValue const & value)
    {
        if (!typereg_writer_setFieldData(
                m_handle, index, documentation.pData, fileName.pData, flags,
                name.pData, typeName.pData, value.m_type, value.m_value))
        {
            throw std::bad_alloc();
        }
    }

    /**
       Sets the data of a method of this type writer.

       @param index a valid index into the range of methods of this type writer

       @param documentation the documentation of the method

       @param flags the flags of the method

       @param name the name of the method

       @param returnTypeName the return type name of the method

       @param parameterCount the number of parameters of the method

       @param exceptionCount the number of exceptions of the method

       @exception std::bad_alloc is raised if an out-of-memory condition occurs
     */
    void setMethodData(
        sal_uInt16 index, rtl::OUString const & documentation,
        RTMethodMode flags, rtl::OUString const & name,
        rtl::OUString const & returnTypeName, sal_uInt16 parameterCount,
        sal_uInt16 exceptionCount)
    {
        if (!typereg_writer_setMethodData(
                m_handle, index, documentation.pData, flags, name.pData,
                returnTypeName.pData, parameterCount, exceptionCount))
        {
            throw std::bad_alloc();
        }
    }

    /**
       Sets the data of a parameter of a method of this type writer.

       @param methodIndex a valid index into the range of methods of this type
       writer

       @param parameterIndex a valid index into the range of parameters of the
       given method

       @param flags the flags of the parameter

       @param name the name of the parameter

       @param typeName the type name of the parameter

       @exception std::bad_alloc is raised if an out-of-memory condition occurs
     */
    void setMethodParameterData(
        sal_uInt16 methodIndex, sal_uInt16 parameterIndex,
        RTParamMode flags, rtl::OUString const & name,
        rtl::OUString const & typeName)
    {
        if (!typereg_writer_setMethodParameterData(
                m_handle, methodIndex, parameterIndex, flags, name.pData,
                typeName.pData))
        {
            throw std::bad_alloc();
        }
    }

    /**
       Sets an exception type name of a method of this type writer.

       @param methodIndex a valid index into the range of methods of this type
       writer

       @param exceptionIndex a valid index into the range of exceptions of the
       given method

       @param typeName the exception type name

       @exception std::bad_alloc is raised if an out-of-memory condition occurs
     */
    void setMethodExceptionTypeName(
        sal_uInt16 methodIndex, sal_uInt16 exceptionIndex,
        rtl::OUString const & typeName)
    {
        if (!typereg_writer_setMethodExceptionTypeName(
                m_handle, methodIndex, exceptionIndex, typeName.pData))
        {
            throw std::bad_alloc();
        }
    }

    /**
       Sets the data of a reference of this type writer.

       @param index a valid index into the range of references of this type
       writer

       @param documentation the documentation of the reference

       @param sort the sort of the reference

       @param flags the flags of the reference

       @param typeName the type name of the reference

       @exception std::bad_alloc is raised if an out-of-memory condition occurs
     */
    void setReferenceData(
        sal_uInt16 index, rtl::OUString const & documentation,
        RTReferenceType sort, RTFieldAccess flags,
        rtl::OUString const & typeName)
    {
        if (!typereg_writer_setReferenceData(
                m_handle, index, documentation.pData, sort, flags,
                typeName.pData))
        {
            throw std::bad_alloc();
        }
    }

    /**
       Returns the blob of this type writer.

       @param size an out-parameter obtaining the size of the blob

       @return a (byte-aligned) pointer to the blob; the returned pointer and
       the returned <code>size</code> remain valid until the next function is
       called on this type writer

       @exception std::bad_alloc is raised if an out-of-memory condition occurs
       (in which case <code>size</code> is not modified
     */
    void const * getBlob(sal_uInt32 * size) {
        void const * p = typereg_writer_getBlob(m_handle, size);
        if (p == nullptr) {
            throw std::bad_alloc();
        }
        return p;
    }

private:
    Writer(Writer &) = delete;
    void operator =(Writer) = delete;

    void * m_handle;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
