/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "BezierObjectBar.hxx"
#include <sfx2/app.hxx>
#include <sfx2/msg.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/objface.hxx>

#include <svx/svxids.hrc>
#include <svl/eitem.hxx>
#include <sfx2/request.hxx>
#include <svx/svdopath.hxx>
#include <svl/aeitem.hxx>
#include <svx/svdundo.hxx>
#include <sfx2/dispatch.hxx>


#include "sdresid.hxx"



#include "res_bmp.hrc"
#include "glob.hrc"
#include "strings.hrc"

#include "DrawDocShell.hxx"
#include "ViewShell.hxx"
#include "Window.hxx"
#include "drawview.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "fusel.hxx"
#include "fuconbez.hxx"


using namespace sd;
#define BezierObjectBar
#include "sdslots.hxx"

namespace sd {

/*************************************************************************
|*
|* Standardinterface deklarieren (Die Slotmap darf nicht leer sein)
|*
\************************************************************************/



SFX_IMPL_INTERFACE(BezierObjectBar, ::SfxShell, SdResId(STR_BEZIEROBJECTBARSHELL))
{
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
      mpView(pSdView),
      mpViewSh(pSdViewShell)
{
    DrawDocShell* pDocShell = mpViewSh->GetDocSh();
    SetPool(&pDocShell->GetPool());
    SetUndoManager(pDocShell->GetUndoManager());
    SetRepeatTarget(mpView);

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
    SfxItemSet aAttrSet( mpView->GetDoc()->GetPool() );
    mpView->GetAttributes( aAttrSet );
    rSet.Put(aAttrSet, sal_False); // <- sal_False, damit DontCare-Status uebernommen wird

    FunctionReference xFunc( mpViewSh->GetCurrentFunction() );

    if(xFunc.is())
    {
        if(xFunc->ISA(FuSelection))
        {
            sal_uInt16 nEditMode = static_cast<FuSelection*>(xFunc.get())->GetEditMode();
            rSet.Put(SfxBoolItem(nEditMode, sal_True));
        }
        else if (xFunc->ISA(FuConstructBezierPolygon))
        {
            sal_uInt16 nEditMode = static_cast<FuConstructBezierPolygon*>(xFunc.get())->GetEditMode();
            rSet.Put(SfxBoolItem(nEditMode, sal_True));
        }
    }

    if(!mpView->IsMoveAllowed() || !mpView->IsResizeAllowed())
    {
        // #i77187# if object is move and/or size protected, do not allow point editing at all
        rSet.DisableItem(SID_BEZIER_MOVE);
        rSet.DisableItem(SID_BEZIER_INSERT);

        rSet.DisableItem(SID_BEZIER_DELETE);
        rSet.DisableItem(SID_BEZIER_CUTLINE);
        rSet.DisableItem(SID_BEZIER_CONVERT);

        rSet.DisableItem(SID_BEZIER_EDGE);
        rSet.DisableItem(SID_BEZIER_SMOOTH);
        rSet.DisableItem(SID_BEZIER_SYMMTR);

        rSet.DisableItem(SID_BEZIER_CLOSE);

        rSet.DisableItem(SID_BEZIER_ELIMINATE_POINTS);
    }
    else
    {
        IPolyPolygonEditorController* pIPPEC = 0;
        if( mpView->GetMarkedObjectList().GetMarkCount() )
            pIPPEC = mpView;
        else
            pIPPEC = dynamic_cast< IPolyPolygonEditorController* >( mpView->getSmartTags().getSelected().get() );

        if ( !pIPPEC || !pIPPEC->IsRipUpAtMarkedPointsPossible())
        {
            rSet.DisableItem(SID_BEZIER_CUTLINE);
        }
        if (!pIPPEC || !pIPPEC->IsDeleteMarkedPointsPossible())
        {
            rSet.DisableItem(SID_BEZIER_DELETE);
        }
        if (!pIPPEC || !pIPPEC->IsSetMarkedSegmentsKindPossible())
        {
            rSet.DisableItem(SID_BEZIER_CONVERT);
        }
        else
        {
            SdrPathSegmentKind eSegm = pIPPEC->GetMarkedSegmentsKind();
            switch (eSegm)
            {
                case SDRPATHSEGMENT_DONTCARE: rSet.InvalidateItem(SID_BEZIER_CONVERT); break;
                case SDRPATHSEGMENT_LINE    : rSet.Put(SfxBoolItem(SID_BEZIER_CONVERT,sal_False)); break; // Button reingedrueckt = Kurve
                case SDRPATHSEGMENT_CURVE   : rSet.Put(SfxBoolItem(SID_BEZIER_CONVERT,sal_True));  break;
                default: break;
            }
        }
        if (!pIPPEC || !pIPPEC->IsSetMarkedPointsSmoothPossible())
        {
            rSet.DisableItem(SID_BEZIER_EDGE);
            rSet.DisableItem(SID_BEZIER_SMOOTH);
            rSet.DisableItem(SID_BEZIER_SYMMTR);
        }
        else
        {
            SdrPathSmoothKind eSmooth = pIPPEC->GetMarkedPointsSmooth();
            switch (eSmooth)
            {
                case SDRPATHSMOOTH_DONTCARE  : break;
                case SDRPATHSMOOTH_ANGULAR   : rSet.Put(SfxBoolItem(SID_BEZIER_EDGE,  sal_True)); break;
                case SDRPATHSMOOTH_ASYMMETRIC: rSet.Put(SfxBoolItem(SID_BEZIER_SMOOTH,sal_True)); break;
                case SDRPATHSMOOTH_SYMMETRIC : rSet.Put(SfxBoolItem(SID_BEZIER_SYMMTR,sal_True)); break;
            }
        }
        if (!pIPPEC || !pIPPEC->IsOpenCloseMarkedObjectsPossible())
        {
            rSet.DisableItem(SID_BEZIER_CLOSE);
        }
        else
        {
            SdrObjClosedKind eClose = pIPPEC->GetMarkedObjectsClosedState();
            switch (eClose)
            {
                case SDROBJCLOSED_DONTCARE: rSet.InvalidateItem(SID_BEZIER_CLOSE); break;
                case SDROBJCLOSED_OPEN    : rSet.Put(SfxBoolItem(SID_BEZIER_CLOSE,sal_False)); break;
                case SDROBJCLOSED_CLOSED  : rSet.Put(SfxBoolItem(SID_BEZIER_CLOSE,sal_True)); break;
                default: break;
            }
        }

        if(pIPPEC == mpView)
            rSet.Put(SfxBoolItem(SID_BEZIER_ELIMINATE_POINTS, mpView->IsEliminatePolyPoints()));
        else
            rSet.DisableItem( SID_BEZIER_ELIMINATE_POINTS ); // only works for views
    }
}


/*************************************************************************
|*
|* Bearbeitung der SfxRequests
|*
\************************************************************************/

void BezierObjectBar::Execute(SfxRequest& rReq)
{
    sal_uInt16 nSId = rReq.GetSlot();

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
            const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

            IPolyPolygonEditorController* pIPPEC = 0;
            if( rMarkList.GetMarkCount() )
                pIPPEC = mpView;
            else
                pIPPEC = dynamic_cast< IPolyPolygonEditorController* >( mpView->getSmartTags().getSelected().get() );

            if( pIPPEC && !mpView->IsAction())
            {
                switch (nSId)
                {
                    case SID_BEZIER_DELETE:
                        pIPPEC->DeleteMarkedPoints();
                        break;

                    case SID_BEZIER_CUTLINE:
                        pIPPEC->RipUpAtMarkedPoints();
                        break;

                    case SID_BEZIER_CONVERT:
                    {
                        pIPPEC->SetMarkedSegmentsKind(SDRPATHSEGMENT_TOGGLE);
                        break;
                    }

                    case SID_BEZIER_EDGE:
                    case SID_BEZIER_SMOOTH:
                    case SID_BEZIER_SYMMTR:
                    {
                        SdrPathSmoothKind eKind;

                        switch (nSId)
                        {
                            default:
                            case SID_BEZIER_EDGE:   eKind = SDRPATHSMOOTH_ANGULAR; break;
                            case SID_BEZIER_SMOOTH: eKind = SDRPATHSMOOTH_ASYMMETRIC; break;
                            case SID_BEZIER_SYMMTR: eKind = SDRPATHSMOOTH_SYMMETRIC; break;
                        }

                        pIPPEC->SetMarkedPointsSmooth(eKind);
                        break;
                    }

                    case SID_BEZIER_CLOSE:
                    {
                        SdrPathObj* pPathObj = (SdrPathObj*) rMarkList.GetMark(0)->GetMarkedSdrObj();
                        const bool bUndo = mpView->IsUndoEnabled();
                        if( bUndo )
                            mpView->BegUndo(String(SdResId(STR_UNDO_BEZCLOSE)));

                        mpView->UnmarkAllPoints();

                        if( bUndo )
                            mpView->AddUndo(mpView->GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pPathObj));

                        pPathObj->ToggleClosed();

                        if( bUndo )
                            mpView->EndUndo();
                        break;
                    }
                }
            }

            if( (pIPPEC == mpView) && !mpView->AreObjectsMarked() )
                mpViewSh->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);

            rReq.Ignore();
        }
        break;

        case SID_BEZIER_ELIMINATE_POINTS:
        {
            mpView->SetEliminatePolyPoints(!mpView->IsEliminatePolyPoints());
            Invalidate(SID_BEZIER_ELIMINATE_POINTS);
            rReq.Done();
        }
        break;

        case SID_BEZIER_MOVE:
        case SID_BEZIER_INSERT:
        {
            FunctionReference xFunc( mpViewSh->GetCurrentFunction() );

            if(xFunc.is())
            {
                if(xFunc->ISA(FuSelection))
                {
                    static_cast<FuSelection*>(xFunc.get())->SetEditMode(rReq.GetSlot());
                }
                else if(xFunc->ISA(FuConstructBezierPolygon))
                {
                    static_cast<FuConstructBezierPolygon*>(xFunc.get())->SetEditMode(rReq.GetSlot());
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
