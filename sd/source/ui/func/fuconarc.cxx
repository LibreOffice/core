/*************************************************************************
 *
 *  $RCSfile: fuconarc.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tbe $ $Date: 2000-10-23 10:27:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


//#include <basctl/idetemp.hxx>
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

#pragma hdrstop

#include <svx/svxids.hrc>
#include <math.h>

#include "app.hrc"

#include "viewshel.hxx"
#include "sdview.hxx"
#include "sdwindow.hxx"
#include "drawdoc.hxx"
#include "fuconarc.hxx"
#include "res_bmp.hrc"

TYPEINIT1( FuConstArc, FuConstruct );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConstArc::FuConstArc( SdViewShell*    pViewSh,
                        SdWindow*       pWin,
                        SdView*         pView,
                        SdDrawDocument* pDoc,
                        SfxRequest&     rReq )
    : FuConstruct( pViewSh, pWin, pView, pDoc, rReq )
{
    pViewShell->SwitchObjectBar(RID_DRAW_OBJ_TOOLBOX);

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
        SdrPageView *pPV = pView->GetPageViewPvNum (0);

        pView->InsertObject(pNewCircle, *pPV, SDRINSERT_SETDEFLAYER);
    }
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuConstArc::~FuConstArc()
{
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

BOOL FuConstArc::MouseButtonDown( const MouseEvent& rMEvt )
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
            pObj->NbcSetAttributes(aAttr, FALSE);
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

BOOL FuConstArc::MouseMove( const MouseEvent& rMEvt )
{
    return FuConstruct::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL FuConstArc::MouseButtonUp( const MouseEvent& rMEvt )
{
    BOOL bReturn = FALSE;
    BOOL bCreated = FALSE;

    if ( pView->IsCreateObj() && rMEvt.IsLeft() )
    {
        Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

        ULONG nCount = pView->GetPageViewPvNum(0)->GetObjList()->GetObjCount();

        if (pView->EndCreateObj(SDRCREATE_NEXTPOINT) )
        {
            if (nCount != pView->GetPageViewPvNum(0)->GetObjList()->GetObjCount())
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

BOOL FuConstArc::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FuConstruct::KeyInput(rKEvt);
    return(bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuConstArc::Activate()
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

void FuConstArc::Deactivate()
{
    FuConstruct::Deactivate();
//  FuDraw::Deactivate();
}


