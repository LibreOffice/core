/*************************************************************************
 *
 *  $RCSfile: writer.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 02:43:44 $
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

#ifndef INCLUDED_registry_writer_hxx
#define INCLUDED_registry_writer_hxx

#include "registry/writer.h"
#include "registry/refltype.hxx"
#include "registry/types.h"
#include "registry/version.h"

#include "rtl/ustring.hxx"
#include "sal/types.h"

#include <new>

namespace typereg {

/// @HTML

/**
   A type writer working on a binary blob that represents a UNOIDL type.

   <p>Instances of this class are not multi-thread&ndash;safe.</p>

   @since #i21150#
 */
class Writer {
public:
    /**
       Creates a type writer.

       @param version the version of the created type writer; must not be
       negative

       @param documentation the documentation

       @param fileName the file name

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
        if (m_handle == 0) {
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

       @since #i21150#
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

       @param fileName the file name of the field

       @param flags the flags of the field

       @param name the name of the field

       @param typeName the type name of the field

       @param valueType the type of the value of the field

       @param valueValue the value of the value of the field

       @exception std::bad_alloc is raised if an out-of-memory condition occurs

       @since #i21150#
     */
    void setFieldData(
        sal_uInt16 index, rtl::OUString const & documentation,
        rtl::OUString const & fileName, RTFieldAccess flags,
        rtl::OUString const & name, rtl::OUString const & typeName,
        RTConstValue const & value)
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

       @since #i21150#
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

       @since #i21150#
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

       @since #i21150#
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

       @param handle a handle on a type writer

       @param index a valid index into the range of references of this type
       writer

       @param documentation the documentation of the reference

       @param sort the sort of the reference

       @param flags the flags of the reference

       @param typeName the type name of the reference

       @exception std::bad_alloc is raised if an out-of-memory condition occurs

       @since #i21150#
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
       (in which case <code>siez</code> is not modified
     */
    void const * getBlob(sal_uInt32 * size) {
        void const * p = typereg_writer_getBlob(m_handle, size);
        if (p == 0) {
            throw std::bad_alloc();
        }
        return p;
    }

private:
    Writer(Writer &); // not implemented
    void operator =(Writer); // not implemented

    void * m_handle;
};

}

#endif
