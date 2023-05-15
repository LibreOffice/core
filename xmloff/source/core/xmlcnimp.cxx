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

#include "SvXMLAttrCollection.hxx"
#include <xmloff/xmlcnimp.hxx>
#include <rtl/ustring.hxx>

SvXMLAttrContainerData::SvXMLAttrContainerData() : m_pImpl( new SvXMLAttrCollection )
{
}

SvXMLAttrContainerData::SvXMLAttrContainerData(const SvXMLAttrContainerData &rCopy) :
        m_pImpl( new SvXMLAttrCollection( *(rCopy.m_pImpl) ) )
{
}

SvXMLAttrContainerData& SvXMLAttrContainerData::operator=(const SvXMLAttrContainerData &rCopy)
{
    m_pImpl.reset( new SvXMLAttrCollection( *rCopy.m_pImpl ) );
    return *this;
}

SvXMLAttrContainerData& SvXMLAttrContainerData::operator=(SvXMLAttrContainerData&& rCopy) noexcept
{
    m_pImpl = std::move( rCopy.m_pImpl );
    return *this;
}

// Need destructor defined (despite it being empty) to avoid "checked_delete"
// compiler errors.
SvXMLAttrContainerData::~SvXMLAttrContainerData()
{
}

bool SvXMLAttrContainerData::operator ==( const SvXMLAttrContainerData& rCmp ) const
{
    return ( *(rCmp.m_pImpl) == *m_pImpl );
}

bool SvXMLAttrContainerData::AddAttr( const OUString& rLName,
                                          const OUString& rValue )
{
    assert( !rLName.isEmpty() && "empty attribute name is invalid");
    assert( rLName.indexOf(':') == -1 && "colon in name?");
    return m_pImpl->AddAttr(rLName, rValue);
}

bool SvXMLAttrContainerData::AddAttr( const OUString& rPrefix,
                                          const OUString& rNamespace,
                                          const OUString& rLName,
                                          const OUString& rValue )
{
    assert( !rLName.isEmpty() && "empty attribute name is invalid");
    assert( rPrefix.indexOf(':') == -1 && "colon in prefix?");
    assert( rLName.indexOf(':') == -1 && "colon in name?");
    return m_pImpl->AddAttr(rPrefix, rNamespace, rLName, rValue);
}

bool SvXMLAttrContainerData::AddAttr( const OUString& rPrefix,
                                          const OUString& rLName,
                                          const OUString& rValue )
{
    assert( !rLName.isEmpty() && "empty attribute name is invalid");
    assert( rPrefix.indexOf(':') == -1 && "colon in prefix?");
    assert( rLName.indexOf(':') == -1 && "colon in name?");
    return m_pImpl->AddAttr(rPrefix, rLName, rValue);
}

bool SvXMLAttrContainerData::SetAt( size_t i,
                                        const OUString& rLName,
                                        const OUString& rValue )
{
    assert( !rLName.isEmpty() && "empty attribute name is invalid");
    assert( rLName.indexOf(':') == -1 && "colon in name?");
    return m_pImpl->SetAt(i, rLName, rValue);
}

bool SvXMLAttrContainerData::SetAt( size_t i,
                                        const OUString& rPrefix,
                                        const OUString& rNamespace,
                                        const OUString& rLName,
                                        const OUString& rValue )
{
    assert( !rLName.isEmpty() && "empty attribute name is invalid");
    assert( rPrefix.indexOf(':') == -1 && "colon in prefix?");
    assert( rLName.indexOf(':') == -1 && "colon in name?");
    return m_pImpl->SetAt(i, rPrefix, rNamespace, rLName, rValue);
}

bool SvXMLAttrContainerData::SetAt( size_t i,
                                        const OUString& rPrefix,
                                        const OUString& rLName,
                                        const OUString& rValue )
{
    assert( !rLName.isEmpty() && "empty attribute name is invalid");
    assert( rPrefix.indexOf(':') == -1 && "colon in prefix?");
    assert( rLName.indexOf(':') == -1 && "colon in name?");
    return m_pImpl->SetAt(i, rPrefix, rLName, rValue);
}

void SvXMLAttrContainerData::Remove( size_t i )
{
    m_pImpl->Remove(i);
}

size_t SvXMLAttrContainerData::GetAttrCount() const
{
    return m_pImpl->GetAttrCount();
}

const OUString& SvXMLAttrContainerData::GetAttrLName(size_t i) const
{
    return m_pImpl->GetAttrLName(i);
}

const OUString& SvXMLAttrContainerData::GetAttrValue(size_t i) const
{
    return m_pImpl->GetAttrValue(i);
}

OUString SvXMLAttrContainerData::GetAttrNamespace( size_t i ) const
{
    return m_pImpl->GetAttrNamespace(i);
}

OUString SvXMLAttrContainerData::GetAttrPrefix( size_t i ) const
{
    return m_pImpl->GetAttrPrefix(i);
}

const OUString& SvXMLAttrContainerData::GetNamespace( sal_uInt16 i ) const
{
    return m_pImpl->GetNamespace(i);
}

const OUString& SvXMLAttrContainerData::GetPrefix( sal_uInt16 i ) const
{
    return m_pImpl->GetPrefix(i);
}

sal_uInt16 SvXMLAttrContainerData::GetFirstNamespaceIndex() const
{
    return m_pImpl->GetFirstNamespaceIndex();
}

sal_uInt16 SvXMLAttrContainerData::GetNextNamespaceIndex( sal_uInt16 nIdx ) const
{
    return m_pImpl->GetNextNamespaceIndex( nIdx );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
