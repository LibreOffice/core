/*************************************************************************
 *
 *  $RCSfile: fuconarc.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2002-07-18 09:50:10 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif

#include <svx/svdview.hxx>

#include "fuconarc.hxx"
#include "sc.hrc"
#include "tabvwsh.hxx"

// #98185# Create default drawing objects via keyboard
#ifndef _SVDOCIRC_HXX
#include <svx/svdocirc.hxx>
#endif

#ifndef _SXCIAITM_HXX
#include <svx/sxciaitm.hxx>
#endif


//------------------------------------------------------------------------


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConstArc::FuConstArc( ScTabViewShell* pViewSh, Window* pWin, SdrView* pView,
                   SdrModel* pDoc, SfxRequest& rReq )
    : FuConstruct( pViewSh, pWin, pView, pDoc, rReq )
{
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

BOOL __EXPORT FuConstArc::MouseButtonDown( const MouseEvent& rMEvt )
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    BOOL bReturn = FuConstruct::MouseButtonDown( rMEvt );

    if ( rMEvt.IsLeft() && !pView->IsAction() )
    {
        Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        pWindow->CaptureMouse();
        pView->BegCreateObj( aPnt );
        bReturn = TRUE;
    }
    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

BOOL __EXPORT FuConstArc::MouseMove( const MouseEvent& rMEvt )
{
    return FuConstruct::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL __EXPORT FuConstArc::MouseButtonUp( const MouseEvent& rMEvt )
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    BOOL bReturn = FALSE;

    if ( pView->IsCreateObj() && rMEvt.IsLeft() )
    {
        // Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        pView->EndCreateObj( SDRCREATE_NEXTPOINT );
        bReturn = TRUE;
    }
/*
    else if ( pView->IsCreateObj() && rMEvt.IsRight() )
    {
        // Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        pView->EndCreateObj( SDRCREATE_FORCEEND );
        bReturn = TRUE;
    }
*/
    return (FuConstruct::MouseButtonUp(rMEvt) || bReturn);
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/

BOOL __EXPORT FuConstArc::KeyInput(const KeyEvent& rKEvt)
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

    switch (aSfxRequest.GetSlot() )
    {
        case SID_DRAW_ARC:
            aNewPointer = Pointer( POINTER_DRAW_ARC );
            aObjKind = OBJ_CARC;
            break;

        case SID_DRAW_PIE:
            aNewPointer = Pointer( POINTER_DRAW_PIE );
            aObjKind = OBJ_SECT;
            break;

        case SID_DRAW_CIRCLECUT:
            aNewPointer = Pointer( POINTER_DRAW_CIRCLECUT );
            aObjKind = OBJ_CCUT;
            break;

        default:
            aNewPointer = Pointer( POINTER_CROSS );
            aObjKind = OBJ_CARC;
            break;
    }

    pView->SetCurrentObj( aObjKind );

    aOldPointer = pWindow->GetPointer();
    pViewShell->SetActivePointer( aNewPointer );

    FuDraw::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuConstArc::Deactivate()
{
    FuDraw::Deactivate();
    pViewShell->SetActivePointer( aOldPointer );
}

// #98185# Create default drawing objects via keyboard
SdrObject* FuConstArc::CreateDefaultObject(const sal_uInt16 nID, const Rectangle& rRectangle)
{
    // case SID_DRAW_ARC:
    // case SID_DRAW_PIE:
    // case SID_DRAW_CIRCLECUT:

    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        pView->GetCurrentObjInventor(), pView->GetCurrentObjIdentifier(),
        0L, pDrDoc);

    if(pObj)
    {
        if(pObj->ISA(SdrCircObj))
        {
            Rectangle aRect(rRectangle);

            if(SID_DRAW_ARC == nID || SID_DRAW_CIRCLECUT == nID)
            {
                // force quadratic
                ImpForceQuadratic(aRect);
            }

            pObj->SetLogicRect(aRect);

            SfxItemSet aAttr(pDrDoc->GetItemPool());
            aAttr.Put(SdrCircStartAngleItem(9000));
            aAttr.Put(SdrCircEndAngleItem(0));

            pObj->SetItemSet(aAttr);
        }
        else
        {
            DBG_ERROR("Object is NO circle object");
        }
    }

    return pObj;
}


