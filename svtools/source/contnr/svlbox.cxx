/*************************************************************************
 *
 *  $RCSfile: svlbox.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/*
    Todo:
        - Anker loeschen in SelectionEngine bei manuellem Selektieren
        - SelectAll( FALSE ), nur die deselektierten Entries repainten
*/


#include <string.h>
#ifndef _SVEDI_HXX
#include <svmedit.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_ACCEL_HXX
#include <vcl/accel.hxx>
#endif
#ifndef _SV_DRAG_HXX
#include <vcl/drag.hxx>
#endif
#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif

#pragma hdrstop

#include <svlbox.hxx>
#include <svlbitm.hxx>

// fuer Drag&Drop
static SvLBox* pDDSource = 0;
static SvLBox* pDDTarget = 0;

DBG_NAME(SvInplaceEdit);
DBG_NAME(SvInplaceEdit2);

#define SVLBOX_ACC_RETURN 1
#define SVLBOX_ACC_ESCAPE 2

SvInplaceEdit::SvInplaceEdit( Window* pParent, const Point& rPos,
    const Size& rSize, const XubString& rData, const Link& rNotifyEditEnd,
    const Selection& rSelection) :
#ifndef OS2
    Edit( pParent, WB_LEFT ),
#else
    Edit( pParent, WB_LEFT | WB_BORDER ),
#endif
    aCallBackHdl( rNotifyEditEnd ),
    bCanceled( FALSE ),
    bAlreadyInCallBack( FALSE )
{
    DBG_CTOR(SvInplaceEdit,0);
    Font aFont( pParent->GetFont() );
    aFont.SetTransparent( FALSE );
    Color aColor( pParent->GetBackground().GetColor() );
    aFont.SetFillColor(aColor );
    SetFont( aFont );
    SetBackground( pParent->GetBackground() );
    SetPosPixel( rPos );
    SetSizePixel( rSize );
    SetText( rData );
    SetSelection( rSelection );
    SaveValue();

    aAccReturn.InsertItem( SVLBOX_ACC_RETURN, KeyCode(KEY_RETURN) );
    aAccEscape.InsertItem( SVLBOX_ACC_ESCAPE, KeyCode(KEY_ESCAPE) );

    aAccReturn.SetActivateHdl( LINK( this, SvInplaceEdit, ReturnHdl_Impl) );
    aAccEscape.SetActivateHdl( LINK( this, SvInplaceEdit, EscapeHdl_Impl) );
    GetpApp()->InsertAccel( &aAccReturn  );
    GetpApp()->InsertAccel( &aAccEscape );

    Show();
    GrabFocus();
}

__EXPORT SvInplaceEdit::~SvInplaceEdit()
{
    DBG_DTOR(SvInplaceEdit,0);
    if( !bAlreadyInCallBack )
    {
        GetpApp()->RemoveAccel( &aAccReturn );
        GetpApp()->RemoveAccel( &aAccEscape );
    }
}



IMPL_LINK_INLINE_START( SvInplaceEdit, ReturnHdl_Impl, Accelerator *, pAccelerator )
{
    DBG_CHKTHIS(SvInplaceEdit,0);
    bCanceled = FALSE;
    CallCallBackHdl_Impl();
    return 1;
}
IMPL_LINK_INLINE_END( SvInplaceEdit, ReturnHdl_Impl, Accelerator *, pAccelerator )

IMPL_LINK_INLINE_START( SvInplaceEdit, EscapeHdl_Impl, Accelerator *, pAccelerator )
{
    DBG_CHKTHIS(SvInplaceEdit,0);
    bCanceled = TRUE;
    CallCallBackHdl_Impl();
    return 1;
}
IMPL_LINK_INLINE_END( SvInplaceEdit, EscapeHdl_Impl, Accelerator *, pAccelerator )


void __EXPORT SvInplaceEdit::KeyInput( const KeyEvent& rKEvt )
{
    DBG_CHKTHIS(SvInplaceEdit,0);
    USHORT nCode = rKEvt.GetKeyCode().GetCode();
    switch ( nCode )
    {
        case KEY_ESCAPE:
            bCanceled = TRUE;
            CallCallBackHdl_Impl();
            break;

        case KEY_RETURN:
            bCanceled = FALSE;
            CallCallBackHdl_Impl();
            break;

        default:
            Edit::KeyInput( rKEvt );
    }
}

void SvInplaceEdit::StopEditing( BOOL bCancel )
{
    DBG_CHKTHIS(SvInplaceEdit,0);
    if ( !bAlreadyInCallBack )
    {
        bCanceled = bCancel;
        CallCallBackHdl_Impl();
    }
}

void __EXPORT SvInplaceEdit::LoseFocus()
{
    DBG_CHKTHIS(SvInplaceEdit,0);
    if ( !bAlreadyInCallBack )
    {
        bCanceled = FALSE;
        aTimer.SetTimeout(10);
        aTimer.SetTimeoutHdl(LINK(this,SvInplaceEdit,Timeout_Impl));
        aTimer.Start();
    }
}

IMPL_LINK_INLINE_START( SvInplaceEdit, Timeout_Impl, Timer *, pTimer )
{
    DBG_CHKTHIS(SvInplaceEdit,0);
    CallCallBackHdl_Impl();
    return 0;
}
IMPL_LINK_INLINE_END( SvInplaceEdit, Timeout_Impl, Timer *, pTimer )

void SvInplaceEdit::CallCallBackHdl_Impl()
{
    DBG_CHKTHIS(SvInplaceEdit,0);
    aTimer.Stop();
    if ( !bAlreadyInCallBack )
    {
        bAlreadyInCallBack = TRUE;
        GetpApp()->RemoveAccel( &aAccReturn );
        GetpApp()->RemoveAccel( &aAccEscape );
        Hide();
        aCallBackHdl.Call( this );
        // bAlreadyInCallBack = FALSE;
    }
}


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

MyEdit_Impl::MyEdit_Impl( Window* pParent, SvInplaceEdit2* _pOwner )
    :
#ifndef OS2
    Edit( pParent, WB_LEFT ),
#else
    Edit( pParent, WB_LEFT | WB_BORDER ),
#endif
    pOwner(_pOwner)
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
#if !defined(VCL)
#if defined(WIN) || defined(WNT)
    WB_CENTER
#else
    WB_LEFT
#endif
#else
    WB_CENTER
#endif
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


SvInplaceEdit2::SvInplaceEdit2( Window* pParent, const Point& rPos,
    const Size& rSize, const XubString& rData, const Link& rNotifyEditEnd,
    const Selection& rSelection, BOOL bMulti) :
    aCallBackHdl( rNotifyEditEnd ),
    bCanceled( FALSE ),
    bAlreadyInCallBack( FALSE ),
    bMultiLine( bMulti )
{
    DBG_CTOR(SvInplaceEdit2,0);
    if( bMulti )
        pEdit = new MyMultiEdit_Impl( pParent, this );
    else
        pEdit = new MyEdit_Impl( pParent, this );

    Font aFont( pParent->GetFont() );
    aFont.SetTransparent( FALSE );
    Color aColor( pParent->GetBackground().GetColor() );
    aFont.SetFillColor(aColor );
    pEdit->SetFont( aFont );
    pEdit->SetBackground( pParent->GetBackground() );
    pEdit->SetPosPixel( rPos );
    pEdit->SetSizePixel( rSize );
    pEdit->SetText( rData );
#ifndef OS2
    pEdit->SetSelection( rSelection );
#else
    if( !bMulti )
        pEdit->SetSelection( rSelection );
#endif
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

XubString SvInplaceEdit2::GetSavedValue() const
{
    return pEdit->GetSavedValue();
}

void SvInplaceEdit2::Hide()
{
    pEdit->Hide();
}


IMPL_LINK_INLINE_START( SvInplaceEdit2, ReturnHdl_Impl, Accelerator *, pAccelerator )
{
    DBG_CHKTHIS(SvInplaceEdit2,0);
    bCanceled = FALSE;
    CallCallBackHdl_Impl();
    return 1;
}
IMPL_LINK_INLINE_END( SvInplaceEdit2, ReturnHdl_Impl, Accelerator *, pAccelerator )

IMPL_LINK_INLINE_START( SvInplaceEdit2, EscapeHdl_Impl, Accelerator *, pAccelerator )
{
    DBG_CHKTHIS(SvInplaceEdit2,0);
    bCanceled = TRUE;
    CallCallBackHdl_Impl();
    return 1;
}
IMPL_LINK_INLINE_END( SvInplaceEdit2, EscapeHdl_Impl, Accelerator *, pAccelerator )


BOOL SvInplaceEdit2::KeyInput( const KeyEvent& rKEvt )
{
    DBG_CHKTHIS(SvInplaceEdit2,0);
    KeyCode aCode = rKEvt.GetKeyCode();
    USHORT nCode = aCode.GetCode();

    switch ( nCode )
    {
        case KEY_ESCAPE:
            bCanceled = TRUE;
            CallCallBackHdl_Impl();
            return TRUE;

        case KEY_RETURN:
//          if( !aCode.IsShift() && !aCode.IsMod1() && !aCode.IsMod2() )
            {
                bCanceled = FALSE;
                CallCallBackHdl_Impl();
                return TRUE;
            }
            break;
    }
    return FALSE;
}

void SvInplaceEdit2::StopEditing( BOOL bCancel )
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
#ifdef VCL
    && ((!Application::GetFocusWindow()) || !pEdit->IsChild( Application::GetFocusWindow()) )
#endif
    )
    {
        bCanceled = FALSE;
        aTimer.SetTimeout(10);
        aTimer.SetTimeoutHdl(LINK(this,SvInplaceEdit2,Timeout_Impl));
        aTimer.Start();
    }
}

IMPL_LINK_INLINE_START( SvInplaceEdit2, Timeout_Impl, Timer *, pTimer )
{
    DBG_CHKTHIS(SvInplaceEdit2,0);
    CallCallBackHdl_Impl();
    return 0;
}
IMPL_LINK_INLINE_END( SvInplaceEdit2, Timeout_Impl, Timer *, pTimer )

void SvInplaceEdit2::CallCallBackHdl_Impl()
{
    DBG_CHKTHIS(SvInplaceEdit2,0);
    aTimer.Stop();
    if ( !bAlreadyInCallBack )
    {
        bAlreadyInCallBack = TRUE;
        GetpApp()->RemoveAccel( &aAccReturn );
        GetpApp()->RemoveAccel( &aAccEscape );
        pEdit->Hide();
        aCallBackHdl.Call( this );
    }
}

XubString SvInplaceEdit2::GetText() const
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

SvLBoxTab::SvLBoxTab( long nPosition, USHORT nTabFlags )
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
            //richtige Implementierung der Zentrierung
            nOffset = ( nTabWidth - nItemWidth ) / 2;
            if( nOffset < 0 )
                nOffset = 0;
        }
        else
        {
            // historisch gewachsene falsche Berechnung des Tabs, auf die sich
            // Abo-Tabbox, Extras/Optionen/Anpassen etc. verlassen
            nItemWidth++;
            nOffset = -( nItemWidth / 2 );
        }
    }
    return nOffset;
}

/*
long SvLBoxTab::CalcOffset( const XubString& rStr, const OutputDevice& rOutDev )
{
    DBG_CHKTHIS(SvLBoxTab,0);
    long nWidth;
    if ( nFlags & SV_LBOXTAB_ADJUST_NUMERIC )
    {
        USHORT nPos = rStr.Search( '.' );
        if ( nPos == STRING_NOTFOUND )
            nPos = rStr.Search( ',' );
        if ( nPos == STRING_NOTFOUND )
            nPos = STRING_LEN;

        nWidth = rOutDev.GetTextSize( rStr, 0, nPos ).Width();
        nWidth *= -1;
    }
    else
    {
        nWidth = rOutDev.GetTextSize( rStr ).Width();
        nWidth = CalcOffset( nWidth );
    }
    return nWidth;
}
*/

// ***************************************************************
// class SvLBoxItem
// ***************************************************************

DBG_NAME(SvLBoxItem);

SvLBoxItem::SvLBoxItem( SvLBoxEntry*, USHORT )
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

const Size& SvLBoxItem::GetSize( SvLBoxEntry* pEntry, SvViewDataEntry* pViewData)
{
    DBG_CHKTHIS(SvLBoxItem,0);
    USHORT nItemPos = pEntry->GetPos( this );
    SvViewDataItem* pItemData = pViewData->pItemData+nItemPos;
    return pItemData->aSize;
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
    USHORT nCount = aItems.Count();
    while( nCount )
    {
        nCount--;
        SvLBoxItem* pItem = (SvLBoxItem*)aItems.GetObject( nCount );
        delete pItem;
    }
    aItems.Remove(0, aItems.Count() );
}


void SvLBoxEntry::AddItem( SvLBoxItem* pItem )
{
    DBG_CHKTHIS(SvLBoxEntry,0);
    aItems.Insert( pItem, aItems.Count() );
}

void SvLBoxEntry::Clone( SvListEntry* pSource )
{
    DBG_CHKTHIS(SvLBoxEntry,0);
    SvListEntry::Clone( pSource );
    SvLBoxItem* pNewItem;
    DeleteItems_Impl();
    USHORT nCount = ((SvLBoxEntry*)pSource)->ItemCount();
    USHORT nCurPos = 0;
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

void SvLBoxEntry::EnableChildsOnDemand( BOOL bEnable )
{
    DBG_CHKTHIS(SvLBoxEntry,0);
    if ( bEnable )
        nEntryFlags |= SV_ENTRYFLAG_CHILDS_ON_DEMAND;
    else
        nEntryFlags &= (~SV_ENTRYFLAG_CHILDS_ON_DEMAND);
}

void SvLBoxEntry::ReplaceItem( SvLBoxItem* pNewItem, USHORT nPos )
{
    DBG_CHKTHIS(SvLBoxEntry,0);
    DBG_ASSERT(pNewItem,"ReplaceItem:No Item")
    SvLBoxItem* pOld = GetItem( nPos );
    if ( pOld )
    {
        aItems.Remove( nPos );
        aItems.Insert( pNewItem, nPos );
        delete pOld;
    }
}

SvLBoxItem* SvLBoxEntry::GetFirstItem( USHORT nId )
{
    USHORT nCount = aItems.Count();
    USHORT nCur = 0;
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
    __DELETE(nItmCnt) pItemData;
}

// ***************************************************************
// class SvLBox
// ***************************************************************

DBG_NAME(SvLBox);


__EXPORT SvLBox::SvLBox( Window* pParent, WinBits nWinStyle ) :
    Control( pParent, nWinStyle | WB_CLIPCHILDREN )
{
    DBG_CTOR(SvLBox,0);
    nWindowStyle = nWinStyle;
    nDragOptions = DRAG_ALL;
    nImpFlags = 0;
    pTargetEntry = 0;
    nDragDropMode = 0;
    pReserved = 0;
    nReserved = 0;
    SvLBoxTreeList* pTempModel = new SvLBoxTreeList;
    pTempModel->SetRefCount( 0 );
    SetModel( pTempModel );
    pModel->SetCloneLink( LINK(this, SvLBox, CloneHdl_Impl ));
    pModel->InsertView( this );
    pHdlEntry = 0;
    pEdCtrl = 0;
    aMovePtr = Pointer( POINTER_MOVEDATA );
    aCopyPtr = Pointer( POINTER_COPYDATA );
    SetSelectionMode( SINGLE_SELECTION );  // pruefen ob TreeListBox gecallt wird
    SetDragDropMode( SV_DRAGDROP_NONE );
#ifdef MAC
    Font aFont( "Geneva", Size( 0, 10 ) );
    SetFont( aFont );
#endif
}

__EXPORT SvLBox::SvLBox( Window* pParent, const ResId& rResId ) :
    Control( pParent, rResId )
{
    DBG_CTOR(SvLBox,0);
    pTargetEntry = 0;
    nImpFlags = 0;
    nWindowStyle = 0;
    pReserved = 0;
    nReserved = 0;
    nDragOptions = DRAG_ALL;
    nDragDropMode = 0;
    SvLBoxTreeList* pTempModel = new SvLBoxTreeList;
    pTempModel->SetRefCount( 0 );
    SetModel( pTempModel );
    pModel->InsertView( this );
    pHdlEntry = 0;
    pEdCtrl = 0;
    pModel->SetCloneLink( LINK(this, SvLBox, CloneHdl_Impl ));
    aMovePtr = Pointer( POINTER_MOVEDATA );
    aCopyPtr = Pointer( POINTER_COPYDATA );
#ifdef MAC
    Font aFont( "Geneva", Size( 0, 10 ) );
    SetFont( aFont );
#endif
}


__EXPORT SvLBox::~SvLBox()
{
    DBG_DTOR(SvLBox,0);
    delete pEdCtrl;
    pEdCtrl = 0;
    pModel->RemoveView( this );
    if ( pModel->GetRefCount() == 0 )
    {
        pModel->Clear();
        delete pModel;
    }
    if( nReserved )
    {
        *((BOOL*)nReserved) = TRUE;
    }
}

void SvLBox::SetModel( SvLBoxTreeList* pNewModel )
{
    DBG_CHKTHIS(SvLBox,0);
    // erledigt das ganz CleanUp
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
    SvListView::SetModel( pNewModel );
}

void SvLBox::Clear()
{
    DBG_CHKTHIS(SvLBox,0);
    pModel->Clear();  // Model ruft SvLBox::ModelHasCleared() auf
}


USHORT SvLBox::IsA()
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

ULONG SvLBox::Insert( SvLBoxEntry* pEntry, SvLBoxEntry* pParent, ULONG nPos )
{
    DBG_CHKTHIS(SvLBox,0);
    ULONG nInsPos = pModel->Insert( pEntry, pParent, nPos );
    return nInsPos;
}

ULONG SvLBox::Insert( SvLBoxEntry* pEntry,ULONG nRootPos )
{
    DBG_CHKTHIS(SvLBox,0);
    ULONG nInsPos = pModel->Insert( pEntry, nRootPos );
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

BOOL SvLBox::DoubleClickHdl()
{
    DBG_CHKTHIS(SvLBox,0);
    aDoubleClickHdl.Call( this );
    return TRUE;
}

void __EXPORT SvLBox::BeginDrag( const Point& rPos )
{
    DBG_CHKTHIS(SvLBox,0);
    BOOL bDeleted = FALSE;

    ReleaseMouse();
    SvLBoxDDInfo aDDInfo;
    SvLBoxEntry* pEntry = GetEntry( rPos ); // GetDropTarget( rPos );
    if ( !pEntry )
    {
        EndDrag();
        return;
    }

    DragServer::Clear();

    DragDropMode nTemp = GetDragDropMode();
    nDragDropMode = NotifyBeginDrag(pEntry);
    if( !nDragDropMode || GetSelectionCount() == 0 )
    {
        nDragDropMode = nTemp;
        EndDrag();
        return;
    }

    ULONG nDDFormatId = SOT_FORMATSTR_ID_TREELISTBOX;
    memset(&aDDInfo,0,sizeof(SvLBoxDDInfo));
    aDDInfo.pApp = GetpApp();
    aDDInfo.pSource = this;
    aDDInfo.pDDStartEntry = pEntry;
    // abgeleitete Views zum Zuge kommen lassen
    WriteDragServerInfo( rPos, &aDDInfo );
    DragServer::CopyData( &aDDInfo, sizeof(SvLBoxDDInfo), nDDFormatId );
    pDDSource = this;
    pDDTarget = 0;
    DropAction eAction;

    BOOL bOldUpdateMode = Control::IsUpdateMode();
    Control::SetUpdateMode( TRUE );
    Update();
    Control::SetUpdateMode( bOldUpdateMode );

    // Selektion & deren Childs im Model als DropTargets sperren
    // Wichtig: Wenn im DropHandler die Selektion der
    // SourceListBox veraendert wird, muessen vorher die Eintraege
    // als DropTargets wieder freigeschaltet werden:
    // (GetSourceListBox()->EnableSelectionAsDropTarget( TRUE, TRUE );)
    EnableSelectionAsDropTarget( FALSE, TRUE /* with Childs */ );

    Region aRegion( GetDragRegion() ); // fuer die Mac-Leute
    nReserved = (ULONG)&bDeleted;
    eAction = ExecuteDrag( aMovePtr, aCopyPtr, nDragOptions, &aRegion );
    nReserved = 0;
    if( bDeleted )
    {
        EndDrag();
        return;
    }

    EnableSelectionAsDropTarget( TRUE, TRUE );

    if( (eAction == DROP_MOVE) &&
       (
        (pDDTarget && ((ULONG)(pDDTarget->GetModel())!=(ULONG)(this->GetModel()))) ||
        !pDDTarget ))
    {
        RemoveSelection();
    }

    ImplShowTargetEmphasis( pTargetEntry, FALSE );
    EndDrag();
    nDragDropMode = nTemp;
}


void SvLBox::EndDrag()
{
    DBG_CHKTHIS(SvLBox,0);
    pDDSource = 0;
    pDDTarget = 0;
    pTargetEntry = 0;
}

BOOL SvLBox::CheckDragAndDropMode( SvLBox* pSource, DropAction eAction )
{
    DBG_CHKTHIS(SvLBox,0);
    if ( pSource == this )
    {
        if ( !(nDragDropMode & (SV_DRAGDROP_CTRL_MOVE | SV_DRAGDROP_CTRL_COPY) ) )
            return FALSE; // D&D innerhalb der Liste gesperrt
        if ( eAction == DROP_MOVE )
        {
            if ( !(nDragDropMode & SV_DRAGDROP_CTRL_MOVE) )
                 return FALSE; // kein lokales Move
        }
        else
        {
            if ( !(nDragDropMode & SV_DRAGDROP_CTRL_COPY))
                return FALSE; // kein lokales Copy
        }
    }
    else
    {
        if ( !(nDragDropMode & SV_DRAGDROP_APP_DROP ) )
            return FALSE; // kein Drop
        if ( eAction == DROP_MOVE )
        {
            if ( !(nDragDropMode & SV_DRAGDROP_APP_MOVE) )
                return FALSE; // kein globales Move
        }
        else
        {
            if ( !(nDragDropMode & SV_DRAGDROP_APP_COPY))
                return FALSE; // kein globales Copy
        }
    }
    return TRUE;
}


BOOL __EXPORT SvLBox::QueryDrop( DropEvent& rDEvt )
{
    DBG_CHKTHIS(SvLBox,0);

    if ( rDEvt.IsLeaveWindow() || !CheckDragAndDropMode( pDDSource, rDEvt.GetAction() ) )
    {
        ImplShowTargetEmphasis( pTargetEntry, FALSE );
        return FALSE;
    }

    if ( !nDragDropMode )
    {
        DBG_ERRORFILE( "SvLBox::QueryDrop(): no target" );
        return FALSE;
    }

    BOOL bAllowDrop = TRUE;

    ULONG nDDFormatId = SOT_FORMATSTR_ID_TREELISTBOX;
    if ( !DragServer::HasFormat( 0, nDDFormatId ) )
    {
        DBG_ERRORFILE( "SvLBox::QueryDrop(): no format" );
        bAllowDrop = FALSE;
    }

    SvLBoxEntry* pEntry = GetDropTarget( rDEvt.GetPosPixel() );

    if ( bAllowDrop )
    {
        DBG_ASSERT( pDDSource, "SvLBox::QueryDrop(): SourceBox == 0 (__EXPORT?)" );
        if (    pEntry && pDDSource->GetModel() == this->GetModel()
            &&  rDEvt.GetAction()==DROP_MOVE
            &&  ( pEntry->nEntryFlags & SV_ENTRYFLAG_DISABLE_DROP ) )
        {
            bAllowDrop = FALSE; // nicht auf sich selbst moven
        }
    }
    if ( bAllowDrop )
        bAllowDrop = NotifyQueryDrop( pEntry );


    // **** Emphasis zeichnen ****

    if ( bAllowDrop )
    {
        if ( pEntry != pTargetEntry || !(nImpFlags & SVLBOX_TARGEMPH_VIS) )
        {
            ImplShowTargetEmphasis( pTargetEntry, FALSE );
            pTargetEntry = pEntry;
            ImplShowTargetEmphasis( pTargetEntry, TRUE );
        }
    }
    else
       ImplShowTargetEmphasis( pTargetEntry, FALSE );

    return bAllowDrop;
}

BOOL __EXPORT SvLBox::Drop( const DropEvent& rDEvt )
{
    DBG_CHKTHIS(SvLBox,0);
    GetSourceView()->EnableSelectionAsDropTarget( TRUE, TRUE );

    ImplShowTargetEmphasis( pTargetEntry, FALSE );
    pDDTarget = this;
    ULONG nDDFormatId = SOT_FORMATSTR_ID_TREELISTBOX;

    SvLBoxDDInfo aDDInfo;
    if( !DragServer::PasteData(0, &aDDInfo, sizeof(SvLBoxDDInfo),nDDFormatId ))
        return FALSE;
    ReadDragServerInfo( rDEvt.GetPosPixel(), &aDDInfo );

    SvLBoxEntry* pTarget = pTargetEntry; // !!! kann 0 sein !!!
    BOOL bDataAccepted;
    if ( rDEvt.GetAction() == DROP_COPY )
        bDataAccepted = CopySelection( aDDInfo.pSource, pTarget );
    else
        bDataAccepted = MoveSelection( aDDInfo.pSource, pTarget );
    return bDataAccepted;
}

DragDropMode SvLBox::NotifyBeginDrag( SvLBoxEntry* )
{
    DBG_CHKTHIS(SvLBox,0);
    return (DragDropMode)0xffff;
}

BOOL SvLBox::NotifyQueryDrop( SvLBoxEntry* )
{
    DBG_CHKTHIS(SvLBox,0);
    return TRUE;
}

void SvLBox::NotifyRemoving( SvLBoxEntry* )
{
    DBG_CHKTHIS(SvLBox,0);
}

/*
    NotifyMoving/Copying
    ====================

    Standard-Verhalten:

    1. Target hat keine Childs
        - Entry wird Sibling des Targets. Entry steht hinter dem
          Target (->Fenster: Unter dem Target)
    2. Target ist ein aufgeklappter Parent
        - Entry wird an den Anfang der Target-Childlist gehaengt
    3. Target ist ein zugeklappter Parent
        - Entry wird an das Ende der Target-Childlist gehaengt
*/

BOOL SvLBox::NotifyMoving(
    SvLBoxEntry*  pTarget,       // D&D-Drop-Position in this->GetModel()
    SvLBoxEntry*  pEntry,        // Zu verschiebender Entry aus
                                 // GetSourceListBox()->GetModel()
    SvLBoxEntry*& rpNewParent,   // Neuer Target-Parent
    ULONG&        rNewChildPos)  // Position in Childlist des Target-Parents
{
    DBG_CHKTHIS(SvLBox,0);
    DBG_ASSERT(pEntry,"NotifyMoving:SoureEntry?")
    if( !pTarget )
    {
        rpNewParent = 0;
        rNewChildPos = 0;
        return TRUE;
    }
    if ( !pTarget->HasChilds() && !pTarget->HasChildsOnDemand() )
    {
        // Fall 1
        rpNewParent = GetParent( pTarget );
        rNewChildPos = pModel->GetRelPos( pTarget ) + 1;
        rNewChildPos += nCurEntrySelPos;
        nCurEntrySelPos++;
    }
    else
    {
        // Faelle 2 & 3
        rpNewParent = pTarget;
        if( IsExpanded(pTarget))
            rNewChildPos = 0;
        else
            rNewChildPos = LIST_APPEND;
    }
    return TRUE;
}

BOOL SvLBox::NotifyCopying(
    SvLBoxEntry*  pTarget,       // D&D-Drop-Position in this->GetModel()
    SvLBoxEntry*  pEntry,        // Zu kopierender Entry aus
                                 // GetSourceListBox()->GetModel()
    SvLBoxEntry*& rpNewParent,   // Neuer Target-Parent
    ULONG&        rNewChildPos)  // Position in Childlist des Target-Parents
{
    DBG_CHKTHIS(SvLBox,0);
    return NotifyMoving(pTarget,pEntry,rpNewParent,rNewChildPos);
    /*
    DBG_ASSERT(pEntry,"NotifyCopying:SourceEntry?")
    if( !pTarget )
    {
        rpNewParent = 0;
        rNewChildPos = 0;
        return TRUE;
    }
    if ( !pTarget->HasChilds() && !pTarget->HasChildsOnDemand() )
    {
        // Fall 1
        rpNewParent = GetParent( pTarget );
        rNewChildPos = GetRelPos( pTarget ) + 1;
    }
    else
    {
        // Faelle 2 & 3
        rpNewParent = pTarget;
        if( IsExpanded(pTarget))
            rNewChildPos = 0;
        else
            rNewChildPos = LIST_APPEND;
    }
    return TRUE;
    */
}

SvLBoxEntry* __EXPORT SvLBox::CloneEntry( SvLBoxEntry* pSource )
{
    DBG_CHKTHIS(SvLBox,0);
    SvLBoxEntry* pEntry = (SvLBoxEntry*)CreateEntry(); // new SvLBoxEntry;
    pEntry->Clone( (SvListEntry*)pSource );
    return pEntry;
}


// Rueckgabe: Alle Entries wurden kopiert
BOOL SvLBox::CopySelection( SvLBox* pSource, SvLBoxEntry* pTarget )
{
    DBG_CHKTHIS(SvLBox,0);
    nCurEntrySelPos = 0; // Selektionszaehler fuer NotifyMoving/Copying
    BOOL bSuccess = TRUE;
    SvTreeEntryList aList;
    BOOL bClone = (BOOL)( (ULONG)(pSource->GetModel()) != (ULONG)GetModel() );
    Link aCloneLink( pModel->GetCloneLink() );
    pModel->SetCloneLink( LINK(this, SvLBox, CloneHdl_Impl ));

    // Selektion zwischenspeichern, um bei D&D-Austausch
    // innerhalb der gleichen Listbox das Iterieren ueber
    // die Selektion zu vereinfachen
    SvLBoxEntry* pSourceEntry = pSource->FirstSelected();
    while ( pSourceEntry )
    {
        // Childs werden automatisch mitkopiert
        pSource->SelectChilds( pSourceEntry, FALSE );
        aList.Insert( pSourceEntry, LIST_APPEND );
        pSourceEntry = pSource->NextSelected( pSourceEntry );
    }

    pSourceEntry = (SvLBoxEntry*)aList.First();
    while ( pSourceEntry )
    {
        SvLBoxEntry* pNewParent = 0;
        ULONG nInsertionPos = LIST_APPEND;
        BOOL bOk=NotifyCopying(pTarget,pSourceEntry,pNewParent,nInsertionPos);
        if ( bOk )
        {
            if ( bClone )
            {
                ULONG nCloneCount = 0;
                pSourceEntry = (SvLBoxEntry*)
                    pModel->Clone( (SvListEntry*)pSourceEntry, nCloneCount );
                pModel->InsertTree( (SvListEntry*)pSourceEntry,
                                    (SvListEntry*)pNewParent, nInsertionPos );
            }
            else
            {
                ULONG nListPos = pModel->Copy( (SvListEntry*)pSourceEntry,
                    (SvListEntry*)pNewParent, nInsertionPos );
                pSourceEntry = GetEntry( pNewParent, nListPos );
            }
        }
        else
            bSuccess = FALSE;

        if( bOk == (BOOL)2 )  // !!!HACK  verschobenen Entry sichtbar machen?
            MakeVisible( pSourceEntry );

        pSourceEntry = (SvLBoxEntry*)aList.Next();
    }
    pModel->SetCloneLink( aCloneLink );
    return bSuccess;
}

// Rueckgabe: Alle Entries wurden verschoben
BOOL SvLBox::MoveSelection( SvLBox* pSource, SvLBoxEntry* pTarget )
{
    DBG_CHKTHIS(SvLBox,0);
    nCurEntrySelPos = 0; // Selektionszaehler fuer NotifyMoving/Copying
    BOOL bSuccess = TRUE;
    SvTreeEntryList aList;
    BOOL bClone = (BOOL)( (ULONG)(pSource->GetModel()) != (ULONG)GetModel() );
    Link aCloneLink( pModel->GetCloneLink() );
    if ( bClone )
        pModel->SetCloneLink( LINK(this, SvLBox, CloneHdl_Impl ));

    SvLBoxEntry* pSourceEntry = pSource->FirstSelected();
    while ( pSourceEntry )
    {
        // Childs werden automatisch mitbewegt
        pSource->SelectChilds( pSourceEntry, FALSE );
        aList.Insert( pSourceEntry, LIST_APPEND );
        pSourceEntry = pSource->NextSelected( pSourceEntry );
    }

    pSourceEntry = (SvLBoxEntry*)aList.First();
    while ( pSourceEntry )
    {
        SvLBoxEntry* pNewParent = 0;
        ULONG nInsertionPos = LIST_APPEND;
        BOOL bOk= NotifyMoving(pTarget,pSourceEntry,pNewParent,nInsertionPos);
        if ( bOk )
        {
            if ( bClone )
            {
                ULONG nCloneCount = 0;
                pSourceEntry = (SvLBoxEntry*)
                    pModel->Clone( (SvListEntry*)pSourceEntry, nCloneCount );
                pModel->InsertTree( (SvListEntry*)pSourceEntry,
                                    (SvListEntry*)pNewParent, nInsertionPos );
            }
            else
                pModel->Move( (SvListEntry*)pSourceEntry,
                              (SvListEntry*)pNewParent, nInsertionPos );
        }
        else
            bSuccess = FALSE;

        if( bOk == (BOOL)2 )  // !!!HACK  verschobenen Entry sichtbar machen?
            MakeVisible( pSourceEntry );

        pSourceEntry = (SvLBoxEntry*)aList.Next();
    }
    pModel->SetCloneLink( aCloneLink );
    return bSuccess;
}

void SvLBox::RemoveSelection()
{
    DBG_CHKTHIS(SvLBox,0);
    SvTreeEntryList aList;
    // Selektion zwischenspeichern, da die Impl bei
    // dem ersten Remove alles deselektiert!
    SvLBoxEntry* pEntry = FirstSelected();
    while ( pEntry )
    {
        aList.Insert( pEntry );
        if ( pEntry->HasChilds() )
            // Remove loescht Childs automatisch
            SelectChilds( pEntry, FALSE );
        pEntry = NextSelected( pEntry );
    }
    pEntry = (SvLBoxEntry*)aList.First();
    while ( pEntry )
    {
        pModel->Remove( pEntry );
        pEntry = (SvLBoxEntry*)aList.Next();
    }
}


SvLBox* __EXPORT SvLBox::GetSourceView() const
    { return pDDSource; }

SvLBox* __EXPORT SvLBox::GetTargetView() const
    { return pDDTarget; }

void SvLBox::RequestingChilds( SvLBoxEntry*  )
{
    DBG_CHKTHIS(SvLBox,0);
    DBG_ERROR("Child-Request-Hdl not implemented!")
}

void SvLBox::RecalcViewData()
{
    DBG_CHKTHIS(SvLBox,0);
    SvLBoxEntry* pEntry = First();
    while( pEntry )
    {
        USHORT nCount = pEntry->ItemCount();
        USHORT nCurPos = 0;
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


void SvLBox::ImplShowTargetEmphasis( SvLBoxEntry* pEntry, BOOL bShow)
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

void SvLBox::ShowTargetEmphasis( SvLBoxEntry*, BOOL /* bShow */ )
{
    DBG_CHKTHIS(SvLBox,0);
}


BOOL SvLBox::Expand( SvLBoxEntry* )
{
    DBG_CHKTHIS(SvLBox,0);
    return TRUE;
}

BOOL SvLBox::Collapse( SvLBoxEntry* )
{
    DBG_CHKTHIS(SvLBox,0);
    return TRUE;
}

BOOL SvLBox::Select( SvLBoxEntry*, BOOL  )
{
    DBG_CHKTHIS(SvLBox,0);
    return FALSE;
}

ULONG SvLBox::SelectChilds( SvLBoxEntry* , BOOL  )
{
    DBG_CHKTHIS(SvLBox,0);
    return 0;
}

void SvLBox::SelectAll( BOOL /* bSelect */ , BOOL /* bPaint */ )
{
    DBG_CHKTHIS(SvLBox,0);
}

void SvLBox::SetSelectionMode( SelectionMode eSelectMode )
{
    DBG_CHKTHIS(SvLBox,0);
    eSelMode = eSelectMode;
}

void SvLBox::SetDragDropMode( DragDropMode nDDMode )
{
    DBG_CHKTHIS(SvLBox,0);
    if( nDDMode && !nDragDropMode )
        EnableDrop();
    nDragDropMode = nDDMode;
}

SvViewData* SvLBox::CreateViewData( SvListEntry* pEntry )
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
    pEntryData->nItmCnt = pInhEntry->ItemCount(); // Anzahl Items fuer delete
    USHORT nCount = pInhEntry->ItemCount();
    USHORT nCurPos = 0;
    while( nCurPos < nCount )
    {
        SvLBoxItem* pItem = pInhEntry->GetItem( nCurPos );
        pItem->InitViewData( this, pInhEntry, pItemData );
        pItemData++;
        nCurPos++;
    }
}



void SvLBox::EnableSelectionAsDropTarget( BOOL bEnable, BOOL bWithChilds )
{
    DBG_CHKTHIS(SvLBox,0);
    USHORT nRefDepth;
    SvLBoxEntry* pTemp;

    SvLBoxEntry* pSelEntry = FirstSelected();
    while( pSelEntry )
    {
        if ( !bEnable )
        {
            pSelEntry->nEntryFlags |= SV_ENTRYFLAG_DISABLE_DROP;
            if ( bWithChilds )
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
            if ( bWithChilds )
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

void SvLBox::EditText( const XubString& rStr, const Rectangle& rRect,
    const Selection& rSel )
{
    EditText( rStr, rRect, rSel, FALSE );
}

void SvLBox::EditText( const XubString& rStr, const Rectangle& rRect,
    const Selection& rSel, BOOL bMulti )
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

IMPL_LINK( SvLBox, TextEditEndedHdl_Impl, SvInplaceEdit2 *, pSvInplaceEdit )
{
    DBG_CHKTHIS(SvLBox,0);
    if ( nImpFlags & SVLBOX_EDTEND_CALLED ) // Nesting verhindern
        return 0;
    nImpFlags |= SVLBOX_EDTEND_CALLED;
    XubString aStr;
    if ( !pEdCtrl->EditingCanceled() )
        aStr = pEdCtrl->GetText();
    else
        aStr = pEdCtrl->GetSavedValue();
    EditedText( aStr );
    // Hide darf erst gerufen werden, nachdem der neue Text in den
    // Entry gesetzt wurde, damit im GetFocus der ListBox nicht
    // der Selecthandler mit dem alten EntryText gerufen wird.
    pEdCtrl->Hide();
    // delete pEdCtrl;
    // pEdCtrl = 0;
    nImpFlags &= (~SVLBOX_IN_EDT);
//  GrabFocus();
    return 0;
}

void SvLBox::CancelTextEditing()
{
    DBG_CHKTHIS(SvLBox,0);
    if ( pEdCtrl )
        pEdCtrl->StopEditing( TRUE );
    nImpFlags &= (~SVLBOX_IN_EDT);
}

void SvLBox::EndEditing( BOOL bCancel )
{
    DBG_CHKTHIS(SvLBox,0);
    if( pEdCtrl )
        pEdCtrl->StopEditing( bCancel );
    nImpFlags &= (~SVLBOX_IN_EDT);
}


void SvLBox::EditedText( const XubString& )
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

Region SvLBox::GetDragRegion() const
{
    DBG_CHKTHIS(SvLBox,0);
    Region aRegion;
    return aRegion;
}

void SvLBox::Command( const CommandEvent& )
{
    DBG_CHKTHIS(SvLBox,0);
}

SvLBoxEntry* SvLBox::GetEntry( const Point& rPos, BOOL ) const
{
    DBG_CHKTHIS(SvLBox,0);
    return 0;
}

void __EXPORT SvLBox::ModelHasEntryInvalidated( SvListEntry* pEntry )
{
    DBG_CHKTHIS(SvLBox,0);
    USHORT nCount = ((SvLBoxEntry*)pEntry)->ItemCount();
    for( USHORT nIdx = 0; nIdx < nCount; nIdx++ )
    {
        SvLBoxItem* pItem = ((SvLBoxEntry*)pEntry)->GetItem( nIdx );
        pItem->InitViewData( this, (SvLBoxEntry*)pEntry, 0 );
    }
}

void SvLBox::SetInUseEmphasis( SvLBoxEntry* pEntry, BOOL bInUse )
{
    DBG_CHKTHIS(SvLBox,0);
    DBG_ASSERT(pEntry,"SetInUseEmphasis:No Entry");
    if( bInUse )
    {
        if( !pEntry->HasInUseEmphasis() )
        {
            pEntry->nEntryFlags |= SV_ENTRYFLAG_IN_USE;
            pModel->InvalidateEntry( pEntry );
        }
    }
    else
    {
        if( pEntry->HasInUseEmphasis() )
        {
            pEntry->nEntryFlags &= (~SV_ENTRYFLAG_IN_USE);
            pModel->InvalidateEntry( pEntry );
        }
    }
}

void SvLBox::SetCursorEmphasis( SvLBoxEntry* pEntry, BOOL bCursored )
{
    DBG_CHKTHIS(SvLBox,0);
    DBG_ASSERT(pEntry,"SetInUseEmphasis:No Entry");
    SvViewDataEntry* pViewData = GetViewDataEntry( pEntry );
    if( pViewData && (bCursored != pViewData->IsCursored()) )
    {
        pViewData->SetCursored( bCursored );
        // paintet in allen Views
        // pModel->InvalidateEntry( pEntry );
        // invalidiert nur in dieser View
        ModelHasEntryInvalidated( pEntry );
    }
}

BOOL SvLBox::HasCursorEmphasis( SvLBoxEntry* pEntry ) const
{
    DBG_CHKTHIS(SvLBox,0);
    DBG_ASSERT(pEntry,"SetInUseEmphasis:No Entry");
    SvViewDataEntry* pViewData = GetViewDataEntry( pEntry );
    DBG_ASSERT(pViewData,"Entry not in View");
    return pViewData->IsCursored();
}

void SvLBox::WriteDragServerInfo( const Point&, SvLBoxDDInfo* )
{
    DBG_CHKTHIS(SvLBox,0);
}

void SvLBox::ReadDragServerInfo(const Point&, SvLBoxDDInfo* )
{
    DBG_CHKTHIS(SvLBox,0);
}

BOOL SvLBox::EditingCanceled() const
{
    if( pEdCtrl && pEdCtrl->EditingCanceled() )
        return TRUE;
    return FALSE;
}


