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

#include <svtools/treelist.hxx>
#include <svtools/treelistentry.hxx>
#include <svtools/viewdataentry.hxx>
#include <osl/diagnose.h>

SvTreeList::SvTreeList() :
    mbEnableInvalidate(true)
{
    nEntryCount = 0;
    bAbsPositionsValid = false;
    nRefCount = 1;
    pRootItem = new SvTreeListEntry;
    eSortMode = SortNone;
}

SvTreeList::~SvTreeList()
{
    Clear();
    delete pRootItem;
#ifdef DBG_UTIL
    pRootItem = 0;
#endif
}

void SvTreeList::Broadcast(
    SvListAction nActionId,
    SvTreeListEntry* pEntry1,
    SvTreeListEntry* pEntry2,
    sal_uLong nPos
) {
    sal_uLong nViewCount = aViewList.size();
    for( sal_uLong nCurView = 0; nCurView < nViewCount; nCurView++ )
    {
        SvListView* pView = aViewList[ nCurView ];
        if( pView )
            pView->ModelNotification( nActionId, pEntry1, pEntry2, nPos );
    }
}

void SvTreeList::InsertView( SvListView* pView )
{
    for ( sal_uLong i = 0, n = aViewList.size(); i < n; ++i ) {
        if ( aViewList[ i ] == pView ) {
            return;
        }
    }
    aViewList.push_back( pView );
    nRefCount++;
}

void SvTreeList::RemoveView( SvListView* pView )
{
    for ( ListViewsType::iterator it = aViewList.begin(); it != aViewList.end(); ++it )
    {
        if ( *it == pView )
        {
            aViewList.erase( it );
            --nRefCount;
            break;
        }
    }
}


// an entry is visible if all parents are expanded
bool SvTreeList::IsEntryVisible( const SvListView* pView, SvTreeListEntry* pEntry ) const
{
    DBG_ASSERT(pView&&pEntry,"IsVisible:Invalid Params");
    bool bRetVal = false;
    do
    {
        if ( pEntry == pRootItem )
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
    DBG_ASSERT(pEntry&&pEntry!=pRootItem,"GetDepth:Bad Entry");
    sal_uInt16 nDepth = 0;
    while( pEntry->pParent != pRootItem )
    {
        nDepth++;
        pEntry = pEntry->pParent;
    }
    return nDepth;
}

bool SvTreeList::IsAtRootDepth( const SvTreeListEntry* pEntry ) const
{
    return pEntry->pParent == pRootItem;
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
        pParent = pRootItem;

    if (pParent->maChildren.empty())
        return false;

    SvTreeListEntries::const_iterator it = pParent->maChildren.begin(), itEnd = pParent->maChildren.end();
    for (; it != itEnd; ++it)
    {
        const SvTreeListEntry* pThis = &(*it);
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

class FindByPointer : std::unary_function<SvTreeListEntry, bool>
{
    const SvTreeListEntry* mpEntry;
public:
    FindByPointer(const SvTreeListEntry* p) : mpEntry(p) {}

    bool operator() (const SvTreeListEntry& rEntry) const
    {
        return mpEntry == &rEntry;
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
        pTargetParent = pRootItem;
    DBG_ASSERT(pSrcEntry!=pTargetParent,"Move:Source=Target");

    Broadcast( SvListAction::MOVING, pSrcEntry, pTargetParent, nListPos );

    if ( pSrcEntry == pTargetParent )
        // You can't move an entry onto itself as the parent. Just return its
        // position and bail out.
        return pSrcEntry->GetChildListPos();

    bAbsPositionsValid = false;

    SvTreeListEntries& rDst = pTargetParent->maChildren;
    SvTreeListEntries& rSrc = pSrcEntry->pParent->maChildren;

    bool bSameParent = pTargetParent == pSrcEntry->pParent;

    // Find the position of the entry being moved in the source container.
    SvTreeListEntries::iterator itSrcPos = rSrc.begin(), itEnd = rSrc.end();
    for (; itSrcPos != itEnd; ++itSrcPos)
    {
        const SvTreeListEntry* p = &(*itSrcPos);
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
        SvTreeListEntries::auto_type p = rSrc.release(itSrcPos);
        // Determine the insertion position.
        SvTreeListEntries::iterator itDstPos = rSrc.end();
        if (nListPos < rSrc.size())
        {
            itDstPos = rSrc.begin();
            std::advance(itDstPos, nListPos);
        }
        rSrc.insert(itDstPos, p.release());
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
        SvTreeListEntries::auto_type p = rSrc.release(itSrcPos);
        rDst.insert(itDstPos, p.release());
    }

    // move parent umsetzen (do this only now, because we need the parent for
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
        pTargetParent = pRootItem;

    bAbsPositionsValid = false;

    sal_uLong nCloneCount = 0;
    SvTreeListEntry* pClonedEntry = Clone( pSrcEntry, nCloneCount );
    nEntryCount += nCloneCount;

    SvTreeListEntries& rDst = pTargetParent->maChildren;

    pClonedEntry->pParent = pTargetParent;      // move parent

    if (nListPos < rDst.size())
    {
        SvTreeListEntries::iterator itPos = rDst.begin(); // insertion position.
        std::advance(itPos, nListPos);
        rDst.insert(itPos, pClonedEntry);
    }
    else
        rDst.push_back(pClonedEntry);

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
        pParent = pRootItem;
        nPos = 0UL;
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
        pTargetParent = pRootItem;

    // take sorting into account
    GetInsertionPos( pSrcEntry, pTargetParent, nListPos );

    bAbsPositionsValid = false;

    pSrcEntry->pParent = pTargetParent; // move parent
    SvTreeListEntries& rDst = pTargetParent->maChildren;

    if (nListPos < rDst.size())
    {
        SvTreeListEntries::iterator itPos = rDst.begin();
        std::advance(itPos, nListPos);
        rDst.insert(itPos, pSrcEntry);
    }
    else
        rDst.push_back(pSrcEntry);

    SetListPositions(rDst); // correct list position in target list
    nEntryCount += GetChildCount( pSrcEntry );
    nEntryCount++; // the parent is new, too

    Broadcast(SvListAction::INSERTED_TREE, pSrcEntry );
}

SvTreeListEntry* SvTreeList::CloneEntry( SvTreeListEntry* pSource ) const
{
    if( aCloneLink.IsSet() )
        return reinterpret_cast<SvTreeListEntry*>(aCloneLink.Call( pSource ));
    SvTreeListEntry* pEntry = CreateEntry();
    pEntry->Clone(pSource);
    return pEntry;
}

SvTreeListEntry* SvTreeList::CreateEntry()
{
    return new SvTreeListEntry;
}

SvTreeListEntry* SvTreeList::Clone( SvTreeListEntry* pEntry, sal_uLong& nCloneCount ) const
{
    SvTreeListEntry* pClonedEntry = CloneEntry( pEntry );
    nCloneCount = 1;
    if (!pEntry->maChildren.empty())
        // Clone the child entries.
        CloneChildren(pClonedEntry->maChildren, nCloneCount, pEntry->maChildren, pClonedEntry);

    return pClonedEntry;
}

void SvTreeList::CloneChildren(
        SvTreeListEntries& rDst, sal_uLong& rCloneCount, SvTreeListEntries& rSrc, SvTreeListEntry* pNewParent) const
{
    SvTreeListEntries aClone;
    SvTreeListEntries::iterator it = rSrc.begin(), itEnd = rSrc.end();
    for (; it != itEnd; ++it)
    {
        SvTreeListEntry& rEntry = *it;
        SvTreeListEntry* pNewEntry = CloneEntry(&rEntry);
        ++rCloneCount;
        pNewEntry->pParent = pNewParent;
        if (!rEntry.maChildren.empty())
            // Clone entries recursively.
            CloneChildren(pNewEntry->maChildren, rCloneCount, rEntry.maChildren, pNewEntry);

        aClone.push_back(pNewEntry);
    }

    rDst.swap(aClone);
}

sal_uLong SvTreeList::GetChildCount( const SvTreeListEntry* pParent ) const
{
    if ( !pParent )
        return GetEntryCount();

    if (!pParent || pParent->maChildren.empty())
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
        pParent = pRootItem;

    if (!pParent || !pView->IsExpanded(pParent) || pParent->maChildren.empty())
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
        pParent = pRootItem;

    if (!pParent || pParent->maChildren.empty())
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
        return &pRootItem->maChildren[0];
    else
        return 0;
}

SvTreeListEntry* SvTreeList::Next( SvTreeListEntry* pActEntry, sal_uInt16* pDepth ) const
{
    DBG_ASSERT( pActEntry && pActEntry->pParent, "SvTreeList::Next: invalid entry/parent!" );
    if ( !pActEntry || !pActEntry->pParent )
        return NULL;

    sal_uInt16 nDepth = 0;
    bool bWithDepth = false;
    if ( pDepth )
    {
        nDepth = *pDepth;
        bWithDepth = true;
    }

    // Get the list where the current entry belongs to (from its parent).
    SvTreeListEntries* pActualList = &pActEntry->pParent->maChildren;
    sal_uLong nActualPos = pActEntry->GetChildListPos();

    if (!pActEntry->maChildren.empty())
    {
        // The current entry has children. Get its first child entry.
        nDepth++;
        pActEntry = &pActEntry->maChildren[0];
        if ( bWithDepth )
            *pDepth = nDepth;
        return pActEntry;
    }

    if (pActualList->size() > (nActualPos+1))
    {
        // Get the next sibling of the current entry.
        pActEntry = &(*pActualList)[nActualPos+1];
        if ( bWithDepth )
            *pDepth = nDepth;
        return pActEntry;
    }

    // Move up level(s) until we find the level where the next sibling exists.
    SvTreeListEntry* pParent = pActEntry->pParent;
    nDepth--;
    while( pParent != pRootItem && pParent != 0 )
    {
        DBG_ASSERT(pParent!=0,"TreeData corrupt!");
        pActualList = &pParent->pParent->maChildren;
        nActualPos = pParent->GetChildListPos();
        if (pActualList->size() > (nActualPos+1))
        {
            pActEntry = &(*pActualList)[nActualPos+1];
            if ( bWithDepth )
                *pDepth = nDepth;
            return pActEntry;
        }
        pParent = pParent->pParent;
        nDepth--;
    }
    return 0;
}

SvTreeListEntry* SvTreeList::Prev( SvTreeListEntry* pActEntry, sal_uInt16* pDepth ) const
{
    DBG_ASSERT(pActEntry!=0,"Entry?");

    sal_uInt16 nDepth = 0;
    bool bWithDepth = false;
    if ( pDepth )
    {
        nDepth = *pDepth;
        bWithDepth = true;
    }

    SvTreeListEntries* pActualList = &pActEntry->pParent->maChildren;
    sal_uLong nActualPos = pActEntry->GetChildListPos();

    if ( nActualPos > 0 )
    {
        pActEntry = &(*pActualList)[nActualPos-1];
        while (!pActEntry->maChildren.empty())
        {
            pActualList = &pActEntry->maChildren;
            nDepth++;
            pActEntry = &pActualList->back();
        }
        if ( bWithDepth )
            *pDepth = nDepth;
        return pActEntry;
    }
    if ( pActEntry->pParent == pRootItem )
        return 0;

    pActEntry = pActEntry->pParent;

    if ( pActEntry )
    {
        nDepth--;
        if ( bWithDepth )
            *pDepth = nDepth;
        return pActEntry;
    }
    return 0;
}

SvTreeListEntry* SvTreeList::Last() const
{
    SvTreeListEntries* pActList = &pRootItem->maChildren;
    SvTreeListEntry* pEntry = NULL;
    while (!pActList->empty())
    {
        pEntry = &pActList->back();
        pActList = &pEntry->maChildren;
    }
    return pEntry;
}

sal_uLong SvTreeList::GetVisiblePos( const SvListView* pView, SvTreeListEntry* pEntry ) const
{
    DBG_ASSERT(pView&&pEntry,"View/Entry?");

    if ( !pView->bVisPositionsValid )
    {
        // to make GetVisibleCount refresh the positions
        const_cast<SvListView*>(pView)->nVisibleCount = 0;
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
    if ( pView->nVisibleCount )
        return pView->nVisibleCount;

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
    pView->nVisibleCount = nPos;
    pView->bVisPositionsValid = true;
    return nPos;
}


// For performance reasons, this function assumes that the passed entry is
// already visible.
SvTreeListEntry* SvTreeList::NextVisible(const SvListView* pView,SvTreeListEntry* pActEntry,sal_uInt16* pActDepth) const
{
    DBG_ASSERT(pView,"NextVisible:No View");
    if ( !pActEntry )
        return 0;

    sal_uInt16 nDepth = 0;
    bool bWithDepth = false;
    if ( pActDepth )
    {
        nDepth = *pActDepth;
        bWithDepth = true;
    }

    SvTreeListEntries* pActualList = &pActEntry->pParent->maChildren;
    sal_uLong nActualPos = pActEntry->GetChildListPos();

    if ( pView->IsExpanded(pActEntry) )
    {
        OSL_ENSURE(!pActEntry->maChildren.empty(), "Pass entry is supposed to have child entries.");

        nDepth++;
        pActEntry = &pActEntry->maChildren[0];
        if ( bWithDepth )
            *pActDepth = nDepth;
        return pActEntry;
    }

    nActualPos++;
    if ( pActualList->size() > nActualPos  )
    {
        pActEntry = &(*pActualList)[nActualPos];
        if ( bWithDepth )
            *pActDepth = nDepth;
        return pActEntry;
    }

    SvTreeListEntry* pParent = pActEntry->pParent;
    nDepth--;
    while( pParent != pRootItem )
    {
        pActualList = &pParent->pParent->maChildren;
        nActualPos = pParent->GetChildListPos();
        nActualPos++;
        if ( pActualList->size() > nActualPos )
        {
            pActEntry = &(*pActualList)[nActualPos];
            if ( bWithDepth )
                *pActDepth = nDepth;
            return pActEntry;
        }
        pParent = pParent->pParent;
        nDepth--;
    }
    return 0;
}


// For performance reasons, this function assumes that the passed entry is
// already visible.

SvTreeListEntry* SvTreeList::PrevVisible(const SvListView* pView, SvTreeListEntry* pActEntry, sal_uInt16* pActDepth) const
{
    DBG_ASSERT(pView&&pActEntry,"PrevVis:View/Entry?");

    sal_uInt16 nDepth = 0;
    bool bWithDepth = false;
    if ( pActDepth )
    {
        nDepth = *pActDepth;
        bWithDepth = true;
    }

    SvTreeListEntries* pActualList = &pActEntry->pParent->maChildren;
    sal_uLong nActualPos = pActEntry->GetChildListPos();

    if ( nActualPos > 0 )
    {
        pActEntry = &(*pActualList)[nActualPos-1];
        while( pView->IsExpanded(pActEntry) )
        {
            pActualList = &pActEntry->maChildren;
            nDepth++;
            pActEntry = &pActualList->back();
        }
        if ( bWithDepth )
            *pActDepth = nDepth;
        return pActEntry;
    }

    if ( pActEntry->pParent == pRootItem )
        return 0;

    pActEntry = pActEntry->pParent;
    if ( pActEntry )
    {
        nDepth--;
        if ( bWithDepth )
            *pActDepth = nDepth;
        return pActEntry;
    }
    return 0;
}

SvTreeListEntry* SvTreeList::LastVisible( const SvListView* pView, sal_uInt16* pDepth) const
{
    DBG_ASSERT(pView,"LastVis:No View");
    SvTreeListEntry* pEntry = Last();
    while( pEntry && !IsEntryVisible( pView, pEntry ) )
        pEntry = PrevVisible( pView, pEntry );
    if ( pEntry && pDepth )
        *pDepth = GetDepth( pEntry );
    return pEntry;
}

SvTreeListEntry* SvTreeList::NextVisible(const SvListView* pView,SvTreeListEntry* pEntry,sal_uInt16& nDelta) const
{
    DBG_ASSERT(pView&&pEntry&&IsEntryVisible(pView,pEntry),"NextVis:Wrong Prms/!Vis");

    sal_uLong nVisPos = GetVisiblePos( pView, pEntry );
    // nDelta entries existent?
    // example: 0,1,2,3,4,5,6,7,8,9 nVisPos=5 nDelta=7
    //           nNewDelta = 10-nVisPos-1 == 4
    if (  nVisPos+nDelta >= pView->nVisibleCount )
    {
        nDelta = (sal_uInt16)(pView->nVisibleCount-nVisPos);
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
        nDelta = (sal_uInt16)nVisPos;
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
        return 0;
    SvTreeListEntry* pActSelEntry = First();
    while( pActSelEntry && !pView->IsSelected(pActSelEntry) )
        pActSelEntry = NextVisible( pView, pActSelEntry );
    return pActSelEntry;
}


SvTreeListEntry* SvTreeList::FirstChild( SvTreeListEntry* pParent ) const
{
    if ( !pParent )
        pParent = pRootItem;
    SvTreeListEntry* pResult;
    if (!pParent->maChildren.empty())
        pResult = &pParent->maChildren[0];
    else
        pResult = 0;
    return pResult;
}

SvTreeListEntry* SvTreeList::NextSibling( SvTreeListEntry* pEntry )
{
    DBG_ASSERT(pEntry,"Entry?");
    if( !pEntry )
        return 0;

    SvTreeListEntries& rList = pEntry->pParent->maChildren;
    sal_uLong nPos = pEntry->GetChildListPos();
    nPos++;
    return nPos < rList.size() ? &rList[nPos] : NULL;
}

SvTreeListEntry* SvTreeList::PrevSibling( SvTreeListEntry* pEntry )
{
    DBG_ASSERT(pEntry,"Entry?");
    if( !pEntry )
        return 0;

    SvTreeListEntries& rList = pEntry->pParent->maChildren;
    sal_uLong nPos = pEntry->GetChildListPos();
    if ( nPos == 0 )
        return 0;
    nPos--;
    pEntry = &rList[nPos];
    return pEntry;
}


SvTreeListEntry* SvTreeList::LastSibling( SvTreeListEntry* pEntry )
{
    DBG_ASSERT(pEntry,"LastSibling:Entry?");
    if( !pEntry )
        return 0;

    SvTreeListEntries& rChildren = pEntry->pParent->maChildren;
    return rChildren.empty() ? NULL : &rChildren.back();
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
        pParent = pRootItem;


    SvTreeListEntries& rList = pParent->maChildren;

    // take sorting into account
    GetInsertionPos( pEntry, pParent, nPos );

    bAbsPositionsValid = false;
    pEntry->pParent = pParent;

    if (nPos < rList.size())
    {
        SvTreeListEntries::iterator itPos = rList.begin();
        std::advance(itPos, nPos);
        rList.insert(itPos, pEntry);
    }
    else
        rList.push_back(pEntry);

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

void SvTreeList::Expand( SvListView* pView, SvTreeListEntry* pEntry )
{
    DBG_ASSERT(pEntry&&pView,"Expand:View/Entry?");
    if ( pView->IsExpanded(pEntry) )
        return;

    DBG_ASSERT(!pEntry->maChildren.empty(), "SvTreeList::Expand: We expected to have child entries.");

    SvViewDataEntry* pViewData = pView->GetViewData(pEntry);
    pViewData->SetExpanded(true);
    SvTreeListEntry* pParent = pEntry->pParent;
    // if parent is visible, invalidate status data
    if ( pView->IsExpanded( pParent ) )
    {
        pView->bVisPositionsValid = false;
        pView->nVisibleCount = 0;
    }
}

void SvTreeList::Collapse( SvListView* pView, SvTreeListEntry* pEntry )
{
    DBG_ASSERT(pView&&pEntry,"Collapse:View/Entry?");
    if ( !pView->IsExpanded(pEntry) )
        return;

    DBG_ASSERT(!pEntry->maChildren.empty(), "SvTreeList::Collapse: We expected have child entries.");

    SvViewDataEntry* pViewData = pView->GetViewData( pEntry );
    pViewData->SetExpanded(false);

    SvTreeListEntry* pParent = pEntry->pParent;
    if ( pView->IsExpanded(pParent) )
    {
        pView->nVisibleCount = 0;
        pView->bVisPositionsValid = false;
    }
}

bool SvTreeList::Select( SvListView* pView, SvTreeListEntry* pEntry, bool bSelect )
{
    DBG_ASSERT(pView&&pEntry,"Select:View/Entry?");
    SvViewDataEntry* pViewData = pView->GetViewData( pEntry );
    if ( bSelect )
    {
        if ( pViewData->IsSelected() || !pViewData->IsSelectable() )
            return false;
        else
        {
            pViewData->SetSelected(true);
            pView->nSelectionCount++;
        }
    }
    else
    {
        if ( !pViewData->IsSelected() )
            return false;
        else
        {
            pViewData->SetSelected(false);
            pView->nSelectionCount--;
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
    SvTreeListEntries& rList = pParent->maChildren;
    bool bLastEntry = false;

    // Since we need the live instance of SvTreeListEntry for broadcasting,
    // we first need to pop it from the container, broadcast it, then delete
    // the instance manually at the end.

    if ( pEntry->HasChildListPos() )
    {
        size_t nListPos = pEntry->GetChildListPos();
        bLastEntry = (nListPos == (rList.size()-1));
        SvTreeListEntries::iterator it = rList.begin();
        std::advance(it, nListPos);
        rList.release(it).release();
    }
    else
    {
        SvTreeListEntries::iterator it =
            std::find_if(rList.begin(), rList.end(), FindByPointer(pEntry));
        if (it != rList.end())
            rList.release(it).release();
    }

    if (!rList.empty() && !bLastEntry)
        SetListPositions(rList);

    nEntryCount -= nRemoved;
    Broadcast(SvListAction::REMOVED, const_cast<SvTreeListEntry*>(pEntry));
    delete pEntry;

    return true;
}

void SvTreeList::SelectAll( SvListView* pView, bool bSelect )
{
    assert(pView && "SelectAll:NoView");
    SvTreeListEntry* pEntry = First();
    while ( pEntry )
    {
        SvViewDataEntry* pViewData = pView->GetViewData( pEntry );
        pViewData->SetSelected(bSelect);
        pEntry = Next( pEntry );
    }
    if ( bSelect )
        pView->nSelectionCount = nEntryCount;
    else
        pView->nSelectionCount = 0;
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

    SvTreeListEntry& rFirst = rEntries.front();
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
    SvTreeListEntry* pCurParent = 0;
    if ( pEntry )
    {
        pCurParent = pEntry->pParent;
        if ( pCurParent == pRootItem )
            return pEntry; // is its own parent
        while( pCurParent && pCurParent->pParent != pRootItem )
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
        pParent = pRootItem;

    if (pParent->maChildren.empty())
        // This entry has no children.
        return aRet;

    aRet.first = pParent->maChildren.begin();
    aRet.second = pParent->maChildren.end();

    return aRet;
}


SvListView::SvListView()
{
    pModel = 0;
    nSelectionCount = 0;
    nVisibleCount = 0;
    bVisPositionsValid = false;
}


SvListView::~SvListView()
{
    maDataTable.clear();
}

void SvListView::InitTable()
{
    DBG_ASSERT(pModel,"InitTable:No Model");
    DBG_ASSERT(!nSelectionCount&&!nVisibleCount&&!bVisPositionsValid,"InitTable: Not cleared!");

    if( maDataTable.size() )
    {
        DBG_ASSERT(maDataTable.size()==1,"InitTable: TableCount != 1");
        // Delete the view data allocated to the Clear in the root.
        // Attention: The model belonging to the root entry (and thus the entry
        // itself) might already be deleted.
        maDataTable.clear();
    }

    SvTreeListEntry* pEntry;
    SvViewDataEntry* pViewData;

    // insert root entry
    pEntry = pModel->pRootItem;
    pViewData = new SvViewDataEntry;
    pViewData->SetExpanded(true);
    maDataTable.insert( pEntry, pViewData );
    // now all the other entries
    pEntry = pModel->First();
    while( pEntry )
    {
        pViewData = CreateViewData( pEntry );
        DBG_ASSERT(pViewData,"InitTable:No ViewData");
        InitViewData( pViewData, pEntry );
        maDataTable.insert( pEntry, pViewData );
        pEntry = pModel->Next( pEntry );
    }
}

SvViewDataEntry* SvListView::CreateViewData( SvTreeListEntry* )
{
    return new SvViewDataEntry;
}

void SvListView::Clear()
{
    maDataTable.clear();
    nSelectionCount = 0;
    nVisibleCount = 0;
    bVisPositionsValid = false;
    if( pModel )
    {
        // insert root entry
        SvTreeListEntry* pEntry = pModel->pRootItem;
        SvViewDataEntry* pViewData = new SvViewDataEntry;
        pViewData->SetExpanded(true);
        maDataTable.insert( pEntry, pViewData );
    }
}

void SvListView::SetModel( SvTreeList* pNewModel )
{
    bool bBroadcastCleared = false;
    if ( pModel )
    {
        pModel->RemoveView( this );
        bBroadcastCleared = true;
        ModelNotification( SvListAction::CLEARING,0,0,0 );
        if ( pModel->GetRefCount() == 0 )
            delete pModel;
    }
    pModel = pNewModel;
    InitTable();
    pNewModel->InsertView( this );
    if( bBroadcastCleared )
        ModelNotification( SvListAction::CLEARED,0,0,0 );
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

void SvListView::ActionMoving( SvTreeListEntry* pEntry,SvTreeListEntry*,sal_uLong)
{
    SvTreeListEntry* pParent = pEntry->pParent;
    DBG_ASSERT(pParent,"Model not consistent");
    if (pParent != pModel->pRootItem && pParent->maChildren.size() == 1)
    {
        SvViewDataEntry* pViewData = maDataTable.find( pParent )->second;
        pViewData->SetExpanded(false);
    }
    // vorlaeufig
    nVisibleCount = 0;
    bVisPositionsValid = false;
}

void SvListView::ActionMoved( SvTreeListEntry* /* pEntry */ ,
                            SvTreeListEntry* /* pTargetPrnt */ ,
                            sal_uLong /* nChildPos */ )
{
    nVisibleCount = 0;
    bVisPositionsValid = false;
}

void SvListView::ActionInserted( SvTreeListEntry* pEntry )
{
    DBG_ASSERT(pEntry,"Insert:No Entry");
    SvViewDataEntry* pData = CreateViewData( pEntry );
    InitViewData( pData, pEntry );
    #ifdef DBG_UTIL
    std::pair<SvDataTable::iterator, bool> aSuccess =
    #endif
        maDataTable.insert( pEntry, pData );
    DBG_ASSERT(aSuccess.second,"Entry already in View");
    if ( nVisibleCount && pModel->IsEntryVisible( this, pEntry ))
    {
        nVisibleCount = 0;
        bVisPositionsValid = false;
    }
}

void SvListView::ActionInsertedTree( SvTreeListEntry* pEntry )
{
    if ( pModel->IsEntryVisible( this, pEntry ))
    {
        nVisibleCount = 0;
        bVisPositionsValid = false;
    }
    // iterate over entry and its children
    SvTreeListEntry* pCurEntry = pEntry;
    sal_uInt16 nRefDepth = pModel->GetDepth( pCurEntry );
    while( pCurEntry )
    {
        DBG_ASSERT(maDataTable.find(pCurEntry) != maDataTable.end(),"Entry already in Table");
        SvViewDataEntry* pViewData = CreateViewData( pCurEntry );
        DBG_ASSERT(pViewData,"No ViewData");
        InitViewData( pViewData, pEntry );
        maDataTable.insert( pCurEntry, pViewData );
        pCurEntry = pModel->Next( pCurEntry );
        if ( pCurEntry && pModel->GetDepth(pCurEntry) <= nRefDepth)
            pCurEntry = 0;
    }
}

void SvListView::RemoveViewData( SvTreeListEntry* pParent )
{
    SvTreeListEntries::iterator it = pParent->maChildren.begin(), itEnd = pParent->maChildren.end();
    for (; it != itEnd; ++it)
    {
        SvTreeListEntry& rEntry = *it;
        maDataTable.erase(&rEntry);
        if (rEntry.HasChildren())
            RemoveViewData(&rEntry);
    }
}



void SvListView::ActionRemoving( SvTreeListEntry* pEntry )
{
    DBG_ASSERT(pEntry,"Remove:No Entry");

    SvViewDataEntry* pViewData = maDataTable.find( pEntry )->second;
    sal_uLong nSelRemoved = 0;
    if ( pViewData->IsSelected() )
        nSelRemoved = 1 + pModel->GetChildSelectionCount( this, pEntry );
    nSelectionCount -= nSelRemoved;
    sal_uLong nVisibleRemoved = 0;
    if ( pModel->IsEntryVisible( this, pEntry ) )
        nVisibleRemoved = 1 + pModel->GetVisibleChildCount( this, pEntry );
    if( nVisibleCount )
    {
#ifdef DBG_UTIL
        if( nVisibleCount < nVisibleRemoved )
        {
            OSL_FAIL("nVisibleRemoved bad");
        }
#endif
        nVisibleCount -= nVisibleRemoved;
    }
    bVisPositionsValid = false;

    maDataTable.erase(pEntry);
    RemoveViewData( pEntry );

    SvTreeListEntry* pCurEntry = pEntry->pParent;
    if (pCurEntry && pCurEntry != pModel->pRootItem && pCurEntry->maChildren.size() == 1)
    {
        pViewData = maDataTable.find(pCurEntry)->second;
        pViewData->SetExpanded(false);
    }
}

void SvListView::ActionClear()
{
    Clear();
}

void SvListView::ModelNotification( SvListAction nActionId, SvTreeListEntry* pEntry1,
                        SvTreeListEntry* pEntry2, sal_uLong nPos )
{
    switch( nActionId )
    {
        case SvListAction::INSERTED:
            ActionInserted( pEntry1 );
            ModelHasInserted( pEntry1 );
            break;
        case SvListAction::INSERTED_TREE:
            ActionInsertedTree( pEntry1 );
            ModelHasInsertedTree( pEntry1 );
            break;
        case SvListAction::REMOVING:
            ModelIsRemoving( pEntry1 );
            ActionRemoving( pEntry1 );
            break;
        case SvListAction::REMOVED:
            ModelHasRemoved( pEntry1 );
            break;
        case SvListAction::MOVING:
            ModelIsMoving( pEntry1, pEntry2, nPos );
            ActionMoving( pEntry1, pEntry2, nPos );
            break;
        case SvListAction::MOVED:
            ActionMoved( pEntry1, pEntry2, nPos );
            ModelHasMoved( pEntry1 );
            break;
        case SvListAction::CLEARING:
            ActionClear();
            ModelHasCleared(); // sic! for compatibility reasons!
            break;
        case SvListAction::CLEARED:
            break;
        case SvListAction::INVALIDATE_ENTRY:
            // no action for the base class
            ModelHasEntryInvalidated( pEntry1 );
            break;
        case SvListAction::RESORTED:
            bVisPositionsValid = false;
            break;
        case SvListAction::RESORTING:
            break;
        case SvListAction::REVERSING:
            break;
        case SvListAction::REVERSED:
            bVisPositionsValid = false;
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
    SvDataTable::const_iterator itr = maDataTable.find(pEntry);
    DBG_ASSERT(itr != maDataTable.end(),"Entry not in Table");
    if (itr == maDataTable.end())
        return false;
    return itr->second->IsExpanded();
}

bool SvListView::IsSelected( SvTreeListEntry* pEntry ) const
{
    DBG_ASSERT(pEntry,"IsExpanded:No Entry");
    SvDataTable::const_iterator itr = maDataTable.find(pEntry );
    if (itr == maDataTable.end())
        return false;
    return itr->second->IsSelected();
}

void SvListView::SetEntryFocus( SvTreeListEntry* pEntry, bool bFocus )
{
    DBG_ASSERT(pEntry,"SetEntryFocus:No Entry");
    SvDataTable::iterator itr = maDataTable.find(pEntry);
    DBG_ASSERT(itr != maDataTable.end(),"Entry not in Table");
    itr->second->SetFocus(bFocus);
}

const SvViewDataEntry* SvListView::GetViewData( const SvTreeListEntry* pEntry ) const
{
    SvDataTable::const_iterator itr = maDataTable.find( const_cast<SvTreeListEntry*>(pEntry) );
    if (itr == maDataTable.end())
        return NULL;
    return itr->second;
}

SvViewDataEntry* SvListView::GetViewData( SvTreeListEntry* pEntry )
{
    SvDataTable::iterator itr = maDataTable.find( pEntry );
    DBG_ASSERT(itr != maDataTable.end(),"Entry not in model or wrong view");
    return itr->second;
}

sal_Int32 SvTreeList::Compare(const SvTreeListEntry* pLeft, const SvTreeListEntry* pRight) const
{
    if( aCompareLink.IsSet())
    {
        SvSortData aSortData;
        aSortData.pLeft = pLeft;
        aSortData.pRight = pRight;
        return aCompareLink.Call( &aSortData );
    }
    return 0;
}

void SvTreeList::Resort()
{
    Broadcast( SvListAction::RESORTING );
    bAbsPositionsValid = false;
    ResortChildren( pRootItem );
    Broadcast( SvListAction::RESORTED );
}

namespace {

class SortComparator : public std::binary_function<SvTreeListEntry,SvTreeListEntry,bool>
{
    SvTreeList& mrList;
public:

    SortComparator( SvTreeList& rList ) : mrList(rList) {}

    bool operator() ( const SvTreeListEntry& rLeft, const SvTreeListEntry& rRight ) const
    {
        return mrList.Compare(&rLeft, &rRight) < 0;
    }
};

}

void SvTreeList::ResortChildren( SvTreeListEntry* pParent )
{
    DBG_ASSERT(pParent,"Parent not set");

    if (pParent->maChildren.empty())
        return;

    SortComparator aComp(*this);
    pParent->maChildren.sort(aComp);

    // Recursively sort child entries.
    SvTreeListEntries::iterator it = pParent->maChildren.begin(), itEnd = pParent->maChildren.end();
    for (; it != itEnd; ++it)
    {
        SvTreeListEntry& r = *it;
        ResortChildren(&r);
    }

    SetListPositions(pParent->maChildren); // correct list position in target list
}

void SvTreeList::Reverse()
{
    Broadcast(SvListAction::REVERSING);
    bAbsPositionsValid = false;
    ReverseChildren(pRootItem);
    Broadcast(SvListAction::REVERSED);
}

void SvTreeList::ReverseChildren( SvTreeListEntry* pParent )
{
    DBG_ASSERT(pParent,"Parent not set");

    if (pParent->maChildren.empty())
        return;

    std::reverse(pParent->maChildren.base().begin(), pParent->maChildren.base().end());
    // Recursively sort child entries.
    SvTreeListEntries::iterator it = pParent->maChildren.begin(), itEnd = pParent->maChildren.end();
    for (; it != itEnd; ++it)
    {
        SvTreeListEntry& r = *it;
        ReverseChildren(&r);
    }

    SetListPositions(pParent->maChildren); // correct list position in target list
}

void SvTreeList::GetInsertionPos( SvTreeListEntry* pEntry, SvTreeListEntry* pParent,
    sal_uLong& rPos )
{
    DBG_ASSERT(pEntry,"No Entry");

    if( eSortMode == SortNone )
        return;

    rPos = TREELIST_ENTRY_NOTFOUND;
    const SvTreeListEntries& rChildList = GetChildList(pParent);

    if (!rChildList.empty())
    {
        long i = 0;
        long j = rChildList.size()-1;
        long k;
        sal_Int32 nCompare = 1;

        do
        {
            k = (i+j)/2;
            const SvTreeListEntry* pTempEntry = &rChildList[k];
            nCompare = Compare( pEntry, pTempEntry );
            if( eSortMode == SortDescending && nCompare != 0 )
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
}

bool SvTreeList::HasChildren( const SvTreeListEntry* pEntry ) const
{
    if ( !pEntry )
        pEntry = pRootItem;

    return !pEntry->maChildren.empty();
}

bool SvTreeList::HasParent( const SvTreeListEntry* pEntry ) const
{
    return pEntry->pParent != pRootItem;
}

SvTreeListEntry* SvTreeList::GetEntry( SvTreeListEntry* pParent, sal_uLong nPos ) const
{   if ( !pParent )
        pParent = pRootItem;
    SvTreeListEntry* pRet = 0;
    if (nPos < pParent->maChildren.size())
        pRet = &pParent->maChildren[nPos];
    return pRet;
}

SvTreeListEntry* SvTreeList::GetEntry( sal_uLong nRootPos ) const
{
    SvTreeListEntry* pRet = 0;
    if ( nEntryCount && nRootPos < pRootItem->maChildren.size())
        pRet = &pRootItem->maChildren[nRootPos];
    return pRet;
}

const SvTreeListEntries& SvTreeList::GetChildList( SvTreeListEntry* pParent ) const
{
    if ( !pParent )
        pParent = pRootItem;
    return pParent->maChildren;
}

SvTreeListEntries& SvTreeList::GetChildList( SvTreeListEntry* pParent )
{
    if ( !pParent )
        pParent = pRootItem;
    return pParent->maChildren;
}

const SvTreeListEntry* SvTreeList::GetParent( const SvTreeListEntry* pEntry ) const
{
    const SvTreeListEntry* pParent = pEntry->pParent;
    if (pParent == pRootItem)
        pParent = NULL;
    return pParent;
}

SvTreeListEntry* SvTreeList::GetParent( SvTreeListEntry* pEntry )
{
    SvTreeListEntry* pParent = pEntry->pParent;
    if (pParent == pRootItem)
        pParent = NULL;
    return pParent;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
