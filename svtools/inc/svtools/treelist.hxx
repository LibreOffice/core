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

#ifndef _SVTREELIST_HXX
#define _SVTREELIST_HXX

#include "svtools/svtdllapi.h"
#include "svtools/treelistentry.hxx"
#include <tools/solar.h>
#include <tools/link.hxx>
#include <tools/string.hxx>
#include <tools/debug.hxx>

#include <limits.h>
#include <vector>
#include <boost/ptr_container/ptr_map.hpp>

#define LISTACTION_INSERTED         1
#define LISTACTION_REMOVING         2
#define LISTACTION_REMOVED          3
#define LISTACTION_MOVING           4
#define LISTACTION_MOVED            5
#define LISTACTION_CLEARING         6
#define LISTACTION_INSERTED_TREE    7
#define LISTACTION_INVALIDATE_ENTRY 8
#define LISTACTION_RESORTING        9
#define LISTACTION_RESORTED         10
#define LISTACTION_CLEARED          11

// Entryflags, die an der View haengen
#define SVLISTENTRYFLAG_SELECTED        0x0001
#define SVLISTENTRYFLAG_EXPANDED        0x0002
#define SVLISTENTRYFLAG_FOCUSED         0x0004
#define SVLISTENTRYFLAG_CURSORED        0x0008
#define SVLISTENTRYFLAG_NOT_SELECTABLE  0x0010

class SvTreeListEntry;
class SvListView;

class SvViewData
{
friend class SvTreeList;
friend class SvListView;

    sal_uLong           nVisPos;
protected:
    sal_uInt16          nFlags;
public:
                        SvViewData();
                        SvViewData( const SvViewData& );
    virtual             ~SvViewData();

    sal_Bool            IsSelected() const
    { return (sal_Bool)(nFlags & SVLISTENTRYFLAG_SELECTED) != 0; }

    sal_Bool            IsExpanded() const
    { return (sal_Bool)(nFlags & SVLISTENTRYFLAG_EXPANDED) != 0; }

    sal_Bool            HasFocus() const
    { return (sal_Bool)(nFlags & SVLISTENTRYFLAG_FOCUSED) != 0; }

    sal_Bool            IsCursored() const
    { return (sal_Bool)(nFlags & SVLISTENTRYFLAG_CURSORED) != 0; }

    bool                IsSelectable() const
    { return (bool)(nFlags & SVLISTENTRYFLAG_NOT_SELECTABLE) == 0; }

    void                SetFocus( sal_Bool bFocus)
    {
        if ( !bFocus )
            nFlags &= (~SVLISTENTRYFLAG_FOCUSED);
        else
            nFlags |= SVLISTENTRYFLAG_FOCUSED;
    }

    void                SetCursored( sal_Bool bCursored )
    {
        if ( !bCursored )
            nFlags &= (~SVLISTENTRYFLAG_CURSORED);
        else
            nFlags |= SVLISTENTRYFLAG_CURSORED;
    }

    sal_uInt16          GetFlags() const
    { return nFlags; }

    void                SetSelectable( bool bSelectable )
    {
        if( bSelectable )
            nFlags &= (~SVLISTENTRYFLAG_NOT_SELECTABLE);
        else
            nFlags |= SVLISTENTRYFLAG_NOT_SELECTABLE;
    }
};

enum SvSortMode { SortAscending, SortDescending, SortNone };

// Rueckgabewerte Sortlink:
// siehe International::Compare( pLeft, pRight )
// ( Compare(a,b) ==> b.Compare(a) ==> strcmp(a,b) )
struct SvSortData
{
    const SvTreeListEntry* pLeft;
    const SvTreeListEntry* pRight;
};

class SVT_DLLPUBLIC SvTreeList
{
    typedef std::vector<SvListView*> ListViewsType;

    friend class        SvListView;

    ListViewsType       aViewList;
    sal_uLong           nEntryCount;

    Link                aCloneLink;
    Link                aCompareLink;
    SvSortMode          eSortMode;

    sal_uInt16          nRefCount;

    sal_Bool            bAbsPositionsValid;

    SvTreeListEntry*        FirstVisible() const { return First(); }
    SvTreeListEntry*        NextVisible( const SvListView*,SvTreeListEntry* pEntry, sal_uInt16* pDepth=0 ) const;
    SvTreeListEntry*        PrevVisible( const SvListView*,SvTreeListEntry* pEntry, sal_uInt16* pDepth=0 ) const;
    SvTreeListEntry*        LastVisible( const SvListView*,sal_uInt16* pDepth=0 ) const;
    SvTreeListEntry*        NextVisible( const SvListView*,SvTreeListEntry* pEntry, sal_uInt16& rDelta ) const;
    SvTreeListEntry*        PrevVisible( const SvListView*,SvTreeListEntry* pEntry, sal_uInt16& rDelta ) const;

    sal_Bool            IsEntryVisible( const SvListView*,SvTreeListEntry* pEntry ) const;
    SvTreeListEntry*        GetEntryAtVisPos( const SvListView*,sal_uLong nVisPos ) const;
    sal_uLong           GetVisiblePos( const SvListView*,SvTreeListEntry* pEntry ) const;
    sal_uLong           GetVisibleCount( SvListView* ) const;
    sal_uLong           GetVisibleChildCount( const SvListView*,SvTreeListEntry* pParent ) const;

    SvTreeListEntry*        FirstSelected( const SvListView*) const;
    SvTreeListEntry*        NextSelected( const SvListView*,SvTreeListEntry* pEntry ) const;
    SvTreeListEntry*        PrevSelected( const SvListView*,SvTreeListEntry* pEntry ) const;
    SvTreeListEntry*        LastSelected( const SvListView*) const;

    sal_Bool            Select( SvListView*,SvTreeListEntry* pEntry, sal_Bool bSelect=sal_True );
    void                SelectAll( SvListView*,sal_Bool bSelect ); // ruft nicht Select-Hdl
    sal_uLong           GetChildSelectionCount( const SvListView*,SvTreeListEntry* pParent ) const;

    void                Expand( SvListView*,SvTreeListEntry* pParent );
    void                Collapse( SvListView*,SvTreeListEntry* pParent );

    SVT_DLLPRIVATE void SetAbsolutePositions();

    SVT_DLLPRIVATE void CloneChildren(
        SvTreeListEntries& rDst, sal_uLong& rCloneCount, SvTreeListEntries& rSrc, SvTreeListEntry* pNewParent) const;

    /**
     * Invalidate the cached position data to have them re-generated before
     * the next access.
     */
    SVT_DLLPRIVATE void SetListPositions( SvTreeListEntries& rEntries );

    // rPos wird bei SortModeNone nicht geaendert
    SVT_DLLPRIVATE void GetInsertionPos(
                            SvTreeListEntry* pEntry,
                            SvTreeListEntry* pParent,
                            sal_uLong& rPos
                        );

    SVT_DLLPRIVATE void ResortChildren( SvTreeListEntry* pParent );

protected:
    SvTreeListEntry*        pRootItem;

public:

                        SvTreeList();
    virtual             ~SvTreeList();

    void                InsertView( SvListView* );
    void                RemoveView( SvListView* );
    sal_uLong           GetViewCount() const
    { return aViewList.size(); }

    SvListView*         GetView( sal_uLong nPos ) const
    { return ( nPos < aViewList.size() ) ? aViewList[ nPos ] : NULL; }

    void                Broadcast(
                            sal_uInt16 nActionId,
                            SvTreeListEntry* pEntry1=0,
                            SvTreeListEntry* pEntry2=0,
                            sal_uLong nPos=0
                        );

    // informiert alle Listener
    void                InvalidateEntry( SvTreeListEntry* );

    sal_uLong           GetEntryCount() const { return nEntryCount; }
    SvTreeListEntry*        First() const;
    SvTreeListEntry*        Next( SvTreeListEntry* pEntry, sal_uInt16* pDepth=0 ) const;
    SvTreeListEntry*        Prev( SvTreeListEntry* pEntry, sal_uInt16* pDepth=0 ) const;
    SvTreeListEntry*        Last() const;

    SvTreeListEntry*        FirstChild( SvTreeListEntry* pParent ) const;
    SvTreeListEntry*        NextSibling( SvTreeListEntry* pEntry ) const;
    SvTreeListEntry*        PrevSibling( SvTreeListEntry* pEntry ) const;
    SvTreeListEntry*        LastSibling( SvTreeListEntry* pEntry ) const;

    sal_uLong           Insert( SvTreeListEntry* pEntry,SvTreeListEntry* pPar,sal_uLong nPos=ULONG_MAX);
    sal_uLong           Insert( SvTreeListEntry* pEntry,sal_uLong nRootPos = ULONG_MAX )
    { return Insert(pEntry, pRootItem, nRootPos ); }

    void                InsertTree( SvTreeListEntry* pTree, SvTreeListEntry* pTargetParent, sal_uLong nListPos );

    // Entries muessen im gleichen Model stehen!
    void                Move( SvTreeListEntry* pSource, SvTreeListEntry* pTarget );

    // erzeugt ggf. Child-List
    sal_uLong           Move( SvTreeListEntry* pSource, SvTreeListEntry* pTargetParent, sal_uLong nListPos);
    void                Copy( SvTreeListEntry* pSource, SvTreeListEntry* pTarget );
    sal_uLong           Copy( SvTreeListEntry* pSource, SvTreeListEntry* pTargetParent, sal_uLong nListPos);

    bool Remove( const SvTreeListEntry* pEntry );
    void                Clear();

    sal_Bool            HasChildren( SvTreeListEntry* pEntry ) const;
    sal_Bool            HasParent( SvTreeListEntry* pEntry ) const
    { return (sal_Bool)(pEntry->pParent!=pRootItem); }

    bool                IsChild(const SvTreeListEntry* pParent, const SvTreeListEntry* pChild) const;
    SvTreeListEntry*        GetEntry( SvTreeListEntry* pParent, sal_uLong nPos ) const;
    SvTreeListEntry*        GetEntry( sal_uLong nRootPos ) const;
    SvTreeListEntry*        GetEntryAtAbsPos( sal_uLong nAbsPos ) const;
    SvTreeListEntry*        GetParent( SvTreeListEntry* pEntry ) const;
    SvTreeListEntry*        GetRootLevelParent( SvTreeListEntry* pEntry ) const;
    const SvTreeListEntries& GetChildList( SvTreeListEntry* pParent ) const;
    SvTreeListEntries& GetChildList( SvTreeListEntry* pParent );

    std::pair<SvTreeListEntries::const_iterator, SvTreeListEntries::const_iterator>
        GetChildIterators(const SvTreeListEntry* pParent) const;

    std::pair<SvTreeListEntries::iterator, SvTreeListEntries::iterator>
        GetChildIterators(SvTreeListEntry* pParent);

    sal_uLong           GetAbsPos( SvTreeListEntry* pEntry ) const;
    sal_uLong           GetRelPos( SvTreeListEntry* pChild ) const
    { return pChild->GetChildListPos(); }

    sal_uLong GetChildCount( const SvTreeListEntry* pParent ) const;
    sal_uInt16 GetDepth( const SvTreeListEntry* pEntry ) const;
    bool IsAtRootDepth( const SvTreeListEntry* pEntry ) const;

    // das Model ruft zum Clonen von Entries den Clone-Link auf,
    // damit man sich nicht vom Model ableiten muss, wenn man
    // sich von SvTreeListEntry ableitet.
    // Deklaration des Clone-Handlers:
    // DECL_LINK(CloneHdl,SvTreeListEntry*);
    // der Handler muss einen SvTreeListEntry* zurueckgeben
    SvTreeListEntry*        Clone( SvTreeListEntry* pEntry, sal_uLong& nCloneCount ) const;
    void                SetCloneLink( const Link& rLink )
    { aCloneLink=rLink; }

    const Link&         GetCloneLink() const
    { return aCloneLink; }

    virtual SvTreeListEntry*    CloneEntry( SvTreeListEntry* pSource ) const; // ruft den Clone-Link
    virtual SvTreeListEntry*    CreateEntry() const; // zum 'new'en von Entries

    sal_uInt16          GetRefCount() const { return nRefCount; }
    void                SetRefCount( sal_uInt16 nRef ) { nRefCount = nRef; }

    void                SetSortMode( SvSortMode eMode ) { eSortMode = eMode; }
    SvSortMode          GetSortMode() const { return eSortMode; }
    StringCompare Compare(const SvTreeListEntry* pLeft, const SvTreeListEntry* pRight) const;
    void                SetCompareHdl( const Link& rLink ) { aCompareLink = rLink; }
    const Link&         GetCompareHdl() const { return aCompareLink; }
    void                Resort();

    void                CheckIntegrity() const;
};

class SVT_DLLPUBLIC SvListView
{
    friend class SvTreeList;

    typedef boost::ptr_map<SvTreeListEntry*, SvViewData> SvDataTable;

    sal_uLong           nVisibleCount;
    sal_uLong           nSelectionCount;
    sal_Bool            bVisPositionsValid;

    SVT_DLLPRIVATE void InitTable();
    SVT_DLLPRIVATE void RemoveViewData( SvTreeListEntry* pParent );

    SvDataTable maDataTable;  // Mapping SvTreeListEntry -> ViewData

    void                ActionMoving( SvTreeListEntry* pEntry,SvTreeListEntry* pTargetPrnt,sal_uLong nChildPos);
    void                ActionMoved( SvTreeListEntry* pEntry,SvTreeListEntry* pTargetPrnt,sal_uLong nChildPos);
    void                ActionInserted( SvTreeListEntry* pEntry );
    void                ActionInsertedTree( SvTreeListEntry* pEntry );
    void                ActionRemoving( SvTreeListEntry* pEntry );
    void                ActionRemoved( SvTreeListEntry* pEntry );
    void                ActionClear();

protected:
    SvTreeList* pModel;

public:
                        SvListView();   // !!! setzt das Model auf 0
    virtual             ~SvListView();
    void                Clear();
    SvTreeList*         GetModel() const;
    virtual void        SetModel( SvTreeList* );
    virtual void        ModelNotification(
                            sal_uInt16 nActionId,
                            SvTreeListEntry* pEntry1,
                            SvTreeListEntry* pEntry2,
                            sal_uLong nPos
                        );

    sal_uLong           GetVisibleCount() const
    { return pModel->GetVisibleCount( (SvListView*)this ); }

    SvTreeListEntry*        FirstVisible() const
    { return pModel->FirstVisible(); }

    SvTreeListEntry*        NextVisible( SvTreeListEntry* pEntry, sal_uInt16* pDepth=0 ) const
    { return pModel->NextVisible(this,pEntry,pDepth); }

    SvTreeListEntry*        PrevVisible( SvTreeListEntry* pEntry, sal_uInt16* pDepth=0 ) const
    { return pModel->PrevVisible(this,pEntry,pDepth); }

    SvTreeListEntry*        LastVisible( sal_uInt16* pDepth=0 ) const
    { return pModel->LastVisible(this,pDepth); }

    SvTreeListEntry*        NextVisible( SvTreeListEntry* pEntry, sal_uInt16& rDelta ) const
    { return pModel->NextVisible(this,pEntry,rDelta); }

    SvTreeListEntry*        PrevVisible( SvTreeListEntry* pEntry, sal_uInt16& rDelta ) const
    { return pModel->PrevVisible(this,pEntry,rDelta); }

    sal_uLong           GetSelectionCount() const
    { return nSelectionCount; }

    SvTreeListEntry* FirstSelected() const
    { return pModel->FirstSelected(this); }

    SvTreeListEntry*        NextSelected( SvTreeListEntry* pEntry ) const
    { return pModel->NextSelected(this,pEntry); }

    SvTreeListEntry*        PrevSelected( SvTreeListEntry* pEntry ) const
    { return pModel->PrevSelected(this,pEntry); }

    SvTreeListEntry*        LastSelected() const
    { return pModel->LastSelected(this); }
    SvTreeListEntry*        GetEntryAtAbsPos( sal_uLong nAbsPos ) const
    { return pModel->GetEntryAtAbsPos(nAbsPos); }

    SvTreeListEntry*        GetEntryAtVisPos( sal_uLong nVisPos ) const
    { return pModel->GetEntryAtVisPos((SvListView*)this,nVisPos); }

    sal_uLong           GetAbsPos( SvTreeListEntry* pEntry ) const
    { return pModel->GetAbsPos(pEntry); }

    sal_uLong           GetVisiblePos( SvTreeListEntry* pEntry ) const
    { return pModel->GetVisiblePos((SvListView*)this,pEntry); }

    sal_uLong           GetVisibleChildCount(SvTreeListEntry* pParent ) const
    { return pModel->GetVisibleChildCount((SvListView*)this,pParent); }

    sal_uLong           GetChildSelectionCount( SvTreeListEntry* pParent ) const
    { return pModel->GetChildSelectionCount((SvListView*)this,pParent); }

    void                Expand( SvTreeListEntry* pParent )
    { pModel->Expand((SvListView*)this,pParent); }

    void                Collapse( SvTreeListEntry* pParent )
    { pModel->Collapse((SvListView*)this,pParent); }

    sal_Bool            Select( SvTreeListEntry* pEntry, sal_Bool bSelect=sal_True )
    { return pModel->Select((SvListView*)this,pEntry,bSelect); }

    // ruft nicht Select-Hdl
    virtual void        SelectAll( sal_Bool bSelect, sal_Bool )
    { pModel->SelectAll((SvListView*)this, bSelect); }

    sal_Bool            IsEntryVisible( SvTreeListEntry* pEntry ) const
    { return pModel->IsEntryVisible((SvListView*)this,pEntry); }

    sal_Bool            IsExpanded( SvTreeListEntry* pEntry ) const;
    sal_Bool            IsSelected( SvTreeListEntry* pEntry ) const;
    sal_Bool            HasEntryFocus( SvTreeListEntry* pEntry ) const;
    void                SetEntryFocus( SvTreeListEntry* pEntry, sal_Bool bFocus );
    const SvViewData*         GetViewData( SvTreeListEntry* pEntry ) const;
    SvViewData*         GetViewData( SvTreeListEntry* pEntry );
    sal_Bool            HasViewData() const
    { return maDataTable.size() > 1; }  // eine ROOT gibts immer

    virtual SvViewData* CreateViewData( SvTreeListEntry* pEntry );
    virtual void        InitViewData( SvViewData*, SvTreeListEntry* pEntry );

    virtual void        ModelHasCleared();
    virtual void        ModelHasInserted( SvTreeListEntry* pEntry );
    virtual void        ModelHasInsertedTree( SvTreeListEntry* pEntry );
    virtual void        ModelIsMoving(
                            SvTreeListEntry* pSource,
                            SvTreeListEntry* pTargetParent,
                            sal_uLong nPos
                        );
    virtual void        ModelHasMoved( SvTreeListEntry* pSource );
    virtual void        ModelIsRemoving( SvTreeListEntry* pEntry );
    virtual void        ModelHasRemoved( SvTreeListEntry* pEntry );
    virtual void        ModelHasEntryInvalidated( SvTreeListEntry* pEntry );
};

inline sal_Bool SvListView::IsExpanded( SvTreeListEntry* pEntry ) const
{
    DBG_ASSERT(pEntry,"IsExpanded:No Entry");
    SvDataTable::const_iterator itr = maDataTable.find(pEntry);
    DBG_ASSERT(itr != maDataTable.end(),"Entry not in Table");
    return itr->second->IsExpanded();
}

inline sal_Bool SvListView::IsSelected( SvTreeListEntry* pEntry ) const
{
    DBG_ASSERT(pEntry,"IsExpanded:No Entry");
    SvDataTable::const_iterator itr = maDataTable.find(pEntry );
    DBG_ASSERT(itr != maDataTable.end(),"Entry not in Table");
    return itr->second->IsSelected();
}

inline sal_Bool SvListView::HasEntryFocus( SvTreeListEntry* pEntry ) const
{
    DBG_ASSERT(pEntry,"IsExpanded:No Entry");
    SvDataTable::const_iterator itr = maDataTable.find(pEntry );
    DBG_ASSERT(itr != maDataTable.end(),"Entry not in Table");
    return itr->second->HasFocus();
}

inline void SvListView::SetEntryFocus( SvTreeListEntry* pEntry, sal_Bool bFocus )
{
    DBG_ASSERT(pEntry,"SetEntryFocus:No Entry");
    SvDataTable::iterator itr = maDataTable.find(pEntry);
    DBG_ASSERT(itr != maDataTable.end(),"Entry not in Table");
    itr->second->SetFocus(bFocus);
}

inline const SvViewData* SvListView::GetViewData( SvTreeListEntry* pEntry ) const
{
#ifndef DBG_UTIL
    return maDataTable.find( pEntry )->second;
#else
    SvDataTable::const_iterator itr = maDataTable.find( pEntry );
    DBG_ASSERT(itr != maDataTable.end(),"Entry not in model or wrong view");
    return itr->second;
#endif
}

inline SvViewData* SvListView::GetViewData( SvTreeListEntry* pEntry )
{
#ifndef DBG_UTIL
    return maDataTable.find( pEntry )->second;
#else
    SvDataTable::iterator itr = maDataTable.find( pEntry );
    DBG_ASSERT(itr != maDataTable.end(),"Entry not in model or wrong view");
    return itr->second;
#endif
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
