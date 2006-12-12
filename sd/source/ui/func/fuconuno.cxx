/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuconuno.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:16:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "fuconuno.hxx"

#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif



#ifndef _SVX_FMGLOB_HXX
#include <svx/fmglob.hxx>
#endif

#include <svx/dialogs.hrc>

class SbModule;


#include "app.hrc"
#include "glob.hrc"

#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#ifndef SD_TOOL_BAR_MANAGER_HXX
#include "ToolBarManager.hxx"
#endif
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "res_bmp.hrc"

namespace sd {

TYPEINIT1( FuConstructUnoControl, FuConstruct );

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

    SFX_REQUEST_ARG( rReq, pInventorItem, SfxUInt32Item, SID_FM_CONTROL_INVENTOR, FALSE );
    SFX_REQUEST_ARG( rReq, pIdentifierItem, SfxUInt16Item, SID_FM_CONTROL_IDENTIFIER, FALSE );
    if( pInventorItem )
        nInventor = pInventorItem->GetValue();
    if( pIdentifierItem )
        nIdentifier = pIdentifierItem->GetValue();

    mpViewShell->GetViewShellBase().GetToolBarManager().SetToolBar(
        ToolBarManager::TBG_FUNCTION,
        ToolBarManager::msDrawingObjectToolBar);
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/
BOOL FuConstructUnoControl::MouseButtonDown(const MouseEvent& rMEvt)
{
    BOOL bReturn = FuConstruct::MouseButtonDown(rMEvt);

    if ( rMEvt.IsLeft() && !mpView->IsAction() )
    {
        Point aPnt( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        mpWindow->CaptureMouse();
        USHORT nDrgLog = USHORT ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
        mpView->BegCreateObj(aPnt, (OutputDevice*) NULL, nDrgLog);
        bReturn = TRUE;
    }
    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/
BOOL FuConstructUnoControl::MouseMove(const MouseEvent& rMEvt)
{
    return FuConstruct::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/
BOOL FuConstructUnoControl::MouseButtonUp(const MouseEvent& rMEvt)
{
    BOOL bReturn = FALSE;

    if ( mpView->IsCreateObj() && rMEvt.IsLeft() )
    {
        Point aPnt( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        mpView->EndCreateObj(SDRCREATE_FORCEEND);
        bReturn = TRUE;
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
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/
BOOL FuConstructUnoControl::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FuConstruct::KeyInput(rKEvt);
    return(bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/
void FuConstructUnoControl::Activate()
{
    mpView->SetCurrentObj( nIdentifier, nInventor );

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
SdrObject* FuConstructUnoControl::CreateDefaultObject(const sal_uInt16, const Rectangle& rRectangle)
{
    // case SID_FM_CREATE_CONTROL:

    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        mpView->GetCurrentObjInventor(), mpView->GetCurrentObjIdentifier(),
        0L, mpDoc);

    if(pObj)
    {
        pObj->SetLogicRect(rRectangle);
    }

    return pObj;
}

} // end of namespace sd
