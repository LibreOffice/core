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

#ifndef INCLUDED_SVTOOLS_TREELIST_HXX
#define INCLUDED_SVTOOLS_TREELIST_HXX

#include <svtools/svtdllapi.h>
#include <svtools/treelistentries.hxx>
#include <svtools/viewdataentry.hxx>

#include <tools/solar.h>
#include <tools/link.hxx>
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

class SvTreeListEntry;
class SvListView;

enum SvSortMode { SortAscending, SortDescending, SortNone };

// For the return values of Sortlink:
// See International::Compare( pLeft, pRight )
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
    void                SelectAll( SvListView*,sal_Bool bSelect ); // Does not call Select Handler
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

    // rPos is not changed for SortModeNone
    SVT_DLLPRIVATE void GetInsertionPos(
                            SvTreeListEntry* pEntry,
                            SvTreeListEntry* pParent,
                            sal_uLong& rPos
                        );

    SVT_DLLPRIVATE void ResortChildren( SvTreeListEntry* pParent );

    SvTreeList(const SvTreeList&); // disabled
    SvTreeList& operator= (const SvTreeList&); // disabled

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

    // Notify all Listeners
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

    // Entries need to be in the same Model!
    void                Move( SvTreeListEntry* pSource, SvTreeListEntry* pTarget );

    // Creates ChildList if needed
    sal_uLong           Move( SvTreeListEntry* pSource, SvTreeListEntry* pTargetParent, sal_uLong nListPos);
    void                Copy( SvTreeListEntry* pSource, SvTreeListEntry* pTarget );
    sal_uLong           Copy( SvTreeListEntry* pSource, SvTreeListEntry* pTargetParent, sal_uLong nListPos);

    bool Remove( const SvTreeListEntry* pEntry );
    void                Clear();

    bool HasChildren( const SvTreeListEntry* pEntry ) const;
    bool HasParent( const SvTreeListEntry* pEntry ) const;

    bool                IsChild(const SvTreeListEntry* pParent, const SvTreeListEntry* pChild) const;
    SvTreeListEntry*        GetEntry( SvTreeListEntry* pParent, sal_uLong nPos ) const;
    SvTreeListEntry*        GetEntry( sal_uLong nRootPos ) const;
    SvTreeListEntry*        GetEntryAtAbsPos( sal_uLong nAbsPos ) const;

    const SvTreeListEntry* GetParent( const SvTreeListEntry* pEntry ) const;
    SvTreeListEntry* GetParent( SvTreeListEntry* pEntry );

    SvTreeListEntry*        GetRootLevelParent( SvTreeListEntry* pEntry ) const;
    const SvTreeListEntries& GetChildList( SvTreeListEntry* pParent ) const;
    SvTreeListEntries& GetChildList( SvTreeListEntry* pParent );

    std::pair<SvTreeListEntries::iterator, SvTreeListEntries::iterator>
        GetChildIterators(SvTreeListEntry* pParent);

    sal_uLong GetAbsPos( const SvTreeListEntry* pEntry ) const;
    sal_uLong GetRelPos( const SvTreeListEntry* pChild ) const;

    sal_uLong GetChildCount( const SvTreeListEntry* pParent ) const;
    sal_uInt16 GetDepth( const SvTreeListEntry* pEntry ) const;
    bool IsAtRootDepth( const SvTreeListEntry* pEntry ) const;

    // The Model calls the Clone Link to clone Entries.
    // Thus we do not need to derive from the Model if we derive from SvTreeListEntry.
    // Declaration of the Clone Handler:
    // DECL_LINK(CloneHdl,SvTreeListEntry*);
    // The Handler needs to return a SvTreeListEntry*
    SvTreeListEntry*        Clone( SvTreeListEntry* pEntry, sal_uLong& nCloneCount ) const;
    void                SetCloneLink( const Link& rLink )
    { aCloneLink=rLink; }

    const Link&         GetCloneLink() const
    { return aCloneLink; }

    virtual SvTreeListEntry*    CloneEntry( SvTreeListEntry* pSource ) const; // Calls the Clone Link
    virtual SvTreeListEntry*    CreateEntry() const; // To create Entries

    sal_uInt16          GetRefCount() const { return nRefCount; }
    void                SetRefCount( sal_uInt16 nRef ) { nRefCount = nRef; }

    void                SetSortMode( SvSortMode eMode ) { eSortMode = eMode; }
    SvSortMode          GetSortMode() const { return eSortMode; }
    sal_Int32           Compare(const SvTreeListEntry* pLeft, const SvTreeListEntry* pRight) const;
    void                SetCompareHdl( const Link& rLink ) { aCompareLink = rLink; }
    const Link&         GetCompareHdl() const { return aCompareLink; }
    void                Resort();
};

class SVT_DLLPUBLIC SvListView
{
    friend class SvTreeList;

    typedef boost::ptr_map<SvTreeListEntry*, SvViewDataEntry> SvDataTable;

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

    void                ExpandListEntry( SvTreeListEntry* pParent )
    { pModel->Expand((SvListView*)this,pParent); }

    void                CollapseListEntry( SvTreeListEntry* pParent )
    { pModel->Collapse((SvListView*)this,pParent); }

    sal_Bool            SelectListEntry( SvTreeListEntry* pEntry, sal_Bool bSelect )
    { return pModel->Select((SvListView*)this,pEntry,bSelect); }

public:
                        SvListView();   // Sets the Model to 0
    virtual             ~SvListView();
    void                Clear();
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

    // Does not call the Select Handler
    virtual void        SelectAll( sal_Bool bSelect, sal_Bool )
    { pModel->SelectAll((SvListView*)this, bSelect); }

    sal_Bool            IsEntryVisible( SvTreeListEntry* pEntry ) const
    { return pModel->IsEntryVisible((SvListView*)this,pEntry); }

    sal_Bool            IsExpanded( SvTreeListEntry* pEntry ) const;
    sal_Bool            IsSelected( SvTreeListEntry* pEntry ) const;
    void                SetEntryFocus( SvTreeListEntry* pEntry, sal_Bool bFocus );
    const SvViewDataEntry*         GetViewData( const SvTreeListEntry* pEntry ) const;
    SvViewDataEntry*         GetViewData( SvTreeListEntry* pEntry );
    sal_Bool            HasViewData() const
    { return maDataTable.size() > 1; }  // There's always a ROOT

    virtual SvViewDataEntry* CreateViewData( SvTreeListEntry* pEntry );
    virtual void        InitViewData( SvViewDataEntry*, SvTreeListEntry* pEntry );

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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
