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

#include <MarkManager.hxx>
#include <bookmrk.hxx>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <cntfrm.hxx>
#include <crossrefbookmark.hxx>
#include <annotationmark.hxx>
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
#include <edimp.hxx>
#include <stdio.h>

using namespace ::boost;
using namespace ::sw::mark;

namespace
{
    static bool lcl_GreaterThan( const SwPosition& rPos, const SwNodeIndex& rNdIdx, const SwIndex* pIdx )
    {
        return pIdx != NULL
               ? ( rPos.nNode > rNdIdx
                   || ( rPos.nNode == rNdIdx
                        && rPos.nContent >= pIdx->GetIndex() ) )
               : rPos.nNode >= rNdIdx;
    }

    static bool lcl_Lower( const SwPosition& rPos, const SwNodeIndex& rNdIdx, const SwIndex* pIdx )
    {
        return rPos.nNode < rNdIdx
               || ( pIdx != NULL
                    && rPos.nNode == rNdIdx
                    && rPos.nContent < pIdx->GetIndex() );
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
    static inline ::std::auto_ptr<SwPosition> lcl_PositionFromCntntNode(
        SwCntntNode * const pCntntNode,
        const bool bAtEnd=false)
    {
        ::std::auto_ptr<SwPosition> pResult(new SwPosition(*pCntntNode));
        pResult->nContent.Assign(pCntntNode, bAtEnd ? pCntntNode->Len() : 0);
        return pResult;
    }
    SAL_WNODEPRECATED_DECLARATIONS_POP

    // return a position at the begin of rEnd, if it is a CntntNode
    // else set it to the begin of the Node after rEnd, if there is one
    // else set it to the end of the node before rStt
    // else set it to the CntntNode of the Pos outside the Range
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    static inline ::std::auto_ptr<SwPosition> lcl_FindExpelPosition(
        const SwNodeIndex& rStt,
        const SwNodeIndex& rEnd,
        const SwPosition& rOtherPosition)
    {
        SwCntntNode * pNode = rEnd.GetNode().GetCntntNode();
        bool bPosAtEndOfNode = false;
        if ( pNode == NULL)
        {
            SwNodeIndex aEnd = SwNodeIndex(rEnd);
            pNode = rEnd.GetNodes().GoNext( &aEnd );
            bPosAtEndOfNode = false;
        }
        if ( pNode == NULL )
        {
            SwNodeIndex aStt = SwNodeIndex(rStt);
            pNode = rStt.GetNodes().GoPrevious(&aStt);
            bPosAtEndOfNode = true;
        }
        if ( pNode != NULL )
        {
            return lcl_PositionFromCntntNode( pNode, bPosAtEndOfNode );
        }

        return ::std::auto_ptr<SwPosition>(new SwPosition(rOtherPosition));
    }
    SAL_WNODEPRECATED_DECLARATIONS_POP

    static IMark* lcl_getMarkAfter(const IDocumentMarkAccess::container_t& rMarks, const SwPosition& rPos)
    {
        IDocumentMarkAccess::const_iterator_t pMarkAfter = upper_bound(
            rMarks.begin(),
            rMarks.end(),
            rPos,
            sw::mark::CompareIMarkStartsAfter());
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
            sw::mark::CompareIMarkStartsAfter());
        vCandidates.reserve(pCandidatesEnd - rMarks.begin());
        // only marks ending before are candidates
        remove_copy_if(
            rMarks.begin(),
            pCandidatesEnd,
            back_inserter(vCandidates),
            boost::bind( ::std::logical_not<bool>(), boost::bind( &IMark::EndsBefore, _1, rPos ) ) );
        // no candidate left => we are in front of the first mark or there are none
        if(!vCandidates.size()) return NULL;
        // return the highest (last) candidate using mark end ordering
        return max_element(vCandidates.begin(), vCandidates.end(), &lcl_MarkOrderingByEnd)->get();
    }

    static bool lcl_FixCorrectedMark(
        const bool bChangedPos,
        const bool bChangedOPos,
        MarkBase* io_pMark )
    {
        if ( IDocumentMarkAccess::GetType(*io_pMark) == IDocumentMarkAccess::ANNOTATIONMARK )
        {
            // annotation marks are allowed to span a table cell range.
            // but trigger sorting to be save
            return true;
        }

        if ( ( bChangedPos || bChangedOPos )
             && io_pMark->IsExpanded()
             && io_pMark->GetOtherMarkPos().nNode.GetNode().FindTableBoxStartNode() !=
                    io_pMark->GetMarkPos().nNode.GetNode().FindTableBoxStartNode() )
        {
            if ( !bChangedOPos )
            {
                io_pMark->SetMarkPos( io_pMark->GetOtherMarkPos() );
            }
            io_pMark->ClearOtherMarkPos();
            DdeBookmark * const pDdeBkmk = dynamic_cast< DdeBookmark*>(io_pMark);
            if ( pDdeBkmk != NULL
                 && pDdeBkmk->IsServer() )
            {
                pDdeBkmk->SetRefObject(NULL);
            }
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
                //    OUStringToOString(ppCurrentMark->get()->GetName(), RTL_TEXTENCODING_UTF8).getStr());
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
                sw::mark::CompareIMarkStartsBefore());
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
                //    OUStringToOString(ppCurrentMark->get()->GetName(), RTL_TEXTENCODING_UTF8).getStr());
                return ppCurrentMark;
            }
        }
        // reached a mark starting on a later start pos or the end of the
        // vector => not found
        return rMarks.end();
    };

    static IDocumentMarkAccess::const_iterator_t lcl_FindMarkByName(
        const OUString& rName,
        IDocumentMarkAccess::const_iterator_t ppMarksBegin,
        IDocumentMarkAccess::const_iterator_t ppMarksEnd)
    {
        return find_if(
            ppMarksBegin,
            ppMarksEnd,
            boost::bind(&OUString::equals, boost::bind(&IMark::GetName, _1), rName));
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
            OString sName = OUStringToOString(pMark->GetName(), RTL_TEXTENCODING_UTF8);
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
    else if(*pMarkTypeInfo == typeid(AnnotationMark))
        return ANNOTATIONMARK;
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

OUString IDocumentMarkAccess::GetCrossRefHeadingBookmarkNamePrefix()
{
    return OUString("__RefHeading__");
}

bool IDocumentMarkAccess::IsLegalPaMForCrossRefHeadingBookmark( const SwPaM& rPaM )
{
    return rPaM.Start()->nNode.GetNode().IsTxtNode() &&
           rPaM.Start()->nContent.GetIndex() == 0 &&
           ( !rPaM.HasMark() ||
             ( rPaM.GetMark()->nNode == rPaM.GetPoint()->nNode &&
               rPaM.End()->nContent.GetIndex() == rPaM.End()->nNode.GetNode().GetTxtNode()->Len() ) );
}

namespace sw { namespace mark
{
    MarkManager::MarkManager(SwDoc& rDoc)
        : m_vAllMarks()
        , m_vBookmarks()
        , m_vFieldmarks()
        , m_vAnnotationMarks()
        , m_pDoc(&rDoc)
    { }

    ::sw::mark::IMark* MarkManager::makeMark(const SwPaM& rPaM,
        const OUString& rName,
        const IDocumentMarkAccess::MarkType eType)
    {
#if 0
        {
            OString sName = OUStringToOString(rName, RTL_TEXTENCODING_UTF8);
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
        OSL_PRECOND(m_vAllMarks.size() < USHRT_MAX,
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
                pMark = boost::shared_ptr<IMark>(new Bookmark(rPaM, KeyCode(), rName, OUString()));
                break;
            case IDocumentMarkAccess::DDE_BOOKMARK:
                pMark = boost::shared_ptr<IMark>(new DdeBookmark(rPaM));
                break;
            case IDocumentMarkAccess::CROSSREF_HEADING_BOOKMARK:
                pMark = boost::shared_ptr<IMark>(new CrossRefHeadingBookmark(rPaM, KeyCode(), rName, OUString()));
                break;
            case IDocumentMarkAccess::CROSSREF_NUMITEM_BOOKMARK:
                pMark = boost::shared_ptr<IMark>(new CrossRefNumItemBookmark(rPaM, KeyCode(), rName, OUString()));
                break;
            case IDocumentMarkAccess::UNO_BOOKMARK:
                pMark = boost::shared_ptr<IMark>(new UnoMark(rPaM));
                break;
            case IDocumentMarkAccess::ANNOTATIONMARK:
                pMark = boost::shared_ptr<IMark>(new AnnotationMark( rPaM, rName ));
                break;
        }
        OSL_ENSURE(pMark.get(),
            "MarkManager::makeMark(..)"
            " - Mark was not created.");
        MarkBase* pMarkBase = dynamic_cast<MarkBase*>(pMark.get());

        if (!pMarkBase)
            return 0;

        if(pMark->GetMarkPos() != pMark->GetMarkStart())
            pMarkBase->Swap();

        // for performance reasons, we trust UnoMarks to have a (generated) unique name
        if ( eType != IDocumentMarkAccess::UNO_BOOKMARK )
            pMarkBase->SetName( getUniqueMarkName( pMarkBase->GetName() ) );

        // register mark
        m_aMarkNamesSet.insert(pMarkBase->GetName());
        lcl_InsertMarkSorted(m_vAllMarks, pMark);
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
            case IDocumentMarkAccess::ANNOTATIONMARK:
                lcl_InsertMarkSorted( m_vAnnotationMarks, pMark );
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
        lcl_DebugMarks(m_vAllMarks);
        OSL_TRACE("Bookmarks");
        lcl_DebugMarks(m_vBookmarks);
        OSL_TRACE("Fieldmarks");
        lcl_DebugMarks(m_vFieldmarks);
#endif

        return pMark.get();
    }

    ::sw::mark::IFieldmark* MarkManager::makeFieldBookmark(
        const SwPaM& rPaM,
        const OUString& rName,
        const OUString& rType )
    {
        sw::mark::IMark* pMark = makeMark( rPaM, rName,
                IDocumentMarkAccess::TEXT_FIELDMARK );
        sw::mark::IFieldmark* pFieldMark = dynamic_cast<sw::mark::IFieldmark*>( pMark );
        if (pFieldMark)
            pFieldMark->SetFieldname( rType );

        return pFieldMark;
    }

    ::sw::mark::IFieldmark* MarkManager::makeNoTextFieldBookmark(
        const SwPaM& rPaM,
        const OUString& rName,
        const OUString& rType)
    {
        sw::mark::IMark* pMark = makeMark( rPaM, rName,
                IDocumentMarkAccess::CHECKBOX_FIELDMARK );
        sw::mark::IFieldmark* pFieldMark = dynamic_cast<sw::mark::IFieldmark*>( pMark );
        if (pFieldMark)
            pFieldMark->SetFieldname( rType );

        return pFieldMark;
    }

    ::sw::mark::IMark* MarkManager::getMarkForTxtNode(
        const SwTxtNode& rTxtNode,
        const IDocumentMarkAccess::MarkType eType )
    {
        SwPosition aPos(rTxtNode);
        aPos.nContent.Assign(&(const_cast<SwTxtNode&>(rTxtNode)), 0);
        const iterator_t ppExistingMark = lcl_FindMarkAtPos(m_vBookmarks, aPos, eType);
        if(ppExistingMark != m_vBookmarks.end())
            return ppExistingMark->get();
        const SwPaM aPaM(aPos);
        return makeMark(aPaM, OUString(), eType);
    }

    sw::mark::IMark* MarkManager::makeAnnotationMark(
        const SwPaM& rPaM,
        const ::rtl::OUString& rName )
    {
        return makeMark( rPaM, rName, IDocumentMarkAccess::ANNOTATIONMARK );
    }

    void MarkManager::repositionMark(
        ::sw::mark::IMark* const io_pMark,
        const SwPaM& rPaM)
    {
        OSL_PRECOND(io_pMark->GetMarkPos().GetDoc() == m_pDoc,
            "<MarkManager::repositionMark(..)>"
            " - Mark is not in my doc.");
        MarkBase* const pMarkBase = dynamic_cast< MarkBase* >(io_pMark);
        if (!pMarkBase)
            return;

        pMarkBase->SetMarkPos(*(rPaM.GetPoint()));
        if(rPaM.HasMark())
            pMarkBase->SetOtherMarkPos(*(rPaM.GetMark()));
        else
            pMarkBase->ClearOtherMarkPos();

        if(pMarkBase->GetMarkPos() != pMarkBase->GetMarkStart())
            pMarkBase->Swap();

        sortMarks();
    }

    bool MarkManager::renameMark(
        ::sw::mark::IMark* io_pMark,
        const OUString& rNewName )
    {
        OSL_PRECOND(io_pMark->GetMarkPos().GetDoc() == m_pDoc,
            "<MarkManager::renameMark(..)>"
            " - Mark is not in my doc.");
        if ( io_pMark->GetName() == rNewName )
            return true;
        if ( findMark(rNewName) != m_vAllMarks.end() )
            return false;
        if (::sw::mark::MarkBase* pMarkBase = dynamic_cast< ::sw::mark::MarkBase* >(io_pMark))
        {
            m_aMarkNamesSet.erase(pMarkBase->GetName());
            m_aMarkNamesSet.insert(rNewName);
            pMarkBase->SetName(rNewName);
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

        for(iterator_t ppMark = m_vAllMarks.begin();
            ppMark != m_vAllMarks.end();
            ++ppMark)
        {
            ::sw::mark::MarkBase* pMark = dynamic_cast< ::sw::mark::MarkBase* >(ppMark->get());
            if (!pMark)
                continue;
            // is on position ??
            bool bChangedPos = false;
            if(&pMark->GetMarkPos().nNode.GetNode() == pOldNode)
            {
                pMark->SetMarkPos(aNewPos);
                bChangedPos = true;
            }
            bool bChangedOPos = false;
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
        lcl_DebugMarks(m_vAllMarks);
#endif
    }

    void MarkManager::correctMarksRelative(const SwNodeIndex& rOldNode, const SwPosition& rNewPos, const sal_Int32 nOffset)
    {
        const SwNode* const pOldNode = &rOldNode.GetNode();
        SwPosition aNewPos(rNewPos);
        aNewPos.nContent += nOffset;
        bool isSortingNeeded = false;

        for(iterator_t ppMark = m_vAllMarks.begin();
            ppMark != m_vAllMarks.end();
            ++ppMark)
        {
            // is on position ??
            bool bChangedPos = false, bChangedOPos = false;
            ::sw::mark::MarkBase* const pMark = dynamic_cast< ::sw::mark::MarkBase* >(ppMark->get());
            if (!pMark)
                continue;
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
        lcl_DebugMarks(m_vAllMarks);
#endif
    }

    void MarkManager::deleteMarks(
            const SwNodeIndex& rStt,
            const SwNodeIndex& rEnd,
            ::std::vector<SaveBookmark>* pSaveBkmk,
            const SwIndex* pSttIdx,
            const SwIndex* pEndIdx )
    {
        ::std::vector<const_iterator_t> vMarksToDelete;
        bool bIsSortingNeeded = false;

        // boolean indicating, if at least one mark has been moved while collecting marks for deletion
        bool bMarksMoved = false;

        // copy all bookmarks in the move area to a vector storing all position data as offset
        // reassignment is performed after the move
        for(iterator_t ppMark = m_vAllMarks.begin();
            ppMark != m_vAllMarks.end();
            ++ppMark)
        {
            // navigator marks should not be moved
            // TODO: Check if this might make them invalid
            if(IDocumentMarkAccess::GetType(**ppMark) == NAVIGATOR_REMINDER)
                continue;

            ::sw::mark::MarkBase* pMark = dynamic_cast< ::sw::mark::MarkBase* >(ppMark->get());

            if (!pMark)
                continue;

            // on position ??
            bool bIsPosInRange = lcl_GreaterThan(pMark->GetMarkPos(), rStt, pSttIdx)
                                 && lcl_Lower(pMark->GetMarkPos(), rEnd, pEndIdx);
            bool bIsOtherPosInRange = pMark->IsExpanded()
                                      && lcl_GreaterThan(pMark->GetOtherMarkPos(), rStt, pSttIdx)
                                      && lcl_Lower(pMark->GetOtherMarkPos(), rEnd, pEndIdx);
            // special case: completely in range, touching the end?
            if ( pEndIdx != NULL
                 && ( ( bIsOtherPosInRange
                        && pMark->GetMarkPos().nNode == rEnd
                        && pMark->GetMarkPos().nContent == *pEndIdx )
                      || ( bIsPosInRange
                           && pMark->IsExpanded()
                           && pMark->GetOtherMarkPos().nNode == rEnd
                           && pMark->GetOtherMarkPos().nContent == *pEndIdx ) ) )
            {
                bIsPosInRange = true, bIsOtherPosInRange = true;
            }

            if ( bIsPosInRange
                 && ( bIsOtherPosInRange
                      || !pMark->IsExpanded() ) )
            {
                // completely in range

                bool bDeleteMark = true;
                {
                    switch ( IDocumentMarkAccess::GetType( *pMark ) )
                    {
                    case IDocumentMarkAccess::CROSSREF_HEADING_BOOKMARK:
                    case IDocumentMarkAccess::CROSSREF_NUMITEM_BOOKMARK:
                        // no delete of cross-reference bookmarks, if range is inside one paragraph
                        bDeleteMark = rStt != rEnd;
                        break;
                    case IDocumentMarkAccess::UNO_BOOKMARK:
                        // no delete of UNO mark, if it is not expanded and only touches the start of the range
                        bDeleteMark = bIsOtherPosInRange
                                      || pMark->IsExpanded()
                                      || pSttIdx == NULL
                                      || !( pMark->GetMarkPos().nNode == rStt
                                            && pMark->GetMarkPos().nContent == *pSttIdx );
                        break;
                    default:
                        bDeleteMark = true;
                        break;
                    }
                }

                if ( bDeleteMark )
                {
                    if ( pSaveBkmk )
                    {
                        pSaveBkmk->push_back( SaveBookmark( true, true, *pMark, rStt, pSttIdx ) );
                    }
                    vMarksToDelete.push_back(ppMark);
                }
            }
            else if ( bIsPosInRange != bIsOtherPosInRange )
            {
                // the bookmark is partially in the range
                // move position of that is in the range out of it

                SAL_WNODEPRECATED_DECLARATIONS_PUSH
                ::std::auto_ptr< SwPosition > pNewPos;
                {
                    if ( pEndIdx != NULL )
                    {
                        pNewPos = ::std::auto_ptr< SwPosition >( new SwPosition( rEnd, *pEndIdx ) );
                    }
                    else
                    {
                        pNewPos =
                            lcl_FindExpelPosition( rStt, rEnd, bIsPosInRange ? pMark->GetOtherMarkPos() : pMark->GetMarkPos() );
                    }
                }

                bool bMoveMark = true;
                {
                    switch ( IDocumentMarkAccess::GetType( *pMark ) )
                    {
                    case IDocumentMarkAccess::CROSSREF_HEADING_BOOKMARK:
                    case IDocumentMarkAccess::CROSSREF_NUMITEM_BOOKMARK:
                        // no move of cross-reference bookmarks, if move occurs inside a certain node
                        bMoveMark = pMark->GetMarkPos().nNode != pNewPos->nNode;
                        break;
                    case IDocumentMarkAccess::ANNOTATIONMARK:
                        // no move of annotation marks, if method is called to collect deleted marks
                        bMoveMark = pSaveBkmk == NULL;
                        break;
                    default:
                        bMoveMark = true;
                        break;
                    }
                }
                SAL_WNODEPRECATED_DECLARATIONS_POP
                if ( bMoveMark )
                {
                    if ( bIsPosInRange )
                        pMark->SetMarkPos(*pNewPos);
                    else
                        pMark->SetOtherMarkPos(*pNewPos);
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
            std::vector< ::boost::shared_ptr<ILazyDeleter> > vDelay;
            vDelay.reserve(vMarksToDelete.size());

            // If needed, sort mark containers containing subsets of the marks
            // in order to assure sorting.  The sorting is critical for the
            // deletion of a mark as it is searched in these container for
            // deletion.
            if ( vMarksToDelete.size() > 0 && bMarksMoved )
            {
                sortSubsetMarks();
            }
            // we just remembered the iterators to delete, so we do not need to search
            // for the shared_ptr<> (the entry in m_vAllMarks) again
            // reverse iteration, since erasing an entry invalidates iterators
            // behind it (the iterators in vMarksToDelete are sorted)
            for ( ::std::vector< const_iterator_t >::reverse_iterator pppMark = vMarksToDelete.rbegin();
                  pppMark != vMarksToDelete.rend();
                  ++pppMark )
            {
                vDelay.push_back(deleteMark(*pppMark));
            }
        } // scope to kill vDelay

        if ( bIsSortingNeeded )
        {
            sortMarks();
        }

#if 0
        OSL_TRACE("deleteMarks");
        lcl_DebugMarks(m_vAllMarks);
#endif
    }

    struct LazyFieldmarkDeleter : public IDocumentMarkAccess::ILazyDeleter
    {
        ::boost::shared_ptr<IMark> const m_pFieldmark;
        SwDoc *const m_pDoc;
        LazyFieldmarkDeleter(
                ::boost::shared_ptr<IMark> const& pMark, SwDoc *const pDoc)
            : m_pFieldmark(pMark), m_pDoc(pDoc)
        { }
        virtual ~LazyFieldmarkDeleter()
        {
            dynamic_cast<Fieldmark&>(*m_pFieldmark.get()).ReleaseDoc(m_pDoc);
        }
    };

    ::boost::shared_ptr<IDocumentMarkAccess::ILazyDeleter>
        MarkManager::deleteMark(const const_iterator_t ppMark)
    {
        ::boost::shared_ptr<ILazyDeleter> ret;
        if (ppMark == m_vAllMarks.end()) return ret;

        switch(IDocumentMarkAccess::GetType(**ppMark))
        {
            case IDocumentMarkAccess::BOOKMARK:
            case IDocumentMarkAccess::CROSSREF_HEADING_BOOKMARK:
            case IDocumentMarkAccess::CROSSREF_NUMITEM_BOOKMARK:
                {
                    IDocumentMarkAccess::iterator_t ppBookmark = lcl_FindMark(m_vBookmarks, *ppMark);
                    if ( ppBookmark != m_vBookmarks.end() )
                    {
                        m_vBookmarks.erase(ppBookmark);
                    }
                    else
                    {
                        OSL_ENSURE( false, "<MarkManager::deleteMark(..)> - Bookmark not found in Bookmark container.");
                    }
                }
                break;

            case IDocumentMarkAccess::TEXT_FIELDMARK:
            case IDocumentMarkAccess::CHECKBOX_FIELDMARK:
                {
                    IDocumentMarkAccess::iterator_t ppFieldmark = lcl_FindMark(m_vFieldmarks, *ppMark);
                    if ( ppFieldmark != m_vFieldmarks.end() )
                    {
                        m_vFieldmarks.erase(ppFieldmark);
                        ret.reset(new LazyFieldmarkDeleter(*ppMark, m_pDoc));
                    }
                    else
                    {
                        OSL_ENSURE( false, "<MarkManager::deleteMark(..)> - Fieldmark not found in Fieldmark container.");
                    }
                }
                break;

            case IDocumentMarkAccess::ANNOTATIONMARK:
                {
                    IDocumentMarkAccess::iterator_t ppAnnotationMark = lcl_FindMark(m_vAnnotationMarks, *ppMark);
                    if ( ppAnnotationMark != m_vAnnotationMarks.end() )
                    {
                        m_vAnnotationMarks.erase(ppAnnotationMark);
                    }
                    else
                    {
                        OSL_ENSURE( false, "<MarkManager::deleteMark(..)> - Annotation Mark not found in Annotation Mark container.");
                    }
                }
                break;

            case IDocumentMarkAccess::NAVIGATOR_REMINDER:
            case IDocumentMarkAccess::DDE_BOOKMARK:
            case IDocumentMarkAccess::UNO_BOOKMARK:
                // no special marks container
                break;
        }
        DdeBookmark* const pDdeBookmark = dynamic_cast<DdeBookmark*>(ppMark->get());
        if(pDdeBookmark)
            pDdeBookmark->DeregisterFromDoc(m_pDoc);
        //Effective STL Item 27, get a non-const iterator aI at the same
        //position as const iterator ppMark was
        iterator_t aI = m_vAllMarks.begin();
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
        m_vAllMarks.erase(aI);
        return ret;
    }

    void MarkManager::deleteMark(const IMark* const pMark)
    {
        OSL_PRECOND(pMark->GetMarkPos().GetDoc() == m_pDoc,
            "<MarkManager::deleteMark(..)>"
            " - Mark is not in my doc.");
        // finds the last Mark that is starting before pMark
        // (pMarkLow < pMark)
        iterator_t pMarkLow =
            lower_bound(
                m_vAllMarks.begin(),
                m_vAllMarks.end(),
                pMark->GetMarkStart(),
                sw::mark::CompareIMarkStartsBefore());
        iterator_t pMarkHigh = m_vAllMarks.end();
        iterator_t pMarkFound =
            find_if(
                pMarkLow,
                pMarkHigh,
                boost::bind( ::std::equal_to<const IMark*>(), boost::bind(&boost::shared_ptr<IMark>::get, _1), pMark ) );
        if(pMarkFound != pMarkHigh)
            deleteMark(pMarkFound);
    }

    void MarkManager::clearAllMarks()
    {
        m_vFieldmarks.clear();
        m_vBookmarks.clear();
        m_aMarkNamesSet.clear();

        m_vAnnotationMarks.clear();

#if OSL_DEBUG_LEVEL > 0
        for(iterator_t pBkmk = m_vAllMarks.begin();
            pBkmk != m_vAllMarks.end();
            ++pBkmk)
            OSL_ENSURE( pBkmk->unique(),
                        "<MarkManager::clearAllMarks(..)> - a Bookmark is still in use.");
#endif
        m_vAllMarks.clear();
    }

    IDocumentMarkAccess::const_iterator_t MarkManager::findMark(const OUString& rName) const
    {
        return lcl_FindMarkByName(rName, m_vAllMarks.begin(), m_vAllMarks.end());
    }

    IDocumentMarkAccess::const_iterator_t MarkManager::findBookmark(const OUString& rName) const
    {
        return lcl_FindMarkByName(rName, m_vBookmarks.begin(), m_vBookmarks.end());
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

    IFieldmark* MarkManager::getFieldmarkFor(const SwPosition& rPos) const
    {
        const_iterator_t pFieldmark = find_if(
            m_vFieldmarks.begin(),
            m_vFieldmarks.end( ),
            boost::bind(&IMark::IsCoveringPosition, _1, rPos));
        if(pFieldmark == m_vFieldmarks.end()) return NULL;
        return dynamic_cast<IFieldmark*>(pFieldmark->get());
    }

    IFieldmark* MarkManager::getDropDownFor(const SwPosition& rPos) const
    {
        IFieldmark *pMark = getFieldmarkFor(rPos);
        if (!pMark || pMark->GetFieldname() != ODF_FORMDROPDOWN)
            return NULL;
        return pMark;
    }

    std::vector<IFieldmark*> MarkManager::getDropDownsFor(const SwPaM &rPaM) const
    {
        std::vector<IFieldmark*> aRet;

        for (IDocumentMarkAccess::const_iterator_t aI = m_vFieldmarks.begin(),
            aEnd = m_vFieldmarks.end(); aI != aEnd; ++aI)
        {
            boost::shared_ptr<IMark> xI = *aI;
            const SwPosition &rStart = xI->GetMarkPos();
            if (!rPaM.ContainsPosition(rStart))
                continue;

            IFieldmark *pMark = dynamic_cast<IFieldmark*>(xI.get());
            if (!pMark || pMark->GetFieldname() != ODF_FORMDROPDOWN)
                continue;

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

    IDocumentMarkAccess::const_iterator_t MarkManager::findAnnotationMark( const ::rtl::OUString& rName ) const
    {
        return lcl_FindMarkByName( rName, m_vAnnotationMarks.begin(), m_vAnnotationMarks.end() );
    }

    OUString MarkManager::getUniqueMarkName(const OUString& rName) const
    {
        OSL_ENSURE(rName.getLength(),
            "<MarkManager::getUniqueMarkName(..)> - a name should be proposed");
        if ( findMark(rName) == getAllMarksEnd() )
        {
            return rName;
        }
        OUStringBuffer sBuf;
        OUString sTmp;

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
            if ( findMark(sTmp) == getAllMarksEnd() )
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
        sort(m_vBookmarks.begin(), m_vBookmarks.end(), &lcl_MarkOrderingByStart);
        sort(m_vFieldmarks.begin(), m_vFieldmarks.end(), &lcl_MarkOrderingByStart);
        sort(m_vAnnotationMarks.begin(), m_vAnnotationMarks.end(), &lcl_MarkOrderingByStart);
    }

    void MarkManager::sortMarks()
    {
        sort(m_vAllMarks.begin(), m_vAllMarks.end(), &lcl_MarkOrderingByStart);
        sortSubsetMarks();
    }

    bool MarkManager::hasMark(const OUString& rName) const
    {
        return (m_aMarkNamesSet.find(rName) != m_aMarkNamesSet.end());
    }

}} // namespace ::sw::mark

#define PCURSH ((SwCrsrShell*)_pStartShell)
#define FOREACHSHELL_START( pEShell ) \
    {\
        SwViewShell *_pStartShell = pEShell; \
        do { \
            if( _pStartShell->IsA( TYPE( SwCrsrShell )) ) \
            {

#define FOREACHSHELL_END( pEShell ) \
            } \
        } while((_pStartShell=(SwViewShell*)_pStartShell->GetNext())!= pEShell ); \
    }

namespace
{
    // Array structure: 2 longs,
    //  1st long contains the type and position in the DocArray,
    //  2nd long contains the ContentPosition

    //  CntntType --
    //          0x2000 = Paragraph anchored frame
    //          0x2001 = frame anchored at character, which should be moved
    //          0x0800 = Crsr from the CrsrShell Mark
    //          0x0801 = Crsr from the CrsrShell Point

    class _SwSaveTypeCountContent
    {
        union {
            struct { sal_uInt16 nType, nCount; } TC;
            sal_uLong nTypeCount;
            } TYPECOUNT;
        sal_Int32 nContent;

    public:
        _SwSaveTypeCountContent() { TYPECOUNT.nTypeCount = 0; nContent = 0; }
        _SwSaveTypeCountContent( const std::vector<sal_uLong> &rArr, sal_uInt16& rPos )
            {
                TYPECOUNT.nTypeCount = rArr[ rPos++ ];
                nContent = static_cast<sal_Int32>(rArr[ rPos++ ]);
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

        void SetContent( sal_Int32 n )     { nContent = n; }
        sal_Int32 GetContent() const       { return nContent; }
#if OSL_DEBUG_LEVEL > 0
        void Dump()
        {
            SAL_INFO("sw.core", "Count: " << GetCount() << "\tType: " << GetType() << "\tContent: " << GetContent());
        }
#endif

    };

    // #i59534: If a paragraph will be splitted we have to restore some redline positions
    // This help function checks a position compared with a node and an content index

    static const int BEFORE_NODE = 0;          // Position before the given node index
    static const int BEFORE_SAME_NODE = 1;     // Same node index but content index before given content index
    static const int SAME_POSITION = 2;        // Same node index and samecontent index
    static const int BEHIND_SAME_NODE = 3;     // Same node index but content index behind given content index
    static const int BEHIND_NODE = 4;          // Position behind the given node index

    static int lcl_RelativePosition( const SwPosition& rPos, sal_uLong nNode, sal_Int32 nCntnt )
    {
        sal_uLong nIndex = rPos.nNode.GetIndex();
        int nReturn = BEFORE_NODE;
        if( nIndex == nNode )
        {
            const sal_Int32 nCntIdx = rPos.nContent.GetIndex();
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

    static inline bool lcl_Greater( const SwPosition& rPos, const SwNodeIndex& rNdIdx, const SwIndex* pIdx )
    {
        return rPos.nNode > rNdIdx || ( pIdx && rPos.nNode == rNdIdx && rPos.nContent > pIdx->GetIndex() );
    }

    static void lcl_ChkPaM( std::vector<sal_uLong> &rSaveArr, sal_uLong nNode, sal_Int32 nCntnt,
                    const SwPaM& rPam, _SwSaveTypeCountContent& rSave,
                    bool bChkSelDirection )
    {
        // Respect direction of selection
        bool bBound1IsStart = !bChkSelDirection ||
                            ( *rPam.GetPoint() < *rPam.GetMark()
                                ? rPam.GetPoint() == &rPam.GetBound()
                                : rPam.GetMark() == &rPam.GetBound());

        const SwPosition* pPos = &rPam.GetBound( true );
        if( pPos->nNode.GetIndex() == nNode &&
            ( bBound1IsStart ? pPos->nContent.GetIndex() < nCntnt
                                : pPos->nContent.GetIndex() <= nCntnt ))
        {
            rSave.SetContent( pPos->nContent.GetIndex() );
            rSave.Add( rSaveArr );
        }

        pPos = &rPam.GetBound( false );
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
#if 0
    static void DumpSaves(std::vector<sal_uLong> &rSaveArr)
    {
        sal_uInt16 n = 0;
        while( n < rSaveArr.size() )
        {
            _SwSaveTypeCountContent aSave( rSaveArr, n );
            aSave.Dump();
        }
    }
#endif
}

// IDocumentMarkAccess for SwDoc
IDocumentMarkAccess* SwDoc::getIDocumentMarkAccess()
    { return static_cast< IDocumentMarkAccess* >(mpMarkManager.get()); }

const IDocumentMarkAccess* SwDoc::getIDocumentMarkAccess() const
    { return static_cast< IDocumentMarkAccess* >(mpMarkManager.get()); }

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
    {
        m_nNode2 = ULONG_MAX;
        m_nCntnt2 = -1;
    }
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
                aPam.GetMark()->nContent.Assign(aPam.GetCntntNode(false), m_nCntnt2);
        }
        else
        {
            aPam.GetMark()->nNode = m_nNode2;
            aPam.GetMark()->nContent.Assign(aPam.GetCntntNode(false), m_nCntnt2);
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
        || CheckNodesRange(aPam.GetPoint()->nNode, aPam.GetMark()->nNode, true))
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
        SwRangeRedline* pRedl = rTbl[ nCnt ];

        SwPosition *pRStt = &pRedl->GetBound(true),
                   *pREnd = &pRedl->GetBound(false);
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
                bool bStt = true;
                SwCntntNode* pCNd = pRStt->nNode.GetNode().GetCntntNode();
                if( !pCNd && 0 == ( pCNd = pDoc->GetNodes().GoNext( &pRStt->nNode )) )
                {
                    bStt = false;
                    pRStt->nNode = rStt;
                    if( 0 == ( pCNd = pDoc->GetNodes().GoPrevious( &pRStt->nNode )) )
                    {
                        pRStt->nNode = pREnd->nNode;
                        pCNd = pRStt->nNode.GetNode().GetCntntNode();
                    }
                }
                pRStt->nContent.Assign( pCNd, bStt ? 0 : pCNd->Len() );
            }
        }
        if( lcl_Greater( *pREnd, rStt, pSttIdx ) && lcl_Lower( *pREnd, rEnd, pEndIdx ))
        {
            pREnd->nNode = rStt;
            if( pSttIdx )
                pREnd->nContent = *pSttIdx;
            else
            {
                bool bStt = false;
                SwCntntNode* pCNd = pREnd->nNode.GetNode().GetCntntNode();
                if( !pCNd && 0 == ( pCNd = pDoc->GetNodes().GoPrevious( &pREnd->nNode )) )
                {
                    bStt = true;
                    pREnd->nNode = rEnd;
                    if( 0 == ( pCNd = pDoc->GetNodes().GoNext( &pREnd->nNode )) )
                    {
                        pREnd->nNode = pRStt->nNode;
                        pCNd = pREnd->nNode.GetNode().GetCntntNode();
                    }
                }
                pREnd->nContent.Assign( pCNd, bStt ? 0 : pCNd->Len() );
            }
        }
    }
}


void _SaveCntntIdx(SwDoc* pDoc,
    sal_uLong nNode,
    sal_Int32 nCntnt,
    std::vector<sal_uLong> &rSaveArr,
    sal_uInt8 nSaveFly)
{
    // 1. Bookmarks
    _SwSaveTypeCountContent aSave;
    // 4. Paragraph anchored objects
    {
        SwCntntNode *pNode = pDoc->GetNodes()[nNode]->GetCntntNode();
        if( pNode )
        {

            SwFrm* pFrm = pNode->getLayoutFrm( pDoc->GetCurrentLayout() );
#if OSL_DEBUG_LEVEL > 1
            static bool bViaDoc = false;
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
//    {
//        SwCrsrShell* pShell = pDoc->GetEditShell();
//        if( pShell )
//        {
//            aSave.SetTypeAndCount( 0x800, 0 );
//            FOREACHSHELL_START( pShell )
//                SwPaM *_pStkCrsr = PCURSH->GetStkCrsr();
//                if( _pStkCrsr )
//                    do {
//                        lcl_ChkPaM( rSaveArr, nNode, nCntnt, *_pStkCrsr,
//                                    aSave, false );
//                        aSave.IncCount();
//                    } while ( (_pStkCrsr != 0 ) &&
//                        ((_pStkCrsr=(SwPaM *)_pStkCrsr->GetNext()) != PCURSH->GetStkCrsr()) );
//
//                FOREACHPAM_START( PCURSH->_GetCrsr() )
//                    lcl_ChkPaM( rSaveArr, nNode, nCntnt, *PCURCRSR,
//                                aSave, false );
//                    aSave.IncCount();
//                FOREACHPAM_END()
//
//            FOREACHSHELL_END( pShell )
//        }
//    }
}

void _RestoreCntntIdx(SwDoc* pDoc,
    std::vector<sal_uLong> &rSaveArr,
    sal_uLong nNode,
    sal_Int32 nOffset,
    bool bAuto)
{
    SwCntntNode* pCNd = pDoc->GetNodes()[ nNode ]->GetCntntNode();
    SwFrmFmts* pSpz = pDoc->GetSpzFrmFmts();
    sal_uInt16 n = 0;
    while( n < rSaveArr.size() )
    {
        _SwSaveTypeCountContent aSave( rSaveArr, n );
        SwPosition* pPos = 0;
        switch( aSave.GetType() )
        {
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
        }

        if( pPos )
        {
#if OSL_DEBUG_LEVEL > 0
            aSave.Dump();
#endif
            SAL_INFO("sw.core", "setting " << pPos << " for Index " << aSave.GetCount() << " on Node " << nNode << " from " << pPos->nContent.GetIndex() << " to " << (aSave.GetContent() + nOffset));
            pPos->nNode = *pCNd;
            pPos->nContent.Assign( pCNd, aSave.GetContent() + nOffset );
        }
    }
}

void _RestoreCntntIdx(std::vector<sal_uLong> &rSaveArr,
    const SwNode& rNd,
    sal_Int32 nLen,
    sal_Int32 nChkLen)
{
    const SwDoc* pDoc = rNd.GetDoc();
    const SwFrmFmts* pSpz = pDoc->GetSpzFrmFmts();
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
                            aNewPos.nContent.Assign( pCNd, std::min(
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
            }

            if( pPos )
            {
                pPos->nNode = rNd;
                pPos->nContent.Assign( pCNd, std::min( aSave.GetContent(), nLen ) );
            }
            n -= 2;
            rSaveArr.erase( rSaveArr.begin() + n, rSaveArr.begin() + n + 2);
        }
    }
}

namespace
{
    struct MarkEntry
    {
        long int m_nIdx;
        bool m_bOther;
        sal_Int32 m_nCntnt;
#if 0
        void Dump()
        {
            SAL_INFO("sw.core", "Index: " << m_nIdx << "\tOther: " << m_bOther << "\tContent: " << m_nCntnt);
        }
#endif
    };
    struct PaMEntry
    {
        SwPaM* m_pPaM;
        bool m_isMark;
        sal_Int32 m_nCntnt;
    };
    struct OffsetUpdater
    {
        const SwCntntNode* m_pNewCntntNode;
        const sal_Int32 m_nOffset;
        OffsetUpdater(SwCntntNode* pNewCntntNode, sal_Int32 nOffset)
            : m_pNewCntntNode(pNewCntntNode), m_nOffset(nOffset) {};
        void operator()(SwPosition& rPos, sal_Int32 nCntnt) const
        {
            rPos.nNode = *m_pNewCntntNode;
            rPos.nContent.Assign(const_cast<SwCntntNode*>(m_pNewCntntNode), nCntnt + m_nOffset);
        };
    };
    struct LimitUpdater
    {
        const SwCntntNode* m_pNewCntntNode;
        const sal_uLong m_nLen;
        const sal_Int32 m_nCorrLen;
        LimitUpdater(SwCntntNode* pNewCntntNode, sal_uLong nLen, sal_Int32 nCorrLen)
            : m_pNewCntntNode(pNewCntntNode), m_nLen(nLen), m_nCorrLen(nCorrLen) {};
        void operator()(SwPosition& rPos, sal_Int32 nCntnt) const
        {
            if( nCntnt < m_nCorrLen )
            {
                rPos.nNode = *m_pNewCntntNode;
                rPos.nContent.Assign(const_cast<SwCntntNode*>(m_pNewCntntNode), std::min( nCntnt, static_cast<sal_Int32>(m_nLen) ) );
            }
        };
    };
    struct CntntIdxStoreImpl : sw::mark::CntntIdxStore
    {
        std::vector<MarkEntry> m_aBkmkEntries;
        std::vector<MarkEntry> m_aRedlineEntries;
        std::vector<MarkEntry> m_aUnoCrsrEntries;
        std::vector<PaMEntry> m_aShellCrsrEntries;
        std::vector<sal_uLong> m_aSaveArr;
        typedef boost::function<void (SwPosition& rPos, sal_Int32 nCntnt)> updater_t;
        virtual void Clear() SAL_OVERRIDE
        {
            m_aBkmkEntries.clear();
            m_aRedlineEntries.clear();
            m_aUnoCrsrEntries.clear();
            m_aShellCrsrEntries.clear();
            m_aSaveArr.clear();
        }
        virtual bool Empty() SAL_OVERRIDE
        {
            return m_aBkmkEntries.empty() && m_aRedlineEntries.empty() && m_aUnoCrsrEntries.empty() && m_aShellCrsrEntries.empty() && m_aSaveArr.empty();
        }
        virtual void Save(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nCntnt, sal_uInt8 nSaveFly=0) SAL_OVERRIDE
        {
            SaveBkmks(pDoc, nNode, nCntnt);
            SaveRedlines(pDoc, nNode, nCntnt);
            SaveUnoCrsrs(pDoc, nNode, nCntnt);
            SaveShellCrsrs(pDoc, nNode, nCntnt);
            return _SaveCntntIdx(pDoc, nNode, nCntnt, m_aSaveArr, nSaveFly);
        }
        virtual void Restore(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nOffset=0, bool bAuto = false) SAL_OVERRIDE
        {
            SwCntntNode* pCNd = pDoc->GetNodes()[ nNode ]->GetCntntNode();
            updater_t aUpdater = OffsetUpdater(pCNd, nOffset);
            RestoreBkmks(pDoc, aUpdater);
            RestoreRedlines(pDoc, aUpdater);
            _RestoreCntntIdx(pDoc, m_aSaveArr, nNode, nOffset, bAuto);
            RestoreUnoCrsrs(pDoc, aUpdater);
            RestoreShellCrsrs(pDoc, aUpdater);
        }
        virtual void Restore(SwNode& rNd, sal_Int32 nLen, sal_Int32 nCorrLen) SAL_OVERRIDE
        {
            SwCntntNode* pCNd = (SwCntntNode*)rNd.GetCntntNode();
            SwDoc* pDoc = rNd.GetDoc();
            updater_t aUpdater = LimitUpdater(pCNd, nLen, nCorrLen);
            RestoreBkmks(pDoc, aUpdater);
            RestoreRedlines(pDoc, aUpdater);
            _RestoreCntntIdx(m_aSaveArr, rNd, nLen, nCorrLen);
            RestoreUnoCrsrs(pDoc, aUpdater);
            RestoreShellCrsrs(pDoc, aUpdater);
        }
        virtual ~CntntIdxStoreImpl(){};
        private:
            inline void SaveBkmks(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nCntnt);
            inline void RestoreBkmks(SwDoc* pDoc, updater_t& rUpdater);
            inline void SaveRedlines(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nCntnt);
            inline void RestoreRedlines(SwDoc* pDoc, updater_t& rUpdater);
            inline void SaveUnoCrsrs(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nCntnt);
            inline void RestoreUnoCrsrs(SwDoc* pDoc, updater_t& rUpdater);
            inline void SaveShellCrsrs(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nCntnt);
            inline void RestoreShellCrsrs(SwDoc* pDoc, updater_t& rUpdater);
            inline const SwPosition& GetRightMarkPos(::sw::mark::IMark* pMark, bool bOther)
                { return bOther ? pMark->GetOtherMarkPos() : pMark->GetMarkPos(); };
            inline void SetRightMarkPos(MarkBase* pMark, bool bOther, const SwPosition* const pPos)
                { bOther ? pMark->SetOtherMarkPos(*pPos) : pMark->SetMarkPos(*pPos); };
    };
    static void lcl_ChkPaM( std::vector<MarkEntry>& rMarkEntries, sal_uLong nNode, sal_Int32 nCntnt,
                    const SwPaM& rPam, const long int nIdx,
                    bool bChkSelDirection )
    {
        // Respect direction of selection
        bool bBound1IsStart = !bChkSelDirection ||
                            ( *rPam.GetPoint() < *rPam.GetMark()
                                ? rPam.GetPoint() == &rPam.GetBound()
                                : rPam.GetMark() == &rPam.GetBound());

        const SwPosition* pPos = &rPam.GetBound( true );
        if( pPos->nNode.GetIndex() == nNode &&
            ( bBound1IsStart ? pPos->nContent.GetIndex() < nCntnt
                                : pPos->nContent.GetIndex() <= nCntnt ))
        {
            const MarkEntry aEntry = { nIdx, false, pPos->nContent.GetIndex() };
            rMarkEntries.push_back(aEntry);
        }

        pPos = &rPam.GetBound( false );
        if( pPos->nNode.GetIndex() == nNode &&
            ( (bBound1IsStart && bChkSelDirection)
                        ? pPos->nContent.GetIndex() <= nCntnt
                        : pPos->nContent.GetIndex() < nCntnt ))
        {
            const MarkEntry aEntry = { nIdx, true, pPos->nContent.GetIndex() };
            rMarkEntries.push_back(aEntry);
        }
    }
#if 0
    static void DumpEntries(std::vector<MarkEntry>* pEntries)
    {
        BOOST_FOREACH(MarkEntry& aEntry, *pEntries)
            aEntry.Dump();
    }
#endif
}

void CntntIdxStoreImpl::SaveBkmks(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nCntnt)
{
    IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
    const IDocumentMarkAccess::const_iterator_t ppBkmkEnd = pMarkAccess->getAllMarksEnd();
    for(
        IDocumentMarkAccess::const_iterator_t ppBkmk = pMarkAccess->getAllMarksBegin();
        ppBkmk != ppBkmkEnd;
        ++ppBkmk)
    {
        const ::sw::mark::IMark* pBkmk = ppBkmk->get();
        bool bMarkPosEqual = false;
        if(pBkmk->GetMarkPos().nNode.GetIndex() == nNode
            && pBkmk->GetMarkPos().nContent.GetIndex() <= nCntnt)
        {
            if(pBkmk->GetMarkPos().nContent.GetIndex() < nCntnt)
            {
                const MarkEntry aEntry = { ppBkmk - pMarkAccess->getAllMarksBegin(), false, pBkmk->GetMarkPos().nContent.GetIndex() };
                m_aBkmkEntries.push_back(aEntry);
            }
            else // if a bookmark position is equal nCntnt, the other position
                bMarkPosEqual = true; // has to decide if it is added to the array
        }
        if(pBkmk->IsExpanded()
            && pBkmk->GetOtherMarkPos().nNode.GetIndex() == nNode
            && pBkmk->GetOtherMarkPos().nContent.GetIndex() <= nCntnt)
        {
            if(bMarkPosEqual)
            { // the other position is before, the (main) position is equal
                const MarkEntry aEntry = { ppBkmk - pMarkAccess->getAllMarksBegin(), false, pBkmk->GetMarkPos().nContent.GetIndex() };
                m_aBkmkEntries.push_back(aEntry);
            }
            const MarkEntry aEntry = { ppBkmk - pMarkAccess->getAllMarksBegin(), true, pBkmk->GetOtherMarkPos().nContent.GetIndex() };
            m_aBkmkEntries.push_back(aEntry);
        }
    }
}

void CntntIdxStoreImpl::RestoreBkmks(SwDoc* pDoc, updater_t& rUpdater)
{
    IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
    BOOST_FOREACH(const MarkEntry& aEntry, m_aBkmkEntries)
    {
        if (MarkBase* pMark = dynamic_cast<MarkBase*>(pMarkAccess->getAllMarksBegin()[aEntry.m_nIdx].get()))
        {
            SwPosition aNewPos(GetRightMarkPos(pMark, aEntry.m_bOther));
            rUpdater(aNewPos, aEntry.m_nCntnt);
            SetRightMarkPos(pMark, aEntry.m_bOther, &aNewPos);
        }
    }
}

void CntntIdxStoreImpl::SaveRedlines(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nCntnt)
{
    const SwRedlineTbl& rRedlTbl = pDoc->GetRedlineTbl();
    for( long int nIdx = 0 ; static_cast<unsigned long int>(nIdx) < rRedlTbl.size(); ++nIdx )
    {
        const SwRangeRedline* pRdl = rRedlTbl[ nIdx ];
        int nPointPos = lcl_RelativePosition( *pRdl->GetPoint(), nNode, nCntnt );
        int nMarkPos = pRdl->HasMark() ? lcl_RelativePosition( *pRdl->GetMark(), nNode, nCntnt ) :
                                          nPointPos;
        // #i59534: We have to store the positions inside the same node before the insert position
        // and the one at the insert position if the corresponding Point/Mark position is before
        // the insert position.
        if( nPointPos == BEFORE_SAME_NODE ||
            ( nPointPos == SAME_POSITION && nMarkPos < SAME_POSITION ) )
        {
            const MarkEntry aEntry = { nIdx, false, pRdl->GetPoint()->nContent.GetIndex() };
            m_aRedlineEntries.push_back(aEntry);
        }
        if( pRdl->HasMark() && ( nMarkPos == BEFORE_SAME_NODE ||
            ( nMarkPos == SAME_POSITION && nPointPos < SAME_POSITION ) ) )
        {
            const MarkEntry aEntry = { nIdx, true, pRdl->GetMark()->nContent.GetIndex() };
            m_aRedlineEntries.push_back(aEntry);
        }
    }
}

void CntntIdxStoreImpl::RestoreRedlines(SwDoc* pDoc, updater_t& rUpdater)
{
    const SwRedlineTbl& rRedlTbl = pDoc->GetRedlineTbl();
    SwPosition* pPos = NULL;
    BOOST_FOREACH(const MarkEntry& aEntry, m_aRedlineEntries)
    {
        pPos = (SwPosition*)( aEntry.m_bOther
            ? rRedlTbl[ aEntry.m_nIdx ]->GetMark()
            : rRedlTbl[ aEntry.m_nIdx ]->GetPoint());
        rUpdater(*pPos, aEntry.m_nCntnt);
    }
}

void CntntIdxStoreImpl::SaveUnoCrsrs(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nCntnt)
{
    long int nIdx = 0;
    BOOST_FOREACH(const SwUnoCrsr* pUnoCrsr, pDoc->GetUnoCrsrTbl())
    {
        FOREACHPAM_START( const_cast<SwUnoCrsr*>(pUnoCrsr) )
            lcl_ChkPaM( m_aUnoCrsrEntries, nNode, nCntnt, *PCURCRSR, nIdx++, false );
        FOREACHPAM_END()
        const SwUnoTableCrsr* pUnoTblCrsr = dynamic_cast<const SwUnoTableCrsr*>(pUnoCrsr);
        if( pUnoTblCrsr )
        {
            FOREACHPAM_START( &(const_cast<SwUnoTableCrsr*>(pUnoTblCrsr))->GetSelRing() )
                lcl_ChkPaM( m_aUnoCrsrEntries, nNode, nCntnt, *PCURCRSR, nIdx++, false );
            FOREACHPAM_END()
        }
    }
}

void CntntIdxStoreImpl::RestoreUnoCrsrs(SwDoc* pDoc, updater_t& rUpdater)
{
    BOOST_FOREACH(const MarkEntry& aEntry, m_aUnoCrsrEntries)
    {
        sal_uInt16 nCnt = 0;
        BOOST_FOREACH(const SwUnoCrsr* pUnoCrsr, pDoc->GetUnoCrsrTbl())
        {
            SAL_INFO("sw.core", "Looking for Index " << aEntry.m_nIdx << " now at PaM Index" << nCnt << ": " << pUnoCrsr);
            SwPosition* pPos = NULL;
            FOREACHPAM_START( const_cast<SwUnoCrsr*>(pUnoCrsr) )
                if( aEntry.m_nIdx == nCnt )
                {
                    SAL_INFO("sw.core", "Found PaM " << PCURCRSR << " for Index " << aEntry.m_nIdx);
                    pPos = &PCURCRSR->GetBound( !aEntry.m_bOther );
                    break;
                }
                else
                    SAL_INFO("sw.core", "Skipping PaM " << PCURCRSR << " for Index " << aEntry.m_nIdx);
                ++nCnt;
            FOREACHPAM_END()
            const SwUnoTableCrsr* pUnoTblCrsr = dynamic_cast<const SwUnoTableCrsr*>(pUnoCrsr);
            if( !pPos && pUnoTblCrsr )
            {
                FOREACHPAM_START( &(const_cast<SwUnoTableCrsr*>(pUnoTblCrsr))->GetSelRing() )
                    if( aEntry.m_nIdx == nCnt )
                    {
                        SAL_INFO("sw.core", "Found Table PaM " << PCURCRSR << " for Index " << aEntry.m_nIdx);
                        pPos = &PCURCRSR->GetBound( !aEntry.m_bOther );
                        break;
                    }
                    else
                        SAL_INFO("sw.core", "Skipping Table PaM " << PCURCRSR << " for Index " << aEntry.m_nIdx);
                    ++nCnt;
                FOREACHPAM_END()
            }
            if( pPos )
            {
                SAL_INFO("sw.core", "Would be setting " << pPos << " for Index " << aEntry.m_nIdx);
                rUpdater(*pPos, aEntry.m_nCntnt);
                break;
            }
        }
    }
}

static void lcl_ChkPaM( std::vector<PaMEntry> &rPaMEntries, sal_uLong nNode, sal_Int32 nCntnt, SwPaM& rPaM, const bool bPoint)
{
    const SwPosition* pPos = &rPaM.GetBound( bPoint );
    if( pPos->nNode.GetIndex() == nNode && pPos->nContent.GetIndex() < nCntnt )
    {
        const PaMEntry aEntry = { &rPaM, bPoint, pPos->nContent.GetIndex() };
        rPaMEntries.push_back(aEntry);
    }
}
void CntntIdxStoreImpl::SaveShellCrsrs(SwDoc* pDoc, sal_uLong nNode, sal_Int32 nCntnt)
{
    SwCrsrShell* pShell = pDoc->GetEditShell();
    if( pShell )
    {
        FOREACHSHELL_START( pShell )
            SwPaM *_pStkCrsr = PCURSH->GetStkCrsr();
            if( _pStkCrsr )
                do {
                    lcl_ChkPaM( m_aShellCrsrEntries, nNode, nCntnt, *_pStkCrsr, true);
                    lcl_ChkPaM( m_aShellCrsrEntries, nNode, nCntnt, *_pStkCrsr, false);
                } while ( (_pStkCrsr != 0 ) &&
                    ((_pStkCrsr=(SwPaM *)_pStkCrsr->GetNext()) != PCURSH->GetStkCrsr()) );

            FOREACHPAM_START( PCURSH->_GetCrsr() )
                lcl_ChkPaM( m_aShellCrsrEntries, nNode, nCntnt, *PCURCRSR, true);
                lcl_ChkPaM( m_aShellCrsrEntries, nNode, nCntnt, *PCURCRSR, false);
            FOREACHPAM_END()

        FOREACHSHELL_END( pShell )
    }
}

void CntntIdxStoreImpl::RestoreShellCrsrs(SwDoc* /* pDoc */, updater_t& rUpdater)
{
    BOOST_FOREACH(const PaMEntry& aEntry, m_aShellCrsrEntries)
    {
        rUpdater(aEntry.m_pPaM->GetBound(aEntry.m_isMark), aEntry.m_nCntnt);
    }
}
namespace sw { namespace mark {
    boost::shared_ptr<CntntIdxStore> CntntIdxStore::Create()
    {
       return boost::shared_ptr<CntntIdxStore>(new CntntIdxStoreImpl());
    }
}}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
