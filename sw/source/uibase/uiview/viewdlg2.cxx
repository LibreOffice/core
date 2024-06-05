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

#include <sfx2/request.hxx>
#include <sfx2/objface.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdview.hxx>
#include <osl/diagnose.h>
#include <fldmgr.hxx>
#include <expfld.hxx>
#include <modcfg.hxx>

#include <swmodule.hxx>
#include <view.hxx>
#include <wview.hxx>
#include <wrtsh.hxx>
#include <cmdid.h>
#include <caption.hxx>
#include <poolfmt.hxx>
#include <edtwin.hxx>
#include <SwStyleNameMapper.hxx>

#include <swabstdlg.hxx>

#include <strings.hrc>

#include <memory>

#include <svl/stritem.hxx>

using namespace css;

void SwView::ExecDlgExt(SfxRequest& rReq)
{
    switch (rReq.GetSlot())
    {
        case FN_INSERT_CAPTION:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            VclPtr<VclAbstractDialog> pDialog(
                pFact->CreateSwCaptionDialog(GetFrameWeld(), *this));
            pDialog->StartExecuteAsync([pDialog](sal_Int32) {
                pDialog->disposeOnce();
            });
            break;
        }
        case SID_INSERT_SIGNATURELINE:
        case SID_EDIT_SIGNATURELINE:
        {
            VclAbstractDialogFactory* pFact = VclAbstractDialogFactory::Create();
            const uno::Reference<frame::XModel> xModel(GetCurrentDocument());
            VclPtr<AbstractSignatureLineDialog> pDialog(pFact->CreateSignatureLineDialog(
                GetFrameWeld(), xModel, rReq.GetSlot() == SID_EDIT_SIGNATURELINE));
            auto xRequest = std::make_shared<SfxRequest>(rReq);
            rReq.Ignore(); // the 'old' request is not relevant any more
            pDialog->StartExecuteAsync(
                [pDialog, xRequest=std::move(xRequest)] (sal_Int32 nResult)->void
                {
                    if (nResult == RET_OK)
                        pDialog->Apply();
                    pDialog->disposeOnce();
                    xRequest->Done();
                }
            );
            break;
        }
        case SID_INSERT_QRCODE:
        case SID_EDIT_QRCODE:
        {
            VclAbstractDialogFactory* pFact = VclAbstractDialogFactory::Create();
            const uno::Reference<frame::XModel> xModel(GetCurrentDocument());
            VclPtr<AbstractQrCodeGenDialog> pDialog(pFact->CreateQrCodeGenDialog(
                GetFrameWeld(), xModel, rReq.GetSlot() == SID_EDIT_QRCODE));
            pDialog->StartExecuteAsync([pDialog](sal_Int32) {
                pDialog->disposeOnce();
            });
            break;
        }
        case SID_ADDITIONS_DIALOG:
        {
            OUString sAdditionsTag = u""_ustr;

            const SfxStringItem* pStringArg = rReq.GetArg<SfxStringItem>(FN_PARAM_ADDITIONS_TAG);
            if (pStringArg)
                sAdditionsTag = pStringArg->GetValue();

            VclAbstractDialogFactory* pFact = VclAbstractDialogFactory::Create();
            VclPtr<AbstractAdditionsDialog> pDialog(
                pFact->CreateAdditionsDialog(GetFrameWeld(), sAdditionsTag));
            pDialog->StartExecuteAsync(
                [pDialog] (sal_Int32 /*nResult*/)->void
                {
                    pDialog->disposeOnce();
                }
            );
            break;
        }
        case SID_SIGN_SIGNATURELINE:
        {
            VclAbstractDialogFactory* pFact = VclAbstractDialogFactory::Create();
            const uno::Reference<frame::XModel> xModel(GetCurrentDocument());
            VclPtr<AbstractSignSignatureLineDialog> pDialog(
                pFact->CreateSignSignatureLineDialog(GetFrameWeld(), xModel));
            pDialog->StartExecuteAsync(
                [pDialog] (sal_Int32 nResult)->void
                {
                    if (nResult == RET_OK)
                        pDialog->Apply();
                    pDialog->disposeOnce();
                }
            );
            break;
        }
        case  FN_EDIT_FOOTNOTE:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            VclPtr<AbstractInsFootNoteDlg> pDlg(pFact->CreateInsFootNoteDlg(
                GetFrameWeld(), *m_pWrtShell, true));

            pDlg->SetHelpId(GetStaticInterface()->GetSlot(FN_EDIT_FOOTNOTE)->GetCommand());
            pDlg->SetText( SwResId(STR_EDIT_FOOTNOTE) );
            pDlg->StartExecuteAsync(
                [pDlg] (sal_Int32 nResult)->void
                {
                    if (nResult == RET_OK)
                        pDlg->Apply();
                    pDlg->disposeOnce();
                }
            );
            break;
        }
    }
}

bool SwView::isSignatureLineSelected() const
{
    SwWrtShell& rSh = GetWrtShell();
    SdrView* pSdrView = rSh.GetDrawView();
    if (!pSdrView)
        return false;

    if (pSdrView->GetMarkedObjectList().GetMarkCount() != 1)
        return false;

    SdrObject* pPickObj = pSdrView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
    if (!pPickObj)
        return false;

    SdrGrafObj* pGraphic = dynamic_cast<SdrGrafObj*>(pPickObj);
    if (!pGraphic)
        return false;

    return pGraphic->isSignatureLine();
}

bool SwView::isSignatureLineSigned() const
{
    SwWrtShell& rSh = GetWrtShell();
    SdrView* pSdrView = rSh.GetDrawView();
    if (!pSdrView)
        return false;

    if (pSdrView->GetMarkedObjectList().GetMarkCount() != 1)
        return false;

    SdrObject* pPickObj = pSdrView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
    if (!pPickObj)
        return false;

    SdrGrafObj* pGraphic = dynamic_cast<SdrGrafObj*>(pPickObj);
    if (!pGraphic)
        return false;

    return pGraphic->isSignatureLineSigned();
}

bool SwView::isQRCodeSelected() const
{
    SwWrtShell& rSh = GetWrtShell();
    SdrView* pSdrView = rSh.GetDrawView();
    if (!pSdrView)
        return false;

    if (pSdrView->GetMarkedObjectList().GetMarkCount() != 1)
        return false;

    SdrObject* pPickObj = pSdrView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
    if (!pPickObj)
        return false;

    SdrGrafObj* pGraphic = dynamic_cast<SdrGrafObj*>(pPickObj);
    if (!pGraphic)
        return false;

    return pGraphic->getQrCode() != nullptr;
}

void SwView::AutoCaption(const sal_uInt16 nType, const SvGlobalName *pOleId)
{
    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    bool bWeb = dynamic_cast<SwWebView*>( this ) !=  nullptr;
    if (pModOpt->IsInsWithCaption(bWeb))
    {
        const InsCaptionOpt *pOpt = pModOpt->GetCapOption(bWeb, static_cast<SwCapObjType>(nType), pOleId);
        if (pOpt && pOpt->UseCaption())
            InsertCaption(pOpt);
    }
}

void SwView::InsertCaption(const InsCaptionOpt *pOpt)
{
    if (!pOpt)
        return;

    const OUString &rName = pOpt->GetCategory();

    // Is there a pool template with the same name?
    SwWrtShell &rSh = GetWrtShell();
    if(!rName.isEmpty())
    {
        sal_uInt16 nPoolId = SwStyleNameMapper::GetPoolIdFromUIName(rName, SwGetPoolIdFromName::TxtColl);
        if( USHRT_MAX != nPoolId )
            rSh.GetTextCollFromPool(nPoolId);
            // Pool template does not exist: Does it exist on the document?
        else if( !rSh.GetParaStyle(rName) )
        {
            // It also does not exist in the document: generate
            SwTextFormatColl* pDerivedFrom = rSh.GetTextCollFromPool(RES_POOLCOLL_LABEL);
            rSh.MakeTextFormatColl(rName, pDerivedFrom);
        }
    }

    SelectionType eType = rSh.GetSelectionType();
    if (eType & SelectionType::Ole)
        eType = SelectionType::Graphic;

    const SwLabelType eT = (eType & SelectionType::Table) ? SwLabelType::Table :
                      (eType & SelectionType::Frame) ? SwLabelType::Fly :
                      (eType == SelectionType::Text) ? SwLabelType::Fly :
                      (eType & SelectionType::DrawObject) ? SwLabelType::Draw :
                                                    SwLabelType::Object;

    SwFieldMgr aMgr(&rSh);
    SwSetExpFieldType* pFieldType =
            static_cast<SwSetExpFieldType*>(aMgr.GetFieldType(SwFieldIds::SetExp, rName));
    if (!pFieldType && !rName.isEmpty() )
    {
        // Create new field types
        SwSetExpFieldType aSwSetExpFieldType(rSh.GetDoc(), rName, nsSwGetSetExpType::GSE_SEQ);
        aMgr.InsertFieldType(aSwSetExpFieldType);
        pFieldType = static_cast<SwSetExpFieldType*>(aMgr.GetFieldType(SwFieldIds::SetExp, rName));
    }

    if (!pOpt->IgnoreSeqOpts())
    {
        if (pFieldType)
        {
            pFieldType->SetDelimiter(pOpt->GetSeparator());
            pFieldType->SetOutlineLvl( static_cast< sal_uInt8 >(pOpt->GetLevel()) );
        }
    }

    sal_uInt16       nID    = USHRT_MAX;
    SwFieldType* pType  = nullptr;
    const size_t nCount = aMgr.GetFieldTypeCount();
    if( !rName.isEmpty() )
    {
        for (size_t i = 0; i < nCount; ++i)
        {
            pType = aMgr.GetFieldType(SwFieldIds::Unknown, i);
            OUString aTmpName( pType->GetName() );
            if (aTmpName == rName && pType->Which() == SwFieldIds::SetExp)
            {
                nID = i;
                OSL_ENSURE(nID==i, "Downcasting to sal_uInt16 lost information!");
                break;
            }
        }
    }
    rSh.StartAllAction();

    GetWrtShell().InsertLabel( eT,
                                pOpt->GetCaption(),
                                !pOpt->IgnoreSeqOpts() ? OUString() : pOpt->GetSeparator(),
                                pOpt->GetNumSeparator(),
                                !pOpt->GetPos(),
                                nID,
                                pOpt->GetCharacterStyle(),
                                pOpt->CopyAttributes() );
    // Set Number Format
    if(pType)
        static_cast<SwSetExpFieldType*>(pType)->SetSeqFormat(pOpt->GetNumType());

    rSh.UpdateExpFields( true );

    rSh.EndAllAction();

    if ( rSh.IsFrameSelected() )
    {
        GetEditWin().StopInsFrame();
        rSh.EnterSelFrameMode();
    }

    // remember category
    if (eType & SelectionType::Graphic)
        SetOldGrfCat(rName);
    else if( eType & SelectionType::Table)
        SetOldTabCat(rName);
    else if( eType & SelectionType::Frame)
        SetOldFrameCat(rName);
    else if( eType == SelectionType::Text)
        SetOldFrameCat(rName);
    else if( eType & SelectionType::DrawObject)
        SetOldDrwCat(rName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
