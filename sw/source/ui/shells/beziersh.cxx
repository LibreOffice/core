/*************************************************************************
 *
 *  $RCSfile: beziersh.cxx,v $
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

#include "cmdid.h"

#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _SVX_SRCHITEM_HXX //autogen
#include <svx/srchitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SVDOPATH_HXX //autogen
#include <svx/svdopath.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXOBJFACE_HXX //autogen
#include <sfx2/objface.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif

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
#include "itemdef.hxx"
#include "swslots.hxx"

#define C2S(cChar) UniString::CreateFromAscii(cChar)
SFX_IMPL_INTERFACE(SwBezierShell, SwBezierShell, SW_RES(STR_SHELLNAME_BEZIER))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_DRAW_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_BEZIER_TOOLBOX));
    SFX_OBJECTMENU_REGISTRATION(SID_OBJECTMENU0, SW_RES(MN_OBJECTMENU_DRAW));
}

TYPEINIT1(SwBezierShell,SwBaseShell)

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SwBezierShell::SwBezierShell(SwView &rView):
    SwBaseShell( rView )
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
    USHORT      nSlotId = rReq.GetSlot();
    BOOL        bChanged = pSdrView->GetModel()->IsChanged();
    pSdrView->GetModel()->SetChanged(FALSE);
    const SfxPoolItem* pItem;
    if(pArgs)
        pArgs->GetItemState(nSlotId, FALSE, &pItem);

    switch (nSlotId)
    {
        case SID_DELETE:
        case FN_BACKSPACE:
            if (pSh->IsObjSelected())
            {
                if (pSdrView->HasMarkedPoints())
                    pSh->GetView().GetViewFrame()->GetDispatcher()->Execute(SID_BEZIER_DELETE, FALSE);
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
                static USHORT __READONLY_DATA aInva[] =
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
            const SdrMarkList& rMarkList = pSdrView->GetMarkList();

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
                        SdrPathSmoothKind eKind;

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

                            static USHORT __READONLY_DATA aInva[] =
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
                        SdrPathObj* pPathObj = (SdrPathObj*) rMarkList.GetMark(0)->GetObj();
                        pSdrView->UnmarkAllPoints();
                        Size aDist(GetView().GetEditWin().PixelToLogic(Size(8,8)));
                        pPathObj->ToggleClosed(aDist.Width());
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
        pSdrView->GetModel()->SetChanged(TRUE);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwBezierShell::GetState(SfxItemSet &rSet)
{
    SdrView* pSdrView = GetShell().GetDrawView();

    SfxWhichIter aIter( rSet );
    USHORT nWhich = aIter.FirstWhich();

    while( nWhich )
    {
        switch( nWhich )
        {
            case SID_BEZIER_MOVE:
            case SID_BEZIER_INSERT:
            {
                USHORT nEditMode = GetView().GetEditWin().GetBezierMode();

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
                        case SDRPATHSEGMENT_LINE    : rSet.Put(SfxBoolItem(SID_BEZIER_CONVERT,FALSE)); break; // Button reingedrueckt = Kurve
                        case SDRPATHSEGMENT_CURVE   : rSet.Put(SfxBoolItem(SID_BEZIER_CONVERT,TRUE));  break;
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
                    BOOL bEnable = FALSE;
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
                        case SDROBJCLOSED_OPEN    : rSet.Put(SfxBoolItem(SID_BEZIER_CLOSE,FALSE)); break;
                        case SDROBJCLOSED_CLOSED  : rSet.Put(SfxBoolItem(SID_BEZIER_CLOSE,TRUE)); break;
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

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.21  2000/09/18 16:06:03  willem.vandorp
    OpenOffice header added.

    Revision 1.20  2000/09/07 15:59:28  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.19  2000/05/26 07:21:32  os
    old SW Basic API Slots removed

    Revision 1.18  2000/05/10 11:53:01  os
    Basic API removed

    Revision 1.17  2000/04/18 14:58:23  os
    UNICODE

    Revision 1.16  1997/11/29 14:51:26  MA
    includes


      Rev 1.15   29 Nov 1997 15:51:26   MA
   includes

      Rev 1.14   24 Nov 1997 09:46:54   MA
   includes

      Rev 1.13   03 Nov 1997 13:55:46   MA
   precomp entfernt

      Rev 1.12   05 Sep 1997 12:02:00   MH
   chg: header

      Rev 1.11   05 Aug 1997 16:18:42   TJ
   include svx/srchitem.hxx

      Rev 1.10   01 Aug 1997 11:45:44   MH
   chg: header

      Rev 1.9   08 Jul 1997 12:07:10   OM
   Draw-Selektionsmodi aufgeraeumt

      Rev 1.8   16 Jun 1997 15:30:24   OM
   GPF behoben: Backspace wie Delete behandeln

      Rev 1.7   07 Apr 1997 17:50:00   MH
   chg: header

      Rev 1.6   21 Feb 1997 13:25:14   MA
   #35942# Hack: CheckUnboundObjects

      Rev 1.5   23 Jan 1997 16:07:44   OM
   Aufgeraeumt

      Rev 1.4   22 Jan 1997 11:20:48   OM
   Neue Shells: DrawBaseShell und DrawControlShell

      Rev 1.3   16 Jan 1997 14:46:24   OM
   Zweistufiges Escape

      Rev 1.2   14 Jan 1997 15:37:38   OM
   Statusmethode fuer Bezier-Fkts

      Rev 1.1   13 Jan 1997 15:56:10   OM
   Bezier-Punkte loeschen

      Rev 1.0   08 Jan 1997 12:46:44   OM
   Initial revision.

------------------------------------------------------------------------*/


