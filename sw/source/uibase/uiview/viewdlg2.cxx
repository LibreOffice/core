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
#include <fldmgr.hxx>
#include <expfld.hxx>
#include <modcfg.hxx>

#include "swmodule.hxx"
#include "view.hxx"
#include "wview.hxx"
#include "wrtsh.hxx"
#include "cmdid.h"
#include "caption.hxx"
#include "poolfmt.hxx"
#include "edtwin.hxx"
#include <SwStyleNameMapper.hxx>

#include "initui.hxx"
#include "swabstdlg.hxx"
#include "frmui.hrc"
#include "misc.hrc"

#include "view.hrc"

#include <memory>

void SwView::ExecDlgExt(SfxRequest &rReq)
{
    vcl::Window *pMDI = &GetViewFrame()->GetWindow();

    switch ( rReq.GetSlot() )
    {
        case FN_INSERT_CAPTION:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            assert(pFact && "SwAbstractDialogFactory fail!");

            std::unique_ptr<VclAbstractDialog> pDialog(pFact->CreateSwCaptionDialog( pMDI, *this, DLG_CAPTION ));
            assert(pDialog && "Dialog creation failed!");
            if ( pDialog )
            {
                pDialog->Execute();
            }
            break;
        }
        case  FN_EDIT_FOOTNOTE:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            assert(pFact && "Dialog creation failed!");
            std::unique_ptr<AbstractInsFootNoteDlg> pDlg(pFact->CreateInsFootNoteDlg(
                pMDI, *m_pWrtShell, true));
            assert(pDlg && "Dialog creation failed!");

            pDlg->SetHelpId(GetStaticInterface()->GetSlot(FN_EDIT_FOOTNOTE)->GetCommand());
            pDlg->SetText( SW_RESSTR(STR_EDIT_FOOTNOTE) );
            pDlg->Execute();
            break;
        }
    }
}

void SwView::AutoCaption(const sal_uInt16 nType, const SvGlobalName *pOleId)
{
    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    bool bWeb = dynamic_cast<SwWebView*>( this ) !=  nullptr;
    if (pModOpt->IsInsWithCaption(bWeb))
    {
        const InsCaptionOpt *pOpt = pModOpt->GetCapOption(bWeb, (SwCapObjType)nType, pOleId);
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
        sal_uInt16 nPoolId = SwStyleNameMapper::GetPoolIdFromUIName(rName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL);
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
    if (eType & nsSelectionType::SEL_OLE)
        eType = nsSelectionType::SEL_GRF;

    const SwLabelType eT = eType & nsSelectionType::SEL_TBL ? LTYPE_TABLE :
                      eType & nsSelectionType::SEL_FRM ? LTYPE_FLY :
                      eType == nsSelectionType::SEL_TXT ? LTYPE_FLY :
                      eType & nsSelectionType::SEL_DRW ? LTYPE_DRAW :
                                                    LTYPE_OBJECT;

    SwFieldMgr aMgr(&rSh);
    SwSetExpFieldType* pFieldType =
            static_cast<SwSetExpFieldType*>(aMgr.GetFieldType(RES_SETEXPFLD, rName));
    if (!pFieldType && !rName.isEmpty() )
    {
        // Create new field types
        SwSetExpFieldType aSwSetExpFieldType(rSh.GetDoc(), rName, nsSwGetSetExpType::GSE_SEQ);
        aMgr.InsertFieldType(aSwSetExpFieldType);
        pFieldType = static_cast<SwSetExpFieldType*>(aMgr.GetFieldType(RES_SETEXPFLD, rName));
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
            pType = aMgr.GetFieldType(USHRT_MAX, i);
            OUString aTmpName( pType->GetName() );
            if (aTmpName == rName && pType->Which() == RES_SETEXPFLD)
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
    if (eType & nsSelectionType::SEL_GRF)
        SetOldGrfCat(rName);
    else if( eType & nsSelectionType::SEL_TBL)
        SetOldTabCat(rName);
    else if( eType & nsSelectionType::SEL_FRM)
        SetOldFrameCat(rName);
    else if( eType == nsSelectionType::SEL_TXT)
        SetOldFrameCat(rName);
    else if( eType & nsSelectionType::SEL_DRW)
        SetOldDrwCat(rName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
