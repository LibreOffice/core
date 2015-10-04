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

#include <sal/config.h>

#include <boost/noncopyable.hpp>
#include <tools/debug.hxx>
#include <sal/types.h>
#include <vcl/lstbox.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/event.hxx>
#include <sfx2/app.hxx>
#include <sfx2/tbxctrl.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewsh.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svtools/stdctrl.hxx>
#include <svl/slstitm.hxx>
#include <svl/stritem.hxx>
#include <svx/dialmgr.hxx>
#include <svx/lboxctrl.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/settings.hxx>
#include <tools/urlobj.hxx>

#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;

class SvxPopupWindowListBox;

class SvxPopupWindowListBox: public SfxPopupWindow
{
    using FloatingWindow::StateChanged;

    VclPtr<ListBox> m_pListBox;
    ToolBox &       rToolBox;
    bool            bUserSel;
    sal_uInt16      nTbxId;

public:
    SvxPopupWindowListBox( sal_uInt16 nSlotId, const OUString& rCommandURL, sal_uInt16 nTbxId, ToolBox& rTbx );
    virtual ~SvxPopupWindowListBox();
    virtual void dispose() SAL_OVERRIDE;

    // SfxPopupWindow
    virtual void                PopupModeEnd() SAL_OVERRIDE;
    virtual void                StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                              const SfxPoolItem* pState ) SAL_OVERRIDE;

    inline ListBox &            GetListBox()    { return *m_pListBox; }

    bool                        IsUserSelected() const          { return bUserSel; }
    void                        SetUserSelected( bool bVal )    { bUserSel = bVal; }
    virtual vcl::Window*             GetPreferredKeyInputWindow() SAL_OVERRIDE;
};

SvxPopupWindowListBox::SvxPopupWindowListBox(sal_uInt16 nSlotId, const OUString& rCommandURL, sal_uInt16 nId, ToolBox& rTbx)
    : SfxPopupWindow(nSlotId, "FloatingUndoRedo", "svx/ui/floatingundoredo.ui")
    , rToolBox(rTbx)
    , bUserSel(false)
    , nTbxId(nId)
{
    DBG_ASSERT( nSlotId == GetId(), "id mismatch" );
    get(m_pListBox, "treeview");
    WinBits nBits(m_pListBox->GetStyle());
    nBits &= ~(WB_SIMPLEMODE);
    m_pListBox->SetStyle(nBits);
    Size aSize(LogicToPixel(Size(100, 85), MAP_APPFONT));
    m_pListBox->set_width_request(aSize.Width());
    m_pListBox->set_height_request(aSize.Height());
    m_pListBox->EnableMultiSelection( true, true );
    SetBackground( GetSettings().GetStyleSettings().GetDialogColor() );
    AddStatusListener( rCommandURL );
}

SvxPopupWindowListBox::~SvxPopupWindowListBox()
{
    disposeOnce();
}

void SvxPopupWindowListBox::dispose()
{
    m_pListBox.clear();
    SfxPopupWindow::dispose();
}

void SvxPopupWindowListBox::PopupModeEnd()
{
    rToolBox.EndSelection();
    SfxPopupWindow::PopupModeEnd();
    //FloatingWindow::PopupModeEnd();

    if( SfxViewShell::Current() )
    {
        vcl::Window* pShellWnd = SfxViewShell::Current()->GetWindow();
        if (pShellWnd)
            pShellWnd->GrabFocus();
    }
}


void SvxPopupWindowListBox::StateChanged(
        sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    rToolBox.EnableItem( nTbxId, ( SfxToolBoxControl::GetItemState( pState ) != SfxItemState::DISABLED) );
    SfxPopupWindow::StateChanged( nSID, eState, pState );
}

vcl::Window* SvxPopupWindowListBox::GetPreferredKeyInputWindow()
{
    // allows forwarding key events in the correct window
    // without setting the focus
    return m_pListBox->GetPreferredKeyInputWindow();
}

SvxListBoxControl::SvxListBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx )
    :SfxToolBoxControl( nSlotId, nId, rTbx ),
    pPopupWin   ( 0 )
{
    rTbx.SetItemBits( nId, ToolBoxItemBits::DROPDOWN | rTbx.GetItemBits( nId ) );
    rTbx.Invalidate();
}


SvxListBoxControl::~SvxListBoxControl()
{}

VclPtr<SfxPopupWindow> SvxListBoxControl::CreatePopupWindow()
{
    OSL_FAIL( "not implemented" );
    return 0;
}


void SvxListBoxControl::StateChanged(
        sal_uInt16, SfxItemState, const SfxPoolItem* pState )
{
    GetToolBox().EnableItem( GetId(),
                            SfxItemState::DISABLED != GetItemState(pState) );
}


IMPL_LINK_NOARG_TYPED(SvxListBoxControl, PopupModeEndHdl, FloatingWindow*, void)
{
    if( pPopupWin && FloatWinPopupFlags::NONE == pPopupWin->GetPopupModeFlags()  &&
        pPopupWin->IsUserSelected() )
    {
        const sal_Int32 nCount = pPopupWin->GetListBox().GetSelectEntryCount();

        INetURLObject aObj( m_aCommandURL );

        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name   = aObj.GetURLPath();
        aArgs[0].Value  = makeAny( sal_Int16( nCount ));
        SfxToolBoxControl::Dispatch( m_aCommandURL, aArgs );
    }
}


void SvxListBoxControl::Impl_SetInfo( sal_Int32 nCount )
{
    DBG_ASSERT( pPopupWin, "NULL pointer, PopupWindow missing" );

//    ListBox &rListBox = pPopupWin->GetListBox();

    sal_uInt16 nId;
    if (nCount == 1)
        nId = SID_UNDO == GetSlotId() ? RID_SVXSTR_NUM_UNDO_ACTION : RID_SVXSTR_NUM_REDO_ACTION;
    else
        nId = SID_UNDO == GetSlotId() ? RID_SVXSTR_NUM_UNDO_ACTIONS : RID_SVXSTR_NUM_REDO_ACTIONS;

    aActionStr = SVX_RESSTR(nId);

    OUString aText = aActionStr.replaceAll("$(ARG1)", OUString::number(nCount));
    pPopupWin->SetText(aText);
}


IMPL_LINK_NOARG_TYPED(SvxListBoxControl, SelectHdl, ListBox&, void)
{
    if (pPopupWin)
    {
        //pPopupWin->SetUserSelected( false );

        ListBox &rListBox = pPopupWin->GetListBox();
        if (rListBox.IsTravelSelect())
            Impl_SetInfo( rListBox.GetSelectEntryCount() );
        else
        {
            pPopupWin->SetUserSelected( true );
            pPopupWin->EndPopupMode();
        }
    }
}



SFX_IMPL_TOOLBOX_CONTROL( SvxUndoRedoControl, SfxStringItem );

SvxUndoRedoControl::SvxUndoRedoControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx )
    : SvxListBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, ToolBoxItemBits::DROPDOWN | rTbx.GetItemBits( nId ) );
    rTbx.Invalidate();
    aDefaultText = MnemonicGenerator::EraseAllMnemonicChars( rTbx.GetItemText( nId ) );
}

SvxUndoRedoControl::~SvxUndoRedoControl()
{
}

void SvxUndoRedoControl::StateChanged(
    sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    if ( nSID == SID_UNDO || nSID == SID_REDO )
    {
        if ( eState == SfxItemState::DISABLED )
        {
            ToolBox& rBox = GetToolBox();
            rBox.SetQuickHelpText( GetId(), aDefaultText );
        }
        else if ( pState && dynamic_cast<const SfxStringItem*>( pState) !=  nullptr )
        {
            const SfxStringItem& rItem = *static_cast<const SfxStringItem *>(pState);
            ToolBox& rBox = GetToolBox();
            OUString aQuickHelpText = MnemonicGenerator::EraseAllMnemonicChars( rItem.GetValue() );
            rBox.SetQuickHelpText( GetId(), aQuickHelpText );
        }
        SvxListBoxControl::StateChanged( nSID, eState, pState );
    }
    else
    {
        aUndoRedoList.clear();

        if ( pState && dynamic_cast<const SfxStringListItem*>( pState) !=  nullptr )
        {
            const SfxStringListItem &rItem = *static_cast<const SfxStringListItem *>(pState);

            const std::vector<OUString> &aLst = rItem.GetList();
            for( long nI = 0, nEnd = aLst.size(); nI < nEnd; ++nI )
                aUndoRedoList.push_back( aLst[nI] );
        }
    }
}

VclPtr<SfxPopupWindow> SvxUndoRedoControl::CreatePopupWindow()
{
    DBG_ASSERT(( SID_UNDO == GetSlotId() || SID_REDO == GetSlotId() ), "mismatching ids" );

    if ( m_aCommandURL == ".uno:Undo" )
        updateStatus( OUString( ".uno:GetUndoStrings" ));
    else
        updateStatus( OUString( ".uno:GetRedoStrings" ));

    ToolBox& rBox = GetToolBox();

    pPopupWin = VclPtr<SvxPopupWindowListBox>::Create( GetSlotId(), m_aCommandURL, GetId(), rBox );
    pPopupWin->SetPopupModeEndHdl( LINK( this, SvxUndoRedoControl, PopupModeEndHdl ) );
    ListBox &rListBox = pPopupWin->GetListBox();
    rListBox.SetSelectHdl( LINK( this, SvxUndoRedoControl, SelectHdl ) );

    for( size_t n = 0; n < aUndoRedoList.size(); n++ )
        rListBox.InsertEntry( aUndoRedoList[n] );

    rListBox.SelectEntryPos( 0 );
    aActionStr = SVX_RESSTR(SID_UNDO == GetSlotId() ?
                                  RID_SVXSTR_NUM_UNDO_ACTIONS : RID_SVXSTR_NUM_REDO_ACTIONS);
    Impl_SetInfo( rListBox.GetSelectEntryCount() );

    // move focus in floating window without
    // closing it (GrabFocus() would close it!)
    pPopupWin->StartPopupMode( &rBox, FloatWinPopupFlags::GrabFocus );
    //pPopupWin->GetListBox().GrabFocus();

    return pPopupWin;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
