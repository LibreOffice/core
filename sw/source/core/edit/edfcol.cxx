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
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <edimp.hxx>
#include <ndtxt.hxx>
#include <paratr.hxx>
#include <fmtpdsc.hxx>
#include <viewopt.hxx>
#include <SwRewriter.hxx>
#include <numrule.hxx>
#include <swundo.hxx>
#include <docary.hxx>

SwTextFormatColl& SwEditShell::GetDfltTextFormatColl() const
{
    return *static_cast<SwTextFormatColl*>( (GetDoc()->GetDfltTextFormatColl()));
}

sal_uInt16 SwEditShell::GetTextFormatCollCount() const
{
    return GetDoc()->GetTextFormatColls()->size();
}

SwTextFormatColl& SwEditShell::GetTextFormatColl( sal_uInt16 nFormatColl) const
{
    return *((*(GetDoc()->GetTextFormatColls()))[nFormatColl]);
}

// #i62675#
void SwEditShell::SetTextFormatColl(SwTextFormatColl *pFormat,
                                const bool bResetListAttrs)
{
    SwTextFormatColl *pLocal = pFormat? pFormat: (*GetDoc()->GetTextFormatColls())[0];
    StartAllAction();

    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1, pLocal->GetName());

    GetDoc()->GetIDocumentUndoRedo().StartUndo(UNDO_SETFMTCOLL, &aRewriter);
    for(SwPaM& rPaM : GetCursor()->GetRingContainer())
    {

        if ( !rPaM.HasReadonlySel( GetViewOptions()->IsFormView() ) )
        {
            // Change the paragraph style to pLocal and remove all direct paragraph formatting.
            GetDoc()->SetTextFormatColl( rPaM, pLocal, true, bResetListAttrs );

            // If there are hints on the nodes which cover the whole node, then remove those, too.
            SwPaM aPaM(*rPaM.Start(), *rPaM.End());
            if (SwTextNode* pEndTextNode = aPaM.End()->nNode.GetNode().GetTextNode())
            {
                aPaM.Start()->nContent = 0;
                aPaM.End()->nContent = pEndTextNode->GetText().getLength();
            }
            GetDoc()->RstTextAttrs(aPaM, /*bInclRefToxMark=*/false, /*bExactRange=*/true);
        }

    }
    GetDoc()->GetIDocumentUndoRedo().EndUndo(UNDO_SETFMTCOLL, &aRewriter);
    EndAllAction();
}

SwTextFormatColl* SwEditShell::MakeTextFormatColl(const OUString& rFormatCollName,
        SwTextFormatColl* pParent)
{
    SwTextFormatColl *pColl;
    if ( pParent == nullptr )
        pParent = &GetTextFormatColl(0);
    if (  (pColl=GetDoc()->MakeTextFormatColl(rFormatCollName, pParent)) == nullptr )
    {
        OSL_FAIL( "MakeTextFormatColl failed" );
    }
    return pColl;

}

void SwEditShell::FillByEx(SwTextFormatColl* pColl, bool bReset)
{
    if( bReset )
    {
        pColl->ResetAllFormatAttr();
    }

    SwPaM * pCursor = GetCursor();
    SwContentNode * pCnt = pCursor->GetContentNode();
    const SfxItemSet* pSet = pCnt->GetpSwAttrSet();
    if( pSet )
    {
        // JP 05.10.98: Special treatment if one of the attribues Break/PageDesc/NumRule(auto) is
        //      in the ItemSet. Otherwise there will be too much or wrong processing (NumRules!)
        //      Bug 57568

        // Do NOT copy AutoNumRules into the template
        const SfxPoolItem* pItem;
        const SwNumRule* pRule = nullptr;
        if( SfxItemState::SET == pSet->GetItemState( RES_BREAK, false ) ||
            SfxItemState::SET == pSet->GetItemState( RES_PAGEDESC,false ) ||
            ( SfxItemState::SET == pSet->GetItemState( RES_PARATR_NUMRULE,
                false, &pItem ) && nullptr != (pRule = GetDoc()->FindNumRulePtr(
                static_cast<const SwNumRuleItem*>(pItem)->GetValue() )) &&
                pRule && pRule->IsAutoRule() )
            )
        {
            SfxItemSet aSet( *pSet );
            aSet.ClearItem( RES_BREAK );
            aSet.ClearItem( RES_PAGEDESC );

            if( pRule || (SfxItemState::SET == pSet->GetItemState( RES_PARATR_NUMRULE,
                false, &pItem ) && nullptr != (pRule = GetDoc()->FindNumRulePtr(
                static_cast<const SwNumRuleItem*>(pItem)->GetValue() )) &&
                pRule && pRule->IsAutoRule() ))
                aSet.ClearItem( RES_PARATR_NUMRULE );

            if( aSet.Count() )
                GetDoc()->ChgFormat(*pColl, aSet );
        }
        else
            GetDoc()->ChgFormat(*pColl, *pSet );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
