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


#include <hintids.hxx>
#include <editeng/brkitem.hxx>
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

/*************************************
 * FormatColl
 *************************************/
// TXT


SwTxtFmtColl& SwEditShell::GetDfltTxtFmtColl() const
{
    return *((SwTxtFmtColl*) (GetDoc()->GetDfltTxtFmtColl()));
}


sal_uInt16 SwEditShell::GetTxtFmtCollCount() const
{
    return GetDoc()->GetTxtFmtColls()->Count();
}


SwTxtFmtColl& SwEditShell::GetTxtFmtColl( sal_uInt16 nFmtColl) const
{
    return *((*(GetDoc()->GetTxtFmtColls()))[nFmtColl]);
}

// --> OD 2007-11-06 #i62675#
void SwEditShell::SetTxtFmtColl( SwTxtFmtColl *pFmt,
                                 bool bResetListAttrs )
{
    SwTxtFmtColl *pLocal = pFmt? pFmt: (*GetDoc()->GetTxtFmtColls())[0];
    StartAllAction();

    SwRewriter aRewriter;
    aRewriter.AddRule(UNDO_ARG1, pLocal->GetName());

    GetDoc()->GetIDocumentUndoRedo().StartUndo(UNDO_SETFMTCOLL, &aRewriter);
    FOREACHPAM_START(this)

        if( !PCURCRSR->HasReadonlySel(
                    // Formular view
                    GetViewOptions()->IsFormView() ) )
            GetDoc()->SetTxtFmtColl( *PCURCRSR, pLocal, true, bResetListAttrs );

    FOREACHPAM_END()
    GetDoc()->GetIDocumentUndoRedo().EndUndo(UNDO_SETFMTCOLL, &aRewriter);
    EndAllAction();
}
// <--


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
        // --> OD 2007-01-25 #i73790# - method renamed
        pColl->ResetAllFmtAttr();
        // <--
    }

    SwPaM * pCrsr = GetCrsr();
    SwCntntNode * pCnt = pCrsr->GetCntntNode();
    const SfxItemSet* pSet = pCnt->GetpSwAttrSet();
    if( pSet )
    {
        // JP 05.10.98: falls eines der Attribute Break/PageDesc/NumRule(auto)
        //      im ItemSet steht, so entferne die VORM setzen an der Collection.
        //      Ansonst wird zu viel gemacht oder falsche gemacht (NumRules!)
        //      Bug 57568

        // AutoNumRules NICHT in die Vorlagen uebernehmen
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
