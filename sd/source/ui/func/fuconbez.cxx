/*************************************************************************
 *
 *  $RCSfile: fuconbez.cxx,v $
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

#include <basctl/idetemp.hxx>
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
#include "viewshel.hxx"
#include "sdview.hxx"
#include "sdwindow.hxx"
#include "drawdoc.hxx"
#include "fuconbez.hxx"
#include "res_bmp.hrc"

TYPEINIT1( FuConstBezPoly, FuConstruct );


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConstBezPoly::FuConstBezPoly(SdViewShell* pViewSh, SdWindow* pWin,
                               SdView* pView, SdDrawDocument* pDoc,
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

FuConstBezPoly::~FuConstBezPoly()
{
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

BOOL FuConstBezPoly::MouseButtonDown(const MouseEvent& rMEvt)
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
            pObj->NbcSetAttributes(aAttr, FALSE);
        }
    }

    return(bReturn);
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

BOOL FuConstBezPoly::MouseMove(const MouseEvent& rMEvt)
{
    BOOL bReturn = FuConstruct::MouseMove(rMEvt);
    return(bReturn);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL FuConstBezPoly::MouseButtonUp(const MouseEvent& rMEvt)
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

BOOL FuConstBezPoly::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FuConstruct::KeyInput(rKEvt);

    return(bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuConstBezPoly::Activate()
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

void FuConstBezPoly::Deactivate()
{
    pView->EnableExtendedMouseEventDispatcher(FALSE);

    FuConstruct::Deactivate();
}


/*************************************************************************
|*
|* Selektion hat sich geaendert
|*
\************************************************************************/

void FuConstBezPoly::SelectionHasChanged()
{
    FuDraw::SelectionHasChanged();

    /**************************************************************************
    * ObjectBar einschalten
    **************************************************************************/
    USHORT nObjBarId = RID_DRAW_OBJ_TOOLBOX;

    if (pView->GetContext() == SDRCONTEXT_POINTEDIT)
    {
        nObjBarId = RID_BEZIER_TOOLBOX;
    }

    pViewShell->SwitchObjectBar(nObjBarId);
}



/*************************************************************************
|*
|* Aktuellen Bezier-Editmodus setzen
|*
\************************************************************************/

void FuConstBezPoly::SetEditMode(USHORT nMode)
{
    nEditMode = nMode;
    ForcePointer();

    SfxBindings& rBindings = SFX_BINDINGS();
    rBindings.Invalidate(SID_BEZIER_MOVE);
    rBindings.Invalidate(SID_BEZIER_INSERT);
}





