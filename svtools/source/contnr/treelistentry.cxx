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

#include <svtools/treelistentry.hxx>
#include <svtools/treelist.hxx>

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <limits>

void SvTreeListEntry::ClearChildren()
{
    m_Children.clear();
}

void SvTreeListEntry::SetListPositions()
{
    sal_uLong nCur = 0;
    for (auto const& pEntry : m_Children)
    {
        SvTreeListEntry& rEntry = *pEntry;
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

SvTreeListEntry::SvTreeListEntry()
    : pParent(nullptr)
    , nAbsPos(0)
    , nListPos(0)
    , bIsMarked(false)
    , pUserData(nullptr)
    , nEntryFlags(SvTLEntryFlags::NONE)
    , maBackColor(Application::GetSettings().GetStyleSettings().GetWindowColor())
{
}

SvTreeListEntry::SvTreeListEntry(const SvTreeListEntry& r)
    : pParent(nullptr)
    , nAbsPos(r.nAbsPos)
    , nListPos(r.nListPos & 0x7FFFFFFF)
    , bIsMarked(r.bIsMarked)
    , pUserData(r.pUserData)
    , nEntryFlags(r.nEntryFlags)
    , maBackColor(Application::GetSettings().GetStyleSettings().GetWindowColor())
{
    for (auto const& it : r.m_Children)
        m_Children.push_back(std::unique_ptr<SvTreeListEntry>(new SvTreeListEntry(*it)));
}

SvTreeListEntry::~SvTreeListEntry()
{
#ifdef DBG_UTIL
    pParent     = nullptr;
#endif

    m_Children.clear();
    m_Items.clear();
}

bool SvTreeListEntry::HasChildren() const
{
    return !m_Children.empty();
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



void SvTreeListEntry::Clone(SvTreeListEntry* pSource)
{
    nListPos &= 0x80000000;
    nListPos |= ( pSource->nListPos & 0x7fffffff);
    nAbsPos     = pSource->nAbsPos;

    m_Items.clear();
    for (auto const& it : pSource->m_Items)
    {
        SvLBoxItem* pItem = &(*it);
        std::unique_ptr<SvLBoxItem> pNewItem(pItem->Create());
        pNewItem->Clone(pItem);
        m_Items.push_back(std::move(pNewItem));
    }

    pUserData = pSource->GetUserData();
    nEntryFlags = pSource->nEntryFlags;
}

size_t SvTreeListEntry::ItemCount() const
{
    return m_Items.size();
}

void SvTreeListEntry::AddItem(std::unique_ptr<SvLBoxItem> pItem)
{
    m_Items.push_back(std::move(pItem));
}

void SvTreeListEntry::EnableChildrenOnDemand( bool bEnable )
{
    if ( bEnable )
        nEntryFlags |= SvTLEntryFlags::CHILDREN_ON_DEMAND;
    else
        nEntryFlags &= (~SvTLEntryFlags::CHILDREN_ON_DEMAND);
}

void SvTreeListEntry::ReplaceItem(std::unique_ptr<SvLBoxItem> pNewItem, size_t const nPos)
{
    DBG_ASSERT(pNewItem,"ReplaceItem:No Item");
    if (nPos >= m_Items.size())
    {
        // Out of bound. Bail out.
        pNewItem.reset();
        return;
    }

    m_Items.erase(m_Items.begin()+nPos);
    m_Items.insert(m_Items.begin()+nPos, std::move(pNewItem));
}

const SvLBoxItem& SvTreeListEntry::GetItem( size_t nPos ) const
{
    return *m_Items[nPos];
}

SvLBoxItem& SvTreeListEntry::GetItem( size_t nPos )
{
    return *m_Items[nPos];
}

namespace {

class FindByType : std::unary_function<SvLBoxItem, void>
{
    sal_uInt16 mnId;
public:
    explicit FindByType(sal_uInt16 nId) : mnId(nId) {}
    bool operator() (const std::unique_ptr<SvLBoxItem>& rpItem) const
    {
        return rpItem->GetType() == mnId;
    }
};

class FindByPointer : std::unary_function<SvLBoxItem, void>
{
    const SvLBoxItem* mpItem;
public:
    explicit FindByPointer(const SvLBoxItem* p) : mpItem(p) {}
    bool operator() (const std::unique_ptr<SvLBoxItem>& rpItem) const
    {
        return rpItem.get() == mpItem;
    }
};

}

const SvLBoxItem* SvTreeListEntry::GetFirstItem( sal_uInt16 nId ) const
{
    ItemsType::const_iterator it = std::find_if(m_Items.begin(), m_Items.end(), FindByType(nId));
    return (it == m_Items.end()) ? nullptr : (*it).get();
}

SvLBoxItem* SvTreeListEntry::GetFirstItem( sal_uInt16 nId )
{
    ItemsType::iterator it = std::find_if(m_Items.begin(), m_Items.end(), FindByType(nId));
    return (it == m_Items.end()) ? nullptr : (*it).get();
}

size_t SvTreeListEntry::GetPos( const SvLBoxItem* pItem ) const
{
    ItemsType::const_iterator it = std::find_if(m_Items.begin(), m_Items.end(), FindByPointer(pItem));
    return it == m_Items.end() ? ITEM_NOT_FOUND : std::distance(m_Items.begin(), it);
}


void SvTreeListEntry::SetUserData( void* pPtr )
{
    pUserData = pPtr;
}

bool SvTreeListEntry::HasChildrenOnDemand() const
{
    return (bool)(nEntryFlags & SvTLEntryFlags::CHILDREN_ON_DEMAND);
}

bool SvTreeListEntry::HasInUseEmphasis() const
{
    return (bool)(nEntryFlags & SvTLEntryFlags::IN_USE);
}


void SvTreeListEntry::SetFlags( SvTLEntryFlags nFlags )
{
    nEntryFlags = nFlags;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
