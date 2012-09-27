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


/*
    TODO:
        - delete anchor in SelectionEngine when selecting manually
        - SelectAll( sal_False ) => only repaint the delselected entries
*/

#include <string.h>
#include <svtools/svlbox.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <vcl/svapp.hxx>
#include <vcl/accel.hxx>
#include <vcl/i18nhelp.hxx>
#include <sot/formats.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <rtl/instance.hxx>

#include <svtools/svmedit.hxx>
#include <svtools/svlbitm.hxx>

#include <set>

using namespace ::com::sun::star::accessibility;

// Drag&Drop
static SvLBox* pDDSource = NULL;
static SvLBox* pDDTarget = NULL;

DBG_NAME(SvInplaceEdit2)

#define SVLBOX_ACC_RETURN 1
#define SVLBOX_ACC_ESCAPE 2

// ***************************************************************

class MyEdit_Impl : public Edit
{
    SvInplaceEdit2* pOwner;
public:
                 MyEdit_Impl( Window* pParent, SvInplaceEdit2* pOwner );
    virtual void KeyInput( const KeyEvent& rKEvt );
    virtual void LoseFocus();
};

class MyMultiEdit_Impl : public MultiLineEdit
{
    SvInplaceEdit2* pOwner;
public:
                 MyMultiEdit_Impl( Window* pParent, SvInplaceEdit2* pOwner );
    virtual void KeyInput( const KeyEvent& rKEvt );
    virtual void LoseFocus();
};

MyEdit_Impl::MyEdit_Impl( Window* pParent, SvInplaceEdit2* _pOwner ) :

    Edit( pParent, WB_LEFT ),

    pOwner( _pOwner )

{
}

void MyEdit_Impl::KeyInput( const KeyEvent& rKEvt )
{
    if( !pOwner->KeyInput( rKEvt ))
        Edit::KeyInput( rKEvt );
}

void MyEdit_Impl::LoseFocus()
{
    pOwner->LoseFocus();
}

MyMultiEdit_Impl::MyMultiEdit_Impl( Window* pParent, SvInplaceEdit2* _pOwner )
    : MultiLineEdit( pParent,
    WB_CENTER
    ), pOwner(_pOwner)
{
}

void MyMultiEdit_Impl::KeyInput( const KeyEvent& rKEvt )
{
    if( !pOwner->KeyInput( rKEvt ))
        MultiLineEdit::KeyInput( rKEvt );
}

void MyMultiEdit_Impl::LoseFocus()
{
    pOwner->LoseFocus();
}


SvInplaceEdit2::SvInplaceEdit2
(
    Window* pParent, const Point& rPos,
    const Size& rSize,
    const String& rData,
    const Link& rNotifyEditEnd,
    const Selection& rSelection,
    sal_Bool bMulti
) :

     aCallBackHdl       ( rNotifyEditEnd ),
    bCanceled           ( sal_False ),
    bAlreadyInCallBack  ( sal_False )

{
    DBG_CTOR(SvInplaceEdit2,0);

    if( bMulti )
        pEdit = new MyMultiEdit_Impl( pParent, this );
    else
        pEdit = new MyEdit_Impl( pParent, this );

    Font aFont( pParent->GetFont() );
    aFont.SetTransparent( sal_False );
    Color aColor( pParent->GetBackground().GetColor() );
    aFont.SetFillColor(aColor );
    pEdit->SetFont( aFont );
    pEdit->SetBackground( pParent->GetBackground() );
    pEdit->SetPosPixel( rPos );
    pEdit->SetSizePixel( rSize );
    pEdit->SetText( rData );
    pEdit->SetSelection( rSelection );
    pEdit->SaveValue();

    aAccReturn.InsertItem( SVLBOX_ACC_RETURN, KeyCode(KEY_RETURN) );
    aAccEscape.InsertItem( SVLBOX_ACC_ESCAPE, KeyCode(KEY_ESCAPE) );

    aAccReturn.SetActivateHdl( LINK( this, SvInplaceEdit2, ReturnHdl_Impl) );
    aAccEscape.SetActivateHdl( LINK( this, SvInplaceEdit2, EscapeHdl_Impl) );
    GetpApp()->InsertAccel( &aAccReturn );
    GetpApp()->InsertAccel( &aAccEscape );

    pEdit->Show();
    pEdit->GrabFocus();
}

SvInplaceEdit2::~SvInplaceEdit2()
{
    DBG_DTOR(SvInplaceEdit2,0);
    if( !bAlreadyInCallBack )
    {
        GetpApp()->RemoveAccel( &aAccReturn );
        GetpApp()->RemoveAccel( &aAccEscape );
    }
    delete pEdit;
}

String SvInplaceEdit2::GetSavedValue() const
{
    return pEdit->GetSavedValue();
}

void SvInplaceEdit2::Hide()
{
    pEdit->Hide();
}


IMPL_LINK_NOARG_INLINE_START(SvInplaceEdit2, ReturnHdl_Impl)
{
    DBG_CHKTHIS(SvInplaceEdit2,0);
    bCanceled = sal_False;
    CallCallBackHdl_Impl();
    return 1;
}
IMPL_LINK_NOARG_INLINE_END(SvInplaceEdit2, ReturnHdl_Impl)

IMPL_LINK_NOARG_INLINE_START(SvInplaceEdit2, EscapeHdl_Impl)
{
    DBG_CHKTHIS(SvInplaceEdit2,0);
    bCanceled = sal_True;
    CallCallBackHdl_Impl();
    return 1;
}
IMPL_LINK_NOARG_INLINE_END(SvInplaceEdit2, EscapeHdl_Impl)


sal_Bool SvInplaceEdit2::KeyInput( const KeyEvent& rKEvt )
{
    DBG_CHKTHIS(SvInplaceEdit2,0);
    KeyCode aCode = rKEvt.GetKeyCode();
    sal_uInt16 nCode = aCode.GetCode();

    switch ( nCode )
    {
        case KEY_ESCAPE:
            bCanceled = sal_True;
            CallCallBackHdl_Impl();
            return sal_True;

        case KEY_RETURN:
            bCanceled = sal_False;
            CallCallBackHdl_Impl();
            return sal_True;
    }
    return sal_False;
}

void SvInplaceEdit2::StopEditing( sal_Bool bCancel )
{
    DBG_CHKTHIS(SvInplaceEdit2,0);
    if ( !bAlreadyInCallBack )
    {
        bCanceled = bCancel;
        CallCallBackHdl_Impl();
    }
}

void SvInplaceEdit2::LoseFocus()
{
    DBG_CHKTHIS(SvInplaceEdit2,0);
    if ( !bAlreadyInCallBack
    && ((!Application::GetFocusWindow()) || !pEdit->IsChild( Application::GetFocusWindow()) )
    )
    {
        bCanceled = sal_False;
        aTimer.SetTimeout(10);
        aTimer.SetTimeoutHdl(LINK(this,SvInplaceEdit2,Timeout_Impl));
        aTimer.Start();
    }
}

IMPL_LINK_NOARG_INLINE_START(SvInplaceEdit2, Timeout_Impl)
{
    DBG_CHKTHIS(SvInplaceEdit2,0);
    CallCallBackHdl_Impl();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SvInplaceEdit2, Timeout_Impl)

void SvInplaceEdit2::CallCallBackHdl_Impl()
{
    DBG_CHKTHIS(SvInplaceEdit2,0);
    aTimer.Stop();
    if ( !bAlreadyInCallBack )
    {
        bAlreadyInCallBack = sal_True;
        GetpApp()->RemoveAccel( &aAccReturn );
        GetpApp()->RemoveAccel( &aAccEscape );
        pEdit->Hide();
        aCallBackHdl.Call( this );
    }
}

String SvInplaceEdit2::GetText() const
{
    return pEdit->GetText();
}

// ***************************************************************
// class SvLBoxTab
// ***************************************************************

DBG_NAME(SvLBoxTab);

SvLBoxTab::SvLBoxTab()
{
    DBG_CTOR(SvLBoxTab,0);
    nPos = 0;
    pUserData = 0;
    nFlags = 0;
}

SvLBoxTab::SvLBoxTab( long nPosition, sal_uInt16 nTabFlags )
{
    DBG_CTOR(SvLBoxTab,0);
    nPos = nPosition;
    pUserData = 0;
    nFlags = nTabFlags;
}

SvLBoxTab::SvLBoxTab( const SvLBoxTab& rTab )
{
    DBG_CTOR(SvLBoxTab,0);
    nPos = rTab.nPos;
    pUserData = rTab.pUserData;
    nFlags = rTab.nFlags;
}

SvLBoxTab::~SvLBoxTab()
{
    DBG_DTOR(SvLBoxTab,0);
}


long SvLBoxTab::CalcOffset( long nItemWidth, long nTabWidth )
{
    DBG_CHKTHIS(SvLBoxTab,0);
    long nOffset = 0;
    if ( nFlags & SV_LBOXTAB_ADJUST_RIGHT )
    {
        nOffset = nTabWidth - nItemWidth;
        if( nOffset < 0 )
            nOffset = 0;
    }
    else if ( nFlags & SV_LBOXTAB_ADJUST_CENTER )
    {
        if( nFlags & SV_LBOXTAB_FORCE )
        {
            // correct implementation of centering
            nOffset = ( nTabWidth - nItemWidth ) / 2;
            if( nOffset < 0 )
                nOffset = 0;
        }
        else
        {
            // historically grown, wrong calculation of tabs which is needed by
            // Abo-Tabbox, Tools/Options/Customize etc.
            nItemWidth++;
            nOffset = -( nItemWidth / 2 );
        }
    }
    return nOffset;
}

// ***************************************************************
// class SvLBoxItem
// ***************************************************************

DBG_NAME(SvLBoxItem);

SvLBoxItem::SvLBoxItem( SvLBoxEntry*, sal_uInt16 )
{
    DBG_CTOR(SvLBoxItem,0);
}

SvLBoxItem::SvLBoxItem()
{
    DBG_CTOR(SvLBoxItem,0);
}

SvLBoxItem::~SvLBoxItem()
{
    DBG_DTOR(SvLBoxItem,0);
}

const Size& SvLBoxItem::GetSize( SvLBox* pView,SvLBoxEntry* pEntry )
{
    DBG_CHKTHIS(SvLBoxItem,0);
    SvViewDataItem* pViewData = pView->GetViewDataItem( pEntry, this );
    return pViewData->aSize;
}

DBG_NAME(SvViewDataItem);

SvViewDataItem::SvViewDataItem()
{
    DBG_CTOR(SvViewDataItem,0);
}

SvViewDataItem::~SvViewDataItem()
{
    DBG_DTOR(SvViewDataItem,0);
}



// ***************************************************************
// class SvLBoxEntry
// ***************************************************************

DBG_NAME(SvLBoxEntry);

SvLBoxEntry::SvLBoxEntry() : aItems()
{
    DBG_CTOR(SvLBoxEntry,0);
    nEntryFlags = 0;
    pUserData = 0;
}

SvLBoxEntry::~SvLBoxEntry()
{
    DBG_DTOR(SvLBoxEntry,0);
    DeleteItems_Impl();
}

void SvLBoxEntry::DeleteItems_Impl()
{
    DBG_CHKTHIS(SvLBoxEntry,0);
    sal_uInt16 nCount = aItems.size();
    while( nCount )
    {
        nCount--;
        SvLBoxItem* pItem = aItems[ nCount ];
        delete pItem;
    }
    aItems.clear();
}


void SvLBoxEntry::AddItem( SvLBoxItem* pItem )
{
    DBG_CHKTHIS(SvLBoxEntry,0);
    aItems.push_back( pItem );
}

void SvLBoxEntry::Clone( SvListEntry* pSource )
{
    DBG_CHKTHIS(SvLBoxEntry,0);
    SvListEntry::Clone( pSource );
    SvLBoxItem* pNewItem;
    DeleteItems_Impl();
    sal_uInt16 nCount = ((SvLBoxEntry*)pSource)->ItemCount();
    sal_uInt16 nCurPos = 0;
    while( nCurPos < nCount )
    {
        SvLBoxItem* pItem = ((SvLBoxEntry*)pSource)->GetItem( nCurPos );
        pNewItem = pItem->Create();
        pNewItem->Clone( pItem );
        AddItem( pNewItem );
        nCurPos++;
    }
    pUserData = ((SvLBoxEntry*)pSource)->GetUserData();
    nEntryFlags = ((SvLBoxEntry*)pSource)->nEntryFlags;
}

void SvLBoxEntry::EnableChildrenOnDemand( sal_Bool bEnable )
{
    DBG_CHKTHIS(SvLBoxEntry,0);
    if ( bEnable )
        nEntryFlags |= SV_ENTRYFLAG_CHILDREN_ON_DEMAND;
    else
        nEntryFlags &= (~SV_ENTRYFLAG_CHILDREN_ON_DEMAND);
}

void SvLBoxEntry::ReplaceItem( SvLBoxItem* pNewItem, sal_uInt16 nPos )
{
    DBG_CHKTHIS(SvLBoxEntry,0);
    DBG_ASSERT(pNewItem,"ReplaceItem:No Item");
    SvLBoxItem* pOld = GetItem( nPos );
    if ( pOld )
    {
        aItems[ nPos ] = pNewItem;
        delete pOld;
    }
}

SvLBoxItem* SvLBoxEntry::GetFirstItem( sal_uInt16 nId )
{
    sal_uInt16 nCount = aItems.size();
    sal_uInt16 nCur = 0;
    SvLBoxItem* pItem;
    while( nCur < nCount )
    {
        pItem = GetItem( nCur );
        if( pItem->IsA() == nId )
            return pItem;
        nCur++;
    }
    return 0;
}

SvLBoxEntry* SvLBoxTreeList::First() const
{
    return (SvLBoxEntry*)SvTreeList::First();
}

SvLBoxEntry* SvLBoxTreeList::Next( SvListEntry* pEntry, sal_uInt16* pDepth ) const
{
    return (SvLBoxEntry*)SvTreeList::Next(pEntry,pDepth);
}

SvLBoxEntry* SvLBoxTreeList::Prev( SvListEntry* pEntry, sal_uInt16* pDepth ) const
{
    return (SvLBoxEntry*)SvTreeList::Prev(pEntry,pDepth);
}

SvLBoxEntry* SvLBoxTreeList::Last() const
{
    return (SvLBoxEntry*)SvTreeList::Last();
}

SvLBoxEntry* SvLBoxTreeList::Clone( SvListEntry* pEntry, sal_uLong& nCloneCount ) const
{
    return (SvLBoxEntry*)SvTreeList::Clone(pEntry,nCloneCount);
}

SvLBoxEntry* SvLBoxTreeList::GetEntry( SvListEntry* pParent, sal_uLong nPos ) const
{
    return (SvLBoxEntry*)SvTreeList::GetEntry(pParent,nPos);
}

SvLBoxEntry* SvLBoxTreeList::GetEntry( sal_uLong nRootPos ) const
{
    return (SvLBoxEntry*)SvTreeList::GetEntry(nRootPos);
}

SvLBoxEntry* SvLBoxTreeList::GetParent( SvListEntry* pEntry ) const
{
    return (SvLBoxEntry*)SvTreeList::GetParent(pEntry);
}

SvLBoxEntry* SvLBoxTreeList::FirstChild( SvLBoxEntry* pParent ) const
{
    return (SvLBoxEntry*)SvTreeList::FirstChild(pParent);
}

SvLBoxEntry* SvLBoxTreeList::NextSibling( SvLBoxEntry* pEntry ) const
{
    return (SvLBoxEntry*)SvTreeList::NextSibling(pEntry);
}

SvLBoxEntry* SvLBoxTreeList::PrevSibling( SvLBoxEntry* pEntry ) const
{
    return (SvLBoxEntry*)SvTreeList::PrevSibling(pEntry);
}

SvLBoxEntry* SvLBoxTreeList::LastSibling( SvLBoxEntry* pEntry ) const
{
    return (SvLBoxEntry*)SvTreeList::LastSibling(pEntry);
}

SvLBoxEntry* SvLBoxTreeList::GetEntryAtAbsPos( sal_uLong nAbsPos ) const
{
    return (SvLBoxEntry*)SvTreeList::GetEntryAtAbsPos( nAbsPos);
}

// ***************************************************************
// class SvLBoxViewData
// ***************************************************************

DBG_NAME(SvViewDataEntry);

SvViewDataEntry::SvViewDataEntry()
    : SvViewData()
{
    DBG_CTOR(SvViewDataEntry,0);
    pItemData = 0;
}

SvViewDataEntry::~SvViewDataEntry()
{
    DBG_DTOR(SvViewDataEntry,0);
    delete [] pItemData;
}

// ***************************************************************
// struct SvLBox_Impl
// ***************************************************************
SvLBox_Impl::SvLBox_Impl( SvLBox& _rBox )
    :m_bIsEmptyTextAllowed( true )
    ,m_bEntryMnemonicsEnabled( false )
    ,m_bDoingQuickSelection( false )
    ,m_pLink( NULL )
    ,m_aMnemonicEngine( _rBox )
    ,m_aQuickSelectionEngine( _rBox )
{
}

// ***************************************************************
// class SvLBox
// ***************************************************************

DBG_NAME(SvLBox);

SvLBox::SvLBox( Window* pParent, WinBits nWinStyle  ) :
    Control( pParent, nWinStyle | WB_CLIPCHILDREN ),
    DropTargetHelper( this ), DragSourceHelper( this ), eSelMode( NO_SELECTION )
{
    DBG_CTOR(SvLBox,0);
    nDragOptions =  DND_ACTION_COPYMOVE | DND_ACTION_LINK;
    nImpFlags = 0;
    pTargetEntry = 0;
    nDragDropMode = 0;
    pLBoxImpl = new SvLBox_Impl( *this );
    SvLBoxTreeList* pTempModel = new SvLBoxTreeList;
    pTempModel->SetRefCount( 0 );
    SetModel( pTempModel );
    pModel->SetCloneLink( LINK(this, SvLBox, CloneHdl_Impl ));
    pModel->InsertView( this );
    pHdlEntry = 0;
    pEdCtrl = 0;
    SetSelectionMode( SINGLE_SELECTION );  // check if TreeListBox is called
    SetDragDropMode( SV_DRAGDROP_NONE );
    SetType(WINDOW_TREELISTBOX);
}

SvLBox::SvLBox( Window* pParent, const ResId& rResId ) :
    Control( pParent, rResId ),
    DropTargetHelper( this ), DragSourceHelper( this ), eSelMode( NO_SELECTION )
{
    DBG_CTOR(SvLBox,0);
    pTargetEntry = 0;
    nImpFlags = 0;
    pLBoxImpl = new SvLBox_Impl( *this );
    nDragOptions = DND_ACTION_COPYMOVE | DND_ACTION_LINK;
    nDragDropMode = 0;
    SvLBoxTreeList* pTempModel = new SvLBoxTreeList;
    pTempModel->SetRefCount( 0 );
    SetModel( pTempModel );
    pModel->InsertView( this );
    pHdlEntry = 0;
    pEdCtrl = 0;
    pModel->SetCloneLink( LINK(this, SvLBox, CloneHdl_Impl ));
    SetType(WINDOW_TREELISTBOX);
}

SvLBox::~SvLBox()
{
    DBG_DTOR(SvLBox,0);
    delete pEdCtrl;
    pEdCtrl = 0;
    pModel->RemoveView( this );
    if ( pModel->GetRefCount() == 0 )
    {
        pModel->Clear();
        delete pModel;
        pModel = NULL;
    }

    SvLBox::RemoveBoxFromDDList_Impl( *this );

    if( this == pDDSource )
        pDDSource = 0;
    if( this == pDDTarget )
        pDDTarget = 0;
    delete pLBoxImpl;
}

void SvLBox::SetModel( SvLBoxTreeList* pNewModel )
{
    DBG_CHKTHIS(SvLBox,0);
    // does the CleanUp
    SvListView::SetModel( pNewModel );
    pModel->SetCloneLink( LINK(this, SvLBox, CloneHdl_Impl ));
    SvLBoxEntry* pEntry = First();
    while( pEntry )
    {
        ModelHasInserted( pEntry );
        pEntry = Next( pEntry );
    }
}

void SvLBox::DisconnectFromModel()
{
    DBG_CHKTHIS(SvLBox,0);
    SvLBoxTreeList* pNewModel = new SvLBoxTreeList;
    pNewModel->SetRefCount( 0 );    // else this will never be deleted
    SvListView::SetModel( pNewModel );
}

void SvLBox::Clear()
{
    DBG_CHKTHIS(SvLBox,0);
    pModel->Clear();  // Model calls SvLBox::ModelHasCleared()
}

void SvLBox::EnableEntryMnemonics( bool _bEnable )
{
    if ( _bEnable == IsEntryMnemonicsEnabled() )
        return;

    pLBoxImpl->m_bEntryMnemonicsEnabled = _bEnable;
    Invalidate();
}

bool SvLBox::IsEntryMnemonicsEnabled() const
{
    return pLBoxImpl->m_bEntryMnemonicsEnabled;
}

sal_uInt16 SvLBox::IsA()
{
    DBG_CHKTHIS(SvLBox,0);
    return SVLISTBOX_ID_LBOX;
}

IMPL_LINK_INLINE_START( SvLBox, CloneHdl_Impl, SvListEntry*, pEntry )
{
    DBG_CHKTHIS(SvLBox,0);
    return (long)(CloneEntry((SvLBoxEntry*)pEntry));
}
IMPL_LINK_INLINE_END( SvLBox, CloneHdl_Impl, SvListEntry*, pEntry )

sal_uLong SvLBox::Insert( SvLBoxEntry* pEntry, SvLBoxEntry* pParent, sal_uLong nPos )
{
    DBG_CHKTHIS(SvLBox,0);
    sal_uLong nInsPos = pModel->Insert( pEntry, pParent, nPos );
    return nInsPos;
}

sal_uLong SvLBox::Insert( SvLBoxEntry* pEntry,sal_uLong nRootPos )
{
    DBG_CHKTHIS(SvLBox,0);
    sal_uLong nInsPos = pModel->Insert( pEntry, nRootPos );
    return nInsPos;
}

long SvLBox::ExpandingHdl()
{
    DBG_CHKTHIS(SvLBox,0);
    return aExpandingHdl.IsSet() ? aExpandingHdl.Call( this ) : 1;
}

void SvLBox::ExpandedHdl()
{
    DBG_CHKTHIS(SvLBox,0);
    aExpandedHdl.Call( this );
}

void SvLBox::SelectHdl()
{
    DBG_CHKTHIS(SvLBox,0);
    aSelectHdl.Call( this );
}

void SvLBox::DeselectHdl()
{
    DBG_CHKTHIS(SvLBox,0);
    aDeselectHdl.Call( this );
}

sal_Bool SvLBox::DoubleClickHdl()
{
    DBG_CHKTHIS(SvLBox,0);
    aDoubleClickHdl.Call( this );
    return sal_True;
}


sal_Bool SvLBox::CheckDragAndDropMode( SvLBox* pSource, sal_Int8 nAction )
{
    DBG_CHKTHIS(SvLBox,0);
    if ( pSource == this )
    {
        if ( !(nDragDropMode & (SV_DRAGDROP_CTRL_MOVE | SV_DRAGDROP_CTRL_COPY) ) )
            return sal_False; // D&D locked within list
        if( DND_ACTION_MOVE == nAction )
        {
            if ( !(nDragDropMode & SV_DRAGDROP_CTRL_MOVE) )
                 return sal_False; // no local move
        }
        else
        {
            if ( !(nDragDropMode & SV_DRAGDROP_CTRL_COPY))
                return sal_False; // no local copy
        }
    }
    else
    {
        if ( !(nDragDropMode & SV_DRAGDROP_APP_DROP ) )
            return sal_False; // no drop
        if ( DND_ACTION_MOVE == nAction )
        {
            if ( !(nDragDropMode & SV_DRAGDROP_APP_MOVE) )
                return sal_False; // no global move
        }
        else
        {
            if ( !(nDragDropMode & SV_DRAGDROP_APP_COPY))
                return sal_False; // no global copy
        }
    }
    return sal_True;
}




void SvLBox::NotifyRemoving( SvLBoxEntry* )
{
    DBG_CHKTHIS(SvLBox,0);
}

/*
    NotifyMoving/Copying
    ====================

    default behavior:

    1. target doesn't have children
        - entry becomes sibling of target. entry comes after target
          (->Window: below the target)
    2. target is an expanded parent
        - entry inserted at the beginning of the target childlist
    3. target is a collapsed parent
        - entry is inserted at the end of the target childlist
*/
#ifdef DBG_UTIL
sal_Bool SvLBox::NotifyMoving(
    SvLBoxEntry*  pTarget,       // D&D dropping position in this->GetModel()
    SvLBoxEntry*  pEntry,        // entry that we want to move, from
                                 // GetSourceListBox()->GetModel()
    SvLBoxEntry*& rpNewParent,   // new target parent
    sal_uLong&        rNewChildPos)  // position in childlist of target parent
#else
sal_Bool SvLBox::NotifyMoving(
    SvLBoxEntry*  pTarget,       // D&D dropping position in this->GetModel()
    SvLBoxEntry*,                // entry that we want to move, from
                                 // GetSourceListBox()->GetModel()
    SvLBoxEntry*& rpNewParent,   // new target parent
    sal_uLong&        rNewChildPos)  // position in childlist of target parent
#endif
{
    DBG_CHKTHIS(SvLBox,0);
    DBG_ASSERT(pEntry,"NotifyMoving:SoureEntry?");
    if( !pTarget )
    {
        rpNewParent = 0;
        rNewChildPos = 0;
        return sal_True;
    }
    if ( !pTarget->HasChildren() && !pTarget->HasChildrenOnDemand() )
    {
        // case 1
        rpNewParent = GetParent( pTarget );
        rNewChildPos = pModel->GetRelPos( pTarget ) + 1;
        rNewChildPos += nCurEntrySelPos;
        nCurEntrySelPos++;
    }
    else
    {
        // cases 2 & 3
        rpNewParent = pTarget;
        if( IsExpanded(pTarget))
            rNewChildPos = 0;
        else
            rNewChildPos = LIST_APPEND;
    }
    return sal_True;
}

sal_Bool SvLBox::NotifyCopying(
    SvLBoxEntry*  pTarget,       // D&D dropping position in this->GetModel()
    SvLBoxEntry*  pEntry,        // entry that we want to move, from
                                 // GetSourceListBox()->GetModel()
    SvLBoxEntry*& rpNewParent,   // new target parent
    sal_uLong&        rNewChildPos)  // position in childlist of target parent
{
    DBG_CHKTHIS(SvLBox,0);
    return NotifyMoving(pTarget,pEntry,rpNewParent,rNewChildPos);
}

SvLBoxEntry* SvLBox::CloneEntry( SvLBoxEntry* pSource )
{
    DBG_CHKTHIS(SvLBox,0);
    SvLBoxEntry* pEntry = (SvLBoxEntry*)CreateEntry(); // new SvLBoxEntry;
    pEntry->Clone( (SvListEntry*)pSource );
    return pEntry;
}


// return: all entries copied
sal_Bool SvLBox::CopySelection( SvLBox* pSource, SvLBoxEntry* pTarget )
{
    DBG_CHKTHIS(SvLBox,0);
    nCurEntrySelPos = 0; // selection counter for NotifyMoving/Copying
    sal_Bool bSuccess = sal_True;
    SvTreeEntryList aList;
    sal_Bool bClone = (sal_Bool)( (sal_uLong)(pSource->GetModel()) != (sal_uLong)GetModel() );
    Link aCloneLink( pModel->GetCloneLink() );
    pModel->SetCloneLink( LINK(this, SvLBox, CloneHdl_Impl ));

    // cache selection to simplify iterating over the selection when doing a D&D
    // exchange within the same listbox
    SvLBoxEntry* pSourceEntry = pSource->FirstSelected();
    while ( pSourceEntry )
    {
        // children are copied automatically
        pSource->SelectChildren( pSourceEntry, sal_False );
        aList.push_back( pSourceEntry );
        pSourceEntry = pSource->NextSelected( pSourceEntry );
    }

    SvTreeEntryList::iterator it = aList.begin(), itEnd = aList.end();
    for (; it != itEnd; ++it)
    {
        pSourceEntry = static_cast<SvLBoxEntry*>(*it);
        SvLBoxEntry* pNewParent = 0;
        sal_uLong nInsertionPos = ULONG_MAX;
        sal_Bool bOk=NotifyCopying(pTarget,pSourceEntry,pNewParent,nInsertionPos);
        if ( bOk )
        {
            if ( bClone )
            {
                sal_uLong nCloneCount = 0;
                pSourceEntry = (SvLBoxEntry*)
                    pModel->Clone( (SvListEntry*)pSourceEntry, nCloneCount );
                pModel->InsertTree( (SvListEntry*)pSourceEntry,
                                    (SvListEntry*)pNewParent, nInsertionPos );
            }
            else
            {
                sal_uLong nListPos = pModel->Copy( (SvListEntry*)pSourceEntry,
                    (SvListEntry*)pNewParent, nInsertionPos );
                pSourceEntry = GetEntry( pNewParent, nListPos );
            }
        }
        else
            bSuccess = sal_False;

        if( bOk == (sal_Bool)2 )  // HACK: make visible moved entry?
            MakeVisible( pSourceEntry );
    }
    pModel->SetCloneLink( aCloneLink );
    return bSuccess;
}

// return: all entries were moved
sal_Bool SvLBox::MoveSelection( SvLBox* pSource, SvLBoxEntry* pTarget )
{
    return MoveSelectionCopyFallbackPossible( pSource, pTarget, sal_False );
}

sal_Bool SvLBox::MoveSelectionCopyFallbackPossible( SvLBox* pSource, SvLBoxEntry* pTarget, sal_Bool bAllowCopyFallback )
{
    DBG_CHKTHIS(SvLBox,0);
    nCurEntrySelPos = 0; // selection counter for NotifyMoving/Copying
    sal_Bool bSuccess = sal_True;
    SvTreeEntryList aList;
    sal_Bool bClone = (sal_Bool)( (sal_uLong)(pSource->GetModel()) != (sal_uLong)GetModel() );
    Link aCloneLink( pModel->GetCloneLink() );
    if ( bClone )
        pModel->SetCloneLink( LINK(this, SvLBox, CloneHdl_Impl ));

    SvLBoxEntry* pSourceEntry = pSource->FirstSelected();
    while ( pSourceEntry )
    {
        // children are automatically moved
        pSource->SelectChildren( pSourceEntry, sal_False );
        aList.push_back( pSourceEntry );
        pSourceEntry = pSource->NextSelected( pSourceEntry );
    }

    SvTreeEntryList::iterator it = aList.begin(), itEnd = aList.end();
    for (; it != itEnd; ++it)
    {
        pSourceEntry = static_cast<SvLBoxEntry*>(*it);

        SvLBoxEntry* pNewParent = 0;
        sal_uLong nInsertionPos = ULONG_MAX;
        sal_Bool bOk = NotifyMoving(pTarget,pSourceEntry,pNewParent,nInsertionPos);
        sal_Bool bCopyOk = bOk;
        if ( !bOk && bAllowCopyFallback )
        {
            nInsertionPos = LIST_APPEND;
            bCopyOk = NotifyCopying(pTarget,pSourceEntry,pNewParent,nInsertionPos);
        }

        if ( bOk || bCopyOk )
        {
            if ( bClone )
            {
                sal_uLong nCloneCount = 0;
                pSourceEntry = (SvLBoxEntry*)
                    pModel->Clone( (SvListEntry*)pSourceEntry, nCloneCount );
                pModel->InsertTree( (SvListEntry*)pSourceEntry,
                                    (SvListEntry*)pNewParent, nInsertionPos );
            }
            else
            {
                if ( bOk )
                    pModel->Move( (SvListEntry*)pSourceEntry,
                                  (SvListEntry*)pNewParent, nInsertionPos );
                else
                    pModel->Copy( (SvListEntry*)pSourceEntry,
                                  (SvListEntry*)pNewParent, nInsertionPos );
            }
        }
        else
            bSuccess = sal_False;

        if( bOk == (sal_Bool)2 )  // HACK: make moved entry visible?
            MakeVisible( pSourceEntry );
    }
    pModel->SetCloneLink( aCloneLink );
    return bSuccess;
}

void SvLBox::RemoveSelection()
{
    DBG_CHKTHIS(SvLBox,0);
    SvTreeEntryList aList;
    // cache selection, as the implementation deselects everything on the first
    // remove
    SvLBoxEntry* pEntry = FirstSelected();
    while ( pEntry )
    {
        aList.push_back( pEntry );
        if ( pEntry->HasChildren() )
            // remove deletes all children automatically
            SelectChildren( pEntry, sal_False );
        pEntry = NextSelected( pEntry );
    }

    SvTreeEntryList::iterator it = aList.begin(), itEnd = aList.end();
    for (; it != itEnd; ++it)
    {
        pEntry = static_cast<SvLBoxEntry*>(*it);
        pModel->Remove(pEntry);
    }
}

SvLBox* SvLBox::GetSourceView() const
{
    return pDDSource;
}

void SvLBox::RequestingChildren( SvLBoxEntry*  )
{
    DBG_CHKTHIS(SvLBox,0);
    OSL_FAIL("Child-Request-Hdl not implemented!");
}

void SvLBox::RecalcViewData()
{
    DBG_CHKTHIS(SvLBox,0);
    SvLBoxEntry* pEntry = First();
    while( pEntry )
    {
        sal_uInt16 nCount = pEntry->ItemCount();
        sal_uInt16 nCurPos = 0;
        while ( nCurPos < nCount )
        {
            SvLBoxItem* pItem = pEntry->GetItem( nCurPos );
            pItem->InitViewData( this, pEntry );
            nCurPos++;
        }
        ViewDataInitialized( pEntry );
        pEntry = Next( pEntry );
    }
}

void SvLBox::ViewDataInitialized( SvLBoxEntry* )
{
    DBG_CHKTHIS(SvLBox,0);
}

void SvLBox::StateChanged( StateChangedType eType )
{
    if( eType == STATE_CHANGE_ENABLE )
        Invalidate( INVALIDATE_CHILDREN );
    Control::StateChanged( eType );
}

void SvLBox::ImplShowTargetEmphasis( SvLBoxEntry* pEntry, sal_Bool bShow)
{
    DBG_CHKTHIS(SvLBox,0);
    if ( bShow && (nImpFlags & SVLBOX_TARGEMPH_VIS) )
        return;
    if ( !bShow && !(nImpFlags & SVLBOX_TARGEMPH_VIS) )
        return;
    ShowTargetEmphasis( pEntry, bShow );
    if( bShow )
        nImpFlags |= SVLBOX_TARGEMPH_VIS;
    else
        nImpFlags &= ~SVLBOX_TARGEMPH_VIS;
}

void SvLBox::ShowTargetEmphasis( SvLBoxEntry*, sal_Bool /* bShow */ )
{
    DBG_CHKTHIS(SvLBox,0);
}


sal_Bool SvLBox::Expand( SvLBoxEntry* )
{
    DBG_CHKTHIS(SvLBox,0);
    return sal_True;
}

sal_Bool SvLBox::Collapse( SvLBoxEntry* )
{
    DBG_CHKTHIS(SvLBox,0);
    return sal_True;
}

sal_Bool SvLBox::Select( SvLBoxEntry*, sal_Bool  )
{
    DBG_CHKTHIS(SvLBox,0);
    return sal_False;
}

sal_uLong SvLBox::SelectChildren( SvLBoxEntry* , sal_Bool  )
{
    DBG_CHKTHIS(SvLBox,0);
    return 0;
}

void SvLBox::OnCurrentEntryChanged()
{
    if ( !pLBoxImpl->m_bDoingQuickSelection )
        pLBoxImpl->m_aQuickSelectionEngine.Reset();
}

void SvLBox::SelectAll( sal_Bool /* bSelect */ , sal_Bool /* bPaint */ )
{
    DBG_CHKTHIS(SvLBox,0);
}

SvLBoxEntry* SvLBox::GetEntryFromPath( const ::std::deque< sal_Int32 >& _rPath ) const
{
    DBG_CHKTHIS(SvLBox,0);

    SvLBoxEntry* pEntry = NULL;
    SvLBoxEntry* pParent = NULL;
    for( ::std::deque< sal_Int32 >::const_iterator pItem = _rPath.begin(); pItem != _rPath.end(); ++pItem )
    {
        pEntry = GetEntry( pParent, *pItem );
        if ( !pEntry )
            break;
        pParent = pEntry;
    }

    return pEntry;
}

void SvLBox::FillEntryPath( SvLBoxEntry* pEntry, ::std::deque< sal_Int32 >& _rPath ) const
{
    DBG_CHKTHIS(SvLBox,0);

    if ( pEntry )
    {
        SvLBoxEntry* pParentEntry = GetParent( pEntry );
        while ( sal_True )
        {
            sal_uLong i, nCount = GetLevelChildCount( pParentEntry );
            for ( i = 0; i < nCount; ++i )
            {
                SvLBoxEntry* pTemp = GetEntry( pParentEntry, i );
                DBG_ASSERT( pEntry, "invalid entry" );
                if ( pEntry == pTemp )
                {
                    _rPath.push_front( (sal_Int32)i );
                    break;
                }
            }

            if ( pParentEntry )
            {
                pEntry = pParentEntry;
                pParentEntry = GetParent( pParentEntry );
            }
            else
                break;
        }
    }
}

String SvLBox::GetEntryText( SvLBoxEntry* ) const
{
    DBG_CHKTHIS(SvLBox,0);

    return String();
}

sal_uLong SvLBox::GetLevelChildCount( SvLBoxEntry* _pParent ) const
{
    DBG_CHKTHIS(SvLBox,0);

    sal_uLong nCount = 0;
    SvLBoxEntry* pEntry = FirstChild( _pParent );
    while ( pEntry )
    {
        ++nCount;
        pEntry = NextSibling( pEntry );
    }

    return nCount;
}

void SvLBox::SetSelectionMode( SelectionMode eSelectMode )
{
    DBG_CHKTHIS(SvLBox,0);
    eSelMode = eSelectMode;
}

void SvLBox::SetDragDropMode( DragDropMode nDDMode )
{
    DBG_CHKTHIS(SvLBox,0);
    nDragDropMode = nDDMode;
}

SvViewData* SvLBox::CreateViewData( SvListEntry* )
{
    DBG_CHKTHIS(SvLBox,0);
    SvViewDataEntry* pEntryData = new SvViewDataEntry;
    return (SvViewData*)pEntryData;
}

void SvLBox::InitViewData( SvViewData* pData, SvListEntry* pEntry )
{
    DBG_CHKTHIS(SvLBox,0);
    SvLBoxEntry* pInhEntry = (SvLBoxEntry*)pEntry;
    SvViewDataEntry* pEntryData = (SvViewDataEntry*)pData;

    pEntryData->pItemData = new SvViewDataItem[ pInhEntry->ItemCount() ];
    SvViewDataItem* pItemData = pEntryData->pItemData;
    pEntryData->nItmCnt = pInhEntry->ItemCount(); // number of items to delete
    sal_uInt16 nCount = pInhEntry->ItemCount();
    sal_uInt16 nCurPos = 0;
    while( nCurPos < nCount )
    {
        SvLBoxItem* pItem = pInhEntry->GetItem( nCurPos );
        pItem->InitViewData( this, pInhEntry, pItemData );
        pItemData++;
        nCurPos++;
    }
}



void SvLBox::EnableSelectionAsDropTarget( sal_Bool bEnable, sal_Bool bWithChildren )
{
    DBG_CHKTHIS(SvLBox,0);
    sal_uInt16 nRefDepth;
    SvLBoxEntry* pTemp;

    SvLBoxEntry* pSelEntry = FirstSelected();
    while( pSelEntry )
    {
        if ( !bEnable )
        {
            pSelEntry->nEntryFlags |= SV_ENTRYFLAG_DISABLE_DROP;
            if ( bWithChildren )
            {
                nRefDepth = pModel->GetDepth( pSelEntry );
                pTemp = Next( pSelEntry );
                while( pTemp && pModel->GetDepth( pTemp ) > nRefDepth )
                {
                    pTemp->nEntryFlags |= SV_ENTRYFLAG_DISABLE_DROP;
                    pTemp = Next( pTemp );
                }
            }
        }
        else
        {
            pSelEntry->nEntryFlags &= (~SV_ENTRYFLAG_DISABLE_DROP);
            if ( bWithChildren )
            {
                nRefDepth = pModel->GetDepth( pSelEntry );
                pTemp = Next( pSelEntry );
                while( pTemp && pModel->GetDepth( pTemp ) > nRefDepth )
                {
                    pTemp->nEntryFlags &= (~SV_ENTRYFLAG_DISABLE_DROP);
                    pTemp = Next( pTemp );
                }
            }
        }
        pSelEntry = NextSelected( pSelEntry );
    }
}

SvLBoxEntry* SvLBox::GetDropTarget( const Point& )
{
    DBG_CHKTHIS(SvLBox,0);
    return 0;
}

// ******************************************************************
// InplaceEditing
// ******************************************************************

void SvLBox::EditText( const String& rStr, const Rectangle& rRect,
    const Selection& rSel )
{
    EditText( rStr, rRect, rSel, sal_False );
}

void SvLBox::EditText( const String& rStr, const Rectangle& rRect,
    const Selection& rSel, sal_Bool bMulti )
{
    DBG_CHKTHIS(SvLBox,0);
    if( pEdCtrl )
        delete pEdCtrl;
    nImpFlags |= SVLBOX_IN_EDT;
    nImpFlags &= ~SVLBOX_EDTEND_CALLED;
    HideFocus();
    pEdCtrl = new SvInplaceEdit2(
        this, rRect.TopLeft(), rRect.GetSize(), rStr,
        LINK( this, SvLBox, TextEditEndedHdl_Impl ),
        rSel, bMulti );
}

IMPL_LINK_NOARG(SvLBox, TextEditEndedHdl_Impl)
{
    DBG_CHKTHIS(SvLBox,0);
    if ( nImpFlags & SVLBOX_EDTEND_CALLED ) // avoid nesting
        return 0;
    nImpFlags |= SVLBOX_EDTEND_CALLED;
    String aStr;
    if ( !pEdCtrl->EditingCanceled() )
        aStr = pEdCtrl->GetText();
    else
        aStr = pEdCtrl->GetSavedValue();
    if ( IsEmptyTextAllowed() || aStr.Len() > 0 )
        EditedText( aStr );
    // Hide may only be called after the new text was put into the entry, so
    // that we don't call the selection handler in the GetFocus of the listbox
    // with the old entry text.
    pEdCtrl->Hide();
    // delete pEdCtrl;
    // pEdCtrl = 0;
    nImpFlags &= (~SVLBOX_IN_EDT);
    GrabFocus();
    return 0;
}

void SvLBox::CancelTextEditing()
{
    DBG_CHKTHIS(SvLBox,0);
    if ( pEdCtrl )
        pEdCtrl->StopEditing( sal_True );
    nImpFlags &= (~SVLBOX_IN_EDT);
}

void SvLBox::EndEditing( sal_Bool bCancel )
{
    DBG_CHKTHIS(SvLBox,0);
    if( pEdCtrl )
        pEdCtrl->StopEditing( bCancel );
    nImpFlags &= (~SVLBOX_IN_EDT);
}


bool SvLBox::IsEmptyTextAllowed() const
{
    DBG_CHKTHIS(SvLBox,0);
    return pLBoxImpl->m_bIsEmptyTextAllowed;
}

void SvLBox::ForbidEmptyText()
{
    DBG_CHKTHIS(SvLBox,0);
    pLBoxImpl->m_bIsEmptyTextAllowed = false;
}

void SvLBox::EditedText( const String& )
{
    DBG_CHKTHIS(SvLBox,0);
}

void SvLBox::EditingRequest( SvLBoxEntry*, SvLBoxItem*,const Point& )
{
    DBG_CHKTHIS(SvLBox,0);
}


SvLBoxEntry* SvLBox::CreateEntry() const
{
    DBG_CHKTHIS(SvLBox,0);
    return new SvLBoxEntry;
}

void SvLBox::MakeVisible( SvLBoxEntry* )
{
    DBG_CHKTHIS(SvLBox,0);
}

void SvLBox::Command( const CommandEvent& i_rCommandEvent )
{
    DBG_CHKTHIS(SvLBox,0);

    if ( COMMAND_STARTDRAG == i_rCommandEvent.GetCommand() )
    {
        Point aEventPos( i_rCommandEvent.GetMousePosPixel() );
        MouseEvent aMouseEvt( aEventPos, 1, MOUSE_SELECT, MOUSE_LEFT );
        MouseButtonUp( aMouseEvt );
    }
    Control::Command( i_rCommandEvent );
}

void SvLBox::KeyInput( const KeyEvent& rKEvt )
{
    bool bHandled = HandleKeyInput( rKEvt );
    if ( !bHandled )
        Control::KeyInput( rKEvt );
}

const void* SvLBox::FirstSearchEntry( String& _rEntryText ) const
{
    SvLBoxEntry* pEntry = GetCurEntry();
    if ( pEntry )
        pEntry = const_cast< SvLBoxEntry* >( static_cast< const SvLBoxEntry* >( NextSearchEntry( pEntry, _rEntryText ) ) );
    else
    {
        pEntry = FirstSelected();
        if ( !pEntry )
            pEntry = First();
    }

    if ( pEntry )
        _rEntryText = GetEntryText( pEntry );

    return pEntry;
}

const void* SvLBox::NextSearchEntry( const void* _pCurrentSearchEntry, String& _rEntryText ) const
{
    SvLBoxEntry* pEntry = const_cast< SvLBoxEntry* >( static_cast< const SvLBoxEntry* >( _pCurrentSearchEntry ) );

    if  (   (   ( GetChildCount( pEntry ) > 0 )
            ||  ( pEntry->HasChildrenOnDemand() )
            )
        &&  !IsExpanded( pEntry )
        )
    {
        pEntry = NextSibling( pEntry );
    }
    else
    {
        pEntry = Next( pEntry );
    }

    if ( !pEntry )
        pEntry = First();

    if ( pEntry )
        _rEntryText = GetEntryText( pEntry );

    return pEntry;
}

void SvLBox::SelectSearchEntry( const void* _pEntry )
{
    SvLBoxEntry* pEntry = const_cast< SvLBoxEntry* >( static_cast< const SvLBoxEntry* >( _pEntry ) );
    DBG_ASSERT( pEntry, "SvLBox::SelectSearchEntry: invalid entry!" );
    if ( !pEntry )
        return;

    SelectAll( sal_False );
    SetCurEntry( pEntry );
    Select( pEntry );
}

void SvLBox::ExecuteSearchEntry( const void* /*_pEntry*/ ) const
{
    // nothing to do here, we have no "execution"
}

::vcl::StringEntryIdentifier SvLBox::CurrentEntry( String& _out_entryText ) const
{
    // always accept the current entry if there is one
    SvLBoxEntry* pCurrentEntry( GetCurEntry() );
    if ( pCurrentEntry )
    {
        _out_entryText = GetEntryText( pCurrentEntry );
        return pCurrentEntry;
    }
    return FirstSearchEntry( _out_entryText );
}

::vcl::StringEntryIdentifier SvLBox::NextEntry( ::vcl::StringEntryIdentifier _currentEntry, String& _out_entryText ) const
{
    return NextSearchEntry( _currentEntry, _out_entryText );
}

void SvLBox::SelectEntry( ::vcl::StringEntryIdentifier _entry )
{
    SelectSearchEntry( _entry );
}

bool SvLBox::HandleKeyInput( const KeyEvent& _rKEvt )
{
    if  (   IsEntryMnemonicsEnabled()
        &&  pLBoxImpl->m_aMnemonicEngine.HandleKeyEvent( _rKEvt )
        )
        return true;

    if ( ( GetStyle() & WB_QUICK_SEARCH ) != 0 )
    {
        pLBoxImpl->m_bDoingQuickSelection = true;
        const bool bHandled = pLBoxImpl->m_aQuickSelectionEngine.HandleKeyEvent( _rKEvt );
        pLBoxImpl->m_bDoingQuickSelection = false;
        if ( bHandled )
            return true;
    }

    return false;
}

SvLBoxEntry* SvLBox::GetEntry( const Point&, sal_Bool ) const
{
    DBG_CHKTHIS(SvLBox,0);
    return 0;
}

void SvLBox::ModelHasEntryInvalidated( SvListEntry* pEntry )
{
    DBG_CHKTHIS(SvLBox,0);
    sal_uInt16 nCount = ((SvLBoxEntry*)pEntry)->ItemCount();
    for( sal_uInt16 nIdx = 0; nIdx < nCount; nIdx++ )
    {
        SvLBoxItem* pItem = ((SvLBoxEntry*)pEntry)->GetItem( nIdx );
        pItem->InitViewData( this, (SvLBoxEntry*)pEntry, 0 );
    }
}

void SvLBox::WriteDragServerInfo( const Point&, SvLBoxDDInfo* )
{
    DBG_CHKTHIS(SvLBox,0);
}

void SvLBox::ReadDragServerInfo(const Point&, SvLBoxDDInfo* )
{
    DBG_CHKTHIS(SvLBox,0);
}

sal_Bool SvLBox::EditingCanceled() const
{
    if( pEdCtrl && pEdCtrl->EditingCanceled() )
        return sal_True;
    return sal_False;
}


//JP 28.3.2001: new Drag & Drop API
sal_Int8 SvLBox::AcceptDrop( const AcceptDropEvent& rEvt )
{
    DBG_CHKTHIS(SvLBox,0);
    sal_Int8 nRet = DND_ACTION_NONE;

    if( rEvt.mbLeaving || !CheckDragAndDropMode( pDDSource, rEvt.mnAction ) )
    {
        ImplShowTargetEmphasis( pTargetEntry, sal_False );
    }
    else if( !nDragDropMode )
    {
        SAL_WARN( "svtools.contnr", "SvLBox::QueryDrop(): no target" );
    }
    else
    {
        SvLBoxEntry* pEntry = GetDropTarget( rEvt.maPosPixel );
        if( !IsDropFormatSupported( SOT_FORMATSTR_ID_TREELISTBOX ) )
        {
            SAL_WARN( "svtools.contnr", "SvLBox::QueryDrop(): no format" );
        }
        else
        {
            DBG_ASSERT( pDDSource, "SvLBox::QueryDrop(): SourceBox == 0" );
            if( !( pEntry && pDDSource->GetModel() == this->GetModel()
                    && DND_ACTION_MOVE == rEvt.mnAction
                    && ( pEntry->nEntryFlags & SV_ENTRYFLAG_DISABLE_DROP ) ))
            {
                if( NotifyAcceptDrop( pEntry ))
                    nRet = rEvt.mnAction;
            }
        }

        // **** draw emphasis ****
        if( DND_ACTION_NONE == nRet )
               ImplShowTargetEmphasis( pTargetEntry, sal_False );
        else if( pEntry != pTargetEntry || !(nImpFlags & SVLBOX_TARGEMPH_VIS) )
        {
            ImplShowTargetEmphasis( pTargetEntry, sal_False );
            pTargetEntry = pEntry;
            ImplShowTargetEmphasis( pTargetEntry, sal_True );
        }
    }
    return nRet;
}

sal_Int8 SvLBox::ExecuteDrop( const ExecuteDropEvent& rEvt, SvLBox* pSourceView )
{
    DBG_CHKTHIS(SvLBox,0);
    sal_Int8 nRet = DND_ACTION_NONE;

    DBG_ASSERT( pSourceView, "SvLBox::ExecuteDrop(): no source view" );
    pSourceView->EnableSelectionAsDropTarget( sal_True, sal_True );

    ImplShowTargetEmphasis( pTargetEntry, sal_False );
    pDDTarget = this;

    SvLBoxDDInfo aDDInfo;

    TransferableDataHelper aData( rEvt.maDropEvent.Transferable );
    if( aData.HasFormat( SOT_FORMATSTR_ID_TREELISTBOX ))
    {
        ::com::sun::star::uno::Sequence< sal_Int8 > aSeq;
        if( aData.GetSequence( SOT_FORMATSTR_ID_TREELISTBOX, aSeq ) &&
            sizeof(SvLBoxDDInfo) == aSeq.getLength() )
        {
            memcpy( &aDDInfo, aSeq.getConstArray(), sizeof(SvLBoxDDInfo) );
            nRet = rEvt.mnAction;
        }
    }

    if( DND_ACTION_NONE != nRet )
    {
        nRet = DND_ACTION_NONE;

        ReadDragServerInfo( rEvt.maPosPixel, &aDDInfo );

        SvLBoxEntry* pTarget = pTargetEntry; // may be 0!

        if( DND_ACTION_COPY == rEvt.mnAction )
        {
            if ( CopySelection( aDDInfo.pSource, pTarget ) )
                nRet = rEvt.mnAction;
        }
        else if( DND_ACTION_MOVE == rEvt.mnAction )
        {
            if ( MoveSelection( aDDInfo.pSource, pTarget ) )
                nRet = rEvt.mnAction;
        }
        else if( DND_ACTION_COPYMOVE == rEvt.mnAction )
        {
            if ( MoveSelectionCopyFallbackPossible( aDDInfo.pSource, pTarget, sal_True ) )
                nRet = rEvt.mnAction;
        }
    }
    return nRet;
}

sal_Int8 SvLBox::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    DBG_CHKTHIS(SvLBox,0);
    return ExecuteDrop( rEvt, GetSourceView() );
}

void SvLBox::StartDrag( sal_Int8, const Point& rPosPixel )
{
    DBG_CHKTHIS(SvLBox,0);

    Point aEventPos( rPosPixel );
    MouseEvent aMouseEvt( aEventPos, 1, MOUSE_SELECT, MOUSE_LEFT );
    MouseButtonUp( aMouseEvt );

    nOldDragMode = GetDragDropMode();
    if ( !nOldDragMode )
        return;

    ReleaseMouse();

    SvLBoxEntry* pEntry = GetEntry( rPosPixel ); // GetDropTarget( rPos );
    if( !pEntry )
    {
        DragFinished( DND_ACTION_NONE );
        return;
    }

    TransferDataContainer* pContainer = new TransferDataContainer;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::datatransfer::XTransferable > xRef( pContainer );

    nDragDropMode = NotifyStartDrag( *pContainer, pEntry );
    if( !nDragDropMode || 0 == GetSelectionCount() )
    {
        nDragDropMode = nOldDragMode;
        DragFinished( DND_ACTION_NONE );
        return;
    }

    SvLBoxDDInfo aDDInfo;
    memset(&aDDInfo,0,sizeof(SvLBoxDDInfo));
    aDDInfo.pApp = GetpApp();
    aDDInfo.pSource = this;
    aDDInfo.pDDStartEntry = pEntry;
    // let derived views do their thing
    WriteDragServerInfo( rPosPixel, &aDDInfo );

    pContainer->CopyAnyData( SOT_FORMATSTR_ID_TREELISTBOX,
                        (sal_Char*)&aDDInfo, sizeof(SvLBoxDDInfo) );
    pDDSource = this;
    pDDTarget = 0;

    sal_Bool bOldUpdateMode = Control::IsUpdateMode();
    Control::SetUpdateMode( sal_True );
    Update();
    Control::SetUpdateMode( bOldUpdateMode );

    // Disallow using the selection and its children as drop targets.
    // Important: If the selection of the SourceListBox is changed in the
    // DropHandler, the entries have to be allowed as drop targets again:
    // (GetSourceListBox()->EnableSelectionAsDropTarget( sal_True, sal_True );)
    EnableSelectionAsDropTarget( sal_False, sal_True /* with children */ );

    pContainer->StartDrag( this, nDragOptions, GetDragFinishedHdl() );
}

void SvLBox::DragFinished( sal_Int8
#ifndef UNX
nAction
#endif
)
{
    EnableSelectionAsDropTarget( sal_True, sal_True );

#ifndef UNX
    if( (nAction == DND_ACTION_MOVE) && ( (pDDTarget &&
        ((sal_uLong)(pDDTarget->GetModel())!=(sal_uLong)(this->GetModel()))) ||
        !pDDTarget ))
    {
        RemoveSelection();
    }
#endif

    ImplShowTargetEmphasis( pTargetEntry, sal_False );
    pDDSource = 0;
    pDDTarget = 0;
    pTargetEntry = 0;
    nDragDropMode = nOldDragMode;
}

DragDropMode SvLBox::NotifyStartDrag( TransferDataContainer&, SvLBoxEntry* )
{
    DBG_CHKTHIS(SvLBox,0);
    return (DragDropMode)0xffff;
}

sal_Bool SvLBox::NotifyAcceptDrop( SvLBoxEntry* )
{
    DBG_CHKTHIS(SvLBox,0);
    return sal_True;
}

// Handler and methods for Drag - finished handler.
// The with get GetDragFinishedHdl() get link can set on the
// TransferDataContainer. This link is a callback for the DragFinished
// call. AddBox method is called from the GetDragFinishedHdl() and the
// remove is called in link callback and in the destructor. So it can't
// called to a deleted object.

namespace
{
    struct SortLBoxes : public rtl::Static<std::set<sal_uLong>, SortLBoxes> {};
}

void SvLBox::AddBoxToDDList_Impl( const SvLBox& rB )
{
    sal_uLong nVal = (sal_uLong)&rB;
    SortLBoxes::get().insert( nVal );
}

void SvLBox::RemoveBoxFromDDList_Impl( const SvLBox& rB )
{
    sal_uLong nVal = (sal_uLong)&rB;
    SortLBoxes::get().erase( nVal );
}

IMPL_STATIC_LINK( SvLBox, DragFinishHdl_Impl, sal_Int8*, pAction )
{
    sal_uLong nVal = (sal_uLong)pThis;
    std::set<sal_uLong> &rSortLBoxes = SortLBoxes::get();
    std::set<sal_uLong>::const_iterator it = rSortLBoxes.find(nVal);
    if( it != rSortLBoxes.end() )
    {
        pThis->DragFinished( *pAction );
        rSortLBoxes.erase( it );
    }
    return 0;
}

Link SvLBox::GetDragFinishedHdl() const
{
    AddBoxToDDList_Impl( *this );
    return STATIC_LINK( this, SvLBox, DragFinishHdl_Impl );
}

void SvLBox::FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& ) const
{
}

::com::sun::star::uno::Reference< XAccessible > SvLBox::CreateAccessible()
{
    return ::com::sun::star::uno::Reference< XAccessible >();
}

Rectangle SvLBox::GetBoundingRect( SvLBoxEntry* )
{
    return Rectangle();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
