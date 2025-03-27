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

#include <memory>
#include <utility>

#include <MarkManager.hxx>
#include <bookmark.hxx>
#include <crossrefbookmark.hxx>
#include <crsrsh.hxx>
#include <annotationmark.hxx>
#include <doc.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentState.hxx>
#include <IDocumentUndoRedo.hxx>
#include <docary.hxx>
#include <xmloff/odffields.hxx>
#include <mvsave.hxx>
#include <ndtxt.hxx>
#include <node.hxx>
#include <pam.hxx>
#include <redline.hxx>
#include <rolbck.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <sal/log.hxx>
#include <UndoBookmark.hxx>
#include <tools/datetimeutils.hxx>
#include <txtfrm.hxx>
#include <view.hxx>

#include <libxml/xmlstring.h>
#include <libxml/xmlwriter.h>
#include <comphelper/lok.hxx>
#include <strings.hrc>

constexpr OUString S_ANNOTATION_BOOKMARK = u"____"_ustr;

using namespace ::sw::mark;

static bool IsAnnotationMark(const sw::mark::MarkBase& rBkmk);

namespace
{
    bool lcl_GreaterThan( const SwPosition& rPos, const SwNode& rNdIdx, std::optional<sal_Int32> oContentIdx )
    {
        return oContentIdx.has_value()
               ? ( rPos.GetNode() > rNdIdx
                   || ( rPos.GetNode() == rNdIdx
                        && rPos.GetContentIndex() >= *oContentIdx ) )
               : rPos.GetNode() >= rNdIdx;
    }

    bool lcl_Lower( const SwPosition& rPos, const SwNode& rNdIdx, std::optional<sal_Int32> oContentIdx )
    {
        if (rPos.GetNode() < rNdIdx)
            return true;

        if (rPos.GetNode() != rNdIdx || !oContentIdx)
            return false;

        if (rPos.GetContentIndex() < *oContentIdx)
            return true;

        // paragraph end selected?
        return rNdIdx.IsTextNode() && *oContentIdx == rNdIdx.GetTextNode()->Len();
    }

    template<class MarkT>
    bool lcl_MarkOrderingByStart(const MarkT *const pFirst,
                                 const MarkT *const pSecond)
    {
        SwPosition const& rFirstStart(pFirst->GetMarkStart());
        SwPosition const& rSecondStart(pSecond->GetMarkStart());
        if (rFirstStart.GetNode() != rSecondStart.GetNode())
        {
            return rFirstStart.GetNode() < rSecondStart.GetNode();
        }
        const sal_Int32 nFirstContent = rFirstStart.GetContentIndex();
        const sal_Int32 nSecondContent = rSecondStart.GetContentIndex();
        if (nFirstContent != 0 || nSecondContent != 0)
        {
            return nFirstContent < nSecondContent;
        }
        SwContentNode const*const pFirstNode(rFirstStart.nContent.GetContentNode());
        SwContentNode const*const pSecondNode(rSecondStart.nContent.GetContentNode());
        if ((pFirstNode != nullptr) != (pSecondNode != nullptr))
        {   // consistency with SwPosition::operator<
            return pSecondNode != nullptr;
        }
        bool const bCRFirst (IsCrossRefBookmark(*pFirst));
        bool const bCRSecond(IsCrossRefBookmark(*pSecond));
        if (bCRFirst == bCRSecond)
        {
            return false; // equal
        }
        return bCRFirst; // cross-ref sorts *before*
    }

    // specialise to avoid loplugin:faileddyncast
    template<>
    bool lcl_MarkOrderingByStart(const AnnotationMark *const pFirst,
                                 const AnnotationMark *const pSecond)
    {
        SwPosition const& rFirstStart(pFirst->GetMarkStart());
        SwPosition const& rSecondStart(pSecond->GetMarkStart());
        if (rFirstStart.GetNode() != rSecondStart.GetNode())
        {
            return rFirstStart.GetNode() < rSecondStart.GetNode();
        }
        const sal_Int32 nFirstContent = rFirstStart.GetContentIndex();
        const sal_Int32 nSecondContent = rSecondStart.GetContentIndex();
        if (nFirstContent != 0 || nSecondContent != 0)
        {
            return nFirstContent < nSecondContent;
        }
        SwContentNode const*const pFirstNode(rFirstStart.nContent.GetContentNode());
        SwContentNode const*const pSecondNode(rSecondStart.nContent.GetContentNode());
        if ((pFirstNode != nullptr) != (pSecondNode != nullptr))
        {   // consistency with SwPosition::operator<
            return pSecondNode != nullptr;
        }
        return false; // equal
    }

    template<class MarkT>
    bool lcl_MarkOrderingByEnd(const MarkT *const pFirst,
                               const MarkT *const pSecond)
    {
        return pFirst->GetMarkEnd() < pSecond->GetMarkEnd();
    }

    template<class MarkT>
    void lcl_InsertMarkSorted(std::vector<MarkT*>& io_vMarks,
                              MarkT *const pMark)
    {
        io_vMarks.insert(
            lower_bound(
                io_vMarks.begin(),
                io_vMarks.end(),
                pMark,
                &lcl_MarkOrderingByStart<MarkT>),
            pMark);
    }

    void lcl_PositionFromContentNode(
        std::optional<SwPosition>& rFoundPos,
        const SwContentNode * const pContentNode,
        const bool bAtEnd)
    {
        rFoundPos.emplace(*pContentNode, bAtEnd ? pContentNode->Len() : 0);
    }

    // return a position at the begin of rEnd, if it is a ContentNode
    // else set it to the begin of the Node after rEnd, if there is one
    // else set it to the end of the node before rStt
    // else set it to the ContentNode of the Pos outside the Range
    void lcl_FindExpelPosition(
        std::optional<SwPosition>& rFoundPos,
        const SwNode& rStt,
        const SwNode& rEnd,
        const SwPosition& rOtherPosition)
    {
        const SwContentNode * pNode = rEnd.GetContentNode();
        bool bPosAtEndOfNode = false;
        if ( pNode == nullptr)
        {
            SwNodeIndex aEnd(rEnd);
            pNode = SwNodes::GoNext(&aEnd);
            bPosAtEndOfNode = false;
        }
        if ( pNode == nullptr )
        {
            SwNodeIndex aStt(rStt);
            pNode = SwNodes::GoPrevious(&aStt);
            bPosAtEndOfNode = true;
        }
        if ( pNode != nullptr )
        {
            lcl_PositionFromContentNode( rFoundPos, pNode, bPosAtEndOfNode );
            return;
        }

        rFoundPos = rOtherPosition;
    }

    template<class MarkT>
    struct CompareIMarkStartsBefore
    {
        bool operator()(SwPosition const& rPos, const  MarkT* pMark)
        {
            return rPos < pMark->GetMarkStart();
        }
        bool operator()(const  MarkT* pMark, SwPosition const& rPos)
        {
            return pMark->GetMarkStart() < rPos;
        }
        bool operator()(const  MarkT* pMark, SwNode const& rPos)
        {
            return pMark->GetMarkStart().GetNode() < rPos;
        }
    };

    // Apple llvm-g++ 4.2.1 with _GLIBCXX_DEBUG won't eat boost::bind for this
    // Neither will MSVC 2008 with _DEBUG
    template<class MarkT>
    struct CompareIMarkStartsAfter
    {
        bool operator()(SwPosition const& rPos, const MarkT* pMark)
        {
            return pMark->GetMarkStart() > rPos;
        }
    };

    struct CompareIMarkStartsAfterReverse
    {
        bool operator()(const sw::mark::MarkBase* pMark, SwPosition const& rPos)
        {
            return pMark->GetMarkStart() > rPos;
        }
    };

    template<class MarkT>
    MarkT* lcl_getMarkAfter(const std::vector<MarkT*>& rMarks, const SwPosition& rPos,
                            bool bLoop)
    {
        auto const pMarkAfter = upper_bound(
            rMarks.begin(),
            rMarks.end(),
            rPos,
            CompareIMarkStartsAfter<MarkT>());
        if(pMarkAfter == rMarks.end())
        {
            if (bLoop && rMarks.begin() != rMarks.end())
                return *rMarks.begin();

            return nullptr;
        }
        return *pMarkAfter;
    };

    template<class MarkT>
    MarkT* lcl_getMarkBefore(const std::vector<MarkT*>& rMarks, const SwPosition& rPos,
                             bool bLoop)
    {
        // candidates from which to choose the mark before
        std::vector<MarkT*> vCandidates;
        // no need to consider marks starting after rPos
        auto const pCandidatesEnd = upper_bound(
            rMarks.begin(),
            rMarks.end(),
            rPos,
            CompareIMarkStartsAfter<MarkT>());
        vCandidates.reserve(pCandidatesEnd - rMarks.begin());
        // only marks ending before are candidates
        remove_copy_if(
            rMarks.begin(),
            pCandidatesEnd,
            back_inserter(vCandidates),
            [&rPos] (const MarkT *const pMark) { return !(pMark->GetMarkEnd() < rPos); } );
        // no candidate left => we are in front of the first mark or there are none
        if(vCandidates.empty())
        {
            if (bLoop && rMarks.begin() != rMarks.end())
                return *(rMarks.end() - 1);

            return nullptr;
        }
        // return the highest (last) candidate using mark end ordering
        return *max_element(vCandidates.begin(), vCandidates.end(), &lcl_MarkOrderingByEnd<MarkT>);
    }

    bool lcl_FixCorrectedMark(
        const bool bChangedPos,
        const bool bChangedOPos,
        MarkBase* io_pMark )
    {
        if ( IsAnnotationMark(*io_pMark) )
        {
            // annotation marks are allowed to span a table cell range.
            // but trigger sorting to be save
            return true;
        }

        if ( ( bChangedPos || bChangedOPos )
             && io_pMark->IsExpanded()
             && io_pMark->GetOtherMarkPos().GetNode().FindTableBoxStartNode() !=
                    io_pMark->GetMarkPos().GetNode().FindTableBoxStartNode() )
        {
            if ( !bChangedOPos )
            {
                io_pMark->SetMarkPos( io_pMark->GetOtherMarkPos() );
            }
            io_pMark->ClearOtherMarkPos();
            DdeBookmark * const pDdeBkmk = dynamic_cast< DdeBookmark*>(io_pMark);
            if ( pDdeBkmk != nullptr
                 && pDdeBkmk->IsServer() )
            {
                pDdeBkmk->SetRefObject(nullptr);
            }
            return true;
        }
        return false;
    }

    template<class MarkT>
    bool lcl_MarkEqualByStart(const MarkT *const pFirst,
                              const MarkT *const pSecond)
    {
        return !lcl_MarkOrderingByStart<MarkT>(pFirst, pSecond) &&
               !lcl_MarkOrderingByStart<MarkT>(pSecond, pFirst);
    }

    template<class MarkT>
    typename std::vector<MarkT*>::const_iterator lcl_FindMark(
        std::vector<MarkT*>& rMarks,
        const MarkT *const pMarkToFind)
    {
        auto ppCurrentMark = lower_bound(
            rMarks.begin(), rMarks.end(),
            pMarkToFind, &lcl_MarkOrderingByStart<MarkT>);
        // since there are usually not too many marks on the same start
        // position, we are not doing a bisect search for the upper bound
        // but instead start to iterate from pMarkLow directly
        while (ppCurrentMark != rMarks.end() && lcl_MarkEqualByStart<MarkT>(*ppCurrentMark, pMarkToFind))
        {
            if(*ppCurrentMark == pMarkToFind)
            {
                return ppCurrentMark;
            }
            ++ppCurrentMark;
        }
        // reached a mark starting on a later start pos or the end of the
        // vector => not found
        return rMarks.end();
    };

    template<class MarkT>
    typename std::vector<MarkT*>::const_iterator lcl_FindMarkAtPos(
        std::vector<MarkT*>& rMarks,
        const SwPosition& rPos,
        const IDocumentMarkAccess::MarkType eType)
    {
        for (auto ppCurrentMark = lower_bound(
                rMarks.begin(), rMarks.end(),
                rPos,
                CompareIMarkStartsBefore<MarkT>());
            ppCurrentMark != rMarks.end();
            ++ppCurrentMark)
        {
            // Once we reach a mark starting after the target pos
            // we do not need to continue
            if((*ppCurrentMark)->GetMarkStart() > rPos)
                break;
            if(IDocumentMarkAccess::GetType(**ppCurrentMark) == eType)
            {
                return ppCurrentMark;
            }
        }
        // reached a mark starting on a later start pos or the end of the
        // vector => not found
        return rMarks.end();
    };

    template <class container_t>
    auto lcl_FindMarkByName(const OUString& rName, const container_t& container)
    {
        return find_if(begin(container), end(container),
                       [&rName](const auto* item) { return item->GetName() == rName; });
    }

    template<class MarkT>
    void lcl_DebugMarks(std::vector<MarkT*> const& rMarks)
    {
#if OSL_DEBUG_LEVEL > 0
        SAL_INFO("sw.core", rMarks.size() << " Marks");
        for (auto ppMark = rMarks.begin();
             ppMark != rMarks.end();
             ++ppMark)
        {
            MarkT* pMark = *ppMark;
            const SwPosition* const pStPos = &pMark->GetMarkStart();
            const SwPosition* const pEndPos = &pMark->GetMarkEnd();
            SAL_INFO("sw.core",
                sal_Int32(pStPos->GetNodeIndex()) << "," <<
                pStPos->GetContentIndex() << " " <<
                sal_Int32(pEndPos->GetNodeIndex()) << "," <<
                pEndPos->GetContentIndex() << " " <<
                typeid(*pMark).name() << " " <<
                pMark->GetName());
        }
#else
        (void) rMarks;
#endif
        assert(std::is_sorted(rMarks.begin(), rMarks.end(), lcl_MarkOrderingByStart<MarkT>));
    };
}

static bool IsNavigatorReminder(const MarkBase& rBkmk)
{
    const std::type_info* const pMarkTypeInfo = &typeid(rBkmk);
    // not using dynamic_cast<> here for performance
    return (*pMarkTypeInfo == typeid(NavigatorReminder));
}

static bool IsCrossRefBookmark(const sw::mark::MarkBase& rBkmk)
{
    // not using dynamic_cast<> here for performance
    const std::type_info* const pMarkTypeInfo = &typeid(rBkmk);
    return (*pMarkTypeInfo == typeid(CrossRefHeadingBookmark))
        || (*pMarkTypeInfo == typeid(CrossRefNumItemBookmark));
}

static bool IsAnnotationMark(const sw::mark::MarkBase& rBkmk)
{
    // not using dynamic_cast<> here for performance
    const std::type_info* const pMarkTypeInfo = &typeid(rBkmk);
    return (*pMarkTypeInfo == typeid(AnnotationMark));
}

IDocumentMarkAccess::MarkType IDocumentMarkAccess::GetType(const MarkBase& rBkmk)
{
    const std::type_info* const pMarkTypeInfo = &typeid(rBkmk);
    // not using dynamic_cast<> here for performance
    if(*pMarkTypeInfo == typeid(UnoMark))
        return MarkType::UNO_BOOKMARK;
    else if(*pMarkTypeInfo == typeid(DdeBookmark))
        return MarkType::DDE_BOOKMARK;
    else if(*pMarkTypeInfo == typeid(Bookmark))
        return MarkType::BOOKMARK;
    else if(*pMarkTypeInfo == typeid(CrossRefHeadingBookmark))
        return MarkType::CROSSREF_HEADING_BOOKMARK;
    else if(*pMarkTypeInfo == typeid(CrossRefNumItemBookmark))
        return MarkType::CROSSREF_NUMITEM_BOOKMARK;
    else if(*pMarkTypeInfo == typeid(AnnotationMark))
        return MarkType::ANNOTATIONMARK;
    else if(*pMarkTypeInfo == typeid(TextFieldmark))
        return MarkType::TEXT_FIELDMARK;
    else if(*pMarkTypeInfo == typeid(CheckboxFieldmark))
        return MarkType::CHECKBOX_FIELDMARK;
    else if(*pMarkTypeInfo == typeid(DropDownFieldmark))
        return MarkType::DROPDOWN_FIELDMARK;
    else if(*pMarkTypeInfo == typeid(DateFieldmark))
        return MarkType::DATE_FIELDMARK;
    else if(*pMarkTypeInfo == typeid(NavigatorReminder))
        return MarkType::NAVIGATOR_REMINDER;
    else
    {
        assert(false && "IDocumentMarkAccess::GetType(..)"
            " - unknown MarkType. This needs to be fixed!");
        return MarkType::UNO_BOOKMARK;
    }
}

OUString IDocumentMarkAccess::GetCrossRefHeadingBookmarkNamePrefix()
{
    return u"__RefHeading__"_ustr;
}

bool IDocumentMarkAccess::IsLegalPaMForCrossRefHeadingBookmark( const SwPaM& rPaM )
{
    return rPaM.Start()->GetNode().IsTextNode() &&
           rPaM.Start()->GetContentIndex() == 0 &&
           ( !rPaM.HasMark() ||
             ( rPaM.GetMark()->GetNode() == rPaM.GetPoint()->GetNode() &&
               rPaM.End()->GetContentIndex() == rPaM.End()->GetNode().GetTextNode()->Len() ) );
}

void IDocumentMarkAccess::DeleteFieldmarkCommand(::sw::mark::Fieldmark const& rMark)
{
    if (GetType(rMark) != MarkType::TEXT_FIELDMARK)
    {
        return; // TODO FORMDATE has no command?
    }
    SwPaM pam(sw::mark::FindFieldSep(rMark), rMark.GetMarkStart());
    pam.GetPoint()->AdjustContent(+1); // skip CH_TXT_ATR_FIELDSTART
    pam.GetDoc().getIDocumentContentOperations().DeleteAndJoin(pam);
}

namespace sw::mark
{
    MarkManager::MarkManager(SwDoc& rDoc)
        : m_rDoc(rDoc)
        , m_pLastActiveFieldmark(nullptr)
    { }

    ::sw::mark::MarkBase* MarkManager::makeMark(const SwPaM& rPaM,
        const OUString& rName,
        const IDocumentMarkAccess::MarkType eType,
        sw::mark::InsertMode const eMode,
        SwPosition const*const pSepPos)
    {
#if OSL_DEBUG_LEVEL > 0
        {
            const SwPosition* const pPos1 = rPaM.GetPoint();
            const SwPosition* pPos2 = pPos1;
            if(rPaM.HasMark())
                pPos2 = rPaM.GetMark();
            SAL_INFO("sw.core",
                rName << " " <<
                sal_Int32(pPos1->GetNodeIndex() )<< "," <<
                pPos1->GetContentIndex() << " " <<
                sal_Int32(pPos2->GetNodeIndex()) << "," <<
                pPos2->GetContentIndex());
        }
#endif
        if (   (!rPaM.GetPoint()->GetNode().IsTextNode()
                && (eType != MarkType::UNO_BOOKMARK
                // SwXTextRange can be on table node or plain start node (FLY_AT_FLY)
                    || !rPaM.GetPoint()->GetNode().IsStartNode()))
            || (!rPaM.GetMark()->GetNode().IsTextNode()
                && (eType != MarkType::UNO_BOOKMARK
                    || !rPaM.GetMark()->GetNode().IsStartNode())))
        {
            SAL_WARN("sw.core", "MarkManager::makeMark(..)"
                " - refusing to create mark on non-textnode");
            return nullptr;
        }
        // There should only be one CrossRefBookmark per Textnode per Type
        if ((eType == MarkType::CROSSREF_NUMITEM_BOOKMARK || eType == MarkType::CROSSREF_HEADING_BOOKMARK)
            && (lcl_FindMarkAtPos(m_vBookmarks, *rPaM.Start(), eType) != m_vBookmarks.end()))
        {   // this can happen via UNO API
            SAL_WARN("sw.core", "MarkManager::makeMark(..)"
                " - refusing to create duplicate CrossRefBookmark");
            return nullptr;
        }

        if ((eType == MarkType::CHECKBOX_FIELDMARK || eType == MarkType::DROPDOWN_FIELDMARK)
            && (eMode == InsertMode::New
                ? *rPaM.GetPoint() != *rPaM.GetMark()
                // CopyText: pam covers CH_TXT_ATR_FORMELEMENT
                : (rPaM.GetPoint()->GetNode() != rPaM.GetMark()->GetNode()
                    || rPaM.Start()->GetContentIndex() + 1 != rPaM.End()->GetContentIndex())))
        {
            SAL_WARN("sw.core", "MarkManager::makeMark(..)"
                " - invalid range on point fieldmark");
            return nullptr;
        }

        if ((eType == MarkType::TEXT_FIELDMARK || eType == MarkType::DATE_FIELDMARK)
            && (rPaM.GetPoint()->GetNode().StartOfSectionNode() != rPaM.GetMark()->GetNode().StartOfSectionNode()
                || (pSepPos && rPaM.GetPoint()->GetNode().StartOfSectionNode() != pSepPos->GetNode().StartOfSectionNode())))
        {
            SAL_WARN("sw.core", "MarkManager::makeMark(..)"
                " - invalid range on fieldmark, different nodes array sections");
            return nullptr;
        }

        if ((eType == MarkType::TEXT_FIELDMARK || eType == MarkType::DATE_FIELDMARK)
            // can't check for Copy - it asserts - but it's also obviously unnecessary
            && eMode == InsertMode::New
            && sw::mark::IsFieldmarkOverlap(rPaM))
        {
            SAL_WARN("sw.core", "MarkManager::makeMark(..)"
                " - invalid range on fieldmark, overlaps existing fieldmark or meta-field");
            return nullptr;
        }

        // create mark
        std::unique_ptr<::sw::mark::MarkBase> pMark;
        switch(eType)
        {
            case IDocumentMarkAccess::MarkType::TEXT_FIELDMARK:
                pMark = std::make_unique<TextFieldmark>(rPaM, rName);
                break;
            case IDocumentMarkAccess::MarkType::CHECKBOX_FIELDMARK:
                pMark = std::make_unique<CheckboxFieldmark>(rPaM, rName);
                break;
            case IDocumentMarkAccess::MarkType::DROPDOWN_FIELDMARK:
                pMark = std::make_unique<DropDownFieldmark>(rPaM, rName);
                break;
            case IDocumentMarkAccess::MarkType::DATE_FIELDMARK:
                pMark = std::make_unique<DateFieldmark>(rPaM);
                break;
            case IDocumentMarkAccess::MarkType::NAVIGATOR_REMINDER:
                pMark = std::make_unique<NavigatorReminder>(rPaM);
                break;
            case IDocumentMarkAccess::MarkType::BOOKMARK:
                pMark = std::make_unique<Bookmark>(rPaM, vcl::KeyCode(), rName);
                break;
            case IDocumentMarkAccess::MarkType::DDE_BOOKMARK:
                pMark = std::make_unique<DdeBookmark>(rPaM);
                break;
            case IDocumentMarkAccess::MarkType::CROSSREF_HEADING_BOOKMARK:
                pMark = std::make_unique<CrossRefHeadingBookmark>(rPaM, vcl::KeyCode(), rName);
                break;
            case IDocumentMarkAccess::MarkType::CROSSREF_NUMITEM_BOOKMARK:
                pMark = std::make_unique<CrossRefNumItemBookmark>(rPaM, vcl::KeyCode(), rName);
                break;
            case IDocumentMarkAccess::MarkType::UNO_BOOKMARK:
                pMark = std::make_unique<UnoMark>(rPaM);
                break;
            case IDocumentMarkAccess::MarkType::ANNOTATIONMARK:
                pMark = std::make_unique<AnnotationMark>( rPaM, rName );
                break;
        }
        assert(pMark && "MarkManager::makeMark(..) - Mark was not created.");

        if(pMark->GetMarkPos() != pMark->GetMarkStart())
            pMark->Swap();

        // for performance reasons, we trust UnoMarks to have a (generated) unique name
        if ( eType != IDocumentMarkAccess::MarkType::UNO_BOOKMARK )
            pMark->SetName( getUniqueMarkName( pMark->GetName() ) );

        // insert any dummy chars before inserting into sorted vectors
        pMark->InitDoc(m_rDoc, eMode, pSepPos);

        // register mark
        lcl_InsertMarkSorted(m_vAllMarks, pMark.get());
        switch(eType)
        {
            case IDocumentMarkAccess::MarkType::BOOKMARK:
            case IDocumentMarkAccess::MarkType::CROSSREF_NUMITEM_BOOKMARK:
            case IDocumentMarkAccess::MarkType::CROSSREF_HEADING_BOOKMARK:
                lcl_InsertMarkSorted(m_vBookmarks, static_cast<Bookmark*>(pMark.get()));
                break;
            case IDocumentMarkAccess::MarkType::TEXT_FIELDMARK:
            case IDocumentMarkAccess::MarkType::CHECKBOX_FIELDMARK:
            case IDocumentMarkAccess::MarkType::DROPDOWN_FIELDMARK:
            case IDocumentMarkAccess::MarkType::DATE_FIELDMARK:
                lcl_InsertMarkSorted(m_vFieldmarks, static_cast<Fieldmark*>(pMark.get()));
                break;
            case IDocumentMarkAccess::MarkType::ANNOTATIONMARK:
                lcl_InsertMarkSorted( m_vAnnotationMarks, static_cast<AnnotationMark*>(pMark.get()) );
                break;
            case IDocumentMarkAccess::MarkType::NAVIGATOR_REMINDER:
            case IDocumentMarkAccess::MarkType::DDE_BOOKMARK:
            case IDocumentMarkAccess::MarkType::UNO_BOOKMARK:
                // no special array for these
                break;
        }
        if (eMode == InsertMode::New
            && (eType == IDocumentMarkAccess::MarkType::TEXT_FIELDMARK
                || eType == IDocumentMarkAccess::MarkType::DATE_FIELDMARK))
        {
            // due to sw::InsertText notifications everything is visible now - tell
            // layout to hide as appropriate
            // note: we don't know how many layouts there are and which
            // parts they hide, so just notify the entire fieldmark, it
            // should give the right result if not in the most efficient way
            // note2: can't be done in InitDoc() because it requires the mark
            // to be inserted in the vectors.
            SwPaM const tmp(pMark->GetMarkPos(), pMark->GetOtherMarkPos());
            sw::UpdateFramesForAddDeleteRedline(m_rDoc, tmp);
        }

        SAL_INFO("sw.core", "--- makeType ---");
        SAL_INFO("sw.core", "Marks");
        lcl_DebugMarks(m_vAllMarks);
        SAL_INFO("sw.core", "Bookmarks");
        lcl_DebugMarks(m_vBookmarks);
        SAL_INFO("sw.core", "Fieldmarks");
        lcl_DebugMarks(m_vFieldmarks);

        return pMark.release();
    }

    ::sw::mark::Fieldmark* MarkManager::makeFieldBookmark(
        const SwPaM& rPaM,
        const OUString& rName,
        const OUString& rType,
        SwPosition const*const pSepPos)
    {

        // Disable undo, because we handle it using SwUndoInsTextFieldmark
        bool bUndoIsEnabled = m_rDoc.GetIDocumentUndoRedo().DoesUndo();
        m_rDoc.GetIDocumentUndoRedo().DoUndo(false);

        sw::mark::MarkBase* pMark = nullptr;
        if(rType == ODF_FORMDATE)
        {
            pMark = makeMark(rPaM, rName,
                             IDocumentMarkAccess::MarkType::DATE_FIELDMARK,
                             sw::mark::InsertMode::New,
                             pSepPos);
        }
        else
        {
            pMark = makeMark(rPaM, rName,
                             IDocumentMarkAccess::MarkType::TEXT_FIELDMARK,
                             sw::mark::InsertMode::New,
                             pSepPos);
        }
        sw::mark::Fieldmark* pFieldMark = dynamic_cast<sw::mark::Fieldmark*>( pMark );
        if (pFieldMark)
            pFieldMark->SetFieldname( rType );

        if (bUndoIsEnabled)
        {
            m_rDoc.GetIDocumentUndoRedo().DoUndo(bUndoIsEnabled);
            if (pFieldMark)
                m_rDoc.GetIDocumentUndoRedo().AppendUndo(std::make_unique<SwUndoInsTextFieldmark>(*pFieldMark));
        }

        return pFieldMark;
    }

    ::sw::mark::Fieldmark* MarkManager::makeNoTextFieldBookmark(
        const SwPaM& rPaM,
        const OUString& rName,
        const OUString& rType)
    {
        // Disable undo, because we handle it using SwUndoInsNoTextFieldmark
        bool bUndoIsEnabled = m_rDoc.GetIDocumentUndoRedo().DoesUndo();
        m_rDoc.GetIDocumentUndoRedo().DoUndo(false);

        bool bEnableSetModified = m_rDoc.getIDocumentState().IsEnableSetModified();
        m_rDoc.getIDocumentState().SetEnableSetModified(false);

        sw::mark::MarkBase* pMark = nullptr;
        if(rType == ODF_FORMCHECKBOX)
        {
            pMark = makeMark( rPaM, rName,
                    IDocumentMarkAccess::MarkType::CHECKBOX_FIELDMARK,
                    sw::mark::InsertMode::New);
        }
        else if(rType == ODF_FORMDROPDOWN)
        {
            pMark = makeMark( rPaM, rName,
                    IDocumentMarkAccess::MarkType::DROPDOWN_FIELDMARK,
                    sw::mark::InsertMode::New);
        }
        else if(rType == ODF_FORMDATE)
        {
            pMark = makeMark( rPaM, rName,
                    IDocumentMarkAccess::MarkType::DATE_FIELDMARK,
                    sw::mark::InsertMode::New);
        }

        sw::mark::Fieldmark* pFieldMark = dynamic_cast<sw::mark::Fieldmark*>( pMark );
        if (pFieldMark)
            pFieldMark->SetFieldname( rType );

        if (bUndoIsEnabled)
        {
            m_rDoc.GetIDocumentUndoRedo().DoUndo(bUndoIsEnabled);
            if (pFieldMark)
                m_rDoc.GetIDocumentUndoRedo().AppendUndo(std::make_unique<SwUndoInsNoTextFieldmark>(*pFieldMark));
        }

        m_rDoc.getIDocumentState().SetEnableSetModified(bEnableSetModified);
        m_rDoc.getIDocumentState().SetModified();

        return pFieldMark;
    }

    ::sw::mark::MarkBase* MarkManager::getMarkForTextNode(
        const SwTextNode& rTextNode,
        const IDocumentMarkAccess::MarkType eType )
    {
        SwPosition aPos(rTextNode);
        auto const ppExistingMark = lcl_FindMarkAtPos(m_vBookmarks, aPos, eType);
        if(ppExistingMark != m_vBookmarks.end())
            return *ppExistingMark;
        const SwPaM aPaM(aPos);
        return makeMark(aPaM, OUString(), eType, sw::mark::InsertMode::New);
    }

    sw::mark::MarkBase* MarkManager::makeAnnotationMark(
        const SwPaM& rPaM,
        const OUString& rName )
    {
        return makeMark(rPaM, rName, IDocumentMarkAccess::MarkType::ANNOTATIONMARK,
                sw::mark::InsertMode::New);
    }

    void MarkManager::repositionMark(
        ::sw::mark::MarkBase* const io_pMark,
        const SwPaM& rPaM)
    {
        assert(&io_pMark->GetMarkPos().GetDoc() == &m_rDoc &&
            "<MarkManager::repositionMark(..)>"
            " - Mark is not in my doc.");
        MarkBase* const pMarkBase = io_pMark;
        if (!pMarkBase)
            return;

        pMarkBase->InvalidateFrames();

        pMarkBase->SetMarkPos(*(rPaM.GetPoint()));
        if(rPaM.HasMark())
            pMarkBase->SetOtherMarkPos(*(rPaM.GetMark()));
        else
            pMarkBase->ClearOtherMarkPos();

        if(pMarkBase->GetMarkPos() != pMarkBase->GetMarkStart())
            pMarkBase->Swap();

        pMarkBase->InvalidateFrames();

        sortMarks();
    }

    bool MarkManager::renameMark(
        ::sw::mark::MarkBase* io_pMark,
        const OUString& rNewName )
    {
        assert(&io_pMark->GetMarkPos().GetDoc() == &m_rDoc &&
            "<MarkManager::renameMark(..)>"
            " - Mark is not in my doc.");
        if ( io_pMark->GetName() == rNewName )
            return true;
        if (lcl_FindMarkByName(rNewName, m_vAllMarks) != m_vAllMarks.end())
            return false;
        const OUString sOldName(io_pMark->GetName());
        io_pMark->SetName(rNewName);

        if (dynamic_cast< ::sw::mark::Bookmark* >(io_pMark))
        {
            if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
            {
                m_rDoc.GetIDocumentUndoRedo().AppendUndo(
                        std::make_unique<SwUndoRenameBookmark>(sOldName, rNewName, m_rDoc));
            }
            m_rDoc.getIDocumentState().SetModified();
        }
        return true;
    }

    void MarkManager::correctMarksAbsolute(
        const SwNode& rOldNode,
        const SwPosition& rNewPos,
        const sal_Int32 nOffset)
    {
        const SwNode* const pOldNode = &rOldNode;
        SwPosition aNewPos(rNewPos);
        aNewPos.AdjustContent(nOffset);
        bool isSortingNeeded = false;

        for (auto ppMark = m_vAllMarks.begin();
            ppMark != m_vAllMarks.end();
            ++ppMark)
        {
            ::sw::mark::MarkBase *const pMark = *ppMark;
            // correction of non-existent non-MarkBase instances cannot be done
            assert(pMark);
            // is on position ??
            bool bChangedPos = false;
            if(&pMark->GetMarkPos().GetNode() == pOldNode)
            {
                pMark->SetMarkPos(aNewPos);
                bChangedPos = true;
                isSortingNeeded = true;
            }
            bool bChangedOPos = false;
            if (pMark->IsExpanded() &&
                &pMark->GetOtherMarkPos().GetNode() == pOldNode)
            {
                // shift the OtherMark to aNewPos
                pMark->SetOtherMarkPos(aNewPos);
                bChangedOPos= true;
                isSortingNeeded = true;
            }
            // illegal selection? collapse the mark and restore sorting later
            isSortingNeeded |= lcl_FixCorrectedMark(bChangedPos, bChangedOPos, pMark);
        }

        // restore sorting if needed
        if(isSortingNeeded)
            sortMarks();

        SAL_INFO("sw.core", "correctMarksAbsolute");
        lcl_DebugMarks(m_vAllMarks);
    }

    void MarkManager::correctMarksRelative(const SwNode& rOldNode, const SwPosition& rNewPos, const sal_Int32 nOffset)
    {
        const SwNode* const pOldNode = &rOldNode;
        SwPosition aNewPos(rNewPos);
        aNewPos.AdjustContent(nOffset);
        bool isSortingNeeded = false;

        for (auto ppMark = m_vAllMarks.begin();
            ppMark != m_vAllMarks.end();
            ++ppMark)
        {
            // is on position ??
            bool bChangedPos = false, bChangedOPos = false;
            ::sw::mark::MarkBase* const pMark = *ppMark;
            // correction of non-existent non-MarkBase instances cannot be done
            assert(pMark);
            if(&pMark->GetMarkPos().GetNode() == pOldNode)
            {
                SwPosition aNewPosRel(aNewPos);
                if (dynamic_cast< ::sw::mark::CrossRefBookmark *>(pMark))
                {
                    // ensure that cross ref bookmark always starts at 0
                    aNewPosRel.SetContent(0); // HACK for WW8 import
                    isSortingNeeded = true; // and sort them to be safe...
                }
                aNewPosRel.AdjustContent(pMark->GetMarkPos().GetContentIndex());
                pMark->SetMarkPos(aNewPosRel);
                bChangedPos = true;
            }
            if(pMark->IsExpanded() &&
                &pMark->GetOtherMarkPos().GetNode() == pOldNode)
            {
                SwPosition aNewPosRel(aNewPos);
                aNewPosRel.AdjustContent(pMark->GetOtherMarkPos().GetContentIndex());
                pMark->SetOtherMarkPos(aNewPosRel);
                bChangedOPos = true;
            }
            // illegal selection? collapse the mark and restore sorting later
            isSortingNeeded |= lcl_FixCorrectedMark(bChangedPos, bChangedOPos, pMark);
        }

        // restore sorting if needed
        if(isSortingNeeded)
            sortMarks();

        SAL_INFO("sw.core", "correctMarksRelative");
        lcl_DebugMarks(m_vAllMarks);
    }

    static bool isDeleteMark(
            ::sw::mark::MarkBase const*const pMark,
            bool const isReplace,
            SwNode const& rStt,
            SwNode const& rEnd,
            std::optional<sal_Int32> oStartContentIdx,
            std::optional<sal_Int32> oEndContentIdx,
            bool & rbIsPosInRange,
            bool & rbIsOtherPosInRange)
    {
        assert(pMark);
        // navigator marks should not be moved
        // TODO: Check if this might make them invalid
        if (IsNavigatorReminder(*pMark))
        {
            return false;
        }

        // on position ??
        rbIsPosInRange = lcl_GreaterThan(pMark->GetMarkPos(), rStt, oStartContentIdx)
                            && lcl_Lower(pMark->GetMarkPos(), rEnd, oEndContentIdx);
        rbIsOtherPosInRange = pMark->IsExpanded()
                            && lcl_GreaterThan(pMark->GetOtherMarkPos(), rStt, oStartContentIdx)
                            && lcl_Lower(pMark->GetOtherMarkPos(), rEnd, oEndContentIdx);
        // special case: completely in range, touching the end?
        if ( oEndContentIdx.has_value()
             && !(isReplace && IDocumentMarkAccess::GetType(*pMark)
                                    == IDocumentMarkAccess::MarkType::BOOKMARK)
             && ( ( rbIsOtherPosInRange
                    && pMark->GetMarkPos().GetNode() == rEnd
                    && pMark->GetMarkPos().GetContentIndex() == *oEndContentIdx )
                  || ( rbIsPosInRange
                       && pMark->IsExpanded()
                       && pMark->GetOtherMarkPos().GetNode() == rEnd
                       && pMark->GetOtherMarkPos().GetContentIndex() == *oEndContentIdx ) ) )
        {
            rbIsPosInRange = true;
            rbIsOtherPosInRange = true;
        }

        if (rbIsPosInRange
             && (rbIsOtherPosInRange
                  || !pMark->IsExpanded()))
        {
            // completely in range

            bool bDeleteMark = true;
            {
                switch ( IDocumentMarkAccess::GetType( *pMark ) )
                {
                case IDocumentMarkAccess::MarkType::CROSSREF_HEADING_BOOKMARK:
                case IDocumentMarkAccess::MarkType::CROSSREF_NUMITEM_BOOKMARK:
                    // no delete of cross-reference bookmarks, if range is inside one paragraph
                    bDeleteMark = &rStt != &rEnd;
                    break;
                case IDocumentMarkAccess::MarkType::UNO_BOOKMARK:
                    // no delete of UNO mark, if it is not expanded and only touches the start of the range
                    bDeleteMark = rbIsOtherPosInRange
                                  || pMark->IsExpanded()
                                  || !oStartContentIdx.has_value()
                                  || pMark->GetMarkPos().GetNode() != rStt
                                  || pMark->GetMarkPos().GetContentIndex() != *oStartContentIdx;
                    break;
                default:
                    break;
                }
            }
            return bDeleteMark;
        }
        return false;
    }

    bool MarkManager::isBookmarkDeleted(SwPaM const& rPaM, bool const isReplace) const
    {
        SwPosition const& rStart(*rPaM.Start());
        SwPosition const& rEnd(*rPaM.End());
        for (auto ppMark = m_vBookmarks.begin();
            ppMark != m_vBookmarks.end();
            ++ppMark)
        {
            bool bIsPosInRange(false);
            bool bIsOtherPosInRange(false);
            bool const bDeleteMark = isDeleteMark(*ppMark, isReplace,
                rStart.GetNode(), rEnd.GetNode(), rStart.GetContentIndex(), rEnd.GetContentIndex(),
                bIsPosInRange, bIsOtherPosInRange);
            if (bDeleteMark
                && IDocumentMarkAccess::GetType(**ppMark) == MarkType::BOOKMARK)
            {
                return true;
            }
        }
        return false;
    }

    void MarkManager::deleteMarks(
            const SwNode& rStt,
            const SwNode& rEnd,
            std::vector<SaveBookmark>* pSaveBkmk,
            std::optional<sal_Int32> oStartContentIdx,
            std::optional<sal_Int32> oEndContentIdx,
            bool const isReplace)
    {
        std::vector<const_iterator> vMarksToDelete;
        bool bIsSortingNeeded = false;

        // boolean indicating, if at least one mark has been moved while collecting marks for deletion
        bool bMarksMoved = false;
        // have marks in the range been skipped instead of deleted
        bool bMarksSkipDeletion = false;

        // copy all bookmarks in the move area to a vector storing all position data as offset
        // reassignment is performed after the move
        for (auto ppMark = m_vAllMarks.begin();
            ppMark != m_vAllMarks.end();
            ++ppMark)
        {
            ::sw::mark::MarkBase *const pMark = *ppMark;
            bool bIsPosInRange(false);
            bool bIsOtherPosInRange(false);
            bool const bDeleteMark = isDeleteMark(pMark, isReplace, rStt, rEnd,
                oStartContentIdx, oEndContentIdx, bIsPosInRange, bIsOtherPosInRange);

            if ( bIsPosInRange
                 && ( bIsOtherPosInRange
                      || !pMark->IsExpanded() ) )
            {
                if ( bDeleteMark )
                {
                    if ( pSaveBkmk )
                    {
                        pSaveBkmk->push_back( SaveBookmark( *pMark, rStt, oStartContentIdx ) );
                    }
                    vMarksToDelete.emplace_back(ppMark);
                }
                else
                {
                    bMarksSkipDeletion = true;
                }
            }
            else if ( bIsPosInRange != bIsOtherPosInRange )
            {
                // the bookmark is partially in the range
                // move position of that is in the range out of it

                std::optional< SwPosition > oNewPos;
                if ( oEndContentIdx )
                {
                    oNewPos.emplace( *rEnd.GetContentNode(), *oEndContentIdx );
                }
                else
                {
                    lcl_FindExpelPosition( oNewPos, rStt, rEnd, bIsPosInRange ? pMark->GetOtherMarkPos() : pMark->GetMarkPos() );
                }

                bool bMoveMark = true;
                {
                    switch ( IDocumentMarkAccess::GetType( *pMark ) )
                    {
                    case IDocumentMarkAccess::MarkType::CROSSREF_HEADING_BOOKMARK:
                    case IDocumentMarkAccess::MarkType::CROSSREF_NUMITEM_BOOKMARK:
                        // no move of cross-reference bookmarks, if move occurs inside a certain node
                        bMoveMark = pMark->GetMarkPos().GetNode() != oNewPos->GetNode();
                        break;
                    case IDocumentMarkAccess::MarkType::ANNOTATIONMARK:
                        // no move of annotation marks, if method is called to collect deleted marks
                        bMoveMark = pSaveBkmk == nullptr;
                        break;
                    default:
                        bMoveMark = true;
                        break;
                    }
                }
                if ( bMoveMark )
                {
                    if ( bIsPosInRange )
                        pMark->SetMarkPos(*oNewPos);
                    else
                        pMark->SetOtherMarkPos(*oNewPos);
                    bMarksMoved = true;

                    // illegal selection? collapse the mark and restore sorting later
                    bIsSortingNeeded |= lcl_FixCorrectedMark( bIsPosInRange, bIsOtherPosInRange, pMark );
                }
            }
        }

        {
            // fdo#61016 delay the deletion of the fieldmark characters
            // to prevent that from deleting the marks on that position
            // which would invalidate the iterators in vMarksToDelete
            std::vector< std::unique_ptr<ILazyDeleter> > vDelay;
            vDelay.reserve(vMarksToDelete.size());

            // If needed, sort mark containers containing subsets of the marks
            // in order to assure sorting.  The sorting is critical for the
            // deletion of a mark as it is searched in these container for
            // deletion.
            if ( !vMarksToDelete.empty() && bMarksMoved )
            {
                sortSubsetMarks();
            }
            // we just remembered the iterators to delete, so we do not need to search
            // for the shared_ptr<> (the entry in m_vAllMarks) again
            // reverse iteration, since erasing an entry invalidates iterators
            // behind it (the iterators in vMarksToDelete are sorted)
            for ( std::vector< const_iterator >::reverse_iterator pppMark = vMarksToDelete.rbegin();
                  pppMark != vMarksToDelete.rend();
                  ++pppMark )
            {
                vDelay.push_back(deleteMark(*pppMark, pSaveBkmk != nullptr));
            }
        } // scope to kill vDelay

        // also need to sort if both marks were moved and not-deleted because
        // the not-deleted marks could be in wrong order vs. the moved ones
        if (bIsSortingNeeded || (bMarksMoved && bMarksSkipDeletion))
        {
            sortMarks();
        }

        SAL_INFO("sw.core", "deleteMarks");
        lcl_DebugMarks(m_vAllMarks);
    }

    namespace {

    struct LazyFieldmarkDeleter : public IDocumentMarkAccess::ILazyDeleter
    {
        std::unique_ptr<Fieldmark> m_pFieldmark;
        SwDoc& m_rDoc;
        bool const m_isMoveNodes;
        LazyFieldmarkDeleter(Fieldmark *const pMark, SwDoc& rDoc, bool const isMoveNodes)
            : m_pFieldmark(pMark), m_rDoc(rDoc), m_isMoveNodes(isMoveNodes)
        {
            assert(m_pFieldmark);
        }
        virtual ~LazyFieldmarkDeleter() override
        {
            // note: because of the call chain from SwUndoDelete, the field
            // command *cannot* be deleted here as it would create a separate
            // SwUndoDelete that's interleaved with the SwHistory of the outer
            // one - only delete the CH_TXT_ATR_FIELD*!
            if (!m_isMoveNodes)
            {
                m_pFieldmark->ReleaseDoc(m_rDoc);
            }
        }
    };

    // Call DeregisterFromDoc() lazily, because it can call selection change listeners, which
    // may mutate the marks container
    struct LazyDdeBookmarkDeleter : public IDocumentMarkAccess::ILazyDeleter
    {
        std::unique_ptr<DdeBookmark> m_pDdeBookmark;
        SwDoc& m_rDoc;
        LazyDdeBookmarkDeleter(DdeBookmark *const pDdeBookmark, SwDoc& rDoc)
            : m_pDdeBookmark(pDdeBookmark), m_rDoc(rDoc)
        {
            assert(pDdeBookmark);
        }
        virtual ~LazyDdeBookmarkDeleter() override
        {
            m_pDdeBookmark->DeregisterFromDoc(m_rDoc);
        }
    };

    }

    std::unique_ptr<IDocumentMarkAccess::ILazyDeleter>
        MarkManager::deleteMark(const const_iterator& ppMark, bool const isMoveNodes)
    {
        std::unique_ptr<ILazyDeleter> ret;
        if (ppMark == m_vAllMarks.end())
            return ret;
        MarkBase* pMark = *ppMark;

        switch(IDocumentMarkAccess::GetType(*pMark))
        {
            case IDocumentMarkAccess::MarkType::BOOKMARK:
                {
                    auto const ppBookmark = lcl_FindMark(m_vBookmarks, static_cast<sw::mark::Bookmark*>(pMark));
                    if ( ppBookmark != m_vBookmarks.end() )
                    {
                        Bookmark* pBookmark = *ppBookmark;

                        if(pBookmark)
                            pBookmark->sendLOKDeleteCallback();

                        m_vBookmarks.erase(ppBookmark);
                    }
                    else
                    {
                        assert(false &&
                            "<MarkManager::deleteMark(..)> - Bookmark not found in Bookmark container.");
                    }
                }
                break;
            case IDocumentMarkAccess::MarkType::CROSSREF_HEADING_BOOKMARK:
            case IDocumentMarkAccess::MarkType::CROSSREF_NUMITEM_BOOKMARK:
                {
                    auto const ppBookmark = lcl_FindMark(m_vBookmarks, static_cast<Bookmark*>(pMark));
                    if ( ppBookmark != m_vBookmarks.end() )
                    {
                        m_vBookmarks.erase(ppBookmark);
                    }
                    else
                    {
                        assert(false &&
                            "<MarkManager::deleteMark(..)> - Bookmark not found in Bookmark container.");
                    }
                }
                break;

            case IDocumentMarkAccess::MarkType::TEXT_FIELDMARK:
            case IDocumentMarkAccess::MarkType::CHECKBOX_FIELDMARK:
            case IDocumentMarkAccess::MarkType::DROPDOWN_FIELDMARK:
            case IDocumentMarkAccess::MarkType::DATE_FIELDMARK:
                {
                    auto const ppFieldmark = lcl_FindMark(m_vFieldmarks, static_cast<Fieldmark*>(pMark));
                    if ( ppFieldmark != m_vFieldmarks.end() )
                    {
                        if(m_pLastActiveFieldmark == *ppFieldmark)
                            ClearFieldActivation();

                        m_vFieldmarks.erase(ppFieldmark);
                        ret.reset(new LazyFieldmarkDeleter(static_cast<Fieldmark*>(pMark), m_rDoc, isMoveNodes));
                        pMark = nullptr;
                    }
                    else
                    {
                        assert(false &&
                            "<MarkManager::deleteMark(..)> - Fieldmark not found in Fieldmark container.");
                    }
                }
                break;

            case IDocumentMarkAccess::MarkType::ANNOTATIONMARK:
                {
                    auto const ppAnnotationMark = lcl_FindMark(m_vAnnotationMarks, static_cast<AnnotationMark*>(pMark));
                    assert(ppAnnotationMark != m_vAnnotationMarks.end() &&
                        "<MarkManager::deleteMark(..)> - Annotation Mark not found in Annotation Mark container.");
                    m_vAnnotationMarks.erase(ppAnnotationMark);
                }
                break;

            case IDocumentMarkAccess::MarkType::DDE_BOOKMARK:
            case IDocumentMarkAccess::MarkType::NAVIGATOR_REMINDER:
            case IDocumentMarkAccess::MarkType::UNO_BOOKMARK:
                // no special marks container
                break;
        }
        //Effective STL Item 27, get a non-const iterator aI at the same
        //position as const iterator ppMark was
        auto aI = m_vAllMarks.begin();
        std::advance(aI, std::distance<container_t::const_iterator>(aI, ppMark));
        DdeBookmark* const pDdeBookmark = dynamic_cast<DdeBookmark*>(pMark);
        if (pDdeBookmark)
        {
            ret.reset(new LazyDdeBookmarkDeleter(pDdeBookmark, m_rDoc));
            pMark = nullptr;
        }

        m_vAllMarks.erase(aI);
        // delete after we remove from the list, because the destructor can
        // recursively call into this method.
        delete pMark; // If we have a lazy deleter, pMark was null-ed

        return ret;
    }

    void MarkManager::deleteMark(const MarkBase* const pMark)
    {
        assert(&pMark->GetMarkPos().GetDoc() == &m_rDoc &&
            "<MarkManager::deleteMark(..)>"
            " - Mark is not in my doc.");
        // finds the last Mark that is starting before pMark
        // (pMarkLow < pMark)
        auto [it, endIt] = equal_range(
                m_vAllMarks.begin(),
                m_vAllMarks.end(),
                pMark->GetMarkStart(),
                CompareIMarkStartsBefore<MarkBase>());
        for ( ; it != endIt; ++it)
            if (*it == pMark)
            {
                deleteMark(it, false);
                break;
            }
    }

    void MarkManager::clearAllMarks()
    {
        ClearFieldActivation();
        m_vFieldmarks.clear();
        m_vBookmarks.clear();
        m_vAnnotationMarks.clear();
        for (const auto & p : m_vAllMarks)
            delete p;
        m_vAllMarks.clear();
    }

    IDocumentMarkAccess::const_iterator MarkManager::findMark(const OUString& rName) const
    {
        return lcl_FindMarkByName(rName, m_vAllMarks);
    }

    std::vector<sw::mark::Bookmark*>::const_iterator MarkManager::findBookmark(const OUString& rName) const
    {
        return lcl_FindMarkByName(rName, m_vBookmarks);
    }

    std::vector<sw::mark::AnnotationMark*>::const_iterator MarkManager::findAnnotationMark( const OUString& rName ) const
    {
        return lcl_FindMarkByName(rName, m_vAnnotationMarks);
    }

    // find the first Bookmark that does not start before
    std::vector<sw::mark::Bookmark*>::const_iterator MarkManager::findFirstBookmarkNotStartsBefore(const SwPosition& rPos) const
    {
        return std::lower_bound(
                m_vBookmarks.begin(),
                m_vBookmarks.end(),
                rPos,
                CompareIMarkStartsBefore<Bookmark>());
    }

    IDocumentMarkAccess::const_iterator MarkManager::getAllMarksBegin() const
        { return m_vAllMarks.begin(); }

    IDocumentMarkAccess::const_iterator MarkManager::getAllMarksEnd() const
        { return m_vAllMarks.end(); }

    sal_Int32 MarkManager::getAllMarksCount() const
        { return m_vAllMarks.size(); }

    std::vector<sw::mark::Bookmark*>::const_iterator MarkManager::getBookmarksBegin() const
        { return m_vBookmarks.begin(); }

    std::vector<sw::mark::Bookmark*>::const_iterator MarkManager::getBookmarksEnd() const
        { return m_vBookmarks.end(); }

    sal_Int32 MarkManager::getBookmarksCount() const
        { return m_vBookmarks.size(); }

    std::vector<Fieldmark*>::const_iterator MarkManager::getFieldmarksBegin() const
        { return m_vFieldmarks.begin(); }

    std::vector<Fieldmark*>::const_iterator MarkManager::getFieldmarksEnd() const
        { return m_vFieldmarks.end(); }

    sal_Int32 MarkManager::getFieldmarksCount() const { return m_vFieldmarks.size(); }


    // finds the first that is starting after
    std::vector<sw::mark::Bookmark*>::const_iterator MarkManager::findFirstBookmarkStartsAfter(const SwPosition& rPos) const
    {
        return std::upper_bound(
            m_vBookmarks.begin(),
            m_vBookmarks.end(),
            rPos,
            CompareIMarkStartsAfter<sw::mark::Bookmark>());
    }

    Fieldmark* MarkManager::getFieldmarkAt(const SwPosition& rPos) const
    {
        auto const pFieldmark = find_if(
            m_vFieldmarks.begin(),
            m_vFieldmarks.end(),
            [&rPos] (::sw::mark::MarkBase const*const pMark) {
                    auto [/*const SwPosition&*/ rStartPos, rEndPos] = pMark->GetMarkStartEnd();
                    return rStartPos == rPos
                            // end position includes the CH_TXT_ATR_FIELDEND
                        || (rEndPos.GetContentIndex() == rPos.GetContentIndex() + 1
                            && rEndPos.GetNode() == rPos.GetNode());
                } );
        return (pFieldmark == m_vFieldmarks.end())
            ? nullptr
            : *pFieldmark;
    }

    Fieldmark* MarkManager::getInnerFieldmarkFor(const SwPosition& rPos) const
    {
        // find the first mark starting on or before the position in reverse order
        // (as we are reverse searching, this is the one closest to the position)
        // m_vFieldmarks should be ordered by mark start, so we can bisect with lower_bound
        auto itEnd = m_vFieldmarks.rend();
        auto itStart = lower_bound(
            m_vFieldmarks.rbegin(),
            itEnd,
            rPos,
            CompareIMarkStartsAfterReverse());
        // now continue a linear search for the first (still in reverse order) ending behind the position
        auto itCurrent = find_if(
            itStart,
            itEnd,
            [&rPos](const sw::mark::MarkBase* const pMark) { return rPos < pMark->GetMarkEnd(); });
        // if we reached the end (in reverse order) there is no match
        if(itCurrent == itEnd)
            return nullptr;
        // we found our first candidate covering the position ...
        auto pMark = *itCurrent;
        auto aMarkStartEndPair = pMark->GetMarkStartEnd();
        const SwPosition* pMarkStart = &aMarkStartEndPair.first;
        const SwPosition* pMarkEnd = &aMarkStartEndPair.second;
        // ... however we still need to check if there is a smaller/'more inner' one with the same start position
        for(++itCurrent; itCurrent != itEnd; ++itCurrent)
        {
            if((*itCurrent)->GetMarkStart() < *pMarkStart)
                // any following mark (in reverse order) will have an earlier
                // start and thus can not be more 'inner' than our previous
                // match, so we are done.
                break;
            const SwPosition& rCurrentMarkEnd = (*itCurrent)->GetMarkEnd();
            if(rPos < rCurrentMarkEnd && rCurrentMarkEnd <= *pMarkEnd)
            {
                // both covering the position and more inner/smaller => use this one instead
                pMark = *itCurrent;
                pMarkEnd = &rCurrentMarkEnd;
            }
        }
        return pMark;
    }

    sw::mark::Bookmark* MarkManager::getOneInnermostBookmarkFor(const SwPosition& rPos) const
    {
        auto it = std::find_if(m_vBookmarks.begin(), m_vBookmarks.end(),
                               [&rPos](const sw::mark::Bookmark* pMark)
                               { return pMark->IsCoveringPosition(rPos); });
        if (it == m_vBookmarks.end())
        {
            return nullptr;
        }
        sw::mark::Bookmark* pBookmark = *it;

        // See if any bookmarks after the first hit are closer to rPos.
        ++it;

        for (; it != m_vBookmarks.end(); ++it)
        {
            // Find the innermost bookmark.
            auto [/*const SwPosition&*/ rMarkStart, rMarkEnd] = (*it)->GetMarkStartEnd();
            if (rMarkStart > rPos)
                break;
            if (rPos < rMarkEnd
                && (pBookmark->GetMarkStart() < rMarkStart
                    || rMarkEnd < pBookmark->GetMarkEnd()))
            {
                pBookmark = *it;
            }
        }
        return pBookmark;
    }

    void MarkManager::deleteFieldmarkAt(const SwPosition& rPos)
    {
        Fieldmark* const pFieldmark = getFieldmarkAt(rPos);
        assert(pFieldmark); // currently all callers require it to be there

        deleteMark(lcl_FindMark<MarkBase>(m_vAllMarks, pFieldmark), false);
    }

    ::sw::mark::Fieldmark* MarkManager::changeFormFieldmarkType(::sw::mark::Fieldmark* pFieldmark, const OUString& rNewType)
    {
        bool bActualChange = false;
        if(rNewType == ODF_FORMDROPDOWN)
        {
            if (!dynamic_cast<::sw::mark::DropDownFieldmark*>(pFieldmark))
                bActualChange = true;
            if (!dynamic_cast<::sw::mark::CheckboxFieldmark*>(pFieldmark)) // only allowed converting between checkbox <-> dropdown
                return nullptr;
        }
        else if(rNewType == ODF_FORMCHECKBOX)
        {
            if (!dynamic_cast<::sw::mark::CheckboxFieldmark*>(pFieldmark))
                bActualChange = true;
            if (!dynamic_cast<::sw::mark::DropDownFieldmark*>(pFieldmark)) // only allowed converting between checkbox <-> dropdown
                return nullptr;
        }
        else if(rNewType == ODF_FORMDATE)
        {
            if (!dynamic_cast<::sw::mark::DateFieldmark*>(pFieldmark))
                bActualChange = true;
            if (!dynamic_cast<::sw::mark::TextFieldmark*>(pFieldmark)) // only allowed converting between date field <-> text field
                return nullptr;
        }

        if (!bActualChange)
            return nullptr;

        // Store attributes needed to create the new fieldmark
        OUString sName = pFieldmark->GetName();
        SwPaM const aPaM(pFieldmark->GetMarkStart());

        // Remove the old fieldmark and create a new one with the new type
        if (rNewType == ODF_FORMDROPDOWN || rNewType == ODF_FORMCHECKBOX)
        {
            SwPosition aNewPos (*aPaM.GetPoint());
            deleteFieldmarkAt(aNewPos);
            return makeNoTextFieldBookmark(aPaM, sName, rNewType);
        }
        else if(rNewType == ODF_FORMDATE)
        {
            SwPosition aPos (*aPaM.GetPoint());
            SwPaM aNewPaM(pFieldmark->GetMarkStart(), pFieldmark->GetMarkEnd());
            deleteFieldmarkAt(aPos);
            // HACK: hard-code the separator position here at the start because
            // writerfilter put it in the wrong place (at the end) on attach()
            SwPosition const sepPos(*aNewPaM.Start());
            return makeFieldBookmark(aNewPaM, sName, rNewType, &sepPos);
        }
        return nullptr;
    }

    void MarkManager::NotifyCursorUpdate(const SwCursorShell& rCursorShell)
    {
        SwView* pSwView = dynamic_cast<SwView *>(rCursorShell.GetSfxViewShell());
        if(!pSwView)
            return;

        SwEditWin& rEditWin = pSwView->GetEditWin();
        SwPosition aPos(*rCursorShell.GetCursor()->GetPoint());
        Fieldmark* pFieldBM = getInnerFieldmarkFor(aPos);
        FieldmarkWithDropDownButton* pNewActiveFieldmark = nullptr;
        if ((!pFieldBM || (pFieldBM->GetFieldname() != ODF_FORMDROPDOWN && pFieldBM->GetFieldname() != ODF_FORMDATE))
            && aPos.GetContentIndex() > 0 )
        {
            aPos.AdjustContent(-1);
            pFieldBM = getInnerFieldmarkFor(aPos);
        }

        if ( pFieldBM && (pFieldBM->GetFieldname() == ODF_FORMDROPDOWN ||
                          pFieldBM->GetFieldname() == ODF_FORMDATE))
        {
            if (m_pLastActiveFieldmark != pFieldBM)
            {
                FieldmarkWithDropDownButton& rFormField = dynamic_cast<FieldmarkWithDropDownButton&>(*pFieldBM);
                pNewActiveFieldmark = &rFormField;
            }
            else
            {
                pNewActiveFieldmark = m_pLastActiveFieldmark;
            }
        }

        if(pNewActiveFieldmark != m_pLastActiveFieldmark)
        {
            ClearFieldActivation();
            m_pLastActiveFieldmark = pNewActiveFieldmark;
            if(pNewActiveFieldmark)
                pNewActiveFieldmark->ShowButton(&rEditWin);
        }

        LOKUpdateActiveField(pSwView);
    }

    void MarkManager::ClearFieldActivation()
    {
        if(m_pLastActiveFieldmark)
            m_pLastActiveFieldmark->RemoveButton();

        m_pLastActiveFieldmark = nullptr;
    }

    void MarkManager::LOKUpdateActiveField(const SfxViewShell* pViewShell)
    {
        if (!comphelper::LibreOfficeKit::isActive())
            return;

        if (m_pLastActiveFieldmark)
        {
            if (auto pDrowDown = m_pLastActiveFieldmark->GetFieldname() == ODF_FORMDROPDOWN ?
                                dynamic_cast<::sw::mark::DropDownFieldmark*>(m_pLastActiveFieldmark) :
                                nullptr)
            {
                pDrowDown->SendLOKShowMessage(pViewShell);
            }
        }
        else
        {
            // Check whether we have any drop down fieldmark at all.
            bool bDropDownFieldExist = false;
            for (auto aIter = m_vFieldmarks.begin(); aIter != m_vFieldmarks.end(); ++aIter)
            {
                Fieldmark *pMark = *aIter;
                if (pMark && pMark->GetFieldname() == ODF_FORMDROPDOWN)
                {
                    bDropDownFieldExist = true;
                    break;
                }
            }

            if (bDropDownFieldExist)
                ::sw::mark::DropDownFieldmark::SendLOKHideMessage(pViewShell);
        }
    }

    Fieldmark* MarkManager::getDropDownFor(const SwPosition& rPos) const
    {
        Fieldmark *pMark = getFieldmarkAt(rPos);
        if (!pMark || pMark->GetFieldname() != ODF_FORMDROPDOWN)
            return nullptr;
        return pMark;
    }

    std::vector<Fieldmark*> MarkManager::getNoTextFieldmarksIn(const SwPaM &rPaM) const
    {
        std::vector<Fieldmark*> aRet;

        for (auto aI = m_vFieldmarks.begin(),
            aEnd = m_vFieldmarks.end(); aI != aEnd; ++aI)
        {
            ::sw::mark::MarkBase* pI = *aI;
            const SwPosition &rStart = pI->GetMarkPos();
            if (!rPaM.ContainsPosition(rStart))
                continue;

            Fieldmark *pMark = dynamic_cast<Fieldmark*>(pI);
            if (!pMark || (pMark->GetFieldname() != ODF_FORMDROPDOWN
                            && pMark->GetFieldname() != ODF_FORMCHECKBOX))
            {
                continue;
            }

            aRet.push_back(pMark);
        }

        return aRet;
    }

    Fieldmark* MarkManager::getFieldmarkAfter(const SwPosition& rPos, bool bLoop) const
        { return lcl_getMarkAfter(m_vFieldmarks, rPos, bLoop); }

    Fieldmark* MarkManager::getFieldmarkBefore(const SwPosition& rPos, bool bLoop) const
        { return lcl_getMarkBefore(m_vFieldmarks, rPos, bLoop); }

    std::vector<sw::mark::AnnotationMark*>::const_iterator MarkManager::getAnnotationMarksBegin() const
    {
        return m_vAnnotationMarks.begin();
    }

    std::vector<sw::mark::AnnotationMark*>::const_iterator MarkManager::getAnnotationMarksEnd() const
    {
        return m_vAnnotationMarks.end();
    }

    sal_Int32 MarkManager::getAnnotationMarksCount() const
    {
        return m_vAnnotationMarks.size();
    }

    AnnotationMark* MarkManager::getAnnotationMarkFor(const SwPosition& rPos) const
    {
        auto const pAnnotationMark = find_if(
            m_vAnnotationMarks.begin(),
            m_vAnnotationMarks.end(),
            [&rPos] (const ::sw::mark::AnnotationMark *const pMark) { return pMark->IsCoveringPosition(rPos); } );
        if (pAnnotationMark == m_vAnnotationMarks.end())
            return nullptr;
        return *pAnnotationMark;
    }

    // create helper bookmark for annotations on tracked deletions
    ::sw::mark::Bookmark* MarkManager::makeAnnotationBookmark(const SwPaM& rPaM,
        const OUString& rName,
        sw::mark::InsertMode const eMode,
        SwPosition const*const pSepPos)
    {
        OUString sAnnotationBookmarkName(rName + S_ANNOTATION_BOOKMARK);
        return static_cast<sw::mark::Bookmark*>(makeMark( rPaM, sAnnotationBookmarkName, MarkType::BOOKMARK, eMode, pSepPos));
    }

    // find the first AnnotationMark that does not start before
    std::vector<sw::mark::AnnotationMark*>::const_iterator MarkManager::findFirstAnnotationMarkNotStartsBefore(const SwPosition& rPos) const
    {
        return std::lower_bound(
                m_vAnnotationMarks.begin(),
                m_vAnnotationMarks.end(),
                rPos,
                CompareIMarkStartsBefore<AnnotationMark>());
    }

    // find the first AnnotationMark that does not start before
    std::vector<sw::mark::AnnotationMark*>::const_iterator MarkManager::findFirstAnnotationMarkNotStartsBefore(const SwNode& rPos) const
    {
        return std::lower_bound(
                m_vAnnotationMarks.begin(),
                m_vAnnotationMarks.end(),
                rPos,
                CompareIMarkStartsBefore<AnnotationMark>());
    }

    // find helper bookmark of annotations on tracked deletions
    std::vector<sw::mark::Bookmark*>::const_iterator MarkManager::findAnnotationBookmark(const OUString& rName) const
    {
        OUString sAnnotationBookmarkName(rName + S_ANNOTATION_BOOKMARK);
        return findBookmark(sAnnotationBookmarkName);
    }

    // restore text ranges of annotations on tracked deletions
    // based on the helper bookmarks (which can survive I/O and hiding redlines)
    void MarkManager::restoreAnnotationMarks(bool bDelete)
    {
        for (auto iter = getBookmarksBegin();
              iter != getBookmarksEnd(); )
        {
            const OUString & rBookmarkName = (**iter).GetName();
            sal_Int32 nPos;
            if ( rBookmarkName.startsWith("__Annotation__") &&
                  (nPos = rBookmarkName.indexOf(S_ANNOTATION_BOOKMARK)) > -1 )
            {
                ::sw::UndoGuard const undoGuard(m_rDoc.GetIDocumentUndoRedo());
                auto pMark = findAnnotationMark(rBookmarkName.copy(0, nPos));
                if ( pMark != getAnnotationMarksEnd() )
                {
                    const SwPaM aPam((**iter).GetMarkStart(), (**pMark).GetMarkEnd());
                    repositionMark(*pMark, aPam);
                }
                if (bDelete)
                {
                    deleteMark(&**iter);
                    // this invalidates iter, have to start over...
                    iter = getBookmarksBegin();
                }
                else
                    ++iter;
            }
            else
                ++iter;
        }
    }

    OUString MarkManager::getUniqueMarkName(const OUString& rName) const
    {
        OSL_ENSURE(rName.getLength(),
            "<MarkManager::getUniqueMarkName(..)> - a name should be proposed");
        if( m_rDoc.IsInMailMerge())
        {
            OUString newName = rName + "MailMergeMark"
                    + DateTimeToOUString( DateTime( DateTime::SYSTEM ) )
                    + OUString::number( m_vAllMarks.size() + 1 );
            return newName;
        }

        if (lcl_FindMarkByName(rName, m_vAllMarks) == m_vAllMarks.end())
        {
            return rName;
        }
        OUString sTmp;

        // try the name "<rName>XXX" (where XXX is a number starting from 1) unless there is
        // an unused name. Due to performance-reasons (especially in mailmerge-scenarios) there
        // is a map m_aMarkBasenameMapUniqueOffset which holds the next possible offset (XXX) for
        // rName (so there is no need to test for nCnt-values smaller than the offset).
        sal_Int32 nCnt = 1;
        MarkBasenameMapUniqueOffset_t::const_iterator aIter = m_aMarkBasenameMapUniqueOffset.find(rName);
        if(aIter != m_aMarkBasenameMapUniqueOffset.end()) nCnt = aIter->second;
        OUString aPrefix = SwResId(STR_MARK_COPY).replaceFirst("%1", rName);
        while(nCnt < SAL_MAX_INT32)
        {
            sTmp = aPrefix + OUString::number(nCnt);
            nCnt++;
            if (lcl_FindMarkByName(sTmp, m_vAllMarks) == m_vAllMarks.end())
            {
                break;
            }
        }
        m_aMarkBasenameMapUniqueOffset[rName] = nCnt;

        return sTmp;
    }

    void MarkManager::assureSortedMarkContainers() const
    {
        const_cast< MarkManager* >(this)->sortMarks();
    }

    void MarkManager::sortMarks()
    {
        sort(m_vAllMarks.begin(), m_vAllMarks.end(), &lcl_MarkOrderingByStart<MarkBase>);
        sortSubsetMarks();
    }

    void MarkManager::sortSubsetMarks()
    {
        stable_sort(m_vBookmarks.begin(), m_vBookmarks.end(), &lcl_MarkOrderingByStart<MarkBase>);
        sort(m_vFieldmarks.begin(), m_vFieldmarks.end(), &lcl_MarkOrderingByStart<Fieldmark>);
        sort(m_vAnnotationMarks.begin(), m_vAnnotationMarks.end(), &lcl_MarkOrderingByStart<MarkBase>);
    }

    template<class MarkT>
    static void lcl_assureSortedMarkContainers(typename std::vector<MarkT*>& rContainer,
                    sal_Int32 nMinIndexModified)
    {
        // We know that the range nMinIndexModified.. has been modified, now we need to extend that range
        // to find the total range of elements that need to be sorted.
        // We know that the marks have been modified in fairly limited ways, see ContentIdxStoreImpl.
        sal_Int32 nMin = nMinIndexModified;
        while (nMin != 0)
        {
            nMin--;
            if (rContainer[nMin]->GetMarkStart() < rContainer[nMinIndexModified]->GetMarkStart())
                break;
        }
        sort(rContainer.begin() + nMin, rContainer.end(), &lcl_MarkOrderingByStart<MarkT>);
    }

    template<class MarkT>
    static void lcl_assureSortedMarkSubContainers(typename std::vector<MarkT*>& rContainer,
                    MarkT* pFound)
    {
        if (pFound)
        {
            auto it = std::find(rContainer.rbegin(), rContainer.rend(), pFound);
            sal_Int32 nFirstModified = std::distance(rContainer.begin(), (it+1).base());
            lcl_assureSortedMarkContainers<MarkT>(rContainer, nFirstModified);
        }
    }

    /**
     * called when we need to sort a sub-range of the container, elements starting
     * at nMinIndexModified were modified. This is used from ContentIdxStoreImpl::RestoreBkmks,
     * where we are only modifying a small range at the end of the container.
     */
    void MarkManager::assureSortedMarkContainers(sal_Int32 nMinIndexModified) const
    {
        // check if the modified range contains elements from the other sorted containers
        Bookmark* pBookmark = nullptr;
        Fieldmark* pFieldmark = nullptr;
        AnnotationMark* pAnnotationMark = nullptr;
        for (auto it = m_vAllMarks.begin() + nMinIndexModified; it != m_vAllMarks.end(); ++it)
        {
            switch(IDocumentMarkAccess::GetType(**it))
            {
                case IDocumentMarkAccess::MarkType::BOOKMARK:
                case IDocumentMarkAccess::MarkType::CROSSREF_HEADING_BOOKMARK:
                case IDocumentMarkAccess::MarkType::CROSSREF_NUMITEM_BOOKMARK:
                    if (!pBookmark)
                        pBookmark = static_cast<Bookmark*>(*it);
                    break;
                case IDocumentMarkAccess::MarkType::TEXT_FIELDMARK:
                case IDocumentMarkAccess::MarkType::CHECKBOX_FIELDMARK:
                case IDocumentMarkAccess::MarkType::DROPDOWN_FIELDMARK:
                case IDocumentMarkAccess::MarkType::DATE_FIELDMARK:
                    if (!pFieldmark)
                        pFieldmark = static_cast<Fieldmark*>(*it);
                    break;

                case IDocumentMarkAccess::MarkType::ANNOTATIONMARK:
                    if (!pAnnotationMark)
                        pAnnotationMark = static_cast<AnnotationMark*>(*it);
                    break;

                case IDocumentMarkAccess::MarkType::DDE_BOOKMARK:
                case IDocumentMarkAccess::MarkType::NAVIGATOR_REMINDER:
                case IDocumentMarkAccess::MarkType::UNO_BOOKMARK:
                    // no special marks container
                    break;
            }
        }

        auto pThis = const_cast<MarkManager*>(this);
        lcl_assureSortedMarkContainers<MarkBase>(pThis->m_vAllMarks, nMinIndexModified);
        lcl_assureSortedMarkSubContainers<Bookmark>(pThis->m_vBookmarks, pBookmark);
        lcl_assureSortedMarkSubContainers<Fieldmark>(pThis->m_vFieldmarks, pFieldmark);
        lcl_assureSortedMarkSubContainers<AnnotationMark>(pThis->m_vAnnotationMarks, pAnnotationMark);
    }

template<class MarkT>
static void dumpContainerAsXml(xmlTextWriterPtr pWriter,
                                const std::vector<MarkT*>& rContainer,
                                const char* pName)
{
    if (!rContainer.empty())
    {
        (void)xmlTextWriterStartElement(pWriter, BAD_CAST(pName));
        for (auto it = rContainer.begin(); it != rContainer.end(); ++it)
            (*it)->dumpAsXml(pWriter);
        (void)xmlTextWriterEndElement(pWriter);
    }
}

void MarkManager::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("MarkManager"));
    dumpContainerAsXml(pWriter, m_vAllMarks, "allmarks");
    dumpContainerAsXml(pWriter, m_vBookmarks, "bookmarks");
    dumpContainerAsXml(pWriter, m_vFieldmarks, "fieldmarks");
    dumpContainerAsXml(pWriter, m_vAnnotationMarks, "annotationmarks");
    (void)xmlTextWriterEndElement(pWriter);
}

} // namespace ::sw::mark

namespace
{
    bool lcl_Greater( const SwPosition& rPos, const SwNode& rNdIdx, std::optional<sal_Int32> oContentIdx )
    {
        return rPos.GetNode() > rNdIdx ||
                ( oContentIdx && rPos.GetNode() == rNdIdx && rPos.GetContentIndex() > *oContentIdx );
    }
}

MarkManager& SwDoc::GetMarkManager()
{
    return *mpMarkManager;
}

// IDocumentMarkAccess for SwDoc
IDocumentMarkAccess* SwDoc::getIDocumentMarkAccess()
    { return static_cast< IDocumentMarkAccess* >(mpMarkManager.get()); }

const IDocumentMarkAccess* SwDoc::getIDocumentMarkAccess() const
    { return static_cast< IDocumentMarkAccess* >(mpMarkManager.get()); }

SaveBookmark::SaveBookmark(
    const MarkBase& rBkmk,
    const SwNode& rMvPos,
    std::optional<sal_Int32> oContentIdx)
    : m_aName(rBkmk.GetName())
    , m_bHidden(false)
    , m_eOrigBkmType(IDocumentMarkAccess::GetType(rBkmk))
{
    const Bookmark* const pBookmark = dynamic_cast< const Bookmark* >(&rBkmk);
    if(pBookmark)
    {
        m_aShortName = pBookmark->GetShortName();
        m_aCode = pBookmark->GetKeyCode();
        m_bHidden = pBookmark->IsHidden();
        m_aHideCondition = pBookmark->GetHideCondition();
        m_pMetadataUndo = pBookmark->CreateUndo();
    }
    m_nNode1 = rBkmk.GetMarkPos().GetNodeIndex();
    m_nContent1 = rBkmk.GetMarkPos().GetContentIndex();

    m_nNode1 -= rMvPos.GetIndex();
    if(oContentIdx && !m_nNode1)
        m_nContent1 -= *oContentIdx;

    if(rBkmk.IsExpanded())
    {
        m_nNode2 = rBkmk.GetOtherMarkPos().GetNodeIndex();
        m_nContent2 = rBkmk.GetOtherMarkPos().GetContentIndex();

        m_nNode2 -= rMvPos.GetIndex();
        if(oContentIdx && !m_nNode2)
            m_nContent2 -= *oContentIdx;
    }
    else
    {
        m_nNode2 = NODE_OFFSET_MAX;
        m_nContent2 = -1;
    }
}

void SaveBookmark::SetInDoc(
    SwDoc* pDoc,
    const SwNode& rNewPos,
    std::optional<sal_Int32> oContentIdx)
{
    SwPaM aPam(rNewPos);
    if(oContentIdx)
    {
        if (aPam.GetPoint()->GetNode().IsContentNode())
            aPam.GetPoint()->SetContent( *oContentIdx );
        else
            SAL_WARN("sw", "trying to sent content index, but point node is not a content node");
    }

    if(NODE_OFFSET_MAX != m_nNode2)
    {
        aPam.SetMark();

        aPam.GetMark()->Adjust(m_nNode2);
        if (aPam.GetMark()->GetNode().IsContentNode())
        {
            if(oContentIdx && !m_nNode2)
                aPam.GetMark()->SetContent(*oContentIdx + m_nContent2);
            else
                aPam.GetMark()->SetContent(m_nContent2);
        }
        else
            SAL_WARN("sw", "trying to sent content index, but mark node is not a content node");
    }

    aPam.GetPoint()->Adjust(m_nNode1);

    if (aPam.GetPoint()->GetNode().IsContentNode())
    {
        if(oContentIdx && !m_nNode1)
            aPam.GetPoint()->SetContent(*oContentIdx + m_nContent1);
        else
            aPam.GetPoint()->SetContent(m_nContent1);
    }

    if(aPam.HasMark()
        && !CheckNodesRange(aPam.GetPoint()->GetNode(), aPam.GetMark()->GetNode(), true))
        return;

    ::sw::mark::Bookmark* const pBookmark = dynamic_cast<::sw::mark::Bookmark*>(
        pDoc->getIDocumentMarkAccess()->makeMark(aPam, m_aName,
            m_eOrigBkmType, sw::mark::InsertMode::CopyText));
    if(!pBookmark)
        return;

    pBookmark->SetKeyCode(m_aCode);
    pBookmark->SetShortName(m_aShortName);
    pBookmark->Hide(m_bHidden);
    pBookmark->SetHideCondition(m_aHideCondition);

    if (m_pMetadataUndo)
        pBookmark->RestoreMetadata(m_pMetadataUndo);
}

// DelBookmarks

void DelBookmarks(
    SwNode& rStt,
    const SwNode& rEnd,
    std::vector<SaveBookmark> * pSaveBkmk,
    std::optional<sal_Int32> oStartContentIdx,
    std::optional<sal_Int32> oEndContentIdx,
    bool const isReplace)
{
    // illegal range ??
    if(rStt.GetIndex() > rEnd.GetIndex()
        || (&rStt == &rEnd && (!oStartContentIdx || !oEndContentIdx || *oStartContentIdx >= *oEndContentIdx)))
        return;
    SwDoc& rDoc = rStt.GetDoc();

    rDoc.getIDocumentMarkAccess()->deleteMarks(rStt, rEnd, pSaveBkmk,
        oStartContentIdx,
        oEndContentIdx,
        isReplace);

    // Copy all Redlines which are in the move area into an array
    // which holds all position information as offset.
    // Assignment happens after moving.
    SwRedlineTable& rTable = rDoc.getIDocumentRedlineAccess().GetRedlineTable();
    for(SwRangeRedline* pRedl : rTable)
    {
        // Is at position?
        auto [pRStt, pREnd] = pRedl->StartEnd();

        if( lcl_Greater( *pRStt, rStt, oStartContentIdx ) && lcl_Lower( *pRStt, rEnd, oEndContentIdx ))
        {
            pRStt->Assign( rEnd );
            if( oEndContentIdx )
                pRStt->SetContent( *oEndContentIdx );
            else
            {
                bool bStt = true;
                SwContentNode* pCNd = pRStt->GetNode().GetContentNode();
                if( !pCNd )
                    pCNd = SwNodes::GoNext(pRStt);
                if (!pCNd)
                {
                    bStt = false;
                    pRStt->Assign(rStt);
                    pCNd = SwNodes::GoPrevious( pRStt );
                    if( !pCNd )
                    {
                        *pRStt = *pREnd;
                        pCNd = pRStt->GetNode().GetContentNode();
                    }
                }
                if (pCNd && !bStt)
                    pRStt->AssignEndIndex( *pCNd );
            }
        }
        if( lcl_Greater( *pREnd, rStt, oStartContentIdx ) && lcl_Lower( *pREnd, rEnd, oEndContentIdx ))
        {
            pREnd->Assign( rStt );
            if (oStartContentIdx && rStt.IsContentNode())
                pREnd->SetContent( *oStartContentIdx );
            else
            {
                bool bStt = false;
                SwContentNode* pCNd = pREnd->GetNode().GetContentNode();
                if( !pCNd )
                    pCNd = SwNodes::GoPrevious( pREnd );
                if( !pCNd )
                {
                    bStt = true;
                    pREnd->Assign(rEnd);
                    pCNd = SwNodes::GoNext(pREnd);
                    if( !pCNd )
                    {
                        *pREnd = *pRStt;
                        pCNd = pREnd->GetNode().GetContentNode();
                    }
                }
                if (pCNd && !bStt)
                    pREnd->AssignEndIndex( *pCNd );
            }
            if( lcl_Greater( *pRStt, rEnd, oEndContentIdx ) )
                break;
        }
    }
}

namespace sw {

InsertText MakeInsertText(SwTextNode& rNode, const sal_Int32 nPos, const sal_Int32 nLen)
{
    SwCursor cursor(SwPosition(rNode, nPos), nullptr);
    bool isInsideFieldmarkCommand(false);
    bool isInsideFieldmarkResult(false);
    while (auto const*const pMark = rNode.GetDoc().getIDocumentMarkAccess()->getInnerFieldmarkFor(*cursor.GetPoint()))
    {
        if (sw::mark::FindFieldSep(*pMark) < *cursor.GetPoint())
        {
            isInsideFieldmarkResult = true;
        }
        else
        {
            isInsideFieldmarkCommand = true;
        }
        *cursor.GetPoint() = pMark->GetMarkStart();
        if (!cursor.Left(1))
        {
            break;
        }
    }
    return InsertText(nPos, nLen, isInsideFieldmarkCommand, isInsideFieldmarkResult);
}

} // namespace sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
