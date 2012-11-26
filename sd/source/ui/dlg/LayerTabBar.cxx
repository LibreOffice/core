/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    bool bSetPageID=false;

    if (rMEvt.IsLeft() && !rMEvt.IsMod1() && !rMEvt.IsMod2())
    {
        Point aPosPixel = rMEvt.GetPosPixel();
        sal_uInt16 aLayerId = GetPageId( PixelToLogic(aPosPixel) );

        if (aLayerId == 0)
        {
            SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
            pDispatcher->Execute(SID_INSERTLAYER, SFX_CALLMODE_SYNCHRON);

            bSetPageID=true;
        }
        else if (rMEvt.IsShift())
        {
            // Toggle zw. Layer sichtbar / unsichtbar
            String aName(GetPageText(aLayerId));
            SdrPageView* pPV = pDrViewSh->GetView()->GetSdrPageView();

            if(pPV)
            {
                bool bVisible = pPV->IsLayerVisible(aName);
                pPV->SetLayerVisible(aName, !bVisible);
                pDrViewSh->ResetActualLayer();
            }
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
        const sal_uInt32 nPageId(SDRPAGE_NOTFOUND);
        const Point aPos( PixelToLogic( rEvt.maPosPixel ) );
        const SdrLayerID aLayerId(pDrViewSh->GetView()->GetDoc()->GetModelLayerAdmin().GetLayerID( GetPageText( GetPageId( aPos ) ), false ));

        nRet = pDrViewSh->AcceptDrop( rEvt, *this, NULL, nPageId, aLayerId );

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
    const sal_uInt32 nPageId(SDRPAGE_NOTFOUND);
    const SdrLayerID aLayerId(pDrViewSh->GetView()->GetDoc()->GetModelLayerAdmin().GetLayerID( GetPageText( GetPageId( PixelToLogic( rEvt.maPosPixel ) ) ), false ));
    sal_Int8 nRet = pDrViewSh->ExecuteDrop( rEvt, *this, NULL, nPageId, aLayerId );

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
    bool bOK = true;
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

    return(bOK);
}

/*************************************************************************
|*
\************************************************************************/

long LayerTabBar::AllowRenaming()
{
    bool bOK = true;

    // Ueberpruefung auf schon vorhandene Namen
    ::sd::View* pView = pDrViewSh->GetView();
    SdDrawDocument* pDoc = pView->GetDoc();
    String aLayerName = pView->GetActiveLayer();
    SdrLayerAdmin& rLayerAdmin = pDoc->GetModelLayerAdmin();
    String aNewName( GetEditText() );

    if ( aNewName.Len() == 0 ||
        (rLayerAdmin.GetLayer( aNewName, false ) && aLayerName != aNewName) )
    {
        // Name ist schon vorhanden
        WarningBox aWarningBox( &pDrViewSh->GetViewFrame()->GetWindow(), WinBits( WB_OK ),
                                String(SdResId( STR_WARN_NAME_DUPLICATE ) ) );
        aWarningBox.Execute();
        bOK = false;
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
            bOK = false;
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
        DrawView* pDrView = dynamic_cast< DrawView* >(pView );

        SdDrawDocument* pDoc = pView->GetDoc();
        String aLayerName = pView->GetActiveLayer();
        SdrLayerAdmin& rLayerAdmin = pDoc->GetModelLayerAdmin();
        SdrLayer* pLayer = rLayerAdmin.GetLayer(aLayerName, false);

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
            pDoc->SetChanged(true);
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
