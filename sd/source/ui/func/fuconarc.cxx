/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuconarc.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 14:27:20 $
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

#include "fuconarc.hxx"

#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SVDOCIRC_HXX //autogen
#include <svx/svdocirc.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif


#include <svx/svxids.hrc>
#include <math.h>

#include "app.hrc"

#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#include "drawdoc.hxx"
#include "res_bmp.hrc"
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#ifndef SD_TOOL_BAR_MANAGER_HXX
#include "ToolBarManager.hxx"
#endif

// #97016#
#ifndef _SXCIAITM_HXX
#include <svx/sxciaitm.hxx>
#endif

namespace sd {

TYPEINIT1( FuConstructArc, FuConstruct );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConstructArc::FuConstructArc (
    ViewShell*  pViewSh,
    ::sd::Window*       pWin,
    ::sd::View*         pView,
    SdDrawDocument* pDoc,
    SfxRequest&     rReq )
    : FuConstruct( pViewSh, pWin, pView, pDoc, rReq )
{
}

FunctionReference FuConstructArc::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq, bool bPermanent  )
{
    FuConstructArc* pFunc;
    FunctionReference xFunc( pFunc = new FuConstructArc( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    pFunc->SetPermanent(bPermanent);
    return xFunc;
}

void FuConstructArc::DoExecute( SfxRequest& rReq )
{
    FuConstruct::DoExecute( rReq );

    pViewShell->GetViewShellBase().GetToolBarManager().SetToolBar(
        ToolBarManager::TBG_FUNCTION,
        ToolBarManager::msDrawingObjectToolBar);

    const SfxItemSet *pArgs = rReq.GetArgs ();

    if (pArgs)
    {
        SFX_REQUEST_ARG (rReq, pCenterX, SfxUInt32Item, ID_VAL_CENTER_X, FALSE);
        SFX_REQUEST_ARG (rReq, pCenterY, SfxUInt32Item, ID_VAL_CENTER_Y, FALSE);
        SFX_REQUEST_ARG (rReq, pAxisX, SfxUInt32Item, ID_VAL_AXIS_X, FALSE);
        SFX_REQUEST_ARG (rReq, pAxisY, SfxUInt32Item, ID_VAL_AXIS_Y, FALSE);
        SFX_REQUEST_ARG (rReq, pPhiStart, SfxUInt32Item, ID_VAL_ANGLESTART, FALSE);
        SFX_REQUEST_ARG (rReq, pPhiEnd, SfxUInt32Item, ID_VAL_ANGLEEND, FALSE);

        Rectangle   aNewRectangle (pCenterX->GetValue () - pAxisX->GetValue () / 2,
                                   pCenterY->GetValue () - pAxisY->GetValue () / 2,
                                   pCenterX->GetValue () + pAxisX->GetValue () / 2,
                                   pCenterY->GetValue () + pAxisY->GetValue () / 2);

        Activate();  // Setzt aObjKind
        SdrCircObj* pNewCircle =
        new SdrCircObj((SdrObjKind) pView->GetCurrentObjIdentifier(),
                       aNewRectangle,
                       (long) (pPhiStart->GetValue () * 10.0),
                       (long) (pPhiEnd->GetValue () * 10.0));
        SdrPageView *pPV = pView->GetSdrPageView();

        pView->InsertObjectAtView(pNewCircle, *pPV, SDRINSERT_SETDEFLAYER);
    }
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

BOOL FuConstructArc::MouseButtonDown( const MouseEvent& rMEvt )
{
    BOOL bReturn = FuConstruct::MouseButtonDown( rMEvt );

    if ( rMEvt.IsLeft() && !pView->IsAction() )
    {
        Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        pWindow->CaptureMouse();
        USHORT nDrgLog = USHORT ( pWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
        pView->BegCreateObj(aPnt, (OutputDevice*) NULL, nDrgLog);

        SdrObject* pObj = pView->GetCreateObj();

        if (pObj)
        {
            SfxItemSet aAttr(pDoc->GetPool());
            SetStyleSheet(aAttr, pObj);

//-/            pObj->NbcSetAttributes(aAttr, FALSE);
            pObj->SetMergedItemSet(aAttr);
        }

        bReturn = TRUE;
    }
    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

BOOL FuConstructArc::MouseMove( const MouseEvent& rMEvt )
{
    return FuConstruct::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL FuConstructArc::MouseButtonUp( const MouseEvent& rMEvt )
{
    BOOL bReturn = FALSE;
    BOOL bCreated = FALSE;

    if ( pView->IsCreateObj() && rMEvt.IsLeft() )
    {
        Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

        ULONG nCount = pView->GetSdrPageView()->GetObjList()->GetObjCount();

        if (pView->EndCreateObj(SDRCREATE_NEXTPOINT) )
        {
            if (nCount != pView->GetSdrPageView()->GetObjList()->GetObjCount())
            {
                bCreated = TRUE;
            }
        }

        bReturn = TRUE;
    }

    bReturn = FuConstruct::MouseButtonUp (rMEvt) || bReturn;

    if (!bPermanent && bCreated)
        pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);

    return bReturn;
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/

BOOL FuConstructArc::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FuConstruct::KeyInput(rKEvt);
    return(bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuConstructArc::Activate()
{
    SdrObjKind aObjKind;

    switch( nSlotId )
    {
        case SID_DRAW_ARC      :
        case SID_DRAW_CIRCLEARC:
        {
            aObjKind = OBJ_CARC;
        }
        break;

        case SID_DRAW_PIE             :
        case SID_DRAW_PIE_NOFILL      :
        case SID_DRAW_CIRCLEPIE       :
        case SID_DRAW_CIRCLEPIE_NOFILL:
        {
            aObjKind = OBJ_SECT;
        }
        break;

        case SID_DRAW_ELLIPSECUT       :
        case SID_DRAW_ELLIPSECUT_NOFILL:
        case SID_DRAW_CIRCLECUT        :
        case SID_DRAW_CIRCLECUT_NOFILL :
        {
            aObjKind = OBJ_CCUT;
        }
        break;

        default:
        {
            aObjKind = OBJ_CARC;
        }
        break;
    }

    pView->SetCurrentObj(aObjKind);

    FuConstruct::Activate();
//  FuDraw::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuConstructArc::Deactivate()
{
    FuConstruct::Deactivate();
//  FuDraw::Deactivate();
}

// #97016#
SdrObject* FuConstructArc::CreateDefaultObject(const sal_uInt16 nID, const Rectangle& rRectangle)
{
    // case SID_DRAW_ARC:
    // case SID_DRAW_CIRCLEARC:
    // case SID_DRAW_PIE:
    // case SID_DRAW_PIE_NOFILL:
    // case SID_DRAW_CIRCLEPIE:
    // case SID_DRAW_CIRCLEPIE_NOFILL:
    // case SID_DRAW_ELLIPSECUT:
    // case SID_DRAW_ELLIPSECUT_NOFILL:
    // case SID_DRAW_CIRCLECUT:
    // case SID_DRAW_CIRCLECUT_NOFILL:

    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        pView->GetCurrentObjInventor(), pView->GetCurrentObjIdentifier(),
        0L, pDoc);

    if(pObj)
    {
        if(pObj->ISA(SdrCircObj))
        {
            Rectangle aRect(rRectangle);

            if(SID_DRAW_ARC == nID ||
                SID_DRAW_CIRCLEARC == nID ||
                SID_DRAW_CIRCLEPIE == nID ||
                SID_DRAW_CIRCLEPIE_NOFILL == nID ||
                SID_DRAW_CIRCLECUT == nID ||
                SID_DRAW_CIRCLECUT_NOFILL == nID)
            {
                // force quadratic
                ImpForceQuadratic(aRect);
            }

            pObj->SetLogicRect(aRect);

            SfxItemSet aAttr(pDoc->GetPool());
            aAttr.Put(SdrCircStartAngleItem(9000));
            aAttr.Put(SdrCircEndAngleItem(0));

            if(SID_DRAW_PIE_NOFILL == nID ||
                SID_DRAW_CIRCLEPIE_NOFILL == nID ||
                SID_DRAW_ELLIPSECUT_NOFILL == nID ||
                SID_DRAW_CIRCLECUT_NOFILL == nID)
            {
                aAttr.Put(XFillStyleItem(XFILL_NONE));
            }

            pObj->SetMergedItemSet(aAttr);
        }
        else
        {
            DBG_ERROR("Object is NO circle object");
        }
    }

    return pObj;
}

} // end of namespace sd
