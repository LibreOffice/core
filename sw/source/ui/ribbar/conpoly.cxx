/*************************************************************************
 *
 *  $RCSfile: conpoly.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:46 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

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
                const SdrMarkList& rMarkList = pSdrView->GetMarkList();
                if (rMarkList.GetMark(0))
                {
                    SdrPathObj* pPathObj = (SdrPathObj *)rMarkList.GetMark(0)->GetObj();
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

/*************************************************************************

      Source Code Control System - History

      $Log: not supported by cvs2svn $
      Revision 1.29  2000/09/18 16:06:01  willem.vandorp
      OpenOffice header added.

      Revision 1.28  1997/11/28 19:14:18  MA
      includes


      Rev 1.27   28 Nov 1997 20:14:18   MA
   includes

      Rev 1.26   24 Nov 1997 14:53:58   MA
   includes

      Rev 1.25   03 Nov 1997 13:24:20   MA
   precomp entfernt

      Rev 1.24   05 Sep 1997 11:57:52   MH
   chg: header

      Rev 1.23   13 Aug 1997 12:18:56   OM
   #42383# Selektion von Punkten im Konstruktionsmode

      Rev 1.22   20 Mar 1997 12:14:28   OM
   GPF in Beziermode behoben

      Rev 1.21   30 Jan 1997 11:48:08   OM
   Freihandzeichnen

      Rev 1.20   29 Jan 1997 11:22:08   OM
   Freihandzeichnen

      Rev 1.19   23 Jan 1997 16:07:06   OM
   Neuer Polygon Zeichenmode

      Rev 1.18   16 Jan 1997 12:35:58   OM
   Neuer Objekttyp: Bezier

      Rev 1.17   28 Aug 1996 13:39:32   OS
   includes

      Rev 1.16   14 Aug 1996 11:31:00   JP
   svdraw.hxx entfernt

      Rev 1.15   24 Nov 1995 16:58:06   OM
   PCH->PRECOMPILED

      Rev 1.14   15 Nov 1995 18:35:28   OM
   Virtuelle Fkt rausgeworfen

      Rev 1.13   10 Aug 1995 18:15:00   OM
   CreateMode geaendert

      Rev 1.12   09 Aug 1995 16:53:02   MA
   drawing-undo-header rein

      Rev 1.11   20 Jul 1995 19:26:38   OM
   Umbenamsung obj_pathline->obj_plin

      Rev 1.10   06 May 1995 20:38:04   OM
   Polygonerzeugung bei Einfachklick

      Rev 1.9   21 Apr 1995 02:18:44   ER
   ToggleClosed erwartet jetzt ein long und kein Size() mehr

      Rev 1.8   06 Apr 1995 20:15:56   OM
   Polygon beenden mit rechter Maustaste

      Rev 1.7   21 Mar 1995 00:47:02   ER
   _svdorect_hxx definiert => _svdcapt_hxx definieren

*************************************************************************/


