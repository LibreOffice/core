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
#include <IDocumentLinksAdministration.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentState.hxx>
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
#include <memory>
#include <ndsect.hxx>
#include <tools/datetimeutils.hxx>

// #i21457# - new implementation of local method <lcl_IsInSameTableBox(..)>.
// Method now determines the previous/next on its own. Thus, it can be controlled,
// for which previous/next is checked, if it's visible.
static bool lcl_IsInSameTableBox( SwNodes& _rNds,
                         const SwNode& _rNd,
                         const bool _bPrev )
{
    const SwTableNode* pTableNd = _rNd.FindTableNode();
    if ( !pTableNd )
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
                 ? !SwNodes::GoPrevSection( &aChkIdx, false, false )
                 : !_rNds.GoNextSection( &aChkIdx, false, false ) )
            {
                OSL_FAIL( "<lcl_IsInSameTableBox(..)> - no previous/next!" );
                return false;
            }
            else
            {
                if ( aChkIdx < pTableNd->GetIndex() ||
                     aChkIdx > pTableNd->EndOfSectionNode()->GetIndex() )
                {
                    return false;
                }
                else
                {
                    // check, if found one isn't inside a hidden section, which
                    // is also inside the table.
                    SwSectionNode* pSectNd = aChkIdx.GetNode().FindSectionNode();
                    if ( !pSectNd ||
                         pSectNd->GetIndex() < pTableNd->GetIndex() ||
                         !pSectNd->GetSection().IsHiddenFlag() )
                    {
                        bFound = true;
                    }
                }
            }
        } while ( !bFound );
    }

    // Find the Box's StartNode
    const SwTableSortBoxes& rSortBoxes = pTableNd->GetTable().GetTabSortBoxes();
    sal_uLong nIdx = _rNd.GetIndex();
    for (size_t n = 0; n < rSortBoxes.size(); ++n)
    {
        const SwStartNode* pNd = rSortBoxes[ n ]->GetSttNd();
        if ( pNd->GetIndex() < nIdx && nIdx < pNd->EndOfSectionIndex() )
        {
            // The other index needs to be within the same Section
            nIdx = aChkIdx.GetIndex();
            return pNd->GetIndex() < nIdx && nIdx < pNd->EndOfSectionIndex();
        }
    }

    return true;
}

static void lcl_CheckEmptyLayFrame( SwNodes& rNds, SwSectionData& rSectionData,
                        const SwNode& rStt, const SwNode& rEnd )
{
    SwNodeIndex aIdx( rStt );
    if( !SwNodes::GoPrevSection( &aIdx, true, false ) ||
        !CheckNodesRange( rStt, aIdx, true ) ||
        // #i21457#
        !lcl_IsInSameTableBox( rNds, rStt, true ))
    {
        aIdx = rEnd;
        if( !rNds.GoNextSection( &aIdx, true, false ) ||
            !CheckNodesRange( rEnd, aIdx, true ) ||
            // #i21457#
            !lcl_IsInSameTableBox( rNds, rEnd, false ))
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
    const SwNode* pPrvNd = nullptr;
    sal_uInt16 nRegionRet = 0;
    if( rRange.HasMark() &&
        0 == ( nRegionRet = IsInsRegionAvailable( rRange, &pPrvNd ) ))
    {
        // demoted to info because this is called from SwXTextSection::attach,
        // so it could be invalid input
        SAL_INFO("sw.core" , "InsertSwSection: rRange overlaps other sections");
        return nullptr;
    }

    // See if the whole Document should be hidden, which we currently are not able to do.
    if (rNewData.IsHidden() && rRange.HasMark())
    {
        const SwPosition *pStt = rRange.Start(), *pEnd = rRange.End();
        if( !pStt->nContent.GetIndex() &&
            pEnd->nNode.GetNode().GetContentNode()->Len() ==
            pEnd->nContent.GetIndex() )
        {
            ::lcl_CheckEmptyLayFrame( GetNodes(),
                                    rNewData,
                                    pStt->nNode.GetNode(),
                                    pEnd->nNode.GetNode() );
        }
    }

    SwUndoInsSection* pUndoInsSect = nullptr;
    bool const bUndo(GetIDocumentUndoRedo().DoesUndo());
    if (bUndo)
    {
        pUndoInsSect = new SwUndoInsSection(rRange, rNewData, pAttr, pTOXBase);
        GetIDocumentUndoRedo().AppendUndo( pUndoInsSect );
        GetIDocumentUndoRedo().DoUndo(false);
    }

    SwSectionFormat* const pFormat = MakeSectionFormat( nullptr );
    if ( pAttr )
    {
        pFormat->SetFormatAttr( *pAttr );
    }

    SwSectionNode* pNewSectNode = nullptr;

    RedlineMode_t eOld = getIDocumentRedlineAccess().GetRedlineMode();
    getIDocumentRedlineAccess().SetRedlineMode_intern( (RedlineMode_t)((eOld & ~nsRedlineMode_t::REDLINE_SHOW_MASK) | nsRedlineMode_t::REDLINE_IGNORE));

    if( rRange.HasMark() )
    {
        SwPosition *pSttPos = const_cast<SwPosition*>(rRange.Start()),
                    *pEndPos = const_cast<SwPosition*>(rRange.End());
        if( pPrvNd && 3 == nRegionRet )
        {
            OSL_ENSURE( pPrvNd, "The SectionNode is missing" );
            SwNodeIndex aStt( pSttPos->nNode ), aEnd( pEndPos->nNode, +1 );
            while( pPrvNd != aStt.GetNode().StartOfSectionNode() )
                --aStt;
            while( pPrvNd != aEnd.GetNode().StartOfSectionNode() )
                ++aEnd;

            --aEnd; // End is inclusive in the InsertSection
            pNewSectNode = GetNodes().InsertTextSection(
                        aStt, *pFormat, rNewData, pTOXBase, & aEnd);
        }
        else
        {
            if( pUndoInsSect )
            {
                if( !( pPrvNd && 1 == nRegionRet ) &&
                    pSttPos->nContent.GetIndex() )
                {
                    SwTextNode* const pTNd =
                        pSttPos->nNode.GetNode().GetTextNode();
                    if (pTNd)
                    {
                        pUndoInsSect->SaveSplitNode( pTNd, true );
                    }
                }

                if ( !( pPrvNd && 2 == nRegionRet ) )
                {
                    SwTextNode *const pTNd =
                        pEndPos->nNode.GetNode().GetTextNode();
                    if (pTNd && (pTNd->GetText().getLength()
                                    != pEndPos->nContent.GetIndex()))
                    {
                        pUndoInsSect->SaveSplitNode( pTNd, false );
                    }
                }
            }

            if( pPrvNd && 1 == nRegionRet )
            {
                pSttPos->nNode.Assign( *pPrvNd );
                pSttPos->nContent.Assign( pSttPos->nNode.GetNode().GetContentNode(), 0 );
            }
            else if( pSttPos->nContent.GetIndex() )
            {
                getIDocumentContentOperations().SplitNode( *pSttPos, false );
            }

            if( pPrvNd && 2 == nRegionRet )
            {
                pEndPos->nNode.Assign( *pPrvNd );
                pEndPos->nContent.Assign( pEndPos->nNode.GetNode().GetContentNode(), 0 );
            }
            else
            {
                const SwContentNode* pCNd = pEndPos->nNode.GetNode().GetContentNode();
                if( pCNd && pCNd->Len() != pEndPos->nContent.GetIndex() )
                {
                    sal_Int32 nContent = pSttPos->nContent.GetIndex();
                    getIDocumentContentOperations().SplitNode( *pEndPos, false );

                    SwTextNode* pTNd;
                    if( pEndPos->nNode.GetIndex() == pSttPos->nNode.GetIndex() )
                    {
                        --pSttPos->nNode;
                        --pEndPos->nNode;
                        pTNd = pSttPos->nNode.GetNode().GetTextNode();
                        pSttPos->nContent.Assign( pTNd, nContent );
                    }
                    else
                    {
                        // Set to the end of the previous
                        --pEndPos->nNode;
                        pTNd = pEndPos->nNode.GetNode().GetTextNode();
                    }
                    nContent = (pTNd) ? pTNd->GetText().getLength() : 0;
                    pEndPos->nContent.Assign( pTNd, nContent );
                }
            }
            pNewSectNode = GetNodes().InsertTextSection(
                pSttPos->nNode, *pFormat, rNewData, pTOXBase, &pEndPos->nNode);
        }
    }
    else
    {
        const SwPosition* pPos = rRange.GetPoint();
        const SwContentNode* pCNd = pPos->nNode.GetNode().GetContentNode();
        if( !pPos->nContent.GetIndex() )
        {
            pNewSectNode = GetNodes().InsertTextSection(
                pPos->nNode, *pFormat, rNewData, pTOXBase, nullptr);
        }
        else if( pPos->nContent.GetIndex() == pCNd->Len() )
        {
            pNewSectNode = GetNodes().InsertTextSection(
                pPos->nNode, *pFormat, rNewData, pTOXBase, nullptr, false);
        }
        else
        {
            if( pUndoInsSect && pCNd->IsTextNode() )
            {
                pUndoInsSect->SaveSplitNode( const_cast<SwTextNode*>(static_cast<const SwTextNode*>(pCNd)), true );
            }
            getIDocumentContentOperations().SplitNode( *pPos, false );
            pNewSectNode = GetNodes().InsertTextSection(
                pPos->nNode, *pFormat, rNewData, pTOXBase, nullptr);
        }
    }

//FEATURE::CONDCOLL
    pNewSectNode->CheckSectionCondColl();
//FEATURE::CONDCOLL

    getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );

    // To-Do - add 'SwExtraRedlineTable' also ?
    if( getIDocumentRedlineAccess().IsRedlineOn() || (!getIDocumentRedlineAccess().IsIgnoreRedline() && !getIDocumentRedlineAccess().GetRedlineTable().empty() ))
    {
        SwPaM aPam( *pNewSectNode->EndOfSectionNode(), *pNewSectNode, 1 );
        if( getIDocumentRedlineAccess().IsRedlineOn() )
        {
            getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( nsRedlineType_t::REDLINE_INSERT, aPam ), true);
        }
        else
        {
            getIDocumentRedlineAccess().SplitRedline( aPam );
        }
    }

    // Is a Condition set?
    if (rNewData.IsHidden() && !rNewData.GetCondition().isEmpty())
    {
        // The calculate up to that position
        SwCalc aCalc( *this );
        if( ! IsInReading() )
        {
            getIDocumentFieldsAccess().FieldsToCalc( aCalc, pNewSectNode->GetIndex(), USHRT_MAX );
        }
        SwSection& rNewSect = pNewSectNode->GetSection();
        rNewSect.SetCondHidden( aCalc.Calculate( rNewSect.GetCondition() ).GetBool() );
    }

    bool bUpdateFootnote = false;
    if( GetFootnoteIdxs().size() && pAttr )
    {
        sal_uInt16 nVal = static_cast<const SwFormatFootnoteAtTextEnd&>(pAttr->Get(
                                            RES_FTN_AT_TXTEND )).GetValue();
           if( ( FTNEND_ATTXTEND_OWNNUMSEQ == nVal ||
              FTNEND_ATTXTEND_OWNNUMANDFMT == nVal ) ||
            ( FTNEND_ATTXTEND_OWNNUMSEQ == ( nVal = static_cast<const SwFormatEndAtTextEnd&>(
                            pAttr->Get( RES_END_AT_TXTEND )).GetValue() ) ||
              FTNEND_ATTXTEND_OWNNUMANDFMT == nVal ))
        {
            bUpdateFootnote = true;
        }
    }

    if( pUndoInsSect )
    {
        pUndoInsSect->SetSectNdPos( pNewSectNode->GetIndex() );
        pUndoInsSect->SetUpdateFootnoteFlag( bUpdateFootnote );
        GetIDocumentUndoRedo().DoUndo(bUndo);
    }

    if (rNewData.IsLinkType())
    {
        pNewSectNode->GetSection().CreateLink( bUpdate ? CREATE_UPDATE : CREATE_CONNECT );
    }

    if( bUpdateFootnote )
    {
        GetFootnoteIdxs().UpdateFootnote( SwNodeIndex( *pNewSectNode ));
    }

    getIDocumentState().SetModified();
    return &pNewSectNode->GetSection();
}

sal_uInt16 SwDoc::IsInsRegionAvailable( const SwPaM& rRange,
                                const SwNode** ppSttNd )
{
    sal_uInt16 nRet = 1;
    if( rRange.HasMark() )
    {
        // See if we have a valid Section
        const SwPosition* pStt = rRange.Start();
        const SwPosition* pEnd = rRange.End();

        const SwContentNode* pCNd = pEnd->nNode.GetNode().GetContentNode();
        const SwNode* pNd = &pStt->nNode.GetNode();
        const SwSectionNode* pSectNd = pNd->FindSectionNode();
        const SwSectionNode* pEndSectNd = pCNd ? pCNd->FindSectionNode() : nullptr;
        if( pSectNd && pEndSectNd && pSectNd != pEndSectNd )
        {
            // Try to create an enclosing Section, but only if Start is
            // located at the Section's beginning and End at it's end
            nRet = 0;
            if( !pStt->nContent.GetIndex()
                && pSectNd->GetIndex() == pStt->nNode.GetIndex() - 1
                && pEnd->nContent.GetIndex() == pCNd->Len() )
            {
                SwNodeIndex aIdx( pStt->nNode, -1 );
                sal_uLong nCmp = pEnd->nNode.GetIndex();
                const SwStartNode* pPrvNd;
                const SwEndNode* pNxtNd;
                while( nullptr != ( pPrvNd = (pNd = &aIdx.GetNode())->GetSectionNode() ) &&
                    !( aIdx.GetIndex() < nCmp &&
                        nCmp < pPrvNd->EndOfSectionIndex() ) )
                {
                    --aIdx;
                }
                if( !pPrvNd )
                    pPrvNd = pNd->IsStartNode() ? static_cast<const SwStartNode*>(pNd)
                                                : pNd->StartOfSectionNode();

                aIdx = pEnd->nNode.GetIndex() + 1;
                nCmp = pStt->nNode.GetIndex();
                while( nullptr != ( pNxtNd = (pNd = &aIdx.GetNode())->GetEndNode() ) &&
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
            // Try to create an enclosing Section, but only if the End
            // is at the Section's end.
            nRet = 0;
            if( pEnd->nContent.GetIndex() == pCNd->Len() )
            {
                SwNodeIndex aIdx( pEnd->nNode, 1 );
                if( aIdx.GetNode().IsEndNode() &&
                        nullptr != aIdx.GetNode().FindSectionNode() )
                {
                    do {
                        ++aIdx;
                    } while( aIdx.GetNode().IsEndNode() &&
                                nullptr != aIdx.GetNode().FindSectionNode() );
                    {
                        nRet = 2;
                        if( ppSttNd )
                        {
                            --aIdx;
                            *ppSttNd = &aIdx.GetNode();
                        }
                    }
                }
            }
        }
        else if( pSectNd && !pEndSectNd )
        {
            // Try to create an enclosing Section, but only if Start
            // is at the Section's start.
            nRet = 0;
            if( !pStt->nContent.GetIndex() )
            {
                SwNodeIndex aIdx( pStt->nNode, -1 );
                if( aIdx.GetNode().IsSectionNode() )
                {
                    do {
                        --aIdx;
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

SwSection* SwDoc::GetCurrSection( const SwPosition& rPos )
{
    const SwSectionNode* pSectNd = rPos.nNode.GetNode().FindSectionNode();
    if( pSectNd )
        return const_cast<SwSection*>(&pSectNd->GetSection());
    return nullptr;
}

SwSectionFormat* SwDoc::MakeSectionFormat( SwSectionFormat *pDerivedFrom )
{
    SwSectionFormat* pNew = new SwSectionFormat(
        pDerivedFrom == nullptr ? mpDfltFrameFormat : pDerivedFrom, this );
    mpSectionFormatTable->push_back( pNew );
    return pNew;
}

void SwDoc::DelSectionFormat( SwSectionFormat *pFormat, bool bDelNodes )
{
    SwSectionFormats::iterator itFormatPos = std::find( mpSectionFormatTable->begin(), mpSectionFormatTable->end(), pFormat );

    GetIDocumentUndoRedo().StartUndo(UNDO_DELSECTION, nullptr);

    if( mpSectionFormatTable->end() != itFormatPos )
    {
        const SwNodeIndex* pIdx = pFormat->GetContent( false ).GetContentIdx();
        const SfxPoolItem* pFootnoteEndAtTextEnd;
        if( SfxItemState::SET != pFormat->GetItemState(
                            RES_FTN_AT_TXTEND, true, &pFootnoteEndAtTextEnd ) ||
            SfxItemState::SET != pFormat->GetItemState(
                            RES_END_AT_TXTEND, true, &pFootnoteEndAtTextEnd ))
            pFootnoteEndAtTextEnd = nullptr;

        const SwSectionNode* pSectNd;

        if( GetIDocumentUndoRedo().DoesUndo() )
        {
            if( bDelNodes && pIdx && &GetNodes() == &pIdx->GetNodes() &&
                nullptr != (pSectNd = pIdx->GetNode().GetSectionNode() ))
            {
                SwNodeIndex aUpdIdx( *pIdx );
                SwPaM aPaM( *pSectNd->EndOfSectionNode(), *pSectNd );
                GetIDocumentUndoRedo().AppendUndo( new SwUndoDelete( aPaM ));
                if( pFootnoteEndAtTextEnd )
                    GetFootnoteIdxs().UpdateFootnote( aUpdIdx );
                getIDocumentState().SetModified();
                //#126178# start/end undo have to be pairs!
                GetIDocumentUndoRedo().EndUndo(UNDO_DELSECTION, nullptr);
                return ;
            }
            GetIDocumentUndoRedo().AppendUndo( MakeUndoDelSection( *pFormat ) );
        }
        else if( bDelNodes && pIdx && &GetNodes() == &pIdx->GetNodes() &&
                nullptr != (pSectNd = pIdx->GetNode().GetSectionNode() ))
        {
            SwNodeIndex aUpdIdx( *pIdx );
            getIDocumentContentOperations().DeleteSection( const_cast<SwNode*>(static_cast<SwNode const *>(pSectNd)) );
            if( pFootnoteEndAtTextEnd )
                GetFootnoteIdxs().UpdateFootnote( aUpdIdx );
            getIDocumentState().SetModified();
            //#126178# start/end undo have to be pairs!
            GetIDocumentUndoRedo().EndUndo(UNDO_DELSECTION, nullptr);
            return ;
        }

        {
            SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT, pFormat );
            pFormat->ModifyNotification( &aMsgHint, &aMsgHint );
        }

        // A ClearRedo could result in a recursive call of this function and delete some section
        // formats, thus the position inside the SectionFormatTable could have changed
        itFormatPos = std::find( mpSectionFormatTable->begin(), mpSectionFormatTable->end(), pFormat );

        // WARNING: First remove from the array and then delete,
        //          as the Section DTOR tries to delete it's format itself.
        mpSectionFormatTable->erase( itFormatPos );
//FEATURE::CONDCOLL
        sal_uLong nCnt = 0, nSttNd = 0;
        if( pIdx && &GetNodes() == &pIdx->GetNodes() &&
            nullptr != (pSectNd = pIdx->GetNode().GetSectionNode() ))
        {
            nSttNd = pSectNd->GetIndex();
            nCnt = pSectNd->EndOfSectionIndex() - nSttNd - 1;
        }
//FEATURE::CONDCOLL

        delete pFormat;

        if( nSttNd && pFootnoteEndAtTextEnd )
        {
            SwNodeIndex aUpdIdx( GetNodes(), nSttNd );
            GetFootnoteIdxs().UpdateFootnote( aUpdIdx );
        }

//FEATURE::CONDCOLL
        SwContentNode* pCNd;
        for( ; nCnt--; ++nSttNd )
            if( nullptr != (pCNd = GetNodes()[ nSttNd ]->GetContentNode() ) &&
                RES_CONDTXTFMTCOLL == pCNd->GetFormatColl()->Which() )
                pCNd->ChkCondColl();
//FEATURE::CONDCOLL
    }

    GetIDocumentUndoRedo().EndUndo(UNDO_DELSECTION, nullptr);

    getIDocumentState().SetModified();
}

void SwDoc::UpdateSection( size_t const nPos, SwSectionData & rNewData,
        SfxItemSet const*const pAttr, bool const bPreventLinkUpdate )
{
    SwSectionFormat* pFormat = (*mpSectionFormatTable)[ nPos ];
    SwSection* pSection = pFormat->GetSection();

    /// remember hidden condition flag of SwSection before changes
    bool bOldCondHidden = pSection->IsCondHidden();

    if (pSection->DataEquals(rNewData))
    {
        // Check Attributes
        bool bOnlyAttrChg = false;
        if( pAttr && pAttr->Count() )
        {
            SfxItemIter aIter( *pAttr );
            sal_uInt16 nWhich = aIter.GetCurItem()->Which();
            while( true )
            {
                if( pFormat->GetFormatAttr( nWhich ) != *aIter.GetCurItem() )
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
                    MakeUndoUpdateSection( *pFormat, true ) );
            }
            // #i32968# Inserting columns in the section causes MakeFrameFormat
            // to put two  objects of type SwUndoFrameFormat on the undo stack.
            // We don't want them.
            ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());
            pFormat->SetFormatAttr( *pAttr );
            getIDocumentState().SetModified();
        }
        return;
    }

    // Test if the whole Content Section (Document/TableBox/Fly) should be hidden,
    // which we're currently not able to do.
    const SwNodeIndex* pIdx = nullptr;
    {
        if (rNewData.IsHidden())
        {
            pIdx = pFormat->GetContent().GetContentIdx();
            if (pIdx)
            {
                const SwSectionNode* pSectNd =
                    pIdx->GetNode().GetSectionNode();
                if (pSectNd)
                {
                    ::lcl_CheckEmptyLayFrame( GetNodes(), rNewData,
                                *pSectNd, *pSectNd->EndOfSectionNode() );
                }
            }
        }
    }

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo(MakeUndoUpdateSection(*pFormat, false));
    }
    // #i32968# Inserting columns in the section causes MakeFrameFormat to put two
    // objects of type SwUndoFrameFormat on the undo stack. We don't want them.
    ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

    // The LinkFileName could only consist of separators
    OUString sCompareString = OUString(sfx2::cTokenSeparator) + OUString(sfx2::cTokenSeparator);
    const bool bUpdate =
           (!pSection->IsLinkType() && rNewData.IsLinkType())
            ||  (!rNewData.GetLinkFileName().isEmpty()
                &&  (rNewData.GetLinkFileName() != sCompareString)
                &&  (rNewData.GetLinkFileName() != pSection->GetLinkFileName()));

    OUString sSectName( rNewData.GetSectionName() );
    if (sSectName != pSection->GetSectionName())
        sSectName = GetUniqueSectionName( &sSectName );
    else
        sSectName.clear();

    /// In SwSection::operator=(..) class member m_bCondHiddenFlag is always set to true.
    /// IMHO this have to be changed, but I can't estimate the consequences:
    /// Either it is set to true using corresponding method <SwSection.SetCondHidden(..)>,
    /// or it is set to the value of SwSection which is assigned to it.
    /// Discussion with AMA results that the adjustment to the assignment operator
    /// could be very risky.
    pSection->SetSectionData(rNewData);

    if( pAttr )
        pSection->GetFormat()->SetFormatAttr( *pAttr );

    if( !sSectName.isEmpty() )
    {
        pSection->SetSectionName( sSectName );
    }

    // Is a Condition set
    if( pSection->IsHidden() && !pSection->GetCondition().isEmpty() )
    {
        // Then calculate up to that position
        SwCalc aCalc( *this );
        if( !pIdx )
            pIdx = pFormat->GetContent().GetContentIdx();
        getIDocumentFieldsAccess().FieldsToCalc( aCalc, pIdx->GetIndex(), USHRT_MAX );

        /// Because on using SwSection::operator=() to set up <pSection>
        /// with <rNewData> and the above given note, the hidden condition flag
        /// has to be set to false, if hidden condition flag of <pFormat->GetSection()>
        /// (SwSection before the changes) is false (already saved in <bOldCondHidden>)
        /// and new calculated condition is true.
        /// This is necessary, because otherwise the <SetCondHidden> would have
        /// no effect.
        bool bCalculatedCondHidden =
                aCalc.Calculate( pSection->GetCondition() ).GetBool();
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
        getIDocumentLinksAdministration().GetLinkManager().Remove( &pSection->GetBaseLink() );
    }

    getIDocumentState().SetModified();
}

void sw_DeleteFootnote( SwSectionNode *pNd, sal_uLong nStt, sal_uLong nEnd )
{
    SwFootnoteIdxs& rFootnoteArr = pNd->GetDoc()->GetFootnoteIdxs();
    if( rFootnoteArr.size() )
    {
        size_t nPos = 0;
        rFootnoteArr.SeekEntry( SwNodeIndex( *pNd ), &nPos );
        SwTextFootnote* pSrch;

        // Delete all succeeding Footnotes
        while( nPos < rFootnoteArr.size() &&
            _SwTextFootnote_GetIndex( (pSrch = rFootnoteArr[ nPos ]) ) <= nEnd )
        {
            // If the Nodes are not deleted, they need to deregister at the Pages
            // (delete Frames) or else they will remain there (Undo does not delete them!)
            pSrch->DelFrames(nullptr);
            ++nPos;
        }

        while( nPos-- &&
            _SwTextFootnote_GetIndex( (pSrch = rFootnoteArr[ nPos ]) ) >= nStt )
        {
            // If the Nodes are not deleted, they need to deregister at the Pages
            // (delete Frames) or else they will remain there (Undo does not delete them!)
            pSrch->DelFrames(nullptr);
        }
    }
}

static inline bool lcl_IsTOXSection(SwSectionData const& rSectionData)
{
    return (TOX_CONTENT_SECTION == rSectionData.GetType())
        || (TOX_HEADER_SECTION  == rSectionData.GetType());
}

SwSectionNode* SwNodes::InsertTextSection(SwNodeIndex const& rNdIdx,
                                SwSectionFormat& rSectionFormat,
                                SwSectionData const& rSectionData,
                                SwTOXBase const*const pTOXBase,
                                SwNodeIndex const*const pEnde,
                                bool const bInsAtStart, bool const bCreateFrames)
{
    SwNodeIndex aInsPos( rNdIdx );
    if( !pEnde ) // No Area, thus create a new Section before/after it
    {
        // #i26762#
        OSL_ENSURE(!pEnde || rNdIdx <= *pEnde,
               "Section start and end in wrong order!");

        if( bInsAtStart )
        {
            if (!lcl_IsTOXSection(rSectionData))
            {
                do {
                    --aInsPos;
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
            new SwSectionNode(aInsPos, rSectionFormat, pTOXBase);
    if( pEnde )
    {
        // Special case for the Reader/Writer
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
        // This StartNode should be in front of me, but if not, I want to survive
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
        SwTextNode* pCpyTNd = rNdIdx.GetNode().GetTextNode();
        if( pCpyTNd )
        {
            SwTextNode* pTNd = new SwTextNode( aInsPos, pCpyTNd->GetTextColl() );
            if( pCpyTNd->HasSwAttrSet() )
            {
                // Move PageDesc/Break to the first Node of the section
                const SfxItemSet& rSet = *pCpyTNd->GetpSwAttrSet();
                if( SfxItemState::SET == rSet.GetItemState( RES_BREAK ) ||
                    SfxItemState::SET == rSet.GetItemState( RES_PAGEDESC ))
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
            // Do not forget to create the Frame!
            pCpyTNd->MakeFrames( *pTNd );
        }
        else
            new SwTextNode( aInsPos, GetDoc()->GetDfltTextFormatColl() );
    }
    new SwEndNode( aInsPos, *pSectNd );

    pSectNd->GetSection().SetSectionData(rSectionData);
    SwSectionFormat* pSectFormat = pSectNd->GetSection().GetFormat();

    // We could optimize this, by not removing already contained Frames and recreating them,
    // but by simply rewiring them
    bool bInsFrame = bCreateFrames && !pSectNd->GetSection().IsHidden() &&
                   GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    SwNode2Layout *pNode2Layout = nullptr;
    if( bInsFrame )
    {
        SwNodeIndex aTmp( *pSectNd );
        if( !pSectNd->GetNodes().FindPrvNxtFrameNode( aTmp, pSectNd->EndOfSectionNode() ) )
            // Collect all Uppers
            pNode2Layout = new SwNode2Layout( *pSectNd );
    }

    // Set the right StartNode for all in this Area
    sal_uLong nEnde = pSectNd->EndOfSectionIndex();
    sal_uLong nStart = pSectNd->GetIndex()+1;
    sal_uLong nSkipIdx = ULONG_MAX;
    for( sal_uLong n = nStart; n < nEnde; ++n )
    {
        SwNode* pNd = (*this)[n];

        // Attach all Sections in the NodeSection underneath the new one
        if( ULONG_MAX == nSkipIdx )
            pNd->m_pStartOfSection = pSectNd;
        else if( n >= nSkipIdx )
            nSkipIdx = ULONG_MAX;

        if( pNd->IsStartNode() )
        {
            // Make up the Format's nesting
            if( pNd->IsSectionNode() )
            {
                static_cast<SwSectionNode*>(pNd)->GetSection().GetFormat()->
                                    SetDerivedFrom( pSectFormat );
                static_cast<SwSectionNode*>(pNd)->DelFrames();
                n = pNd->EndOfSectionIndex();
            }
            else
            {
                if( pNd->IsTableNode() )
                    static_cast<SwTableNode*>(pNd)->DelFrames();

                if( ULONG_MAX == nSkipIdx )
                    nSkipIdx = pNd->EndOfSectionIndex();
            }
        }
        else if( pNd->IsContentNode() )
            static_cast<SwContentNode*>(pNd)->DelFrames();
    }

    sw_DeleteFootnote( pSectNd, nStart, nEnde );

    if( bInsFrame )
    {
        if( pNode2Layout )
        {
            sal_uLong nIdx = pSectNd->GetIndex();
            pNode2Layout->RestoreUpperFrames( pSectNd->GetNodes(), nIdx, nIdx + 1 );
            delete pNode2Layout;
        }
        else
            pSectNd->MakeFrames( &aInsPos );
    }

    return pSectNd;
}

SwSectionNode* SwNode::FindSectionNode()
{
    if( IsSectionNode() )
        return GetSectionNode();
    SwStartNode* pTmp = m_pStartOfSection;
    while( !pTmp->IsSectionNode() && pTmp->GetIndex() )
        pTmp = pTmp->m_pStartOfSection;
    return pTmp->GetSectionNode();
}

// SwSectionNode

// ugly hack to make m_pSection const
static SwSectionFormat &
lcl_initParent(SwSectionNode & rThis, SwSectionFormat & rFormat)
{
    SwSectionNode *const pParent =
        rThis.StartOfSectionNode()->FindSectionNode();
    if( pParent )
    {
        // Register the Format at the right Parent
        rFormat.SetDerivedFrom( pParent->GetSection().GetFormat() );
    }
    return rFormat;
}

SwSectionNode::SwSectionNode(SwNodeIndex const& rIdx,
        SwSectionFormat & rFormat, SwTOXBase const*const pTOXBase)
    : SwStartNode( rIdx, ND_SECTIONNODE )
    , m_pSection( (pTOXBase)
        ? new SwTOXBaseSection(*pTOXBase, lcl_initParent(*this, rFormat))
        : new SwSection( CONTENT_SECTION, rFormat.GetName(),
                lcl_initParent(*this, rFormat) ) )
{
    // Set the connection from Format to Node
    // Suppress Modify; no one's interessted anyway
    rFormat.LockModify();
    rFormat.SetFormatAttr( SwFormatContent( this ) );
    rFormat.UnlockModify();
}

SwSectionNode::~SwSectionNode()
{
    // mba: test if iteration works as clients will be removed in callback
    // use hint which allows to specify, if the content shall be saved or not
    m_pSection->GetFormat()->CallSwClientNotify( SwSectionFrameMoveAndDeleteHint( true ) );
    SwSectionFormat* pFormat = m_pSection->GetFormat();
    if( pFormat )
    {
        // Remove the Attribute, because the Section deletes it's Format
        // and it will neutralize the Section, if the Content Attribute is set
        pFormat->LockModify();
        pFormat->ResetFormatAttr( RES_CNTNT );
        pFormat->UnlockModify();
    }
}

SwFrame *SwSectionNode::MakeFrame( SwFrame *pSib )
{
    m_pSection->m_Data.SetHiddenFlag(false);
    return new SwSectionFrame( *m_pSection, pSib );
}

// Creates all Document Views for the precedeing Node.
// The created ContentFrames are attached to the corresponding Layout
void SwSectionNode::MakeFrames(const SwNodeIndex & rIdx )
{
    // Take my succsessive or preceding ContentFrame
    SwNodes& rNds = GetNodes();
    if( rNds.IsDocNodes() && rNds.GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell() )
    {
        if( GetSection().IsHidden() || IsContentHidden() )
        {
            SwNodeIndex aIdx( *EndOfSectionNode() );
            SwContentNode* pCNd = rNds.GoNextSection( &aIdx, true, false );
            if( !pCNd )
            {
                aIdx = *this;
                pCNd = SwNodes::GoPrevSection(&aIdx, true, false);
                if (!pCNd)
                    return;
            }
            pCNd = aIdx.GetNode().GetContentNode();
            pCNd->MakeFrames( static_cast<SwContentNode&>(rIdx.GetNode()) );
        }
        else
        {
            SwNode2Layout aNode2Layout( *this, rIdx.GetIndex() );
            SwFrame *pFrame;
            while( nullptr != (pFrame = aNode2Layout.NextFrame()) )
            {
                OSL_ENSURE( pFrame->IsSctFrame(), "Depend of Section not a Section." );
                SwFrame *pNew = rIdx.GetNode().GetContentNode()->MakeFrame( pFrame );

                SwSectionNode* pS = rIdx.GetNode().FindSectionNode();

                // Assure that node is not inside a table, which is inside the
                // found section.
                if ( pS )
                {
                    SwTableNode* pTableNode = rIdx.GetNode().FindTableNode();
                    if ( pTableNode &&
                         pTableNode->GetIndex() > pS->GetIndex() )
                    {
                        pS = nullptr;
                    }
                }

                // if the node is in a section, the sectionframe now
                // has to be created..
                // boolean to control <Init()> of a new section frame.
                bool bInitNewSect = false;
                if( pS )
                {
                    SwSectionFrame *pSct = new SwSectionFrame( pS->GetSection(), pFrame );
                    // prepare <Init()> of new section frame.
                    bInitNewSect = true;
                    SwLayoutFrame* pUp = pSct;
                    while( pUp->Lower() )  // for columned sections
                    {
                        OSL_ENSURE( pUp->Lower()->IsLayoutFrame(),"Who's in there?" );
                        pUp = static_cast<SwLayoutFrame*>(pUp->Lower());
                    }
                    pNew->Paste( pUp );
                    // #i27138#
                    // notify accessibility paragraphs objects about changed
                    // CONTENT_FLOWS_FROM/_TO relation.
                    // Relation CONTENT_FLOWS_FROM for next paragraph will change
                    // and relation CONTENT_FLOWS_TO for previous paragraph will change.
                    if ( pNew->IsTextFrame() )
                    {
                        SwViewShell* pViewShell( pNew->getRootFrame()->GetCurrShell() );
                        if ( pViewShell && pViewShell->GetLayout() &&
                             pViewShell->GetLayout()->IsAnyShellAccessible() )
                        {
                            pViewShell->InvalidateAccessibleParaFlowRelation(
                                dynamic_cast<SwTextFrame*>(pNew->FindNextCnt( true )),
                                dynamic_cast<SwTextFrame*>(pNew->FindPrevCnt( true )) );
                        }
                    }
                    pNew = pSct;
                }

                // If a Node got Frames attached before or after
                if ( rIdx < GetIndex() )
                    // the new one precedes me
                    pNew->Paste( pFrame->GetUpper(), pFrame );
                else
                    // the new one succeeds me
                    pNew->Paste( pFrame->GetUpper(), pFrame->GetNext() );
                // #i27138#
                // notify accessibility paragraphs objects about changed
                // CONTENT_FLOWS_FROM/_TO relation.
                // Relation CONTENT_FLOWS_FROM for next paragraph will change
                // and relation CONTENT_FLOWS_TO for previous paragraph will change.
                if ( pNew->IsTextFrame() )
                {
                    SwViewShell* pViewShell( pNew->getRootFrame()->GetCurrShell() );
                    if ( pViewShell && pViewShell->GetLayout() &&
                         pViewShell->GetLayout()->IsAnyShellAccessible() )
                    {
                        pViewShell->InvalidateAccessibleParaFlowRelation(
                            dynamic_cast<SwTextFrame*>(pNew->FindNextCnt( true )),
                            dynamic_cast<SwTextFrame*>(pNew->FindPrevCnt( true )) );
                    }
                }
                if ( bInitNewSect )
                    static_cast<SwSectionFrame*>(pNew)->Init();
            }
        }
    }
}

// Create a new SectionFrame for every occurrence in the Layout and insert before
// the corresponding ContentFrame
void SwSectionNode::MakeFrames( SwNodeIndex* pIdxBehind, SwNodeIndex* pEndIdx )
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
        ::MakeFrames( pDoc, *pIdxBehind, *pEnd );
        if( !pEndIdx )
            delete pEnd;
    }
}

void SwSectionNode::DelFrames()
{
    sal_uLong nStt = GetIndex()+1, nEnd = EndOfSectionIndex();
    if( nStt >= nEnd )
    {
        return ;
    }

    SwNodes& rNds = GetNodes();
    m_pSection->GetFormat()->DelFrames();

    // Update our Flag
    m_pSection->m_Data.SetHiddenFlag(true);

    // If the Area is within a Fly or TableBox, we can only hide it if
    // there is more Content which has Frames.
    // Or else the Fly/TableBox Frame does not have a Lower!
    {
        SwNodeIndex aIdx( *this );
        if( !SwNodes::GoPrevSection( &aIdx, true, false ) ||
            !CheckNodesRange( *this, aIdx, true ) ||
            // #i21457#
            !lcl_IsInSameTableBox( rNds, *this, true ))
        {
            aIdx = *EndOfSectionNode();
            if( !rNds.GoNextSection( &aIdx, true, false ) ||
                !CheckNodesRange( *EndOfSectionNode(), aIdx, true ) ||
                // #i21457#
                !lcl_IsInSameTableBox( rNds, *EndOfSectionNode(), false ))
            {
                m_pSection->m_Data.SetHiddenFlag(false);
            }
        }
    }
}

SwSectionNode* SwSectionNode::MakeCopy( SwDoc* pDoc, const SwNodeIndex& rIdx ) const
{
    // In which array am I: Nodes, UndoNodes?
    const SwNodes& rNds = GetNodes();

    // Copy the SectionFrameFormat
    SwSectionFormat* pSectFormat = pDoc->MakeSectionFormat( nullptr );
    pSectFormat->CopyAttrs( *GetSection().GetFormat() );

    std::unique_ptr<SwTOXBase> pTOXBase;
    if (TOX_CONTENT_SECTION == GetSection().GetType())
    {
        OSL_ENSURE( dynamic_cast< const SwTOXBaseSection* >( &GetSection() ) !=  nullptr , "no TOXBaseSection!" );
        SwTOXBaseSection const& rTBS(
            dynamic_cast<SwTOXBaseSection const&>(GetSection()));
        pTOXBase.reset( new SwTOXBase(rTBS, pDoc) );
    }

    SwSectionNode *const pSectNd =
        new SwSectionNode(rIdx, *pSectFormat, pTOXBase.get());
    SwEndNode* pEndNd = new SwEndNode( rIdx, *pSectNd );
    SwNodeIndex aInsPos( *pEndNd );

    // Take over values
    SwSection *const pNewSect = pSectNd->m_pSection.get();

    if (TOX_CONTENT_SECTION != GetSection().GetType())
    {
        // Keep the Name for Move
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
        pNewSect->SetHidden();
    if( !pNewSect->IsProtectFlag() && GetSection().IsProtect() )
        pNewSect->SetProtect();
    // edit in readonly sections
    if( !pNewSect->IsEditInReadonlyFlag() && GetSection().IsEditInReadonly() )
        pNewSect->SetEditInReadonly();

    SwNodeRange aRg( *this, +1, *EndOfSectionNode() ); // Where am I?
    rNds._Copy( aRg, aInsPos, false );

    // Delete all Frames from the copied Area. They are created when creating
    // the SectionFrames.
    pSectNd->DelFrames();

    // Copy the Links/Server
    if( pNewSect->IsLinkType() ) // Add the Link
        pNewSect->CreateLink( pDoc->getIDocumentLayoutAccess().GetCurrentViewShell() ? CREATE_CONNECT : CREATE_NONE );

    // If we copy from the Undo as Server, enter it again
    if (m_pSection->IsServer()
        && pDoc->GetIDocumentUndoRedo().IsUndoNodes(rNds))
    {
        pNewSect->SetRefObject( m_pSection->GetObject() );
        pDoc->getIDocumentLinksAdministration().GetLinkManager().InsertServer( pNewSect->GetObject() );
    }

    // METADATA: copy xml:id; must be done after insertion of node
    pSectFormat->RegisterAsCopyOf(*GetSection().GetFormat());

    return pSectNd;
}

bool SwSectionNode::IsContentHidden() const
{
    OSL_ENSURE( !m_pSection->IsHidden(),
            "That's simple: Hidden Section => Hidden Content" );
    SwNodeIndex aTmp( *this, 1 );
    sal_uLong nEnd = EndOfSectionIndex();
    while( aTmp < nEnd )
    {
        if( aTmp.GetNode().IsSectionNode() )
        {
            const SwSection& rSect = static_cast<SwSectionNode&>(aTmp.GetNode()).GetSection();
            if( rSect.IsHiddenFlag() )
                // Skip this Section
                aTmp = *aTmp.GetNode().EndOfSectionNode();
        }
        else
        {
            if( aTmp.GetNode().IsContentNode() || aTmp.GetNode().IsTableNode() )
                return false; // We found non-hidden content
            OSL_ENSURE( aTmp.GetNode().IsEndNode(), "EndNode expected" );
        }
        ++aTmp;
    }
    return true; // Hide everything
}

void SwSectionNode::NodesArrChgd()
{
    SwSectionFormat *const pFormat = m_pSection->GetFormat();
    if( pFormat )
    {
        SwNodes& rNds = GetNodes();
        SwDoc* pDoc = pFormat->GetDoc();

        if( !rNds.IsDocNodes() )
        {
            SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT, pFormat );
            pFormat->ModifyNotification( &aMsgHint, &aMsgHint );
        }

        pFormat->LockModify();
        pFormat->SetFormatAttr( SwFormatContent( this ));
        pFormat->UnlockModify();

        SwSectionNode* pSectNd = StartOfSectionNode()->FindSectionNode();
        // set the correct parent from the new section
        pFormat->SetDerivedFrom( pSectNd ? pSectNd->GetSection().GetFormat()
                                      : pDoc->GetDfltFrameFormat() );

        // Set the right StartNode for all in this Area
        sal_uLong nStart = GetIndex()+1, nEnde = EndOfSectionIndex();
        for( sal_uLong n = nStart; n < nEnde; ++n )
            // Make up the Format's nesting
            if( nullptr != ( pSectNd = rNds[ n ]->GetSectionNode() ) )
            {
                pSectNd->GetSection().GetFormat()->SetDerivedFrom( pFormat );
                n = pSectNd->EndOfSectionIndex();
            }

        // Moving Nodes to the UndoNodes array?
        if( rNds.IsDocNodes() )
        {
            OSL_ENSURE( pDoc == GetDoc(),
                    "Moving to different Documents?" );
            if( m_pSection->IsLinkType() ) // Remove the Link
                m_pSection->CreateLink( pDoc->getIDocumentLayoutAccess().GetCurrentViewShell() ? CREATE_CONNECT : CREATE_NONE );

            if (m_pSection->IsServer())
                pDoc->getIDocumentLinksAdministration().GetLinkManager().InsertServer( m_pSection->GetObject() );
        }
        else
        {
            if (CONTENT_SECTION != m_pSection->GetType()
                && m_pSection->IsConnected())
            {
                pDoc->getIDocumentLinksAdministration().GetLinkManager().Remove( &m_pSection->GetBaseLink() );
            }
            if (m_pSection->IsServer())
                pDoc->getIDocumentLinksAdministration().GetLinkManager().RemoveServer( m_pSection->GetObject() );
        }
    }
}

OUString SwDoc::GetUniqueSectionName( const OUString* pChkStr ) const
{
    if( IsInMailMerge())
    {
        OUString newName = "MailMergeSection"
            + OStringToOUString( DateTimeToOString( DateTime( DateTime::SYSTEM )), RTL_TEXTENCODING_ASCII_US )
            + OUString::number( mpSectionFormatTable->size() + 1 );
        if( pChkStr )
            newName += *pChkStr;
        return newName;
    }

    const OUString aName( ResId( STR_REGION_DEFNAME, *pSwResMgr ) );

    SwSectionFormats::size_type nNum = 0;
    const SwSectionFormats::size_type nFlagSize = ( mpSectionFormatTable->size() / 8 ) + 2;
    sal_uInt8* pSetFlags = new sal_uInt8[ nFlagSize ];
    memset( pSetFlags, 0, nFlagSize );

    for( auto pFormat : *mpSectionFormatTable )
    {
        const SwSectionNode *const pSectNd = pFormat->GetSectionNode();
        if( pSectNd != nullptr )
        {
            const OUString rNm = pSectNd->GetSection().GetSectionName();
            if (rNm.startsWith( aName ))
            {
                // Calculate the Number and reset the Flag
                nNum = rNm.copy( aName.getLength() ).toInt32();
                if( nNum-- && nNum < mpSectionFormatTable->size() )
                    pSetFlags[ nNum / 8 ] |= (0x01 << ( nNum & 0x07 ));
            }
            if( pChkStr && *pChkStr==rNm )
                pChkStr = nullptr;
        }
    }

    if( !pChkStr )
    {
        // Flagged all Numbers accordingly, so get the right Number
        nNum = mpSectionFormatTable->size();
        for( SwSectionFormats::size_type n = 0; n < nFlagSize; ++n )
        {
            auto nTmp = pSetFlags[ n ];
            if( nTmp != 0xFF )
            {
                // Calculate the Number
                nNum = n * 8;
                while( nTmp & 1 )
                {
                    ++nNum;
                    nTmp >>= 1;
                }
                break;
            }
        }
    }
    delete [] pSetFlags;
    if( pChkStr )
        return *pChkStr;
    return aName + OUString::number( ++nNum );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
