/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"
#include <comphelper/attributelist.hxx>
#include <osl/diagnose.h>

#include <vector>

using namespace osl;
using namespace com::sun::star;

using ::rtl::OUString;

namespace comphelper {

struct TagAttribute_Impl
{
    TagAttribute_Impl(){}
    TagAttribute_Impl( const OUString &aName, const OUString &aType,
                         const OUString &aValue )
    {
        this->sName     = aName;
        this->sType     = aType;
        this->sValue    = aValue;
    }

    OUString sName;
    OUString sType;
    OUString sValue;
};

struct AttributeList_Impl
{
    AttributeList_Impl()
    {
        // performance improvement during adding
        vecAttribute.reserve(20);
    }
    ::std::vector<struct TagAttribute_Impl> vecAttribute;
};

sal_Int16 SAL_CALL AttributeList::getLength(void) throw( ::com::sun::star::uno::RuntimeException )
{
    return (sal_Int16)(m_pImpl->vecAttribute.size());
}

OUString SAL_CALL AttributeList::getNameByIndex(sal_Int16 i) throw( ::com::sun::star::uno::RuntimeException )
{
    return ( i < static_cast < sal_Int16 > (m_pImpl->vecAttribute.size()) ) ? m_pImpl->vecAttribute[i].sName : OUString();
}

OUString SAL_CALL AttributeList::getTypeByIndex(sal_Int16 i) throw( ::com::sun::star::uno::RuntimeException )
{
    if( i < static_cast < sal_Int16 > (m_pImpl->vecAttribute.size() ) ) {
        return m_pImpl->vecAttribute[i].sType;
    }
    return OUString();
}

OUString SAL_CALL  AttributeList::getValueByIndex(sal_Int16 i) throw( ::com::sun::star::uno::RuntimeException )
{
    return ( i < static_cast < sal_Int16 > (m_pImpl->vecAttribute.size() ) ) ? m_pImpl->vecAttribute[i].sValue : OUString();
}

OUString SAL_CALL AttributeList::getTypeByName( const OUString& sName ) throw( ::com::sun::star::uno::RuntimeException )
{
    ::std::vector<struct TagAttribute_Impl>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ii ++ ) {
        if( (*ii).sName == sName ) {
            return (*ii).sType;
        }
    }
    return OUString();
}

OUString SAL_CALL AttributeList::getValueByName(const OUString& sName) throw( ::com::sun::star::uno::RuntimeException )
{
    ::std::vector<struct TagAttribute_Impl>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ii ++ ) {
        if( (*ii).sName == sName ) {
            return (*ii).sValue;
        }
    }
    return OUString();
}


AttributeList::AttributeList()
{
    m_pImpl = new AttributeList_Impl;
}



AttributeList::~AttributeList()
{
    delete m_pImpl;
}

void AttributeList::AddAttribute(   const OUString &sName ,
                                        const OUString &sType ,
                                        const OUString &sValue )
{
    m_pImpl->vecAttribute.push_back( TagAttribute_Impl( sName , sType , sValue ) );
}

void AttributeList::Clear()
{
    m_pImpl->vecAttribute.clear();

    OSL_ENSURE( ! getLength(), "Length > 0 after AttributeList::Clear!");
}

void AttributeList::RemoveAttribute( const OUString sName )
{
    ::std::vector<struct TagAttribute_Impl>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ii ++ ) {
        if( (*ii).sName == sName ) {
            m_pImpl->vecAttribute.erase( ii );
            break;
        }
    }
}


void AttributeList::SetAttributeList( const uno::Reference< ::com::sun::star::xml::sax::XAttributeList >  &r )
{
    Clear();
    AppendAttributeList( r );
}

void AttributeList::AppendAttributeList( const uno::Reference< ::com::sun::star::xml::sax::XAttributeList >  &r )
{
    OSL_ENSURE( r.is(), "r isn't!" );

    sal_Int32 nMax = r->getLength();
    sal_Int32 nTotalSize = m_pImpl->vecAttribute.size() + nMax;
    m_pImpl->vecAttribute.reserve( nTotalSize );

    for( sal_Int16 i = 0 ; i < nMax ; i ++ ) {
        m_pImpl->vecAttribute.push_back( TagAttribute_Impl(
            r->getNameByIndex( i ) ,
            r->getTypeByIndex( i ) ,
            r->getValueByIndex( i )));
    }

    OSL_ENSURE( nTotalSize == getLength(), "nTotalSize != getLength()");
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
