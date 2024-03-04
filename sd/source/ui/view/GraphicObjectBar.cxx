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

#include <GraphicObjectBar.hxx>

#include <sfx2/shell.hxx>
#include <svx/svxids.hrc>
#include <sfx2/request.hxx>
#include <svx/svdograf.hxx>
#include <svx/grfflt.hxx>
#include <svx/grafctrl.hxx>

#include <sfx2/objface.hxx>

#include <strings.hrc>
#include <DrawDocShell.hxx>
#include <ViewShell.hxx>
#include <sdresid.hxx>

using namespace sd;
#define ShellClass_GraphicObjectBar
#include <sdslots.hxx>

namespace sd {


SFX_IMPL_INTERFACE(GraphicObjectBar, SfxShell)

void GraphicObjectBar::InitInterface_Impl()
{
}


GraphicObjectBar::GraphicObjectBar (
    const ViewShell* pSdViewShell,
    ::sd::View* pSdView )
    : SfxShell (pSdViewShell->GetViewShell()),
      mpView   ( pSdView )
{
    DrawDocShell* pDocShell = pSdViewShell->GetDocSh();

    SetPool( &pDocShell->GetPool() );
    SetUndoManager( pDocShell->GetUndoManager() );
    SetRepeatTarget( mpView );
    SetName( "Graphic objectbar");
}

GraphicObjectBar::~GraphicObjectBar()
{
    SetRepeatTarget( nullptr );
}

void GraphicObjectBar::GetAttrState( SfxItemSet& rSet )
{
    if( mpView )
        SvxGrafAttrHelper::GetGrafAttrState( rSet, *mpView );
}

void GraphicObjectBar::Execute( SfxRequest& rReq )
{
    if( mpView )
    {
        SvxGrafAttrHelper::ExecuteGrafAttr( rReq, *mpView );
        Invalidate();
    }
}

void GraphicObjectBar::GetFilterState( SfxItemSet& rSet )
{
    const SdrMarkList&  rMarkList = mpView->GetMarkedObjectList();
    bool                bEnable = false;

    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( auto pGrafObj = dynamic_cast< SdrGrafObj *>( pObj ) )
            if( pGrafObj->GetGraphicType() == GraphicType::Bitmap )
                bEnable = true;
    }

    if( !bEnable )
        SvxGraphicFilter::DisableGraphicFilterSlots( rSet );
}

void GraphicObjectBar::ExecuteFilter( SfxRequest const & rReq )
{
    const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( auto pGrafObj = dynamic_cast< SdrGrafObj *>( pObj ) )
            if( pGrafObj->GetGraphicType() == GraphicType::Bitmap )
            {
                SvxGraphicFilter::ExecuteGrfFilterSlot( rReq, pGrafObj->GetGraphicObject(),
                    [this, pObj] (GraphicObject aFilterObj) -> void
                    {
                        if (SdrPageView* pPageView = mpView->GetSdrPageView())
                        {
                            rtl::Reference<SdrGrafObj> pFilteredObj = SdrObject::Clone(static_cast<SdrGrafObj&>(*pObj), pObj->getSdrModelFromSdrObject());
                            OUString aStr = mpView->GetDescriptionOfMarkedObjects() +
                                " " + SdResId(STR_UNDO_GRAFFILTER);
                            mpView->BegUndo( aStr );
                            pFilteredObj->SetGraphicObject( aFilterObj );
                            ::sd::View* const pView = mpView;
                            pView->ReplaceObjectAtView( pObj, *pPageView, pFilteredObj.get() );
                            pView->EndUndo();
                        }
                    });
                return;
            }
    }
    Invalidate();
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
