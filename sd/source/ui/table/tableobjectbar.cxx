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
#include "precompiled_sd.hxx"

#include <sfx2/msg.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/request.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/msgpool.hxx>
#include <svl/whiter.hxx>
#include <svl/itempool.hxx>
#include <svx/svdomedia.hxx>
#include <svx/sdr/contact/viewcontactofsdrmediaobj.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <svx/svxids.hrc>

#include "app.hrc"
#include "res_bmp.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "DrawDocShell.hxx"
#include "ViewShell.hxx"
#include "Window.hxx"
#include "drawview.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "DrawViewShell.hxx"

#include "tableobjectbar.hxx"

using namespace sd;
using namespace sd::ui::table;

#define TableObjectBar
#include "sdslots.hxx"


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

// ------------------
// - TableObjectBar -
// ------------------

TYPEINIT1( TableObjectBar, SfxShell );

// -----------------------------------------------------------------------------

SFX_IMPL_INTERFACE( TableObjectBar, SfxShell, SdResId( STR_TABLEOBJECTBARSHELL ) )
{
}

// -----------------------------------------------------------------------------

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
    SetHelpId( SD_IF_SDDRAWTABLEOBJECTBAR );
    SetName( String( SdResId( RID_DRAW_TABLE_TOOLBOX ) ) );
}

// -----------------------------------------------------------------------------

TableObjectBar::~TableObjectBar()
{
    SetRepeatTarget( NULL );
}

// -----------------------------------------------------------------------------

void TableObjectBar::GetState( SfxItemSet& rSet )
{
    bool bReadOnly = false;

    if( bReadOnly )
    {
        rSet.DisableItem (SID_INSERT_TABLE );
    }

    if( mpView )
    {
        rtl::Reference< sdr::SelectionController > xController( mpView->getSelectionController() );
        if( xController.is() )
        {
            xController->GetState( rSet );
        }
    }
}

// -----------------------------------------------------------------------------

void TableObjectBar::GetAttrState( SfxItemSet& rSet )
{
    DrawViewShell* pDrawViewShell = dynamic_cast< DrawViewShell* >( mpViewSh );
    if( pDrawViewShell )
        pDrawViewShell->GetAttrState( rSet );
}

// -----------------------------------------------------------------------------

void TableObjectBar::Execute( SfxRequest& rReq )
{
    if( mpView )
    {
        SdrView* pView = mpView;
        SfxBindings* pBindings = &mpViewSh->GetViewFrame()->GetBindings();

        rtl::Reference< sdr::SelectionController > xController( mpView->getSelectionController() );
        sal_uLong nSlotId = rReq.GetSlot();
        if( xController.is() )
        {
            switch( nSlotId )
            {
            case SID_TABLE_INSERT_ROW_DLG:
            case SID_TABLE_INSERT_COL_DLG:
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                ::std::auto_ptr<SvxAbstractInsRowColDlg> pDlg( pFact ? pFact->CreateSvxInsRowColDlg( mpView->GetViewShell()->GetParentWindow(), nSlotId == SID_TABLE_INSERT_COL_DLG, SD_MOD()->GetSlotPool()->GetSlot(nSlotId)->GetCommand()) : 0);

                if( pDlg.get() && (pDlg->Execute() == 1) )
                {
                    if( nSlotId == SID_TABLE_INSERT_ROW_DLG )
                        nSlotId = SID_TABLE_INSERT_ROW;
                    else
                        nSlotId = SID_TABLE_INSERT_COL;

                    rReq.AppendItem( SfxInt16Item( (sal_uInt16)nSlotId, (sal_uInt16)pDlg->getInsertCount() ) );
                    rReq.AppendItem( SfxBoolItem( SID_TABLE_PARAM_INSERT_AFTER, !pDlg->isInsertBefore() ) );

                     rReq.SetSlot( (sal_uInt16)nSlotId );
                }
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
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
