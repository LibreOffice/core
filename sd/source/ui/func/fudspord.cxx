/*************************************************************************
 *
 *  $RCSfile: fudspord.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:35 $
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


#pragma hdrstop

#include <svx/svxids.hrc>
#ifndef _VCL_POINTR_HXX //autogen
#include <vcl/pointr.hxx>
#endif

#include "app.hrc"
#include "fudspord.hxx"
#include "fupoor.hxx"
#include "viewshel.hxx"
#include "sdview.hxx"
#include "sdwindow.hxx"
#include "drawdoc.hxx"

TYPEINIT1( FuDisplayOrder, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuDisplayOrder::FuDisplayOrder(SdViewShell*     pViewSh,
                               SdWindow*        pWin,
                               SdView*          pView,
                               SdDrawDocument*  pDoc,
                               SfxRequest&      rReq) :
    FuPoor(pViewSh, pWin, pView, pDoc, rReq),
    pUserMarker(NULL),
    pRefObj(NULL)
{
    pUserMarker = new SdrViewUserMarker(pView);
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuDisplayOrder::~FuDisplayOrder()
{
    delete pUserMarker;
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

BOOL FuDisplayOrder::MouseButtonDown(const MouseEvent& rMEvt)
{
    return TRUE;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

BOOL FuDisplayOrder::MouseMove(const MouseEvent& rMEvt)
{
    SdrObject* pPickObj;
    SdrPageView* pPV;
    Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
    USHORT nHitLog = USHORT ( pWindow->PixelToLogic(Size(HITPIX,0)).Width() );

    if ( pView->PickObj(aPnt, pPickObj, pPV) )
    {
        if (pRefObj != pPickObj)
        {
            pRefObj = pPickObj;
            pUserMarker->SetXPolyPolygon(pRefObj, pView->GetPageViewPvNum(0));
            pUserMarker->Show();
        }
    }
    else
    {
        pRefObj = NULL;
        pUserMarker->Hide();
    }

    return TRUE;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL FuDisplayOrder::MouseButtonUp(const MouseEvent& rMEvt)
{
    SdrPageView* pPV = NULL;
    Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
    USHORT nHitLog = USHORT ( pWindow->PixelToLogic(Size(HITPIX,0)).Width() );

    if ( pView->PickObj(aPnt, pRefObj, pPV) )
    {
        if (nSlotId == SID_BEFORE_OBJ)
        {
            pView->PutMarkedInFrontOfObj(pRefObj);
        }
        else
        {
            pView->PutMarkedBehindObj(pRefObj);
        }
    }

    pViewShell->Cancel();

    return TRUE;
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuDisplayOrder::Activate()
{
    aPtr = pWindow->GetPointer();
    pWindow->SetPointer( Pointer( POINTER_REFHAND ) );

    if (pUserMarker)
    {
        pUserMarker->Show();
    }
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuDisplayOrder::Deactivate()
{
    if (pUserMarker)
    {
        pUserMarker->Hide();
    }

    pWindow->SetPointer( aPtr );
}


