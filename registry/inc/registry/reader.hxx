/*************************************************************************
 *
 *  $RCSfile: reader.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 02:42:36 $
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

#ifndef INCLUDED_registry_reader_hxx
#define INCLUDED_registry_reader_hxx

#include "registry/reader.h"
#include "registry/refltype.hxx"
#include "registry/types.h"
#include "registry/version.h"

#include "rtl/ustring.hxx"
#include "sal/types.h"

#include <algorithm>
#include <new>

namespace typereg {

/// @HTML

/**
   A type reader working on a binary blob that represents a UNOIDL type.

   <p>Instances of this class are not multi-thread&ndash;safe.</p>

   @since #i21150#
 */
class Reader {
public:
    /**
       Creates an invalid type reader.
     */
    Reader(): m_handle(0) {}

    /**
       Creates a type reader.

       <p>If the given binary blob is malformed, or of a version larger than
       <code>maxVersion</code>, the created type reader is flagged as
       invalid.</p>

       @param buffer the binary blob representing the type; must point to at
       least <code>length</code> bytes, and need only be byte-aligned

       @param length the size in bytes of the binary blob representing the type

       @param copy if true, the type reader creates an internal copy of the
       given buffer, and the given buffer is not accessed after this constructor
       returns; if false, the type reader works directly on the given buffer,
       which must remain available unmodified until the underlying type reader
       is destroyed (note that the lifetime of the underlying type reader can be
       different from the lifetime of this <code>Reader</code> instance)

       @param maxVersion the maximum binary blob version the client is prepared
       to handle; must not be negative

       @exception std::bad_alloc is raised if an out-of-memory condition occurs
     */
    Reader(
        void const * buffer, sal_uInt32 length, bool copy,
        typereg_Version maxVersion)
    {
        if (!typereg_reader_create(buffer, length, copy, maxVersion, &m_handle))
        {
            throw std::bad_alloc();
        }
    }

    /**
       Shares a type reader between two <code>Reader</code> instances.

       @param other another <code>Reader</code> instance
     */
    Reader(Reader const & other): m_handle(other.m_handle) {
        typereg_reader_acquire(m_handle);
    }

    /**
       Destroys this <code>Reader</code> instance.

       <p>The underlying type reader is only destroyed if this instance was its
       last user.</p>
     */
    ~Reader() {
        typereg_reader_release(m_handle);
    }

    /**
       Replaces the underlying type reader.

       @param other any <code>Reader</code> instance

       @return this <code>Reader</code> instance
     */
    Reader & operator =(Reader const & other) {
        Reader temp(other);
        std::swap(this->m_handle, temp.m_handle);
        return *this;
    }

    /**
       Returns whether this type reader is valid.

       @return true iff this type reader is valid
     */
    bool isValid() const {
        return m_handle != 0;
    }

    /**
       Returns the binary blob version of this type reader.

       @return the version of the binary blob from which this type reader was
       constructed; if this type reader is invalid,
       <code>TYPEREG_VERSION_0</code> is returned
     */
    typereg_Version getVersion() const {
        return typereg_reader_getVersion(m_handle);
    }

    /**
       Returns the documentation of this type reader.

       @return the documentation of this type reader; if this type reader is
       invalid, an empty string is returned

       @exception std::bad_alloc is raised if an out-of-memory condition occurs
     */
    rtl::OUString getDocumentation() const {
        rtl_uString * s = 0;
        typereg_reader_getDocumentation(m_handle, &s);
        if (s == 0) {
            throw std::bad_alloc();
        }
        return rtl::OUString(s, SAL_NO_ACQUIRE);
    }

    /**
       Returns the file name of this type reader.

       @return the file name of this type reader; if this type reader is
       invalid, an empty string is returned

       @exception std::bad_alloc is raised if an out-of-memory condition occurs
     */
    rtl::OUString getFileName() const {
        rtl_uString * s = 0;
        typereg_reader_getFileName(m_handle, &s);
        if (s == 0) {
            throw std::bad_alloc();
        }
        return s;
    }

    /**
       Returns the type class of this type reader.

       <p>This function will always return the type class without the internal
       <code>RT_TYPE_PUBLISHED</code> flag set.  Use <code>isPublished</code> to
       determine whether this type reader is published.</p>

       @return the type class of this type reader; if this type reader is
       invalid, <code>RT_TYPE_INVALID</code> is returned
     */
    RTTypeClass getTypeClass() const {
        return typereg_reader_getTypeClass(m_handle);
    }

    /**
       Returns whether this type reader is published.

       @return whether this type reader is published; if this type reader is
       invalid, <code>false</code> is returned
     */
    bool isPublished() const {
        return typereg_reader_isPublished(m_handle);
    }

    /**
       Returns the type name of this type reader.

       @return the type name of this type reader; if this type reader is
       invalid, an empty string is returned

       @exception std::bad_alloc is raised if an out-of-memory condition occurs
     */
    rtl::OUString getTypeName() const {
        rtl_uString * s = 0;
        typereg_reader_getTypeName(m_handle, &s);
        if (s == 0) {
            throw std::bad_alloc();
        }
        return s;
    }

    /**
       Returns the number of super types of this type reader.

       @return the number of super types of this type reader; if this type
       reader is invalid, zero is returned
     */
    sal_uInt16 getSuperTypeCount() const {
        return typereg_reader_getSuperTypeCount(m_handle);
    }

    /**
       Returns the type name of a super type of this type reader.

       @param index a valid index into the range of super types of this type
       reader

       @return the type name of the given super type

       @exception std::bad_alloc is raised if an out-of-memory condition occurs
     */
    rtl::OUString getSuperTypeName(sal_uInt16 index) const {
        rtl_uString * s = 0;
        typereg_reader_getSuperTypeName(m_handle, &s, index);
        if (s == 0) {
            throw std::bad_alloc();
        }
        return s;
    }

    /**
       Returns the number of fields of this type reader.

       @return the number of fields of this type reader; if this type reader is
       invalid, zero is returned
     */
    sal_uInt16 getFieldCount() const {
        return typereg_reader_getFieldCount(m_handle);
    }

    /**
       Returns the documentation of a field of this type reader.

       @param index a valid index into the range of fields of this type reader

       @return the documentation of the given field

       @exception std::bad_alloc is raised if an out-of-memory condition occurs
     */
    rtl::OUString getFieldDocumentation(sal_uInt16 index) const {
        rtl_uString * s = 0;
        typereg_reader_getFieldDocumentation(m_handle, &s, index);
        if (s == 0) {
            throw std::bad_alloc();
        }
        return s;
    }

    /**
       Returns the file name of a field of this type reader.

       @param index a valid index into the range of fields of this type reader

       @return the file name of the given field

       @exception std::bad_alloc is raised if an out-of-memory condition occurs
     */
    rtl::OUString getFieldFileName(sal_uInt16 index) const {
        rtl_uString * s = 0;
        typereg_reader_getFieldFileName(m_handle, &s, index);
        if (s == 0) {
            throw std::bad_alloc();
        }
        return s;
    }

    /**
       Returns the flags of a field of this type reader.

       @param index a valid index into the range of fields of this type reader

       @return the flags of the given field
     */
    RTFieldAccess getFieldFlags(sal_uInt16 index) const {
        return typereg_reader_getFieldFlags(m_handle, index);
    }

    /**
       Returns the name of a field of this type reader.

       @param index a valid index into the range of fields of this type reader

       @return the name of the given field

       @exception std::bad_alloc is raised if an out-of-memory condition occurs
     */
    rtl::OUString getFieldName(sal_uInt16 index) const {
        rtl_uString * s = 0;
        typereg_reader_getFieldName(m_handle, &s, index);
        if (s == 0) {
            throw std::bad_alloc();
        }
        return s;
    }

    /**
       Returns the type name of a field of this type reader.

       @param index a valid index into the range of fields of this type reader

       @return the type name of the given field

       @exception std::bad_alloc is raised if an out-of-memory condition occurs
     */
    rtl::OUString getFieldTypeName(sal_uInt16 index) const {
        rtl_uString * s = 0;
        typereg_reader_getFieldTypeName(m_handle, &s, index);
        if (s == 0) {
            throw std::bad_alloc();
        }
        return s;
    }

    /**
       Returns the value of a field of this type reader.

       @param index a valid index into the range of fields of this type reader

       @return the value of the given field

       @exception std::bad_alloc is raised if an out-of-memory condition occurs
     */
    RTConstValue getFieldValue(sal_uInt16 index) const {
        RTConstValue v;
        if (!typereg_reader_getFieldValue(
                m_handle, index, &v.m_type, &v.m_value))
        {
            throw std::bad_alloc();
        }
        return v;
    }

    /**
       Returns the number of methods of this type reader.

       @return the number of methods of this type reader; if this type reader is
       invalid, zero is returned
     */
    sal_uInt16 getMethodCount() const {
        return typereg_reader_getMethodCount(m_handle);
    }

    /**
       Returns the documentation of a method of this type reader.

       @param index a valid index into the range of methods of this type reader

       @return the documentation of the given method

       @exception std::bad_alloc is raised if an out-of-memory condition occurs
     */
    rtl::OUString getMethodDocumentation(sal_uInt16 index) const {
        rtl_uString * s = 0;
        typereg_reader_getMethodDocumentation(m_handle, &s, index);
        if (s == 0) {
            throw std::bad_alloc();
        }
        return s;
    }

    /**
       Returns the flags of a method of this type reader.

       @param index a valid index into the range of methods of this type reader

       @return the flags of the given method
     */
    RTMethodMode getMethodFlags(sal_uInt16 index) const {
        return typereg_reader_getMethodFlags(m_handle, index);
    }

    /**
       Returns the name of a method of this type reader.

       @param index a valid index into the range of methods of this type reader

       @return the name of the given method

       @exception std::bad_alloc is raised if an out-of-memory condition occurs
     */
    rtl::OUString getMethodName(sal_uInt16 index) const {
        rtl_uString * s = 0;
        typereg_reader_getMethodName(m_handle, &s, index);
        if (s == 0) {
            throw std::bad_alloc();
        }
        return s;
    }

    /**
       Returns the return type name of a method of this type reader.

       @param index a valid index into the range of methods of this type reader

       @return the return type name of the given method

       @exception std::bad_alloc is raised if an out-of-memory condition occurs
     */
    rtl::OUString getMethodReturnTypeName(sal_uInt16 index) const {
        rtl_uString * s = 0;
        typereg_reader_getMethodReturnTypeName(m_handle, &s, index);
        if (s == 0) {
            throw std::bad_alloc();
        }
        return s;
    }

    /**
       Returns the number of parameters of a method of this type reader.

       @param index a valid index into the range of methods of this type reader

       @return the number of parameters of the given method
     */
    sal_uInt16 getMethodParameterCount(sal_uInt16 index) const {
        return typereg_reader_getMethodParameterCount(m_handle, index);
    }

    /**
       Returns the flags of a parameter of a method of this type reader.

       @param methodIndex a valid index into the range of methods of this type
       reader

       @param parameterIndex a valid index into the range of parameters of the
       given method

       @return the flags of the given method parameter
     */
    RTParamMode getMethodParameterFlags(
        sal_uInt16 methodIndex, sal_uInt16 parameterIndex) const
    {
        return typereg_reader_getMethodParameterFlags(
            m_handle, methodIndex, parameterIndex);
    }

    /**
       Returns the name of a parameter of a method of this type reader.

       @param methodIndex a valid index into the range of methods of this type
       reader

       @param parameterIndex a valid index into the range of parameters of the
       given method

       @return the name of the given method parameter

       @exception std::bad_alloc is raised if an out-of-memory condition occurs
     */
    rtl::OUString getMethodParameterName(
        sal_uInt16 methodIndex, sal_uInt16 parameterIndex) const
    {
        rtl_uString * s = 0;
        typereg_reader_getMethodParameterName(
            m_handle, &s, methodIndex, parameterIndex);
        if (s == 0) {
            throw std::bad_alloc();
        }
        return s;
    }

    /**
       Returns the type name of a parameter of a method of this type reader.

       @param methodIndex a valid index into the range of methods of this type
       reader

       @param parameterIndex a valid index into the range of parameters of the
       given method

       @return the type name of the given method parameter

       @exception std::bad_alloc is raised if an out-of-memory condition occurs
     */
    rtl::OUString getMethodParameterTypeName(
        sal_uInt16 methodIndex, sal_uInt16 parameterIndex) const
    {
        rtl_uString * s = 0;
        typereg_reader_getMethodParameterTypeName(
            m_handle, &s, methodIndex, parameterIndex);
        if (s == 0) {
            throw std::bad_alloc();
        }
        return s;
    }

    /**
       Returns the number of exceptions of a method of this type reader.

       @param index a valid index into the range of methods of this type reader

       @return the number of exceptions of the given method
     */
    sal_uInt16 getMethodExceptionCount(sal_uInt16 index) const {
        return typereg_reader_getMethodExceptionCount(m_handle, index);
    }

    /**
       Returns the type name of an exception of a method of this type reader.

       @param methodIndex a valid index into the range of methods of this type
       reader

       @param exceptionIndex a valid index into the range of exceptions of the
       given method

       @return the type name of the given method exception

       @exception std::bad_alloc is raised if an out-of-memory condition occurs
     */
    rtl::OUString getMethodExceptionTypeName(
        sal_uInt16 methodIndex, sal_uInt16 exceptionIndex) const
    {
        rtl_uString * s = 0;
        typereg_reader_getMethodExceptionTypeName(
            m_handle, &s, methodIndex, exceptionIndex);
        if (s == 0) {
            throw std::bad_alloc();
        }
        return s;
    }

    /**
       Returns the number of references of this type reader.

       @return the number of references of this type reader; if this type reader
       is invalid, zero is returned
     */
    sal_uInt16 getReferenceCount() const {
        return typereg_reader_getReferenceCount(m_handle);
    }

    /**
       Returns the documentation of a reference of this type reader.

       @param index a valid index into the range of references of this type
       reader

       @return the documentation of the given reference

       @exception std::bad_alloc is raised if an out-of-memory condition occurs
     */
    rtl::OUString getReferenceDocumentation(sal_uInt16 index) const {
        rtl_uString * s = 0;
        typereg_reader_getReferenceDocumentation(m_handle, &s, index);
        if (s == 0) {
            throw std::bad_alloc();
        }
        return s;
    }

    /**
       Returns the flags of a reference of this type reader.

       @param index a valid index into the range of references of this type
       reader

       @return the flags of the given reference
     */
    RTFieldAccess getReferenceFlags(sal_uInt16 index) const {
        return typereg_reader_getReferenceFlags(m_handle, index);
    }

    /**
       Returns the sort of a reference of this type reader.

       @param index a valid index into the range of references of this type
       reader

       @return the sort of the given reference
     */
    RTReferenceType getReferenceSort(sal_uInt16 index) const {
        return typereg_reader_getReferenceSort(m_handle, index);
    }

    /**
       Returns the type name of a reference of this type reader.

       @param index a valid index into the range of references of this type
       reader

       @return the type name of the given reference

       @exception std::bad_alloc is raised if an out-of-memory condition occurs
     */
    rtl::OUString getReferenceTypeName(sal_uInt16 index) const {
        rtl_uString * s = 0;
        typereg_reader_getReferenceTypeName(m_handle, &s, index);
        if (s == 0) {
            throw std::bad_alloc();
        }
        return s;
    }

private:
    void * m_handle;
};

}

#endif
