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

#include "doc.hxx"
#include "editsh.hxx"
#include "swtable.hxx"
#include "pam.hxx"
#include <docary.hxx>
#include <fchrfmt.hxx>
#include <frmfmt.hxx>
#include <charfmt.hxx>
#include "ndtxt.hxx"    // for GetXXXFmt
#include "hints.hxx"

/*************************************
 * Formate
 *************************************/
// Char
// OPT: inline


sal_uInt16 SwEditShell::GetCharFmtCount() const
{
    return GetDoc()->GetCharFmts()->size();
}


SwCharFmt& SwEditShell::GetCharFmt(sal_uInt16 nFmt) const
{
    return *((*(GetDoc()->GetCharFmts()))[nFmt]);
}


SwCharFmt* SwEditShell::GetCurCharFmt() const
{
    SwCharFmt *pFmt = 0;
    SfxItemSet aSet( GetDoc()->GetAttrPool(), RES_TXTATR_CHARFMT,
                                                RES_TXTATR_CHARFMT );
    const SfxPoolItem* pItem;
    if( GetCurAttr( aSet ) && SFX_ITEM_SET ==
        aSet.GetItemState( RES_TXTATR_CHARFMT, sal_False, &pItem ) )
        pFmt = ((SwFmtCharFmt*)pItem)->GetCharFmt();

    return pFmt;
}


void SwEditShell::FillByEx(SwCharFmt* pCharFmt, sal_Bool bReset)
{
    if ( bReset )
    {
        // #i73790# - method renamed
        pCharFmt->ResetAllFmtAttr();
    }

    SwPaM* pPam = GetCrsr();
    const SwCntntNode* pCNd = pPam->GetCntntNode();
    if( pCNd->IsTxtNode() )
    {
        SwTxtNode const*const pTxtNode(static_cast<SwTxtNode const*>(pCNd));
        xub_StrLen nStt, nEnd;
        if( pPam->HasMark() )
        {
            const SwPosition* pPtPos = pPam->GetPoint();
            const SwPosition* pMkPos = pPam->GetMark();
            if( pPtPos->nNode == pMkPos->nNode )        // in the same node?
            {
                nStt = pPtPos->nContent.GetIndex();
                if( nStt < pMkPos->nContent.GetIndex() )
                    nEnd = pMkPos->nContent.GetIndex();
                else
                {
                    nEnd = nStt;
                    nStt = pMkPos->nContent.GetIndex();
                }
            }
            else
            {
                nStt = pMkPos->nContent.GetIndex();
                if( pPtPos->nNode < pMkPos->nNode )
                {
                    nEnd = nStt;
                    nStt = 0;
                }
                else
                    nEnd = pTxtNode->GetTxt().getLength();
            }
        }
        else
            nStt = nEnd = pPam->GetPoint()->nContent.GetIndex();

        SfxItemSet aSet( mpDoc->GetAttrPool(),
                            pCharFmt->GetAttrSet().GetRanges() );
        pTxtNode->GetAttr( aSet, nStt, nEnd );
        pCharFmt->SetFmtAttr( aSet );
    }
    else if( pCNd->HasSwAttrSet() )
        pCharFmt->SetFmtAttr( *pCNd->GetpSwAttrSet() );
}

// Frm
sal_uInt16 SwEditShell::GetTblFrmFmtCount(bool bUsed) const
{
    return GetDoc()->GetTblFrmFmtCount(bUsed);
}

SwFrmFmt& SwEditShell::GetTblFrmFmt(sal_uInt16 nFmt, bool bUsed ) const
{
    return GetDoc()->GetTblFrmFmt(nFmt, bUsed );
}

String SwEditShell::GetUniqueTblName() const
{
    return GetDoc()->GetUniqueTblName();
}


SwCharFmt* SwEditShell::MakeCharFmt( const String& rName,
                                    SwCharFmt* pDerivedFrom )
{
    if( !pDerivedFrom )
        pDerivedFrom = GetDoc()->GetDfltCharFmt();

    return GetDoc()->MakeCharFmt( rName, pDerivedFrom );
}

//----------------------------------
// inlines in product


SwTxtFmtColl* SwEditShell::GetTxtCollFromPool( sal_uInt16 nId )
{
    return GetDoc()->GetTxtCollFromPool( nId );
}


    // return the demanded automatic format - base-class !
SwFmt* SwEditShell::GetFmtFromPool( sal_uInt16 nId )
{
    return GetDoc()->GetFmtFromPool( nId );
}


SwPageDesc* SwEditShell::GetPageDescFromPool( sal_uInt16 nId )
{
    return GetDoc()->GetPageDescFromPool( nId );
}


bool SwEditShell::IsUsed( const SwModify& rModify ) const
{
    return mpDoc->IsUsed( rModify );
}

const SwFlyFrmFmt* SwEditShell::FindFlyByName( const String& rName, sal_uInt8 nNdTyp ) const
{
    return mpDoc->FindFlyByName(rName, nNdTyp);
}

SwCharFmt* SwEditShell::FindCharFmtByName( const String& rName ) const
{
    return mpDoc->FindCharFmtByName( rName );
}

SwTxtFmtColl* SwEditShell::FindTxtFmtCollByName( const String& rName ) const
{
    return mpDoc->FindTxtFmtCollByName( rName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
