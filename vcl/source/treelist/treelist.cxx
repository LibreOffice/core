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

#include <vcl/treelist.hxx>
#include <vcl/treelistentry.hxx>
#include <vcl/viewdataentry.hxx>
#include <tools/debug.hxx>
#include <osl/diagnose.h>

#include <memory>
#include <map>


typedef std::map<SvTreeListEntry*, std::unique_ptr<SvViewDataEntry>> SvDataTable;

struct SvListView::Impl
{
    SvListView & m_rThis;

    SvDataTable m_DataTable;  // Mapping SvTreeListEntry -> ViewData

    sal_uLong   m_nVisibleCount;
    sal_uLong   m_nSelectionCount;
    bool        m_bVisPositionsValid;

    explicit Impl(SvListView & rThis)
        : m_rThis(rThis)
        , m_nVisibleCount(0)
        , m_nSelectionCount(0)
        , m_bVisPositionsValid(false)
    {}

    void InitTable();
    void RemoveViewData( SvTreeListEntry* pParent );

    void ActionMoving(SvTreeListEntry* pEntry);
    void ActionMoved();
    void ActionInserted(SvTreeListEntry* pEntry);
    void ActionInsertedTree(SvTreeListEntry* pEntry);
    void ActionRemoving(SvTreeListEntry* pEntry);
    void ActionClear();
};


SvTreeList::SvTreeList(SvListView& listView) :
    mrOwnerListView(listView),
    mbEnableInvalidate(true)
{
    nEntryCount = 0;
    bAbsPositionsValid = false;
    pRootItem.reset(new SvTreeListEntry);
    eSortMode = SortNone;
}

SvTreeList::~SvTreeList()
{
}

void SvTreeList::Broadcast(
    SvListAction nActionId,
    SvTreeListEntry* pEntry1,
    SvTreeListEntry* pEntry2,
    sal_uLong nPos
)
{
    mrOwnerListView.ModelNotification(nActionId, pEntry1, pEntry2, nPos);
}

// an entry is visible if all parents are expanded
bool SvTreeList::IsEntryVisible( const SvListView* pView, SvTreeListEntry* pEntry ) const
{
    DBG_ASSERT(pView&&pEntry,"IsVisible:Invalid Params");
    bool bRetVal = false;
    do
    {
        if ( pEntry == pRootItem.get() )
        {
            bRetVal = true;
            break;
        }
        pEntry = pEntry->pParent;
    }  while( pView->IsExpanded( pEntry ) );
    return bRetVal;
}

sal_uInt16 SvTreeList::GetDepth( const SvTreeListEntry* pEntry ) const
{
    DBG_ASSERT(pEntry && pEntry!=pRootItem.get(),"GetDepth:Bad Entry");
    sal_uInt16 nDepth = 0;
    while( pEntry->pParent != pRootItem.get() )
    {
        nDepth++;
        pEntry = pEntry->pParent;
    }
    return nDepth;
}

bool SvTreeList::IsAtRootDepth( const SvTreeListEntry* pEntry ) const
{
    return pEntry->pParent == pRootItem.get();
}

void SvTreeList::Clear()
{
    Broadcast( SvListAction::CLEARING );
    pRootItem->ClearChildren();
    nEntryCount = 0;
    Broadcast( SvListAction::CLEARED );
}

bool SvTreeList::IsChild(const SvTreeListEntry* pParent, const SvTreeListEntry* pChild) const
{
    if ( !pParent )
        pParent = pRootItem.get();

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

sal_uLong findEntryPosition(const SvTreeListEntries& rDst, const SvTreeListEntry* pEntry)
{
    SvTreeListEntries::const_iterator itPos = std::find_if(rDst.begin(), rDst.end(), FindByPointer(pEntry));
    if (itPos == rDst.end())
        return static_cast<sal_uLong>(~0);

    return static_cast<sal_uLong>(std::distance(rDst.begin(), itPos));
}

}

sal_uLong SvTreeList::Move(SvTreeListEntry* pSrcEntry,SvTreeListEntry* pTargetParent,sal_uLong nListPos)
{
    // pDest may be 0!
    DBG_ASSERT(pSrcEntry,"Entry?");
    if ( !pTargetParent )
        pTargetParent = pRootItem.get();
    DBG_ASSERT(pSrcEntry!=pTargetParent,"Move:Source=Target");

    Broadcast( SvListAction::MOVING, pSrcEntry, pTargetParent, nListPos );

    if ( pSrcEntry == pTargetParent )
        // You can't move an entry onto itself as the parent. Just return its
        // position and bail out.
        return pSrcEntry->GetChildListPos();

    bAbsPositionsValid = false;

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
        assert(pOriginal.get());
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
        assert(pOriginal.get());
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

    sal_uLong nRetVal = findEntryPosition(rDst, pSrcEntry);
    OSL_ENSURE(nRetVal == pSrcEntry->GetChildListPos(), "ListPos not valid");
    Broadcast( SvListAction::MOVED,pSrcEntry,pTargetParent,nRetVal);
    return nRetVal;
}

sal_uLong SvTreeList::Copy(SvTreeListEntry* pSrcEntry,SvTreeListEntry* pTargetParent,sal_uLong nListPos)
{
    // pDest may be 0!
    DBG_ASSERT(pSrcEntry,"Entry?");
    if ( !pTargetParent )
        pTargetParent = pRootItem.get();

    bAbsPositionsValid = false;

    sal_uLong nCloneCount = 0;
    SvTreeListEntry* pClonedEntry = Clone( pSrcEntry, nCloneCount );
    nEntryCount += nCloneCount;

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
    sal_uLong nRetVal = findEntryPosition(rDst, pClonedEntry);
    return nRetVal;
}

void SvTreeList::Move( SvTreeListEntry* pSrcEntry, SvTreeListEntry* pDstEntry )
{
    SvTreeListEntry* pParent;
    sal_uLong nPos;

    if ( !pDstEntry )
    {
        pParent = pRootItem.get();
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
    SvTreeListEntry* pTargetParent,sal_uLong nListPos)
{
    DBG_ASSERT(pSrcEntry,"InsertTree:Entry?");
    if ( !pSrcEntry )
        return;

    if ( !pTargetParent )
        pTargetParent = pRootItem.get();

    // take sorting into account
    GetInsertionPos( pSrcEntry, pTargetParent, nListPos );

    bAbsPositionsValid = false;

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
    nEntryCount += GetChildCount( pSrcEntry );
    nEntryCount++; // the parent is new, too

    Broadcast(SvListAction::INSERTED_TREE, pSrcEntry );
}

SvTreeListEntry* SvTreeList::CloneEntry( SvTreeListEntry* pSource ) const
{
    if( aCloneLink.IsSet() )
        return aCloneLink.Call( pSource );
    SvTreeListEntry* pEntry = new SvTreeListEntry;
    pEntry->Clone(pSource);
    return pEntry;
}

SvTreeListEntry* SvTreeList::Clone( SvTreeListEntry* pEntry, sal_uLong& nCloneCount ) const
{
    SvTreeListEntry* pClonedEntry = CloneEntry( pEntry );
    nCloneCount = 1;
    if (!pEntry->m_Children.empty())
        // Clone the child entries.
        CloneChildren(pClonedEntry->m_Children, nCloneCount, pEntry->m_Children, *pClonedEntry);

    return pClonedEntry;
}

void SvTreeList::CloneChildren(
        SvTreeListEntries& rDst, sal_uLong& rCloneCount, SvTreeListEntries& rSrc, SvTreeListEntry& rNewParent) const
{
    SvTreeListEntries aClone;
    for (auto const& elem : rSrc)
    {
        SvTreeListEntry& rEntry = *elem;
        std::unique_ptr<SvTreeListEntry> pNewEntry(CloneEntry(&rEntry));
        ++rCloneCount;
        pNewEntry->pParent = &rNewParent;
        if (!rEntry.m_Children.empty())
            // Clone entries recursively.
            CloneChildren(pNewEntry->m_Children, rCloneCount, rEntry.m_Children, *pNewEntry);

        aClone.push_back(std::move(pNewEntry));
    }

    rDst.swap(aClone);
}

sal_uLong SvTreeList::GetChildCount( const SvTreeListEntry* pParent ) const
{
    if ( !pParent )
        return GetEntryCount();

    if (pParent->m_Children.empty())
        return 0;

    sal_uLong nCount = 0;
    sal_uInt16 nRefDepth = GetDepth( pParent );
    sal_uInt16 nActDepth = nRefDepth;
    do
    {
        pParent = Next(const_cast<SvTreeListEntry*>(pParent), &nActDepth);
        nCount++;
    } while( pParent && nRefDepth < nActDepth );
    nCount--;
    return nCount;
}

sal_uLong SvTreeList::GetVisibleChildCount(const SvListView* pView, SvTreeListEntry* pParent) const
{
    DBG_ASSERT(pView,"GetVisChildCount:No View");
    if ( !pParent )
        pParent = pRootItem.get();

    if (!pParent || !pView->IsExpanded(pParent) || pParent->m_Children.empty())
        return 0;

    sal_uLong nCount = 0;
    sal_uInt16 nRefDepth = GetDepth( pParent );
    sal_uInt16 nActDepth = nRefDepth;
    do
    {
        pParent = NextVisible( pView, pParent, &nActDepth );
        nCount++;
    } while( pParent && nRefDepth < nActDepth );
    nCount--;
    return nCount;
}

sal_uLong SvTreeList::GetChildSelectionCount(const SvListView* pView,SvTreeListEntry* pParent) const
{
    DBG_ASSERT(pView,"GetChildSelCount:No View");
    if ( !pParent )
        pParent = pRootItem.get();

    if (!pParent || pParent->m_Children.empty())
        return 0;

    sal_uLong nCount = 0;
    sal_uInt16 nRefDepth = GetDepth( pParent );
    sal_uInt16 nActDepth = nRefDepth;
    do
    {
        pParent = Next( pParent, &nActDepth );
        if( pParent && pView->IsSelected( pParent ) && nRefDepth < nActDepth)
            nCount++;
    } while( pParent && nRefDepth < nActDepth );
//  nCount--;
    return nCount;
}

SvTreeListEntry* SvTreeList::First() const
{
    if ( nEntryCount )
        return pRootItem->m_Children[0].get();
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

    // Get the list where the current entry belongs to (from its parent).
    SvTreeListEntries* pActualList = &pActEntry->pParent->m_Children;
    sal_uLong nActualPos = pActEntry->GetChildListPos();

    if (!pActEntry->m_Children.empty())
    {
        // The current entry has children. Get its first child entry.
        nDepth++;
        pActEntry = pActEntry->m_Children[0].get();
        if ( bWithDepth )
            *pDepth = nDepth;
        return pActEntry;
    }

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
    while( pParent != pRootItem.get() && pParent != nullptr )
    {
        DBG_ASSERT(pParent!=nullptr,"TreeData corrupt!");
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

SvTreeListEntry* SvTreeList::Prev( SvTreeListEntry* pActEntry ) const
{
    DBG_ASSERT(pActEntry!=nullptr,"Entry?");

    sal_uInt16 nDepth = 0;

    SvTreeListEntries* pActualList = &pActEntry->pParent->m_Children;
    sal_uLong nActualPos = pActEntry->GetChildListPos();

    if ( nActualPos > 0 )
    {
        pActEntry = (*pActualList)[nActualPos-1].get();
        while (!pActEntry->m_Children.empty())
        {
            pActualList = &pActEntry->m_Children;
            nDepth++;
            pActEntry = pActualList->back().get();
        }
        return pActEntry;
    }
    if ( pActEntry->pParent == pRootItem.get() )
        return nullptr;

    pActEntry = pActEntry->pParent;

    if ( pActEntry )
    {
        nDepth--;
        return pActEntry;
    }
    return nullptr;
}

SvTreeListEntry* SvTreeList::Last() const
{
    SvTreeListEntries* pActList = &pRootItem->m_Children;
    SvTreeListEntry* pEntry = nullptr;
    while (!pActList->empty())
    {
        pEntry = pActList->back().get();
        pActList = &pEntry->m_Children;
    }
    return pEntry;
}

sal_uLong SvTreeList::GetVisiblePos( const SvListView* pView, SvTreeListEntry const * pEntry ) const
{
    DBG_ASSERT(pView&&pEntry,"View/Entry?");

    if (!pView->m_pImpl->m_bVisPositionsValid)
    {
        // to make GetVisibleCount refresh the positions
        const_cast<SvListView*>(pView)->m_pImpl->m_nVisibleCount = 0;
        GetVisibleCount( const_cast<SvListView*>(pView) );
    }
    const SvViewDataEntry* pViewData = pView->GetViewData( pEntry );
    return pViewData->nVisPos;
}

sal_uLong SvTreeList::GetVisibleCount( SvListView* pView ) const
{
    assert(pView && "GetVisCount:No View");
    if( !pView->HasViewData() )
        return 0;
    if (pView->m_pImpl->m_nVisibleCount)
        return pView->m_pImpl->m_nVisibleCount;

    sal_uLong nPos = 0;
    SvTreeListEntry* pEntry = First();  // first entry is always visible
    while ( pEntry )
    {
        SvViewDataEntry* pViewData = pView->GetViewData( pEntry );
        pViewData->nVisPos = nPos;
        nPos++;
        pEntry = NextVisible( pView, pEntry );
    }
#ifdef DBG_UTIL
    if( nPos > 10000000 )
    {
        OSL_FAIL("nVisibleCount bad");
    }
#endif
    pView->m_pImpl->m_nVisibleCount = nPos;
    pView->m_pImpl->m_bVisPositionsValid = true;
    return nPos;
}


// For performance reasons, this function assumes that the passed entry is
// already visible.
SvTreeListEntry* SvTreeList::NextVisible(const SvListView* pView,SvTreeListEntry* pActEntry,sal_uInt16* pActDepth) const
{
    DBG_ASSERT(pView,"NextVisible:No View");
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
    sal_uLong nActualPos = pActEntry->GetChildListPos();

    if ( pView->IsExpanded(pActEntry) )
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
    while( pParent != pRootItem.get() )
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

SvTreeListEntry* SvTreeList::PrevVisible(const SvListView* pView, SvTreeListEntry* pActEntry) const
{
    DBG_ASSERT(pView&&pActEntry,"PrevVis:View/Entry?");

    sal_uInt16 nDepth = 0;

    SvTreeListEntries* pActualList = &pActEntry->pParent->m_Children;
    sal_uLong nActualPos = pActEntry->GetChildListPos();

    if ( nActualPos > 0 )
    {
        pActEntry = (*pActualList)[nActualPos-1].get();
        while( pView->IsExpanded(pActEntry) )
        {
            pActualList = &pActEntry->m_Children;
            nDepth++;
            pActEntry = pActualList->back().get();
        }
        return pActEntry;
    }

    if ( pActEntry->pParent == pRootItem.get() )
        return nullptr;

    pActEntry = pActEntry->pParent;
    if ( pActEntry )
    {
        nDepth--;
        return pActEntry;
    }
    return nullptr;
}

SvTreeListEntry* SvTreeList::LastVisible( const SvListView* pView) const
{
    DBG_ASSERT(pView,"LastVis:No View");
    SvTreeListEntry* pEntry = Last();
    while( pEntry && !IsEntryVisible( pView, pEntry ) )
        pEntry = PrevVisible( pView, pEntry );
    return pEntry;
}

SvTreeListEntry* SvTreeList::NextVisible(const SvListView* pView,SvTreeListEntry* pEntry,sal_uInt16& nDelta) const
{
    DBG_ASSERT(pView&&pEntry&&IsEntryVisible(pView,pEntry),"NextVis:Wrong Prms/!Vis");

    sal_uLong nVisPos = GetVisiblePos( pView, pEntry );
    // nDelta entries existent?
    // example: 0,1,2,3,4,5,6,7,8,9 nVisPos=5 nDelta=7
    //           nNewDelta = 10-nVisPos-1 == 4
    if (nVisPos+nDelta >= pView->m_pImpl->m_nVisibleCount)
    {
        nDelta = static_cast<sal_uInt16>(pView->m_pImpl->m_nVisibleCount-nVisPos);
        nDelta--;
    }
    sal_uInt16 nDeltaTmp = nDelta;
    while( nDeltaTmp )
    {
        pEntry = NextVisible( pView, pEntry );
        nDeltaTmp--;
        DBG_ASSERT(pEntry,"Entry?");
    }
    return pEntry;
}

SvTreeListEntry* SvTreeList::PrevVisible( const SvListView* pView, SvTreeListEntry* pEntry, sal_uInt16& nDelta ) const
{
    DBG_ASSERT(pView&&pEntry&&IsEntryVisible(pView,pEntry),"PrevVis:Parms/!Vis");

    sal_uLong nVisPos = GetVisiblePos( pView, pEntry );
    // nDelta entries existent?
    // example: 0,1,2,3,4,5,6,7,8,9 nVisPos=8 nDelta=20
    //           nNewDelta = nNewVisPos
    if (  nDelta > nVisPos )
        nDelta = static_cast<sal_uInt16>(nVisPos);
    sal_uInt16 nDeltaTmp = nDelta;
    while( nDeltaTmp )
    {
        pEntry = PrevVisible( pView, pEntry );
        nDeltaTmp--;
        DBG_ASSERT(pEntry,"Entry?");
    }
    return pEntry;
}

SvTreeListEntry* SvTreeList::FirstSelected( const SvListView* pView) const
{
    DBG_ASSERT(pView,"FirstSel:No View");
    if( !pView )
        return nullptr;
    SvTreeListEntry* pActSelEntry = First();
    while( pActSelEntry && !pView->IsSelected(pActSelEntry) )
        pActSelEntry = NextVisible( pView, pActSelEntry );
    return pActSelEntry;
}


SvTreeListEntry* SvTreeList::FirstChild( SvTreeListEntry* pParent ) const
{
    if ( !pParent )
        pParent = pRootItem.get();
    SvTreeListEntry* pResult;
    if (!pParent->m_Children.empty())
        pResult = pParent->m_Children[0].get();
    else
        pResult = nullptr;
    return pResult;
}

SvTreeListEntry* SvTreeList::NextSelected( const SvListView* pView, SvTreeListEntry* pEntry ) const
{
    DBG_ASSERT(pView&&pEntry,"NextSel:View/Entry?");
    pEntry = Next( pEntry );
    while( pEntry && !pView->IsSelected(pEntry) )
        pEntry = Next( pEntry );
    return pEntry;
}

SvTreeListEntry* SvTreeList::PrevSelected( const SvListView* pView, SvTreeListEntry* pEntry) const
{
    DBG_ASSERT(pView&&pEntry,"PrevSel:View/Entry?");
    pEntry = Prev( pEntry );
    while( pEntry && !pView->IsSelected(pEntry) )
        pEntry = Prev( pEntry );

    return pEntry;
}

SvTreeListEntry* SvTreeList::LastSelected( const SvListView* pView ) const
{
    DBG_ASSERT(pView,"LastSel:No View");
    SvTreeListEntry* pEntry = Last();
    while( pEntry && !pView->IsSelected(pEntry) )
        pEntry = Prev( pEntry );
    return pEntry;
}

sal_uLong SvTreeList::Insert( SvTreeListEntry* pEntry,SvTreeListEntry* pParent,sal_uLong nPos )
{
    DBG_ASSERT( pEntry,"Entry?");

    if ( !pParent )
        pParent = pRootItem.get();

    SvTreeListEntries& rList = pParent->m_Children;

    // take sorting into account
    GetInsertionPos( pEntry, pParent, nPos );

    bAbsPositionsValid = false;
    pEntry->pParent = pParent;

    if (nPos < rList.size())
    {
        SvTreeListEntries::iterator itPos = rList.begin();
        std::advance(itPos, nPos);
        rList.insert(itPos, std::unique_ptr<SvTreeListEntry>(pEntry));
    }
    else
        rList.push_back(std::unique_ptr<SvTreeListEntry>(pEntry));

    nEntryCount++;
    if (nPos != TREELIST_APPEND && (nPos != (rList.size()-1)))
        SetListPositions(rList);
    else
        pEntry->nListPos = rList.size()-1;

    Broadcast( SvListAction::INSERTED, pEntry );
    return nPos; // pEntry->nListPos;
}

sal_uLong SvTreeList::GetAbsPos( const SvTreeListEntry* pEntry) const
{
    if ( !bAbsPositionsValid )
        const_cast<SvTreeList*>(this)->SetAbsolutePositions();
    return pEntry->nAbsPos;
}

sal_uLong SvTreeList::GetRelPos( const SvTreeListEntry* pChild )
{
    return pChild->GetChildListPos();
}

void SvTreeList::SetAbsolutePositions()
{
    sal_uLong nPos = 0;
    SvTreeListEntry* pEntry = First();
    while ( pEntry )
    {
        pEntry->nAbsPos = nPos;
        nPos++;
        pEntry = Next( pEntry );
    }
    bAbsPositionsValid = true;
}

void SvListView::ExpandListEntry( SvTreeListEntry* pEntry )
{
    DBG_ASSERT(pEntry,"Expand:View/Entry?");
    if ( IsExpanded(pEntry) )
        return;

    DBG_ASSERT(!pEntry->m_Children.empty(), "SvTreeList::Expand: We expected to have child entries.");

    SvViewDataEntry* pViewData = GetViewData(pEntry);
    pViewData->SetExpanded(true);
    SvTreeListEntry* pParent = pEntry->pParent;
    // if parent is visible, invalidate status data
    if ( IsExpanded( pParent ) )
    {
        m_pImpl->m_bVisPositionsValid = false;
        m_pImpl->m_nVisibleCount = 0;
    }
}

void SvListView::CollapseListEntry( SvTreeListEntry* pEntry )
{
    DBG_ASSERT(pEntry,"Collapse:View/Entry?");
    if ( !IsExpanded(pEntry) )
        return;

    DBG_ASSERT(!pEntry->m_Children.empty(), "SvTreeList::Collapse: We expected to have child entries.");

    SvViewDataEntry* pViewData = GetViewData( pEntry );
    pViewData->SetExpanded(false);

    SvTreeListEntry* pParent = pEntry->pParent;
    if ( IsExpanded(pParent) )
    {
        m_pImpl->m_nVisibleCount = 0;
        m_pImpl->m_bVisPositionsValid = false;
    }
}

bool SvListView::SelectListEntry( SvTreeListEntry* pEntry, bool bSelect )
{
    DBG_ASSERT(pEntry,"Select:View/Entry?");
    SvViewDataEntry* pViewData = GetViewData( pEntry );
    if ( bSelect )
    {
        if ( pViewData->IsSelected() || !pViewData->IsSelectable() )
            return false;
        else
        {
            pViewData->SetSelected(true);
            m_pImpl->m_nSelectionCount++;
        }
    }
    else
    {
        if ( !pViewData->IsSelected() )
            return false;
        else
        {
            pViewData->SetSelected(false);
            m_pImpl->m_nSelectionCount--;
        }
    }
    return true;
}

bool SvTreeList::Remove( const SvTreeListEntry* pEntry )
{
    DBG_ASSERT(pEntry,"Cannot remove root, use clear");

    if( !pEntry->pParent )
    {
        OSL_FAIL("Removing entry not in model!");
        // Under certain circumstances (which?), the explorer deletes entries
        // from the view that it hasn't inserted into the view. We don't want
        // to crash, so we catch this case here.
        return false;
    }

    Broadcast(SvListAction::REMOVING, const_cast<SvTreeListEntry*>(pEntry));
    sal_uLong nRemoved = 1 + GetChildCount(pEntry);
    bAbsPositionsValid = false;

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

    nEntryCount -= nRemoved;
    Broadcast(SvListAction::REMOVED, const_cast<SvTreeListEntry*>(pEntry));

    return true;
}

SvTreeListEntry* SvTreeList::GetEntryAtAbsPos( sal_uLong nAbsPos ) const
{
    SvTreeListEntry* pEntry = First();
    while ( nAbsPos && pEntry )
    {
        pEntry = Next( pEntry );
        nAbsPos--;
    }
    return pEntry;
}

SvTreeListEntry* SvTreeList::GetEntryAtVisPos( const SvListView* pView, sal_uLong nVisPos ) const
{
    DBG_ASSERT(pView,"GetEntryAtVisPos:No View");
    SvTreeListEntry* pEntry = First();
    while ( nVisPos && pEntry )
    {
        pEntry = NextVisible( pView, pEntry );
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

void SvTreeList::InvalidateEntry( SvTreeListEntry* pEntry )
{
    if (!mbEnableInvalidate)
        return;

    Broadcast( SvListAction::INVALIDATE_ENTRY, pEntry );
}

SvTreeListEntry* SvTreeList::GetRootLevelParent( SvTreeListEntry* pEntry ) const
{
    DBG_ASSERT(pEntry,"GetRootLevelParent:No Entry");
    SvTreeListEntry* pCurParent = nullptr;
    if ( pEntry )
    {
        pCurParent = pEntry->pParent;
        if ( pCurParent == pRootItem.get() )
            return pEntry; // is its own parent
        while( pCurParent && pCurParent->pParent != pRootItem.get() )
            pCurParent = pCurParent->pParent;
    }
    return pCurParent;
}

std::pair<SvTreeListEntries::iterator, SvTreeListEntries::iterator>
    SvTreeList::GetChildIterators(SvTreeListEntry* pParent)
{
    typedef std::pair<SvTreeListEntries::iterator, SvTreeListEntries::iterator> IteratorPair;

    static SvTreeListEntries dummy; // prevent singular iterator asserts
    IteratorPair aRet(dummy.begin(), dummy.end());

    if (!pParent)
        pParent = pRootItem.get();

    if (pParent->m_Children.empty())
        // This entry has no children.
        return aRet;

    aRet.first = pParent->m_Children.begin();
    aRet.second = pParent->m_Children.end();

    return aRet;
}


SvListView::SvListView()
    : m_pImpl(new Impl(*this))
{
    pModel.reset(new SvTreeList(*this));
    m_pImpl->InitTable();
}

void SvListView::dispose()
{
    pModel.reset();
}

SvListView::~SvListView()
{
    m_pImpl->m_DataTable.clear();
}

sal_uLong SvListView::GetSelectionCount() const
{ return m_pImpl->m_nSelectionCount; }

bool SvListView::HasViewData() const
{ return m_pImpl->m_DataTable.size() > 1; }  // There's always a ROOT


void SvListView::Impl::InitTable()
{
    DBG_ASSERT(m_rThis.pModel,"InitTable:No Model");
    DBG_ASSERT(!m_nSelectionCount && !m_nVisibleCount && !m_bVisPositionsValid,
            "InitTable: Not cleared!");

    if (!m_DataTable.empty())
    {
        DBG_ASSERT(m_DataTable.size() == 1, "InitTable: TableCount != 1");
        // Delete the view data allocated to the Clear in the root.
        // Attention: The model belonging to the root entry (and thus the entry
        // itself) might already be deleted.
        m_DataTable.clear();
    }

    SvTreeListEntry* pEntry;

    // insert root entry
    pEntry = m_rThis.pModel->pRootItem.get();
    std::unique_ptr<SvViewDataEntry> pViewData(new SvViewDataEntry);
    pViewData->SetExpanded(true);
    m_DataTable.insert(std::make_pair(pEntry, std::move(pViewData)));
    // now all the other entries
    pEntry = m_rThis.pModel->First();
    while( pEntry )
    {
        pViewData = std::make_unique<SvViewDataEntry>();
        m_rThis.InitViewData( pViewData.get(), pEntry );
        m_DataTable.insert(std::make_pair(pEntry, std::move(pViewData)));
        pEntry = m_rThis.pModel->Next( pEntry );
    }
}

void SvListView::Clear()
{
    m_pImpl->m_DataTable.clear();
    m_pImpl->m_nSelectionCount = 0;
    m_pImpl->m_nVisibleCount = 0;
    m_pImpl->m_bVisPositionsValid = false;
    if( pModel )
    {
        // insert root entry
        SvTreeListEntry* pEntry = pModel->pRootItem.get();
        std::unique_ptr<SvViewDataEntry> pViewData(new SvViewDataEntry);
        pViewData->SetExpanded(true);
        m_pImpl->m_DataTable.insert(std::make_pair(pEntry, std::move(pViewData)));
    }
}

void SvListView::ModelHasCleared()
{
}

void SvListView::ModelHasInserted( SvTreeListEntry* )
{
}

void SvListView::ModelHasInsertedTree( SvTreeListEntry* )
{
}

void SvListView::ModelIsMoving( SvTreeListEntry* /*  pSource */ ,
    SvTreeListEntry* /* pTargetParent */ ,  sal_uLong /* nPos */    )
{
}


void SvListView::ModelHasMoved( SvTreeListEntry* )
{
}

void SvListView::ModelIsRemoving( SvTreeListEntry* )
{
}

void SvListView::ModelHasRemoved( SvTreeListEntry* )
{
    //WARNING WARNING WARNING
    //The supplied pointer should have been deleted
    //before this call. Be careful not to use it!!!
}

void SvListView::ModelHasEntryInvalidated( SvTreeListEntry*)
{
}

void SvListView::Impl::ActionMoving( SvTreeListEntry* pEntry )
{
    SvTreeListEntry* pParent = pEntry->pParent;
    DBG_ASSERT(pParent,"Model not consistent");
    if (pParent != m_rThis.pModel->pRootItem.get() && pParent->m_Children.size() == 1)
    {
        SvViewDataEntry* pViewData = m_DataTable.find( pParent )->second.get();
        pViewData->SetExpanded(false);
    }
    // preliminary
    m_nVisibleCount = 0;
    m_bVisPositionsValid = false;
}

void SvListView::Impl::ActionMoved()
{
    m_nVisibleCount = 0;
    m_bVisPositionsValid = false;
}

void SvListView::Impl::ActionInserted( SvTreeListEntry* pEntry )
{
    DBG_ASSERT(pEntry,"Insert:No Entry");
    std::unique_ptr<SvViewDataEntry> pData(new SvViewDataEntry());
    m_rThis.InitViewData( pData.get(), pEntry );
    std::pair<SvDataTable::iterator, bool> aSuccess =
        m_DataTable.insert(std::make_pair(pEntry, std::move(pData)));
    DBG_ASSERT(aSuccess.second,"Entry already in View");
    if (m_nVisibleCount && m_rThis.pModel->IsEntryVisible(&m_rThis, pEntry))
    {
        m_nVisibleCount = 0;
        m_bVisPositionsValid = false;
    }
}

void SvListView::Impl::ActionInsertedTree( SvTreeListEntry* pEntry )
{
    if (m_rThis.pModel->IsEntryVisible(&m_rThis, pEntry))
    {
        m_nVisibleCount = 0;
        m_bVisPositionsValid = false;
    }
    // iterate over entry and its children
    SvTreeListEntry* pCurEntry = pEntry;
    sal_uInt16 nRefDepth = m_rThis.pModel->GetDepth( pCurEntry );
    while( pCurEntry )
    {
        DBG_ASSERT(m_DataTable.find(pCurEntry) != m_DataTable.end(),"Entry already in Table");
        std::unique_ptr<SvViewDataEntry> pViewData(new SvViewDataEntry());
        m_rThis.InitViewData( pViewData.get(), pEntry );
        m_DataTable.insert(std::make_pair(pCurEntry, std::move(pViewData)));
        pCurEntry = m_rThis.pModel->Next( pCurEntry );
        if ( pCurEntry && m_rThis.pModel->GetDepth(pCurEntry) <= nRefDepth)
            pCurEntry = nullptr;
    }
}

void SvListView::Impl::RemoveViewData( SvTreeListEntry* pParent )
{
    for (auto const& it : pParent->m_Children)
    {
        SvTreeListEntry& rEntry = *it;
        m_DataTable.erase(&rEntry);
        if (rEntry.HasChildren())
            RemoveViewData(&rEntry);
    }
}


void SvListView::Impl::ActionRemoving( SvTreeListEntry* pEntry )
{
    DBG_ASSERT(pEntry,"Remove:No Entry");

    SvViewDataEntry* pViewData = m_DataTable.find( pEntry )->second.get();
    sal_uLong nSelRemoved = 0;
    if ( pViewData->IsSelected() )
        nSelRemoved = 1 + m_rThis.pModel->GetChildSelectionCount(&m_rThis, pEntry);
    m_nSelectionCount -= nSelRemoved;
    sal_uLong nVisibleRemoved = 0;
    if (m_rThis.pModel->IsEntryVisible(&m_rThis, pEntry))
        nVisibleRemoved = 1 + m_rThis.pModel->GetVisibleChildCount(&m_rThis, pEntry);
    if( m_nVisibleCount )
    {
#ifdef DBG_UTIL
        if (m_nVisibleCount < nVisibleRemoved)
        {
            OSL_FAIL("nVisibleRemoved bad");
        }
#endif
        m_nVisibleCount -= nVisibleRemoved;
    }
    m_bVisPositionsValid = false;

    m_DataTable.erase(pEntry);
    RemoveViewData( pEntry );

    SvTreeListEntry* pCurEntry = pEntry->pParent;
    if (pCurEntry && pCurEntry != m_rThis.pModel->pRootItem.get() && pCurEntry->m_Children.size() == 1)
    {
        pViewData = m_DataTable.find(pCurEntry)->second.get();
        pViewData->SetExpanded(false);
    }
}

void SvListView::Impl::ActionClear()
{
    m_rThis.Clear();
}

void SvListView::ModelNotification( SvListAction nActionId, SvTreeListEntry* pEntry1,
                        SvTreeListEntry* pEntry2, sal_uLong nPos )
{
    switch( nActionId )
    {
        case SvListAction::INSERTED:
            m_pImpl->ActionInserted( pEntry1 );
            ModelHasInserted( pEntry1 );
            break;
        case SvListAction::INSERTED_TREE:
            m_pImpl->ActionInsertedTree( pEntry1 );
            ModelHasInsertedTree( pEntry1 );
            break;
        case SvListAction::REMOVING:
            ModelIsRemoving( pEntry1 );
            m_pImpl->ActionRemoving( pEntry1 );
            break;
        case SvListAction::REMOVED:
            ModelHasRemoved( pEntry1 );
            break;
        case SvListAction::MOVING:
            ModelIsMoving( pEntry1, pEntry2, nPos );
            m_pImpl->ActionMoving( pEntry1 );
            break;
        case SvListAction::MOVED:
            m_pImpl->ActionMoved();
            ModelHasMoved( pEntry1 );
            break;
        case SvListAction::CLEARING:
            m_pImpl->ActionClear();
            ModelHasCleared(); // sic! for compatibility reasons!
            break;
        case SvListAction::CLEARED:
            break;
        case SvListAction::INVALIDATE_ENTRY:
            // no action for the base class
            ModelHasEntryInvalidated( pEntry1 );
            break;
        case SvListAction::RESORTED:
            m_pImpl->m_bVisPositionsValid = false;
            break;
        case SvListAction::RESORTING:
            break;
        case SvListAction::REVERSED:
            m_pImpl->m_bVisPositionsValid = false;
            break;
        default:
            OSL_FAIL("unknown ActionId");
    }
}

void SvListView::InitViewData( SvViewDataEntry*, SvTreeListEntry* )
{
}

bool SvListView::IsExpanded( SvTreeListEntry* pEntry ) const
{
    DBG_ASSERT(pEntry,"IsExpanded:No Entry");
    SvDataTable::const_iterator itr = m_pImpl->m_DataTable.find(pEntry);
    DBG_ASSERT(itr != m_pImpl->m_DataTable.end(),"Entry not in Table");
    if (itr == m_pImpl->m_DataTable.end())
        return false;
    return itr->second->IsExpanded();
}

bool SvListView::IsSelected( SvTreeListEntry* pEntry ) const
{
    DBG_ASSERT(pEntry,"IsExpanded:No Entry");
    SvDataTable::const_iterator itr = m_pImpl->m_DataTable.find(pEntry);
    if (itr == m_pImpl->m_DataTable.end())
        return false;
    return itr->second->IsSelected();
}

void SvListView::SetEntryFocus( SvTreeListEntry* pEntry, bool bFocus )
{
    DBG_ASSERT(pEntry,"SetEntryFocus:No Entry");
    SvDataTable::iterator itr = m_pImpl->m_DataTable.find(pEntry);
    DBG_ASSERT(itr != m_pImpl->m_DataTable.end(),"Entry not in Table");
    itr->second->SetFocus(bFocus);
}

const SvViewDataEntry* SvListView::GetViewData( const SvTreeListEntry* pEntry ) const
{
    SvDataTable::const_iterator itr =
        m_pImpl->m_DataTable.find(const_cast<SvTreeListEntry*>(pEntry));
    if (itr == m_pImpl->m_DataTable.end())
        return nullptr;
    return itr->second.get();
}

SvViewDataEntry* SvListView::GetViewData( SvTreeListEntry* pEntry )
{
    SvDataTable::iterator itr = m_pImpl->m_DataTable.find( pEntry );
    DBG_ASSERT(itr != m_pImpl->m_DataTable.end(),"Entry not in model or wrong view");
    return itr->second.get();
}

sal_Int32 SvTreeList::Compare(const SvTreeListEntry* pLeft, const SvTreeListEntry* pRight) const
{
    if( aCompareLink.IsSet())
    {
        SvSortData aSortData;
        aSortData.pLeft = pLeft;
        aSortData.pRight = pRight;
        return aCompareLink.Call( aSortData );
    }
    return 0;
}

void SvTreeList::Resort()
{
    Broadcast( SvListAction::RESORTING );
    bAbsPositionsValid = false;
    ResortChildren( pRootItem.get() );
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
        if (nCompare != 0 && mrList.GetSortMode() == SortDescending)
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
    DBG_ASSERT(pParent,"Parent not set");

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

void SvTreeList::Reverse()
{
    bAbsPositionsValid = false;
    ReverseChildren(pRootItem.get());
    Broadcast(SvListAction::REVERSED);
}

void SvTreeList::ReverseChildren( SvTreeListEntry* pParent )
{
    DBG_ASSERT(pParent,"Parent not set");

    if (pParent->m_Children.empty())
        return;

    std::reverse(pParent->m_Children.begin(), pParent->m_Children.end());
    // Recursively sort child entries.
    for (auto const& it : pParent->m_Children)
    {
        SvTreeListEntry& r = *it;
        ReverseChildren(&r);
    }

    SetListPositions(pParent->m_Children); // correct list position in target list
}

void SvTreeList::GetInsertionPos( SvTreeListEntry const * pEntry, SvTreeListEntry* pParent,
    sal_uLong& rPos )
{
    DBG_ASSERT(pEntry,"No Entry");

    if( eSortMode == SortNone )
        return;

    rPos = TREELIST_ENTRY_NOTFOUND;
    const SvTreeListEntries& rChildList = GetChildList(pParent);

    if (rChildList.empty())
        return;

    long i = 0;
    long j = rChildList.size()-1;
    long k;
    sal_Int32 nCompare = 1;

    do
    {
        k = (i+j)/2;
        const SvTreeListEntry* pTempEntry = rChildList[k].get();
        nCompare = Compare( pEntry, pTempEntry );
        if (nCompare != 0 && eSortMode == SortDescending)
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
        if (i > static_cast<long>(rChildList.size()-1)) // not found, end of list
            rPos = TREELIST_ENTRY_NOTFOUND;
        else
            rPos = i;              // not found, middle of list
    }
    else
        rPos = k;
}

bool SvTreeList::HasChildren( const SvTreeListEntry* pEntry ) const
{
    if ( !pEntry )
        pEntry = pRootItem.get();

    return !pEntry->m_Children.empty();
}

bool SvTreeList::HasParent( const SvTreeListEntry* pEntry ) const
{
    return pEntry->pParent != pRootItem.get();
}

SvTreeListEntry* SvTreeList::GetEntry( SvTreeListEntry* pParent, sal_uLong nPos ) const
{   if ( !pParent )
        pParent = pRootItem.get();
    SvTreeListEntry* pRet = nullptr;
    if (nPos < pParent->m_Children.size())
        pRet = pParent->m_Children[nPos].get();
    return pRet;
}

SvTreeListEntry* SvTreeList::GetEntry( sal_uLong nRootPos ) const
{
    SvTreeListEntry* pRet = nullptr;
    if (nEntryCount && nRootPos < pRootItem->m_Children.size())
        pRet = pRootItem->m_Children[nRootPos].get();
    return pRet;
}

const SvTreeListEntries& SvTreeList::GetChildList( SvTreeListEntry* pParent ) const
{
    if ( !pParent )
        pParent = pRootItem.get();
    return pParent->m_Children;
}

SvTreeListEntries& SvTreeList::GetChildList( SvTreeListEntry* pParent )
{
    if ( !pParent )
        pParent = pRootItem.get();
    return pParent->m_Children;
}

const SvTreeListEntry* SvTreeList::GetParent( const SvTreeListEntry* pEntry ) const
{
    const SvTreeListEntry* pParent = pEntry->pParent;
    if (pParent == pRootItem.get())
        pParent = nullptr;
    return pParent;
}

SvTreeListEntry* SvTreeList::GetParent( SvTreeListEntry* pEntry )
{
    SvTreeListEntry* pParent = pEntry->pParent;
    if (pParent == pRootItem.get())
        pParent = nullptr;
    return pParent;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
