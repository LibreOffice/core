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

#include <MarkManager.hxx>
#include <bookmrk.hxx>
#include <boost/bind.hpp>
#include <cntfrm.hxx>
#include <crossrefbookmark.hxx>
#include <dcontact.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <xmloff/odffields.hxx>
#include <editsh.hxx>
#include <fmtanchr.hxx>
#include <frmfmt.hxx>
#include <functional>
#include <hintids.hxx>
#include <mvsave.hxx>
#include <ndtxt.hxx>
#include <node.hxx>
#include <pam.hxx>
#include <redline.hxx>
#include <rolbck.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <sortedobjs.hxx>
#include <sfx2/linkmgr.hxx>
#include <swserv.hxx>
#include <swundo.hxx>
#include <unocrsr.hxx>
#include <viscrs.hxx>
#include <stdio.h>


using namespace ::std;
using namespace ::boost;
using namespace ::sw::mark;

namespace
{
    static bool lcl_GreaterThan( const SwPosition& rPos, const SwNodeIndex& rNdIdx, const SwIndex* pIdx )
    {
        return pIdx ? ( rPos.nNode > rNdIdx || ( rPos.nNode == rNdIdx && rPos.nContent >= pIdx->GetIndex() )) : rPos.nNode >= rNdIdx;
    }

    static bool lcl_Lower( const SwPosition& rPos, const SwNodeIndex& rNdIdx, const SwIndex* pIdx )
    {
        return rPos.nNode < rNdIdx || ( pIdx && rPos.nNode == rNdIdx && rPos.nContent < pIdx->GetIndex() );
    }

    static bool lcl_MarkOrderingByStart(const IDocumentMarkAccess::pMark_t& rpFirst,
        const IDocumentMarkAccess::pMark_t& rpSecond)
    {
        return rpFirst->GetMarkStart() < rpSecond->GetMarkStart();
    }

    static bool lcl_MarkOrderingByEnd(const IDocumentMarkAccess::pMark_t& rpFirst,
        const IDocumentMarkAccess::pMark_t& rpSecond)
    {
        return rpFirst->GetMarkEnd() < rpSecond->GetMarkEnd();
    }

    static void lcl_InsertMarkSorted(IDocumentMarkAccess::container_t& io_vMarks,
        const IDocumentMarkAccess::pMark_t& pMark)
    {
        io_vMarks.insert(
            lower_bound(
                io_vMarks.begin(),
                io_vMarks.end(),
                pMark,
                &lcl_MarkOrderingByStart),
            pMark);
    }

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    static inline auto_ptr<SwPosition> lcl_PositionFromCntntNode(SwCntntNode * const pCntntNode, const bool bAtEnd=false)
    {
        auto_ptr<SwPosition> pResult(new SwPosition(*pCntntNode));
        pResult->nContent.Assign(pCntntNode, bAtEnd ? pCntntNode->Len() : 0);
        return pResult;
    }
    SAL_WNODEPRECATED_DECLARATIONS_POP

    // return a position at the begin of rEnd, if it is a CntntNode
    // else set it to the begin of the Node after rEnd, if there is one
    // else set it to the end of the node before rStt
    // else set it to the CntntNode of the Pos outside the Range
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    static inline auto_ptr<SwPosition> lcl_FindExpelPosition(const SwNodeIndex& rStt,
        const SwNodeIndex& rEnd,
        const SwPosition& rOtherPosition)
    {
        SwCntntNode * pNode = rEnd.GetNode().GetCntntNode();
        SwNodeIndex aStt = SwNodeIndex(rStt);
        SwNodeIndex aEnd = SwNodeIndex(rEnd);
        bool bAtEnd = false;
        if(!pNode)
            pNode = rEnd.GetNodes().GoNext(&aEnd), bAtEnd = false;
        if(!pNode)
            pNode = rStt.GetNodes().GoPrevious(&aStt), bAtEnd = true;
        if(pNode)
            return lcl_PositionFromCntntNode(pNode, bAtEnd);
        return auto_ptr<SwPosition>(new SwPosition(rOtherPosition));
    }
    SAL_WNODEPRECATED_DECLARATIONS_POP

    static IMark* lcl_getMarkAfter(const IDocumentMarkAccess::container_t& rMarks, const SwPosition& rPos)
    {
        IDocumentMarkAccess::const_iterator_t pMarkAfter = upper_bound(
            rMarks.begin(),
            rMarks.end(),
            rPos,
            boost::bind(&IMark::StartsAfter, _2, _1)); // finds the first that is starting after
        if(pMarkAfter == rMarks.end()) return NULL;
        return pMarkAfter->get();
    };

    static IMark* lcl_getMarkBefore(const IDocumentMarkAccess::container_t& rMarks, const SwPosition& rPos)
    {
        // candidates from which to choose the mark before
        IDocumentMarkAccess::container_t vCandidates;
        // no need to consider marks starting after rPos
        IDocumentMarkAccess::const_iterator_t pCandidatesEnd = upper_bound(
            rMarks.begin(),
            rMarks.end(),
            rPos,
            boost::bind(&IMark::StartsAfter, _2, _1));
        vCandidates.reserve(pCandidatesEnd - rMarks.begin());
        // only marks ending before are candidates
        remove_copy_if(
            rMarks.begin(),
            pCandidatesEnd,
            back_inserter(vCandidates),
            boost::bind(logical_not<bool>(), boost::bind(&IMark::EndsBefore, _1, rPos)));
        // no candidate left => we are in front of the first mark or there are none
        if(!vCandidates.size()) return NULL;
        // return the highest (last) candidate using mark end ordering
        return max_element(vCandidates.begin(), vCandidates.end(), &lcl_MarkOrderingByEnd)->get();
    }

    static bool lcl_FixCorrectedMark(bool bChangedPos, bool bChangedOPos, MarkBase* io_pMark)
    {
        if( (bChangedPos || bChangedOPos) && io_pMark->IsExpanded() &&
            io_pMark->GetOtherMarkPos().nNode.GetNode().FindTableBoxStartNode() !=
            io_pMark->GetMarkPos().nNode.GetNode().FindTableBoxStartNode() )
        {
            if(!bChangedOPos)
                io_pMark->SetMarkPos(io_pMark->GetOtherMarkPos());
            io_pMark->ClearOtherMarkPos();
            DdeBookmark * const pDdeBkmk = dynamic_cast< DdeBookmark*>(io_pMark);
            if(pDdeBkmk && pDdeBkmk->IsServer())
                pDdeBkmk->SetRefObject(NULL);
            return true;
        }
        return false;
    }

    static IDocumentMarkAccess::iterator_t lcl_FindMark(
        IDocumentMarkAccess::container_t& rMarks,
        const IDocumentMarkAccess::pMark_t& rpMarkToFind)
    {
        IDocumentMarkAccess::iterator_t ppCurrentMark = lower_bound(
            rMarks.begin(), rMarks.end(),
            rpMarkToFind, &lcl_MarkOrderingByStart);
        // since there are usually not too many marks on the same start
        // position, we are not doing a bisect search for the upper bound
        // but instead start to iterate from pMarkLow directly
        while(ppCurrentMark != rMarks.end() && **ppCurrentMark == *rpMarkToFind)
        {
            if(ppCurrentMark->get() == rpMarkToFind.get())
            {
                //OSL_TRACE("found mark named '%s'",
                //    ::rtl::OUStringToOString(ppCurrentMark->get()->GetName(), RTL_TEXTENCODING_UTF8).getStr());
                return ppCurrentMark;
            }
            ++ppCurrentMark;
        }
        // reached a mark starting on a later start pos or the end of the
        // vector => not found
        return rMarks.end();
    };

    static IDocumentMarkAccess::iterator_t lcl_FindMarkAtPos(
        IDocumentMarkAccess::container_t& rMarks,
        const SwPosition& rPos,
        const IDocumentMarkAccess::MarkType eType)
    {
        for(IDocumentMarkAccess::iterator_t ppCurrentMark = lower_bound(
                rMarks.begin(), rMarks.end(),
                rPos,
                boost::bind(&IMark::StartsBefore, _1, _2));
            ppCurrentMark != rMarks.end();
            ++ppCurrentMark)
        {
            // Once we reach a mark starting after the target pos
            // we do not need to continue
            if(ppCurrentMark->get()->StartsAfter(rPos))
                break;
            if(IDocumentMarkAccess::GetType(**ppCurrentMark) == eType)
            {
                //OSL_TRACE("found mark named '%s'",
                //    ::rtl::OUStringToOString(ppCurrentMark->get()->GetName(), RTL_TEXTENCODING_UTF8).getStr());
                return ppCurrentMark;
            }
        }
        // reached a mark starting on a later start pos or the end of the
        // vector => not found
        return rMarks.end();
    };

    static IDocumentMarkAccess::const_iterator_t lcl_FindMarkByName(
        const ::rtl::OUString& rName,
        IDocumentMarkAccess::const_iterator_t ppMarksBegin,
        IDocumentMarkAccess::const_iterator_t ppMarksEnd)
    {
        return find_if(
            ppMarksBegin,
            ppMarksEnd,
            boost::bind(&::rtl::OUString::equals, boost::bind(&IMark::GetName, _1), rName));
    }

#if 0
    static void lcl_DebugMarks(IDocumentMarkAccess::container_t vMarks)
    {
        OSL_TRACE("%d Marks", vMarks.size());
        for(IDocumentMarkAccess::iterator_t ppMark = vMarks.begin();
            ppMark != vMarks.end();
            ppMark++)
        {
            IMark* pMark = ppMark->get();
            ::rtl::OString sName = ::rtl::OUStringToOString(pMark->GetName(), RTL_TEXTENCODING_UTF8);
            const SwPosition* const pStPos = &pMark->GetMarkStart();
            const SwPosition* const pEndPos = &pMark->GetMarkEnd();
            OSL_TRACE("%s %s %d,%d %d,%d",
                typeid(*pMark).name(),
                sName.getStr(),
                pStPos->nNode.GetIndex(),
                pStPos->nContent.GetIndex(),
                pEndPos->nNode.GetIndex(),
                pEndPos->nContent.GetIndex());
        }
    };
#endif
}

IDocumentMarkAccess::MarkType IDocumentMarkAccess::GetType(const IMark& rBkmk)
{
    const std::type_info* const pMarkTypeInfo = &typeid(rBkmk);
    // not using dynamic_cast<> here for performance
    if(*pMarkTypeInfo == typeid(UnoMark))
        return UNO_BOOKMARK;
    else if(*pMarkTypeInfo == typeid(DdeBookmark))
        return DDE_BOOKMARK;
    else if(*pMarkTypeInfo == typeid(Bookmark))
        return BOOKMARK;
    else if(*pMarkTypeInfo == typeid(CrossRefHeadingBookmark))
        return CROSSREF_HEADING_BOOKMARK;
    else if(*pMarkTypeInfo == typeid(CrossRefNumItemBookmark))
        return CROSSREF_NUMITEM_BOOKMARK;
    else if(*pMarkTypeInfo == typeid(TextFieldmark))
        return TEXT_FIELDMARK;
    else if(*pMarkTypeInfo == typeid(CheckboxFieldmark))
        return CHECKBOX_FIELDMARK;
    else if(*pMarkTypeInfo == typeid(NavigatorReminder))
        return NAVIGATOR_REMINDER;
    else
    {
        OSL_FAIL("IDocumentMarkAccess::GetType(..)"
            " - unknown MarkType. This needs to be fixed!");
        return UNO_BOOKMARK;
    }
}

namespace sw { namespace mark
{
    MarkManager::MarkManager(SwDoc& rDoc)
        : m_pDoc(&rDoc)
    { }
    ::sw::mark::IMark* MarkManager::makeMark(const SwPaM& rPaM,
        const ::rtl::OUString& rName,
        const IDocumentMarkAccess::MarkType eType)
    {
#if 0
        {
            ::rtl::OString sName = ::rtl::OUStringToOString(rName, RTL_TEXTENCODING_UTF8);
            const SwPosition* const pPos1 = rPaM.GetPoint();
            const SwPosition* pPos2 = pPos1;
            if(rPaM.HasMark())
                pPos2 = rPaM.GetMark();
            OSL_TRACE("%s %d,%d %d,%d",
                sName.getStr(),
                pPos1->nNode.GetIndex(),
                pPos1->nContent.GetIndex(),
                pPos2->nNode.GetIndex(),
                pPos2->nContent.GetIndex());
        }
#endif
        // see for example _SaveCntntIdx, Shells
        OSL_PRECOND(m_vMarks.size() < USHRT_MAX,
            "MarkManager::makeMark(..)"
            " - more than USHRT_MAX marks are not supported correctly");
        // There should only be one CrossRefBookmark per Textnode per Type
        OSL_PRECOND(
            (eType != CROSSREF_NUMITEM_BOOKMARK && eType != CROSSREF_HEADING_BOOKMARK)
            || (lcl_FindMarkAtPos(m_vBookmarks, *rPaM.GetPoint(), eType) == m_vBookmarks.end()),
            "MarkManager::makeMark(..)"
            " - creating duplicate CrossRefBookmark");

        // create mark
        pMark_t pMark;
        switch(eType)
        {
            case IDocumentMarkAccess::TEXT_FIELDMARK:
                pMark = boost::shared_ptr<IMark>(new TextFieldmark(rPaM));
                break;
            case IDocumentMarkAccess::CHECKBOX_FIELDMARK:
                pMark = boost::shared_ptr<IMark>(new CheckboxFieldmark(rPaM));
                break;
            case IDocumentMarkAccess::NAVIGATOR_REMINDER:
                pMark = boost::shared_ptr<IMark>(new NavigatorReminder(rPaM));
                break;
            case IDocumentMarkAccess::BOOKMARK:
                pMark = boost::shared_ptr<IMark>(new Bookmark(rPaM, KeyCode(), rName, ::rtl::OUString()));
                break;
            case IDocumentMarkAccess::DDE_BOOKMARK:
                pMark = boost::shared_ptr<IMark>(new DdeBookmark(rPaM));
                break;
            case IDocumentMarkAccess::CROSSREF_HEADING_BOOKMARK:
                pMark = boost::shared_ptr<IMark>(new CrossRefHeadingBookmark(rPaM, KeyCode(), rName, ::rtl::OUString()));
                break;
            case IDocumentMarkAccess::CROSSREF_NUMITEM_BOOKMARK:
                pMark = boost::shared_ptr<IMark>(new CrossRefNumItemBookmark(rPaM, KeyCode(), rName, ::rtl::OUString()));
                break;
            case IDocumentMarkAccess::UNO_BOOKMARK:
                pMark = boost::shared_ptr<IMark>(new UnoMark(rPaM));
                break;
        }
        OSL_ENSURE(pMark.get(),
            "MarkManager::makeMark(..)"
            " - Mark was not created.");
        MarkBase* pMarkBase = dynamic_cast<MarkBase*>(pMark.get());

        if(pMark->GetMarkPos() != pMark->GetMarkStart())
            pMarkBase->Swap();

        // for performance reasons, we trust UnoMarks to have a (generated) unique name
        if(eType != IDocumentMarkAccess::UNO_BOOKMARK)
            pMarkBase->SetName(getUniqueMarkName(pMarkBase->GetName()));

        // register mark
        m_aMarkNamesSet.insert(pMarkBase->GetName());
        lcl_InsertMarkSorted(m_vMarks, pMark);
        switch(eType)
        {
            case IDocumentMarkAccess::BOOKMARK:
            case IDocumentMarkAccess::CROSSREF_NUMITEM_BOOKMARK:
            case IDocumentMarkAccess::CROSSREF_HEADING_BOOKMARK:
                lcl_InsertMarkSorted(m_vBookmarks, pMark);
                break;
            case IDocumentMarkAccess::TEXT_FIELDMARK:
            case IDocumentMarkAccess::CHECKBOX_FIELDMARK:
                lcl_InsertMarkSorted(m_vFieldmarks, pMark);
                break;
            case IDocumentMarkAccess::NAVIGATOR_REMINDER:
            case IDocumentMarkAccess::DDE_BOOKMARK:
            case IDocumentMarkAccess::UNO_BOOKMARK:
            // no special array for these
                break;
        }
        pMarkBase->InitDoc(m_pDoc);
#if 0
        OSL_TRACE("--- makeType ---");
        OSL_TRACE("Marks");
        lcl_DebugMarks(m_vMarks);
        OSL_TRACE("Bookmarks");
        lcl_DebugMarks(m_vBookmarks);
        OSL_TRACE("Fieldmarks");
        lcl_DebugMarks(m_vFieldmarks);
#endif

        return pMark.get();
    }

    ::sw::mark::IFieldmark* MarkManager::makeFieldBookmark( const SwPaM& rPaM,
        const rtl::OUString& rName,
        const rtl::OUString& rType )
    {
        sw::mark::IMark* pMark = makeMark( rPaM, rName,
                IDocumentMarkAccess::TEXT_FIELDMARK );
        sw::mark::IFieldmark* pFieldMark = dynamic_cast<sw::mark::IFieldmark*>( pMark );
        pFieldMark->SetFieldname( rType );

        return pFieldMark;
    }

    ::sw::mark::IFieldmark* MarkManager::makeNoTextFieldBookmark( const SwPaM& rPaM,
        const rtl::OUString& rName,
        const rtl::OUString& rType)
    {
        sw::mark::IMark* pMark = makeMark( rPaM, rName,
                IDocumentMarkAccess::CHECKBOX_FIELDMARK );
        sw::mark::IFieldmark* pFieldMark = dynamic_cast<sw::mark::IFieldmark*>( pMark );
        pFieldMark->SetFieldname( rType );

        return pFieldMark;
    }

    ::sw::mark::IMark* MarkManager::getMarkForTxtNode(const SwTxtNode& rTxtNode,
        const IDocumentMarkAccess::MarkType eType)
    {
        SwPosition aPos(rTxtNode);
        aPos.nContent.Assign(&(const_cast<SwTxtNode&>(rTxtNode)), 0);
        const iterator_t ppExistingMark = lcl_FindMarkAtPos(m_vBookmarks, aPos, eType);
        if(ppExistingMark != m_vBookmarks.end())
            return ppExistingMark->get();
        const SwPaM aPaM(aPos);
        return makeMark(aPaM, ::rtl::OUString(), eType);
    }

    void MarkManager::repositionMark( ::sw::mark::IMark* const io_pMark,
        const SwPaM& rPaM)
    {
        OSL_PRECOND(io_pMark->GetMarkPos().GetDoc() == m_pDoc,
            "<MarkManager::repositionMark(..)>"
            " - Mark is not in my doc.");
        MarkBase* const pMarkBase = dynamic_cast< MarkBase* >(io_pMark);
        pMarkBase->SetMarkPos(*(rPaM.GetPoint()));
        if(rPaM.HasMark())
            pMarkBase->SetOtherMarkPos(*(rPaM.GetMark()));
        else
            pMarkBase->ClearOtherMarkPos();

        if(pMarkBase->GetMarkPos() != pMarkBase->GetMarkStart())
            pMarkBase->Swap();

        sortMarks();
    }

    bool MarkManager::renameMark(::sw::mark::IMark* io_pMark, const ::rtl::OUString& rNewName)
    {
        OSL_PRECOND(io_pMark->GetMarkPos().GetDoc() == m_pDoc,
            "<MarkManager::renameMark(..)>"
            " - Mark is not in my doc.");
        if(io_pMark->GetName() == rNewName)
            return true;
        if(hasMark(rNewName))
            return false;
        m_aMarkNamesSet.erase(dynamic_cast< ::sw::mark::MarkBase* >(io_pMark)->GetName());
        m_aMarkNamesSet.insert(rNewName);
        dynamic_cast< ::sw::mark::MarkBase* >(io_pMark)->SetName(rNewName);
        return true;
    }

    void MarkManager::correctMarksAbsolute(const SwNodeIndex& rOldNode, const SwPosition& rNewPos, const xub_StrLen nOffset)
    {
        const SwNode* const pOldNode = &rOldNode.GetNode();
        SwPosition aNewPos(rNewPos);
        aNewPos.nContent += nOffset;
        bool isSortingNeeded = false;
        for(iterator_t ppMark = m_vMarks.begin();
            ppMark != m_vMarks.end();
            ppMark++)
        {
            // is on position ??
            bool bChangedPos = false, bChangedOPos = false;
            ::sw::mark::MarkBase* pMark = dynamic_cast< ::sw::mark::MarkBase* >(ppMark->get());
            if(&pMark->GetMarkPos().nNode.GetNode() == pOldNode)
            {
                pMark->SetMarkPos(aNewPos);
                bChangedPos = true;
            }
            if (pMark->IsExpanded() &&
                &pMark->GetOtherMarkPos().nNode.GetNode() == pOldNode)
            {
                pMark->SetMarkPos(aNewPos);
                bChangedOPos= true;
            }
            // illegal selection? collapse the mark and restore sorting later
            isSortingNeeded |= lcl_FixCorrectedMark(bChangedPos, bChangedOPos, pMark);
        }
        // restore sorting if needed
        if(isSortingNeeded)
            sortMarks();
#if 0
        OSL_TRACE("correctMarksAbsolute");
        lcl_DebugMarks(m_vMarks);
#endif
    }

    void MarkManager::correctMarksRelative(const SwNodeIndex& rOldNode, const SwPosition& rNewPos, const xub_StrLen nOffset)
    {
        const SwNode* const pOldNode = &rOldNode.GetNode();
        SwPosition aNewPos(rNewPos);
        aNewPos.nContent += nOffset;
        bool isSortingNeeded = false;
        for(iterator_t ppMark = m_vMarks.begin();
            ppMark != m_vMarks.end();
            ppMark++)
        {
            // is on position ??
            bool bChangedPos = false, bChangedOPos = false;
            ::sw::mark::MarkBase* const pMark = dynamic_cast< ::sw::mark::MarkBase* >(ppMark->get());
            if(&pMark->GetMarkPos().nNode.GetNode() == pOldNode)
            {
                SwPosition aNewPosRel(aNewPos);
                aNewPosRel.nContent += pMark->GetMarkPos().nContent.GetIndex();
                pMark->SetMarkPos(aNewPosRel);
                bChangedPos = true;
            }
            if(pMark->IsExpanded() &&
                &pMark->GetOtherMarkPos().nNode.GetNode() == pOldNode)
            {
                SwPosition aNewPosRel(aNewPos);
                aNewPosRel.nContent += pMark->GetOtherMarkPos().nContent.GetIndex();
                pMark->SetOtherMarkPos(aNewPosRel);
                bChangedOPos = true;
            }
            // illegal selection? collapse the mark and restore sorting later
            isSortingNeeded |= lcl_FixCorrectedMark(bChangedPos, bChangedOPos, pMark);
        }
        // restore sorting if needed
        if(isSortingNeeded)
            sortMarks();
#if 0
        OSL_TRACE("correctMarksRelative");
        lcl_DebugMarks(m_vMarks);
#endif
    }

    void MarkManager::deleteMarks(
            const SwNodeIndex& rStt,
            const SwNodeIndex& rEnd,
            ::std::vector<SaveBookmark>* pSaveBkmk,
            const SwIndex* pSttIdx,
            const SwIndex* pEndIdx )
    {
        vector<const_iterator_t> vMarksToDelete;
        bool isSortingNeeded = false;
        // copy all bookmarks in the move area to a vector storing all position data as offsets
        // reassignment is performed after the move
        for(iterator_t ppMark = m_vMarks.begin();
            ppMark != m_vMarks.end();
            ppMark++)
        {
            // navigator marks should not be moved
            // TODO: Check if this might make them invalid
            if(IDocumentMarkAccess::GetType(**ppMark) == NAVIGATOR_REMINDER)
                continue;

            ::sw::mark::MarkBase* pMark = dynamic_cast< ::sw::mark::MarkBase* >(ppMark->get());
            // on position ??
            bool isPosInRange = (lcl_GreaterThan(pMark->GetMarkPos(), rStt, pSttIdx) &&
                lcl_Lower(pMark->GetMarkPos(), rEnd, pEndIdx));
            bool isOtherPosInRange = (pMark->IsExpanded() &&
                lcl_GreaterThan(pMark->GetOtherMarkPos(), rStt, pSttIdx) &&
                lcl_Lower(pMark->GetOtherMarkPos(), rEnd, pEndIdx));
            // special case: completely in range, touching the end?
            if(pEndIdx &&
                    ((isOtherPosInRange
                    && pMark->GetMarkPos().nNode == rEnd
                    && pMark->GetMarkPos().nContent == *pEndIdx)
                || (isPosInRange
                    && pMark->IsExpanded()
                    && pMark->GetOtherMarkPos().nNode == rEnd
                    && pMark->GetOtherMarkPos().nContent == *pEndIdx)))
            {
                isPosInRange = true, isOtherPosInRange = true;
            }

            if(isPosInRange && (isOtherPosInRange || !pMark->IsExpanded()))
            {
                // completely in range

                // #i92125#
                bool bKeepCrossRefBkmk( false );
                {
                    if ( rStt == rEnd &&
                         ( IDocumentMarkAccess::GetType(*pMark) ==
                            IDocumentMarkAccess::CROSSREF_HEADING_BOOKMARK ||
                           IDocumentMarkAccess::GetType(*pMark) ==
                            IDocumentMarkAccess::CROSSREF_NUMITEM_BOOKMARK ) )
                    {
                        bKeepCrossRefBkmk = true;
                    }
                }
                if ( !bKeepCrossRefBkmk )
                {
                    if(pSaveBkmk)
                        pSaveBkmk->push_back(SaveBookmark(true, true, *pMark, rStt, pSttIdx));
                    vMarksToDelete.push_back(ppMark);
                }
            }
            else if(isPosInRange ^ isOtherPosInRange)
            {
                // the bookmark is partitially in the range
                // move position of that is in the range out of it
                SAL_WNODEPRECATED_DECLARATIONS_PUSH
                auto_ptr<SwPosition> pNewPos;
                if(pEndIdx)
                    pNewPos = auto_ptr<SwPosition>(new SwPosition(
                        rEnd,
                        *pEndIdx));
                else
                    pNewPos = lcl_FindExpelPosition(
                        rStt,
                        rEnd,
                        isPosInRange ? pMark->GetOtherMarkPos() : pMark->GetMarkPos());
                SAL_WNODEPRECATED_DECLARATIONS_POP

                // #i92125#
                // no move of position for cross-reference bookmarks,
                // if move occurs inside a certain node
                if ( ( IDocumentMarkAccess::GetType(*pMark) !=
                                IDocumentMarkAccess::CROSSREF_HEADING_BOOKMARK &&
                       IDocumentMarkAccess::GetType(*pMark) !=
                                IDocumentMarkAccess::CROSSREF_NUMITEM_BOOKMARK ) ||
                     pMark->GetMarkPos().nNode != pNewPos->nNode )
                {
                    if(isPosInRange)
                        pMark->SetMarkPos(*pNewPos);
                    else
                        pMark->SetOtherMarkPos(*pNewPos);

                    // illegal selection? collapse the mark and restore sorting later
                    isSortingNeeded |= lcl_FixCorrectedMark(isPosInRange, isOtherPosInRange, pMark);
                }
            }
        }

        // we just remembered the iterators to delete, so we do not need to search
        // for the boost::shared_ptr<> (the entry in m_vMarks) again
        // reverse iteration, since erasing an entry invalidates iterators
        // behind it (the iterators in vMarksToDelete are sorted)
        for(vector<const_iterator_t>::reverse_iterator pppMark = vMarksToDelete.rbegin();
            pppMark != vMarksToDelete.rend();
            ++pppMark)
        {
            deleteMark(*pppMark);
        }
        if(isSortingNeeded)
            sortMarks();
#if 0
        OSL_TRACE("deleteMarks");
        lcl_DebugMarks(m_vMarks);
#endif
    }

    void MarkManager::deleteMark(const const_iterator_t ppMark)
    {
        if(ppMark == m_vMarks.end()) return;

        switch(IDocumentMarkAccess::GetType(**ppMark))
        {
            case IDocumentMarkAccess::BOOKMARK:
            case IDocumentMarkAccess::CROSSREF_HEADING_BOOKMARK:
            case IDocumentMarkAccess::CROSSREF_NUMITEM_BOOKMARK:
            // if(dynamic_cast<IBookmark*>)
            {
                IDocumentMarkAccess::iterator_t ppBookmark = lcl_FindMark(m_vBookmarks, *ppMark);
                OSL_ENSURE(ppBookmark != m_vBookmarks.end(),
                    "<MarkManager::deleteMark(..)>"
                    " - Bookmark not found.");
                m_vBookmarks.erase(ppBookmark);
                break;
            }
            case IDocumentMarkAccess::TEXT_FIELDMARK:
            case IDocumentMarkAccess::CHECKBOX_FIELDMARK:
            {
                IDocumentMarkAccess::iterator_t ppFieldmark = lcl_FindMark(m_vFieldmarks, *ppMark);
                OSL_ENSURE(ppFieldmark != m_vFieldmarks.end(),
                    "<MarkManager::deleteMark(..)>"
                    " - Bookmark not found.");
                m_vFieldmarks.erase(ppFieldmark);
                sw::mark::TextFieldmark* pTextFieldmark = dynamic_cast<sw::mark::TextFieldmark*>(ppMark->get());
                if (pTextFieldmark)
                    pTextFieldmark->ReleaseDoc(m_pDoc);
                break;
            }
            case IDocumentMarkAccess::NAVIGATOR_REMINDER:
            case IDocumentMarkAccess::DDE_BOOKMARK:
            case IDocumentMarkAccess::UNO_BOOKMARK:
            // no special array for these
                break;
        }
        DdeBookmark* const pDdeBookmark = dynamic_cast<DdeBookmark*>(ppMark->get());
        if(pDdeBookmark)
            pDdeBookmark->DeregisterFromDoc(m_pDoc);
        //Effective STL Item 27, get a non-const iterator aI at the same
        //position as const iterator ppMark was
        iterator_t aI = m_vMarks.begin();
        std::advance(aI, std::distance<const_iterator_t>(aI, ppMark));

        //fdo#37974
        //a) a mark destructor may callback into this method.
        //b) vector::erase first calls the destructor of the object, then
        //removes it from the vector.
        //So if the only reference to the object is the one
        //in the vector then we may reenter this method when the mark
        //is destructed but before it is removed, i.e. findMark still
        //finds the object whose destructor is being run. Take a temp
        //extra reference on the shared_ptr, remove the entry from the
        //vector, and on xHoldPastErase release findMark won't find
        //it anymore.
        pMark_t xHoldPastErase = *aI;
        m_aMarkNamesSet.erase(ppMark->get()->GetName());
        m_vMarks.erase(aI);
    }

    void MarkManager::deleteMark(const IMark* const pMark)
    {
        OSL_PRECOND(pMark->GetMarkPos().GetDoc() == m_pDoc,
            "<MarkManager::deleteMark(..)>"
            " - Mark is not in my doc.");
        // finds the last Mark that is starting before pMark
        // (pMarkLow < pMark)
        iterator_t pMarkLow = lower_bound(
            m_vMarks.begin(), m_vMarks.end(),
            pMark->GetMarkStart(),
            boost::bind(&IMark::StartsBefore, _1, _2));
        // finds the first Mark that pMark is starting before
        // (pMark < pMarkHigh)
        //iterator_t pMarkHigh = upper_bound(
        //    pMarkLow, m_vMarks.end(),
        //    pMark->GetMarkStart(),
        //    bind(&IMark::StartsBefore, _2, _1));
        // since it should be rare that pMark isnt found at all
        // we skip the bisect search on the upper bound
        iterator_t pMarkHigh = m_vMarks.end();
        iterator_t pMarkFound = find_if(
            pMarkLow, pMarkHigh,
            boost::bind(equal_to<const IMark*>(), boost::bind(&boost::shared_ptr<IMark>::get, _1), pMark));
        if(pMarkFound != pMarkHigh)
            deleteMark(pMarkFound);
    }

    void MarkManager::clearAllMarks()
    {
        m_vFieldmarks.clear();
        m_vBookmarks.clear();
        m_aMarkNamesSet.clear();
#if OSL_DEBUG_LEVEL > 0
        for(iterator_t pBkmk = m_vMarks.begin();
            pBkmk != m_vMarks.end();
            ++pBkmk)
            OSL_ENSURE(pBkmk->unique(),
                "<MarkManager::clearAllMarks(..)>"
                " - a Bookmark is still in use.");
#endif
        m_vMarks.clear();
    }

    IDocumentMarkAccess::const_iterator_t MarkManager::findMark(const ::rtl::OUString& rName) const
    {
        return lcl_FindMarkByName(rName, m_vMarks.begin(), m_vMarks.end());
    }

    IDocumentMarkAccess::const_iterator_t MarkManager::findBookmark(const ::rtl::OUString& rName) const
    {
        return lcl_FindMarkByName(rName, m_vBookmarks.begin(), m_vBookmarks.end());
    }

    IDocumentMarkAccess::const_iterator_t MarkManager::getMarksBegin() const
        { return m_vMarks.begin(); }

    IDocumentMarkAccess::const_iterator_t MarkManager::getMarksEnd() const
        { return m_vMarks.end(); }

    sal_Int32 MarkManager::getMarksCount() const
        { return m_vMarks.size(); }

    IDocumentMarkAccess::const_iterator_t MarkManager::getBookmarksBegin() const
        { return m_vBookmarks.begin(); }

    IDocumentMarkAccess::const_iterator_t MarkManager::getBookmarksEnd() const
        { return m_vBookmarks.end(); }

    sal_Int32 MarkManager::getBookmarksCount() const
        { return m_vBookmarks.size(); }

    IFieldmark* MarkManager::getFieldmarkFor(const SwPosition& rPos) const
    {
        const_iterator_t pFieldmark = find_if(
            m_vFieldmarks.begin(),
            m_vFieldmarks.end( ),
            boost::bind(&IMark::IsCoveringPosition, _1, rPos));
        if(pFieldmark == m_vFieldmarks.end()) return NULL;
        return dynamic_cast<IFieldmark*>(pFieldmark->get());
    }

    IFieldmark* MarkManager::getFieldmarkAfter(const SwPosition& rPos) const
        { return dynamic_cast<IFieldmark*>(lcl_getMarkAfter(m_vFieldmarks, rPos)); }

    IFieldmark* MarkManager::getFieldmarkBefore(const SwPosition& rPos) const
        { return dynamic_cast<IFieldmark*>(lcl_getMarkBefore(m_vFieldmarks, rPos)); }


    ::rtl::OUString MarkManager::getUniqueMarkName(const ::rtl::OUString& rName) const
    {
        OSL_ENSURE(!rName.isEmpty(),
            "<MarkManager::getUniqueMarkName(..)>"
            " - a name should be proposed");
        if(!hasMark(rName)) return rName;
        ::rtl::OUStringBuffer sBuf;
        ::rtl::OUString sTmp;

        // try the name "<rName>XXX" (where XXX is a number starting from 1) unless there is
        // a unused name. Due to performance-reasons (especially in mailmerge-Szenarios) there
        // is a map m_aMarkBasenameMapUniqueOffset which holds the next possible offset (XXX) for
        // rName (so there is no need to test for nCnt-values smaller than the offset).
        sal_Int32 nCnt = 1;
        MarkBasenameMapUniqueOffset_t::const_iterator aIter = m_aMarkBasenameMapUniqueOffset.find(rName);
        if(aIter != m_aMarkBasenameMapUniqueOffset.end()) nCnt = aIter->second;
        while(nCnt < SAL_MAX_INT32)
        {
            sTmp = sBuf.append(rName).append(nCnt).makeStringAndClear();
            nCnt++;
            if(!hasMark(sTmp)) break;
        }
        m_aMarkBasenameMapUniqueOffset[rName] = nCnt;

        return sTmp;
    }

    void MarkManager::sortMarks()
    {
        sort(m_vMarks.begin(), m_vMarks.end(), &lcl_MarkOrderingByStart);
        sort(m_vBookmarks.begin(), m_vBookmarks.end(), &lcl_MarkOrderingByStart);
        sort(m_vFieldmarks.begin(), m_vFieldmarks.end(), &lcl_MarkOrderingByStart);
    }

    bool MarkManager::hasMark(const ::rtl::OUString& rName) const
    {
        return (m_aMarkNamesSet.find(rName) != m_aMarkNamesSet.end());
    }

}} // namespace ::sw::mark


#define PCURCRSR (_pCurrCrsr)
#define FOREACHPAM_START(pSttCrsr) \
    {\
        SwPaM *_pStartCrsr = pSttCrsr, *_pCurrCrsr = pSttCrsr; \
        do {

#define FOREACHPAM_END() \
        } while( (_pCurrCrsr=(SwPaM *)_pCurrCrsr->GetNext()) != _pStartCrsr ); \
    }
#define PCURSH ((SwCrsrShell*)_pStartShell)
#define FOREACHSHELL_START( pEShell ) \
    {\
        ViewShell *_pStartShell = pEShell; \
        do { \
            if( _pStartShell->IsA( TYPE( SwCrsrShell )) ) \
            {

#define FOREACHSHELL_END( pEShell ) \
            } \
        } while((_pStartShell=(ViewShell*)_pStartShell->GetNext())!= pEShell ); \
    }

namespace
{
    // Array structure: 2 longs,
    //  1st long contains the type and position in the DocArray,
    //  2nd long contains the ContentPosition
    //
    //  CntntType --
    //          0x8000 = Bookmark Pos1
    //          0x8001 = Bookmark Pos2
    //          0x2000 = Paragraph anchored frame
    //          0x2001 = frame anchored at character, which should be moved
    //          0x1000 = Redline Mark
    //          0x1001 = Redline Point
    //          0x0800 = Crsr from the CrsrShell Mark
    //          0x0801 = Crsr from the CrsrShell Point
    //          0x0400 = UnoCrsr Mark
    //          0x0401 = UnoCrsr Point
    //

    class _SwSaveTypeCountContent
    {
        union {
            struct { sal_uInt16 nType, nCount; } TC;
            sal_uLong nTypeCount;
            } TYPECOUNT;
        xub_StrLen nContent;

    public:
        _SwSaveTypeCountContent() { TYPECOUNT.nTypeCount = 0; nContent = 0; }
        _SwSaveTypeCountContent( sal_uInt16 nType )
            {
                SetTypeAndCount( nType, 0 );
                nContent = 0;
            }
        _SwSaveTypeCountContent( const std::vector<sal_uLong> &rArr, sal_uInt16& rPos )
            {
                TYPECOUNT.nTypeCount = rArr[ rPos++ ];
                nContent = static_cast<xub_StrLen>(rArr[ rPos++ ]);
            }
        void Add( std::vector<sal_uLong> &rArr )
        {
            rArr.push_back( TYPECOUNT.nTypeCount );
            rArr.push_back( nContent );
        }

        void SetType( sal_uInt16 n )        { TYPECOUNT.TC.nType = n; }
        sal_uInt16 GetType() const          { return TYPECOUNT.TC.nType; }
        void IncType()                  { ++TYPECOUNT.TC.nType; }
        void DecType()                  { --TYPECOUNT.TC.nType; }

        void SetCount( sal_uInt16 n )       { TYPECOUNT.TC.nCount = n; }
        sal_uInt16 GetCount() const         { return TYPECOUNT.TC.nCount; }
        sal_uInt16 IncCount()               { return ++TYPECOUNT.TC.nCount; }
        sal_uInt16 DecCount()               { return --TYPECOUNT.TC.nCount; }

        void SetTypeAndCount( sal_uInt16 nT, sal_uInt16 nC )
            { TYPECOUNT.TC.nCount = nC; TYPECOUNT.TC.nType = nT; }

        void SetContent( xub_StrLen n )     { nContent = n; }
        xub_StrLen GetContent() const       { return nContent; }
    };

    // #i59534: If a paragraph will be splitted we have to restore some redline positions
    // This help function checks a position compared with a node and an content index

    static const int BEFORE_NODE = 0;          // Position before the given node index
    static const int BEFORE_SAME_NODE = 1;     // Same node index but content index before given content index
    static const int SAME_POSITION = 2;        // Same node index and samecontent index
    static const int BEHIND_SAME_NODE = 3;     // Same node index but content index behind given content index
    static const int BEHIND_NODE = 4;          // Position behind the given node index

    static int lcl_RelativePosition( const SwPosition& rPos, sal_uLong nNode, xub_StrLen nCntnt )
    {
        sal_uLong nIndex = rPos.nNode.GetIndex();
        int nReturn = BEFORE_NODE;
        if( nIndex == nNode )
        {
            xub_StrLen nCntIdx = rPos.nContent.GetIndex();
            if( nCntIdx < nCntnt )
                nReturn = BEFORE_SAME_NODE;
            else if( nCntIdx == nCntnt )
                nReturn = SAME_POSITION;
            else
                nReturn = BEHIND_SAME_NODE;
        }
        else if( nIndex > nNode )
            nReturn = BEHIND_NODE;
        return nReturn;
    }


    static inline int lcl_Greater( const SwPosition& rPos, const SwNodeIndex& rNdIdx, const SwIndex* pIdx )
    {
        return rPos.nNode > rNdIdx || ( pIdx && rPos.nNode == rNdIdx && rPos.nContent > pIdx->GetIndex() );
    }

    static void lcl_ChkPaM( std::vector<sal_uLong> &rSaveArr, sal_uLong nNode, xub_StrLen nCntnt,
                    const SwPaM& rPam, _SwSaveTypeCountContent& rSave,
                    sal_Bool bChkSelDirection )
    {
        // Respect direction of selection
        bool bBound1IsStart = !bChkSelDirection ? sal_True :
                            ( *rPam.GetPoint() < *rPam.GetMark()
                                ? rPam.GetPoint() == &rPam.GetBound()
                                : rPam.GetMark() == &rPam.GetBound());

        const SwPosition* pPos = &rPam.GetBound( sal_True );
        if( pPos->nNode.GetIndex() == nNode &&
            ( bBound1IsStart ? pPos->nContent.GetIndex() < nCntnt
                                : pPos->nContent.GetIndex() <= nCntnt ))
        {
            rSave.SetContent( pPos->nContent.GetIndex() );
            rSave.Add( rSaveArr );
        }

        pPos = &rPam.GetBound( sal_False );
        if( pPos->nNode.GetIndex() == nNode &&
            ( (bBound1IsStart && bChkSelDirection)
                        ? pPos->nContent.GetIndex() <= nCntnt
                        : pPos->nContent.GetIndex() < nCntnt ))
        {
            rSave.SetContent( pPos->nContent.GetIndex() );
            rSave.IncType();
            rSave.Add( rSaveArr );
            rSave.DecType();
        }
    }

}


// IDocumentMarkAccess for SwDoc

IDocumentMarkAccess* SwDoc::getIDocumentMarkAccess()
    { return static_cast< IDocumentMarkAccess* >(pMarkManager.get()); }

const IDocumentMarkAccess* SwDoc::getIDocumentMarkAccess() const
    { return static_cast< IDocumentMarkAccess* >(pMarkManager.get()); }

// SaveBookmark

SaveBookmark::SaveBookmark(
    bool bSavePos,
    bool bSaveOtherPos,
    const IMark& rBkmk,
    const SwNodeIndex & rMvPos,
    const SwIndex* pIdx)
    : m_aName(rBkmk.GetName())
    , m_aShortName()
    , m_aCode()
    , m_bSavePos(bSavePos)
    , m_bSaveOtherPos(bSaveOtherPos)
    , m_eOrigBkmType(IDocumentMarkAccess::GetType(rBkmk))
{
    const IBookmark* const pBookmark = dynamic_cast< const IBookmark* >(&rBkmk);
    if(pBookmark)
    {
        m_aShortName = pBookmark->GetShortName();
        m_aCode = pBookmark->GetKeyCode();

        ::sfx2::Metadatable const*const pMetadatable(
                dynamic_cast< ::sfx2::Metadatable const* >(pBookmark));
        if (pMetadatable)
        {
            m_pMetadataUndo = pMetadatable->CreateUndo();
        }
    }
    m_nNode1 = rBkmk.GetMarkPos().nNode.GetIndex();
    m_nCntnt1 = rBkmk.GetMarkPos().nContent.GetIndex();

    if(m_bSavePos)
    {
        m_nNode1 -= rMvPos.GetIndex();
        if(pIdx && !m_nNode1)
            m_nCntnt1 -= pIdx->GetIndex();
    }

    if(rBkmk.IsExpanded())
    {
        m_nNode2 = rBkmk.GetOtherMarkPos().nNode.GetIndex();
        m_nCntnt2 = rBkmk.GetOtherMarkPos().nContent.GetIndex();

        if(m_bSaveOtherPos)
        {
            m_nNode2 -= rMvPos.GetIndex();
            if(pIdx && !m_nNode2)
                m_nCntnt2 -= pIdx->GetIndex();
        }
    }
    else
        m_nNode2 = ULONG_MAX, m_nCntnt2 = STRING_NOTFOUND;
}

void SaveBookmark::SetInDoc(
    SwDoc* pDoc,
    const SwNodeIndex& rNewPos,
    const SwIndex* pIdx)
{
    SwPaM aPam(rNewPos.GetNode());
    if(pIdx)
        aPam.GetPoint()->nContent = *pIdx;

    if(ULONG_MAX != m_nNode2)
    {
        aPam.SetMark();

        if(m_bSaveOtherPos)
        {
            aPam.GetMark()->nNode += m_nNode2;
            if(pIdx && !m_nNode2)
                aPam.GetMark()->nContent += m_nCntnt2;
            else
                aPam.GetMark()->nContent.Assign(aPam.GetCntntNode(sal_False), m_nCntnt2);
        }
        else
        {
            aPam.GetMark()->nNode = m_nNode2;
            aPam.GetMark()->nContent.Assign(aPam.GetCntntNode(sal_False), m_nCntnt2);
        }
    }

    if(m_bSavePos)
    {
        aPam.GetPoint()->nNode += m_nNode1;

        if(pIdx && !m_nNode1)
            aPam.GetPoint()->nContent += m_nCntnt1;
        else
            aPam.GetPoint()->nContent.Assign(aPam.GetCntntNode(), m_nCntnt1);
    }
    else
    {
        aPam.GetPoint()->nNode = m_nNode1;
        aPam.GetPoint()->nContent.Assign(aPam.GetCntntNode(), m_nCntnt1);
    }

    if(!aPam.HasMark()
        || CheckNodesRange(aPam.GetPoint()->nNode, aPam.GetMark()->nNode, sal_True))
    {
        ::sw::mark::IBookmark* const pBookmark = dynamic_cast< ::sw::mark::IBookmark* >(pDoc->getIDocumentMarkAccess()->makeMark(aPam, m_aName, m_eOrigBkmType));
        if(pBookmark)
        {
            pBookmark->SetKeyCode(m_aCode);
            pBookmark->SetShortName(m_aShortName);
            if (m_pMetadataUndo)
            {
                ::sfx2::Metadatable * const pMeta(
                    dynamic_cast< ::sfx2::Metadatable* >(pBookmark));
                OSL_ENSURE(pMeta, "metadata undo, but not metadatable?");
                if (pMeta)
                {
                    pMeta->RestoreMetadata(m_pMetadataUndo);
                }
            }
        }
    }
}

// _DelBookmarks, _{Save,Restore}CntntIdx

void _DelBookmarks(
    const SwNodeIndex& rStt,
    const SwNodeIndex& rEnd,
    ::std::vector<SaveBookmark> * pSaveBkmk,
    const SwIndex* pSttIdx,
    const SwIndex* pEndIdx)
{
    // illegal range ??
    if(rStt.GetIndex() > rEnd.GetIndex()
        || (rStt == rEnd && (!pSttIdx || pSttIdx->GetIndex() >= pEndIdx->GetIndex())))
        return;
    SwDoc* const pDoc = rStt.GetNode().GetDoc();

    pDoc->getIDocumentMarkAccess()->deleteMarks(rStt, rEnd, pSaveBkmk, pSttIdx, pEndIdx);

    // Copy all Redlines which are in the move area into an array
    // which holds all position information as offset.
    // Assignement happens after moving.
    SwRedlineTbl& rTbl = (SwRedlineTbl&)pDoc->GetRedlineTbl();
    for(sal_uInt16 nCnt = 0; nCnt < rTbl.size(); ++nCnt )
    {
        // Is at position?
        SwRedline* pRedl = rTbl[ nCnt ];

        SwPosition *pRStt = &pRedl->GetBound(sal_True),
                   *pREnd = &pRedl->GetBound(sal_False);
        if( *pRStt > *pREnd )
        {
            SwPosition *pTmp = pRStt; pRStt = pREnd, pREnd = pTmp;
        }

        if( lcl_Greater( *pRStt, rStt, pSttIdx ) && lcl_Lower( *pRStt, rEnd, pEndIdx ))
        {
            pRStt->nNode = rEnd;
            if( pEndIdx )
                pRStt->nContent = *pEndIdx;
            else
            {
                sal_Bool bStt = sal_True;
                SwCntntNode* pCNd = pRStt->nNode.GetNode().GetCntntNode();
                if( !pCNd && 0 == ( pCNd = pDoc->GetNodes().GoNext( &pRStt->nNode )) )
                {
                    bStt = sal_False;
                    pRStt->nNode = rStt;
                    if( 0 == ( pCNd = pDoc->GetNodes().GoPrevious( &pRStt->nNode )) )
                    {
                        pRStt->nNode = pREnd->nNode;
                        pCNd = pRStt->nNode.GetNode().GetCntntNode();
                    }
                }
                xub_StrLen nTmp = bStt ? 0 : pCNd->Len();
                pRStt->nContent.Assign( pCNd, nTmp );
            }
        }
        if( lcl_Greater( *pREnd, rStt, pSttIdx ) && lcl_Lower( *pREnd, rEnd, pEndIdx ))
        {
            pREnd->nNode = rStt;
            if( pSttIdx )
                pREnd->nContent = *pSttIdx;
            else
            {
                sal_Bool bStt = sal_False;
                SwCntntNode* pCNd = pREnd->nNode.GetNode().GetCntntNode();
                if( !pCNd && 0 == ( pCNd = pDoc->GetNodes().GoPrevious( &pREnd->nNode )) )
                {
                    bStt = sal_True;
                    pREnd->nNode = rEnd;
                    if( 0 == ( pCNd = pDoc->GetNodes().GoNext( &pREnd->nNode )) )
                    {
                        pREnd->nNode = pRStt->nNode;
                        pCNd = pREnd->nNode.GetNode().GetCntntNode();
                    }
                }
                xub_StrLen nTmp = bStt ? 0 : pCNd->Len();
                pREnd->nContent.Assign( pCNd, nTmp );
            }
        }
    }
}

void _SaveCntntIdx(SwDoc* pDoc,
    sal_uLong nNode,
    xub_StrLen nCntnt,
    std::vector<sal_uLong> &rSaveArr,
    sal_uInt8 nSaveFly)
{
    // 1. Bookmarks
    _SwSaveTypeCountContent aSave;
    aSave.SetTypeAndCount( 0x8000, 0 );

    IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
    const sal_Int32 nBkmks = pMarkAccess->getMarksCount();
    for(; aSave.GetCount() < nBkmks; aSave.IncCount())
    {
        bool bEqual = false;
        bool bLower = false;
        const ::sw::mark::IMark* pBkmk = (pMarkAccess->getMarksBegin() + aSave.GetCount())->get();
        if(pBkmk->GetMarkPos().nNode.GetIndex() == nNode
            && pBkmk->GetMarkPos().nContent.GetIndex() <= nCntnt)
        {
            if(pBkmk->GetMarkPos().nContent.GetIndex() < nCntnt)
            {
                bLower = true; // a hint for the other position...
                aSave.SetContent(pBkmk->GetMarkPos().nContent.GetIndex());
                aSave.Add(rSaveArr);
            }
            else // if a bookmark position is equal nCntnt, the other position
                bEqual = true; // has to decide if it is added to the array
        }

        if(pBkmk->IsExpanded()
            && pBkmk->GetOtherMarkPos().nNode.GetIndex() == nNode
            && pBkmk->GetOtherMarkPos().nContent.GetIndex() <= nCntnt)
        {
            if(bLower || pBkmk->GetOtherMarkPos().nContent.GetIndex() < nCntnt)
            {
                if(bEqual)
                { // the other position is before, the (main) position is equal
                    aSave.SetContent(pBkmk->GetMarkPos().nContent.GetIndex());
                    aSave.Add(rSaveArr);
                }
                aSave.SetContent(pBkmk->GetOtherMarkPos().nContent.GetIndex());
                aSave.IncType();
                aSave.Add(rSaveArr);
                aSave.DecType();
            }
        }
    }

    // 2. Redlines
    aSave.SetTypeAndCount( 0x1000, 0 );
    const SwRedlineTbl& rRedlTbl = pDoc->GetRedlineTbl();
    for( ; aSave.GetCount() < rRedlTbl.size(); aSave.IncCount() )
    {
        const SwRedline* pRdl = rRedlTbl[ aSave.GetCount() ];
        int nPointPos = lcl_RelativePosition( *pRdl->GetPoint(), nNode, nCntnt );
        int nMarkPos = pRdl->HasMark() ? lcl_RelativePosition( *pRdl->GetMark(), nNode, nCntnt ) :
                                          nPointPos;
        // #i59534: We have to store the positions inside the same node before the insert position
        // and the one at the insert position if the corresponding Point/Mark position is before
        // the insert position.
        if( nPointPos == BEFORE_SAME_NODE ||
            ( nPointPos == SAME_POSITION && nMarkPos < SAME_POSITION ) )
        {
            aSave.SetContent( pRdl->GetPoint()->nContent.GetIndex() );
            aSave.IncType();
            aSave.Add( rSaveArr );
            aSave.DecType();
        }
        if( pRdl->HasMark() && ( nMarkPos == BEFORE_SAME_NODE ||
            ( nMarkPos == SAME_POSITION && nPointPos < SAME_POSITION ) ) )
        {
            aSave.SetContent( pRdl->GetMark()->nContent.GetIndex() );
            aSave.Add( rSaveArr );
        }
    }

    // 4. Paragraph anchored objects
    {
        SwCntntNode *pNode = pDoc->GetNodes()[nNode]->GetCntntNode();
        if( pNode )
        {

            SwFrm* pFrm = pNode->getLayoutFrm( pDoc->GetCurrentLayout() );
#if OSL_DEBUG_LEVEL > 1
            static sal_Bool bViaDoc = sal_False;
            if( bViaDoc )
                pFrm = NULL;
#endif
            if( pFrm ) // Do we have a layout? Then it's a bit cheaper ...
            {
                if( pFrm->GetDrawObjs() )
                {
                    const SwSortedObjs& rDObj = *pFrm->GetDrawObjs();
                    for( sal_uInt32 n = rDObj.Count(); n; )
                    {
                        SwAnchoredObject* pObj = rDObj[ --n ];
                        const SwFrmFmt& rFmt = pObj->GetFrmFmt();
                        const SwFmtAnchor& rAnchor = rFmt.GetAnchor();
                        SwPosition const*const pAPos = rAnchor.GetCntntAnchor();
                        if ( pAPos &&
                             ( ( nSaveFly &&
                                 FLY_AT_PARA == rAnchor.GetAnchorId() ) ||
                               ( FLY_AT_CHAR == rAnchor.GetAnchorId() ) ) )
                        {
                            aSave.SetType( 0x2000 );
                            aSave.SetContent( pAPos->nContent.GetIndex() );

                            OSL_ENSURE( nNode == pAPos->nNode.GetIndex(),
                                    "_SaveCntntIdx: Wrong Node-Index" );
                            if ( FLY_AT_CHAR == rAnchor.GetAnchorId() )
                            {
                                if( nCntnt <= aSave.GetContent() )
                                {
                                    if( SAVEFLY_SPLIT == nSaveFly )
                                        aSave.IncType(); // = 0x2001;
                                    else
                                        continue;
                                }
                            }
                            aSave.SetCount( pDoc->GetSpzFrmFmts()->size() );
                            while( aSave.GetCount() &&
                                    &rFmt != (*pDoc->GetSpzFrmFmts())[
                                    aSave.DecCount() ] )
                                ; // nothing
                            OSL_ENSURE( &rFmt == (*pDoc->GetSpzFrmFmts())[
                                                    aSave.GetCount() ],
                                    "_SaveCntntIdx: Lost FrameFormat" );
                            aSave.Add( rSaveArr );
                        }
                    }
                }
            }
            else // No layout, so it's a bit more expensive ...
            {
                for( aSave.SetCount( pDoc->GetSpzFrmFmts()->size() );
                        aSave.GetCount() ; )
                {
                    SwFrmFmt* pFrmFmt = (*pDoc->GetSpzFrmFmts())[
                                                aSave.DecCount() ];
                    if ( RES_FLYFRMFMT != pFrmFmt->Which() &&
                            RES_DRAWFRMFMT != pFrmFmt->Which() )
                        continue;

                    const SwFmtAnchor& rAnchor = pFrmFmt->GetAnchor();
                    SwPosition const*const pAPos = rAnchor.GetCntntAnchor();
                    if ( pAPos && ( nNode == pAPos->nNode.GetIndex() ) &&
                         ( FLY_AT_PARA == rAnchor.GetAnchorId() ||
                           FLY_AT_CHAR == rAnchor.GetAnchorId() ) )
                    {
                        aSave.SetType( 0x2000 );
                        aSave.SetContent( pAPos->nContent.GetIndex() );
                        if ( FLY_AT_CHAR == rAnchor.GetAnchorId() )
                        {
                            if( nCntnt <= aSave.GetContent() )
                            {
                                if( SAVEFLY_SPLIT == nSaveFly )
                                    aSave.IncType(); // = 0x2001;
                                else
                                    continue;
                            }
                        }
                        aSave.Add( rSaveArr );
                    }
                }
            }
        }
    }
    // 5. CrsrShell
    {
        SwCrsrShell* pShell = pDoc->GetEditShell();
        if( pShell )
        {
            aSave.SetTypeAndCount( 0x800, 0 );
            FOREACHSHELL_START( pShell )
                SwPaM *_pStkCrsr = PCURSH->GetStkCrsr();
                if( _pStkCrsr )
                do {
                    lcl_ChkPaM( rSaveArr, nNode, nCntnt, *_pStkCrsr,
                                aSave, sal_False );
                    aSave.IncCount();
                } while ( (_pStkCrsr != 0 ) &&
                    ((_pStkCrsr=(SwPaM *)_pStkCrsr->GetNext()) != PCURSH->GetStkCrsr()) );

                FOREACHPAM_START( PCURSH->_GetCrsr() )
                    lcl_ChkPaM( rSaveArr, nNode, nCntnt, *PCURCRSR,
                                aSave, sal_False );
                    aSave.IncCount();
                FOREACHPAM_END()

            FOREACHSHELL_END( pShell )
        }
    }
    // 6. UnoCrsr
    {
        aSave.SetTypeAndCount( 0x400, 0 );
        const SwUnoCrsrTbl& rTbl = pDoc->GetUnoCrsrTbl();
        for (SwUnoCrsrTbl::const_iterator it = rTbl.begin();
                it != rTbl.end(); ++it)
        {
            FOREACHPAM_START( *it )
                lcl_ChkPaM( rSaveArr, nNode, nCntnt, *PCURCRSR, aSave, sal_False );
                aSave.IncCount();
            FOREACHPAM_END()

            SwUnoTableCrsr* pUnoTblCrsr =
                dynamic_cast<SwUnoTableCrsr*>(*it);
            if( pUnoTblCrsr )
            {
                FOREACHPAM_START( &pUnoTblCrsr->GetSelRing() )
                    lcl_ChkPaM( rSaveArr, nNode, nCntnt, *PCURCRSR, aSave, sal_False );
                    aSave.IncCount();
                FOREACHPAM_END()
            }
        }
    }
}


void _RestoreCntntIdx(SwDoc* pDoc,
    std::vector<sal_uLong> &rSaveArr,
    sal_uLong nNode,
    xub_StrLen nOffset,
    sal_Bool bAuto)
{
    SwCntntNode* pCNd = pDoc->GetNodes()[ nNode ]->GetCntntNode();
    const SwRedlineTbl& rRedlTbl = pDoc->GetRedlineTbl();
    SwFrmFmts* pSpz = pDoc->GetSpzFrmFmts();
    IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
    sal_uInt16 n = 0;
    while( n < rSaveArr.size() )
    {
        _SwSaveTypeCountContent aSave( rSaveArr, n );
        SwPosition* pPos = 0;
        switch( aSave.GetType() )
        {
            case 0x8000:
            {
                MarkBase* pMark = dynamic_cast<MarkBase*>(pMarkAccess->getMarksBegin()[aSave.GetCount()].get());
                SwPosition aNewPos(pMark->GetMarkPos());
                aNewPos.nNode = *pCNd;
                aNewPos.nContent.Assign(pCNd, aSave.GetContent() + nOffset);
                pMark->SetMarkPos(aNewPos);
            }
            break;
            case 0x8001:
            {
                MarkBase* pMark = dynamic_cast<MarkBase*>(pMarkAccess->getMarksBegin()[aSave.GetCount()].get());
                SwPosition aNewPos(pMark->GetOtherMarkPos());
                aNewPos.nNode = *pCNd;
                aNewPos.nContent.Assign(pCNd, aSave.GetContent() + nOffset);
                pMark->SetOtherMarkPos(aNewPos);
            }
            break;
            case 0x1001:
                pPos = (SwPosition*)rRedlTbl[ aSave.GetCount() ]->GetPoint();
                break;
            case 0x1000:
                pPos = (SwPosition*)rRedlTbl[ aSave.GetCount() ]->GetMark();
                break;
            case 0x2000:
                {
                    SwFrmFmt *pFrmFmt = (*pSpz)[ aSave.GetCount() ];
                    const SwFmtAnchor& rFlyAnchor = pFrmFmt->GetAnchor();
                    if( rFlyAnchor.GetCntntAnchor() )
                    {
                        SwFmtAnchor aNew( rFlyAnchor );
                        SwPosition aNewPos( *rFlyAnchor.GetCntntAnchor() );
                        aNewPos.nNode = *pCNd;
                        if ( FLY_AT_CHAR == rFlyAnchor.GetAnchorId() )
                        {
                            aNewPos.nContent.Assign( pCNd,
                                                     aSave.GetContent() + nOffset );
                        }
                        else
                        {
                            aNewPos.nContent.Assign( 0, 0 );
                        }
                        aNew.SetAnchor( &aNewPos );
                        pFrmFmt->SetFmtAttr( aNew );
                    }
                }
                break;
            case 0x2001:
                if( bAuto )
                {
                    SwFrmFmt *pFrmFmt = (*pSpz)[ aSave.GetCount() ];
                    SfxPoolItem *pAnchor = (SfxPoolItem*)&pFrmFmt->GetAnchor();
                    pFrmFmt->NotifyClients( pAnchor, pAnchor );
                }
                break;

            case 0x0800:
            case 0x0801:
                {
                    SwCrsrShell* pShell = pDoc->GetEditShell();
                    if( pShell )
                    {
                        sal_uInt16 nCnt = 0;
                        FOREACHSHELL_START( pShell )
                            SwPaM *_pStkCrsr = PCURSH->GetStkCrsr();
                            if( _pStkCrsr )
                            do {
                                if( aSave.GetCount() == nCnt )
                                {
                                    pPos = &_pStkCrsr->GetBound( 0x0800 ==
                                                        aSave.GetType() );
                                    break;
                                }
                                ++nCnt;
                            } while ( (_pStkCrsr != 0 ) &&
                                ((_pStkCrsr=(SwPaM *)_pStkCrsr->GetNext()) != PCURSH->GetStkCrsr()) );

                            if( pPos )
                                break;

                            FOREACHPAM_START( PCURSH->_GetCrsr() )
                                if( aSave.GetCount() == nCnt )
                                {
                                    pPos = &PCURCRSR->GetBound( 0x0800 ==
                                                        aSave.GetType() );
                                    break;
                                }
                                ++nCnt;
                            FOREACHPAM_END()
                            if( pPos )
                                break;

                        FOREACHSHELL_END( pShell )
                    }
            }
            break;

        case 0x0400:
        case 0x0401:
            {
                sal_uInt16 nCnt = 0;
                const SwUnoCrsrTbl& rTbl = pDoc->GetUnoCrsrTbl();
                for (SwUnoCrsrTbl::const_iterator it = rTbl.begin();
                        it != rTbl.end(); ++it)
                {
                    FOREACHPAM_START( *it )
                        if( aSave.GetCount() == nCnt )
                        {
                            pPos = &PCURCRSR->GetBound( 0x0400 ==
                                                    aSave.GetType() );
                            break;
                        }
                        ++nCnt;
                    FOREACHPAM_END()
                    if( pPos )
                        break;

                    SwUnoTableCrsr* pUnoTblCrsr =
                        dynamic_cast<SwUnoTableCrsr*>(*it);
                    if ( pUnoTblCrsr )
                    {
                        FOREACHPAM_START( &pUnoTblCrsr->GetSelRing() )
                            if( aSave.GetCount() == nCnt )
                            {
                                pPos = &PCURCRSR->GetBound( 0x0400 ==
                                                    aSave.GetType() );
                                break;
                            }
                            ++nCnt;
                        FOREACHPAM_END()
                    }
                    if ( pPos )
                        break;
                }
            }
            break;
        }

        if( pPos )
        {
            pPos->nNode = *pCNd;
            pPos->nContent.Assign( pCNd, aSave.GetContent() + nOffset );
        }
    }
}

void _RestoreCntntIdx(std::vector<sal_uLong> &rSaveArr,
    const SwNode& rNd,
    xub_StrLen nLen,
    xub_StrLen nChkLen)
{
    const SwDoc* pDoc = rNd.GetDoc();
    const SwRedlineTbl& rRedlTbl = pDoc->GetRedlineTbl();
    const SwFrmFmts* pSpz = pDoc->GetSpzFrmFmts();
    const IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
    SwCntntNode* pCNd = (SwCntntNode*)rNd.GetCntntNode();

    sal_uInt16 n = 0;
    while( n < rSaveArr.size() )
    {
        _SwSaveTypeCountContent aSave( rSaveArr, n );
        if( aSave.GetContent() >= nChkLen )
            rSaveArr[ n-1 ] -= nChkLen;
        else
        {
            SwPosition* pPos = 0;
            switch( aSave.GetType() )
            {
            case 0x8000:
            {
                MarkBase* pMark = dynamic_cast<MarkBase*>(pMarkAccess->getMarksBegin()[aSave.GetCount()].get());
                SwPosition aNewPos(pMark->GetMarkPos());
                aNewPos.nNode = rNd;
                aNewPos.nContent.Assign(pCNd, Min(aSave.GetContent(), nLen));
                pMark->SetMarkPos(aNewPos);
            }
            break;
            case 0x8001:
            {
                MarkBase* pMark = dynamic_cast<MarkBase*>(pMarkAccess->getMarksBegin()[aSave.GetCount()].get());
                SwPosition aNewPos(pMark->GetOtherMarkPos());
                aNewPos.nNode = rNd;
                aNewPos.nContent.Assign(pCNd, Min(aSave.GetContent(), nLen));
                pMark->SetOtherMarkPos(aNewPos);
            }
            break;
            case 0x1001:
                pPos = (SwPosition*)rRedlTbl[ aSave.GetCount() ]->GetPoint();
                break;
            case 0x1000:
                pPos = (SwPosition*)rRedlTbl[ aSave.GetCount() ]->GetMark();
                break;
            case 0x2000:
            case 0x2001:
                {
                    SwFrmFmt *pFrmFmt = (*pSpz)[ aSave.GetCount() ];
                    const SwFmtAnchor& rFlyAnchor = pFrmFmt->GetAnchor();
                    if( rFlyAnchor.GetCntntAnchor() )
                    {
                        SwFmtAnchor aNew( rFlyAnchor );
                        SwPosition aNewPos( *rFlyAnchor.GetCntntAnchor() );
                        aNewPos.nNode = rNd;
                        if ( FLY_AT_CHAR == rFlyAnchor.GetAnchorId() )
                        {
                            aNewPos.nContent.Assign( pCNd, Min(
                                                     aSave.GetContent(), nLen ) );
                        }
                        else
                        {
                            aNewPos.nContent.Assign( 0, 0 );
                        }
                        aNew.SetAnchor( &aNewPos );
                        pFrmFmt->SetFmtAttr( aNew );
                    }
                }
                break;

            case 0x0800:
            case 0x0801:
                {
                    SwCrsrShell* pShell = pDoc->GetEditShell();
                    if( pShell )
                    {
                        sal_uInt16 nCnt = 0;
                        FOREACHSHELL_START( pShell )
                            SwPaM *_pStkCrsr = PCURSH->GetStkCrsr();
                            if( _pStkCrsr )
                            do {
                                if( aSave.GetCount() == nCnt )
                                {
                                    pPos = &_pStkCrsr->GetBound( 0x0800 ==
                                                aSave.GetType() );
                                    break;
                                }
                                ++nCnt;
                            } while ( (_pStkCrsr != 0 ) &&
                                ((_pStkCrsr=(SwPaM *)_pStkCrsr->GetNext()) != PCURSH->GetStkCrsr()) );

                            if( pPos )
                                break;

                            FOREACHPAM_START( PCURSH->_GetCrsr() )
                                if( aSave.GetCount() == nCnt )
                                {
                                    pPos = &PCURCRSR->GetBound( 0x0800 ==
                                                aSave.GetType() );
                                    break;
                                }
                                ++nCnt;
                            FOREACHPAM_END()
                            if( pPos )
                                break;

                        FOREACHSHELL_END( pShell )
                    }
                }
                break;

            case 0x0400:
            case 0x0401:
                {
                    sal_uInt16 nCnt = 0;
                    const SwUnoCrsrTbl& rTbl = pDoc->GetUnoCrsrTbl();
                    for (SwUnoCrsrTbl::const_iterator it = rTbl.begin();
                            it != rTbl.end(); ++it)
                    {
                        FOREACHPAM_START( *it )
                            if( aSave.GetCount() == nCnt )
                            {
                                pPos = &PCURCRSR->GetBound( 0x0400 ==
                                                    aSave.GetType() );
                                break;
                            }
                            ++nCnt;
                        FOREACHPAM_END()
                        if( pPos )
                            break;

                        SwUnoTableCrsr* pUnoTblCrsr =
                            dynamic_cast<SwUnoTableCrsr*>(*it);
                        if ( pUnoTblCrsr )
                        {
                            FOREACHPAM_START( &pUnoTblCrsr->GetSelRing() )
                                if( aSave.GetCount() == nCnt )
                                {
                                    pPos = &PCURCRSR->GetBound( 0x0400 ==
                                                    aSave.GetType() );
                                    break;
                                }
                                ++nCnt;
                            FOREACHPAM_END()
                        }
                        if ( pPos )
                            break;
                    }
                }
                break;
            }

            if( pPos )
            {
                pPos->nNode = rNd;
                pPos->nContent.Assign( pCNd, Min( aSave.GetContent(), nLen ) );
            }
            n -= 2;
            rSaveArr.erase( rSaveArr.begin() + n, rSaveArr.begin() + n + 2);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
