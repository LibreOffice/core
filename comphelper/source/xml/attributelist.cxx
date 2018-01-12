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

#include <comphelper/attributelist.hxx>

#include <vector>

using namespace osl;
using namespace com::sun::star;


namespace comphelper {

struct TagAttribute_Impl
{
    TagAttribute_Impl( const OUString &aName, const OUString &aType,
                         const OUString &aValue )
    {
        sName     = aName;
        sType     = aType;
        sValue    = aValue;
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
    std::vector<struct TagAttribute_Impl> vecAttribute;
};

sal_Int16 SAL_CALL AttributeList::getLength()
{
    return static_cast<sal_Int16>(m_pImpl->vecAttribute.size());
}

OUString SAL_CALL AttributeList::getNameByIndex(sal_Int16 i)
{
    return ( i < static_cast < sal_Int16 > (m_pImpl->vecAttribute.size()) ) ? m_pImpl->vecAttribute[i].sName : OUString();
}

OUString SAL_CALL AttributeList::getTypeByIndex(sal_Int16 i)
{
    if( i < static_cast < sal_Int16 > (m_pImpl->vecAttribute.size() ) ) {
        return m_pImpl->vecAttribute[i].sType;
    }
    return OUString();
}

OUString SAL_CALL  AttributeList::getValueByIndex(sal_Int16 i)
{
    return ( i < static_cast < sal_Int16 > (m_pImpl->vecAttribute.size() ) ) ? m_pImpl->vecAttribute[i].sValue : OUString();
}

OUString SAL_CALL AttributeList::getTypeByName( const OUString& sName )
{
    std::vector<struct TagAttribute_Impl>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ++ii ) {
        if( (*ii).sName == sName ) {
            return (*ii).sType;
        }
    }
    return OUString();
}

OUString SAL_CALL AttributeList::getValueByName(const OUString& sName)
{
    std::vector<struct TagAttribute_Impl>::iterator ii = m_pImpl->vecAttribute.begin();

    for( ; ii != m_pImpl->vecAttribute.end() ; ++ii ) {
        if( (*ii).sName == sName ) {
            return (*ii).sValue;
        }
    }
    return OUString();
}

AttributeList::AttributeList()
    : m_pImpl(new AttributeList_Impl)
{
}

AttributeList::AttributeList(const AttributeList &r)
    : cppu::WeakImplHelper<XAttributeList, XCloneable>()
    , m_pImpl(new AttributeList_Impl)
{
    *m_pImpl = *(r.m_pImpl);
}

AttributeList::~AttributeList()
{
}

void AttributeList::AddAttribute(const OUString &sName,
        const OUString &sType, const OUString &sValue)
{
    m_pImpl->vecAttribute.emplace_back(sName, sType, sValue );
}

void AttributeList::Clear()
{
    m_pImpl->vecAttribute.clear();
}

css::uno::Reference< css::util::XCloneable > AttributeList::createClone()

{
    AttributeList *p = new AttributeList( *this );
    return css::uno::Reference< css::util::XCloneable > ( static_cast<css::util::XCloneable *>(p) );
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
