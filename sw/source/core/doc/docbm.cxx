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

constexpr OUStringLiteral S_ANNOTATION_BOOKMARK = u"____";

using namespace ::sw::mark;

std::vector<::sw::mark::MarkBase*>::const_iterator const&
IDocumentMarkAccess::iterator::get() const
{
    return *m_pIter;
}

IDocumentMarkAccess::iterator::iterator(std::vector<::sw::mark::MarkBase*>::const_iterator const& rIter)
    : m_pIter(rIter)
{
}

IDocumentMarkAccess::iterator::iterator(iterator const& rOther)
    : m_pIter(rOther.m_pIter)
{
}

auto IDocumentMarkAccess::iterator::operator=(iterator const& rOther) -> iterator&
{
    m_pIter = rOther.m_pIter;
    return *this;
}

IDocumentMarkAccess::iterator::iterator(iterator && rOther) noexcept
    : m_pIter(std::move(rOther.m_pIter))
{
}

auto IDocumentMarkAccess::iterator::operator=(iterator && rOther) noexcept -> iterator&
{
    m_pIter = std::move(rOther.m_pIter);
    return *this;
}

// ARGH why does it *need* to return const& ?
::sw::mark::IMark* /*const&*/
IDocumentMarkAccess::iterator::operator*() const
{
    return static_cast<sw::mark::IMark*>(**m_pIter);
}

auto IDocumentMarkAccess::iterator::operator++() -> iterator&
{
    ++(*m_pIter);
    return *this;
}
auto IDocumentMarkAccess::iterator::operator++(int) -> iterator
{
    iterator tmp(*this);
    ++(*m_pIter);
    return tmp;
}

bool IDocumentMarkAccess::iterator::operator==(iterator const& rOther) const
{
    return *m_pIter == *rOther.m_pIter;
}

bool IDocumentMarkAccess::iterator::operator!=(iterator const& rOther) const
{
    return *m_pIter != *rOther.m_pIter;
}

IDocumentMarkAccess::iterator::iterator()
    : m_pIter(std::in_place)
{
}

auto IDocumentMarkAccess::iterator::operator--() -> iterator&
{
    --(*m_pIter);
    return *this;
}

auto IDocumentMarkAccess::iterator::operator--(int) -> iterator
{
    iterator tmp(*this);
    --(*m_pIter);
    return tmp;
}

auto IDocumentMarkAccess::iterator::operator+=(difference_type const n) -> iterator&
{
    (*m_pIter) += n;
    return *this;
}

auto IDocumentMarkAccess::iterator::operator+(difference_type const n) const -> iterator
{
    return iterator(*m_pIter + n);
}

auto IDocumentMarkAccess::iterator::operator-=(difference_type const n) -> iterator&
{
    (*m_pIter) -= n;
    return *this;
}

auto IDocumentMarkAccess::iterator::operator-(difference_type const n) const -> iterator
{
    return iterator(*m_pIter - n);
}

auto IDocumentMarkAccess::iterator::operator-(iterator const& rOther) const -> difference_type
{
    return *m_pIter - *rOther.m_pIter;
}

auto IDocumentMarkAccess::iterator::operator[](difference_type const n) const -> value_type
{
    return static_cast<sw::mark::IMark*>((*m_pIter)[n]);
}

bool IDocumentMarkAccess::iterator::operator<(iterator const& rOther) const
{
    return *m_pIter < *rOther.m_pIter;
}
bool IDocumentMarkAccess::iterator::operator>(iterator const& rOther) const
{
    return *m_pIter > *rOther.m_pIter;
}
bool IDocumentMarkAccess::iterator::operator<=(iterator const& rOther) const
{
    return *m_pIter <= *rOther.m_pIter;
}
bool IDocumentMarkAccess::iterator::operator>=(iterator const& rOther) const
{
    return *m_pIter >= *rOther.m_pIter;
}


namespace
{
    bool lcl_GreaterThan( const SwPosition& rPos, const SwNode& rNdIdx, std::optional<sal_Int32> oContentIdx )
    {
        return oContentIdx.has_value()
               ? ( rPos.GetNode() > rNdIdx
                   || ( rPos.GetNode() == rNdIdx
                        && rPos.nContent >= *oContentIdx ) )
               : rPos.GetNode() >= rNdIdx;
    }

    bool lcl_Lower( const SwPosition& rPos, const SwNode& rNdIdx, std::optional<sal_Int32> oContentIdx )
    {
        return rPos.nNode < rNdIdx
               || ( oContentIdx.has_value()
                    && rPos.GetNode() == rNdIdx
                    && rPos.nContent < *oContentIdx );
    }

    bool lcl_MarkOrderingByStart(const ::sw::mark::MarkBase *const pFirst,
                                 const ::sw::mark::MarkBase *const pSecond)
    {
        auto const& rFirstStart(pFirst->GetMarkStart());
        auto const& rSecondStart(pSecond->GetMarkStart());
        if (rFirstStart.nNode != rSecondStart.nNode)
        {
            return rFirstStart.nNode < rSecondStart.nNode;
        }
        const sal_Int32 nFirstContent = rFirstStart.GetContentIndex();
        const sal_Int32 nSecondContent = rSecondStart.GetContentIndex();
        if (nFirstContent != 0 || nSecondContent != 0)
        {
            return nFirstContent < nSecondContent;
        }
        auto *const pCRFirst (dynamic_cast<::sw::mark::CrossRefBookmark const*>(pFirst));
        auto *const pCRSecond(dynamic_cast<::sw::mark::CrossRefBookmark const*>(pSecond));
        if ((pCRFirst == nullptr) == (pCRSecond == nullptr))
        {
            return false; // equal
        }
        return pCRFirst != nullptr; // cross-ref sorts *before*
    }

    bool lcl_MarkOrderingByEnd(const ::sw::mark::MarkBase *const pFirst,
                               const ::sw::mark::MarkBase *const pSecond)
    {
        return pFirst->GetMarkEnd() < pSecond->GetMarkEnd();
    }

    void lcl_InsertMarkSorted(MarkManager::container_t& io_vMarks,
                              ::sw::mark::MarkBase *const pMark)
    {
        io_vMarks.insert(
            lower_bound(
                io_vMarks.begin(),
                io_vMarks.end(),
                pMark,
                &lcl_MarkOrderingByStart),
            pMark);
    }

    void lcl_PositionFromContentNode(
        std::optional<SwPosition>& rFoundPos,
        const SwContentNode * const pContentNode,
        const bool bAtEnd)
    {
        rFoundPos.emplace(*pContentNode);
        rFoundPos->nContent.Assign(pContentNode, bAtEnd ? pContentNode->Len() : 0);
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
            pNode = rEnd.GetNodes().GoNext( &aEnd );
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

    struct CompareIMarkStartsBefore
    {
        bool operator()(SwPosition const& rPos,
                        const sw::mark::IMark* pMark)
        {
            return rPos < pMark->GetMarkStart();
        }
        bool operator()(const sw::mark::IMark* pMark,
                        SwPosition const& rPos)
        {
            return pMark->GetMarkStart() < rPos;
        }
    };

    // Apple llvm-g++ 4.2.1 with _GLIBCXX_DEBUG won't eat boost::bind for this
    // Neither will MSVC 2008 with _DEBUG
    struct CompareIMarkStartsAfter
    {
        bool operator()(SwPosition const& rPos,
                        const sw::mark::IMark* pMark)
        {
            return pMark->GetMarkStart() > rPos;
        }
    };


    IMark* lcl_getMarkAfter(const MarkManager::container_t& rMarks, const SwPosition& rPos)
    {
        auto const pMarkAfter = upper_bound(
            rMarks.begin(),
            rMarks.end(),
            rPos,
            CompareIMarkStartsAfter());
        if(pMarkAfter == rMarks.end())
            return nullptr;
        return *pMarkAfter;
    };

    IMark* lcl_getMarkBefore(const MarkManager::container_t& rMarks, const SwPosition& rPos)
    {
        // candidates from which to choose the mark before
        MarkManager::container_t vCandidates;
        // no need to consider marks starting after rPos
        auto const pCandidatesEnd = upper_bound(
            rMarks.begin(),
            rMarks.end(),
            rPos,
            CompareIMarkStartsAfter());
        vCandidates.reserve(pCandidatesEnd - rMarks.begin());
        // only marks ending before are candidates
        remove_copy_if(
            rMarks.begin(),
            pCandidatesEnd,
            back_inserter(vCandidates),
            [&rPos] (const ::sw::mark::MarkBase *const pMark) { return !(pMark->GetMarkEnd() < rPos); } );
        // no candidate left => we are in front of the first mark or there are none
        if(vCandidates.empty()) return nullptr;
        // return the highest (last) candidate using mark end ordering
        return *max_element(vCandidates.begin(), vCandidates.end(), &lcl_MarkOrderingByEnd);
    }

    bool lcl_FixCorrectedMark(
        const bool bChangedPos,
        const bool bChangedOPos,
        MarkBase* io_pMark )
    {
        if ( IDocumentMarkAccess::GetType(*io_pMark) == IDocumentMarkAccess::MarkType::ANNOTATIONMARK )
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

    bool lcl_MarkEqualByStart(const ::sw::mark::MarkBase *const pFirst,
                              const ::sw::mark::MarkBase *const pSecond)
    {
        return !lcl_MarkOrderingByStart(pFirst, pSecond) &&
               !lcl_MarkOrderingByStart(pSecond, pFirst);
    }

    MarkManager::container_t::const_iterator lcl_FindMark(
        MarkManager::container_t& rMarks,
        const ::sw::mark::MarkBase *const pMarkToFind)
    {
        auto ppCurrentMark = lower_bound(
            rMarks.begin(), rMarks.end(),
            pMarkToFind, &lcl_MarkOrderingByStart);
        // since there are usually not too many marks on the same start
        // position, we are not doing a bisect search for the upper bound
        // but instead start to iterate from pMarkLow directly
        while (ppCurrentMark != rMarks.end() && lcl_MarkEqualByStart(*ppCurrentMark, pMarkToFind))
        {
            if(*ppCurrentMark == pMarkToFind)
            {
                return MarkManager::container_t::const_iterator(std::move(ppCurrentMark));
            }
            ++ppCurrentMark;
        }
        // reached a mark starting on a later start pos or the end of the
        // vector => not found
        return rMarks.end();
    };

    MarkManager::container_t::const_iterator lcl_FindMarkAtPos(
        MarkManager::container_t& rMarks,
        const SwPosition& rPos,
        const IDocumentMarkAccess::MarkType eType)
    {
        for (auto ppCurrentMark = lower_bound(
                rMarks.begin(), rMarks.end(),
                rPos,
                CompareIMarkStartsBefore());
            ppCurrentMark != rMarks.end();
            ++ppCurrentMark)
        {
            // Once we reach a mark starting after the target pos
            // we do not need to continue
            if((*ppCurrentMark)->GetMarkStart() > rPos)
                break;
            if(IDocumentMarkAccess::GetType(**ppCurrentMark) == eType)
            {
                return MarkManager::container_t::const_iterator(std::move(ppCurrentMark));
            }
        }
        // reached a mark starting on a later start pos or the end of the
        // vector => not found
        return rMarks.end();
    };

    MarkManager::container_t::const_iterator lcl_FindMarkByName(
        const OUString& rName,
        const MarkManager::container_t::const_iterator& ppMarksBegin,
        const MarkManager::container_t::const_iterator& ppMarksEnd)
    {
        return find_if(
            ppMarksBegin,
            ppMarksEnd,
            [&rName] (::sw::mark::MarkBase const*const pMark) { return pMark->GetName() == rName; } );
    }

    void lcl_DebugMarks(MarkManager::container_t const& rMarks)
    {
#if OSL_DEBUG_LEVEL > 0
        SAL_INFO("sw.core", rMarks.size() << " Marks");
        for (auto ppMark = rMarks.begin();
             ppMark != rMarks.end();
             ++ppMark)
        {
            IMark* pMark = *ppMark;
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
        assert(std::is_sorted(rMarks.begin(), rMarks.end(), lcl_MarkOrderingByStart));
    };
}

IDocumentMarkAccess::MarkType IDocumentMarkAccess::GetType(const IMark& rBkmk)
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
    return "__RefHeading__";
}

bool IDocumentMarkAccess::IsLegalPaMForCrossRefHeadingBookmark( const SwPaM& rPaM )
{
    return rPaM.Start()->GetNode().IsTextNode() &&
           rPaM.Start()->GetContentIndex() == 0 &&
           ( !rPaM.HasMark() ||
             ( rPaM.GetMark()->GetNode() == rPaM.GetPoint()->GetNode() &&
               rPaM.End()->GetContentIndex() == rPaM.End()->GetNode().GetTextNode()->Len() ) );
}

void IDocumentMarkAccess::DeleteFieldmarkCommand(::sw::mark::IFieldmark const& rMark)
{
    if (GetType(rMark) != MarkType::TEXT_FIELDMARK)
    {
        return; // TODO FORMDATE has no command?
    }
    SwPaM pam(sw::mark::FindFieldSep(rMark), rMark.GetMarkStart());
    ++pam.GetPoint()->nContent; // skip CH_TXT_ATR_FIELDSTART
    pam.GetDoc().getIDocumentContentOperations().DeleteAndJoin(pam);
}

namespace sw::mark
{
    MarkManager::MarkManager(SwDoc& rDoc)
        : m_rDoc(rDoc)
        , m_pLastActiveFieldmark(nullptr)
    { }

    ::sw::mark::IMark* MarkManager::makeMark(const SwPaM& rPaM,
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
                : (rPaM.GetPoint()->nNode != rPaM.GetMark()->nNode
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
                pMark = std::make_unique<CheckboxFieldmark>(rPaM);
                break;
            case IDocumentMarkAccess::MarkType::DROPDOWN_FIELDMARK:
                pMark = std::make_unique<DropDownFieldmark>(rPaM);
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
                lcl_InsertMarkSorted(m_vBookmarks, pMark.get());
                break;
            case IDocumentMarkAccess::MarkType::TEXT_FIELDMARK:
            case IDocumentMarkAccess::MarkType::CHECKBOX_FIELDMARK:
            case IDocumentMarkAccess::MarkType::DROPDOWN_FIELDMARK:
            case IDocumentMarkAccess::MarkType::DATE_FIELDMARK:
                lcl_InsertMarkSorted(m_vFieldmarks, pMark.get());
                break;
            case IDocumentMarkAccess::MarkType::ANNOTATIONMARK:
                lcl_InsertMarkSorted( m_vAnnotationMarks, pMark.get() );
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
            // due to SwInsText notifications everything is visible now - tell
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

    ::sw::mark::IFieldmark* MarkManager::makeFieldBookmark(
        const SwPaM& rPaM,
        const OUString& rName,
        const OUString& rType,
        SwPosition const*const pSepPos)
    {

        // Disable undo, because we handle it using SwUndoInsTextFieldmark
        bool bUndoIsEnabled = m_rDoc.GetIDocumentUndoRedo().DoesUndo();
        m_rDoc.GetIDocumentUndoRedo().DoUndo(false);

        sw::mark::IMark* pMark = nullptr;
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
        sw::mark::IFieldmark* pFieldMark = dynamic_cast<sw::mark::IFieldmark*>( pMark );
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

    ::sw::mark::IFieldmark* MarkManager::makeNoTextFieldBookmark(
        const SwPaM& rPaM,
        const OUString& rName,
        const OUString& rType)
    {
        // Disable undo, because we handle it using SwUndoInsNoTextFieldmark
        bool bUndoIsEnabled = m_rDoc.GetIDocumentUndoRedo().DoesUndo();
        m_rDoc.GetIDocumentUndoRedo().DoUndo(false);

        bool bEnableSetModified = m_rDoc.getIDocumentState().IsEnableSetModified();
        m_rDoc.getIDocumentState().SetEnableSetModified(false);

        sw::mark::IMark* pMark = nullptr;
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

        sw::mark::IFieldmark* pFieldMark = dynamic_cast<sw::mark::IFieldmark*>( pMark );
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

    ::sw::mark::IMark* MarkManager::getMarkForTextNode(
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

    sw::mark::IMark* MarkManager::makeAnnotationMark(
        const SwPaM& rPaM,
        const OUString& rName )
    {
        return makeMark(rPaM, rName, IDocumentMarkAccess::MarkType::ANNOTATIONMARK,
                sw::mark::InsertMode::New);
    }

    void MarkManager::repositionMark(
        ::sw::mark::IMark* const io_pMark,
        const SwPaM& rPaM)
    {
        assert(&io_pMark->GetMarkPos().GetDoc() == &m_rDoc &&
            "<MarkManager::repositionMark(..)>"
            " - Mark is not in my doc.");
        MarkBase* const pMarkBase = dynamic_cast< MarkBase* >(io_pMark);
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
        ::sw::mark::IMark* io_pMark,
        const OUString& rNewName )
    {
        assert(&io_pMark->GetMarkPos().GetDoc() == &m_rDoc &&
            "<MarkManager::renameMark(..)>"
            " - Mark is not in my doc.");
        if ( io_pMark->GetName() == rNewName )
            return true;
        if (lcl_FindMarkByName(rNewName, m_vAllMarks.begin(), m_vAllMarks.end()) != m_vAllMarks.end())
            return false;
        if (::sw::mark::MarkBase* pMarkBase = dynamic_cast< ::sw::mark::MarkBase* >(io_pMark))
        {
            const OUString sOldName(pMarkBase->GetName());
            pMarkBase->SetName(rNewName);

            if (dynamic_cast< ::sw::mark::Bookmark* >(io_pMark))
            {
                if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
                {
                    m_rDoc.GetIDocumentUndoRedo().AppendUndo(
                            std::make_unique<SwUndoRenameBookmark>(sOldName, rNewName, m_rDoc));
                }
                m_rDoc.getIDocumentState().SetModified();
            }
        }
        return true;
    }

    void MarkManager::correctMarksAbsolute(
        const SwNodeIndex& rOldNode,
        const SwPosition& rNewPos,
        const sal_Int32 nOffset)
    {
        const SwNode* const pOldNode = &rOldNode.GetNode();
        SwPosition aNewPos(rNewPos);
        aNewPos.nContent += nOffset;
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

    void MarkManager::correctMarksRelative(const SwNodeIndex& rOldNode, const SwPosition& rNewPos, const sal_Int32 nOffset)
    {
        const SwNode* const pOldNode = &rOldNode.GetNode();
        SwPosition aNewPos(rNewPos);
        aNewPos.nContent += nOffset;
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
                    aNewPosRel.nContent = 0; // HACK for WW8 import
                    isSortingNeeded = true; // and sort them to be safe...
                }
                aNewPosRel.nContent += pMark->GetMarkPos().GetContentIndex();
                pMark->SetMarkPos(aNewPosRel);
                bChangedPos = true;
            }
            if(pMark->IsExpanded() &&
                &pMark->GetOtherMarkPos().GetNode() == pOldNode)
            {
                SwPosition aNewPosRel(aNewPos);
                aNewPosRel.nContent += pMark->GetOtherMarkPos().GetContentIndex();
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
        if (IDocumentMarkAccess::GetType(*pMark) == IDocumentMarkAccess::MarkType::NAVIGATOR_REMINDER)
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
             && ( ( rbIsOtherPosInRange
                    && pMark->GetMarkPos().GetNode() == rEnd
                    && pMark->GetMarkPos().nContent == *oEndContentIdx )
                  || ( rbIsPosInRange
                       && pMark->IsExpanded()
                       && pMark->GetOtherMarkPos().GetNode() == rEnd
                       && pMark->GetOtherMarkPos().nContent == *oEndContentIdx ) ) )
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
                                  || pMark->GetMarkPos().nNode != rStt
                                  || pMark->GetMarkPos().nContent != *oStartContentIdx;
                    break;
                default:
                    bDeleteMark = true;
                    break;
                }
            }
            return bDeleteMark;
        }
        return false;
    }

    bool MarkManager::isBookmarkDeleted(SwPaM const& rPaM) const
    {
        SwPosition const& rStart(*rPaM.Start());
        SwPosition const& rEnd(*rPaM.End());
        for (auto ppMark = m_vBookmarks.begin();
            ppMark != m_vBookmarks.end();
            ++ppMark)
        {
            bool bIsPosInRange(false);
            bool bIsOtherPosInRange(false);
            bool const bDeleteMark = isDeleteMark(*ppMark,
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
            std::optional<sal_Int32> oEndContentIdx )
    {
        std::vector<const_iterator_t> vMarksToDelete;
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
            bool const bDeleteMark = isDeleteMark(pMark, rStt, rEnd, oStartContentIdx, oEndContentIdx, bIsPosInRange, bIsOtherPosInRange);

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
                        bMoveMark = pMark->GetMarkPos().nNode != oNewPos->nNode;
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
            for ( std::vector< const_iterator_t >::reverse_iterator pppMark = vMarksToDelete.rbegin();
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

    }

    std::unique_ptr<IDocumentMarkAccess::ILazyDeleter>
        MarkManager::deleteMark(const const_iterator_t& ppMark, bool const isMoveNodes)
    {
        std::unique_ptr<ILazyDeleter> ret;
        if (ppMark.get() == m_vAllMarks.end())
            return ret;
        IMark* pMark = *ppMark;

        switch(IDocumentMarkAccess::GetType(*pMark))
        {
            case IDocumentMarkAccess::MarkType::BOOKMARK:
            case IDocumentMarkAccess::MarkType::CROSSREF_HEADING_BOOKMARK:
            case IDocumentMarkAccess::MarkType::CROSSREF_NUMITEM_BOOKMARK:
                {
                    auto const ppBookmark = lcl_FindMark(m_vBookmarks, *ppMark.get());
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
                    auto const ppFieldmark = lcl_FindMark(m_vFieldmarks, *ppMark.get());
                    if ( ppFieldmark != m_vFieldmarks.end() )
                    {
                        if(m_pLastActiveFieldmark == *ppFieldmark)
                            ClearFieldActivation();

                        m_vFieldmarks.erase(ppFieldmark);
                        ret.reset(new LazyFieldmarkDeleter(dynamic_cast<Fieldmark*>(pMark), m_rDoc, isMoveNodes));
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
                    auto const ppAnnotationMark = lcl_FindMark(m_vAnnotationMarks, *ppMark.get());
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
        DdeBookmark* const pDdeBookmark = dynamic_cast<DdeBookmark*>(pMark);
        if (pDdeBookmark)
            pDdeBookmark->DeregisterFromDoc(m_rDoc);
        //Effective STL Item 27, get a non-const iterator aI at the same
        //position as const iterator ppMark was
        auto aI = m_vAllMarks.begin();
        std::advance(aI, std::distance<container_t::const_iterator>(aI, ppMark.get()));

        m_vAllMarks.erase(aI);
        // If we don't have a lazy deleter
        if (!ret)
            // delete after we remove from the list, because the destructor can
            // recursively call into this method.
            delete pMark;
        return ret;
    }

    void MarkManager::deleteMark(const IMark* const pMark)
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
                CompareIMarkStartsBefore());
        for ( ; it != endIt; ++it)
            if (*it == pMark)
            {
                deleteMark(iterator(it), false);
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

    IDocumentMarkAccess::const_iterator_t MarkManager::findMark(const OUString& rName) const
    {
        auto const ret = lcl_FindMarkByName(rName, m_vAllMarks.begin(), m_vAllMarks.end());
        return IDocumentMarkAccess::iterator(ret);
    }

    IDocumentMarkAccess::const_iterator_t MarkManager::findBookmark(const OUString& rName) const
    {
        auto const ret = lcl_FindMarkByName(rName, m_vBookmarks.begin(), m_vBookmarks.end());
        return IDocumentMarkAccess::iterator(ret);
    }

    IDocumentMarkAccess::const_iterator_t MarkManager::getAllMarksBegin() const
        { return m_vAllMarks.begin(); }

    IDocumentMarkAccess::const_iterator_t MarkManager::getAllMarksEnd() const
        { return m_vAllMarks.end(); }

    sal_Int32 MarkManager::getAllMarksCount() const
        { return m_vAllMarks.size(); }

    IDocumentMarkAccess::const_iterator_t MarkManager::getBookmarksBegin() const
        { return m_vBookmarks.begin(); }

    IDocumentMarkAccess::const_iterator_t MarkManager::getBookmarksEnd() const
        { return m_vBookmarks.end(); }

    sal_Int32 MarkManager::getBookmarksCount() const
        { return m_vBookmarks.size(); }

    IDocumentMarkAccess::const_iterator_t MarkManager::getFieldmarksBegin() const
        { return m_vFieldmarks.begin(); }

    IDocumentMarkAccess::const_iterator_t MarkManager::getFieldmarksEnd() const
        { return m_vFieldmarks.end(); }


    // finds the first that is starting after
    IDocumentMarkAccess::const_iterator_t MarkManager::findFirstBookmarkStartsAfter(const SwPosition& rPos) const
    {
        return std::upper_bound(
            m_vBookmarks.begin(),
            m_vBookmarks.end(),
            rPos,
            CompareIMarkStartsAfter());
    }

    IFieldmark* MarkManager::getFieldmarkAt(const SwPosition& rPos) const
    {
        auto const pFieldmark = find_if(
            m_vFieldmarks.begin(),
            m_vFieldmarks.end(),
            [&rPos] (::sw::mark::MarkBase const*const pMark) {
                    return pMark->GetMarkStart() == rPos
                            // end position includes the CH_TXT_ATR_FIELDEND
                        || (pMark->GetMarkEnd().GetContentIndex() == rPos.GetContentIndex() + 1
                            && pMark->GetMarkEnd().GetNode() == rPos.GetNode());
                } );
        return (pFieldmark == m_vFieldmarks.end())
            ? nullptr
            : dynamic_cast<IFieldmark*>(*pFieldmark);
    }

    IFieldmark* MarkManager::getFieldmarkFor(const SwPosition& rPos) const
    {
        auto itFieldmark = find_if(
            m_vFieldmarks.begin(),
            m_vFieldmarks.end(),
            [&rPos] (const ::sw::mark::MarkBase *const pMark) { return pMark->IsCoveringPosition(rPos); } );
        if (itFieldmark == m_vFieldmarks.end())
            return nullptr;
        auto pFieldmark(*itFieldmark);
        for ( ; itFieldmark != m_vFieldmarks.end()
                && (**itFieldmark).GetMarkStart() <= rPos; ++itFieldmark)
        {   // find the innermost fieldmark
            if (rPos < (**itFieldmark).GetMarkEnd()
                && (pFieldmark->GetMarkStart() < (**itFieldmark).GetMarkStart()
                    || (**itFieldmark).GetMarkEnd() < pFieldmark->GetMarkEnd()))
            {
                pFieldmark = *itFieldmark;
            }
        }
        return dynamic_cast<IFieldmark*>(pFieldmark);
    }

    void MarkManager::deleteFieldmarkAt(const SwPosition& rPos)
    {
        auto const pFieldmark = dynamic_cast<Fieldmark*>(getFieldmarkAt(rPos));
        assert(pFieldmark); // currently all callers require it to be there

        deleteMark(lcl_FindMark(m_vAllMarks, pFieldmark), false);
    }

    ::sw::mark::IFieldmark* MarkManager::changeFormFieldmarkType(::sw::mark::IFieldmark* pFieldmark, const OUString& rNewType)
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
        IFieldmark* pFieldBM = getFieldmarkFor(aPos);
        FieldmarkWithDropDownButton* pNewActiveFieldmark = nullptr;
        if ((!pFieldBM || (pFieldBM->GetFieldname() != ODF_FORMDROPDOWN && pFieldBM->GetFieldname() != ODF_FORMDATE))
            && aPos.GetContentIndex() > 0 )
        {
            --aPos.nContent;
            pFieldBM = getFieldmarkFor(aPos);
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
                IFieldmark *pMark = dynamic_cast<IFieldmark*>(*aIter);
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

    IFieldmark* MarkManager::getDropDownFor(const SwPosition& rPos) const
    {
        IFieldmark *pMark = getFieldmarkAt(rPos);
        if (!pMark || pMark->GetFieldname() != ODF_FORMDROPDOWN)
            return nullptr;
        return pMark;
    }

    std::vector<IFieldmark*> MarkManager::getNoTextFieldmarksIn(const SwPaM &rPaM) const
    {
        std::vector<IFieldmark*> aRet;

        for (auto aI = m_vFieldmarks.begin(),
            aEnd = m_vFieldmarks.end(); aI != aEnd; ++aI)
        {
            ::sw::mark::IMark* pI = *aI;
            const SwPosition &rStart = pI->GetMarkPos();
            if (!rPaM.ContainsPosition(rStart))
                continue;

            IFieldmark *pMark = dynamic_cast<IFieldmark*>(pI);
            if (!pMark || (pMark->GetFieldname() != ODF_FORMDROPDOWN
                            && pMark->GetFieldname() != ODF_FORMCHECKBOX))
            {
                continue;
            }

            aRet.push_back(pMark);
        }

        return aRet;
    }

    IFieldmark* MarkManager::getFieldmarkAfter(const SwPosition& rPos) const
        { return dynamic_cast<IFieldmark*>(lcl_getMarkAfter(m_vFieldmarks, rPos)); }

    IFieldmark* MarkManager::getFieldmarkBefore(const SwPosition& rPos) const
        { return dynamic_cast<IFieldmark*>(lcl_getMarkBefore(m_vFieldmarks, rPos)); }

    IDocumentMarkAccess::const_iterator_t MarkManager::getAnnotationMarksBegin() const
    {
        return m_vAnnotationMarks.begin();
    }

    IDocumentMarkAccess::const_iterator_t MarkManager::getAnnotationMarksEnd() const
    {
        return m_vAnnotationMarks.end();
    }

    sal_Int32 MarkManager::getAnnotationMarksCount() const
    {
        return m_vAnnotationMarks.size();
    }

    IDocumentMarkAccess::const_iterator_t MarkManager::findAnnotationMark( const OUString& rName ) const
    {
        auto const ret = lcl_FindMarkByName( rName, m_vAnnotationMarks.begin(), m_vAnnotationMarks.end() );
        return IDocumentMarkAccess::iterator(ret);
    }

    IMark* MarkManager::getAnnotationMarkFor(const SwPosition& rPos) const
    {
        auto const pAnnotationMark = find_if(
            m_vAnnotationMarks.begin(),
            m_vAnnotationMarks.end(),
            [&rPos] (const ::sw::mark::MarkBase *const pMark) { return pMark->IsCoveringPosition(rPos); } );
        if (pAnnotationMark == m_vAnnotationMarks.end())
            return nullptr;
        return *pAnnotationMark;
    }

    // finds the first that is starting after
    IDocumentMarkAccess::const_iterator_t MarkManager::findFirstAnnotationStartsAfter(const SwPosition& rPos) const
    {
        return std::upper_bound(
            m_vAnnotationMarks.begin(),
            m_vAnnotationMarks.end(),
            rPos,
            CompareIMarkStartsAfter());
    }

    // create helper bookmark for annotations on tracked deletions
    ::sw::mark::IMark* MarkManager::makeAnnotationBookmark(const SwPaM& rPaM,
        const OUString& rName,
        const IDocumentMarkAccess::MarkType eType,
        sw::mark::InsertMode const eMode,
        SwPosition const*const pSepPos)
    {
        OUString sAnnotationBookmarkName(rName + S_ANNOTATION_BOOKMARK);
        return makeMark( rPaM, sAnnotationBookmarkName, eType, eMode, pSepPos);
    }

    // find helper bookmark of annotations on tracked deletions
    IDocumentMarkAccess::const_iterator_t MarkManager::findAnnotationBookmark(const OUString& rName) const
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
                IDocumentMarkAccess::const_iterator_t pMark = findAnnotationMark(rBookmarkName.copy(0, nPos));
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
                    + OStringToOUString( DateTimeToOString( DateTime( DateTime::SYSTEM )), RTL_TEXTENCODING_ASCII_US )
                    + OUString::number( m_vAllMarks.size() + 1 );
            return newName;
        }

        if (lcl_FindMarkByName(rName, m_vAllMarks.begin(), m_vAllMarks.end()) == m_vAllMarks.end())
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
        while(nCnt < SAL_MAX_INT32)
        {
            sTmp = rName + OUString::number(nCnt);
            nCnt++;
            if (lcl_FindMarkByName(sTmp, m_vAllMarks.begin(), m_vAllMarks.end()) == m_vAllMarks.end())
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

    void MarkManager::sortSubsetMarks()
    {
        stable_sort(m_vBookmarks.begin(), m_vBookmarks.end(), &lcl_MarkOrderingByStart);
        sort(m_vFieldmarks.begin(), m_vFieldmarks.end(), &lcl_MarkOrderingByStart);
        sort(m_vAnnotationMarks.begin(), m_vAnnotationMarks.end(), &lcl_MarkOrderingByStart);
    }

    void MarkManager::sortMarks()
    {
        sort(m_vAllMarks.begin(), m_vAllMarks.end(), &lcl_MarkOrderingByStart);
        sortSubsetMarks();
    }

void MarkManager::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    struct
    {
        const char* pName;
        const container_t* pContainer;
    } aContainers[] =
    {
        // UNO marks are only part of all marks.
        {"allmarks", &m_vAllMarks},
        {"bookmarks", &m_vBookmarks},
        {"fieldmarks", &m_vFieldmarks},
        {"annotationmarks", &m_vAnnotationMarks}
    };

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("MarkManager"));
    for (const auto & rContainer : aContainers)
    {
        if (!rContainer.pContainer->empty())
        {
            (void)xmlTextWriterStartElement(pWriter, BAD_CAST(rContainer.pName));
            for (auto it = rContainer.pContainer->begin(); it != rContainer.pContainer->end(); ++it)
                (*it)->dumpAsXml(pWriter);
            (void)xmlTextWriterEndElement(pWriter);
        }
    }
    (void)xmlTextWriterEndElement(pWriter);
}

} // namespace ::sw::mark

namespace
{
    bool lcl_Greater( const SwPosition& rPos, const SwNode& rNdIdx, std::optional<sal_Int32> oContentIdx )
    {
        return rPos.GetNode() > rNdIdx || ( oContentIdx && rPos.GetNode() == rNdIdx && rPos.nContent > *oContentIdx );
    }
}

// IDocumentMarkAccess for SwDoc
IDocumentMarkAccess* SwDoc::getIDocumentMarkAccess()
    { return static_cast< IDocumentMarkAccess* >(mpMarkManager.get()); }

const IDocumentMarkAccess* SwDoc::getIDocumentMarkAccess() const
    { return static_cast< IDocumentMarkAccess* >(mpMarkManager.get()); }

SaveBookmark::SaveBookmark(
    const IMark& rBkmk,
    const SwNode& rMvPos,
    std::optional<sal_Int32> oContentIdx)
    : m_aName(rBkmk.GetName())
    , m_bHidden(false)
    , m_eOrigBkmType(IDocumentMarkAccess::GetType(rBkmk))
{
    const IBookmark* const pBookmark = dynamic_cast< const IBookmark* >(&rBkmk);
    if(pBookmark)
    {
        m_aShortName = pBookmark->GetShortName();
        m_aCode = pBookmark->GetKeyCode();
        m_bHidden = pBookmark->IsHidden();
        m_aHideCondition = pBookmark->GetHideCondition();

        ::sfx2::Metadatable const*const pMetadatable(
                dynamic_cast< ::sfx2::Metadatable const* >(pBookmark));
        if (pMetadatable)
        {
            m_pMetadataUndo = pMetadatable->CreateUndo();
        }
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
        aPam.GetPoint()->nContent = *oContentIdx;

    if(NODE_OFFSET_MAX != m_nNode2)
    {
        aPam.SetMark();

        aPam.GetMark()->nNode += m_nNode2;
        if(oContentIdx && !m_nNode2)
            aPam.GetMark()->nContent += m_nContent2;
        else
            aPam.GetMark()->nContent.Assign(aPam.GetMarkContentNode(), m_nContent2);
    }

    aPam.GetPoint()->nNode += m_nNode1;

    if(oContentIdx && !m_nNode1)
        aPam.GetPoint()->nContent += m_nContent1;
    else
        aPam.GetPoint()->nContent.Assign(aPam.GetPointContentNode(), m_nContent1);

    if(aPam.HasMark()
        && !CheckNodesRange(aPam.GetPoint()->GetNode(), aPam.GetMark()->GetNode(), true))
        return;

    ::sw::mark::IBookmark* const pBookmark = dynamic_cast<::sw::mark::IBookmark*>(
        pDoc->getIDocumentMarkAccess()->makeMark(aPam, m_aName,
            m_eOrigBkmType, sw::mark::InsertMode::CopyText));
    if(!pBookmark)
        return;

    pBookmark->SetKeyCode(m_aCode);
    pBookmark->SetShortName(m_aShortName);
    pBookmark->Hide(m_bHidden);
    pBookmark->SetHideCondition(m_aHideCondition);

    if (m_pMetadataUndo)
    {
        ::sfx2::Metadatable * const pMeta(
            dynamic_cast< ::sfx2::Metadatable* >(pBookmark));
        assert(pMeta && "metadata undo, but not metadatable?");
        if (pMeta)
        {
            pMeta->RestoreMetadata(m_pMetadataUndo);
        }
    }
}

// DelBookmarks

void DelBookmarks(
    SwNode& rStt,
    const SwNode& rEnd,
    std::vector<SaveBookmark> * pSaveBkmk,
    std::optional<sal_Int32> oStartContentIdx,
    std::optional<sal_Int32> oEndContentIdx)
{
    // illegal range ??
    if(rStt.GetIndex() > rEnd.GetIndex()
        || (&rStt == &rEnd && (!oStartContentIdx || !oEndContentIdx || *oStartContentIdx >= *oEndContentIdx)))
        return;
    SwDoc& rDoc = rStt.GetDoc();

    rDoc.getIDocumentMarkAccess()->deleteMarks(rStt, rEnd, pSaveBkmk,
        oStartContentIdx,
        oEndContentIdx);

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
            pRStt->nNode = rEnd;
            if( oEndContentIdx )
                pRStt->nContent = *oEndContentIdx;
            else
            {
                bool bStt = true;
                SwContentNode* pCNd = pRStt->GetNode().GetContentNode();
                if( !pCNd )
                    pCNd = rDoc.GetNodes().GoNext( pRStt );
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
            pREnd->nNode = rStt;
            if( oStartContentIdx )
                pREnd->nContent = *oStartContentIdx;
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
                    pCNd = rDoc.GetNodes().GoNext( pREnd );
                    if( !pCNd )
                    {
                        *pREnd = *pRStt;
                        pCNd = pREnd->GetNode().GetContentNode();
                    }
                }
                if (pCNd && !bStt)
                    pREnd->AssignEndIndex( *pCNd );
            }
        }
    }
}

namespace sw {

SwInsText MakeSwInsText(SwTextNode & rNode, sal_Int32 const nPos, sal_Int32 const nLen)
{
    SwCursor cursor(SwPosition(rNode, nPos), nullptr);
    bool isInsideFieldmarkCommand(false);
    bool isInsideFieldmarkResult(false);
    while (auto const*const pMark = rNode.GetDoc().getIDocumentMarkAccess()->getFieldmarkFor(*cursor.GetPoint()))
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
    return SwInsText(nPos, nLen, isInsideFieldmarkCommand, isInsideFieldmarkResult);
}

} // namespace sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
