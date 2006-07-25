/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: conpoly.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-25 12:41:45 $
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


#pragma hdrstop

#ifndef _SVDMARK_HXX //autogen
#include <svx/svdmark.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _SVDOPATH_HXX //autogen
#include <svx/svdopath.hxx>
#endif

#include "view.hxx"
#include "edtwin.hxx"
#include "wrtsh.hxx"
#include "drawbase.hxx"
#include "conpoly.hxx"


/************************************************************************/

#define CLOSE_PIXDIST   5   // Pixelabstand, ab dem geschlossen wird

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/



ConstPolygon::ConstPolygon(SwWrtShell* pWrtShell, SwEditWin* pEditWin, SwView* pSwView) :
                SwDrawBase(pWrtShell, pEditWin, pSwView)
{
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/



BOOL ConstPolygon::MouseButtonDown(const MouseEvent& rMEvt)
{
    BOOL bReturn;

    if ((bReturn = SwDrawBase::MouseButtonDown(rMEvt)) == TRUE)
        aLastPos = rMEvt.GetPosPixel();

    return (bReturn);
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/



BOOL ConstPolygon::MouseMove(const MouseEvent& rMEvt)
{
    BOOL bReturn = FALSE;

    bReturn = SwDrawBase::MouseMove(rMEvt);

    return bReturn;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/



BOOL ConstPolygon::MouseButtonUp(const MouseEvent& rMEvt)
{
    BOOL bReturn = FALSE;

    if (pSh->IsDrawCreate())
    {
        if (rMEvt.IsLeft() && rMEvt.GetClicks() == 1 &&
                                        pWin->GetDrawMode() != OBJ_FREELINE)
        {
            if (!pSh->EndCreate(SDRCREATE_NEXTPOINT))
            {
                pSh->BreakCreate();
                EnterSelectMode(rMEvt);
                return TRUE;
            }
        }
        else
        {
            Point aPnt(pWin->PixelToLogic(rMEvt.GetPosPixel()));
            bReturn = SwDrawBase::MouseButtonUp(rMEvt);

            if (!(bReturn && (aPnt == aStartPos || rMEvt.IsRight())))
            {
                SdrView *pSdrView = pSh->GetDrawView();

                long nCloseDist = pWin->PixelToLogic(Size(CLOSE_PIXDIST, 0)).Width();
                const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
                if (rMarkList.GetMark(0))
                {
                    SdrObject* pMarkedObject = rMarkList.GetMark(0)->GetMarkedSdrObj();
                    // #127440# - crash report shows that the marked object is not always an SdrPathObj
                    SdrPathObj* pPathObj = dynamic_cast< SdrPathObj*>(pMarkedObject);
                    DBG_ASSERT(pPathObj, "issue #127440# SdrPathObj expected")
                    if( pPathObj )
                    {
                        const XPolyPolygon& rXPP = pPathObj->GetPathPoly();
                        if (rXPP.Count() == 1)
                        {
                            USHORT nPntMax = rXPP[0].GetPointCount() - 1;
                            Point aDiff = rXPP[0][nPntMax] - rXPP[0][0];
                            long nSqDist = aDiff.X() * aDiff.X() + aDiff.Y() * aDiff.Y();
                            nCloseDist *= nCloseDist;

                            if (nSqDist <= nCloseDist && !pPathObj->IsClosed())
                                pPathObj->ToggleClosed(0);
                        }
                    }
                }
            }
        }
    }
    else
        bReturn = SwDrawBase::MouseButtonUp(rMEvt);

    return (bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/



void ConstPolygon::Activate(const USHORT nSlotId)
{
    switch (nSlotId)
    {
        case SID_DRAW_POLYGON_NOFILL:
            pWin->SetDrawMode(OBJ_PLIN);
            break;

        case SID_DRAW_BEZIER_NOFILL:
            pWin->SetDrawMode(OBJ_PATHLINE);
            break;

        case SID_DRAW_FREELINE_NOFILL:
            pWin->SetDrawMode(OBJ_FREELINE);
            break;

        default:
            break;
    }

    SwDrawBase::Activate(nSlotId);
}



