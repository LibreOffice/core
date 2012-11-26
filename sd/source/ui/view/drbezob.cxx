/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    SfxItemSet aAttrSet( mpView->GetDoc()->GetItemPool() );
    mpView->GetAttributes( aAttrSet );
    rSet.Put(aAttrSet, false); // <- false, damit DontCare-Status uebernommen wird

    FunctionReference xFunc( mpViewSh->GetCurrentFunction() );

    if(xFunc.is())
    {
        FuSelection* pFuSelection = dynamic_cast< FuSelection* >(xFunc.get());

        if(pFuSelection)
        {
            sal_uInt16 nEditMode = pFuSelection->GetEditMode();
            rSet.Put(SfxBoolItem(nEditMode, true));
        }
        else
        {
            FuConstructBezierPolygon* pFuConstructBezierPolygon = dynamic_cast< FuConstructBezierPolygon* >(xFunc.get());

            if (pFuConstructBezierPolygon)
            {
                sal_uInt16 nEditMode = pFuConstructBezierPolygon->GetEditMode();
                rSet.Put(SfxBoolItem(nEditMode, true));
            }
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
        if( mpView->areSdrObjectsSelected() )
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
                case SDRPATHSEGMENT_LINE    : rSet.Put(SfxBoolItem(SID_BEZIER_CONVERT,false)); break; // Button reingedrueckt = Kurve
                case SDRPATHSEGMENT_CURVE   : rSet.Put(SfxBoolItem(SID_BEZIER_CONVERT,true));  break;
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
                case SDRPATHSMOOTH_ANGULAR   : rSet.Put(SfxBoolItem(SID_BEZIER_EDGE,  true)); break;
                case SDRPATHSMOOTH_ASYMMETRIC: rSet.Put(SfxBoolItem(SID_BEZIER_SMOOTH,true)); break;
                case SDRPATHSMOOTH_SYMMETRIC : rSet.Put(SfxBoolItem(SID_BEZIER_SYMMTR,true)); break;
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
                case SDROBJCLOSED_OPEN    : rSet.Put(SfxBoolItem(SID_BEZIER_CLOSE,false)); break;
                case SDROBJCLOSED_CLOSED  : rSet.Put(SfxBoolItem(SID_BEZIER_CLOSE,true)); break;
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
            IPolyPolygonEditorController* pIPPEC = 0;
            if( mpView->areSdrObjectsSelected() )
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
                        SdrPathObj* pPathObj = dynamic_cast< SdrPathObj* >(mpView->getSelectedIfSingle());
                        OSL_ENSURE(pPathObj, "Here the single selected object should be a SdrPathObj (!)");

                        if(pPathObj)
                        {
                            const bool bUndo = mpView->IsUndoEnabled();

                            if( bUndo )
                                mpView->BegUndo(String(SdResId(STR_UNDO_BEZCLOSE)));

                            mpView->MarkPoints(0, true); // unmarkall

                            if( bUndo )
                                mpView->AddUndo(mpView->getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pPathObj));

                            pPathObj->ToggleClosed();

                            if( bUndo )
                                mpView->EndUndo();
                        }
                        break;
                    }
                }
            }

            if( (pIPPEC == mpView) && !mpView->areSdrObjectsSelected() )
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
                FuSelection* pFuSelection = dynamic_cast< FuSelection* >(xFunc.get());

                if(pFuSelection)
                {
                    pFuSelection->SetEditMode(rReq.GetSlot());
                }
                else
                {
                    FuConstructBezierPolygon* pFuConstructBezierPolygon = dynamic_cast< FuConstructBezierPolygon* >(xFunc.get());

                    if(pFuConstructBezierPolygon)
                    {
                        pFuConstructBezierPolygon->SetEditMode(rReq.GetSlot());
                    }
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
