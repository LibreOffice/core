/*************************************************************************
 *
 *  $RCSfile: drbezob.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:15:35 $
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

#include "BezierObjectBar.hxx"

#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFXMSG_HXX
#include <sfx2/msg.hxx>
#endif


#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SVDOPATH_HXX //autogen
#include <svx/svdopath.hxx>
#endif
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SVDUNDO_HXX //autogen
#include <svx/svdundo.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif


#ifndef SD_RESID_HXX
#include "sdresid.hxx"
#endif


#pragma hdrstop

#include "res_bmp.hrc"
#include "glob.hrc"
#include "strings.hrc"

#include "DrawDocShell.hxx"
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#include "drawdoc.hxx"
#include "sdresid.hxx"
#ifndef SD_FU_SELECTION_HXX
#include "fusel.hxx"
#endif
#ifndef SD_FU_CONSTRUCT_BEZIER_HXX
#include "fuconbez.hxx"
#endif


using namespace sd;
#define BezierObjectBar
#include "sdslots.hxx"

namespace sd {

/*************************************************************************
|*
|* Standardinterface deklarieren (Die Slotmap darf nicht leer sein)
|*
\************************************************************************/


SFX_DECL_TYPE(13);


SFX_IMPL_INTERFACE(BezierObjectBar, ::SfxShell, SdResId(STR_BEZIEROBJECTBARSHELL))
{
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT, SdResId(RID_BEZIER_TOOLBOX) );
}

TYPEINIT1(BezierObjectBar, ::SfxShell);

/*************************************************************************
|*
|* Standard-Konstruktor
|*
\************************************************************************/

BezierObjectBar::BezierObjectBar(
    ViewShell* pSdViewShell,
    ::sd::View* pSdView)
    : SfxShell(pSdViewShell->GetViewShell()),
      pView(pSdView),
      pViewSh(pSdViewShell)
{
    DrawDocShell* pDocShell = pViewSh->GetDocSh();
    SetPool(&pDocShell->GetPool());
    SetUndoManager(pDocShell->GetUndoManager());
    SetRepeatTarget(pView);

    SetHelpId( SD_IF_SDDRAWBEZIEROBJECTBAR );
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

BezierObjectBar::~BezierObjectBar()
{
    SetRepeatTarget(NULL);
}


/*************************************************************************
|*
|* Status der Attribut-Items
|*
\************************************************************************/

void BezierObjectBar::GetAttrState(SfxItemSet& rSet)
{
    SfxItemSet aAttrSet( pView->GetDoc()->GetPool() );
    pView->GetAttributes( aAttrSet );
    rSet.Put(aAttrSet, FALSE); // <- FALSE, damit DontCare-Status uebernommen wird

    FuPoor* pFuActual = pViewSh->GetActualFunction();

    if (pFuActual)
    {
        if (pFuActual->ISA(FuSelection))
        {
            USHORT nEditMode = ((FuSelection*) pFuActual)->GetEditMode();
            rSet.Put(SfxBoolItem(nEditMode, TRUE));
        }
        else if (pFuActual->ISA(FuConstructBezierPolygon))
        {
            USHORT nEditMode =
                static_cast<FuConstructBezierPolygon*>(pFuActual)
                ->GetEditMode();
            rSet.Put(SfxBoolItem(nEditMode, TRUE));
        }
    }

    if (!pView->IsRipUpAtMarkedPointsPossible())
    {
        rSet.DisableItem(SID_BEZIER_CUTLINE);
    }
    if (!pView->IsDeleteMarkedPointsPossible())
    {
        rSet.DisableItem(SID_BEZIER_DELETE);
    }
    if (!pView->IsSetMarkedSegmentsKindPossible())
    {
        rSet.DisableItem(SID_BEZIER_CONVERT);
    }
    else
    {
        SdrPathSegmentKind eSegm = pView->GetMarkedSegmentsKind();
        switch (eSegm)
        {
            case SDRPATHSEGMENT_DONTCARE: rSet.InvalidateItem(SID_BEZIER_CONVERT); break;
            case SDRPATHSEGMENT_LINE    : rSet.Put(SfxBoolItem(SID_BEZIER_CONVERT,FALSE)); break; // Button reingedrueckt = Kurve
            case SDRPATHSEGMENT_CURVE   : rSet.Put(SfxBoolItem(SID_BEZIER_CONVERT,TRUE));  break;
        }
    }
    if (!pView->IsSetMarkedPointsSmoothPossible())
    {
        rSet.DisableItem(SID_BEZIER_EDGE);
        rSet.DisableItem(SID_BEZIER_SMOOTH);
        rSet.DisableItem(SID_BEZIER_SYMMTR);
    }
    else
    {
        SdrPathSmoothKind eSmooth = pView->GetMarkedPointsSmooth();
        switch (eSmooth)
        {
            case SDRPATHSMOOTH_DONTCARE  : break;
            case SDRPATHSMOOTH_ANGULAR   : rSet.Put(SfxBoolItem(SID_BEZIER_EDGE,  TRUE)); break;
            case SDRPATHSMOOTH_ASYMMETRIC: rSet.Put(SfxBoolItem(SID_BEZIER_SMOOTH,TRUE)); break;
            case SDRPATHSMOOTH_SYMMETRIC : rSet.Put(SfxBoolItem(SID_BEZIER_SYMMTR,TRUE)); break;
        }
    }
    if (!pView->IsOpenCloseMarkedObjectsPossible())
    {
        rSet.DisableItem(SID_BEZIER_CLOSE);
    }
    else
    {
        SdrObjClosedKind eClose = pView->GetMarkedObjectsClosedState();
        switch (eClose)
        {
            case SDROBJCLOSED_DONTCARE: rSet.InvalidateItem(SID_BEZIER_CLOSE); break;
            case SDROBJCLOSED_OPEN    : rSet.Put(SfxBoolItem(SID_BEZIER_CLOSE,FALSE)); break;
            case SDROBJCLOSED_CLOSED  : rSet.Put(SfxBoolItem(SID_BEZIER_CLOSE,TRUE)); break;
        }
    }

    rSet.Put(SfxBoolItem(SID_BEZIER_ELIMINATE_POINTS, pView->IsEliminatePolyPoints()));
}


/*************************************************************************
|*
|* Bearbeitung der SfxRequests
|*
\************************************************************************/

void BezierObjectBar::Execute(SfxRequest& rReq)
{
    USHORT nSId = rReq.GetSlot();

    switch (nSId)
    {
        case SID_BEZIER_CUTLINE:
        case SID_BEZIER_CONVERT:
        case SID_BEZIER_DELETE:
        case SID_BEZIER_EDGE:
        case SID_BEZIER_SMOOTH:
        case SID_BEZIER_SYMMTR:
        case SID_BEZIER_CLOSE:
        {
            const SdrMarkList& rMarkList = pView->GetMarkedObjectList();

            if (rMarkList.GetMark(0) && !pView->IsAction())
            {
                switch (nSId)
                {
                    case SID_BEZIER_DELETE:
                        pView->DeleteMarkedPoints();
                        break;

                    case SID_BEZIER_CUTLINE:
                        pView->RipUpAtMarkedPoints();
                        break;

                    case SID_BEZIER_CONVERT:
                    {
                        pView->SetMarkedSegmentsKind(SDRPATHSEGMENT_TOGGLE);
                        break;
                    }

                    case SID_BEZIER_EDGE:
                    case SID_BEZIER_SMOOTH:
                    case SID_BEZIER_SYMMTR:
                    {
                        SdrPathSmoothKind eKind;

                        switch (nSId)
                        {
                            case SID_BEZIER_EDGE:   eKind = SDRPATHSMOOTH_ANGULAR; break;
                            case SID_BEZIER_SMOOTH: eKind = SDRPATHSMOOTH_ASYMMETRIC; break;
                            case SID_BEZIER_SYMMTR: eKind = SDRPATHSMOOTH_SYMMETRIC; break;
                        }

                        pView->SetMarkedPointsSmooth(eKind);
                        break;
                    }

                    case SID_BEZIER_CLOSE:
                    {
                        SdrPathObj* pPathObj = (SdrPathObj*) rMarkList.GetMark(0)->GetObj();
                        pView->BegUndo(String(SdResId(STR_UNDO_BEZCLOSE)));
                        pView->UnmarkAllPoints();
                        Size aDist(pViewSh->GetActiveWindow()->PixelToLogic(Size(8,8)));
                        pView->AddUndo(new SdrUndoGeoObj(*pPathObj));
                        pPathObj->ToggleClosed(aDist.Width());
                        pView->EndUndo();
                        break;
                    }
                }
            }

            if ( !pView->AreObjectsMarked() )
                pViewSh->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);

            rReq.Ignore();
        }
        break;

        case SID_BEZIER_ELIMINATE_POINTS:
        {
            pView->SetEliminatePolyPoints(!pView->IsEliminatePolyPoints());
            Invalidate(SID_BEZIER_ELIMINATE_POINTS);
            rReq.Done();
        }
        break;

        case SID_BEZIER_MOVE:
        case SID_BEZIER_INSERT:
        {
            FuPoor* pFuActual = pViewSh->GetActualFunction();

            if (pFuActual)
            {
                if (pFuActual->ISA(FuSelection))
                {
                    ((FuSelection*) pFuActual)->SetEditMode(rReq.GetSlot());
                }
                else if (pFuActual->ISA(FuConstructBezierPolygon))
                {
                    static_cast<FuConstructBezierPolygon*>(pFuActual)
                        ->SetEditMode(rReq.GetSlot());
                }
            }

            rReq.Ignore ();
        }
        break;

        default:
        break;
    }

    Invalidate();
}


} // end of namespace sd
