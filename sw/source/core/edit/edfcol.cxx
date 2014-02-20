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


#include <hintids.hxx>
#include <editeng/brkitem.hxx>
#include <editsh.hxx>
#include <doc.hxx>      // fuer SwTxtFmtColls
#include <IDocumentUndoRedo.hxx>
#include <edimp.hxx>    // fuer MACROS
#include <ndtxt.hxx>
#include <paratr.hxx>
#include <fmtpdsc.hxx>
// --> FME 2004-06-29 #114856# Formular view
#include <viewopt.hxx>
// <--
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

        if ( !PCURCRSR->HasReadonlySel( GetViewOptions()->IsFormView() ) )
        {
            GetDoc()->SetTxtFmtColl( *PCURCRSR, pLocal, true, bResetListAttrs );
        }

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
        ASSERT( sal_False, "MakeTxtFmtColl failed" )
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




