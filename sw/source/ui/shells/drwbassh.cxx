/*************************************************************************
 *
 *  $RCSfile: drwbassh.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-14 16:16:26 $
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


#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#include <helpid.h>
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
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
//CHINA001 #ifndef _SVX_TRANSFRM_HXX //autogen
//CHINA001 #include <svx/transfrm.hxx>
//CHINA001 #endif
//#ifndef _SVX_LABDLG_HXX //autogen
//#include <svx/labdlg.hxx>
//#endif    delete by CHINA001
#ifndef _SVXSWFRAMEVALIDATION_HXX
#include <svx/swframevalidation.hxx>
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

#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _FMTORNT_HXX
#include <fmtornt.hxx>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _WVIEW_HXX
#include <wview.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _WRAP_HXX
#include <wrap.hxx>
#endif
#ifndef _DRAWBASE_HXX
#include <drawbase.hxx>
#endif
#ifndef _DRWBASSH_HXX
#include <drwbassh.hxx>
#endif
#ifndef _SWDTFLVR_HXX
#include <swdtflvr.hxx>
#endif
//CHINA001 #ifndef _SVX_DLG_NAME_HXX
//CHINA001 #include <svx/dlgname.hxx>
//CHINA001 #endif
#ifndef _SVDOGRP_HXX
#include <svx/svdogrp.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif

#include <doc.hxx>
#include <shells.hrc>
#define SwDrawBaseShell
#ifndef _ITEMDEF_HXX
#include <itemdef.hxx>
#endif
#ifndef _SWSLOTS_HXX
#include <swslots.hxx>
#endif
//add header of cui CHINA001
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include "swabstdlg.hxx" //CHINA001
#include "dialog.hrc" //CHINA001

#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATION_HPP_
#include <com/sun/star/text/HoriOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_VERTORIENTATION_HPP_
#include <com/sun/star/text/VertOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_RELORIENTATION_HPP_
#include <com/sun/star/text/RelOrientation.hpp>
#endif

using namespace ::com::sun::star::text;

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

    SwTransferable::CreateSelection( GetShell() );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SwDrawBaseShell::~SwDrawBaseShell()
{
    GetView().ExitDraw();
    GetShell().Edit();
    SwTransferable::ClearSelection( GetShell() );
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
            if(pSdrView->AreObjectsMarked())
            {
                const SfxItemSet* pArgs = rReq.GetArgs();

                if(!pArgs)
                {
                    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
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
                        //CHINA001 SwWrapDlg aDlg(GetView().GetWindow(), aSet, pSh, TRUE);
                        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();//CHINA001
                        DBG_ASSERT(pFact, "SwAbstractDialogFactory fail!");//CHINA001

                        AbstractSfxSingleTabDialog* pDlg = pFact->CreateSwWrapDlg( GetView().GetWindow(), aSet, pSh, TRUE,ResId( RC_DLG_SWWRAPDLG ));
                        DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001

                        if (pDlg->Execute() == RET_OK) //CHINA001 if (aDlg.Execute() == RET_OK)
                        {
                            const SfxPoolItem* pItem;
                            const SfxItemSet* pOutSet = pDlg->GetOutputItemSet(); //CHINA001 const SfxItemSet* pOutSet = aDlg.GetOutputItemSet();
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
                    delete pDlg; //CHINA001
                    }
                }
            }
        }
        break;

        case SID_ATTR_TRANSFORM:
        {
            if(pSdrView->AreObjectsMarked())
            {
                const SfxItemSet* pArgs = rReq.GetArgs();

                if(!pArgs)
                {
                    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
                    if( rMarkList.GetMark(0) != 0 )
                    {
                        SdrObject* pObj = rMarkList.GetMark(0)->GetObj();
                        //SfxTabDialog *pDlg;       delete for cui CHINA001
                        SfxAbstractTabDialog *pDlg=NULL; //add CHINA001
                        BOOL bCaption = FALSE;

                        // Erlaubte Verankerungen:
                        short nAnchor = pSh->GetAnchorId();
                        USHORT nAllowedAnchors = SVX_OBJ_AT_CNTNT|SVX_OBJ_IN_CNTNT;
                        USHORT nHtmlMode = ::GetHtmlMode(pSh->GetView().GetDocShell());

                        if( !((HTMLMODE_ON & nHtmlMode) && (0 == (nHtmlMode & HTMLMODE_SOME_ABS_POS))) )
                            nAllowedAnchors |= SVX_OBJ_PAGE;
                        if ( pSh->IsFlyInFly() )
                            nAllowedAnchors |= SVX_OBJ_AT_FLY;

                        if (pObj->GetObjIdentifier() == OBJ_CAPTION )
                            bCaption = TRUE;

                        if (bCaption)
                            //pDlg = new SvxCaptionTabDialog(NULL, pSdrView, nAllowedAnchors);
                        {//change for cui CHINA001
                            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                            if ( pFact )
                            {
                                AbstractSvxCaptionDialog* pCaptionDlg =
                                        pFact->CreateCaptionDialog( NULL, pSdrView, ResId( RID_SVXDLG_CAPTION ), nAllowedAnchors );
                                pCaptionDlg->SetValidateFramePosLink( LINK(this, SwDrawBaseShell, ValidatePosition) );
                                pDlg = pCaptionDlg;
                                DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
                            }
                        }
                        else
                        {
                            //CHINA001 SfxTabDialog *pDlg = new SvxTransformTabDialog(NULL, NULL, pSdrView, nAllowedAnchors);
                            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                            if ( pFact )
                            {

                                AbstractSvxTransformTabDialog* pTransform =
                                            pFact->CreateSvxTransformTabDialog( NULL, NULL, pSdrView,ResId( RID_SVXDLG_TRANSFORM ), nAllowedAnchors );
                                pTransform->SetValidateFramePosLink( LINK(this, SwDrawBaseShell, ValidatePosition) );
                                pDlg = pTransform;
                                DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
                            }
                        }
                        SfxItemSet aNewAttr(pSdrView->GetGeoAttrFromMarked());

                        const USHORT* pRange = pDlg->GetInputRanges( *aNewAttr.GetPool() );
                        SfxItemSet aSet( *aNewAttr.GetPool(), pRange );
                        SwView& rView = GetView();
                        FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebView, &rView));
                        SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, eMetric));

                        aSet.Put( aNewAttr, FALSE );

                        if (bCaption)
                            pSdrView->GetAttributes( aSet );

                        aSet.Put(SfxInt16Item(SID_ATTR_TRANSFORM_ANCHOR, nAnchor));
                        BOOL bRTL;
                        aSet.Put(SfxBoolItem(SID_ATTR_TRANSFORM_IN_VERTICAL_TEXT, pSh->IsFrmVertical(TRUE, bRTL)));
                        aSet.Put(SfxBoolItem(SID_ATTR_TRANSFORM_IN_RTL_TEXT, bRTL));

                        SwFrmFmt* pFrmFmt = FindFrmFmt( pObj );

                        aSet.Put( pFrmFmt->GetAttr(RES_FOLLOW_TEXT_FLOW) );

                        SwFmtVertOrient aVOrient((const SwFmtVertOrient&)pFrmFmt->GetAttr(RES_VERT_ORIENT));
                        aSet.Put(SfxInt16Item(SID_ATTR_TRANSFORM_VERT_ORIENT, aVOrient.GetVertOrient()));
                        aSet.Put(SfxInt16Item(SID_ATTR_TRANSFORM_VERT_RELATION, aVOrient.GetRelationOrient() ));
                        aSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_VERT_POSITION, aVOrient.GetPos()));

                        SwFmtHoriOrient aHOrient((const SwFmtHoriOrient&)pFrmFmt->GetAttr(RES_HORI_ORIENT));
                        aSet.Put(SfxInt16Item(SID_ATTR_TRANSFORM_HORI_ORIENT, aHOrient.GetHoriOrient()));
                        aSet.Put(SfxInt16Item(SID_ATTR_TRANSFORM_HORI_RELATION, aHOrient.GetRelationOrient() ));
                        aSet.Put(SfxBoolItem(SID_ATTR_TRANSFORM_HORI_MIRROR, aHOrient.IsPosToggle()));
                        aSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_HORI_POSITION, aHOrient.GetPos()));

                        aSet.Put(SfxUInt16Item(SID_HTML_MODE, nHtmlMode));

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

                            SfxItemSet aSet(GetPool(), RES_FRMATR_BEGIN, RES_FRMATR_END - 1);

                            bool bSingleSelection = rMarkList.GetMarkCount() == 1;

                            const SfxPoolItem* pItem;
                            if(SFX_ITEM_SET == pOutSet->GetItemState(
                                SID_ATTR_TRANSFORM_ANCHOR, FALSE, &pItem))
                            {
                                if(!bSingleSelection)
                                    pSh->ChgAnchor(((const SfxInt16Item*)pItem)
                                            ->GetValue(), FALSE, bPosCorr );
                                else
                                {
                                    SwFmtAnchor aAnchor(pFrmFmt->GetAnchor());
                                    aAnchor.SetType((RndStdIds)((const SfxInt16Item*)pItem)->GetValue());
                                    aSet.Put( aAnchor );
                                }
                            }
                            const SfxPoolItem* pHoriOrient = 0;
                            const SfxPoolItem* pHoriRelation = 0;
                            const SfxPoolItem* pHoriPosition = 0;
                            const SfxPoolItem* pHoriMirror = 0;
                            pOutSet->GetItemState(SID_ATTR_TRANSFORM_HORI_ORIENT, FALSE, &pHoriOrient);
                            pOutSet->GetItemState(SID_ATTR_TRANSFORM_HORI_RELATION, FALSE, &pHoriRelation);
                            pOutSet->GetItemState(SID_ATTR_TRANSFORM_HORI_POSITION, FALSE, &pHoriPosition);
                            pOutSet->GetItemState(SID_ATTR_TRANSFORM_HORI_MIRROR, FALSE, &pHoriMirror);
                            if(pHoriOrient || pHoriRelation || pHoriPosition || pHoriMirror)
                            {
                                if(pHoriOrient)
                                    aHOrient.SetHoriOrient( (SwHoriOrient)
                                          static_cast<const SfxInt16Item*>(pHoriOrient)->GetValue());
                                if(pHoriRelation)
                                    aHOrient.SetRelationOrient( (SwRelationOrient)
                                              static_cast<const SfxInt16Item*>(pHoriRelation)->GetValue());
                                if(pHoriPosition)
                                    aHOrient.SetPos( static_cast<const SfxInt32Item*>(pHoriPosition)->GetValue());
                                if(pHoriMirror)
                                    aHOrient.SetPosToggle( static_cast<const SfxBoolItem*>(pHoriMirror)->GetValue());
                                aSet.Put(aHOrient);
                            }

                            const SfxPoolItem* pVertOrient = 0;
                            const SfxPoolItem* pVertRelation = 0;
                            const SfxPoolItem* pVertPosition = 0;
                            pOutSet->GetItemState(SID_ATTR_TRANSFORM_VERT_ORIENT, FALSE, &pVertOrient);
                            pOutSet->GetItemState(SID_ATTR_TRANSFORM_VERT_RELATION, FALSE, &pVertRelation);
                            pOutSet->GetItemState(SID_ATTR_TRANSFORM_VERT_POSITION, FALSE, &pVertPosition);
                            if(pVertOrient || pVertRelation || pVertPosition )
                            {
                                if(pVertOrient)
                                    aVOrient.SetVertOrient( (SwVertOrient)
                                        static_cast<const SfxInt16Item*>(pVertOrient)->GetValue());
                                if(pVertRelation)
                                    aVOrient.SetRelationOrient( (SwRelationOrient)
                                        static_cast<const SfxInt16Item*>(pVertRelation)->GetValue());
                                if(pVertPosition)
                                    aVOrient.SetPos( static_cast<const SfxInt32Item*>(pVertPosition)->GetValue());
                                aSet.Put( aVOrient );
                            }
                            const SfxPoolItem* pFollowItem = 0;
                            pOutSet->GetItemState(RES_FOLLOW_TEXT_FLOW, FALSE, &pFollowItem);
                            if(pFollowItem)
                                aSet.Put(*pFollowItem);

                            if(aSet.Count())
                                pSh->SetDrawingAttr(aSet);


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

                if (rReq.IsAPI() ||
                    SID_OBJECT_SELECT == GetView().GetEditWin().GetDrawMode() )
                {
                    // Wenn Basic-Aufruf, dann zurueck in die Textshell, da das
                    // Basic sonst keine Rueckkehrmoeglichkeit hat.
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
                    // #105852# FME
//                   pSh->NoEdit();
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
                const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
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
        case FN_NAME_GROUP:
        {
            bDone = TRUE;
            const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
            DBG_ASSERT(rMarkList.GetMarkCount() == 1, "Exactly one object has to be selected" )
            SdrObject* pObj = rMarkList.GetMark(0)->GetObj();
            ULONG nMarkCount = rMarkList.GetMarkCount();
            String sName;
            String sDesc(SW_RES( STR_NAME_GROUP_LABEL ) );
            DBG_ASSERT(pObj->ISA(SdrObjGroup),
                "Object is not a group, graphic or OLE shape")
            sName = pObj->GetName();
            //CHINA001 SvxNameDialog* pDlg = new SvxNameDialog( NULL, sName, sDesc );
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
            AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( NULL, sName, sDesc, ResId(RID_SVXDLG_NAME) );
            DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
            pDlg->SetText(SW_RESSTR(STR_NAME_GROUP_DIALOG));
            // #100286# -------------
            pDlg->SetEditHelpId( HID_FORMAT_NAME_OBJECT_NAME );
            pDlg->SetCheckNameHdl(LINK(this, SwDrawBaseShell, CheckGroupShapeNameHdl));
            if( pDlg->Execute() == RET_OK )
            {
                pDlg->GetName( sName );
                pObj->SetName(sName);
                pSh->SetModified();
            }
            delete pDlg;
        }
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
/* -----------------------------27.02.2002 15:27------------------------------
    Checks whether a given name is allowed for a group shape
 ---------------------------------------------------------------------------*/
IMPL_LINK( SwDrawBaseShell, CheckGroupShapeNameHdl, AbstractSvxNameDialog*, pNameDialog )
{
    SwWrtShell          &rSh = GetShell();
    SdrView *pSdrView = rSh.GetDrawView();
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    DBG_ASSERT(rMarkList.GetMarkCount() == 1, "wrong draw selection")
    SdrObject* pObj = rMarkList.GetMark(0)->GetObj();
    ULONG nMarkCount = rMarkList.GetMarkCount();
    const String sCurrentName = pObj->GetName();
    String sNewName;
    pNameDialog->GetName(sNewName);
    long nRet = 0;
    if(!sNewName.Len() || sCurrentName == sNewName)
        nRet = 1;
    else
    {
        nRet = 1;
        SdrModel* pModel = rSh.GetDoc()->GetDrawModel();
        SdrPage* pPage = pModel->GetPage(0);
        sal_uInt32 nCount = pPage->GetObjCount();
        for( sal_uInt32 i=0; i< nCount; i++ )
        {
            SdrObject* pTemp = pPage->GetObj(i);
            if(pObj != pTemp && pTemp->ISA(SdrObjGroup) && pTemp->GetName() == sNewName)
            {
                nRet = 0;
                break;
            }
        }
    }
    return nRet;
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
                if( bProtected || !rSh.IsObjSelected() )
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
                    SfxAllEnumItem aEnumItem(nWhich, USHRT_MAX);
                    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
                    //if only one object is selected it can only be vertically
                    // aligned because it is character bound
                    if( rMarkList.GetMarkCount() == 1 )
                    {
                        aEnumItem.DisableValue(SID_OBJECT_ALIGN_LEFT);
                        aEnumItem.DisableValue(SID_OBJECT_ALIGN_CENTER);
                        aEnumItem.DisableValue(SID_OBJECT_ALIGN_RIGHT);
                    }
                    rSet.Put(aEnumItem);
                }
                break;
            case FN_NAME_GROUP :
            {
                BOOL bDisable = TRUE;
                const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
                if( rMarkList.GetMarkCount() == 1 )
                {
                    SdrObject* pObj = rMarkList.GetMark(0)->GetObj();
                    if(pObj->ISA(SdrObjGroup))
                        bDisable = FALSE;
                }
                if(bDisable)
                    rSet.DisableItem( nWhich );
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

/*-- 09.03.2004 13:15:03---------------------------------------------------
    Validate of drawing positions
  -----------------------------------------------------------------------*/
IMPL_LINK(SwDrawBaseShell, ValidatePosition, SvxSwFrameValidation*, pValidation )
{
    SwWrtShell *pSh = &GetShell();
    pValidation->nMinHeight = MINFLY;
    pValidation->nMinWidth =  MINFLY;

    SwRect aBoundRect;

    // OD 18.09.2003 #i18732# - adjustment for allowing vertical position
    //      aligned to page for fly frame anchored to paragraph or to character.
    const RndStdIds eAnchorType = static_cast<RndStdIds >(pValidation->nAnchorType);
    const SwPosition* pCntntPos = 0;
    SdrView*  pSdrView = pSh->GetDrawView();
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetObj();
        SwFrmFmt* pFrmFmt = FindFrmFmt( pObj );
        pCntntPos = pFrmFmt->GetAnchor().GetCntntAnchor();
    }

    pSh->CalcBoundRect( aBoundRect, eAnchorType,
                           static_cast<SwRelationOrient>(pValidation->nHRelOrient),
                           static_cast<SwRelationOrient>(pValidation->nVRelOrient),
                           pCntntPos,
                           pValidation->bFollowTextFlow,
                           pValidation->bMirror, NULL, &pValidation->aPercentSize);

    BOOL bRTL;
    BOOL bIsInVertical = pSh->IsFrmVertical(TRUE, bRTL);
    if(bIsInVertical)
    {
        Point aPos(aBoundRect.Pos());
        long nTmp = aPos.X();
        aPos.X() = aPos.Y();
        aPos.Y() = nTmp;
        Size aSize(aBoundRect.SSize());
        nTmp = aSize.Width();
        aSize.Width() = aSize.Height();
        aSize.Height() = nTmp;
        aBoundRect.Chg( aPos, aSize );
        //exchange width/height to enable correct values
        nTmp = pValidation->nWidth;
        pValidation->nWidth = pValidation->nHeight;
        pValidation->nHeight = nTmp;
    }
    if ( eAnchorType == FLY_PAGE || eAnchorType == FLY_AT_FLY )
    {
        // MinimalPosition
        pValidation->nMinHPos = aBoundRect.Left();
        pValidation->nMinVPos = aBoundRect.Top();
        SwTwips nH = pValidation->nHPos;
        SwTwips nV = pValidation->nVPos;

        if (pValidation->nHPos + pValidation->nWidth > aBoundRect.Right())
        {
            if (pValidation->nHoriOrient == HoriOrientation::NONE)
            {
                pValidation->nHPos -= ((pValidation->nHPos + pValidation->nWidth) - aBoundRect.Right());
                nH = pValidation->nHPos;
            }
            else
                pValidation->nWidth = aBoundRect.Right() - pValidation->nHPos;
        }

        if (pValidation->nHPos + pValidation->nWidth > aBoundRect.Right())
            pValidation->nWidth = aBoundRect.Right() - pValidation->nHPos;

        if (pValidation->nVPos + pValidation->nHeight > aBoundRect.Bottom())
        {
            if (pValidation->nVertOrient == VertOrientation::NONE)
            {
                pValidation->nVPos -= ((pValidation->nVPos + pValidation->nHeight) - aBoundRect.Bottom());
                nV = pValidation->nVPos;
            }
            else
                pValidation->nHeight = aBoundRect.Bottom() - pValidation->nVPos;
        }

        if (pValidation->nVPos + pValidation->nHeight > aBoundRect.Bottom())
            pValidation->nHeight = aBoundRect.Bottom() - pValidation->nVPos;

        if ( pValidation->nVertOrient != VertOrientation::NONE )
            nV = aBoundRect.Top();

        if ( pValidation->nHoriOrient != HoriOrientation::NONE )
            nH = aBoundRect.Left();

        pValidation->nMaxHPos   = aBoundRect.Right()  - pValidation->nWidth;
        pValidation->nMaxHeight = aBoundRect.Bottom() - nV;

        pValidation->nMaxVPos   = aBoundRect.Bottom() - pValidation->nHeight;
        pValidation->nMaxWidth  = aBoundRect.Right()  - nH;
    }
    else if ( eAnchorType == FLY_AT_CNTNT || eAnchorType == FLY_AUTO_CNTNT )
    {
        if (pValidation->nHPos + pValidation->nWidth > aBoundRect.Right())
        {
            if (pValidation->nHoriOrient == HoriOrientation::NONE)
            {
                pValidation->nHPos -= ((pValidation->nHPos + pValidation->nWidth) - aBoundRect.Right());
            }
            else
                pValidation->nWidth = aBoundRect.Right() - pValidation->nHPos;
        }

        // OD 29.09.2003 #i17567#, #i18732# - consider following the text flow
        // and alignment at page areas.
        const bool bMaxVPosAtBottom = !pValidation->bFollowTextFlow ||
                                      pValidation->nVRelOrient == RelOrientation::PAGE_FRAME ||
                                      pValidation->nVRelOrient == RelOrientation::PAGE_PRINT_AREA;
        {
            SwTwips nTmpMaxVPos = ( bMaxVPosAtBottom
                                    ? aBoundRect.Bottom()
                                    : aBoundRect.Height() ) -
                                  pValidation->nHeight;
            if ( pValidation->nVPos > nTmpMaxVPos )
            {
                if (pValidation->nVertOrient == VertOrientation::NONE)
                {
                    pValidation->nVPos = nTmpMaxVPos;
                }
                else
                {
                    pValidation->nHeight = ( bMaxVPosAtBottom
                                     ? aBoundRect.Bottom()
                                     : aBoundRect.Height() ) - pValidation->nVPos;
                }
            }
        }

        pValidation->nMinHPos  = aBoundRect.Left();
        pValidation->nMaxHPos  = aBoundRect.Right() - pValidation->nWidth;

        pValidation->nMinVPos  = aBoundRect.Top();
        // OD 26.09.2003 #i17567#, #i18732# - determine maximum vertical position
        if ( bMaxVPosAtBottom )
        {
            pValidation->nMaxVPos  = aBoundRect.Bottom() - pValidation->nHeight;
        }
        else
        {
            pValidation->nMaxVPos  = aBoundRect.Height() - pValidation->nHeight;
        }

        // Maximale Breite Hoehe
        const SwTwips nH = ( pValidation->nHoriOrient != HoriOrientation::NONE )
                           ? aBoundRect.Left()
                           : pValidation->nHPos;
        const SwTwips nV = ( pValidation->nVertOrient != VertOrientation::NONE )
                           ? aBoundRect.Top()
                           : pValidation->nVPos;
        pValidation->nMaxHeight  = pValidation->nMaxVPos + pValidation->nHeight - nV;
        pValidation->nMaxWidth   = pValidation->nMaxHPos + pValidation->nWidth - nH;
    }
    else if ( eAnchorType == FLY_IN_CNTNT )
    {
        pValidation->nMinHPos = 0;
        pValidation->nMaxHPos = 0;

        pValidation->nMaxHeight = aBoundRect.Height();
        pValidation->nMaxWidth  = aBoundRect.Width();

        pValidation->nMaxVPos   = aBoundRect.Height();
        pValidation->nMinVPos   = -aBoundRect.Height() + pValidation->nHeight;
        if (pValidation->nMaxVPos < pValidation->nMinVPos)
        {
            pValidation->nMinVPos = pValidation->nMaxVPos;
            pValidation->nMaxVPos = -aBoundRect.Height();
        }
    }
    if(bIsInVertical)
    {
        //restore width/height exchange
        long nTmp = pValidation->nWidth;
        pValidation->nWidth = pValidation->nHeight;
        pValidation->nHeight = nTmp;
    }

    if (pValidation->nMaxWidth < pValidation->nWidth)
        pValidation->nWidth = pValidation->nMaxWidth;
    if (pValidation->nMaxHeight < pValidation->nHeight)
        pValidation->nHeight = pValidation->nMaxHeight;
    return 0;
}




