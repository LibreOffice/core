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

#include "svtools/treelistentry.hxx"
#include "svtools/treelist.hxx"
#include "svtools/treelistbox.hxx"

void SvTreeListEntry::ClearChildren()
{
    maChildren.clear();
}

void SvTreeListEntry::SetListPositions()
{
    SvTreeListEntries::iterator it = maChildren.begin(), itEnd = maChildren.end();
    sal_uLong nCur = 0;
    for (; it != itEnd; ++it)
    {
        SvTreeListEntry& rEntry = *it;
        rEntry.nListPos &= 0x80000000;
        rEntry.nListPos |= nCur;
        ++nCur;
    }

    nListPos &= (~0x80000000); // remove the invalid bit.
}

void SvTreeListEntry::InvalidateChildrensListPositions()
{
    nListPos |= 0x80000000;
}

void SvTreeListEntry::DeleteItems_Impl()
{
    sal_uInt16 nCount = aItems.size();
    while( nCount )
    {
        nCount--;
        SvLBoxItem* pItem = aItems[ nCount ];
        delete pItem;
    }
    aItems.clear();
}

SvTreeListEntry::SvTreeListEntry() :
    pParent(NULL),
    nAbsPos(0),
    nListPos(0),
    pUserData(NULL),
    nEntryFlags(0)
{
}

SvTreeListEntry::SvTreeListEntry(const SvTreeListEntry& r) :
    pParent(NULL),
    nAbsPos(r.nAbsPos),
    nListPos(r.nListPos & 0x7FFFFFFF)
{
    SvTreeListEntries::const_iterator it = r.maChildren.begin(), itEnd = r.maChildren.end();
    for (; it != itEnd; ++it)
        maChildren.push_back(new SvTreeListEntry(*it));
}

SvTreeListEntry::~SvTreeListEntry()
{
#ifdef DBG_UTIL
    pParent     = 0;
#endif

    maChildren.clear();
    DeleteItems_Impl();
}

bool SvTreeListEntry::HasChildren() const
{
    return !maChildren.empty();
}

bool SvTreeListEntry::HasChildListPos() const
{
    if( pParent && !(pParent->nListPos & 0x80000000) )
        return true;
    else return false;
}

sal_uLong SvTreeListEntry::GetChildListPos() const
{
    if( pParent && (pParent->nListPos & 0x80000000) )
        pParent->SetListPositions();
    return ( nListPos & 0x7fffffff );
}

SvTreeListEntries& SvTreeListEntry::GetChildEntries()
{
    return maChildren;
}

const SvTreeListEntries& SvTreeListEntry::GetChildEntries() const
{
    return maChildren;
}

void SvTreeListEntry::Clone(SvTreeListEntry* pSource)
{
    nListPos &= 0x80000000;
    nListPos |= ( pSource->nListPos & 0x7fffffff);
    nAbsPos     = pSource->nAbsPos;

    SvLBoxItem* pNewItem;
    DeleteItems_Impl();
    sal_uInt16 nCount = ((SvTreeListEntry*)pSource)->ItemCount();
    sal_uInt16 nCurPos = 0;
    while( nCurPos < nCount )
    {
        SvLBoxItem* pItem = ((SvTreeListEntry*)pSource)->GetItem( nCurPos );
        pNewItem = pItem->Create();
        pNewItem->Clone( pItem );
        AddItem( pNewItem );
        nCurPos++;
    }
    pUserData = ((SvTreeListEntry*)pSource)->GetUserData();
    nEntryFlags = ((SvTreeListEntry*)pSource)->nEntryFlags;
}

sal_uInt16 SvTreeListEntry::ItemCount() const
{
    return (sal_uInt16)aItems.size();
}

void SvTreeListEntry::AddItem( SvLBoxItem* pItem )
{
    aItems.push_back( pItem );
}

void SvTreeListEntry::EnableChildrenOnDemand( bool bEnable )
{
    if ( bEnable )
        nEntryFlags |= SV_ENTRYFLAG_CHILDREN_ON_DEMAND;
    else
        nEntryFlags &= (~SV_ENTRYFLAG_CHILDREN_ON_DEMAND);
}

void SvTreeListEntry::ReplaceItem( SvLBoxItem* pNewItem, sal_uInt16 nPos )
{
    DBG_ASSERT(pNewItem,"ReplaceItem:No Item");
    SvLBoxItem* pOld = GetItem( nPos );
    if ( pOld )
    {
        aItems[ nPos ] = pNewItem;
        delete pOld;
    }
}

SvLBoxItem* SvTreeListEntry::GetItem( sal_uInt16 nPos ) const
{
    return aItems[nPos];
}

SvLBoxItem* SvTreeListEntry::GetFirstItem( sal_uInt16 nId ) const
{
    sal_uInt16 nCount = aItems.size();
    sal_uInt16 nCur = 0;
    SvLBoxItem* pItem;
    while( nCur < nCount )
    {
        pItem = GetItem( nCur );
        if( pItem->IsA() == nId )
            return pItem;
        nCur++;
    }
    return 0;
}

sal_uInt16 SvTreeListEntry::GetPos( SvLBoxItem* pItem ) const
{
    std::vector<SvLBoxItem*>::const_iterator it = std::find( aItems.begin(), aItems.end(), pItem );
    return it == aItems.end() ? USHRT_MAX : it - aItems.begin();
}

void* SvTreeListEntry::GetUserData() const
{
    return pUserData;
}

void SvTreeListEntry::SetUserData( void* pPtr )
{
    pUserData = pPtr;
}

bool SvTreeListEntry::HasChildrenOnDemand() const
{
    return (bool)((nEntryFlags & SV_ENTRYFLAG_CHILDREN_ON_DEMAND)!=0);
}

bool SvTreeListEntry::HasInUseEmphasis() const
{
    return (bool)((nEntryFlags & SV_ENTRYFLAG_IN_USE)!=0);
}

sal_uInt16 SvTreeListEntry::GetFlags() const
{
    return nEntryFlags;
}

void SvTreeListEntry::SetFlags( sal_uInt16 nFlags )
{
    nEntryFlags = nFlags;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
