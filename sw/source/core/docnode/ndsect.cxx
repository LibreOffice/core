/*************************************************************************
 *
 *  $RCSfile: ndsect.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-08 21:18:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVXLINKMGR_HXX
#include <svx/linkmgr.hxx>
#endif
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif

#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _CALC_HXX
#include <calc.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _SWSERV_HXX
#include <swserv.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _FTNIDX_HXX
#include <ftnidx.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _NDINDEX_HXX
#include <ndindex.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _SECTFRM_HXX
#include <sectfrm.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _NODE2LAY_HXX
#include <node2lay.hxx>
#endif
#ifndef _DOCTXM_HXX
#include <doctxm.hxx>
#endif
#ifndef _FMTFTNTX_HXX
#include <fmtftntx.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif

#ifndef _COMCORE_HRC
#include <comcore.hrc>
#endif

int lcl_IsInSameTblBox( SwNodes& rNds, const SwNode& rNd,
                            const SwNodeIndex& rIdx2 )
{
    const SwTableNode* pTblNd = rNd.FindTableNode();
    if( !pTblNd )
        return TRUE;

    // dann suche den StartNode der Box
    const SwTableSortBoxes& rSortBoxes = pTblNd->GetTable().GetTabSortBoxes();
    ULONG nIdx = rNd.GetIndex();
    for( USHORT n = 0; n < rSortBoxes.Count(); ++n )
    {
        const SwStartNode* pNd = rSortBoxes[ n ]->GetSttNd();
        if( pNd->GetIndex() < nIdx &&
            nIdx < pNd->EndOfSectionIndex() )
        {
            // dann muss der andere Index in derselben Section liegen
            nIdx = rIdx2.GetIndex();
            return pNd->GetIndex() < nIdx && nIdx < pNd->EndOfSectionIndex();
        }
    }
    return TRUE;
}

void lcl_CheckEmptyLayFrm( SwNodes& rNds, SwSection& rSect,
                        const SwNode& rStt, const SwNode& rEnd )
{
    SwNodeIndex aIdx( rStt );
    if( !rNds.GoPrevSection( &aIdx, TRUE, FALSE ) ||
        !CheckNodesRange( rStt, aIdx, TRUE ) ||
        !lcl_IsInSameTblBox( rNds, rStt, aIdx ))
    {
        aIdx = rEnd;
        if( !rNds.GoNextSection( &aIdx, TRUE, FALSE ) ||
            !CheckNodesRange( rEnd, aIdx, TRUE ) ||
            !lcl_IsInSameTblBox( rNds, rEnd, aIdx ))
            rSect.SetHidden( FALSE );
    }
}

SwSection* SwDoc::Insert( const SwPaM& rRange, const SwSection& rNew,
                            const SfxItemSet* pAttr, BOOL bUpdate )
{
    const SwNode* pPrvNd = 0;
    USHORT nRegionRet = 0;
    if( rRange.HasMark() &&
        0 == ( nRegionRet = IsInsRegionAvailable( rRange, &pPrvNd ) ))
    {
        ASSERT( !this, "Selection ueber verschiedene Sections" );
        return 0;
    }

    // Teste ob das gesamte Dokument versteckt werden soll,
    // koennen wir zur Zeit nicht !!!!
    if( rNew.IsHidden() && rRange.HasMark() )
    {
        const SwPosition *pStt = rRange.Start(), *pEnd = rRange.End();
        if( !pStt->nContent.GetIndex() &&
            pEnd->nNode.GetNode().GetCntntNode()->Len() ==
            pEnd->nContent.GetIndex() )

            ::lcl_CheckEmptyLayFrm( GetNodes(), (SwSection&)rNew,
                                    pStt->nNode.GetNode(),
                                    pEnd->nNode.GetNode() );
    }

    SwUndoInsSection* pUndoInsSect = 0;
    if( DoesUndo() )
    {
        ClearRedo();
        pUndoInsSect = new SwUndoInsSection( rRange, rNew, pAttr );
        AppendUndo( pUndoInsSect );
        DoUndo( FALSE );
    }

    SwSectionFmt* pFmt = MakeSectionFmt( 0 );
    if( pAttr )
        pFmt->SetAttr( *pAttr );

    SwSectionNode* pNewSectNode = 0;

    SwRedlineMode eOld = GetRedlineMode();
    SetRedlineMode_intern( (eOld & ~REDLINE_SHOW_MASK) | REDLINE_IGNORE );

    if( rRange.HasMark() )
    {
        SwPosition *pSttPos = (SwPosition*)rRange.Start(),
                    *pEndPos = (SwPosition*)rRange.End();
        if( pPrvNd && 3 == nRegionRet )
        {
            ASSERT( pPrvNd, "der SectionNode fehlt" );
            SwNodeIndex aStt( pSttPos->nNode ), aEnd( pEndPos->nNode, +1 );
            while( pPrvNd != aStt.GetNode().FindStartNode() )
                aStt--;
            while( pPrvNd != aEnd.GetNode().FindStartNode() )
                aEnd++;

            --aEnd;     // im InsertSection ist Ende inclusive
            pNewSectNode = GetNodes().InsertSection( aStt, *pFmt, rNew, &aEnd );
        }
        else
        {
            if( pUndoInsSect )
            {
                SwTxtNode* pTNd;
                if( !( pPrvNd && 1 == nRegionRet ) &&
                    pSttPos->nContent.GetIndex() &&
                    0 != ( pTNd = pSttPos->nNode.GetNode().GetTxtNode() ))
                    pUndoInsSect->SaveSplitNode( pTNd, TRUE );

                if( !( pPrvNd && 2 == nRegionRet ) &&
                    0 != ( pTNd = pEndPos->nNode.GetNode().GetTxtNode() ) &&
                    pTNd->GetTxt().Len() != pEndPos->nContent.GetIndex() )
                    pUndoInsSect->SaveSplitNode( pTNd, FALSE );
            }

            const SwCntntNode* pCNd;
            if( pPrvNd && 1 == nRegionRet )
            {
                pSttPos->nNode.Assign( *pPrvNd );
                pSttPos->nContent.Assign( pSttPos->nNode.GetNode().GetCntntNode(), 0 );
            }
            else if( pSttPos->nContent.GetIndex() )
                SplitNode( *pSttPos );

            if( pPrvNd && 2 == nRegionRet )
            {
                pEndPos->nNode.Assign( *pPrvNd );
                pEndPos->nContent.Assign( pEndPos->nNode.GetNode().GetCntntNode(), 0 );
            }
            else
            {
                pCNd = pEndPos->nNode.GetNode().GetCntntNode();
                if( pCNd && pCNd->Len() != pEndPos->nContent.GetIndex() )
                {
                    xub_StrLen nCntnt = pSttPos->nContent.GetIndex();
                    SplitNode( *pEndPos );

                    SwTxtNode* pTNd;
                    if( pEndPos->nNode.GetIndex() == pSttPos->nNode.GetIndex() )
                    {
                        pSttPos->nNode--;
                        pEndPos->nNode--;
                        pTNd = pSttPos->nNode.GetNode().GetTxtNode();
                        pSttPos->nContent.Assign( pTNd, nCntnt );
                    }
                    else
                    {
                        // wieder ans Ende vom vorherigen setzen
                        pEndPos->nNode--;
                        pTNd = pEndPos->nNode.GetNode().GetTxtNode();
                    }
                    if( pTNd ) nCntnt = pTNd->GetTxt().Len(); else nCntnt = 0;
                    pEndPos->nContent.Assign( pTNd, nCntnt );
                }
            }
            pNewSectNode = GetNodes().InsertSection( pSttPos->nNode, *pFmt, rNew,
                                                    &pEndPos->nNode );
        }
    }
    else
    {
        const SwPosition* pPos = rRange.GetPoint();
        const SwCntntNode* pCNd = pPos->nNode.GetNode().GetCntntNode();
        if( !pPos->nContent.GetIndex() )
        {
            pNewSectNode = GetNodes().InsertSection( pPos->nNode, *pFmt, rNew, 0, TRUE );
        }
        else if( pPos->nContent.GetIndex() == pCNd->Len() )
        {
            pNewSectNode = GetNodes().InsertSection( pPos->nNode, *pFmt, rNew, 0, FALSE );
        }
        else
        {
            if( pUndoInsSect && pCNd->IsTxtNode() )
                pUndoInsSect->SaveSplitNode( (SwTxtNode*)pCNd, TRUE );
            SplitNode( *pPos );
            pNewSectNode = GetNodes().InsertSection( pPos->nNode, *pFmt, rNew, 0, TRUE );
        }
    }

//FEATURE::CONDCOLL
    pNewSectNode->CheckSectionCondColl();
//FEATURE::CONDCOLL

    SetRedlineMode_intern( eOld );

    if( IsRedlineOn() || (!IsIgnoreRedline() && pRedlineTbl->Count() ))
    {
        SwPaM aPam( *pNewSectNode->EndOfSectionNode(), *pNewSectNode, 1 );
        if( IsRedlineOn() )
            AppendRedline( new SwRedline( REDLINE_INSERT, aPam ));
        else
            SplitRedline( aPam );
    }

    // ist eine Condition gesetzt
    if( rNew.IsHidden() && rNew.GetCondition().Len() )
    {
        // dann berechne bis zu dieser Position
        SwCalc aCalc( *this );
        FldsToCalc( aCalc, pNewSectNode->GetIndex() );
        SwSection& rNewSect = pNewSectNode->GetSection();
        rNewSect.SetCondHidden( aCalc.Calculate( rNewSect.GetCondition() ).GetBool() );
    }

    BOOL bUpdateFtn = FALSE;
    if( GetFtnIdxs().Count() && pAttr )
    {
        USHORT nVal = ((SwFmtFtnAtTxtEnd&)pAttr->Get(
                                            RES_FTN_AT_TXTEND )).GetValue();
           if( ( FTNEND_ATTXTEND_OWNNUMSEQ == nVal ||
              FTNEND_ATTXTEND_OWNNUMANDFMT == nVal ) ||
            ( FTNEND_ATTXTEND_OWNNUMSEQ == ( nVal = ((SwFmtEndAtTxtEnd&)
                            pAttr->Get( RES_END_AT_TXTEND )).GetValue() ) ||
              FTNEND_ATTXTEND_OWNNUMANDFMT == nVal ))
            bUpdateFtn = TRUE;
    }

    if( pUndoInsSect )
    {
        pUndoInsSect->SetSectNdPos( pNewSectNode->GetIndex() );
        pUndoInsSect->SetUpdtFtnFlag( bUpdateFtn );
        DoUndo( TRUE );
    }

    if( rNew.IsLinkType() )
        pNewSectNode->GetSection().CreateLink( bUpdate ? CREATE_UPDATE : CREATE_CONNECT );

    if( bUpdateFtn )
        GetFtnIdxs().UpdateFtn( SwNodeIndex( *pNewSectNode ));

    SetModified();
    return &pNewSectNode->GetSection();
}

USHORT SwDoc::IsInsRegionAvailable( const SwPaM& rRange,
                                const SwNode** ppSttNd ) const
{
    USHORT nRet = 1;
    if( rRange.HasMark() )
    {
        // teste ob es sich um eine gueltige Selektion handelt
        const SwPosition* pStt = rRange.Start(),
                        * pEnd = rRange.End();

        const SwCntntNode* pCNd = pEnd->nNode.GetNode().GetCntntNode();
        const SwNode* pNd = &pStt->nNode.GetNode();
        const SwSectionNode* pSectNd = pNd->FindSectionNode();
        const SwSectionNode* pEndSectNd = pCNd->FindSectionNode();
        if( pSectNd && pEndSectNd && pSectNd != pCNd->FindSectionNode() )
        {
            // versuche eine umschliessende Section zu erzeugen
            // Aber, nur wenn der Start am Sectionanfang und das Ende am
            // Section Ende liegt!
            nRet = 0;
            if( !pStt->nContent.GetIndex() && pSectNd->GetIndex()
                == pStt->nNode.GetIndex() - 1 && pEnd->nContent.GetIndex() ==
                pCNd->Len() )
            {
                SwNodeIndex aIdx( pStt->nNode, -1 );
                ULONG nCmp = pEnd->nNode.GetIndex();
                const SwStartNode* pPrvNd;
                const SwEndNode* pNxtNd;
                while( 0 != ( pPrvNd = (pNd = &aIdx.GetNode())->GetSectionNode() ) &&
                    !( aIdx.GetIndex() < nCmp &&
                        nCmp < pPrvNd->EndOfSectionIndex() ) )
                {
                    aIdx--;
                }
                if( !pPrvNd )
                    pPrvNd = pNd->IsStartNode() ? (SwStartNode*)pNd
                                                : pNd->FindStartNode();

                aIdx = pEnd->nNode.GetIndex() + 1;
                nCmp = pStt->nNode.GetIndex();
                while( 0 != ( pNxtNd = (pNd = &aIdx.GetNode())->GetEndNode() ) &&
                    pNxtNd->FindStartNode()->IsSectionNode() &&
                    !( pNxtNd->StartOfSectionIndex() < nCmp &&
                        nCmp < aIdx.GetIndex() ) )
                {
                    aIdx++;
                }
                if( !pNxtNd )
                    pNxtNd = pNd->EndOfSectionNode();

                if( pPrvNd && pNxtNd && pPrvNd == pNxtNd->FindStartNode() )
                {
                    nRet = 3;

                    if( ppSttNd )
                        *ppSttNd = pPrvNd;
                }
            }
        }
        else if( !pSectNd && pEndSectNd )
        {
            // versuche eine umschliessende Section zu erzeugen
            // Aber, nur wenn das Ende am Section Ende liegt!
            nRet = 0;
            if( pEnd->nContent.GetIndex() == pCNd->Len() )
            {
                SwNodeIndex aIdx( pEnd->nNode, 1 );
                if( aIdx.GetNode().IsEndNode() &&
                        0 != aIdx.GetNode().FindSectionNode() )
                {
                    do {
                        aIdx++;
                    } while( aIdx.GetNode().IsEndNode() &&
                                0 != aIdx.GetNode().FindSectionNode() );
//                  if( !aIdx.GetNode().IsEndNode() )
                    {
                        nRet = 2;
                        if( ppSttNd )
                        {
                            aIdx--;
                            *ppSttNd = &aIdx.GetNode();
                        }
                    }
                }
            }
        }
        else if( pSectNd && !pEndSectNd )
        {
            // versuche eine umschliessende Section zu erzeugen
            // Aber, nur wenn der Start am Section Anfang liegt!
            nRet = 0;
            if( !pStt->nContent.GetIndex() )
            {
                SwNodeIndex aIdx( pStt->nNode, -1 );
                if( aIdx.GetNode().IsSectionNode() )
                {
                    do {
                        aIdx--;
                    } while( aIdx.GetNode().IsSectionNode() );
                    if( !aIdx.GetNode().IsSectionNode() )
                    {
                        nRet = 1;
                        if( ppSttNd )
                        {
                            aIdx++;
                            *ppSttNd = &aIdx.GetNode();
                        }
                    }
                }
            }
        }
    }
    return nRet;
}

SwSection* SwDoc::GetCurrSection( const SwPosition& rPos ) const
{
    const SwSectionNode* pSectNd = rPos.nNode.GetNode().FindSectionNode();
    if( pSectNd )
        return (SwSection*)&pSectNd->GetSection();
    return 0;
}

SwSectionFmt* SwDoc::MakeSectionFmt( SwSectionFmt *pDerivedFrom )
{
    if( !pDerivedFrom )
        pDerivedFrom = (SwSectionFmt*)pDfltFrmFmt;
    SwSectionFmt* pNew = new SwSectionFmt( pDerivedFrom, this );
    pSectionFmtTbl->Insert( pNew, pSectionFmtTbl->Count() );
    return pNew;
}

void SwDoc::DelSectionFmt( SwSectionFmt *pFmt, BOOL bDelNodes )
{
    USHORT nPos = pSectionFmtTbl->GetPos( pFmt );
    if( USHRT_MAX != nPos )
    {
        const SwNodeIndex* pIdx = pFmt->GetCntnt( FALSE ).GetCntntIdx();
        const SfxPoolItem* pFtnEndAtTxtEnd;
        if( SFX_ITEM_SET != pFmt->GetItemState(
                            RES_FTN_AT_TXTEND, TRUE, &pFtnEndAtTxtEnd ) ||
            SFX_ITEM_SET != pFmt->GetItemState(
                            RES_END_AT_TXTEND, TRUE, &pFtnEndAtTxtEnd ))
            pFtnEndAtTxtEnd = 0;

        const SwSectionNode* pSectNd;

        if( DoesUndo() )
        {
            ClearRedo();
            if( bDelNodes && pIdx && &GetNodes() == &pIdx->GetNodes() &&
                0 != (pSectNd = pIdx->GetNode().GetSectionNode() ))
            {
                SwNodeIndex aUpdIdx( *pIdx );
                ClearRedo();
                SwPaM aPaM( *pSectNd->EndOfSectionNode(), *pSectNd );
                AppendUndo( new SwUndoDelete( aPaM ));
                if( pFtnEndAtTxtEnd )
                    GetFtnIdxs().UpdateFtn( aUpdIdx );
                SetModified();
                return ;
            }
            AppendUndo( new SwUndoDelSection( *pFmt ) );
        }
        else if( bDelNodes && pIdx && &GetNodes() == &pIdx->GetNodes() &&
                0 != (pSectNd = pIdx->GetNode().GetSectionNode() ))
        {
            SwNodeIndex aUpdIdx( *pIdx );
            DeleteSection( (SwNode*)pSectNd );
            if( pFtnEndAtTxtEnd )
                GetFtnIdxs().UpdateFtn( aUpdIdx );
            SetModified();
            return ;
        }

        {
            SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT, pFmt );
            pFmt->Modify( &aMsgHint, &aMsgHint );
        }

        // ACHTUNG: erst aus dem Array entfernen und dann loeschen.
        //          Der Section-DTOR versucht selbst noch sein Format
        //          zu loeschen!
        pSectionFmtTbl->Remove( nPos );
//FEATURE::CONDCOLL
        ULONG nCnt = 0, nSttNd = 0;
        if( pIdx && &GetNodes() == &pIdx->GetNodes() &&
            0 != (pSectNd = pIdx->GetNode().GetSectionNode() ))
        {
            nSttNd = pSectNd->GetIndex();
            nCnt = pSectNd->EndOfSectionIndex() - nSttNd - 1;
        }
//FEATURE::CONDCOLL

        delete pFmt;

        if( nSttNd && pFtnEndAtTxtEnd )
        {
            SwNodeIndex aUpdIdx( GetNodes(), nSttNd );
            GetFtnIdxs().UpdateFtn( aUpdIdx );
        }

//FEATURE::CONDCOLL
        SwCntntNode* pCNd;
        for( ; nCnt--; ++nSttNd )
            if( 0 != (pCNd = GetNodes()[ nSttNd ]->GetCntntNode() ) &&
                RES_CONDTXTFMTCOLL == pCNd->GetFmtColl()->Which() )
                pCNd->ChkCondColl();
//FEATURE::CONDCOLL
    }
    SetModified();
}

void SwDoc::ChgSection( USHORT nPos, const SwSection& rSect,
                        const SfxItemSet* pAttr,
                        sal_Bool bPreventLinkUpdate )
{
    SwSectionFmt* pFmt = (*pSectionFmtTbl)[ nPos ];
    SwSection* pSection = pFmt->GetSection();

    if( *pSection == rSect )
    {
        // die Attribute ueberpruefen
        BOOL bOnlyAttrChg = FALSE;
        if( pAttr && pAttr->Count() )
        {
            SfxItemIter aIter( *pAttr );
            USHORT nWhich = aIter.GetCurItem()->Which();
            while( TRUE )
            {
                if( pFmt->GetAttr( nWhich ) != *aIter.GetCurItem() )
                {
                    bOnlyAttrChg = TRUE;
                    break;
                }

                if( aIter.IsAtEnd() )
                    break;
                nWhich = aIter.NextItem()->Which();
            }
        }

        if( bOnlyAttrChg )
        {
            if( DoesUndo() )
            {
                ClearRedo();
                AppendUndo( new SwUndoChgSection( *pFmt, TRUE ) );
            }
            pFmt->SetAttr( *pAttr );
            SetModified();
        }
        return;
    }

    // Teste ob eine gesamte Content-Section (Dokument/TabellenBox/Fly)
    // versteckt werden soll, koennen wir zur Zeit nicht !!!!
    const SwNodeIndex* pIdx = 0;
    {
        const SwSectionNode* pSectNd;
        if( rSect.IsHidden() && 0 != (pIdx = pFmt->GetCntnt().GetCntntIdx() )
            && 0 != (pSectNd = pIdx->GetNode().GetSectionNode() ) )
        {
            ::lcl_CheckEmptyLayFrm( GetNodes(), (SwSection&)rSect,
                                *pSectNd, *pSectNd->EndOfSectionNode() );
        }
    }


    if( DoesUndo() )
    {
        ClearRedo();
        AppendUndo( new SwUndoChgSection( *pFmt, FALSE ) );
    }

    // #56167# Der LinkFileName koennte auch nur aus Separatoren bestehen
    String sCompareString = cTokenSeperator;
    sCompareString += cTokenSeperator;
    BOOL bUpdate = ( !pSection->IsLinkType() && rSect.IsLinkType() ) ||
                        ( rSect.GetLinkFileName().Len() &&
                            rSect.GetLinkFileName() != sCompareString &&
                            rSect.GetLinkFileName() !=
                            pSection->GetLinkFileName());

    String sSectName( rSect.GetName() );
    if( sSectName != pSection->GetName() )
        GetUniqueSectionName( &sSectName );
    else
        sSectName.Erase();

    *pSection = rSect;

    if( pAttr )
        pSection->GetFmt()->SetAttr( *pAttr );

    if( sSectName.Len() )
        pSection->SetName( sSectName );

    // ist eine Condition gesetzt
    if( pSection->IsHidden() && pSection->GetCondition().Len() )
    {
        // dann berechne bis zu dieser Position
        SwCalc aCalc( *this );
        if( !pIdx )
            pIdx = pFmt->GetCntnt().GetCntntIdx();
        FldsToCalc( aCalc, pIdx->GetIndex() );
        pSection->SetCondHidden( aCalc.Calculate( pSection->GetCondition() ).GetBool() );
    }

    if( bUpdate )
        pSection->CreateLink( bPreventLinkUpdate ? CREATE_CONNECT : CREATE_UPDATE );
    else if( !pSection->IsLinkType() && pSection->IsConnected() )
    {
        pSection->Disconnect();
        GetLinkManager().Remove( &pSection->GetBaseLink() );
    }

    SetModified();
}

void SwDoc::ChgSectionPasswd(
        const ::com::sun::star::uno::Sequence <sal_Int8>& rNew,
        const SwSection* pSection )
{
    SwSectionNode* pSectNd = 0;
    if( pSection )
        pSectNd = pSection->GetFmt()->GetSectionNode( TRUE );

    if( DoesUndo() )
    {
        ClearRedo();
        AppendUndo( new SwUndoChgSectPsswd( aSectionPasswd , pSectNd ) );
    }

    if( pSectNd )
        pSectNd->GetSection().SetPasswd( rNew );
    else
        aSectionPasswd = rNew;

    SetModified();
}

/* -----------------19.02.99 09:31-------------------
 * LockFrms wurde im InsertSection genutzt, um zu verhindern, dass
 * SectionFrms durch das DelFrms zerstoert werden. Dies ist durch
 * den Destroy-Listen-Mechanismus ueberfluessig geworden.
 * Falls diese Methode doch noch einmal reanimiert wird, bietet es
 * sich vielleicht an, beim Entlocken die SectionFrms auf Inhalt zu
 * pruefen und dann ggf. zur Zerstoerung anzumelden.
 * --------------------------------------------------*/

// und dann waren da noch die Fussnoten:
void lcl_DeleteFtn( SwSectionNode *pNd, ULONG nStt, ULONG nEnd )
{
    SwFtnIdxs& rFtnArr = pNd->GetDoc()->GetFtnIdxs();
    if( rFtnArr.Count() )
    {
        USHORT nPos;
        rFtnArr.SeekEntry( SwNodeIndex( *pNd ), &nPos );
        SwTxtFtn* pSrch;

        // loesche erstmal alle, die dahinter stehen
        while( nPos < rFtnArr.Count() &&
            _SwTxtFtn_GetIndex( (pSrch = rFtnArr[ nPos ]) ) <= nEnd )
        {
            // Werden die Nodes nicht geloescht mussen sie bei den Seiten
            // abmeldet (Frms loeschen) werden, denn sonst bleiben sie
            // stehen (Undo loescht sie nicht!)
            pSrch->DelFrms();
            ++nPos;
        }

        while( nPos-- &&
            _SwTxtFtn_GetIndex( (pSrch = rFtnArr[ nPos ]) ) >= nStt )
        {
            // Werden die Nodes nicht geloescht mussen sie bei den Seiten
            // abmeldet (Frms loeschen) werden, denn sonst bleiben sie
            // stehen (Undo loescht sie nicht!)
            pSrch->DelFrms();
        }
    }
}

inline BOOL lcl_IsTOXSection( const SwSection& rSection )
{
    return TOX_CONTENT_SECTION == rSection.GetType() ||
            TOX_HEADER_SECTION == rSection.GetType();
}

SwSectionNode* SwNodes::InsertSection( const SwNodeIndex& rNdIdx,
                                SwSectionFmt& rSectionFmt,
                                const SwSection& rSection,
                                const SwNodeIndex* pEnde,
                                BOOL bInsAtStart, BOOL bCreateFrms )
{
    SwNodeIndex aInsPos( rNdIdx );
    if( !pEnde )        // kein Bereich also neue Section davor/hinter anlegen
    {
        if( bInsAtStart )
        {
            if( !lcl_IsTOXSection( rSection ))
            {
                do {
                    aInsPos--;
                } while( aInsPos.GetNode().IsSectionNode() );
                aInsPos++;
            }
        }
        else
        {
            SwNode* pNd;
            aInsPos++;
            if( !lcl_IsTOXSection( rSection ))
                while( aInsPos.GetIndex() < Count() - 1 &&
                        ( pNd = &aInsPos.GetNode())->IsEndNode() &&
                        pNd->FindStartNode()->IsSectionNode())
                    aInsPos++;
        }
    }

    SwSectionNode* pSectNd = new SwSectionNode( aInsPos, rSectionFmt );
    if( pEnde )
    {
        // Sonderfall fuer die Reader/Writer
        if( &pEnde->GetNode() != &GetEndOfContent() )
            aInsPos = pEnde->GetIndex()+1;
    }
    else
    {
        SwTxtNode* pCpyTNd = rNdIdx.GetNode().GetTxtNode();
        if( pCpyTNd )
        {
            SwTxtNode* pTNd = new SwTxtNode( aInsPos, pCpyTNd->GetTxtColl() );
            if( pCpyTNd->GetpSwAttrSet() )
            {
                // Task 70955 - move PageDesc/Break to the first Node of the
                //              section
                const SfxItemSet& rSet = *pCpyTNd->GetpSwAttrSet();
                if( SFX_ITEM_SET == rSet.GetItemState( RES_BREAK ) ||
                    SFX_ITEM_SET == rSet.GetItemState( RES_PAGEDESC ))
                {
                    SfxItemSet aSet( rSet );
                    if( bInsAtStart )
                        pCpyTNd->ResetAttr( RES_PAGEDESC, RES_BREAK );
                    else
                    {
                        aSet.ClearItem( RES_PAGEDESC );
                        aSet.ClearItem( RES_BREAK );
                    }
                    pTNd->SwCntntNode::SetAttr( aSet );
                }
                else
                    pTNd->SwCntntNode::SetAttr( rSet );
            }
            // den Frame anlegen nicht vergessen !!
            pCpyTNd->MakeFrms( *pTNd );
        }
        else
            new SwTxtNode( aInsPos, (SwTxtFmtColl*)GetDoc()->GetDfltTxtFmtColl() );
    }
    SwEndNode* pEndNd = new SwEndNode( aInsPos, *pSectNd );

    pSectNd->GetSection() = rSection;
    SwSectionFmt* pSectFmt = pSectNd->GetSection().GetFmt();

    // Hier bietet sich als Optimierung an, vorhandene Frames nicht zu
    // zerstoeren und wieder neu anzulegen, sondern nur umzuhaengen.
    BOOL bInsFrm = bCreateFrms && !pSectNd->GetSection().IsHidden() &&
                   GetDoc()->GetRootFrm();
    SwNode2Layout *pNode2Layout = NULL;
    if( bInsFrm )
    {
        SwNodeIndex aTmp( *pSectNd );
        if( !pSectNd->GetNodes().FindPrvNxtFrmNode( aTmp, pSectNd->EndOfSectionNode() ) )
            // dann sammel mal alle Uppers ein
            pNode2Layout = new SwNode2Layout( *pSectNd );
    }

    // jetzt noch bei allen im Bereich den richtigen StartNode setzen
    ULONG nEnde = pSectNd->EndOfSectionIndex();
    ULONG nStart = pSectNd->GetIndex()+1;
    ULONG nSkipIdx = ULONG_MAX;
    for( ULONG n = nStart; n < nEnde; ++n )
    {
        SwNode* pNd = (*this)[n];

        //JP 30.04.99: Bug 65644 - alle in der NodeSection liegenden
        //              Sections unter die neue haengen
        if( ULONG_MAX == nSkipIdx )
            pNd->pStartOfSection = pSectNd;
        else if( n >= nSkipIdx )
            nSkipIdx = ULONG_MAX;

        if( pNd->IsStartNode() )
        {
            // die Verschachtelung der Formate herstellen!
            if( pNd->IsSectionNode() )
            {
                ((SwSectionNode*)pNd)->GetSection().GetFmt()->
                                    SetDerivedFrom( pSectFmt );
                ((SwSectionNode*)pNd)->DelFrms();
                n = pNd->EndOfSectionIndex();
            }
            else
            {
                if( pNd->IsTableNode() )
                    ((SwTableNode*)pNd)->DelFrms();

                if( ULONG_MAX == nSkipIdx )
                    nSkipIdx = pNd->EndOfSectionIndex();
            }
        }
        else if( pNd->IsCntntNode() )
            ((SwCntntNode*)pNd)->DelFrms();
    }

    lcl_DeleteFtn( pSectNd, nStart, nEnde );

    if( bInsFrm )
    {
        if( pNode2Layout )
        {
            ULONG nIdx = pSectNd->GetIndex();
            pNode2Layout->RestoreUpperFrms( pSectNd->GetNodes(), nIdx, nIdx + 1 );
            delete pNode2Layout;
        }
        else
            pSectNd->MakeFrms( &aInsPos );
    }

    return pSectNd;
}

SwSectionNode* SwNode::FindSectionNode()
{
    if( IsSectionNode() )
        return GetSectionNode();
    SwStartNode* pTmp = pStartOfSection;
    while( !pTmp->IsSectionNode() && pTmp->GetIndex() )
#if defined( ALPHA ) && defined( UNX )
        pTmp = ((SwNode*)pTmp)->pStartOfSection;
#else
        pTmp = pTmp->pStartOfSection;
#endif
    return pTmp->GetSectionNode();
}


//---------
// SwSectionNode
//---------

SwSectionNode::SwSectionNode( const SwNodeIndex& rIdx, SwSectionFmt& rFmt )
    : SwStartNode( rIdx, ND_SECTIONNODE )
{
    SwSectionNode* pParent = FindStartNode()->FindSectionNode();
    if( pParent )
    {
        // das Format beim richtigen Parent anmelden.
        rFmt.SetDerivedFrom( pParent->GetSection().GetFmt() );
    }
    pSection = new SwSection( CONTENT_SECTION, rFmt.GetName(), &rFmt );

    // jetzt noch die Verbindung von Format zum Node setzen
    // Modify unterdruecken, interresiert keinen
    rFmt.LockModify();
    rFmt.SetAttr( SwFmtCntnt( this ) );
    rFmt.UnlockModify();
}

//Hier werden ueberfluessige SectionFrms entfernt
SwFrm* SwClearDummies( SwFrm* pFrm )
{
    SwFrm* pTmp = pFrm;
    while( pTmp )
    {
        ASSERT( !pTmp->GetUpper(), "SwClearDummies: No Upper allowed!" );
        if( pTmp->IsSctFrm() )
        {
            SwSectionFrm* pSectFrm = (SwSectionFrm*)pFrm;
            pTmp = pTmp->GetNext();
            if( !pSectFrm->GetLower() )
            {
                if( pSectFrm->GetPrev() )
                    pSectFrm->GetPrev()->pNext = pTmp;
                else
                    pFrm = pTmp;
                if( pTmp )
                    pTmp->pPrev = pSectFrm->GetPrev();
                delete pSectFrm;
            }
        }
        else
            pTmp = pTmp->GetNext();
    }
    return pFrm;
}

SwSectionNode::~SwSectionNode()
{
    {
        SwClientIter aIter( *(pSection->GetFmt()) );
        SwClient *pLast = aIter.GoStart();
        while ( pLast )
        {
            if ( pLast->IsA( TYPE(SwFrm) ) )
            {
                SwSectionFrm *pSectFrm = (SwSectionFrm*)pLast;
                SwSectionFrm::MoveCntntAndDelete( pSectFrm, TRUE );
                pLast = aIter.GoStart();
            }
            else
                pLast = aIter++;
        }
    }
    SwDoc* pDoc = GetDoc();

    SwSectionFmt* pFmt = pSection->GetFmt();
    if( pFmt )
    {
        // das Attribut entfernen, weil die Section ihr Format loescht
        // und falls das Cntnt-Attribut vorhanden ist, die Section aufhebt.
        pFmt->LockModify();
        pFmt->ResetAttr( RES_CNTNT );
        pFmt->UnlockModify();
    }

    BOOL bUndo = pDoc->DoesUndo();
    // verhinder beim Loeschen aus der Undo/Redo-History einen rekursiven Aufruf
    if( bUndo && &pDoc->GetNodes() != &GetNodes() )
        pDoc->DoUndo( FALSE );
    DELETEZ( pSection );
    pDoc->DoUndo( bUndo );
}

// setze ein neues SectionObject. Erstmal nur gedacht fuer die
// neuen VerzeichnisSections. Der geht ueber in den Besitz des Nodes!
void SwSectionNode::SetNewSection( SwSection* pNewSection )
{
    ASSERT( pNewSection, "ohne Pointer geht hier nichts" );
    if( pNewSection )
    {
        SwNode2Layout aN2L( *this );

        // einige Flags sollten ueber nommen werden!
        pNewSection->bProtectFlag = pSection->bProtectFlag;
        pNewSection->bHiddenFlag = pSection->bHiddenFlag;
        pNewSection->bHidden = pSection->bHidden;
        pNewSection->bCondHiddenFlag = pSection->bCondHiddenFlag;

        // The section frame contains a pointer to the section. That for,
        // the frame must be destroyed before deleting the section.
        DelFrms();

        delete pSection;
        pSection = pNewSection;

        ULONG nIdx = GetIndex();
        aN2L.RestoreUpperFrms( GetNodes(), nIdx, nIdx + 1 );
    }
}

SwFrm *SwSectionNode::MakeFrm()
{
    pSection->bHiddenFlag = FALSE;
    return new SwSectionFrm( *pSection );
}

//Methode erzeugt fuer den vorhergehenden Node alle Ansichten vom
//Dokument. Die erzeugten Contentframes werden in das entsprechende
//Layout gehaengt.
void SwSectionNode::MakeFrms(const SwNodeIndex & rIdx )
{
    // also nehme meinen nachfolgenden oder vorhergehenden ContentFrame:
    SwNodes& rNds = GetNodes();
    if( rNds.IsDocNodes() && rNds.GetDoc()->GetRootFrm() )
    {
        if( GetSection().IsHidden() || IsCntntHidden() )
        {
            SwNodeIndex aIdx( *EndOfSectionNode() );
            SwCntntNode* pCNd = rNds.GoNextSection( &aIdx, TRUE, FALSE );
            if( !pCNd )
            {
                aIdx = *this;
                if( 0 == ( pCNd = rNds.GoPrevSection( &aIdx, TRUE, FALSE )) )
                    return ;
            }
            pCNd = rNds[ aIdx ]->GetCntntNode();
            pCNd->MakeFrms( (SwCntntNode&)rIdx.GetNode() );
        }
        else
        {
            SwNode2Layout aNode2Layout( *this, rIdx.GetIndex() );
            SwFrm *pFrm, *pNew;
            while( 0 != (pFrm = aNode2Layout.NextFrm()) )
            {
                ASSERT( pFrm->IsSctFrm(), "Depend von Section keine Section." );
                pNew = rIdx.GetNode().GetCntntNode()->MakeFrm();

                SwSectionNode *pS = rIdx.GetNode().FindSectionNode();
                // if the node is in a section, the sectionframe now
                // has to be created..
                if( pS )
                {
                    SwSectionFrm *pSct = new SwSectionFrm( pS->GetSection() );
                    SwLayoutFrm* pUp = pSct;
                    while( pUp->Lower() )  // for columned sections
                    {
                        ASSERT( pUp->Lower()->IsLayoutFrm(),"Who's in there?" );
                        pUp = (SwLayoutFrm*)pUp->Lower();
                    }
                    pNew->Paste( pUp, NULL );
                    pNew = pSct;
                }

                // wird ein Node vorher oder nachher mit Frames versehen
                if ( rIdx < GetIndex() )
                    // der neue liegt vor mir
                    pNew->Paste( pFrm->GetUpper(), pFrm );
                else
                    // der neue liegt hinter mir
                    pNew->Paste( pFrm->GetUpper(), pFrm->GetNext() );
            }
        }
    }
}

//Fuer jedes vorkommen im Layout einen SectionFrm anlegen und vor den
//entsprechenden CntntFrm pasten.

void SwSectionNode::MakeFrms( SwNodeIndex* pIdxBehind, SwNodeIndex* pEndIdx )
{
    ASSERT( pIdxBehind, "kein Index" );
    SwNodes& rNds = GetNodes();
    SwDoc* pDoc = rNds.GetDoc();

    *pIdxBehind = *this;

    pSection->bHiddenFlag = TRUE;

    if( rNds.IsDocNodes() )
    {
        SwNodeIndex *pEnd = pEndIdx ? pEndIdx :
                            new SwNodeIndex( *EndOfSectionNode(), 1 );
        ::MakeFrms( pDoc, *pIdxBehind, *pEnd );
        if( !pEndIdx )
            delete pEnd;
    }

}

void SwSectionNode::DelFrms()
{
    ULONG nStt = GetIndex()+1, nEnd = EndOfSectionIndex();
    if( nStt >= nEnd )
    {
        // unser Flag muessen wir noch aktualisieren
        // pSection->bHiddenFlag = TRUE;
        return ;
    }

    SwNodes& rNds = GetNodes();
    pSection->GetFmt()->DelFrms();

    // unser Flag muessen wir noch aktualisieren
    pSection->bHiddenFlag = TRUE;

    // Bug 30582: falls der Bereich in Fly oder TabellenBox ist, dann
    //              kann er nur "gehiddet" werden, wenn weiterer Content
    //              vorhanden ist, der "Frames" haelt. Sonst hat der
    //              Fly/TblBox-Frame keinen Lower !!!
    {
        SwNodeIndex aIdx( *this );
        if( !rNds.GoPrevSection( &aIdx, TRUE, FALSE ) ||
            !CheckNodesRange( *this, aIdx, TRUE ) ||
            !lcl_IsInSameTblBox( rNds, *this, aIdx ))
        {
            aIdx = *EndOfSectionNode();
            if( !rNds.GoNextSection( &aIdx, TRUE, FALSE ) ||
                !CheckNodesRange( *EndOfSectionNode(), aIdx, TRUE ) ||
                !lcl_IsInSameTblBox( rNds, *EndOfSectionNode(), aIdx ))
                pSection->bHiddenFlag = FALSE;
        }
    }
}

SwSectionNode* SwSectionNode::MakeCopy( SwDoc* pDoc, const SwNodeIndex& rIdx ) const
{
    // in welchen Array steht ich denn: Nodes, UndoNodes ??
    const SwNodes& rNds = GetNodes();

    // das SectionFrmFmt kopieren
    SwSectionFmt* pSectFmt = pDoc->MakeSectionFmt( 0 );
    pSectFmt->CopyAttrs( *GetSection().GetFmt() );

    SwSectionNode* pSectNd = new SwSectionNode( rIdx, *pSectFmt );
    SwEndNode* pEndNd = new SwEndNode( rIdx, *pSectNd );
    SwNodeIndex aInsPos( *pEndNd );

    // Werte uebertragen
    SwSection* pNewSect = pSectNd->pSection;

    switch( GetSection().GetType() )
    {
    case TOX_CONTENT_SECTION:
        {
            ASSERT( GetSection().ISA( SwTOXBaseSection ), "keine TOXBaseSection!" );
            SwTOXBaseSection& rTOXSect = (SwTOXBaseSection&)GetSection();
            SwTOXBase aTmp( rTOXSect, pDoc );

            SwTOXBaseSection* pNew = new SwTOXBaseSection( aTmp );

            pNewSect = pNew;
            pSectFmt->Add( pNewSect );
            pSectNd->SetNewSection( pNew );
        }
        break;

    default:
        // beim Move den Namen beibehalten
        if( rNds.GetDoc() == pDoc && pDoc->IsCopyIsMove() )
            pNewSect->SetName( GetSection().GetName() );
        else
            pNewSect->SetName( pDoc->GetUniqueSectionName(
                                        &GetSection().GetName() ) );
        break;
    }


    pNewSect->SetType( GetSection().GetType() );
    pNewSect->SetCondition( GetSection().GetCondition() );
    pNewSect->SetLinkFileName( GetSection().GetLinkFileName() );
    if( !pNewSect->IsHiddenFlag() && GetSection().IsHidden() )
        pNewSect->SetHidden( TRUE );
    if( !pNewSect->IsProtectFlag() && GetSection().IsProtect() )
        pNewSect->SetProtect( TRUE );

    SwNodeRange aRg( *this, +1, *EndOfSectionNode() );  // (wo stehe in denn nun ??)
    rNds._Copy( aRg, aInsPos, FALSE );

    // loesche alle Frames vom kopierten Bereich, diese werden beim
    // erzeugen des SectionFrames angelegt !
    pSectNd->DelFrms();

    // dann kopiere auch noch die Links/Server
    if( pNewSect->IsLinkType() )        // den Link eintragen
        pNewSect->CreateLink( pDoc->GetRootFrm() ? CREATE_CONNECT
                                                 : CREATE_NONE );

    // falls als Server aus dem Undo kopiert wird, wieder eintragen
    if( pSection->IsServer() && pDoc->GetUndoNds() == &rNds )
    {
        pNewSect->SetRefObject( pSection->GetObject() );
        pDoc->GetLinkManager().InsertServer( pNewSect->GetObject() );
    }

    return pSectNd;
}

BOOL SwSectionNode::IsCntntHidden() const
{
    ASSERT( !pSection->IsHidden(), "That's simple: Hidden Section => Hidden Content" );
    SwNodeIndex aTmp( *this, 1 );
    ULONG nEnd = EndOfSectionIndex();
    while( aTmp < nEnd )
    {
        if( aTmp.GetNode().IsSectionNode() )
        {
            const SwSection& rSect = ((SwSectionNode&)aTmp.GetNode()).GetSection();
            if( rSect.IsHiddenFlag() )
                // dann diese Section ueberspringen
                aTmp = *aTmp.GetNode().EndOfSectionNode();
        }
        else
        {
            if( aTmp.GetNode().IsCntntNode() || aTmp.GetNode().IsTableNode() )
                return FALSE; // Nicht versteckter Inhalt wurde gefunden
            ASSERT( aTmp.GetNode().IsEndNode(), "EndNode expected" );
        }
        aTmp++;
    }
    return TRUE; // Alles versteckt
}


void SwSectionNode::NodesArrChgd()
{
    SwSectionFmt* pFmt = pSection->GetFmt();
    if( pFmt )
    {
        SwNodes& rNds = GetNodes();
        SwDoc* pDoc = pFmt->GetDoc();

        if( !rNds.IsDocNodes() )
        {
            SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT, pFmt );
            pFmt->Modify( &aMsgHint, &aMsgHint );
        }

        pFmt->LockModify();
        pFmt->SetAttr( SwFmtCntnt( this ));
        pFmt->UnlockModify();

        SwSectionNode* pSectNd = FindStartNode()->FindSectionNode();
        // set the correct parent from the new section
        pFmt->SetDerivedFrom( pSectNd ? pSectNd->GetSection().GetFmt()
                                      : pDoc->GetDfltFrmFmt() );

        // jetzt noch bei allen im Bereich den richtigen StartNode setzen
        ULONG nStart = GetIndex()+1, nEnde = EndOfSectionIndex();
        for( ULONG n = nStart; n < nEnde; ++n )
            // die Verschachtelung der Formate herstellen!
            if( 0 != ( pSectNd = rNds[ n ]->GetSectionNode() ) )
            {
                pSectNd->GetSection().GetFmt()->SetDerivedFrom( pFmt );
                n = pSectNd->EndOfSectionIndex();
            }

        // verschieben vom Nodes- ins UndoNodes-Array?
        if( rNds.IsDocNodes() )
        {
            ASSERT( pDoc == GetDoc(),
                    "verschieben in unterschiedliche Documente?" );
            if( pSection->IsLinkType() )        // den Link austragen
                pSection->CreateLink( pDoc->GetRootFrm() ? CREATE_CONNECT
                                                         : CREATE_NONE );

            if( pSection->IsServer() )                  // als Server austragen
                pDoc->GetLinkManager().InsertServer( pSection->GetObject() );
        }
        else
        {
            if( CONTENT_SECTION != pSection->GetType() )        // den Link austragen
                pDoc->GetLinkManager().Remove( &pSection->GetBaseLink() );

            if( pSection->IsServer() )                  // als Server austragen
                pDoc->GetLinkManager().RemoveServer( pSection->GetObject() );
        }
    }
}


String SwDoc::GetUniqueSectionName( const String* pChkStr ) const
{
    ResId aId( STR_REGION_DEFNAME, pSwResMgr );
    String aName( aId );
    xub_StrLen nNmLen = aName.Len();

    USHORT nNum, nTmp, nFlagSize = ( pSectionFmtTbl->Count() / 8 ) +2;
    BYTE* pSetFlags = new BYTE[ nFlagSize ];
    memset( pSetFlags, 0, nFlagSize );

    const SwSectionNode* pSectNd;
    for( USHORT n = 0; n < pSectionFmtTbl->Count(); ++n )
        if( 0 != ( pSectNd = (*pSectionFmtTbl)[ n ]->GetSectionNode( FALSE ) ))
        {
            const String& rNm = pSectNd->GetSection().GetName();
            if( rNm.Match( aName ) == nNmLen )
            {
                // Nummer bestimmen und das Flag setzen
                nNum = rNm.Copy( nNmLen ).ToInt32();
                if( nNum-- && nNum < pSectionFmtTbl->Count() )
                    pSetFlags[ nNum / 8 ] |= (0x01 << ( nNum & 0x07 ));
            }
            if( pChkStr && pChkStr->Equals( rNm ) )
                pChkStr = 0;
        }

    if( !pChkStr )
    {
        // alle Nummern entsprechend geflag, also bestimme die richtige Nummer
        nNum = pSectionFmtTbl->Count();
        for( n = 0; n < nFlagSize; ++n )
            if( 0xff != ( nTmp = pSetFlags[ n ] ))
            {
                // also die Nummer bestimmen
                nNum = n * 8;
                while( nTmp & 1 )
                    ++nNum, nTmp >>= 1;
                break;
            }

    }
    __DELETE( nFlagSize ) pSetFlags;
    if( pChkStr )
        return *pChkStr;
    return aName += String::CreateFromInt32( ++nNum );
}


