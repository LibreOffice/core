/*************************************************************************
 *
 *  $RCSfile: fuconbez.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 15:10:29 $
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

#include "fuconbez.hxx"

#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SVDOPATH_HXX //autogen
#include <svx/svdopath.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif

#pragma hdrstop

#include <svx/svxids.hrc>

#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif

#include "app.hrc"
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#ifndef SD_OBJECT_BAR_MANAGER_HXX
#include "ObjectBarManager.hxx"
#endif
#include "drawdoc.hxx"
#include "res_bmp.hrc"

namespace sd {

TYPEINIT1( FuConstructBezierPolygon, FuConstruct );


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConstructBezierPolygon::FuConstructBezierPolygon (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuConstruct(pViewSh, pWin, pView, pDoc, rReq),
      nEditMode(SID_BEZIER_MOVE)
{
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuConstructBezierPolygon::~FuConstructBezierPolygon()
{
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

BOOL FuConstructBezierPolygon::MouseButtonDown(const MouseEvent& rMEvt)
{
    BOOL bReturn = FuConstruct::MouseButtonDown(rMEvt);

    SdrViewEvent aVEvt;
    SdrHitKind eHit = pView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

    if (eHit == SDRHIT_HANDLE || rMEvt.IsMod1())
    {
        pView->SetEditMode(SDREDITMODE_EDIT);
    }
    else
    {
        pView->SetEditMode(SDREDITMODE_CREATE);
    }

    if (aVEvt.eEvent == SDREVENT_BEGTEXTEDIT)
    {
        // Texteingabe hier nicht zulassen
        aVEvt.eEvent = SDREVENT_BEGDRAGOBJ;
        pView->EnableExtendedMouseEventDispatcher(FALSE);
    }
    else
    {
        pView->EnableExtendedMouseEventDispatcher(TRUE);
    }

    if (eHit == SDRHIT_MARKEDOBJECT && nEditMode == SID_BEZIER_INSERT)
    {
        /******************************************************************
        * Klebepunkt einfuegen
        ******************************************************************/
        pView->BegInsObjPoint(aMDPos, rMEvt.IsMod1(), NULL, 0);
    }
    else
    {
        pView->MouseButtonDown(rMEvt, pWindow);

        SdrObject* pObj = pView->GetCreateObj();

        if (pObj)
        {
            SfxItemSet aAttr(pDoc->GetPool());
            SetStyleSheet(aAttr, pObj);
            pObj->SetMergedItemSet(aAttr);
        }
    }

    return(bReturn);
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

BOOL FuConstructBezierPolygon::MouseMove(const MouseEvent& rMEvt)
{
    BOOL bReturn = FuConstruct::MouseMove(rMEvt);
    return(bReturn);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL FuConstructBezierPolygon::MouseButtonUp(const MouseEvent& rMEvt)
{
    BOOL bReturn = FALSE;
    BOOL bCreated = FALSE;

    SdrViewEvent aVEvt;
    SdrHitKind eHit = pView->PickAnything(rMEvt, SDRMOUSEBUTTONUP, aVEvt);

    ULONG nCount = pView->GetPageViewPvNum(0)->GetObjList()->GetObjCount();

    if (pView->IsInsObjPoint())
    {
        pView->EndInsObjPoint(SDRCREATE_FORCEEND);
    }
    else
    {
        pView->MouseButtonUp(rMEvt, pWindow);
    }

    if (aVEvt.eEvent == SDREVENT_ENDCREATE)
    {
        bReturn = TRUE;

        if (nCount != pView->GetPageViewPvNum(0)->GetObjList()->GetObjCount())
        {
            bCreated = TRUE;
        }

        // Trick, um FuDraw::DoubleClick nicht auszuloesen
        bMBDown = FALSE;

    }

    bReturn = FuConstruct::MouseButtonUp(rMEvt) || bReturn;

    if (!bPermanent && bCreated)
        pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);

    return(bReturn);
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/

BOOL FuConstructBezierPolygon::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FuConstruct::KeyInput(rKEvt);

    return(bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuConstructBezierPolygon::Activate()
{
    pView->EnableExtendedMouseEventDispatcher(TRUE);

    SdrObjKind eKind;

    switch (nSlotId)
    {
        case SID_DRAW_POLYGON_NOFILL:
        case SID_DRAW_XPOLYGON_NOFILL:
        {
            eKind = OBJ_PLIN;
        }
        break;

        case SID_DRAW_POLYGON:
        case SID_DRAW_XPOLYGON:
        {
            eKind = OBJ_POLY;
        }
        break;

        case SID_DRAW_BEZIER_NOFILL:
        {
            eKind = OBJ_PATHLINE;
        }
        break;

        case SID_DRAW_BEZIER_FILL:
        {
            eKind = OBJ_PATHFILL;
        }
        break;

        case SID_DRAW_FREELINE_NOFILL:
        {
            eKind = OBJ_FREELINE;
        }
        break;

        case SID_DRAW_FREELINE:
        {
            eKind = OBJ_FREEFILL;
        }
        break;

        default:
        {
            eKind = OBJ_PATHLINE;
        }
        break;
    }

    pView->SetCurrentObj(eKind);

    FuConstruct::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuConstructBezierPolygon::Deactivate()
{
    pView->EnableExtendedMouseEventDispatcher(FALSE);

    FuConstruct::Deactivate();
}


/*************************************************************************
|*
|* Selektion hat sich geaendert
|*
\************************************************************************/

void FuConstructBezierPolygon::SelectionHasChanged()
{
    FuDraw::SelectionHasChanged();

    // Depending on the context of the view we activate the bezier tool bar
    // or switch back to the default tool bar.
    ShellId nObjectBarId (pViewShell->GetObjectBarManager().GetDefaultObjectBarId());

    if (pView->GetContext() == SDRCONTEXT_POINTEDIT)
        nObjectBarId = RID_BEZIER_TOOLBOX;

    pViewShell->GetObjectBarManager().SwitchObjectBar (nObjectBarId);
}



/*************************************************************************
|*
|* Aktuellen Bezier-Editmodus setzen
|*
\************************************************************************/

void FuConstructBezierPolygon::SetEditMode(USHORT nMode)
{
    nEditMode = nMode;
    ForcePointer();

    SfxBindings& rBindings = pViewShell->GetViewFrame()->GetBindings();
    rBindings.Invalidate(SID_BEZIER_MOVE);
    rBindings.Invalidate(SID_BEZIER_INSERT);
}

// #97016#
SdrObject* FuConstructBezierPolygon::CreateDefaultObject(const sal_uInt16 nID, const Rectangle& rRectangle)
{
    // case SID_DRAW_POLYGON:
    // case SID_DRAW_POLYGON_NOFILL:
    // case SID_DRAW_XPOLYGON:
    // case SID_DRAW_XPOLYGON_NOFILL:
    // case SID_DRAW_FREELINE:
    // case SID_DRAW_FREELINE_NOFILL:
    // case SID_DRAW_BEZIER_FILL:          // BASIC
    // case SID_DRAW_BEZIER_NOFILL:        // BASIC

    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        pView->GetCurrentObjInventor(), pView->GetCurrentObjIdentifier(),
        0L, pDoc);

    if(pObj)
    {
        if(pObj->ISA(SdrPathObj))
        {
            XPolyPolygon aPoly;

            switch(nID)
            {
                case SID_DRAW_BEZIER_FILL:
                {
                    sal_Int32 nWdt(rRectangle.GetWidth() / 2);
                    sal_Int32 nHgt(rRectangle.GetHeight() / 2);

                    XPolygon aInnerPoly(rRectangle.Center(), nWdt, nHgt);

                    aPoly.Insert(aInnerPoly);
                    break;
                }
                case SID_DRAW_BEZIER_NOFILL:
                {
                    XPolygon aInnerPoly;
                    aInnerPoly[0] = rRectangle.BottomLeft();
                    aInnerPoly[1] = rRectangle.BottomCenter();
                    aInnerPoly[2] = rRectangle.BottomCenter();
                    aInnerPoly[3] = rRectangle.Center();
                    aInnerPoly[4] = rRectangle.TopCenter();
                    aInnerPoly[5] = rRectangle.TopCenter();
                    aInnerPoly[6] = rRectangle.TopRight();

                    aInnerPoly.SetFlags(1, XPOLY_CONTROL);
                    aInnerPoly.SetFlags(2, XPOLY_CONTROL);
                    aInnerPoly.SetFlags(3, XPOLY_SYMMTR);
                    aInnerPoly.SetFlags(4, XPOLY_CONTROL);
                    aInnerPoly.SetFlags(5, XPOLY_CONTROL);

                    aPoly.Insert(aInnerPoly);
                    break;
                }
                case SID_DRAW_FREELINE:
                case SID_DRAW_FREELINE_NOFILL:
                {
                    XPolygon aInnerPoly;
                    aInnerPoly[0] = rRectangle.BottomLeft();
                    aInnerPoly[1] = rRectangle.TopLeft();
                    aInnerPoly[2] = rRectangle.TopCenter();
                    aInnerPoly[3] = rRectangle.Center();
                    aInnerPoly[4] = rRectangle.BottomCenter();
                    aInnerPoly[5] = rRectangle.BottomRight();
                    aInnerPoly[6] = rRectangle.TopRight();

                    aInnerPoly.SetFlags(1, XPOLY_CONTROL);
                    aInnerPoly.SetFlags(2, XPOLY_CONTROL);
                    aInnerPoly.SetFlags(3, XPOLY_SMOOTH);
                    aInnerPoly.SetFlags(4, XPOLY_CONTROL);
                    aInnerPoly.SetFlags(5, XPOLY_CONTROL);

                    if(SID_DRAW_FREELINE == nID)
                    {
                        aInnerPoly[7] = rRectangle.BottomRight();
                    }

                    aPoly.Insert(aInnerPoly);
                    break;
                }
                case SID_DRAW_XPOLYGON:
                case SID_DRAW_XPOLYGON_NOFILL:
                {
                    XPolygon aInnerPoly;
                    aInnerPoly[0] = rRectangle.BottomLeft();
                    aInnerPoly[1] = rRectangle.TopLeft();
                    aInnerPoly[2] = rRectangle.TopCenter();
                    aInnerPoly[3] = rRectangle.Center();
                    aInnerPoly[4] = rRectangle.RightCenter();
                    aInnerPoly[5] = rRectangle.BottomRight();

                    if(SID_DRAW_XPOLYGON_NOFILL == nID)
                    {
                        aInnerPoly[6] = rRectangle.BottomCenter();
                    }

                    aPoly.Insert(aInnerPoly);
                    break;
                }
                case SID_DRAW_POLYGON:
                case SID_DRAW_POLYGON_NOFILL:
                {
                    XPolygon aInnerPoly;
                    sal_Int32 nWdt(rRectangle.GetWidth());
                    sal_Int32 nHgt(rRectangle.GetHeight());

                    aInnerPoly[0] = rRectangle.BottomLeft();
                    aInnerPoly[1] = rRectangle.TopLeft() + Point((nWdt * 30) / 100, (nHgt * 70) / 100);
                    aInnerPoly[2] = rRectangle.TopLeft() + Point(0, (nHgt * 15) / 100);
                    aInnerPoly[3] = rRectangle.TopLeft() + Point((nWdt * 65) / 100, 0);
                    aInnerPoly[4] = rRectangle.TopLeft() + Point(nWdt, (nHgt * 30) / 100);
                    aInnerPoly[5] = rRectangle.TopLeft() + Point((nWdt * 80) / 100, (nHgt * 50) / 100);
                    aInnerPoly[6] = rRectangle.TopLeft() + Point((nWdt * 80) / 100, (nHgt * 75) / 100);
                    aInnerPoly[7] = rRectangle.BottomRight();

                    if(SID_DRAW_POLYGON_NOFILL == nID)
                    {
                        aInnerPoly[8] = rRectangle.BottomCenter();
                    }

                    aPoly.Insert(aInnerPoly);
                    break;
                }
            }

            ((SdrPathObj*)pObj)->SetPathPoly(aPoly);
        }
        else
        {
            DBG_ERROR("Object is NO path object");
        }

        pObj->SetLogicRect(rRectangle);
    }

    return pObj;
}

} // end of namespace sd
