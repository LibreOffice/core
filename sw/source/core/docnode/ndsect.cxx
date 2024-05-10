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

#include <config_wasm_strip.h>
#include <libxml/xmlwriter.h>
#include <hintids.hxx>
#include <osl/diagnose.h>
#include <sfx2/linkmgr.hxx>
#include <svl/itemiter.hxx>
#include <sal/log.hxx>
#include <fmtcntnt.hxx>
#include <txtftn.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentLinksAdministration.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
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
#include <cntfrm.hxx>
#include <node2lay.hxx>
#include <doctxm.hxx>
#include <fmtftntx.hxx>
#include <strings.hrc>
#include <viewsh.hxx>
#include <memory>
#include "ndsect.hxx"
#include <tools/datetimeutils.hxx>
#include <o3tl/string_view.hxx>

// #i21457# - new implementation of local method <lcl_IsInSameTableBox(..)>.
// Method now determines the previous/next on its own. Thus, it can be controlled,
// for which previous/next is checked, if it's visible.
static bool lcl_IsInSameTableBox(const SwNode& _rNd, const bool _bPrev)
{
    const SwTableNode* pTableNd = _rNd.FindTableNode();
    if ( !pTableNd )
    {
        return true;
    }

    // determine index to be checked. Its assumed that a previous/next exist.
    SwNodeIndex aChkIdx( _rNd );

    // determine index of previous/next - skip hidden ones, which are
    // inside the table.
    // If found one is before/after table, this one isn't in the same
    // table box as <_rNd>.
    for(;;)
    {
        if ( _bPrev
                ? !SwNodes::GoPrevSection( &aChkIdx, false, false )
                : !SwNodes::GoNextSection( &aChkIdx, false, false ) )
        {
            OSL_FAIL( "<lcl_IsInSameTableBox(..)> - no previous/next!" );
            return false;
        }

        if ( aChkIdx < pTableNd->GetIndex() ||
                aChkIdx > pTableNd->EndOfSectionNode()->GetIndex() )
        {
            return false;
        }

        // check, if found one isn't inside a hidden section, which
        // is also inside the table.
        SwSectionNode* pSectNd = aChkIdx.GetNode().FindSectionNode();
        if ( !pSectNd ||
                pSectNd->GetIndex() < pTableNd->GetIndex() ||
                !pSectNd->GetSection().IsHiddenFlag() )
        {
            break;
        }
    }

    // Find the Box's StartNode
    const SwTableSortBoxes& rSortBoxes = pTableNd->GetTable().GetTabSortBoxes();
    SwNodeOffset nIdx = _rNd.GetIndex();
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

static void lcl_CheckEmptyLayFrame( SwSectionData& rSectionData,
                        const SwNode& rStt, const SwNode& rEnd )
{
    SwNodeIndex aIdx( rStt );
    if( !SwNodes::GoPrevSection( &aIdx, true, false ) ||
        !CheckNodesRange( rStt, aIdx.GetNode(), true ) ||
        // #i21457#
        !lcl_IsInSameTableBox( rStt, true ))
    {
        aIdx = rEnd;
        if( !SwNodes::GoNextSection( &aIdx, true, false ) ||
            !CheckNodesRange( rEnd, aIdx.GetNode(), true ) ||
            // #i21457#
            !lcl_IsInSameTableBox( rEnd, false ))
        {
            rSectionData.SetHidden( false );
        }
    }
}

SwSection *
SwDoc::InsertSwSection(SwPaM const& rRange, SwSectionData & rNewData,
       std::tuple<SwTOXBase const*, sw::RedlineMode, sw::FieldmarkMode, sw::ParagraphBreakMode> const*const pTOXBaseAndMode,
                       SfxItemSet const*const pAttr, bool const bUpdate)
{
    const SwNode* pPrvNd = nullptr;
    sal_uInt16 nRegionRet = 0;
    if( rRange.HasMark() )
    {
        nRegionRet = IsInsRegionAvailable( rRange, &pPrvNd );
        if( 0 == nRegionRet )
        {
            // demoted to info because this is called from SwXTextSection::attach,
            // so it could be invalid input
            SAL_INFO("sw.core" , "InsertSwSection: rRange overlaps other sections");
            return nullptr;
        }
    }

    // See if the whole Document should be hidden, which we currently are not able to do.
    if (rNewData.IsHidden() && rRange.HasMark())
    {
        auto [pStt, pEnd] = rRange.StartEnd(); // SwPosition*
        if( !pStt->GetContentIndex() &&
            pEnd->GetNode().GetContentNode()->Len() ==
            pEnd->GetContentIndex() )
        {
            ::lcl_CheckEmptyLayFrame(
                                    rNewData,
                                    pStt->GetNode(),
                                    pEnd->GetNode() );
        }
    }

    SwUndoInsSection* pUndoInsSect = nullptr;
    bool const bUndo(GetIDocumentUndoRedo().DoesUndo());
    if (bUndo)
    {
        pUndoInsSect = new SwUndoInsSection(rRange, rNewData, pAttr, pTOXBaseAndMode);
        GetIDocumentUndoRedo().AppendUndo( std::unique_ptr<SwUndo>(pUndoInsSect) );
        GetIDocumentUndoRedo().DoUndo(false);
    }

    SwSectionFormat* const pFormat = MakeSectionFormat();
    pFormat->SetFormatName(rNewData.GetSectionName());
    if ( pAttr )
    {
        pFormat->SetFormatAttr( *pAttr );
    }

    SwTOXBase const*const pTOXBase(pTOXBaseAndMode ? std::get<0>(*pTOXBaseAndMode) : nullptr);
    SwSectionNode* pNewSectNode = nullptr;

    RedlineFlags eOld = getIDocumentRedlineAccess().GetRedlineFlags();
    getIDocumentRedlineAccess().SetRedlineFlags_intern( (eOld & ~RedlineFlags::ShowMask) | RedlineFlags::Ignore );

    if( rRange.HasMark() )
    {
        auto [pSttPos, pEndPos] = const_cast<SwPaM&>(rRange).StartEnd(); // SwPosition*
        if( pPrvNd && 3 == nRegionRet )
        {
            OSL_ENSURE( pPrvNd, "The SectionNode is missing" );
            SwNodeIndex aStt( pSttPos->GetNode() ), aEnd( pEndPos->GetNode(), +1 );
            while( pPrvNd != aStt.GetNode().StartOfSectionNode() )
                --aStt;
            while( pPrvNd != aEnd.GetNode().StartOfSectionNode() )
                ++aEnd;

            --aEnd; // End is inclusive in the InsertSection
            pNewSectNode = GetNodes().InsertTextSection(
                        aStt.GetNode(), *pFormat, rNewData, pTOXBase, & aEnd.GetNode());
        }
        else
        {
            if( pUndoInsSect )
            {
                if( !( pPrvNd && 1 == nRegionRet ) &&
                    pSttPos->GetContentIndex() )
                {
                    SwTextNode* const pTNd =
                        pSttPos->GetNode().GetTextNode();
                    if (pTNd)
                    {
                        pUndoInsSect->SaveSplitNode( pTNd, true );
                    }
                }

                if ( !( pPrvNd && 2 == nRegionRet ) )
                {
                    SwTextNode *const pTNd =
                        pEndPos->GetNode().GetTextNode();
                    if (pTNd && (pTNd->GetText().getLength()
                                    != pEndPos->GetContentIndex()))
                    {
                        pUndoInsSect->SaveSplitNode( pTNd, false );
                    }
                }
            }

            if( pPrvNd && 1 == nRegionRet )
            {
                pSttPos->Assign( *pPrvNd );
            }
            else if( pSttPos->GetContentIndex() )
            {
                getIDocumentContentOperations().SplitNode( *pSttPos, false );
            }

            if( pPrvNd && 2 == nRegionRet )
            {
                pEndPos->Assign( *pPrvNd );
            }
            else
            {
                const SwContentNode* pCNd = pEndPos->GetNode().GetContentNode();
                if( pCNd && pCNd->Len() != pEndPos->GetContentIndex() )
                {
                    sal_Int32 nContent = pSttPos->GetContentIndex();
                    getIDocumentContentOperations().SplitNode( *pEndPos, false );

                    SwTextNode* pTNd;
                    if( pEndPos->GetNodeIndex() == pSttPos->GetNodeIndex() )
                    {
                        pSttPos->Adjust(SwNodeOffset(-1));
                        pEndPos->Adjust(SwNodeOffset(-1));
                        pTNd = pSttPos->GetNode().GetTextNode();
                        pSttPos->SetContent( nContent );
                    }
                    else
                    {
                        // Set to the end of the previous
                        pEndPos->Adjust(SwNodeOffset(-1));
                        pTNd = pEndPos->GetNode().GetTextNode();
                    }
                    nContent = pTNd ? pTNd->GetText().getLength() : 0;
                    pEndPos->SetContent( nContent );
                }
            }
            pNewSectNode = GetNodes().InsertTextSection(
                pSttPos->GetNode(), *pFormat, rNewData, pTOXBase, &pEndPos->GetNode());
        }
    }
    else
    {
        const SwPosition* pPos = rRange.GetPoint();
        const SwContentNode* pCNd = pPos->GetNode().GetContentNode();
        if( !pPos->GetContentIndex() )
        {
            pNewSectNode = GetNodes().InsertTextSection(
                pPos->GetNode(), *pFormat, rNewData, pTOXBase, nullptr);
        }
        else if( pPos->GetContentIndex() == pCNd->Len() )
        {
            pNewSectNode = GetNodes().InsertTextSection(
                pPos->GetNode(), *pFormat, rNewData, pTOXBase, nullptr, false);
        }
        else
        {
            if (pUndoInsSect)
                if (const SwTextNode* pTextNode = pCNd->GetTextNode())
                    pUndoInsSect->SaveSplitNode(pTextNode, true);
            getIDocumentContentOperations().SplitNode( *pPos, false );
            pNewSectNode = GetNodes().InsertTextSection(
                pPos->GetNode(), *pFormat, rNewData, pTOXBase, nullptr);
        }
    }

    pNewSectNode->CheckSectionCondColl();

    getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );

    // To-Do - add 'SwExtraRedlineTable' also ?
    if( getIDocumentRedlineAccess().IsRedlineOn() || (!getIDocumentRedlineAccess().IsIgnoreRedline() && !getIDocumentRedlineAccess().GetRedlineTable().empty() ))
    {
        SwPaM aPam( *pNewSectNode->EndOfSectionNode(), *pNewSectNode, SwNodeOffset(1) );
        if( getIDocumentRedlineAccess().IsRedlineOn() )
        {
            getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( RedlineType::Insert, aPam ), true);
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
            getIDocumentFieldsAccess().FieldsToCalc(aCalc, pNewSectNode->GetIndex(), SAL_MAX_INT32);
        }
        SwSection& rNewSect = pNewSectNode->GetSection();
        rNewSect.SetCondHidden( aCalc.Calculate( rNewSect.GetCondition() ).GetBool() );
    }

    bool bUpdateFootnote = false;
    if( !GetFootnoteIdxs().empty() && pAttr )
    {
        sal_uInt16 nVal = pAttr->Get( RES_FTN_AT_TXTEND ).GetValue();
        if( ( FTNEND_ATTXTEND_OWNNUMSEQ == nVal ||
              FTNEND_ATTXTEND_OWNNUMANDFMT == nVal ) ||
            ( FTNEND_ATTXTEND_OWNNUMSEQ == ( nVal = pAttr->Get( RES_END_AT_TXTEND ).GetValue() ) ||
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
        pNewSectNode->GetSection().CreateLink( bUpdate ? LinkCreateType::Update : LinkCreateType::Connect );
    }

    if( bUpdateFootnote )
    {
        GetFootnoteIdxs().UpdateFootnote( *pNewSectNode );
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
        auto [pStt, pEnd] = rRange.StartEnd(); // SwPosition*

        const SwContentNode* pCNd = pEnd->GetNode().GetContentNode();
        const SwNode* pNd = &pStt->GetNode();
        const SwSectionNode* pSectNd = pNd->FindSectionNode();
        const SwSectionNode* pEndSectNd = pCNd ? pCNd->FindSectionNode() : nullptr;
        if( pSectNd && pEndSectNd && pSectNd != pEndSectNd )
        {
            // Try to create an enclosing Section, but only if Start is
            // located at the Section's beginning and End at it's end
            nRet = 0;
            if( !pStt->GetContentIndex()
                && pSectNd->GetIndex() == pStt->GetNodeIndex() - 1
                && pEnd->GetContentIndex() == pCNd->Len() )
            {
                SwNodeIndex aIdx( pStt->GetNode(), -1 );
                SwNodeOffset nCmp = pEnd->GetNodeIndex();
                const SwStartNode* pPrvNd;
                const SwEndNode* pNxtNd;
                while( nullptr != ( pPrvNd = (pNd = &aIdx.GetNode())->GetSectionNode() ) &&
                    ( aIdx.GetIndex() >= nCmp ||
                        nCmp >= pPrvNd->EndOfSectionIndex() ) )
                {
                    --aIdx;
                }
                if( !pPrvNd )
                    pPrvNd = pNd->IsStartNode() ? static_cast<const SwStartNode*>(pNd)
                                                : pNd->StartOfSectionNode();

                aIdx = pEnd->GetNodeIndex() + 1;
                nCmp = pStt->GetNodeIndex();
                while( nullptr != ( pNxtNd = (pNd = &aIdx.GetNode())->GetEndNode() ) &&
                    pNxtNd->StartOfSectionNode()->IsSectionNode() &&
                    ( pNxtNd->StartOfSectionIndex() >= nCmp ||
                        nCmp >= aIdx.GetIndex() ) )
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
            if( pEnd->GetContentIndex() == pCNd->Len() )
            {
                SwNodeIndex aIdx( pEnd->GetNode(), 1 );
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
            if( !pStt->GetContentIndex() )
            {
                SwNodeIndex aIdx( pStt->GetNode(), -1 );
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
    SwSectionNode* pSectNd = rPos.GetNode().FindSectionNode();
    if( pSectNd )
        return &pSectNd->GetSection();
    return nullptr;
}

SwSectionFormat* SwDoc::MakeSectionFormat()
{
    SwSectionFormat* pNew = new SwSectionFormat( mpDfltFrameFormat.get(), this );
    mpSectionFormatTable->push_back( pNew );
    return pNew;
}

void SwDoc::DelSectionFormat( SwSectionFormat *pFormat, bool bDelNodes )
{
    SwSectionFormats::iterator itFormatPos = std::find( mpSectionFormatTable->begin(), mpSectionFormatTable->end(), pFormat );

    GetIDocumentUndoRedo().StartUndo(SwUndoId::DELSECTION, nullptr);

    if( mpSectionFormatTable->end() != itFormatPos )
    {
        const SwNodeIndex* pIdx = pFormat->GetContent( false ).GetContentIdx();
        const SfxPoolItem* pFootnoteEndAtTextEnd = pFormat->GetItemIfSet(
                            RES_FTN_AT_TXTEND);
        if( !pFootnoteEndAtTextEnd )
            pFootnoteEndAtTextEnd = pFormat->GetItemIfSet(RES_END_AT_TXTEND);

        SwSectionNode* pSectNd;

        if( GetIDocumentUndoRedo().DoesUndo() )
        {
            if( bDelNodes && pIdx && &GetNodes() == &pIdx->GetNodes() &&
                nullptr != (pSectNd = pIdx->GetNode().GetSectionNode() ))
            {
                SwNodeIndex aUpdIdx( *pIdx );
                SwPaM aPaM( *pSectNd->EndOfSectionNode(), *pSectNd );
                GetIDocumentUndoRedo().AppendUndo(std::make_unique<SwUndoDelete>(aPaM, SwDeleteFlags::Default));
                if( pFootnoteEndAtTextEnd )
                    GetFootnoteIdxs().UpdateFootnote( aUpdIdx.GetNode() );
                getIDocumentState().SetModified();
                //#126178# start/end undo have to be pairs!
                GetIDocumentUndoRedo().EndUndo(SwUndoId::DELSECTION, nullptr);
                return ;
            }
            GetIDocumentUndoRedo().AppendUndo( MakeUndoDelSection( *pFormat ) );
        }
        else if( bDelNodes && pIdx && &GetNodes() == &pIdx->GetNodes() &&
                nullptr != (pSectNd = pIdx->GetNode().GetSectionNode() ))
        {
            SwNodeIndex aUpdIdx( *pIdx );
            getIDocumentContentOperations().DeleteSection(pSectNd);
            if( pFootnoteEndAtTextEnd )
                GetFootnoteIdxs().UpdateFootnote( aUpdIdx.GetNode() );
            getIDocumentState().SetModified();
            //#126178# start/end undo have to be pairs!
            GetIDocumentUndoRedo().EndUndo(SwUndoId::DELSECTION, nullptr);
            return ;
        }

        pFormat->RemoveAllUnos();

        // A ClearRedo could result in a recursive call of this function and delete some section
        // formats, thus the position inside the SectionFormatTable could have changed
        itFormatPos = std::find( mpSectionFormatTable->begin(), mpSectionFormatTable->end(), pFormat );

        // WARNING: First remove from the array and then delete,
        //          as the Section DTOR tries to delete it's format itself.
        mpSectionFormatTable->erase( itFormatPos );

        SwNodeOffset nCnt(0), nSttNd(0);
        if( pIdx && &GetNodes() == &pIdx->GetNodes() &&
            nullptr != (pSectNd = pIdx->GetNode().GetSectionNode() ))
        {
            nSttNd = pSectNd->GetIndex();
            nCnt = pSectNd->EndOfSectionIndex() - nSttNd - 1;
        }

        delete pFormat;

        if( nSttNd && pFootnoteEndAtTextEnd )
        {
            SwNodeIndex aUpdIdx( GetNodes(), nSttNd );
            GetFootnoteIdxs().UpdateFootnote( aUpdIdx.GetNode() );
        }

        SwContentNode* pCNd;
        for( ; nCnt--; ++nSttNd )
            if( nullptr != (pCNd = GetNodes()[ nSttNd ]->GetContentNode() ) &&
                RES_CONDTXTFMTCOLL == pCNd->GetFormatColl()->Which() )
                pCNd->ChkCondColl();
    }

    GetIDocumentUndoRedo().EndUndo(SwUndoId::DELSECTION, nullptr);

    if (GetIDocumentUndoRedo().DoesUndo())
    {   // TODO is this ever needed?
        getIDocumentState().SetModified();
    }
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
            const SfxPoolItem* pItem = aIter.GetCurItem();
            do
            {
                if (pFormat->GetFormatAttr(pItem->Which()) != *pItem)
                {
                    bOnlyAttrChg = true;
                    break;
                }

                pItem = aIter.NextItem();
            } while (pItem);
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
                    ::lcl_CheckEmptyLayFrame( rNewData,
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
    OUString sCompareString = OUStringChar(sfx2::cTokenSeparator) + OUStringChar(sfx2::cTokenSeparator);
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
        getIDocumentFieldsAccess().FieldsToCalc(aCalc, pIdx->GetIndex(), SAL_MAX_INT32);

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
        pSection->CreateLink( bPreventLinkUpdate ? LinkCreateType::Connect : LinkCreateType::Update );
    else if( !pSection->IsLinkType() && pSection->IsConnected() )
    {
        pSection->Disconnect();
        getIDocumentLinksAdministration().GetLinkManager().Remove( &pSection->GetBaseLink() );
    }

    getIDocumentState().SetModified();
}

void sw_DeleteFootnote( SwSectionNode *pNd, SwNodeOffset nStt, SwNodeOffset nEnd )
{
    SwFootnoteIdxs& rFootnoteArr = pNd->GetDoc().GetFootnoteIdxs();
    if( rFootnoteArr.empty() )
        return;

    size_t nPos = 0;
    rFootnoteArr.SeekEntry( *pNd, &nPos );
    SwTextFootnote* pSrch;

    // Delete all succeeding Footnotes
    while( nPos < rFootnoteArr.size() &&
        SwTextFootnote_GetIndex( (pSrch = rFootnoteArr[ nPos ]) ) <= nEnd )
    {
        // If the Nodes are not deleted, they need to deregister at the Pages
        // (delete Frames) or else they will remain there (Undo does not delete them!)
        pSrch->DelFrames(nullptr);
        ++nPos;
    }

    while( nPos-- &&
        SwTextFootnote_GetIndex( (pSrch = rFootnoteArr[ nPos ]) ) >= nStt )
    {
        // If the Nodes are not deleted, they need to deregister at the Pages
        // (delete Frames) or else they will remain there (Undo does not delete them!)
        pSrch->DelFrames(nullptr);
    }
}

static bool lcl_IsTOXSection(SwSectionData const& rSectionData)
{
    return (SectionType::ToxContent == rSectionData.GetType())
        || (SectionType::ToxHeader  == rSectionData.GetType());
}

SwSectionNode* SwNodes::InsertTextSection(SwNode& rNd,
                                SwSectionFormat& rSectionFormat,
                                SwSectionData const& rSectionData,
                                SwTOXBase const*const pTOXBase,
                                SwNode const*const pEndNd,
                                bool const bInsAtStart, bool const bCreateFrames)
{
    SwNodeIndex aInsPos( rNd );
    if( !pEndNd ) // No Area, thus create a new Section before/after it
    {
        // #i26762#
        OSL_ENSURE(!pEndNd || rNd.GetIndex() <= pEndNd->GetIndex(),
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
            new SwSectionNode(aInsPos.GetNode(), rSectionFormat, pTOXBase);

    if (lcl_IsTOXSection(rSectionData))
    {
        // We're inserting a ToX. Make sure that if a redline ends right before the ToX start, then
        // that end now doesn't cross a section start node.
        SwRedlineTable& rRedlines = GetDoc().getIDocumentRedlineAccess().GetRedlineTable();
        for (SwRedlineTable::size_type nIndex = 0; nIndex < rRedlines.size(); ++nIndex)
        {
            SwRangeRedline* pRedline = rRedlines[nIndex];
            if ( RedlineType::Delete != pRedline->GetType() ||
                 !pRedline->HasMark() || pRedline->GetMark()->GetNode() != aInsPos.GetNode() )
            {
                continue;
            }

            // The redline ends at the new section content start, so it originally ended before the
            // section start: move it back.
            SwPaM aRedlineEnd(*pRedline->GetMark());
            aRedlineEnd.Move(fnMoveBackward);
            *pRedline->GetMark() = *aRedlineEnd.GetPoint();
            break;
        }
    }

    if( pEndNd )
    {
        // Special case for the Reader/Writer
        if( *pEndNd != GetEndOfContent() )
            aInsPos = pEndNd->GetIndex()+1;
        // #i58710: We created a RTF document with a section break inside a table cell
        // We are not able to handle a section start inside a table and the section end outside.
        const SwNode* pLastNode = pSectNd->StartOfSectionNode()->EndOfSectionNode();
        if( aInsPos > pLastNode->GetIndex() )
            aInsPos = pLastNode->GetIndex();
        // Another way round: if the section starts outside a table but the end is inside...
        // aInsPos is at the moment the Position where my EndNode will be inserted
        const SwStartNode* pStartNode = aInsPos.GetNode().StartOfSectionNode();
        // This StartNode should be in front of me, but if not, I want to survive
        SwNodeOffset nMyIndex = pSectNd->GetIndex();
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
        SwTextNode* pCpyTNd = rNd.GetTextNode();
        if( pCpyTNd )
        {
            SwTextNode* pTNd = new SwTextNode( aInsPos.GetNode(), pCpyTNd->GetTextColl() );
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
            pCpyTNd->MakeFramesForAdjacentContentNode(*pTNd);
        }
        else
            new SwTextNode( aInsPos.GetNode(), GetDoc().GetDfltTextFormatColl() );
    }
    new SwEndNode( aInsPos.GetNode(), *pSectNd );

    pSectNd->GetSection().SetSectionData(rSectionData);
    SwSectionFormat* pSectFormat = pSectNd->GetSection().GetFormat();

    // We could optimize this, by not removing already contained Frames and recreating them,
    // but by simply rewiring them
    bool bInsFrame = bCreateFrames && !pSectNd->GetSection().IsHiddenFlag() &&
                   GetDoc().getIDocumentLayoutAccess().GetCurrentViewShell();
    std::optional<SwNode2LayoutSaveUpperFrames> oNode2Layout;
    if( bInsFrame )
    {
        if( !pSectNd->GetNodes().FindPrvNxtFrameNode( *pSectNd, pSectNd->EndOfSectionNode() ) )
            // Collect all Uppers
            oNode2Layout.emplace(*pSectNd);
    }

    // Set the right StartNode for all in this Area
    SwNodeOffset nEnd = pSectNd->EndOfSectionIndex();
    SwNodeOffset nStart = pSectNd->GetIndex()+1;
    SwNodeOffset nSkipIdx = NODE_OFFSET_MAX;
    for( SwNodeOffset n = nStart; n < nEnd; ++n )
    {
        SwNode* pNd = (*this)[n];

        // Attach all Sections in the NodeSection underneath the new one
        if( NODE_OFFSET_MAX == nSkipIdx )
            pNd->m_pStartOfSection = pSectNd;
        else if( n >= nSkipIdx )
            nSkipIdx = NODE_OFFSET_MAX;

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

                if( NODE_OFFSET_MAX == nSkipIdx )
                    nSkipIdx = pNd->EndOfSectionIndex();
            }
        }
        else if( pNd->IsContentNode() )
            static_cast<SwContentNode*>(pNd)->DelFrames(nullptr);
    }

    sw_DeleteFootnote( pSectNd, nStart, nEnd );

    if( bInsFrame )
    {
        if( oNode2Layout )
        {
            SwNodeOffset nIdx = pSectNd->GetIndex();
            oNode2Layout->RestoreUpperFrames( pSectNd->GetNodes(), nIdx, nIdx + 1 );
            oNode2Layout.reset();
        }
        else
            pSectNd->MakeOwnFrames(&aInsPos);
    }

    return pSectNd;
}

SwSectionNode* SwNode::FindSectionNode()
{
    for (SwNode* tmp = this;; tmp = tmp->StartOfSectionNode())
        if (SwSectionNode* sectNode = tmp->GetSectionNode(); sectNode || !tmp->GetIndex())
            return sectNode;
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

SwSectionNode::SwSectionNode(const SwNode& rWhere,
        SwSectionFormat & rFormat, SwTOXBase const*const pTOXBase)
    : SwStartNode( rWhere, SwNodeType::Section )
    , m_pSection( pTOXBase
        ? new SwTOXBaseSection(*pTOXBase, lcl_initParent(*this, rFormat))
        : new SwSection( SectionType::Content, rFormat.GetName(),
                lcl_initParent(*this, rFormat) ) )
{
    // Set the connection from Format to Node
    // Suppress Modify; no one's interested anyway
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

SwFrame* SwSectionNode::MakeFrame(SwFrame* pSib, bool bHidden)
{
    m_pSection->m_Data.SetHiddenFlag(bHidden);
    return new SwSectionFrame( *m_pSection, pSib );
}

// Creates all Document Views for the preceding Node.
// The created ContentFrames are attached to the corresponding Layout
void SwSectionNode::MakeFramesForAdjacentContentNode(const SwNodeIndex & rIdx)
{
    // Take my successive or preceding ContentFrame
    SwNodes& rNds = GetNodes();
    if( !(rNds.IsDocNodes() && rNds.GetDoc().getIDocumentLayoutAccess().GetCurrentViewShell()) )
        return;

    if (GetSection().IsHiddenFlag() || IsContentHidden())
    {
        SwNodeIndex aIdx( *EndOfSectionNode() );
        SwContentNode* pCNd = SwNodes::GoNextSection(&aIdx, true, false);
        if( !pCNd )
        {
            aIdx = *this;
            pCNd = SwNodes::GoPrevSection(&aIdx, true, false);
            if (!pCNd)
                return;
        }
        pCNd = aIdx.GetNode().GetContentNode();
        pCNd->MakeFramesForAdjacentContentNode(static_cast<SwContentNode&>(rIdx.GetNode()));
    }
    else
    {
        SwNode2Layout aNode2Layout( *this, rIdx.GetIndex() );
        SwFrame *pFrame;
        while( nullptr != (pFrame = aNode2Layout.NextFrame()) )
        {
            OSL_ENSURE( pFrame->IsSctFrame(), "Depend of Section not a Section." );
            if (pFrame->getRootFrame()->HasMergedParas()
                && !rIdx.GetNode().IsCreateFrameWhenHidingRedlines())
            {
                continue;
            }
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
            // has to be created...
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
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
                if ( pNew->IsTextFrame() )
                {
                    SwViewShell* pViewShell( pNew->getRootFrame()->GetCurrShell() );
                    if ( pViewShell && pViewShell->GetLayout() &&
                         pViewShell->GetLayout()->IsAnyShellAccessible() )
                    {
                        auto pNext = pNew->FindNextCnt( true );
                        auto pPrev = pNew->FindPrevCnt();
                        pViewShell->InvalidateAccessibleParaFlowRelation(
                            pNext ? pNext->DynCastTextFrame() : nullptr,
                            pPrev ? pPrev->DynCastTextFrame() : nullptr );
                    }
                }
#endif
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
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
            if ( pNew->IsTextFrame() )
            {
                SwViewShell* pViewShell( pNew->getRootFrame()->GetCurrShell() );
                if ( pViewShell && pViewShell->GetLayout() &&
                     pViewShell->GetLayout()->IsAnyShellAccessible() )
                {
                    auto pNext = pNew->FindNextCnt( true );
                    auto pPrev = pNew->FindPrevCnt();
                    pViewShell->InvalidateAccessibleParaFlowRelation(
                        pNext ? pNext->DynCastTextFrame() : nullptr,
                        pPrev ? pPrev->DynCastTextFrame() : nullptr );
                }
            }
#endif
            if ( bInitNewSect )
                static_cast<SwSectionFrame*>(pNew)->Init();
        }
    }
}

// Create a new SectionFrame for every occurrence in the Layout and insert before
// the corresponding ContentFrame
void SwSectionNode::MakeOwnFrames(SwNodeIndex* pIdxBehind, SwNodeIndex* pEndIdx)
{
    assert(pIdxBehind && "no Index");
    SwNodes& rNds = GetNodes();
    SwDoc& rDoc = rNds.GetDoc();

    *pIdxBehind = *this;

    m_pSection->m_Data.SetHiddenFlag(true);

    if( rNds.IsDocNodes() )
    {
        if( pEndIdx )
            ::MakeFrames( &rDoc, pIdxBehind->GetNode(), pEndIdx->GetNode() );
        else
            ::MakeFrames( &rDoc, pIdxBehind->GetNode(), SwNodeIndex( *EndOfSectionNode(), 1 ).GetNode() );
    }
}

void SwSectionNode::DelFrames(SwRootFrame const*const /*FIXME TODO*/, bool const bForce)
{
    SwNodeOffset nStt = GetIndex()+1, nEnd = EndOfSectionIndex();
    if( nStt >= nEnd )
    {
        return ;
    }

    m_pSection->GetFormat()->DelFrames();

    // Update our Flag
    m_pSection->m_Data.SetHiddenFlag(true);

    // If the Area is within a Fly or TableBox, we can only hide it if
    // there is more Content which has Frames.
    // Or else the Fly/TableBox Frame does not have a Lower!
    if (bForce)
        return;

    SwNodeIndex aIdx( *this );
    if( !SwNodes::GoPrevSection( &aIdx, true, false ) ||
        !CheckNodesRange( *this, aIdx.GetNode(), true ) ||
        // #i21457#
        !lcl_IsInSameTableBox( *this, true ))
    {
        aIdx = *EndOfSectionNode();
        if( !SwNodes::GoNextSection( &aIdx, true, false ) ||
            !CheckNodesRange( *EndOfSectionNode(), aIdx.GetNode(), true ) ||
            // #i21457#
            !lcl_IsInSameTableBox( *EndOfSectionNode(), false ))
        {
            m_pSection->m_Data.SetHiddenFlag(false);
        }
    }
}

SwSectionNode* SwSectionNode::MakeCopy( SwDoc& rDoc, const SwNodeIndex& rIdx ) const
{
    // In which array am I: Nodes, UndoNodes?
    const SwNodes& rNds = GetNodes();

    // Copy the SectionFrameFormat
    SwSectionFormat* pSectFormat = rDoc.MakeSectionFormat();
    pSectFormat->CopyAttrs( *GetSection().GetFormat() );

    std::unique_ptr<SwTOXBase> pTOXBase;
    if (SectionType::ToxContent == GetSection().GetType())
    {
        assert( dynamic_cast< const SwTOXBaseSection* >( &GetSection() ) && "no TOXBaseSection!" );
        SwTOXBaseSection const& rTBS(
            dynamic_cast<SwTOXBaseSection const&>(GetSection()));
        pTOXBase.reset( new SwTOXBase(rTBS, &rDoc) );
    }

    SwSectionNode *const pSectNd =
        new SwSectionNode(rIdx.GetNode(), *pSectFormat, pTOXBase.get());
    SwEndNode* pEndNd = new SwEndNode( rIdx.GetNode(), *pSectNd );
    SwNodeIndex aInsPos( *pEndNd );

    // Take over values
    SwSection *const pNewSect = pSectNd->m_pSection.get();

    if (SectionType::ToxContent != GetSection().GetType())
    {
        // Keep the Name for Move
        if( &rNds.GetDoc() == &rDoc && rDoc.IsCopyIsMove() )
        {
            pNewSect->SetSectionName( GetSection().GetSectionName() );
        }
        else
        {
            const OUString sSectionName(GetSection().GetSectionName());
            pNewSect->SetSectionName(rDoc.GetUniqueSectionName( &sSectionName ));
        }
    }

    pNewSect->SetType( GetSection().GetType() );
    pNewSect->SetCondition( GetSection().GetCondition() );
    pNewSect->SetCondHidden( GetSection().IsCondHidden() );
    pNewSect->SetLinkFileName( GetSection().GetLinkFileName() );
    if( !pNewSect->IsHiddenFlag() && GetSection().IsHidden() )
        pNewSect->SetHidden();
    if( !pNewSect->IsProtectFlag() && GetSection().IsProtect() )
        pNewSect->SetProtect();
    // edit in readonly sections
    if( !pNewSect->IsEditInReadonlyFlag() && GetSection().IsEditInReadonly() )
        pNewSect->SetEditInReadonly();

    SwNodeRange aRg( *this, SwNodeOffset(+1), *EndOfSectionNode() ); // Where am I?
    rNds.Copy_( aRg, aInsPos.GetNode(), false );

    // Delete all Frames from the copied Area. They are created when creating
    // the SectionFrames.
    pSectNd->DelFrames();

    // Copy the Links/Server
    if( pNewSect->IsLinkType() ) // Add the Link
        pNewSect->CreateLink( rDoc.getIDocumentLayoutAccess().GetCurrentViewShell() ? LinkCreateType::Connect : LinkCreateType::NONE );

    // If we copy from the Undo as Server, enter it again
    if (m_pSection->IsServer()
        && rDoc.GetIDocumentUndoRedo().IsUndoNodes(rNds))
    {
        pNewSect->SetRefObject( m_pSection->GetObject() );
        rDoc.getIDocumentLinksAdministration().GetLinkManager().InsertServer( pNewSect->GetObject() );
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
    SwNodeOffset nEnd = EndOfSectionIndex();
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

void SwSectionNode::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("section"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteAttribute(
        pWriter, BAD_CAST("type"),
        BAD_CAST(OString::number(static_cast<sal_uInt8>(GetNodeType())).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("index"),
                                      BAD_CAST(OString::number(sal_Int32(GetIndex())).getStr()));

    if (m_pSection)
    {
        m_pSection->dumpAsXml(pWriter);
    }

    // (void)xmlTextWriterEndElement(pWriter); - it is a start node, so don't end, will make xml better nested
}

void SwSectionNode::NodesArrChgd()
{
    SwSectionFormat *const pFormat = m_pSection->GetFormat();
    if( !pFormat )
        return;

    SwNodes& rNds = GetNodes();
    SwDoc* pDoc = pFormat->GetDoc();

    if( !rNds.IsDocNodes() )
    {
        pFormat->RemoveAllUnos();
    }

    pFormat->LockModify();
    pFormat->SetFormatAttr( SwFormatContent( this ));
    pFormat->UnlockModify();

    SwSectionNode* pSectNd = StartOfSectionNode()->FindSectionNode();
    // set the correct parent from the new section
    pFormat->SetDerivedFrom( pSectNd ? pSectNd->GetSection().GetFormat()
                                  : pDoc->GetDfltFrameFormat() );

    // Set the right StartNode for all in this Area
    SwNodeOffset nStart = GetIndex()+1, nEnd = EndOfSectionIndex();
    for( SwNodeOffset n = nStart; n < nEnd; ++n )
    {
        // Make up the Format's nesting
        pSectNd = rNds[ n ]->GetSectionNode();
        if( nullptr != pSectNd )
        {
            pSectNd->GetSection().GetFormat()->SetDerivedFrom( pFormat );
            n = pSectNd->EndOfSectionIndex();
        }
    }

    // Moving Nodes to the UndoNodes array?
    if( rNds.IsDocNodes() )
    {
        OSL_ENSURE( pDoc == &GetDoc(),
                "Moving to different Documents?" );
        if( m_pSection->IsLinkType() ) // Remove the Link
            m_pSection->CreateLink( pDoc->getIDocumentLayoutAccess().GetCurrentViewShell() ? LinkCreateType::Connect : LinkCreateType::NONE );

        if (m_pSection->IsServer())
            pDoc->getIDocumentLinksAdministration().GetLinkManager().InsertServer( m_pSection->GetObject() );
    }
    else
    {
        if (SectionType::Content != m_pSection->GetType()
            && m_pSection->IsConnected())
        {
            pDoc->getIDocumentLinksAdministration().GetLinkManager().Remove( &m_pSection->GetBaseLink() );
        }
        if (m_pSection->IsServer())
            pDoc->getIDocumentLinksAdministration().GetLinkManager().RemoveServer( m_pSection->GetObject() );
    }

}

OUString SwDoc::GetUniqueSectionName( const OUString* pChkStr ) const
{
    if( IsInMailMerge())
    {
        OUString newName = "MailMergeSection"
            + DateTimeToOUString( DateTime( DateTime::SYSTEM ) )
            + OUString::number( mpSectionFormatTable->size() + 1 );
        if( pChkStr )
            newName += *pChkStr;
        return newName;
    }

    const OUString aName(SwResId(STR_REGION_DEFNAME));

    SwSectionFormats::size_type nNum = 0;
    const SwSectionFormats::size_type nFlagSize = ( mpSectionFormatTable->size() / 8 ) + 2;
    std::unique_ptr<sal_uInt8[]> pSetFlags(new sal_uInt8[ nFlagSize ]);
    memset( pSetFlags.get(), 0, nFlagSize );

    for( auto pFormat : *mpSectionFormatTable )
    {
        const SwSectionNode *const pSectNd = pFormat->GetSectionNode();
        if( pSectNd != nullptr )
        {
            const OUString& rNm = pSectNd->GetSection().GetSectionName();
            if (rNm.startsWith( aName ))
            {
                // Calculate the Number and reset the Flag
                nNum = o3tl::toInt32(rNm.subView( aName.getLength() ));
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
    pSetFlags.reset();
    if( pChkStr )
        return *pChkStr;
    return aName + OUString::number( ++nNum );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
