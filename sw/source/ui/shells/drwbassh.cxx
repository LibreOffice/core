/*************************************************************************
 *
 *  $RCSfile: drwbassh.cxx,v $
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

#include "hintids.hxx"

#ifndef _SFXOBJFACE_HXX //autogen
#include <sfx2/objface.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVX_SRCHITEM_HXX //autogen
#include <svx/srchitem.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SVX_TRANSFRM_HXX //autogen
#include <svx/transfrm.hxx>
#endif
#ifndef _SVX_LABDLG_HXX //autogen
#include <svx/labdlg.hxx>
#endif
#ifndef _SVX_ANCHORID_HXX //autogen
#include <svx/anchorid.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif


#ifndef _FMTORNT_HXX
#include <fmtornt.hxx>
#endif
#include "cmdid.h"
#include "wrtsh.hxx"
#include "view.hxx"
#include "edtwin.hxx"
#include "viewopt.hxx"
#include "dcontact.hxx"
#include "frmfmt.hxx"
#include "wrap.hxx"
#include "drawbase.hxx"
#include "drwbassh.hxx"

#define SwDrawBaseShell
#include "itemdef.hxx"
#include "swslots.hxx"

SFX_IMPL_INTERFACE(SwDrawBaseShell, SwBaseShell, SW_RES(0))
{
}

TYPEINIT1(SwDrawBaseShell,SwBaseShell)

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SwDrawBaseShell::SwDrawBaseShell(SwView &rView):
    SwBaseShell( rView )
{
    GetShell().NoEdit(TRUE);

    SwEditWin& rWin = GetView().GetEditWin();

    rWin.SetBezierMode(SID_BEZIER_MOVE);

    if ( !rView.GetDrawFuncPtr() )
        rView.GetEditWin().StdDrawMode(SID_OBJECT_SELECT);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


__EXPORT SwDrawBaseShell::~SwDrawBaseShell()
{
    GetView().ExitDraw();
    GetShell().Edit();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwDrawBaseShell::Execute(SfxRequest &rReq)
{
    SwWrtShell *pSh = &GetShell();
    SdrView*    pSdrView = pSh->GetDrawView();
    const SfxItemSet *pArgs = rReq.GetArgs();
    USHORT      nSlotId = rReq.GetSlot();
    BOOL        bChanged = pSdrView->GetModel()->IsChanged();
    pSdrView->GetModel()->SetChanged(FALSE);
    const SfxPoolItem* pItem = 0;
    if(pArgs)
        pArgs->GetItemState(nSlotId, FALSE, &pItem);

    //Sonderfall Align per Menue
    if(pItem && nSlotId == SID_OBJECT_ALIGN)
    {
        DBG_ASSERT(PTR_CAST(SfxEnumItem, pItem),"SfxEnumItem erwartet")
        nSlotId += ((const SfxEnumItem*)pItem)->GetValue();
        nSlotId++;
    }

    BOOL bAlignPossible = pSh->IsAlignPossible();

    BOOL bTopParam = TRUE, bBottomParam = TRUE;
    BOOL bNotify = FALSE;
    BOOL bDone = FALSE;
    SfxBindings& rBind = GetView().GetViewFrame()->GetBindings();

    switch (nSlotId)
    {
        case FN_DRAW_WRAP_DLG:
        {
            if(pSdrView->HasMarkedObj())
            {
                const SfxItemSet* pArgs = rReq.GetArgs();

                if(!pArgs)
                {
                    const SdrMarkList& rMarkList = pSdrView->GetMarkList();
                    if( rMarkList.GetMark(0) != 0 )
                    {
                        SfxItemSet aSet(GetPool(),  RES_SURROUND, RES_SURROUND,
                                                    RES_ANCHOR, RES_ANCHOR,
                                                    RES_LR_SPACE, RES_UL_SPACE,
                                                    SID_HTML_MODE, SID_HTML_MODE,
                                                    FN_DRAW_WRAP_DLG, FN_DRAW_WRAP_DLG,
                                                    0);

                        const SwViewOption* pVOpt = pSh->GetViewOptions();
                        aSet.Put(SfxBoolItem(SID_HTML_MODE,
                            0 != ::GetHtmlMode(pSh->GetView().GetDocShell())));

                        aSet.Put(SfxInt16Item(FN_DRAW_WRAP_DLG, pSh->GetLayerId()));

                        pSh->GetObjAttr(aSet);
                        SwWrapDlg aDlg(GetView().GetWindow(), aSet, pSh, TRUE);

                        if (aDlg.Execute() == RET_OK)
                        {
                            const SfxPoolItem* pItem;
                            const SfxItemSet* pOutSet = aDlg.GetOutputItemSet();
                            if(SFX_ITEM_SET == pOutSet->GetItemState(FN_DRAW_WRAP_DLG, FALSE, &pItem))
                            {
                                short nLayer = ((const SfxInt16Item*)pItem)->GetValue();
                                if (nLayer == 1)
                                    pSh->SelectionToHeaven();
                                else
                                    pSh->SelectionToHell();
                            }

                            pSh->SetObjAttr(*pOutSet);
                        }
                    }
                }
            }
        }
        break;

        case SID_ATTR_TRANSFORM:
        {
            if(pSdrView->HasMarkedObj())
            {
                const SfxItemSet* pArgs = rReq.GetArgs();

                if(!pArgs)
                {
                    const SdrMarkList& rMarkList = pSdrView->GetMarkList();
                    if( rMarkList.GetMark(0) != 0 )
                    {
                        SdrObject* pObj = rMarkList.GetMark(0)->GetObj();
                        SfxTabDialog *pDlg;
                        BOOL bCaption = FALSE;

                        // Erlaubte Verankerungen:
                        USHORT nAnchor = pSh->GetAnchorId();
                        USHORT nAllowedAnchors = SVX_OBJ_AT_CNTNT|SVX_OBJ_IN_CNTNT;
                        USHORT nHtmlMode = ::GetHtmlMode(pSh->GetView().GetDocShell());
                        if( !((HTMLMODE_ON & nHtmlMode) && (0 == (nHtmlMode & HTMLMODE_SOME_ABS_POS))) )
                            nAllowedAnchors |= SVX_OBJ_PAGE;
                        if ( pSh->IsFlyInFly() )
                            nAllowedAnchors |= SVX_OBJ_AT_FLY;

                        if (pObj->GetObjIdentifier() == OBJ_CAPTION )
                            bCaption = TRUE;

                        if (bCaption)
                            pDlg = new SvxCaptionTabDialog(NULL, pSdrView, nAllowedAnchors);
                        else
                            pDlg = new SvxTransformTabDialog(NULL, NULL, pSdrView, nAllowedAnchors);

                        SfxItemSet aNewAttr(pSdrView->GetGeoAttrFromMarked());

                        const USHORT* pRange = pDlg->GetInputRanges( *aNewAttr.GetPool() );
                        SfxItemSet aSet( *aNewAttr.GetPool(), pRange );
                        aSet.Put( aNewAttr, FALSE );

                        if (bCaption)
                            pSdrView->GetAttributes( aSet );

                        aSet.Put(SfxUInt16Item(SID_ATTR_TRANSFORM_ANCHOR, nAnchor));

                        SwFrmFmt* pFrmFmt = FindFrmFmt( pObj );
                        SwFmtVertOrient aVOrient((SwFmtVertOrient&)pFrmFmt->GetAttr(RES_VERT_ORIENT));
                        USHORT nOrient = aVOrient.GetVertOrient();
                        aSet.Put(SfxUInt16Item(SID_ATTR_TRANSFORM_VERT_ORIENT, nOrient));

                        pDlg->SetInputSet( &aSet );

                        if (pDlg->Execute() == RET_OK)
                        {
                            const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                            pSh->StartAllAction();
                            pSdrView->SetGeoAttrToMarked(*pOutSet);

                            if (bCaption)
                                pSdrView->SetAttributes(*pOutSet);

                            BOOL bPosCorr =
                                SFX_ITEM_SET != pOutSet->GetItemState(
                                    SID_ATTR_TRANSFORM_POS_X, FALSE ) &&
                                SFX_ITEM_SET != pOutSet->GetItemState(
                                    SID_ATTR_TRANSFORM_POS_Y, FALSE );

                            const SfxPoolItem* pItem;
                            if(SFX_ITEM_SET == pOutSet->GetItemState(
                                SID_ATTR_TRANSFORM_ANCHOR, FALSE, &pItem))
                                pSh->ChgAnchor(((const SfxUInt16Item*)pItem)
                                    ->GetValue(), FALSE, bPosCorr );

                            if( SFX_ITEM_SET == pOutSet->GetItemState(
                                SID_ATTR_TRANSFORM_VERT_ORIENT, FALSE, &pItem))
                            {
                                aVOrient.SetVertOrient( (SwVertOrient)
                                    ((const SfxUInt16Item*)pItem)->GetValue());
                                pFrmFmt->SetAttr( aVOrient );
                            }

                            rBind.InvalidateAll(FALSE);
                            pSh->EndAllAction();
                        }
                        delete pDlg;
                    }
                }
            }
        }
        break;

        case SID_DELETE:
        case FN_BACKSPACE:
            if (pSh->IsObjSelected() && !pSdrView->IsTextEdit())
            {
                bDone = TRUE;

                if( GetView().IsDrawRotate() )
                {
                    pSh->SetDragMode( SDRDRAG_MOVE );
                    GetView().FlipDrawRotate();
                }

                pSh->SetModified();
                pSh->DelSelectedObj();

                if (rReq.IsAPI())
                {
                    // Wenn Basic-Aufruf, dann zurÅck in die Textshell, da das
                    // Basic sonst keine RÅckkehrmîglichkeit hat.
                    if (GetView().GetDrawFuncPtr())
                    {
                        GetView().GetDrawFuncPtr()->Deactivate();
                        GetView().SetDrawFuncPtr(NULL);
                    }
                    GetView().LeaveDrawCreate();    // In Selektionsmode wechseln
                }

                if (pSh->IsSelFrmMode())
                {
                    pSh->LeaveSelFrmMode();
                    pSh->NoEdit();
                }
                bNotify = TRUE;
            }
            break;

        case SID_GROUP:
            if (pSh->IsObjSelected() > 1 && pSh->IsGroupAllowed())
            {
                pSh->GroupSelection();  // Objekt gruppieren
                rBind.Invalidate(SID_UNGROUP);
            }
            break;

        case SID_UNGROUP:
            if (pSh->IsGroupSelected())
            {
                pSh->UnGroupSelection();    // Objektgruppierung aufheben
                rBind.Invalidate(SID_GROUP);
            }
            break;

        case SID_ENTER_GROUP:
            if (pSh->IsGroupSelected())
            {
                pSdrView->EnterMarkedGroup();
                rBind.InvalidateAll(FALSE);
            }
            break;

        case SID_LEAVE_GROUP:
            if (pSdrView->IsGroupEntered())
            {
                pSdrView->LeaveOneGroup();
                rBind.Invalidate(SID_ENTER_GROUP);
                rBind.Invalidate(SID_UNGROUP);
            }
            break;

        case SID_OBJECT_ALIGN_LEFT:
        case SID_OBJECT_ALIGN_CENTER:
        case SID_OBJECT_ALIGN_RIGHT:
        case SID_OBJECT_ALIGN_UP:
        case SID_OBJECT_ALIGN_MIDDLE:
        case SID_OBJECT_ALIGN_DOWN:
        {
            if ( bAlignPossible )
            {
                const SdrMarkList& rMarkList = pSdrView->GetMarkList();
                if( rMarkList.GetMarkCount() == 1 && bAlignPossible )
                {   // Objekte nicht aneinander ausrichten

                    USHORT nAnchor = pSh->GetAnchorId();
                    if (nAnchor == FLY_IN_CNTNT)
                    {
                        int nVertOrient = -1;

                        switch (nSlotId)
                        {
                            case SID_OBJECT_ALIGN_UP:
                                nVertOrient = SVX_VERT_TOP;
                                break;
                            case SID_OBJECT_ALIGN_MIDDLE:
                                nVertOrient = SVX_VERT_CENTER;
                                break;
                            case SID_OBJECT_ALIGN_DOWN:
                                nVertOrient = SVX_VERT_BOTTOM;
                                break;
                            default:
                                break;
                        }
                        if (nVertOrient != -1)
                        {
                            pSh->StartAction();
                            SdrObject* pObj = rMarkList.GetMark(0)->GetObj();
                            SwFrmFmt* pFrmFmt = FindFrmFmt( pObj );
                            SwFmtVertOrient aVOrient((SwFmtVertOrient&)pFrmFmt->GetAttr(RES_VERT_ORIENT));
                            aVOrient.SetVertOrient((SwVertOrient)nVertOrient);
                            pFrmFmt->SetAttr(aVOrient);
                            pSh->EndAction();
                        }
                        break;
                    }
                    if (nAnchor == FLY_AT_CNTNT)
                        break;  // Absatzverankerte Rahmen nicht ausrichten
                }

                pSh->StartAction();
                switch (nSlotId)
                {
                    case SID_OBJECT_ALIGN_LEFT:
                        pSdrView->AlignMarkedObjects(SDRHALIGN_LEFT, SDRVALIGN_NONE);
                        break;
                    case SID_OBJECT_ALIGN_CENTER:
                        pSdrView->AlignMarkedObjects(SDRHALIGN_CENTER, SDRVALIGN_NONE);
                        break;
                    case SID_OBJECT_ALIGN_RIGHT:
                        pSdrView->AlignMarkedObjects(SDRHALIGN_RIGHT, SDRVALIGN_NONE);
                        break;
                    case SID_OBJECT_ALIGN_UP:
                        pSdrView->AlignMarkedObjects(SDRHALIGN_NONE, SDRVALIGN_TOP);
                        break;
                    case SID_OBJECT_ALIGN_MIDDLE:
                        pSdrView->AlignMarkedObjects(SDRHALIGN_NONE, SDRVALIGN_CENTER);
                        break;
                    case SID_OBJECT_ALIGN_DOWN:
                        pSdrView->AlignMarkedObjects(SDRHALIGN_NONE, SDRVALIGN_BOTTOM);
                        break;
                }
                pSh->EndAction();
            }
        }
        break;

        case FN_FRAME_UP:
            bTopParam = FALSE;
            /* no break */
        case SID_FRAME_TO_TOP:
            pSh->SelectionToTop( bTopParam );
            break;

        case FN_FRAME_DOWN:
            bBottomParam = FALSE;
            /* no break */
        case SID_FRAME_TO_BOTTOM:
            pSh->SelectionToBottom( bBottomParam );
            break;

        default:
            DBG_ASSERT(!this, "falscher Dispatcher");
            return;
    }
    if(!bDone)
    {
        if(nSlotId >= SID_OBJECT_ALIGN_LEFT && nSlotId <= SID_OBJECT_ALIGN_DOWN)
            rBind.Invalidate(SID_ATTR_LONG_LRSPACE);
        if (pSdrView->GetModel()->IsChanged())
            pSh->SetModified();
        else if (bChanged)
            pSdrView->GetModel()->SetChanged(TRUE);
        // 40220: Nach dem Loeschen von DrawObjekten ueber die API GPF durch Selbstzerstoerung
        if(bNotify)
            GetView().AttrChangedNotify(pSh); // ggf Shellwechsel...
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwDrawBaseShell::GetState(SfxItemSet& rSet)
{
    SwWrtShell &rSh = GetShell();
    SdrView* pSdrView = rSh.GetDrawViewWithValidMarkList();
    SfxWhichIter aIter( rSet );
    USHORT nWhich = aIter.FirstWhich();
    BOOL bProtected = rSh.IsSelObjProtected(FLYPROTECT_CONTENT);

    if (!bProtected)    // Im Parent nachsehen
        bProtected |= rSh.IsSelObjProtected( (FlyProtectType)(FLYPROTECT_CONTENT|FLYPROTECT_PARENT) ) != 0;

    while( nWhich )
    {
        switch( nWhich )
        {
            case FN_DRAW_WRAP_DLG:
            case SID_ATTR_TRANSFORM:
            case SID_FRAME_TO_TOP:
            case SID_FRAME_TO_BOTTOM:
            case FN_FRAME_UP:
            case FN_FRAME_DOWN:
            case SID_DELETE:
            case FN_BACKSPACE:
                if (bProtected)
                    rSet.DisableItem( nWhich );
                break;
            case SID_GROUP:
                if ( rSh.IsObjSelected() < 2 || bProtected || !rSh.IsGroupAllowed() )
                    rSet.DisableItem( nWhich );
                break;
            case SID_UNGROUP:
                if ( !rSh.IsGroupSelected() || bProtected )
                    rSet.DisableItem( nWhich );
                break;
            case SID_ENTER_GROUP:
                if ( !rSh.IsGroupSelected() )
                    rSet.DisableItem( nWhich );
                break;
            case SID_LEAVE_GROUP:
                if ( !pSdrView->IsGroupEntered() )
                    rSet.DisableItem( nWhich );
                break;
            case SID_OBJECT_ALIGN_LEFT:
            case SID_OBJECT_ALIGN_CENTER:
            case SID_OBJECT_ALIGN_RIGHT:
            case SID_OBJECT_ALIGN_UP:
            case SID_OBJECT_ALIGN_MIDDLE:
            case SID_OBJECT_ALIGN_DOWN:
            case SID_OBJECT_ALIGN:
                if ( !rSh.IsAlignPossible() || bProtected )
                    rSet.DisableItem( nWhich );
                else
                {
                    rSet.Put(SfxAllEnumItem(nWhich, USHRT_MAX));
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


BOOL SwDrawBaseShell::Disable(SfxItemSet& rSet, USHORT nWhich)
{
    BOOL bDisable = GetShell().IsSelObjProtected(FLYPROTECT_CONTENT);

    if (bDisable)
    {
        if (nWhich)
            rSet.DisableItem( nWhich );
        else
        {
            SfxWhichIter aIter( rSet );
            nWhich = aIter.FirstWhich();
            while (nWhich)
            {
                rSet.DisableItem( nWhich );
                nWhich = aIter.NextWhich();
            }
        }
    }

    return bDisable;
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.49  2000/09/18 16:06:03  willem.vandorp
    OpenOffice header added.

    Revision 1.48  2000/09/07 15:59:29  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.47  2000/08/10 16:34:18  jp
    Bug #77310#: don't call a method with a zero pointer

    Revision 1.46  2000/05/29 16:40:53  jp
    Bug #69159#: delete selection reset the rotate mode

    Revision 1.45  2000/05/26 07:21:32  os
    old SW Basic API Slots removed

    Revision 1.44  2000/05/10 11:53:01  os
    Basic API removed

    Revision 1.43  2000/02/11 14:57:25  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.42  1999/06/21 13:20:24  JP
    Interface changes: SdrView::GetAttributes


      Rev 1.41   21 Jun 1999 15:20:24   JP
   Interface changes: SdrView::GetAttributes

      Rev 1.40   07 Oct 1998 10:58:48   JP
   Bug #57153#: in allen GetStates einer DrawShell die MarkListe aktualisieren

      Rev 1.39   06 Jul 1998 18:10:14   OM
   Images zum gruppieren

      Rev 1.38   06 Jul 1998 16:09:54   OM
   #52065# Gruppe verlassen bei Beendigung der DrawShell

      Rev 1.37   03 Jul 1998 16:28:48   OM
   #51420# Nur erlaubte Verankerungsarten im PositionsDialog

      Rev 1.36   01 Jul 1998 12:13:44   OM
   #51323# Zeichengebundene Objekte nicht gruppieren

      Rev 1.35   16 Jun 1998 13:03:32   OM
   Gruppenobjekte per Ctrl selektieren

      Rev 1.34   12 Jun 1998 16:08:56   OM
   Gruppierung betreten/verlassen

      Rev 1.33   12 Jun 1998 13:34:02   OM
   Wieder SID_OBJECT_SELECT statt SID_DRAW_SELECT

      Rev 1.32   11 Jun 1998 16:13:14   OM
   Gruppierungsmenue

      Rev 1.31   10 Jun 1998 11:23:18   OM
   Hell/Heaven nicht in FormShell

      Rev 1.30   09 Jun 1998 15:36:00   OM
   VC-Controls entfernt

      Rev 1.29   22 May 1998 15:19:06   OS
   wird ein DrawObject geloescht, darf nicht mehr auf this zugegriffen werden

      Rev 1.28   15 May 1998 16:22:02   OM
   #49467# GeschÅtzte Rahmen nicht bearbeiten

      Rev 1.27   15 Apr 1998 15:32:24   OM
   #49467 Objekte innerhalb von geschuetzten Rahmen duerfen nicht veraendert werden

      Rev 1.26   29 Nov 1997 15:51:24   MA
   includes

      Rev 1.25   24 Nov 1997 09:46:58   MA
   includes

      Rev 1.24   20 Oct 1997 12:45:06   OM
   Kein Legenden-Dialog bei URL-Buttons

      Rev 1.23   20 Oct 1997 12:15:18   OM
   #44751# Umankern auch bei Legendenobjekten

      Rev 1.22   15 Oct 1997 13:00:06   AMA
   Fix #44651#: Umankern mit gleichzeitiger Positionsaenderung

      Rev 1.21   05 Sep 1997 12:04:22   MH
   chg: header

      Rev 1.20   02 Sep 1997 13:23:16   OS
   includes

      Rev 1.19   16 Aug 1997 12:59:54   OS
   include

      Rev 1.18   05 Aug 1997 16:23:12   TJ
   include svx/srchitem.hxx

      Rev 1.17   01 Aug 1997 11:47:18   MH
   chg: header

      Rev 1.16   10 Jul 1997 17:27:44   MA
   #41589# nicht die dontcare verlieren, damit auch GPF umpopeln

      Rev 1.15   08 Jul 1997 12:16:26   OM
   Draw-Selektionsmodi aufgeraeumt

      Rev 1.14   08 Jul 1997 12:07:10   OM
   Draw-Selektionsmodi aufgeraeumt

      Rev 1.13   25 Jun 1997 13:34:18   OM
   #40966# DrawBaseShell-Ptr wieder eingefuehrt

      Rev 1.12   25 Jun 1997 09:10:18   MA
   #40965#, SubShell abmelden

      Rev 1.11   17 Jun 1997 15:52:52   MA
   DrawTxtShell nicht von BaseShell ableiten + Opts

      Rev 1.10   16 Jun 1997 15:31:56   OM
   GPF behoben: Backspace wie Delete behandeln

      Rev 1.9   27 May 1997 16:39:24   OM
   Zeichengebundene Zeichenobjekte korrekt ausrichten

      Rev 1.8   27 May 1997 12:52:46   OS
   AttrChangedNotify erst am Ende des ::Execute rufen #40220#

      Rev 1.7   30 Apr 1997 11:34:50   MA
   #39469# erstmal GPF umpopelt

      Rev 1.6   07 Apr 1997 17:43:34   MH
   chg: header

      Rev 1.5   02 Apr 1997 21:42:20   MH
   chg: header SfxDialoge

      Rev 1.4   19 Mar 1997 11:54:26   OM
   #37302# Shellwechsel nach Basic-Delete ausloesen

      Rev 1.3   27 Jan 1997 16:31:04   OS
   HtmlMode entfernt

      Rev 1.2   23 Jan 1997 10:45:52   OM
   Neue Shells: Control- und DrawBaseShell

      Rev 1.1   22 Jan 1997 14:40:32   OM
   unnoetige Includes entfernt

      Rev 1.0   22 Jan 1997 11:23:24   OM
   Initial revision.

------------------------------------------------------------------------*/



