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

#include "SvXMLAttrCollection.hxx"
#include <xmloff/xmlcnimp.hxx>
#include <rtl/ustring.hxx>     //rtl::OUString

SvXMLAttrContainerData::SvXMLAttrContainerData() : pimpl( new SvXMLAttrCollection() )
{
}

SvXMLAttrContainerData::SvXMLAttrContainerData(const SvXMLAttrContainerData &rCopy) :
        pimpl( new SvXMLAttrCollection( *(rCopy.pimpl) ) )
{
}

//Need destructor defined (despite it being empty) to avoid "checked_delete"
//compiler errors.
SvXMLAttrContainerData::~SvXMLAttrContainerData()
{
}

int SvXMLAttrContainerData::operator ==( const SvXMLAttrContainerData& rCmp ) const
{
    return ( *(rCmp.pimpl) == *(pimpl) );
}

sal_Bool SvXMLAttrContainerData::AddAttr( const rtl::OUString& rLName,
                                          const rtl::OUString& rValue )
{
    return pimpl->AddAttr(rLName, rValue);
}

sal_Bool SvXMLAttrContainerData::AddAttr( const rtl::OUString& rPrefix,
                                          const rtl::OUString& rNamespace,
                                          const rtl::OUString& rLName,
                                          const rtl::OUString& rValue )
{
    return pimpl->AddAttr(rPrefix, rNamespace, rLName, rValue);
}

sal_Bool SvXMLAttrContainerData::AddAttr( const rtl::OUString& rPrefix,
                                          const rtl::OUString& rLName,
                                          const rtl::OUString& rValue )
{
    return pimpl->AddAttr(rPrefix, rLName, rValue);
}

sal_Bool SvXMLAttrContainerData::SetAt( size_t i,
                                        const rtl::OUString& rLName,
                                        const rtl::OUString& rValue )
{
    return pimpl->SetAt(i, rLName, rValue);
}

sal_Bool SvXMLAttrContainerData::SetAt( size_t i,
                                        const rtl::OUString& rPrefix,
                                        const rtl::OUString& rNamespace,
                                        const rtl::OUString& rLName,
                                        const rtl::OUString& rValue )
{
    return pimpl->SetAt(i, rPrefix, rNamespace, rLName, rValue);
}

sal_Bool SvXMLAttrContainerData::SetAt( size_t i,
                                        const rtl::OUString& rPrefix,
                                        const rtl::OUString& rLName,
                                        const rtl::OUString& rValue )
{
    return pimpl->SetAt(i, rPrefix, rLName, rValue);
}

void SvXMLAttrContainerData::Remove( size_t i )
{
    pimpl->Remove(i);
}

size_t SvXMLAttrContainerData::GetAttrCount() const
{
    return pimpl->GetAttrCount();
}

const rtl::OUString& SvXMLAttrContainerData::GetAttrLName(size_t i) const
{
    return pimpl->GetAttrLName(i);
}

const rtl::OUString& SvXMLAttrContainerData::GetAttrValue(size_t i) const
{
    return pimpl->GetAttrValue(i);
}

const rtl::OUString SvXMLAttrContainerData::GetAttrNamespace( size_t i ) const
{
    return pimpl->GetAttrNamespace(i);
}

const rtl::OUString SvXMLAttrContainerData::GetAttrPrefix( size_t i ) const
{
    return pimpl->GetAttrPrefix(i);
}

const rtl::OUString& SvXMLAttrContainerData::GetNamespace( sal_uInt16 i ) const
{
    return pimpl->GetNamespace(i);
}

const rtl::OUString& SvXMLAttrContainerData::GetPrefix( sal_uInt16 i ) const
{
    return pimpl->GetPrefix(i);
}

sal_uInt16 SvXMLAttrContainerData::GetFirstNamespaceIndex() const
{
    return pimpl->GetFirstNamespaceIndex();
}

sal_uInt16 SvXMLAttrContainerData::GetNextNamespaceIndex( sal_uInt16 nIdx ) const
{
    return pimpl->GetNextNamespaceIndex( nIdx );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
