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
                    nEnd = ((SwTxtNode*)pCNd)->GetTxt().Len();
            }
        }
        else
            nStt = nEnd = pPam->GetPoint()->nContent.GetIndex();

        SfxItemSet aSet( pDoc->GetAttrPool(),
                            pCharFmt->GetAttrSet().GetRanges() );
        ((SwTxtNode*)pCNd)->GetAttr( aSet, nStt, nEnd );
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


sal_Bool SwEditShell::IsUsed( const SwModify& rModify ) const
{
    return pDoc->IsUsed( rModify );
}

const SwFlyFrmFmt* SwEditShell::FindFlyByName( const String& rName, sal_uInt8 nNdTyp ) const
{
    return pDoc->FindFlyByName(rName, nNdTyp);
}

SwCharFmt* SwEditShell::FindCharFmtByName( const String& rName ) const
{
    return pDoc->FindCharFmtByName( rName );
}

SwTxtFmtColl* SwEditShell::FindTxtFmtCollByName( const String& rName ) const
{
    return pDoc->FindTxtFmtCollByName( rName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
