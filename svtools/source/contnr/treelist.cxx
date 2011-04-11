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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#define _TREELIST_CXX

#include <svtools/treelist.hxx>


DBG_NAME(SvListEntry);

SvListEntry::SvListEntry()
{
    DBG_CTOR(SvListEntry,0);
    pChilds     = 0;
    pParent     = 0;
    nListPos    = 0;
    nAbsPos     = 0;
}

SvListEntry::SvListEntry( const SvListEntry& rEntry )
{
    DBG_CTOR(SvListEntry,0);
    pChilds  = 0;
    pParent  = 0;
    nListPos &= 0x80000000;
    nListPos |= ( rEntry.nListPos & 0x7fffffff);
    nAbsPos  = rEntry.nAbsPos;
}

SvListEntry::~SvListEntry()
{
    DBG_DTOR(SvListEntry,0);
    if ( pChilds )
    {
        pChilds->DestroyAll();
        delete pChilds;
    }
#ifdef DBG_UTIL
    pChilds     = 0;
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
    if( pChilds )
    {
        SvListEntry *pEntry = (SvListEntry*)pChilds->First();
        sal_uLong       nCur = 0;
        while ( pEntry )
        {
            pEntry->nListPos &= 0x80000000;
            pEntry->nListPos |= nCur;
            nCur++;
            pEntry = (SvListEntry*)pChilds->Next();
        }
    }
    nListPos &= (~0x80000000);
}


DBG_NAME(SvViewData);

SvViewData::SvViewData()
{
    DBG_CTOR(SvViewData,0);
    nFlags = 0;
    nVisPos = 0;
}

SvViewData::SvViewData( const SvViewData& rData )
{
    DBG_CTOR(SvViewData,0);
    nFlags  = rData.nFlags;
    nFlags &= ~( SVLISTENTRYFLAG_SELECTED | SVLISTENTRYFLAG_FOCUSED );
    nVisPos = rData.nVisPos;
}

SvViewData::~SvViewData()
{
    DBG_DTOR(SvViewData,0);
#ifdef DBG_UTIL
    nVisPos = 0x12345678;
    nFlags = 0x1234;
#endif
}

void SvTreeEntryList::DestroyAll()
{
    SvListEntry* pPtr = (SvListEntry*)First();
    while( pPtr )
    {
        delete pPtr;
        pPtr = (SvListEntry*)Next();
    }
}


/*************************************************************************
|*
|*    SvTreeList::
|*
*************************************************************************/

SvTreeList::SvTreeList()
{
    nEntryCount = 0;
    bAbsPositionsValid = sal_False;
    nRefCount = 1;
    pRootItem = new SvListEntry;
    eSortMode = SortNone;
}


/*************************************************************************
|*
|*    SvTreeList::~SvTreeList
|*
*************************************************************************/

SvTreeList::~SvTreeList()
{
    Clear();
    delete pRootItem;
#ifdef DBG_UTIL
    pRootItem = 0;
#endif
}

/*************************************************************************
|*
|*    SvTreeList::Broadcast
|*
*************************************************************************/

void SvTreeList::Broadcast( sal_uInt16 nActionId, SvListEntry* pEntry1,
    SvListEntry* pEntry2, sal_uLong nPos )
{
    sal_uLong nViewCount = aViewList.Count();
    for( sal_uLong nCurView = 0; nCurView < nViewCount; nCurView++ )
    {
        SvListView* pView = (SvListView*)aViewList.GetObject( nCurView );
        if( pView )
            pView->ModelNotification( nActionId, pEntry1, pEntry2, nPos );
    }
}

void SvTreeList::InsertView( SvListView* pView)
{
    sal_uLong nPos = aViewList.GetPos( pView );
    if ( nPos == LIST_ENTRY_NOTFOUND )
    {
        aViewList.Insert( pView, LIST_APPEND );
        nRefCount++;
    }
}

void SvTreeList::RemoveView( SvListView* pView )
{
    sal_uLong nPos = aViewList.GetPos( pView );
    if ( nPos != LIST_ENTRY_NOTFOUND )
    {
        aViewList.Remove( pView );
        nRefCount--;
    }
}


// Ein Entry ist sichtbar, wenn alle Parents expandiert sind
sal_Bool SvTreeList::IsEntryVisible( const SvListView* pView, SvListEntry* pEntry ) const
{
    DBG_ASSERT(pView&&pEntry,"IsVisible:Invalid Params");
    sal_Bool bRetVal=sal_False;
    do
    {
        if ( pEntry == pRootItem )
        {
            bRetVal=sal_True;
            break;
        }
        pEntry = pEntry->pParent;
    }  while( pView->IsExpanded( pEntry ) );
    return bRetVal;
}

sal_uInt16 SvTreeList::GetDepth( SvListEntry* pEntry ) const
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

/*************************************************************************
|*
|*    SvTreeList::
|*
*************************************************************************/

void SvTreeList::Clear()
{
    Broadcast( LISTACTION_CLEARING );
    SvTreeEntryList* pRootList = pRootItem->pChilds;
    if ( pRootList )
    {
        SvListEntry* pEntry = (SvListEntry*)(pRootList->First());
        while( pEntry )
        {
            delete pEntry;
            pEntry = (SvListEntry*)(pRootList->Next());
        }
        delete pRootItem->pChilds;
        pRootItem->pChilds = 0;
    }
    nEntryCount = 0;
    Broadcast( LISTACTION_CLEARED );
}


/*************************************************************************
|*
|*    SvTreeList::
|*
*************************************************************************/

sal_Bool SvTreeList::IsChild( SvListEntry* pParent, SvListEntry* pChild ) const
{
    if ( !pParent )
        pParent = pRootItem;

    sal_Bool bIsChild = sal_False;
    SvTreeEntryList* pList = pParent->pChilds;
    if ( !pList )
        return sal_False;
    SvListEntry* pActualChild = (SvListEntry*)(pList->First());
    while( !bIsChild && pActualChild )
    {
        if ( pActualChild == pChild )
            bIsChild = sal_True;
        else
        {
            if ( pActualChild->pChilds )
                bIsChild = IsChild( pActualChild, pChild );
            pActualChild = (SvListEntry*)(pList->Next());
        }
    }
    return bIsChild;
}

sal_uLong SvTreeList::Move(SvListEntry* pSrcEntry,SvListEntry* pTargetParent,sal_uLong nListPos)
{
    // pDest darf Null sein!
    DBG_ASSERT(pSrcEntry,"Entry?");
    if ( !pTargetParent )
        pTargetParent = pRootItem;
    DBG_ASSERT(pSrcEntry!=pTargetParent,"Move:Source=Target");

    Broadcast( LISTACTION_MOVING, pSrcEntry, pTargetParent, nListPos );

    if ( !pTargetParent->pChilds )
        pTargetParent->pChilds = new SvTreeEntryList;
    if ( pSrcEntry == pTargetParent )
        return pSrcEntry->GetChildListPos();

    bAbsPositionsValid = sal_False;

    SvTreeEntryList* pDstList = pTargetParent->pChilds;
    SvTreeEntryList* pSrcList = pSrcEntry->pParent->pChilds;

    // Dummy-Ptr einfuegen, weil nListPos durch das
    // folgende Remove ungueltig werden koennte
    SvListEntry* pDummy = 0; pDstList->Insert( pDummy, nListPos );

    // loeschen
    pSrcList->Remove( pSrcEntry );
    // Hat Parent noch Childs ?
    if ( pSrcList->Count() == 0 )
    {
        // Keine Childs, deshalb Child-List loeschen
        SvListEntry* pParent = pSrcEntry->pParent;
        pParent->pChilds = 0;
        delete pSrcList;
        pSrcList = 0;
    }

    // Parent umsetzen (erst hier, weil wir zum Loeschen
    // der ChildList den alten Parent noch benoetigen!)
    pSrcEntry->pParent = pTargetParent;

    pDstList->Replace( pSrcEntry, pDummy );

    // Listenpositionen in Zielliste korrigieren
    SetListPositions( pDstList );
    if ( pSrcList && (sal_uLong)pSrcList != (sal_uLong)pDstList )
        SetListPositions( pSrcList );

#ifdef CHECK_INTEGRITY
CheckIntegrity();
#endif

    sal_uLong nRetVal = pDstList->GetPos( pSrcEntry );
    DBG_ASSERT(nRetVal==pSrcEntry->GetChildListPos(),"ListPos not valid");
    Broadcast( LISTACTION_MOVED,pSrcEntry,pTargetParent,nRetVal);
    return nRetVal;
}

sal_uLong SvTreeList::Copy(SvListEntry* pSrcEntry,SvListEntry* pTargetParent,sal_uLong nListPos)
{
    // pDest darf Null sein!
    DBG_ASSERT(pSrcEntry,"Entry?");
    if ( !pTargetParent )
        pTargetParent = pRootItem;
    if ( !pTargetParent->pChilds )
        pTargetParent->pChilds = new SvTreeEntryList;

    bAbsPositionsValid = sal_False;

    sal_uLong nCloneCount = 0;
    SvListEntry* pClonedEntry = Clone( pSrcEntry, nCloneCount );
    nEntryCount += nCloneCount;

    SvTreeEntryList* pDstList = pTargetParent->pChilds;
    pClonedEntry->pParent = pTargetParent;      // Parent umsetzen
    pDstList->Insert( pClonedEntry, nListPos ); // Einfuegen
    SetListPositions( pDstList ); // Listenpositionen in Zielliste korrigieren

#ifdef CHECK_INTEGRITY
CheckIntegrity();
#endif
    Broadcast( LISTACTION_INSERTED_TREE, pClonedEntry );
    sal_uLong nRetVal = pDstList->GetPos( pClonedEntry );
    return nRetVal;
}



/*************************************************************************
|*
|*    SvTreeList::
|*
*************************************************************************/

void SvTreeList::Move( SvListEntry* pSrcEntry, SvListEntry* pDstEntry )
{
    SvListEntry* pParent;
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
        nPos++;  // UNTER (Bildschirm) pDstEntry einfuegen
    }
    Move( pSrcEntry, pParent, nPos );
}

/*************************************************************************
|*
|*    SvTreeList::
|*
*************************************************************************/

void SvTreeList::Copy( SvListEntry* pSrcEntry, SvListEntry* pDstEntry )
{
    SvListEntry* pParent;
    sal_uLong nPos;

    if ( !pDstEntry )
    {
        pParent = pRootItem;
        nPos = 0UL;
    }
    else
    {
        pParent = pDstEntry->pParent;
        nPos = pDstEntry->GetChildListPos()+1;
    }
    Copy( pSrcEntry, pParent, nPos );
}

/*************************************************************************
|*
|*    SvTreeList::
|*
*************************************************************************/
void SvTreeList::InsertTree( SvListEntry* pSrcEntry, SvListEntry* pDstEntry)
{
    SvListEntry* pParent;
    sal_uLong nPos;

    if ( !pDstEntry )
    {
        pParent = pRootItem;
        nPos = 0UL;
    }
    else
    {
        pParent = pDstEntry->pParent;
        nPos = pDstEntry->GetChildListPos()+1;
    }
    InsertTree( pSrcEntry, pParent, nPos );
}


void SvTreeList::InsertTree(SvListEntry* pSrcEntry,
    SvListEntry* pTargetParent,sal_uLong nListPos)
{
    DBG_ASSERT(pSrcEntry,"InsertTree:Entry?");
    if ( !pSrcEntry )
        return;

    if ( !pTargetParent )
        pTargetParent = pRootItem;
    if ( !pTargetParent->pChilds )
        pTargetParent->pChilds = new SvTreeEntryList;

    // Sortierung beruecksichtigen
    GetInsertionPos( pSrcEntry, pTargetParent, nListPos );

    bAbsPositionsValid = sal_False;

    pSrcEntry->pParent = pTargetParent; // Parent umsetzen
    SvTreeEntryList* pDstList = pTargetParent->pChilds;
    pDstList->Insert( pSrcEntry, nListPos ); // einfuegen
    SetListPositions(pDstList); // Listenpositionen in Zielliste korrigieren
    nEntryCount += GetChildCount( pSrcEntry );
    nEntryCount++; // der Parent ist ja auch neu

#ifdef CHECK_INTEGRITY
CheckIntegrity();
#endif
    Broadcast(LISTACTION_INSERTED_TREE, pSrcEntry );
}

SvListEntry* SvTreeList::CloneEntry( SvListEntry* pSource ) const
{
    if( aCloneLink.IsSet() )
        return (SvListEntry*)aCloneLink.Call( pSource );
    SvListEntry* pEntry = CreateEntry();
    pSource->Clone( pEntry );
    return pSource;
}

SvListEntry* SvTreeList::CreateEntry() const
{
    return new SvListEntry;
}

/*************************************************************************
|*
|*    SvTreeList::
|*
*************************************************************************/

SvListEntry* SvTreeList::Clone( SvListEntry* pEntry, sal_uLong& nCloneCount ) const
{
    SvListEntry* pClonedEntry = CloneEntry( pEntry );
    nCloneCount = 1;
    SvTreeEntryList* pChilds = pEntry->pChilds;
    if ( pChilds )
        pClonedEntry->pChilds=CloneChilds(pChilds,pClonedEntry,nCloneCount);
    return pClonedEntry;
}

/*************************************************************************
|*
|*    SvTreeList::
|*
*************************************************************************/

SvTreeEntryList* SvTreeList::CloneChilds( SvTreeEntryList* pChilds,
                                      SvListEntry* pNewParent,
                                      sal_uLong& nCloneCount ) const
{
    DBG_ASSERT(pChilds->Count(),"Childs?");
    SvTreeEntryList* pClonedChilds = new SvTreeEntryList;
    SvListEntry* pChild = (SvListEntry*)pChilds->First();
    while ( pChild )
    {
        SvListEntry* pNewChild = CloneEntry( pChild );
        nCloneCount++;
        pNewChild->pParent = pNewParent;
        SvTreeEntryList* pSubChilds = pChild->pChilds;
        if ( pSubChilds )
        {
            pSubChilds = CloneChilds( pSubChilds, pNewChild, nCloneCount );
            pNewChild->pChilds = pSubChilds;
        }

        pClonedChilds->Insert( pNewChild, LIST_APPEND );
        pChild = (SvListEntry*)pChilds->Next();
    }
    return pClonedChilds;
}


/*************************************************************************
|*
|*    SvTreeList::GetChildCount
|*
*************************************************************************/

sal_uLong SvTreeList::GetChildCount( SvListEntry* pParent ) const
{
    if ( !pParent )
        return GetEntryCount();

    if ( !pParent || !pParent->pChilds)
        return 0;
    sal_uLong nCount = 0;
    sal_uInt16 nRefDepth = GetDepth( pParent );
    sal_uInt16 nActDepth = nRefDepth;
    do
    {
        pParent = Next( pParent, &nActDepth );
        nCount++;
    } while( pParent && nRefDepth < nActDepth );
    nCount--;
    return nCount;
}

/*************************************************************************
|*
|*    SvTreeList::
|*
*************************************************************************/

sal_uLong SvTreeList::GetVisibleChildCount(const SvListView* pView, SvListEntry* pParent) const
{
    DBG_ASSERT(pView,"GetVisChildCount:No View");
    if ( !pParent )
        pParent = pRootItem;
    if ( !pParent || !pView->IsExpanded(pParent) || !pParent->pChilds )
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

sal_uLong SvTreeList::GetChildSelectionCount(const SvListView* pView,SvListEntry* pParent) const
{
    DBG_ASSERT(pView,"GetChildSelCount:No View");
    if ( !pParent )
        pParent = pRootItem;
    if ( !pParent || !pParent->pChilds)
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


/*************************************************************************
|*
|*    SvTreeList::
|*
*************************************************************************/

SvListEntry* SvTreeList::First() const
{
    if ( nEntryCount )
        return (SvListEntry*)(pRootItem->pChilds->GetObject(0));
    else
        return 0;
}

/*************************************************************************
|*
|*    SvTreeList::Next
|*
*************************************************************************/
SvListEntry* SvTreeList::Next( SvListEntry* pActEntry, sal_uInt16* pDepth ) const
{
    DBG_ASSERT( pActEntry && pActEntry->pParent, "SvTreeList::Next: invalid entry/parent!" );
    if ( !pActEntry || !pActEntry->pParent )
        return NULL;

    sal_uInt16 nDepth = 0;
    int bWithDepth = sal_False;
    if ( pDepth )
    {
        nDepth = *pDepth;
        bWithDepth = sal_True;
    }

    SvTreeEntryList* pActualList = pActEntry->pParent->pChilds;
    sal_uLong nActualPos = pActEntry->GetChildListPos();

    if ( pActEntry->pChilds /* && pActEntry->pChilds->Count() */ )
    {
        nDepth++;
        pActEntry = (SvListEntry*)(pActEntry->pChilds->GetObject(0));
        if ( bWithDepth )
            *pDepth = nDepth;
        return pActEntry;
    }

    if ( pActualList->Count() > ( nActualPos + 1 ) )
    {
        pActEntry = (SvListEntry*)(pActualList->GetObject( nActualPos + 1 ));
        if ( bWithDepth )
            *pDepth = nDepth;
        return pActEntry;
    }

    SvListEntry* pParent = pActEntry->pParent;
    nDepth--;
    while( pParent != pRootItem && pParent != 0 )
    {
        DBG_ASSERT(pParent!=0,"TreeData corrupt!");
        pActualList = pParent->pParent->pChilds;
        DBG_ASSERT(pActualList,"TreeData corrupt!");
        nActualPos = pParent->GetChildListPos();
        if ( pActualList->Count() > ( nActualPos + 1 ) )
        {
            pActEntry = (SvListEntry*)(pActualList->GetObject( nActualPos + 1 ));
            if ( bWithDepth )
                *pDepth = nDepth;
            return pActEntry;
        }
        pParent = pParent->pParent;
        nDepth--;
    }
    return 0;
}

/*************************************************************************
|*
|*    SvTreeList::Prev
|*
*************************************************************************/
SvListEntry* SvTreeList::Prev( SvListEntry* pActEntry, sal_uInt16* pDepth ) const
{
    DBG_ASSERT(pActEntry!=0,"Entry?");

    sal_uInt16 nDepth = 0;
    int bWithDepth = sal_False;
    if ( pDepth )
    {
        nDepth = *pDepth;
        bWithDepth = sal_True;
    }

    SvTreeEntryList* pActualList = pActEntry->pParent->pChilds;
    sal_uLong nActualPos = pActEntry->GetChildListPos();

    if ( nActualPos > 0 )
    {
        pActEntry = (SvListEntry*)(pActualList->GetObject( nActualPos - 1 ));
        while( pActEntry->pChilds /* && pActEntry->pChilds->Count() */ )
        {
            pActualList = pActEntry->pChilds;
            nDepth++;
            pActEntry = (SvListEntry*)(pActualList->Last());
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

/*************************************************************************
|*
|*    SvTreeList::
|*
*************************************************************************/

SvListEntry* SvTreeList::Last( sal_uInt16* /* nDepth */ ) const
{
    SvTreeEntryList* pActList = pRootItem->pChilds;
//  if ( pActList->Count() == 0 )
//      return 0;
    SvListEntry* pEntry = 0;
    while( pActList )
    {
        pEntry = (SvListEntry*)(pActList->Last());
        pActList = pEntry->pChilds;
//      if ( pActList->Count() == 0 )
//          pActList = 0;
    }
    return pEntry;
}

/*************************************************************************
|*
|*    SvTreeList::
|*
*************************************************************************/

sal_uLong SvTreeList::GetVisiblePos( const SvListView* pView, SvListEntry* pEntry ) const
{
    DBG_ASSERT(pView&&pEntry,"View/Entry?");

    if ( !pView->bVisPositionsValid )
    {
        // damit GetVisibleCount die Positionen aktualisiert
        ((SvListView*)pView)->nVisibleCount = 0;
        GetVisibleCount( pView );
    }
    SvViewData* pViewData = pView->GetViewData( pEntry );
    return pViewData->nVisPos;
}

/*************************************************************************
|*
|*    SvTreeList::
|*
*************************************************************************/

sal_uLong SvTreeList::GetVisibleCount( const SvListView* pView ) const
{
    DBG_ASSERT(pView,"GetVisCount:No View");
    if( !pView->HasViewData() )
        return 0;
    if ( pView->nVisibleCount )
        return pView->nVisibleCount;

    sal_uLong nPos = 0;
    SvListEntry* pEntry = First();  // erster Eintrag immer sichtbar
    while ( pEntry )
    {
        SvViewData* pViewData = pView->GetViewData( pEntry );
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
    ((SvListView*)pView)->nVisibleCount = nPos;
    ((SvListView*)pView)->bVisPositionsValid = sal_True;
    return nPos;
}


/*************************************************************************
|*
|*    SvTreeList::
|*
*************************************************************************/

// Funktion geht aus Geschwindigkeitsgruenden davon aus,
// das der uebergebene Eintrag bereits sichtbar ist

SvListEntry* SvTreeList::NextVisible(const SvListView* pView,SvListEntry* pActEntry,sal_uInt16* pActDepth) const
{
    DBG_ASSERT(pView,"NextVisible:No View");
    if ( !pActEntry )
        return 0;

    sal_uInt16 nDepth = 0;
    int bWithDepth = sal_False;
    if ( pActDepth )
    {
        nDepth = *pActDepth;
        bWithDepth = sal_True;
    }

    SvTreeEntryList* pActualList = pActEntry->pParent->pChilds;
    sal_uLong nActualPos = pActEntry->GetChildListPos();

    if ( pView->IsExpanded(pActEntry) )
    {
        DBG_ASSERT(pActEntry->pChilds,"Childs?");
        nDepth++;
        pActEntry = (SvListEntry*)(pActEntry->pChilds->GetObject(0));
        if ( bWithDepth )
            *pActDepth = nDepth;
        return pActEntry;
    }

    nActualPos++;
    if ( pActualList->Count() > nActualPos  )
    {
        pActEntry = (SvListEntry*)(pActualList->GetObject( nActualPos ));
        if ( bWithDepth )
            *pActDepth = nDepth;
        return pActEntry;
    }

    SvListEntry* pParent = pActEntry->pParent;
    nDepth--;
    while( pParent != pRootItem )
    {
        pActualList = pParent->pParent->pChilds;
        nActualPos = pParent->GetChildListPos();
        nActualPos++;
        if ( pActualList->Count() > nActualPos )
        {
            pActEntry = (SvListEntry*)(pActualList->GetObject( nActualPos ));
            if ( bWithDepth )
                *pActDepth = nDepth;
            return pActEntry;
        }
        pParent = pParent->pParent;
        nDepth--;
    }
    return 0;
}


/*************************************************************************
|*
|*    SvTreeList::
|*
*************************************************************************/

// Funktion geht aus Geschwindigkeitsgruenden davon aus,
// das der uebergebene Eintrag bereits sichtbar ist

SvListEntry* SvTreeList::PrevVisible(const SvListView* pView, SvListEntry* pActEntry, sal_uInt16* pActDepth) const
{
    DBG_ASSERT(pView&&pActEntry,"PrevVis:View/Entry?");

    sal_uInt16 nDepth = 0;
    int bWithDepth = sal_False;
    if ( pActDepth )
    {
        nDepth = *pActDepth;
        bWithDepth = sal_True;
    }

    SvTreeEntryList* pActualList = pActEntry->pParent->pChilds;
    sal_uLong nActualPos = pActEntry->GetChildListPos();

    if ( nActualPos > 0 )
    {
        pActEntry = (SvListEntry*)(pActualList->GetObject( nActualPos - 1 ));
        while( pView->IsExpanded(pActEntry) )
        {
            pActualList = pActEntry->pChilds;
            nDepth++;
            pActEntry = (SvListEntry*)(pActualList->Last());
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

/*************************************************************************
|*
|*    SvTreeList::
|*
*************************************************************************/

SvListEntry* SvTreeList::LastVisible( const SvListView* pView, sal_uInt16* pDepth) const
{
    DBG_ASSERT(pView,"LastVis:No View");
    SvListEntry* pEntry = Last();
    while( pEntry && !IsEntryVisible( pView, pEntry ) )
        pEntry = PrevVisible( pView, pEntry );
    if ( pEntry && pDepth )
        *pDepth = GetDepth( pEntry );
    return pEntry;
}

/*************************************************************************
|*
|*    SvTreeList::
|*
*************************************************************************/

SvListEntry* SvTreeList::NextVisible(const SvListView* pView,SvListEntry* pEntry,sal_uInt16& nDelta) const
{
    DBG_ASSERT(pView&&pEntry&&IsEntryVisible(pView,pEntry),"NextVis:Wrong Prms/!Vis");

    sal_uLong nVisPos = GetVisiblePos( pView, pEntry );
    // nDelta Eintraege vorhanden ?
    // Beispiel: 0,1,2,3,4,5,6,7,8,9 nVisPos=5 nDelta=7
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

/*************************************************************************
|*
|*    SvTreeList::
|*
*************************************************************************/

SvListEntry* SvTreeList::PrevVisible( const SvListView* pView, SvListEntry* pEntry, sal_uInt16& nDelta ) const
{
    DBG_ASSERT(pView&&pEntry&&IsEntryVisible(pView,pEntry),"PrevVis:Parms/!Vis");

    sal_uLong nVisPos = GetVisiblePos( pView, pEntry );
    // nDelta Eintraege vorhanden ?
    // Beispiel: 0,1,2,3,4,5,6,7,8,9 nVisPos=8 nDelta=20
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

/*************************************************************************
|*
|*    SvTreeList::
|*
*************************************************************************/

SvListEntry* SvTreeList::FirstSelected( const SvListView* pView) const
{
    DBG_ASSERT(pView,"FirstSel:No View");
    if( !pView )
        return 0;
    SvListEntry* pActSelEntry = First();
    while( pActSelEntry && !pView->IsSelected(pActSelEntry) )
        pActSelEntry = NextVisible( pView, pActSelEntry );
    return pActSelEntry;
}


SvListEntry* SvTreeList::FirstChild( SvListEntry* pParent ) const
{
    if ( !pParent )
        pParent = pRootItem;
    SvListEntry* pResult;
    if ( pParent->pChilds )
        pResult = (SvListEntry*)(pParent->pChilds->GetObject( 0 ));
    else
        pResult = 0;
    return pResult;
}

SvListEntry* SvTreeList::NextSibling( SvListEntry* pEntry ) const
{
    DBG_ASSERT(pEntry,"Entry?");
    if( !pEntry )
        return 0;
    SvTreeEntryList* pList = pEntry->pParent->pChilds;
//  sal_uLong nPos = pList->GetPos( pEntry );
    sal_uLong nPos = pEntry->GetChildListPos();
    nPos++;
    pEntry = (SvListEntry*)(pList->GetObject( nPos ));
    return pEntry;
}

SvListEntry* SvTreeList::PrevSibling( SvListEntry* pEntry ) const
{
    DBG_ASSERT(pEntry,"Entry?");
    if( !pEntry )
        return 0;

    SvTreeEntryList* pList = pEntry->pParent->pChilds;
    // sal_uLong nPos = pList->GetPos( pEntry );
    sal_uLong nPos = pEntry->GetChildListPos();
    if ( nPos == 0 )
        return 0;
    nPos--;
    pEntry = (SvListEntry*)(pList->GetObject( nPos ));
    return pEntry;
}


SvListEntry* SvTreeList::LastSibling( SvListEntry* pEntry ) const
{
    DBG_ASSERT(pEntry,"LastSibling:Entry?");
    if( !pEntry )
        return 0;
    SvListEntry* pSib = 0;
    SvTreeEntryList* pSibs = pEntry->pParent->pChilds;
    if ( pSibs )
        pSib = (SvListEntry*)(pSibs->Last());
    return pSib;
}



/*************************************************************************
|*
|*    SvTreeList::
|*
*************************************************************************/

SvListEntry* SvTreeList::NextSelected( const SvListView* pView, SvListEntry* pEntry ) const
{
    DBG_ASSERT(pView&&pEntry,"NextSel:View/Entry?");
    pEntry = Next( pEntry );
    while( pEntry && !pView->IsSelected(pEntry) )
        pEntry = Next( pEntry );
    return pEntry;
}

/*************************************************************************
|*
|*    SvTreeList::
|*
*************************************************************************/

SvListEntry* SvTreeList::PrevSelected( const SvListView* pView, SvListEntry* pEntry) const
{
    DBG_ASSERT(pView&&pEntry,"PrevSel:View/Entry?");
    pEntry = Prev( pEntry );
    while( pEntry && !pView->IsSelected(pEntry) )
        pEntry = Prev( pEntry );

    return pEntry;
}

/*************************************************************************
|*
|*    SvTreeList::
|*
*************************************************************************/

SvListEntry* SvTreeList::LastSelected( const SvListView* pView ) const
{
    DBG_ASSERT(pView,"LastSel:No View");
    SvListEntry* pEntry = Last();
    while( pEntry && !pView->IsSelected(pEntry) )
        pEntry = Prev( pEntry );
    return pEntry;
}

/*************************************************************************
|*
|*    SvTreeList::Insert
|*
*************************************************************************/
sal_uLong SvTreeList::Insert( SvListEntry* pEntry,SvListEntry* pParent,sal_uLong nPos )
{
    DBG_ASSERT( pEntry,"Entry?");

    if ( !pParent )
        pParent = pRootItem;


    SvTreeEntryList* pList = pParent->pChilds;
    if ( !pList )
    {
        // Parent bekommt zum erstenmal ein Kind
        pList = new SvTreeEntryList;
        pParent->pChilds = pList;
    }

    // Sortierung beruecksichtigen
    GetInsertionPos( pEntry, pParent, nPos );

    bAbsPositionsValid = sal_False;
    pEntry->pParent = pParent;

    pList->Insert( pEntry, nPos );
    nEntryCount++;
    if( nPos != LIST_APPEND && (nPos != (pList->Count()-1)) )
        SetListPositions( pList );
    else
        pEntry->nListPos = pList->Count()-1;

#ifdef CHECK_INTEGRITY
CheckIntegrity();
#endif
    Broadcast( LISTACTION_INSERTED, pEntry );
    return nPos; // pEntry->nListPos;
}

/*************************************************************************
|*
|*    SvTreeList::
|*
*************************************************************************/

sal_uLong SvTreeList::GetAbsPos( SvListEntry* pEntry) const
{
    if ( !bAbsPositionsValid )
        ((SvTreeList*)this)->SetAbsolutePositions();
    return pEntry->nAbsPos;
}

/*************************************************************************
|*
|*    SvTreeList::
|*
*************************************************************************/

void SvTreeList::SetAbsolutePositions()
{
    sal_uLong nPos = 0;
    SvListEntry* pEntry = First();
    while ( pEntry )
    {
        pEntry->nAbsPos = nPos;
        nPos++;
        pEntry = Next( pEntry );
    }
    bAbsPositionsValid = sal_True;
#ifdef CHECK_INTEGRITY
CheckIntegrity();
#endif
}


/*************************************************************************
|*
|*    SvTreeList::Expand
|*
*************************************************************************/

void SvTreeList::Expand( SvListView* pView, SvListEntry* pEntry )
{
    DBG_ASSERT(pEntry&&pView,"Expand:View/Entry?");
    if ( pView->IsExpanded(pEntry) )
        return;

    DBG_ASSERT(pEntry->pChilds,"Expand:No Childs!");

    SvViewData* pViewData = pView->GetViewData(pEntry);
    pViewData->nFlags |= SVLISTENTRYFLAG_EXPANDED;
    SvListEntry* pParent = pEntry->pParent;
    // wenn Parent sichtbar dann Statusdaten invalidieren
    if ( pView->IsExpanded( pParent ) )
    {
        pView->bVisPositionsValid = sal_False;
        pView->nVisibleCount = 0;
    }
#ifdef CHECK_INTEGRITY
CheckIntegrity();
#endif
}

/*************************************************************************
|*
|*    SvTreeList::Collapse
|*
*************************************************************************/

void SvTreeList::Collapse( SvListView* pView, SvListEntry* pEntry )
{
    DBG_ASSERT(pView&&pEntry,"Collapse:View/Entry?");
    if ( !pView->IsExpanded(pEntry) )
        return;

    DBG_ASSERT(pEntry->pChilds,"Collapse:No Childs!");

    SvViewData* pViewData = pView->GetViewData( pEntry );
    pViewData->nFlags &=(~SVLISTENTRYFLAG_EXPANDED);

    SvListEntry* pParent = pEntry->pParent;
    if ( pView->IsExpanded(pParent) )
    {
        pView->nVisibleCount = 0;
        pView->bVisPositionsValid = sal_False;
    }
#ifdef CHECK_INTEGRITY
CheckIntegrity();
#endif
}


/*************************************************************************
|*
|*    SvTreeList::
|*
*************************************************************************/

sal_Bool SvTreeList::Select( SvListView* pView, SvListEntry* pEntry, sal_Bool bSelect )
{
    DBG_ASSERT(pView&&pEntry,"Select:View/Entry?");
    SvViewData* pViewData = pView->GetViewData( pEntry );
    if ( bSelect )
    {
        if ( pViewData->IsSelected() || !pViewData->IsSelectable() )
            return sal_False;
        else
        {
            pViewData->nFlags |= SVLISTENTRYFLAG_SELECTED;
            pView->nSelectionCount++;
        }
    }
    else
    {
        if ( !pViewData->IsSelected() )
            return sal_False;
        else
        {
            pViewData->nFlags &= ~( SVLISTENTRYFLAG_SELECTED );
            pView->nSelectionCount--;
        }
    }
#ifdef CHECK_INTEGRITY
CheckIntegrity();
#endif
    return sal_True;
}

/*************************************************************************
|*
|*    SvTreeList::Remove
|*
*************************************************************************/
sal_Bool SvTreeList::Remove( SvListEntry* pEntry )
{
    DBG_ASSERT(pEntry,"Cannot remove root, use clear");

    if( !pEntry->pParent )
    {
        OSL_FAIL("Removing entry not in model!");
        // unter gewissen Umstaenden (welche?) loescht der
        // Explorer aus der View Eintraege, die er nicht in die View
        // eingefuegt hat. Da sich der Kunde fuer ein platzendes
        // Office nichts kaufen kann, fange ich diesen Fall ab.
        return sal_False;
    }

    Broadcast( LISTACTION_REMOVING, pEntry );
    sal_uLong nRemoved = 1 + GetChildCount(pEntry);
    bAbsPositionsValid = sal_False;

    SvListEntry* pParent = pEntry->pParent;
    SvTreeEntryList* pList = pParent->pChilds;
    DBG_ASSERT(pList,"Remove:No Childlist");
    sal_Bool bLastEntry = sal_False;

    if ( pEntry->HasChildListPos() )
    {
        sal_uLong nListPos = pEntry->GetChildListPos();
        bLastEntry = (nListPos == (pList->Count()-1) ) ? sal_True : sal_False;
        pList->Remove( nListPos );
    }
    else
    {
        pList->Remove( (void*) pEntry );
    }


    // moved to end of method because it is used later with Broadcast
    // delete pEntry; // loescht auch alle Childs

    if ( pList->Count() == 0 )
    {
        pParent->pChilds = 0;
        delete pList;
    }
    else
    {
        if( !bLastEntry )
            SetListPositions( pList );
    }
    nEntryCount -= nRemoved;

#ifdef CHECK_INTEGRITY
CheckIntegrity();
#endif
    Broadcast( LISTACTION_REMOVED, pEntry );

    delete pEntry; // loescht auch alle Childs
    return sal_True;
}

/*************************************************************************
|*
|*    SvTreeList::
|*
*************************************************************************/

sal_uLong SvTreeList::SelectChilds(SvListView* pView, SvListEntry* pParent,sal_Bool bSelect )
{
    DBG_ASSERT(pView&&pParent,"SelChilds:View/Parent?");
    if ( !pParent->pChilds )
        return 0;
    if ( pParent->pChilds->Count() == 0 )
        return 0;

    sal_uInt16 nRefDepth = GetDepth( pParent );
    sal_uInt16 nDepth = nRefDepth;
    sal_uLong nCount = 0;
    pParent = Next( pParent );
    do
    {
        if ( Select( pView, pParent, bSelect ) )
            nCount++; // nur die tatsaechlichen Selektierungen zaehlen
        pParent = Next( pParent, &nDepth );
    }
    while( pParent && nDepth > nRefDepth );
#ifdef CHECK_INTEGRITY
CheckIntegrity();
#endif
    return nCount;
}

void SvTreeList::SelectAll( SvListView* pView, sal_Bool bSelect )
{
    DBG_ASSERT(pView,"SelectAll:NoView");
    SvListEntry* pEntry = First();
    while ( pEntry )
    {
        SvViewData* pViewData = pView->GetViewData( pEntry );
        if ( bSelect )
            pViewData->nFlags |= SVLISTENTRYFLAG_SELECTED;
        else
            pViewData->nFlags &= (~SVLISTENTRYFLAG_SELECTED);

        pEntry = Next( pEntry );
    }
    if ( bSelect )
        pView->nSelectionCount = nEntryCount;
    else
        pView->nSelectionCount = 0;
#ifdef CHECK_INTEGRITY
CheckIntegrity();
#endif
}


SvListEntry* SvTreeList::GetEntryAtAbsPos( sal_uLong nAbsPos ) const
{
    SvListEntry* pEntry = First();
    while ( nAbsPos && pEntry )
    {
        pEntry = Next( pEntry );
        nAbsPos--;
    }
    return pEntry;
}

SvListEntry* SvTreeList::GetEntryAtVisPos( const SvListView* pView, sal_uLong nVisPos ) const
{
    DBG_ASSERT(pView,"GetEntryAtVisPos:No View");
    SvListEntry* pEntry = First();
    while ( nVisPos && pEntry )
    {
        pEntry = NextVisible( pView, pEntry );
        nVisPos--;
    }
    return pEntry;
}

void SvTreeList::SetListPositions( SvTreeEntryList* pList )
{
    if( pList->Count() )
    {
        SvListEntry* pEntry = (SvListEntry*)(pList->GetObject(0));
        if( pEntry->pParent )
            pEntry->pParent->InvalidateChildrensListPositions();
    }
    /*
    sal_uLong nListPos = 0;
    SvListEntry* pEntry = (SvListEntry*)(pList->First());
    while( pEntry )
    {
        pEntry->nListPos = nListPos;
        nListPos++;
        pEntry = (SvListEntry*)(pList->Next());
    }
    */
}


void SvTreeList::InvalidateEntry( SvListEntry* pEntry )
{
    Broadcast( LISTACTION_INVALIDATE_ENTRY, pEntry );
}

sal_Bool SvTreeList::IsInChildList( SvListEntry* pParent, SvListEntry* pChild) const
{
    if ( !pParent )
        pParent = pRootItem;
    sal_Bool bIsChild = sal_False;
    if ( pParent->pChilds )
        bIsChild = (sal_Bool)(pParent->pChilds->GetPos(pChild) != LIST_ENTRY_NOTFOUND);
    return bIsChild;
}


void lcl_CheckList( SvTreeEntryList* pList )
{
    SvListEntry* pEntry = (SvListEntry*)(pList->First());
    sal_uLong nPos = 0;
    while ( pEntry )
    {
        DBG_ASSERT(pEntry->GetChildListPos()==nPos,"Wrong ListPos");
        pEntry = (SvListEntry*)(pList->Next());
        nPos++;
    }
}

void SvTreeList::CheckIntegrity() const
{
    sal_uLong nMyEntryCount = 0;
    if ( pRootItem->pChilds )
    {
        lcl_CheckList( pRootItem->pChilds );
        SvListEntry* pEntry = First();
        while( pEntry )
        {
            nMyEntryCount++;
            if ( pEntry->pChilds )
                lcl_CheckList( pEntry->pChilds );
            pEntry = Next( pEntry );
        }
    }
    DBG_ASSERT(nMyEntryCount==GetEntryCount(),"Entry count invalid");
}

SvListEntry* SvTreeList::GetRootLevelParent( SvListEntry* pEntry ) const
{
    DBG_ASSERT(pEntry,"GetRootLevelParent:No Entry");
    SvListEntry* pCurParent = 0;
    if ( pEntry )
    {
        pCurParent = pEntry->pParent;
        if ( pCurParent == pRootItem )
            return pEntry; // ist sein eigener Parent
        while( pCurParent && pCurParent->pParent != pRootItem )
            pCurParent = pCurParent->pParent;
    }
    return pCurParent;
}




//*************************************************************************
//*************************************************************************
//*************************************************************************
//*************************************************************************
//*************************************************************************
//*************************************************************************
//*************************************************************************
//*************************************************************************

DBG_NAME(SvListView);

SvListView::SvListView( SvTreeList* pModell )
{
    DBG_CTOR(SvListView,0);
    pModel = 0;
    nSelectionCount = 0;
    nVisibleCount = 0;
    bVisPositionsValid = sal_False;
    SetModel( pModell );
}

SvListView::SvListView()
{
    DBG_CTOR(SvListView,0);
    pModel = 0;
    nSelectionCount = 0;
    nVisibleCount = 0;
    bVisPositionsValid = sal_False;
}


SvListView::~SvListView()
{
    DBG_DTOR(SvListView,0);
    ClearTable();
}

void SvListView::InitTable()
{
    DBG_CHKTHIS(SvListView,0);
    DBG_ASSERT(pModel,"InitTable:No Model");
    DBG_ASSERT(!nSelectionCount&&!nVisibleCount&&!bVisPositionsValid,"InitTable: Not cleared!");

    if( aDataTable.Count() )
    {
        DBG_ASSERT(aDataTable.Count()==1,"InitTable: TableCount != 1");
        // die im Clear fuer die Root allozierten View-Daten loeschen
        // Achtung: Das zu dem RootEntry (und damit auch der Entry)
        // gehoerende Model kann bereits geloescht sein!
        SvViewData* pViewData = (SvViewData*)aDataTable.GetObject( 0 );
        delete pViewData;
        aDataTable.Clear();
    }

    SvListEntry* pEntry;
    SvViewData* pViewData;

    // RootEntry einfuegen
    pEntry = pModel->pRootItem;
    pViewData = new SvViewData;
    pViewData->nFlags = SVLISTENTRYFLAG_EXPANDED;
    aDataTable.Insert( (sal_uLong)pEntry, pViewData );
    // Jetzt alle anderen Entries
    pEntry = pModel->First();
    while( pEntry )
    {
        pViewData = CreateViewData( pEntry );
        DBG_ASSERT(pViewData,"InitTable:No ViewData");
        InitViewData( pViewData, pEntry );
        aDataTable.Insert( (sal_uLong)pEntry, pViewData );
        pEntry = pModel->Next( pEntry );
    }
}

SvViewData* SvListView::CreateViewData( SvListEntry* )
{
    DBG_CHKTHIS(SvListView,0);
    return new SvViewData;
}

void SvListView::ClearTable()
{
    DBG_CHKTHIS(SvListView,0);
    SvViewData* pViewData = (SvViewData*)aDataTable.First();
    while( pViewData )
    {
        delete pViewData;
        pViewData = (SvViewData*)aDataTable.Next();
    }
    aDataTable.Clear();
}

void SvListView::Clear()
{
    ClearTable();
    nSelectionCount = 0;
    nVisibleCount = 0;
    bVisPositionsValid = sal_False;
    if( pModel )
    {
        // RootEntry einfuegen
        SvListEntry* pEntry = pModel->pRootItem;
        SvViewData* pViewData = new SvViewData;
        pViewData->nFlags = SVLISTENTRYFLAG_EXPANDED;
        aDataTable.Insert( (sal_uLong)pEntry, pViewData );
    }
}

void SvListView::SetModel( SvTreeList* pNewModel )
{
    DBG_CHKTHIS(SvListView,0);
    sal_Bool bBroadcastCleared = sal_False;
    if ( pModel )
    {
        pModel->RemoveView( this );
        bBroadcastCleared = sal_True;
        ModelNotification( LISTACTION_CLEARING,0,0,0 );
        if ( pModel->GetRefCount() == 0 )
            delete pModel;
    }
    pModel = pNewModel;
    InitTable();
    pNewModel->InsertView( this );
    if( bBroadcastCleared )
        ModelNotification( LISTACTION_CLEARED,0,0,0 );
}


void SvListView::ModelHasCleared()
{
    DBG_CHKTHIS(SvListView,0);
}

void SvListView::ModelHasInserted( SvListEntry* )
{
    DBG_CHKTHIS(SvListView,0);
}

void SvListView::ModelHasInsertedTree( SvListEntry* )
{
    DBG_CHKTHIS(SvListView,0);
}

void SvListView::ModelIsMoving( SvListEntry* /*  pSource */ ,
    SvListEntry* /* pTargetParent */ ,  sal_uLong /* nPos */    )
{
    DBG_CHKTHIS(SvListView,0);
}


void SvListView::ModelHasMoved( SvListEntry* )
{
    DBG_CHKTHIS(SvListView,0);
}

void SvListView::ModelIsRemoving( SvListEntry* )
{
    DBG_CHKTHIS(SvListView,0);
}

void SvListView::ModelHasRemoved( SvListEntry* )
{
    DBG_CHKTHIS(SvListView,0);
}

void SvListView::ModelHasEntryInvalidated( SvListEntry*)
{
    DBG_CHKTHIS(SvListView,0);
}

void SvListView::ActionMoving( SvListEntry* pEntry,SvListEntry*,sal_uLong)
{
    DBG_CHKTHIS(SvListView,0);
    SvListEntry* pParent = pEntry->pParent;
    DBG_ASSERT(pParent,"Model not consistent");
    if( pParent != pModel->pRootItem && pParent->pChilds->Count() == 1 )
    {
        SvViewData* pViewData = (SvViewData*)aDataTable.Get( (sal_uLong)pParent );
        pViewData->nFlags &= (~SVLISTENTRYFLAG_EXPANDED);
    }
    // vorlaeufig
    nVisibleCount = 0;
    bVisPositionsValid = sal_False;
}

void SvListView::ActionMoved( SvListEntry* /* pEntry */ ,
                            SvListEntry* /* pTargetPrnt */ ,
                            sal_uLong /* nChildPos */ )
{
    DBG_CHKTHIS(SvListView,0);
    nVisibleCount = 0;
    bVisPositionsValid = sal_False;
}

void SvListView::ActionInserted( SvListEntry* pEntry )
{
    DBG_CHKTHIS(SvListView,0);
    DBG_ASSERT(pEntry,"Insert:No Entry");
    SvViewData* pData = CreateViewData( pEntry );
    InitViewData( pData, pEntry );
    #ifdef DBG_UTIL
    sal_Bool bSuccess =
    #endif
        aDataTable.Insert( (sal_uLong)pEntry, pData );
    DBG_ASSERT(bSuccess,"Entry already in View");
    if ( nVisibleCount && pModel->IsEntryVisible( this, pEntry ))
    {
        nVisibleCount = 0;
        bVisPositionsValid = sal_False;
    }
}

void SvListView::ActionInsertedTree( SvListEntry* pEntry )
{
    DBG_CHKTHIS(SvListView,0);
    if ( pModel->IsEntryVisible( this, pEntry ))
    {
        nVisibleCount = 0;
        bVisPositionsValid = sal_False;
    }
    // ueber Entry und seine Childs iterieren
    SvListEntry* pCurEntry = pEntry;
    sal_uInt16 nRefDepth = pModel->GetDepth( pCurEntry );
    while( pCurEntry )
    {
        DBG_ASSERT(aDataTable.Get((sal_uLong)pCurEntry)==0,"Entry already in Table");
        SvViewData* pViewData = CreateViewData( pCurEntry );
        DBG_ASSERT(pViewData,"No ViewData");
        InitViewData( pViewData, pEntry );
        aDataTable.Insert( (sal_uLong)pCurEntry, pViewData );
        pCurEntry = pModel->Next( pCurEntry );
        if ( pCurEntry && pModel->GetDepth(pCurEntry) <= nRefDepth)
            pCurEntry = 0;
    }
}

void SvListView::RemoveViewData( SvListEntry* pParent )
{
    SvTreeEntryList* pChilds = pParent->pChilds;
    if( pChilds )
    {
        SvListEntry* pCur = (SvListEntry*)pChilds->First();
        while( pCur )
        {
            SvViewData* pViewData = (SvViewData*)aDataTable.Get((sal_uLong)pCur);
            delete pViewData;
            aDataTable.Remove( (sal_uLong)pCur );
            if( pCur->HasChilds())
                RemoveViewData( pCur );
            pCur = (SvListEntry*)pChilds->Next();
        }
    }
}



void SvListView::ActionRemoving( SvListEntry* pEntry )
{
    DBG_CHKTHIS(SvListView,0);
    DBG_ASSERT(pEntry,"Remove:No Entry");

    SvViewData* pViewData = (SvViewData*)aDataTable.Get( (sal_uLong)pEntry );
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
    bVisPositionsValid = sal_False;

    pViewData = (SvViewData*)aDataTable.Get((sal_uLong)pEntry);
    delete pViewData;
    aDataTable.Remove( (sal_uLong)pEntry );
    RemoveViewData( pEntry );

    SvListEntry* pCurEntry = pEntry->pParent;
    if ( pCurEntry && pCurEntry != pModel->pRootItem &&
         pCurEntry->pChilds->Count() == 1 )
    {
        pViewData = (SvViewData*)aDataTable.Get((sal_uLong)pCurEntry);
        pViewData->nFlags &= (~SVLISTENTRYFLAG_EXPANDED);
    }
}

void SvListView::ActionRemoved( SvListEntry* /* pEntry  */ )
{
    DBG_CHKTHIS(SvListView,0);
}

void SvListView::ActionClear()
{
    DBG_CHKTHIS(SvListView,0);
    Clear();
}

void SvListView::ModelNotification( sal_uInt16 nActionId, SvListEntry* pEntry1,
                        SvListEntry* pEntry2, sal_uLong nPos )
{
    DBG_CHKTHIS(SvListView,0);
    switch( nActionId )
    {
        case LISTACTION_INSERTED:
            ActionInserted( pEntry1 );
            ModelHasInserted( pEntry1 );
            break;
        case LISTACTION_INSERTED_TREE:
            ActionInsertedTree( pEntry1 );
            ModelHasInsertedTree( pEntry1 );
            break;
        case LISTACTION_REMOVING:
            ModelIsRemoving( pEntry1 );
            ActionRemoving( pEntry1 );
            break;
        case LISTACTION_REMOVED:
            ActionRemoved( pEntry1 );
            ModelHasRemoved( pEntry1 );
            break;
        case LISTACTION_MOVING:
            ModelIsMoving( pEntry1, pEntry2, nPos );
            ActionMoving( pEntry1, pEntry2, nPos );
            break;
        case LISTACTION_MOVED:
            ActionMoved( pEntry1, pEntry2, nPos );
            ModelHasMoved( pEntry1 );
            break;
        case LISTACTION_CLEARING:
            ActionClear();
            ModelHasCleared(); //sic! wg. Kompatibilitaet!
            break;
        case LISTACTION_CLEARED:
            break;
        case LISTACTION_INVALIDATE_ENTRY:
            // keine Action fuer die Basisklasse
            ModelHasEntryInvalidated( pEntry1 );
            break;
        case LISTACTION_RESORTED:
            bVisPositionsValid = sal_False;
            break;
        case LISTACTION_RESORTING:
            break;
        default:
            OSL_FAIL("unknown ActionId");
    }
}

void SvListView::InitViewData( SvViewData*, SvListEntry* )
{
}

StringCompare SvTreeList::Compare( SvListEntry* pLeft, SvListEntry* pRight) const
{
    if( aCompareLink.IsSet())
    {
        SvSortData aSortData;
        aSortData.pLeft = pLeft;
        aSortData.pRight = pRight;
        return (StringCompare)aCompareLink.Call( &aSortData );
    }
    return COMPARE_EQUAL;
}

void SvTreeList::Resort()
{
    Broadcast( LISTACTION_RESORTING );
    bAbsPositionsValid = sal_False;
    ResortChilds( pRootItem );
    Broadcast( LISTACTION_RESORTED );
}

void SvTreeList::ResortChilds( SvListEntry* pParent )
{
    DBG_ASSERT(pParent,"Parent not set");
    List* pChildList = pParent->pChilds;
    if( !pChildList )
        return;
    List aList( *pChildList );
    pChildList->Clear();

    sal_uLong nCount = aList.Count();
    for( sal_uLong nCur = 0; nCur < nCount; nCur++ )
    {
        SvListEntry* pCurEntry = (SvListEntry*)aList.GetObject( nCur );
        sal_uLong nListPos = LIST_APPEND;
        GetInsertionPos( pCurEntry, pParent, nListPos );
        pChildList->Insert( pCurEntry, nListPos );
        if( pCurEntry->pChilds )
            ResortChilds( pCurEntry );
    }
    SetListPositions( (SvTreeEntryList*)pChildList );
}

void SvTreeList::GetInsertionPos( SvListEntry* pEntry, SvListEntry* pParent,
    sal_uLong& rPos )
{
    DBG_ASSERT(pEntry,"No Entry");

    if( eSortMode == SortNone )
        return;

    rPos = LIST_APPEND;
    SvTreeEntryList* pChildList = GetChildList( pParent );

    if( pChildList && pChildList->Count() )
    {
        long i = 0;
        long j = pChildList->Count()-1;
        long k;
        StringCompare eCompare = COMPARE_GREATER;

        do
        {
            k = (i+j)/2;
            SvListEntry* pTempEntry = (SvListEntry*)(pChildList->GetObject(k));
            eCompare = Compare( pEntry, pTempEntry );
            if( eSortMode == SortDescending && eCompare != COMPARE_EQUAL )
            {
                if( eCompare == COMPARE_LESS )
                    eCompare = COMPARE_GREATER;
                else
                    eCompare = COMPARE_LESS;
            }
            if( eCompare == COMPARE_GREATER )
                i = k + 1;
            else
                j = k - 1;
        } while( (eCompare != COMPARE_EQUAL) && (i <= j) );

        if( eCompare != COMPARE_EQUAL )
        {
            if(i > ((long)pChildList->Count() - 1)) // nicht gefunden, Ende der Liste
                rPos = LIST_APPEND;
            else
                rPos = i;              // nicht gefunden, Mitte
        }
        else
            rPos = k;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
