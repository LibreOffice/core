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
#include "precompiled_sw.hxx"


#include "cmdid.h"
#include <svx/svdview.hxx>
#include <svl/srchitem.hxx>
#include <svl/eitem.hxx>
#include <svl/whiter.hxx>
#include <svx/svdopath.hxx>
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
#include "popup.hrc"
#include "shells.hrc"
#define SwBezierShell
#include <sfx2/msg.hxx>
#include "swslots.hxx"

#include <unomid.h>


SFX_IMPL_INTERFACE(SwBezierShell, SwBaseShell, SW_RES(STR_SHELLNAME_BEZIER))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_DRAW_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_BEZIER_TOOLBOX));
}

TYPEINIT1(SwBezierShell,SwBaseShell)

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SwBezierShell::SwBezierShell(SwView &_rView):
    SwBaseShell( _rView )
{
    SetName(C2S("Bezier"));
    SetHelpId(SW_BEZIERSHELL);

    SwWrtShell *pSh = &GetShell();
    SdrView*    pSdrView = pSh->GetDrawView();
    pSdrView->SetEliminatePolyPointLimitAngle(1500L);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwBezierShell::Execute(SfxRequest &rReq)
{
    SwWrtShell *pSh = &GetShell();
    SdrView*    pSdrView = pSh->GetDrawView();
    const SfxItemSet *pArgs = rReq.GetArgs();
    sal_uInt16      nSlotId = rReq.GetSlot();
    sal_Bool        bChanged = pSdrView->GetModel()->IsChanged();
    pSdrView->GetModel()->SetChanged(sal_False);
    const SfxPoolItem* pItem;
    if(pArgs)
        pArgs->GetItemState(nSlotId, sal_False, &pItem);

    switch (nSlotId)
    {
        case SID_DELETE:
        case FN_BACKSPACE:
            if (pSh->IsObjSelected())
            {
                if (pSdrView->HasMarkedPoints())
                    pSh->GetView().GetViewFrame()->GetDispatcher()->Execute(SID_BEZIER_DELETE, sal_False);
                else
                {
                    pSh->DelSelectedObj();
                    if (pSh->IsSelFrmMode())
                    {
                        pSh->LeaveSelFrmMode();
                        pSh->NoEdit();
                    }
                    GetView().AttrChangedNotify(pSh); // ggf Shellwechsel...
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
                    GetView().AttrChangedNotify(pSh); // ggf Shellwechsel...
                }
                else if ( pSh->HasSelection() || GetView().IsDrawMode() )
                {
                    GetView().LeaveDrawCreate();
                    pSh->EnterStdMode();
                    GetView().AttrChangedNotify(pSh); // ggf Shellwechsel...
                }
            }
            break;

        case SID_BEZIER_MOVE:
        case SID_BEZIER_INSERT:
            {
                GetView().GetEditWin().SetBezierMode(nSlotId);
                static sal_uInt16 __READONLY_DATA aInva[] =
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
                        pSdrView->SetMarkedSegmentsKind(SDRPATHSEGMENT_TOGGLE);
                        break;
                    }

                    case SID_BEZIER_EDGE:
                    case SID_BEZIER_SMOOTH:
                    case SID_BEZIER_SYMMTR:
                    {
                        SdrPathSmoothKind eKind = SDRPATHSMOOTH_ASYMMETRIC;

                        switch (nSlotId)
                        {
                            case SID_BEZIER_EDGE:   eKind = SDRPATHSMOOTH_ANGULAR; break;
                            case SID_BEZIER_SMOOTH: eKind = SDRPATHSMOOTH_ASYMMETRIC; break;
                            case SID_BEZIER_SYMMTR: eKind = SDRPATHSMOOTH_SYMMETRIC; break;
                        }

                        SdrPathSmoothKind eSmooth = pSdrView->GetMarkedPointsSmooth();
                        if (eKind != eSmooth)
                        {
                            pSdrView->SetMarkedPointsSmooth(eKind);

                            static sal_uInt16 __READONLY_DATA aInva[] =
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
                        SdrPathObj* pPathObj = (SdrPathObj*) rMarkList.GetMark(0)->GetMarkedSdrObj();
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
        pSdrView->GetModel()->SetChanged(sal_True);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


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
                        case SDRPATHSEGMENT_DONTCARE: rSet.InvalidateItem(SID_BEZIER_CONVERT); break;
                        case SDRPATHSEGMENT_LINE    : rSet.Put(SfxBoolItem(SID_BEZIER_CONVERT,sal_False)); break; // Button reingedrueckt = Kurve
                        case SDRPATHSEGMENT_CURVE   : rSet.Put(SfxBoolItem(SID_BEZIER_CONVERT,sal_True));  break;
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
                    sal_Bool bEnable = sal_False;
                    switch (eSmooth)
                    {
                        case SDRPATHSMOOTH_DONTCARE  :
                            break;
                        case SDRPATHSMOOTH_ANGULAR   :
                            bEnable = nWhich == SID_BEZIER_EDGE;
                            break;
                        case SDRPATHSMOOTH_ASYMMETRIC:
                            bEnable = nWhich == SID_BEZIER_SMOOTH;
                            break;
                        case SDRPATHSMOOTH_SYMMETRIC :
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
                        case SDROBJCLOSED_DONTCARE: rSet.InvalidateItem(SID_BEZIER_CLOSE); break;
                        case SDROBJCLOSED_OPEN    : rSet.Put(SfxBoolItem(SID_BEZIER_CLOSE,sal_False)); break;
                        case SDROBJCLOSED_CLOSED  : rSet.Put(SfxBoolItem(SID_BEZIER_CLOSE,sal_True)); break;
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



