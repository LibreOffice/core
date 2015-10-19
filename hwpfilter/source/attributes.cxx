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


#include <assert.h>
#include <vector>
#include "attributes.hxx"

struct TagAttribute
{
    TagAttribute( const OUString &rName, const OUString &rType , const OUString &rValue )
    {
        sName     = rName;
        sType     = rType;
        sValue    = rValue;
    }

    OUString sName;
    OUString sType;
    OUString sValue;
};

struct AttributeListImpl_impl
{
    AttributeListImpl_impl()
    {
// performance improvement during adding
        vecAttribute.reserve(20);
    }
    std::vector<struct TagAttribute> vecAttribute;
};

sal_Int16 SAL_CALL AttributeListImpl::getLength() throw (RuntimeException, std::exception)
{
    return (sal_Int16)m_pImpl->vecAttribute.size();
}


AttributeListImpl::AttributeListImpl( const AttributeListImpl &r ) :
cppu::WeakImplHelper<css::xml::sax::XAttributeList>( r )
{
    m_pImpl = new AttributeListImpl_impl;
    *m_pImpl = *(r.m_pImpl);
}


OUString AttributeListImpl::getNameByIndex(sal_Int16 i) throw (RuntimeException, std::exception)
{
    sal_uInt32 i2 = sal::static_int_cast<sal_Int16>(i);
    if( i >= 0 &&  i2 < m_pImpl->vecAttribute.size() )
    {
        return m_pImpl->vecAttribute[i].sName;
    }
    return OUString();
}


OUString AttributeListImpl::getTypeByIndex(sal_Int16 i) throw (RuntimeException, std::exception)
{
    sal_uInt32 i2 = sal::static_int_cast<sal_Int16>(i);
    if( i >= 0 &&  i2 < m_pImpl->vecAttribute.size() )
    {
        return m_pImpl->vecAttribute[i].sType;
    }
    return OUString();
}


OUString AttributeListImpl::getValueByIndex(sal_Int16 i) throw (RuntimeException, std::exception)
{
    sal_uInt32 i2 = sal::static_int_cast<sal_Int16>(i);
    if( i >= 0 &&  i2 < m_pImpl->vecAttribute.size() )
    {
        return m_pImpl->vecAttribute[i].sValue;
    }
    return OUString();

}


OUString AttributeListImpl::getTypeByName( const OUString& sName ) throw (RuntimeException, std::exception)
{
    std::vector<struct TagAttribute>::iterator ii = m_pImpl->vecAttribute.begin();

    for (; ii != m_pImpl->vecAttribute.end(); ++ii)
    {
        if( (*ii).sName == sName )
        {
            return (*ii).sType;
        }
    }
    return OUString();
}


OUString AttributeListImpl::getValueByName(const OUString& sName) throw (RuntimeException, std::exception)
{
    std::vector<struct TagAttribute>::iterator ii = m_pImpl->vecAttribute.begin();

    for (; ii != m_pImpl->vecAttribute.end(); ++ii)
    {
        if( (*ii).sName == sName )
        {
            return (*ii).sValue;
        }
    }
    return OUString();
}


AttributeListImpl::AttributeListImpl()
{
    m_pImpl = new AttributeListImpl_impl;
}


AttributeListImpl::~AttributeListImpl()
{
    delete m_pImpl;
}


void AttributeListImpl::addAttribute(   const OUString &sName ,
const OUString &sType ,
const OUString &sValue )
{
    m_pImpl->vecAttribute.push_back( TagAttribute( sName , sType , sValue ) );
}


void AttributeListImpl::clear()
{
    std::vector<struct TagAttribute> dummy;
    m_pImpl->vecAttribute.swap( dummy );

    assert( ! getLength() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
