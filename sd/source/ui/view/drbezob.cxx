/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <BezierObjectBar.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objface.hxx>

#include <svx/svxids.hrc>
#include <svl/eitem.hxx>
#include <sfx2/request.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdundo.hxx>
#include <sfx2/dispatch.hxx>

#include <sdresid.hxx>


#include <strings.hrc>

#include <DrawDocShell.hxx>
#include <ViewShell.hxx>
#include <drawdoc.hxx>
#include <fusel.hxx>
#include <fuconbez.hxx>

using namespace sd;
#define ShellClass_BezierObjectBar
#include <sdslots.hxx>

namespace sd {

/**
 * Declare default interface (Slotmap must not be empty)
 */
SFX_IMPL_INTERFACE(BezierObjectBar, ::SfxShell)

void BezierObjectBar::InitInterface_Impl()
{
}


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
}

BezierObjectBar::~BezierObjectBar()
{
    SetRepeatTarget(nullptr);
}

/**
 * Status of attribute items.
 */

void BezierObjectBar::GetAttrState(SfxItemSet& rSet)
{
    SfxItemSet aAttrSet( mpView->GetDoc().GetPool() );
    mpView->GetAttributes( aAttrSet );
    rSet.Put(aAttrSet, false); // <- sal_False, so DontCare-Status gets acquired

    rtl::Reference<FuPoor> xFunc( mpViewSh->GetCurrentFunction() );

    if(xFunc.is())
    {
        if( auto pFuSelection = dynamic_cast< const FuSelection *>( xFunc.get() ))
        {
            sal_uInt16 nEditMode = pFuSelection->GetEditMode();
            rSet.Put(SfxBoolItem(nEditMode, true));
        }
        else if( auto pFuPolygon = dynamic_cast< const FuConstructBezierPolygon *>( xFunc.get() ))
        {
            sal_uInt16 nEditMode = pFuPolygon->GetEditMode();
            rSet.Put(SfxBoolItem(nEditMode, true));
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
        IPolyPolygonEditorController* pIPPEC = nullptr;
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
                case SdrPathSegmentKind::DontCare: rSet.InvalidateItem(SID_BEZIER_CONVERT); break;
                case SdrPathSegmentKind::Line    : rSet.Put(SfxBoolItem(SID_BEZIER_CONVERT,false)); break; // Button down = curve
                case SdrPathSegmentKind::Curve   : rSet.Put(SfxBoolItem(SID_BEZIER_CONVERT,true));  break;
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
                case SdrPathSmoothKind::DontCare  : break;
                case SdrPathSmoothKind::Angular   : rSet.Put(SfxBoolItem(SID_BEZIER_EDGE,  true)); break;
                case SdrPathSmoothKind::Asymmetric: rSet.Put(SfxBoolItem(SID_BEZIER_SMOOTH,true)); break;
                case SdrPathSmoothKind::Symmetric : rSet.Put(SfxBoolItem(SID_BEZIER_SYMMTR,true)); break;
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
                case SdrObjClosedKind::DontCare: rSet.InvalidateItem(SID_BEZIER_CLOSE); break;
                case SdrObjClosedKind::Open    : rSet.Put(SfxBoolItem(SID_BEZIER_CLOSE,false)); break;
                case SdrObjClosedKind::Closed  : rSet.Put(SfxBoolItem(SID_BEZIER_CLOSE,true)); break;
                default: break;
            }
        }

        if(pIPPEC == mpView)
            rSet.Put(SfxBoolItem(SID_BEZIER_ELIMINATE_POINTS, mpView->IsEliminatePolyPoints()));
        else
            rSet.DisableItem( SID_BEZIER_ELIMINATE_POINTS ); // only works for views
    }
}

/**
 * Process SfxRequests
 */

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
            assert(mpView);
            const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

            IPolyPolygonEditorController* pIPPEC = nullptr;
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
                        pIPPEC->SetMarkedSegmentsKind(SdrPathSegmentKind::Toggle);
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
                            case SID_BEZIER_EDGE:   eKind = SdrPathSmoothKind::Angular; break;
                            case SID_BEZIER_SMOOTH: eKind = SdrPathSmoothKind::Asymmetric; break;
                            case SID_BEZIER_SYMMTR: eKind = SdrPathSmoothKind::Symmetric; break;
                        }

                        pIPPEC->SetMarkedPointsSmooth(eKind);
                        break;
                    }

                    case SID_BEZIER_CLOSE:
                    {
                        SdrPathObj* pPathObj = static_cast<SdrPathObj*>( rMarkList.GetMark(0)->GetMarkedSdrObj() );
                        const bool bUndo = mpView->IsUndoEnabled();
                        if( bUndo )
                            mpView->BegUndo(SdResId(STR_UNDO_BEZCLOSE));

                        mpView->UnmarkAllPoints();

                        if( bUndo )
                            mpView->AddUndo(mpView->GetModel().GetSdrUndoFactory().CreateUndoGeoObject(*pPathObj));

                        pPathObj->ToggleClosed();

                        if( bUndo )
                            mpView->EndUndo();
                        break;
                    }
                }
            }

            if( (pIPPEC == mpView) && mpView->GetMarkedObjectList().GetMarkCount() == 0 )
                mpViewSh->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);

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
            rtl::Reference<FuPoor> xFunc( mpViewSh->GetCurrentFunction() );

            if(xFunc.is())
            {
                if( auto pFuSelection = dynamic_cast<FuSelection *>( xFunc.get() ))
                {
                    pFuSelection->SetEditMode(rReq.GetSlot());
                }
                else if( auto pFuPolygon = dynamic_cast<FuConstructBezierPolygon *>( xFunc.get() ))
                {
                    pFuPolygon->SetEditMode(rReq.GetSlot());
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
