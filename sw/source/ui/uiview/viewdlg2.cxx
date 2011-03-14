/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objface.hxx>
#include <fldmgr.hxx>
#include <expfld.hxx>
#include <modcfg.hxx>

#include <tools/shl.hxx>

#include "swmodule.hxx"
#include "view.hxx"
#include "wview.hxx"
#include "wrtsh.hxx"
#include "cmdid.h"
#include "caption.hxx"
#include "poolfmt.hxx"
#include "edtwin.hxx"
#include <SwStyleNameMapper.hxx>

#include "swabstdlg.hxx"
#include "frmui.hrc"
#include "misc.hrc"

#include "view.hrc"

extern String* pOldGrfCat;
extern String* pOldTabCat;
extern String* pOldFrmCat;
extern String* pOldDrwCat;

void SwView::ExecDlgExt(SfxRequest &rReq)
{
    Window *pMDI = &GetViewFrame()->GetWindow();

    switch ( rReq.GetSlot() )
    {
        case FN_INSERT_CAPTION:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

            VclAbstractDialog* pDialog = pFact->CreateSwCaptionDialog( pMDI, *this, DLG_CAPTION );
            OSL_ENSURE(pDialog, "Dialogdiet fail!");
            if ( pDialog )
            {
                pDialog->Execute();
                delete pDialog;
            }
            break;
        }
        case  FN_EDIT_FOOTNOTE:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "Dialogdiet fail!");
            AbstractInsFootNoteDlg* pDlg = pFact->CreateInsFootNoteDlg( DLG_INS_FOOTNOTE,
                                                        pMDI, *pWrtShell, sal_True );
            OSL_ENSURE(pDlg, "Dialogdiet fail!");

            pDlg->SetHelpId(GetStaticInterface()->GetSlot(FN_EDIT_FOOTNOTE)->GetCommand());
            pDlg->SetText( SW_RESSTR(STR_EDIT_FOOTNOTE) );
            pDlg->Execute();
            delete pDlg;
            break;
        }
    }
}

void SwView::AutoCaption(const sal_uInt16 nType, const SvGlobalName *pOleId)
{
    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    sal_Bool bWeb = 0 != PTR_CAST(SwWebView, this);
    if (pModOpt->IsInsWithCaption(bWeb))
    {
        const InsCaptionOpt *pOpt = pModOpt->GetCapOption(bWeb, (SwCapObjType)nType, pOleId);
        if (pOpt && pOpt->UseCaption() == sal_True)
            InsertCaption(pOpt);
    }
}

void SwView::InsertCaption(const InsCaptionOpt *pOpt)
{
    if (!pOpt)
        return;

    const String &rName = pOpt->GetCategory();

    // Existiert Pool-Vorlage gleichen Namens?
    SwWrtShell &rSh = GetWrtShell();
    if(rName.Len())
    {
        sal_uInt16 nPoolId = SwStyleNameMapper::GetPoolIdFromUIName(rName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL);
        if( USHRT_MAX != nPoolId )
            rSh.GetTxtCollFromPool(nPoolId);
            // Pool-Vorlage existiert nicht: Existiert sie am Dokument?
        else if( !rSh.GetParaStyle(rName) )
        {
            // Sie existiert auch nicht am Dokument: erzeugen
            SwTxtFmtColl* pDerivedFrom = rSh.GetTxtCollFromPool(RES_POOLCOLL_LABEL);
            rSh.MakeTxtFmtColl(rName, pDerivedFrom);
        }
    }

    SelectionType eType = rSh.GetSelectionType();
    if (eType & nsSelectionType::SEL_OLE)
        eType = nsSelectionType::SEL_GRF;

    // SwLabelType
    const SwLabelType eT = eType & nsSelectionType::SEL_TBL ? LTYPE_TABLE :
                      eType & nsSelectionType::SEL_FRM ? LTYPE_FLY :
                      eType == nsSelectionType::SEL_TXT ? LTYPE_FLY :
                      eType & nsSelectionType::SEL_DRW ? LTYPE_DRAW :
                                                    LTYPE_OBJECT;

    SwFldMgr aMgr(&rSh);
    SwSetExpFieldType* pFldType =
            (SwSetExpFieldType*)aMgr.GetFldType(RES_SETEXPFLD, rName);
    if (!pFldType && rName.Len() )
    {
        // Neuen Feldtypen erzeugen
        SwSetExpFieldType aSwSetExpFieldType(rSh.GetDoc(), rName, nsSwGetSetExpType::GSE_SEQ);
        aMgr.InsertFldType(aSwSetExpFieldType);
        pFldType = (SwSetExpFieldType*)aMgr.GetFldType(RES_SETEXPFLD, rName);
    }

    if (!pOpt->IgnoreSeqOpts())
    {
        if (pFldType)
        {
            pFldType->SetDelimiter(pOpt->GetSeparator());
            pFldType->SetOutlineLvl( static_cast< sal_uInt8 >(pOpt->GetLevel()) );
        }
    }

    sal_uInt16       nID    = USHRT_MAX;
    SwFieldType* pType  = 0;
    const sal_uInt16 nCount = aMgr.GetFldTypeCount();
    if( rName.Len() )
    {
        for (sal_uInt16 i = 0; i < nCount; ++i)
        {
            pType = aMgr.GetFldType(USHRT_MAX, i);
            String aTmpName( pType->GetName() );
            if (aTmpName == rName && pType->Which() == RES_SETEXPFLD)
            {
                nID = i;
                break;
            }
        }
    }
    rSh.StartAllAction();

    GetWrtShell().InsertLabel( eT,
                                pOpt->GetCaption(),
                                !pOpt->IgnoreSeqOpts() ? aEmptyStr : pOpt->GetSeparator(),
                                pOpt->GetNumSeparator(),
                                !pOpt->GetPos(),
                                nID,
                                pOpt->GetCharacterStyle(),
                                pOpt->CopyAttributes() );
    // Nummernformat setzen
    if(pType)
        ((SwSetExpFieldType*)pType)->SetSeqFormat(pOpt->GetNumType());

    rSh.UpdateExpFlds( sal_True );

    rSh.EndAllAction();

    if ( rSh.IsFrmSelected() )
    {
        GetEditWin().StopInsFrm();
        rSh.EnterSelFrmMode();
    }

    // Kategorie merken
    String** ppStr = 0;
    if (eType & nsSelectionType::SEL_GRF)
        ppStr = &pOldGrfCat;
    else if( eType & nsSelectionType::SEL_TBL)
        ppStr = &pOldTabCat;
    else if( eType & nsSelectionType::SEL_FRM)
        ppStr = &pOldFrmCat;
    else if( eType == nsSelectionType::SEL_TXT)
        ppStr = &pOldFrmCat;
    else if( eType & nsSelectionType::SEL_DRW)
        ppStr = &pOldDrwCat;

    if( ppStr )
    {
        if( !*ppStr )
            *ppStr = new String( rName );
        else
            **ppStr = rName;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
