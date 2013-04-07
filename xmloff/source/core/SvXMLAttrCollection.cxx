/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Nigel Hawkins  - n.hawkins@gmx.com
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Major Contributor(s):
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "SvXMLAttrCollection.hxx"
#include <limits.h>   //USHRT_MAX

bool SvXMLAttrCollection::operator ==( const SvXMLAttrCollection& rCmp ) const
{
    return (rCmp.aNamespaceMap == aNamespaceMap) &&
           (rCmp.aAttrs == aAttrs);
}

sal_Bool SvXMLAttrCollection::AddAttr( const OUString& rLName,
                                       const OUString& rValue )
{
    aAttrs.push_back( SvXMLAttr(rLName, rValue) );
    return sal_True;
}

sal_Bool SvXMLAttrCollection::AddAttr( const OUString& rPrefix,
                                       const OUString& rNamespace,
                                       const OUString& rLName,
                                       const OUString& rValue )
{
    sal_uInt16 nPos = aNamespaceMap.Add( rPrefix, rNamespace );
    aAttrs.push_back( SvXMLAttr(nPos, rLName, rValue) );
    return sal_True;
}

sal_Bool SvXMLAttrCollection::AddAttr( const OUString& rPrefix,
                                       const OUString& rLName,
                                       const OUString& rValue )
{
    sal_uInt16 nPos = aNamespaceMap.GetIndexByPrefix( rPrefix );
    if( USHRT_MAX == nPos )
        return sal_False;
    aAttrs.push_back( SvXMLAttr(nPos, rLName, rValue) );
    return sal_True;
}

sal_Bool SvXMLAttrCollection::SetAt( size_t i,
                                     const OUString& rLName,
                                     const OUString& rValue )
{
    if( i >= GetAttrCount() )
        return sal_False;
    aAttrs[i] = SvXMLAttr(rLName, rValue);
    return sal_True;
}

sal_Bool SvXMLAttrCollection::SetAt( size_t i,
                                     const OUString& rPrefix,
                                     const OUString& rNamespace,
                                     const OUString& rLName,
                                     const OUString& rValue )
{
    if( i >= GetAttrCount() )
        return sal_False;

    sal_uInt16 nPos = aNamespaceMap.Add( rPrefix, rNamespace );
    if( USHRT_MAX == nPos )
        return sal_False;

    aAttrs[i] = SvXMLAttr(nPos, rLName, rValue);
    return sal_True;
}

sal_Bool SvXMLAttrCollection::SetAt( size_t i,
                                     const OUString& rPrefix,
                                     const OUString& rLName,
                                     const OUString& rValue )
{
    if( i >= GetAttrCount() )
        return sal_False;

    sal_uInt16 nPos = aNamespaceMap.GetIndexByPrefix( rPrefix );
    if( USHRT_MAX == nPos )
        return sal_False;

    aAttrs[i] = SvXMLAttr(nPos, rLName, rValue);
    return sal_True;
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
//  DBG_ASSERT( i >= 0 && i < aAttrs.size(),
//              "SvXMLAttrCollection::GetPrefixPos: illegal index" );
    return aAttrs[i].getPrefixPos();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

