/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <svl/smplhint.hxx>
#include <hintids.hxx>
#include <sfx2/linkmgr.hxx>
#include <svl/itemiter.hxx>
#include <tools/resid.hxx>
#include <fmtcntnt.hxx>
#include <fmtanchr.hxx>
#include <txtftn.hxx>
#include <fmtclds.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <rootfrm.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <section.hxx>
#include <UndoSection.hxx>
#include <UndoDelete.hxx>
#include <swundo.hxx>
#include <calc.hxx>
#include <swtable.hxx>
#include <swserv.hxx>
#include <frmfmt.hxx>
#include <frmtool.hxx>
#include <ftnidx.hxx>
#include <docary.hxx>
#include <redline.hxx>
#include <sectfrm.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <node2lay.hxx>
#include <doctxm.hxx>
#include <fmtftntx.hxx>
#include <comcore.hrc>

#include <viewsh.hxx>
#include <txtfrm.hxx>




static bool lcl_IsInSameTblBox( SwNodes& _rNds,
                         const SwNode& _rNd,
                         const bool _bPrev )
{
    const SwTableNode* pTblNd = _rNd.FindTableNode();
    if ( !pTblNd )
    {
        return true;
    }

    
    SwNodeIndex aChkIdx( _rNd );
    {
        
        
        
        
        bool bFound = false;
        do
        {
            if ( _bPrev
                 ? !_rNds.GoPrevSection( &aChkIdx, false, false )
                 : !_rNds.GoNextSection( &aChkIdx, false, false ) )
            {
                OSL_FAIL( "<lcl_IsInSameTblBox(..)> - no previous/next!" );
                return false;
            }
            else
            {
                if ( aChkIdx < pTblNd->GetIndex() ||
                     aChkIdx > pTblNd->EndOfSectionNode()->GetIndex() )
                {
                    return false;
                }
                else
                {
                    
                    
                    SwSectionNode* pSectNd = aChkIdx.GetNode().FindSectionNode();
                    if ( !pSectNd ||
                         pSectNd->GetIndex() < pTblNd->GetIndex() ||
                         !pSectNd->GetSection().IsHiddenFlag() )
                    {
                        bFound = true;
                    }
                }
            }
        } while ( !bFound );
    }

    
    const SwTableSortBoxes& rSortBoxes = pTblNd->GetTable().GetTabSortBoxes();
    sal_uLong nIdx = _rNd.GetIndex();
    for (size_t n = 0; n < rSortBoxes.size(); ++n)
    {
        const SwStartNode* pNd = rSortBoxes[ n ]->GetSttNd();
        if ( pNd->GetIndex() < nIdx && nIdx < pNd->EndOfSectionIndex() )
        {
            
            nIdx = aChkIdx.GetIndex();
            return pNd->GetIndex() < nIdx && nIdx < pNd->EndOfSectionIndex();
        }
    }

    return true;
}

static void lcl_CheckEmptyLayFrm( SwNodes& rNds, SwSectionData& rSectionData,
                        const SwNode& rStt, const SwNode& rEnd )
{
    SwNodeIndex aIdx( rStt );
    if( !rNds.GoPrevSection( &aIdx, true, false ) ||
        !CheckNodesRange( rStt, aIdx, true ) ||
        
        !lcl_IsInSameTblBox( rNds, rStt, true ))
    {
        aIdx = rEnd;
        if( !rNds.GoNextSection( &aIdx, true, false ) ||
            !CheckNodesRange( rEnd, aIdx, true ) ||
            
            !lcl_IsInSameTblBox( rNds, rEnd, false ))
        {
            rSectionData.SetHidden( false );
        }
    }
}

SwSection *
SwDoc::InsertSwSection(SwPaM const& rRange, SwSectionData & rNewData,
                       SwTOXBase const*const pTOXBase,
                       SfxItemSet const*const pAttr, bool const bUpdate)
{
    const SwNode* pPrvNd = 0;
    sal_uInt16 nRegionRet = 0;
    if( rRange.HasMark() &&
        0 == ( nRegionRet = IsInsRegionAvailable( rRange, &pPrvNd ) ))
    {
        
        
        SAL_INFO("sw.core" , "InsertSwSection: rRange overlaps other sections");
        return 0;
    }

    
    if (rNewData.IsHidden() && rRange.HasMark())
    {
        const SwPosition *pStt = rRange.Start(), *pEnd = rRange.End();
        if( !pStt->nContent.GetIndex() &&
            pEnd->nNode.GetNode().GetCntntNode()->Len() ==
            pEnd->nContent.GetIndex() )
        {
            ::lcl_CheckEmptyLayFrm( GetNodes(),
                                    rNewData,
                                    pStt->nNode.GetNode(),
                                    pEnd->nNode.GetNode() );
        }
    }

    SwUndoInsSection* pUndoInsSect = 0;
    bool const bUndo(GetIDocumentUndoRedo().DoesUndo());
    if (bUndo)
    {
        pUndoInsSect = new SwUndoInsSection(rRange, rNewData, pAttr, pTOXBase);
        GetIDocumentUndoRedo().AppendUndo( pUndoInsSect );
        GetIDocumentUndoRedo().DoUndo(false);
    }

    SwSectionFmt* const pFmt = MakeSectionFmt( 0 );
    if ( pAttr )
    {
        pFmt->SetFmtAttr( *pAttr );
    }

    SwSectionNode* pNewSectNode = 0;

    RedlineMode_t eOld = GetRedlineMode();
    SetRedlineMode_intern( (RedlineMode_t)((eOld & ~nsRedlineMode_t::REDLINE_SHOW_MASK) | nsRedlineMode_t::REDLINE_IGNORE));

    if( rRange.HasMark() )
    {
        SwPosition *pSttPos = (SwPosition*)rRange.Start(),
                    *pEndPos = (SwPosition*)rRange.End();
        if( pPrvNd && 3 == nRegionRet )
        {
            OSL_ENSURE( pPrvNd, "The SectionNode is missing" );
            SwNodeIndex aStt( pSttPos->nNode ), aEnd( pEndPos->nNode, +1 );
            while( pPrvNd != aStt.GetNode().StartOfSectionNode() )
                aStt--;
            while( pPrvNd != aEnd.GetNode().StartOfSectionNode() )
                ++aEnd;

            --aEnd; 
            pNewSectNode = GetNodes().InsertTextSection(
                        aStt, *pFmt, rNewData, pTOXBase, & aEnd);
        }
        else
        {
            if( pUndoInsSect )
            {
                if( !( pPrvNd && 1 == nRegionRet ) &&
                    pSttPos->nContent.GetIndex() )
                {
                    SwTxtNode* const pTNd =
                        pSttPos->nNode.GetNode().GetTxtNode();
                    if (pTNd)
                    {
                        pUndoInsSect->SaveSplitNode( pTNd, true );
                    }
                }

                if ( !( pPrvNd && 2 == nRegionRet ) )
                {
                    SwTxtNode *const pTNd =
                        pEndPos->nNode.GetNode().GetTxtNode();
                    if (pTNd && (pTNd->GetTxt().getLength()
                                    != pEndPos->nContent.GetIndex()))
                    {
                        pUndoInsSect->SaveSplitNode( pTNd, false );
                    }
                }
            }

            if( pPrvNd && 1 == nRegionRet )
            {
                pSttPos->nNode.Assign( *pPrvNd );
                pSttPos->nContent.Assign( pSttPos->nNode.GetNode().GetCntntNode(), 0 );
            }
            else if( pSttPos->nContent.GetIndex() )
            {
                SplitNode( *pSttPos, false );
            }

            if( pPrvNd && 2 == nRegionRet )
            {
                pEndPos->nNode.Assign( *pPrvNd );
                pEndPos->nContent.Assign( pEndPos->nNode.GetNode().GetCntntNode(), 0 );
            }
            else
            {
                const SwCntntNode* pCNd = pEndPos->nNode.GetNode().GetCntntNode();
                if( pCNd && pCNd->Len() != pEndPos->nContent.GetIndex() )
                {
                    sal_Int32 nCntnt = pSttPos->nContent.GetIndex();
                    SplitNode( *pEndPos, false );

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
                        
                        pEndPos->nNode--;
                        pTNd = pEndPos->nNode.GetNode().GetTxtNode();
                    }
                    nCntnt = (pTNd) ? pTNd->GetTxt().getLength() : 0;
                    pEndPos->nContent.Assign( pTNd, nCntnt );
                }
            }
            pNewSectNode = GetNodes().InsertTextSection(
                pSttPos->nNode, *pFmt, rNewData, pTOXBase, &pEndPos->nNode);
        }
    }
    else
    {
        const SwPosition* pPos = rRange.GetPoint();
        const SwCntntNode* pCNd = pPos->nNode.GetNode().GetCntntNode();
        if( !pPos->nContent.GetIndex() )
        {
            pNewSectNode = GetNodes().InsertTextSection(
                pPos->nNode, *pFmt, rNewData, pTOXBase, 0, true);
        }
        else if( pPos->nContent.GetIndex() == pCNd->Len() )
        {
            pNewSectNode = GetNodes().InsertTextSection(
                pPos->nNode, *pFmt, rNewData, pTOXBase, 0, false);
        }
        else
        {
            if( pUndoInsSect && pCNd->IsTxtNode() )
            {
                pUndoInsSect->SaveSplitNode( (SwTxtNode*)pCNd, true );
            }
            SplitNode( *pPos, false );
            pNewSectNode = GetNodes().InsertTextSection(
                pPos->nNode, *pFmt, rNewData, pTOXBase, 0, true);
        }
    }


    pNewSectNode->CheckSectionCondColl();


    SetRedlineMode_intern( eOld );

    
    if( IsRedlineOn() || (!IsIgnoreRedline() && !mpRedlineTbl->empty() ))
    {
        SwPaM aPam( *pNewSectNode->EndOfSectionNode(), *pNewSectNode, 1 );
        if( IsRedlineOn() )
        {
            AppendRedline( new SwRangeRedline( nsRedlineType_t::REDLINE_INSERT, aPam ), true);
        }
        else
        {
            SplitRedline( aPam );
        }
    }

    
    if (rNewData.IsHidden() && !rNewData.GetCondition().isEmpty())
    {
        
        SwCalc aCalc( *this );
        if( ! IsInReading() )
        {
            FldsToCalc( aCalc, pNewSectNode->GetIndex(), USHRT_MAX );
        }
        SwSection& rNewSect = pNewSectNode->GetSection();
        rNewSect.SetCondHidden( aCalc.Calculate( rNewSect.GetCondition() ).GetBool() );
    }

    sal_Bool bUpdateFtn = sal_False;
    if( GetFtnIdxs().size() && pAttr )
    {
        sal_uInt16 nVal = ((SwFmtFtnAtTxtEnd&)pAttr->Get(
                                            RES_FTN_AT_TXTEND )).GetValue();
           if( ( FTNEND_ATTXTEND_OWNNUMSEQ == nVal ||
              FTNEND_ATTXTEND_OWNNUMANDFMT == nVal ) ||
            ( FTNEND_ATTXTEND_OWNNUMSEQ == ( nVal = ((SwFmtEndAtTxtEnd&)
                            pAttr->Get( RES_END_AT_TXTEND )).GetValue() ) ||
              FTNEND_ATTXTEND_OWNNUMANDFMT == nVal ))
        {
            bUpdateFtn = sal_True;
        }
    }

    if( pUndoInsSect )
    {
        pUndoInsSect->SetSectNdPos( pNewSectNode->GetIndex() );
        pUndoInsSect->SetUpdtFtnFlag( bUpdateFtn );
        GetIDocumentUndoRedo().DoUndo(bUndo);
    }

    if (rNewData.IsLinkType())
    {
        pNewSectNode->GetSection().CreateLink( bUpdate ? CREATE_UPDATE : CREATE_CONNECT );
    }

    if( bUpdateFtn )
    {
        GetFtnIdxs().UpdateFtn( SwNodeIndex( *pNewSectNode ));
    }

    SetModified();
    return &pNewSectNode->GetSection();
}

sal_uInt16 SwDoc::IsInsRegionAvailable( const SwPaM& rRange,
                                const SwNode** ppSttNd ) const
{
    sal_uInt16 nRet = 1;
    if( rRange.HasMark() )
    {
        
        const SwPosition* pStt = rRange.Start();
        const SwPosition* pEnd = rRange.End();

        const SwCntntNode* pCNd = pEnd->nNode.GetNode().GetCntntNode();
        const SwNode* pNd = &pStt->nNode.GetNode();
        const SwSectionNode* pSectNd = pNd->FindSectionNode();
        const SwSectionNode* pEndSectNd = pCNd ? pCNd->FindSectionNode() : 0;
        if( pSectNd && pEndSectNd && pSectNd != pEndSectNd )
        {
            
            
            nRet = 0;
            if( !pStt->nContent.GetIndex()
                && pSectNd->GetIndex() == pStt->nNode.GetIndex() - 1
                && pEnd->nContent.GetIndex() == pCNd->Len() )
            {
                SwNodeIndex aIdx( pStt->nNode, -1 );
                sal_uLong nCmp = pEnd->nNode.GetIndex();
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
                                                : pNd->StartOfSectionNode();

                aIdx = pEnd->nNode.GetIndex() + 1;
                nCmp = pStt->nNode.GetIndex();
                while( 0 != ( pNxtNd = (pNd = &aIdx.GetNode())->GetEndNode() ) &&
                    pNxtNd->StartOfSectionNode()->IsSectionNode() &&
                    !( pNxtNd->StartOfSectionIndex() < nCmp &&
                        nCmp < aIdx.GetIndex() ) )
                {
                    ++aIdx;
                }
                if( !pNxtNd )
                    pNxtNd = pNd->EndOfSectionNode();

                if( pPrvNd && pNxtNd && pPrvNd == pNxtNd->StartOfSectionNode() )
                {
                    nRet = 3;

                    if( ppSttNd )
                        *ppSttNd = pPrvNd;
                }
            }
        }
        else if( !pSectNd && pEndSectNd )
        {
            
            
            nRet = 0;
            if( pEnd->nContent.GetIndex() == pCNd->Len() )
            {
                SwNodeIndex aIdx( pEnd->nNode, 1 );
                if( aIdx.GetNode().IsEndNode() &&
                        0 != aIdx.GetNode().FindSectionNode() )
                {
                    do {
                        ++aIdx;
                    } while( aIdx.GetNode().IsEndNode() &&
                                0 != aIdx.GetNode().FindSectionNode() );
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
                            ++aIdx;
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
        pDerivedFrom = (SwSectionFmt*)mpDfltFrmFmt;
    SwSectionFmt* pNew = new SwSectionFmt( pDerivedFrom, this );
    mpSectionFmtTbl->push_back( pNew );
    return pNew;
}

void SwDoc::DelSectionFmt( SwSectionFmt *pFmt, bool bDelNodes )
{
    SwSectionFmts::iterator itFmtPos = std::find( mpSectionFmtTbl->begin(), mpSectionFmtTbl->end(), pFmt );

    GetIDocumentUndoRedo().StartUndo(UNDO_DELSECTION, NULL);

    if( mpSectionFmtTbl->end() != itFmtPos )
    {
        const SwNodeIndex* pIdx = pFmt->GetCntnt( sal_False ).GetCntntIdx();
        const SfxPoolItem* pFtnEndAtTxtEnd;
        if( SFX_ITEM_SET != pFmt->GetItemState(
                            RES_FTN_AT_TXTEND, sal_True, &pFtnEndAtTxtEnd ) ||
            SFX_ITEM_SET != pFmt->GetItemState(
                            RES_END_AT_TXTEND, sal_True, &pFtnEndAtTxtEnd ))
            pFtnEndAtTxtEnd = 0;

        const SwSectionNode* pSectNd;

        if( GetIDocumentUndoRedo().DoesUndo() )
        {
            if( bDelNodes && pIdx && &GetNodes() == &pIdx->GetNodes() &&
                0 != (pSectNd = pIdx->GetNode().GetSectionNode() ))
            {
                SwNodeIndex aUpdIdx( *pIdx );
                SwPaM aPaM( *pSectNd->EndOfSectionNode(), *pSectNd );
                GetIDocumentUndoRedo().AppendUndo( new SwUndoDelete( aPaM ));
                if( pFtnEndAtTxtEnd )
                    GetFtnIdxs().UpdateFtn( aUpdIdx );
                SetModified();
                
                GetIDocumentUndoRedo().EndUndo(UNDO_DELSECTION, NULL);
                return ;
            }
            GetIDocumentUndoRedo().AppendUndo( MakeUndoDelSection( *pFmt ) );
        }
        else if( bDelNodes && pIdx && &GetNodes() == &pIdx->GetNodes() &&
                0 != (pSectNd = pIdx->GetNode().GetSectionNode() ))
        {
            SwNodeIndex aUpdIdx( *pIdx );
            DeleteSection( (SwNode*)pSectNd );
            if( pFtnEndAtTxtEnd )
                GetFtnIdxs().UpdateFtn( aUpdIdx );
            SetModified();
            
            GetIDocumentUndoRedo().EndUndo(UNDO_DELSECTION, NULL);
            return ;
        }

        {
            SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT, pFmt );
            pFmt->ModifyNotification( &aMsgHint, &aMsgHint );
        }

        
        
        itFmtPos = std::find( mpSectionFmtTbl->begin(), mpSectionFmtTbl->end(), pFmt );

        
        
        mpSectionFmtTbl->erase( itFmtPos );

        sal_uLong nCnt = 0, nSttNd = 0;
        if( pIdx && &GetNodes() == &pIdx->GetNodes() &&
            0 != (pSectNd = pIdx->GetNode().GetSectionNode() ))
        {
            nSttNd = pSectNd->GetIndex();
            nCnt = pSectNd->EndOfSectionIndex() - nSttNd - 1;
        }


        delete pFmt;

        if( nSttNd && pFtnEndAtTxtEnd )
        {
            SwNodeIndex aUpdIdx( GetNodes(), nSttNd );
            GetFtnIdxs().UpdateFtn( aUpdIdx );
        }


        SwCntntNode* pCNd;
        for( ; nCnt--; ++nSttNd )
            if( 0 != (pCNd = GetNodes()[ nSttNd ]->GetCntntNode() ) &&
                RES_CONDTXTFMTCOLL == pCNd->GetFmtColl()->Which() )
                pCNd->ChkCondColl();

    }

    GetIDocumentUndoRedo().EndUndo(UNDO_DELSECTION, NULL);

    SetModified();
}

void SwDoc::UpdateSection(sal_uInt16 const nPos, SwSectionData & rNewData,
        SfxItemSet const*const pAttr, bool const bPreventLinkUpdate)
{
    SwSectionFmt* pFmt = (*mpSectionFmtTbl)[ nPos ];
    SwSection* pSection = pFmt->GetSection();

    
    bool bOldCondHidden = pSection->IsCondHidden() ? true : false;

    if (pSection->DataEquals(rNewData))
    {
        
        bool bOnlyAttrChg = false;
        if( pAttr && pAttr->Count() )
        {
            SfxItemIter aIter( *pAttr );
            sal_uInt16 nWhich = aIter.GetCurItem()->Which();
            while( true )
            {
                if( pFmt->GetFmtAttr( nWhich ) != *aIter.GetCurItem() )
                {
                    bOnlyAttrChg = true;
                    break;
                }

                if( aIter.IsAtEnd() )
                    break;
                nWhich = aIter.NextItem()->Which();
            }
        }

        if( bOnlyAttrChg )
        {
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                GetIDocumentUndoRedo().AppendUndo(
                    MakeUndoUpdateSection( *pFmt, true ) );
            }
            
            
            
            ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());
            pFmt->SetFmtAttr( *pAttr );
            SetModified();
        }
        return;
    }

    
    
    const SwNodeIndex* pIdx = 0;
    {
        if (rNewData.IsHidden())
        {
            pIdx = pFmt->GetCntnt().GetCntntIdx();
            if (pIdx)
            {
                const SwSectionNode* pSectNd =
                    pIdx->GetNode().GetSectionNode();
                if (pSectNd)
                {
                    ::lcl_CheckEmptyLayFrm( GetNodes(), rNewData,
                                *pSectNd, *pSectNd->EndOfSectionNode() );
                }
            }
        }
    }

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo(MakeUndoUpdateSection(*pFmt, false));
    }
    
    
    ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

    
    OUString sCompareString = OUString(sfx2::cTokenSeparator) + OUString(sfx2::cTokenSeparator);
    const bool bUpdate =
           (!pSection->IsLinkType() && rNewData.IsLinkType())
            ||  (!rNewData.GetLinkFileName().isEmpty()
                &&  (rNewData.GetLinkFileName() != sCompareString)
                &&  (rNewData.GetLinkFileName() != pSection->GetLinkFileName()));

    OUString sSectName( rNewData.GetSectionName() );
    if (sSectName != pSection->GetSectionName())
        GetUniqueSectionName( &sSectName );
    else
        sSectName = OUString();

    
    
    
    
    
    
    pSection->SetSectionData(rNewData);

    if( pAttr )
        pSection->GetFmt()->SetFmtAttr( *pAttr );

    if( !sSectName.isEmpty() )
    {
        pSection->SetSectionName( sSectName );
    }

    
    if( pSection->IsHidden() && !pSection->GetCondition().isEmpty() )
    {
        
        SwCalc aCalc( *this );
        if( !pIdx )
            pIdx = pFmt->GetCntnt().GetCntntIdx();
        FldsToCalc( aCalc, pIdx->GetIndex(), USHRT_MAX );

        
        
        
        
        
        
        
        bool bCalculatedCondHidden =
                aCalc.Calculate( pSection->GetCondition() ).GetBool() ? true : false;
        if ( bCalculatedCondHidden && !bOldCondHidden )
        {
            pSection->SetCondHidden( false );
        }
        pSection->SetCondHidden( bCalculatedCondHidden );
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

void sw_DeleteFtn( SwSectionNode *pNd, sal_uLong nStt, sal_uLong nEnd )
{
    SwFtnIdxs& rFtnArr = pNd->GetDoc()->GetFtnIdxs();
    if( rFtnArr.size() )
    {
        sal_uInt16 nPos;
        rFtnArr.SeekEntry( SwNodeIndex( *pNd ), &nPos );
        SwTxtFtn* pSrch;

        
        while( nPos < rFtnArr.size() &&
            _SwTxtFtn_GetIndex( (pSrch = rFtnArr[ nPos ]) ) <= nEnd )
        {
            
            
            pSrch->DelFrms(0);
            ++nPos;
        }

        while( nPos-- &&
            _SwTxtFtn_GetIndex( (pSrch = rFtnArr[ nPos ]) ) >= nStt )
        {
            
            
            pSrch->DelFrms(0);
        }
    }
}

static inline bool lcl_IsTOXSection(SwSectionData const& rSectionData)
{
    return (TOX_CONTENT_SECTION == rSectionData.GetType())
        || (TOX_HEADER_SECTION  == rSectionData.GetType());
}

SwSectionNode* SwNodes::InsertTextSection(SwNodeIndex const& rNdIdx,
                                SwSectionFmt& rSectionFmt,
                                SwSectionData const& rSectionData,
                                SwTOXBase const*const pTOXBase,
                                SwNodeIndex const*const pEnde,
                                bool const bInsAtStart, bool const bCreateFrms)
{
    SwNodeIndex aInsPos( rNdIdx );
    if( !pEnde ) 
    {
        
        OSL_ENSURE(!pEnde || rNdIdx <= *pEnde,
               "Section start and end in wrong order!");

        if( bInsAtStart )
        {
            if (!lcl_IsTOXSection(rSectionData))
            {
                do {
                    aInsPos--;
                } while( aInsPos.GetNode().IsSectionNode() );
                ++aInsPos;
            }
        }
        else
        {
            ++aInsPos;
            if (!lcl_IsTOXSection(rSectionData))
            {
                SwNode* pNd;
                while( aInsPos.GetIndex() < Count() - 1 &&
                        ( pNd = &aInsPos.GetNode())->IsEndNode() &&
                        pNd->StartOfSectionNode()->IsSectionNode())
                {
                    ++aInsPos;
                }
            }
        }
    }

    SwSectionNode *const pSectNd =
            new SwSectionNode(aInsPos, rSectionFmt, pTOXBase);
    if( pEnde )
    {
        
        if( &pEnde->GetNode() != &GetEndOfContent() )
            aInsPos = pEnde->GetIndex()+1;
        
        
        const SwNode* pLastNode = pSectNd->StartOfSectionNode()->EndOfSectionNode();
        if( aInsPos > pLastNode->GetIndex() )
            aInsPos = pLastNode->GetIndex();
        
        
        const SwStartNode* pStartNode = aInsPos.GetNode().StartOfSectionNode();
        
        sal_uLong nMyIndex = pSectNd->GetIndex();
        if( pStartNode->GetIndex() > nMyIndex ) 
        {
            const SwNode* pTemp;
            do
            {
                pTemp = pStartNode; 
                pStartNode = pStartNode->StartOfSectionNode();
            }
            while( pStartNode->GetIndex() > nMyIndex );
            pTemp = pTemp->EndOfSectionNode();
            
            if( pTemp->GetIndex() >= aInsPos.GetIndex() )
                aInsPos = pTemp->GetIndex()+1; 
        }
    }
    else
    {
        SwTxtNode* pCpyTNd = rNdIdx.GetNode().GetTxtNode();
        if( pCpyTNd )
        {
            SwTxtNode* pTNd = new SwTxtNode( aInsPos, pCpyTNd->GetTxtColl() );
            if( pCpyTNd->HasSwAttrSet() )
            {
                
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
                    pTNd->SetAttr( aSet );
                }
                else
                    pTNd->SetAttr( rSet );
            }
            
            pCpyTNd->MakeFrms( *pTNd );
        }
        else
            new SwTxtNode( aInsPos, (SwTxtFmtColl*)GetDoc()->GetDfltTxtFmtColl() );
    }
    new SwEndNode( aInsPos, *pSectNd );

    pSectNd->GetSection().SetSectionData(rSectionData);
    SwSectionFmt* pSectFmt = pSectNd->GetSection().GetFmt();

    
    
    bool bInsFrm = bCreateFrms && !pSectNd->GetSection().IsHidden() &&
                   GetDoc()->GetCurrentViewShell();
    SwNode2Layout *pNode2Layout = NULL;
    if( bInsFrm )
    {
        SwNodeIndex aTmp( *pSectNd );
        if( !pSectNd->GetNodes().FindPrvNxtFrmNode( aTmp, pSectNd->EndOfSectionNode() ) )
            
            pNode2Layout = new SwNode2Layout( *pSectNd );
    }

    
    sal_uLong nEnde = pSectNd->EndOfSectionIndex();
    sal_uLong nStart = pSectNd->GetIndex()+1;
    sal_uLong nSkipIdx = ULONG_MAX;
    for( sal_uLong n = nStart; n < nEnde; ++n )
    {
        SwNode* pNd = (*this)[n];

        
        if( ULONG_MAX == nSkipIdx )
            pNd->pStartOfSection = pSectNd;
        else if( n >= nSkipIdx )
            nSkipIdx = ULONG_MAX;

        if( pNd->IsStartNode() )
        {
            
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

    sw_DeleteFtn( pSectNd, nStart, nEnde );

    if( bInsFrm )
    {
        if( pNode2Layout )
        {
            sal_uLong nIdx = pSectNd->GetIndex();
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
        pTmp = pTmp->pStartOfSection;
    return pTmp->GetSectionNode();
}




static SwSectionFmt &
lcl_initParent(SwSectionNode & rThis, SwSectionFmt & rFmt)
{
    SwSectionNode *const pParent =
        rThis.StartOfSectionNode()->FindSectionNode();
    if( pParent )
    {
        
        rFmt.SetDerivedFrom( pParent->GetSection().GetFmt() );
    }
    return rFmt;
}

SwSectionNode::SwSectionNode(SwNodeIndex const& rIdx,
        SwSectionFmt & rFmt, SwTOXBase const*const pTOXBase)
    : SwStartNode( rIdx, ND_SECTIONNODE )
    , m_pSection( (pTOXBase)
        ? new SwTOXBaseSection(*pTOXBase, lcl_initParent(*this, rFmt))
        : new SwSection( CONTENT_SECTION, rFmt.GetName(),
                lcl_initParent(*this, rFmt) ) )
{
    
    
    rFmt.LockModify();
    rFmt.SetFmtAttr( SwFmtCntnt( this ) );
    rFmt.UnlockModify();
}

#ifdef DBG_UTIL

SwFrm* SwClearDummies( SwFrm* pFrm )
{
    SwFrm* pTmp = pFrm;
    while( pTmp )
    {
        OSL_ENSURE( !pTmp->GetUpper(), "SwClearDummies: No Upper allowed!" );
        if( pTmp->IsSctFrm() )
        {
            SwSectionFrm* pSectFrm = (SwSectionFrm*)pFrm;
            pTmp = pTmp->GetNext();
            if( !pSectFrm->GetLower() )
            {
                if( pSectFrm->GetPrev() )
                    pSectFrm->GetPrev()->mpNext = pTmp;
                else
                    pFrm = pTmp;
                if( pTmp )
                    pTmp->mpPrev = pSectFrm->GetPrev();
                delete pSectFrm;
            }
        }
        else
            pTmp = pTmp->GetNext();
    }
    return pFrm;
}
#endif

SwSectionNode::~SwSectionNode()
{
    
    
    m_pSection->GetFmt()->CallSwClientNotify( SwSectionFrmMoveAndDeleteHint( sal_True ) );
    SwSectionFmt* pFmt = m_pSection->GetFmt();
    if( pFmt )
    {
        
        
        pFmt->LockModify();
        pFmt->ResetFmtAttr( RES_CNTNT );
        pFmt->UnlockModify();
    }
}

SwFrm *SwSectionNode::MakeFrm( SwFrm *pSib )
{
    m_pSection->m_Data.SetHiddenFlag(false);
    return new SwSectionFrm( *m_pSection, pSib );
}



void SwSectionNode::MakeFrms(const SwNodeIndex & rIdx )
{
    
    SwNodes& rNds = GetNodes();
    if( rNds.IsDocNodes() && rNds.GetDoc()->GetCurrentViewShell() )
    {
        if( GetSection().IsHidden() || IsCntntHidden() )
        {
            SwNodeIndex aIdx( *EndOfSectionNode() );
            SwCntntNode* pCNd = rNds.GoNextSection( &aIdx, true, false );
            if( !pCNd )
            {
                aIdx = *this;
                if( 0 == ( pCNd = rNds.GoPrevSection( &aIdx, true, false )) )
                    return ;
            }
            pCNd = aIdx.GetNode().GetCntntNode();
            pCNd->MakeFrms( (SwCntntNode&)rIdx.GetNode() );
        }
        else
        {
            SwNode2Layout aNode2Layout( *this, rIdx.GetIndex() );
            SwFrm *pFrm, *pNew;
            while( 0 != (pFrm = aNode2Layout.NextFrm()) )
            {
                OSL_ENSURE( pFrm->IsSctFrm(), "Depend of Section not a Section." );
                pNew = rIdx.GetNode().GetCntntNode()->MakeFrm( pFrm );

                SwSectionNode* pS = rIdx.GetNode().FindSectionNode();

                
                
                if ( pS )
                {
                    SwTableNode* pTableNode = rIdx.GetNode().FindTableNode();
                    if ( pTableNode &&
                         pTableNode->GetIndex() > pS->GetIndex() )
                    {
                        pS = 0;
                    }
                }

                
                
                
                bool bInitNewSect = false;
                if( pS )
                {
                    SwSectionFrm *pSct = new SwSectionFrm( pS->GetSection(), pFrm );
                    
                    bInitNewSect = true;
                    SwLayoutFrm* pUp = pSct;
                    while( pUp->Lower() )  
                    {
                        OSL_ENSURE( pUp->Lower()->IsLayoutFrm(),"Who's in there?" );
                        pUp = (SwLayoutFrm*)pUp->Lower();
                    }
                    pNew->Paste( pUp, NULL );
                    
                    
                    
                    
                    
                    if ( pNew->IsTxtFrm() )
                    {
                        SwViewShell* pViewShell( pNew->getRootFrm()->GetCurrShell() );
                        if ( pViewShell && pViewShell->GetLayout() &&
                             pViewShell->GetLayout()->IsAnyShellAccessible() )
                        {
                            pViewShell->InvalidateAccessibleParaFlowRelation(
                                dynamic_cast<SwTxtFrm*>(pNew->FindNextCnt( true )),
                                dynamic_cast<SwTxtFrm*>(pNew->FindPrevCnt( true )) );
                        }
                    }
                    pNew = pSct;
                }

                
                if ( rIdx < GetIndex() )
                    
                    pNew->Paste( pFrm->GetUpper(), pFrm );
                else
                    
                    pNew->Paste( pFrm->GetUpper(), pFrm->GetNext() );
                
                
                
                
                
                if ( pNew->IsTxtFrm() )
                {
                    SwViewShell* pViewShell( pNew->getRootFrm()->GetCurrShell() );
                    if ( pViewShell && pViewShell->GetLayout() &&
                         pViewShell->GetLayout()->IsAnyShellAccessible() )
                    {
                        pViewShell->InvalidateAccessibleParaFlowRelation(
                            dynamic_cast<SwTxtFrm*>(pNew->FindNextCnt( true )),
                            dynamic_cast<SwTxtFrm*>(pNew->FindPrevCnt( true )) );
                    }
                }
                if ( bInitNewSect )
                    static_cast<SwSectionFrm*>(pNew)->Init();
            }
        }
    }
}



void SwSectionNode::MakeFrms( SwNodeIndex* pIdxBehind, SwNodeIndex* pEndIdx )
{
    OSL_ENSURE( pIdxBehind, "no Index" );
    SwNodes& rNds = GetNodes();
    SwDoc* pDoc = rNds.GetDoc();

    *pIdxBehind = *this;

    m_pSection->m_Data.SetHiddenFlag(true);

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
    sal_uLong nStt = GetIndex()+1, nEnd = EndOfSectionIndex();
    if( nStt >= nEnd )
    {
        return ;
    }

    SwNodes& rNds = GetNodes();
    m_pSection->GetFmt()->DelFrms();

    
    m_pSection->m_Data.SetHiddenFlag(true);

    
    
    
    {
        SwNodeIndex aIdx( *this );
        if( !rNds.GoPrevSection( &aIdx, true, false ) ||
            !CheckNodesRange( *this, aIdx, true ) ||
            
            !lcl_IsInSameTblBox( rNds, *this, true ))
        {
            aIdx = *EndOfSectionNode();
            if( !rNds.GoNextSection( &aIdx, true, false ) ||
                !CheckNodesRange( *EndOfSectionNode(), aIdx, true ) ||
                
                !lcl_IsInSameTblBox( rNds, *EndOfSectionNode(), false ))
            {
                m_pSection->m_Data.SetHiddenFlag(false);
            }
        }
    }
}

SwSectionNode* SwSectionNode::MakeCopy( SwDoc* pDoc, const SwNodeIndex& rIdx ) const
{
    
    const SwNodes& rNds = GetNodes();

    
    SwSectionFmt* pSectFmt = pDoc->MakeSectionFmt( 0 );
    pSectFmt->CopyAttrs( *GetSection().GetFmt() );

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<SwTOXBase> pTOXBase;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    if (TOX_CONTENT_SECTION == GetSection().GetType())
    {
        OSL_ENSURE( GetSection().ISA( SwTOXBaseSection ), "no TOXBaseSection!" );
        SwTOXBaseSection const& rTBS(
            dynamic_cast<SwTOXBaseSection const&>(GetSection()));
        pTOXBase.reset( new SwTOXBase(rTBS, pDoc) );
    }

    SwSectionNode *const pSectNd =
        new SwSectionNode(rIdx, *pSectFmt, pTOXBase.get());
    SwEndNode* pEndNd = new SwEndNode( rIdx, *pSectNd );
    SwNodeIndex aInsPos( *pEndNd );

    
    SwSection *const pNewSect = pSectNd->m_pSection.get();

    if (TOX_CONTENT_SECTION != GetSection().GetType())
    {
        
        if( rNds.GetDoc() == pDoc && pDoc->IsCopyIsMove() )
        {
            pNewSect->SetSectionName( GetSection().GetSectionName() );
        }
        else
        {
            const OUString sSectionName(GetSection().GetSectionName());
            pNewSect->SetSectionName(pDoc->GetUniqueSectionName( &sSectionName ));
        }
    }

    pNewSect->SetType( GetSection().GetType() );
    pNewSect->SetCondition( GetSection().GetCondition() );
    pNewSect->SetLinkFileName( GetSection().GetLinkFileName() );
    if( !pNewSect->IsHiddenFlag() && GetSection().IsHidden() )
        pNewSect->SetHidden( true );
    if( !pNewSect->IsProtectFlag() && GetSection().IsProtect() )
        pNewSect->SetProtect( true );
    
    if( !pNewSect->IsEditInReadonlyFlag() && GetSection().IsEditInReadonly() )
        pNewSect->SetEditInReadonly( true );

    SwNodeRange aRg( *this, +1, *EndOfSectionNode() ); 
    rNds._Copy( aRg, aInsPos, sal_False );

    
    
    pSectNd->DelFrms();

    
    if( pNewSect->IsLinkType() ) 
        pNewSect->CreateLink( pDoc->GetCurrentViewShell() ? CREATE_CONNECT : CREATE_NONE );

    
    if (m_pSection->IsServer()
        && pDoc->GetIDocumentUndoRedo().IsUndoNodes(rNds))
    {
        pNewSect->SetRefObject( m_pSection->GetObject() );
        pDoc->GetLinkManager().InsertServer( pNewSect->GetObject() );
    }

    
    pSectFmt->RegisterAsCopyOf(*GetSection().GetFmt());

    return pSectNd;
}

sal_Bool SwSectionNode::IsCntntHidden() const
{
    OSL_ENSURE( !m_pSection->IsHidden(),
            "That's simple: Hidden Section => Hidden Content" );
    SwNodeIndex aTmp( *this, 1 );
    sal_uLong nEnd = EndOfSectionIndex();
    while( aTmp < nEnd )
    {
        if( aTmp.GetNode().IsSectionNode() )
        {
            const SwSection& rSect = ((SwSectionNode&)aTmp.GetNode()).GetSection();
            if( rSect.IsHiddenFlag() )
                
                aTmp = *aTmp.GetNode().EndOfSectionNode();
        }
        else
        {
            if( aTmp.GetNode().IsCntntNode() || aTmp.GetNode().IsTableNode() )
                return sal_False; 
            OSL_ENSURE( aTmp.GetNode().IsEndNode(), "EndNode expected" );
        }
        ++aTmp;
    }
    return sal_True; 
}

void SwSectionNode::NodesArrChgd()
{
    SwSectionFmt *const pFmt = m_pSection->GetFmt();
    if( pFmt )
    {
        SwNodes& rNds = GetNodes();
        SwDoc* pDoc = pFmt->GetDoc();

        if( !rNds.IsDocNodes() )
        {
            SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT, pFmt );
            pFmt->ModifyNotification( &aMsgHint, &aMsgHint );
        }

        pFmt->LockModify();
        pFmt->SetFmtAttr( SwFmtCntnt( this ));
        pFmt->UnlockModify();

        SwSectionNode* pSectNd = StartOfSectionNode()->FindSectionNode();
        
        pFmt->SetDerivedFrom( pSectNd ? pSectNd->GetSection().GetFmt()
                                      : pDoc->GetDfltFrmFmt() );

        
        sal_uLong nStart = GetIndex()+1, nEnde = EndOfSectionIndex();
        for( sal_uLong n = nStart; n < nEnde; ++n )
            
            if( 0 != ( pSectNd = rNds[ n ]->GetSectionNode() ) )
            {
                pSectNd->GetSection().GetFmt()->SetDerivedFrom( pFmt );
                n = pSectNd->EndOfSectionIndex();
            }

        
        if( rNds.IsDocNodes() )
        {
            OSL_ENSURE( pDoc == GetDoc(),
                    "Moving to different Documents?" );
            if( m_pSection->IsLinkType() ) 
                m_pSection->CreateLink( pDoc->GetCurrentViewShell() ? CREATE_CONNECT : CREATE_NONE );

            if (m_pSection->IsServer())
                pDoc->GetLinkManager().InsertServer( m_pSection->GetObject() );
        }
        else
        {
            if (CONTENT_SECTION != m_pSection->GetType())
                pDoc->GetLinkManager().Remove( &m_pSection->GetBaseLink() );

            if (m_pSection->IsServer())
                pDoc->GetLinkManager().RemoveServer( m_pSection->GetObject() );
        }
    }
}

OUString SwDoc::GetUniqueSectionName( const OUString* pChkStr ) const
{
    const OUString aName( ResId( STR_REGION_DEFNAME, *pSwResMgr ) );

    sal_uInt16 nNum = 0;
    sal_uInt16 nTmp, nFlagSize = ( mpSectionFmtTbl->size() / 8 ) +2;
    sal_uInt8* pSetFlags = new sal_uInt8[ nFlagSize ];
    memset( pSetFlags, 0, nFlagSize );

    const SwSectionNode* pSectNd;
    sal_uInt16 n;

    for( n = 0; n < mpSectionFmtTbl->size(); ++n )
        if( 0 != ( pSectNd = (*mpSectionFmtTbl)[ n ]->GetSectionNode( false ) ))
        {
            const OUString rNm = pSectNd->GetSection().GetSectionName();
            if (rNm.startsWith( aName ))
            {
                
                nNum = static_cast<sal_uInt16>(rNm.copy( aName.getLength() ).toInt32());
                if( nNum-- && nNum < mpSectionFmtTbl->size() )
                    pSetFlags[ nNum / 8 ] |= (0x01 << ( nNum & 0x07 ));
            }
            if( pChkStr && *pChkStr==rNm )
                pChkStr = 0;
        }

    if( !pChkStr )
    {
        
        nNum = mpSectionFmtTbl->size();
        for( n = 0; n < nFlagSize; ++n )
            if( 0xff != ( nTmp = pSetFlags[ n ] ))
            {
                
                nNum = n * 8;
                while( nTmp & 1 )
                    ++nNum, nTmp >>= 1;
                break;
            }

    }
    delete [] pSetFlags;
    if( pChkStr )
        return *pChkStr;
    return aName + OUString::number( ++nNum );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
