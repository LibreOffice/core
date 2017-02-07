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
#include <DocumentRedlineManager.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
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
#include <fmtcol.hxx>
#include <breakit.hxx>
#include <frmfmt.hxx>
#include <fmtanchr.hxx>
#include <fmtcntnt.hxx>
#include <fmtinfmt.hxx>
#include <fmtpdsc.hxx>
#include <fmtcnct.hxx>
#include <SwStyleNameMapper.hxx>
#include <redline.hxx>
#include <unocrsr.hxx>
#include <mvsave.hxx>
#include <ndtxt.hxx>
#include <poolfmt.hxx>
#include <paratr.hxx>
#include <txatbase.hxx>
#include <UndoRedline.hxx>
#include <undobj.hxx>
#include <UndoDelete.hxx>
#include <UndoSplitMove.hxx>
#include <UndoOverwrite.hxx>
#include <UndoInsert.hxx>
#include <UndoAttribute.hxx>
#include <rolbck.hxx>
#include <acorrect.hxx>
#include <ftnidx.hxx>
#include <txtftn.hxx>
#include <hints.hxx>
#include <crsrsh.hxx>
#include <fmtflcnt.hxx>
#include <docedt.hxx>
#include <unotools/charclass.hxx>
#include <sfx2/Metadatable.hxx>
#include <svl/stritem.hxx>
#include <svl/itemiter.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdouno.hxx>
#include <tools/globname.hxx>
#include <editeng/formatbreakitem.hxx>
#include <com/sun/star/i18n/Boundary.hpp>
#include <memory>


using namespace ::com::sun::star::i18n;

namespace
{
    // Copy method from SwDoc
    // Prevent copying in Flys that are anchored in the area
    bool lcl_ChkFlyFly( SwDoc* pDoc, sal_uLong nSttNd, sal_uLong nEndNd,
                        sal_uLong nInsNd )
    {
        const SwFrameFormats& rFrameFormatTable = *pDoc->GetSpzFrameFormats();

        for( size_t n = 0; n < rFrameFormatTable.size(); ++n )
        {
            SwFrameFormat const*const  pFormat = rFrameFormatTable[n];
            SwFormatAnchor const*const pAnchor = &pFormat->GetAnchor();
            SwPosition const*const pAPos = pAnchor->GetContentAnchor();
            if (pAPos &&
                ((FLY_AS_CHAR == pAnchor->GetAnchorId()) ||
                 (FLY_AT_CHAR == pAnchor->GetAnchorId()) ||
                 (FLY_AT_FLY  == pAnchor->GetAnchorId()) ||
                 (FLY_AT_PARA == pAnchor->GetAnchorId())) &&
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

                if( lcl_ChkFlyFly( pDoc, pSNd->GetIndex(),
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
        if (rSourcePaM.GetDoc()->GetNodes().GetEndOfExtras().GetIndex() + 1
                == rStart.GetIndex())
        {
            rDelCount = 1;
            return SwNodeIndex(rStart, +1);
        }
        else
        {
            rDelCount = 0;
            return SwNodeIndex(rStart);
        }
    }

    /*
        The lcl_CopyBookmarks function has to copy bookmarks from the source to the destination nodes
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

    // TODO: use SaveBookmark (from DelBookmarks)
    void lcl_CopyBookmarks(
        const SwPaM& rPam,
        SwPaM& rCpyPam )
    {
        const SwDoc* pSrcDoc = rPam.GetDoc();
        SwDoc* pDestDoc =  rCpyPam.GetDoc();
        const IDocumentMarkAccess* const pSrcMarkAccess = pSrcDoc->getIDocumentMarkAccess();
        ::sw::UndoGuard const undoGuard(pDestDoc->GetIDocumentUndoRedo());

        const SwPosition &rStt = *rPam.Start(), &rEnd = *rPam.End();
        SwPosition* pCpyStt = rCpyPam.Start();

        typedef std::vector< const ::sw::mark::IMark* > mark_vector_t;
        mark_vector_t vMarksToCopy;
        for ( IDocumentMarkAccess::const_iterator_t ppMark = pSrcMarkAccess->getAllMarksBegin();
              ppMark != pSrcMarkAccess->getAllMarksEnd();
              ++ppMark )
        {
            const ::sw::mark::IMark* const pMark = ppMark->get();

            const SwPosition& rMarkStart = pMark->GetMarkStart();
            const SwPosition& rMarkEnd = pMark->GetMarkEnd();
            // only include marks that are in the range and not touching both start and end
            // - not for annotation or checkbox marks.
            const bool bIsNotOnBoundary =
                pMark->IsExpanded()
                ? (rMarkStart != rStt || rMarkEnd != rEnd)  // rMarkStart != rMarkEnd
                : (rMarkStart != rStt && rMarkEnd != rEnd); // rMarkStart == rMarkEnd
            const IDocumentMarkAccess::MarkType aMarkType = IDocumentMarkAccess::GetType(*pMark);
            if ( rMarkStart >= rStt && rMarkEnd <= rEnd
                 && ( bIsNotOnBoundary
                      || aMarkType == IDocumentMarkAccess::MarkType::ANNOTATIONMARK
                      || aMarkType == IDocumentMarkAccess::MarkType::CHECKBOX_FIELDMARK ) )
            {
                vMarksToCopy.push_back(pMark);
            }
        }
        // We have to count the "non-copied" nodes..
        sal_uLong nDelCount;
        SwNodeIndex aCorrIdx(InitDelCount(rPam, nDelCount));
        for(mark_vector_t::const_iterator ppMark = vMarksToCopy.begin();
            ppMark != vMarksToCopy.end();
            ++ppMark)
        {
            const ::sw::mark::IMark* const pMark = *ppMark;
            SwPaM aTmpPam(*pCpyStt);
            lcl_NonCopyCount(rPam, aCorrIdx, pMark->GetMarkPos().nNode.GetIndex(), nDelCount);
            lcl_SetCpyPos( pMark->GetMarkPos(), rStt, *pCpyStt, *aTmpPam.GetPoint(), nDelCount);
            if(pMark->IsExpanded())
            {
                aTmpPam.SetMark();
                lcl_NonCopyCount(rPam, aCorrIdx, pMark->GetOtherMarkPos().nNode.GetIndex(), nDelCount);
                lcl_SetCpyPos(pMark->GetOtherMarkPos(), rStt, *pCpyStt, *aTmpPam.GetMark(), nDelCount);
            }

            const IDocumentMarkAccess::MarkType aMarkType = IDocumentMarkAccess::GetType(*pMark);
            if (aMarkType == IDocumentMarkAccess::MarkType::CHECKBOX_FIELDMARK)
            {
                // Node's CopyText() copies also dummy characters, which need to be removed
                // (they will be added later in MarkBase::InitDoc inside IDocumentMarkAccess::makeMark)
                // CHECKBOX_FIELDMARK doesn't contain any other data in its range, so just clear it
                pDestDoc->getIDocumentContentOperations().DeleteRange(aTmpPam);
            }

            ::sw::mark::IMark* const pNewMark = pDestDoc->getIDocumentMarkAccess()->makeMark(
                aTmpPam,
                pMark->GetName(),
                IDocumentMarkAccess::GetType(*pMark));
            // Explicitly try to get exactly the same name as in the source
            // because NavigatorReminders, DdeBookmarks etc. ignore the proposed name
            pDestDoc->getIDocumentMarkAccess()->renameMark(pNewMark, pMark->GetName());

            // copying additional attributes for bookmarks or fieldmarks
            ::sw::mark::IBookmark* const pNewBookmark =
                dynamic_cast< ::sw::mark::IBookmark* const >(pNewMark);
            const ::sw::mark::IBookmark* const pOldBookmark =
                dynamic_cast< const ::sw::mark::IBookmark* >(pMark);
            if (pNewBookmark && pOldBookmark)
            {
                pNewBookmark->SetKeyCode(pOldBookmark->GetKeyCode());
                pNewBookmark->SetShortName(pOldBookmark->GetShortName());
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
                ::sw::mark::IFieldmark::parameter_map_t::const_iterator pIt = pOldParams->begin();
                for (; pIt != pOldParams->end(); ++pIt )
                {
                    pNewParams->insert( *pIt );
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

    void lcl_DeleteRedlines( const SwPaM& rPam, SwPaM& rCpyPam )
    {
        const SwDoc* pSrcDoc = rPam.GetDoc();
        const SwRedlineTable& rTable = pSrcDoc->getIDocumentRedlineAccess().GetRedlineTable();
        if( !rTable.empty() )
        {
            SwDoc* pDestDoc = rCpyPam.GetDoc();
            SwPosition* pCpyStt = rCpyPam.Start(), *pCpyEnd = rCpyPam.End();
            std::unique_ptr<SwPaM> pDelPam;
            const SwPosition *pStt = rPam.Start(), *pEnd = rPam.End();
            // We have to count the "non-copied" nodes
            sal_uLong nDelCount;
            SwNodeIndex aCorrIdx(InitDelCount(rPam, nDelCount));

            sal_uInt16 n = 0;
            pSrcDoc->getIDocumentRedlineAccess().GetRedline( *pStt, &n );
            for( ; n < rTable.size(); ++n )
            {
                const SwRangeRedline* pRedl = rTable[ n ];
                if( nsRedlineType_t::REDLINE_DELETE == pRedl->GetType() && pRedl->IsVisible() )
                {
                    const SwPosition *pRStt = pRedl->Start(), *pREnd = pRedl->End();

                    SwComparePosition eCmpPos = ComparePosition( *pStt, *pEnd, *pRStt, *pREnd );
                    switch( eCmpPos )
                    {
                    case POS_COLLIDE_END:
                    case POS_BEFORE:
                        // Pos1 is before Pos2
                        break;

                    case POS_COLLIDE_START:
                    case POS_BEHIND:
                        // Pos1 is after Pos2
                        n = rTable.size();
                        break;

                    default:
                        {
                            pDelPam.reset(new SwPaM( *pCpyStt, pDelPam.get() ));
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
                        }
                    }
                }
            }

            if( pDelPam )
            {
                RedlineFlags eOld = pDestDoc->getIDocumentRedlineAccess().GetRedlineFlags();
                pDestDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld | RedlineFlags::Ignore );

                ::sw::UndoGuard const undoGuard(pDestDoc->GetIDocumentUndoRedo());

                do {
                    pDestDoc->getIDocumentContentOperations().DeleteAndJoin( *pDelPam->GetNext() );
                    if( !pDelPam->IsMultiSelection() )
                        break;
                    delete pDelPam->GetNext();
                } while( true );

                pDestDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
            }
        }
    }

    void lcl_DeleteRedlines( const SwNodeRange& rRg, SwNodeRange& rCpyRg )
    {
        SwDoc* pSrcDoc = rRg.aStart.GetNode().GetDoc();
        if( !pSrcDoc->getIDocumentRedlineAccess().GetRedlineTable().empty() )
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
            } while ( pTextNd && pTextNd != pEndTextNd );
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
namespace
{
    void
    lcl_CalcBreaks( std::vector<sal_Int32> & rBreaks, SwPaM const & rPam )
    {
        SwTextNode const * const pTextNode(
                rPam.End()->nNode.GetNode().GetTextNode() );
        if (!pTextNode)
            return; // left-overlap only possible at end of selection...

        const sal_Int32 nStart(rPam.Start()->nContent.GetIndex());
        const sal_Int32 nEnd  (rPam.End  ()->nContent.GetIndex());
        if (nEnd == pTextNode->Len())
            return; // paragraph selected until the end

        for (sal_Int32 i = nStart; i < nEnd; ++i)
        {
            const sal_Unicode c(pTextNode->GetText()[i]);
            if ((CH_TXTATR_INWORD == c) || (CH_TXTATR_BREAKWORD == c))
            {
                SwTextAttr const * const pAttr( pTextNode->GetTextAttrForCharAt(i) );
                if (pAttr && pAttr->End() && (*pAttr->End() > nEnd))
                {
                    OSL_ENSURE(pAttr->HasDummyChar(), "GetTextAttrForCharAt broken?");
                    rBreaks.push_back(i);
                }
            }
        }
    }

    bool lcl_DoWithBreaks(::sw::DocumentContentOperationsManager & rDocumentContentOperations, SwPaM & rPam,
            bool (::sw::DocumentContentOperationsManager::*pFunc)(SwPaM&, bool), const bool bForceJoinNext = false)
    {
        std::vector<sal_Int32> Breaks;

        lcl_CalcBreaks(Breaks, rPam);

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
        std::vector<sal_Int32>::reverse_iterator iter( Breaks.rbegin() );
        SwPaM aPam( rSelectionEnd, rSelectionEnd ); // end node!
        SwPosition & rEnd( *aPam.End() );
        SwPosition & rStart( *aPam.Start() );

        while (iter != Breaks.rend())
        {
            rStart.nContent = *iter + 1;
            if (rEnd.nContent > rStart.nContent) // check if part is empty
            {
                bRet &= (rDocumentContentOperations.*pFunc)(aPam, bForceJoinNext);
            }
            rEnd.nContent = *iter;
            ++iter;
        }

        rStart = *rPam.Start(); // set to original start
        if (rEnd.nContent > rStart.nContent) // check if part is empty
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
                return nSum > static_cast<sal_uInt64>(SAL_MAX_INT32);
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
        SwDoc* pDoc = aPam.GetNode().GetDoc();

        const SwPosition* pStart = aPam.Start();
        const SwPosition* pEnd = aPam.End();

        // get first relevant redline
        sal_uInt16 nCurrentRedline;
        pDoc->getIDocumentRedlineAccess().GetRedline( *pStart, &nCurrentRedline );
        if( nCurrentRedline > 0)
            nCurrentRedline--;

        // redline mode RedlineFlags::Ignore|RedlineFlags::On; save old mode
        RedlineFlags eOld = pDoc->getIDocumentRedlineAccess().GetRedlineFlags();
        pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( (( eOld & ~RedlineFlags::Ignore) | RedlineFlags::On ));

        // iterate over relevant redlines and decide for each whether it should
        // be saved, or split + saved
        SwRedlineTable& rRedlineTable = const_cast<SwRedlineTable&>( pDoc->getIDocumentRedlineAccess().GetRedlineTable() );
        for( ; nCurrentRedline < rRedlineTable.size(); nCurrentRedline++ )
        {
            SwRangeRedline* pCurrent = rRedlineTable[ nCurrentRedline ];
            SwComparePosition eCompare =
                ComparePosition( *pCurrent->Start(), *pCurrent->End(),
                                 *pStart, *pEnd);

            // we must save this redline if it overlaps aPam
            // (we may have to split it, too)
            if( eCompare == POS_OVERLAP_BEHIND  ||
                eCompare == POS_OVERLAP_BEFORE  ||
                eCompare == POS_OUTSIDE ||
                eCompare == POS_INSIDE ||
                eCompare == POS_EQUAL )
            {
                rRedlineTable.Remove( nCurrentRedline-- );

                // split beginning, if necessary
                if( eCompare == POS_OVERLAP_BEFORE  ||
                    eCompare == POS_OUTSIDE )
                {
                    SwRangeRedline* pNewRedline = new SwRangeRedline( *pCurrent );
                    *pNewRedline->End() = *pStart;
                    *pCurrent->Start() = *pStart;
                    pDoc->getIDocumentRedlineAccess().AppendRedline( pNewRedline, true );
                }

                // split end, if necessary
                if( eCompare == POS_OVERLAP_BEHIND  ||
                    eCompare == POS_OUTSIDE )
                {
                    SwRangeRedline* pNewRedline = new SwRangeRedline( *pCurrent );
                    *pNewRedline->Start() = *pEnd;
                    *pCurrent->End() = *pEnd;
                    pDoc->getIDocumentRedlineAccess().AppendRedline( pNewRedline, true );
                }

                // save the current redline
                rArr.push_back(SaveRedline( pCurrent, *pStart ));
            }
        }

        // restore old redline mode
        pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
    }

    void lcl_RestoreRedlines(SwDoc* pDoc, const SwPosition& rPos, SaveRedlines_t& rArr)
    {
        RedlineFlags eOld = pDoc->getIDocumentRedlineAccess().GetRedlineFlags();
        pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( ( eOld & ~RedlineFlags::Ignore) | RedlineFlags::On );

        for(SaveRedline & rSvRedLine : rArr)
        {
            rSvRedLine.SetPos( rPos );
            pDoc->getIDocumentRedlineAccess().AppendRedline( rSvRedLine.pRedl, true );
        }

        pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
    }

    void lcl_SaveRedlines(const SwNodeRange& rRg, SaveRedlines_t& rArr)
    {
        SwDoc* pDoc = rRg.aStart.GetNode().GetDoc();
        sal_uInt16 nRedlPos;
        SwPosition aSrchPos( rRg.aStart ); aSrchPos.nNode--;
        aSrchPos.nContent.Assign( aSrchPos.nNode.GetNode().GetContentNode(), 0 );
        if( pDoc->getIDocumentRedlineAccess().GetRedline( aSrchPos, &nRedlPos ) && nRedlPos )
            --nRedlPos;
        else if( nRedlPos >= pDoc->getIDocumentRedlineAccess().GetRedlineTable().size() )
            return ;

        RedlineFlags eOld = pDoc->getIDocumentRedlineAccess().GetRedlineFlags();
        pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( ( eOld & ~RedlineFlags::Ignore) | RedlineFlags::On );
        SwRedlineTable& rRedlTable = (SwRedlineTable&)pDoc->getIDocumentRedlineAccess().GetRedlineTable();

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

                    rArr.push_back(SaveRedline(pNewRedl, rRg.aStart));

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
                    rArr.push_back(SaveRedline( pTmp, rRg.aStart ));
                }
                else
                {
                    // split
                    SwRangeRedline* pNewRedl = new SwRangeRedline( *pTmp );
                    SwPosition* pTmpPos = pNewRedl->End();
                    pTmpPos->nNode = rRg.aEnd;
                    pTmpPos->nContent.Assign(
                                pTmpPos->nNode.GetNode().GetContentNode(), 0 );

                    rArr.push_back(SaveRedline( pNewRedl, rRg.aStart ));

                    pTmpPos = pTmp->Start();
                    pTmpPos->nNode = rRg.aEnd;
                    pTmpPos->nContent.Assign(
                                pTmpPos->nNode.GetNode().GetContentNode(), 0 );
                    pDoc->getIDocumentRedlineAccess().AppendRedline( pTmp, true );
                }
            }
            else
                break;

        } while( ++nRedlPos < pDoc->getIDocumentRedlineAccess().GetRedlineTable().size() );
        pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
    }

    void lcl_RestoreRedlines(SwDoc *const pDoc, sal_uInt32 const nInsPos, SaveRedlines_t& rArr)
    {
        RedlineFlags eOld = pDoc->getIDocumentRedlineAccess().GetRedlineFlags();
        pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( ( eOld & ~RedlineFlags::Ignore) | RedlineFlags::On );

        for(SaveRedline & rSvRedLine : rArr)
        {
            rSvRedLine.SetPos( nInsPos );
            pDoc->getIDocumentRedlineAccess().AppendRedline( rSvRedLine.pRedl, true );
        }

        pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
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
                        SwTextNode& rTextNd = (SwTextNode&)pSrch->GetTextNode();
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
                    !( (( &rSttNd.GetNode() == pFootnoteNd &&
                        pSttCnt->GetIndex() > nFootnoteSttIdx ) ||
                       ( &rEndNd.GetNode() == pFootnoteNd &&
                        nFootnoteSttIdx >= pEndCnt->GetIndex() )) ))
                {
                    if( bDelFootnote )
                    {
                        // delete it
                        SwTextNode& rTextNd = (SwTextNode&)pSrch->GetTextNode();
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
            case CH_TXT_ATR_FIELDSTART:
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
            const OUString sPara("\\n");
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
    #define DELETECHARSETS if ( bDelete ) { delete pCharSet; delete pOtherSet; }

    /// Insert Hints according to content types;
    // Is used in SwDoc::Insert(..., SwFormatHint &rHt)

    bool lcl_InsAttr(
        SwDoc *const pDoc,
        const SwPaM &rRg,
        const SfxItemSet& rChgSet,
        const SetAttrMode nFlags,
        SwUndoAttr *const pUndo,
        const bool bExpandCharToPara=false)
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
            const SfxPoolItem* pItem = aIter.FirstItem();
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
                     || isDrawingLayerAttribute(nWhich) ) //UUUU
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
            SfxItemSet* pTmpCharItemSet = new SfxItemSet( pDoc->GetAttrPool(),
                                       RES_CHRATR_BEGIN, RES_CHRATR_END-1,
                                       RES_TXTATR_AUTOFMT, RES_TXTATR_AUTOFMT,
                                       RES_TXTATR_INETFMT, RES_TXTATR_INETFMT,
                                       RES_TXTATR_CHARFMT, RES_TXTATR_CHARFMT,
                   RES_TXTATR_UNKNOWN_CONTAINER, RES_TXTATR_UNKNOWN_CONTAINER,
                                       0 );

            SfxItemSet* pTmpOtherItemSet = new SfxItemSet( pDoc->GetAttrPool(),
                                        RES_PARATR_BEGIN, RES_PARATR_END-1,
                                        RES_PARATR_LIST_BEGIN, RES_PARATR_LIST_END-1,
                                        RES_FRMATR_BEGIN, RES_FRMATR_END-1,
                                        RES_GRFATR_BEGIN, RES_GRFATR_END-1,
                                        RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,

                                        //UUUU FillAttribute support
                                        XATTR_FILL_FIRST, XATTR_FILL_LAST,

                                        0 );

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
            // #i27615#
            if (rRg.IsInFrontOfLabel())
            {
                SwTextNode * pTextNd = pNode->GetTextNode();
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

                SwNumFormat aNumFormat = pNumRule->Get(static_cast<sal_uInt16>(nLevel));
                SwCharFormat * pCharFormat =
                    pDoc->FindCharFormatByName(aNumFormat.GetCharFormatName());

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
                SfxItemSet aTextSet( pDoc->GetAttrPool(),
                            RES_TXTATR_NOEND_BEGIN, RES_TXTATR_NOEND_END-1 );
                aTextSet.Put( rChgSet );
                if( aTextSet.Count() )
                {
                    SwRegHistory history( pNode, *pNode, pHistory );
                    bRet = history.InsertItems(
                        aTextSet, rSt.GetIndex(), rSt.GetIndex(), nFlags ) || bRet;

                    if (bRet && (pDoc->getIDocumentRedlineAccess().IsRedlineOn() || (!pDoc->getIDocumentRedlineAccess().IsIgnoreRedline()
                                    && !pDoc->getIDocumentRedlineAccess().GetRedlineTable().empty())))
                    {
                        SwPaM aPam( pStt->nNode, pStt->nContent.GetIndex()-1,
                                    pStt->nNode, pStt->nContent.GetIndex() );

                        if( pUndo )
                            pUndo->SaveRedlineData( aPam, true );

                        if( pDoc->getIDocumentRedlineAccess().IsRedlineOn() )
                            pDoc->getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( nsRedlineType_t::REDLINE_INSERT, aPam ), true);
                        else
                            pDoc->getIDocumentRedlineAccess().SplitRedline( aPam );
                    }
                }
            }

            // TextAttributes with an end never expand their range
            if ( !bCharAttr && !bOtherAttr )
            {
                // CharFormat and URL attributes are treated separately!
                // TEST_TEMP ToDo: AutoFormat!
                SfxItemSet aTextSet( pDoc->GetAttrPool(),
                                    RES_TXTATR_REFMARK, RES_TXTATR_TOXMARK,
                                    RES_TXTATR_META, RES_TXTATR_METAFIELD,
                                    RES_TXTATR_CJK_RUBY, RES_TXTATR_CJK_RUBY,
                                    RES_TXTATR_INPUTFIELD, RES_TXTATR_INPUTFIELD,
                                    0 );

                aTextSet.Put( rChgSet );
                if( aTextSet.Count() )
                {
                    const sal_Int32 nInsCnt = rSt.GetIndex();
                    const sal_Int32 nEnd = pStt->nNode == pEnd->nNode
                                    ? pEnd->nContent.GetIndex()
                                    : pNode->Len();
                    SwRegHistory history( pNode, *pNode, pHistory );
                    bRet = history.InsertItems( aTextSet, nInsCnt, nEnd, nFlags )
                           || bRet;

                    if (bRet && (pDoc->getIDocumentRedlineAccess().IsRedlineOn() || (!pDoc->getIDocumentRedlineAccess().IsIgnoreRedline()
                                    && !pDoc->getIDocumentRedlineAccess().GetRedlineTable().empty())))
                    {
                        // Was text content inserted? (RefMark/TOXMarks without an end)
                        bool bTextIns = nInsCnt != rSt.GetIndex();
                        // Was content inserted or set over the selection?
                        SwPaM aPam( pStt->nNode, bTextIns ? nInsCnt + 1 : nEnd,
                                    pStt->nNode, nInsCnt );
                        if( pUndo )
                            pUndo->SaveRedlineData( aPam, bTextIns );

                        if( pDoc->getIDocumentRedlineAccess().IsRedlineOn() )
                            pDoc->getIDocumentRedlineAccess().AppendRedline(
                                new SwRangeRedline(
                                    bTextIns ? nsRedlineType_t::REDLINE_INSERT : nsRedlineType_t::REDLINE_FORMAT, aPam ),
                                    true);
                        else if( bTextIns )
                            pDoc->getIDocumentRedlineAccess().SplitRedline( aPam );
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
                        SwRegHistory aRegH( pNode, *pNode, pHistory );
                        bRet = pNode->SetAttr( aNew ) || bRet;
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
                    !pDoc->FindNumRulePtr( pRule->GetValue() ) &&
                    USHRT_MAX != (nPoolId = SwStyleNameMapper::GetPoolIdFromUIName ( pRule->GetValue(),
                                    SwGetPoolIdFromName::NumRule )) )
                    pDoc->getIDocumentStylePoolAccess().GetNumRuleFromPool( nPoolId );
            }
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
                    Boundary aBndry;
                    if( g_pBreakIt->GetBreakIter().is() )
                        aBndry = g_pBreakIt->GetBreakIter()->getWordBoundary(
                                    pTextNd->GetText(), nPtPos,
                                    g_pBreakIt->GetLocale( pTextNd->GetLang( nPtPos ) ),
                                    WordType::ANY_WORD /*ANYWORD_IGNOREWHITESPACES*/,
                                    true );

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
                bRet = history.InsertItems( *pCharSet, nMkPos, nPtPos, nFlags )
                    || bRet;

                if( pDoc->getIDocumentRedlineAccess().IsRedlineOn() )
                {
                    SwPaM aPam( *pNode, nMkPos, *pNode, nPtPos );

                    if( pUndo )
                        pUndo->SaveRedlineData( aPam, false );
                    pDoc->getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( nsRedlineType_t::REDLINE_FORMAT, aPam ), true);
                }
            }
            if( pOtherSet && pOtherSet->Count() )
            {
                SwRegHistory aRegH( pNode, *pNode, pHistory );

                //UUUU Need to check for unique item for DrawingLayer items of type NameOrIndex
                // and evtl. correct that item to ensure unique names for that type. This call may
                // modify/correct entries inside of the given SfxItemSet
                SfxItemSet aTempLocalCopy(*pOtherSet);

                pDoc->CheckForUniqueItemForLineFillNameOrIndex(aTempLocalCopy);
                bRet = pNode->SetAttr(aTempLocalCopy) || bRet;
            }

            DELETECHARSETS
            return bRet;
        }

        if( pDoc->getIDocumentRedlineAccess().IsRedlineOn() && pCharSet && pCharSet->Count() )
        {
            if( pUndo )
                pUndo->SaveRedlineData( rRg, false );
            pDoc->getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( nsRedlineType_t::REDLINE_FORMAT, rRg ), true);
        }

        /* now if range */
        sal_uLong nNodes = 0;

        SwNodeIndex aSt( pDoc->GetNodes() );
        SwNodeIndex aEnd( pDoc->GetNodes() );
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
                            pStt->nContent.GetIndex(), aCntEnd.GetIndex(), nFlags)
                        || bRet;
                }

                if( pOtherSet && pOtherSet->Count() )
                {
                    SwRegHistory aRegH( pNode, *pNode, pHistory );
                    bRet = pNode->SetAttr( *pOtherSet ) || bRet;
                }

                // Only selection in a Node.
                if( pStt->nNode == pEnd->nNode )
                {
                //The data parameter flag: bExpandCharToPara, comes from the data member of SwDoc,
                //which is set in SW MS Word Binary filter WW8ImplRreader. With this flag on, means that
                //current setting attribute set is a character range properties set and comes from a MS Word
                //binary file, and the setting range include a paragraph end position (0X0D);
                //more specifications, as such property inside the character range properties set recorded in
                //MS Word binary file are dealed and inserted into data model (SwDoc) one by one, so we
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
                                0, aCntEnd.GetIndex(), nFlags);
                    }

                    if( pOtherSet && pOtherSet->Count() )
                    {
                        SwRegHistory aRegH( pNode, *pNode, pHistory );
                        pNode->SetAttr( *pOtherSet );
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
            ::sw::DocumentContentOperationsManager::ParaRstFormat aPara( pStt, pEnd, pHistory, pCharSet );
            pDoc->GetNodes().ForEach( aSt, aEnd, ::sw::DocumentContentOperationsManager::lcl_RstTextAttr, &aPara );
        }

        bool bCreateSwpHints = pCharSet && (
            SfxItemState::SET == pCharSet->GetItemState( RES_TXTATR_CHARFMT, false ) ||
            SfxItemState::SET == pCharSet->GetItemState( RES_TXTATR_INETFMT, false ) );

        for(; aSt < aEnd; ++aSt )
        {
            pNode = aSt.GetNode().GetContentNode();
            if( !pNode )
                continue;

            SwTextNode* pTNd = pNode->GetTextNode();
            if( pHistory )
            {
                SwRegHistory aRegH( pNode, *pNode, pHistory );

                if( pTNd && pCharSet && pCharSet->Count() )
                {
                    SwpHints *pSwpHints = bCreateSwpHints ? &pTNd->GetOrCreateSwpHints()
                                                : pTNd->GetpSwpHints();
                    if( pSwpHints )
                        pSwpHints->Register( &aRegH );

                    pTNd->SetAttr(*pCharSet, 0, pTNd->GetText().getLength(), nFlags);
                    if( pSwpHints )
                        pSwpHints->DeRegister();
                }
                if( pOtherSet && pOtherSet->Count() )
                    pNode->SetAttr( *pOtherSet );
            }
            else
            {
                if( pTNd && pCharSet && pCharSet->Count() )
                    pTNd->SetAttr(*pCharSet, 0, pTNd->GetText().getLength(), nFlags);
                if( pOtherSet && pOtherSet->Count() )
                    pNode->SetAttr( *pOtherSet );
            }
            ++nNodes;
        }

        //The data parameter flag: bExpandCharToPara, comes from the data member of SwDoc,
        //which is set in SW MS Word Binary filter WW8ImplRreader. With this flag on, means that
        //current setting attribute set is a character range properties set and comes from a MS Word
        //binary file, and the setting range include a paragraph end position (0X0D);
        //more specifications, as such property inside the character range properties set recorded in
        //MS Word binary file are dealed and inserted into data model (SwDoc) one by one, so we
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
                SwNode* pNd = pDoc->GetNodes()[ nStart ];
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

DocumentContentOperationsManager::DocumentContentOperationsManager( SwDoc& i_rSwdoc ) : m_rDoc( i_rSwdoc )
{
}

// Copy an area into this document or into another document
bool
DocumentContentOperationsManager::CopyRange( SwPaM& rPam, SwPosition& rPos, const bool bCopyAll, bool bCheckPos ) const
{
    const SwPosition *pStt = rPam.Start(), *pEnd = rPam.End();

    SwDoc* pDoc = rPos.nNode.GetNode().GetDoc();
    bool bColumnSel = pDoc->IsClipBoard() && pDoc->IsColumnSelection();

    // Catch if there's no copy to do
    if( !rPam.HasMark() || ( *pStt >= *pEnd && !bColumnSel ) )
        return false;

    // Prevent copying in Flys that are anchored in the area
    if( pDoc == &m_rDoc && bCheckPos )
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
            lcl_ChkFlyFly( pDoc, nStt, nEnd, rPos.nNode.GetIndex() ) )
        {
            return false;
        }
    }

    SwPaM* pRedlineRange = nullptr;
    if( pDoc->getIDocumentRedlineAccess().IsRedlineOn() ||
        (!pDoc->getIDocumentRedlineAccess().IsIgnoreRedline() && !pDoc->getIDocumentRedlineAccess().GetRedlineTable().empty() ) )
        pRedlineRange = new SwPaM( rPos );

    RedlineFlags eOld = pDoc->getIDocumentRedlineAccess().GetRedlineFlags();

    bool bRet = false;

    if( pDoc != &m_rDoc )
    {   // ordinary copy
        bRet = CopyImpl( rPam, rPos, true, bCopyAll, pRedlineRange );
    }
    else if( ! ( *pStt <= rPos && rPos < *pEnd &&
            ( pStt->nNode != pEnd->nNode ||
              !pStt->nNode.GetNode().IsTextNode() )) )
    {
        // Copy to a position outside of the area, or copy a single TextNode
        // Do an ordinary copy
        bRet = CopyImpl( rPam, rPos, true, bCopyAll, pRedlineRange );
    }
    else
    {
        // Copy the area in itself
        // Special case for handling an area with several nodes,
        // or a single node that is not a TextNode
        OSL_ENSURE( &m_rDoc == pDoc, " invalid copy branch!" );
        OSL_FAIL("mst: i thought this could be dead code;"
                "please tell me what you did to get here!");
        pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern(eOld | RedlineFlags::Ignore);

        // Then copy the area to the underlying document area
        // (with start/end nodes clamped) and move them to
        // the desired position.

        SwUndoCpyDoc* pUndo = nullptr;
        // Save the Undo area
        SwPaM aPam( rPos );
        if (pDoc->GetIDocumentUndoRedo().DoesUndo())
        {
            pDoc->GetIDocumentUndoRedo().ClearRedo();
            pUndo = new SwUndoCpyDoc( aPam );
        }

        {
            ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());
            SwStartNode* pSttNd = SwNodes::MakeEmptySection(
                                SwNodeIndex( m_rDoc.GetNodes().GetEndOfAutotext() ));
            aPam.GetPoint()->nNode = *pSttNd->EndOfSectionNode();
            // copy without Frames
            pDoc->GetDocumentContentOperationsManager().CopyImpl( rPam, *aPam.GetPoint(), false, bCopyAll, nullptr );

            aPam.GetPoint()->nNode = pDoc->GetNodes().GetEndOfAutotext();
            aPam.SetMark();
            SwContentNode* pNode = SwNodes::GoPrevious( &aPam.GetMark()->nNode );
            pNode->MakeEndIndex( &aPam.GetMark()->nContent );

            aPam.GetPoint()->nNode = *aPam.GetNode().StartOfSectionNode();
            pNode = pDoc->GetNodes().GoNext( &aPam.GetPoint()->nNode );
            pNode->MakeStartIndex( &aPam.GetPoint()->nContent );
            // move to desired position
            pDoc->getIDocumentContentOperations().MoveRange( aPam, rPos, SwMoveFlags::DEFAULT );

            pNode = aPam.GetContentNode();
            *aPam.GetPoint() = rPos;      // Move the cursor for Undo
            aPam.SetMark();               // also move the Mark
            aPam.DeleteMark();            // But don't mark any area
            pDoc->getIDocumentContentOperations().DeleteSection( pNode ); // Delete the area again
        }

        // if Undo is enabled, store the insertion range
        if (pDoc->GetIDocumentUndoRedo().DoesUndo())
        {
            pUndo->SetInsertRange( aPam );
            pDoc->GetIDocumentUndoRedo().AppendUndo(pUndo);
        }

        if( pRedlineRange )
        {
            pRedlineRange->SetMark();
            *pRedlineRange->GetPoint() = *aPam.GetPoint();
            *pRedlineRange->GetMark() = *aPam.GetMark();
        }

        pDoc->getIDocumentState().SetModified();
        bRet = true;
    }

    pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
    if( pRedlineRange )
    {
        if( pDoc->getIDocumentRedlineAccess().IsRedlineOn() )
            pDoc->getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( nsRedlineType_t::REDLINE_INSERT, *pRedlineRange ), true);
        else
            pDoc->getIDocumentRedlineAccess().SplitRedline( *pRedlineRange );
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
    m_rDoc.getIDocumentRedlineAccess().DeleteRedline( *pSttNd, true, USHRT_MAX );
    DelBookmarks(aSttIdx, aEndIdx);

    {
        // move all Cursor/StackCursor/UnoCursor out of the to-be-deleted area
        SwNodeIndex aMvStt( aSttIdx, 1 );
        SwDoc::CorrAbs( aMvStt, aEndIdx, SwPosition( aSttIdx ), true );
    }

    m_rDoc.GetNodes().DelNodes( aSttIdx, aEndIdx.GetIndex() - aSttIdx.GetIndex() + 1 );
}

bool DocumentContentOperationsManager::DeleteRange( SwPaM & rPam )
{
    return lcl_DoWithBreaks( *this, rPam, &DocumentContentOperationsManager::DeleteRangeImpl );
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

    // Move hard page brakes to the following Node.
    bool bSavePageBreak = false, bSavePageDesc = false;

    /* #i9185# This whould lead to a segmentation fault if not caught above. */
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

        SwUndoDelete* pUndo = new SwUndoDelete( aDelPam, true );

        *rPam.GetPoint() = *aDelPam.GetPoint();
        pUndo->SetPgBrkFlags( bSavePageBreak, bSavePageDesc );
        m_rDoc.GetIDocumentUndoRedo().AppendUndo(pUndo);
    }
    else
    {
        SwNodeRange aRg( rStt.nNode, rEnd.nNode );
        if( rPam.GetPoint() != &rEnd )
            rPam.Exchange();

        // Try to move past the End
        if( !rPam.Move( fnMoveForward, GoInNode ) )
        {
            // Fair enough, at the Beginning then
            rPam.Exchange();
            if( !rPam.Move( fnMoveBackward, GoInNode ))
            {
                OSL_FAIL( "no more Nodes" );
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
                    ((FLY_AT_PARA == pAnchor->GetAnchorId()) ||
                     (FLY_AT_CHAR == pAnchor->GetAnchorId())) &&
                    aRg.aStart <= pAPos->nNode && pAPos->nNode <= aRg.aEnd )
                {
                    m_rDoc.getIDocumentLayoutAccess().DelLayoutFormat( pFly );
                    --n;
                }
            }
        }

        SwContentNode *pTmpNode = rPam.GetBound().nNode.GetNode().GetContentNode();
        rPam.GetBound().nContent.Assign( pTmpNode, 0 );
        pTmpNode = rPam.GetBound( false ).nNode.GetNode().GetContentNode();
        rPam.GetBound( false ).nContent.Assign( pTmpNode, 0 );
        m_rDoc.GetNodes().Delete( aRg.aStart, nNodeDiff+1 );
    }
    rPam.DeleteMark();
    m_rDoc.getIDocumentState().SetModified();

    return true;
}

// #i100466# Add handling of new optional parameter <bForceJoinNext>
bool DocumentContentOperationsManager::DeleteAndJoin( SwPaM & rPam,
                           const bool bForceJoinNext )
{
    if ( lcl_StrLenOverflow( rPam ) )
        return false;

    return lcl_DoWithBreaks( *this, rPam, (m_rDoc.getIDocumentRedlineAccess().IsRedlineOn())
                ? &DocumentContentOperationsManager::DeleteAndJoinWithRedlineImpl
                : &DocumentContentOperationsManager::DeleteAndJoinImpl,
                bForceJoinNext );
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

    // Save the paragraph anchored Flys, so that they can be moved.
    SaveFlyArr aSaveFlyArr;
    SaveFlyInRange( rPaM, rPos.nNode, aSaveFlyArr, bool( SwMoveFlags::ALLFLYS & eMvFlags ) );

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

    SwUndoMove * pUndoMove = nullptr;
    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        m_rDoc.GetIDocumentUndoRedo().ClearRedo();
        pUndoMove = new SwUndoMove( rPaM, rPos );
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
        pContentStore->Save( &m_rDoc, rPos.nNode.GetIndex(), rPos.nContent.GetIndex(), true );

        SwTextNode * pOrigNode = pTNd;
        assert(*aSavePam.GetPoint() == *aSavePam.GetMark() &&
               *aSavePam.GetPoint() == rPos);
        assert(aSavePam.GetPoint()->nContent.GetIdxReg() == pOrigNode);
        assert(aSavePam.GetPoint()->nNode == rPos.nNode.GetIndex());
        assert(rPos.nNode.GetIndex() == pOrigNode->GetIndex());

        pTNd = pTNd->SplitContentNode( rPos )->GetTextNode();

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

        if( !pContentStore->Empty() )
            pContentStore->Restore( &m_rDoc, rPos.nNode.GetIndex()-1, 0, true );

        // correct the PaM!
        if( rPos.nNode == rPaM.GetMark()->nNode )
        {
            rPaM.GetMark()->nNode = rPos.nNode.GetIndex()-1;
            rPaM.GetMark()->nContent.Assign( pTNd, nMkContent );
        }
    }

    // Put back the Pam by one "content"; so that it's always outside of
    // the manipulated range.
    // If there's no content anymore, set it to the StartNode (that's
    // always there).
    const bool bNullContent = !aSavePam.Move( fnMoveBackward, GoInContent );
    if( bNullContent )
    {
        aSavePam.GetPoint()->nNode--;
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
            delete pUndoMove;
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
    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        // correct the SavePam's Content first
        if( bNullContent )
        {
            aSavePam.GetPoint()->nContent = 0;
        }

        // The method SwEditShell::Move() merges the TextNode after the Move,
        // where the rPaM is located.
        // If the Content was moved to the back and the SavePam's SPoint is
        // in the next Node, we have to deal with this when saving the Undo object!
        SwTextNode * pPamTextNd = nullptr;

        // Is passed to SwUndoMove, which happens when subsequently calling Undo JoinNext.
        // If it's not possible to call Undo JoinNext here.
        bool bJoin = bSplit && pTNd;
        if( bCorrSavePam )
        {
            pPamTextNd = rPaM.GetNode().GetTextNode();
            bCorrSavePam = (pPamTextNd != nullptr)
                            && pPamTextNd->CanJoinNext()
                            && (*rPaM.GetPoint() <= *aSavePam.GetPoint());
        }

        // Do two Nodes have to be joined at the SavePam?
        if( bJoin && pTNd->CanJoinNext() )
        {
            pTNd->JoinNext();
            // No temporary Index when using &&.
            // We probably only want to compare the indices.
            if( bCorrSavePam && rPaM.GetPoint()->nNode.GetIndex()+1 ==
                                aSavePam.GetPoint()->nNode.GetIndex() )
            {
                aSavePam.GetPoint()->nContent += pPamTextNd->Len();
            }
            bJoin = false;
        }
        else if ( !aSavePam.Move( fnMoveForward, GoInContent ) )
        {
            aSavePam.GetPoint()->nNode++;
        }

        // The newly inserted range is now inbetween SPoint and GetMark.
        pUndoMove->SetDestRange( aSavePam, *rPaM.GetPoint(),
                                    bJoin, bCorrSavePam );
        m_rDoc.GetIDocumentUndoRedo().AppendUndo( pUndoMove );
    }
    else
    {
        bool bRemove = true;
        // Do two Nodes have to be joined at the SavePam?
        if( bSplit && pTNd )
        {
            if( pTNd->CanJoinNext())
            {
                // Always join next, because <pTNd> has to stay as it is.
                // A join previous from its next would more or less delete <pTNd>
                pTNd->JoinNext();
                bRemove = false;
            }
        }
        if( bNullContent )
        {
            aSavePam.GetPoint()->nNode++;
            aSavePam.GetPoint()->nContent.Assign( aSavePam.GetContentNode(), 0 );
        }
        else if( bRemove ) // No move forward after joining with next paragraph
        {
            aSavePam.Move( fnMoveForward, GoInContent );
        }
    }

    // Insert the Bookmarks back into the Document.
    *rPaM.GetMark() = *aSavePam.Start();
    for(
        std::vector< ::sw::mark::SaveBookmark>::iterator pBkmk = aSaveBkmks.begin();
        pBkmk != aSaveBkmks.end();
        ++pBkmk)
        pBkmk->SetInDoc(
            &m_rDoc,
            rPaM.GetMark()->nNode,
            &rPaM.GetMark()->nContent);
    *rPaM.GetPoint() = *aSavePam.End();

    // Move the Flys to the new position.
    RestFlyInRange( aSaveFlyArr, rPaM.Start()->nNode, &(rPos.nNode) );

    // restore redlines (if DOC_MOVEREDLINES is used)
    if( !aSaveRedl.empty() )
    {
        lcl_RestoreRedlines( &m_rDoc, *aSavePam.Start(), aSaveRedl );
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

    SwUndoMove* pUndo = nullptr;
    if ((SwMoveFlags::CREATEUNDOOBJ & eMvFlags ) && m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoMove( &m_rDoc, rRange, rPos );
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
        sal_uInt16 nRedlPos = m_rDoc.getIDocumentRedlineAccess().GetRedlinePos( rPos.GetNode(), USHRT_MAX );
        if( USHRT_MAX != nRedlPos )
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

    SwNodeIndex* pSaveInsPos = nullptr;
    if( pUndo )
        pSaveInsPos = new SwNodeIndex( rRange.aStart, -1 );

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
        delete pUndo;
        pUndo = nullptr;
    }

    // move the Flys to the new position
    if( !aSaveFlyArr.empty() )
        RestFlyInRange( aSaveFlyArr, aIdx, nullptr );

    // Add the Bookmarks back to the Document
    for(
        std::vector< ::sw::mark::SaveBookmark>::iterator pBkmk = aSaveBkmks.begin();
        pBkmk != aSaveBkmks.end();
        ++pBkmk)
        pBkmk->SetInDoc(&m_rDoc, aIdx);

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
        lcl_RestoreRedlines( &m_rDoc, aIdx.GetIndex(), aSaveRedl );

    if( pUndo )
    {
        pUndo->SetDestRange( aIdx, rPos, *pSaveInsPos );
        m_rDoc.GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    delete pSaveInsPos;

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

bool DocumentContentOperationsManager::MoveAndJoin( SwPaM& rPaM, SwPosition& rPos, SwMoveFlags eMvFlags )
{
    SwNodeIndex aIdx( rPaM.Start()->nNode );
    bool bJoinText = aIdx.GetNode().IsTextNode();
    bool bOneNode = rPaM.GetPoint()->nNode == rPaM.GetMark()->nNode;
    aIdx--;             // in front of the move area!

    bool bRet = MoveRange( rPaM, rPos, eMvFlags );
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
    return bRet;
}

bool DocumentContentOperationsManager::Overwrite( const SwPaM &rRg, const OUString &rStr )
{
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
                    bMerged = pUndoOW->CanGrouping( &m_rDoc, rPt, c );
                }
            }
            if (!bMerged)
            {
                SwUndo *const pUndoOW( new SwUndoOverwrite(&m_rDoc, rPt, c) );
                m_rDoc.GetIDocumentUndoRedo().AppendUndo(pUndoOW);
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
        SwUpdateAttr aHint(0,0,0);
        pNode->ModifyBroadcast(nullptr, &aHint);
    }

    if (!m_rDoc.GetIDocumentUndoRedo().DoesUndo() &&
        !m_rDoc.getIDocumentRedlineAccess().IsIgnoreRedline() && !m_rDoc.getIDocumentRedlineAccess().GetRedlineTable().empty())
    {
        SwPaM aPam( rPt.nNode, nStart, rPt.nNode, rPt.nContent.GetIndex() );
        m_rDoc.getIDocumentRedlineAccess().DeleteRedline( aPam, true, USHRT_MAX );
    }
    else if( m_rDoc.getIDocumentRedlineAccess().IsRedlineOn() )
    {
        // FIXME: this redline is WRONG: there is no DELETE, and the skipped
        // characters are also included in aPam
        SwPaM aPam( rPt.nNode, nStart, rPt.nNode, rPt.nContent.GetIndex() );
        m_rDoc.getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( nsRedlineType_t::REDLINE_INSERT, aPam ), true);
    }

    m_rDoc.getIDocumentState().SetModified();
    return true;
}

bool DocumentContentOperationsManager::InsertString( const SwPaM &rRg, const OUString &rStr,
        const SwInsertFlags nInsertMode )
{
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
            SwUndoInsert * const pUndo( new SwUndoInsert(rPos.nNode,
                    rPos.nContent.GetIndex(), ins.getLength(), nInsertMode));
            m_rDoc.GetIDocumentUndoRedo().AppendUndo(pUndo);
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
            m_rDoc.GetIDocumentUndoRedo().AppendUndo( pUndo );
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
                m_rDoc.GetIDocumentUndoRedo().AppendUndo( pUndo );
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
                new SwRangeRedline( nsRedlineType_t::REDLINE_INSERT, aPam ), true);
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
    SwUndoTransliterate *const pUndo = (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
        ?   new SwUndoTransliterate( rPaM, rTrans )
        :   nullptr;

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

        Boundary aBndry;
        if( g_pBreakIt->GetBreakIter().is() )
            aBndry = g_pBreakIt->GetBreakIter()->getWordBoundary(
                        pTNd->GetText(), nSttCnt,
                        g_pBreakIt->GetLocale( pTNd->GetLang( nSttCnt ) ),
                        WordType::ANY_WORD /*ANYWORD_IGNOREWHITESPACES*/,
                        true );

        if( aBndry.startPos < nSttCnt && nSttCnt < aBndry.endPos )
        {
            nSttCnt = aBndry.startPos;
            nEndCnt = aBndry.endPos;
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
                pTNd->TransliterateText(
                        rTrans, nSttCnt, pTNd->GetText().getLength(), pUndo);
        }

        for( ; aIdx.GetIndex() < nEndNd; ++aIdx )
        {
            pTNd = aIdx.GetNode().GetTextNode();
            if (pTNd)
            {
                pTNd->TransliterateText(
                        rTrans, 0, pTNd->GetText().getLength(), pUndo);
            }
        }

        if( nEndCnt && nullptr != ( pTNd = pEnd->nNode.GetNode().GetTextNode() ))
            pTNd->TransliterateText( rTrans, 0, nEndCnt, pUndo );
    }
    else if( pTNd && nSttCnt < nEndCnt )
        pTNd->TransliterateText( rTrans, nSttCnt, nEndCnt, pUndo );

    if( pUndo )
    {
        if( pUndo->HasData() )
        {
            m_rDoc.GetIDocumentUndoRedo().AppendUndo(pUndo);
        }
        else
            delete pUndo;
    }
    m_rDoc.getIDocumentState().SetModified();
}

SwFlyFrameFormat* DocumentContentOperationsManager::Insert( const SwPaM &rRg,
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

SwFlyFrameFormat* DocumentContentOperationsManager::Insert( const SwPaM &rRg, const GraphicObject& rGrfObj,
                            const SfxItemSet* pFlyAttrSet,
                            const SfxItemSet* pGrfAttrSet,
                            SwFrameFormat* pFrameFormat )
{
    if( !pFrameFormat )
        pFrameFormat = m_rDoc.getIDocumentStylePoolAccess().GetFrameFormatFromPool( RES_POOLFRM_GRAPHIC );
    SwGrfNode* pSwGrfNode = SwNodes::MakeGrfNode(
                            SwNodeIndex( m_rDoc.GetNodes().GetEndOfAutotext() ),
                            rGrfObj, m_rDoc.GetDfltGrfFormatColl() );
    SwFlyFrameFormat* pSwFlyFrameFormat = InsNoTextNode( *rRg.GetPoint(), pSwGrfNode,
                            pFlyAttrSet, pGrfAttrSet, pFrameFormat );
    return pSwFlyFrameFormat;
}

SwFlyFrameFormat* DocumentContentOperationsManager::Insert(const SwPaM &rRg, const svt::EmbeddedObjectRef& xObj,
                        const SfxItemSet* pFlyAttrSet)
{
    sal_uInt16 nId = RES_POOLFRM_OLE;
    if (xObj.is())
    {
        SvGlobalName aClassName( xObj->getClassID() );
        if (SotExchange::IsMath(aClassName))
            nId = RES_POOLFRM_FORMEL;
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
                    const OUString& rFltName, const Graphic* pGraphic,
                    const GraphicObject* pGrafObj )
{
    SwGrfNode *pGrfNd;
    if( ( !rPam.HasMark()
         || rPam.GetPoint()->nNode.GetIndex() == rPam.GetMark()->nNode.GetIndex() )
         && nullptr != ( pGrfNd = rPam.GetPoint()->nNode.GetNode().GetGrfNode() ) )
    {
        if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
        {
            m_rDoc.GetIDocumentUndoRedo().AppendUndo(new SwUndoReRead(rPam, *pGrfNd));
        }

        // Because we don't know if we can mirror the graphic, the mirror attribute is always reset
        if( RES_MIRROR_GRAPH_DONT != pGrfNd->GetSwAttrSet().
                                                GetMirrorGrf().GetValue() )
            pGrfNd->SetAttr( SwMirrorGrf() );

        pGrfNd->ReRead( rGrfName, rFltName, pGraphic, pGrafObj );
        m_rDoc.getIDocumentState().SetModified();
    }
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
    const bool bIsAtContent = (FLY_AT_PAGE != eAnchorId);

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
        eAnchorId = FLY_AT_PAGE;
        pFormat->SetFormatAttr( SwFormatAnchor( eAnchorId ) );
    }
    else if( pAnchor == nullptr
             || ( bIsAtContent
                  && pAnchor->GetContentAnchor() == nullptr ) )
    {
        // apply anchor format
        SwFormatAnchor aAnch( pAnchor != nullptr ? *pAnchor : pFormat->GetAnchor() );
        eAnchorId = aAnch.GetAnchorId();
        if ( eAnchorId == FLY_AT_FLY )
        {
            SwPosition aPos( *rRg.GetNode().FindFlyStartNode() );
            aAnch.SetAnchor( &aPos );
        }
        else
        {
            aAnch.SetAnchor( rRg.GetPoint() );
            if ( eAnchorId == FLY_AT_PAGE )
            {
                eAnchorId = dynamic_cast<const SdrUnoObj*>( &rDrawObj) !=  nullptr ? FLY_AS_CHAR : FLY_AT_PARA;
                aAnch.SetType( eAnchorId );
            }
        }
        pFormat->SetFormatAttr( aAnch );
    }

    // insert text attribute for as-character anchored drawing object
    if ( eAnchorId == FLY_AS_CHAR )
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
            pFormat->SetFormatAttr( SwFormatAnchor( FLY_AT_PAGE ) );
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
        m_rDoc.GetIDocumentUndoRedo().AppendUndo( new SwUndoInsLayFormat(pFormat, 0, 0) );
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
        SwDataChanged aTmp( &m_rDoc, rPos );
    }

    SwUndoSplitNode* pUndo = nullptr;
    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        m_rDoc.GetIDocumentUndoRedo().ClearRedo();
        // insert the Undo object (currently only for TextNode)
        if( pNode->IsTextNode() )
        {
            pUndo = new SwUndoSplitNode( &m_rDoc, rPos, bChkTableStart );
            m_rDoc.GetIDocumentUndoRedo().AppendUndo(pUndo);
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
    pContentStore->Save( &m_rDoc, rPos.nNode.GetIndex(), rPos.nContent.GetIndex(), true );
    // FIXME: only SwTextNode has a valid implementation of SplitContentNode!
    OSL_ENSURE(pNode->IsTextNode(), "splitting non-text node?");
    pNode = pNode->SplitContentNode( rPos );
    if (pNode)
    {
        // move all bookmarks, TOXMarks, FlyAtCnt
        if( !pContentStore->Empty() )
            pContentStore->Restore( &m_rDoc, rPos.nNode.GetIndex()-1, 0, true );

        // To-Do - add 'SwExtraRedlineTable' also ?
        if( m_rDoc.getIDocumentRedlineAccess().IsRedlineOn() || (!m_rDoc.getIDocumentRedlineAccess().IsIgnoreRedline() && !m_rDoc.getIDocumentRedlineAccess().GetRedlineTable().empty() ))
        {
            SwPaM aPam( rPos );
            aPam.SetMark();
            aPam.Move( fnMoveBackward );
            if( m_rDoc.getIDocumentRedlineAccess().IsRedlineOn() )
                m_rDoc.getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( nsRedlineType_t::REDLINE_INSERT, aPam ), true);
            else
                m_rDoc.getIDocumentRedlineAccess().SplitRedline( aPam );
        }
    }

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
        m_rDoc.GetIDocumentUndoRedo().AppendUndo( new SwUndoInsert( rPos.nNode ) );
    }

    // To-Do - add 'SwExtraRedlineTable' also ?
    if( m_rDoc.getIDocumentRedlineAccess().IsRedlineOn() || (!m_rDoc.getIDocumentRedlineAccess().IsIgnoreRedline() && !m_rDoc.getIDocumentRedlineAccess().GetRedlineTable().empty() ))
    {
        SwPaM aPam( rPos );
        aPam.SetMark();
        aPam.Move( fnMoveBackward );
        if( m_rDoc.getIDocumentRedlineAccess().IsRedlineOn() )
            m_rDoc.getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( nsRedlineType_t::REDLINE_INSERT, aPam ), true);
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

    std::vector<sal_Int32> Breaks;

    SwPaM aPam( *rPam.GetMark(), *rPam.GetPoint() );
    aPam.Normalize(false);
    if (aPam.GetPoint()->nNode != aPam.GetMark()->nNode)
    {
        aPam.Move(fnMoveBackward);
    }
    OSL_ENSURE((aPam.GetPoint()->nNode == aPam.GetMark()->nNode), "invalid pam?");

    lcl_CalcBreaks(Breaks, aPam);

    while (!Breaks.empty() // skip over prefix of dummy chars
            && (aPam.GetMark()->nContent.GetIndex() == *Breaks.begin()) )
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
    std::vector<sal_Int32>::reverse_iterator iter( Breaks.rbegin() );
    OSL_ENSURE(aPam.GetPoint() == aPam.End(), "wrong!");
    SwPosition & rEnd( *aPam.End() );
    SwPosition & rStart( *aPam.Start() );

    // set end of temp pam to original end (undo Move backward above)
    rEnd = *rPam.End();
    // after first deletion, rEnd will point into the original text node again!

    while (iter != Breaks.rend())
    {
        rStart.nContent = *iter + 1;
        if (rEnd.nContent != rStart.nContent) // check if part is empty
        {
            bRet &= (m_rDoc.getIDocumentRedlineAccess().IsRedlineOn())
                ? DeleteAndJoinWithRedlineImpl(aPam)
                : DeleteAndJoinImpl(aPam, false);
        }
        rEnd.nContent = *iter;
        ++iter;
    }

    rStart = *rPam.Start(); // set to original start
    OSL_ENSURE(rEnd.nContent > rStart.nContent, "replace part empty!");
    if (rEnd.nContent > rStart.nContent) // check if part is empty
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
    const bool bExpandCharToPara)
{
    SwDataChanged aTmp( rRg );
    SwUndoAttr* pUndoAttr = nullptr;
    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        m_rDoc.GetIDocumentUndoRedo().ClearRedo();
        pUndoAttr = new SwUndoAttr( rRg, rHt, nFlags );
    }

    SfxItemSet aSet( m_rDoc.GetAttrPool(), rHt.Which(), rHt.Which() );
    aSet.Put( rHt );
    const bool bRet = lcl_InsAttr( &m_rDoc, rRg, aSet, nFlags, pUndoAttr, bExpandCharToPara );

    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        m_rDoc.GetIDocumentUndoRedo().AppendUndo( pUndoAttr );
    }

    if( bRet )
    {
        m_rDoc.getIDocumentState().SetModified();
    }
    return bRet;
}

bool DocumentContentOperationsManager::InsertItemSet ( const SwPaM &rRg, const SfxItemSet &rSet,
                            const SetAttrMode nFlags )
{
    SwDataChanged aTmp( rRg );
    SwUndoAttr* pUndoAttr = nullptr;
    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        m_rDoc.GetIDocumentUndoRedo().ClearRedo();
        pUndoAttr = new SwUndoAttr( rRg, rSet, nFlags );
    }

    bool bRet = lcl_InsAttr( &m_rDoc, rRg, rSet, nFlags, pUndoAttr );

    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        m_rDoc.GetIDocumentUndoRedo().AppendUndo( pUndoAttr );
    }

    if( bRet )
        m_rDoc.getIDocumentState().SetModified();
    return bRet;
}

void DocumentContentOperationsManager::RemoveLeadingWhiteSpace(const SwPosition & rPos )
{
    const SwTextNode* pTNd = rPos.nNode.GetNode().GetTextNode();
    if ( pTNd )
    {
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
}

// Copy method from SwDoc - "copy Flys in Flys"
void DocumentContentOperationsManager::CopyWithFlyInFly(
    const SwNodeRange& rRg,
    const sal_Int32 nEndContentIndex,
    const SwNodeIndex& rInsPos,
    const std::pair<const SwPaM&, const SwPosition&>* pCopiedPaM /*and real insert pos*/,
    const bool bMakeNewFrames,
    const bool bDelRedlines,
    const bool bCopyFlyAtFly ) const
{
    assert(!pCopiedPaM || pCopiedPaM->first.End()->nContent == nEndContentIndex);
    assert(!pCopiedPaM || pCopiedPaM->first.End()->nNode == rRg.aEnd);

    SwDoc* pDest = rInsPos.GetNode().GetDoc();

    SaveRedlEndPosForRestore aRedlRest( rInsPos, 0 );

    SwNodeIndex aSavePos( rInsPos, -1 );
    bool bEndIsEqualEndPos = rInsPos == rRg.aEnd;
    m_rDoc.GetNodes().CopyNodes( rRg, rInsPos, bMakeNewFrames, true );
    ++aSavePos;
    if( bEndIsEqualEndPos )
        const_cast<SwNodeIndex&>(rRg.aEnd) = aSavePos;

    aRedlRest.Restore();

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
        ::sw::UndoGuard const undoGuard(pDest->GetIDocumentUndoRedo());
        CopyFlyInFlyImpl( rRg, nEndContentIndex, aSavePos, bCopyFlyAtFly );
    }

    SwNodeRange aCpyRange( aSavePos, rInsPos );

    // Also copy all bookmarks
    // guess this must be done before the DelDummyNodes below as that
    // deletes nodes so would mess up the index arithmetic
    if( m_rDoc.getIDocumentMarkAccess()->getAllMarksCount() )
    {
        SwPaM aRgTmp( rRg.aStart, rRg.aEnd );
        SwPaM aCpyPaM(aCpyRange.aStart, aCpyRange.aEnd);
        if (pCopiedPaM && rRg.aStart != pCopiedPaM->first.Start()->nNode)
        {
            // there is 1 (partially selected, maybe) paragraph before
            assert(SwNodeIndex(rRg.aStart, -1) == pCopiedPaM->first.Start()->nNode);
            // only use the passed in target SwPosition if the source PaM point
            // is on a different node; if it was the same node then the target
            // position was likely moved along by the copy operation and now
            // points to the end of the range!
            *aCpyPaM.GetPoint() = pCopiedPaM->second;
        }

        lcl_CopyBookmarks((pCopiedPaM) ? pCopiedPaM->first : aRgTmp, aCpyPaM);
    }

    if( bDelRedlines && ( RedlineFlags::DeleteRedlines & pDest->getIDocumentRedlineAccess().GetRedlineFlags() ))
        lcl_DeleteRedlines( rRg, aCpyRange );

    pDest->GetNodes().DelDummyNodes( aCpyRange );
}

// TODO: there is a limitation here in that it's not possible to pass a start
// content index - which means that at-character anchored frames inside
// partial 1st paragraph of redline is not copied.
// But the DelFlyInRange() that is called from DelCopyOfSection() does not
// delete it either, and it also does not delete those on partial last para of
// redline, so copying those is suppressed here too ...
void DocumentContentOperationsManager::CopyFlyInFlyImpl(
    const SwNodeRange& rRg,
    const sal_Int32 nEndContentIndex,
    const SwNodeIndex& rStartIdx,
    const bool bCopyFlyAtFly ) const
{
    // First collect all Flys, sort them according to their ordering number,
    // and then only copy them. This maintains the ordering numbers (which are only
    // managed in the DrawModel).
    SwDoc *const pDest = rStartIdx.GetNode().GetDoc();
    std::set< ZSortFly > aSet;
    const size_t nArrLen = m_rDoc.GetSpzFrameFormats()->size();

    SwTextBoxHelper::SavedLink aOldTextBoxes;
    SwTextBoxHelper::saveLinks(*m_rDoc.GetSpzFrameFormats(), aOldTextBoxes);
    SwTextBoxHelper::SavedContent aOldContent;

    for ( size_t n = 0; n < nArrLen; ++n )
    {
        SwFrameFormat* pFormat = (*m_rDoc.GetSpzFrameFormats())[n];
        SwFormatAnchor const*const pAnchor = &pFormat->GetAnchor();
        SwPosition const*const pAPos = pAnchor->GetContentAnchor();
        bool bAtContent = (pAnchor->GetAnchorId() == FLY_AT_PARA);
        if ( !pAPos )
            continue;
        sal_uLong nSkipAfter = pAPos->nNode.GetIndex();
        sal_uLong nStart = rRg.aStart.GetIndex();
        switch ( pAnchor->GetAnchorId() )
        {
            case FLY_AT_FLY:
                if(bCopyFlyAtFly)
                    ++nSkipAfter;
                else if(m_rDoc.getIDocumentRedlineAccess().IsRedlineMove())
                    ++nStart;
            break;
            case FLY_AT_CHAR:
            case FLY_AT_PARA:
                if(m_rDoc.getIDocumentRedlineAccess().IsRedlineMove())
                    ++nStart;
            break;
            default:
                continue;
        }
        if ( nStart > nSkipAfter )
            continue;
        if ( pAPos->nNode > rRg.aEnd )
            continue;
        //frames at the last source node are not always copied:
        //- if the node is empty and is the last node of the document or a table cell
        //  or a text frame then they have to be copied
        //- if the content index in this node is > 0 then paragraph and frame bound objects are copied
        //- to-character bound objects are copied if their index is <= nEndContentIndex
        bool bAdd = false;
        if( pAPos->nNode < rRg.aEnd )
            bAdd = true;
        if (!bAdd && !m_rDoc.getIDocumentRedlineAccess().IsRedlineMove()) // fdo#40599: not for redline move
        {
            bool bEmptyNode = false;
            bool bLastNode = false;
            // is the node empty?
            const SwNodes& rNodes = pAPos->nNode.GetNodes();
            SwTextNode* pTextNode;
            if( nullptr != ( pTextNode = pAPos->nNode.GetNode().GetTextNode() ))
            {
                bEmptyNode = pTextNode->GetText().isEmpty();
                if( bEmptyNode )
                {
                    //last node information is only necessary to know for the last TextNode
                    SwNodeIndex aTmp( pAPos->nNode );
                    ++aTmp;//goto next node
                    while (aTmp.GetNode().IsEndNode())
                    {
                        if( aTmp == rNodes.GetEndOfContent().GetIndex() )
                        {
                            bLastNode = true;
                            break;
                        }
                        ++aTmp;
                    }
                }
            }
            bAdd = bLastNode && bEmptyNode;
            if( !bAdd )
            {
                if( bAtContent )
                    bAdd = nEndContentIndex > 0;
                else
                    bAdd = pAPos->nContent <= nEndContentIndex;
            }
        }
        if( bAdd )
        {
            // Make sure draw formats don't refer to content, so that such
            // content can be removed without problems.
            SwTextBoxHelper::resetLink(pFormat, aOldContent);
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
        if ((aAnchor.GetAnchorId() == FLY_AT_PARA) ||
            (aAnchor.GetAnchorId() == FLY_AT_CHAR) )
        {
            // First, determine number of anchor text node in the copied range.
            // Note: The anchor text node *have* to be inside the copied range.
            sal_uLong nAnchorTextNdNumInRange( 0L );
            bool bAnchorTextNdFound( false );
            SwNodeIndex aIdx( rRg.aStart );
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
            long nOffset = newPos.nNode.GetIndex() - rRg.aStart.GetIndex();
            SwNodeIndex aIdx( rStartIdx, nOffset );
            newPos.nNode = aIdx;
        }
        // Set the character bound Flys back at the original character
        if ((FLY_AT_CHAR == aAnchor.GetAnchorId()) &&
             newPos.nNode.GetNode().IsTextNode() )
        {
            newPos.nContent.Assign( newPos.nNode.GetNode().GetTextNode(), newPos.nContent.GetIndex() );
        }
        else
        {
            newPos.nContent.Assign( nullptr, 0 );
        }
        aAnchor.SetAnchor( &newPos );

        // Check recursion: copy content in its own frame, then don't copy it.
        if( pDest == &m_rDoc )
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

        // Copy the format and set the new anchor
        aVecSwFrameFormat.push_back( pDest->getIDocumentLayoutAccess().CopyLayoutFormat( *(*it).GetFormat(),
                aAnchor, false, true ) );
        ++it;
    }

    // Rebuild as much as possible of all chains that are available in the original,
    OSL_ENSURE( aSet.size() == aVecSwFrameFormat.size(), "Missing new Flys" );
    if ( aSet.size() == aVecSwFrameFormat.size() )
    {
        size_t n = 0;
        for (std::set< ZSortFly >::const_iterator nIt=aSet.begin() ; nIt != aSet.end(); ++nIt, ++n )
        {
            const SwFrameFormat *pFormatN = (*nIt).GetFormat();
            const SwFormatChain &rChain = pFormatN->GetChain();
            int nCnt = int(nullptr != rChain.GetPrev());
            nCnt += rChain.GetNext() ? 1: 0;
            size_t k = 0;
            for (std::set< ZSortFly >::const_iterator kIt=aSet.begin() ; kIt != aSet.end(); ++kIt, ++k )
            {
                const SwFrameFormat *pFormatK = (*kIt).GetFormat();
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
            }
        }

        // Re-create content property of draw formats, knowing how old shapes
        // were paired with old fly formats (aOldTextBoxes) and that aSet is
        // parallel with aVecSwFrameFormat.
        SwTextBoxHelper::restoreLinks(aSet, aVecSwFrameFormat, aOldTextBoxes, aOldContent);
    }
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
    OSL_ENSURE( m_rDoc.getIDocumentRedlineAccess().IsRedlineOn(), "DeleteAndJoinWithRedline: redline off" );

    {
        SwUndoRedlineDelete* pUndo = nullptr;
        RedlineFlags eOld = m_rDoc.getIDocumentRedlineAccess().GetRedlineFlags();
        m_rDoc.GetDocumentRedlineManager().checkRedlining( eOld );
        if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
        {

            /* please don't translate -- for cultural reasons this comment is protected
               until the redline implementation is finally fixed some day */
            //JP 06.01.98: MUSS noch optimiert werden!!!
            m_rDoc.getIDocumentRedlineAccess().SetRedlineFlags(
                RedlineFlags::On | RedlineFlags::ShowInsert | RedlineFlags::ShowDelete );

            m_rDoc.GetIDocumentUndoRedo().StartUndo( UNDO_DELETE, nullptr );
            pUndo = new SwUndoRedlineDelete( rPam, UNDO_DELETE );
            m_rDoc.GetIDocumentUndoRedo().AppendUndo( pUndo );
        }

        if ( *rPam.GetPoint() != *rPam.GetMark() )
            m_rDoc.getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( nsRedlineType_t::REDLINE_DELETE, rPam ), true );
        m_rDoc.getIDocumentState().SetModified();

        if ( pUndo )
        {
            m_rDoc.GetIDocumentUndoRedo().EndUndo( UNDO_EMPTY, nullptr );
            // ??? why the hell is the AppendUndo not below the
            // CanGrouping, so this hideous cleanup wouldn't be necessary?
            // bah, this is redlining, probably changing this would break it...
            if ( m_rDoc.GetIDocumentUndoRedo().DoesGroupUndo() )
            {
                SwUndo * const pLastUndo( m_rDoc.GetUndoManager().GetLastUndo() );
                SwUndoRedlineDelete * const pUndoRedlineDel( dynamic_cast< SwUndoRedlineDelete* >( pLastUndo ) );
                if ( pUndoRedlineDel )
                {
                    bool const bMerged = pUndoRedlineDel->CanGrouping( *pUndo );
                    if ( bMerged )
                    {
                        ::sw::UndoGuard const undoGuard( m_rDoc.GetIDocumentUndoRedo() );
                        SwUndo const* const pDeleted = m_rDoc.GetUndoManager().RemoveLastUndo();
                        OSL_ENSURE( pDeleted == pUndo, "DeleteAndJoinWithRedlineImpl: "
                            "undo removed is not undo inserted?" );
                        delete pDeleted;
                    }
                }
            }
            //JP 06.01.98: MUSS noch optimiert werden!!!
            m_rDoc.getIDocumentRedlineAccess().SetRedlineFlags( eOld );
        }
        return true;
    }
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

    if( !rPam.HasMark() || *pStt >= *pEnd )
        return false;

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
                bMerged = pUndoDelete->CanGrouping( &m_rDoc, rPam );
                // if CanGrouping() returns true it's already merged
            }
        }
        if (!bMerged)
        {
            m_rDoc.GetIDocumentUndoRedo().AppendUndo( new SwUndoDelete( rPam ) );
        }

        m_rDoc.getIDocumentState().SetModified();

        return true;
    }

    if( !m_rDoc.getIDocumentRedlineAccess().IsIgnoreRedline() && !m_rDoc.getIDocumentRedlineAccess().GetRedlineTable().empty() )
        m_rDoc.getIDocumentRedlineAccess().DeleteRedline( rPam, true, USHRT_MAX );

    // Delete and move all "Flys at the paragraph", which are within the Selection
    DelFlyInRange(rPam.GetMark()->nNode, rPam.GetPoint()->nNode);
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
        sal_uInt32 nEnde = pEnd->nNode.GetIndex();
        if( pCNd == nullptr )
            nEnde++;

        if( aSttIdx != nEnde )
        {
            // delete the Nodes into the NodesArary
            m_rDoc.GetNodes().Delete( aSttIdx, nEnde - aSttIdx.GetIndex() );
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

    if( !m_rDoc.getIDocumentRedlineAccess().IsIgnoreRedline() && !m_rDoc.getIDocumentRedlineAccess().GetRedlineTable().empty() )
        m_rDoc.getIDocumentRedlineAccess().CompressRedlines();
    m_rDoc.getIDocumentState().SetModified();

    return true;
}

// It's possible to call Replace with a PaM that spans 2 paragraphs:
// search with regex for "$", then replace _all_
bool DocumentContentOperationsManager::ReplaceRangeImpl( SwPaM& rPam, const OUString& rStr,
        const bool bRegExReplace )
{
    if( !rPam.HasMark() || *rPam.GetPoint() == *rPam.GetMark() )
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
        OSL_ENSURE( pStt->nNode == pEnd->nNode ||
                ( pStt->nNode.GetIndex() + 1 == pEnd->nNode.GetIndex() &&
                    !pEnd->nContent.GetIndex() ),
                "invalid range: Point and Mark on different nodes" );
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
            m_rDoc.GetDocumentRedlineManager().checkRedlining(eOld);
            if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
            {
                m_rDoc.GetIDocumentUndoRedo().StartUndo(UNDO_EMPTY, nullptr);

                // If any Redline will change (split!) the node
                const ::sw::mark::IMark* pBkmk = m_rDoc.getIDocumentMarkAccess()->makeMark( aDelPam, OUString(), IDocumentMarkAccess::MarkType::UNO_BOOKMARK );

                //JP 06.01.98: MUSS noch optimiert werden!!!
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
                            RES_CHRATR_BEGIN,     RES_TXTATR_WITHEND_END - 1,
                            RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
                            0 );
                pTextNd->GetAttr( aSet, nStt+1, nStt+1 );

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
                SwUndo *const pUndoRD =
                    new SwUndoRedlineDelete( aDelPam, UNDO_REPLACE );
                m_rDoc.GetIDocumentUndoRedo().AppendUndo(pUndoRD);
            }
            m_rDoc.getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( nsRedlineType_t::REDLINE_DELETE, aDelPam ), true);

            *rPam.GetMark() = *aDelPam.GetMark();
            if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
            {
                *aDelPam.GetPoint() = *rPam.GetPoint();
                m_rDoc.GetIDocumentUndoRedo().EndUndo(UNDO_EMPTY, nullptr);

                // If any Redline will change (split!) the node
                const ::sw::mark::IMark* pBkmk = m_rDoc.getIDocumentMarkAccess()->makeMark( aDelPam, OUString(), IDocumentMarkAccess::MarkType::UNO_BOOKMARK );

                SwIndex& rIdx = aDelPam.GetPoint()->nContent;
                rIdx.Assign( nullptr, 0 );
                aDelPam.GetMark()->nContent = rIdx;
                rPam.GetPoint()->nNode = 0;
                rPam.GetPoint()->nContent = rIdx;
                *rPam.GetMark() = *rPam.GetPoint();
                //JP 06.01.98: MUSS noch optimiert werden!!!
                m_rDoc.getIDocumentRedlineAccess().SetRedlineFlags( eOld );

                *rPam.GetPoint() = pBkmk->GetMarkPos();
                if(pBkmk->IsExpanded())
                    *rPam.GetMark() = pBkmk->GetOtherMarkPos();
                m_rDoc.getIDocumentMarkAccess()->deleteMark(pBkmk);
            }
            bJoinText = false;
        }
        else
        {
            if( !m_rDoc.getIDocumentRedlineAccess().IsIgnoreRedline() && m_rDoc.getIDocumentRedlineAccess().GetRedlineTable().size() )
                m_rDoc.getIDocumentRedlineAccess().DeleteRedline( aDelPam, true, USHRT_MAX );

            SwUndoReplace* pUndoRpl = nullptr;
            bool const bDoesUndo = m_rDoc.GetIDocumentUndoRedo().DoesUndo();
            if (bDoesUndo)
            {
                pUndoRpl = new SwUndoReplace(aDelPam, sRepl, bRegExReplace);
                m_rDoc.GetIDocumentUndoRedo().AppendUndo(pUndoRpl);
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
        pFormat = m_rDoc.MakeFlySection_( rPos, *pNode, FLY_AT_PARA,
                                pFlyAttrSet, pFrameFormat );
        if( pGrfAttrSet )
            pNode->SetAttr( *pGrfAttrSet );
    }
    return pFormat;
}

#define NUMRULE_STATE \
     SfxItemState aNumRuleState = SfxItemState::UNKNOWN; \
     SwNumRuleItem aNumRuleItem; \
     SfxItemState aListIdState = SfxItemState::UNKNOWN; \
     SfxStringItem aListIdItem( RES_PARATR_LIST_ID, OUString() ); \

#define PUSH_NUMRULE_STATE \
     lcl_PushNumruleState( aNumRuleState, aNumRuleItem, aListIdState, aListIdItem, pDestTextNd );

#define POP_NUMRULE_STATE \
     lcl_PopNumruleState( aNumRuleState, aNumRuleItem, aListIdState, aListIdItem, pDestTextNd, rPam );

static void lcl_PushNumruleState( SfxItemState &aNumRuleState, SwNumRuleItem &aNumRuleItem,
                                  SfxItemState &aListIdState, SfxStringItem &aListIdItem,
                                  const SwTextNode *pDestTextNd )
{
    // Safe numrule item at destination.
    // #i86492# - Safe also <ListId> item of destination.
    const SfxItemSet * pAttrSet = pDestTextNd->GetpSwAttrSet();
    if (pAttrSet != nullptr)
    {
        const SfxPoolItem * pItem = nullptr;
        aNumRuleState = pAttrSet->GetItemState(RES_PARATR_NUMRULE, false, &pItem);
        if (SfxItemState::SET == aNumRuleState)
            aNumRuleItem = *static_cast<const SwNumRuleItem *>( pItem);

        aListIdState =
            pAttrSet->GetItemState(RES_PARATR_LIST_ID, false, &pItem);
        if (SfxItemState::SET == aListIdState)
        {
            aListIdItem.SetValue( static_cast<const SfxStringItem*>(pItem)->GetValue() );
        }
    }
}

static void lcl_PopNumruleState( SfxItemState aNumRuleState, const SwNumRuleItem &aNumRuleItem,
                                 SfxItemState aListIdState, const SfxStringItem &aListIdItem,
                                 SwTextNode *pDestTextNd, const SwPaM& rPam )
{
    /* If only a part of one paragraph is copied
       restore the numrule at the destination. */
    // #i86492# - restore also <ListId> item
    if ( !lcl_MarksWholeNode(rPam) )
    {
        if (SfxItemState::SET == aNumRuleState)
        {
            pDestTextNd->SetAttr(aNumRuleItem);
        }
        else
        {
            pDestTextNd->ResetAttr(RES_PARATR_NUMRULE);
        }
        if (SfxItemState::SET == aListIdState)
        {
            pDestTextNd->SetAttr(aListIdItem);
        }
        else
        {
            pDestTextNd->ResetAttr(RES_PARATR_LIST_ID);
        }
    }
}

bool DocumentContentOperationsManager::CopyImpl( SwPaM& rPam, SwPosition& rPos,
        const bool bMakeNewFrames, const bool bCopyAll,
        SwPaM *const pCpyRange ) const
{
    SwDoc* pDoc = rPos.nNode.GetNode().GetDoc();
    const bool bColumnSel = pDoc->IsClipBoard() && pDoc->IsColumnSelection();

    SwPosition* pStt = rPam.Start();
    SwPosition* pEnd = rPam.End();

    // Catch when there's no copy to do.
    if( !rPam.HasMark() || ( *pStt >= *pEnd && !bColumnSel ) ||
        //JP 29.6.2001: 88963 - don't copy if inspos is in region of start to end
        //JP 15.11.2001: don't test inclusive the end, ever exclusive
        ( pDoc == &m_rDoc && *pStt <= rPos && rPos < *pEnd ))
    {
        return false;
    }

    const bool bEndEqualIns = pDoc == &m_rDoc && rPos == *pEnd;

    // If Undo is enabled, create the UndoCopy object
    SwUndoCpyDoc* pUndo = nullptr;
    // lcl_DeleteRedlines may delete the start or end node of the cursor when
    // removing the redlines so use cursor that is corrected by PaMCorrAbs
    std::shared_ptr<SwUnoCursor> const pCopyPam(pDoc->CreateUnoCursor(rPos));

    SwTableNumFormatMerge aTNFM( m_rDoc, *pDoc );

    if (pDoc->GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoCpyDoc(*pCopyPam);
        pDoc->GetIDocumentUndoRedo().AppendUndo( pUndo );
    }

    RedlineFlags eOld = pDoc->getIDocumentRedlineAccess().GetRedlineFlags();
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern(eOld | RedlineFlags::Ignore);

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
        pCopyPam->GetPoint()->nNode--;

    SwNodeRange aRg( pStt->nNode, pEnd->nNode );
    SwNodeIndex aInsPos( rPos.nNode );
    const bool bOneNode = pStt->nNode == pEnd->nNode;
    SwTextNode* pSttTextNd = pStt->nNode.GetNode().GetTextNode();
    SwTextNode* pEndTextNd = pEnd->nNode.GetNode().GetTextNode();
    SwTextNode* pDestTextNd = aInsPos.GetNode().GetTextNode();
    bool bCopyCollFormat = !pDoc->IsInsOnlyTextGlossary() &&
                        ( (pDestTextNd && !pDestTextNd->GetText().getLength()) ||
                          ( !bOneNode && !rPos.nContent.GetIndex() ) );
    bool bCopyBookmarks = true;
    bool bCopyPageSource  = false;
    bool bStartIsTextNode = nullptr != pSttTextNd;

    // #i104585# copy outline num rule to clipboard (for ASCII filter)
    if (pDoc->IsClipBoard() && m_rDoc.GetOutlineNumRule())
    {
        pDoc->SetOutlineNumRule(*m_rDoc.GetOutlineNumRule());
    }

    // #i86492#
    // Correct the search for a previous list:
    // First search for non-outline numbering list. Then search for non-outline
    // bullet list.
    // Keep also the <ListId> value for possible propagation.
    OUString aListIdToPropagate;
    const SwNumRule* pNumRuleToPropagate =
        pDoc->SearchNumRule( rPos, false, true, false, 0, aListIdToPropagate, true );
    if ( !pNumRuleToPropagate )
    {
        pNumRuleToPropagate =
            pDoc->SearchNumRule( rPos, false, false, false, 0, aListIdToPropagate, true );
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
            // Don't copy the beginning completely?
            if( !bCopyCollFormat || bColumnSel || pStt->nContent.GetIndex() )
            {
                SwIndex aDestIdx( rPos.nContent );
                bool bCopyOk = false;
                if( !pDestTextNd )
                {
                    if( pStt->nContent.GetIndex() || bOneNode )
                        pDestTextNd = pDoc->GetNodes().MakeTextNode( aInsPos,
                            pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD));
                    else
                    {
                        pDestTextNd = pSttTextNd->MakeCopy( pDoc, aInsPos )->GetTextNode();
                        bCopyOk = true;
                    }
                    aDestIdx.Assign( pDestTextNd, 0 );
                    bCopyCollFormat = true;
                }
                else if( !bOneNode || bColumnSel )
                {
                    const sal_Int32 nContentEnd = pEnd->nContent.GetIndex();
                    {
                        ::sw::UndoGuard const ug(pDoc->GetIDocumentUndoRedo());
                        pDoc->getIDocumentContentOperations().SplitNode( rPos, false );
                    }

                    if (bCanMoveBack && rPos == *pCopyPam->GetPoint())
                    {
                        // after the SplitNode, span the CpyPam correctly again
                        pCopyPam->Move( fnMoveBackward, GoInContent );
                        pCopyPam->Move( fnMoveBackward, GoInContent );
                    }

                    pDestTextNd = pDoc->GetNodes()[ aInsPos.GetIndex()-1 ]->GetTextNode();
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
                    const sal_Int32 nCpyLen = ( (bOneNode)
                                           ? pEnd->nContent.GetIndex()
                                           : pSttTextNd->GetText().getLength())
                                         - pStt->nContent.GetIndex();
                    pSttTextNd->CopyText( pDestTextNd, aDestIdx,
                                            pStt->nContent, nCpyLen );
                    if( bEndEqualIns )
                        pEnd->nContent -= nCpyLen;
                }

                if( bOneNode )
                {
                    if (bCopyCollFormat)
                    {
                        pSttTextNd->CopyCollFormat( *pDestTextNd );
                        POP_NUMRULE_STATE
                    }

                    break;
                }

                aRg.aStart++;
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
                bStartIsTextNode = true;

                const sal_Int32 nContentEnd = pEnd->nContent.GetIndex();
                {
                    ::sw::UndoGuard const ug(pDoc->GetIDocumentUndoRedo());
                    pDoc->getIDocumentContentOperations().SplitNode( rPos, false );
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
            {   //Insertion at the first position of a text node. It will not be splitted, the table
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
                pDestTextNd = pDoc->GetNodes().MakeTextNode( aInsPos,
                            pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD));
                aDestIdx.Assign( pDestTextNd, 0  );
                aInsPos--;

                // if we have to insert an extra text node
                // at the destination, this node will be our new destination
                // (text) node, and thus we set bStartisTextNode to true. This
                // will ensure that this node will be deleted during Undo
                // using JoinNext.
                OSL_ENSURE( !bStartIsTextNode, "Oops, undo may be instable now." );
                bStartIsTextNode = true;
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
                pEndTextNd->CopyCollFormat( *pDestTextNd );
                if ( bOneNode )
                {
                    POP_NUMRULE_STATE
                }
            }
        }

        if( bCopyAll || aRg.aStart != aRg.aEnd )
        {
            SfxItemSet aBrkSet( pDoc->GetAttrPool(), aBreakSetRange );
            if (pSttTextNd && bCopyCollFormat && pDestTextNd->HasSwAttrSet())
            {
                aBrkSet.Put( *pDestTextNd->GetpSwAttrSet() );
                if( SfxItemState::SET == aBrkSet.GetItemState( RES_BREAK, false ) )
                    pDestTextNd->ResetAttr( RES_BREAK );
                if( SfxItemState::SET == aBrkSet.GetItemState( RES_PAGEDESC, false ) )
                    pDestTextNd->ResetAttr( RES_PAGEDESC );
            }

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
                CopyWithFlyInFly( aRg, 0, aInsPos, &tmp, bMakeNewFrames, false );
                ++aSaveIdx;
                pEnd->nNode = aSaveIdx;
                pEnd->nContent.Assign( aSaveIdx.GetNode().GetTextNode(), 0 );
            }
            else
                CopyWithFlyInFly( aRg, pEnd->nContent.GetIndex(), aInsPos, &tmp, bMakeNewFrames, false );

            bCopyBookmarks = false;

            // Put the breaks back into the first node
            if( aBrkSet.Count() && nullptr != ( pDestTextNd = pDoc->GetNodes()[
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
    if (pDoc->IsClipBoard() && (rPam.GetPageNum(pStt == rPam.GetPoint()) == 1) && !bCopyPageSource)
    {
        pDestTextNd->ResetAttr(RES_BREAK);        // remove the page-break
        pDestTextNd->ResetAttr(RES_PAGEDESC);
    }


    // Adjust position (in case it was moved / in another node)
    rPos.nContent.Assign( rPos.nNode.GetNode().GetContentNode(),
                            rPos.nContent.GetIndex() );

    if( rPos.nNode != aInsPos )
    {
        pCopyPam->GetMark()->nNode = aInsPos;
        pCopyPam->GetMark()->nContent.Assign(pCopyPam->GetContentNode(false), 0);
        rPos = *pCopyPam->GetMark();
    }
    else
        *pCopyPam->GetMark() = rPos;

    if ( !bAfterTable )
        pCopyPam->Move( fnMoveForward, bCanMoveBack ? GoInContent : GoInNode );
    else
    {
        // Reset the offset to 0 as it was before the insertion
        pCopyPam->GetPoint()->nContent = 0;

        pCopyPam->GetPoint()->nNode++;
        // If the next node is a start node, then step back: the start node
        // has been copied and needs to be in the selection for the undo
        if (pCopyPam->GetPoint()->nNode.GetNode().IsStartNode())
            pCopyPam->GetPoint()->nNode--;

    }
    pCopyPam->Exchange();

    // Also copy all bookmarks
    if( bCopyBookmarks && m_rDoc.getIDocumentMarkAccess()->getAllMarksCount() )
        lcl_CopyBookmarks( rPam, *pCopyPam );

    if( RedlineFlags::DeleteRedlines & eOld )
    {
        assert(*pCopyPam->GetPoint() == rPos);
        // the Node rPos points to may be deleted so unregister ...
        rPos.nContent.Assign(nullptr, 0);
        lcl_DeleteRedlines(rPam, *pCopyPam);
        rPos = *pCopyPam->GetPoint(); // ... and restore.
    }

    // If Undo is enabled, store the inserted area
    if (pDoc->GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo->SetInsertRange( *pCopyPam, true, bStartIsTextNode );
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
        pDoc->SetNumRule( *pCopyPam, *pNumRuleToPropagate, false,
                          aListIdToPropagate, true, true );
    }

    pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
    pDoc->getIDocumentState().SetModified();

    return true;
}


}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
