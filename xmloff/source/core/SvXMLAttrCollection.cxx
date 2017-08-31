/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "SvXMLAttrCollection.hxx"
#include <limits.h>
#include <osl/diagnose.h>

bool SvXMLAttrCollection::operator ==( const SvXMLAttrCollection& rCmp ) const
{
    return (rCmp.aNamespaceMap == aNamespaceMap) &&
           (rCmp.aAttrs == aAttrs);
}

bool SvXMLAttrCollection::AddAttr( const OUString& rLName,
                                       const OUString& rValue )
{
    aAttrs.emplace_back(rLName, rValue );
    return true;
}

bool SvXMLAttrCollection::AddAttr( const OUString& rPrefix,
                                       const OUString& rNamespace,
                                       const OUString& rLName,
                                       const OUString& rValue )
{
    sal_uInt16 nPos = aNamespaceMap.Add( rPrefix, rNamespace );
    aAttrs.emplace_back(nPos, rLName, rValue );
    return true;
}

bool SvXMLAttrCollection::AddAttr( const OUString& rPrefix,
                                       const OUString& rLName,
                                       const OUString& rValue )
{
    sal_uInt16 nPos = aNamespaceMap.GetIndexByPrefix( rPrefix );
    if( USHRT_MAX == nPos )
        return false;
    aAttrs.emplace_back(nPos, rLName, rValue );
    return true;
}

bool SvXMLAttrCollection::SetAt( size_t i,
                                     const OUString& rLName,
                                     const OUString& rValue )
{
    if( i >= GetAttrCount() )
        return false;
    aAttrs[i] = SvXMLAttr(rLName, rValue);
    return true;
}

bool SvXMLAttrCollection::SetAt( size_t i,
                                     const OUString& rPrefix,
                                     const OUString& rNamespace,
                                     const OUString& rLName,
                                     const OUString& rValue )
{
    if( i >= GetAttrCount() )
        return false;

    sal_uInt16 nPos = aNamespaceMap.Add( rPrefix, rNamespace );
    if( USHRT_MAX == nPos )
        return false;

    aAttrs[i] = SvXMLAttr(nPos, rLName, rValue);
    return true;
}

bool SvXMLAttrCollection::SetAt( size_t i,
                                     const OUString& rPrefix,
                                     const OUString& rLName,
                                     const OUString& rValue )
{
    if( i >= GetAttrCount() )
        return false;

    sal_uInt16 nPos = aNamespaceMap.GetIndexByPrefix( rPrefix );
    if( USHRT_MAX == nPos )
        return false;

    aAttrs[i] = SvXMLAttr(nPos, rLName, rValue);
    return true;
}

void SvXMLAttrCollection::Remove( size_t i )
{
    if( i < GetAttrCount() )
    {
        aAttrs.erase( aAttrs.begin() + i );
    }
    else
    {
        OSL_FAIL( "illegal index" );
    }
}

size_t SvXMLAttrCollection::GetAttrCount() const
{
    return aAttrs.size();
}

const OUString& SvXMLAttrCollection::GetAttrLName(size_t i) const
{
    OSL_ENSURE( i < aAttrs.size(), "SvXMLAttrContainerData::GetLName: illegal index" );
    return aAttrs[i].getLName();
}

const OUString& SvXMLAttrCollection::GetAttrValue(size_t i) const
{
    OSL_ENSURE( i < aAttrs.size(), "SvXMLAttrContainerData::GetValue: illegal index" );
    return aAttrs[i].getValue();
}

const OUString SvXMLAttrCollection::GetAttrNamespace( size_t i ) const
{
    OUString sRet;
    sal_uInt16 nPos = GetPrefixPos( i );
    //Does this point to a valid namespace entry?
    if( USHRT_MAX != nPos )
        sRet = aNamespaceMap.GetNameByIndex( nPos );
    return sRet;
}

const OUString SvXMLAttrCollection::GetAttrPrefix( size_t i ) const
{
    OUString sRet;
    sal_uInt16 nPos = GetPrefixPos( i );
    //Does this point to a valid namespace entry?
    if( USHRT_MAX != nPos )
        sRet = aNamespaceMap.GetPrefixByIndex( nPos );
    return sRet;
}

const OUString& SvXMLAttrCollection::GetNamespace( sal_uInt16 i ) const
{
    return aNamespaceMap.GetNameByIndex( i );
}

const OUString& SvXMLAttrCollection::GetPrefix( sal_uInt16 i ) const
{
    return aNamespaceMap.GetPrefixByIndex( i );
}

sal_uInt16 SvXMLAttrCollection::GetFirstNamespaceIndex() const
{
    return aNamespaceMap.GetFirstIndex();
}

sal_uInt16 SvXMLAttrCollection::GetNextNamespaceIndex( sal_uInt16 nIdx ) const
{
    return aNamespaceMap.GetNextIndex( nIdx );
}

sal_uInt16 SvXMLAttrCollection::GetPrefixPos( size_t i ) const
{
//  SAL_WARN_IF( i < 0 || i >= aAttrs.size()), "xmloff",
//              "SvXMLAttrCollection::GetPrefixPos: illegal index" );
    return aAttrs[i].getPrefixPos();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

