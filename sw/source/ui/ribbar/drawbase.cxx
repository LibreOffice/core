/*************************************************************************
 *
 *  $RCSfile: drawbase.cxx,v $
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
#include <hintids.hxx>
#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _SFXPTITEM_HXX //autogen
#include <svtools/ptitem.hxx>
#endif
#ifndef _SVX_SIZEITEM_HXX //autogen
#include <svx/sizeitem.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _FMTCLDS_HXX
#include <fmtclds.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif

#include "cmdid.h"
#include "basesh.hxx"
#include "view.hxx"
#include "wrtsh.hxx"
#include "drawbase.hxx"
#include "edtwin.hxx"
#include "caption.hxx"
#include "swundo.hxx"

extern BOOL bNoInterrupt;       // in mainwn.cxx

#define MINMOVE ((USHORT)pSh->GetOut()->PixelToLogic(Size(pSh->GetDrawView()->GetMarkHdlSizePixel()/2,0)).Width())


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/


SwDrawBase::SwDrawBase(SwWrtShell* pSwWrtShell, SwEditWin* pWindow, SwView* pSwView) :
    pSh(pSwWrtShell),
    pWin(pWindow),
    pView(pSwView),
    bInsForm(FALSE),
    bCreateObj(TRUE)
{
    if ( !pSh->HasDrawView() )
        pSh->MakeDrawView();
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

__EXPORT SwDrawBase::~SwDrawBase()
{
    if (pView->GetWrtShellPtr())    // Im view-Dtor koennte die wrtsh bereits geloescht worden sein...
        pSh->GetDrawView()->SetEditMode(TRUE);
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/


BOOL SwDrawBase::MouseButtonDown(const MouseEvent& rMEvt)
{
    BOOL bReturn = FALSE;

    SdrView *pSdrView = pSh->GetDrawView();

    pSdrView->SetOrtho(rMEvt.IsShift());
    pSdrView->SetAngleSnapEnabled(rMEvt.IsShift());

    if (rMEvt.IsMod2())
    {
        pSdrView->SetCreate1stPointAsCenter(TRUE);
        pSdrView->SetResizeAtCenter(TRUE);
    }
    else
    {
        pSdrView->SetCreate1stPointAsCenter(FALSE);
        pSdrView->SetResizeAtCenter(FALSE);
    }

    SdrViewEvent aVEvt;
    SdrHitKind eHit = pSdrView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

    // Nur neues Objekt, wenn nicht im Basismode (bzw reinem Selektionsmode)
    if (rMEvt.IsLeft() && !pWin->IsDrawAction())
    {
        if (IsCreateObj() && (eHit == SDRHIT_UNMARKEDOBJECT || eHit == SDRHIT_NONE || pSh->IsDrawCreate()))
        {
            bNoInterrupt = TRUE;
            pWin->CaptureMouse();

            aStartPos = pWin->PixelToLogic(rMEvt.GetPosPixel());

            bReturn = pSh->BeginCreate(pWin->GetDrawMode(), aStartPos);

            SetDrawPointer();

            if ( bReturn )
                pWin->SetDrawAction(TRUE);
        }
        else if (!pSdrView->IsAction())
        {
            /**********************************************************************
            * BEZIER-EDITOR
            **********************************************************************/
            pWin->CaptureMouse();
            aStartPos = pWin->PixelToLogic(rMEvt.GetPosPixel());
            UINT16 nEditMode = pWin->GetBezierMode();

            if (eHit == SDRHIT_HANDLE && aVEvt.pHdl->GetKind() == HDL_BWGT)
            {
                /******************************************************************
                * Handle draggen
                ******************************************************************/
                bNoInterrupt = TRUE;
                bReturn = pSdrView->BegDragObj(aStartPos, (OutputDevice*) NULL, aVEvt.pHdl);
                pWin->SetDrawAction(TRUE);
            }
            else if (eHit == SDRHIT_MARKEDOBJECT && nEditMode == SID_BEZIER_INSERT)
            {
                /******************************************************************
                * Klebepunkt einfuegen
                ******************************************************************/
                bNoInterrupt = TRUE;
                bReturn = pSdrView->BegInsObjPoint(aStartPos, rMEvt.IsMod1(), NULL, 0);
                pWin->SetDrawAction(TRUE);
            }
            else if (eHit == SDRHIT_MARKEDOBJECT && rMEvt.IsMod1())
            {
                /******************************************************************
                * Klebepunkt selektieren
                ******************************************************************/
                if (!rMEvt.IsShift())
                    pSdrView->UnmarkAllPoints();

                bReturn = pSdrView->BegMarkPoints(aStartPos, (OutputDevice*) NULL);
                pWin->SetDrawAction(TRUE);
            }
            else if (eHit == SDRHIT_MARKEDOBJECT && !rMEvt.IsShift() && !rMEvt.IsMod2())
            {
                /******************************************************************
                * Objekt verschieben
                ******************************************************************/
                return FALSE;
            }
            else if (eHit == SDRHIT_HANDLE)
            {
                /******************************************************************
                * Klebepunkt selektieren
                ******************************************************************/
                if (pSdrView->HasMarkablePoints() && (!pSdrView->IsPointMarked(*aVEvt.pHdl) || rMEvt.IsShift()))
                {
                    SdrHdl* pHdl = NULL;

                    if (!rMEvt.IsShift())
                    {
                        pSdrView->UnmarkAllPoints();
                        pHdl = pSdrView->HitHandle(aStartPos, *pWin);
                    }
                    else
                    {
                        if (pSdrView->IsPointMarked(*aVEvt.pHdl))
                        {
                            bReturn = pSdrView->UnmarkPoint(*aVEvt.pHdl);
                            pHdl = NULL;
                        }
                        else
                        {
                            pHdl = pSdrView->HitHandle(aStartPos, *pWin);
                        }
                    }

                    if (pHdl)
                    {
                        bNoInterrupt = TRUE;
                        pSdrView->MarkPoint(*pHdl);
//                      bReturn = pSdrView->BegDragObj(aStartPos, (OutputDevice*) NULL, pHdl);
//                      pWin->SetDrawAction(TRUE);
                    }
                }
            }
            else
            {
                /******************************************************************
                * Objekt selektieren oder draggen
                ******************************************************************/
                if (pSh->IsObjSelectable(aStartPos) && eHit == SDRHIT_UNMARKEDOBJECT)
                {
                    if (pSdrView->HasMarkablePoints())
                        pSdrView->UnmarkAllPoints();

                    bNoInterrupt = FALSE;
                    // Drag im edtwin verwenden
                    return FALSE;
                }

                bNoInterrupt = TRUE;

                if (pSh->IsObjSelected())
                {
                    if (!rMEvt.IsShift())
                    {
                        if (!pSdrView->HasMarkablePoints())
                            pSh->SelectObj(Point(LONG_MAX, LONG_MAX));  // Alles deselektieren
                        else
                            pSdrView->UnmarkAllPoints();
                    }
                }
                BOOL bMarked = FALSE;

                if (!pSh->IsSelFrmMode())
                    pSh->EnterSelFrmMode(NULL);

                if ((bReturn = pSh->BeginMark(aStartPos)) == TRUE)
                        pWin->SetDrawAction(TRUE);

                SetDrawPointer();
            }
        }
    }
    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/


BOOL SwDrawBase::MouseMove(const MouseEvent& rMEvt)
{
    SdrView *pSdrView = pSh->GetDrawView();
    Point aPnt(pWin->PixelToLogic(rMEvt.GetPosPixel()));
    BOOL bRet = FALSE;

    if (IsCreateObj() && !pWin->IsDrawSelMode() && pSdrView->IsCreateObj())
    {
        pSdrView->SetOrtho(rMEvt.IsShift());
        pSdrView->SetAngleSnapEnabled(rMEvt.IsShift());

        pSh->MoveCreate(aPnt);
        bRet = TRUE;
    }
    else if (pSdrView->IsAction() || pSdrView->IsInsObjPoint() || pSdrView->IsMarkPoints())
    {
        pSh->MoveMark(aPnt);
        bRet = TRUE;
    }

    return (bRet);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/


BOOL SwDrawBase::MouseButtonUp(const MouseEvent& rMEvt)
{
    BOOL bReturn = FALSE;
    BOOL bCheckShell = FALSE;
    BOOL bAutoCap = FALSE;

    Point aPnt(pWin->PixelToLogic(rMEvt.GetPosPixel()));

    if (IsCreateObj() && pSh->IsDrawCreate() && !pWin->IsDrawSelMode())
    {
        if (aPnt == aStartPos || rMEvt.IsRight())
        {
            pSh->BreakCreate();
            pView->LeaveDrawCreate();
        }
        else
        {
            if (pWin->GetDrawMode() == OBJ_NONE)
                pSh->StartUndo(UNDO_INSERT);

            pSh->EndCreate(SDRCREATE_FORCEEND);
            if (pWin->GetDrawMode() == OBJ_NONE)    // Textrahmen eingefuegt
            {
                bAutoCap = TRUE;
                if(pWin->GetFrmColCount() > 1)
                {
                    SfxItemSet aSet(pView->GetPool(),RES_COL,RES_COL);
                    SwFmtCol aCol((const SwFmtCol&)aSet.Get(RES_COL));
                    aCol.Init(pWin->GetFrmColCount(), aCol.GetGutterWidth(), aCol.GetWishWidth());
                    aSet.Put(aCol);
                    // Vorlagen-AutoUpdate
                    SwFrmFmt* pFmt = pSh->GetCurFrmFmt();
                    if(pFmt && pFmt->IsAutoUpdateFmt())
                        pSh->AutoUpdateFrame(pFmt, aSet);
                    else
                        pSh->SetFlyFrmAttr( aSet );
                }
            }
            if (pWin->GetDrawMode() == OBJ_NONE)
                pSh->EndUndo(UNDO_INSERT);
        }

        bReturn = TRUE;

        EnterSelectMode(rMEvt);
    }
    else
    {
        SdrView *pSdrView = pSh->GetDrawView();

        if (!pSdrView->HasMarkablePoints())
        {
            /**********************************************************************
            * KEIN BEZIER_EDITOR
            **********************************************************************/
            if ((pSh->GetDrawView()->IsMarkObj() || pSh->GetDrawView()->IsMarkPoints())
                 && rMEvt.IsLeft())
            {
                bReturn = pSh->EndMark();

                pWin->SetDrawAction(FALSE);

                if (aPnt == aStartPos && pSh->IsObjSelectable(aPnt))
                {
                    pSh->SelectObj(aPnt, rMEvt.IsShift() && pSh->IsSelFrmMode());

                    if (!pSh->IsObjSelected())
                    {
                        pView->LeaveDrawCreate();   // In Selektionsmode wechseln

                        pSh->GetView().GetViewFrame()->GetBindings().Invalidate(SID_INSERT_DRAW);

                        if (pSh->IsSelFrmMode())
                            pSh->LeaveSelFrmMode();
                    }
                    pView->NoRotate();

                    bCheckShell = TRUE; // ggf BezierShell anwerfen
                }
                else if (!pSh->IsObjSelected() && !pWin->IsDrawAction())
                {
                    if (pSh->IsObjSelectable(aPnt))
                        pSh->SelectObj(aPnt, rMEvt.IsShift() && pSh->IsSelFrmMode());
                    else
                    {
                        pView->LeaveDrawCreate();
                        if (pSh->IsSelFrmMode())
                            pSh->LeaveSelFrmMode();
                    }
                    pView->NoRotate();

                    bReturn = TRUE;
                }
            }
        }
        else
        {
            /**********************************************************************
            * BEZIER_EDITOR
            **********************************************************************/
            if ( pSdrView->IsAction() )
            {
                if ( pSdrView->IsInsObjPoint() )
                    bReturn = pSdrView->EndInsObjPoint(SDRCREATE_FORCEEND);
                else if (pSdrView->IsMarkPoints() )
                    bReturn = pSdrView->EndMarkPoints();
                else
                {
                    pSdrView->EndAction();
                    bReturn = TRUE;
                }
                pWin->SetDrawAction(FALSE);

                if (aPnt == aStartPos)
                {
                    if (!pSh->IsObjSelectable(aPnt))
                        pSh->SelectObj(Point(LONG_MAX, LONG_MAX));
                    else if (!bReturn)
                    {
                        if (!rMEvt.IsShift())
                            pSdrView->UnmarkAllPoints();
                        pSh->SelectObj(aPnt, rMEvt.IsShift() && pSh->IsSelFrmMode());
                    }

                    if (!pSh->IsObjSelected())
                    {
                        pView->LeaveDrawCreate();   // In Selektionsmode wechseln

                        pSh->GetView().GetViewFrame()->GetBindings().Invalidate(SID_INSERT_DRAW);

                        if (pSh->IsSelFrmMode())
                            pSh->LeaveSelFrmMode();
                    }
                    pView->NoRotate();

                    bCheckShell = TRUE; // ggf BezierShell anwerfen
                }
            }

            SetDrawPointer();

            if (!pSh->IsObjSelected() && !pWin->IsDrawAction())
            {
                pView->LeaveDrawCreate();
                if (pSh->IsSelFrmMode())
                    pSh->LeaveSelFrmMode();

                pView->NoRotate();
                bReturn = TRUE;
            }
        }
    }

    if (bCheckShell)
        pView->AttrChangedNotify( pSh );    // ggf BezierShell anwerfen

    //!!!!!!!!!! Achtung Suizid !!!!!!!!!!! Sollte alles mal erneuert werden
    if ( bAutoCap )
        pView->AutoCaption(FRAME_CAP);  //Kann derzeit nur FRAME sein, sonst auf
                                        //enums umstellen
    return (bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/


void SwDrawBase::Activate(const USHORT nSlotId)
{
    SdrView *pSdrView = pSh->GetDrawView();

    pSdrView->SetCurrentObj(pWin->GetDrawMode());
    pSdrView->SetEditMode(FALSE);

    SetDrawPointer();
    pSh->NoEdit();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/


void __EXPORT SwDrawBase::Deactivate()
{
    SdrView *pSdrView = pSh->GetDrawView();
    pSdrView->SetOrtho(FALSE);
    pSdrView->SetAngleSnapEnabled(FALSE);

    if (pWin->IsDrawAction() && pSh->IsDrawCreate())
        pSh->BreakCreate();

    pWin->SetDrawAction(FALSE);

    pWin->ReleaseMouse();
    bNoInterrupt = FALSE;

//  if(!pSh->IsObjSelected())
//      pSh->Edit();

    if(pWin->GetApplyTemplate())
        pWin->SetApplyTemplate(SwApplyTemplate());
    pSh->GetView().GetViewFrame()->GetBindings().Invalidate(SID_INSERT_DRAW);
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/


BOOL SwDrawBase::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FALSE;
    USHORT nCode = rKEvt.GetKeyCode().GetCode();

    switch (nCode)
    {
        case KEY_ESCAPE:
        {
            if (pWin->IsDrawAction())
            {
                BreakCreate();
                pView->LeaveDrawCreate();
            }

            bReturn = TRUE;
        }
        break;

        case KEY_DELETE:
        {
            pSh->DelSelectedObj();
            bReturn = TRUE;
        }
        break;

        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
        {
            SdrView *pSdrView = pSh->GetDrawView();

            if (!pSdrView->IsTextEdit())
            {
                long nX = 0;
                long nY = 0;

                if (nCode == KEY_UP)
                {
                    // Scroll nach oben
                    nX = 0;
                    nY =-1;
                }
                else if (nCode == KEY_DOWN)
                {
                    // Scroll nach unten
                    nX = 0;
                    nY = 1;
                }
                else if (nCode == KEY_LEFT)
                {
                    // Scroll nach links
                    nX =-1;
                    nY = 0;
                }
                else if (nCode == KEY_RIGHT)
                {
                    // Scroll nach rechts
                    nX = 1;
                    nY = 0;
                }

                if (pSdrView->HasMarkedObj() && rKEvt.GetKeyCode().IsMod2())
                {
                    // Objekte verschieben
                    nX *= 100;
                    nY *= 100;
                    pSdrView->MoveAllMarked(Size(nX, nY));
                }

                bReturn = TRUE;
            }
        }
        break;
    }

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


void SwDrawBase::BreakCreate()
{
    pSh->BreakCreate();
    pWin->SetDrawAction(FALSE);
    pWin->ReleaseMouse();

    Deactivate();
//  pView->LeaveDrawCreate();
}

/*************************************************************************
|*
|* Mauspointer umschalten
|*
\************************************************************************/


void SwDrawBase::SetDrawPointer()
{
    SdrView *pSdrView = pSh->GetDrawView();
#ifdef VCL
        Point aPnt(pWin->OutputToScreenPixel(pWin->GetPointerPosPixel()));
#else
        Point aPnt = Pointer::GetPosPixel();
#endif
    aPnt = pWin->PixelToLogic(pWin->ScreenToOutputPixel(aPnt));
    const Pointer aPointTyp = pSdrView->GetPreferedPointer(aPnt, pSh->GetOut());
    const Pointer aDrawPt(aPointTyp);
    pWin->SetPointer(aDrawPt);
}

/*************************************************************************
|*
|* Ggf in Selektionsmode wechseln
|*
\************************************************************************/

void SwDrawBase::EnterSelectMode(const MouseEvent& rMEvt)
{
    pWin->SetDrawAction(FALSE);

    if (!pSh->IsObjSelected() && !pWin->IsDrawAction())
    {
        Point aPnt(pWin->PixelToLogic(rMEvt.GetPosPixel()));

        if (pSh->IsObjSelectable(aPnt))
        {
            pSh->SelectObj(aPnt);
            if (rMEvt.GetModifier() == KEY_SHIFT || !pSh->IsObjSelected())
            {
                pView->LeaveDrawCreate();   // In Selektionsmode wechseln

                pSh->GetView().GetViewFrame()->GetBindings().Invalidate(SID_INSERT_DRAW);
            }
        }
        else
        {
            pView->LeaveDrawCreate();
            if (pSh->IsSelFrmMode())
                pSh->LeaveSelFrmMode();
        }
        pView->NoRotate();
    }
}

/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.80  2000/09/18 16:06:01  willem.vandorp
      OpenOffice header added.

      Revision 1.79  2000/09/07 15:59:27  os
      change: SFX_DISPATCHER/SFX_BINDINGS removed

      Revision 1.78  2000/05/10 11:54:15  os
      Basic API removed

      Revision 1.77  1999/05/27 11:04:40  OS
      Einf?gen/Bearbeiten von Spalten ueberarbeitet


      Rev 1.76   27 May 1999 13:04:40   OS
   Einf?gen/Bearbeiten von Spalten ueberarbeitet

      Rev 1.75   11 Mar 1999 11:31:22   MA
   #63192# mit Suizid fertig werden

      Rev 1.74   16 Nov 1998 17:18:52   OM
   #58158# Beschriftung fuer weiter OLE-Objekte

      Rev 1.73   06 Nov 1998 17:05:04   OM
   #58158# Automatische Beschriftung

      Rev 1.72   09 Sep 1998 12:14:46   OM
   #56223# Kein GPF bei Klick in Seitenrand

      Rev 1.71   22 Jul 1998 16:06:00   TJ
   add: header wg. int comp. error

      Rev 1.70   09 Jun 1998 15:34:00   OM
   VC-Controls entfernt

      Rev 1.69   11 Mar 1998 17:12:08   OM
   DB-FormShell

      Rev 1.68   28 Nov 1997 20:16:46   MA
   includes

      Rev 1.67   24 Nov 1997 14:54:00   MA
   includes

      Rev 1.66   18 Sep 1997 13:59:46   OS
   Pointer::GetPosPixel fuer VCL ersetzt

      Rev 1.65   12 Sep 1997 10:36:42   OS
   ITEMID_* definiert

      Rev 1.64   04 Sep 1997 17:14:02   MA
   includes

      Rev 1.63   01 Sep 1997 13:14:38   OS
   DLL-Umstellung

      Rev 1.62   13 Aug 1997 12:19:54   OM
   #42383# Selektion von Punkten im Konstruktionsmode

      Rev 1.61   13 Aug 1997 12:18:56   OM
   #42383# Selektion von Punkten im Konstruktionsmode

      Rev 1.60   09 Aug 1997 13:33:42   OS
   paraitem/frmitems/textitem aufgeteilt

      Rev 1.59   30 Jul 1997 18:40:48   HJS
   includes

      Rev 1.58   07 Apr 1997 13:40:24   MH
   chg: header

      Rev 1.57   20 Mar 1997 11:33:46   OM
   Insert und CreateMode merken

      Rev 1.56   24 Feb 1997 16:51:46   OS
   ::Deactivate: Template-Pointer gfs. abschalten

      Rev 1.55   04 Feb 1997 15:20:32   OM
   Bugs in Selektionsmodi behoben

      Rev 1.54   31 Jan 1997 10:58:26   OM
   Statusupdate

      Rev 1.53   30 Jan 1997 11:48:10   OM
   Freihandzeichnen

      Rev 1.52   29 Jan 1997 11:22:08   OM
   Freihandzeichnen

      Rev 1.51   23 Jan 1997 16:07:06   OM
   Neuer Polygon Zeichenmode

      Rev 1.50   22 Jan 1997 11:19:48   OM
   Neue Shells: DrawBaseShell und DrawControlShell

      Rev 1.49   16 Jan 1997 12:35:58   OM
   Neuer Objekttyp: Bezier

      Rev 1.48   13 Jan 1997 15:50:18   OM
   Bezier-Punkte loeschen

      Rev 1.47   08 Jan 1997 12:44:26   OM
   Aenderungen fuer Bezier-Shell

      Rev 1.46   27 Nov 1996 17:26:22   OM
   Neu: SetAngleSnap

      Rev 1.45   28 Aug 1996 13:39:30   OS
   includes

      Rev 1.44   14 Aug 1996 15:05:28   OM
   Zentrisch konstruieren

      Rev 1.43   14 Aug 1996 11:31:00   JP
   svdraw.hxx entfernt

      Rev 1.42   09 Aug 1996 14:33:20   OM
   #29784# GPF im Dtor behoben

      Rev 1.41   07 Mar 1996 14:27:30   HJS
   2 defines zu viel

      Rev 1.40   24 Jan 1996 15:19:58   OM
   #24663# Haenger bei Polygon im Rastermode behoben

      Rev 1.39   14 Jan 1996 17:59:08   OS
   recording Rahmen einfuegen: Position von GetAnchorObjDiff() holen

      Rev 1.38   17 Dec 1995 17:28:18   OS
   Rahmen einfuegen wird jetzt an der Textshell recorded

      Rev 1.37   30 Nov 1995 13:59:52   MA
   opt: SV304, keine Pruefung vor SetPointer

      Rev 1.36   24 Nov 1995 16:58:04   OM
   PCH->PRECOMPILED

      Rev 1.35   23 Nov 1995 14:29:00   MA
   chg: GetObjAnchorDiff(), GetObjAbsPos()

*************************************************************************/


