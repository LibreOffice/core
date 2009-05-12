/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: registry.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _CODEMAKER_REGISTRY_HXX_
#define _CODEMAKER_REGISTRY_HXX_

#include <rtl/alloc.h>
#include <osl/interlck.h>
#include    <registry/registry.hxx>
#include "registry/reader.hxx"
#include "registry/version.h"
#include    <codemaker/options.hxx>

struct TypeReader_Impl
{
    TypeReader_Impl(const sal_uInt8* buffer,
                       sal_uInt32 bufferLen,
                       sal_Bool copyData)
        : m_refCount(0)
        , m_copyData(copyData)
        , m_blopSize(bufferLen)
        , m_pBlop(buffer)
        {
            if (copyData)
            {
                m_pBlop = (sal_uInt8*)rtl_allocateMemory(bufferLen);
                rtl_copyMemory((void*)m_pBlop, buffer, bufferLen);
            } else
            {
                m_blopSize = bufferLen;
                m_pBlop = buffer;
            }

            m_pReader = new typereg::Reader(
                m_pBlop, m_blopSize, false, TYPEREG_VERSION_1);
        }

    ~TypeReader_Impl()
        {
            if (m_copyData && m_pReader)
            {
                delete m_pReader;
            }
        }

    sal_Int32           m_refCount;
    sal_Bool            m_copyData;
    sal_Int32           m_blopSize;
    const sal_uInt8*    m_pBlop;
    typereg::Reader*    m_pReader;
};

class TypeReader
{
/*
    inline TypeReader(const RegistryTypeReader_Api* pApi,
                              const sal_uInt8* buffer,
                              sal_uInt32 bufferLen,
                              sal_Bool copyData);
*/
public:
    inline TypeReader()
        : m_pImpl(NULL)
    {}

    inline TypeReader(        const sal_uInt8* buffer,
                              sal_uInt32 bufferLen,
                              sal_Bool copyData)
    {
        m_pImpl = new TypeReader_Impl(buffer, bufferLen, copyData);
        acquire();
    }

    inline TypeReader(const TypeReader& toCopy)
        : m_pImpl(toCopy.m_pImpl)
    {
        acquire();
    }

    inline ~TypeReader()
    {
        release();
    }

    inline void acquire()
    {
        if (m_pImpl)
            osl_incrementInterlockedCount(&m_pImpl->m_refCount);
    }

    inline void release()
    {
        if (m_pImpl && 0 == osl_decrementInterlockedCount(&m_pImpl->m_refCount))
        {
            delete m_pImpl;
        }
    }

    inline TypeReader& operator = ( const TypeReader& value )
    {
        release();
        m_pImpl = value.m_pImpl;
        acquire();
        return *this;
    }

    inline sal_Bool         isValid() const
        {
            if (m_pImpl)
                return m_pImpl->m_pReader->isValid();
            else
                return sal_False;
        }

    inline RTTypeClass              getTypeClass() const
        { return m_pImpl->m_pReader->getTypeClass(); }
    inline const ::rtl::OString     getTypeName() const
        { return inGlobalSet( m_pImpl->m_pReader->getTypeName() ); }
    inline sal_uInt16 getSuperTypeCount() const
        { return m_pImpl->m_pReader->getSuperTypeCount(); }
    inline const ::rtl::OString     getSuperTypeName(sal_uInt16 index) const
        { return inGlobalSet( m_pImpl->m_pReader->getSuperTypeName(index) ); }
    inline const ::rtl::OString     getDoku() const
        { return inGlobalSet( m_pImpl->m_pReader->getDocumentation() ); }
    inline const ::rtl::OString     getFileName() const
        { return inGlobalSet( m_pImpl->m_pReader->getFileName() ); }
    inline sal_uInt32               getFieldCount() const
        { return m_pImpl->m_pReader->getFieldCount(); }
    inline const ::rtl::OString     getFieldName( sal_uInt16 index ) const
        { return inGlobalSet( m_pImpl->m_pReader->getFieldName(index) ); }
    inline const ::rtl::OString     getFieldType( sal_uInt16 index ) const
        { return inGlobalSet( m_pImpl->m_pReader->getFieldTypeName(index) ); }
    inline RTFieldAccess            getFieldAccess( sal_uInt16 index ) const
        { return m_pImpl->m_pReader->getFieldFlags(index); }
    inline RTConstValue             getFieldConstValue( sal_uInt16 index ) const
        { return m_pImpl->m_pReader->getFieldValue(index); }
    inline const ::rtl::OString     getFieldDoku( sal_uInt16 index ) const
        { return inGlobalSet( m_pImpl->m_pReader->getFieldDocumentation(index) ); }
    inline const ::rtl::OString     getFieldFileName( sal_uInt16 index ) const
        { return inGlobalSet( m_pImpl->m_pReader->getFieldFileName(index) ); }
    inline sal_uInt32               getMethodCount() const
        { return m_pImpl->m_pReader->getMethodCount(); }
    inline const ::rtl::OString     getMethodName( sal_uInt16 index ) const
        { return inGlobalSet( m_pImpl->m_pReader->getMethodName(index) ); }
    inline sal_uInt32               getMethodParamCount( sal_uInt16 index ) const
        { return m_pImpl->m_pReader->getMethodParameterCount(index); }
    inline const ::rtl::OString     getMethodParamType( sal_uInt16 index, sal_uInt16 paramIndex ) const
        { return inGlobalSet( m_pImpl->m_pReader->getMethodParameterTypeName(index,paramIndex) ); }
    inline const ::rtl::OString     getMethodParamName( sal_uInt16 index, sal_uInt16 paramIndex ) const
        { return inGlobalSet( m_pImpl->m_pReader->getMethodParameterName(index,paramIndex) ); }
    inline RTParamMode              getMethodParamMode( sal_uInt16 index, sal_uInt16 paramIndex ) const
        { return m_pImpl->m_pReader->getMethodParameterFlags(index,paramIndex); }
    inline sal_uInt32               getMethodExcCount( sal_uInt16 index ) const
        { return m_pImpl->m_pReader->getMethodExceptionCount(index); }
    inline const ::rtl::OString     getMethodExcType( sal_uInt16 index, sal_uInt16 excIndex ) const
        { return inGlobalSet( m_pImpl->m_pReader->getMethodExceptionTypeName(index,excIndex) ); }
    inline const ::rtl::OString     getMethodReturnType( sal_uInt16 index ) const
        { return inGlobalSet( m_pImpl->m_pReader->getMethodReturnTypeName(index) ); }
    inline RTMethodMode             getMethodMode( sal_uInt16 index ) const
        { return m_pImpl->m_pReader->getMethodFlags(index); }
    inline const ::rtl::OString     getMethodDoku( sal_uInt16 index ) const
        { return inGlobalSet( m_pImpl->m_pReader->getMethodDocumentation(index) ); }

    inline sal_uInt32               getReferenceCount() const
        { return m_pImpl->m_pReader->getReferenceCount(); }
    inline const ::rtl::OString     getReferenceName( sal_uInt16 index ) const
        { return inGlobalSet( m_pImpl->m_pReader->getReferenceTypeName(index) ); }
    inline RTReferenceType          getReferenceType( sal_uInt16 index ) const
        { return m_pImpl->m_pReader->getReferenceSort(index); }
    inline const ::rtl::OString     getReferenceDoku( sal_uInt16 index ) const
        { return inGlobalSet( m_pImpl->m_pReader->getReferenceDocumentation(index) ); }

    inline sal_uInt32 getBlopSize() const
        { return m_pImpl->m_blopSize; }

    inline const sal_uInt8* getBlop() const
        { return m_pImpl->m_pBlop; }

private:
    TypeReader_Impl* m_pImpl;
};


#endif // _CODEMAKER_REGISTRY_HXX_
