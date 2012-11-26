/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

/* -----------------06.11.98 13:45-------------------
 *
 * --------------------------------------------------*/

void SwView::ExecDlgExt(SfxRequest &rReq)
{
    Window *pMDI = &GetViewFrame()->GetWindow();

    switch ( rReq.GetSlot() )
    {
        case FN_INSERT_CAPTION:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "SwAbstractDialogFactory fail!");

            VclAbstractDialog* pDialog = pFact->CreateSwCaptionDialog( pMDI, *this, DLG_CAPTION );
            DBG_ASSERT(pDialog, "Dialogdiet fail!");
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
            DBG_ASSERT(pFact, "Dialogdiet fail!");
            AbstractInsFootNoteDlg* pDlg = pFact->CreateInsFootNoteDlg( DLG_INS_FOOTNOTE,
                                                        pMDI, *pWrtShell, sal_True );
            DBG_ASSERT(pDlg, "Dialogdiet fail!");

            pDlg->SetHelpId(GetStaticInterface()->GetSlot(FN_EDIT_FOOTNOTE)->GetCommand());
            pDlg->SetText( SW_RESSTR(STR_EDIT_FOOTNOTE) );
            pDlg->Execute();
            delete pDlg;
            break;
        }
    }
}

/* -----------------06.11.98 14:53-------------------
 *
 * --------------------------------------------------*/

void SwView::AutoCaption(const sal_uInt16 nType, const SvGlobalName *pOleId)
{
    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    sal_Bool bWeb = 0 != dynamic_cast< SwWebView* >( this);
    if (pModOpt->IsInsWithCaption(bWeb))
    {
        const InsCaptionOpt *pOpt = pModOpt->GetCapOption(bWeb, (SwCapObjType)nType, pOleId);
        if (pOpt && pOpt->UseCaption() == sal_True)
            InsertCaption(pOpt);
    }
}

/* -----------------06.11.98 12:58-------------------
 *
 * --------------------------------------------------*/

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


