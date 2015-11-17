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

#include "attrlistimpl.hxx"

#include <vector>

#include <cppuhelper/weak.hxx>

using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::xml::sax;


namespace sax_expatwrap {
struct TagAttribute
{
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



sal_Int16 AttributeList::getLength() throw (RuntimeException, std::exception)
{
    return static_cast<sal_Int16>(m_pImpl->vecAttribute.size());
}


AttributeList::AttributeList( const AttributeList &r ) :
    cppu::WeakImplHelper<XAttributeList, XCloneable>(),
    m_pImpl( new AttributeList_impl )
{
    *m_pImpl = *(r.m_pImpl);
}

OUString AttributeList::getNameByIndex(sal_Int16 i) throw (RuntimeException, std::exception)
{
    if( std::vector< TagAttribute >::size_type(i) < m_pImpl->vecAttribute.size() ) {
        return m_pImpl->vecAttribute[i].sName;
    }
    return OUString();
}


OUString AttributeList::getTypeByIndex(sal_Int16 i) throw (RuntimeException, std::exception)
{
    if( std::vector< TagAttribute >::size_type(i) < m_pImpl->vecAttribute.size() ) {
        return m_pImpl->vecAttribute[i].sType;
    }
    return OUString();
}

OUString AttributeList::getValueByIndex(sal_Int16 i) throw (RuntimeException, std::exception)
{
    if( std::vector< TagAttribute >::size_type(i) < m_pImpl->vecAttribute.size() ) {
        return m_pImpl->vecAttribute[i].sValue;
    }
    return OUString();

}

OUString AttributeList::getTypeByName( const OUString& sName ) throw (RuntimeException, std::exception)
{
    vector<struct TagAttribute>::iterator ii = m_pImpl->vecAttribute.begin();

    for (; ii != m_pImpl->vecAttribute.end(); ++ii )
    {
        if( (*ii).sName == sName )
        {
            return (*ii).sType;
        }
    }
    return OUString();
}

OUString AttributeList::getValueByName(const OUString& sName) throw (RuntimeException, std::exception)
{
    vector<struct TagAttribute>::iterator ii = m_pImpl->vecAttribute.begin();

    for (; ii != m_pImpl->vecAttribute.end(); ++ii)
    {
        if( (*ii).sName == sName )
        {
            return (*ii).sValue;
        }
    }
    return OUString();
}


Reference< XCloneable > AttributeList::createClone() throw (RuntimeException, std::exception)
{
    AttributeList *p = new AttributeList( *this );
    return Reference< XCloneable > ( static_cast<XCloneable *>(p) );
}



AttributeList::AttributeList()
   : m_pImpl( new AttributeList_impl )
{
}



AttributeList::~AttributeList()
{
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
