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

#include <sfx2/msg.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/request.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/msgpool.hxx>
#include <vcl/EnumContext.hxx>
#include <svl/whiter.hxx>
#include <svl/itempool.hxx>
#include <svx/svdomedia.hxx>
#include <svx/sdr/contact/viewcontactofsdrmediaobj.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <svx/svxids.hrc>

#include <app.hrc>
#include <createtableobjectbar.hxx>
#include <registerinterfaces.hxx>

#include <strings.hrc>
#include <DrawDocShell.hxx>
#include <ViewShell.hxx>
#include <Window.hxx>
#include <drawview.hxx>
#include <sdmod.hxx>
#include <sdresid.hxx>
#include <drawdoc.hxx>
#include <DrawViewShell.hxx>

#include "tableobjectbar.hxx"

#include <memory>

using namespace sd;
using namespace sd::ui::table;

#define ShellClass_TableObjectBar
#include <sdslots.hxx>

namespace sd { namespace ui { namespace table {

/** creates a table object bar for the given ViewShell */
SfxShell* CreateTableObjectBar( ViewShell& rShell, ::sd::View* pView )
{
    return new TableObjectBar( &rShell, pView );
}

/** registers the interfaces from the table ui */
void RegisterInterfaces(SfxModule* pMod)
{
    TableObjectBar::RegisterInterface(pMod);
}


SFX_IMPL_INTERFACE(TableObjectBar, SfxShell)

void TableObjectBar::InitInterface_Impl()
{
}

TableObjectBar::TableObjectBar( ViewShell* pSdViewShell, ::sd::View* pSdView )
:   SfxShell( pSdViewShell->GetViewShell() )
,   mpView( pSdView )
,   mpViewSh( pSdViewShell )
{
    DrawDocShell* pDocShell = mpViewSh->GetDocSh();
    if( pDocShell )
    {
        SetPool( &pDocShell->GetPool() );
        SetUndoManager( pDocShell->GetUndoManager() );
    }
    SetRepeatTarget( mpView );
    SetName( SdResId( RID_DRAW_TABLE_TOOLBOX ) );
    SetContextName(vcl::EnumContext::GetContextName(vcl::EnumContext::Context::Table));
}

TableObjectBar::~TableObjectBar()
{
    SetRepeatTarget( nullptr );
}

void TableObjectBar::GetState( SfxItemSet& rSet )
{
    if( mpView )
    {
        rtl::Reference< sdr::SelectionController > xController( mpView->getSelectionController() );
        if( xController.is() )
        {
            xController->GetState( rSet );
        }
    }
}

void TableObjectBar::GetAttrState( SfxItemSet& rSet )
{
    DrawViewShell* pDrawViewShell = dynamic_cast< DrawViewShell* >( mpViewSh );
    if( pDrawViewShell )
        pDrawViewShell->GetAttrState( rSet );
}

void TableObjectBar::Execute( SfxRequest& rReq )
{
    if( !mpView )
        return;

    SdrView* pView = mpView;
    SfxBindings* pBindings = &mpViewSh->GetViewFrame()->GetBindings();

    rtl::Reference< sdr::SelectionController > xController( mpView->getSelectionController() );
    sal_uLong nSlotId = rReq.GetSlot();
    if( xController.is() )
    {
        switch( nSlotId )
        {
        case SID_TABLE_INSERT_ROW_DLG:
        case SID_TABLE_INSERT_ROW_BEFORE:
        case SID_TABLE_INSERT_ROW_AFTER:
        case SID_TABLE_INSERT_COL_DLG:
        case SID_TABLE_INSERT_COL_BEFORE:
        case SID_TABLE_INSERT_COL_AFTER:
        {
            ScopedVclPtr<SvxAbstractInsRowColDlg> pDlg;
            if (nSlotId == SID_TABLE_INSERT_ROW_DLG || nSlotId == SID_TABLE_INSERT_COL_DLG)
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                vcl::Window* pWin = mpView->GetViewShell()->GetParentWindow();
                pDlg.disposeAndReset( pFact->CreateSvxInsRowColDlg(pWin ? pWin->GetFrameWeld() : nullptr,
                                                                   nSlotId == SID_TABLE_INSERT_COL_DLG,
                                                                   SD_MOD()->GetSlotPool()->GetSlot(nSlotId)->GetCommand()) );

                if (pDlg->Execute() != 1)
                    break;
            }

            sal_uInt16 nCount = 1;
            bool bInsertAfter = (nSlotId == SID_TABLE_INSERT_ROW_AFTER) || (nSlotId == SID_TABLE_INSERT_COL_AFTER);

            if (nSlotId == SID_TABLE_INSERT_ROW_DLG)
            {
                nCount = pDlg->getInsertCount();
                bInsertAfter = !pDlg->isInsertBefore();
            }
            else if (nSlotId == SID_TABLE_INSERT_COL_DLG)
            {
                nCount = pDlg->getInsertCount();
                bInsertAfter = !pDlg->isInsertBefore();
            }

            if (nSlotId == SID_TABLE_INSERT_ROW_DLG || nSlotId == SID_TABLE_INSERT_ROW_BEFORE || nSlotId == SID_TABLE_INSERT_ROW_AFTER)
                nSlotId = SID_TABLE_INSERT_ROW;
            else
                nSlotId = SID_TABLE_INSERT_COL;

            rReq.AppendItem(SfxInt16Item(static_cast<sal_uInt16>(nSlotId), nCount));
            rReq.AppendItem(SfxBoolItem(SID_TABLE_PARAM_INSERT_AFTER, bInsertAfter));

            rReq.SetSlot( static_cast<sal_uInt16>(nSlotId) );
        }
        }

        xController->Execute( rReq );
    }

    // note: we may be deleted at this point, no more member access possible

    switch( rReq.GetSlot() )
    {
    case SID_ATTR_BORDER:
    case SID_TABLE_MERGE_CELLS:
    case SID_TABLE_SPLIT_CELLS:
    case SID_OPTIMIZE_TABLE:
    case SID_TABLE_DELETE_ROW:
    case SID_TABLE_DELETE_COL:
    case SID_FORMAT_TABLE_DLG:
    case SID_TABLE_INSERT_ROW:
    case SID_TABLE_INSERT_COL:
    {
        pView->AdjustMarkHdl();
        pBindings->Invalidate( SID_TABLE_DELETE_ROW );
        pBindings->Invalidate( SID_TABLE_DELETE_COL );
        pBindings->Invalidate( SID_FRAME_LINESTYLE );
        pBindings->Invalidate( SID_FRAME_LINECOLOR );
        pBindings->Invalidate( SID_ATTR_BORDER );
        pBindings->Invalidate( SID_ATTR_FILL_STYLE );
        pBindings->Invalidate( SID_ATTR_FILL_TRANSPARENCE );
        pBindings->Invalidate( SID_ATTR_FILL_FLOATTRANSPARENCE );
        pBindings->Invalidate( SID_TABLE_MERGE_CELLS );
        pBindings->Invalidate( SID_TABLE_SPLIT_CELLS );
        pBindings->Invalidate( SID_OPTIMIZE_TABLE );
        pBindings->Invalidate( SID_TABLE_VERT_BOTTOM );
        pBindings->Invalidate( SID_TABLE_VERT_CENTER );
        pBindings->Invalidate( SID_TABLE_VERT_NONE );
        break;
    }
    case SID_TABLE_VERT_BOTTOM:
    case SID_TABLE_VERT_CENTER:
    case SID_TABLE_VERT_NONE:
    {
        pBindings->Invalidate( SID_TABLE_VERT_BOTTOM );
        pBindings->Invalidate( SID_TABLE_VERT_CENTER );
        pBindings->Invalidate( SID_TABLE_VERT_NONE );
        break;
    }
    }

    pBindings->Invalidate( SID_UNDO );
    pBindings->Invalidate( SID_REDO );
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
