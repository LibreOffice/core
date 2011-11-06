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



#include "doc.hxx"
#include "editsh.hxx"
#include "swtable.hxx"
#include "pam.hxx"
#include <docary.hxx>
#include <fchrfmt.hxx>
#include <frmfmt.hxx>
#include <charfmt.hxx>
#include "ndtxt.hxx"    // Fuer GetXXXFmt
#include "hints.hxx"

/*************************************
 * Formate
 *************************************/
// Char
// OPT: inline


sal_uInt16 SwEditShell::GetCharFmtCount() const
{
    return GetDoc()->GetCharFmts()->Count();
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
        // --> OD 2007-01-25 #i73790# - method renamed
        pCharFmt->ResetAllFmtAttr();
        // <--
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
            if( pPtPos->nNode == pMkPos->nNode )        // im selben Node ?
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
sal_uInt16 SwEditShell::GetTblFrmFmtCount(sal_Bool bUsed) const
{
    return GetDoc()->GetTblFrmFmtCount(bUsed);
}

SwFrmFmt& SwEditShell::GetTblFrmFmt(sal_uInt16 nFmt, sal_Bool bUsed ) const
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
// inlines im Product


SwTxtFmtColl* SwEditShell::GetTxtCollFromPool( sal_uInt16 nId )
{
    return GetDoc()->GetTxtCollFromPool( nId );
}


    // return das geforderte automatische  Format - Basis-Klasse !
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
