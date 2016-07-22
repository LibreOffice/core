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
#include <tools/contnr.hxx>

#include <limits.h>
#include <vector>

enum class SvListAction
{
    INSERTED         = 1,
    REMOVING         = 2,
    REMOVED          = 3,
    MOVING           = 4,
    MOVED            = 5,
    CLEARING         = 6,
    INSERTED_TREE    = 7,
    INVALIDATE_ENTRY = 8,
    RESORTING        = 9,
    RESORTED         = 10,
    CLEARED          = 11,
    REVERSING        = 12,
    REVERSED         = 13
};

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

    Link<SvTreeListEntry*, SvTreeListEntry*>  aCloneLink;
    Link<const SvSortData&, sal_Int32>        aCompareLink;
    SvSortMode          eSortMode;

    sal_uInt16          nRefCount;

    bool                bAbsPositionsValid;

    bool mbEnableInvalidate;

    SvTreeListEntry*        FirstVisible() const { return First(); }
    SvTreeListEntry*        NextVisible( const SvListView*,SvTreeListEntry* pEntry, sal_uInt16* pDepth=nullptr ) const;
    SvTreeListEntry*        PrevVisible( const SvListView*,SvTreeListEntry* pEntry ) const;
    SvTreeListEntry*        LastVisible( const SvListView* ) const;
    SvTreeListEntry*        NextVisible( const SvListView*,SvTreeListEntry* pEntry, sal_uInt16& rDelta ) const;
    SvTreeListEntry*        PrevVisible( const SvListView*,SvTreeListEntry* pEntry, sal_uInt16& rDelta ) const;

    bool               IsEntryVisible( const SvListView*,SvTreeListEntry* pEntry ) const;
    SvTreeListEntry*    GetEntryAtVisPos( const SvListView*,sal_uLong nVisPos ) const;
    sal_uLong           GetVisiblePos( const SvListView*,SvTreeListEntry* pEntry ) const;
    sal_uLong           GetVisibleCount( SvListView* ) const;
    sal_uLong           GetVisibleChildCount( const SvListView*,SvTreeListEntry* pParent ) const;

    SvTreeListEntry*        FirstSelected( const SvListView*) const;
    SvTreeListEntry*        NextSelected( const SvListView*,SvTreeListEntry* pEntry ) const;
    SvTreeListEntry*        PrevSelected( const SvListView*,SvTreeListEntry* pEntry ) const;
    SvTreeListEntry*        LastSelected( const SvListView*) const;

    static bool         Select( SvListView*,SvTreeListEntry* pEntry, bool bSelect=true );
    void                SelectAll( SvListView*, bool bSelect ); // Does not call Select Handler
    sal_uLong           GetChildSelectionCount( const SvListView*,SvTreeListEntry* pParent ) const;

    static void         Expand( SvListView*,SvTreeListEntry* pParent );
    static void         Collapse( SvListView*,SvTreeListEntry* pParent );

    SVT_DLLPRIVATE void SetAbsolutePositions();

    SVT_DLLPRIVATE void CloneChildren(
        SvTreeListEntries& rDst, sal_uLong& rCloneCount, SvTreeListEntries& rSrc, SvTreeListEntry& rNewParent) const;

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
    SVT_DLLPRIVATE void ReverseChildren( SvTreeListEntry* pParent );

    SvTreeList(const SvTreeList&) = delete;
    SvTreeList& operator= (const SvTreeList&) = delete;

protected:
    SvTreeListEntry*        pRootItem;

public:

                        SvTreeList();
    virtual             ~SvTreeList();

    void                InsertView( SvListView* );
    void                RemoveView( SvListView* );

    SvListView*         GetView( sal_uLong nPos ) const
    { return ( nPos < aViewList.size() ) ? aViewList[ nPos ] : nullptr; }

    void                Broadcast(
                            SvListAction nActionId,
                            SvTreeListEntry* pEntry1=nullptr,
                            SvTreeListEntry* pEntry2=nullptr,
                            sal_uLong nPos=0
                        );

    void                EnableInvalidate( bool bEnable );

    // Notify all Listeners
    void                InvalidateEntry( SvTreeListEntry* );

    sal_uLong           GetEntryCount() const { return nEntryCount; }
    SvTreeListEntry*    First() const;
    SvTreeListEntry*    Next( SvTreeListEntry* pEntry, sal_uInt16* pDepth=nullptr ) const;
    SvTreeListEntry*    Prev( SvTreeListEntry* pEntry ) const;
    SvTreeListEntry*    Last() const;

    SvTreeListEntry*    FirstChild( SvTreeListEntry* pParent ) const;
    static SvTreeListEntry*    NextSibling( SvTreeListEntry* pEntry );
    static SvTreeListEntry*    PrevSibling( SvTreeListEntry* pEntry );
    static SvTreeListEntry*    LastSibling( SvTreeListEntry* pEntry );

    sal_uLong           Insert( SvTreeListEntry* pEntry,SvTreeListEntry* pPar,sal_uLong nPos = TREELIST_APPEND);
    sal_uLong           Insert( SvTreeListEntry* pEntry,sal_uLong nRootPos = TREELIST_APPEND )
    { return Insert(pEntry, pRootItem, nRootPos ); }

    void                InsertTree( SvTreeListEntry* pTree, SvTreeListEntry* pTargetParent, sal_uLong nListPos );

    // Entries need to be in the same Model!
    void                Move( SvTreeListEntry* pSource, SvTreeListEntry* pTarget );

    // Creates ChildList if needed
    sal_uLong           Move( SvTreeListEntry* pSource, SvTreeListEntry* pTargetParent, sal_uLong nListPos);
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
    static sal_uLong GetRelPos( const SvTreeListEntry* pChild );

    sal_uLong GetChildCount( const SvTreeListEntry* pParent ) const;
    sal_uInt16 GetDepth( const SvTreeListEntry* pEntry ) const;
    bool IsAtRootDepth( const SvTreeListEntry* pEntry ) const;

    // The Model calls the Clone Link to clone Entries.
    // Thus we do not need to derive from the Model if we derive from SvTreeListEntry.
    // The Handler needs to return a SvTreeListEntry*
    SvTreeListEntry*    Clone( SvTreeListEntry* pEntry, sal_uLong& nCloneCount ) const;
    void                SetCloneLink( const Link<SvTreeListEntry*,SvTreeListEntry*>& rLink )
    { aCloneLink=rLink; }

    const Link<SvTreeListEntry*,SvTreeListEntry*>&       GetCloneLink() const
    { return aCloneLink; }

    SvTreeListEntry*    CloneEntry( SvTreeListEntry* pSource ) const; // Calls the Clone Link
    static SvTreeListEntry* CreateEntry(); // To create Entries

    sal_uInt16          GetRefCount() const { return nRefCount; }
    void                SetRefCount( sal_uInt16 nRef ) { nRefCount = nRef; }

    void                SetSortMode( SvSortMode eMode ) { eSortMode = eMode; }
    SvSortMode          GetSortMode() const { return eSortMode; }
    sal_Int32           Compare(const SvTreeListEntry* pLeft, const SvTreeListEntry* pRight) const;
    void                SetCompareHdl( const Link<const SvSortData&, sal_Int32>& rLink ) { aCompareLink = rLink; }
    void                Resort();
    void                Reverse();
};

class SVT_DLLPUBLIC SvListView
{
    friend class SvTreeList;

    struct Impl;
    std::unique_ptr<Impl> m_pImpl;

protected:
    SvTreeList* pModel;

    void                ExpandListEntry( SvTreeListEntry* pParent )
    { SvTreeList::Expand(this,pParent); }

    void                CollapseListEntry( SvTreeListEntry* pParent )
    { SvTreeList::Collapse(this,pParent); }

    bool                SelectListEntry( SvTreeListEntry* pEntry, bool bSelect )
    { return SvTreeList::Select(this,pEntry,bSelect); }

public:
                        SvListView();   // Sets the Model to 0
    virtual             ~SvListView();
    void                Clear();
    virtual void        SetModel( SvTreeList* );
    virtual void        ModelNotification(
                            SvListAction nActionId,
                            SvTreeListEntry* pEntry1,
                            SvTreeListEntry* pEntry2,
                            sal_uLong nPos
                        );

    sal_uLong           GetVisibleCount() const
    { return pModel->GetVisibleCount( const_cast<SvListView*>(this) ); }

    SvTreeListEntry*        FirstVisible() const
    { return pModel->FirstVisible(); }

    SvTreeListEntry*        NextVisible( SvTreeListEntry* pEntry, sal_uInt16* pDepth=nullptr ) const
    { return pModel->NextVisible(this,pEntry,pDepth); }

    SvTreeListEntry*        PrevVisible( SvTreeListEntry* pEntry ) const
    { return pModel->PrevVisible(this,pEntry); }

    SvTreeListEntry*        LastVisible() const
    { return pModel->LastVisible(this); }

    SvTreeListEntry*        NextVisible( SvTreeListEntry* pEntry, sal_uInt16& rDelta ) const
    { return pModel->NextVisible(this,pEntry,rDelta); }

    SvTreeListEntry*        PrevVisible( SvTreeListEntry* pEntry, sal_uInt16& rDelta ) const
    { return pModel->PrevVisible(this,pEntry,rDelta); }

    sal_uLong           GetSelectionCount() const;

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
    { return pModel->GetEntryAtVisPos(this,nVisPos); }

    sal_uLong           GetAbsPos( SvTreeListEntry* pEntry ) const
    { return pModel->GetAbsPos(pEntry); }

    sal_uLong           GetVisiblePos( SvTreeListEntry* pEntry ) const
    { return pModel->GetVisiblePos(this,pEntry); }

    sal_uLong           GetVisibleChildCount(SvTreeListEntry* pParent ) const
    { return pModel->GetVisibleChildCount(this,pParent); }

    sal_uLong           GetChildSelectionCount( SvTreeListEntry* pParent ) const
    { return pModel->GetChildSelectionCount(this,pParent); }

    // Does not call the Select Handler
    virtual void        SelectAll( bool bSelect, bool /*bPaint*/ = true )
    { pModel->SelectAll(this, bSelect); }

    bool               IsEntryVisible( SvTreeListEntry* pEntry ) const
    { return pModel->IsEntryVisible(this,pEntry); }

    bool                IsExpanded( SvTreeListEntry* pEntry ) const;
    bool                IsSelected( SvTreeListEntry* pEntry ) const;
    void                SetEntryFocus( SvTreeListEntry* pEntry, bool bFocus );
    const SvViewDataEntry*         GetViewData( const SvTreeListEntry* pEntry ) const;
    SvViewDataEntry*         GetViewData( SvTreeListEntry* pEntry );
    bool                HasViewData() const;

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
