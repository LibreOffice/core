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

#include <hintids.hxx>
#include <helpid.h>
#include <swtypes.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>
#include <svl/aeitem.hxx>
#include <svx/svdview.hxx>
#include <vcl/msgbox.hxx>
#include <svl/srchitem.hxx>
#include <svl/whiter.hxx>
#include <svx/swframevalidation.hxx>
#include <svx/anchorid.hxx>
#include <sfx2/htmlmode.hxx>
#include <drawdoc.hxx>
#include <uitool.hxx>
#include <fmtornt.hxx>
#include <cmdid.h>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <wview.hxx>
#include <edtwin.hxx>
#include <viewopt.hxx>
#include <dcontact.hxx>
#include <frmfmt.hxx>
#include <wrap.hxx>
#include <drawbase.hxx>
#include <drwbassh.hxx>
#include <swdtflvr.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdpage.hxx>
#include <svx/svditer.hxx>
#include <shells.hrc>
#define SwDrawBaseShell
#include <sfx2/msg.hxx>
#include <swslots.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include "swabstdlg.hxx"
#include "dialog.hrc"
#include <swundo.hxx>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <IDocumentDrawModelAccess.hxx>
#include <memory>

using namespace ::com::sun::star;

SFX_IMPL_SUPERCLASS_INTERFACE(SwDrawBaseShell, SwBaseShell)

void SwDrawBaseShell::InitInterface_Impl()
{
}


SwDrawBaseShell::SwDrawBaseShell(SwView &_rView)
    : SwBaseShell(_rView)
{
    GetShell().NoEdit();

    SwEditWin& rWin = GetView().GetEditWin();

    rWin.SetBezierMode(SID_BEZIER_MOVE);

    if ( !_rView.GetDrawFuncPtr() )
        _rView.GetEditWin().StdDrawMode( OBJ_NONE, true );

    SwTransferable::CreateSelection( GetShell() );
}

SwDrawBaseShell::~SwDrawBaseShell()
{
    GetView().ExitDraw();
    GetShell().Edit();
    SwTransferable::ClearSelection( GetShell() );
}

void SwDrawBaseShell::Execute(SfxRequest &rReq)
{
    SwWrtShell *pSh = &GetShell();
    SdrView*    pSdrView = pSh->GetDrawView();
    const SfxItemSet *pArgs = rReq.GetArgs();
    sal_uInt16      nSlotId = rReq.GetSlot();
    bool        bChanged = pSdrView->GetModel()->IsChanged();
    pSdrView->GetModel()->SetChanged(false);
    const SfxPoolItem* pItem = nullptr;
    if(pArgs)
        pArgs->GetItemState(nSlotId, false, &pItem);

    //Special case align by menu
    if(pItem && nSlotId == SID_OBJECT_ALIGN)
    {
        OSL_ENSURE(dynamic_cast<const SfxEnumItem*>( pItem),"SfxEnumItem expected" );
        nSlotId = nSlotId + static_cast<const SfxEnumItem*>(pItem)->GetValue();
        nSlotId++;
    }

    bool bAlignPossible = pSh->IsAlignPossible();

    bool bTopParam = true, bBottomParam = true;
    bool bDone = false;
    SfxBindings& rBind = GetView().GetViewFrame()->GetBindings();

    switch (nSlotId)
    {
        case FN_DRAW_WRAP_DLG:
        {
            if(pSdrView->AreObjectsMarked())
            {
                if(!pArgs)
                {
                    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
                    if( rMarkList.GetMark(0) != nullptr )
                    {
                        SfxItemSet aSet(GetPool(),  RES_SURROUND, RES_SURROUND,
                                                    RES_ANCHOR, RES_ANCHOR,
                                                    RES_LR_SPACE, RES_UL_SPACE,
                                                    SID_HTML_MODE, SID_HTML_MODE,
                                                    FN_DRAW_WRAP_DLG, FN_DRAW_WRAP_DLG,
                                                    0);

                        aSet.Put(SfxBoolItem(SID_HTML_MODE,
                            0 != ::GetHtmlMode(pSh->GetView().GetDocShell())));

                        aSet.Put(SfxInt16Item(FN_DRAW_WRAP_DLG, pSh->GetLayerId()));

                        pSh->GetObjAttr(aSet);
                        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                        OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

                        std::unique_ptr<SfxAbstractDialog> pDlg(pFact->CreateSwWrapDlg( GetView().GetWindow(), aSet, pSh, true, RC_DLG_SWWRAPDLG ));
                        OSL_ENSURE(pDlg, "Dialog creation failed!");

                        if (pDlg->Execute() == RET_OK)
                        {
                            const SfxPoolItem* pWrapItem;
                            const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                            if(SfxItemState::SET == pOutSet->GetItemState(FN_DRAW_WRAP_DLG, false, &pWrapItem))
                            {
                                short nLayer = static_cast<const SfxInt16Item*>(pWrapItem)->GetValue();
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
            if(pSdrView->AreObjectsMarked())
            {
                if(!pArgs)
                {
                    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
                    if( rMarkList.GetMark(0) != nullptr )
                    {
                        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                        std::unique_ptr<SfxAbstractTabDialog> pDlg;
                        bool bCaption = false;

                        // Allowed anchorages:
                        short nAnchor = pSh->GetAnchorId();
                        sal_uInt16 nAllowedAnchors = SVX_OBJ_AT_CNTNT | SVX_OBJ_IN_CNTNT | SVX_OBJ_PAGE;
                        sal_uInt16 nHtmlMode = ::GetHtmlMode(pSh->GetView().GetDocShell());

                        if ( pSh->IsFlyInFly() )
                            nAllowedAnchors |= SVX_OBJ_AT_FLY;

                        if (pObj->GetObjIdentifier() == OBJ_CAPTION )
                            bCaption = true;

                        if (bCaption)
                        {
                            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                            AbstractSvxCaptionDialog* pCaptionDlg =
                                    pFact->CreateCaptionDialog( nullptr, pSdrView, nAllowedAnchors );
                            pCaptionDlg->SetValidateFramePosLink( LINK(this, SwDrawBaseShell, ValidatePosition) );
                            pDlg.reset(pCaptionDlg);
                        }
                        else
                        {
                            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                            AbstractSvxTransformTabDialog* pTransform =
                                        pFact->CreateSvxTransformTabDialog( nullptr, nullptr, pSdrView, nAllowedAnchors );
                            pTransform->SetValidateFramePosLink( LINK(this, SwDrawBaseShell, ValidatePosition) );
                            pDlg.reset(pTransform);
                        }
                        SfxItemSet aNewAttr(pSdrView->GetGeoAttrFromMarked());

                        const sal_uInt16* pRange = pDlg->GetInputRanges( *aNewAttr.GetPool() );
                        SfxItemSet aSet( *aNewAttr.GetPool(), pRange );
                        FieldUnit eMetric = ::GetDfltMetric( dynamic_cast<SwWebView*>(&GetView()) != nullptr );
                        SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)) );

                        aSet.Put( aNewAttr, false );

                        if (bCaption)
                            pSdrView->GetAttributes( aSet );

                        aSet.Put(SfxInt16Item(SID_ATTR_TRANSFORM_ANCHOR, nAnchor));
                        bool bRTL;
                        bool bVertL2R;
                        aSet.Put(SfxBoolItem(SID_ATTR_TRANSFORM_IN_VERTICAL_TEXT, pSh->IsFrameVertical(true, bRTL, bVertL2R)));
                        aSet.Put(SfxBoolItem(SID_ATTR_TRANSFORM_IN_RTL_TEXT, bRTL));

                        SwFrameFormat* pFrameFormat = FindFrameFormat( pObj );

                        aSet.Put( pFrameFormat->GetFormatAttr(RES_FOLLOW_TEXT_FLOW) );

                        SwFormatVertOrient aVOrient(static_cast<const SwFormatVertOrient&>(pFrameFormat->GetFormatAttr(RES_VERT_ORIENT)));
                        aSet.Put(SfxInt16Item(SID_ATTR_TRANSFORM_VERT_ORIENT, aVOrient.GetVertOrient()));
                        aSet.Put(SfxInt16Item(SID_ATTR_TRANSFORM_VERT_RELATION, aVOrient.GetRelationOrient() ));
                        aSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_VERT_POSITION, aVOrient.GetPos()));

                        SwFormatHoriOrient aHOrient(static_cast<const SwFormatHoriOrient&>(pFrameFormat->GetFormatAttr(RES_HORI_ORIENT)));
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

                            // #i30451#
                            pSh->StartUndo(UNDO_INSFMTATTR);

                            pSdrView->SetGeoAttrToMarked(*pOutSet);

                            if (bCaption)
                                pSdrView->SetAttributes(*pOutSet);

                            bool bPosCorr =
                                SfxItemState::SET != pOutSet->GetItemState(
                                    SID_ATTR_TRANSFORM_POS_X, false ) &&
                                SfxItemState::SET != pOutSet->GetItemState(
                                    SID_ATTR_TRANSFORM_POS_Y, false );

                            SfxItemSet aFrameAttrSet(GetPool(), RES_FRMATR_BEGIN, RES_FRMATR_END - 1);

                            bool bSingleSelection = rMarkList.GetMarkCount() == 1;

                            const SfxPoolItem* pAnchorItem;
                            if(SfxItemState::SET == pOutSet->GetItemState(
                                SID_ATTR_TRANSFORM_ANCHOR, false, &pAnchorItem))
                            {
                                if(!bSingleSelection)
                                    pSh->ChgAnchor(static_cast<const SfxInt16Item*>(pAnchorItem)
                                            ->GetValue(), false, bPosCorr );
                                else
                                {
                                    SwFormatAnchor aAnchor(pFrameFormat->GetAnchor());
                                    aAnchor.SetType((RndStdIds)static_cast<const SfxInt16Item*>(pAnchorItem)->GetValue());
                                    aFrameAttrSet.Put( aAnchor );
                                }
                            }
                            const SfxPoolItem* pHoriOrient = nullptr;
                            const SfxPoolItem* pHoriRelation = nullptr;
                            const SfxPoolItem* pHoriPosition = nullptr;
                            const SfxPoolItem* pHoriMirror = nullptr;
                            pOutSet->GetItemState(SID_ATTR_TRANSFORM_HORI_ORIENT, false, &pHoriOrient);
                            pOutSet->GetItemState(SID_ATTR_TRANSFORM_HORI_RELATION, false, &pHoriRelation);
                            pOutSet->GetItemState(SID_ATTR_TRANSFORM_HORI_POSITION, false, &pHoriPosition);
                            pOutSet->GetItemState(SID_ATTR_TRANSFORM_HORI_MIRROR, false, &pHoriMirror);
                            if(pHoriOrient || pHoriRelation || pHoriPosition || pHoriMirror)
                            {
                                if(pHoriOrient)
                                    aHOrient.SetHoriOrient(
                                          static_cast<const SfxInt16Item*>(pHoriOrient)->GetValue());
                                if(pHoriRelation)
                                    aHOrient.SetRelationOrient(
                                              static_cast<const SfxInt16Item*>(pHoriRelation)->GetValue());
                                if(pHoriPosition)
                                    aHOrient.SetPos( static_cast<const SfxInt32Item*>(pHoriPosition)->GetValue());
                                if(pHoriMirror)
                                    aHOrient.SetPosToggle( static_cast<const SfxBoolItem*>(pHoriMirror)->GetValue());
                                aFrameAttrSet.Put(aHOrient);
                            }

                            const SfxPoolItem* pVertOrient = nullptr;
                            const SfxPoolItem* pVertRelation = nullptr;
                            const SfxPoolItem* pVertPosition = nullptr;
                            pOutSet->GetItemState(SID_ATTR_TRANSFORM_VERT_ORIENT, false, &pVertOrient);
                            pOutSet->GetItemState(SID_ATTR_TRANSFORM_VERT_RELATION, false, &pVertRelation);
                            pOutSet->GetItemState(SID_ATTR_TRANSFORM_VERT_POSITION, false, &pVertPosition);
                            if(pVertOrient || pVertRelation || pVertPosition )
                            {
                                if(pVertOrient)
                                    aVOrient.SetVertOrient(
                                        static_cast<const SfxInt16Item*>(pVertOrient)->GetValue());
                                if(pVertRelation)
                                    aVOrient.SetRelationOrient(
                                        static_cast<const SfxInt16Item*>(pVertRelation)->GetValue());
                                if(pVertPosition)
                                    aVOrient.SetPos( static_cast<const SfxInt32Item*>(pVertPosition)->GetValue());
                                aFrameAttrSet.Put( aVOrient );
                            }
                            const SfxPoolItem* pFollowItem = nullptr;
                            pOutSet->GetItemState(RES_FOLLOW_TEXT_FLOW, false, &pFollowItem);
                            if(pFollowItem)
                                aFrameAttrSet.Put(*pFollowItem);

                            if(aFrameAttrSet.Count())
                                pSh->SetDrawingAttr(aFrameAttrSet);

                            rBind.InvalidateAll(false);

                            // #i30451#
                            pSh->EndUndo( UNDO_INSFMTATTR );

                            pSh->EndAllAction();
                        }
                    }
                }
                else
                {
                    pSdrView->SetGeoAttrToMarked( *pArgs );
                }
            }
        }
        break;

        case SID_DELETE:
        case FN_BACKSPACE:
            if (pSh->IsObjSelected() && !pSdrView->IsTextEdit())
            {
                bDone = true;

                if( GetView().IsDrawRotate() )
                {
                    pSh->SetDragMode( SDRDRAG_MOVE );
                    GetView().FlipDrawRotate();
                }

                pSh->SetModified();
                pSh->DelSelectedObj();

                if (rReq.IsAPI() ||
                    GetView().GetEditWin().IsObjectSelect() )
                {
                    // If basic call, then back to the text shell, because the
                    // Basic otherwise has no possibility to return.
                    if (GetView().GetDrawFuncPtr())
                    {
                        GetView().GetDrawFuncPtr()->Deactivate();
                        GetView().SetDrawFuncPtr(nullptr);
                    }
                    GetView().LeaveDrawCreate();    // Switch to selection mode
                }

                if (pSh->IsSelFrameMode())
                {
                    pSh->LeaveSelFrameMode();
                    // #105852# FME
                }
            }
            break;

        case SID_GROUP:
            if (pSh->IsObjSelected() > 1 && pSh->IsGroupAllowed())
            {
                pSh->GroupSelection();
                rBind.Invalidate(SID_UNGROUP);
            }
            break;

        case SID_UNGROUP:
            if (pSh->IsGroupSelected() && pSh->IsUnGroupAllowed())
            {
                pSh->UnGroupSelection();
                rBind.Invalidate(SID_GROUP);
            }
            break;

        case SID_ENTER_GROUP:
            if (pSh->IsGroupSelected())
            {
                pSdrView->EnterMarkedGroup();
                rBind.InvalidateAll(false);
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
                {   // Do not align objects to each other
                    sal_uInt16 nAnchor = pSh->GetAnchorId();
                    if (nAnchor == FLY_AS_CHAR)
                    {
                        sal_Int16 nVertOrient = -1;

                        switch (nSlotId)
                        {
                            case SID_OBJECT_ALIGN_UP:
                                nVertOrient = text::VertOrientation::TOP;
                                break;
                            case SID_OBJECT_ALIGN_MIDDLE:
                                nVertOrient = text::VertOrientation::CENTER;
                                break;
                            case SID_OBJECT_ALIGN_DOWN:
                                nVertOrient = text::VertOrientation::BOTTOM;
                                break;
                            default:
                                break;
                        }
                        if (nVertOrient != -1)
                        {
                            pSh->StartAction();
                            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                            SwFrameFormat* pFrameFormat = FindFrameFormat( pObj );
                            SwFormatVertOrient aVOrient(static_cast<const SwFormatVertOrient&>(pFrameFormat->GetFormatAttr(RES_VERT_ORIENT)));
                            aVOrient.SetVertOrient( nVertOrient );
                            pFrameFormat->SetFormatAttr(aVOrient);
                            pSh->EndAction();
                        }
                        break;
                    }
                    if (nAnchor == FLY_AT_PARA)
                        break;  // Do not align frames of an anchored paragraph
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
            bTopParam = false;
            SAL_FALLTHROUGH;
        case SID_FRAME_TO_TOP:
            pSh->SelectionToTop( bTopParam );
            break;

        case FN_FRAME_DOWN:
            bBottomParam = false;
            SAL_FALLTHROUGH;
        case SID_FRAME_TO_BOTTOM:
            pSh->SelectionToBottom( bBottomParam );
            break;

        case FN_NAME_SHAPE:
        {
            bDone = true;

            if(1L == pSdrView->GetMarkedObjectCount())
            {
                // #i68101#
                SdrObject* pSelected = pSdrView->GetMarkedObjectByIndex(0);
                OSL_ENSURE(pSelected, "DrawViewShell::FuTemp03: nMarkCount, but no object (!)");
                OUString aName(pSelected->GetName());

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "Dialog creation failed!");
                std::unique_ptr<AbstractSvxObjectNameDialog> pDlg(pFact->CreateSvxObjectNameDialog(nullptr, aName));
                OSL_ENSURE(pDlg, "Dialog creation failed!");

                pDlg->SetCheckNameHdl(LINK(this, SwDrawBaseShell, CheckGroupShapeNameHdl));

                if(RET_OK == pDlg->Execute())
                {
                    pDlg->GetName(aName);
                    pSelected->SetName(aName);
                    pSh->SetModified();
                }
            }

            break;
        }

        // #i68101#
        case FN_TITLE_DESCRIPTION_SHAPE:
        {
            bDone = true;

            if(1L == pSdrView->GetMarkedObjectCount())
            {
                SdrObject* pSelected = pSdrView->GetMarkedObjectByIndex(0);
                OSL_ENSURE(pSelected, "DrawViewShell::FuTemp03: nMarkCount, but no object (!)");
                OUString aTitle(pSelected->GetTitle());
                OUString aDescription(pSelected->GetDescription());

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "Dialog creation failed!");
                std::unique_ptr<AbstractSvxObjectTitleDescDialog> pDlg(pFact->CreateSvxObjectTitleDescDialog(nullptr, aTitle, aDescription));
                OSL_ENSURE(pDlg, "Dialog creation failed!");

                if(RET_OK == pDlg->Execute())
                {
                    pDlg->GetTitle(aTitle);
                    pDlg->GetDescription(aDescription);

                    pSelected->SetTitle(aTitle);
                    pSelected->SetDescription(aDescription);

                    pSh->SetModified();
                }
            }

            break;
        }

        default:
            OSL_ENSURE(false, "wrong Dispatcher");
            return;
    }
    if(!bDone)
    {
        if(nSlotId >= SID_OBJECT_ALIGN_LEFT && nSlotId <= SID_OBJECT_ALIGN_DOWN)
            rBind.Invalidate(SID_ATTR_LONG_LRSPACE);
        if (pSdrView->GetModel()->IsChanged())
            pSh->SetModified();
        else if (bChanged)
            pSdrView->GetModel()->SetChanged();
    }
}

// Checks whether a given name is allowed for a group shape

IMPL_LINK_TYPED( SwDrawBaseShell, CheckGroupShapeNameHdl, AbstractSvxObjectNameDialog&, rNameDialog, bool )
{
    SwWrtShell          &rSh = GetShell();
    SdrView *pSdrView = rSh.GetDrawView();
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    OSL_ENSURE(rMarkList.GetMarkCount() == 1, "wrong draw selection");
    SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
    const OUString sCurrentName = pObj->GetName();
    OUString sNewName;
    rNameDialog.GetName(sNewName);
    bool bRet = false;
    if (sNewName.isEmpty() || sCurrentName == sNewName)
        bRet = true;
    else
    {
        bRet = true;
        SwDrawModel* pModel = rSh.getIDocumentDrawModelAccess().GetDrawModel();
        SdrObjListIter aIter( *(pModel->GetPage(0)), IM_DEEPWITHGROUPS );
        while( aIter.IsMore() )
        {
            SdrObject* pTempObj = aIter.Next();
            if ( pObj != pTempObj && pTempObj->GetName().equals(sNewName) )
            {
                bRet = false;
                break;
            }
        }
    }
    return bRet;
}

void SwDrawBaseShell::GetState(SfxItemSet& rSet)
{
    SwWrtShell &rSh = GetShell();
    SdrView* pSdrView = rSh.GetDrawViewWithValidMarkList();
    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    bool bProtected = rSh.IsSelObjProtected(FlyProtectFlags::Content) != FlyProtectFlags::NONE;

    if (!bProtected)    // Look in the parent
        bProtected |= rSh.IsSelObjProtected( FlyProtectFlags::Content|FlyProtectFlags::Parent ) != FlyProtectFlags::NONE;

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
                if ( !rSh.IsGroupSelected() || bProtected || !rSh.IsUnGroupAllowed() )
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

            case FN_NAME_SHAPE :
                {
                    if(1L != pSdrView->GetMarkedObjectCount())
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;

            // #i68101#
            case FN_TITLE_DESCRIPTION_SHAPE:
                {
                    const bool bIsWebView(nullptr != dynamic_cast<SwWebView*>(&GetView()));

                    if(!bIsWebView && 1L != pSdrView->GetMarkedObjectCount())
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

void SwDrawBaseShell::GetDrawAttrStateForIFBX( SfxItemSet& rSet )
{
    SwWrtShell *pSh = &GetShell();
    SdrView*    pSdrView = pSh->GetDrawView();
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    if( rMarkList.GetMark(0) != nullptr )
    {
        SfxItemSet aNewAttr(pSdrView->GetGeoAttrFromMarked());
        rSet.Put(aNewAttr,false);
    }
}

bool SwDrawBaseShell::Disable(SfxItemSet& rSet, sal_uInt16 nWhich)
{
    bool bDisable = GetShell().IsSelObjProtected(FlyProtectFlags::Content) != FlyProtectFlags::NONE;

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

// Validate of drawing positions

IMPL_LINK_TYPED(SwDrawBaseShell, ValidatePosition, SvxSwFrameValidation&, rValidation, void )
{
    SwWrtShell *pSh = &GetShell();
    rValidation.nMinHeight = MINFLY;
    rValidation.nMinWidth =  MINFLY;

    SwRect aBoundRect;

    // OD 18.09.2003 #i18732# - adjustment for allowing vertical position
    //      aligned to page for fly frame anchored to paragraph or to character.
    const RndStdIds eAnchorType = static_cast<RndStdIds >(rValidation.nAnchorType);
    const SwPosition* pContentPos = nullptr;
    SdrView*  pSdrView = pSh->GetDrawView();
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        SwFrameFormat* pFrameFormat = FindFrameFormat( pObj );
        pContentPos = pFrameFormat->GetAnchor().GetContentAnchor();
    }

    pSh->CalcBoundRect( aBoundRect, eAnchorType,
                           rValidation.nHRelOrient,
                           rValidation.nVRelOrient,
                           pContentPos,
                           rValidation.bFollowTextFlow,
                           rValidation.bMirror, nullptr, &rValidation.aPercentSize);

    bool bIsInVertical( false );
    {
        bool bRTL;
        bool bVertL2R;
        bIsInVertical = pSh->IsFrameVertical(true, bRTL, bVertL2R);
    }
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
        nTmp = rValidation.nWidth;
        rValidation.nWidth = rValidation.nHeight;
        rValidation.nHeight = nTmp;
    }
    if ((eAnchorType == FLY_AT_PAGE) || (eAnchorType == FLY_AT_FLY))
    {
        // MinimalPosition
        rValidation.nMinHPos = aBoundRect.Left();
        rValidation.nMinVPos = aBoundRect.Top();
        SwTwips nH = rValidation.nHPos;
        SwTwips nV = rValidation.nVPos;

        if (rValidation.nHPos + rValidation.nWidth > aBoundRect.Right())
        {
            if (rValidation.nHoriOrient == text::HoriOrientation::NONE)
            {
                rValidation.nHPos -= ((rValidation.nHPos + rValidation.nWidth) - aBoundRect.Right());
                nH = rValidation.nHPos;
            }
            else
                rValidation.nWidth = aBoundRect.Right() - rValidation.nHPos;
        }

        if (rValidation.nHPos + rValidation.nWidth > aBoundRect.Right())
            rValidation.nWidth = aBoundRect.Right() - rValidation.nHPos;

        if (rValidation.nVPos + rValidation.nHeight > aBoundRect.Bottom())
        {
            if (rValidation.nVertOrient == text::VertOrientation::NONE)
            {
                rValidation.nVPos -= ((rValidation.nVPos + rValidation.nHeight) - aBoundRect.Bottom());
                nV = rValidation.nVPos;
            }
            else
                rValidation.nHeight = aBoundRect.Bottom() - rValidation.nVPos;
        }

        if (rValidation.nVPos + rValidation.nHeight > aBoundRect.Bottom())
            rValidation.nHeight = aBoundRect.Bottom() - rValidation.nVPos;

        if ( rValidation.nVertOrient != text::VertOrientation::NONE )
            nV = aBoundRect.Top();

        if ( rValidation.nHoriOrient != text::HoriOrientation::NONE )
            nH = aBoundRect.Left();

        rValidation.nMaxHPos   = aBoundRect.Right()  - rValidation.nWidth;
        rValidation.nMaxHeight = aBoundRect.Bottom() - nV;

        rValidation.nMaxVPos   = aBoundRect.Bottom() - rValidation.nHeight;
        rValidation.nMaxWidth  = aBoundRect.Right()  - nH;
    }
    else if ((eAnchorType == FLY_AT_PARA) || (eAnchorType == FLY_AT_CHAR))
    {
        if (rValidation.nHPos + rValidation.nWidth > aBoundRect.Right())
        {
            if (rValidation.nHoriOrient == text::HoriOrientation::NONE)
            {
                rValidation.nHPos -= ((rValidation.nHPos + rValidation.nWidth) - aBoundRect.Right());
            }
            else
                rValidation.nWidth = aBoundRect.Right() - rValidation.nHPos;
        }

        // OD 29.09.2003 #i17567#, #i18732# - consider following the text flow
        // and alignment at page areas.
        const bool bMaxVPosAtBottom = !rValidation.bFollowTextFlow ||
                                      rValidation.nVRelOrient == text::RelOrientation::PAGE_FRAME ||
                                      rValidation.nVRelOrient == text::RelOrientation::PAGE_PRINT_AREA;
        {
            SwTwips nTmpMaxVPos = ( bMaxVPosAtBottom
                                    ? aBoundRect.Bottom()
                                    : aBoundRect.Height() ) -
                                  rValidation.nHeight;
            if ( rValidation.nVPos > nTmpMaxVPos )
            {
                if (rValidation.nVertOrient == text::VertOrientation::NONE)
                {
                    rValidation.nVPos = nTmpMaxVPos;
                }
                else
                {
                    rValidation.nHeight = ( bMaxVPosAtBottom
                                     ? aBoundRect.Bottom()
                                     : aBoundRect.Height() ) - rValidation.nVPos;
                }
            }
        }

        rValidation.nMinHPos  = aBoundRect.Left();
        rValidation.nMaxHPos  = aBoundRect.Right() - rValidation.nWidth;

        rValidation.nMinVPos  = aBoundRect.Top();
        // OD 26.09.2003 #i17567#, #i18732# - determine maximum vertical position
        if ( bMaxVPosAtBottom )
        {
            rValidation.nMaxVPos  = aBoundRect.Bottom() - rValidation.nHeight;
        }
        else
        {
            rValidation.nMaxVPos  = aBoundRect.Height() - rValidation.nHeight;
        }

        // Maximum width height
        const SwTwips nH = ( rValidation.nHoriOrient != text::HoriOrientation::NONE )
                           ? aBoundRect.Left()
                           : rValidation.nHPos;
        const SwTwips nV = ( rValidation.nVertOrient != text::VertOrientation::NONE )
                           ? aBoundRect.Top()
                           : rValidation.nVPos;
        rValidation.nMaxHeight  = rValidation.nMaxVPos + rValidation.nHeight - nV;
        rValidation.nMaxWidth   = rValidation.nMaxHPos + rValidation.nWidth - nH;
    }
    else if (eAnchorType == FLY_AS_CHAR)
    {
        rValidation.nMinHPos = 0;
        rValidation.nMaxHPos = 0;

        rValidation.nMaxHeight = aBoundRect.Height();
        rValidation.nMaxWidth  = aBoundRect.Width();

        rValidation.nMaxVPos   = aBoundRect.Height();
        rValidation.nMinVPos   = -aBoundRect.Height() + rValidation.nHeight;
        if (rValidation.nMaxVPos < rValidation.nMinVPos)
        {
            rValidation.nMinVPos = rValidation.nMaxVPos;
            rValidation.nMaxVPos = -aBoundRect.Height();
        }
    }
    if(bIsInVertical)
    {
        //restore width/height exchange
        long nTmp = rValidation.nWidth;
        rValidation.nWidth = rValidation.nHeight;
        rValidation.nHeight = nTmp;
    }

    if (rValidation.nMaxWidth < rValidation.nWidth)
        rValidation.nWidth = rValidation.nMaxWidth;
    if (rValidation.nMaxHeight < rValidation.nHeight)
        rValidation.nHeight = rValidation.nMaxHeight;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
