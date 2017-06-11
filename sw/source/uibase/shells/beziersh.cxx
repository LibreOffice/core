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

#include "cmdid.h"
#include <svx/svdview.hxx>
#include <svl/srchitem.hxx>
#include <svl/eitem.hxx>
#include <svl/whiter.hxx>
#include <svx/svdopath.hxx>
#include <vcl/EnumContext.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objface.hxx>

#include "wrtsh.hxx"
#include "view.hxx"
#include "edtwin.hxx"
#include "helpid.h"
#include "globals.hrc"
#include "drawbase.hxx"
#include "beziersh.hxx"
#include "strings.hrc"
#define SwBezierShell
#include <sfx2/msg.hxx>
#include "swslots.hxx"

#include <unomid.h>

SFX_IMPL_INTERFACE(SwBezierShell, SwBaseShell)

void SwBezierShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterPopupMenu("draw");

    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT, SfxVisibilityFlags::Invisible, ToolbarId::Bezier_Toolbox_Sw);
}


SwBezierShell::SwBezierShell(SwView &_rView):
    SwBaseShell( _rView )
{
    SetName("Bezier");

    SwWrtShell *pSh = &GetShell();
    SdrView*    pSdrView = pSh->GetDrawView();
    pSdrView->SetEliminatePolyPointLimitAngle(1500L);

    SfxShell::SetContextName(vcl::EnumContext::GetContextName(vcl::EnumContext::Context::Draw));
}

void SwBezierShell::Execute(SfxRequest &rReq)
{
    SwWrtShell *pSh = &GetShell();
    SdrView*    pSdrView = pSh->GetDrawView();
    const SfxItemSet *pArgs = rReq.GetArgs();
    sal_uInt16      nSlotId = rReq.GetSlot();
    bool        bChanged = pSdrView->GetModel()->IsChanged();
    pSdrView->GetModel()->SetChanged(false);
    const SfxPoolItem* pItem;
    if(pArgs)
        pArgs->GetItemState(nSlotId, false, &pItem);

    switch (nSlotId)
    {
        case SID_DELETE:
        case FN_BACKSPACE:
            if (pSh->IsObjSelected())
            {
                if (pSdrView->HasMarkedPoints())
                    pSh->GetView().GetViewFrame()->GetDispatcher()->Execute(SID_BEZIER_DELETE);
                else
                {
                    pSh->DelSelectedObj();
                    if (pSh->IsSelFrameMode())
                    {
                        pSh->LeaveSelFrameMode();
                        pSh->NoEdit();
                    }
                    GetView().AttrChangedNotify(pSh); // Shell change if applicable...
                }
            }
            break;

        case FN_ESCAPE:
            if (pSdrView->HasMarkedPoints())
                pSdrView->UnmarkAllPoints();
            else
            {
                if ( pSh->IsDrawCreate() )
                {
                    GetView().GetDrawFuncPtr()->BreakCreate();
                    GetView().AttrChangedNotify(pSh); // Shell change if applicable...
                }
                else if ( pSh->HasSelection() || GetView().IsDrawMode() )
                {
                    GetView().LeaveDrawCreate();
                    pSh->EnterStdMode();
                    GetView().AttrChangedNotify(pSh); // Shell change if applicable...
                }
            }
            break;

        case SID_BEZIER_MOVE:
        case SID_BEZIER_INSERT:
            {
                GetView().GetEditWin().SetBezierMode(nSlotId);
                static sal_uInt16 aInva[] =
                                {
                                    SID_BEZIER_INSERT,
                                    SID_BEZIER_MOVE,
                                    0
                                };
                GetView().GetViewFrame()->GetBindings().Invalidate(aInva);
            }
            break;

        case SID_BEZIER_DELETE:
        case SID_BEZIER_CUTLINE:
        case SID_BEZIER_CONVERT:
        case SID_BEZIER_EDGE:
        case SID_BEZIER_SMOOTH:
        case SID_BEZIER_SYMMTR:
        case SID_BEZIER_CLOSE:
        case SID_BEZIER_ELIMINATE_POINTS:
        {
            const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();

            if (rMarkList.GetMark(0) && !pSdrView->IsAction())
            {
                switch (nSlotId)
                {
                    case SID_BEZIER_DELETE:
                        pSdrView->DeleteMarkedPoints();
                        break;

                    case SID_BEZIER_CUTLINE:
                        {
                            pSdrView->RipUpAtMarkedPoints();
                            pSh->CheckUnboundObjects();
                        }
                        break;

                    case SID_BEZIER_CONVERT:
                    {
                        pSdrView->SetMarkedSegmentsKind(SdrPathSegmentKind::Toggle);
                        break;
                    }

                    case SID_BEZIER_EDGE:
                    case SID_BEZIER_SMOOTH:
                    case SID_BEZIER_SYMMTR:
                    {
                        SdrPathSmoothKind eKind = SdrPathSmoothKind::Asymmetric;

                        switch (nSlotId)
                        {
                            case SID_BEZIER_EDGE:   eKind = SdrPathSmoothKind::Angular; break;
                            case SID_BEZIER_SMOOTH: eKind = SdrPathSmoothKind::Asymmetric; break;
                            case SID_BEZIER_SYMMTR: eKind = SdrPathSmoothKind::Symmetric; break;
                        }

                        SdrPathSmoothKind eSmooth = pSdrView->GetMarkedPointsSmooth();
                        if (eKind != eSmooth)
                        {
                            pSdrView->SetMarkedPointsSmooth(eKind);

                            static sal_uInt16 aInva[] =
                                            {
                                                SID_BEZIER_SMOOTH,
                                                SID_BEZIER_EDGE,
                                                SID_BEZIER_SYMMTR,
                                                0
                                            };
                            GetView().GetViewFrame()->GetBindings().Invalidate(aInva);
                        }
                        break;
                    }

                    case SID_BEZIER_CLOSE:
                    {
                        SdrPathObj* pPathObj = static_cast<SdrPathObj*>( rMarkList.GetMark(0)->GetMarkedSdrObj() );
                        pSdrView->UnmarkAllPoints();
                        // Size aDist(GetView().GetEditWin().PixelToLogic(Size(8,8)));
                        pPathObj->ToggleClosed(); // aDist.Width());
                        break;
                    }

                    case SID_BEZIER_ELIMINATE_POINTS:
                        pSdrView->SetEliminatePolyPoints(!pSdrView->IsEliminatePolyPoints());
                        break;
                }
            }
        }
        break;

        default:
            break;
    }

    if (pSdrView->GetModel()->IsChanged())
        GetShell().SetModified();
    else if (bChanged)
        pSdrView->GetModel()->SetChanged();
}

void SwBezierShell::GetState(SfxItemSet &rSet)
{
    SdrView* pSdrView = GetShell().GetDrawView();

    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();

    while( nWhich )
    {
        switch( nWhich )
        {
            case SID_BEZIER_MOVE:
            case SID_BEZIER_INSERT:
            {
                sal_uInt16 nEditMode = GetView().GetEditWin().GetBezierMode();

                rSet.Put(SfxBoolItem(nWhich, nEditMode == nWhich));
            }
            break;

            case SID_BEZIER_CUTLINE:
                if (!pSdrView->IsRipUpAtMarkedPointsPossible())
                {
                    rSet.DisableItem(SID_BEZIER_CUTLINE);
                }
                break;

            case SID_BEZIER_DELETE:
                if (!pSdrView->IsDeleteMarkedPointsPossible())
                {
                    rSet.DisableItem(SID_BEZIER_DELETE);
                }
                break;

            case SID_BEZIER_CONVERT:
                if (!pSdrView->IsSetMarkedSegmentsKindPossible())
                {
                    rSet.DisableItem(SID_BEZIER_CONVERT);
                }
                else
                {
                    SdrPathSegmentKind eSegm = pSdrView->GetMarkedSegmentsKind();
                    switch (eSegm)
                    {
                        case SdrPathSegmentKind::DontCare: rSet.InvalidateItem(SID_BEZIER_CONVERT); break;
                        case SdrPathSegmentKind::Line    : rSet.Put(SfxBoolItem(SID_BEZIER_CONVERT,false)); break; // Button pressed = curve
                        case SdrPathSegmentKind::Curve   : rSet.Put(SfxBoolItem(SID_BEZIER_CONVERT,true));  break;
                        default:; //prevent warning
                    }
                }
                break;

            case SID_BEZIER_EDGE:
            case SID_BEZIER_SMOOTH:
            case SID_BEZIER_SYMMTR:
                if (!pSdrView->IsSetMarkedPointsSmoothPossible())
                    rSet.DisableItem(nWhich);
                else
                {
                    SdrPathSmoothKind eSmooth = pSdrView->GetMarkedPointsSmooth();
                    bool bEnable = false;
                    switch (eSmooth)
                    {
                        case SdrPathSmoothKind::DontCare  :
                            break;
                        case SdrPathSmoothKind::Angular   :
                            bEnable = nWhich == SID_BEZIER_EDGE;
                            break;
                        case SdrPathSmoothKind::Asymmetric:
                            bEnable = nWhich == SID_BEZIER_SMOOTH;
                            break;
                        case SdrPathSmoothKind::Symmetric :
                            bEnable = nWhich == SID_BEZIER_SYMMTR;
                            break;
                    }
                    rSet.Put(SfxBoolItem(nWhich, bEnable));
                }
                break;

            case SID_BEZIER_CLOSE:
                if (!pSdrView->IsOpenCloseMarkedObjectsPossible())
                {
                    rSet.DisableItem(SID_BEZIER_CLOSE);
                }
                else
                {
                    SdrObjClosedKind eClose = pSdrView->GetMarkedObjectsClosedState();
                    switch (eClose)
                    {
                        case SdrObjClosedKind::DontCare: rSet.InvalidateItem(SID_BEZIER_CLOSE); break;
                        case SdrObjClosedKind::Open    : rSet.Put(SfxBoolItem(SID_BEZIER_CLOSE,false)); break;
                        case SdrObjClosedKind::Closed  : rSet.Put(SfxBoolItem(SID_BEZIER_CLOSE,true)); break;
                        default:; //prevent warning
                    }
                }
                break;

            case SID_BEZIER_ELIMINATE_POINTS:
                rSet.Put(SfxBoolItem(SID_BEZIER_ELIMINATE_POINTS, pSdrView->IsEliminatePolyPoints()));
                break;

            default:
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
