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
    VclPtr<ListBox> m_pListBox;
    ToolBox &       rToolBox;
    bool            bUserSel;
    sal_uInt16      nTbxId;

public:
    SvxPopupWindowListBox( sal_uInt16 nSlotId, const OUString& rCommandURL, sal_uInt16 nTbxId, ToolBox& rTbx );
    virtual ~SvxPopupWindowListBox() override;
    virtual void dispose() override;

    // SfxPopupWindow
    virtual void                PopupModeEnd() override;
    virtual void                statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;

    ListBox &            GetListBox()    { return *m_pListBox; }

    bool                        IsUserSelected() const          { return bUserSel; }
    void                        SetUserSelected( bool bVal )    { bUserSel = bVal; }
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
    nBits &= ~WB_SIMPLEMODE;
    m_pListBox->SetStyle(nBits);
    Size aSize(LogicToPixel(Size(100, 85), MapUnit::MapAppFont));
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


void SvxPopupWindowListBox::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    rToolBox.EnableItem( nTbxId, rEvent.IsEnabled );
    SfxPopupWindow::statusChanged( rEvent );
}

IMPL_LINK_NOARG(SvxUndoRedoControl, PopupModeEndHdl, FloatingWindow*, void)
{
    if( pPopupWin && FloatWinPopupFlags::NONE == pPopupWin->GetPopupModeFlags()  &&
        pPopupWin->IsUserSelected() )
    {
        const sal_Int32 nCount = pPopupWin->GetListBox().GetSelectEntryCount();

        INetURLObject aObj( m_aCommandURL );

        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name   = aObj.GetURLPath();
        aArgs[0].Value  <<= sal_Int16( nCount );
        SfxToolBoxControl::Dispatch( m_aCommandURL, aArgs );
    }
}


void SvxUndoRedoControl::Impl_SetInfo( sal_Int32 nCount )
{
    DBG_ASSERT( pPopupWin, "NULL pointer, PopupWindow missing" );

//    ListBox &rListBox = pPopupWin->GetListBox();

    sal_uInt16 nId;
    if (nCount == 1)
        nId = SID_UNDO == GetSlotId() ? RID_SVXSTR_NUM_UNDO_ACTION : RID_SVXSTR_NUM_REDO_ACTION;
    else
        nId = SID_UNDO == GetSlotId() ? RID_SVXSTR_NUM_UNDO_ACTIONS : RID_SVXSTR_NUM_REDO_ACTIONS;

    aActionStr = SvxResId(nId);

    OUString aText = aActionStr.replaceAll("$(ARG1)", OUString::number(nCount));
    pPopupWin->SetText(aText);
}


IMPL_LINK_NOARG(SvxUndoRedoControl, SelectHdl, ListBox&, void)
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
    :SfxToolBoxControl( nSlotId, nId, rTbx ),
    pPopupWin   ( nullptr )
{
    rTbx.SetItemBits( nId, ToolBoxItemBits::DROPDOWN | rTbx.GetItemBits( nId ) );
    rTbx.Invalidate();
    aDefaultTooltip = rTbx.GetQuickHelpText( nId );
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
            rBox.SetQuickHelpText( GetId(), aDefaultTooltip );
        }
        else if ( pState && dynamic_cast<const SfxStringItem*>( pState) !=  nullptr )
        {
            const SfxStringItem& rItem = *static_cast<const SfxStringItem *>(pState);
            ToolBox& rBox = GetToolBox();
            const OUString& aQuickHelpText = rItem.GetValue();
            rBox.SetQuickHelpText( GetId(), aQuickHelpText );
        }
        GetToolBox().EnableItem( GetId(),
                                SfxItemState::DISABLED != GetItemState(pState) );
    }
    else
    {
        aUndoRedoList.clear();

        if ( pState && dynamic_cast<const SfxStringListItem*>( pState) !=  nullptr )
        {
            const SfxStringListItem &rItem = *static_cast<const SfxStringListItem *>(pState);

            const std::vector<OUString> &aLst = rItem.GetList();
            for(const auto & i : aLst)
                aUndoRedoList.push_back( i );
        }
    }
}

VclPtr<SfxPopupWindow> SvxUndoRedoControl::CreatePopupWindow()
{
    DBG_ASSERT(( SID_UNDO == GetSlotId() || SID_REDO == GetSlotId() ), "mismatching ids" );

    if ( m_aCommandURL == ".uno:Undo" )
        updateStatus( ".uno:GetUndoStrings");
    else
        updateStatus( ".uno:GetRedoStrings");

    ToolBox& rBox = GetToolBox();

    pPopupWin = VclPtr<SvxPopupWindowListBox>::Create( GetSlotId(), m_aCommandURL, GetId(), rBox );
    pPopupWin->SetPopupModeEndHdl( LINK( this, SvxUndoRedoControl, PopupModeEndHdl ) );
    ListBox &rListBox = pPopupWin->GetListBox();
    rListBox.SetSelectHdl( LINK( this, SvxUndoRedoControl, SelectHdl ) );

    for(const OUString & s : aUndoRedoList)
        rListBox.InsertEntry( s );

    rListBox.SelectEntryPos( 0 );
    aActionStr = SvxResId(SID_UNDO == GetSlotId() ?
                                  RID_SVXSTR_NUM_UNDO_ACTIONS : RID_SVXSTR_NUM_REDO_ACTIONS);
    Impl_SetInfo( rListBox.GetSelectEntryCount() );

    // move focus in floating window without
    // closing it (GrabFocus() would close it!)
    pPopupWin->StartPopupMode( &rBox, FloatWinPopupFlags::GrabFocus );
    //pPopupWin->GetListBox().GrabFocus();

    return pPopupWin;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
