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

#include "LayerTabBar.hxx"
#include <svx/svdlayer.hxx>
#include <svx/svdpagv.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/dispatch.hxx>

#include "sdattr.hxx"
#include "sdmod.hxx"
#include "helpids.h"
#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "DrawViewShell.hxx"
#include "Window.hxx"
#include "View.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "DrawDocShell.hxx"
#include "drawview.hxx"
#include "undolayer.hxx"

namespace sd {

/**
 * default constructor
 */
LayerTabBar::LayerTabBar(DrawViewShell* pViewSh, vcl::Window* pParent)
    : TabBar( pParent, WinBits( WB_BORDER | WB_3DLOOK | WB_SCROLL ) ),
    DropTargetHelper( this ),
    pDrViewSh(pViewSh)
{
    EnableEditMode();
    SetSizePixel(Size(0, 0));
    SetMaxPageWidth( 150 );
    SetHelpId( HID_SD_TABBAR_LAYERS );
}

LayerTabBar::~LayerTabBar()
{
}

void LayerTabBar::Select()
{
    SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
    pDispatcher->Execute(SID_SWITCHLAYER, SfxCallMode::ASYNCHRON);
}

void LayerTabBar::MouseButtonDown(const MouseEvent& rMEvt)
{
    bool bSetPageID=false;

    if (rMEvt.IsLeft() && !rMEvt.IsMod1() && !rMEvt.IsMod2())
    {
        Point aPosPixel = rMEvt.GetPosPixel();
        sal_uInt16 aLayerId = GetPageId( PixelToLogic(aPosPixel) );

        if (aLayerId == 0)
        {
            SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
            pDispatcher->Execute(SID_INSERTLAYER, SfxCallMode::SYNCHRON);

            bSetPageID=true;
        }
        else if (rMEvt.IsShift())
        {
            // Toggle between layer visible / hidden
            OUString aName(GetPageText(aLayerId));
            SdrPageView* pPV = pDrViewSh->GetView()->GetSdrPageView();
            bool bVisible = pPV->IsLayerVisible(aName);
            pPV->SetLayerVisible(aName, !bVisible);
            pDrViewSh->ResetActualLayer();
        }
    }

    // If you insert a new layer you must not call TabBar::MouseButtonDown(rMEvt);
    // because you want to activate the new layer
    if( !bSetPageID )
        TabBar::MouseButtonDown(rMEvt);
}

void LayerTabBar::DoubleClick()
{
    if (GetCurPageId() != 0)
    {
        SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
        pDispatcher->Execute( SID_MODIFYLAYER, SfxCallMode::SYNCHRON );
    }
}

/**
 * AcceptDrop-Event
 */

sal_Int8 LayerTabBar::AcceptDrop( const AcceptDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;

    if( rEvt.mbLeaving )
        EndSwitchPage();

    if( !pDrViewSh->GetDocSh()->IsReadOnly() )
    {
        sal_uInt16          nPageId = SDRPAGE_NOTFOUND;
        Point           aPos( PixelToLogic( rEvt.maPosPixel ) );
        sal_uInt16 nLayerId = pDrViewSh->GetView()->GetDoc().GetLayerAdmin().GetLayerID(
            GetPageText( GetPageId( aPos ) ), false );

        nRet = pDrViewSh->AcceptDrop( rEvt, *this, nullptr, nPageId, nLayerId );

        SwitchPage( aPos );
    }

    return nRet;
}

/**
 * ExecuteDrop-Event
 */
sal_Int8 LayerTabBar::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    sal_uInt16          nPageId = SDRPAGE_NOTFOUND;
    sal_uInt16 nLayerId = pDrViewSh->GetView()->GetDoc().GetLayerAdmin().GetLayerID(
        GetPageText( GetPageId( PixelToLogic( rEvt.maPosPixel ) ) ), false );
    sal_Int8        nRet = pDrViewSh->ExecuteDrop( rEvt, *this, nullptr, nPageId, nLayerId );

    EndSwitchPage();

    return nRet;

}

void  LayerTabBar::Command(const CommandEvent& rCEvt)
{
    if ( rCEvt.GetCommand() == CommandEventId::ContextMenu )
    {
        SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
        pDispatcher->ExecutePopup(SdResId(RID_LAYERTAB_POPUP));
    }
}

bool LayerTabBar::StartRenaming()
{
    bool bOK = true;
    OUString aLayerName = GetPageText( GetEditPageId() );
    OUString aLayoutLayer = SD_RESSTR(STR_LAYER_LAYOUT);
    OUString aControlsLayer = SD_RESSTR(STR_LAYER_CONTROLS);
    OUString aMeasureLinesLayer = SD_RESSTR(STR_LAYER_MEASURELINES);
    OUString aBackgroundLayer = SD_RESSTR(STR_LAYER_BCKGRND);
    OUString aBackgroundObjLayer = SD_RESSTR(STR_LAYER_BCKGRNDOBJ);

    if ( aLayerName == aLayoutLayer       || aLayerName == aControlsLayer  ||
         aLayerName == aMeasureLinesLayer ||
         aLayerName == aBackgroundLayer   || aLayerName == aBackgroundObjLayer )
    {
        // It is not allowed to change this names
        bOK = false;
    }
    else
    {
        ::sd::View* pView = pDrViewSh->GetView();

        if ( pView->IsTextEdit() )
        {
            pView->SdrEndTextEdit();
        }
    }

    return bOK;
}

TabBarAllowRenamingReturnCode LayerTabBar::AllowRenaming()
{
    bool bOK = true;

    // Check if names already exists
    ::sd::View* pView = pDrViewSh->GetView();
    SdDrawDocument& rDoc = pView->GetDoc();
    OUString aLayerName = pView->GetActiveLayer();
    SdrLayerAdmin& rLayerAdmin = rDoc.GetLayerAdmin();
    OUString aNewName( GetEditText() );

    if (aNewName.isEmpty() ||
        (rLayerAdmin.GetLayer( aNewName, false ) && aLayerName != aNewName) )
    {
        // Name already exists
        ScopedVclPtrInstance<WarningBox> aWarningBox( &pDrViewSh->GetViewFrame()->GetWindow(), WinBits( WB_OK ),
                                SD_RESSTR( STR_WARN_NAME_DUPLICATE ) );
        aWarningBox->Execute();
        bOK = false;
    }

    if (bOK)
    {
        OUString aLayoutLayer = SD_RESSTR(STR_LAYER_LAYOUT);
        OUString aControlsLayer = SD_RESSTR(STR_LAYER_CONTROLS);
        OUString aMeasureLinesLayer = SD_RESSTR(STR_LAYER_MEASURELINES);
        OUString aBackgroundLayer = SD_RESSTR(STR_LAYER_BCKGRND);
        OUString aBackgroundObjLayer = SD_RESSTR(STR_LAYER_BCKGRNDOBJ);

        if ( aNewName == aLayoutLayer       || aNewName == aControlsLayer  ||
             aNewName == aMeasureLinesLayer ||
             aNewName == aBackgroundLayer   || aNewName == aBackgroundObjLayer )
        {
            // It is not allowed to use his names
            bOK = false;
        }
    }

    return bOK ? TABBAR_RENAMING_YES : TABBAR_RENAMING_NO;
}

void LayerTabBar::EndRenaming()
{
    if( !IsEditModeCanceled() )
    {
        ::sd::View* pView = pDrViewSh->GetView();
        DrawView* pDrView = dynamic_cast<DrawView*>( pView  );

        SdDrawDocument& rDoc = pView->GetDoc();
        OUString aLayerName = pView->GetActiveLayer();
        SdrLayerAdmin& rLayerAdmin = rDoc.GetLayerAdmin();
        SdrLayer* pLayer = rLayerAdmin.GetLayer(aLayerName, false);

        if (pLayer)
        {
            OUString aNewName( GetEditText() );

            DBG_ASSERT( pDrView, "Rename layer undo action is only working with a SdDrawView" );
            if( pDrView )
            {
                ::svl::IUndoManager* pManager = rDoc.GetDocSh()->GetUndoManager();
                SdLayerModifyUndoAction* pAction = new SdLayerModifyUndoAction(
                    &rDoc,
                    pLayer,
                    aLayerName,
                    pLayer->GetTitle(),
                    pLayer->GetDescription(),
                    pDrView->IsLayerVisible(aLayerName),
                    pDrView->IsLayerLocked(aLayerName),
                    pDrView->IsLayerPrintable(aLayerName),
                    aNewName,
                    pLayer->GetTitle(),
                    pLayer->GetDescription(),
                    pDrView->IsLayerVisible(aLayerName),
                    pDrView->IsLayerLocked(aLayerName),
                    pDrView->IsLayerPrintable(aLayerName)
                    );
                pManager->AddUndoAction( pAction );
            }

            // First notify View since SetName() calls ResetActualLayer() and
            // the View then already has to know the Layer
            pView->SetActiveLayer(aNewName);
            pLayer->SetName(aNewName);
            rDoc.SetChanged();
        }
    }
}

void LayerTabBar::ActivatePage()
{
    if ( /*IsInSwitching*/ true && pDrViewSh!=nullptr)
    {

        SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
        pDispatcher->Execute(SID_SWITCHLAYER, SfxCallMode::ASYNCHRON);
    }
}

void LayerTabBar::SendActivatePageEvent()
{
    CallEventListeners (VCLEVENT_TABBAR_PAGEACTIVATED,
        reinterpret_cast<void*>(GetCurPageId()));
}

void LayerTabBar::SendDeactivatePageEvent()
{
    CallEventListeners (VCLEVENT_TABBAR_PAGEDEACTIVATED,
        reinterpret_cast<void*>(GetCurPageId()));
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
