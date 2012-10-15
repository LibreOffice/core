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

#include "tools/debug.hxx"

DBG_NAME(SvListEntry);

SvListEntry::SvListEntry()
{
    DBG_CTOR(SvListEntry,0);
    pChildren     = 0;
    pParent     = 0;
    nListPos    = 0;
    nAbsPos     = 0;
}

SvListEntry::SvListEntry( const SvListEntry& rEntry )
{
    DBG_CTOR(SvListEntry,0);
    pChildren  = 0;
    pParent  = 0;
    nListPos &= 0x80000000;
    nListPos |= ( rEntry.nListPos & 0x7fffffff);
    nAbsPos  = rEntry.nAbsPos;
}

SvListEntry::~SvListEntry()
{
    DBG_DTOR(SvListEntry,0);
    if ( pChildren )
    {
        pChildren->DestroyAll();
        delete pChildren;
    }
#ifdef DBG_UTIL
    pChildren     = 0;
    pParent     = 0;
#endif
}

void SvListEntry::Clone( SvListEntry* pSource)
{
    DBG_CHKTHIS(SvListEntry,0);
    nListPos &= 0x80000000;
    nListPos |= ( pSource->nListPos & 0x7fffffff);
    nAbsPos     = pSource->nAbsPos;
}

void SvListEntry::SetListPositions()
{
    if( pChildren )
    {
        SvTreeEntryList::iterator it = pChildren->begin(), itEnd = pChildren->end();
        sal_uLong nCur = 0;
        for (; it != itEnd; ++it)
        {
            SvListEntry* pEntry = *it;
            pEntry->nListPos &= 0x80000000;
            pEntry->nListPos |= nCur;
            ++nCur;
        }
    }
    nListPos &= (~0x80000000);
}

// ***************************************************************
// class SvLBoxEntry
// ***************************************************************

DBG_NAME(SvLBoxEntry);

SvLBoxEntry::SvLBoxEntry() : aItems()
{
    DBG_CTOR(SvLBoxEntry,0);
    nEntryFlags = 0;
    pUserData = 0;
}

SvLBoxEntry::~SvLBoxEntry()
{
    DBG_DTOR(SvLBoxEntry,0);
    DeleteItems_Impl();
}

void SvLBoxEntry::DeleteItems_Impl()
{
    DBG_CHKTHIS(SvLBoxEntry,0);
    sal_uInt16 nCount = aItems.size();
    while( nCount )
    {
        nCount--;
        SvLBoxItem* pItem = aItems[ nCount ];
        delete pItem;
    }
    aItems.clear();
}


void SvLBoxEntry::AddItem( SvLBoxItem* pItem )
{
    DBG_CHKTHIS(SvLBoxEntry,0);
    aItems.push_back( pItem );
}

void SvLBoxEntry::Clone( SvListEntry* pSource )
{
    DBG_CHKTHIS(SvLBoxEntry,0);
    SvListEntry::Clone( pSource );
    SvLBoxItem* pNewItem;
    DeleteItems_Impl();
    sal_uInt16 nCount = ((SvLBoxEntry*)pSource)->ItemCount();
    sal_uInt16 nCurPos = 0;
    while( nCurPos < nCount )
    {
        SvLBoxItem* pItem = ((SvLBoxEntry*)pSource)->GetItem( nCurPos );
        pNewItem = pItem->Create();
        pNewItem->Clone( pItem );
        AddItem( pNewItem );
        nCurPos++;
    }
    pUserData = ((SvLBoxEntry*)pSource)->GetUserData();
    nEntryFlags = ((SvLBoxEntry*)pSource)->nEntryFlags;
}

void SvLBoxEntry::EnableChildrenOnDemand( sal_Bool bEnable )
{
    DBG_CHKTHIS(SvLBoxEntry,0);
    if ( bEnable )
        nEntryFlags |= SV_ENTRYFLAG_CHILDREN_ON_DEMAND;
    else
        nEntryFlags &= (~SV_ENTRYFLAG_CHILDREN_ON_DEMAND);
}

void SvLBoxEntry::ReplaceItem( SvLBoxItem* pNewItem, sal_uInt16 nPos )
{
    DBG_CHKTHIS(SvLBoxEntry,0);
    DBG_ASSERT(pNewItem,"ReplaceItem:No Item");
    SvLBoxItem* pOld = GetItem( nPos );
    if ( pOld )
    {
        aItems[ nPos ] = pNewItem;
        delete pOld;
    }
}

SvLBoxItem* SvLBoxEntry::GetFirstItem( sal_uInt16 nId )
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

sal_uInt16 SvLBoxEntry::GetPos( SvLBoxItem* pItem ) const
{
    std::vector<SvLBoxItem*>::const_iterator it = std::find( aItems.begin(), aItems.end(), pItem );
    return it == aItems.end() ? USHRT_MAX : it - aItems.begin();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
