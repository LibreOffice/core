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
#include <editeng/formatbreakitem.hxx>
#include <editsh.hxx>
#include <doc.hxx>      // fuer SwTxtFmtColls
#include <IDocumentUndoRedo.hxx>
#include <edimp.hxx>    // fuer MACROS
#include <ndtxt.hxx>
#include <paratr.hxx>
#include <fmtpdsc.hxx>
#include <viewopt.hxx>  // Formular view
#include <SwRewriter.hxx>
#include <numrule.hxx>
#include <swundo.hxx>
#include <docary.hxx>

SwTxtFmtColl& SwEditShell::GetDfltTxtFmtColl() const
{
    return *((SwTxtFmtColl*) (GetDoc()->GetDfltTxtFmtColl()));
}

sal_uInt16 SwEditShell::GetTxtFmtCollCount() const
{
    return GetDoc()->GetTxtFmtColls()->size();
}

SwTxtFmtColl& SwEditShell::GetTxtFmtColl( sal_uInt16 nFmtColl) const
{
    return *((*(GetDoc()->GetTxtFmtColls()))[nFmtColl]);
}

// #i62675#
void SwEditShell::SetTxtFmtColl( SwTxtFmtColl *pFmt,
                                 bool bResetListAttrs )
{
    SwTxtFmtColl *pLocal = pFmt? pFmt: (*GetDoc()->GetTxtFmtColls())[0];
    StartAllAction();

    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1, pLocal->GetName());

    GetDoc()->GetIDocumentUndoRedo().StartUndo(UNDO_SETFMTCOLL, &aRewriter);
    FOREACHPAM_START(GetCrsr())

        if( !PCURCRSR->HasReadonlySel(
                    // Formular view
                    GetViewOptions()->IsFormView() ) )
            GetDoc()->SetTxtFmtColl( *PCURCRSR, pLocal, true, bResetListAttrs );

    FOREACHPAM_END()
    GetDoc()->GetIDocumentUndoRedo().EndUndo(UNDO_SETFMTCOLL, &aRewriter);
    EndAllAction();
}

SwTxtFmtColl* SwEditShell::MakeTxtFmtColl(const String& rFmtCollName,
        SwTxtFmtColl* pParent)
{
    SwTxtFmtColl *pColl;
    if ( pParent == 0 )
        pParent = &GetTxtFmtColl(0);
    if (  (pColl=GetDoc()->MakeTxtFmtColl(rFmtCollName, pParent)) == 0 )
    {
        OSL_FAIL( "MakeTxtFmtColl failed" );
    }
    return pColl;

}

void SwEditShell::FillByEx(SwTxtFmtColl* pColl, sal_Bool bReset)
{
    if( bReset )
    {
        pColl->ResetAllFmtAttr();
    }

    SwPaM * pCrsr = GetCrsr();
    SwCntntNode * pCnt = pCrsr->GetCntntNode();
    const SfxItemSet* pSet = pCnt->GetpSwAttrSet();
    if( pSet )
    {
        // JP 05.10.98: Special treatment if one of the attribues Break/PageDesc/NumRule(auto) is
        //      in the ItemSet. Otherwise there will be too much or wrong processing (NumRules!)
        //      Bug 57568

        // Do NOT copy AutoNumRules into the template
        const SfxPoolItem* pItem;
        const SwNumRule* pRule = 0;
        if( SFX_ITEM_SET == pSet->GetItemState( RES_BREAK, sal_False ) ||
            SFX_ITEM_SET == pSet->GetItemState( RES_PAGEDESC,sal_False ) ||
            ( SFX_ITEM_SET == pSet->GetItemState( RES_PARATR_NUMRULE,
                sal_False, &pItem ) && 0 != (pRule = GetDoc()->FindNumRulePtr(
                ((SwNumRuleItem*)pItem)->GetValue() )) &&
                pRule && pRule->IsAutoRule() )
            )
        {
            SfxItemSet aSet( *pSet );
            aSet.ClearItem( RES_BREAK );
            aSet.ClearItem( RES_PAGEDESC );

            if( pRule || (SFX_ITEM_SET == pSet->GetItemState( RES_PARATR_NUMRULE,
                sal_False, &pItem ) && 0 != (pRule = GetDoc()->FindNumRulePtr(
                ((SwNumRuleItem*)pItem)->GetValue() )) &&
                pRule && pRule->IsAutoRule() ))
                aSet.ClearItem( RES_PARATR_NUMRULE );

            if( aSet.Count() )
                GetDoc()->ChgFmt(*pColl, aSet );
        }
        else
            GetDoc()->ChgFmt(*pColl, *pSet );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
