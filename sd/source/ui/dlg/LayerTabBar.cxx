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

#include "LayerTabBar.hxx"
#include <svx/svdlayer.hxx>
#include <svx/svdpagv.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/dispatch.hxx>


#include "sdattr.hxx"
#include "app.hxx"
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

#define SWITCH_TIMEOUT  20


/*************************************************************************
|*
|* Standard-Konstruktor
|*
\************************************************************************/

LayerTabBar::LayerTabBar(DrawViewShell* pViewSh, Window* pParent)
    : TabBar( pParent, WinBits( WB_BORDER | WB_3DLOOK | WB_SCROLL | WB_SIZEABLE ) ),
    DropTargetHelper( this ),
    pDrViewSh(pViewSh)
{
    EnableEditMode();
    SetSizePixel(Size(0, 0));
    SetMaxPageWidth( 150 );
    SetHelpId( HID_SD_TABBAR_LAYERS );
}


LayerTabBar::LayerTabBar (
    DrawViewShell* pViewSh,
    Window* pParent,
    const ResId& rResId)
    : TabBar (pParent, rResId.GetWinBits()),
    DropTargetHelper( this ),
    pDrViewSh(pViewSh)
{
    EnableEditMode();
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

LayerTabBar::~LayerTabBar()
{
}

/*************************************************************************
|*
\************************************************************************/

void LayerTabBar::Select()
{
    SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
    pDispatcher->Execute(SID_SWITCHLAYER, SFX_CALLMODE_ASYNCHRON);
}

/*************************************************************************
|*
\************************************************************************/

void LayerTabBar::MouseButtonDown(const MouseEvent& rMEvt)
{
    BOOL bSetPageID=FALSE;

    if (rMEvt.IsLeft() && !rMEvt.IsMod1() && !rMEvt.IsMod2())
    {
        Point aPosPixel = rMEvt.GetPosPixel();
        USHORT aLayerId = GetPageId( PixelToLogic(aPosPixel) );

        if (aLayerId == 0)
        {
            SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
            pDispatcher->Execute(SID_INSERTLAYER, SFX_CALLMODE_SYNCHRON);

            bSetPageID=TRUE;
        }
        else if (rMEvt.IsShift())
        {
            // Toggle zw. Layer sichtbar / unsichtbar
            String aName(GetPageText(aLayerId));
            SdrPageView* pPV = pDrViewSh->GetView()->GetSdrPageView();
            BOOL bVisible = pPV->IsLayerVisible(aName);
            pPV->SetLayerVisible(aName, !bVisible);
            pDrViewSh->ResetActualLayer();
        }
    }

    // If you insert a new layer you must not call TabBar::MouseButtonDown(rMEvt);
    // because you want to activate the new layer
    if( !bSetPageID )
        TabBar::MouseButtonDown(rMEvt);
}

/*************************************************************************
|*
\************************************************************************/

void LayerTabBar::DoubleClick()
{
    if (GetCurPageId() != 0)
    {
        SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
        pDispatcher->Execute( SID_MODIFYLAYER, SFX_CALLMODE_SYNCHRON );
    }
}


/*************************************************************************
|*
|* AcceptDrop-Event
|*
\************************************************************************/

sal_Int8 LayerTabBar::AcceptDrop( const AcceptDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;

    if( rEvt.mbLeaving )
        EndSwitchPage();

    if( !pDrViewSh->GetDocSh()->IsReadOnly() )
    {
        USHORT          nPageId = SDRPAGE_NOTFOUND;
        Point           aPos( PixelToLogic( rEvt.maPosPixel ) );
        USHORT          nLayerId = pDrViewSh->GetView()->GetDoc()->GetLayerAdmin().GetLayerID( GetPageText( GetPageId( aPos ) ), FALSE );

        nRet = pDrViewSh->AcceptDrop( rEvt, *this, NULL, nPageId, nLayerId );

        SwitchPage( aPos );
    }

    return nRet;
}

/*************************************************************************
|*
|* ExecuteDrop-Event
|*
\************************************************************************/

sal_Int8 LayerTabBar::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    USHORT          nPageId = SDRPAGE_NOTFOUND;
    USHORT          nLayerId = pDrViewSh->GetView()->GetDoc()->GetLayerAdmin().GetLayerID( GetPageText( GetPageId( PixelToLogic( rEvt.maPosPixel ) ) ), FALSE );
    sal_Int8        nRet = pDrViewSh->ExecuteDrop( rEvt, *this, NULL, nPageId, nLayerId );

    EndSwitchPage();

    return nRet;

}

/*************************************************************************
|*
\************************************************************************/

void  LayerTabBar::Command(const CommandEvent& rCEvt)
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU )
    {
        SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
        pDispatcher->ExecutePopup(SdResId(RID_LAYERTAB_POPUP));
    }
}


/*************************************************************************
|*
\************************************************************************/
long LayerTabBar::StartRenaming()
{
    BOOL bOK = TRUE;
    String aLayerName = GetPageText( GetEditPageId() );
    String aLayoutLayer ( SdResId(STR_LAYER_LAYOUT) );
    String aControlsLayer ( SdResId(STR_LAYER_CONTROLS) );
    String aMeasureLinesLayer ( SdResId(STR_LAYER_MEASURELINES) );
    String aBackgroundLayer( SdResId(STR_LAYER_BCKGRND) );
    String aBackgroundObjLayer( SdResId(STR_LAYER_BCKGRNDOBJ) );

    if ( aLayerName == aLayoutLayer       || aLayerName == aControlsLayer  ||
         aLayerName == aMeasureLinesLayer ||
         aLayerName == aBackgroundLayer   || aLayerName == aBackgroundObjLayer )
    {
        // Diese Namen duerfen nicht veraendert werden
        bOK = FALSE;
    }
    else
    {
        ::sd::View* pView = pDrViewSh->GetView();

        if ( pView->IsTextEdit() )
        {
            pView->SdrEndTextEdit();
        }
    }

    return(bOK);
}

/*************************************************************************
|*
\************************************************************************/

long LayerTabBar::AllowRenaming()
{
    BOOL bOK = TRUE;

    // Ueberpruefung auf schon vorhandene Namen
    ::sd::View* pView = pDrViewSh->GetView();
    SdDrawDocument* pDoc = pView->GetDoc();
    String aLayerName = pView->GetActiveLayer();
    SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
    String aNewName( GetEditText() );

    if ( aNewName.Len() == 0 ||
        (rLayerAdmin.GetLayer( aNewName, FALSE ) && aLayerName != aNewName) )
    {
        // Name ist schon vorhanden
        WarningBox aWarningBox( &pDrViewSh->GetViewFrame()->GetWindow(), WinBits( WB_OK ),
                                String(SdResId( STR_WARN_NAME_DUPLICATE ) ) );
        aWarningBox.Execute();
        bOK = FALSE;
    }

    if (bOK)
    {
        String aLayoutLayer ( SdResId(STR_LAYER_LAYOUT) );
        String aControlsLayer ( SdResId(STR_LAYER_CONTROLS) );
        String aMeasureLinesLayer ( SdResId(STR_LAYER_MEASURELINES) );
        String aBackgroundLayer( SdResId(STR_LAYER_BCKGRND) );
        String aBackgroundObjLayer( SdResId(STR_LAYER_BCKGRNDOBJ) );

        if ( aNewName == aLayoutLayer       || aNewName == aControlsLayer  ||
             aNewName == aMeasureLinesLayer ||
             aNewName == aBackgroundLayer   || aNewName == aBackgroundObjLayer )
        {
            // Diese Namen duerfen nicht vergeben werden
            bOK = FALSE;
        }
    }

    return(bOK);
}

/*************************************************************************
|*
\************************************************************************/

void LayerTabBar::EndRenaming()
{
    if( !IsEditModeCanceled() )
    {
        ::sd::View* pView = pDrViewSh->GetView();
        DrawView* pDrView = PTR_CAST( DrawView, pView );

        SdDrawDocument* pDoc = pView->GetDoc();
        String aLayerName = pView->GetActiveLayer();
        SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
        SdrLayer* pLayer = rLayerAdmin.GetLayer(aLayerName, FALSE);

        if (pLayer)
        {
            String aNewName( GetEditText() );

            DBG_ASSERT( pDrView, "Rename layer undo action is only working with a SdDrawView" );
            if( pDrView )
            {
                ::svl::IUndoManager* pManager = pDoc->GetDocSh()->GetUndoManager();
                SdLayerModifyUndoAction* pAction = new SdLayerModifyUndoAction(
                    pDoc,
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

            // Zuerst View benachrichtigen, da innerhalb von SetName() schon
            // ResetActualLayer() gerufen wird und an der View der Layer dann
            // schon bekannt sein muss.
            pView->SetActiveLayer(aNewName);
            pLayer->SetName(aNewName);
            pDoc->SetChanged(TRUE);
        }
    }
}


/*************************************************************************
|*
\************************************************************************/

void LayerTabBar::ActivatePage()
{
    if ( /*IsInSwitching*/ 1 && pDrViewSh!=NULL)
    {

        SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
        pDispatcher->Execute(SID_SWITCHLAYER, SFX_CALLMODE_ASYNCHRON);
    }
}




void LayerTabBar::SendActivatePageEvent (void)
{
    CallEventListeners (VCLEVENT_TABBAR_PAGEACTIVATED,
        reinterpret_cast<void*>(GetCurPageId()));
}




void LayerTabBar::SendDeactivatePageEvent (void)
{
    CallEventListeners (VCLEVENT_TABBAR_PAGEDEACTIVATED,
        reinterpret_cast<void*>(GetCurPageId()));
}

} // end of namespace sd
