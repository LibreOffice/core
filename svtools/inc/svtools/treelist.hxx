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
#include <tools/solar.h>
#include <tools/list.hxx>

#include <tools/table.hxx>
#include <tools/link.hxx>
#include <tools/string.hxx>
#include <tools/debug.hxx>

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

#define SV_TREELIST_ROOT_ENTRY  (SvListEntry*)0
#define SV_TREELIST_ERROR        0xFFFFFFFF

// Entryflags, die an der View haengen
#define SVLISTENTRYFLAG_SELECTED        0x0001
#define SVLISTENTRYFLAG_EXPANDED        0x0002
#define SVLISTENTRYFLAG_FOCUSED         0x0004
#define SVLISTENTRYFLAG_CURSORED        0x0008
#define SVLISTENTRYFLAG_NOT_SELECTABLE  0x0010

class SvListEntry;

class SvTreeEntryList : public List // SvEntryListStd
{
public:
    SvTreeEntryList(USHORT nInitPos=16, USHORT nResize=16 )
            : List( nInitPos, nResize )
    {}
    SvTreeEntryList(USHORT BlockSize, USHORT InitSize, USHORT Resize )
        : List(BlockSize, InitSize, Resize )
    {}

    void DestroyAll();
};

class SVT_DLLPUBLIC SvListEntry
{
friend class SvTreeList;
friend class SvListView;

private:
    SvListEntry*        pParent;
    SvTreeEntryList*    pChilds;
    ULONG               nAbsPos;
    ULONG               nListPos;

    void                SetListPositions();
    void                InvalidateChildrensListPositions()
                        {
                            nListPos |= 0x80000000;
                        }
public:
                        SvListEntry();
                        SvListEntry( const SvListEntry& );
    virtual             ~SvListEntry();
    BOOL                HasChilds() { return (BOOL)(pChilds!=0); }
    BOOL                HasChildListPos() const
    {
        if( pParent && !(pParent->nListPos & 0x80000000) )
            return TRUE;
        else return FALSE;
    }
    ULONG               GetChildListPos() const
    {
        if( pParent && (pParent->nListPos & 0x80000000) )
            pParent->SetListPositions();
        return ( nListPos & 0x7fffffff );
    }
    virtual void        Clone( SvListEntry* pSource );
};

class SvListView;

class SvViewData
{
friend class SvTreeList;
friend class SvListView;

    ULONG       nVisPos;
protected:
    USHORT      nFlags;
public:
    SvViewData();
    SvViewData( const SvViewData& );
    virtual ~SvViewData();

    BOOL IsSelected() const { return (BOOL)(nFlags&SVLISTENTRYFLAG_SELECTED)!=0; }
    BOOL IsExpanded() const { return (BOOL)(nFlags&SVLISTENTRYFLAG_EXPANDED)!=0; }
    BOOL HasFocus() const { return (BOOL)(nFlags&SVLISTENTRYFLAG_FOCUSED)!=0; }
    void SetFocus( BOOL bFocus)
    {
        if ( !bFocus )
            nFlags &= (~SVLISTENTRYFLAG_FOCUSED);
        else
            nFlags |= SVLISTENTRYFLAG_FOCUSED;
    }
    BOOL IsCursored() const { return (BOOL)(nFlags&SVLISTENTRYFLAG_CURSORED)!=0; }
    void SetCursored( BOOL bCursored )
    {
        if ( !bCursored )
            nFlags &= (~SVLISTENTRYFLAG_CURSORED);
        else
            nFlags |= SVLISTENTRYFLAG_CURSORED;
    }

    USHORT GetFlags() const { return nFlags; }

    void SetSelectable( bool bSelectable )
    {
        if( bSelectable )
            nFlags &= (~SVLISTENTRYFLAG_NOT_SELECTABLE);
        else
            nFlags |= SVLISTENTRYFLAG_NOT_SELECTABLE;
    }
    bool IsSelectable() const { return (bool)(nFlags&SVLISTENTRYFLAG_NOT_SELECTABLE)==0; }
};

enum SvSortMode { SortAscending, SortDescending, SortNone };

// Rueckgabewerte Sortlink:
// siehe International::Compare( pLeft, pRight )
// ( Compare(a,b) ==> b.Compare(a) ==> strcmp(a,b) )
struct SvSortData
{
    SvListEntry* pLeft;
    SvListEntry* pRight;
};

class SVT_DLLPUBLIC SvTreeList
{
    friend class SvListView;

    List            aViewList;
    ULONG           nEntryCount;

    Link            aCloneLink;
    Link            aCompareLink;
    SvSortMode      eSortMode;

    USHORT          nRefCount;

    BOOL            bAbsPositionsValid;

    SvListEntry*    FirstVisible() const { return First(); }
    SvListEntry*    NextVisible( const SvListView*,SvListEntry* pEntry, USHORT* pDepth=0 ) const;
    SvListEntry*    PrevVisible( const SvListView*,SvListEntry* pEntry, USHORT* pDepth=0 ) const;
    SvListEntry*    LastVisible( const SvListView*,USHORT* pDepth=0 ) const;
    SvListEntry*    NextVisible( const SvListView*,SvListEntry* pEntry, USHORT& rDelta ) const;
    SvListEntry*    PrevVisible( const SvListView*,SvListEntry* pEntry, USHORT& rDelta ) const;

    BOOL            IsEntryVisible( const SvListView*,SvListEntry* pEntry ) const;
    SvListEntry*    GetEntryAtVisPos( const SvListView*,ULONG nVisPos ) const;
    ULONG           GetVisiblePos( const SvListView*,SvListEntry* pEntry ) const;
    ULONG           GetVisibleCount( const SvListView* ) const;
    ULONG           GetVisibleChildCount( const SvListView*,SvListEntry* pParent ) const;

    SvListEntry*    FirstSelected( const SvListView*) const;
    SvListEntry*    NextSelected( const SvListView*,SvListEntry* pEntry ) const;
    SvListEntry*    PrevSelected( const SvListView*,SvListEntry* pEntry ) const;
    SvListEntry*    LastSelected( const SvListView*) const;

    BOOL            Select( SvListView*,SvListEntry* pEntry, BOOL bSelect=TRUE );
    ULONG           SelectChilds( SvListView*,SvListEntry* pParent, BOOL bSelect );
    void            SelectAll( SvListView*,BOOL bSelect ); // ruft nicht Select-Hdl
    ULONG           GetChildSelectionCount( const SvListView*,SvListEntry* pParent ) const;

    void            Expand( SvListView*,SvListEntry* pParent );
    void            Collapse( SvListView*,SvListEntry* pParent );

    SVT_DLLPRIVATE void             SetAbsolutePositions();
    SVT_DLLPRIVATE SvTreeEntryList*CloneChilds( SvTreeEntryList* pChilds,
                                 SvListEntry* pNewParent,
                                 ULONG& nCloneCount ) const;
    SVT_DLLPRIVATE void         SetListPositions( SvTreeEntryList* );

    // rPos wird bei SortModeNone nicht geaendert
    SVT_DLLPRIVATE void         GetInsertionPos( SvListEntry* pEntry, SvListEntry* pParent,
                        ULONG& rPos );
    SVT_DLLPRIVATE void         ResortChilds( SvListEntry* pParent );

protected:

    SvListEntry*      pRootItem;

public:

    SvTreeList();
    virtual ~SvTreeList();

    void            InsertView( SvListView* );
    void            RemoveView( SvListView* );
    ULONG           GetViewCount() const { return aViewList.Count(); }
    SvListView*     GetView(ULONG nPos) const {return (SvListView*)aViewList.GetObject(nPos);}
    void            Broadcast( USHORT nActionId, SvListEntry* pEntry1=0,
                        SvListEntry* pEntry2=0, ULONG nPos=0 );
    // informiert alle Listener
    void            InvalidateEntry( SvListEntry* );

    ULONG           GetEntryCount() const { return nEntryCount; }
    SvListEntry*    First() const;
    SvListEntry*    Next( SvListEntry* pEntry, USHORT* pDepth=0 ) const;
    SvListEntry*    Prev( SvListEntry* pEntry, USHORT* pDepth=0 ) const;
    SvListEntry*    Last( USHORT* pDepth=0 ) const;

    SvListEntry*    FirstChild( SvListEntry* pParent ) const;
    SvListEntry*    NextSibling( SvListEntry* pEntry ) const;
    SvListEntry*    PrevSibling( SvListEntry* pEntry ) const;
    SvListEntry*    LastSibling( SvListEntry* pEntry ) const;

    ULONG           Insert( SvListEntry* pEntry,SvListEntry* pPar,ULONG nPos=LIST_APPEND);
    ULONG           Insert( SvListEntry* pEntry,ULONG nRootPos = LIST_APPEND ) { return Insert(pEntry, pRootItem, nRootPos ); }
    void            InsertTree( SvListEntry* pTree, SvListEntry* pTarget );
    void            InsertTree( SvListEntry* pTree, SvListEntry* pTargetParent,
                                ULONG nListPos );
    // Entries muessen im gleichen Model stehen!
    void            Move( SvListEntry* pSource, SvListEntry* pTarget );
    // erzeugt ggf. Child-List
    ULONG           Move( SvListEntry* pSource, SvListEntry* pTargetParent,
                          ULONG nListPos);
    void            Copy( SvListEntry* pSource, SvListEntry* pTarget );
    ULONG           Copy( SvListEntry* pSource, SvListEntry* pTargetParent,
                          ULONG nListPos);

    BOOL            Remove( SvListEntry* pEntry );
    void            Clear();

    BOOL            HasChilds( SvListEntry* pEntry ) const;
    BOOL            HasParent( SvListEntry* pEntry ) const  { return (BOOL)(pEntry->pParent!=pRootItem); }
    BOOL            IsChild( SvListEntry* pParent, SvListEntry* pChild ) const;
    BOOL            IsInChildList( SvListEntry* pParent, SvListEntry* pChild) const;
    SvListEntry*    GetEntry( SvListEntry* pParent, ULONG nPos ) const;
    SvListEntry*    GetEntry( ULONG nRootPos ) const;
    SvListEntry*    GetEntryAtAbsPos( ULONG nAbsPos ) const;
    SvListEntry*    GetParent( SvListEntry* pEntry ) const;
    SvListEntry*    GetRootLevelParent( SvListEntry* pEntry ) const;
    SvTreeEntryList* GetChildList( SvListEntry* pParent ) const;

    ULONG           GetAbsPos( SvListEntry* pEntry ) const;
    ULONG           GetRelPos( SvListEntry* pChild ) const { return pChild->GetChildListPos(); }
    ULONG           GetChildCount( SvListEntry* pParent ) const;
    USHORT          GetDepth( SvListEntry* pEntry ) const;
    BOOL            IsAtRootDepth( SvListEntry* pEntry ) const { return (BOOL)(pEntry->pParent==pRootItem); }

    // das Model ruft zum Clonen von Entries den Clone-Link auf,
    // damit man sich nicht vom Model ableiten muss, wenn man
    // sich von SvListEntry ableitet.
    // Deklaration des Clone-Handlers:
    // DECL_LINK(CloneHdl,SvListEntry*);
    // der Handler muss einen SvListEntry* zurueckgeben
    SvListEntry*    Clone( SvListEntry* pEntry, ULONG& nCloneCount ) const;
    void            SetCloneLink( const Link& rLink ) { aCloneLink=rLink; }
    const Link&     GetCloneLink() const { return aCloneLink; }
    virtual SvListEntry* CloneEntry( SvListEntry* ) const; // ruft den Clone-Link
    virtual SvListEntry* CreateEntry() const; // zum 'new'en von Entries

    USHORT          GetRefCount() const { return nRefCount; }
    void            SetRefCount( USHORT nRef ) { nRefCount = nRef; }

    void            SetSortMode( SvSortMode eMode ) { eSortMode = eMode; }
    SvSortMode      GetSortMode() const { return eSortMode; }
    virtual StringCompare Compare( SvListEntry*, SvListEntry* ) const;
    void            SetCompareHdl( const Link& rLink ) { aCompareLink = rLink; }
    const Link&     GetCompareHdl() const { return aCompareLink; }
    void            Resort();

    void            CheckIntegrity() const;
};

class SVT_DLLPUBLIC SvListView
{
    friend class SvTreeList;

    ULONG           nVisibleCount;
    ULONG           nSelectionCount;
    BOOL            bVisPositionsValid;

    SVT_DLLPRIVATE void         InitTable();
    SVT_DLLPRIVATE void         ClearTable();
    SVT_DLLPRIVATE void         RemoveViewData( SvListEntry* pParent );

protected:
    Table           aDataTable;  // Mapping SvListEntry -> ViewData
    SvTreeList*     pModel;

    void ActionMoving( SvListEntry* pEntry,SvListEntry* pTargetPrnt,ULONG nChildPos);
    void ActionMoved( SvListEntry* pEntry,SvListEntry* pTargetPrnt,ULONG nChildPos);
    void ActionInserted( SvListEntry* pEntry );
    void ActionInsertedTree( SvListEntry* pEntry );
    void ActionRemoving( SvListEntry* pEntry );
    void ActionRemoved( SvListEntry* pEntry );
    void ActionClear();

public:

    SvListView();   // !!! setzt das Model auf 0
    SvListView( SvTreeList* pModel );
    virtual ~SvListView();
    void            Clear();
    SvTreeList*     GetModel() const { return pModel; }
    virtual void    SetModel( SvTreeList* );
    virtual void    ModelNotification( USHORT nActionId, SvListEntry* pEntry1,
                        SvListEntry* pEntry2, ULONG nPos );

    ULONG           GetVisibleCount() const { return pModel->GetVisibleCount( (SvListView*)this );}
    SvListEntry*    FirstVisible() const { return pModel->FirstVisible(); }
    SvListEntry*    NextVisible( SvListEntry* pEntry, USHORT* pDepth=0 ) const {return pModel->NextVisible(this,pEntry,pDepth); }
    SvListEntry*    PrevVisible( SvListEntry* pEntry, USHORT* pDepth=0 ) const {return pModel->PrevVisible(this,pEntry,pDepth); }
    SvListEntry*    LastVisible( USHORT* pDepth=0 ) const { return pModel->LastVisible(this,pDepth);}
    SvListEntry*    NextVisible( SvListEntry* pEntry, USHORT& rDelta ) const { return pModel->NextVisible(this,pEntry,rDelta); }
    SvListEntry*    PrevVisible( SvListEntry* pEntry, USHORT& rDelta ) const { return pModel->PrevVisible(this,pEntry,rDelta); }

    ULONG           GetSelectionCount() const { return nSelectionCount; }
    SvListEntry*    FirstSelected() const { return pModel->FirstSelected(this);}
    SvListEntry*    NextSelected( SvListEntry* pEntry ) const { return pModel->NextSelected(this,pEntry); }
    SvListEntry*    PrevSelected( SvListEntry* pEntry ) const { return pModel->PrevSelected(this,pEntry); }
    SvListEntry*    LastSelected() const { return pModel->LastSelected(this); }
    SvListEntry*    GetEntryAtVisPos( ULONG nVisPos ) const { return pModel->GetEntryAtVisPos((SvListView*)this,nVisPos); }
    ULONG           GetVisiblePos( SvListEntry* pEntry ) const { return pModel->GetVisiblePos((SvListView*)this,pEntry); }

    ULONG           GetVisibleChildCount(SvListEntry* pParent ) const { return pModel->GetVisibleChildCount((SvListView*)this,pParent); }
    ULONG           GetChildSelectionCount( SvListEntry* pParent ) const { return pModel->GetChildSelectionCount((SvListView*)this,pParent); }
    void            Expand( SvListEntry* pParent ) { pModel->Expand((SvListView*)this,pParent); }
    void            Collapse( SvListEntry* pParent ) { pModel->Collapse((SvListView*)this,pParent); }
    BOOL            Select( SvListEntry* pEntry, BOOL bSelect=TRUE ) { return pModel->Select((SvListView*)this,pEntry,bSelect); }
    ULONG           SelectChilds( SvListEntry* pParent, BOOL bSelect ) { return pModel->SelectChilds((SvListView*)this,pParent, bSelect); }
    // ruft nicht Select-Hdl
    virtual void    SelectAll( BOOL bSelect, BOOL ) { pModel->SelectAll((SvListView*)this, bSelect); }
    BOOL            IsEntryVisible( SvListEntry* pEntry ) const { return pModel->IsEntryVisible((SvListView*)this,pEntry); }
    BOOL            IsExpanded( SvListEntry* pEntry ) const;
    BOOL            IsSelected( SvListEntry* pEntry ) const;
    BOOL            HasEntryFocus( SvListEntry* pEntry ) const;
    void            SetEntryFocus( SvListEntry* pEntry, BOOL bFocus ) const;
    SvViewData*     GetViewData( SvListEntry* pEntry ) const;
    BOOL            HasViewData() const { return aDataTable.Count() > 1;}   // eine ROOT gibts immer
    virtual SvViewData* CreateViewData( SvListEntry* pEntry );
    virtual void InitViewData( SvViewData*, SvListEntry* pEntry );

    virtual void    ModelHasCleared();
    virtual void    ModelHasInserted( SvListEntry* pEntry );
    virtual void    ModelHasInsertedTree( SvListEntry* pEntry );
    virtual void    ModelIsMoving( SvListEntry* pSource, SvListEntry* pTargetParent,
                                ULONG nPos  );
    virtual void    ModelHasMoved( SvListEntry* pSource );
    virtual void    ModelIsRemoving( SvListEntry* pEntry );
    virtual void    ModelHasRemoved( SvListEntry* pEntry );
    virtual void    ModelHasEntryInvalidated( SvListEntry* pEntry );
};

inline BOOL SvListView::IsExpanded( SvListEntry* pEntry ) const
{
    DBG_ASSERT(pEntry,"IsExpanded:No Entry");
    SvViewData* pData = (SvViewData*)aDataTable.Get( (ULONG)pEntry );
    DBG_ASSERT(pData,"Entry not in Table");
    return pData->IsExpanded();
}
inline BOOL SvListView::IsSelected( SvListEntry* pEntry ) const
{
    DBG_ASSERT(pEntry,"IsExpanded:No Entry");
    SvViewData* pData = (SvViewData*)aDataTable.Get( (ULONG)pEntry );
    DBG_ASSERT(pData,"Entry not in Table");
    return pData->IsSelected();
}
inline BOOL SvListView::HasEntryFocus( SvListEntry* pEntry ) const
{
    DBG_ASSERT(pEntry,"IsExpanded:No Entry");
    SvViewData* pData = (SvViewData*)aDataTable.Get( (ULONG)pEntry );
    DBG_ASSERT(pData,"Entry not in Table");
    return pData->HasFocus();
}
inline void SvListView::SetEntryFocus( SvListEntry* pEntry, BOOL bFocus ) const
{
    DBG_ASSERT(pEntry,"SetEntryFocus:No Entry");
    SvViewData* pData = (SvViewData*)aDataTable.Get( (ULONG)pEntry );
    DBG_ASSERT(pData,"Entry not in Table");
    pData->SetFocus(bFocus);
}

inline SvViewData* SvListView::GetViewData( SvListEntry* pEntry ) const
{
#ifndef DBG_UTIL
    return (SvViewData*)aDataTable.Get( (ULONG)pEntry );
#else
    SvViewData* pResult = (SvViewData*)aDataTable.Get( (ULONG)pEntry );
    DBG_ASSERT(pResult,"Entry not in model or wrong view");
    return pResult;
#endif
}

inline BOOL SvTreeList::HasChilds( SvListEntry* pEntry ) const
{
    if ( !pEntry )
        pEntry = pRootItem;
    return (BOOL)(pEntry->pChilds != 0);
}

inline SvListEntry* SvTreeList::GetEntry( SvListEntry* pParent, ULONG nPos ) const
{   if ( !pParent )
        pParent = pRootItem;
    SvListEntry* pRet = 0;
    if ( pParent->pChilds )
        pRet = (SvListEntry*)(pParent->pChilds->GetObject(nPos));
    return pRet;
}

inline SvListEntry* SvTreeList::GetEntry( ULONG nRootPos ) const
{
    SvListEntry* pRet;
    if ( nEntryCount )
        pRet = (SvListEntry*)(pRootItem->pChilds->GetObject(nRootPos));
    else
        pRet = 0;
    return pRet;
}

inline SvTreeEntryList* SvTreeList::GetChildList( SvListEntry* pParent ) const
{
    if ( !pParent )
        pParent = pRootItem;
    return pParent->pChilds;
}

inline SvListEntry* SvTreeList::GetParent( SvListEntry* pEntry ) const
{
    SvListEntry* pParent = pEntry->pParent;
    if ( pParent==pRootItem )
        pParent = 0;
    return pParent;
}

#define DECLARE_SVTREELIST( ClassName, Type )                                   \
class ClassName : public SvTreeList                                         \
{                                                                           \
public:                                                                     \
    Type        First() const                                               \
                    { return (Type)SvTreeList::First(); }                     \
    Type        Next( SvListEntry* pEntry, USHORT* pDepth=0 ) const           \
                    { return (Type)SvTreeList::Next(pEntry,pDepth); }         \
    Type        Prev( SvListEntry* pEntry, USHORT* pDepth=0 ) const           \
                    { return (Type)SvTreeList::Prev(pEntry,pDepth); }         \
    Type        Last( USHORT* pDepth=0 ) const                              \
                    { return (Type)SvTreeList::Last(pDepth); }              \
                                                                            \
    Type        Clone( SvListEntry* pEntry, ULONG& nCloneCount ) const        \
                    { return (Type)SvTreeList::Clone(pEntry,nCloneCount); }   \
    Type        GetEntry( SvListEntry* pParent, ULONG nPos ) const            \
                    { return (Type)SvTreeList::GetEntry(pParent,nPos); }      \
    Type        GetEntry( ULONG nRootPos ) const                            \
                    { return (Type)SvTreeList::GetEntry(nRootPos); }          \
    Type        GetParent( SvListEntry* pEntry ) const                        \
                    { return (Type)SvTreeList::GetParent(pEntry); }           \
    using SvTreeList::FirstChild;                                             \
    Type        FirstChild( Type pParent ) const                            \
                    { return (Type)SvTreeList::FirstChild(pParent); }           \
    using SvTreeList::NextSibling;                                             \
    Type        NextSibling( Type pEntry ) const                            \
                    { return (Type)SvTreeList::NextSibling(pEntry); }           \
    using SvTreeList::PrevSibling;                                             \
    Type        PrevSibling( Type pEntry ) const                            \
                    { return (Type)SvTreeList::PrevSibling(pEntry); }           \
    using SvTreeList::LastSibling;                                             \
    Type        LastSibling( Type pEntry ) const                            \
                    { return (Type)SvTreeList::LastSibling(pEntry); }           \
    Type        GetEntryAtAbsPos( ULONG nAbsPos ) const                     \
                    { return (Type)SvTreeList::GetEntryAtAbsPos( nAbsPos); }    \
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
