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
#include <sfx2/viewfrm.hxx>
#include <sfx2/objface.hxx>
#include <svx/svdograf.hxx>
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

#include <initui.hxx>
#include <swabstdlg.hxx>

#include <strings.hrc>

#include <memory>

using namespace css;

void SwView::ExecDlgExt(SfxRequest const &rReq)
{
    vcl::Window& rMDI = GetViewFrame()->GetWindow();

    switch ( rReq.GetSlot() )
    {
        case FN_INSERT_CAPTION:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            ScopedVclPtr<VclAbstractDialog> pDialog(pFact->CreateSwCaptionDialog( &rMDI, *this ));
            pDialog->Execute();
            break;
        }
        case SID_INSERT_SIGNATURELINE:
        case SID_EDIT_SIGNATURELINE:
        {
            VclAbstractDialogFactory* pFact = VclAbstractDialogFactory::Create();
            const uno::Reference<frame::XModel> xModel(GetCurrentDocument());
            ScopedVclPtr<AbstractSignatureLineDialog> pDialog(pFact->CreateSignatureLineDialog(
                GetFrameWeld(), xModel, rReq.GetSlot() == SID_EDIT_SIGNATURELINE));
            pDialog->Execute();
            break;
        }
        case SID_SIGN_SIGNATURELINE:
        {
            VclAbstractDialogFactory* pFact = VclAbstractDialogFactory::Create();
            const uno::Reference<frame::XModel> xModel(GetCurrentDocument());
            ScopedVclPtr<AbstractSignSignatureLineDialog> pDialog(
                pFact->CreateSignSignatureLineDialog(GetFrameWeld(), xModel));
            pDialog->Execute();
            break;
        }
        case  FN_EDIT_FOOTNOTE:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            ScopedVclPtr<AbstractInsFootNoteDlg> pDlg(pFact->CreateInsFootNoteDlg(
                GetFrameWeld(), *m_pWrtShell, true));

            pDlg->SetHelpId(GetStaticInterface()->GetSlot(FN_EDIT_FOOTNOTE)->GetCommand());
            pDlg->SetText( SwResId(STR_EDIT_FOOTNOTE) );
            pDlg->Execute();
            break;
        }
    }
}

bool SwView::isSignatureLineSelected()
{
    SwWrtShell& rSh = GetWrtShell();
    SdrView* pSdrView = rSh.GetDrawView();
    if (!pSdrView)
        return false;

    if (pSdrView->GetMarkedObjectCount() != 1)
        return false;

    SdrObject* pPickObj = pSdrView->GetMarkedObjectByIndex(0);
    if (!pPickObj)
        return false;

    SdrGrafObj* pGraphic = dynamic_cast<SdrGrafObj*>(pPickObj);
    if (!pGraphic)
        return false;

    return pGraphic->isSignatureLine();
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

    const SwLabelType eT = (eType & SelectionType::Table) ? LTYPE_TABLE :
                      (eType & SelectionType::Frame) ? LTYPE_FLY :
                      (eType == SelectionType::Text) ? LTYPE_FLY :
                      (eType & SelectionType::DrawObject) ? LTYPE_DRAW :
                                                    LTYPE_OBJECT;

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
