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

#include "fuconuno.hxx"
#include <svl/aeitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <svl/intitem.hxx>
#include <svx/fmglob.hxx>
#include <svx/dialogs.hrc>
#include <svx/svdlegacy.hxx>
class SbModule;

#include "app.hrc"
#include "glob.hrc"
#include "ViewShell.hxx"
#include "View.hxx"
#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif
#include "ViewShellBase.hxx"
#include "ToolBarManager.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "res_bmp.hrc"

namespace sd {

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConstructUnoControl::FuConstructUnoControl (
    ViewShell*  pViewSh,
    ::sd::Window*       pWin,
    ::sd::View*         pView,
    SdDrawDocument* pDoc,
    SfxRequest&     rReq)
    : FuConstruct(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuConstructUnoControl::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq, bool bPermanent )
{
    FuConstructUnoControl* pFunc;
    FunctionReference xFunc( pFunc = new FuConstructUnoControl( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    pFunc->SetPermanent(bPermanent);
    return xFunc;
}

void FuConstructUnoControl::DoExecute( SfxRequest& rReq )
{
    FuConstruct::DoExecute( rReq );

    SFX_REQUEST_ARG( rReq, pInventorItem, SfxUInt32Item, SID_FM_CONTROL_INVENTOR );
    SFX_REQUEST_ARG( rReq, pIdentifierItem, SfxUInt16Item, SID_FM_CONTROL_IDENTIFIER );
    if( pInventorItem )
        nInventor = pInventorItem->GetValue();
    if( pIdentifierItem )
        nIdentifier = pIdentifierItem->GetValue();

    mpViewShell->GetViewShellBase().GetToolBarManager()->SetToolBar(
        ToolBarManager::TBG_FUNCTION,
        ToolBarManager::msDrawingObjectToolBar);
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/
bool FuConstructUnoControl::MouseButtonDown(const MouseEvent& rMEvt)
{
    bool bReturn = FuConstruct::MouseButtonDown(rMEvt);

    if ( rMEvt.IsLeft() && !mpView->IsAction() )
    {
        const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
        const basegfx::B2DPoint aLogicPos(mpWindow->GetInverseViewTransformation() * aPixelPos);

        mpWindow->CaptureMouse();
        const double fTolerance(basegfx::B2DVector(mpWindow->GetInverseViewTransformation() * basegfx::B2DVector(DRGPIX, 0.0)).getLength());
        mpView->BegCreateObj(aLogicPos, fTolerance);
        bReturn = true;
    }
    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/
bool FuConstructUnoControl::MouseMove(const MouseEvent& rMEvt)
{
    return FuConstruct::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/
bool FuConstructUnoControl::MouseButtonUp(const MouseEvent& rMEvt)
{
    bool bReturn = false;

    if ( mpView->GetCreateObj() && rMEvt.IsLeft() )
    {
        Point aPnt( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        mpView->EndCreateObj(SDRCREATE_FORCEEND);
        bReturn = true;
    }

    bReturn = (FuConstruct::MouseButtonUp(rMEvt) || bReturn);

    if (!bPermanent)
        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);

    return (bReturn);
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert true, andernfalls
|* false.
|*
\************************************************************************/
bool FuConstructUnoControl::KeyInput(const KeyEvent& rKEvt)
{
    bool bReturn = FuConstruct::KeyInput(rKEvt);
    return(bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/
void FuConstructUnoControl::Activate()
{
    mpView->setSdrObjectCreationInfo(SdrObjectCreationInfo(nIdentifier, nInventor));

    aNewPointer = Pointer(POINTER_DRAW_RECT);
    aOldPointer = mpWindow->GetPointer();
    mpWindow->SetPointer( aNewPointer );

    aOldLayer = mpView->GetActiveLayer();
    String aStr(SdResId(STR_LAYER_CONTROLS));
    mpView->SetActiveLayer( aStr );

    FuConstruct::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/
void FuConstructUnoControl::Deactivate()
{
    FuConstruct::Deactivate();
    mpView->SetActiveLayer( aOldLayer );
    mpWindow->SetPointer( aOldPointer );
}

// #97016#
SdrObject* FuConstructUnoControl::CreateDefaultObject(const sal_uInt16, const basegfx::B2DRange& rRange)
{
    // case SID_FM_CREATE_CONTROL:

    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        mpView->getSdrModelFromSdrView(),
        mpView->getSdrObjectCreationInfo());

    if(pObj)
    {
        sdr::legacy::SetLogicRange(*pObj, rRange);
    }

    return pObj;
}

} // end of namespace sd
