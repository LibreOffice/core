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
#include <swtypes.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svdview.hxx>
#include <svl/whiter.hxx>
#include <svx/swframevalidation.hxx>
#include <svx/anchorid.hxx>
#include <svx/hlnkitem.hxx>
#include <osl/diagnose.h>
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
#include <drawbase.hxx>
#include <drwbassh.hxx>
#include <swdtflvr.hxx>
#include <svx/svditer.hxx>
#define ShellClass_SwDrawBaseShell
#include <sfx2/msg.hxx>
#include <swslots.hxx>
#include <svx/svxdlg.hxx>
#include <svx/svdogrp.hxx>
#include <vcl/unohelp2.hxx>
#include <swabstdlg.hxx>
#include <swundo.hxx>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <fmtfollowtextflow.hxx>
#include <textboxhelper.hxx>
#include <svx/diagram/IDiagramHelper.hxx>
#include <svl/grabbagitem.hxx>
#include <IDocumentSettingAccess.hxx>

using namespace ::com::sun::star;
using namespace css::beans;
using namespace css::uno;

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
        _rView.GetEditWin().StdDrawMode( SdrObjKind::NONE, true );

    SwTransferable::CreateSelection( GetShell() );
}

SwDrawBaseShell::~SwDrawBaseShell()
{
    GetView().ExitDraw();
    GetShell().Edit();
    SwTransferable::ClearSelection( GetShell() );
}

void SwDrawBaseShell::Execute(SfxRequest& rReq)
{
    SwWrtShell *pSh = &GetShell();
    SdrView*    pSdrView = pSh->GetDrawView();
    const SfxItemSet *pArgs = rReq.GetArgs();
    sal_uInt16      nSlotId = rReq.GetSlot();
    bool        bChanged = pSdrView->GetModel().IsChanged();
    pSdrView->GetModel().SetChanged(false);
    const SfxPoolItem* pItem = nullptr;
    if(pArgs)
        pArgs->GetItemState(nSlotId, false, &pItem);

    bool bAlignPossible = pSh->IsAlignPossible();

    bool bTopParam = true, bBottomParam = true;
    bool bDone = false;
    SfxBindings& rBind = GetView().GetViewFrame().GetBindings();

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
                        SfxItemSetFixed<
                                RES_LR_SPACE, RES_UL_SPACE,
                                RES_SURROUND, RES_SURROUND,
                                RES_ANCHOR, RES_ANCHOR,
                                RES_WRAP_INFLUENCE_ON_OBJPOS, RES_WRAP_INFLUENCE_ON_OBJPOS,
                                SID_HTML_MODE, SID_HTML_MODE,
                                FN_DRAW_WRAP_DLG, FN_DRAW_WRAP_DLG>
                            aSet( GetPool() );

                        aSet.Put(SfxBoolItem(SID_HTML_MODE,
                            0 != ::GetHtmlMode(pSh->GetView().GetDocShell())));

                        aSet.Put(SfxInt16Item(FN_DRAW_WRAP_DLG, pSh->GetLayerId().get()));

                        pSh->GetObjAttr(aSet);

                        auto xRequest = std::make_shared<SfxRequest>(rReq);
                        rReq.Ignore(); // the 'old' request is not relevant any more
                        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                        VclPtr<SfxAbstractDialog> pDlg(pFact->CreateSwWrapDlg(GetView().GetFrameWeld(), aSet, pSh));
                        pDlg->StartExecuteAsync(
                            [pDlg, pSh, xRequest=std::move(xRequest)] (sal_Int32 nResult)->void
                            {
                                if (nResult == RET_OK)
                                {
                                    const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                                    if(const SfxInt16Item* pWrapItem = pOutSet->GetItemIfSet(FN_DRAW_WRAP_DLG, false))
                                    {
                                        short nLayer = pWrapItem->GetValue();
                                        if (nLayer == 1)
                                            pSh->SelectionToHeaven();
                                        else
                                            pSh->SelectionToHell();
                                    }

                                    pSh->SetObjAttr(*pOutSet);
                                }
                                pDlg->disposeOnce();
                                xRequest->Done();
                            }
                        );
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
                        VclPtr<SfxAbstractTabDialog> pDlg;
                        bool bCaption = false;

                        // Allowed anchorages:
                        RndStdIds nAnchor = pSh->GetAnchorId();
                        SvxAnchorIds nAllowedAnchors = SvxAnchorIds::Paragraph | SvxAnchorIds::Character | SvxAnchorIds::Page;
                        sal_uInt16 nHtmlMode = ::GetHtmlMode(pSh->GetView().GetDocShell());

                        if ( pSh->IsFlyInFly() )
                            nAllowedAnchors |= SvxAnchorIds::Fly;

                        if (pObj->GetObjIdentifier() == SdrObjKind::Caption )
                            bCaption = true;

                        if (bCaption)
                        {
                            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                            VclPtr<AbstractSvxCaptionDialog> pCaptionDlg =
                                    pFact->CreateCaptionDialog( rReq.GetFrameWeld(), pSdrView, nAllowedAnchors );
                            pDlg.reset(pCaptionDlg);
                            pCaptionDlg->SetValidateFramePosLink( LINK(this, SwDrawBaseShell, ValidatePosition) );
                        }
                        else
                        {
                            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                            VclPtr<AbstractSvxTransformTabDialog> pTransform =
                                        pFact->CreateSvxTransformTabDialog(rReq.GetFrameWeld(), nullptr, pSdrView, nAllowedAnchors);
                            pDlg.reset(pTransform);
                            pTransform->SetValidateFramePosLink( LINK(this, SwDrawBaseShell, ValidatePosition) );
                        }
                        SfxItemSet aNewAttr(pSdrView->GetGeoAttrFromMarked());

                        const WhichRangesContainer& pRange = pDlg->GetInputRanges( *aNewAttr.GetPool() );
                        SfxItemSet aSet( *aNewAttr.GetPool(), pRange );
                        FieldUnit eMetric = ::GetDfltMetric( dynamic_cast<SwWebView*>(&GetView()) != nullptr );
                        SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)) );

                        aSet.Put( aNewAttr, false );

                        if (bCaption)
                            pSdrView->GetAttributes( aSet );

                        aSet.Put(SfxInt16Item(SID_ATTR_TRANSFORM_ANCHOR, static_cast<sal_Int16>(nAnchor)));
                        bool bRTL;
                        bool bVertL2R;
                        aSet.Put(SfxBoolItem(SID_ATTR_TRANSFORM_IN_VERTICAL_TEXT, pSh->IsFrameVertical(true, bRTL, bVertL2R)));
                        aSet.Put(SfxBoolItem(SID_ATTR_TRANSFORM_IN_RTL_TEXT, bRTL));

                        SwFrameFormat* pFrameFormat = FindFrameFormat( pObj );

                        aSet.Put( pFrameFormat->GetFormatAttr(RES_FOLLOW_TEXT_FLOW) );

                        SwFormatVertOrient aVOrient(pFrameFormat->GetFormatAttr(RES_VERT_ORIENT));
                        aSet.Put(SfxInt16Item(SID_ATTR_TRANSFORM_VERT_ORIENT, aVOrient.GetVertOrient()));
                        aSet.Put(SfxInt16Item(SID_ATTR_TRANSFORM_VERT_RELATION, aVOrient.GetRelationOrient() ));
                        aSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_VERT_POSITION, aVOrient.GetPos()));

                        SwFormatHoriOrient aHOrient(pFrameFormat->GetFormatAttr(RES_HORI_ORIENT));
                        aSet.Put(SfxInt16Item(SID_ATTR_TRANSFORM_HORI_ORIENT, aHOrient.GetHoriOrient()));
                        aSet.Put(SfxInt16Item(SID_ATTR_TRANSFORM_HORI_RELATION, aHOrient.GetRelationOrient() ));
                        aSet.Put(SfxBoolItem(SID_ATTR_TRANSFORM_HORI_MIRROR, aHOrient.IsPosToggle()));
                        aSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_HORI_POSITION, aHOrient.GetPos()));

                        const IDocumentSettingAccess& rIDSA = pFrameFormat->getIDocumentSettingAccess();
                        if (rIDSA.get(DocumentSettingId::DO_NOT_MIRROR_RTL_DRAW_OBJS))
                        {
                            SfxGrabBagItem aItem(RES_CHRATR_GRABBAG);
                            aItem.GetGrabBag()["DoNotMirrorRtlDrawObjs"] <<= true;
                            aSet.Put(aItem);
                        }

                        aSet.Put(SfxUInt16Item(SID_HTML_MODE, nHtmlMode));

                        pDlg->SetInputSet( &aSet );

                        pDlg->StartExecuteAsync([bCaption, bChanged, pDlg, pFrameFormat, pSdrView,
                                                 pSh, &rMarkList, this](
                                                    sal_Int32 nResult){
                            pSdrView->GetModel().SetChanged(false);

                            if (nResult == RET_OK)
                            {
                                SwFormatVertOrient aVOrientFinal(pFrameFormat->GetFormatAttr(RES_VERT_ORIENT));
                                SwFormatHoriOrient aHOrientFinal(pFrameFormat->GetFormatAttr(RES_HORI_ORIENT));

                                const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                                pSh->StartAllAction();

                                // #i30451#
                                pSh->StartUndo(SwUndoId::INSFMTATTR);

                                pSdrView->SetGeoAttrToMarked(*pOutSet);

                                if (bCaption)
                                    pSdrView->SetAttributes(*pOutSet);

                                bool bPosCorr =
                                    SfxItemState::SET != pOutSet->GetItemState(
                                        SID_ATTR_TRANSFORM_POS_X, false ) &&
                                    SfxItemState::SET != pOutSet->GetItemState(
                                        SID_ATTR_TRANSFORM_POS_Y, false );

                                SfxItemSetFixed<RES_FRMATR_BEGIN, RES_FRMATR_END - 1> aFrameAttrSet(GetPool());

                                bool bSingleSelection = rMarkList.GetMarkCount() == 1;

                                if(const SfxInt16Item* pAnchorItem = pOutSet->GetItemIfSet(
                                    SID_ATTR_TRANSFORM_ANCHOR, false))
                                {
                                    if(!bSingleSelection)
                                        pSh->ChgAnchor(static_cast<RndStdIds>(pAnchorItem
                                                ->GetValue()), false, bPosCorr );
                                    else
                                    {
                                        SwFormatAnchor aAnchor(pFrameFormat->GetAnchor());
                                        aAnchor.SetType(static_cast<RndStdIds>(pAnchorItem->GetValue()));
                                        aFrameAttrSet.Put( aAnchor );
                                    }
                                }
                                const SfxInt16Item* pHoriOrient =
                                    pOutSet->GetItemIfSet(SID_ATTR_TRANSFORM_HORI_ORIENT, false);
                                const SfxInt16Item* pHoriRelation =
                                    pOutSet->GetItemIfSet(SID_ATTR_TRANSFORM_HORI_RELATION, false);
                                const SfxInt32Item* pHoriPosition =
                                    pOutSet->GetItemIfSet(SID_ATTR_TRANSFORM_HORI_POSITION, false);
                                const SfxBoolItem* pHoriMirror =
                                    pOutSet->GetItemIfSet(SID_ATTR_TRANSFORM_HORI_MIRROR, false);
                                if(pHoriOrient || pHoriRelation || pHoriPosition || pHoriMirror)
                                {
                                    if(pHoriOrient)
                                        aHOrientFinal.SetHoriOrient(pHoriOrient->GetValue());
                                    if(pHoriRelation)
                                        aHOrientFinal.SetRelationOrient(pHoriRelation->GetValue());
                                    if(pHoriPosition)
                                        aHOrientFinal.SetPos( pHoriPosition->GetValue());
                                    if(pHoriMirror)
                                        aHOrientFinal.SetPosToggle( pHoriMirror->GetValue());
                                    aFrameAttrSet.Put(aHOrientFinal);
                                }

                                const SfxInt16Item* pVertOrient =
                                    pOutSet->GetItemIfSet(SID_ATTR_TRANSFORM_VERT_ORIENT, false);
                                const SfxInt16Item* pVertRelation =
                                    pOutSet->GetItemIfSet(SID_ATTR_TRANSFORM_VERT_RELATION, false);
                                const SfxInt32Item* pVertPosition =
                                    pOutSet->GetItemIfSet(SID_ATTR_TRANSFORM_VERT_POSITION, false);
                                if(pVertOrient || pVertRelation || pVertPosition )
                                {
                                    if(pVertOrient)
                                        aVOrientFinal.SetVertOrient(pVertOrient->GetValue());
                                    if(pVertRelation)
                                        aVOrientFinal.SetRelationOrient(pVertRelation->GetValue());
                                    if(pVertPosition)
                                        aVOrientFinal.SetPos( pVertPosition->GetValue());
                                    aFrameAttrSet.Put( aVOrientFinal );
                                }
                                const SwFormatFollowTextFlow* pFollowItem =
                                    pOutSet->GetItemIfSet(RES_FOLLOW_TEXT_FLOW, false);
                                if(pFollowItem)
                                    aFrameAttrSet.Put(*pFollowItem);

                                if(aFrameAttrSet.Count())
                                    pSh->SetDrawingAttr(aFrameAttrSet);

                                GetView().GetViewFrame().GetBindings().InvalidateAll(false);

                                // #i30451#
                                pSh->EndUndo( SwUndoId::INSFMTATTR );

                                pSh->EndAllAction();
                            }

                            if (pSdrView->GetModel().IsChanged())
                                pSh->SetModified();
                            else if (bChanged)
                                pSdrView->GetModel().SetChanged();

                            pDlg->disposeOnce();
                        });
                    }
                }
                else
                {
                    pSh->StartAllAction();
                    pSdrView->SetGeoAttrToMarked( *pArgs );
                    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
                    SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                    if (pObj)
                    {
                        SwFrameFormat* pFrameFormat = FindFrameFormat(pObj);
                        if (pFrameFormat)
                        {
                            const SwFormatAnchor& rAnchor = pFrameFormat->GetAnchor();
                            // Don't change shape position / size, just update the anchor doc model
                            // position.
                            pSh->ChgAnchor(rAnchor.GetAnchorId(), /*bSameOnly=*/true);
                        }
                    }
                    pSh->EndAllAction();
                }
            }
        }
        break;

        case SID_DELETE:
        case FN_BACKSPACE:
            if (pSh->IsObjSelected() && !pSdrView->IsTextEdit())
            {
                bDone = true;

                const Point aPt = pSh->GetObjRect().TopLeft(); // tdf#150589

                if( GetView().IsDrawRotate() )
                {
                    pSh->SetDragMode( SdrDragMode::Move );
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
                    // #105852# FME <- perhaps fixed by tdf#150589
                    static_cast<SwEditShell*>(pSh)->SetCursor(aPt);
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
            if (pSh->IsGroupSelected(true) && pSh->IsUnGroupAllowed())
            {
                pSh->UnGroupSelection();
                rBind.Invalidate(SID_GROUP);
            }
            break;

        case SID_ENTER_GROUP:
            if (pSh->IsGroupSelected(false))
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

        case SID_REGENERATE_DIAGRAM:
        case SID_EDIT_DIAGRAM:
            {
                const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();

                if (1 == rMarkList.GetMarkCount())
                {
                    SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

                    // Support advanced DiagramHelper
                    if(nullptr != pObj && pObj->isDiagram())
                    {
                        if(SID_REGENERATE_DIAGRAM == nSlotId)
                        {
                            pSdrView->UnmarkAll();
                            pObj->getDiagramHelper()->reLayout(*static_cast<SdrObjGroup*>(pObj));
                            pSdrView->MarkObj(pObj, pSdrView->GetSdrPageView());
                        }
                        else // SID_EDIT_DIAGRAM
                        {
                            VclAbstractDialogFactory* pFact = VclAbstractDialogFactory::Create();
                            VclPtr<VclAbstractDialog> pDlg = pFact->CreateDiagramDialog(
                                GetView().GetFrameWeld(),
                                *static_cast<SdrObjGroup*>(pObj));
                            pDlg->StartExecuteAsync(
                                [pDlg] (sal_Int32 /*nResult*/)->void
                                {
                                    pDlg->disposeOnce();
                                }
                            );
                        }
                    }
                }
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
                if (rMarkList.GetMarkCount() == 1
                    && !SwTextBoxHelper::hasTextFrame(rMarkList.GetMark(0)->GetMarkedSdrObj()))
                {
                    sal_Int16 nHorizOrient = -1, nVertOrient = -1;

                    switch (nSlotId)
                    {
                        case SID_OBJECT_ALIGN_LEFT:
                            nHorizOrient = text::HoriOrientation::LEFT;
                            break;
                        case SID_OBJECT_ALIGN_CENTER:
                            nHorizOrient = text::HoriOrientation::CENTER;
                            break;
                        case SID_OBJECT_ALIGN_RIGHT:
                            nHorizOrient = text::HoriOrientation::RIGHT;
                            break;
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

                    if (nHorizOrient != -1)
                    {
                        pSh->StartAction();
                        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                        SwFrameFormat* pFrameFormat = FindFrameFormat( pObj );
                        SwFormatHoriOrient aHOrient(pFrameFormat->GetFormatAttr(RES_HORI_ORIENT));
                        aHOrient.SetHoriOrient( nHorizOrient );
                        pFrameFormat->SetFormatAttr(aHOrient);
                        pSh->EndAction();
                    }

                    if (nVertOrient != -1)
                    {
                        pSh->StartAction();
                        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                        SwFrameFormat* pFrameFormat = FindFrameFormat( pObj );
                        SwFormatVertOrient aVOrient(pFrameFormat->GetFormatAttr(RES_VERT_ORIENT));
                        aVOrient.SetVertOrient( nVertOrient );
                        pFrameFormat->SetFormatAttr(aVOrient);
                        pSh->EndAction();
                    }

                    break;
                }

                pSh->StartAction();
                switch (nSlotId)
                {
                    case SID_OBJECT_ALIGN_LEFT:
                        pSdrView->AlignMarkedObjects(SdrHorAlign::Left, SdrVertAlign::NONE);
                        break;
                    case SID_OBJECT_ALIGN_CENTER:
                        pSdrView->AlignMarkedObjects(SdrHorAlign::Center, SdrVertAlign::NONE);
                        break;
                    case SID_OBJECT_ALIGN_RIGHT:
                        pSdrView->AlignMarkedObjects(SdrHorAlign::Right, SdrVertAlign::NONE);
                        break;
                    case SID_OBJECT_ALIGN_UP:
                        pSdrView->AlignMarkedObjects(SdrHorAlign::NONE, SdrVertAlign::Top);
                        break;
                    case SID_OBJECT_ALIGN_MIDDLE:
                        pSdrView->AlignMarkedObjects(SdrHorAlign::NONE, SdrVertAlign::Center);
                        break;
                    case SID_OBJECT_ALIGN_DOWN:
                        pSdrView->AlignMarkedObjects(SdrHorAlign::NONE, SdrVertAlign::Bottom);
                        break;
                }
                pSh->EndAction();
            }
        }
        break;

        case FN_FRAME_UP:
            bTopParam = false;
            [[fallthrough]];
        case SID_FRAME_TO_TOP:
            pSh->SelectionToTop( bTopParam );
            break;

        case FN_FRAME_DOWN:
            bBottomParam = false;
            [[fallthrough]];
        case SID_FRAME_TO_BOTTOM:
            pSh->SelectionToBottom( bBottomParam );
            break;

        case FN_NAME_SHAPE:
        {
            bDone = true;

            if(1 == pSdrView->GetMarkedObjectList().GetMarkCount())
            {
                // #i68101#
                rtl::Reference<SdrObject> pSelected = pSdrView->GetMarkedObjectByIndex(0);
                assert(pSelected && "DrawViewShell::FuTemp03: nMarkCount, but no object (!)");
                OUString aOrigName(pSelected->GetName());

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                VclPtr<AbstractSvxObjectNameDialog> pDlg(pFact->CreateSvxObjectNameDialog(GetView().GetFrameWeld(), aOrigName));

                pDlg->SetCheckNameHdl(LINK(this, SwDrawBaseShell, CheckGroupShapeNameHdl));

                pDlg->StartExecuteAsync(
                    [pDlg, pSelected, pSh, aOrigName] (sal_Int32 nResult)->void
                    {
                        if (nResult == RET_OK)
                        {
                            OUString aNewName = pDlg->GetName();
                            pSelected->SetName(aNewName);
                            pSh->SetModified();

                            // update accessibility sidebar object name if we modify the object name on the navigator bar
                            if (!aNewName.isEmpty() && aOrigName != aNewName)
                            {
                                auto pFrameFormat = FindFrameFormat(pSelected.get());
                                if (pFrameFormat)
                                {
                                    if (SwNode* pSwNode = pFrameFormat->GetAnchor().GetAnchorNode())
                                        pSwNode->resetAndQueueAccessibilityCheck(true);
                                }
                            }
                        }
                        pDlg->disposeOnce();
                    }
                );
            }

            break;
        }

        // #i68101#
        case FN_TITLE_DESCRIPTION_SHAPE:
        {
            bDone = true;

            if(1 == pSdrView->GetMarkedObjectList().GetMarkCount())
            {
                rtl::Reference<SdrObject> pSelected = pSdrView->GetMarkedObjectByIndex(0);
                assert(pSelected && "DrawViewShell::FuTemp03: nMarkCount, but no object (!)");
                OUString aTitle(pSelected->GetTitle());
                OUString aDescription(pSelected->GetDescription());
                bool isDecorative(pSelected->IsDecorative());

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                VclPtr<AbstractSvxObjectTitleDescDialog> pDlg(pFact->CreateSvxObjectTitleDescDialog(GetView().GetFrameWeld(),
                            aTitle, aDescription, isDecorative));

                pDlg->StartExecuteAsync(
                    [pDlg, pSelected, pSh] (sal_Int32 nResult)->void
                    {
                        if (nResult == RET_OK)
                        {
                            pSelected->SetTitle(pDlg->GetTitle());
                            pSelected->SetDescription(pDlg->GetDescription());
                            pSelected->SetDecorative(pDlg->IsDecorative());

                            pSh->SetModified();
                        }
                        pDlg->disposeOnce();
                    }
                );
            }

            break;
        }

        case SID_OPEN_HYPERLINK:
        {
            const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            LoadURL(GetShell(), pObj->getHyperlink(), LoadUrlFlags::NewView,
                    /*rTargetFrameName=*/OUString());
            break;
        }

        case SID_EDIT_HYPERLINK:
        case SID_HYPERLINK_DIALOG:
        {
            GetView().GetViewFrame().SetChildWindow(SID_HYPERLINK_DIALOG, true);
            break;
        }

        case SID_HYPERLINK_SETLINK:
        {
            if(pItem)
            {
                const SvxHyperlinkItem& rHLinkItem = *static_cast<const SvxHyperlinkItem *>(pItem);
                const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
                SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                pObj->setHyperlink(rHLinkItem.GetURL());
            }
            break;
        }

        case SID_REMOVE_HYPERLINK:
        {
            const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            pObj->setHyperlink(OUString());
            break;
        }

        case SID_COPY_HYPERLINK_LOCATION:
        {
            const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            uno::Reference<datatransfer::clipboard::XClipboard> xClipboard
                = GetView().GetEditWin().GetClipboard();
            vcl::unohelper::TextDataObject::CopyStringTo(pObj->getHyperlink(), xClipboard);
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
        if (pSdrView->GetModel().IsChanged())
            pSh->SetModified();
        else if (bChanged)
            pSdrView->GetModel().SetChanged();
    }
}

// Checks whether a given name is allowed for a group shape

IMPL_LINK( SwDrawBaseShell, CheckGroupShapeNameHdl, AbstractSvxObjectNameDialog&, rNameDialog, bool )
{
    SwWrtShell          &rSh = GetShell();
    SdrView *pSdrView = rSh.GetDrawView();
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    OSL_ENSURE(rMarkList.GetMarkCount() == 1, "wrong draw selection");
    SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
    const OUString sCurrentName = pObj->GetName();
    OUString sNewName = rNameDialog.GetName();
    bool bRet = false;
    if (sNewName.isEmpty() || sCurrentName == sNewName)
        bRet = true;
    else
    {
        bRet = true;
        SwDrawModel* pModel = rSh.getIDocumentDrawModelAccess().GetDrawModel();
        SdrObjListIter aIter( pModel->GetPage(0), SdrIterMode::DeepWithGroups );
        while( aIter.IsMore() )
        {
            SdrObject* pTempObj = aIter.Next();
            if ( pObj != pTempObj && pTempObj->GetName() == sNewName )
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
                if ( !rSh.IsGroupSelected(true) || bProtected || !rSh.IsUnGroupAllowed() )
                    rSet.DisableItem( nWhich );
                break;
            case SID_ENTER_GROUP:
                if ( !rSh.IsGroupSelected(false) )
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
                {
                    bool bDisableThis = false;
                    bool bDisableHoriz = false;
                    bool bHoriz = (nWhich == SID_OBJECT_ALIGN_LEFT || nWhich == SID_OBJECT_ALIGN_CENTER ||
                            nWhich == SID_OBJECT_ALIGN_RIGHT);
                    bool bVert = (nWhich == SID_OBJECT_ALIGN_UP || nWhich == SID_OBJECT_ALIGN_MIDDLE ||
                            nWhich == SID_OBJECT_ALIGN_DOWN);
                    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
                    if ( !rSh.IsAlignPossible() || bProtected )
                    {
                        bDisableThis = true;
                        rSet.DisableItem( nWhich );
                    }
                    else if ( rSh.GetAnchorId() == RndStdIds::FLY_AS_CHAR )
                    {
                        //if only one object is selected it can only be vertically
                        // aligned because it is character bound
                        if( rMarkList.GetMarkCount() == 1 )
                        {
                            bDisableHoriz = true;
                            rSet.DisableItem(SID_OBJECT_ALIGN_LEFT);
                            rSet.DisableItem(SID_OBJECT_ALIGN_CENTER);
                            rSet.DisableItem(SID_OBJECT_ALIGN_RIGHT);
                        }
                    }

                    if (bHoriz && !bDisableThis && !bDisableHoriz &&
                        rMarkList.GetMarkCount() == 1)
                    {
                        sal_Int16 nHoriOrient = -1;
                        switch(nWhich)
                        {
                            case SID_OBJECT_ALIGN_LEFT:
                                nHoriOrient = text::HoriOrientation::LEFT;
                                break;
                            case SID_OBJECT_ALIGN_CENTER:
                                nHoriOrient = text::HoriOrientation::CENTER;
                                break;
                            case SID_OBJECT_ALIGN_RIGHT:
                                nHoriOrient = text::HoriOrientation::RIGHT;
                                break;
                            default:
                                break;
                        }

                        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                        SwFrameFormat* pFrameFormat = FindFrameFormat(pObj);
                        if (pFrameFormat)
                        {
                            SwFormatHoriOrient aHOrient(
                                pFrameFormat->GetFormatAttr(RES_HORI_ORIENT));
                            rSet.Put(SfxBoolItem(nWhich, aHOrient.GetHoriOrient() == nHoriOrient));
                        }
                    }

                    if (bVert && !bDisableThis && rMarkList.GetMarkCount() == 1)
                    {
                        sal_Int16 nVertOrient = -1;
                        switch(nWhich)
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

                        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                        SwFrameFormat* pFrameFormat = FindFrameFormat(pObj);
                        if (pFrameFormat)
                        {
                            SwFormatVertOrient aVOrient(
                                pFrameFormat->GetFormatAttr(RES_VERT_ORIENT));
                            rSet.Put(SfxBoolItem(nWhich, aVOrient.GetVertOrient() == nVertOrient));
                        }
                    }
                }
                break;

            case FN_NAME_SHAPE :
                {
                    if(1 != pSdrView->GetMarkedObjectList().GetMarkCount())
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;

            // #i68101#
            case FN_TITLE_DESCRIPTION_SHAPE:
                {
                    const bool bIsWebView(nullptr != dynamic_cast<SwWebView*>(&GetView()));

                    if(!bIsWebView && 1 != pSdrView->GetMarkedObjectList().GetMarkCount())
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;

            case SID_OPEN_HYPERLINK:
            case SID_EDIT_HYPERLINK:
            case SID_HYPERLINK_DIALOG:
            case SID_REMOVE_HYPERLINK:
            case SID_COPY_HYPERLINK_LOCATION:
            {
                if (pSdrView->GetMarkedObjectList().GetMarkCount() != 1)
                {
                    rSet.DisableItem(nWhich);
                    break;
                }

                const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
                SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                SdrObjKind nObjType = pObj->GetObjIdentifier();

                // Only enable hyperlink for the following types
                switch (nObjType)
                {
                    case SdrObjKind::PathFill:
                    case SdrObjKind::CircleSection:
                    case SdrObjKind::Line:
                    case SdrObjKind::CustomShape:
                    case SdrObjKind::Text:
                    case SdrObjKind::Rectangle:
                    case SdrObjKind::Caption:
                    case SdrObjKind::Polygon:
                    case SdrObjKind::PolyLine:
                    case SdrObjKind::E3D_Scene:
                    case SdrObjKind::Measure:
                    case SdrObjKind::Edge:
                        break;
                    default:
                        rSet.DisableItem(nWhich);
                        break;
                }

                if (nWhich == SID_OPEN_HYPERLINK || nWhich == SID_REMOVE_HYPERLINK
                    || nWhich == SID_EDIT_HYPERLINK || nWhich == SID_COPY_HYPERLINK_LOCATION)
                {
                    if (pObj->getHyperlink().isEmpty())
                        rSet.DisableItem(nWhich);
                }
            }
            break;

            case SID_HYPERLINK_GETLINK:
            {
                const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
                if (rMarkList.GetMark(0) != nullptr)
                {
                    SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                    OUString sHyperLink = pObj->getHyperlink();
                    SvxHyperlinkItem aHLinkItem;
                    aHLinkItem.SetURL(sHyperLink);
                    rSet.Put(aHLinkItem);
                }
            }
            break;

            case SID_REGENERATE_DIAGRAM:
            case SID_EDIT_DIAGRAM:
            {
                bool bDisable(true);
                const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
                if (nullptr != rMarkList.GetMark(0))
                {
                    SdrObject* pObj(rMarkList.GetMark(0)->GetMarkedSdrObj());

                    if(nullptr != pObj && pObj->isDiagram())
                    {
                        bDisable = false;
                    }
                }

                if(bDisable)
                {
                    rSet.DisableItem(nWhich);
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

void SwDrawBaseShell::DisableState( SfxItemSet& rSet )
{
    SwWrtShell *pSh = &GetShell();
    SdrView*    pSdrView = pSh->GetDrawView();
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    const size_t nMarkCount = rMarkList.GetMarkCount();
    bool bShowArea = true, bShowMeasure = true;

    for (size_t i = 0; i < nMarkCount && i < 50; ++i)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
        SdrObjKind nObjType = pObj->GetObjIdentifier();

        if ( nObjType != SdrObjKind::Measure )
            bShowMeasure = false;

        // If marked object is 2D, disable format area command.
        if ( nObjType == SdrObjKind::PolyLine     ||
             nObjType == SdrObjKind::Line     ||
             nObjType == SdrObjKind::PathLine ||
             nObjType == SdrObjKind::FreehandLine ||
             nObjType == SdrObjKind::Edge     ||
             nObjType == SdrObjKind::CircleArc     ||
             bShowMeasure )
            bShowArea = false;

        if (!bShowArea && !bShowMeasure)
            break;
    }

    if (!bShowArea)
        rSet.DisableItem(SID_ATTRIBUTES_AREA);

    if (!bShowMeasure)
        rSet.DisableItem(SID_MEASURE_DLG);

    Disable(rSet);

}

// Validate of drawing positions

IMPL_LINK(SwDrawBaseShell, ValidatePosition, SvxSwFrameValidation&, rValidation, void )
{
    SwWrtShell *pSh = &GetShell();
    rValidation.nMinHeight = MINFLY;
    rValidation.nMinWidth =  MINFLY;

    SwRect aBoundRect;

    // OD 18.09.2003 #i18732# - adjustment for allowing vertical position
    //      aligned to page for fly frame anchored to paragraph or to character.
    const RndStdIds eAnchorType = rValidation.nAnchorType;
    const SwFormatAnchor* pAnchor = nullptr;
    SdrView*  pSdrView = pSh->GetDrawView();
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        SwFrameFormat* pFrameFormat = FindFrameFormat( pObj );
        pAnchor = &pFrameFormat->GetAnchor();
    }

    pSh->CalcBoundRect( aBoundRect, eAnchorType,
                           rValidation.nHRelOrient,
                           rValidation.nVRelOrient,
                           pAnchor,
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
        tools::Long nTmp = aPos.X();
        aPos.setX( aPos.Y() );
        aPos.setY( nTmp );
        Size aSize(aBoundRect.SSize());
        nTmp = aSize.Width();
        aSize.setWidth( aSize.Height() );
        aSize.setHeight( nTmp );
        aBoundRect.Chg( aPos, aSize );
        //exchange width/height to enable correct values
        nTmp = rValidation.nWidth;
        rValidation.nWidth = rValidation.nHeight;
        rValidation.nHeight = nTmp;
    }
    if ((eAnchorType == RndStdIds::FLY_AT_PAGE) || (eAnchorType == RndStdIds::FLY_AT_FLY))
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
    else if ((eAnchorType == RndStdIds::FLY_AT_PARA) || (eAnchorType == RndStdIds::FLY_AT_CHAR))
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
                                      rValidation.nVRelOrient == text::RelOrientation::PAGE_PRINT_AREA ||
                                      rValidation.nVRelOrient == text::RelOrientation::PAGE_PRINT_AREA_BOTTOM;
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
    else if (eAnchorType == RndStdIds::FLY_AS_CHAR)
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
        tools::Long nTmp = rValidation.nWidth;
        rValidation.nWidth = rValidation.nHeight;
        rValidation.nHeight = nTmp;
    }

    if (rValidation.nMaxWidth < rValidation.nWidth)
        rValidation.nWidth = rValidation.nMaxWidth;
    if (rValidation.nMaxHeight < rValidation.nHeight)
        rValidation.nHeight = rValidation.nMaxHeight;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
