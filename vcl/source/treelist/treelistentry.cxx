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

#include <memory>
#include <vcl/treelistentry.hxx>
#include <vcl/treelist.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <tools/debug.hxx>

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
    , pUserData(nullptr)
    , nEntryFlags(SvTLEntryFlags::NONE)
    , maBackColor(Application::GetSettings().GetStyleSettings().GetWindowColor())
{
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
    return pParent && !(pParent->nListPos & 0x80000000);
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
        std::unique_ptr<SvLBoxItem> pNewItem(pItem->Clone(pItem));
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
        nEntryFlags &= ~SvTLEntryFlags::CHILDREN_ON_DEMAND;
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

class FindByType
{
    SvLBoxItemType const meType;
public:
    explicit FindByType(SvLBoxItemType eType) : meType(eType) {}
    bool operator() (const std::unique_ptr<SvLBoxItem>& rpItem) const
    {
        return rpItem->GetType() == meType;
    }
};

class FindByPointer
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

const SvLBoxItem* SvTreeListEntry::GetFirstItem(SvLBoxItemType eType) const
{
    ItemsType::const_iterator it = std::find_if(m_Items.begin(), m_Items.end(), FindByType(eType));
    return (it == m_Items.end()) ? nullptr : (*it).get();
}

SvLBoxItem* SvTreeListEntry::GetFirstItem(SvLBoxItemType eType)
{
    ItemsType::iterator it = std::find_if(m_Items.begin(), m_Items.end(), FindByType(eType));
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
    return static_cast<bool>(nEntryFlags & SvTLEntryFlags::CHILDREN_ON_DEMAND);
}

void SvTreeListEntry::SetFlags( SvTLEntryFlags nFlags )
{
    nEntryFlags = nFlags;
}

SvTreeListEntry* SvTreeListEntry::NextSibling() const
{
    SvTreeListEntries& rList = pParent->m_Children;
    sal_uLong nPos = GetChildListPos();
    nPos++;
    return (nPos < rList.size()) ? rList[nPos].get() : nullptr;
}

SvTreeListEntry* SvTreeListEntry::PrevSibling() const
{
    SvTreeListEntries& rList = pParent->m_Children;
    sal_uLong nPos = GetChildListPos();
    if ( nPos == 0 )
        return nullptr;
    nPos--;
    return rList[nPos].get();
}


SvTreeListEntry* SvTreeListEntry::LastSibling() const
{
    SvTreeListEntries& rChildren = pParent->m_Children;
    return (rChildren.empty()) ? nullptr : rChildren.back().get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
