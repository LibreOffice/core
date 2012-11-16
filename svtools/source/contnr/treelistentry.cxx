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

#include <limits>

size_t SvTreeListEntry::ITEM_NOT_FOUND = std::numeric_limits<size_t>::max();

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
    maItems.clear();
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
    maItems.clear();
    ItemsType::iterator it = pSource->maItems.begin(), itEnd = pSource->maItems.end();
    for (; it != itEnd; ++it)
    {
        SvLBoxItem* pItem = &(*it);
        pNewItem = pItem->Create();
        pNewItem->Clone(pItem);
        maItems.push_back(pNewItem);
    }

    pUserData = pSource->GetUserData();
    nEntryFlags = pSource->nEntryFlags;
}

size_t SvTreeListEntry::ItemCount() const
{
    return maItems.size();
}

void SvTreeListEntry::AddItem( SvLBoxItem* pItem )
{
    maItems.push_back( pItem );
}

void SvTreeListEntry::EnableChildrenOnDemand( bool bEnable )
{
    if ( bEnable )
        nEntryFlags |= SV_ENTRYFLAG_CHILDREN_ON_DEMAND;
    else
        nEntryFlags &= (~SV_ENTRYFLAG_CHILDREN_ON_DEMAND);
}

void SvTreeListEntry::ReplaceItem( SvLBoxItem* pNewItem, size_t nPos )
{
    DBG_ASSERT(pNewItem,"ReplaceItem:No Item");
    if (nPos >= maItems.size())
    {
        // Out of bound. Bail out.
        delete pNewItem;
        return;
    }

    maItems.erase(maItems.begin()+nPos);
    maItems.insert(maItems.begin()+nPos, pNewItem);
}

const SvLBoxItem* SvTreeListEntry::GetItem( size_t nPos ) const
{
    return &maItems[nPos];
}

SvLBoxItem* SvTreeListEntry::GetItem( size_t nPos )
{
    return &maItems[nPos];
}

namespace {

class FindByType : std::unary_function<SvLBoxItem, void>
{
    sal_uInt16 mnId;
public:
    FindByType(sal_uInt16 nId) : mnId(nId) {}
    bool operator() (const SvLBoxItem& rItem) const
    {
        return rItem.GetType() == mnId;
    }
};

class FindByPointer : std::unary_function<SvLBoxItem, void>
{
    const SvLBoxItem* mpItem;
public:
    FindByPointer(const SvLBoxItem* p) : mpItem(p) {}
    bool operator() (const SvLBoxItem& rItem) const
    {
        return &rItem == mpItem;
    }
};

}

const SvLBoxItem* SvTreeListEntry::GetFirstItem( sal_uInt16 nId ) const
{
    ItemsType::const_iterator it = std::find_if(maItems.begin(), maItems.end(), FindByType(nId));
    return it == maItems.end() ? NULL : &(*it);
}

SvLBoxItem* SvTreeListEntry::GetFirstItem( sal_uInt16 nId )
{
    ItemsType::iterator it = std::find_if(maItems.begin(), maItems.end(), FindByType(nId));
    return it == maItems.end() ? NULL : &(*it);
}

size_t SvTreeListEntry::GetPos( const SvLBoxItem* pItem ) const
{
    ItemsType::const_iterator it = std::find_if(maItems.begin(), maItems.end(), FindByPointer(pItem));
    return it == maItems.end() ? ITEM_NOT_FOUND : std::distance(maItems.begin(), it);
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
