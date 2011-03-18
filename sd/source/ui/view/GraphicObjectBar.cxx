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

#include "GraphicObjectBar.hxx"

#include <limits.h>
#include <vcl/msgbox.hxx>
#include <svl/whiter.hxx>
#include <svl/itempool.hxx>
#include <sfx2/app.hxx>
#include <sfx2/shell.hxx>
#include <svx/svxids.hrc>
#include <sfx2/request.hxx>
#include <sfx2/basedlgs.hxx>
#include <svx/svdograf.hxx>
#include <svx/grfflt.hxx>
#include <svl/aeitem.hxx>
#include <svx/grafctrl.hxx>


#include <sfx2/objface.hxx>

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

using namespace sd;
#define GraphicObjectBar
#include "sdslots.hxx"

namespace sd {



// -----------------------
// - GraphicObjectBar -
// -----------------------

SFX_IMPL_INTERFACE( GraphicObjectBar, SfxShell, SdResId( STR_GRAFOBJECTBARSHELL ) )
{
}

// -----------------------------------------------------------------------------

TYPEINIT1( GraphicObjectBar, SfxShell );

// -----------------------------------------------------------------------------

GraphicObjectBar::GraphicObjectBar (
    ViewShell* pSdViewShell,
    ::sd::View* pSdView )
    : SfxShell (pSdViewShell->GetViewShell()),
      mpView     ( pSdView ),
      mpViewSh ( pSdViewShell ),
      nMappedSlotFilter ( SID_GRFFILTER_INVERT )
{
    DrawDocShell* pDocShell = mpViewSh->GetDocSh();

    SetPool( &pDocShell->GetPool() );
    SetUndoManager( pDocShell->GetUndoManager() );
    SetRepeatTarget( mpView );
    SetHelpId( SD_IF_SDDRAWGRAFOBJECTBAR );
    SetName( String( RTL_CONSTASCII_USTRINGPARAM( "Graphic objectbar" )));
}

// -----------------------------------------------------------------------------

GraphicObjectBar::~GraphicObjectBar()
{
    SetRepeatTarget( NULL );
}

// -----------------------------------------------------------------------------

void GraphicObjectBar::GetAttrState( SfxItemSet& rSet )
{
    if( mpView )
        SvxGrafAttrHelper::GetGrafAttrState( rSet, *mpView );
}

// -----------------------------------------------------------------------------

void GraphicObjectBar::Execute( SfxRequest& rReq )
{
    if( mpView )
    {
        SvxGrafAttrHelper::ExecuteGrafAttr( rReq, *mpView );
        Invalidate();
    }
}

// -----------------------------------------------------------------------------

void GraphicObjectBar::GetFilterState( SfxItemSet& rSet )
{
    const SdrMarkList&  rMarkList = mpView->GetMarkedObjectList();
    sal_Bool                bEnable = sal_False;

    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( pObj && pObj->ISA( SdrGrafObj ) && ( ( (SdrGrafObj*) pObj )->GetGraphicType() == GRAPHIC_BITMAP ) )
            bEnable = sal_True;
    }

    if( !bEnable )
        SvxGraphicFilter::DisableGraphicFilterSlots( rSet );
}

// -----------------------------------------------------------------------------

void GraphicObjectBar::ExecuteFilter( SfxRequest& rReq )
{
    const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( pObj && pObj->ISA( SdrGrafObj ) && ( (SdrGrafObj*) pObj )->GetGraphicType() == GRAPHIC_BITMAP )
        {
            GraphicObject aFilterObj( ( (SdrGrafObj*) pObj )->GetGraphicObject() );

            if( SVX_GRAPHICFILTER_ERRCODE_NONE ==
                SvxGraphicFilter::ExecuteGrfFilterSlot( rReq, aFilterObj ) )
            {
                SdrPageView* pPageView = mpView->GetSdrPageView();

                if( pPageView )
                {
                    SdrGrafObj* pFilteredObj = (SdrGrafObj*) pObj->Clone();
                    String      aStr( mpView->GetDescriptionOfMarkedObjects() );

                    aStr.Append( sal_Unicode(' ') );
                    aStr.Append( String( SdResId( STR_UNDO_GRAFFILTER ) ) );
                    mpView->BegUndo( aStr );
                    pFilteredObj->SetGraphicObject( aFilterObj );
                    ::sd::View* const pView = mpView;
                    pView->ReplaceObjectAtView( pObj, *pPageView, pFilteredObj );
                    pView->EndUndo();
                    return;
                }
            }
        }
    }

    Invalidate();
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
