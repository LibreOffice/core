/*************************************************************************
 *
 *  $RCSfile: lboxctrl.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 13:19:31 $
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

#ifdef   _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <vcl/event.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFXTBXCTRL_HXX
#include <sfx2/tbxctrl.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _STDCTRL_HXX
#include <svtools/stdctrl.hxx>
#endif
#ifndef _SFXSLSTITM_HXX
#include <svtools/slstitm.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif

#ifndef _SVX_DIALMGR_HXX
#include <dialmgr.hxx>
#endif
#ifndef _SVX_LBOXCTRL_HXX_
#include <lboxctrl.hxx>
#endif
#ifndef _VCL_MNEMONIC_HXX_
#include <vcl/mnemonic.hxx>
#endif
#include <tools/urlobj.hxx>

#include <svxids.hrc>
#include <dialogs.hrc>

#include "lboxctrl.hrc"


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;

class SvxPopupWindowListBox;

/////////////////////////////////////////////////////////////////

class SvxPopupWindowListBox : public SfxPopupWindow
{
    FixedInfo       aInfo;
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
    inline FixedInfo &          GetInfo()       { return aInfo; }

    BOOL                        IsUserSelected() const          { return bUserSel; }
    void                        SetUserSelected( BOOL bVal )    { bUserSel = bVal; }
    /*virtual*/Window*                     GetPreferredKeyInputWindow();
};

/////////////////////////////////////////////////////////////////

SvxPopupWindowListBox::SvxPopupWindowListBox( USHORT nSlotId, const rtl::OUString& rCommandURL, USHORT nId, ToolBox& rTbx ) :
    SfxPopupWindow( nSlotId, Reference< XFrame >(), SVX_RES( RID_SVXTBX_UNDO_REDO_CTRL ) ),
    aInfo       ( this, ResId( FT_NUM_OPERATIONS ) ),
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

    Window* pShellWnd = SfxViewShell::Current()->GetWindow();
    if (pShellWnd)
        pShellWnd->GrabFocus();
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
    DBG_ERROR( "not implemented" );
    return 0;
}


SfxPopupWindowType SvxListBoxControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONTIMEOUT;
}


void SvxListBoxControl::StateChanged(
        USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
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

    ListBox &rListBox = pPopupWin->GetListBox();

    USHORT nId;
    if (nCount == 1)
        nId = SID_UNDO == GetSlotId() ? RID_SVXSTR_NUM_UNDO_ACTION : RID_SVXSTR_NUM_REDO_ACTION;
    else
        nId = SID_UNDO == GetSlotId() ? RID_SVXSTR_NUM_UNDO_ACTIONS : RID_SVXSTR_NUM_REDO_ACTIONS;

    aActionStr = String(SVX_RES(nId));

    String aText( aActionStr );
    aText.SearchAndReplaceAllAscii( "$(ARG1)", String::CreateFromInt32( nCount ) );
    pPopupWin->GetInfo().SetText( aText );
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
}

SvxUndoRedoControl::~SvxUndoRedoControl()
{
}

void SvxUndoRedoControl::StateChanged(
    USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    if ( nSID == SID_UNDO || nSID == SID_REDO )
    {
        if ( pState && pState->ISA( SfxStringItem ))
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

SfxPopupWindow* SvxUndoRedoControl::CreatePopupWindow()
{
    DBG_ASSERT(( SID_UNDO == GetSlotId() || SID_REDO == GetSlotId() ), "mismatching ids" );

    if ( m_aCommandURL.equalsAscii( ".uno:Undo" ))
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
