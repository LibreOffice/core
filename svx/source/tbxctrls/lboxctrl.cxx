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
#include "precompiled_svx.hxx"

#ifdef   _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#include <vcl/lstbox.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/event.hxx>
#include <sfx2/app.hxx>
#include <sfx2/tbxctrl.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewsh.hxx>
#include <tools/gen.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svtools/stdctrl.hxx>
#include <svl/slstitm.hxx>
#include <svl/stritem.hxx>
#include <svx/dialmgr.hxx>
#include <lboxctrl.hxx>
#include <vcl/mnemonic.hxx>
#include <tools/urlobj.hxx>

#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>

#include "lboxctrl.hrc"


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;

class SvxPopupWindowListBox;

/////////////////////////////////////////////////////////////////

class SvxPopupWindowListBox : public SfxPopupWindow
{
    using FloatingWindow::StateChanged;

    ListBox *       pListBox;
    ToolBox &       rToolBox;
    BOOL            bUserSel;
    USHORT          nTbxId;
    rtl::OUString   maCommandURL;
    // disallow copy-constructor and assignment-operator

    SvxPopupWindowListBox(const int& );
    SvxPopupWindowListBox & operator = (const int& );

//  SvxPopupWindowListBox( USHORT nSlotId, ToolBox& rTbx, USHORT nTbxItemId );

public:
    SvxPopupWindowListBox( USHORT nSlotId, const rtl::OUString& rCommandURL, USHORT nTbxId, ToolBox& rTbx );
    virtual ~SvxPopupWindowListBox();

    // SfxPopupWindow
    virtual SfxPopupWindow *    Clone() const;
    virtual void                PopupModeEnd();
    virtual void                StateChanged( USHORT nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );

    void                        StartSelection();
    inline ListBox &            GetListBox()    { return *pListBox; }

    BOOL                        IsUserSelected() const          { return bUserSel; }
    void                        SetUserSelected( BOOL bVal )    { bUserSel = bVal; }
    /*virtual*/Window*                     GetPreferredKeyInputWindow();
};

/////////////////////////////////////////////////////////////////

SvxPopupWindowListBox::SvxPopupWindowListBox( USHORT nSlotId, const rtl::OUString& rCommandURL, USHORT nId, ToolBox& rTbx ) :
    SfxPopupWindow( nSlotId, Reference< XFrame >(), SVX_RES( RID_SVXTBX_UNDO_REDO_CTRL ) ),
    rToolBox    ( rTbx ),
    bUserSel    ( FALSE ),
    nTbxId      ( nId ),
    maCommandURL( rCommandURL )
{
    DBG_ASSERT( nSlotId == GetId(), "id mismatch" );
    pListBox = new ListBox( this, SVX_RES( LB_SVXTBX_UNDO_REDO_CTRL ) );
    FreeResource();
    pListBox->EnableMultiSelection( TRUE, TRUE );
    SetBackground( GetSettings().GetStyleSettings().GetDialogColor() );
    AddStatusListener( rCommandURL );
}


SvxPopupWindowListBox::~SvxPopupWindowListBox()
{
    delete pListBox;
}


SfxPopupWindow* SvxPopupWindowListBox::Clone() const
{
    return new SvxPopupWindowListBox( GetId(), maCommandURL, nTbxId, rToolBox );
}


void SvxPopupWindowListBox::PopupModeEnd()
{
    rToolBox.EndSelection();
    SfxPopupWindow::PopupModeEnd();
    //FloatingWindow::PopupModeEnd();

    if( SfxViewShell::Current() )
    {
        Window* pShellWnd = SfxViewShell::Current()->GetWindow();
        if (pShellWnd)
            pShellWnd->GrabFocus();
    }
}


void SvxPopupWindowListBox::StateChanged(
        USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    rToolBox.EnableItem( nTbxId, ( SfxToolBoxControl::GetItemState( pState ) != SFX_ITEM_DISABLED) );
    SfxPopupWindow::StateChanged( nSID, eState, pState );
}


void SvxPopupWindowListBox::StartSelection()
{
    rToolBox.StartSelection();
}

Window* SvxPopupWindowListBox::GetPreferredKeyInputWindow()
{
    // allows forwarding key events in the correct window
    // without setting the focus
    return pListBox->GetPreferredKeyInputWindow();
}

/////////////////////////////////////////////////////////////////

SFX_IMPL_TOOLBOX_CONTROL( SvxListBoxControl, SfxStringItem );


SvxListBoxControl::SvxListBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx )
    :SfxToolBoxControl( nSlotId, nId, rTbx ),
    pPopupWin   ( 0 )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
    rTbx.Invalidate();
}


SvxListBoxControl::~SvxListBoxControl()
{
}


SfxPopupWindow* SvxListBoxControl::CreatePopupWindow()
{
    OSL_FAIL( "not implemented" );
    return 0;
}


SfxPopupWindowType SvxListBoxControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONTIMEOUT;
}


void SvxListBoxControl::StateChanged(
        USHORT, SfxItemState, const SfxPoolItem* pState )
{
    GetToolBox().EnableItem( GetId(),
                            SFX_ITEM_DISABLED != GetItemState(pState) );
}


IMPL_LINK( SvxListBoxControl, PopupModeEndHdl, void *, EMPTYARG )
{
    if( pPopupWin && 0 == pPopupWin->GetPopupModeFlags()  &&
        pPopupWin->IsUserSelected() )
    {
        USHORT nCount = pPopupWin->GetListBox().GetSelectEntryCount();

        INetURLObject aObj( m_aCommandURL );

        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name   = aObj.GetURLPath();
        aArgs[0].Value  = makeAny( sal_Int16( nCount ));
        SfxToolBoxControl::Dispatch( m_aCommandURL, aArgs );
    }
    return 0;
}


void SvxListBoxControl::Impl_SetInfo( USHORT nCount )
{
    DBG_ASSERT( pPopupWin, "NULL pointer, PopupWindow missing" );

//    ListBox &rListBox = pPopupWin->GetListBox();

    USHORT nId;
    if (nCount == 1)
        nId = SID_UNDO == GetSlotId() ? RID_SVXSTR_NUM_UNDO_ACTION : RID_SVXSTR_NUM_REDO_ACTION;
    else
        nId = SID_UNDO == GetSlotId() ? RID_SVXSTR_NUM_UNDO_ACTIONS : RID_SVXSTR_NUM_REDO_ACTIONS;

    aActionStr = String(SVX_RES(nId));

    String aText( aActionStr );
    aText.SearchAndReplaceAllAscii( "$(ARG1)", String::CreateFromInt32( nCount ) );
    pPopupWin->SetText( aText );
}


IMPL_LINK( SvxListBoxControl, SelectHdl, void *, EMPTYARG )
{
    if (pPopupWin)
    {
        //pPopupWin->SetUserSelected( FALSE );

        ListBox &rListBox = pPopupWin->GetListBox();
        if (rListBox.IsTravelSelect())
            Impl_SetInfo( rListBox.GetSelectEntryCount() );
        else
        {
            pPopupWin->SetUserSelected( TRUE );
            pPopupWin->EndPopupMode( 0 );
        }
    }
    return 0;
}

/////////////////////////////////////////////////////////////////

SFX_IMPL_TOOLBOX_CONTROL( SvxUndoRedoControl, SfxStringItem );

SvxUndoRedoControl::SvxUndoRedoControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx )
    : SvxListBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
    rTbx.Invalidate();
    aDefaultText = MnemonicGenerator::EraseAllMnemonicChars( rTbx.GetItemText( nId ) );
}

SvxUndoRedoControl::~SvxUndoRedoControl()
{
}

void SvxUndoRedoControl::StateChanged(
    USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    if ( nSID == SID_UNDO || nSID == SID_REDO )
    {
        if ( eState == SFX_ITEM_DISABLED )
        {
            ToolBox& rBox = GetToolBox();
            rBox.SetQuickHelpText( GetId(), aDefaultText );
        }
        else if ( pState && pState->ISA( SfxStringItem ) )
        {
            SfxStringItem& rItem = *(SfxStringItem *)pState;
            ToolBox& rBox = GetToolBox();
            String aQuickHelpText = MnemonicGenerator::EraseAllMnemonicChars( rItem.GetValue() );
            rBox.SetQuickHelpText( GetId(), aQuickHelpText );
        }
        SvxListBoxControl::StateChanged( nSID, eState, pState );
    }
    else
    {
        aUndoRedoList.clear();

        if ( pState && pState->ISA( SfxStringListItem ) )
        {
            SfxStringListItem &rItem = *(SfxStringListItem *)pState;
            const List* pLst = rItem.GetList();
            DBG_ASSERT( pLst, "no undo actions available" );
            if ( pLst )
            {
                for( long nI = 0, nEnd = pLst->Count(); nI < nEnd; ++nI )
                    aUndoRedoList.push_back( rtl::OUString( *(String *)pLst->GetObject( nI )));
            }
        }
    }
}

SfxPopupWindow* SvxUndoRedoControl::CreatePopupWindow()
{
    DBG_ASSERT(( SID_UNDO == GetSlotId() || SID_REDO == GetSlotId() ), "mismatching ids" );

    if ( m_aCommandURL.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ".uno:Undo" ) ))
        updateStatus( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:GetUndoStrings" )));
    else
        updateStatus( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:GetRedoStrings" )));

    ToolBox& rBox = GetToolBox();

    pPopupWin = new SvxPopupWindowListBox( GetSlotId(), m_aCommandURL, GetId(), rBox );
    pPopupWin->SetPopupModeEndHdl( LINK( this, SvxUndoRedoControl,
                                            PopupModeEndHdl ) );
    ListBox &rListBox = pPopupWin->GetListBox();
    rListBox.SetSelectHdl( LINK( this, SvxUndoRedoControl, SelectHdl ) );

    for( sal_uInt32 n = 0; n < aUndoRedoList.size(); n++ )
        rListBox.InsertEntry( String( aUndoRedoList[n] ));

    rListBox.SelectEntryPos( 0 );
    aActionStr = String( SVX_RES( SID_UNDO == GetSlotId() ?
                                  RID_SVXSTR_NUM_UNDO_ACTIONS : RID_SVXSTR_NUM_REDO_ACTIONS ) );
    Impl_SetInfo( rListBox.GetSelectEntryCount() );

    // move focus in floating window without
    // closing it (GrabFocus() would close it!)
    pPopupWin->StartPopupMode( &rBox, FLOATWIN_POPUPMODE_GRABFOCUS );
    //pPopupWin->GetListBox().GrabFocus();

    return pPopupWin;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
