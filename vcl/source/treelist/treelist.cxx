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

#include <vcl/toolkit/treelist.hxx>
#include <vcl/toolkit/treelistentry.hxx>
#include <vcl/toolkit/viewdataentry.hxx>
#include <tools/debug.hxx>
#include <osl/diagnose.h>

#include <cassert>
#include <memory>
#include <utility>

SvTreeList::SvTreeList(SvTreeListBox& rTreeListBox)
    : mrOwnerListView(rTreeListBox)
    , mbEnableInvalidate(true)
{
    m_nEntryCount = 0;
    m_bAbsPositionsValid = false;
    m_pRootItem.reset(new SvTreeListEntry);
    m_eSortMode = SvSortMode::None;
}

SvTreeList::~SvTreeList()
{
}

void SvTreeList::Broadcast(SvListAction eAction, SvTreeListEntry* pEntry)
{
    mrOwnerListView.ModelNotification(eAction, pEntry);
}

// an entry is visible if all parents are expanded
bool SvTreeList::IsEntryVisible(const SvTreeListBox& rView, SvTreeListEntry* pEntry) const
{
    assert(pEntry && "IsVisible: Invalid Param");
    bool bRetVal = false;
    do
    {
        if (pEntry == m_pRootItem.get())
        {
            bRetVal = true;
            break;
        }
        pEntry = pEntry->pParent;
    } while (rView.IsExpanded(pEntry));
    return bRetVal;
}

sal_uInt16 SvTreeList::GetDepth( const SvTreeListEntry* pEntry ) const
{
    DBG_ASSERT(pEntry && pEntry != m_pRootItem.get(), "GetDepth:Bad Entry");
    sal_uInt16 nDepth = 0;
    while (pEntry && pEntry->pParent != m_pRootItem.get())
    {
        nDepth++;
        pEntry = pEntry->pParent;
    }
    return nDepth;
}

bool SvTreeList::IsAtRootDepth( const SvTreeListEntry* pEntry ) const
{
    return pEntry->pParent == m_pRootItem.get();
}

void SvTreeList::Clear()
{
    m_pRootItem->ClearChildren();
    m_nEntryCount = 0;
}

bool SvTreeList::IsChild(const SvTreeListEntry* pParent, const SvTreeListEntry* pChild) const
{
    if ( !pParent )
        pParent = m_pRootItem.get();

    if (pParent->m_Children.empty())
        return false;

    for (auto const& it : pParent->m_Children)
    {
        const SvTreeListEntry* pThis = it.get();
        if (pThis == pChild)
            return true;
        else
        {
            bool bIsChild = IsChild(pThis, pChild);
            if (bIsChild)
                return true;
        }
    }
    return false;
}

namespace {

class FindByPointer
{
    const SvTreeListEntry* mpEntry;
public:
    explicit FindByPointer(const SvTreeListEntry* p) : mpEntry(p) {}

    bool operator() (std::unique_ptr<SvTreeListEntry> const& rpEntry) const
    {
        return mpEntry == rpEntry.get();
    }
};

sal_uInt32 findEntryPosition(const SvTreeListEntries& rDst, const SvTreeListEntry* pEntry)
{
    SvTreeListEntries::const_iterator itPos = std::find_if(rDst.begin(), rDst.end(), FindByPointer(pEntry));
    if (itPos == rDst.end())
        return static_cast<sal_uInt32>(~0);

    return static_cast<sal_uInt32>(std::distance(rDst.begin(), itPos));
}

}

sal_uInt32 SvTreeList::Move(SvTreeListEntry* pSrcEntry,SvTreeListEntry* pTargetParent,sal_uInt32 nListPos)
{
    // pDest may be 0!
    assert(pSrcEntry && "Entry?");
    if ( !pTargetParent )
        pTargetParent = m_pRootItem.get();
    DBG_ASSERT(pSrcEntry!=pTargetParent,"Move:Source=Target");

    Broadcast(SvListAction::MOVING, pSrcEntry);

    if ( pSrcEntry == pTargetParent )
        // You can't move an entry onto itself as the parent. Just return its
        // position and bail out.
        return pSrcEntry->GetChildListPos();

    m_bAbsPositionsValid = false;

    SvTreeListEntries& rDst = pTargetParent->m_Children;
    SvTreeListEntries& rSrc = pSrcEntry->pParent->m_Children;

    bool bSameParent = pTargetParent == pSrcEntry->pParent;

    // Find the position of the entry being moved in the source container.
    SvTreeListEntries::iterator itSrcPos = rSrc.begin(), itEnd = rSrc.end();
    for (; itSrcPos != itEnd; ++itSrcPos)
    {
        const SvTreeListEntry* p = (*itSrcPos).get();
        if (p == pSrcEntry)
            // Found
            break;
    }

    if (itSrcPos == itEnd)
    {
        OSL_FAIL("Source entry not found! This should never happen.");
        return pSrcEntry->GetChildListPos();
    }

    if (bSameParent)
    {
        // Moving within the same parent.

        size_t nSrcPos = std::distance(rSrc.begin(), itSrcPos);
        if (nSrcPos == nListPos)
            // Nothing to move here.
            return pSrcEntry->GetChildListPos();

        if (nSrcPos < nListPos)
            // Destination position shifts left after removing the original.
            --nListPos;

        // Release the original.
        std::unique_ptr<SvTreeListEntry> pOriginal(std::move(*itSrcPos));
        assert(pOriginal);
        rSrc.erase(itSrcPos);

        // Determine the insertion position.
        SvTreeListEntries::iterator itDstPos = rSrc.end();
        if (nListPos < rSrc.size())
        {
            itDstPos = rSrc.begin();
            std::advance(itDstPos, nListPos);
        }
        rSrc.insert(itDstPos, std::move(pOriginal));
    }
    else
    {
        // Moving from one parent to another.
        SvTreeListEntries::iterator itDstPos = rDst.end();
        if (nListPos < rDst.size())
        {
            itDstPos = rDst.begin();
            std::advance(itDstPos, nListPos);
        }
        std::unique_ptr<SvTreeListEntry> pOriginal(std::move(*itSrcPos));
        assert(pOriginal);
        rSrc.erase(itSrcPos);
        rDst.insert(itDstPos, std::move(pOriginal));
    }

    // move parent (do this only now, because we need the parent for
    // deleting the old child list!)
    pSrcEntry->pParent = pTargetParent;

    // correct list position in target list
    SetListPositions(rDst);
    if (!bSameParent)
        SetListPositions(rSrc);

    sal_uInt32 nRetVal = findEntryPosition(rDst, pSrcEntry);
    OSL_ENSURE(nRetVal == pSrcEntry->GetChildListPos(), "ListPos not valid");
    Broadcast(SvListAction::MOVED, pSrcEntry);
    return nRetVal;
}

sal_uInt32 SvTreeList::Copy(SvTreeListEntry& rSrcEntry, SvTreeListEntry* pTargetParent,
                            sal_uInt32 nListPos)
{
    if ( !pTargetParent )
        pTargetParent = m_pRootItem.get();

    m_bAbsPositionsValid = false;

    sal_uInt32 nCloneCount = 0;
    SvTreeListEntry* pClonedEntry = Clone(rSrcEntry, nCloneCount);
    m_nEntryCount += nCloneCount;

    SvTreeListEntries& rDst = pTargetParent->m_Children;

    pClonedEntry->pParent = pTargetParent;      // move parent

    if (nListPos < rDst.size())
    {
        SvTreeListEntries::iterator itPos = rDst.begin(); // insertion position.
        std::advance(itPos, nListPos);
        rDst.insert(itPos, std::unique_ptr<SvTreeListEntry>(pClonedEntry));
    }
    else
        rDst.push_back(std::unique_ptr<SvTreeListEntry>(pClonedEntry));

    SetListPositions(rDst); // correct list position in target list

    Broadcast( SvListAction::INSERTED_TREE, pClonedEntry );
    sal_uInt32 nRetVal = findEntryPosition(rDst, pClonedEntry);
    return nRetVal;
}

void SvTreeList::Move( SvTreeListEntry* pSrcEntry, SvTreeListEntry* pDstEntry )
{
    SvTreeListEntry* pParent;
    sal_uInt32 nPos;

    if ( !pDstEntry )
    {
        pParent = m_pRootItem.get();
        nPos = 0;
    }
    else
    {
        pParent = pDstEntry->pParent;
        nPos = pDstEntry->GetChildListPos();
        nPos++;  // (On screen:) insert _below_ pDstEntry
    }
    Move( pSrcEntry, pParent, nPos );
}

void SvTreeList::InsertTree(SvTreeListEntry* pSrcEntry,
    SvTreeListEntry* pTargetParent,sal_uInt32 nListPos)
{
    DBG_ASSERT(pSrcEntry,"InsertTree:Entry?");
    if ( !pSrcEntry )
        return;

    if ( !pTargetParent )
        pTargetParent = m_pRootItem.get();

    // take sorting into account
    GetInsertionPos(*pSrcEntry, pTargetParent, nListPos);

    m_bAbsPositionsValid = false;

    pSrcEntry->pParent = pTargetParent; // move parent
    SvTreeListEntries& rDst = pTargetParent->m_Children;

    if (nListPos < rDst.size())
    {
        SvTreeListEntries::iterator itPos = rDst.begin();
        std::advance(itPos, nListPos);
        rDst.insert(itPos, std::unique_ptr<SvTreeListEntry>(pSrcEntry));
    }
    else
        rDst.push_back(std::unique_ptr<SvTreeListEntry>(pSrcEntry));

    SetListPositions(rDst); // correct list position in target list
    m_nEntryCount += GetChildCount(pSrcEntry);
    m_nEntryCount++; // the parent is new, too

    Broadcast(SvListAction::INSERTED_TREE, pSrcEntry );
}

SvTreeListEntry* SvTreeList::CloneEntry(SvTreeListEntry& rSource) const
{
    if (m_aCloneLink.IsSet())
        return m_aCloneLink.Call(rSource);
    SvTreeListEntry* pEntry = new SvTreeListEntry;
    pEntry->Clone(rSource);
    return pEntry;
}

SvTreeListEntry* SvTreeList::Clone(SvTreeListEntry& rEntry, sal_uInt32& nCloneCount) const
{
    SvTreeListEntry* pClonedEntry = CloneEntry(rEntry);
    nCloneCount = 1;
    if (!rEntry.m_Children.empty())
        // Clone the child entries.
        CloneChildren(pClonedEntry->m_Children, nCloneCount, rEntry.m_Children, *pClonedEntry);

    return pClonedEntry;
}

void SvTreeList::CloneChildren(
        SvTreeListEntries& rDst, sal_uInt32& rCloneCount, SvTreeListEntries& rSrc, SvTreeListEntry& rNewParent) const
{
    SvTreeListEntries aClone;
    for (auto const& elem : rSrc)
    {
        SvTreeListEntry& rEntry = *elem;
        std::unique_ptr<SvTreeListEntry> pNewEntry(CloneEntry(rEntry));
        ++rCloneCount;
        pNewEntry->pParent = &rNewParent;
        if (!rEntry.m_Children.empty())
            // Clone entries recursively.
            CloneChildren(pNewEntry->m_Children, rCloneCount, rEntry.m_Children, *pNewEntry);

        aClone.push_back(std::move(pNewEntry));
    }

    rDst.swap(aClone);
}

sal_uInt32 SvTreeList::GetChildCount( const SvTreeListEntry* pParent ) const
{
    if ( !pParent )
        return GetEntryCount();

    if (pParent->m_Children.empty())
        return 0;

    sal_uInt32 nCount = 0;
    sal_uInt16 nRefDepth = GetDepth( pParent );
    sal_uInt16 nActDepth = nRefDepth;
    do
    {
        pParent = Next(const_cast<SvTreeListEntry*>(pParent), &nActDepth);
        nCount++;
    } while( pParent && nRefDepth < nActDepth );

    assert(nCount > 0 && "given do...while");
    return nCount - 1;
}

sal_uInt32 SvTreeList::GetVisibleChildCount(const SvTreeListBox& rView,
                                            SvTreeListEntry* pParent) const
{
    if ( !pParent )
        pParent = m_pRootItem.get();

    if (!pParent || !rView.IsExpanded(pParent) || pParent->m_Children.empty())
        return 0;

    sal_uInt32 nCount = 0;
    sal_uInt16 nRefDepth = GetDepth( pParent );
    sal_uInt16 nActDepth = nRefDepth;
    do
    {
        pParent = NextVisible(rView, pParent, &nActDepth);
        nCount++;
    } while( pParent && nRefDepth < nActDepth );

    assert(nCount > 0 && "given do...while");
    return nCount - 1;
}

sal_uInt32 SvTreeList::GetChildSelectionCount(const SvTreeListBox& rView,
                                              SvTreeListEntry* pParent) const
{
    if ( !pParent )
        pParent = m_pRootItem.get();

    if (!pParent || pParent->m_Children.empty())
        return 0;

    sal_uInt32 nCount = 0;
    sal_uInt16 nRefDepth = GetDepth( pParent );
    sal_uInt16 nActDepth = nRefDepth;
    do
    {
        pParent = Next( pParent, &nActDepth );
        if (pParent && rView.IsSelected(pParent) && nRefDepth < nActDepth)
            nCount++;
    } while( pParent && nRefDepth < nActDepth );

    return nCount;
}

SvTreeListEntry* SvTreeList::First() const
{
    if (m_nEntryCount)
        return m_pRootItem->m_Children[0].get();
    else
        return nullptr;
}

SvTreeListEntry* SvTreeList::Next( SvTreeListEntry* pActEntry, sal_uInt16* pDepth ) const
{
    DBG_ASSERT( pActEntry && pActEntry->pParent, "SvTreeList::Next: invalid entry/parent!" );
    if ( !pActEntry || !pActEntry->pParent )
        return nullptr;

    sal_uInt16 nDepth = 0;
    bool bWithDepth = false;
    if ( pDepth )
    {
        nDepth = *pDepth;
        bWithDepth = true;
    }

    if (!pActEntry->m_Children.empty())
    {
        // The current entry has children. Get its first child entry.
        nDepth++;
        pActEntry = pActEntry->m_Children[0].get();
        if ( bWithDepth )
            *pDepth = nDepth;
        return pActEntry;
    }

    // Get the list where the current entry belongs to (from its parent).
    SvTreeListEntries* pActualList = &pActEntry->pParent->m_Children;
    sal_uInt32 nActualPos = pActEntry->GetChildListPos();
    if (pActualList->size() > (nActualPos+1))
    {
        // Get the next sibling of the current entry.
        pActEntry = (*pActualList)[nActualPos+1].get();
        if ( bWithDepth )
            *pDepth = nDepth;
        return pActEntry;
    }

    // Move up level(s) until we find the level where the next sibling exists.
    SvTreeListEntry* pParent = pActEntry->pParent;
    nDepth--;
    while (pParent != m_pRootItem.get() && pParent != nullptr)
    {
        pActualList = &pParent->pParent->m_Children;
        nActualPos = pParent->GetChildListPos();
        if (pActualList->size() > (nActualPos+1))
        {
            pActEntry = (*pActualList)[nActualPos+1].get();
            if ( bWithDepth )
                *pDepth = nDepth;
            return pActEntry;
        }
        pParent = pParent->pParent;
        nDepth--;
    }
    return nullptr;
}

SvTreeListEntry* SvTreeList::Last() const
{
    SvTreeListEntries* pActList = &m_pRootItem->m_Children;
    SvTreeListEntry* pEntry = nullptr;
    while (!pActList->empty())
    {
        pEntry = pActList->back().get();
        pActList = &pEntry->m_Children;
    }
    return pEntry;
}

sal_uInt32 SvTreeList::GetVisiblePos(const SvTreeListBox& rView,
                                     SvTreeListEntry const* pEntry) const
{
    DBG_ASSERT(pEntry,"Entry?");

    if (!rView.m_bVisPositionsValid)
    {
        // to make GetVisibleCount refresh the positions
        const_cast<SvTreeListBox&>(rView).m_nVisibleCount = 0;
        GetVisibleCount(const_cast<SvTreeListBox&>(rView));
    }
    const SvViewDataEntry* pViewData = rView.GetViewData(pEntry);
    if (!pViewData)
        return 0;
    return pViewData->nVisPos;
}

sal_uInt32 SvTreeList::GetVisibleCount(SvTreeListBox& rView) const
{
    if (!rView.HasViewData())
        return 0;
    if (rView.m_nVisibleCount)
        return rView.m_nVisibleCount;

    sal_uInt32 nPos = 0;
    SvTreeListEntry* pEntry = First();  // first entry is always visible
    while ( pEntry )
    {
        if (SvViewDataEntry* pViewData = rView.GetViewData(pEntry))
            pViewData->nVisPos = nPos;
        nPos++;
        pEntry = NextVisible(rView, pEntry);
    }
#ifdef DBG_UTIL
    if( nPos > 10000000 )
    {
        OSL_FAIL("nVisibleCount bad");
    }
#endif
    rView.m_nVisibleCount = nPos;
    rView.m_bVisPositionsValid = true;
    return nPos;
}


// For performance reasons, this function assumes that the passed entry is
// already visible.
SvTreeListEntry* SvTreeList::NextVisible(const SvTreeListBox& rView, SvTreeListEntry* pActEntry,
                                         sal_uInt16* pActDepth) const
{
    if ( !pActEntry )
        return nullptr;

    sal_uInt16 nDepth = 0;
    bool bWithDepth = false;
    if ( pActDepth )
    {
        nDepth = *pActDepth;
        bWithDepth = true;
    }

    SvTreeListEntries* pActualList = &pActEntry->pParent->m_Children;
    sal_uInt32 nActualPos = pActEntry->GetChildListPos();

    if (rView.IsExpanded(pActEntry))
    {
        OSL_ENSURE(!pActEntry->m_Children.empty(), "Pass entry is supposed to have child entries.");

        nDepth++;
        pActEntry = pActEntry->m_Children[0].get();
        if ( bWithDepth )
            *pActDepth = nDepth;
        return pActEntry;
    }

    nActualPos++;
    if ( pActualList->size() > nActualPos  )
    {
        pActEntry = (*pActualList)[nActualPos].get();
        if ( bWithDepth )
            *pActDepth = nDepth;
        return pActEntry;
    }

    SvTreeListEntry* pParent = pActEntry->pParent;
    nDepth--;
    while (pParent != m_pRootItem.get())
    {
        pActualList = &pParent->pParent->m_Children;
        nActualPos = pParent->GetChildListPos();
        nActualPos++;
        if ( pActualList->size() > nActualPos )
        {
            pActEntry = (*pActualList)[nActualPos].get();
            if ( bWithDepth )
                *pActDepth = nDepth;
            return pActEntry;
        }
        pParent = pParent->pParent;
        nDepth--;
    }
    return nullptr;
}


// For performance reasons, this function assumes that the passed entry is
// already visible.

SvTreeListEntry* SvTreeList::PrevVisible(const SvTreeListBox& rView,
                                         SvTreeListEntry* pActEntry) const
{
    assert(pActEntry && "PrevVis: Entry?");

    SvTreeListEntries* pActualList = &pActEntry->pParent->m_Children;
    sal_uInt32 nActualPos = pActEntry->GetChildListPos();

    if ( nActualPos > 0 )
    {
        pActEntry = (*pActualList)[nActualPos-1].get();
        while (rView.IsExpanded(pActEntry))
        {
            pActualList = &pActEntry->m_Children;
            pActEntry = pActualList->back().get();
        }
        return pActEntry;
    }

    if (pActEntry->pParent == m_pRootItem.get())
        return nullptr;

    pActEntry = pActEntry->pParent;
    if ( pActEntry )
    {
        return pActEntry;
    }
    return nullptr;
}

SvTreeListEntry* SvTreeList::LastVisible(const SvTreeListBox& rView) const
{
    SvTreeListEntry* pEntry = Last();
    while (pEntry && !IsEntryVisible(rView, pEntry))
        pEntry = PrevVisible(rView, pEntry);
    return pEntry;
}

SvTreeListEntry* SvTreeList::NextVisible(const SvTreeListBox& rView, SvTreeListEntry* pEntry,
                                         sal_uInt16& nDelta) const
{
    DBG_ASSERT(IsEntryVisible(rView, pEntry), "NextVis:Wrong Vis");

    sal_uInt32 nVisPos = GetVisiblePos(rView, pEntry);
    // nDelta entries existent?
    // example: 0,1,2,3,4,5,6,7,8,9 nVisPos=5 nDelta=7
    //           nNewDelta = 10-nVisPos-1 == 4
    if (nVisPos + nDelta >= rView.m_nVisibleCount)
    {
        nDelta = static_cast<sal_uInt16>(rView.m_nVisibleCount - nVisPos);
        nDelta--;
    }
    sal_uInt16 nDeltaTmp = nDelta;
    while( nDeltaTmp )
    {
        pEntry = NextVisible(rView, pEntry);
        nDeltaTmp--;
        DBG_ASSERT(pEntry,"Entry?");
    }
    return pEntry;
}

SvTreeListEntry* SvTreeList::PrevVisible(const SvTreeListBox& rView, SvTreeListEntry* pEntry,
                                         sal_uInt16& nDelta) const
{
    DBG_ASSERT(pEntry && IsEntryVisible(rView, pEntry), "PrevVis:Parms/!Vis");

    sal_uInt32 nVisPos = GetVisiblePos(rView, pEntry);
    // nDelta entries existent?
    // example: 0,1,2,3,4,5,6,7,8,9 nVisPos=8 nDelta=20
    //           nNewDelta = nNewVisPos
    if (  nDelta > nVisPos )
        nDelta = static_cast<sal_uInt16>(nVisPos);
    sal_uInt16 nDeltaTmp = nDelta;
    while( nDeltaTmp )
    {
        pEntry = PrevVisible(rView, pEntry);
        nDeltaTmp--;
        DBG_ASSERT(pEntry,"Entry?");
    }
    return pEntry;
}

SvTreeListEntry* SvTreeList::FirstSelected(const SvTreeListBox& rView) const
{
    SvTreeListEntry* pActSelEntry = First();
    while (pActSelEntry && !rView.IsSelected(pActSelEntry))
        pActSelEntry = NextVisible(rView, pActSelEntry);
    return pActSelEntry;
}

SvTreeListEntry* SvTreeList::FirstChild(const SvTreeListEntry* pParent) const
{
    if ( !pParent )
        pParent = m_pRootItem.get();
    SvTreeListEntry* pResult;
    if (!pParent->m_Children.empty())
        pResult = pParent->m_Children[0].get();
    else
        pResult = nullptr;
    return pResult;
}

SvTreeListEntry* SvTreeList::NextSelected(const SvTreeListBox& rView, SvTreeListEntry* pEntry) const
{
    assert(pEntry && "NextSelected: Entry?");
    pEntry = Next( pEntry );
    while (pEntry && !rView.IsSelected(pEntry))
        pEntry = Next( pEntry );
    return pEntry;
}

void SvTreeList::Insert(SvTreeListEntry* pEntry, SvTreeListEntry* pParent, sal_uInt32 nPos)
{
    assert(pEntry && "Entry?");

    if ( !pParent )
        pParent = m_pRootItem.get();

    SvTreeListEntries& rList = pParent->m_Children;

    // take sorting into account
    GetInsertionPos(*pEntry, pParent, nPos);

    m_bAbsPositionsValid = false;
    pEntry->pParent = pParent;

    if (nPos < rList.size())
    {
        SvTreeListEntries::iterator itPos = rList.begin();
        std::advance(itPos, nPos);
        rList.insert(itPos, std::unique_ptr<SvTreeListEntry>(pEntry));
    }
    else
        rList.push_back(std::unique_ptr<SvTreeListEntry>(pEntry));

    m_nEntryCount++;
    if (nPos != TREELIST_APPEND && (nPos != (rList.size()-1)))
        SetListPositions(rList);
    else
        pEntry->nListPos = rList.size()-1;

    Broadcast(SvListAction::INSERTED, pEntry);
}

sal_uInt32 SvTreeList::GetAbsPos( const SvTreeListEntry* pEntry) const
{
    if (!m_bAbsPositionsValid)
        const_cast<SvTreeList*>(this)->SetAbsolutePositions();
    return pEntry->nAbsPos;
}

sal_uInt32 SvTreeList::GetRelPos( const SvTreeListEntry* pChild )
{
    return pChild->GetChildListPos();
}

void SvTreeList::SetAbsolutePositions()
{
    sal_uInt32 nPos = 0;
    SvTreeListEntry* pEntry = First();
    while ( pEntry )
    {
        pEntry->nAbsPos = nPos;
        nPos++;
        pEntry = Next( pEntry );
    }
    m_bAbsPositionsValid = true;
}

bool SvTreeList::Remove( const SvTreeListEntry* pEntry )
{
    assert(pEntry && "Cannot remove root, use clear");

    if( !pEntry->pParent )
    {
        OSL_FAIL("Removing entry not in model!");
        // Under certain circumstances (which?), the explorer deletes entries
        // from the view that it hasn't inserted into the view. We don't want
        // to crash, so we catch this case here.
        return false;
    }

    Broadcast(SvListAction::REMOVING, const_cast<SvTreeListEntry*>(pEntry));
    sal_uInt32 nRemoved = 1 + GetChildCount(pEntry);
    m_bAbsPositionsValid = false;

    SvTreeListEntry* pParent = pEntry->pParent;
    SvTreeListEntries& rList = pParent->m_Children;
    bool bLastEntry = false;

    // Since we need the live instance of SvTreeListEntry for broadcasting,
    // we first need to pop it from the container, broadcast it, then delete
    // the instance manually at the end.

    std::unique_ptr<SvTreeListEntry> pEntryDeleter;
    if ( pEntry->HasChildListPos() )
    {
        size_t nListPos = pEntry->GetChildListPos();
        bLastEntry = (nListPos == (rList.size()-1));
        SvTreeListEntries::iterator it = rList.begin();
        std::advance(it, nListPos);
        pEntryDeleter = std::move(*it);
        rList.erase(it);
    }
    else
    {
        SvTreeListEntries::iterator it =
            std::find_if(rList.begin(), rList.end(), FindByPointer(pEntry));
        if (it != rList.end())
        {
            pEntryDeleter = std::move(*it);
            rList.erase(it);
        }
    }

    if (!rList.empty() && !bLastEntry)
        SetListPositions(rList);

    m_nEntryCount -= nRemoved;
    Broadcast(SvListAction::REMOVED, const_cast<SvTreeListEntry*>(pEntry));

    return true;
}

SvTreeListEntry* SvTreeList::GetEntryAtAbsPos( sal_uInt32 nAbsPos ) const
{
    SvTreeListEntry* pEntry = First();
    while ( nAbsPos && pEntry )
    {
        pEntry = Next( pEntry );
        nAbsPos--;
    }
    return pEntry;
}

SvTreeListEntry* SvTreeList::GetEntryAtVisPos(const SvTreeListBox& rView, sal_uInt32 nVisPos) const
{
    SvTreeListEntry* pEntry = First();
    while ( nVisPos && pEntry )
    {
        pEntry = NextVisible(rView, pEntry);
        nVisPos--;
    }
    return pEntry;
}

void SvTreeList::SetListPositions( SvTreeListEntries& rEntries )
{
    if (rEntries.empty())
        return;

    SvTreeListEntry& rFirst = *rEntries.front();
    if (rFirst.pParent)
        rFirst.pParent->InvalidateChildrensListPositions();
}

void SvTreeList::EnableInvalidate( bool bEnable )
{
    mbEnableInvalidate = bEnable;
}

void SvTreeList::InvalidateEntry(SvTreeListEntry& rEntry)
{
    if (!mbEnableInvalidate)
        return;

    Broadcast(SvListAction::INVALIDATE_ENTRY, &rEntry);
}

sal_Int32 SvTreeList::Compare(const SvTreeListEntry* pLeft, const SvTreeListEntry* pRight) const
{
    if (m_aCompareLink.IsSet())
    {
        SvSortData aSortData;
        aSortData.pLeft = pLeft;
        aSortData.pRight = pRight;
        return m_aCompareLink.Call(aSortData);
    }
    return 0;
}

void SvTreeList::Resort()
{
    Broadcast( SvListAction::RESORTING );
    m_bAbsPositionsValid = false;
    ResortChildren(m_pRootItem.get());
    Broadcast( SvListAction::RESORTED );
}

namespace {

class SortComparator
{
    SvTreeList& mrList;
public:

    explicit SortComparator( SvTreeList& rList ) : mrList(rList) {}

    bool operator() (std::unique_ptr<SvTreeListEntry> const& rpLeft,
                     std::unique_ptr<SvTreeListEntry> const& rpRight) const
    {
        int nCompare = mrList.Compare(rpLeft.get(), rpRight.get());
        if (nCompare != 0 && mrList.GetSortMode() == SvSortMode::Descending)
        {
            if( nCompare < 0 )
                nCompare = 1;
            else
                nCompare = -1;
        }
        return nCompare < 0;
    }
};

}

void SvTreeList::ResortChildren( SvTreeListEntry* pParent )
{
    assert(pParent && "Parent not set");

    if (pParent->m_Children.empty())
        return;

    SortComparator aComp(*this);
    std::sort(pParent->m_Children.begin(), pParent->m_Children.end(), aComp);

    // Recursively sort child entries.
    for (auto const& it : pParent->m_Children)
    {
        SvTreeListEntry& r = *it;
        ResortChildren(&r);
    }

    SetListPositions(pParent->m_Children); // correct list position in target list
}

void SvTreeList::GetInsertionPos(const SvTreeListEntry& rEntry, SvTreeListEntry* pParent,
    sal_uInt32& rPos )
{
    if (m_eSortMode == SvSortMode::None)
        return;

    rPos = TREELIST_ENTRY_NOTFOUND;
    const SvTreeListEntries& rChildList = GetChildList(pParent);

    if (rChildList.empty())
        return;

    tools::Long i = 0;
    tools::Long j = rChildList.size()-1;
    tools::Long k;
    sal_Int32 nCompare = 1;

    do
    {
        k = (i+j)/2;
        const SvTreeListEntry* pTempEntry = rChildList[k].get();
        nCompare = Compare(&rEntry, pTempEntry);
        if (nCompare != 0 && m_eSortMode == SvSortMode::Descending)
        {
            if( nCompare < 0 )
                nCompare = 1;
            else
                nCompare = -1;
        }
        if( nCompare > 0 )
            i = k + 1;
        else
            j = k - 1;
    } while( (nCompare != 0) && (i <= j) );

    if( nCompare != 0 )
    {
        if (i > static_cast<tools::Long>(rChildList.size()-1)) // not found, end of list
            rPos = TREELIST_ENTRY_NOTFOUND;
        else
            rPos = i;              // not found, middle of list
    }
    else
        rPos = k;
}

SvTreeListEntry* SvTreeList::GetEntry( SvTreeListEntry* pParent, sal_uInt32 nPos ) const
{   if ( !pParent )
        pParent = m_pRootItem.get();
    SvTreeListEntry* pRet = nullptr;
    if (nPos < pParent->m_Children.size())
        pRet = pParent->m_Children[nPos].get();
    return pRet;
}

SvTreeListEntry* SvTreeList::GetEntry( sal_uInt32 nRootPos ) const
{
    SvTreeListEntry* pRet = nullptr;
    if (m_nEntryCount && nRootPos < m_pRootItem->m_Children.size())
        pRet = m_pRootItem->m_Children[nRootPos].get();
    return pRet;
}

const SvTreeListEntries& SvTreeList::GetChildList( SvTreeListEntry* pParent ) const
{
    if ( !pParent )
        pParent = m_pRootItem.get();
    return pParent->m_Children;
}

SvTreeListEntries& SvTreeList::GetChildList( SvTreeListEntry* pParent )
{
    if ( !pParent )
        pParent = m_pRootItem.get();
    return pParent->m_Children;
}

const SvTreeListEntry* SvTreeList::GetParent( const SvTreeListEntry* pEntry ) const
{
    const SvTreeListEntry* pParent = pEntry->pParent;
    if (pParent == m_pRootItem.get())
        pParent = nullptr;
    return pParent;
}

SvTreeListEntry* SvTreeList::GetParent( SvTreeListEntry* pEntry )
{
    SvTreeListEntry* pParent = pEntry->pParent;
    if (pParent == m_pRootItem.get())
        pParent = nullptr;
    return pParent;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
