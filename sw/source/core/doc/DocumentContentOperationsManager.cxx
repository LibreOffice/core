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
#include <DocumentContentOperationsManager.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentMarkAccess.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentSettingAccess.hxx>
#include <UndoManager.hxx>
#include <docary.hxx>
#include <textboxhelper.hxx>
#include <dcontact.hxx>
#include <grfatr.hxx>
#include <numrule.hxx>
#include <charfmt.hxx>
#include <ndgrf.hxx>
#include <ndnotxt.hxx>
#include <ndole.hxx>
#include <breakit.hxx>
#include <frmfmt.hxx>
#include <fmtanchr.hxx>
#include <fmtcntnt.hxx>
#include <fmtinfmt.hxx>
#include <fmtpdsc.hxx>
#include <fmtcnct.hxx>
#include <SwStyleNameMapper.hxx>
#include <redline.hxx>
#include <txtfrm.hxx>
#include <rootfrm.hxx>
#include <frmtool.hxx>
#include <unocrsr.hxx>
#include <mvsave.hxx>
#include <ndtxt.hxx>
#include <poolfmt.hxx>
#include <paratr.hxx>
#include <txatbase.hxx>
#include <UndoRedline.hxx>
#include <undobj.hxx>
#include <UndoBookmark.hxx>
#include <UndoDelete.hxx>
#include <UndoSplitMove.hxx>
#include <UndoOverwrite.hxx>
#include <UndoInsert.hxx>
#include <UndoAttribute.hxx>
#include <rolbck.hxx>
#include <acorrect.hxx>
#include <bookmrk.hxx>
#include <ftnidx.hxx>
#include <txtftn.hxx>
#include <hints.hxx>
#include <fmtflcnt.hxx>
#include <docedt.hxx>
#include <frameformats.hxx>
#include <o3tl/safeint.hxx>
#include <sal/log.hxx>
#include <unotools/charclass.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <i18nutil/transliteration.hxx>
#include <sfx2/Metadatable.hxx>
#include <sot/exchange.hxx>
#include <svl/stritem.hxx>
#include <svl/itemiter.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdouno.hxx>
#include <tools/globname.hxx>
#include <editeng/formatbreakitem.hxx>
#include <com/sun/star/i18n/Boundary.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>

#include <tuple>
#include <memory>

using namespace ::com::sun::star::i18n;

namespace
{
    // Copy method from SwDoc
    // Prevent copying into Flys that are anchored in the range
    bool lcl_ChkFlyFly( SwDoc& rDoc, sal_uLong nSttNd, sal_uLong nEndNd,
                        sal_uLong nInsNd )
    {
        const SwFrameFormats& rFrameFormatTable = *rDoc.GetSpzFrameFormats();

        for( size_t n = 0; n < rFrameFormatTable.size(); ++n )
        {
            SwFrameFormat const*const  pFormat = rFrameFormatTable[n];
            SwFormatAnchor const*const pAnchor = &pFormat->GetAnchor();
            SwPosition const*const pAPos = pAnchor->GetContentAnchor();
            if (pAPos &&
                ((RndStdIds::FLY_AS_CHAR == pAnchor->GetAnchorId()) ||
                 (RndStdIds::FLY_AT_CHAR == pAnchor->GetAnchorId()) ||
                 (RndStdIds::FLY_AT_FLY  == pAnchor->GetAnchorId()) ||
                 (RndStdIds::FLY_AT_PARA == pAnchor->GetAnchorId())) &&
                nSttNd <= pAPos->nNode.GetIndex() &&
                pAPos->nNode.GetIndex() < nEndNd )
            {
                const SwFormatContent& rContent = pFormat->GetContent();
                SwStartNode* pSNd;
                if( !rContent.GetContentIdx() ||
                    nullptr == ( pSNd = rContent.GetContentIdx()->GetNode().GetStartNode() ))
                    continue;

                if( pSNd->GetIndex() < nInsNd &&
                    nInsNd < pSNd->EndOfSectionIndex() )
                    // Do not copy !
                    return true;

                if( lcl_ChkFlyFly( rDoc, pSNd->GetIndex(),
                            pSNd->EndOfSectionIndex(), nInsNd ) )
                    // Do not copy !
                    return true;
            }
        }

        return false;
    }

    SwNodeIndex InitDelCount(SwPaM const& rSourcePaM, sal_uLong & rDelCount)
    {
        SwNodeIndex const& rStart(rSourcePaM.Start()->nNode);
        // Special handling for SwDoc::AppendDoc
        if (rSourcePaM.GetDoc().GetNodes().GetEndOfExtras().GetIndex() + 1
                == rStart.GetIndex())
        {
            rDelCount = 1;
            return SwNodeIndex(rStart, +1);
        }
        else
        {
            rDelCount = 0;
            return rStart;
        }
    }

    /*
        The CopyBookmarks function has to copy bookmarks from the source to the destination nodes
        array. It is called after a call of the CopyNodes(..) function. But this function does not copy
        every node (at least at the moment: 2/08/2006 ), section start and end nodes will not be copied
        if the corresponding end/start node is outside the copied pam.
        The lcl_NonCopyCount function counts the number of these nodes, given the copied pam and a node
        index inside the pam.
        rPam is the original source pam, rLastIdx is the last calculated position, rDelCount the number
        of "non-copy" nodes between rPam.Start() and rLastIdx.
        nNewIdx is the new position of interest.
    */
    void lcl_NonCopyCount( const SwPaM& rPam, SwNodeIndex& rLastIdx, const sal_uLong nNewIdx, sal_uLong& rDelCount )
    {
        sal_uLong nStart = rPam.Start()->nNode.GetIndex();
        sal_uLong nEnd = rPam.End()->nNode.GetIndex();
        if( rLastIdx.GetIndex() < nNewIdx ) // Moving forward?
        {
            // We never copy the StartOfContent node
            do // count "non-copy" nodes
            {
                SwNode& rNode = rLastIdx.GetNode();
                if( ( rNode.IsSectionNode() && rNode.EndOfSectionIndex() >= nEnd )
                    || ( rNode.IsEndNode() && rNode.StartOfSectionNode()->GetIndex() < nStart ) )
                {
                    ++rDelCount;
                }
                ++rLastIdx;
            }
            while( rLastIdx.GetIndex() < nNewIdx );
        }
        else if( rDelCount ) // optimization: if there are no "non-copy" nodes until now,
                             // no move backward needed
        {
            while( rLastIdx.GetIndex() > nNewIdx )
            {
                SwNode& rNode = rLastIdx.GetNode();
                if( ( rNode.IsSectionNode() && rNode.EndOfSectionIndex() >= nEnd )
                    || ( rNode.IsEndNode() && rNode.StartOfSectionNode()->GetIndex() < nStart ) )
                {
                    --rDelCount;
                }
                rLastIdx--;
            }
        }
    }

    void lcl_SetCpyPos( const SwPosition& rOrigPos,
                        const SwPosition& rOrigStt,
                        const SwPosition& rCpyStt,
                        SwPosition& rChgPos,
                        sal_uLong nDelCount )
    {
        sal_uLong nNdOff = rOrigPos.nNode.GetIndex();
        nNdOff -= rOrigStt.nNode.GetIndex();
        nNdOff -= nDelCount;
        sal_Int32 nContentPos = rOrigPos.nContent.GetIndex();

        // Always adjust <nNode> at to be changed <SwPosition> instance <rChgPos>
        rChgPos.nNode = nNdOff + rCpyStt.nNode.GetIndex();
        if( !nNdOff )
        {
            // just adapt the content index
            if( nContentPos > rOrigStt.nContent.GetIndex() )
                nContentPos -= rOrigStt.nContent.GetIndex();
            else
                nContentPos = 0;
            nContentPos += rCpyStt.nContent.GetIndex();
        }
        rChgPos.nContent.Assign( rChgPos.nNode.GetNode().GetContentNode(), nContentPos );
    }

}

namespace sw
{
    // TODO: use SaveBookmark (from DelBookmarks)
    void CopyBookmarks(const SwPaM& rPam, SwPosition& rCpyPam)
    {
        const SwDoc& rSrcDoc = rPam.GetDoc();
        SwDoc& rDestDoc =  rCpyPam.GetDoc();
        const IDocumentMarkAccess* const pSrcMarkAccess = rSrcDoc.getIDocumentMarkAccess();
        ::sw::UndoGuard const undoGuard(rDestDoc.GetIDocumentUndoRedo());

        const SwPosition &rStt = *rPam.Start(), &rEnd = *rPam.End();
        SwPosition const*const pCpyStt = &rCpyPam;

        std::vector< const ::sw::mark::IMark* > vMarksToCopy;
        for ( IDocumentMarkAccess::const_iterator_t ppMark = pSrcMarkAccess->getAllMarksBegin();
              ppMark != pSrcMarkAccess->getAllMarksEnd();
              ++ppMark )
        {
            const ::sw::mark::IMark* const pMark = *ppMark;

            const SwPosition& rMarkStart = pMark->GetMarkStart();
            const SwPosition& rMarkEnd = pMark->GetMarkEnd();
            // only include marks that are in the range and not touching both start and end
            // - not for annotation or checkbox marks.
            bool const isIncludeStart(
                   (rStt.nContent.GetIndex() == 0 // paragraph start selected?
                    // also: only if inserting at the start - cross reference
                    // marks require index to be 0, and there could be one
                    // on the target node already
                    && rCpyPam.nContent.GetIndex() == 0)
                || rMarkStart != rStt);
            bool const isIncludeEnd(
                   (rEnd.nNode.GetNode().IsTextNode() // paragraph end selected?
                    && rEnd.nContent.GetIndex() == rEnd.nNode.GetNode().GetTextNode()->Len())
                || rMarkEnd != rEnd);
            const bool bIsNotOnBoundary =
                pMark->IsExpanded()
                ? (isIncludeStart || isIncludeEnd)  // rMarkStart != rMarkEnd
                : (isIncludeStart && isIncludeEnd); // rMarkStart == rMarkEnd
            const IDocumentMarkAccess::MarkType aMarkType = IDocumentMarkAccess::GetType(*pMark);
            if ( rMarkStart >= rStt && rMarkEnd <= rEnd
                 && ( bIsNotOnBoundary
                      || aMarkType == IDocumentMarkAccess::MarkType::ANNOTATIONMARK
                      || aMarkType == IDocumentMarkAccess::MarkType::TEXT_FIELDMARK
                      || aMarkType == IDocumentMarkAccess::MarkType::CHECKBOX_FIELDMARK
                      || aMarkType == IDocumentMarkAccess::MarkType::DROPDOWN_FIELDMARK
                      || aMarkType == IDocumentMarkAccess::MarkType::DATE_FIELDMARK))
            {
                vMarksToCopy.push_back(pMark);
            }
        }
        // We have to count the "non-copied" nodes...
        sal_uLong nDelCount;
        SwNodeIndex aCorrIdx(InitDelCount(rPam, nDelCount));
        for(const sw::mark::IMark* const pMark : vMarksToCopy)
        {
            SwPaM aTmpPam(*pCpyStt);
            lcl_NonCopyCount(rPam, aCorrIdx, pMark->GetMarkPos().nNode.GetIndex(), nDelCount);
            lcl_SetCpyPos( pMark->GetMarkPos(), rStt, *pCpyStt, *aTmpPam.GetPoint(), nDelCount);
            if(pMark->IsExpanded())
            {
                aTmpPam.SetMark();
                lcl_NonCopyCount(rPam, aCorrIdx, pMark->GetOtherMarkPos().nNode.GetIndex(), nDelCount);
                lcl_SetCpyPos(pMark->GetOtherMarkPos(), rStt, *pCpyStt, *aTmpPam.GetMark(), nDelCount);
            }

            ::sw::mark::IMark* const pNewMark = rDestDoc.getIDocumentMarkAccess()->makeMark(
                aTmpPam,
                pMark->GetName(),
                IDocumentMarkAccess::GetType(*pMark),
                ::sw::mark::InsertMode::CopyText);
            // Explicitly try to get exactly the same name as in the source
            // because NavigatorReminders, DdeBookmarks etc. ignore the proposed name
            if (pNewMark == nullptr)
            {
                assert(IDocumentMarkAccess::GetType(*pMark) == IDocumentMarkAccess::MarkType::CROSSREF_NUMITEM_BOOKMARK
                    || IDocumentMarkAccess::GetType(*pMark) == IDocumentMarkAccess::MarkType::CROSSREF_HEADING_BOOKMARK);
                continue; // can't insert duplicate cross reference mark
            }
            rDestDoc.getIDocumentMarkAccess()->renameMark(pNewMark, pMark->GetName());

            // copying additional attributes for bookmarks or fieldmarks
            ::sw::mark::IBookmark* const pNewBookmark =
                dynamic_cast< ::sw::mark::IBookmark* const >(pNewMark);
            const ::sw::mark::IBookmark* const pOldBookmark =
                dynamic_cast< const ::sw::mark::IBookmark* >(pMark);
            if (pNewBookmark && pOldBookmark)
            {
                pNewBookmark->SetKeyCode(pOldBookmark->GetKeyCode());
                pNewBookmark->SetShortName(pOldBookmark->GetShortName());
                pNewBookmark->Hide(pOldBookmark->IsHidden());
                pNewBookmark->SetHideCondition(pOldBookmark->GetHideCondition());
            }
            ::sw::mark::IFieldmark* const pNewFieldmark =
                dynamic_cast< ::sw::mark::IFieldmark* const >(pNewMark);
            const ::sw::mark::IFieldmark* const pOldFieldmark =
                dynamic_cast< const ::sw::mark::IFieldmark* >(pMark);
            if (pNewFieldmark && pOldFieldmark)
            {
                pNewFieldmark->SetFieldname(pOldFieldmark->GetFieldname());
                pNewFieldmark->SetFieldHelptext(pOldFieldmark->GetFieldHelptext());
                ::sw::mark::IFieldmark::parameter_map_t* pNewParams = pNewFieldmark->GetParameters();
                const ::sw::mark::IFieldmark::parameter_map_t* pOldParams = pOldFieldmark->GetParameters();
                for (const auto& rEntry : *pOldParams )
                {
                    pNewParams->insert( rEntry );
                }
            }

            ::sfx2::Metadatable const*const pMetadatable(
                    dynamic_cast< ::sfx2::Metadatable const* >(pMark));
            ::sfx2::Metadatable      *const pNewMetadatable(
                    dynamic_cast< ::sfx2::Metadatable      * >(pNewMark));
            if (pMetadatable && pNewMetadatable)
            {
                pNewMetadatable->RegisterAsCopyOf(*pMetadatable);
            }
        }
    }
} // namespace sw

namespace
{
    void lcl_DeleteRedlines( const SwPaM& rPam, SwPaM& rCpyPam )
    {
        const SwDoc& rSrcDoc = rPam.GetDoc();
        const SwRedlineTable& rTable = rSrcDoc.getIDocumentRedlineAccess().GetRedlineTable();
        if( rTable.empty() )
            return;

        SwDoc& rDestDoc = rCpyPam.GetDoc();
        SwPosition* pCpyStt = rCpyPam.Start(), *pCpyEnd = rCpyPam.End();
        std::unique_ptr<SwPaM> pDelPam;
        const SwPosition *pStt = rPam.Start(), *pEnd = rPam.End();
        // We have to count the "non-copied" nodes
        sal_uLong nDelCount;
        SwNodeIndex aCorrIdx(InitDelCount(rPam, nDelCount));

        SwRedlineTable::size_type n = 0;
        rSrcDoc.getIDocumentRedlineAccess().GetRedline( *pStt, &n );
        for( ; n < rTable.size(); ++n )
        {
            const SwRangeRedline* pRedl = rTable[ n ];
            if( RedlineType::Delete == pRedl->GetType() && pRedl->IsVisible() )
            {
                const SwPosition *pRStt = pRedl->Start(), *pREnd = pRedl->End();

                SwComparePosition eCmpPos = ComparePosition( *pStt, *pEnd, *pRStt, *pREnd );
                switch( eCmpPos )
                {
                case SwComparePosition::CollideEnd:
                case SwComparePosition::Before:
                    // Pos1 is before Pos2
                    break;

                case SwComparePosition::CollideStart:
                case SwComparePosition::Behind:
                    // Pos1 is after Pos2
                    n = rTable.size();
                    break;

                default:
                    {
                        pDelPam.reset(new SwPaM( *pCpyStt, pDelPam.release() ));
                        if( *pStt < *pRStt )
                        {
                            lcl_NonCopyCount( rPam, aCorrIdx, pRStt->nNode.GetIndex(), nDelCount );
                            lcl_SetCpyPos( *pRStt, *pStt, *pCpyStt,
                                            *pDelPam->GetPoint(), nDelCount );
                        }
                        pDelPam->SetMark();

                        if( *pEnd < *pREnd )
                            *pDelPam->GetPoint() = *pCpyEnd;
                        else
                        {
                            lcl_NonCopyCount( rPam, aCorrIdx, pREnd->nNode.GetIndex(), nDelCount );
                            lcl_SetCpyPos( *pREnd, *pStt, *pCpyStt,
                                            *pDelPam->GetPoint(), nDelCount );
                        }

                        if (pDelPam->GetNext() != pDelPam.get()
                            && *pDelPam->GetNext()->End() == *pDelPam->Start())
                        {
                            *pDelPam->GetNext()->End() = *pDelPam->End();
                            pDelPam.reset(pDelPam->GetNext());
                        }
                    }
                }
            }
        }

        if( !pDelPam )
            return;

        RedlineFlags eOld = rDestDoc.getIDocumentRedlineAccess().GetRedlineFlags();
        rDestDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld | RedlineFlags::Ignore );

        ::sw::UndoGuard const undoGuard(rDestDoc.GetIDocumentUndoRedo());

        do {
            rDestDoc.getIDocumentContentOperations().DeleteAndJoin( *pDelPam->GetNext() );
            if( !pDelPam->IsMultiSelection() )
                break;
            delete pDelPam->GetNext();
        } while( true );

        rDestDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
    }

    void lcl_DeleteRedlines( const SwNodeRange& rRg, SwNodeRange const & rCpyRg )
    {
        SwDoc& rSrcDoc = rRg.aStart.GetNode().GetDoc();
        if( !rSrcDoc.getIDocumentRedlineAccess().GetRedlineTable().empty() )
        {
            SwPaM aRgTmp( rRg.aStart, rRg.aEnd );
            SwPaM aCpyTmp( rCpyRg.aStart, rCpyRg.aEnd );
            lcl_DeleteRedlines( aRgTmp, aCpyTmp );
        }
    }

    void lcl_ChainFormats( SwFlyFrameFormat *pSrc, SwFlyFrameFormat *pDest )
    {
        SwFormatChain aSrc( pSrc->GetChain() );
        if ( !aSrc.GetNext() )
        {
            aSrc.SetNext( pDest );
            pSrc->SetFormatAttr( aSrc );
        }
        SwFormatChain aDest( pDest->GetChain() );
        if ( !aDest.GetPrev() )
        {
            aDest.SetPrev( pSrc );
            pDest->SetFormatAttr( aDest );
        }
    }

    // #i86492#
    bool lcl_ContainsOnlyParagraphsInList( const SwPaM& rPam )
    {
        bool bRet = false;

        const SwTextNode* pTextNd = rPam.Start()->nNode.GetNode().GetTextNode();
        const SwTextNode* pEndTextNd = rPam.End()->nNode.GetNode().GetTextNode();
        if ( pTextNd && pTextNd->IsInList() &&
             pEndTextNd && pEndTextNd->IsInList() )
        {
            bRet = true;
            SwNodeIndex aIdx(rPam.Start()->nNode);

            do
            {
                ++aIdx;
                pTextNd = aIdx.GetNode().GetTextNode();

                if ( !pTextNd || !pTextNd->IsInList() )
                {
                    bRet = false;
                    break;
                }
            } while (pTextNd != pEndTextNd);
        }

        return bRet;
    }

    bool lcl_MarksWholeNode(const SwPaM & rPam)
    {
        bool bResult = false;
        const SwPosition* pStt = rPam.Start();
        const SwPosition* pEnd = rPam.End();

        if (nullptr != pStt && nullptr != pEnd)
        {
            const SwTextNode* pSttNd = pStt->nNode.GetNode().GetTextNode();
            const SwTextNode* pEndNd = pEnd->nNode.GetNode().GetTextNode();

            if (nullptr != pSttNd && nullptr != pEndNd &&
                pStt->nContent.GetIndex() == 0 &&
                pEnd->nContent.GetIndex() == pEndNd->Len())
            {
                bResult = true;
            }
        }

        return bResult;
    }
}

//local functions originally from sw/source/core/doc/docedt.cxx
namespace sw
{
    void CalcBreaks(std::vector<std::pair<sal_uLong, sal_Int32>> & rBreaks,
            SwPaM const & rPam, bool const isOnlyFieldmarks)
    {
        sal_uLong const nStartNode(rPam.Start()->nNode.GetIndex());
        sal_uLong const nEndNode(rPam.End()->nNode.GetIndex());
        SwNodes const& rNodes(rPam.GetPoint()->nNode.GetNodes());
        IDocumentMarkAccess const& rIDMA(*rPam.GetDoc().getIDocumentMarkAccess());

        std::stack<std::tuple<sw::mark::IFieldmark const*, bool, sal_uLong, sal_Int32>> startedFields;

        for (sal_uLong n = nStartNode; n <= nEndNode; ++n)
        {
            SwNode *const pNode(rNodes[n]);
            if (pNode->IsTextNode())
            {
                SwTextNode & rTextNode(*pNode->GetTextNode());
                sal_Int32 const nStart(n == nStartNode
                        ? rPam.Start()->nContent.GetIndex()
                        : 0);
                sal_Int32 const nEnd(n == nEndNode
                        ? rPam.End()->nContent.GetIndex()
                        : rTextNode.Len());
                for (sal_Int32 i = nStart; i < nEnd; ++i)
                {
                    const sal_Unicode c(rTextNode.GetText()[i]);
                    switch (c)
                    {
                        // note: CH_TXT_ATR_FORMELEMENT does not need handling
                        // not sure how CH_TXT_ATR_INPUTFIELDSTART/END are currently handled
                        case CH_TXTATR_INWORD:
                        case CH_TXTATR_BREAKWORD:
                        {
                            // META hints only have dummy char at the start, not
                            // at the end, so no need to check in nStartNode
                            if (n == nEndNode && !isOnlyFieldmarks)
                            {
                                SwTextAttr const*const pAttr(rTextNode.GetTextAttrForCharAt(i));
                                if (pAttr && pAttr->End() && (nEnd  < *pAttr->End()))
                                {
                                    assert(pAttr->HasDummyChar());
                                    rBreaks.emplace_back(n, i);
                                }
                            }
                            break;
                        }
                        case CH_TXT_ATR_FIELDSTART:
                        {
                            auto const pFieldMark(rIDMA.getFieldmarkAt(SwPosition(rTextNode, i)));
                            startedFields.emplace(pFieldMark, false, 0, 0);
                            break;
                        }
                        case CH_TXT_ATR_FIELDSEP:
                        {
                            if (startedFields.empty())
                            {
                                rBreaks.emplace_back(n, i);
                            }
                            else
                            {   // no way to find the field via MarkManager...
                                assert(std::get<0>(startedFields.top())->IsCoveringPosition(SwPosition(rTextNode, i)));
                                std::get<1>(startedFields.top()) = true;
                                std::get<2>(startedFields.top()) = n;
                                std::get<3>(startedFields.top()) = i;
                            }
                            break;
                        }
                        case CH_TXT_ATR_FIELDEND:
                        {
                            if (startedFields.empty())
                            {
                                rBreaks.emplace_back(n, i);
                            }
                            else
                            {   // fieldmarks must not overlap => stack
                                assert(std::get<0>(startedFields.top()) == rIDMA.getFieldmarkAt(SwPosition(rTextNode, i)));
                                startedFields.pop();
                            }
                            break;
                        }
                    }
                }
            }
            else if (pNode->IsStartNode())
            {
                if (pNode->EndOfSectionIndex() <= nEndNode)
                {   // fieldmark cannot overlap node section
                    n = pNode->EndOfSectionIndex();
                }
            }
            else
            {   // EndNode can actually happen with sections :(
                assert(pNode->IsEndNode() || pNode->IsNoTextNode());
            }
        }
        while (!startedFields.empty())
        {
            SwPosition const& rStart(std::get<0>(startedFields.top())->GetMarkStart());
            std::pair<sal_uLong, sal_Int32> const pos(
                    rStart.nNode.GetIndex(), rStart.nContent.GetIndex());
            auto it = std::lower_bound(rBreaks.begin(), rBreaks.end(), pos);
            assert(it == rBreaks.end() || *it != pos);
            rBreaks.insert(it, pos);
            if (std::get<1>(startedFields.top()))
            {
                std::pair<sal_uLong, sal_Int32> const posSep(
                    std::get<2>(startedFields.top()),
                    std::get<3>(startedFields.top()));
                it = std::lower_bound(rBreaks.begin(), rBreaks.end(), posSep);
                assert(it == rBreaks.end() || *it != posSep);
                rBreaks.insert(it, posSep);
            }
            startedFields.pop();
        }
    }
}

namespace
{

    bool lcl_DoWithBreaks(::sw::DocumentContentOperationsManager & rDocumentContentOperations, SwPaM & rPam,
            bool (::sw::DocumentContentOperationsManager::*pFunc)(SwPaM&, bool), const bool bForceJoinNext = false)
    {
        std::vector<std::pair<sal_uLong, sal_Int32>> Breaks;

        sw::CalcBreaks(Breaks, rPam);

        if (Breaks.empty())
        {
            return (rDocumentContentOperations.*pFunc)(rPam, bForceJoinNext);
        }

        // Deletion must be split into several parts if the text node
        // contains a text attribute with end and with dummy character
        // and the selection does not contain the text attribute completely,
        // but overlaps its start (left), where the dummy character is.

        SwPosition const & rSelectionEnd( *rPam.End() );

        bool bRet( true );
        // iterate from end to start, to avoid invalidating the offsets!
        auto iter( Breaks.rbegin() );
        sal_uLong nOffset(0);
        SwNodes const& rNodes(rPam.GetPoint()->nNode.GetNodes());
        SwPaM aPam( rSelectionEnd, rSelectionEnd ); // end node!
        SwPosition & rEnd( *aPam.End() );
        SwPosition & rStart( *aPam.Start() );

        while (iter != Breaks.rend())
        {
            rStart = SwPosition(*rNodes[iter->first - nOffset]->GetTextNode(), iter->second + 1);
            if (rStart < rEnd) // check if part is empty
            {
                bRet &= (rDocumentContentOperations.*pFunc)(aPam, bForceJoinNext);
                nOffset = iter->first - rStart.nNode.GetIndex(); // deleted fly nodes...
            }
            rEnd = SwPosition(*rNodes[iter->first - nOffset]->GetTextNode(), iter->second);
            ++iter;
        }

        rStart = *rPam.Start(); // set to original start
        if (rStart < rEnd) // check if part is empty
        {
            bRet &= (rDocumentContentOperations.*pFunc)(aPam, bForceJoinNext);
        }

        return bRet;
    }

    bool lcl_StrLenOverflow( const SwPaM& rPam )
    {
        // If we try to merge two paragraphs we have to test if afterwards
        // the string doesn't exceed the allowed string length
        if( rPam.GetPoint()->nNode != rPam.GetMark()->nNode )
        {
            const SwPosition* pStt = rPam.Start(), *pEnd = rPam.End();
            const SwTextNode* pEndNd = pEnd->nNode.GetNode().GetTextNode();
            if( (nullptr != pEndNd) && pStt->nNode.GetNode().IsTextNode() )
            {
                const sal_uInt64 nSum = pStt->nContent.GetIndex() +
                    pEndNd->GetText().getLength() - pEnd->nContent.GetIndex();
                return nSum > o3tl::make_unsigned(SAL_MAX_INT32);
            }
        }
        return false;
    }

    struct SaveRedline
    {
        SwRangeRedline* pRedl;
        sal_uInt32 nStt, nEnd;
        sal_Int32 nSttCnt;
        sal_Int32 nEndCnt;

        SaveRedline( SwRangeRedline* pR, const SwNodeIndex& rSttIdx )
            : pRedl(pR)
            , nEnd(0)
            , nEndCnt(0)
        {
            const SwPosition* pStt = pR->Start(),
                * pEnd = pR->GetMark() == pStt ? pR->GetPoint() : pR->GetMark();
            sal_uInt32 nSttIdx = rSttIdx.GetIndex();
            nStt = pStt->nNode.GetIndex() - nSttIdx;
            nSttCnt = pStt->nContent.GetIndex();
            if( pR->HasMark() )
            {
                nEnd = pEnd->nNode.GetIndex() - nSttIdx;
                nEndCnt = pEnd->nContent.GetIndex();
            }

            pRedl->GetPoint()->nNode = 0;
            pRedl->GetPoint()->nContent.Assign( nullptr, 0 );
            pRedl->GetMark()->nNode = 0;
            pRedl->GetMark()->nContent.Assign( nullptr, 0 );
        }

        SaveRedline( SwRangeRedline* pR, const SwPosition& rPos )
            : pRedl(pR)
            , nEnd(0)
            , nEndCnt(0)
        {
            const SwPosition* pStt = pR->Start(),
                * pEnd = pR->GetMark() == pStt ? pR->GetPoint() : pR->GetMark();
            sal_uInt32 nSttIdx = rPos.nNode.GetIndex();
            nStt = pStt->nNode.GetIndex() - nSttIdx;
            nSttCnt = pStt->nContent.GetIndex();
            if( nStt == 0 )
                nSttCnt = nSttCnt - rPos.nContent.GetIndex();
            if( pR->HasMark() )
            {
                nEnd = pEnd->nNode.GetIndex() - nSttIdx;
                nEndCnt = pEnd->nContent.GetIndex();
                if( nEnd == 0 )
                    nEndCnt = nEndCnt - rPos.nContent.GetIndex();
            }

            pRedl->GetPoint()->nNode = 0;
            pRedl->GetPoint()->nContent.Assign( nullptr, 0 );
            pRedl->GetMark()->nNode = 0;
            pRedl->GetMark()->nContent.Assign( nullptr, 0 );
        }

        void SetPos( sal_uInt32 nInsPos )
        {
            pRedl->GetPoint()->nNode = nInsPos + nStt;
            pRedl->GetPoint()->nContent.Assign( pRedl->GetContentNode(), nSttCnt );
            if( pRedl->HasMark() )
            {
                pRedl->GetMark()->nNode = nInsPos + nEnd;
                pRedl->GetMark()->nContent.Assign( pRedl->GetContentNode(false), nEndCnt );
            }
        }

        void SetPos( const SwPosition& aPos )
        {
            pRedl->GetPoint()->nNode = aPos.nNode.GetIndex() + nStt;
            pRedl->GetPoint()->nContent.Assign( pRedl->GetContentNode(), nSttCnt + ( nStt == 0 ? aPos.nContent.GetIndex() : 0 ) );
            if( pRedl->HasMark() )
            {
                pRedl->GetMark()->nNode = aPos.nNode.GetIndex() + nEnd;
                pRedl->GetMark()->nContent.Assign( pRedl->GetContentNode(false), nEndCnt  + ( nEnd == 0 ? aPos.nContent.GetIndex() : 0 ) );
            }
        }
    };

    typedef std::vector< SaveRedline > SaveRedlines_t;

    void lcl_SaveRedlines(const SwPaM& aPam, SaveRedlines_t& rArr)
    {
        SwDoc& rDoc = aPam.GetNode().GetDoc();

        const SwPosition* pStart = aPam.Start();
        const SwPosition* pEnd = aPam.End();

        // get first relevant redline
        SwRedlineTable::size_type nCurrentRedline;
        rDoc.getIDocumentRedlineAccess().GetRedline( *pStart, &nCurrentRedline );
        if( nCurrentRedline > 0)
            nCurrentRedline--;

        // redline mode RedlineFlags::Ignore|RedlineFlags::On; save old mode
        RedlineFlags eOld = rDoc.getIDocumentRedlineAccess().GetRedlineFlags();
        rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( ( eOld & ~RedlineFlags::Ignore) | RedlineFlags::On );

        // iterate over relevant redlines and decide for each whether it should
        // be saved, or split + saved
        SwRedlineTable& rRedlineTable = rDoc.getIDocumentRedlineAccess().GetRedlineTable();
        for( ; nCurrentRedline < rRedlineTable.size(); nCurrentRedline++ )
        {
            SwRangeRedline* pCurrent = rRedlineTable[ nCurrentRedline ];
            SwComparePosition eCompare =
                ComparePosition( *pCurrent->Start(), *pCurrent->End(),
                                 *pStart, *pEnd);

            // we must save this redline if it overlaps aPam
            // (we may have to split it, too)
            if( eCompare == SwComparePosition::OverlapBehind  ||
                eCompare == SwComparePosition::OverlapBefore  ||
                eCompare == SwComparePosition::Outside ||
                eCompare == SwComparePosition::Inside ||
                eCompare == SwComparePosition::Equal )
            {
                rRedlineTable.Remove( nCurrentRedline-- );

                // split beginning, if necessary
                if( eCompare == SwComparePosition::OverlapBefore  ||
                    eCompare == SwComparePosition::Outside )
                {
                    SwRangeRedline* pNewRedline = new SwRangeRedline( *pCurrent );
                    *pNewRedline->End() = *pStart;
                    *pCurrent->Start() = *pStart;
                    rDoc.getIDocumentRedlineAccess().AppendRedline( pNewRedline, true );
                }

                // split end, if necessary
                if( eCompare == SwComparePosition::OverlapBehind  ||
                    eCompare == SwComparePosition::Outside )
                {
                    SwRangeRedline* pNewRedline = new SwRangeRedline( *pCurrent );
                    *pNewRedline->Start() = *pEnd;
                    *pCurrent->End() = *pEnd;
                    rDoc.getIDocumentRedlineAccess().AppendRedline( pNewRedline, true );
                }

                // save the current redline
                rArr.emplace_back( pCurrent, *pStart );
            }
        }

        // restore old redline mode
        rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
    }

    void lcl_RestoreRedlines(SwDoc& rDoc, const SwPosition& rPos, SaveRedlines_t& rArr)
    {
        RedlineFlags eOld = rDoc.getIDocumentRedlineAccess().GetRedlineFlags();
        rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( ( eOld & ~RedlineFlags::Ignore) | RedlineFlags::On );

        for(SaveRedline & rSvRedLine : rArr)
        {
            rSvRedLine.SetPos( rPos );
            rDoc.getIDocumentRedlineAccess().AppendRedline( rSvRedLine.pRedl, true );
        }

        rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
    }

    void lcl_SaveRedlines(const SwNodeRange& rRg, SaveRedlines_t& rArr)
    {
        SwDoc& rDoc = rRg.aStart.GetNode().GetDoc();
        SwRedlineTable::size_type nRedlPos;
        SwPosition aSrchPos( rRg.aStart ); aSrchPos.nNode--;
        aSrchPos.nContent.Assign( aSrchPos.nNode.GetNode().GetContentNode(), 0 );
        if( rDoc.getIDocumentRedlineAccess().GetRedline( aSrchPos, &nRedlPos ) && nRedlPos )
            --nRedlPos;
        else if( nRedlPos >= rDoc.getIDocumentRedlineAccess().GetRedlineTable().size() )
            return ;

        RedlineFlags eOld = rDoc.getIDocumentRedlineAccess().GetRedlineFlags();
        rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( ( eOld & ~RedlineFlags::Ignore) | RedlineFlags::On );
        SwRedlineTable& rRedlTable = rDoc.getIDocumentRedlineAccess().GetRedlineTable();

        do {
            SwRangeRedline* pTmp = rRedlTable[ nRedlPos ];

            const SwPosition* pRStt = pTmp->Start(),
                            * pREnd = pTmp->GetMark() == pRStt
                                ? pTmp->GetPoint() : pTmp->GetMark();

            if( pRStt->nNode < rRg.aStart )
            {
                if( pREnd->nNode > rRg.aStart && pREnd->nNode < rRg.aEnd )
                {
                    // Create a copy and set the end of the original to the end of the MoveArea.
                    // The copy is moved too.
                    SwRangeRedline* pNewRedl = new SwRangeRedline( *pTmp );
                    SwPosition* pTmpPos = pNewRedl->Start();
                    pTmpPos->nNode = rRg.aStart;
                    pTmpPos->nContent.Assign(
                                pTmpPos->nNode.GetNode().GetContentNode(), 0 );

                    rArr.emplace_back(pNewRedl, rRg.aStart);

                    pTmpPos = pTmp->End();
                    pTmpPos->nNode = rRg.aEnd;
                    pTmpPos->nContent.Assign(
                                pTmpPos->nNode.GetNode().GetContentNode(), 0 );
                }
                else if( pREnd->nNode == rRg.aStart )
                {
                    SwPosition* pTmpPos = pTmp->End();
                    pTmpPos->nNode = rRg.aEnd;
                    pTmpPos->nContent.Assign(
                                pTmpPos->nNode.GetNode().GetContentNode(), 0 );
                }
            }
            else if( pRStt->nNode < rRg.aEnd )
            {
                rRedlTable.Remove( nRedlPos-- );
                if( pREnd->nNode < rRg.aEnd ||
                    ( pREnd->nNode == rRg.aEnd && !pREnd->nContent.GetIndex()) )
                {
                    // move everything
                    rArr.emplace_back( pTmp, rRg.aStart );
                }
                else
                {
                    // split
                    SwRangeRedline* pNewRedl = new SwRangeRedline( *pTmp );
                    SwPosition* pTmpPos = pNewRedl->End();
                    pTmpPos->nNode = rRg.aEnd;
                    pTmpPos->nContent.Assign(
                                pTmpPos->nNode.GetNode().GetContentNode(), 0 );

                    rArr.emplace_back( pNewRedl, rRg.aStart );

                    pTmpPos = pTmp->Start();
                    pTmpPos->nNode = rRg.aEnd;
                    pTmpPos->nContent.Assign(
                                pTmpPos->nNode.GetNode().GetContentNode(), 0 );
                    rDoc.getIDocumentRedlineAccess().AppendRedline( pTmp, true );
                }
            }
            else
                break;

        } while( ++nRedlPos < rDoc.getIDocumentRedlineAccess().GetRedlineTable().size() );
        rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
    }

    void lcl_RestoreRedlines(SwDoc& rDoc, sal_uInt32 const nInsPos, SaveRedlines_t& rArr)
    {
        RedlineFlags eOld = rDoc.getIDocumentRedlineAccess().GetRedlineFlags();
        rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( ( eOld & ~RedlineFlags::Ignore) | RedlineFlags::On );

        for(SaveRedline & rSvRedLine : rArr)
        {
            rSvRedLine.SetPos( nInsPos );
            IDocumentRedlineAccess::AppendResult const result(
                rDoc.getIDocumentRedlineAccess().AppendRedline( rSvRedLine.pRedl, true ));
            if ( IDocumentRedlineAccess::AppendResult::APPENDED == result &&
                rSvRedLine.pRedl->GetType() == RedlineType::Delete )
            {
                UpdateFramesForAddDeleteRedline(rDoc, *rSvRedLine.pRedl);
            }
        }

        rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
    }

    bool lcl_SaveFootnote( const SwNodeIndex& rSttNd, const SwNodeIndex& rEndNd,
                     const SwNodeIndex& rInsPos,
                     SwFootnoteIdxs& rFootnoteArr, SwFootnoteIdxs& rSaveArr,
                     const SwIndex* pSttCnt = nullptr, const SwIndex* pEndCnt = nullptr )
    {
        bool bUpdateFootnote = false;
        const SwNodes& rNds = rInsPos.GetNodes();
        const bool bDelFootnote = rInsPos.GetIndex() < rNds.GetEndOfAutotext().GetIndex() &&
                    rSttNd.GetIndex() >= rNds.GetEndOfAutotext().GetIndex();
        const bool bSaveFootnote = !bDelFootnote &&
                        rInsPos.GetIndex() >= rNds.GetEndOfExtras().GetIndex();
        if( !rFootnoteArr.empty() )
        {

            size_t nPos = 0;
            rFootnoteArr.SeekEntry( rSttNd, &nPos );
            SwTextFootnote* pSrch;
            const SwNode* pFootnoteNd;

            // Delete/save all that come after it
            while( nPos < rFootnoteArr.size() && ( pFootnoteNd =
                &( pSrch = rFootnoteArr[ nPos ] )->GetTextNode())->GetIndex()
                        <= rEndNd.GetIndex() )
            {
                const sal_Int32 nFootnoteSttIdx = pSrch->GetStart();
                if( ( pEndCnt && pSttCnt )
                    ? (( &rSttNd.GetNode() == pFootnoteNd &&
                         pSttCnt->GetIndex() > nFootnoteSttIdx) ||
                       ( &rEndNd.GetNode() == pFootnoteNd &&
                        nFootnoteSttIdx >= pEndCnt->GetIndex() ))
                    : ( &rEndNd.GetNode() == pFootnoteNd ))
                {
                    ++nPos;     // continue searching
                }
                else
                {
                    // delete it
                    if( bDelFootnote )
                    {
                        SwTextNode& rTextNd = const_cast<SwTextNode&>(pSrch->GetTextNode());
                        SwIndex aIdx( &rTextNd, nFootnoteSttIdx );
                        rTextNd.EraseText( aIdx, 1 );
                    }
                    else
                    {
                        pSrch->DelFrames(nullptr);
                        rFootnoteArr.erase( rFootnoteArr.begin() + nPos );
                        if( bSaveFootnote )
                            rSaveArr.insert( pSrch );
                    }
                    bUpdateFootnote = true;
                }
            }

            while( nPos-- && ( pFootnoteNd = &( pSrch = rFootnoteArr[ nPos ] )->
                    GetTextNode())->GetIndex() >= rSttNd.GetIndex() )
            {
                const sal_Int32 nFootnoteSttIdx = pSrch->GetStart();
                if( !pEndCnt || !pSttCnt ||
                    !  (( &rSttNd.GetNode() == pFootnoteNd &&
                        pSttCnt->GetIndex() > nFootnoteSttIdx ) ||
                        ( &rEndNd.GetNode() == pFootnoteNd &&
                        nFootnoteSttIdx >= pEndCnt->GetIndex() )) )
                {
                    if( bDelFootnote )
                    {
                        // delete it
                        SwTextNode& rTextNd = const_cast<SwTextNode&>(pSrch->GetTextNode());
                        SwIndex aIdx( &rTextNd, nFootnoteSttIdx );
                        rTextNd.EraseText( aIdx, 1 );
                    }
                    else
                    {
                        pSrch->DelFrames(nullptr);
                        rFootnoteArr.erase( rFootnoteArr.begin() + nPos );
                        if( bSaveFootnote )
                            rSaveArr.insert( pSrch );
                    }
                    bUpdateFootnote = true;
                }
            }
        }
        // When moving from redline section into document content section, e.g.
        // after loading a document with (delete-)redlines, the footnote array
        // has to be adjusted... (#i70572)
        if( bSaveFootnote )
        {
            SwNodeIndex aIdx( rSttNd );
            while( aIdx < rEndNd ) // Check the moved section
            {
                SwNode* pNode = &aIdx.GetNode();
                if( pNode->IsTextNode() ) // Looking for text nodes...
                {
                    SwpHints *pHints = pNode->GetTextNode()->GetpSwpHints();
                    if( pHints && pHints->HasFootnote() ) //...with footnotes
                    {
                        bUpdateFootnote = true; // Heureka
                        const size_t nCount = pHints->Count();
                        for( size_t i = 0; i < nCount; ++i )
                        {
                            SwTextAttr *pAttr = pHints->Get( i );
                            if ( pAttr->Which() == RES_TXTATR_FTN )
                            {
                                rSaveArr.insert( static_cast<SwTextFootnote*>(pAttr) );
                            }
                        }
                    }
                }
                ++aIdx;
            }
        }
        return bUpdateFootnote;
    }

    bool lcl_MayOverwrite( const SwTextNode *pNode, const sal_Int32 nPos )
    {
        sal_Unicode const cChr = pNode->GetText()[nPos];
        switch (cChr)
        {
            case CH_TXTATR_BREAKWORD:
            case CH_TXTATR_INWORD:
                return !pNode->GetTextAttrForCharAt(nPos);// how could there be none?
            case CH_TXT_ATR_INPUTFIELDSTART:
            case CH_TXT_ATR_INPUTFIELDEND:
            case CH_TXT_ATR_FIELDSTART:
            case CH_TXT_ATR_FIELDSEP:
            case CH_TXT_ATR_FIELDEND:
            case CH_TXT_ATR_FORMELEMENT:
                return false;
            default:
                return true;
        }
    }

    void lcl_SkipAttr( const SwTextNode *pNode, SwIndex &rIdx, sal_Int32 &rStart )
    {
        if( !lcl_MayOverwrite( pNode, rStart ) )
        {
            // skip all special attributes
            do {
                ++rIdx;
                rStart = rIdx.GetIndex();
            } while (rStart < pNode->GetText().getLength()
                   && !lcl_MayOverwrite(pNode, rStart) );
        }
    }

    bool lcl_GetTokenToParaBreak( OUString& rStr, OUString& rRet, bool bRegExpRplc )
    {
        if( bRegExpRplc )
        {
            sal_Int32 nPos = 0;
            static const OUStringLiteral sPara(u"\\n");
            for (;;)
            {
                nPos = rStr.indexOf( sPara, nPos );
                if (nPos<0)
                {
                    break;
                }
                // Has this been escaped?
                if( nPos && '\\' == rStr[nPos-1])
                {
                    ++nPos;
                    if( nPos >= rStr.getLength() )
                    {
                        break;
                    }
                }
                else
                {
                    rRet = rStr.copy( 0, nPos );
                    rStr = rStr.copy( nPos + sPara.getLength() );
                    return true;
                }
            }
        }
        rRet = rStr;
        rStr.clear();
        return false;
    }
}

namespace //local functions originally from docfmt.cxx
{

    bool lcl_ApplyOtherSet(
            SwContentNode & rNode,
            SwHistory *const pHistory,
            SfxItemSet const& rOtherSet,
            SfxItemSet const& rFirstSet,
            SfxItemSet const& rPropsSet,
            SwRootFrame const*const pLayout,
            SwNodeIndex *const o_pIndex = nullptr)
    {
        assert(rOtherSet.Count());

        bool ret(false);
        SwTextNode *const pTNd = rNode.GetTextNode();
        sw::MergedPara const* pMerged(nullptr);
        if (pLayout && pLayout->HasMergedParas() && pTNd)
        {
            SwTextFrame const*const pTextFrame(static_cast<SwTextFrame const*>(
                pTNd->getLayoutFrame(pLayout)));
            if (pTextFrame)
            {
                pMerged = pTextFrame->GetMergedPara();
            }
            if (pMerged)
            {
                if (rFirstSet.Count())
                {
                    if (pHistory)
                    {
                        SwRegHistory aRegH(pMerged->pFirstNode, *pMerged->pFirstNode, pHistory);
                        ret = pMerged->pFirstNode->SetAttr(rFirstSet);
                    }
                    else
                    {
                        ret = pMerged->pFirstNode->SetAttr(rFirstSet);
                    }
                }
                if (rPropsSet.Count())
                {
                    if (pHistory)
                    {
                        SwRegHistory aRegH(pMerged->pParaPropsNode, *pMerged->pParaPropsNode, pHistory);
                        ret = pMerged->pParaPropsNode->SetAttr(rPropsSet) || ret;
                    }
                    else
                    {
                        ret = pMerged->pParaPropsNode->SetAttr(rPropsSet) || ret;
                    }
                }
                if (o_pIndex)
                {
                    *o_pIndex = *pMerged->pLastNode; // skip hidden
                }
            }
        }

        // input cursor can't be on hidden node, and iteration skips them
        assert(!pLayout || !pLayout->HasMergedParas()
            || rNode.GetRedlineMergeFlag() != SwNode::Merge::Hidden);

        if (!pMerged)
        {
            if (pHistory)
            {
                SwRegHistory aRegH(&rNode, rNode, pHistory);
                ret = rNode.SetAttr( rOtherSet );
            }
            else
            {
                ret = rNode.SetAttr( rOtherSet );
            }
        }
        return ret;
    }

    #define DELETECHARSETS if ( bDelete ) { delete pCharSet; delete pOtherSet; }

    /// Insert Hints according to content types;
    // Is used in SwDoc::Insert(..., SwFormatHint &rHt)

    bool lcl_InsAttr(
        SwDoc& rDoc,
        const SwPaM &rRg,
        const SfxItemSet& rChgSet,
        const SetAttrMode nFlags,
        SwUndoAttr *const pUndo,
        SwRootFrame const*const pLayout,
        const bool bExpandCharToPara,
        SwTextAttr **ppNewTextAttr)
    {
        // Divide the Sets (for selections in Nodes)
        const SfxItemSet* pCharSet = nullptr;
        const SfxItemSet* pOtherSet = nullptr;
        bool bDelete = false;
        bool bCharAttr = false;
        bool bOtherAttr = false;

        // Check, if we can work with rChgSet or if we have to create additional SfxItemSets
        if ( 1 == rChgSet.Count() )
        {
            SfxItemIter aIter( rChgSet );
            const SfxPoolItem* pItem = aIter.GetCurItem();
            if (pItem && !IsInvalidItem(pItem))
            {
                const sal_uInt16 nWhich = pItem->Which();

                if ( isCHRATR(nWhich) ||
                     (RES_TXTATR_CHARFMT == nWhich) ||
                     (RES_TXTATR_INETFMT == nWhich) ||
                     (RES_TXTATR_AUTOFMT == nWhich) ||
                     (RES_TXTATR_UNKNOWN_CONTAINER == nWhich) )
                {
                    pCharSet  = &rChgSet;
                    bCharAttr = true;
                }

                if (    isPARATR(nWhich)
                     || isPARATR_LIST(nWhich)
                     || isFRMATR(nWhich)
                     || isGRFATR(nWhich)
                     || isUNKNOWNATR(nWhich)
                     || isDrawingLayerAttribute(nWhich) )
                {
                    pOtherSet = &rChgSet;
                    bOtherAttr = true;
                }
            }
        }

        // Build new itemset if either
        // - rChgSet.Count() > 1 or
        // - The attribute in rChgSet does not belong to one of the above categories
        if ( !bCharAttr && !bOtherAttr )
        {
            SfxItemSet* pTmpCharItemSet = new SfxItemSet(
                rDoc.GetAttrPool(),
                svl::Items<
                    RES_CHRATR_BEGIN, RES_CHRATR_END - 1,
                    RES_TXTATR_AUTOFMT, RES_TXTATR_CHARFMT,
                    RES_TXTATR_UNKNOWN_CONTAINER,
                        RES_TXTATR_UNKNOWN_CONTAINER>{});

            SfxItemSet* pTmpOtherItemSet = new SfxItemSet(
                rDoc.GetAttrPool(),
                svl::Items<
                    RES_PARATR_BEGIN, RES_GRFATR_END - 1,
                    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END - 1,
                    // FillAttribute support:
                    XATTR_FILL_FIRST, XATTR_FILL_LAST>{});

            pTmpCharItemSet->Put( rChgSet );
            pTmpOtherItemSet->Put( rChgSet );

            pCharSet = pTmpCharItemSet;
            pOtherSet = pTmpOtherItemSet;

            bDelete = true;
        }

        SwHistory* pHistory = pUndo ? &pUndo->GetHistory() : nullptr;
        bool bRet = false;
        const SwPosition *pStt = rRg.Start(), *pEnd = rRg.End();
        SwContentNode* pNode = pStt->nNode.GetNode().GetContentNode();

        if( pNode && pNode->IsTextNode() )
        {
            // tdf#127606 at editing, remove different formatting of DOCX-like numbering symbol
            if (pLayout && pNode->GetTextNode()->getIDocumentSettingAccess()->
                    get(DocumentSettingId::APPLY_PARAGRAPH_MARK_FORMAT_TO_NUMBERING ))
            {
                SwContentNode* pEndNode = pEnd->nNode.GetNode().GetContentNode();
                SwContentNode* pCurrentNode = pEndNode;
                auto nStartIndex = pNode->GetIndex();
                auto nEndIndex = pEndNode->GetIndex();
                SwNodeIndex aIdx( pEnd->nNode.GetNode() );
                while ( pCurrentNode != nullptr && nStartIndex <= pCurrentNode->GetIndex() )
                {
                    if (pCurrentNode->GetSwAttrSet().HasItem(RES_PARATR_LIST_AUTOFMT) &&
                        // remove character formatting only on wholly selected paragraphs
                        (nStartIndex < pCurrentNode->GetIndex() || pStt->nContent.GetIndex() == 0) &&
                        (pCurrentNode->GetIndex() < nEndIndex || pEnd->nContent.GetIndex() == pEndNode->Len()))
                    {
                        pCurrentNode->ResetAttr(RES_PARATR_LIST_AUTOFMT);
                        // reset also paragraph marker
                        SwIndex nIdx( pCurrentNode, pCurrentNode->Len() );
                        pCurrentNode->GetTextNode()->RstTextAttr(nIdx, 1);
                    }
                    pCurrentNode = SwNodes::GoPrevious( &aIdx );
                }
            }
            // #i27615#
            if (rRg.IsInFrontOfLabel())
            {
                SwTextNode * pTextNd = pNode->GetTextNode();
                if (pLayout)
                {
                    pTextNd = sw::GetParaPropsNode(*pLayout, *pTextNd);
                }
                SwNumRule * pNumRule = pTextNd->GetNumRule();

                if ( !pNumRule )
                {
                    OSL_FAIL( "<InsAttr(..)> - PaM in front of label, but text node has no numbering rule set. This is a serious defect." );
                    DELETECHARSETS
                    return false;
                }

                int nLevel = pTextNd->GetActualListLevel();

                if (nLevel < 0)
                    nLevel = 0;

                if (nLevel >= MAXLEVEL)
                    nLevel = MAXLEVEL - 1;

                SwNumFormat aNumFormat = pNumRule->Get(o3tl::narrowing<sal_uInt16>(nLevel));
                SwCharFormat * pCharFormat =
                    rDoc.FindCharFormatByName(aNumFormat.GetCharFormatName());

                if (pCharFormat)
                {
                    if (pHistory)
                        pHistory->Add(pCharFormat->GetAttrSet(), *pCharFormat);

                    if ( pCharSet )
                        pCharFormat->SetFormatAttr(*pCharSet);
                }

                DELETECHARSETS
                return true;
            }

            const SwIndex& rSt = pStt->nContent;

            // Attributes without an end do not have a range
            if ( !bCharAttr && !bOtherAttr )
            {
                SfxItemSet aTextSet( rDoc.GetAttrPool(),
                            svl::Items<RES_TXTATR_NOEND_BEGIN, RES_TXTATR_NOEND_END-1>{} );
                aTextSet.Put( rChgSet );
                if( aTextSet.Count() )
                {
                    SwRegHistory history( pNode, *pNode, pHistory );
                    bRet = history.InsertItems(
                        aTextSet, rSt.GetIndex(), rSt.GetIndex(), nFlags, /*ppNewTextAttr*/nullptr ) || bRet;

                    if (bRet && (rDoc.getIDocumentRedlineAccess().IsRedlineOn() || (!rDoc.getIDocumentRedlineAccess().IsIgnoreRedline()
                                    && !rDoc.getIDocumentRedlineAccess().GetRedlineTable().empty())))
                    {
                        SwPaM aPam( pStt->nNode, pStt->nContent.GetIndex()-1,
                                    pStt->nNode, pStt->nContent.GetIndex() );

                        if( pUndo )
                            pUndo->SaveRedlineData( aPam, true );

                        if( rDoc.getIDocumentRedlineAccess().IsRedlineOn() )
                            rDoc.getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( RedlineType::Insert, aPam ), true);
                        else
                            rDoc.getIDocumentRedlineAccess().SplitRedline( aPam );
                    }
                }
            }

            // TextAttributes with an end never expand their range
            if ( !bCharAttr && !bOtherAttr )
            {
                // CharFormat and URL attributes are treated separately!
                // TEST_TEMP ToDo: AutoFormat!
                SfxItemSet aTextSet(
                    rDoc.GetAttrPool(),
                    svl::Items<
                        RES_TXTATR_REFMARK, RES_TXTATR_METAFIELD,
                        RES_TXTATR_CJK_RUBY, RES_TXTATR_CJK_RUBY,
                        RES_TXTATR_INPUTFIELD, RES_TXTATR_INPUTFIELD>{});

                aTextSet.Put( rChgSet );
                if( aTextSet.Count() )
                {
                    const sal_Int32 nInsCnt = rSt.GetIndex();
                    const sal_Int32 nEnd = pStt->nNode == pEnd->nNode
                                    ? pEnd->nContent.GetIndex()
                                    : pNode->Len();
                    SwRegHistory history( pNode, *pNode, pHistory );
                    bRet = history.InsertItems( aTextSet, nInsCnt, nEnd, nFlags, ppNewTextAttr )
                           || bRet;

                    if (bRet && (rDoc.getIDocumentRedlineAccess().IsRedlineOn() || (!rDoc.getIDocumentRedlineAccess().IsIgnoreRedline()
                                    && !rDoc.getIDocumentRedlineAccess().GetRedlineTable().empty())))
                    {
                        // Was text content inserted? (RefMark/TOXMarks without an end)
                        bool bTextIns = nInsCnt != rSt.GetIndex();
                        // Was content inserted or set over the selection?
                        SwPaM aPam( pStt->nNode, bTextIns ? nInsCnt + 1 : nEnd,
                                    pStt->nNode, nInsCnt );
                        if( pUndo )
                            pUndo->SaveRedlineData( aPam, bTextIns );

                        if( rDoc.getIDocumentRedlineAccess().IsRedlineOn() )
                            rDoc.getIDocumentRedlineAccess().AppendRedline(
                                new SwRangeRedline(
                                    bTextIns ? RedlineType::Insert : RedlineType::Format, aPam ),
                                    true);
                        else if( bTextIns )
                            rDoc.getIDocumentRedlineAccess().SplitRedline( aPam );
                    }
                }
            }
        }

        // We always have to set the auto flag for PageDescs that are set at the Node!
        if( pOtherSet && pOtherSet->Count() )
        {
            SwTableNode* pTableNd;
            const SwFormatPageDesc* pDesc;
            if( SfxItemState::SET == pOtherSet->GetItemState( RES_PAGEDESC,
                            false, reinterpret_cast<const SfxPoolItem**>(&pDesc) ))
            {
                if( pNode )
                {
                    // Set auto flag. Only in the template it's without auto!
                    SwFormatPageDesc aNew( *pDesc );

                    // Tables now also know line breaks
                    if( !(nFlags & SetAttrMode::APICALL) &&
                        nullptr != ( pTableNd = pNode->FindTableNode() ) )
                    {
                        SwTableNode* pCurTableNd = pTableNd;
                        while ( nullptr != ( pCurTableNd = pCurTableNd->StartOfSectionNode()->FindTableNode() ) )
                            pTableNd = pCurTableNd;

                        // set the table format
                        SwFrameFormat* pFormat = pTableNd->GetTable().GetFrameFormat();
                        SwRegHistory aRegH( pFormat, *pTableNd, pHistory );
                        pFormat->SetFormatAttr( aNew );
                        bRet = true;
                    }
                    else
                    {
                        SwContentNode * pFirstNode(pNode);
                        if (pLayout && pLayout->HasMergedParas())
                        {
                            pFirstNode = sw::GetFirstAndLastNode(*pLayout, pStt->nNode).first;
                        }
                        SwRegHistory aRegH( pFirstNode, *pFirstNode, pHistory );
                        bRet = pFirstNode->SetAttr( aNew ) || bRet;
                    }
                }

                // bOtherAttr = true means that pOtherSet == rChgSet. In this case
                // we know, that there is only one attribute in pOtherSet. We cannot
                // perform the following operations, instead we return:
                if ( bOtherAttr )
                    return bRet;

                const_cast<SfxItemSet*>(pOtherSet)->ClearItem( RES_PAGEDESC );
                if( !pOtherSet->Count() )
                {
                    DELETECHARSETS
                    return bRet;
                }
            }

            // Tables now also know line breaks
            const SvxFormatBreakItem* pBreak;
            if( pNode && !(nFlags & SetAttrMode::APICALL) &&
                nullptr != (pTableNd = pNode->FindTableNode() ) &&
                SfxItemState::SET == pOtherSet->GetItemState( RES_BREAK,
                            false, reinterpret_cast<const SfxPoolItem**>(&pBreak) ) )
            {
                SwTableNode* pCurTableNd = pTableNd;
                while ( nullptr != ( pCurTableNd = pCurTableNd->StartOfSectionNode()->FindTableNode() ) )
                    pTableNd = pCurTableNd;

                 // set the table format
                SwFrameFormat* pFormat = pTableNd->GetTable().GetFrameFormat();
                SwRegHistory aRegH( pFormat, *pTableNd, pHistory );
                pFormat->SetFormatAttr( *pBreak );
                bRet = true;

                // bOtherAttr = true means that pOtherSet == rChgSet. In this case
                // we know, that there is only one attribute in pOtherSet. We cannot
                // perform the following operations, instead we return:
                if ( bOtherAttr )
                    return bRet;

                const_cast<SfxItemSet*>(pOtherSet)->ClearItem( RES_BREAK );
                if( !pOtherSet->Count() )
                {
                    DELETECHARSETS
                    return bRet;
                }
            }

            {
                // If we have a PoolNumRule, create it if needed
                const SwNumRuleItem* pRule;
                sal_uInt16 nPoolId=0;
                if( SfxItemState::SET == pOtherSet->GetItemState( RES_PARATR_NUMRULE,
                                    false, reinterpret_cast<const SfxPoolItem**>(&pRule) ) &&
                    !rDoc.FindNumRulePtr( pRule->GetValue() ) &&
                    USHRT_MAX != (nPoolId = SwStyleNameMapper::GetPoolIdFromUIName ( pRule->GetValue(),
                                    SwGetPoolIdFromName::NumRule )) )
                    rDoc.getIDocumentStylePoolAccess().GetNumRuleFromPool( nPoolId );
            }
        }

        SfxItemSet firstSet(rDoc.GetAttrPool(),
                svl::Items<RES_PAGEDESC, RES_BREAK>{});
        if (pOtherSet && pOtherSet->Count())
        {   // actually only RES_BREAK is possible here...
            firstSet.Put(*pOtherSet);
        }
        SfxItemSet propsSet(rDoc.GetAttrPool(),
            svl::Items<RES_PARATR_BEGIN, RES_PAGEDESC,
                       RES_BREAK+1, RES_FRMATR_END,
                       XATTR_FILL_FIRST, XATTR_FILL_LAST+1>{});
        if (pOtherSet && pOtherSet->Count())
        {
            propsSet.Put(*pOtherSet);
        }

        if( !rRg.HasMark() )        // no range
        {
            if( !pNode )
            {
                DELETECHARSETS
                return bRet;
            }

            if( pNode->IsTextNode() && pCharSet && pCharSet->Count() )
            {
                SwTextNode* pTextNd = pNode->GetTextNode();
                const SwIndex& rSt = pStt->nContent;
                sal_Int32 nMkPos, nPtPos = rSt.GetIndex();
                const OUString& rStr = pTextNd->GetText();

                // Special case: if the Cursor is located within a URL attribute, we take over it's area
                SwTextAttr const*const pURLAttr(
                    pTextNd->GetTextAttrAt(rSt.GetIndex(), RES_TXTATR_INETFMT));
                if (pURLAttr && !pURLAttr->GetINetFormat().GetValue().isEmpty())
                {
                    nMkPos = pURLAttr->GetStart();
                    nPtPos = *pURLAttr->End();
                }
                else
                {
                    assert(g_pBreakIt && g_pBreakIt->GetBreakIter().is());
                    Boundary aBndry = g_pBreakIt->GetBreakIter()->getWordBoundary(
                                        pTextNd->GetText(), nPtPos,
                                        g_pBreakIt->GetLocale( pTextNd->GetLang( nPtPos ) ),
                                        WordType::ANY_WORD /*ANYWORD_IGNOREWHITESPACES*/,
                                        true);

                    if( aBndry.startPos < nPtPos && nPtPos < aBndry.endPos )
                    {
                        nMkPos = aBndry.startPos;
                        nPtPos = aBndry.endPos;
                    }
                    else
                        nPtPos = nMkPos = rSt.GetIndex();
                }

                // Remove the overriding attributes from the SwpHintsArray,
                // if the selection spans across the whole paragraph.
                // These attributes are inserted as FormatAttributes and
                // never override the TextAttributes!
                if( !(nFlags & SetAttrMode::DONTREPLACE ) &&
                    pTextNd->HasHints() && !nMkPos && nPtPos == rStr.getLength())
                {
                    SwIndex aSt( pTextNd );
                    if( pHistory )
                    {
                        // Save all attributes for the Undo.
                        SwRegHistory aRHst( *pTextNd, pHistory );
                        pTextNd->GetpSwpHints()->Register( &aRHst );
                        pTextNd->RstTextAttr( aSt, nPtPos, 0, pCharSet );
                        if( pTextNd->GetpSwpHints() )
                            pTextNd->GetpSwpHints()->DeRegister();
                    }
                    else
                        pTextNd->RstTextAttr( aSt, nPtPos, 0, pCharSet );
                }

                // the SwRegHistory inserts the attribute into the TextNode!
                SwRegHistory history( pNode, *pNode, pHistory );
                bRet = history.InsertItems( *pCharSet, nMkPos, nPtPos, nFlags, /*ppNewTextAttr*/nullptr )
                    || bRet;

                if( rDoc.getIDocumentRedlineAccess().IsRedlineOn() )
                {
                    SwPaM aPam( *pNode, nMkPos, *pNode, nPtPos );

                    if( pUndo )
                        pUndo->SaveRedlineData( aPam, false );
                    rDoc.getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( RedlineType::Format, aPam ), true);
                }
            }
            if( pOtherSet && pOtherSet->Count() )
            {
                // Need to check for unique item for DrawingLayer items of type NameOrIndex
                // and evtl. correct that item to ensure unique names for that type. This call may
                // modify/correct entries inside of the given SfxItemSet
                SfxItemSet aTempLocalCopy(*pOtherSet);

                rDoc.CheckForUniqueItemForLineFillNameOrIndex(aTempLocalCopy);
                bRet = lcl_ApplyOtherSet(*pNode, pHistory, aTempLocalCopy, firstSet, propsSet, pLayout) || bRet;
            }

            DELETECHARSETS
            return bRet;
        }

        if( rDoc.getIDocumentRedlineAccess().IsRedlineOn() && pCharSet && pCharSet->Count() )
        {
            if( pUndo )
                pUndo->SaveRedlineData( rRg, false );
            rDoc.getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( RedlineType::Format, rRg ), true);
        }

        /* now if range */
        sal_uLong nNodes = 0;

        SwNodeIndex aSt( rDoc.GetNodes() );
        SwNodeIndex aEnd( rDoc.GetNodes() );
        SwIndex aCntEnd( pEnd->nContent );

        if( pNode )
        {
            const sal_Int32 nLen = pNode->Len();
            if( pStt->nNode != pEnd->nNode )
                aCntEnd.Assign( pNode, nLen );

            if( pStt->nContent.GetIndex() != 0 || aCntEnd.GetIndex() != nLen )
            {
                // the SwRegHistory inserts the attribute into the TextNode!
                if( pNode->IsTextNode() && pCharSet && pCharSet->Count() )
                {
                    SwRegHistory history( pNode, *pNode, pHistory );
                    bRet = history.InsertItems(*pCharSet,
                            pStt->nContent.GetIndex(), aCntEnd.GetIndex(), nFlags, /*ppNewTextAttr*/nullptr)
                        || bRet;
                }

                if( pOtherSet && pOtherSet->Count() )
                {
                    bRet = lcl_ApplyOtherSet(*pNode, pHistory, *pOtherSet, firstSet, propsSet, pLayout) || bRet;
                }

                // Only selection in a Node.
                if( pStt->nNode == pEnd->nNode )
                {
                //The data parameter flag: bExpandCharToPara, comes from the data member of SwDoc,
                //which is set in SW MS Word Binary filter WW8ImplRreader. With this flag on, means that
                //current setting attribute set is a character range properties set and comes from a MS Word
                //binary file, and the setting range include a paragraph end position (0X0D);
                //more specifications, as such property inside the character range properties set recorded in
                //MS Word binary file are dealt and inserted into data model (SwDoc) one by one, so we
                //only dealing the scenario that the char properties set with 1 item inside;

                    if (bExpandCharToPara && pCharSet && pCharSet->Count() ==1 )
                    {
                        SwTextNode* pCurrentNd = pStt->nNode.GetNode().GetTextNode();

                        if (pCurrentNd)
                        {
                             pCurrentNd->TryCharSetExpandToNum(*pCharSet);

                        }
                    }
                    DELETECHARSETS
                    return bRet;
                }
                ++nNodes;
                aSt.Assign( pStt->nNode.GetNode(), +1 );
            }
            else
                aSt = pStt->nNode;
            aCntEnd = pEnd->nContent; // aEnd was changed!
        }
        else
            aSt.Assign( pStt->nNode.GetNode(), +1 );

        // aSt points to the first full Node now

        /*
         * The selection spans more than one Node.
         */
        if( pStt->nNode < pEnd->nNode )
        {
            pNode = pEnd->nNode.GetNode().GetContentNode();
            if(pNode)
            {
                if( aCntEnd.GetIndex() != pNode->Len() )
                {
                    // the SwRegHistory inserts the attribute into the TextNode!
                    if( pNode->IsTextNode() && pCharSet && pCharSet->Count() )
                    {
                        SwRegHistory history( pNode, *pNode, pHistory );
                        (void)history.InsertItems(*pCharSet,
                                0, aCntEnd.GetIndex(), nFlags, /*ppNewTextAttr*/nullptr);
                    }

                    if( pOtherSet && pOtherSet->Count() )
                    {
                        lcl_ApplyOtherSet(*pNode, pHistory, *pOtherSet, firstSet, propsSet, pLayout);
                    }

                    ++nNodes;
                    aEnd = pEnd->nNode;
                }
                else
                    aEnd.Assign( pEnd->nNode.GetNode(), +1 );
            }
            else
                aEnd = pEnd->nNode;
        }
        else
            aEnd.Assign( pEnd->nNode.GetNode(), +1 );

        // aEnd points BEHIND the last full node now

        /* Edit the fully selected Nodes. */
        // Reset all attributes from the set!
        if( pCharSet && pCharSet->Count() && !( SetAttrMode::DONTREPLACE & nFlags ) )
        {
            ::sw::DocumentContentOperationsManager::ParaRstFormat aPara(
                    pStt, pEnd, pHistory, pCharSet, pLayout);
            rDoc.GetNodes().ForEach( aSt, aEnd, ::sw::DocumentContentOperationsManager::lcl_RstTextAttr, &aPara );
        }

        bool bCreateSwpHints = pCharSet && (
            SfxItemState::SET == pCharSet->GetItemState( RES_TXTATR_CHARFMT, false ) ||
            SfxItemState::SET == pCharSet->GetItemState( RES_TXTATR_INETFMT, false ) );

        for (SwNodeIndex current = aSt; current < aEnd; ++current)
        {
            SwTextNode *const pTNd = current.GetNode().GetTextNode();
            if (!pTNd)
                continue;

            if (pLayout && pLayout->HasMergedParas()
                && pTNd->GetRedlineMergeFlag() == SwNode::Merge::Hidden)
            {   // not really sure what to do here, but applying to hidden
                continue; // nodes doesn't make sense...
            }

            if( pHistory )
            {
                SwRegHistory aRegH( pTNd, *pTNd, pHistory );

                if (pCharSet && pCharSet->Count())
                {
                    if (SwpHints *pSwpHints = bCreateSwpHints ? &pTNd->GetOrCreateSwpHints()
                                                : pTNd->GetpSwpHints())
                    {
                        pSwpHints->Register( &aRegH );
                    }

                    pTNd->SetAttr(*pCharSet, 0, pTNd->GetText().getLength(), nFlags);

                    // re-fetch as it may be deleted by SetAttr
                    if (SwpHints *pSwpHints = pTNd->GetpSwpHints())
                        pSwpHints->DeRegister();
                }
            }
            else
            {
                if (pCharSet && pCharSet->Count())
                    pTNd->SetAttr(*pCharSet, 0, pTNd->GetText().getLength(), nFlags);
            }
            ++nNodes;
        }

        if (pOtherSet && pOtherSet->Count())
        {
            for (; aSt < aEnd; ++aSt)
            {
                pNode = aSt.GetNode().GetContentNode();
                if (!pNode)
                    continue;

                lcl_ApplyOtherSet(*pNode, pHistory, *pOtherSet, firstSet, propsSet, pLayout, &aSt);
                ++nNodes;
            }
        }

        //The data parameter flag: bExpandCharToPara, comes from the data member of SwDoc,
        //which is set in SW MS Word Binary filter WW8ImplRreader. With this flag on, means that
        //current setting attribute set is a character range properties set and comes from a MS Word
        //binary file, and the setting range include a paragraph end position (0X0D);
        //more specifications, as such property inside the character range properties set recorded in
        //MS Word binary file are dealt and inserted into data model (SwDoc) one by one, so we
        //only dealing the scenario that the char properties set with 1 item inside;
        if (bExpandCharToPara && pCharSet && pCharSet->Count() ==1)
        {
            SwPosition aStartPos (*rRg.Start());
            SwPosition aEndPos (*rRg.End());

            if (aEndPos.nNode.GetNode().GetTextNode() && aEndPos.nContent != aEndPos.nNode.GetNode().GetTextNode()->Len())
                aEndPos.nNode--;

            sal_uLong nStart = aStartPos.nNode.GetIndex();
            sal_uLong nEnd = aEndPos.nNode.GetIndex();
            for(; nStart <= nEnd; ++nStart)
            {
                SwNode* pNd = rDoc.GetNodes()[ nStart ];
                if (!pNd || !pNd->IsTextNode())
                    continue;
                SwTextNode *pCurrentNd = pNd->GetTextNode();
                pCurrentNd->TryCharSetExpandToNum(*pCharSet);
            }
        }

        DELETECHARSETS
        return (nNodes != 0) || bRet;
    }
}

namespace sw
{

namespace mark
{
    bool IsFieldmarkOverlap(SwPaM const& rPaM)
    {
        std::vector<std::pair<sal_uLong, sal_Int32>> Breaks;
        sw::CalcBreaks(Breaks, rPaM);
        return !Breaks.empty();
    }
}

DocumentContentOperationsManager::DocumentContentOperationsManager( SwDoc& i_rSwdoc ) : m_rDoc( i_rSwdoc )
{
}

/**
 * Checks if rStart..rEnd mark a range that makes sense to copy.
 *
 * IsMoveToFly means the copy is a move to create a fly
 * and so existing flys at the edge must not be copied.
 */
static bool IsEmptyRange(const SwPosition& rStart, const SwPosition& rEnd,
        SwCopyFlags const flags)
{
    if (rStart == rEnd)
    {   // check if a fly anchored there would be copied - then copy...
        return !IsDestroyFrameAnchoredAtChar(rStart, rStart, rEnd,
                (flags & SwCopyFlags::IsMoveToFly)
                    ? DelContentType::WriterfilterHack|DelContentType::AllMask
                    : DelContentType::AllMask);
    }
    else
    {
        return rEnd < rStart;
    }
}

// Copy an area into this document or into another document
bool
DocumentContentOperationsManager::CopyRange( SwPaM& rPam, SwPosition& rPos,
        SwCopyFlags const flags) const
{
    const SwPosition *pStt = rPam.Start(), *pEnd = rPam.End();

    SwDoc& rDoc = rPos.nNode.GetNode().GetDoc();
    bool bColumnSel = rDoc.IsClipBoard() && rDoc.IsColumnSelection();

    // Catch if there's no copy to do
    if (!rPam.HasMark() || (IsEmptyRange(*pStt, *pEnd, flags) && !bColumnSel))
        return false;

    // Prevent copying into Flys that are anchored in the source range
    if (&rDoc == &m_rDoc && (flags & SwCopyFlags::CheckPosInFly))
    {
        // Correct the Start-/EndNode
        sal_uLong nStt = pStt->nNode.GetIndex(),
                nEnd = pEnd->nNode.GetIndex(),
                nDiff = nEnd - nStt +1;
        SwNode* pNd = m_rDoc.GetNodes()[ nStt ];
        if( pNd->IsContentNode() && pStt->nContent.GetIndex() )
        {
            ++nStt;
            --nDiff;
        }
        if( (pNd = m_rDoc.GetNodes()[ nEnd ])->IsContentNode() &&
            static_cast<SwContentNode*>(pNd)->Len() != pEnd->nContent.GetIndex() )
        {
            --nEnd;
            --nDiff;
        }
        if( nDiff &&
            lcl_ChkFlyFly( rDoc, nStt, nEnd, rPos.nNode.GetIndex() ) )
        {
            return false;
        }
    }

    SwPaM* pRedlineRange = nullptr;
    if( rDoc.getIDocumentRedlineAccess().IsRedlineOn() ||
        (!rDoc.getIDocumentRedlineAccess().IsIgnoreRedline() && !rDoc.getIDocumentRedlineAccess().GetRedlineTable().empty() ) )
        pRedlineRange = new SwPaM( rPos );

    RedlineFlags eOld = rDoc.getIDocumentRedlineAccess().GetRedlineFlags();

    bool bRet = false;

    if( &rDoc != &m_rDoc )
    {   // ordinary copy
        bRet = CopyImpl(rPam, rPos, flags & ~SwCopyFlags::CheckPosInFly, pRedlineRange);
    }
    else if( ! ( *pStt <= rPos && rPos < *pEnd &&
            ( pStt->nNode != pEnd->nNode ||
              !pStt->nNode.GetNode().IsTextNode() )) )
    {
        // Copy to a position outside of the area, or copy a single TextNode
        // Do an ordinary copy
        bRet = CopyImpl(rPam, rPos, flags & ~SwCopyFlags::CheckPosInFly, pRedlineRange);
    }
    else
    {
        // Copy the range in itself
        assert(!"mst: this is assumed to be dead code");
    }

    rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
    if( pRedlineRange )
    {
        if( rDoc.getIDocumentRedlineAccess().IsRedlineOn() )
            rDoc.getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( RedlineType::Insert, *pRedlineRange ), true);
        else
            rDoc.getIDocumentRedlineAccess().SplitRedline( *pRedlineRange );
        delete pRedlineRange;
    }

    return bRet;
}

/// Delete a full Section of the NodeArray.
/// The passed Node is located somewhere in the designated Section.
void DocumentContentOperationsManager::DeleteSection( SwNode *pNode )
{
    assert(pNode && "Didn't pass a Node.");

    SwStartNode* pSttNd = pNode->IsStartNode() ? static_cast<SwStartNode*>(pNode)
                                               : pNode->StartOfSectionNode();
    SwNodeIndex aSttIdx( *pSttNd ), aEndIdx( *pNode->EndOfSectionNode() );

    // delete all Flys, Bookmarks, ...
    DelFlyInRange( aSttIdx, aEndIdx );
    m_rDoc.getIDocumentRedlineAccess().DeleteRedline( *pSttNd, true, RedlineType::Any );
    DelBookmarks(aSttIdx, aEndIdx);

    {
        // move all Cursor/StackCursor/UnoCursor out of the to-be-deleted area
        SwNodeIndex aMvStt( aSttIdx, 1 );
        SwDoc::CorrAbs( aMvStt, aEndIdx, SwPosition( aSttIdx ), true );
    }

    m_rDoc.GetNodes().DelNodes( aSttIdx, aEndIdx.GetIndex() - aSttIdx.GetIndex() + 1 );
}

void DocumentContentOperationsManager::DeleteDummyChar(
        SwPosition const& rPos, sal_Unicode const cDummy)
{
    SwPaM aPam(rPos, rPos);
    ++aPam.GetPoint()->nContent;
    assert(aPam.GetText().getLength() == 1 && aPam.GetText()[0] == cDummy);
    (void) cDummy;

    DeleteRangeImpl(aPam);

    if (!m_rDoc.getIDocumentRedlineAccess().IsIgnoreRedline()
        && !m_rDoc.getIDocumentRedlineAccess().GetRedlineTable().empty())
    {
        m_rDoc.getIDocumentRedlineAccess().CompressRedlines();
    }
}

void DocumentContentOperationsManager::DeleteRange( SwPaM & rPam )
{
    lcl_DoWithBreaks( *this, rPam, &DocumentContentOperationsManager::DeleteRangeImpl );

    if (!m_rDoc.getIDocumentRedlineAccess().IsIgnoreRedline()
        && !m_rDoc.getIDocumentRedlineAccess().GetRedlineTable().empty())
    {
        m_rDoc.getIDocumentRedlineAccess().CompressRedlines();
    }
}

bool DocumentContentOperationsManager::DelFullPara( SwPaM& rPam )
{
    const SwPosition &rStt = *rPam.Start(), &rEnd = *rPam.End();
    const SwNode* pNd = &rStt.nNode.GetNode();
    sal_uInt32 nSectDiff = pNd->StartOfSectionNode()->EndOfSectionIndex() -
                        pNd->StartOfSectionIndex();
    sal_uInt32 nNodeDiff = rEnd.nNode.GetIndex() - rStt.nNode.GetIndex();

    if ( nSectDiff-2 <= nNodeDiff || m_rDoc.getIDocumentRedlineAccess().IsRedlineOn() ||
         /* #i9185# Prevent getting the node after the end node (see below) */
        rEnd.nNode.GetIndex() + 1 == m_rDoc.GetNodes().Count() )
    {
        return false;
    }

    {
        SwPaM temp(rPam, nullptr);
        if (!temp.HasMark())
        {
            temp.SetMark();
        }
        if (SwTextNode *const pNode = temp.Start()->nNode.GetNode().GetTextNode())
        { // rPam may not have nContent set but IsFieldmarkOverlap requires it
            pNode->MakeStartIndex(&temp.Start()->nContent);
        }
        if (SwTextNode *const pNode = temp.End()->nNode.GetNode().GetTextNode())
        {
            pNode->MakeEndIndex(&temp.End()->nContent);
        }
        if (sw::mark::IsFieldmarkOverlap(temp))
        {   // a bit of a problem: we want to completely remove the nodes
            // but then how can the CH_TXT_ATR survive?
            return false;
        }
    }

    // Move hard page brakes to the following Node.
    bool bSavePageBreak = false, bSavePageDesc = false;

    /* #i9185# This would lead to a segmentation fault if not caught above. */
    sal_uLong nNextNd = rEnd.nNode.GetIndex() + 1;
    SwTableNode *const pTableNd = m_rDoc.GetNodes()[ nNextNd ]->GetTableNode();

    if( pTableNd && pNd->IsContentNode() )
    {
        SwFrameFormat* pTableFormat = pTableNd->GetTable().GetFrameFormat();

        {
            const SfxPoolItem *pItem;
            const SfxItemSet* pSet = static_cast<const SwContentNode*>(pNd)->GetpSwAttrSet();
            if( pSet && SfxItemState::SET == pSet->GetItemState( RES_PAGEDESC,
                false, &pItem ) )
            {
                pTableFormat->SetFormatAttr( *pItem );
                bSavePageDesc = true;
            }

            if( pSet && SfxItemState::SET == pSet->GetItemState( RES_BREAK,
                false, &pItem ) )
            {
                pTableFormat->SetFormatAttr( *pItem );
                bSavePageBreak = true;
            }
        }
    }

    bool const bDoesUndo = m_rDoc.GetIDocumentUndoRedo().DoesUndo();
    if( bDoesUndo )
    {
        if( !rPam.HasMark() )
            rPam.SetMark();
        else if( rPam.GetPoint() == &rStt )
            rPam.Exchange();
        rPam.GetPoint()->nNode++;

        SwContentNode *pTmpNode = rPam.GetPoint()->nNode.GetNode().GetContentNode();
        rPam.GetPoint()->nContent.Assign( pTmpNode, 0 );
        bool bGoNext = (nullptr == pTmpNode);
        pTmpNode = rPam.GetMark()->nNode.GetNode().GetContentNode();
        rPam.GetMark()->nContent.Assign( pTmpNode, 0 );

        m_rDoc.GetIDocumentUndoRedo().ClearRedo();

        SwPaM aDelPam( *rPam.GetMark(), *rPam.GetPoint() );
        {
            SwPosition aTmpPos( *aDelPam.GetPoint() );
            if( bGoNext )
            {
                pTmpNode = m_rDoc.GetNodes().GoNext( &aTmpPos.nNode );
                aTmpPos.nContent.Assign( pTmpNode, 0 );
            }
            ::PaMCorrAbs( aDelPam, aTmpPos );
        }

        std::unique_ptr<SwUndoDelete> pUndo(new SwUndoDelete( aDelPam, true ));

        *rPam.GetPoint() = *aDelPam.GetPoint();
        pUndo->SetPgBrkFlags( bSavePageBreak, bSavePageDesc );
        m_rDoc.GetIDocumentUndoRedo().AppendUndo(std::move(pUndo));
        rPam.DeleteMark();
    }
    else
    {
        SwNodeRange aRg( rStt.nNode, rEnd.nNode );
        rPam.Normalize(false);

        // Try to move past the End
        if( !rPam.Move( fnMoveForward, GoInNode ) )
        {
            // Fair enough, at the Beginning then
            rPam.Exchange();
            if( !rPam.Move( fnMoveBackward, GoInNode ))
            {
                SAL_WARN("sw.core", "DelFullPara: no more Nodes");
                return false;
            }
        }
        // move bookmarks, redlines etc.
        if (aRg.aStart == aRg.aEnd) // only first CorrAbs variant handles this
        {
            m_rDoc.CorrAbs( aRg.aStart, *rPam.GetPoint(), 0, true );
        }
        else
        {
            SwDoc::CorrAbs( aRg.aStart, aRg.aEnd, *rPam.GetPoint(), true );
        }

            // What's with Flys?
        {
            // If there are FlyFrames left, delete these too
            for( size_t n = 0; n < m_rDoc.GetSpzFrameFormats()->size(); ++n )
            {
                SwFrameFormat* pFly = (*m_rDoc.GetSpzFrameFormats())[n];
                const SwFormatAnchor* pAnchor = &pFly->GetAnchor();
                SwPosition const*const pAPos = pAnchor->GetContentAnchor();
                if (pAPos &&
                    ((RndStdIds::FLY_AT_PARA == pAnchor->GetAnchorId()) ||
                     (RndStdIds::FLY_AT_CHAR == pAnchor->GetAnchorId())) &&
                    // note: here use <= not < like in
                    // IsDestroyFrameAnchoredAtChar() because of the increment
                    // of rPam in the bDoesUndo path above!
                    aRg.aStart <= pAPos->nNode && pAPos->nNode <= aRg.aEnd )
                {
                    m_rDoc.getIDocumentLayoutAccess().DelLayoutFormat( pFly );
                    --n;
                }
            }
        }

        rPam.DeleteMark();
        m_rDoc.GetNodes().Delete( aRg.aStart, nNodeDiff+1 );
    }
    m_rDoc.getIDocumentState().SetModified();

    return true;
}

// #i100466# Add handling of new optional parameter <bForceJoinNext>
bool DocumentContentOperationsManager::DeleteAndJoin( SwPaM & rPam,
                           const bool bForceJoinNext )
{
    if ( lcl_StrLenOverflow( rPam ) )
        return false;

    bool const ret = lcl_DoWithBreaks( *this, rPam, (m_rDoc.getIDocumentRedlineAccess().IsRedlineOn())
                ? &DocumentContentOperationsManager::DeleteAndJoinWithRedlineImpl
                : &DocumentContentOperationsManager::DeleteAndJoinImpl,
                bForceJoinNext );

    return ret;
}

// It seems that this is mostly used by SwDoc internals; the only
// way to call this from the outside seems to be the special case in
// SwDoc::CopyRange (but I have not managed to actually hit that case).
bool DocumentContentOperationsManager::MoveRange( SwPaM& rPaM, SwPosition& rPos, SwMoveFlags eMvFlags )
{
    // nothing moved: return
    const SwPosition *pStt = rPaM.Start(), *pEnd = rPaM.End();
    if( !rPaM.HasMark() || *pStt >= *pEnd || (*pStt <= rPos && rPos < *pEnd))
        return false;

    assert(!sw::mark::IsFieldmarkOverlap(rPaM)); // probably an invalid redline was created?

    // Save the paragraph anchored Flys, so that they can be moved.
    SaveFlyArr aSaveFlyArr;
    SaveFlyInRange( rPaM, rPos, aSaveFlyArr, bool( SwMoveFlags::ALLFLYS & eMvFlags ) );

    // save redlines (if DOC_MOVEREDLINES is used)
    SaveRedlines_t aSaveRedl;
    if( SwMoveFlags::REDLINES & eMvFlags && !m_rDoc.getIDocumentRedlineAccess().GetRedlineTable().empty() )
    {
        lcl_SaveRedlines( rPaM, aSaveRedl );

        // #i17764# unfortunately, code below relies on undos being
        //          in a particular order, and presence of bookmarks
        //          will change this order. Hence, we delete bookmarks
        //          here without undo.
        ::sw::UndoGuard const undoGuard(m_rDoc.GetIDocumentUndoRedo());
        DelBookmarks(
            pStt->nNode,
            pEnd->nNode,
            nullptr,
            &pStt->nContent,
            &pEnd->nContent);
    }

    bool bUpdateFootnote = false;
    SwFootnoteIdxs aTmpFntIdx;

    std::unique_ptr<SwUndoMove> pUndoMove;
    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        m_rDoc.GetIDocumentUndoRedo().ClearRedo();
        pUndoMove.reset(new SwUndoMove( rPaM, rPos ));
        pUndoMove->SetMoveRedlines( eMvFlags == SwMoveFlags::REDLINES );
    }
    else
    {
        bUpdateFootnote = lcl_SaveFootnote( pStt->nNode, pEnd->nNode, rPos.nNode,
                                    m_rDoc.GetFootnoteIdxs(), aTmpFntIdx,
                                    &pStt->nContent, &pEnd->nContent );
    }

    bool bSplit = false;
    SwPaM aSavePam( rPos, rPos );

    // Move the SPoint to the beginning of the range
    if( rPaM.GetPoint() == pEnd )
        rPaM.Exchange();

    // If there is a TextNode before and after the Move, create a JoinNext in the EditShell.
    SwTextNode* pSrcNd = rPaM.GetPoint()->nNode.GetNode().GetTextNode();
    bool bCorrSavePam = pSrcNd && pStt->nNode != pEnd->nNode;

    // If one ore more TextNodes are moved, SwNodes::Move will do a SplitNode.
    // However, this does not update the cursor. So we create a TextNode to keep
    // updating the indices. After the Move the Node is optionally deleted.
    SwTextNode * pTNd = rPos.nNode.GetNode().GetTextNode();
    if( pTNd && rPaM.GetPoint()->nNode != rPaM.GetMark()->nNode &&
        ( rPos.nContent.GetIndex() || ( pTNd->Len() && bCorrSavePam  )) )
    {
        bSplit = true;
        const sal_Int32 nMkContent = rPaM.GetMark()->nContent.GetIndex();

        const std::shared_ptr<sw::mark::ContentIdxStore> pContentStore(sw::mark::ContentIdxStore::Create());
        pContentStore->Save( m_rDoc, rPos.nNode.GetIndex(), rPos.nContent.GetIndex(), true );

        SwTextNode * pOrigNode = pTNd;
        assert(*aSavePam.GetPoint() == *aSavePam.GetMark() &&
               *aSavePam.GetPoint() == rPos);
        assert(aSavePam.GetPoint()->nContent.GetIdxReg() == pOrigNode);
        assert(aSavePam.GetPoint()->nNode == rPos.nNode.GetIndex());
        assert(rPos.nNode.GetIndex() == pOrigNode->GetIndex());

        std::function<void (SwTextNode *, sw::mark::RestoreMode, bool)> restoreFunc(
            [&](SwTextNode *const, sw::mark::RestoreMode const eMode, bool)
            {
                if (!pContentStore->Empty())
                {
                    pContentStore->Restore(m_rDoc, pOrigNode->GetIndex()-1, 0, true, false, eMode);
                }
            });
        pTNd = pTNd->SplitContentNode(rPos, &restoreFunc)->GetTextNode();

        //A new node was inserted before the orig pTNd and the content up to
        //rPos moved into it. The old node is returned with the remainder
        //of the content in it.
        //
        //aSavePam was created with rPos, it continues to point to the
        //old node, but with the *original* content index into the node.
        //Seeing as all the orignode content before that index has
        //been removed, the new index into the original node should now be set
        //to 0 and the content index of rPos should also be adapted to the
        //truncated node
        assert(*aSavePam.GetPoint() == *aSavePam.GetMark() &&
               *aSavePam.GetPoint() == rPos);
        assert(aSavePam.GetPoint()->nContent.GetIdxReg() == pOrigNode);
        assert(aSavePam.GetPoint()->nNode == rPos.nNode.GetIndex());
        assert(rPos.nNode.GetIndex() == pOrigNode->GetIndex());
        aSavePam.GetPoint()->nContent.Assign(pOrigNode, 0);
        rPos = *aSavePam.GetMark() = *aSavePam.GetPoint();

        // correct the PaM!
        if( rPos.nNode == rPaM.GetMark()->nNode )
        {
            rPaM.GetMark()->nNode = rPos.nNode.GetIndex()-1;
            rPaM.GetMark()->nContent.Assign( pTNd, nMkContent );
        }
    }

    // Put back the Pam by one "content"; so that it's always outside of
    // the manipulated range.
    // tdf#99692 don't Move() back if that would end up in another node
    // because moving backward is not necessarily the inverse of forward then.
    // (but do Move() back if we have split the node)
    const bool bNullContent = !bSplit && aSavePam.GetPoint()->nContent == 0;
    if( bNullContent )
    {
        aSavePam.GetPoint()->nNode--;
        aSavePam.GetPoint()->nContent.Assign(aSavePam.GetContentNode(), 0);
    }
    else
    {
        bool const success(aSavePam.Move(fnMoveBackward, GoInContent));
        assert(success);
        (void) success;
    }

    // Copy all Bookmarks that are within the Move range into an array,
    // that saves the position as an offset.
    std::vector< ::sw::mark::SaveBookmark> aSaveBkmks;
    DelBookmarks(
        pStt->nNode,
        pEnd->nNode,
        &aSaveBkmks,
        &pStt->nContent,
        &pEnd->nContent);

    // If there is no range anymore due to the above deletions (e.g. the
    // footnotes got deleted), it's still a valid Move!
    if( *rPaM.GetPoint() != *rPaM.GetMark() )
    {
        // now do the actual move
        m_rDoc.GetNodes().MoveRange( rPaM, rPos, m_rDoc.GetNodes() );

        // after a MoveRange() the Mark is deleted
        if ( rPaM.HasMark() ) // => no Move occurred!
        {
            return false;
        }
    }
    else
        rPaM.DeleteMark();

    OSL_ENSURE( *aSavePam.GetMark() == rPos ||
            ( aSavePam.GetMark()->nNode.GetNode().GetContentNode() == nullptr ),
            "PaM was not moved. Aren't there ContentNodes at the beginning/end?" );
    *aSavePam.GetMark() = rPos;

    rPaM.SetMark();         // create a Sel. around the new range
    pTNd = aSavePam.GetNode().GetTextNode();
    assert(!m_rDoc.GetIDocumentUndoRedo().DoesUndo());
    bool bRemove = true;
    // Do two Nodes have to be joined at the SavePam?
    if (bSplit && pTNd)
    {
        if (pTNd->CanJoinNext())
        {
            // Always join next, because <pTNd> has to stay as it is.
            // A join previous from its next would more or less delete <pTNd>
            pTNd->JoinNext();
            bRemove = false;
        }
    }
    if (bNullContent)
    {
        aSavePam.GetPoint()->nNode++;
        aSavePam.GetPoint()->nContent.Assign( aSavePam.GetContentNode(), 0 );
    }
    else if (bRemove) // No move forward after joining with next paragraph
    {
        aSavePam.Move( fnMoveForward, GoInContent );
    }

    // Insert the Bookmarks back into the Document.
    *rPaM.GetMark() = *aSavePam.Start();
    for(auto& rBkmk : aSaveBkmks)
        rBkmk.SetInDoc(
            &m_rDoc,
            rPaM.GetMark()->nNode,
            &rPaM.GetMark()->nContent);
    *rPaM.GetPoint() = *aSavePam.End();

    // Move the Flys to the new position.
    // note: rPos is at the end here; can't really tell flys that used to be
    // at the start of rPam from flys that used to be at the end of rPam
    // unfortunately, so some of them are going to end up with wrong anchor...
    RestFlyInRange( aSaveFlyArr, *rPaM.Start(), &(rPos.nNode) );

    // restore redlines (if DOC_MOVEREDLINES is used)
    if( !aSaveRedl.empty() )
    {
        lcl_RestoreRedlines( m_rDoc, *aSavePam.Start(), aSaveRedl );
    }

    if( bUpdateFootnote )
    {
        if( !aTmpFntIdx.empty() )
        {
            m_rDoc.GetFootnoteIdxs().insert( aTmpFntIdx );
            aTmpFntIdx.clear();
        }

        m_rDoc.GetFootnoteIdxs().UpdateAllFootnote();
    }

    m_rDoc.getIDocumentState().SetModified();
    return true;
}

bool DocumentContentOperationsManager::MoveNodeRange( SwNodeRange& rRange, SwNodeIndex& rPos,
        SwMoveFlags eMvFlags )
{
    // Moves all Nodes to the new position.
    // Bookmarks are moved too (currently without Undo support).

    // If footnotes are being moved to the special section, remove them now.

    // Or else delete the Frames for all footnotes that are being moved
    // and have it rebuild after the Move (footnotes can change pages).
    // Additionally we have to correct the FootnoteIdx array's sorting.
    bool bUpdateFootnote = false;
    SwFootnoteIdxs aTmpFntIdx;

    std::unique_ptr<SwUndoMove> pUndo;
    if ((SwMoveFlags::CREATEUNDOOBJ & eMvFlags ) && m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo.reset(new SwUndoMove( m_rDoc, rRange, rPos ));
    }
    else
    {
        bUpdateFootnote = lcl_SaveFootnote( rRange.aStart, rRange.aEnd, rPos,
                                    m_rDoc.GetFootnoteIdxs(), aTmpFntIdx );
    }

    SaveRedlines_t aSaveRedl;
    std::vector<SwRangeRedline*> aSavRedlInsPosArr;
    if( SwMoveFlags::REDLINES & eMvFlags && !m_rDoc.getIDocumentRedlineAccess().GetRedlineTable().empty() )
    {
        lcl_SaveRedlines( rRange, aSaveRedl );

        // Find all RedLines that end at the InsPos.
        // These have to be moved back to the "old" position after the Move.
        SwRedlineTable::size_type nRedlPos = m_rDoc.getIDocumentRedlineAccess().GetRedlinePos( rPos.GetNode(), RedlineType::Any );
        if( SwRedlineTable::npos != nRedlPos )
        {
            const SwPosition *pRStt, *pREnd;
            do {
                SwRangeRedline* pTmp = m_rDoc.getIDocumentRedlineAccess().GetRedlineTable()[ nRedlPos ];
                pRStt = pTmp->Start();
                pREnd = pTmp->End();
                if( pREnd->nNode == rPos && pRStt->nNode < rPos )
                {
                    aSavRedlInsPosArr.push_back( pTmp );
                }
            } while( pRStt->nNode < rPos && ++nRedlPos < m_rDoc.getIDocumentRedlineAccess().GetRedlineTable().size());
        }
    }

    // Copy all Bookmarks that are within the Move range into an array
    // that stores all references to positions as an offset.
    // The final mapping happens after the Move.
    std::vector< ::sw::mark::SaveBookmark> aSaveBkmks;
    DelBookmarks(rRange.aStart, rRange.aEnd, &aSaveBkmks);

    // Save the paragraph-bound Flys, so that they can be moved.
    SaveFlyArr aSaveFlyArr;
    if( !m_rDoc.GetSpzFrameFormats()->empty() )
        SaveFlyInRange( rRange, aSaveFlyArr );

    // Set it to before the Position, so that it cannot be moved further.
    SwNodeIndex aIdx( rPos, -1 );

    std::unique_ptr<SwNodeIndex> pSaveInsPos;
    if( pUndo )
        pSaveInsPos.reset(new SwNodeIndex( rRange.aStart, -1 ));

    // move the Nodes
    bool bNoDelFrames = bool(SwMoveFlags::NO_DELFRMS & eMvFlags);
    if( m_rDoc.GetNodes().MoveNodes( rRange, m_rDoc.GetNodes(), rPos, !bNoDelFrames ) )
    {
        ++aIdx;     // again back to old position
        if( pSaveInsPos )
            ++(*pSaveInsPos);
    }
    else
    {
        aIdx = rRange.aStart;
        pUndo.reset();
    }

    // move the Flys to the new position
    if( !aSaveFlyArr.empty() )
    {
        SwPosition const tmp(aIdx);
        RestFlyInRange(aSaveFlyArr, tmp, nullptr);
    }

    // Add the Bookmarks back to the Document
    for(auto& rBkmk : aSaveBkmks)
        rBkmk.SetInDoc(&m_rDoc, aIdx);

    if( !aSavRedlInsPosArr.empty() )
    {
        SwNode* pNewNd = &aIdx.GetNode();
        for(SwRangeRedline* pTmp : aSavRedlInsPosArr)
        {
            if( m_rDoc.getIDocumentRedlineAccess().GetRedlineTable().Contains( pTmp ) )
            {
                SwPosition* pEnd = pTmp->End();
                pEnd->nNode = aIdx;
                pEnd->nContent.Assign( pNewNd->GetContentNode(), 0 );
            }
        }
    }

    if( !aSaveRedl.empty() )
        lcl_RestoreRedlines( m_rDoc, aIdx.GetIndex(), aSaveRedl );

    if( pUndo )
    {
        pUndo->SetDestRange( aIdx, rPos, *pSaveInsPos );
        m_rDoc.GetIDocumentUndoRedo().AppendUndo(std::move(pUndo));
    }

    pSaveInsPos.reset();

    if( bUpdateFootnote )
    {
        if( !aTmpFntIdx.empty() )
        {
            m_rDoc.GetFootnoteIdxs().insert( aTmpFntIdx );
            aTmpFntIdx.clear();
        }

        m_rDoc.GetFootnoteIdxs().UpdateAllFootnote();
    }

    m_rDoc.getIDocumentState().SetModified();
    return true;
}

void DocumentContentOperationsManager::MoveAndJoin( SwPaM& rPaM, SwPosition& rPos )
{
    SwNodeIndex aIdx( rPaM.Start()->nNode );
    bool bJoinText = aIdx.GetNode().IsTextNode();
    bool bOneNode = rPaM.GetPoint()->nNode == rPaM.GetMark()->nNode;
    aIdx--;             // in front of the move area!

    bool bRet = MoveRange( rPaM, rPos, SwMoveFlags::DEFAULT );
    if( bRet && !bOneNode )
    {
        if( bJoinText )
            ++aIdx;
        SwTextNode * pTextNd = aIdx.GetNode().GetTextNode();
        SwNodeIndex aNxtIdx( aIdx );
        if( pTextNd && pTextNd->CanJoinNext( &aNxtIdx ) )
        {
            {   // Block so SwIndex into node is deleted before Join
                m_rDoc.CorrRel( aNxtIdx, SwPosition( aIdx, SwIndex(pTextNd,
                            pTextNd->GetText().getLength()) ), 0, true );
            }
            pTextNd->JoinNext();
        }
    }
}

// Overwrite only uses the point of the PaM, the mark is ignored; characters
// are replaced from point until the end of the node; at the end of the node,
// characters are inserted.
bool DocumentContentOperationsManager::Overwrite( const SwPaM &rRg, const OUString &rStr )
{
    assert(rStr.getLength());
    SwPosition& rPt = *const_cast<SwPosition*>(rRg.GetPoint());
    if( m_rDoc.GetAutoCorrExceptWord() )                  // Add to AutoCorrect
    {
        if( 1 == rStr.getLength() )
            m_rDoc.GetAutoCorrExceptWord()->CheckChar( rPt, rStr[ 0 ] );
        m_rDoc.DeleteAutoCorrExceptWord();
    }

    SwTextNode *pNode = rPt.nNode.GetNode().GetTextNode();
    if (!pNode || rStr.getLength() > pNode->GetSpaceLeft()) // worst case: no erase
    {
        return false;
    }

    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        m_rDoc.GetIDocumentUndoRedo().ClearRedo(); // AppendUndo not always called
    }

    const size_t nOldAttrCnt = pNode->GetpSwpHints()
                                ? pNode->GetpSwpHints()->Count() : 0;
    SwDataChanged aTmp( rRg );
    SwIndex& rIdx = rPt.nContent;
    sal_Int32 const nActualStart(rIdx.GetIndex());
    sal_Int32 nStart = 0;

    bool bOldExpFlg = pNode->IsIgnoreDontExpand();
    pNode->SetIgnoreDontExpand( true );

    for( sal_Int32 nCnt = 0; nCnt < rStr.getLength(); ++nCnt )
    {
        // start behind the characters (to fix the attributes!)
        nStart = rIdx.GetIndex();
        if  (nStart < pNode->GetText().getLength())
        {
            lcl_SkipAttr( pNode, rIdx, nStart );
        }
        sal_Unicode c = rStr[ nCnt ];
        if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
        {
            bool bMerged(false);
            if (m_rDoc.GetIDocumentUndoRedo().DoesGroupUndo())
            {
                SwUndo *const pUndo = m_rDoc.GetUndoManager().GetLastUndo();
                SwUndoOverwrite *const pUndoOW(
                    dynamic_cast<SwUndoOverwrite *>(pUndo) );
                if (pUndoOW)
                {
                    // if CanGrouping() returns true it's already merged
                    bMerged = pUndoOW->CanGrouping(m_rDoc, rPt, c);
                }
            }
            if (!bMerged)
            {
                m_rDoc.GetIDocumentUndoRedo().AppendUndo(
                    std::make_unique<SwUndoOverwrite>(m_rDoc, rPt, c) );
            }
        }
        else
        {
            // start behind the characters (to fix the attributes!)
            if (nStart < pNode->GetText().getLength())
                ++rIdx;
            pNode->InsertText( OUString(c), rIdx, SwInsertFlags::EMPTYEXPAND );
            if( nStart+1 < rIdx.GetIndex() )
            {
                rIdx = nStart;
                pNode->EraseText( rIdx, 1 );
                ++rIdx;
            }
        }
    }
    pNode->SetIgnoreDontExpand( bOldExpFlg );

    const size_t nNewAttrCnt = pNode->GetpSwpHints()
                                ? pNode->GetpSwpHints()->Count() : 0;
    if( nOldAttrCnt != nNewAttrCnt )
    {
        const SwUpdateAttr aHint(0,0,0);
        pNode->TriggerNodeUpdate(sw::LegacyModifyHint(&aHint, &aHint));
    }

    if (!m_rDoc.GetIDocumentUndoRedo().DoesUndo() &&
        !m_rDoc.getIDocumentRedlineAccess().IsIgnoreRedline() && !m_rDoc.getIDocumentRedlineAccess().GetRedlineTable().empty())
    {
        SwPaM aPam(rPt.nNode, nActualStart, rPt.nNode, rPt.nContent.GetIndex());
        m_rDoc.getIDocumentRedlineAccess().DeleteRedline( aPam, true, RedlineType::Any );
    }
    else if( m_rDoc.getIDocumentRedlineAccess().IsRedlineOn() )
    {
        // FIXME: this redline is WRONG: there is no DELETE, and the skipped
        // characters are also included in aPam
        SwPaM aPam(rPt.nNode, nActualStart, rPt.nNode, rPt.nContent.GetIndex());
        m_rDoc.getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( RedlineType::Insert, aPam ), true);
    }

    m_rDoc.getIDocumentState().SetModified();
    return true;
}

bool DocumentContentOperationsManager::InsertString( const SwPaM &rRg, const OUString &rStr,
        const SwInsertFlags nInsertMode )
{
    // tdf#119019 accept tracked paragraph formatting to do not hide new insertions
    if( m_rDoc.getIDocumentRedlineAccess().IsRedlineOn() )
    {
        RedlineFlags eOld = m_rDoc.getIDocumentRedlineAccess().GetRedlineFlags();
        m_rDoc.getIDocumentRedlineAccess().AcceptRedlineParagraphFormatting( rRg );
        if (eOld != m_rDoc.getIDocumentRedlineAccess().GetRedlineFlags())
            m_rDoc.getIDocumentRedlineAccess().SetRedlineFlags( eOld );
    }

    // fetching DoesUndo is surprisingly expensive
    bool bDoesUndo = m_rDoc.GetIDocumentUndoRedo().DoesUndo();
    if (bDoesUndo)
        m_rDoc.GetIDocumentUndoRedo().ClearRedo(); // AppendUndo not always called!

    const SwPosition& rPos = *rRg.GetPoint();

    if( m_rDoc.GetAutoCorrExceptWord() )                  // add to auto correction
    {
        if( 1 == rStr.getLength() && m_rDoc.GetAutoCorrExceptWord()->IsDeleted() )
        {
            m_rDoc.GetAutoCorrExceptWord()->CheckChar( rPos, rStr[ 0 ] );
        }
        m_rDoc.DeleteAutoCorrExceptWord();
    }

    SwTextNode *const pNode = rPos.nNode.GetNode().GetTextNode();
    if(!pNode)
        return false;

    SwDataChanged aTmp( rRg );

    if (!bDoesUndo || !m_rDoc.GetIDocumentUndoRedo().DoesGroupUndo())
    {
        OUString const ins(pNode->InsertText(rStr, rPos.nContent, nInsertMode));
        if (bDoesUndo)
        {
            m_rDoc.GetIDocumentUndoRedo().AppendUndo(
                std::make_unique<SwUndoInsert>(rPos.nNode,
                        rPos.nContent.GetIndex(), ins.getLength(), nInsertMode));
        }
    }
    else
    {   // if Undo and grouping is enabled, everything changes!
        SwUndoInsert * pUndo = nullptr;

        // don't group the start if hints at the start should be expanded
        if (!(nInsertMode & SwInsertFlags::FORCEHINTEXPAND))
        {
            SwUndo *const pLastUndo = m_rDoc.GetUndoManager().GetLastUndo();
            SwUndoInsert *const pUndoInsert(
                dynamic_cast<SwUndoInsert *>(pLastUndo) );
            if (pUndoInsert && pUndoInsert->CanGrouping(rPos))
            {
                pUndo = pUndoInsert;
            }
        }

        CharClass const& rCC = GetAppCharClass();
        sal_Int32 nInsPos = rPos.nContent.GetIndex();

        if (!pUndo)
        {
            pUndo = new SwUndoInsert( rPos.nNode, nInsPos, 0, nInsertMode,
                            !rCC.isLetterNumeric( rStr, 0 ) );
            m_rDoc.GetIDocumentUndoRedo().AppendUndo( std::unique_ptr<SwUndo>(pUndo) );
        }

        OUString const ins(pNode->InsertText(rStr, rPos.nContent, nInsertMode));

        for (sal_Int32 i = 0; i < ins.getLength(); ++i)
        {
            nInsPos++;
            // if CanGrouping() returns true, everything has already been done
            if (!pUndo->CanGrouping(ins[i]))
            {
                pUndo = new SwUndoInsert(rPos.nNode, nInsPos, 1, nInsertMode,
                            !rCC.isLetterNumeric(ins, i));
                m_rDoc.GetIDocumentUndoRedo().AppendUndo( std::unique_ptr<SwUndo>(pUndo) );
            }
        }
    }

    // To-Do - add 'SwExtraRedlineTable' also ?
    if( m_rDoc.getIDocumentRedlineAccess().IsRedlineOn() || (!m_rDoc.getIDocumentRedlineAccess().IsIgnoreRedline() && !m_rDoc.getIDocumentRedlineAccess().GetRedlineTable().empty() ))
    {
        SwPaM aPam( rPos.nNode, aTmp.GetContent(),
                    rPos.nNode, rPos.nContent.GetIndex());
        if( m_rDoc.getIDocumentRedlineAccess().IsRedlineOn() )
        {
            m_rDoc.getIDocumentRedlineAccess().AppendRedline(
                new SwRangeRedline( RedlineType::Insert, aPam ), true);
        }
        else
        {
            m_rDoc.getIDocumentRedlineAccess().SplitRedline( aPam );
        }
    }

    m_rDoc.getIDocumentState().SetModified();
    return true;
}

void DocumentContentOperationsManager::TransliterateText(
    const SwPaM& rPaM,
    utl::TransliterationWrapper& rTrans )
{
    std::unique_ptr<SwUndoTransliterate> pUndo;
    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
        pUndo.reset(new SwUndoTransliterate( rPaM, rTrans ));

    const SwPosition* pStt = rPaM.Start(),
                       * pEnd = rPaM.End();
    sal_uLong nSttNd = pStt->nNode.GetIndex(),
          nEndNd = pEnd->nNode.GetIndex();
    sal_Int32 nSttCnt = pStt->nContent.GetIndex();
    sal_Int32 nEndCnt = pEnd->nContent.GetIndex();

    SwTextNode* pTNd = pStt->nNode.GetNode().GetTextNode();
    if( pStt == pEnd && pTNd )  // no selection?
    {
        // set current word as 'area of effect'

        assert(g_pBreakIt && g_pBreakIt->GetBreakIter().is());
        Boundary aBndry = g_pBreakIt->GetBreakIter()->getWordBoundary(
                            pTNd->GetText(), nSttCnt,
                            g_pBreakIt->GetLocale( pTNd->GetLang( nSttCnt ) ),
                            WordType::ANY_WORD /*ANYWORD_IGNOREWHITESPACES*/,
                            true);

        if( aBndry.startPos < nSttCnt && nSttCnt < aBndry.endPos )
        {
            nSttCnt = aBndry.startPos;
            nEndCnt = aBndry.endPos;
        }
    }

    bool bUseRedlining = m_rDoc.getIDocumentRedlineAccess().IsRedlineOn();
    // as a workaround for a known performance problem, switch off redlining
    // to avoid freezing, if transliteration could result too many redlines
    if ( bUseRedlining )
    {
        const sal_uLong nMaxRedlines = 500;
        const bool bIsTitleCase = rTrans.getType() == TransliterationFlags::TITLE_CASE;
        sal_uLong nAffectedNodes = 0;
        sal_uLong nAffectedChars = nEndCnt;
        SwNodeIndex aIdx( pStt->nNode );
        for( ; aIdx.GetIndex() <= nEndNd; ++aIdx )
        {
            SwTextNode* pAffectedNode = aIdx.GetNode().GetTextNode();

            // don't count not text nodes or empty text nodes
            if( !pAffectedNode || pAffectedNode->GetText().isEmpty() )
                continue;

            nAffectedNodes++;

            // count characters of the node (the last - maybe partially
            // selected - node was counted at initialization of nAffectedChars)
            if( aIdx.GetIndex() < nEndNd )
                nAffectedChars += pAffectedNode->GetText().getLength();

            // transliteration creates n redlines for n nodes, except in the
            // case of title case, where it creates n redlines for n words
            if( nAffectedNodes > nMaxRedlines ||
                    // estimate word count based on the character count, where
                    // 6 = average English word length is ~5 letters + space
                    ( bIsTitleCase && (nAffectedChars - nSttCnt)/6 > nMaxRedlines ) )
            {
                bUseRedlining = false;
                break;
            }
        }
    }

    if( nSttNd != nEndNd )  // is more than one text node involved?
    {
        // iterate over all effected text nodes, the first and the last one
        // may be incomplete because the selection starts and/or ends there

        SwNodeIndex aIdx( pStt->nNode );
        if( nSttCnt )
        {
            ++aIdx;
            if( pTNd )
            {
                pTNd->TransliterateText(
                        rTrans, nSttCnt, pTNd->GetText().getLength(), pUndo.get(), bUseRedlining);
            }
        }

        for( ; aIdx.GetIndex() < nEndNd; ++aIdx )
        {
            pTNd = aIdx.GetNode().GetTextNode();
            if (pTNd)
            {
                pTNd->TransliterateText(
                        rTrans, 0, pTNd->GetText().getLength(), pUndo.get(), bUseRedlining);
            }
        }

        if( nEndCnt && nullptr != ( pTNd = pEnd->nNode.GetNode().GetTextNode() ))
        {
            pTNd->TransliterateText( rTrans, 0, nEndCnt, pUndo.get(), bUseRedlining );
        }
    }
    else if( pTNd && nSttCnt < nEndCnt )
    {
        pTNd->TransliterateText( rTrans, nSttCnt, nEndCnt, pUndo.get(), bUseRedlining );
    }
    if( pUndo && pUndo->HasData() )
    {
        m_rDoc.GetIDocumentUndoRedo().AppendUndo(std::move(pUndo));
    }
    m_rDoc.getIDocumentState().SetModified();
}

SwFlyFrameFormat* DocumentContentOperationsManager::InsertGraphic(
        const SwPaM &rRg,
                            const OUString& rGrfName,
                            const OUString& rFltName,
                            const Graphic* pGraphic,
                            const SfxItemSet* pFlyAttrSet,
                            const SfxItemSet* pGrfAttrSet,
                            SwFrameFormat* pFrameFormat )
{
    if( !pFrameFormat )
        pFrameFormat = m_rDoc.getIDocumentStylePoolAccess().GetFrameFormatFromPool( RES_POOLFRM_GRAPHIC );
    SwGrfNode* pSwGrfNode = SwNodes::MakeGrfNode(
                            SwNodeIndex( m_rDoc.GetNodes().GetEndOfAutotext() ),
                            rGrfName, rFltName, pGraphic,
                            m_rDoc.GetDfltGrfFormatColl() );
    SwFlyFrameFormat* pSwFlyFrameFormat = InsNoTextNode( *rRg.GetPoint(), pSwGrfNode,
                            pFlyAttrSet, pGrfAttrSet, pFrameFormat );
    return pSwFlyFrameFormat;
}

SwFlyFrameFormat* DocumentContentOperationsManager::InsertGraphicObject(
        const SwPaM &rRg, const GraphicObject& rGrfObj,
                            const SfxItemSet* pFlyAttrSet,
                            const SfxItemSet* pGrfAttrSet )
{
    SwFrameFormat* pFrameFormat = m_rDoc.getIDocumentStylePoolAccess().GetFrameFormatFromPool( RES_POOLFRM_GRAPHIC );
    SwGrfNode* pSwGrfNode = SwNodes::MakeGrfNode(
                            SwNodeIndex( m_rDoc.GetNodes().GetEndOfAutotext() ),
                            rGrfObj, m_rDoc.GetDfltGrfFormatColl() );
    SwFlyFrameFormat* pSwFlyFrameFormat = InsNoTextNode( *rRg.GetPoint(), pSwGrfNode,
                            pFlyAttrSet, pGrfAttrSet, pFrameFormat );
    return pSwFlyFrameFormat;
}

SwFlyFrameFormat* DocumentContentOperationsManager::InsertEmbObject(
        const SwPaM &rRg, const svt::EmbeddedObjectRef& xObj,
                        SfxItemSet* pFlyAttrSet)
{
    sal_uInt16 nId = RES_POOLFRM_OLE;
    if (xObj.is())
    {
        SvGlobalName aClassName( xObj->getClassID() );
        if (SotExchange::IsMath(aClassName))
        {
            nId = RES_POOLFRM_FORMEL;
        }
    }

    SwFrameFormat* pFrameFormat = m_rDoc.getIDocumentStylePoolAccess().GetFrameFormatFromPool( nId );

    return InsNoTextNode( *rRg.GetPoint(), m_rDoc.GetNodes().MakeOLENode(
                            SwNodeIndex( m_rDoc.GetNodes().GetEndOfAutotext() ),
                            xObj,
                            m_rDoc.GetDfltGrfFormatColl() ),
                            pFlyAttrSet, nullptr,
                            pFrameFormat );
}

SwFlyFrameFormat* DocumentContentOperationsManager::InsertOLE(const SwPaM &rRg, const OUString& rObjName,
                        sal_Int64 nAspect,
                        const SfxItemSet* pFlyAttrSet,
                        const SfxItemSet* pGrfAttrSet)
{
    SwFrameFormat* pFrameFormat = m_rDoc.getIDocumentStylePoolAccess().GetFrameFormatFromPool( RES_POOLFRM_OLE );

    return InsNoTextNode( *rRg.GetPoint(),
                            m_rDoc.GetNodes().MakeOLENode(
                                SwNodeIndex( m_rDoc.GetNodes().GetEndOfAutotext() ),
                                rObjName,
                                nAspect,
                                m_rDoc.GetDfltGrfFormatColl(),
                                nullptr ),
                            pFlyAttrSet, pGrfAttrSet,
                            pFrameFormat );
}

void DocumentContentOperationsManager::ReRead( SwPaM& rPam, const OUString& rGrfName,
                    const OUString& rFltName, const Graphic* pGraphic )
{
    SwGrfNode *pGrfNd;
    if( !(( !rPam.HasMark()
         || rPam.GetPoint()->nNode.GetIndex() == rPam.GetMark()->nNode.GetIndex() )
         && nullptr != ( pGrfNd = rPam.GetPoint()->nNode.GetNode().GetGrfNode() )) )
        return;

    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        m_rDoc.GetIDocumentUndoRedo().AppendUndo(std::make_unique<SwUndoReRead>(rPam, *pGrfNd));
    }

    // Because we don't know if we can mirror the graphic, the mirror attribute is always reset
    if( MirrorGraph::Dont != pGrfNd->GetSwAttrSet().
                                            GetMirrorGrf().GetValue() )
        pGrfNd->SetAttr( SwMirrorGrf() );

    pGrfNd->ReRead( rGrfName, rFltName, pGraphic );
    m_rDoc.getIDocumentState().SetModified();
}

// Insert drawing object, which has to be already inserted in the DrawModel
SwDrawFrameFormat* DocumentContentOperationsManager::InsertDrawObj(
    const SwPaM &rRg,
    SdrObject& rDrawObj,
    const SfxItemSet& rFlyAttrSet )
{
    SwDrawFrameFormat* pFormat = m_rDoc.MakeDrawFrameFormat( OUString(), m_rDoc.GetDfltFrameFormat() );

    const SwFormatAnchor* pAnchor = nullptr;
    rFlyAttrSet.GetItemState( RES_ANCHOR, false, reinterpret_cast<const SfxPoolItem**>(&pAnchor) );
    pFormat->SetFormatAttr( rFlyAttrSet );

    // Didn't set the Anchor yet?
    // DrawObjecte must never end up in the Header/Footer!
    RndStdIds eAnchorId = pAnchor != nullptr ? pAnchor->GetAnchorId() : pFormat->GetAnchor().GetAnchorId();
    const bool bIsAtContent = (RndStdIds::FLY_AT_PAGE != eAnchorId);

    const SwNodeIndex* pChkIdx = nullptr;
    if ( pAnchor == nullptr )
    {
        pChkIdx = &rRg.GetPoint()->nNode;
    }
    else if ( bIsAtContent )
    {
        pChkIdx =
            pAnchor->GetContentAnchor() ? &pAnchor->GetContentAnchor()->nNode : &rRg.GetPoint()->nNode;
    }

    // allow drawing objects in header/footer, but control objects aren't allowed in header/footer.
    if( pChkIdx != nullptr
        && ::CheckControlLayer( &rDrawObj )
        && m_rDoc.IsInHeaderFooter( *pChkIdx ) )
    {
        // apply at-page anchor format
        eAnchorId = RndStdIds::FLY_AT_PAGE;
        pFormat->SetFormatAttr( SwFormatAnchor( eAnchorId ) );
    }
    else if( pAnchor == nullptr
             || ( bIsAtContent
                  && pAnchor->GetContentAnchor() == nullptr ) )
    {
        // apply anchor format
        SwFormatAnchor aAnch( pAnchor != nullptr ? *pAnchor : pFormat->GetAnchor() );
        eAnchorId = aAnch.GetAnchorId();
        if ( eAnchorId == RndStdIds::FLY_AT_FLY )
        {
            const SwStartNode* pStartNode = rRg.GetNode().FindFlyStartNode();
            assert(pStartNode);
            SwPosition aPos(*pStartNode);
            aAnch.SetAnchor( &aPos );
        }
        else
        {
            aAnch.SetAnchor( rRg.GetPoint() );
            if ( eAnchorId == RndStdIds::FLY_AT_PAGE )
            {
                eAnchorId = dynamic_cast<const SdrUnoObj*>( &rDrawObj) !=  nullptr ? RndStdIds::FLY_AS_CHAR : RndStdIds::FLY_AT_PARA;
                aAnch.SetType( eAnchorId );
            }
        }
        pFormat->SetFormatAttr( aAnch );
    }

    // insert text attribute for as-character anchored drawing object
    if ( eAnchorId == RndStdIds::FLY_AS_CHAR )
    {
        bool bAnchorAtPageAsFallback = true;
        const SwFormatAnchor& rDrawObjAnchorFormat = pFormat->GetAnchor();
        if ( rDrawObjAnchorFormat.GetContentAnchor() != nullptr )
        {
            SwTextNode* pAnchorTextNode =
                    rDrawObjAnchorFormat.GetContentAnchor()->nNode.GetNode().GetTextNode();
            if ( pAnchorTextNode != nullptr )
            {
                const sal_Int32 nStt = rDrawObjAnchorFormat.GetContentAnchor()->nContent.GetIndex();
                SwFormatFlyCnt aFormat( pFormat );
                pAnchorTextNode->InsertItem( aFormat, nStt, nStt );
                bAnchorAtPageAsFallback = false;
            }
        }

        if ( bAnchorAtPageAsFallback )
        {
            OSL_ENSURE( false, "DocumentContentOperationsManager::InsertDrawObj(..) - missing content anchor for as-character anchored drawing object --> anchor at-page" );
            pFormat->SetFormatAttr( SwFormatAnchor( RndStdIds::FLY_AT_PAGE ) );
        }
    }

    SwDrawContact* pContact = new SwDrawContact( pFormat, &rDrawObj );

    // Create Frames if necessary
    if( m_rDoc.getIDocumentLayoutAccess().GetCurrentViewShell() )
    {
        // create layout representation
        pFormat->MakeFrames();
        // #i42319# - follow-up of #i35635#
        // move object to visible layer
        // #i79391#
        if ( pContact->GetAnchorFrame() )
        {
            pContact->MoveObjToVisibleLayer( &rDrawObj );
        }
    }

    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        m_rDoc.GetIDocumentUndoRedo().AppendUndo( std::make_unique<SwUndoInsLayFormat>(pFormat, 0, 0) );
    }

    m_rDoc.getIDocumentState().SetModified();
    return pFormat;
}

bool DocumentContentOperationsManager::SplitNode( const SwPosition &rPos, bool bChkTableStart )
{
    SwContentNode *pNode = rPos.nNode.GetNode().GetContentNode();
    if(nullptr == pNode)
        return false;

    {
        // BUG 26675: Send DataChanged before deleting, so that we notice which objects are in scope.
        //            After that they can be before/after the position.
        SwDataChanged aTmp( m_rDoc, rPos );
    }

    SwUndoSplitNode* pUndo = nullptr;
    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        m_rDoc.GetIDocumentUndoRedo().ClearRedo();
        // insert the Undo object (currently only for TextNode)
        if( pNode->IsTextNode() )
        {
            pUndo = new SwUndoSplitNode( m_rDoc, rPos, bChkTableStart );
            m_rDoc.GetIDocumentUndoRedo().AppendUndo(std::unique_ptr<SwUndo>(pUndo));
        }
    }

    // Update the rsid of the old and the new node unless
    // the old node is split at the beginning or at the end
    SwTextNode *pTextNode =  rPos.nNode.GetNode().GetTextNode();
    const sal_Int32 nPos = rPos.nContent.GetIndex();
    if( pTextNode && nPos && nPos != pTextNode->Len() )
    {
        m_rDoc.UpdateParRsid( pTextNode );
    }

    //JP 28.01.97: Special case for SplitNode at table start:
    //             If it is at the beginning of a Doc/Fly/Footer/... or right at after a table
    //             then insert a paragraph before it.
    if( bChkTableStart && !rPos.nContent.GetIndex() && pNode->IsTextNode() )
    {
        sal_uLong nPrevPos = rPos.nNode.GetIndex() - 1;
        const SwTableNode* pTableNd;
        const SwNode* pNd = m_rDoc.GetNodes()[ nPrevPos ];
        if( pNd->IsStartNode() &&
            SwTableBoxStartNode == static_cast<const SwStartNode*>(pNd)->GetStartNodeType() &&
            nullptr != ( pTableNd = m_rDoc.GetNodes()[ --nPrevPos ]->GetTableNode() ) &&
            ((( pNd = m_rDoc.GetNodes()[ --nPrevPos ])->IsStartNode() &&
               SwTableBoxStartNode != static_cast<const SwStartNode*>(pNd)->GetStartNodeType() )
               || ( pNd->IsEndNode() && pNd->StartOfSectionNode()->IsTableNode() )
               || pNd->IsContentNode() ))
        {
            if( pNd->IsContentNode() )
            {
                //JP 30.04.99 Bug 65660:
                // There are no page breaks outside of the normal body area,
                // so this is not a valid condition to insert a paragraph.
                if( nPrevPos < m_rDoc.GetNodes().GetEndOfExtras().GetIndex() )
                    pNd = nullptr;
                else
                {
                    // Only if the table has page breaks!
                    const SwFrameFormat* pFrameFormat = pTableNd->GetTable().GetFrameFormat();
                    if( SfxItemState::SET != pFrameFormat->GetItemState(RES_PAGEDESC, false) &&
                        SfxItemState::SET != pFrameFormat->GetItemState( RES_BREAK, false ) )
                        pNd = nullptr;
                }
            }

            if( pNd )
            {
                SwTextNode* pTextNd = m_rDoc.GetNodes().MakeTextNode(
                                        SwNodeIndex( *pTableNd ),
                                        m_rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_TEXT ));
                if( pTextNd )
                {
                    const_cast<SwPosition&>(rPos).nNode = pTableNd->GetIndex()-1;
                    const_cast<SwPosition&>(rPos).nContent.Assign( pTextNd, 0 );

                    // only add page breaks/styles to the body area
                    if( nPrevPos > m_rDoc.GetNodes().GetEndOfExtras().GetIndex() )
                    {
                        SwFrameFormat* pFrameFormat = pTableNd->GetTable().GetFrameFormat();
                        const SfxPoolItem *pItem;
                        if( SfxItemState::SET == pFrameFormat->GetItemState( RES_PAGEDESC,
                            false, &pItem ) )
                        {
                            pTextNd->SetAttr( *pItem );
                            pFrameFormat->ResetFormatAttr( RES_PAGEDESC );
                        }
                        if( SfxItemState::SET == pFrameFormat->GetItemState( RES_BREAK,
                            false, &pItem ) )
                        {
                            pTextNd->SetAttr( *pItem );
                            pFrameFormat->ResetFormatAttr( RES_BREAK );
                        }
                    }

                    if( pUndo )
                        pUndo->SetTableFlag();
                    m_rDoc.getIDocumentState().SetModified();
                    return true;
                }
            }
        }
    }

    const std::shared_ptr<sw::mark::ContentIdxStore> pContentStore(sw::mark::ContentIdxStore::Create());
    pContentStore->Save( m_rDoc, rPos.nNode.GetIndex(), rPos.nContent.GetIndex(), true );
    assert(pNode->IsTextNode());
    std::function<void (SwTextNode *, sw::mark::RestoreMode, bool bAtStart)> restoreFunc(
        [&](SwTextNode *const, sw::mark::RestoreMode const eMode, bool const bAtStart)
        {
            if (!pContentStore->Empty())
            {   // move all bookmarks, TOXMarks, FlyAtCnt
                pContentStore->Restore(m_rDoc, rPos.nNode.GetIndex()-1, 0, true, bAtStart && (eMode & sw::mark::RestoreMode::Flys), eMode);
            }
            if (eMode & sw::mark::RestoreMode::NonFlys)
            {
                // To-Do - add 'SwExtraRedlineTable' also ?
                if (m_rDoc.getIDocumentRedlineAccess().IsRedlineOn() ||
                    (!m_rDoc.getIDocumentRedlineAccess().IsIgnoreRedline() &&
                     !m_rDoc.getIDocumentRedlineAccess().GetRedlineTable().empty()))
                {
                    SwPaM aPam( rPos );
                    aPam.SetMark();
                    aPam.Move( fnMoveBackward );
                    if (m_rDoc.getIDocumentRedlineAccess().IsRedlineOn())
                    {
                        m_rDoc.getIDocumentRedlineAccess().AppendRedline(
                            new SwRangeRedline(RedlineType::Insert, aPam), true);
                    }
                    else
                    {
                        m_rDoc.getIDocumentRedlineAccess().SplitRedline(aPam);
                    }
                }
            }
        });
    pNode->GetTextNode()->SplitContentNode(rPos, &restoreFunc);

    m_rDoc.getIDocumentState().SetModified();
    return true;
}

bool DocumentContentOperationsManager::AppendTextNode( SwPosition& rPos )
{
    // create new node before EndOfContent
    SwTextNode * pCurNode = rPos.nNode.GetNode().GetTextNode();
    if( !pCurNode )
    {
        // so then one can be created!
        SwNodeIndex aIdx( rPos.nNode, 1 );
        pCurNode = m_rDoc.GetNodes().MakeTextNode( aIdx,
                        m_rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_STANDARD ));
    }
    else
        pCurNode = pCurNode->AppendNode( rPos )->GetTextNode();

    rPos.nNode++;
    rPos.nContent.Assign( pCurNode, 0 );

    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        m_rDoc.GetIDocumentUndoRedo().AppendUndo( std::make_unique<SwUndoInsert>( rPos.nNode ) );
    }

    // To-Do - add 'SwExtraRedlineTable' also ?
    if( m_rDoc.getIDocumentRedlineAccess().IsRedlineOn() || (!m_rDoc.getIDocumentRedlineAccess().IsIgnoreRedline() && !m_rDoc.getIDocumentRedlineAccess().GetRedlineTable().empty() ))
    {
        SwPaM aPam( rPos );
        aPam.SetMark();
        aPam.Move( fnMoveBackward );
        if( m_rDoc.getIDocumentRedlineAccess().IsRedlineOn() )
            m_rDoc.getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( RedlineType::Insert, aPam ), true);
        else
            m_rDoc.getIDocumentRedlineAccess().SplitRedline( aPam );
    }

    m_rDoc.getIDocumentState().SetModified();
    return true;
}

bool DocumentContentOperationsManager::ReplaceRange( SwPaM& rPam, const OUString& rStr,
        const bool bRegExReplace )
{
    // unfortunately replace works slightly differently from delete,
    // so we cannot use lcl_DoWithBreaks here...

    std::vector<std::pair<sal_uLong, sal_Int32>> Breaks;

    SwPaM aPam( *rPam.GetMark(), *rPam.GetPoint() );
    aPam.Normalize(false);
    if (aPam.GetPoint()->nNode != aPam.GetMark()->nNode)
    {
        aPam.Move(fnMoveBackward);
    }
    OSL_ENSURE((aPam.GetPoint()->nNode == aPam.GetMark()->nNode), "invalid pam?");

    sw::CalcBreaks(Breaks, aPam);

    while (!Breaks.empty() // skip over prefix of dummy chars
            && (aPam.GetMark()->nNode.GetIndex() == Breaks.begin()->first)
            && (aPam.GetMark()->nContent.GetIndex() == Breaks.begin()->second))
    {
        // skip!
        ++aPam.GetMark()->nContent; // always in bounds if Breaks valid
        Breaks.erase(Breaks.begin());
    }
    *rPam.Start() = *aPam.GetMark(); // update start of original pam w/ prefix

    if (Breaks.empty())
    {
        // park aPam somewhere so it does not point to node that is deleted
        aPam.DeleteMark();
        *aPam.GetPoint() = SwPosition(m_rDoc.GetNodes().GetEndOfContent());
        return ReplaceRangeImpl(rPam, rStr, bRegExReplace); // original pam!
    }

    // Deletion must be split into several parts if the text node
    // contains a text attribute with end and with dummy character
    // and the selection does not contain the text attribute completely,
    // but overlaps its start (left), where the dummy character is.

    bool bRet( true );
    // iterate from end to start, to avoid invalidating the offsets!
    auto iter( Breaks.rbegin() );
    sal_uLong nOffset(0);
    SwNodes const& rNodes(rPam.GetPoint()->nNode.GetNodes());
    OSL_ENSURE(aPam.GetPoint() == aPam.End(), "wrong!");
    SwPosition & rEnd( *aPam.End() );
    SwPosition & rStart( *aPam.Start() );

    // set end of temp pam to original end (undo Move backward above)
    rEnd = *rPam.End();
    // after first deletion, rEnd will point into the original text node again!

    while (iter != Breaks.rend())
    {
        rStart = SwPosition(*rNodes[iter->first - nOffset]->GetTextNode(), iter->second + 1);
        if (rStart < rEnd) // check if part is empty
        {
            bRet &= (m_rDoc.getIDocumentRedlineAccess().IsRedlineOn())
                ? DeleteAndJoinWithRedlineImpl(aPam)
                : DeleteAndJoinImpl(aPam, false);
            nOffset = iter->first - rStart.nNode.GetIndex(); // deleted fly nodes...
        }
        rEnd = SwPosition(*rNodes[iter->first - nOffset]->GetTextNode(), iter->second);
        ++iter;
    }

    rStart = *rPam.Start(); // set to original start
    assert(rStart < rEnd && "replace part empty!");
    if (rStart < rEnd) // check if part is empty
    {
        bRet &= ReplaceRangeImpl(aPam, rStr, bRegExReplace);
    }

    rPam = aPam; // update original pam (is this required?)

    return bRet;
}

///Add a para for the char attribute exp...
bool DocumentContentOperationsManager::InsertPoolItem(
    const SwPaM &rRg,
    const SfxPoolItem &rHt,
    const SetAttrMode nFlags,
    SwRootFrame const*const pLayout,
    const bool bExpandCharToPara,
    SwTextAttr **ppNewTextAttr)
{
    if (utl::ConfigManager::IsFuzzing())
        return false;

    SwDataChanged aTmp( rRg );
    std::unique_ptr<SwUndoAttr> pUndoAttr;
    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        m_rDoc.GetIDocumentUndoRedo().ClearRedo();
        pUndoAttr.reset(new SwUndoAttr( rRg, rHt, nFlags ));
    }

    SfxItemSet aSet( m_rDoc.GetAttrPool(), {{rHt.Which(), rHt.Which()}} );
    aSet.Put( rHt );
    const bool bRet = lcl_InsAttr(m_rDoc, rRg, aSet, nFlags, pUndoAttr.get(), pLayout, bExpandCharToPara, ppNewTextAttr);

    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        m_rDoc.GetIDocumentUndoRedo().AppendUndo( std::move(pUndoAttr) );
    }

    if( bRet )
    {
        m_rDoc.getIDocumentState().SetModified();
    }
    return bRet;
}

void DocumentContentOperationsManager::InsertItemSet ( const SwPaM &rRg, const SfxItemSet &rSet,
        const SetAttrMode nFlags, SwRootFrame const*const pLayout)
{
    SwDataChanged aTmp( rRg );
    std::unique_ptr<SwUndoAttr> pUndoAttr;
    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        m_rDoc.GetIDocumentUndoRedo().ClearRedo();
        pUndoAttr.reset(new SwUndoAttr( rRg, rSet, nFlags ));
    }

    bool bRet = lcl_InsAttr(m_rDoc, rRg, rSet, nFlags, pUndoAttr.get(), pLayout, /*bExpandCharToPara*/false, /*ppNewTextAttr*/nullptr );

    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        m_rDoc.GetIDocumentUndoRedo().AppendUndo( std::move(pUndoAttr) );
    }

    if( bRet )
        m_rDoc.getIDocumentState().SetModified();
}

void DocumentContentOperationsManager::RemoveLeadingWhiteSpace(const SwPosition & rPos )
{
    const SwTextNode* pTNd = rPos.nNode.GetNode().GetTextNode();
    if ( !pTNd )
        return;

    const OUString& rText = pTNd->GetText();
    sal_Int32 nIdx = 0;
    while (nIdx < rText.getLength())
    {
        sal_Unicode const cCh = rText[nIdx];
        if (('\t' != cCh) && (' ' != cCh))
        {
            break;
        }
        ++nIdx;
    }

    if ( nIdx > 0 )
    {
        SwPaM aPam(rPos);
        aPam.GetPoint()->nContent = 0;
        aPam.SetMark();
        aPam.GetMark()->nContent = nIdx;
        DeleteRange( aPam );
    }
}

// Copy method from SwDoc - "copy Flys in Flys"
/// note: rRg/rInsPos *exclude* a partially selected start text node;
///       pCopiedPaM *includes* a partially selected start text node
void DocumentContentOperationsManager::CopyWithFlyInFly(
    const SwNodeRange& rRg,
    const SwNodeIndex& rInsPos,
    const std::pair<const SwPaM&, const SwPosition&>* pCopiedPaM /*and real insert pos*/,
    const bool bMakeNewFrames,
    const bool bDelRedlines,
    const bool bCopyFlyAtFly,
    SwCopyFlags const flags) const
{
    assert(!pCopiedPaM || pCopiedPaM->first.End()->nNode == rRg.aEnd);
    assert(!pCopiedPaM || pCopiedPaM->second.nNode <= rInsPos);

    SwDoc& rDest = rInsPos.GetNode().GetDoc();
    SwNodeIndex aSavePos( rInsPos );

    if (rRg.aStart != rRg.aEnd)
    {
        bool bEndIsEqualEndPos = rInsPos == rRg.aEnd;
        --aSavePos;
        SaveRedlEndPosForRestore aRedlRest( rInsPos, 0 );

        // insert behind the already copied start node
        m_rDoc.GetNodes().CopyNodes( rRg, rInsPos, false, true );
        aRedlRest.Restore();

        if (bEndIsEqualEndPos)
        {
            const_cast<SwNodeIndex&>(rRg.aEnd).Assign(aSavePos.GetNode(), +1);
        }
    }

    // Also copy all bookmarks
    // guess this must be done before the DelDummyNodes below as that
    // deletes nodes so would mess up the index arithmetic
    // sw_fieldmarkhide: also needs to be done before making frames
    if (m_rDoc.getIDocumentMarkAccess()->getAllMarksCount())
    {
        SwPaM aRgTmp( rRg.aStart, rRg.aEnd );
        SwPosition targetPos(SwNodeIndex(aSavePos,
                                         rRg.aStart != rRg.aEnd ? +1 : 0));
        if (pCopiedPaM && rRg.aStart != pCopiedPaM->first.Start()->nNode)
        {
            // there is 1 (partially selected, maybe) paragraph before
            assert(SwNodeIndex(rRg.aStart, -1) == pCopiedPaM->first.Start()->nNode);
            // only use the passed in target SwPosition if the source PaM point
            // is on a different node; if it was the same node then the target
            // position was likely moved along by the copy operation and now
            // points to the end of the range!
            targetPos = pCopiedPaM->second;
        }

        sw::CopyBookmarks(pCopiedPaM ? pCopiedPaM->first : aRgTmp, targetPos);
    }

    if (rRg.aStart != rRg.aEnd)
    {
        bool isRecreateEndNode(false);
        if (bMakeNewFrames) // tdf#130685 only after aRedlRest
        {   // recreate from previous node (could be merged now)
            if (SwTextNode *const pNode = aSavePos.GetNode().GetTextNode())
            {
                o3tl::sorted_vector<SwTextFrame*> frames;
                SwTextNode *const pEndNode = rInsPos.GetNode().GetTextNode();
                if (pEndNode)
                {
                    SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIter(*pEndNode);
                    for (SwTextFrame* pFrame = aIter.First(); pFrame; pFrame = aIter.Next())
                    {
                        if (pFrame->getRootFrame()->HasMergedParas())
                        {
                            frames.insert(pFrame);
                        }
                    }
                }
                sw::RecreateStartTextFrames(*pNode);
                if (!frames.empty())
                {   // tdf#132187 check if the end node needs new frames
                    SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIter(*pEndNode);
                    for (SwTextFrame* pFrame = aIter.First(); pFrame; pFrame = aIter.Next())
                    {
                        if (pFrame->getRootFrame()->HasMergedParas())
                        {
                            auto const it = frames.find(pFrame);
                            if (it != frames.end())
                            {
                                frames.erase(it);
                            }
                        }
                    }
                    if (!frames.empty()) // existing frame was deleted
                    {   // all layouts because MakeFrames recreates all layouts
                        pEndNode->DelFrames(nullptr);
                        isRecreateEndNode = true;
                    }
                }
            }
        }
        bool const isAtStartOfSection(aSavePos.GetNode().IsStartNode());
        ++aSavePos;
        if (bMakeNewFrames)
        {
            // it's possible that CheckParaRedlineMerge() deleted frames
            // on rInsPos so have to include it, but it must not be included
            // if it was the first node in the document so that MakeFrames()
            // will find the existing (wasn't deleted) frame on it
            SwNodeIndex const end(rInsPos,
                    (!isRecreateEndNode || isAtStartOfSection)
                    ? 0 : +1);
            ::MakeFrames(&rDest, aSavePos, end);
        }
    }

#if OSL_DEBUG_LEVEL > 0
    {
        //JP 17.06.99: Bug 66973 - check count only if the selection is in
        // the same section or there's no section, because sections that are
        // not fully selected are not copied.
        const SwSectionNode* pSSectNd = rRg.aStart.GetNode().FindSectionNode();
        SwNodeIndex aTmpI( rRg.aEnd, -1 );
        const SwSectionNode* pESectNd = aTmpI.GetNode().FindSectionNode();
        if( pSSectNd == pESectNd &&
            !rRg.aStart.GetNode().IsSectionNode() &&
            !aTmpI.GetNode().IsEndNode() )
        {
            // If the range starts with a SwStartNode, it isn't copied
            sal_uInt16 offset = (rRg.aStart.GetNode().GetNodeType() != SwNodeType::Start) ? 1 : 0;
            OSL_ENSURE( rInsPos.GetIndex() - aSavePos.GetIndex() ==
                    rRg.aEnd.GetIndex() - rRg.aStart.GetIndex() - 1 + offset,
                    "An insufficient number of nodes were copied!" );
        }
    }
#endif

    {
        ::sw::UndoGuard const undoGuard(rDest.GetIDocumentUndoRedo());
        CopyFlyInFlyImpl(rRg, pCopiedPaM ? &pCopiedPaM->first : nullptr,
            // see comment below regarding use of pCopiedPaM->second
            (pCopiedPaM && rRg.aStart != pCopiedPaM->first.Start()->nNode)
                ? pCopiedPaM->second.nNode
                : aSavePos,
            bCopyFlyAtFly,
            flags);
    }

    SwNodeRange aCpyRange( aSavePos, rInsPos );

    if( bDelRedlines && ( RedlineFlags::DeleteRedlines & rDest.getIDocumentRedlineAccess().GetRedlineFlags() ))
        lcl_DeleteRedlines( rRg, aCpyRange );

    rDest.GetNodes().DelDummyNodes( aCpyRange );
}

// note: for the redline Show/Hide this must be in sync with
// SwRangeRedline::CopyToSection()/DelCopyOfSection()/MoveFromSection()
void DocumentContentOperationsManager::CopyFlyInFlyImpl(
    const SwNodeRange& rRg,
    SwPaM const*const pCopiedPaM,
    const SwNodeIndex& rStartIdx,
    const bool bCopyFlyAtFly,
    SwCopyFlags const flags) const
{
    assert(!pCopiedPaM || pCopiedPaM->End()->nNode == rRg.aEnd);

    // First collect all Flys, sort them according to their ordering number,
    // and then only copy them. This maintains the ordering numbers (which are only
    // managed in the DrawModel).
    SwDoc& rDest = rStartIdx.GetNode().GetDoc();
    std::set< ZSortFly > aSet;
    const size_t nArrLen = m_rDoc.GetSpzFrameFormats()->size();

    SwTextBoxHelper::SavedLink aOldTextBoxes;
    SwTextBoxHelper::saveLinks(*m_rDoc.GetSpzFrameFormats(), aOldTextBoxes);

    for ( size_t n = 0; n < nArrLen; ++n )
    {
        SwFrameFormat* pFormat = (*m_rDoc.GetSpzFrameFormats())[n];
        SwFormatAnchor const*const pAnchor = &pFormat->GetAnchor();
        SwPosition const*const pAPos = pAnchor->GetContentAnchor();
        if ( !pAPos )
            continue;
        bool bAdd = false;
        sal_uLong nSkipAfter = pAPos->nNode.GetIndex();
        sal_uLong nStart = rRg.aStart.GetIndex();
        switch ( pAnchor->GetAnchorId() )
        {
            case RndStdIds::FLY_AT_FLY:
                if(bCopyFlyAtFly)
                    ++nSkipAfter;
                else if(m_rDoc.getIDocumentRedlineAccess().IsRedlineMove())
                    ++nStart;
            break;
            case RndStdIds::FLY_AT_PARA:
                {
                    bAdd = IsSelectFrameAnchoredAtPara(*pAPos,
                        pCopiedPaM ? *pCopiedPaM->Start() : SwPosition(rRg.aStart),
                        pCopiedPaM ? *pCopiedPaM->End() : SwPosition(rRg.aEnd),
                        (flags & SwCopyFlags::IsMoveToFly)
                            ? DelContentType::AllMask|DelContentType::WriterfilterHack
                            : DelContentType::AllMask);
                }
            break;
            case RndStdIds::FLY_AT_CHAR:
                {
                    bAdd = IsDestroyFrameAnchoredAtChar(*pAPos,
                        pCopiedPaM ? *pCopiedPaM->Start() : SwPosition(rRg.aStart),
                        pCopiedPaM ? *pCopiedPaM->End() : SwPosition(rRg.aEnd),
                        (flags & SwCopyFlags::IsMoveToFly)
                            ? DelContentType::AllMask|DelContentType::WriterfilterHack
                            : DelContentType::AllMask);
                }
            break;
            default:
                continue;
        }
        if (RndStdIds::FLY_AT_FLY == pAnchor->GetAnchorId())
        {
            if (nStart > nSkipAfter)
                continue;
            if (pAPos->nNode > rRg.aEnd)
                continue;
            //frames at the last source node are not always copied:
            //- if the node is empty and is the last node of the document or a table cell
            //  or a text frame then they have to be copied
            //- if the content index in this node is > 0 then paragraph and frame bound objects are copied
            //- to-character bound objects are copied if their index is <= nEndContentIndex
            if (pAPos->nNode < rRg.aEnd)
                bAdd = true;
            if (!bAdd && !m_rDoc.getIDocumentRedlineAccess().IsRedlineMove()) // fdo#40599: not for redline move
            {
                if (!bAdd)
                {
                    // technically old code checked nContent of AT_FLY which is pointless
                    bAdd = pCopiedPaM && 0 < pCopiedPaM->End()->nContent.GetIndex();
                }
            }
        }
        if( bAdd )
        {
            aSet.insert( ZSortFly( pFormat, pAnchor, nArrLen + aSet.size() ));
        }
    }

    // Store all copied (and also the newly created) frames in another array.
    // They are stored as matching the originals, so that we will be later
    // able to build the chains accordingly.
    std::vector< SwFrameFormat* > aVecSwFrameFormat;
    std::set< ZSortFly >::const_iterator it=aSet.begin();

    while (it != aSet.end())
    {
        // #i59964#
        // correct determination of new anchor position
        SwFormatAnchor aAnchor( *(*it).GetAnchor() );
        assert( aAnchor.GetContentAnchor() != nullptr );
        SwPosition newPos = *aAnchor.GetContentAnchor();
        // for at-paragraph and at-character anchored objects the new anchor
        // position can *not* be determined by the difference of the current
        // anchor position to the start of the copied range, because not
        // complete selected sections in the copied range aren't copied - see
        // method <SwNodes::CopyNodes(..)>.
        // Thus, the new anchor position in the destination document is found
        // by counting the text nodes.
        if ((aAnchor.GetAnchorId() == RndStdIds::FLY_AT_PARA) ||
            (aAnchor.GetAnchorId() == RndStdIds::FLY_AT_CHAR) )
        {
            // First, determine number of anchor text node in the copied range.
            // Note: The anchor text node *have* to be inside the copied range.
            sal_uLong nAnchorTextNdNumInRange( 0 );
            bool bAnchorTextNdFound( false );
            // start at the first node for which flys are copied
            SwNodeIndex aIdx(pCopiedPaM ? pCopiedPaM->Start()->nNode : rRg.aStart);
            while ( !bAnchorTextNdFound && aIdx <= rRg.aEnd )
            {
                if ( aIdx.GetNode().IsTextNode() )
                {
                    ++nAnchorTextNdNumInRange;
                    bAnchorTextNdFound = aAnchor.GetContentAnchor()->nNode == aIdx;
                }

                ++aIdx;
            }

            if ( !bAnchorTextNdFound )
            {
                // This case can *not* happen, but to be robust take the first
                // text node in the destination document.
                OSL_FAIL( "<SwDoc::_CopyFlyInFly(..)> - anchor text node in copied range not found" );
                nAnchorTextNdNumInRange = 1;
            }
            // Second, search corresponding text node in destination document
            // by counting forward from start insert position <rStartIdx> the
            // determined number of text nodes.
            aIdx = rStartIdx;
            SwNodeIndex aAnchorNdIdx( rStartIdx );
            const SwNode& aEndOfContentNd =
                                    aIdx.GetNode().GetNodes().GetEndOfContent();
            while ( nAnchorTextNdNumInRange > 0 &&
                    &(aIdx.GetNode()) != &aEndOfContentNd )
            {
                if ( aIdx.GetNode().IsTextNode() )
                {
                    --nAnchorTextNdNumInRange;
                    aAnchorNdIdx = aIdx;
                }

                ++aIdx;
            }
            if ( !aAnchorNdIdx.GetNode().IsTextNode() )
            {
                // This case can *not* happen, but to be robust take the first
                // text node in the destination document.
                OSL_FAIL( "<SwDoc::_CopyFlyInFly(..)> - found anchor node index isn't a text node" );
                aAnchorNdIdx = rStartIdx;
                while ( !aAnchorNdIdx.GetNode().IsTextNode() )
                {
                    ++aAnchorNdIdx;
                }
            }
            // apply found anchor text node as new anchor position
            newPos.nNode = aAnchorNdIdx;
        }
        else
        {
            tools::Long nOffset = newPos.nNode.GetIndex() - rRg.aStart.GetIndex();
            SwNodeIndex aIdx( rStartIdx, nOffset );
            newPos.nNode = aIdx;
        }
        // Set the character bound Flys back at the original character
        if ((RndStdIds::FLY_AT_CHAR == aAnchor.GetAnchorId()) &&
             newPos.nNode.GetNode().IsTextNode() )
        {
            // only if pCopiedPaM: care about partially selected start node
            sal_Int32 const nContent = pCopiedPaM && pCopiedPaM->Start()->nNode == aAnchor.GetContentAnchor()->nNode
                ? newPos.nContent.GetIndex() - pCopiedPaM->Start()->nContent.GetIndex()
                : newPos.nContent.GetIndex();
            newPos.nContent.Assign(newPos.nNode.GetNode().GetTextNode(), nContent);
        }
        else
        {
            newPos.nContent.Assign( nullptr, 0 );
        }
        aAnchor.SetAnchor( &newPos );

        // Check recursion: if copying content inside the same frame, then don't copy the format.
        if( &rDest == &m_rDoc )
        {
            const SwFormatContent& rContent = (*it).GetFormat()->GetContent();
            const SwStartNode* pSNd;
            if( rContent.GetContentIdx() &&
                nullptr != ( pSNd = rContent.GetContentIdx()->GetNode().GetStartNode() ) &&
                pSNd->GetIndex() < rStartIdx.GetIndex() &&
                rStartIdx.GetIndex() < pSNd->EndOfSectionIndex() )
            {
                it = aSet.erase(it);
                continue;
            }
        }

        // Ignore TextBoxes, they are already handled in
        // sw::DocumentLayoutManager::CopyLayoutFormat().
        if (SwTextBoxHelper::isTextBox(it->GetFormat(), RES_FLYFRMFMT))
        {
            it = aSet.erase(it);
            continue;
        }

        // Copy the format and set the new anchor
        aVecSwFrameFormat.push_back( rDest.getIDocumentLayoutAccess().CopyLayoutFormat( *(*it).GetFormat(),
                aAnchor, false, true ) );
        ++it;
    }

    // Rebuild as much as possible of all chains that are available in the original,
    OSL_ENSURE( aSet.size() == aVecSwFrameFormat.size(), "Missing new Flys" );
    if ( aSet.size() != aVecSwFrameFormat.size() )
        return;

    size_t n = 0;
    for (const auto& rFlyN : aSet)
    {
        const SwFrameFormat *pFormatN = rFlyN.GetFormat();
        const SwFormatChain &rChain = pFormatN->GetChain();
        int nCnt = int(nullptr != rChain.GetPrev());
        nCnt += rChain.GetNext() ? 1: 0;
        size_t k = 0;
        for (const auto& rFlyK : aSet)
        {
            const SwFrameFormat *pFormatK = rFlyK.GetFormat();
            if ( rChain.GetPrev() == pFormatK )
            {
                ::lcl_ChainFormats( static_cast< SwFlyFrameFormat* >(aVecSwFrameFormat[k]),
                                 static_cast< SwFlyFrameFormat* >(aVecSwFrameFormat[n]) );
                --nCnt;
            }
            else if ( rChain.GetNext() == pFormatK )
            {
                ::lcl_ChainFormats( static_cast< SwFlyFrameFormat* >(aVecSwFrameFormat[n]),
                                 static_cast< SwFlyFrameFormat* >(aVecSwFrameFormat[k]) );
                --nCnt;
            }
            ++k;
        }
        ++n;
    }

    // Re-create content property of draw formats, knowing how old shapes
    // were paired with old fly formats (aOldTextBoxes) and that aSet is
    // parallel with aVecSwFrameFormat.
    SwTextBoxHelper::restoreLinks(aSet, aVecSwFrameFormat, aOldTextBoxes);
}

/*
 * Reset the text's hard formatting
 */
/** @params pArgs contains the document's ChrFormatTable
 *                Is need for selections at the beginning/end and with no SSelection.
 */
bool DocumentContentOperationsManager::lcl_RstTextAttr( const SwNodePtr& rpNd, void* pArgs )
{
    ParaRstFormat* pPara = static_cast<ParaRstFormat*>(pArgs);
    if (pPara->pLayout && pPara->pLayout->HasMergedParas()
        && rpNd->GetRedlineMergeFlag() == SwNode::Merge::Hidden)
    {
        return true; // skip hidden, since new items aren't applied
    }
    SwTextNode * pTextNode = rpNd->GetTextNode();
    if( pTextNode && pTextNode->GetpSwpHints() )
    {
        SwIndex aSt( pTextNode, 0 );
        sal_Int32 nEnd = pTextNode->Len();

        if( &pPara->pSttNd->nNode.GetNode() == pTextNode &&
            pPara->pSttNd->nContent.GetIndex() )
            aSt = pPara->pSttNd->nContent.GetIndex();

        if( &pPara->pEndNd->nNode.GetNode() == rpNd )
            nEnd = pPara->pEndNd->nContent.GetIndex();

        if( pPara->pHistory )
        {
            // Save all attributes for the Undo.
            SwRegHistory aRHst( *pTextNode, pPara->pHistory );
            pTextNode->GetpSwpHints()->Register( &aRHst );
            pTextNode->RstTextAttr( aSt, nEnd - aSt.GetIndex(), pPara->nWhich,
                                  pPara->pDelSet, pPara->bInclRefToxMark, pPara->bExactRange );
            if( pTextNode->GetpSwpHints() )
                pTextNode->GetpSwpHints()->DeRegister();
        }
        else
            pTextNode->RstTextAttr( aSt, nEnd - aSt.GetIndex(), pPara->nWhich,
                                  pPara->pDelSet, pPara->bInclRefToxMark, pPara->bExactRange );
    }
    return true;
}

DocumentContentOperationsManager::~DocumentContentOperationsManager()
{
}
//Private methods

bool DocumentContentOperationsManager::DeleteAndJoinWithRedlineImpl( SwPaM & rPam, const bool )
{
    assert(m_rDoc.getIDocumentRedlineAccess().IsRedlineOn());

    RedlineFlags eOld = m_rDoc.getIDocumentRedlineAccess().GetRedlineFlags();

    if (*rPam.GetPoint() == *rPam.GetMark())
    {
        return false; // do not add empty redlines
    }

    std::vector<SwRangeRedline*> redlines;
    {
        auto pRedline(std::make_unique<SwRangeRedline>(RedlineType::Delete, rPam));
        if (pRedline->HasValidRange())
        {
            redlines.push_back(pRedline.release());
        }
        else // sigh ... why is such a selection even possible...
        {    // split it up so we get one SwUndoRedlineDelete per inserted RL
            redlines = GetAllValidRanges(std::move(pRedline));
        }
    }

    if (redlines.empty())
    {
        return false;
    }

    // tdf#54819 current redlining needs also modification of paragraph style and
    // attributes added to the same grouped Undo
    if (m_rDoc.GetIDocumentUndoRedo().DoesGroupUndo())
        m_rDoc.GetIDocumentUndoRedo().StartUndo(SwUndoId::EMPTY, nullptr);

    auto & rDMA(*m_rDoc.getIDocumentMarkAccess());
    std::vector<std::unique_ptr<SwUndo>> MarkUndos;
    for (auto iter = rDMA.getAnnotationMarksBegin();
              iter != rDMA.getAnnotationMarksEnd(); )
    {
        // tdf#111524 remove annotation marks that have their field
        // characters deleted
        SwPosition const& rEndPos((**iter).GetMarkEnd());
        if (*rPam.Start() < rEndPos && rEndPos <= *rPam.End())
        {
            if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
            {
                MarkUndos.emplace_back(std::make_unique<SwUndoDeleteBookmark>(**iter));
            }
            // iter is into annotation mark vector so must be dereferenced!
            rDMA.deleteMark(&**iter);
            // this invalidates iter, have to start over...
            iter = rDMA.getAnnotationMarksBegin();
        }
        else
        {   // marks are sorted by start
            if (*rPam.End() < (**iter).GetMarkStart())
            {
                break;
            }
            ++iter;
        }
    }

    // tdf#119019 accept tracked paragraph formatting to do not hide new deletions
    if (*rPam.GetPoint() != *rPam.GetMark())
        m_rDoc.getIDocumentRedlineAccess().AcceptRedlineParagraphFormatting(rPam);

    std::vector<std::unique_ptr<SwUndoRedlineDelete>> undos;
    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        // this should no longer happen in calls from the UI but maybe via API
        // (randomTest and testTdf54819 triggers it)
        SAL_WARN_IF((eOld & RedlineFlags::ShowMask) != RedlineFlags::ShowMask,
                "sw.core", "redlines will be moved in DeleteAndJoin");
        m_rDoc.getIDocumentRedlineAccess().SetRedlineFlags(
            RedlineFlags::On | RedlineFlags::ShowInsert | RedlineFlags::ShowDelete);

        for (SwRangeRedline * pRedline : redlines)
        {
            assert(pRedline->HasValidRange());
            undos.emplace_back(std::make_unique<SwUndoRedlineDelete>(
                        *pRedline, SwUndoId::DELETE));
        }
        const SwRewriter aRewriter = undos.front()->GetRewriter();
        // can only group a single undo action
        if (MarkUndos.empty() && undos.size() == 1
            && m_rDoc.GetIDocumentUndoRedo().DoesGroupUndo())
        {
            SwUndo * const pLastUndo( m_rDoc.GetUndoManager().GetLastUndo() );
            SwUndoRedlineDelete *const pUndoRedlineDel(dynamic_cast<SwUndoRedlineDelete*>(pLastUndo));
            bool const bMerged = pUndoRedlineDel
                && pUndoRedlineDel->CanGrouping(*undos.front());
            if (!bMerged)
            {
                m_rDoc.GetIDocumentUndoRedo().AppendUndo(std::move(undos.front()));
            }
            undos.clear(); // prevent unmatched EndUndo
        }
        else
        {
            m_rDoc.GetIDocumentUndoRedo().StartUndo(SwUndoId::DELETE, &aRewriter);
            for (auto& it : MarkUndos)
            {
                m_rDoc.GetIDocumentUndoRedo().AppendUndo(std::move(it));
            }
            for (auto & it : undos)
            {
                m_rDoc.GetIDocumentUndoRedo().AppendUndo(std::move(it));
            }
        }
    }

    for (SwRangeRedline *const pRedline : redlines)
    {
        // note: 1. the pRedline can still be merged & deleted
        //       2. the impl. can even DeleteAndJoin the range => no plain PaM
        std::shared_ptr<SwUnoCursor> const pCursor(m_rDoc.CreateUnoCursor(*pRedline->GetMark()));
        pCursor->SetMark();
        *pCursor->GetPoint() = *pRedline->GetPoint();
        m_rDoc.getIDocumentRedlineAccess().AppendRedline(pRedline, true);
        // sw_redlinehide: 2 reasons why this is needed:
        // 1. it's the first redline in node => RedlineDelText was sent but ignored
        // 2. redline spans multiple nodes => must merge text frames
        sw::UpdateFramesForAddDeleteRedline(m_rDoc, *pCursor);
    }
    m_rDoc.getIDocumentState().SetModified();

    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        if (!undos.empty())
        {
            m_rDoc.GetIDocumentUndoRedo().EndUndo(SwUndoId::EMPTY, nullptr);
        }
        m_rDoc.getIDocumentRedlineAccess().SetRedlineFlags( eOld );
    }

    if (m_rDoc.GetIDocumentUndoRedo().DoesGroupUndo())
        m_rDoc.GetIDocumentUndoRedo().EndUndo(SwUndoId::EMPTY, nullptr);

    return true;
}

bool DocumentContentOperationsManager::DeleteAndJoinImpl( SwPaM & rPam,
                               const bool bForceJoinNext )
{
    bool bJoinText, bJoinPrev;
    ::sw_GetJoinFlags( rPam, bJoinText, bJoinPrev );
    // #i100466#
    if ( bForceJoinNext )
    {
        bJoinPrev = false;
    }

    {
        bool const bSuccess( DeleteRangeImpl( rPam ) );
        if (!bSuccess)
            return false;
    }

    if( bJoinText )
    {
        ::sw_JoinText( rPam, bJoinPrev );
    }

    if (!m_rDoc.getIDocumentRedlineAccess().IsIgnoreRedline()
        && !m_rDoc.getIDocumentRedlineAccess().GetRedlineTable().empty())
    {
        m_rDoc.getIDocumentRedlineAccess().CompressRedlines();
    }

    return true;
}

bool DocumentContentOperationsManager::DeleteRangeImpl(SwPaM & rPam, const bool)
{
    // Move all cursors out of the deleted range, but first copy the
    // passed PaM, because it could be a cursor that would be moved!
    SwPaM aDelPam( *rPam.GetMark(), *rPam.GetPoint() );
    ::PaMCorrAbs( aDelPam, *aDelPam.GetPoint() );

    bool const bSuccess( DeleteRangeImplImpl( aDelPam ) );
    if (bSuccess)
    {   // now copy position from temp copy to given PaM
        *rPam.GetPoint() = *aDelPam.GetPoint();
    }

    return bSuccess;
}

bool DocumentContentOperationsManager::DeleteRangeImplImpl(SwPaM & rPam)
{
    SwPosition *pStt = rPam.Start(), *pEnd = rPam.End();

    if (!rPam.HasMark()
        || (*pStt == *pEnd && !IsFlySelectedByCursor(m_rDoc, *pStt, *pEnd)))
    {
        return false;
    }

    if( m_rDoc.GetAutoCorrExceptWord() )
    {
        // if necessary the saved Word for the exception
        if( m_rDoc.GetAutoCorrExceptWord()->IsDeleted() ||  pStt->nNode != pEnd->nNode ||
            pStt->nContent.GetIndex() + 1 != pEnd->nContent.GetIndex() ||
            !m_rDoc.GetAutoCorrExceptWord()->CheckDelChar( *pStt ))
                { m_rDoc.DeleteAutoCorrExceptWord(); }
    }

    {
        // Delete all empty TextHints at the Mark's position
        SwTextNode* pTextNd = rPam.GetMark()->nNode.GetNode().GetTextNode();
        SwpHints* pHts;
        if( pTextNd &&  nullptr != ( pHts = pTextNd->GetpSwpHints()) && pHts->Count() )
        {
            const sal_Int32 nMkCntPos = rPam.GetMark()->nContent.GetIndex();
            for( size_t n = pHts->Count(); n; )
            {
                const SwTextAttr* pAttr = pHts->Get( --n );
                if( nMkCntPos > pAttr->GetStart() )
                    break;

                const sal_Int32 *pEndIdx;
                if( nMkCntPos == pAttr->GetStart() &&
                    nullptr != (pEndIdx = pAttr->End()) &&
                    *pEndIdx == pAttr->GetStart() )
                    pTextNd->DestroyAttr( pHts->Cut( n ) );
            }
        }
    }

    {
        // Send DataChanged before deletion, so that we still know
        // which objects are in the range.
        // Afterwards they could be before/after the Position.
        SwDataChanged aTmp( rPam );
    }

    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        m_rDoc.GetIDocumentUndoRedo().ClearRedo();
        bool bMerged(false);
        if (m_rDoc.GetIDocumentUndoRedo().DoesGroupUndo())
        {
            SwUndo *const pLastUndo( m_rDoc.GetUndoManager().GetLastUndo() );
            SwUndoDelete *const pUndoDelete(
                    dynamic_cast<SwUndoDelete *>(pLastUndo) );
            if (pUndoDelete)
            {
                bMerged = pUndoDelete->CanGrouping(m_rDoc, rPam);
                // if CanGrouping() returns true it's already merged
            }
        }
        if (!bMerged)
        {
            m_rDoc.GetIDocumentUndoRedo().AppendUndo( std::make_unique<SwUndoDelete>( rPam ) );
        }

        m_rDoc.getIDocumentState().SetModified();

        return true;
    }

    if( !m_rDoc.getIDocumentRedlineAccess().IsIgnoreRedline() && !m_rDoc.getIDocumentRedlineAccess().GetRedlineTable().empty() )
        m_rDoc.getIDocumentRedlineAccess().DeleteRedline( rPam, true, RedlineType::Any );

    // Delete and move all "Flys at the paragraph", which are within the Selection
    DelFlyInRange(rPam.GetMark()->nNode, rPam.GetPoint()->nNode,
        &rPam.GetMark()->nContent, &rPam.GetPoint()->nContent);
    DelBookmarks(
        pStt->nNode,
        pEnd->nNode,
        nullptr,
        &pStt->nContent,
        &pEnd->nContent);

    SwNodeIndex aSttIdx( pStt->nNode );
    SwContentNode * pCNd = aSttIdx.GetNode().GetContentNode();

    do {        // middle checked loop!
        if( pCNd )
        {
            SwTextNode * pStartTextNode( pCNd->GetTextNode() );
            if ( pStartTextNode )
            {
                // now move the Content to the new Node
                bool bOneNd = pStt->nNode == pEnd->nNode;
                const sal_Int32 nLen = ( bOneNd ? pEnd->nContent.GetIndex()
                                           : pCNd->Len() )
                                        - pStt->nContent.GetIndex();

                // Don't call again, if already empty
                if( nLen )
                {
                    pStartTextNode->EraseText( pStt->nContent, nLen );

                    if( !pStartTextNode->Len() )
                    {
                // METADATA: remove reference if empty (consider node deleted)
                        pStartTextNode->RemoveMetadataReference();
                    }
                }

                if( bOneNd )        // that's it
                    break;

                ++aSttIdx;
            }
            else
            {
                // So that there are no indices left registered when deleted,
                // we remove a SwPaM from the Content here.
                pStt->nContent.Assign( nullptr, 0 );
            }
        }

        pCNd = pEnd->nNode.GetNode().GetContentNode();
        if( pCNd )
        {
            SwTextNode * pEndTextNode( pCNd->GetTextNode() );
            if( pEndTextNode )
            {
                // if already empty, don't call again
                if( pEnd->nContent.GetIndex() )
                {
                    SwIndex aIdx( pCNd, 0 );
                    pEndTextNode->EraseText( aIdx, pEnd->nContent.GetIndex() );

                    if( !pEndTextNode->Len() )
                    {
                        // METADATA: remove reference if empty (consider node deleted)
                        pEndTextNode->RemoveMetadataReference();
                    }
                }
            }
            else
            {
                // So that there are no indices left registered when deleted,
                // we remove a SwPaM from the Content here.
                pEnd->nContent.Assign( nullptr, 0 );
            }
        }

        // if the end is not a content node, delete it as well
        sal_uInt32 nEnd = pEnd->nNode.GetIndex();
        if( pCNd == nullptr )
            nEnd++;

        if( aSttIdx != nEnd )
        {
            // tdf#134436 delete section nodes like SwUndoDelete::SwUndoDelete
            SwNode *pTmpNd;
            while (pEnd == rPam.GetPoint()
                && nEnd + 2 < m_rDoc.GetNodes().Count()
                && (pTmpNd = m_rDoc.GetNodes()[nEnd + 1])->IsEndNode()
                && pTmpNd->StartOfSectionNode()->IsSectionNode()
                && aSttIdx <= pTmpNd->StartOfSectionNode()->GetIndex())
            {
                SwNodeRange range(*pTmpNd->StartOfSectionNode(), *pTmpNd);
                m_rDoc.GetNodes().SectionUp(&range);
                --nEnd; // account for deleted start node
            }

            // delete the Nodes from the NodesArray
            m_rDoc.GetNodes().Delete( aSttIdx, nEnd - aSttIdx.GetIndex() );
        }

        // If the Node that contained the Cursor has been deleted,
        // the Content has to be assigned to the current Content.
        pStt->nContent.Assign( pStt->nNode.GetNode().GetContentNode(),
                                pStt->nContent.GetIndex() );

        // If we deleted across Node boundaries we have to correct the PaM,
        // because they are in different Nodes now.
        // Also, the Selection is revoked.
        *pEnd = *pStt;
        rPam.DeleteMark();

    } while( false );

    m_rDoc.getIDocumentState().SetModified();

    return true;
}

// It's possible to call Replace with a PaM that spans 2 paragraphs:
// search with regex for "$", then replace _all_
bool DocumentContentOperationsManager::ReplaceRangeImpl( SwPaM& rPam, const OUString& rStr,
        const bool bRegExReplace )
{
    if (!rPam.HasMark())
        return false;

    bool bJoinText, bJoinPrev;
    ::sw_GetJoinFlags( rPam, bJoinText, bJoinPrev );

    {
        // Create a copy of the Cursor in order to move all Pams from
        // the other views out of the deletion range.
        // Except for itself!
        SwPaM aDelPam( *rPam.GetMark(), *rPam.GetPoint() );
        ::PaMCorrAbs( aDelPam, *aDelPam.GetPoint() );

        SwPosition *pStt = aDelPam.Start(),
                   *pEnd = aDelPam.End();
        bool bOneNode = pStt->nNode == pEnd->nNode;

        // Own Undo?
        OUString sRepl( rStr );
        SwTextNode* pTextNd = pStt->nNode.GetNode().GetTextNode();
        sal_Int32 nStt = pStt->nContent.GetIndex();
        sal_Int32 nEnd;

        SwDataChanged aTmp( aDelPam );

        if( m_rDoc.getIDocumentRedlineAccess().IsRedlineOn() )
        {
            RedlineFlags eOld = m_rDoc.getIDocumentRedlineAccess().GetRedlineFlags();
            if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
            {
                // this should no longer happen in calls from the UI but maybe via API
                SAL_WARN_IF((eOld & RedlineFlags::ShowMask) != RedlineFlags::ShowMask,
                        "sw.core", "redlines will be moved in ReplaceRange");

                m_rDoc.GetIDocumentUndoRedo().StartUndo(SwUndoId::EMPTY, nullptr);

                // If any Redline will change (split!) the node
                const ::sw::mark::IMark* pBkmk =
                    m_rDoc.getIDocumentMarkAccess()->makeMark( aDelPam,
                        OUString(), IDocumentMarkAccess::MarkType::UNO_BOOKMARK,
                        ::sw::mark::InsertMode::New);

                m_rDoc.getIDocumentRedlineAccess().SetRedlineFlags(
                    RedlineFlags::On | RedlineFlags::ShowInsert | RedlineFlags::ShowDelete );

                *aDelPam.GetPoint() = pBkmk->GetMarkPos();
                if(pBkmk->IsExpanded())
                    *aDelPam.GetMark() = pBkmk->GetOtherMarkPos();
                m_rDoc.getIDocumentMarkAccess()->deleteMark(pBkmk);
                pStt = aDelPam.Start();
                pTextNd = pStt->nNode.GetNode().GetTextNode();
                nStt = pStt->nContent.GetIndex();
            }

            if( !sRepl.isEmpty() )
            {
                // Apply the first character's attributes to the ReplaceText
                SfxItemSet aSet( m_rDoc.GetAttrPool(),
                            svl::Items<RES_CHRATR_BEGIN,     RES_TXTATR_WITHEND_END - 1,
                            RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1>{} );
                pTextNd->GetParaAttr( aSet, nStt+1, nStt+1 );

                aSet.ClearItem( RES_TXTATR_REFMARK );
                aSet.ClearItem( RES_TXTATR_TOXMARK );
                aSet.ClearItem( RES_TXTATR_CJK_RUBY );
                aSet.ClearItem( RES_TXTATR_INETFMT );
                aSet.ClearItem( RES_TXTATR_META );
                aSet.ClearItem( RES_TXTATR_METAFIELD );

                if( aDelPam.GetPoint() != aDelPam.End() )
                    aDelPam.Exchange();

                // Remember the End
                SwNodeIndex aPtNd( aDelPam.GetPoint()->nNode, -1 );
                const sal_Int32 nPtCnt = aDelPam.GetPoint()->nContent.GetIndex();

                bool bFirst = true;
                OUString sIns;
                while ( lcl_GetTokenToParaBreak( sRepl, sIns, bRegExReplace ) )
                {
                    InsertString( aDelPam, sIns );
                    if( bFirst )
                    {
                        SwNodeIndex aMkNd( aDelPam.GetMark()->nNode, -1 );
                        const sal_Int32 nMkCnt = aDelPam.GetMark()->nContent.GetIndex();

                        SplitNode( *aDelPam.GetPoint(), false );

                        ++aMkNd;
                        aDelPam.GetMark()->nNode = aMkNd;
                        aDelPam.GetMark()->nContent.Assign(
                                    aMkNd.GetNode().GetContentNode(), nMkCnt );
                        bFirst = false;
                    }
                    else
                        SplitNode( *aDelPam.GetPoint(), false );
                }
                if( !sIns.isEmpty() )
                {
                    InsertString( aDelPam, sIns );
                }

                SwPaM aTmpRange( *aDelPam.GetPoint() );
                aTmpRange.SetMark();

                ++aPtNd;
                aDelPam.GetPoint()->nNode = aPtNd;
                aDelPam.GetPoint()->nContent.Assign( aPtNd.GetNode().GetContentNode(),
                                                    nPtCnt);
                *aTmpRange.GetMark() = *aDelPam.GetPoint();

                m_rDoc.RstTextAttrs( aTmpRange );
                InsertItemSet( aTmpRange, aSet );
            }

            if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
            {
                m_rDoc.GetIDocumentUndoRedo().AppendUndo(
                    std::make_unique<SwUndoRedlineDelete>( aDelPam, SwUndoId::REPLACE ));
            }
            m_rDoc.getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( RedlineType::Delete, aDelPam ), true);

            *rPam.GetMark() = *aDelPam.GetMark();
            if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
            {
                *aDelPam.GetPoint() = *rPam.GetPoint();
                m_rDoc.GetIDocumentUndoRedo().EndUndo(SwUndoId::EMPTY, nullptr);

                // If any Redline will change (split!) the node
                const ::sw::mark::IMark* pBkmk =
                    m_rDoc.getIDocumentMarkAccess()->makeMark( aDelPam,
                        OUString(), IDocumentMarkAccess::MarkType::UNO_BOOKMARK,
                        ::sw::mark::InsertMode::New);

                SwIndex& rIdx = aDelPam.GetPoint()->nContent;
                rIdx.Assign( nullptr, 0 );
                aDelPam.GetMark()->nContent = rIdx;
                rPam.GetPoint()->nNode = 0;
                rPam.GetPoint()->nContent = rIdx;
                *rPam.GetMark() = *rPam.GetPoint();
                m_rDoc.getIDocumentRedlineAccess().SetRedlineFlags( eOld );

                *rPam.GetPoint() = pBkmk->GetMarkPos();
                *rPam.GetMark() = pBkmk->IsExpanded() ? pBkmk->GetOtherMarkPos() : pBkmk->GetMarkPos();

                m_rDoc.getIDocumentMarkAccess()->deleteMark(pBkmk);
            }
            bJoinText = false;
        }
        else
        {
            assert((pStt->nNode == pEnd->nNode ||
                    ( pStt->nNode.GetIndex() + 1 == pEnd->nNode.GetIndex() &&
                        !pEnd->nContent.GetIndex() )) &&
                    "invalid range: Point and Mark on different nodes" );

            if( !m_rDoc.getIDocumentRedlineAccess().IsIgnoreRedline() && !m_rDoc.getIDocumentRedlineAccess().GetRedlineTable().empty() )
                m_rDoc.getIDocumentRedlineAccess().DeleteRedline( aDelPam, true, RedlineType::Any );

            SwUndoReplace* pUndoRpl = nullptr;
            bool const bDoesUndo = m_rDoc.GetIDocumentUndoRedo().DoesUndo();
            if (bDoesUndo)
            {
                pUndoRpl = new SwUndoReplace(aDelPam, sRepl, bRegExReplace);
                m_rDoc.GetIDocumentUndoRedo().AppendUndo(std::unique_ptr<SwUndo>(pUndoRpl));
            }
            ::sw::UndoGuard const undoGuard(m_rDoc.GetIDocumentUndoRedo());

            if( aDelPam.GetPoint() != pStt )
                aDelPam.Exchange();

            SwNodeIndex aPtNd( pStt->nNode, -1 );
            const sal_Int32 nPtCnt = pStt->nContent.GetIndex();

            // Set the values again, if Frames or footnotes on the Text have been removed.
            nStt = nPtCnt;
            nEnd = bOneNode ? pEnd->nContent.GetIndex()
                            : pTextNd->GetText().getLength();

            bool bFirst = true;
            OUString sIns;
            while ( lcl_GetTokenToParaBreak( sRepl, sIns, bRegExReplace ) )
            {
                if (!bFirst || nStt == pTextNd->GetText().getLength())
                {
                    InsertString( aDelPam, sIns );
                }
                else if( nStt < nEnd || !sIns.isEmpty() )
                {
                    pTextNd->ReplaceText( pStt->nContent, nEnd - nStt, sIns );
                }
                SplitNode( *pStt, false);
                bFirst = false;
            }

            if( bFirst || !sIns.isEmpty() )
            {
                if (!bFirst || nStt == pTextNd->GetText().getLength())
                {
                    InsertString( aDelPam, sIns );
                }
                else if( nStt < nEnd || !sIns.isEmpty() )
                {
                    pTextNd->ReplaceText( pStt->nContent, nEnd - nStt, sIns );
                }
            }

            *rPam.GetPoint() = *aDelPam.GetMark();
            ++aPtNd;
            rPam.GetMark()->nNode = aPtNd;
            rPam.GetMark()->nContent.Assign( aPtNd.GetNode().GetContentNode(),
                                                nPtCnt );

            if (bJoinText)
            {
                assert(rPam.GetPoint() == rPam.End());
                // move so that SetEnd remembers position after sw_JoinText
                rPam.Move(fnMoveBackward);
            }
            else if (aDelPam.GetPoint() == pStt) // backward selection?
            {
                assert(*rPam.GetMark() <= *rPam.GetPoint());
                rPam.Exchange(); // swap so that rPam is backwards
            }

            if( pUndoRpl )
            {
                pUndoRpl->SetEnd(rPam);
            }
        }
    }

    bool bRet(true);
    if (bJoinText)
    {
        bRet = ::sw_JoinText(rPam, bJoinPrev);
    }

    m_rDoc.getIDocumentState().SetModified();
    return bRet;
}

SwFlyFrameFormat* DocumentContentOperationsManager::InsNoTextNode( const SwPosition& rPos, SwNoTextNode* pNode,
                                    const SfxItemSet* pFlyAttrSet,
                                    const SfxItemSet* pGrfAttrSet,
                                    SwFrameFormat* pFrameFormat)
{
    SwFlyFrameFormat *pFormat = nullptr;
    if( pNode )
    {
        pFormat = m_rDoc.MakeFlySection_( rPos, *pNode, RndStdIds::FLY_AT_PARA,
                                pFlyAttrSet, pFrameFormat );
        if( pGrfAttrSet )
            pNode->SetAttr( *pGrfAttrSet );
    }
    return pFormat;
}

#define NUMRULE_STATE \
    std::shared_ptr<SwNumRuleItem> aNumRuleItemHolderIfSet; \
    std::shared_ptr<SfxStringItem> aListIdItemHolderIfSet; \

#define PUSH_NUMRULE_STATE \
     lcl_PushNumruleState( aNumRuleItemHolderIfSet, aListIdItemHolderIfSet, pDestTextNd );

#define POP_NUMRULE_STATE \
     lcl_PopNumruleState( aNumRuleItemHolderIfSet, aListIdItemHolderIfSet, pDestTextNd, rPam );

static void lcl_PushNumruleState(
    std::shared_ptr<SwNumRuleItem>& aNumRuleItemHolderIfSet,
    std::shared_ptr<SfxStringItem>& aListIdItemHolderIfSet,
    const SwTextNode *pDestTextNd )
{
    // Safe numrule item at destination.
    // #i86492# - Safe also <ListId> item of destination.
    const SfxItemSet * pAttrSet = pDestTextNd->GetpSwAttrSet();
    if (pAttrSet == nullptr)
        return;

    const SfxPoolItem* pItem(nullptr);
    if (SfxItemState::SET == pAttrSet->GetItemState(RES_PARATR_NUMRULE, false, &pItem))
    {
        aNumRuleItemHolderIfSet.reset(&pItem->Clone()->StaticWhichCast(RES_PARATR_NUMRULE));
    }

    if (SfxItemState::SET == pAttrSet->GetItemState(RES_PARATR_LIST_ID, false, &pItem))
    {
        aListIdItemHolderIfSet.reset(&pItem->Clone()->StaticWhichCast(RES_PARATR_LIST_ID));
    }
}

static void lcl_PopNumruleState(
    std::shared_ptr<SwNumRuleItem>& aNumRuleItemHolderIfSet,
    std::shared_ptr<SfxStringItem>& aListIdItemHolderIfSet,
    SwTextNode *pDestTextNd, const SwPaM& rPam )
{
    /* If only a part of one paragraph is copied
       restore the numrule at the destination. */
    // #i86492# - restore also <ListId> item
    if ( lcl_MarksWholeNode(rPam) )
        return;

    if (aNumRuleItemHolderIfSet)
    {
        pDestTextNd->SetAttr(*aNumRuleItemHolderIfSet);
    }
    else
    {
        pDestTextNd->ResetAttr(RES_PARATR_NUMRULE);
    }

    if (aListIdItemHolderIfSet)
    {
        pDestTextNd->SetAttr(*aListIdItemHolderIfSet);
    }
    else
    {
        pDestTextNd->ResetAttr(RES_PARATR_LIST_ID);
    }
}

bool DocumentContentOperationsManager::CopyImpl(SwPaM& rPam, SwPosition& rPos,
        SwCopyFlags const flags,
        SwPaM *const pCopyRange) const
{
    std::vector<std::pair<sal_uLong, sal_Int32>> Breaks;

    sw::CalcBreaks(Breaks, rPam, true);

    if (Breaks.empty())
    {
        return CopyImplImpl(rPam, rPos, flags, pCopyRange);
    }

    SwPosition const & rSelectionEnd( *rPam.End() );

    bool bRet(true);
    bool bFirst(true);
    // iterate from end to start, ... don't think it's necessary here?
    auto iter( Breaks.rbegin() );
    sal_uLong nOffset(0);
    SwNodes const& rNodes(rPam.GetPoint()->nNode.GetNodes());
    SwPaM aPam( rSelectionEnd, rSelectionEnd ); // end node!
    SwPosition & rEnd( *aPam.End() );
    SwPosition & rStart( *aPam.Start() );
    SwPaM copyRange(rPos, rPos);

    while (iter != Breaks.rend())
    {
        rStart = SwPosition(*rNodes[iter->first - nOffset]->GetTextNode(), iter->second + 1);
        if (rStart < rEnd) // check if part is empty
        {
            // pass in copyRange member as rPos; should work ...
            bRet &= CopyImplImpl(aPam, *copyRange.Start(), flags & ~SwCopyFlags::IsMoveToFly, &copyRange);
            nOffset = iter->first - rStart.nNode.GetIndex(); // fly nodes...
            if (pCopyRange)
            {
                if (bFirst)
                {
                    pCopyRange->SetMark();
                    *pCopyRange->GetMark() = *copyRange.End();
                }
                *pCopyRange->GetPoint() = *copyRange.Start();
            }
            bFirst = false;
        }
        rEnd = SwPosition(*rNodes[iter->first - nOffset]->GetTextNode(), iter->second);
        ++iter;
    }

    rStart = *rPam.Start(); // set to original start
    if (rStart < rEnd) // check if part is empty
    {
        bRet &= CopyImplImpl(aPam, *copyRange.Start(), flags & ~SwCopyFlags::IsMoveToFly, &copyRange);
        if (pCopyRange)
        {
            if (bFirst)
            {
                pCopyRange->SetMark();
                *pCopyRange->GetMark() = *copyRange.End();
            }
            *pCopyRange->GetPoint() = *copyRange.Start();
        }
    }

    return bRet;
}

bool DocumentContentOperationsManager::CopyImplImpl(SwPaM& rPam, SwPosition& rPos,
        SwCopyFlags const flags,
        SwPaM *const pCpyRange) const
{
    SwDoc& rDoc = rPos.nNode.GetNode().GetDoc();
    const bool bColumnSel = rDoc.IsClipBoard() && rDoc.IsColumnSelection();

    SwPosition const*const pStt = rPam.Start();
    SwPosition *const pEnd = rPam.End();

    // Catch when there's no copy to do.
    if (!rPam.HasMark() || (IsEmptyRange(*pStt, *pEnd, flags) && !bColumnSel) ||
        //JP 29.6.2001: 88963 - don't copy if inspos is in region of start to end
        //JP 15.11.2001: don't test inclusive the end, ever exclusive
        ( &rDoc == &m_rDoc && *pStt <= rPos && rPos < *pEnd ))
    {
        return false;
    }

    const bool bEndEqualIns = &rDoc == &m_rDoc && rPos == *pEnd;

    // If Undo is enabled, create the UndoCopy object
    SwUndoCpyDoc* pUndo = nullptr;
    // lcl_DeleteRedlines may delete the start or end node of the cursor when
    // removing the redlines so use cursor that is corrected by PaMCorrAbs
    std::shared_ptr<SwUnoCursor> const pCopyPam(rDoc.CreateUnoCursor(rPos));

    SwTableNumFormatMerge aTNFM( m_rDoc, rDoc );
    std::optional<std::vector<SwFrameFormat*>> pFlys;
    std::vector<SwFrameFormat*> const* pFlysAtInsPos;

    if (rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoCpyDoc(*pCopyPam);
        pFlysAtInsPos = pUndo->GetFlysAnchoredAt();
    }
    else
    {
        pFlys = sw::GetFlysAnchoredAt(rDoc, rPos.nNode.GetIndex());
        pFlysAtInsPos = pFlys ? &*pFlys : nullptr;
    }

    RedlineFlags eOld = rDoc.getIDocumentRedlineAccess().GetRedlineFlags();
    rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern(eOld | RedlineFlags::Ignore);

    // Move the PaM one node back from the insert position, so that
    // the position doesn't get moved
    pCopyPam->SetMark();
    bool bCanMoveBack = pCopyPam->Move(fnMoveBackward, GoInContent);
    // If the position was shifted from more than one node, an end node has been skipped
    bool bAfterTable = false;
    if ((rPos.nNode.GetIndex() - pCopyPam->GetPoint()->nNode.GetIndex()) > 1)
    {
        // First go back to the original place
        pCopyPam->GetPoint()->nNode = rPos.nNode;
        pCopyPam->GetPoint()->nContent = rPos.nContent;

        bCanMoveBack = false;
        bAfterTable = true;
    }
    if( !bCanMoveBack )
    {
        pCopyPam->GetPoint()->nNode--;
        assert(pCopyPam->GetPoint()->nContent.GetIndex() == 0);
    }

    SwNodeRange aRg( pStt->nNode, pEnd->nNode );
    SwNodeIndex aInsPos( rPos.nNode );
    const bool bOneNode = pStt->nNode == pEnd->nNode;
    SwTextNode* pSttTextNd = pStt->nNode.GetNode().GetTextNode();
    SwTextNode* pEndTextNd = pEnd->nNode.GetNode().GetTextNode();
    SwTextNode* pDestTextNd = aInsPos.GetNode().GetTextNode();
    bool bCopyCollFormat = !rDoc.IsInsOnlyTextGlossary() &&
                        ( (pDestTextNd && !pDestTextNd->GetText().getLength()) ||
                          ( !bOneNode && !rPos.nContent.GetIndex() ) );
    bool bCopyBookmarks = true;
    bool bCopyPageSource  = false;
    int nDeleteTextNodes = 0;

    // #i104585# copy outline num rule to clipboard (for ASCII filter)
    if (rDoc.IsClipBoard() && m_rDoc.GetOutlineNumRule())
    {
        rDoc.SetOutlineNumRule(*m_rDoc.GetOutlineNumRule());
    }

    // #i86492#
    // Correct the search for a previous list:
    // First search for non-outline numbering list. Then search for non-outline
    // bullet list.
    // Keep also the <ListId> value for possible propagation.
    OUString aListIdToPropagate;
    const SwNumRule* pNumRuleToPropagate =
        rDoc.SearchNumRule( rPos, false, true, false, 0, aListIdToPropagate, nullptr, true );
    if ( !pNumRuleToPropagate )
    {
        pNumRuleToPropagate =
            rDoc.SearchNumRule( rPos, false, false, false, 0, aListIdToPropagate, nullptr, true );
    }
    // #i86492#
    // Do not propagate previous found list, if
    // - destination is an empty paragraph which is not in a list and
    // - source contains at least one paragraph which is not in a list
    if ( pNumRuleToPropagate &&
         pDestTextNd && !pDestTextNd->GetText().getLength() &&
         !pDestTextNd->IsInList() &&
         !lcl_ContainsOnlyParagraphsInList( rPam ) )
    {
        pNumRuleToPropagate = nullptr;
    }

    // This do/while block is only there so that we can break out of it!
    do {
        if( pSttTextNd )
        {
            ++nDeleteTextNodes; // must be joined in Undo
            // Don't copy the beginning completely?
            if( !bCopyCollFormat || bColumnSel || pStt->nContent.GetIndex() )
            {
                SwIndex aDestIdx( rPos.nContent );
                bool bCopyOk = false;
                if( !pDestTextNd )
                {
                    if( pStt->nContent.GetIndex() || bOneNode )
                        pDestTextNd = rDoc.GetNodes().MakeTextNode( aInsPos,
                            rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD));
                    else
                    {
                        pDestTextNd = pSttTextNd->MakeCopy(rDoc, aInsPos, true)->GetTextNode();
                        bCopyOk = true;
                    }
                    aDestIdx.Assign( pDestTextNd, 0 );
                    bCopyCollFormat = true;
                }
                else if( !bOneNode || bColumnSel )
                {
                    const sal_Int32 nContentEnd = pEnd->nContent.GetIndex();
                    {
                        ::sw::UndoGuard const ug(rDoc.GetIDocumentUndoRedo());
                        rDoc.getIDocumentContentOperations().SplitNode( rPos, false );
                    }

                    if (bCanMoveBack && rPos == *pCopyPam->GetPoint())
                    {
                        // after the SplitNode, span the CpyPam correctly again
                        pCopyPam->Move( fnMoveBackward, GoInContent );
                        pCopyPam->Move( fnMoveBackward, GoInContent );
                    }

                    pDestTextNd = rDoc.GetNodes()[ aInsPos.GetIndex()-1 ]->GetTextNode();
                    aDestIdx.Assign(
                            pDestTextNd, pDestTextNd->GetText().getLength());

                    // Correct the area again
                    if( bEndEqualIns )
                    {
                        bool bChg = pEnd != rPam.GetPoint();
                        if( bChg )
                            rPam.Exchange();
                        rPam.Move( fnMoveBackward, GoInContent );
                        if( bChg )
                            rPam.Exchange();
                    }
                    else if( rPos == *pEnd )
                    {
                        // The end was also moved
                        pEnd->nNode--;
                        pEnd->nContent.Assign( pDestTextNd, nContentEnd );
                    }
                    // tdf#63022 always reset pEndTextNd after SplitNode
                    aRg.aEnd = pEnd->nNode;
                    pEndTextNd = pEnd->nNode.GetNode().GetTextNode();
                }

                NUMRULE_STATE
                if( bCopyCollFormat && bOneNode )
                {
                    PUSH_NUMRULE_STATE
                }

                if( !bCopyOk )
                {
                    const sal_Int32 nCpyLen = ( bOneNode
                                           ? pEnd->nContent.GetIndex()
                                           : pSttTextNd->GetText().getLength())
                                         - pStt->nContent.GetIndex();
                    pSttTextNd->CopyText( pDestTextNd, aDestIdx,
                                            pStt->nContent, nCpyLen );
                    if( bEndEqualIns )
                        pEnd->nContent -= nCpyLen;
                }

                aRg.aStart++;

                if( bOneNode )
                {
                    if (bCopyCollFormat)
                    {
                        // tdf#138897 no Undo for applying style, SwUndoInserts does it
                        pSttTextNd->CopyCollFormat(*pDestTextNd, false);
                        POP_NUMRULE_STATE
                    }

                    // copy at-char flys in rPam
                    SwNodeIndex temp(*pDestTextNd); // update to new (start) node for flys
                    // tdf#126626 prevent duplicate Undos
                    ::sw::UndoGuard const ug(rDoc.GetIDocumentUndoRedo());
                    CopyFlyInFlyImpl(aRg, &rPam, temp, false);

                    break;
                }
            }
        }
        else if( pDestTextNd )
        {
            // Problems with insertion of table selections into "normal" text solved.
            // We have to set the correct PaM for Undo, if this PaM starts in a textnode,
            // the undo operation will try to merge this node after removing the table.
            // If we didn't split a textnode, the PaM should start at the inserted table node
            if( rPos.nContent.GetIndex() == pDestTextNd->Len() )
            {    // Insertion at the last position of a textnode (empty or not)
                ++aInsPos; // The table will be inserted behind the text node
            }
            else if( rPos.nContent.GetIndex() )
            {   // Insertion in the middle of a text node, it has to be split
                // (and joined from undo)
                ++nDeleteTextNodes;

                const sal_Int32 nContentEnd = pEnd->nContent.GetIndex();
                {
                    ::sw::UndoGuard const ug(rDoc.GetIDocumentUndoRedo());
                    rDoc.getIDocumentContentOperations().SplitNode( rPos, false );
                }

                if (bCanMoveBack && rPos == *pCopyPam->GetPoint())
                {
                    // after the SplitNode, span the CpyPam correctly again
                    pCopyPam->Move( fnMoveBackward, GoInContent );
                    pCopyPam->Move( fnMoveBackward, GoInContent );
                }

                // Correct the area again
                if( bEndEqualIns )
                    aRg.aEnd--;
                // The end would also be moved
                else if( rPos == *pEnd )
                {
                    rPos.nNode-=2;
                    rPos.nContent.Assign( rPos.nNode.GetNode().GetContentNode(),
                                            nContentEnd );
                    rPos.nNode++;
                    aRg.aEnd--;
                }
            }
            else if( bCanMoveBack )
            {   // Insertion at the first position of a text node. It will not be split, the table
                // will be inserted before the text node.
                // See below, before the SetInsertRange function of the undo object will be called,
                // the CpyPam would be moved to the next content position. This has to be avoided
                // We want to be moved to the table node itself thus we have to set bCanMoveBack
                // and to manipulate pCopyPam.
                bCanMoveBack = false;
                pCopyPam->GetPoint()->nNode--;
            }
        }

        pDestTextNd = aInsPos.GetNode().GetTextNode();
        if (pEndTextNd)
        {
            SwIndex aDestIdx( rPos.nContent );
            if( !pDestTextNd )
            {
                pDestTextNd = rDoc.GetNodes().MakeTextNode( aInsPos,
                            rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD));
                aDestIdx.Assign( pDestTextNd, 0  );
                aInsPos--;

                // if we have to insert an extra text node
                // at the destination, this node will be our new destination
                // (text) node, and thus we increment nDeleteTextNodes. This
                // will ensure that this node will be deleted during Undo.
                ++nDeleteTextNodes; // must be deleted
            }

            const bool bEmptyDestNd = pDestTextNd->GetText().isEmpty();

            NUMRULE_STATE
            if( bCopyCollFormat && ( bOneNode || bEmptyDestNd ))
            {
                PUSH_NUMRULE_STATE
            }

            pEndTextNd->CopyText( pDestTextNd, aDestIdx, SwIndex( pEndTextNd ),
                            pEnd->nContent.GetIndex() );

            // Also copy all format templates
            if( bCopyCollFormat && ( bOneNode || bEmptyDestNd ))
            {
                // tdf#138897 no Undo for applying style, SwUndoInserts does it
                pEndTextNd->CopyCollFormat(*pDestTextNd, false);
                if ( bOneNode )
                {
                    POP_NUMRULE_STATE
                }
            }
        }

        SfxItemSet aBrkSet( rDoc.GetAttrPool(), aBreakSetRange );
        if ((flags & SwCopyFlags::CopyAll) || aRg.aStart != aRg.aEnd)
        {
            if (pSttTextNd && bCopyCollFormat && pDestTextNd->HasSwAttrSet())
            {
                aBrkSet.Put( *pDestTextNd->GetpSwAttrSet() );
                if( SfxItemState::SET == aBrkSet.GetItemState( RES_BREAK, false ) )
                    pDestTextNd->ResetAttr( RES_BREAK );
                if( SfxItemState::SET == aBrkSet.GetItemState( RES_PAGEDESC, false ) )
                    pDestTextNd->ResetAttr( RES_PAGEDESC );
            }
        }

        {
            SwPosition startPos(SwNodeIndex(pCopyPam->GetPoint()->nNode, +1),
                SwIndex(SwNodeIndex(pCopyPam->GetPoint()->nNode, +1).GetNode().GetContentNode()));
            if (bCanMoveBack)
            {   // pCopyPam is actually 1 before the copy range so move it fwd
                SwPaM temp(*pCopyPam->GetPoint());
                temp.Move(fnMoveForward, GoInContent);
                startPos = *temp.GetPoint();
            }
            assert(startPos.nNode.GetNode().IsContentNode());
            std::pair<SwPaM const&, SwPosition const&> tmp(rPam, startPos);
            if( aInsPos == pEnd->nNode )
            {
                SwNodeIndex aSaveIdx( aInsPos, -1 );
                assert(pStt->nNode != pEnd->nNode);
                pEnd->nContent = 0; // TODO why this?
                CopyWithFlyInFly(aRg, aInsPos, &tmp, /*bMakeNewFrames*/true, false, /*bCopyFlyAtFly=*/false, flags);
                ++aSaveIdx;
                pEnd->nNode = aSaveIdx;
                pEnd->nContent.Assign( aSaveIdx.GetNode().GetTextNode(), 0 );
            }
            else
                CopyWithFlyInFly(aRg, aInsPos, &tmp, /*bMakeNewFrames*/true, false, /*bCopyFlyAtFly=*/false, flags);

            bCopyBookmarks = false;
        }

        // at-char anchors post SplitNode are on index 0 of 2nd node and will
        // remain there - move them back to the start (end would also work?)
        // ... also for at-para anchors; here start is preferable because
        // it's consistent with SplitNode from SwUndoInserts::RedoImpl()
        if (pFlysAtInsPos)
        {
            // init *again* - because CopyWithFlyInFly moved startPos
            SwPosition startPos(SwNodeIndex(pCopyPam->GetPoint()->nNode, +1),
                SwIndex(SwNodeIndex(pCopyPam->GetPoint()->nNode, +1).GetNode().GetContentNode()));
            if (bCanMoveBack)
            {   // pCopyPam is actually 1 before the copy range so move it fwd
                SwPaM temp(*pCopyPam->GetPoint());
                temp.Move(fnMoveForward, GoInContent);
                startPos = *temp.GetPoint();
            }
            assert(startPos.nNode.GetNode().IsContentNode());
            SwPosition startPosAtPara(startPos);
            startPosAtPara.nContent.Assign(nullptr, 0);

            for (SwFrameFormat * pFly : *pFlysAtInsPos)
            {
                SwFormatAnchor const*const pAnchor = &pFly->GetAnchor();
                if (pAnchor->GetAnchorId() == RndStdIds::FLY_AT_CHAR)
                {
                    SwFormatAnchor anchor(*pAnchor);
                    anchor.SetAnchor( &startPos );
                    pFly->SetFormatAttr(anchor);
                }
                else if (pAnchor->GetAnchorId() == RndStdIds::FLY_AT_PARA)
                {
                    SwFormatAnchor anchor(*pAnchor);
                    anchor.SetAnchor( &startPosAtPara );
                    pFly->SetFormatAttr(anchor);
                }
            }
        }

        if ((flags & SwCopyFlags::CopyAll) || aRg.aStart != aRg.aEnd)
        {
            // Put the breaks back into the first node
            if( aBrkSet.Count() && nullptr != ( pDestTextNd = rDoc.GetNodes()[
                    pCopyPam->GetPoint()->nNode.GetIndex()+1 ]->GetTextNode()))
            {
                pDestTextNd->SetAttr( aBrkSet );
                bCopyPageSource = true;
            }
        }
    } while( false );


    // it is not possible to make this test when copy from the clipBoard to document
    //  in this case the PageNum not exist anymore
    // tdf#39400 and tdf#97526
    // when copy from document to ClipBoard, and it is from the first page
    //  and not the source has the page break
    if (rDoc.IsClipBoard() && (rPam.GetPageNum(pStt == rPam.GetPoint()) == 1) && !bCopyPageSource)
    {
        if (pDestTextNd)
        {
            pDestTextNd->ResetAttr(RES_BREAK);        // remove the page-break
            pDestTextNd->ResetAttr(RES_PAGEDESC);
        }
    }


    // Adjust position (in case it was moved / in another node)
    rPos.nContent.Assign( rPos.nNode.GetNode().GetContentNode(),
                            rPos.nContent.GetIndex() );

    if( rPos.nNode != aInsPos )
    {
        pCopyPam->GetMark()->nNode = aInsPos;
        if (aInsPos < rPos.nNode)
        {   // tdf#134250 decremented in (pEndTextNd && !pDestTextNd) above
            pCopyPam->GetContentNode(false)->MakeEndIndex(&pCopyPam->GetMark()->nContent);
        }
        else // incremented in (!pSttTextNd && pDestTextNd) above
        {
            pCopyPam->GetMark()->nContent.Assign(pCopyPam->GetContentNode(false), 0);
        }
        rPos = *pCopyPam->GetMark();
    }
    else
        *pCopyPam->GetMark() = rPos;

    if ( !bAfterTable )
        pCopyPam->Move( fnMoveForward, bCanMoveBack ? GoInContent : GoInNode );
    else
    {
        pCopyPam->GetPoint()->nNode++;

        // Reset the offset to 0 as it was before the insertion
        pCopyPam->GetPoint()->nContent.Assign(pCopyPam->GetPoint()->nNode.GetNode().GetContentNode(), 0);
        // If the next node is a start node, then step back: the start node
        // has been copied and needs to be in the selection for the undo
        if (pCopyPam->GetPoint()->nNode.GetNode().IsStartNode())
            pCopyPam->GetPoint()->nNode--;

    }
    pCopyPam->Exchange();

    // Also copy all bookmarks
    if( bCopyBookmarks && m_rDoc.getIDocumentMarkAccess()->getAllMarksCount() )
    {
        sw::CopyBookmarks(rPam, *pCopyPam->Start());
    }

    if( RedlineFlags::DeleteRedlines & eOld )
    {
        assert(*pCopyPam->GetPoint() == rPos);
        // the Node rPos points to may be deleted so unregister ...
        rPos.nContent.Assign(nullptr, 0);
        lcl_DeleteRedlines(rPam, *pCopyPam);
        rPos = *pCopyPam->GetPoint(); // ... and restore.
    }

    // If Undo is enabled, store the inserted area
    if (rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        // append it after styles have been copied when copying nodes
        rDoc.GetIDocumentUndoRedo().AppendUndo( std::unique_ptr<SwUndo>(pUndo) );
        pUndo->SetInsertRange(*pCopyPam, true, nDeleteTextNodes);
    }

    if( pCpyRange )
    {
        pCpyRange->SetMark();
        *pCpyRange->GetPoint() = *pCopyPam->GetPoint();
        *pCpyRange->GetMark() = *pCopyPam->GetMark();
    }

    if ( pNumRuleToPropagate != nullptr )
    {
        // #i86492# - use <SwDoc::SetNumRule(..)>, because it also handles the <ListId>
        // Don't reset indent attributes, that would mean loss of direct
        // formatting.
        rDoc.SetNumRule( *pCopyPam, *pNumRuleToPropagate, false, nullptr,
                          aListIdToPropagate, true, /*bResetIndentAttrs=*/false );
    }

    rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
    rDoc.getIDocumentState().SetModified();

    return true;
}


}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
