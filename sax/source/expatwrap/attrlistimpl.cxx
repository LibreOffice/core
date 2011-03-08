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

#include "attrlistimpl.hxx"

#include <vector>

#include <cppuhelper/weak.hxx>

using namespace ::std;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::xml::sax;


namespace sax_expatwrap {
struct TagAttribute
{
    TagAttribute()
        {}
    TagAttribute( const OUString &aName, const OUString &aType , const OUString &aValue )
    {
        this->sName     = aName;
        this->sType     = aType;
        this->sValue    = aValue;
    }

    OUString sName;
    OUString sType;
    OUString sValue;
};

struct AttributeList_impl
{
    AttributeList_impl()
    {
        // performance improvement during adding
        vecAttribute.reserve(20);
    }
    vector<struct TagAttribute> vecAttribute;
};



sal_Int16 AttributeList::getLength(void) throw (RuntimeException)
{
    return static_cast<sal_Int16>(m_pImpl->vecAttribute.size());
}


AttributeList::AttributeList( const AttributeList &r ) :
    cppu::WeakImplHelper2<XAttributeList, XCloneable>()
{
    m_pImpl = new AttributeList_impl;
    *m_pImpl = *(r.m_pImpl);
}

OUString AttributeList::getNameByIndex(sal_Int16 i) throw (RuntimeException)
{
    if( std::vector< TagAttribute >::size_type(i) < m_pImpl->vecAttribute.size() ) {
        return m_pImpl->vecAttribute[i].sName;
    }
    return OUString();
}


OUString AttributeList::getTypeByIndex(sal_Int16 i) throw (RuntimeException)
{
    if( std::vector< TagAttribute >::size_type(i) < m_pImpl->vecAttribute.size() ) {
        return m_pImpl->vecAttribute[i].sType;
    }
    return OUString();
}

OUString AttributeList::getValueByIndex(sal_Int16 i) throw (RuntimeException)
{
    if( std::vector< TagAttribute >::size_type(i) < m_pImpl->vecAttribute.size() ) {
        return m_pImpl->vecAttribute[i].sValue;
    }
    return OUString();

}

OUString AttributeList::getTypeByName( const OUString& sName ) throw (RuntimeException)
{
    vector<struct TagAttribute>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ii ++ ) {
        if( (*ii).sName == sName ) {
            return (*ii).sType;
        }
    }
    return OUString();
}

OUString AttributeList::getValueByName(const OUString& sName) throw (RuntimeException)
{
    vector<struct TagAttribute>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ii ++ ) {
        if( (*ii).sName == sName ) {
            return (*ii).sValue;
        }
    }
    return OUString();
}


Reference< XCloneable > AttributeList::createClone() throw (RuntimeException)
{
    AttributeList *p = new AttributeList( *this );
    return Reference< XCloneable > ( (XCloneable * ) p );
}



AttributeList::AttributeList()
{
    m_pImpl = new AttributeList_impl;
}



AttributeList::~AttributeList()
{
    delete m_pImpl;
}


void AttributeList::addAttribute(   const OUString &sName ,
                                        const OUString &sType ,
                                        const OUString &sValue )
{
    m_pImpl->vecAttribute.push_back( TagAttribute( sName , sType , sValue ) );
}

void AttributeList::clear()
{
    m_pImpl->vecAttribute.clear();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
