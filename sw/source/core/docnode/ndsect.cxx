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
// #i27138#
#include <viewsh.hxx>
#include <txtfrm.hxx>


// #i21457# - new implementation of local method <lcl_IsInSameTblBox(..)>.
// Method now determines the previous/next on its own. Thus, it can be controlled,
// for which previous/next is checked, if it's visible.
bool lcl_IsInSameTblBox( SwNodes& _rNds,
                         const SwNode& _rNd,
                         const bool _bPrev )
{
    const SwTableNode* pTblNd = _rNd.FindTableNode();
    if ( !pTblNd )
    {
        return true;
    }

    // determine index to be checked. Its assumed that a previous/next exist.
    SwNodeIndex aChkIdx( _rNd );
    {
        // determine index of previous/next - skip hidden ones, which are
        // inside the table.
        // If found one is before/after table, this one isn't in the same
        // table box as <_rNd>.
        bool bFound = false;
        do
        {
            if ( _bPrev
                 ? !_rNds.GoPrevSection( &aChkIdx, sal_False, sal_False )
                 : !_rNds.GoNextSection( &aChkIdx, sal_False, sal_False ) )
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
                    // check, if found one isn't inside a hidden section, which
                    // is also inside the table.
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

    // dann suche den StartNode der Box
    const SwTableSortBoxes& rSortBoxes = pTblNd->GetTable().GetTabSortBoxes();
    sal_uLong nIdx = _rNd.GetIndex();
    for (size_t n = 0; n < rSortBoxes.size(); ++n)
    {
        const SwStartNode* pNd = rSortBoxes[ n ]->GetSttNd();
        if ( pNd->GetIndex() < nIdx && nIdx < pNd->EndOfSectionIndex() )
        {
            // dann muss der andere Index in derselben Section liegen
            nIdx = aChkIdx.GetIndex();
            return pNd->GetIndex() < nIdx && nIdx < pNd->EndOfSectionIndex();
        }
    }

    return true;
}

void lcl_CheckEmptyLayFrm( SwNodes& rNds, SwSectionData& rSectionData,
                        const SwNode& rStt, const SwNode& rEnd )
{
    SwNodeIndex aIdx( rStt );
    if( !rNds.GoPrevSection( &aIdx, sal_True, sal_False ) ||
        !CheckNodesRange( rStt, aIdx, sal_True ) ||
        // #i21457#
        !lcl_IsInSameTblBox( rNds, rStt, true ))
    {
        aIdx = rEnd;
        if( !rNds.GoNextSection( &aIdx, sal_True, sal_False ) ||
            !CheckNodesRange( rEnd, aIdx, sal_True ) ||
            // #i21457#
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
        // demoted to info because this is called from SwXTextSection::attach,
        // so it could be invalid input
        SAL_INFO("sw.core" , "InsertSwSection: rRange overlaps other sections");
        return 0;
    }

    // Teste ob das gesamte Dokument versteckt werden soll,
    // koennen wir zur Zeit nicht !!!!
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
            OSL_ENSURE( pPrvNd, "der SectionNode fehlt" );
            SwNodeIndex aStt( pSttPos->nNode ), aEnd( pEndPos->nNode, +1 );
            while( pPrvNd != aStt.GetNode().StartOfSectionNode() )
                aStt--;
            while( pPrvNd != aEnd.GetNode().StartOfSectionNode() )
                aEnd++;

            --aEnd;     // im InsertSection ist Ende inclusive
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
                        pUndoInsSect->SaveSplitNode( pTNd, sal_True );
                    }
                }

                if ( !( pPrvNd && 2 == nRegionRet ) )
                {
                    SwTxtNode *const pTNd =
                        pEndPos->nNode.GetNode().GetTxtNode();
                    if (pTNd &&
                        (pTNd->GetTxt().Len() != pEndPos->nContent.GetIndex()))
                    {
                        pUndoInsSect->SaveSplitNode( pTNd, sal_False );
                    }
                }
            }

            const SwCntntNode* pCNd;
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
                pCNd = pEndPos->nNode.GetNode().GetCntntNode();
                if( pCNd && pCNd->Len() != pEndPos->nContent.GetIndex() )
                {
                    xub_StrLen nCntnt = pSttPos->nContent.GetIndex();
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
                        // wieder ans Ende vom vorherigen setzen
                        pEndPos->nNode--;
                        pTNd = pEndPos->nNode.GetNode().GetTxtNode();
                    }
                    if( pTNd ) nCntnt = pTNd->GetTxt().Len(); else nCntnt = 0;
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
                pUndoInsSect->SaveSplitNode( (SwTxtNode*)pCNd, sal_True );
            }
            SplitNode( *pPos, false );
            pNewSectNode = GetNodes().InsertTextSection(
                pPos->nNode, *pFmt, rNewData, pTOXBase, 0, true);
        }
    }

//FEATURE::CONDCOLL
    pNewSectNode->CheckSectionCondColl();
//FEATURE::CONDCOLL

    SetRedlineMode_intern( eOld );

    if( IsRedlineOn() || (!IsIgnoreRedline() && !pRedlineTbl->empty() ))
    {
        SwPaM aPam( *pNewSectNode->EndOfSectionNode(), *pNewSectNode, 1 );
        if( IsRedlineOn() )
        {
            AppendRedline( new SwRedline( nsRedlineType_t::REDLINE_INSERT, aPam ), true);
        }
        else
        {
            SplitRedline( aPam );
        }
    }

    // ist eine Condition gesetzt
    if (rNewData.IsHidden() && rNewData.GetCondition().Len())
    {
        // dann berechne bis zu dieser Position
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
        // teste ob es sich um eine gueltige Selektion handelt
        const SwPosition* pStt = rRange.Start(),
                        * pEnd = rRange.End();

        const SwCntntNode* pCNd = pEnd->nNode.GetNode().GetCntntNode();
        const SwNode* pNd = &pStt->nNode.GetNode();
        const SwSectionNode* pSectNd = pNd->FindSectionNode();
        const SwSectionNode* pEndSectNd = pCNd ? pCNd->FindSectionNode() : 0;
        if( pSectNd && pEndSectNd && pSectNd != pEndSectNd )
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
                    aIdx++;
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
    pSectionFmtTbl->push_back( pNew );
    return pNew;
}

void SwDoc::DelSectionFmt( SwSectionFmt *pFmt, sal_Bool bDelNodes )
{
    SwSectionFmts::iterator itFmtPos = std::find( pSectionFmtTbl->begin(), pSectionFmtTbl->end(), pFmt );

    GetIDocumentUndoRedo().StartUndo(UNDO_DELSECTION, NULL);

    if( pSectionFmtTbl->end() != itFmtPos )
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
                //#126178# start/end undo have to be pairs!
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
            //#126178# start/end undo have to be pairs!
            GetIDocumentUndoRedo().EndUndo(UNDO_DELSECTION, NULL);
            return ;
        }

        {
            SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT, pFmt );
            pFmt->ModifyNotification( &aMsgHint, &aMsgHint );
        }

        // A ClearRedo could result in a rekursive call of this function and delete some section
        // formats => the position iside the SectionFmtTbl could have changed
        itFmtPos = std::find( pSectionFmtTbl->begin(), pSectionFmtTbl->end(), pFmt );

        // ACHTUNG: erst aus dem Array entfernen und dann loeschen.
        //          Der Section-DTOR versucht selbst noch sein Format
        //          zu loeschen!
        pSectionFmtTbl->erase( itFmtPos );
//FEATURE::CONDCOLL
        sal_uLong nCnt = 0, nSttNd = 0;
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

    GetIDocumentUndoRedo().EndUndo(UNDO_DELSECTION, NULL);

    SetModified();
}

void SwDoc::UpdateSection(sal_uInt16 const nPos, SwSectionData & rNewData,
        SfxItemSet const*const pAttr, bool const bPreventLinkUpdate)
{
    SwSectionFmt* pFmt = (*pSectionFmtTbl)[ nPos ];
    SwSection* pSection = pFmt->GetSection();

    /// remember hidden condition flag of SwSection before changes
    bool bOldCondHidden = pSection->IsCondHidden() ? true : false;

    if (pSection->DataEquals(rNewData))
    {
        // die Attribute ueberpruefen
        sal_Bool bOnlyAttrChg = sal_False;
        if( pAttr && pAttr->Count() )
        {
            SfxItemIter aIter( *pAttr );
            sal_uInt16 nWhich = aIter.GetCurItem()->Which();
            while( sal_True )
            {
                if( pFmt->GetFmtAttr( nWhich ) != *aIter.GetCurItem() )
                {
                    bOnlyAttrChg = sal_True;
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
            // #i32968# Inserting columns in the section causes MakeFrmFmt
            // to put two  objects of type SwUndoFrmFmt on the undo stack.
            // We don't want them.
            ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());
            pFmt->SetFmtAttr( *pAttr );
            SetModified();
        }
        return;
    }

    // Teste ob eine gesamte Content-Section (Dokument/TabellenBox/Fly)
    // versteckt werden soll, koennen wir zur Zeit nicht !!!!
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
    // #i32968# Inserting columns in the section causes MakeFrmFmt to put two
    // objects of type SwUndoFrmFmt on the undo stack. We don't want them.
    ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

    // Der LinkFileName koennte auch nur aus Separatoren bestehen
    String sCompareString = rtl::OUString(sfx2::cTokenSeperator);
    sCompareString += sfx2::cTokenSeperator;
    const bool bUpdate =
           (!pSection->IsLinkType() && rNewData.IsLinkType())
        || (    rNewData.GetLinkFileName().Len()
            &&  (rNewData.GetLinkFileName() != sCompareString)
            &&  (rNewData.GetLinkFileName() != pSection->GetLinkFileName()));

    String sSectName( rNewData.GetSectionName() );
    if (sSectName != pSection->GetSectionName())
        GetUniqueSectionName( &sSectName );
    else
        sSectName.Erase();

    /// In SwSection::operator=(..) class member bCondHiddenFlag is always set to sal_True.
    /// IMHO this have to be changed, but I can't estimate the consequences:
    /// Either it is set to sal_True using corresponding method <SwSection.SetCondHidden(..)>,
    /// or it is set to the value of SwSection which is assigned to it.
    /// Discussion with AMA results that the adjustment to the assignment operator
    /// could be very risky.
    pSection->SetSectionData(rNewData);

    if( pAttr )
        pSection->GetFmt()->SetFmtAttr( *pAttr );

    if( sSectName.Len() )
    {
        pSection->SetSectionName( sSectName );
    }

    // ist eine Condition gesetzt
    if( pSection->IsHidden() && pSection->GetCondition().Len() )
    {
        // dann berechne bis zu dieser Position
        SwCalc aCalc( *this );
        if( !pIdx )
            pIdx = pFmt->GetCntnt().GetCntntIdx();
        FldsToCalc( aCalc, pIdx->GetIndex(), USHRT_MAX );

        /// Because on using SwSection::operator=() to set up <pSection>
        /// with <rNewData> and the above given note, the hidden condition flag
        /// has to be set to sal_False, if hidden condition flag of <pFmt->GetSection()>
        /// (SwSection before the changes) is sal_False (already saved in <bOldCondHidden>)
        /// and new calculated condition is sal_True.
        /// This is necessary, because otherwise the <SetCondHidden> would have
        /// no effect.
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

/* --------------------------------------------------
 * LockFrms wurde im InsertSection genutzt, um zu verhindern, dass
 * SectionFrms durch das DelFrms zerstoert werden. Dies ist durch
 * den Destroy-Listen-Mechanismus ueberfluessig geworden.
 * Falls diese Methode doch noch einmal reanimiert wird, bietet es
 * sich vielleicht an, beim Entlocken die SectionFrms auf Inhalt zu
 * pruefen und dann ggf. zur Zerstoerung anzumelden.
 * --------------------------------------------------*/

// und dann waren da noch die Fussnoten:
void lcl_DeleteFtn( SwSectionNode *pNd, sal_uLong nStt, sal_uLong nEnd )
{
    SwFtnIdxs& rFtnArr = pNd->GetDoc()->GetFtnIdxs();
    if( rFtnArr.size() )
    {
        sal_uInt16 nPos;
        rFtnArr.SeekEntry( SwNodeIndex( *pNd ), &nPos );
        SwTxtFtn* pSrch;

        // loesche erstmal alle, die dahinter stehen
        while( nPos < rFtnArr.size() &&
            _SwTxtFtn_GetIndex( (pSrch = rFtnArr[ nPos ]) ) <= nEnd )
        {
            // Werden die Nodes nicht geloescht mussen sie bei den Seiten
            // abmeldet (Frms loeschen) werden, denn sonst bleiben sie
            // stehen (Undo loescht sie nicht!)
            pSrch->DelFrms(0);
            ++nPos;
        }

        while( nPos-- &&
            _SwTxtFtn_GetIndex( (pSrch = rFtnArr[ nPos ]) ) >= nStt )
        {
            // Werden die Nodes nicht geloescht mussen sie bei den Seiten
            // abmeldet (Frms loeschen) werden, denn sonst bleiben sie
            // stehen (Undo loescht sie nicht!)
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
    if( !pEnde )        // kein Bereich also neue Section davor/hinter anlegen
    {
        // #i26762#
        OSL_ENSURE(!pEnde || rNdIdx <= *pEnde,
               "Section start and end in wrong order!");

        if( bInsAtStart )
        {
            if (!lcl_IsTOXSection(rSectionData))
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
            if (!lcl_IsTOXSection(rSectionData))
            {
                while( aInsPos.GetIndex() < Count() - 1 &&
                        ( pNd = &aInsPos.GetNode())->IsEndNode() &&
                        pNd->StartOfSectionNode()->IsSectionNode())
                {
                    aInsPos++;
                }
            }
        }
    }

    SwSectionNode *const pSectNd =
            new SwSectionNode(aInsPos, rSectionFmt, pTOXBase);
    if( pEnde )
    {
        // Sonderfall fuer die Reader/Writer
        if( &pEnde->GetNode() != &GetEndOfContent() )
            aInsPos = pEnde->GetIndex()+1;
        // #i58710: We created a RTF document with a section break inside a table cell
        // We are not able to handle a section start inside a table and the section end outside.
        const SwNode* pLastNode = pSectNd->StartOfSectionNode()->EndOfSectionNode();
        if( aInsPos > pLastNode->GetIndex() )
            aInsPos = pLastNode->GetIndex();
        // Another way round: if the section starts outside a table but the end is inside...
        // aInsPos is at the moment the Position where my EndNode will be inserted
        const SwStartNode* pStartNode = aInsPos.GetNode().StartOfSectionNode();
        // This StartNode should be in front of me, but if not, I wanna survive
        sal_uLong nMyIndex = pSectNd->GetIndex();
        if( pStartNode->GetIndex() > nMyIndex ) // Suspicious!
        {
            const SwNode* pTemp;
            do
            {
                pTemp = pStartNode; // pTemp is a suspicious one
                pStartNode = pStartNode->StartOfSectionNode();
            }
            while( pStartNode->GetIndex() > nMyIndex );
            pTemp = pTemp->EndOfSectionNode();
            // If it starts behind me but ends behind my end...
            if( pTemp->GetIndex() >= aInsPos.GetIndex() )
                aInsPos = pTemp->GetIndex()+1; // ...I have to correct my end position
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
                    pTNd->SetAttr( aSet );
                }
                else
                    pTNd->SetAttr( rSet );
            }
            // den Frame anlegen nicht vergessen !!
            pCpyTNd->MakeFrms( *pTNd );
        }
        else
            new SwTxtNode( aInsPos, (SwTxtFmtColl*)GetDoc()->GetDfltTxtFmtColl() );
    }
    new SwEndNode( aInsPos, *pSectNd );

    pSectNd->GetSection().SetSectionData(rSectionData);
    SwSectionFmt* pSectFmt = pSectNd->GetSection().GetFmt();

    // Hier bietet sich als Optimierung an, vorhandene Frames nicht zu
    // zerstoeren und wieder neu anzulegen, sondern nur umzuhaengen.
    sal_Bool bInsFrm = bCreateFrms && !pSectNd->GetSection().IsHidden() &&
                   GetDoc()->GetCurrentViewShell(); //swmod 071108//swmod 071225
    SwNode2Layout *pNode2Layout = NULL;
    if( bInsFrm )
    {
        SwNodeIndex aTmp( *pSectNd );
        if( !pSectNd->GetNodes().FindPrvNxtFrmNode( aTmp, pSectNd->EndOfSectionNode() ) )
            // dann sammel mal alle Uppers ein
            pNode2Layout = new SwNode2Layout( *pSectNd );
    }

    // jetzt noch bei allen im Bereich den richtigen StartNode setzen
    sal_uLong nEnde = pSectNd->EndOfSectionIndex();
    sal_uLong nStart = pSectNd->GetIndex()+1;
    sal_uLong nSkipIdx = ULONG_MAX;
    for( sal_uLong n = nStart; n < nEnde; ++n )
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


//---------
// SwSectionNode
//---------

// ugly hack to make m_pSection const
static SwSectionFmt &
lcl_initParent(SwSectionNode & rThis, SwSectionFmt & rFmt)
{
    SwSectionNode *const pParent =
        rThis.StartOfSectionNode()->FindSectionNode();
    if( pParent )
    {
        // das Format beim richtigen Parent anmelden.
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
    // jetzt noch die Verbindung von Format zum Node setzen
    // Modify unterdruecken, interresiert keinen
    rFmt.LockModify();
    rFmt.SetFmtAttr( SwFmtCntnt( this ) );
    rFmt.UnlockModify();
}

#ifdef DBG_UTIL
//remove superfluous SectionFrms
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
#endif

SwSectionNode::~SwSectionNode()
{
    // mba: test if iteration works as clients will be removed in callback
    // use hint which allows to specify, if the content shall be saved or not
    m_pSection->GetFmt()->CallSwClientNotify( SwSectionFrmMoveAndDeleteHint( sal_True ) );
    SwSectionFmt* pFmt = m_pSection->GetFmt();
    if( pFmt )
    {
        // das Attribut entfernen, weil die Section ihr Format loescht
        // und falls das Cntnt-Attribut vorhanden ist, die Section aufhebt.
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

//Methode erzeugt fuer den vorhergehenden Node alle Ansichten vom
//Dokument. Die erzeugten Contentframes werden in das entsprechende
//Layout gehaengt.
void SwSectionNode::MakeFrms(const SwNodeIndex & rIdx )
{
    // also nehme meinen nachfolgenden oder vorhergehenden ContentFrame:
    SwNodes& rNds = GetNodes();
    if( rNds.IsDocNodes() && rNds.GetDoc()->GetCurrentViewShell() ) //swmod 071108//swmod 071225
    {
        if( GetSection().IsHidden() || IsCntntHidden() )
        {
            SwNodeIndex aIdx( *EndOfSectionNode() );
            SwCntntNode* pCNd = rNds.GoNextSection( &aIdx, sal_True, sal_False );
            if( !pCNd )
            {
                aIdx = *this;
                if( 0 == ( pCNd = rNds.GoPrevSection( &aIdx, sal_True, sal_False )) )
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
                OSL_ENSURE( pFrm->IsSctFrm(), "Depend von Section keine Section." );
                pNew = rIdx.GetNode().GetCntntNode()->MakeFrm( pFrm );

                SwSectionNode* pS = rIdx.GetNode().FindSectionNode();

                // Assure that node is not inside a table, which is inside the
                // found section.
                if ( pS )
                {
                    SwTableNode* pTableNode = rIdx.GetNode().FindTableNode();
                    if ( pTableNode &&
                         pTableNode->GetIndex() > pS->GetIndex() )
                    {
                        pS = 0;
                    }
                }

                // if the node is in a section, the sectionframe now
                // has to be created..
                // boolean to control <Init()> of a new section frame.
                bool bInitNewSect = false;
                if( pS )
                {
                    SwSectionFrm *pSct = new SwSectionFrm( pS->GetSection(), pFrm );
                    // prepare <Init()> of new section frame.
                    bInitNewSect = true;
                    SwLayoutFrm* pUp = pSct;
                    while( pUp->Lower() )  // for columned sections
                    {
                        OSL_ENSURE( pUp->Lower()->IsLayoutFrm(),"Who's in there?" );
                        pUp = (SwLayoutFrm*)pUp->Lower();
                    }
                    pNew->Paste( pUp, NULL );
                    // #i27138#
                    // notify accessibility paragraphs objects about changed
                    // CONTENT_FLOWS_FROM/_TO relation.
                    // Relation CONTENT_FLOWS_FROM for next paragraph will change
                    // and relation CONTENT_FLOWS_TO for previous paragraph will change.
                    if ( pNew->IsTxtFrm() )
                    {
                        ViewShell* pViewShell( pNew->getRootFrm()->GetCurrShell() );
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

                // wird ein Node vorher oder nachher mit Frames versehen
                if ( rIdx < GetIndex() )
                    // der neue liegt vor mir
                    pNew->Paste( pFrm->GetUpper(), pFrm );
                else
                    // der neue liegt hinter mir
                    pNew->Paste( pFrm->GetUpper(), pFrm->GetNext() );
                // #i27138#
                // notify accessibility paragraphs objects about changed
                // CONTENT_FLOWS_FROM/_TO relation.
                // Relation CONTENT_FLOWS_FROM for next paragraph will change
                // and relation CONTENT_FLOWS_TO for previous paragraph will change.
                if ( pNew->IsTxtFrm() )
                {
                    ViewShell* pViewShell( pNew->getRootFrm()->GetCurrShell() );
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

//Fuer jedes vorkommen im Layout einen SectionFrm anlegen und vor den
//entsprechenden CntntFrm pasten.

void SwSectionNode::MakeFrms( SwNodeIndex* pIdxBehind, SwNodeIndex* pEndIdx )
{
    OSL_ENSURE( pIdxBehind, "kein Index" );
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

    // unser Flag muessen wir noch aktualisieren
    m_pSection->m_Data.SetHiddenFlag(true);

    // Bug 30582: falls der Bereich in Fly oder TabellenBox ist, dann
    //              kann er nur "gehiddet" werden, wenn weiterer Content
    //              vorhanden ist, der "Frames" haelt. Sonst hat der
    //              Fly/TblBox-Frame keinen Lower !!!
    {
        SwNodeIndex aIdx( *this );
        if( !rNds.GoPrevSection( &aIdx, sal_True, sal_False ) ||
            !CheckNodesRange( *this, aIdx, sal_True ) ||
            // #i21457#
            !lcl_IsInSameTblBox( rNds, *this, true ))
        {
            aIdx = *EndOfSectionNode();
            if( !rNds.GoNextSection( &aIdx, sal_True, sal_False ) ||
                !CheckNodesRange( *EndOfSectionNode(), aIdx, sal_True ) ||
                // #i21457#
                !lcl_IsInSameTblBox( rNds, *EndOfSectionNode(), false ))
            {
                m_pSection->m_Data.SetHiddenFlag(false);
            }
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

    // Werte uebertragen
    SwSection *const pNewSect = pSectNd->m_pSection.get();

    if (TOX_CONTENT_SECTION != GetSection().GetType())
    {
        // beim Move den Namen beibehalten
        if( rNds.GetDoc() == pDoc && pDoc->IsCopyIsMove() )
        {
            pNewSect->SetSectionName( GetSection().GetSectionName() );
        }
        else
        {
            pNewSect->SetSectionName(
                pDoc->GetUniqueSectionName( &GetSection().GetSectionName() ));
        }
    }


    pNewSect->SetType( GetSection().GetType() );
    pNewSect->SetCondition( GetSection().GetCondition() );
    pNewSect->SetLinkFileName( GetSection().GetLinkFileName() );
    if( !pNewSect->IsHiddenFlag() && GetSection().IsHidden() )
        pNewSect->SetHidden( sal_True );
    if( !pNewSect->IsProtectFlag() && GetSection().IsProtect() )
        pNewSect->SetProtect( sal_True );
    // edit in readonly sections
    if( !pNewSect->IsEditInReadonlyFlag() && GetSection().IsEditInReadonly() )
        pNewSect->SetEditInReadonly( sal_True );

    SwNodeRange aRg( *this, +1, *EndOfSectionNode() );  // (wo stehe in denn nun ??)
    rNds._Copy( aRg, aInsPos, sal_False );

    // loesche alle Frames vom kopierten Bereich, diese werden beim
    // erzeugen des SectionFrames angelegt !
    pSectNd->DelFrms();

    // dann kopiere auch noch die Links/Server
    if( pNewSect->IsLinkType() )        // den Link eintragen
        pNewSect->CreateLink( pDoc->GetCurrentViewShell() ? CREATE_CONNECT  //swmod 071108//swmod 071225
                                                 : CREATE_NONE );

    // falls als Server aus dem Undo kopiert wird, wieder eintragen
    if (m_pSection->IsServer()
        && pDoc->GetIDocumentUndoRedo().IsUndoNodes(rNds))
    {
        pNewSect->SetRefObject( m_pSection->GetObject() );
        pDoc->GetLinkManager().InsertServer( pNewSect->GetObject() );
    }

    // METADATA: copy xml:id; must be done after insertion of node
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
                // dann diese Section ueberspringen
                aTmp = *aTmp.GetNode().EndOfSectionNode();
        }
        else
        {
            if( aTmp.GetNode().IsCntntNode() || aTmp.GetNode().IsTableNode() )
                return sal_False; // Nicht versteckter Inhalt wurde gefunden
            OSL_ENSURE( aTmp.GetNode().IsEndNode(), "EndNode expected" );
        }
        aTmp++;
    }
    return sal_True; // Alles versteckt
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
        // set the correct parent from the new section
        pFmt->SetDerivedFrom( pSectNd ? pSectNd->GetSection().GetFmt()
                                      : pDoc->GetDfltFrmFmt() );

        // jetzt noch bei allen im Bereich den richtigen StartNode setzen
        sal_uLong nStart = GetIndex()+1, nEnde = EndOfSectionIndex();
        for( sal_uLong n = nStart; n < nEnde; ++n )
            // die Verschachtelung der Formate herstellen!
            if( 0 != ( pSectNd = rNds[ n ]->GetSectionNode() ) )
            {
                pSectNd->GetSection().GetFmt()->SetDerivedFrom( pFmt );
                n = pSectNd->EndOfSectionIndex();
            }

        // verschieben vom Nodes- ins UndoNodes-Array?
        if( rNds.IsDocNodes() )
        {
            OSL_ENSURE( pDoc == GetDoc(),
                    "verschieben in unterschiedliche Documente?" );
            if( m_pSection->IsLinkType() )      // den Link austragen
                m_pSection->CreateLink( pDoc->GetCurrentViewShell() ? CREATE_CONNECT    //swmod 071108
                                                         : CREATE_NONE );//swmod 071225
            if (m_pSection->IsServer())
            {
                pDoc->GetLinkManager().InsertServer( m_pSection->GetObject() );
            }
        }
        else
        {
            if (CONTENT_SECTION != m_pSection->GetType())
            {
                pDoc->GetLinkManager().Remove( &m_pSection->GetBaseLink() );
            }

            if (m_pSection->IsServer())
            {
                pDoc->GetLinkManager().RemoveServer( m_pSection->GetObject() );
            }
        }
    }
}


String SwDoc::GetUniqueSectionName( const String* pChkStr ) const
{
    ResId aId( STR_REGION_DEFNAME, *pSwResMgr );
    String aName( aId );
    xub_StrLen nNmLen = aName.Len();

    sal_uInt16 nNum = 0;
    sal_uInt16 nTmp, nFlagSize = ( pSectionFmtTbl->size() / 8 ) +2;
    sal_uInt8* pSetFlags = new sal_uInt8[ nFlagSize ];
    memset( pSetFlags, 0, nFlagSize );

    const SwSectionNode* pSectNd;
    sal_uInt16 n;

    for( n = 0; n < pSectionFmtTbl->size(); ++n )
        if( 0 != ( pSectNd = (*pSectionFmtTbl)[ n ]->GetSectionNode( sal_False ) ))
        {
            const String& rNm = pSectNd->GetSection().GetSectionName();
            if( rNm.Match( aName ) == nNmLen )
            {
                // Nummer bestimmen und das Flag setzen
                nNum = static_cast<sal_uInt16>(rNm.Copy( nNmLen ).ToInt32());
                if( nNum-- && nNum < pSectionFmtTbl->size() )
                    pSetFlags[ nNum / 8 ] |= (0x01 << ( nNum & 0x07 ));
            }
            if( pChkStr && pChkStr->Equals( rNm ) )
                pChkStr = 0;
        }

    if( !pChkStr )
    {
        // alle Nummern entsprechend geflag, also bestimme die richtige Nummer
        nNum = pSectionFmtTbl->size();
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
    delete [] pSetFlags;
    if( pChkStr )
        return *pChkStr;
    return aName += String::CreateFromInt32( ++nNum );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
