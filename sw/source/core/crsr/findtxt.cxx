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

#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/util/SearchResult.hpp>
#include <comphelper/lok.hxx>
#include <o3tl/safeint.hxx>
#include <rtl/ustrbuf.hxx>
#include <svx/svdview.hxx>
#include <svl/srchitem.hxx>
#include <sfx2/sfxsids.hrc>
#include <editeng/outliner.hxx>

#include <wrtsh.hxx>
#include <txatritr.hxx>
#include <fldbas.hxx>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <txtfrm.hxx>
#include <rootfrm.hxx>
#include <swcrsr.hxx>
#include <redline.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentState.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <dcontact.hxx>
#include <pamtyp.hxx>
#include <ndtxt.hxx>
#include <swundo.hxx>
#include <UndoInsert.hxx>
#include <breakit.hxx>
#include <docsh.hxx>
#include <PostItMgr.hxx>
#include <view.hxx>

using namespace ::com::sun::star;
using namespace util;

namespace {

/// because the Find may be called on the View or the Model, we need an index
/// afflicted by multiple personality disorder
struct AmbiguousIndex
{
private:
    sal_Int32 m_value;

#ifndef NDEBUG
    enum class tags : char { Any, Frame, Model };
    tags m_tag;
#endif

public:
    AmbiguousIndex() : m_value(-1)
#ifndef NDEBUG
           , m_tag(tags::Any)
#endif
    {}
    explicit AmbiguousIndex(sal_Int32 const value
#ifndef NDEBUG
            , tags const tag
#endif
        )   : m_value(value)
#ifndef NDEBUG
            , m_tag(tag)
#endif
    {}

    sal_Int32 & GetAnyIndex() { return m_value; } ///< for arithmetic
    sal_Int32 const& GetAnyIndex() const { return m_value; } ///< for arithmetic
    TextFrameIndex GetFrameIndex() const
    {
        assert(m_tag != tags::Model);
        return TextFrameIndex(m_value);
    }
    sal_Int32 GetModelIndex() const
    {
        assert(m_tag != tags::Frame);
        return m_value;
    }
    void SetFrameIndex(TextFrameIndex const value)
    {
#ifndef NDEBUG
        m_tag = tags::Frame;
#endif
        m_value = sal_Int32(value);
    }
    void SetModelIndex(sal_Int32 const value)
    {
#ifndef NDEBUG
        m_tag = tags::Model;
#endif
        m_value = value;
    }

    bool operator ==(AmbiguousIndex const& rOther) const
    {
        assert(m_tag == tags::Any || rOther.m_tag == tags::Any || m_tag == rOther.m_tag);
        return m_value == rOther.m_value;
    }
    bool operator <=(AmbiguousIndex const& rOther) const
    {
        assert(m_tag == tags::Any || rOther.m_tag == tags::Any || m_tag == rOther.m_tag);
        return m_value <= rOther.m_value;
    }
    bool operator < (AmbiguousIndex const& rOther) const
    {
        assert(m_tag == tags::Any || rOther.m_tag == tags::Any || m_tag == rOther.m_tag);
        return m_value <  rOther.m_value;
    }
    AmbiguousIndex operator - (AmbiguousIndex const& rOther) const
    {
        assert(m_tag == tags::Any || rOther.m_tag == tags::Any || m_tag == rOther.m_tag);
        return AmbiguousIndex(m_value - rOther.m_value
#ifndef NDEBUG
                , std::max(m_tag, rOther.m_tag)
#endif
            );
    }
};

class MaybeMergedIter
{
    std::optional<sw::MergedAttrIter> m_oMergedIter;
    SwTextNode const*const m_pNode;
    size_t m_HintIndex;

public:
    MaybeMergedIter(SwTextFrame const*const pFrame, SwTextNode const*const pNode)
        : m_pNode(pNode)
        , m_HintIndex(0)
    {
        if (pFrame)
        {
#if BOOST_VERSION < 105600
            m_oMergedIter.reset(*pFrame);
#else
            m_oMergedIter.emplace(*pFrame);
#endif
        }
    }

    SwTextAttr const* NextAttr(SwTextNode const*& rpNode)
    {
        if (m_oMergedIter)
        {
            return m_oMergedIter->NextAttr(&rpNode);
        }
        if (SwpHints const*const pHints = m_pNode->GetpSwpHints())
        {
            if (m_HintIndex < pHints->Count())
            {
                rpNode = m_pNode;
                return pHints->Get(m_HintIndex++);
            }
        }
        return nullptr;
    }
};

}

static OUString
lcl_CleanStr(const SwTextNode& rNd,
             SwTextFrame const*const pFrame,
             SwRootFrame const*const pLayout,
             AmbiguousIndex const nStart, AmbiguousIndex & rEnd,
             std::vector<AmbiguousIndex> &rArr,
             bool const bRemoveSoftHyphen, bool const bRemoveCommentAnchors)
{
    OUStringBuffer buf(pLayout ? pFrame->GetText() : rNd.GetText());
    rArr.clear();

    MaybeMergedIter iter(pLayout ? pFrame : nullptr, pLayout ? nullptr : &rNd);

    AmbiguousIndex nSoftHyphen = nStart;
    AmbiguousIndex nHintStart;
    bool bNewHint       = true;
    bool bNewSoftHyphen = true;
    const AmbiguousIndex nEnd = rEnd;
    std::vector<AmbiguousIndex> aReplaced;
    SwTextNode const* pNextHintNode(nullptr);
    SwTextAttr const* pNextHint(iter.NextAttr(pNextHintNode));

    do
    {
        if ( bNewHint )
        {
            if (pLayout)
            {
                nHintStart.SetFrameIndex(pNextHint
                        ? pFrame->MapModelToView(pNextHintNode, pNextHint->GetStart())
                        : TextFrameIndex(-1));
            }
            else
            {
                nHintStart.SetModelIndex(pNextHint ? pNextHint->GetStart() : -1);
            }
        }

        if ( bNewSoftHyphen )
        {
            if (pLayout)
            {
                nSoftHyphen.SetFrameIndex(TextFrameIndex(bRemoveSoftHyphen
                    ? pFrame->GetText().indexOf(CHAR_SOFTHYPHEN, nSoftHyphen.GetAnyIndex())
                    : -1));
            }
            else
            {
                nSoftHyphen.SetModelIndex(bRemoveSoftHyphen
                    ? rNd.GetText().indexOf(CHAR_SOFTHYPHEN, nSoftHyphen.GetAnyIndex())
                    : -1);
            }
        }

        bNewHint       = false;
        bNewSoftHyphen = false;
        AmbiguousIndex nStt;

        // Check if next stop is a hint.
        if (0 <= nHintStart.GetAnyIndex()
            && (-1 == nSoftHyphen.GetAnyIndex() || nHintStart < nSoftHyphen)
            && nHintStart < nEnd )
        {
            nStt = nHintStart;
            bNewHint = true;
        }
        // Check if next stop is a soft hyphen.
        else if (   -1 != nSoftHyphen.GetAnyIndex()
                 && (-1 == nHintStart.GetAnyIndex() || nSoftHyphen < nHintStart)
                 && nSoftHyphen < nEnd)
        {
            nStt = nSoftHyphen;
            bNewSoftHyphen = true;
        }
        // If nSoftHyphen == nHintStart, the current hint *must* be a hint with an end.
        else if (-1 != nSoftHyphen.GetAnyIndex() && nSoftHyphen == nHintStart)
        {
            nStt = nSoftHyphen;
            bNewHint = true;
            bNewSoftHyphen = true;
        }
        else
            break;

        AmbiguousIndex nCurrent(nStt);
        nCurrent.GetAnyIndex() -= rArr.size();

        if ( bNewHint )
        {
            if (pNextHint && pNextHint->HasDummyChar() && (nStart <= nStt))
            {
                switch (pNextHint->Which())
                {
                case RES_TXTATR_FLYCNT:
                case RES_TXTATR_FTN:
                case RES_TXTATR_FIELD:
                case RES_TXTATR_REFMARK:
                case RES_TXTATR_TOXMARK:
                case RES_TXTATR_META:
                case RES_TXTATR_METAFIELD:
                    {
                        // (1998) they are desired as separators and
                        // belong not any longer to a word.
                        // they should also be ignored at a
                        // beginning/end of a sentence if blank. Those are
                        // simply removed if first. If at the end, we keep the
                        // replacement and remove afterwards all at a string's
                        // end (might be normal 0x7f).
                        const bool bEmpty = pNextHint->Which() != RES_TXTATR_FIELD
                            || (static_txtattr_cast<SwTextField const*>(pNextHint)->GetFormatField().GetField()->ExpandField(true, pLayout).isEmpty());
                        if ( bEmpty && nStart == nCurrent )
                        {
                            rArr.push_back( nCurrent );
                            if (rEnd.GetAnyIndex() > nCurrent.GetAnyIndex())
                            {
                                --rEnd.GetAnyIndex();
                            }
                            buf.remove(nCurrent.GetAnyIndex(), 1);
                        }
                        else
                        {
                            if ( bEmpty )
                                aReplaced.push_back( nCurrent );
                            buf[nCurrent.GetAnyIndex()] = '\x7f';
                        }
                    }
                    break;
                case RES_TXTATR_ANNOTATION:
                    {
                        if( bRemoveCommentAnchors )
                        {
                            rArr.push_back( nCurrent );
                            if (rEnd.GetAnyIndex() > nCurrent.GetAnyIndex())
                            {
                                --rEnd.GetAnyIndex();
                            }
                            buf.remove( nCurrent.GetAnyIndex(), 1 );
                        }
                    }
                    break;
                default:
                    OSL_FAIL( "unknown case in lcl_CleanStr" );
                    break;
                }
            }
            pNextHint = iter.NextAttr(pNextHintNode);
        }

        if ( bNewSoftHyphen )
        {
            rArr.push_back( nCurrent );

            // If the soft hyphen to be removed is past the end of the range we're searching in,
            // don't adjust the end.
            if (rEnd.GetAnyIndex() > nCurrent.GetAnyIndex())
            {
                --rEnd.GetAnyIndex();
            }

            buf.remove(nCurrent.GetAnyIndex(), 1);
            ++nSoftHyphen.GetAnyIndex();
        }
    }
    while ( true );

    for (auto i = aReplaced.size(); i; )
    {
        const AmbiguousIndex nTmp = aReplaced[ --i ];
        if (nTmp.GetAnyIndex() == buf.getLength() - 1)
        {
            buf.truncate(nTmp.GetAnyIndex());
            rArr.push_back( nTmp );
            --rEnd.GetAnyIndex();
        }
    }

    return buf.makeStringAndClear();
}

static bool DoSearch(SwPaM & rSearchPam,
    const i18nutil::SearchOptions2& rSearchOpt, utl::TextSearch& rSText,
    SwMoveFnCollection const & fnMove,
    bool bSrchForward, bool bRegSearch, bool bChkEmptyPara, bool bChkParaEnd,
    AmbiguousIndex & nStart, AmbiguousIndex & nEnd, AmbiguousIndex nTextLen,
    SwTextNode const* pNode, SwTextFrame const* pTextFrame,
    SwRootFrame const* pLayout, SwPaM* pPam);

namespace sw {

// @param xSearchItem allocate in parent so we can do so outside the calling loop
bool FindTextImpl(SwPaM & rSearchPam,
        const i18nutil::SearchOptions2& rSearchOpt, bool bSearchInNotes,
        utl::TextSearch& rSText,
        SwMoveFnCollection const & fnMove, const SwPaM & rRegion,
        bool bInReadOnly, SwRootFrame const*const pLayout,
        std::unique_ptr<SvxSearchItem>& xSearchItem)
{
    if( rSearchOpt.searchString.isEmpty() )
        return false;

    std::unique_ptr<SwPaM> pPam = sw::MakeRegion(fnMove, rRegion);
    const bool bSrchForward = &fnMove == &fnMoveForward;
    SwNodeIndex& rNdIdx = pPam->GetPoint()->nNode;
    SwIndex& rContentIdx = pPam->GetPoint()->nContent;

    // If bFound is true then the string was found and is between nStart and nEnd
    bool bFound = false;
    // start position in text or initial position
    bool bFirst = true;
    SwContentNode * pNode;

    const bool bRegSearch = SearchAlgorithms2::REGEXP == rSearchOpt.AlgorithmType2;
    const bool bChkEmptyPara = bRegSearch && 2 == rSearchOpt.searchString.getLength() &&
                        ( rSearchOpt.searchString == "^$" ||
                          rSearchOpt.searchString == "$^" );
    const bool bChkParaEnd = bRegSearch && rSearchOpt.searchString == "$";

    if (!xSearchItem)
    {
        xSearchItem.reset(new SvxSearchItem(SID_SEARCH_ITEM)); // this is a very expensive operation (calling configmgr etc.)
        xSearchItem->SetSearchOptions(rSearchOpt);
        xSearchItem->SetBackward(!bSrchForward);
    }

    // LanguageType eLastLang = 0;
    while (nullptr != (pNode = ::GetNode(*pPam, bFirst, fnMove, bInReadOnly, pLayout)))
    {
        if( pNode->IsTextNode() )
        {
            SwTextNode& rTextNode = *pNode->GetTextNode();
            SwTextFrame const*const pFrame(pLayout
                ? static_cast<SwTextFrame const*>(rTextNode.getLayoutFrame(pLayout))
                : nullptr);
            assert(!pLayout || pFrame);
            AmbiguousIndex nTextLen;
            if (pLayout)
            {
                nTextLen.SetFrameIndex(TextFrameIndex(pFrame->GetText().getLength()));
            }
            else
            {
                nTextLen.SetModelIndex(rTextNode.GetText().getLength());
            }
            AmbiguousIndex nEnd;
            if (pLayout
                    ? FrameContainsNode(*pFrame, pPam->GetMark()->nNode.GetIndex())
                    : rNdIdx == pPam->GetMark()->nNode)
            {
                if (pLayout)
                {
                    nEnd.SetFrameIndex(pFrame->MapModelToViewPos(*pPam->GetMark()));
                }
                else
                {
                    nEnd.SetModelIndex(pPam->GetMark()->nContent.GetIndex());
                }
            }
            else
            {
                if (bSrchForward)
                {
                    nEnd = nTextLen;
                }
                else
                {
                    if (pLayout)
                    {
                        nEnd.SetFrameIndex(TextFrameIndex(0));
                    }
                    else
                    {
                        nEnd.SetModelIndex(0);
                    }
                }
            }
            AmbiguousIndex nStart;
            if (pLayout)
            {
                nStart.SetFrameIndex(pFrame->MapModelToViewPos(*pPam->GetPoint()));
            }
            else
            {
                nStart.SetModelIndex(rContentIdx.GetIndex());
            }

            /* #i80135# */
            // if there are SwPostItFields inside our current node text, we
            // split the text into separate pieces and search for text inside
            // the pieces as well as inside the fields
            MaybeMergedIter iter(pLayout ? pFrame : nullptr, pLayout ? nullptr : &rTextNode);

            // count PostItFields by looping over all fields
            std::vector<std::pair<SwTextAttr const*, AmbiguousIndex>> postits;
            if (bSearchInNotes)
            {
                if (!bSrchForward)
                {
                    std::swap(nStart, nEnd);
                }

                SwTextNode const* pTemp(nullptr);
                while (SwTextAttr const*const pTextAttr = iter.NextAttr(pTemp))
                {
                    if ( pTextAttr->Which()==RES_TXTATR_ANNOTATION )
                    {
                        AmbiguousIndex aPos;
                        aPos.SetModelIndex(pTextAttr->GetStart());
                        if (pLayout)
                        {
                            aPos.SetFrameIndex(pFrame->MapModelToView(pTemp, aPos.GetModelIndex()));
                        }
                        if ((nStart <= aPos) && (aPos <= nEnd))
                        {
                            postits.emplace_back(pTextAttr, aPos);
                        }
                    }
                }

                if (!bSrchForward)
                {
                    std::swap(nStart, nEnd);
                }

            }

            SwDocShell *const pDocShell = pNode->GetDoc().GetDocShell();
            SwWrtShell *const pWrtShell = pDocShell ? pDocShell->GetWrtShell() : nullptr;
            SwPostItMgr *const pPostItMgr = pWrtShell ? pWrtShell->GetPostItMgr() : nullptr;

            // If there is an active text edit, then search there.
            bool bEndedTextEdit = false;
            SdrView* pSdrView = pWrtShell ? pWrtShell->GetDrawView() : nullptr;
            if (pSdrView)
            {
                // If the edited object is not anchored to this node, then ignore it.
                SdrObject* pObject = pSdrView->GetTextEditObject();
                if (pObject)
                {
                    if (SwFrameFormat* pFrameFormat = FindFrameFormat(pObject))
                    {
                        const SwPosition* pPosition = pFrameFormat->GetAnchor().GetContentAnchor();
                        if (!pPosition || (pLayout
                                ? !FrameContainsNode(*pFrame, pPosition->nNode.GetIndex())
                                : pPosition->nNode.GetIndex() != pNode->GetIndex()))
                            pObject = nullptr;
                    }
                }

                if (pObject)
                {
                    sal_uInt16 nResult = pSdrView->GetTextEditOutlinerView()->StartSearchAndReplace(*xSearchItem);
                    if (!nResult)
                    {
                        // If not found, end the text edit.
                        pSdrView->SdrEndTextEdit();
                        const Point aPoint(pSdrView->GetAllMarkedRect().TopLeft());
                        pSdrView->UnmarkAll();
                        pWrtShell->CallSetCursor(&aPoint, true);
                        pWrtShell->Edit();
                        bEndedTextEdit = true;
                    }
                    else
                    {
                        bFound = true;
                        break;
                    }
                }
            }

            if (comphelper::LibreOfficeKit::isActive())
            {
                // Writer and editeng selections are not supported in parallel.
                SvxSearchItem* pSearchItem = SwView::GetSearchItem();
                // If we just finished search in shape text, don't attempt to do that again.
                if (!bEndedTextEdit && !(pSearchItem && pSearchItem->GetCommand() == SvxSearchCmd::FIND_ALL))
                {
                    // If there are any shapes anchored to this node, search there.
                    SwPaM aPaM(pNode->GetDoc().GetNodes().GetEndOfContent());
                    if (pLayout)
                    {
                        *aPaM.GetPoint() = pFrame->MapViewToModelPos(nStart.GetFrameIndex());
                    }
                    else
                    {
                        aPaM.GetPoint()->nNode = rTextNode;
                        aPaM.GetPoint()->nContent.Assign(
                            aPaM.GetPoint()->nNode.GetNode().GetTextNode(),
                            nStart.GetModelIndex());
                    }
                    aPaM.SetMark();
                    if (pLayout)
                    {
                        aPaM.GetMark()->nNode = (pFrame->GetMergedPara()
                                ? *pFrame->GetMergedPara()->pLastNode
                                : rTextNode)
                            .GetIndex() + 1;
                    }
                    else
                    {
                        aPaM.GetMark()->nNode = rTextNode.GetIndex() + 1;
                    }
                    aPaM.GetMark()->nContent.Assign(aPaM.GetMark()->nNode.GetNode().GetTextNode(), 0);
                    if (pNode->GetDoc().getIDocumentDrawModelAccess().Search(aPaM, *xSearchItem) && pSdrView)
                    {
                        if (SdrObject* pObject = pSdrView->GetTextEditObject())
                        {
                            if (SwFrameFormat* pFrameFormat = FindFrameFormat(pObject))
                            {
                                const SwPosition* pPosition = pFrameFormat->GetAnchor().GetContentAnchor();
                                if (pPosition)
                                {
                                    // Set search position to the shape's anchor point.
                                    *rSearchPam.GetPoint() = *pPosition;
                                    rSearchPam.GetPoint()->nContent.Assign(pPosition->nNode.GetNode().GetContentNode(), 0);
                                    rSearchPam.SetMark();
                                    bFound = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            // do we need to finish a note?
            if (pPostItMgr && pPostItMgr->HasActiveSidebarWin())
            {
                if (bSearchInNotes)
                {
                    if (!postits.empty())
                    {
                        if (bSrchForward)
                        {
                            postits.erase(postits.begin());
                        }
                        else
                        {
                            postits.pop_back(); // hope that's the right one?
                        }
                    }
                    //search inside, finish and put focus back into the doc
                    if (pPostItMgr->FinishSearchReplace(rSearchOpt,bSrchForward))
                    {
                        bFound = true ;
                        break;
                    }
                }
                else
                {
                    pPostItMgr->SetActiveSidebarWin(nullptr);
                }
            }

            if (!postits.empty())
            {
                // now we have to split
                AmbiguousIndex nStartInside;
                AmbiguousIndex nEndInside;
                sal_Int32 aLoop = bSrchForward ? 0 : postits.size();

                while ((0 <= aLoop) && (o3tl::make_unsigned(aLoop) <= postits.size()))
                {
                    if (bSrchForward)
                    {
                        if (aLoop == 0)
                        {
                            nStartInside = nStart;
                        }
                        else if (pLayout)
                        {
                            nStartInside.SetFrameIndex(postits[aLoop - 1].second.GetFrameIndex() + TextFrameIndex(1));
                        }
                        else
                        {
                            nStartInside.SetModelIndex(postits[aLoop - 1].second.GetModelIndex() + 1);
                        }
                        nEndInside = static_cast<size_t>(aLoop) == postits.size()
                            ? nEnd
                            : postits[aLoop].second;
                        nTextLen = nEndInside - nStartInside;
                    }
                    else
                    {
                        nStartInside = static_cast<size_t>(aLoop) == postits.size()
                            ? nStart
                            : postits[aLoop].second;
                        if (aLoop == 0)
                        {
                            nEndInside = nEnd;
                        }
                        else if (pLayout)
                        {
                            nEndInside.SetFrameIndex(postits[aLoop - 1].second.GetFrameIndex() + TextFrameIndex(1));
                        }
                        else
                        {
                            nEndInside.SetModelIndex(postits[aLoop - 1].second.GetModelIndex() + 1);
                        }
                        nTextLen = nStartInside - nEndInside;
                    }
                    // search inside the text between a note
                    bFound = DoSearch( rSearchPam,
                                       rSearchOpt, rSText, fnMove, bSrchForward,
                                       bRegSearch, bChkEmptyPara, bChkParaEnd,
                                       nStartInside, nEndInside, nTextLen,
                                       pNode->GetTextNode(), pFrame, pLayout,
                                       pPam.get() );
                    if ( bFound )
                        break;
                    else
                    {
                        // we should now be right in front of a note, search inside
                        if (bSrchForward
                            ? (static_cast<size_t>(aLoop) != postits.size())
                            : (aLoop != 0))
                        {
                            const SwTextAttr *const pTextAttr = bSrchForward
                                ? postits[aLoop].first
                                : postits[aLoop - 1].first;
                            if (pPostItMgr && pPostItMgr->SearchReplace(
                                    static_txtattr_cast<SwTextField const*>(pTextAttr)->GetFormatField(),rSearchOpt,bSrchForward))
                            {
                                bFound = true ;
                                break;
                            }
                        }
                    }
                    aLoop = bSrchForward ? aLoop+1 : aLoop-1;
                }
            }
            else
            {
                // if there is no SwPostItField inside or searching inside notes
                // is disabled, we search the whole length just like before
                bFound = DoSearch( rSearchPam,
                                   rSearchOpt, rSText, fnMove, bSrchForward,
                                   bRegSearch, bChkEmptyPara, bChkParaEnd,
                                   nStart, nEnd, nTextLen,
                                   pNode->GetTextNode(), pFrame, pLayout,
                                   pPam.get() );
            }
            if (bFound)
                break;
        }
    }
    return bFound;
}

} // namespace sw

bool DoSearch(SwPaM & rSearchPam,
        const i18nutil::SearchOptions2& rSearchOpt, utl::TextSearch& rSText,
                      SwMoveFnCollection const & fnMove, bool bSrchForward, bool bRegSearch,
                      bool bChkEmptyPara, bool bChkParaEnd,
        AmbiguousIndex & nStart, AmbiguousIndex & nEnd, AmbiguousIndex const nTextLen,
        SwTextNode const*const pNode, SwTextFrame const*const pFrame,
        SwRootFrame const*const pLayout, SwPaM* pPam)
{
    bool bFound = false;
    SwNodeIndex& rNdIdx = pPam->GetPoint()->nNode;
    OUString sCleanStr;
    std::vector<AmbiguousIndex> aFltArr;
    LanguageType eLastLang = LANGUAGE_SYSTEM;
    // if the search string contains a soft hyphen,
    // we don't strip them from the text:
    bool bRemoveSoftHyphens = true;
    // if the search string contains a comment, we don't strip them from the text
    const bool bRemoveCommentAnchors = rSearchOpt.searchString.indexOf( CH_TXTATR_INWORD ) == -1;

    if ( bRegSearch )
    {
        if (   -1 != rSearchOpt.searchString.indexOf("\\xAD")
            || -1 != rSearchOpt.searchString.indexOf("\\x{00AD}")
            || -1 != rSearchOpt.searchString.indexOf("\\u00AD")
            || -1 != rSearchOpt.searchString.indexOf("\\U000000AD")
            || -1 != rSearchOpt.searchString.indexOf("\\N{SOFT HYPHEN}"))
        {
             bRemoveSoftHyphens = false;
        }
    }
    else
    {
        if ( 1 == rSearchOpt.searchString.getLength() &&
             CHAR_SOFTHYPHEN == rSearchOpt.searchString.toChar() )
             bRemoveSoftHyphens = false;
    }

    if( bSrchForward )
        sCleanStr = lcl_CleanStr(*pNode, pFrame, pLayout, nStart, nEnd,
                        aFltArr, bRemoveSoftHyphens, bRemoveCommentAnchors);
    else
        sCleanStr = lcl_CleanStr(*pNode, pFrame, pLayout, nEnd, nStart,
                        aFltArr, bRemoveSoftHyphens, bRemoveCommentAnchors);

    std::unique_ptr<SwScriptIterator> pScriptIter;
    sal_uInt16 nSearchScript = 0;
    sal_uInt16 nCurrScript = 0;

    if (SearchAlgorithms2::APPROXIMATE == rSearchOpt.AlgorithmType2)
    {
        pScriptIter.reset(new SwScriptIterator(sCleanStr, nStart.GetAnyIndex(), bSrchForward));
        nSearchScript = g_pBreakIt->GetRealScriptOfText( rSearchOpt.searchString, 0 );
    }

    const AmbiguousIndex nStringEnd = nEnd;
    bool bZeroMatch = false;    // zero-length match, i.e. only $ anchor as regex
    while ( ((bSrchForward && nStart < nStringEnd) ||
            (!bSrchForward && nStringEnd < nStart)) && !bZeroMatch )
    {
        // SearchAlgorithms_APPROXIMATE works on a per word base so we have to
        // provide the text searcher with the correct locale, because it uses
        // the break-iterator
        if ( pScriptIter )
        {
            nEnd.GetAnyIndex() = pScriptIter->GetScriptChgPos();
            nCurrScript = pScriptIter->GetCurrScript();
            if ( nSearchScript == nCurrScript )
            {
                const LanguageType eCurrLang = pLayout
                        ? pFrame->GetLangOfChar(bSrchForward
                                ? nStart.GetFrameIndex()
                                : nEnd.GetFrameIndex(),
                            0, true)
                        : pNode->GetLang(bSrchForward
                                ? nStart.GetModelIndex()
                                : nEnd.GetModelIndex());

                if ( eCurrLang != eLastLang )
                {
                    const lang::Locale aLocale(
                            g_pBreakIt->GetLocale( eCurrLang ) );
                    rSText.SetLocale( utl::TextSearch::UpgradeToSearchOptions2( rSearchOpt), aLocale );
                    eLastLang = eCurrLang;
                }
            }
            pScriptIter->Next();
        }
        AmbiguousIndex nProxyStart = nStart;
        AmbiguousIndex nProxyEnd = nEnd;
        if( nSearchScript == nCurrScript &&
                (rSText.*fnMove.fnSearch)( sCleanStr, &nProxyStart.GetAnyIndex(), &nProxyEnd.GetAnyIndex(), nullptr) &&
                !(bZeroMatch = (nProxyStart == nProxyEnd)))
        {
            nStart = nProxyStart;
            nEnd = nProxyEnd;
            // set section correctly
            *rSearchPam.GetPoint() = *pPam->GetPoint();
            rSearchPam.SetMark();

            // adjust start and end
            if( !aFltArr.empty() )
            {
                // if backward search, switch positions temporarily
                if (!bSrchForward) { std::swap(nStart, nEnd); }

                AmbiguousIndex nNew = nStart;
                for (size_t n = 0; n < aFltArr.size() && aFltArr[ n ] <= nStart; ++n )
                {
                    ++nNew.GetAnyIndex();
                }

                nStart = nNew;
                nNew = nEnd;
                for( size_t n = 0; n < aFltArr.size() && aFltArr[ n ] < nEnd; ++n )
                {
                    ++nNew.GetAnyIndex();
                }

                nEnd = nNew;
                // if backward search, switch positions temporarily
                if( !bSrchForward ) { std::swap(nStart, nEnd); }
            }
            if (pLayout)
            {
                *rSearchPam.GetMark() = pFrame->MapViewToModelPos(nStart.GetFrameIndex());
                *rSearchPam.GetPoint() = pFrame->MapViewToModelPos(nEnd.GetFrameIndex());
            }
            else
            {
                rSearchPam.GetMark()->nContent = nStart.GetModelIndex();
                rSearchPam.GetPoint()->nContent = nEnd.GetModelIndex();
            }

            // if backward search, switch point and mark
            if( !bSrchForward )
                rSearchPam.Exchange();
            bFound = true;
            break;
        }
        else
        {
            nEnd = nProxyEnd;
        }
        nStart = nEnd;
    }

    pScriptIter.reset();

    if ( bFound )
        return true;
    else if ((bChkEmptyPara && !nStart.GetAnyIndex() && !nTextLen.GetAnyIndex())
             || bChkParaEnd)
    {
        *rSearchPam.GetPoint() = *pPam->GetPoint();
        if (pLayout)
        {
            *rSearchPam.GetPoint() = pFrame->MapViewToModelPos(
                bChkParaEnd ? nTextLen.GetFrameIndex() : TextFrameIndex(0));
        }
        else
        {
            rSearchPam.GetPoint()->nContent = bChkParaEnd ? nTextLen.GetModelIndex() : 0;
        }
        rSearchPam.SetMark();
        const SwNode *const pSttNd = bSrchForward
            ? &rSearchPam.GetPoint()->nNode.GetNode() // end of the frame
            : &rNdIdx.GetNode(); // keep the bug as-is for now...
        /* FIXME: this condition does not work for !bSrchForward backward
         * search, it probably never did. (pSttNd != &rNdIdx.GetNode())
         * is never true in this case. */
        if( (bSrchForward || pSttNd != &rNdIdx.GetNode()) &&
            rSearchPam.Move(fnMoveForward, GoInContent) &&
            (!bSrchForward || pSttNd != &rSearchPam.GetPoint()->nNode.GetNode()) &&
            1 == std::abs(static_cast<int>(rSearchPam.GetPoint()->nNode.GetIndex() -
                             rSearchPam.GetMark()->nNode.GetIndex())))
        {
            // if backward search, switch point and mark
            if( !bSrchForward )
                rSearchPam.Exchange();
            return true;
        }
    }
    return bFound;
}

namespace {

/// parameters for search and replace in text
struct SwFindParaText : public SwFindParas
{
    const i18nutil::SearchOptions2& m_rSearchOpt;
    SwCursor& m_rCursor;
    SwRootFrame const* m_pLayout;
    utl::TextSearch m_aSText;
    bool m_bReplace;
    bool m_bSearchInNotes;

    SwFindParaText(const i18nutil::SearchOptions2& rOpt, bool bSearchInNotes,
            bool bRepl, SwCursor& rCursor, SwRootFrame const*const pLayout)
        : m_rSearchOpt( rOpt )
        , m_rCursor( rCursor )
        , m_pLayout(pLayout)
        , m_aSText( utl::TextSearch::UpgradeToSearchOptions2(rOpt) )
        , m_bReplace( bRepl )
        , m_bSearchInNotes( bSearchInNotes )
    {}
    virtual int DoFind(SwPaM &, SwMoveFnCollection const &, const SwPaM &, bool bInReadOnly, std::unique_ptr<SvxSearchItem>& xSearchItem) override;
    virtual bool IsReplaceMode() const override;
    virtual ~SwFindParaText();
};

}

SwFindParaText::~SwFindParaText()
{
}

int SwFindParaText::DoFind(SwPaM & rCursor, SwMoveFnCollection const & fnMove,
                          const SwPaM & rRegion, bool bInReadOnly,
                          std::unique_ptr<SvxSearchItem>& xSearchItem)
{
    if( bInReadOnly && m_bReplace )
        bInReadOnly = false;

    const bool bFnd = sw::FindTextImpl(rCursor, m_rSearchOpt, m_bSearchInNotes,
            m_aSText, fnMove, rRegion, bInReadOnly, m_pLayout, xSearchItem);

    if( bFnd && m_bReplace ) // replace string
    {
        // use replace method in SwDoc
        const bool bRegExp(SearchAlgorithms2::REGEXP == m_rSearchOpt.AlgorithmType2);
        SwIndex& rSttCntIdx = rCursor.Start()->nContent;
        const sal_Int32 nSttCnt = rSttCntIdx.GetIndex();
        // add to shell-cursor-ring so that the regions will be moved eventually
        SwPaM* pPrev(nullptr);
        if( bRegExp )
        {
            pPrev = const_cast<SwPaM&>(rRegion).GetPrev();
            const_cast<SwPaM&>(rRegion).GetRingContainer().merge( m_rCursor.GetRingContainer() );
        }

        std::optional<OUString> xRepl;
        if (bRegExp)
            xRepl = sw::ReplaceBackReferences(m_rSearchOpt, &rCursor, m_pLayout);
        bool const bReplaced = sw::ReplaceImpl(rCursor,
                xRepl ? *xRepl : m_rSearchOpt.replaceString,
                bRegExp, *m_rCursor.GetDoc(), m_pLayout);

        m_rCursor.SaveTableBoxContent( rCursor.GetPoint() );

        if( bRegExp )
        {
            // and remove region again
            SwPaM* p;
            SwPaM* pNext(const_cast<SwPaM*>(&rRegion));
            do {
                p = pNext;
                pNext = p->GetNext();
                p->MoveTo(const_cast<SwPaM*>(&rRegion));
            } while( p != pPrev );
        }
        if (bRegExp && !bReplaced)
        {   // fdo#80715 avoid infinite loop if join failed
            bool bRet = ((&fnMoveForward == &fnMove) ? &GoNextPara : &GoPrevPara)
                (rCursor, fnMove);
            (void) bRet;
            assert(bRet); // if join failed, next node must be SwTextNode
        }
        else
            rCursor.Start()->nContent = nSttCnt;
        return FIND_NO_RING;
    }
    return bFnd ? FIND_FOUND : FIND_NOT_FOUND;
}

bool SwFindParaText::IsReplaceMode() const
{
    return m_bReplace;
}

sal_uLong SwCursor::Find_Text( const i18nutil::SearchOptions2& rSearchOpt, bool bSearchInNotes,
                          SwDocPositions nStart, SwDocPositions nEnd,
                          bool& bCancel, FindRanges eFndRngs, bool bReplace,
                          SwRootFrame const*const pLayout)
{
    // switch off OLE-notifications
    SwDoc* pDoc = GetDoc();
    Link<bool,void> aLnk( pDoc->GetOle2Link() );
    pDoc->SetOle2Link( Link<bool,void>() );

    bool const bStartUndo = pDoc->GetIDocumentUndoRedo().DoesUndo() && bReplace;
    if (bStartUndo)
    {
        pDoc->GetIDocumentUndoRedo().StartUndo( SwUndoId::REPLACE, nullptr );
    }

    bool bSearchSel = 0 != (rSearchOpt.searchFlag & SearchFlags::REG_NOT_BEGINOFLINE);
    if( bSearchSel )
        eFndRngs = static_cast<FindRanges>(eFndRngs | FindRanges::InSel);
    SwFindParaText aSwFindParaText(rSearchOpt, bSearchInNotes, bReplace, *this, pLayout);
    sal_uLong nRet = FindAll( aSwFindParaText, nStart, nEnd, eFndRngs, bCancel );
    pDoc->SetOle2Link( aLnk );
    if( nRet && bReplace )
        pDoc->getIDocumentState().SetModified();

    if (bStartUndo)
    {
        SwRewriter rewriter(MakeUndoReplaceRewriter(
                nRet, rSearchOpt.searchString, rSearchOpt.replaceString));
        pDoc->GetIDocumentUndoRedo().EndUndo( SwUndoId::REPLACE, & rewriter );
    }
    return nRet;
}

namespace sw {

bool ReplaceImpl(
        SwPaM & rCursor,
        OUString const& rReplacement,
        bool const bRegExp,
        SwDoc & rDoc,
        SwRootFrame const*const pLayout)
{
    bool bReplaced(true);
    IDocumentContentOperations & rIDCO(rDoc.getIDocumentContentOperations());
#if 0
    // FIXME there's some problem with multiple redlines here on Undo
    std::vector<std::shared_ptr<SwUnoCursor>> ranges;
    if (rDoc.getIDocumentRedlineAccess().IsRedlineOn()
        || !pLayout
        || !pLayout->IsHideRedlines()
        || sw::GetRanges(ranges, rDoc, rCursor))
    {
        bReplaced = rIDCO.ReplaceRange(rCursor, rReplacement, bRegExp);
    }
    else
    {
        assert(!ranges.empty());
        assert(ranges.front()->GetPoint()->nNode == ranges.front()->GetMark()->nNode);
        bReplaced = rIDCO.ReplaceRange(*ranges.front(), rReplacement, bRegExp);
        for (auto it = ranges.begin() + 1; it != ranges.end(); ++it)
        {
            bReplaced &= rIDCO.DeleteAndJoin(**it);
        }
    }
#else
    IDocumentRedlineAccess const& rIDRA(rDoc.getIDocumentRedlineAccess());
    if (pLayout && pLayout->IsHideRedlines()
        && !rIDRA.IsRedlineOn() // otherwise: ReplaceRange will handle it
        && (rIDRA.GetRedlineFlags() & RedlineFlags::ShowDelete)) // otherwise: ReplaceRange will DeleteRedline()
    {
        SwRedlineTable::size_type tmp;
        rIDRA.GetRedline(*rCursor.Start(), &tmp);
        while (tmp < rIDRA.GetRedlineTable().size())
        {
            SwRangeRedline const*const pRedline(rIDRA.GetRedlineTable()[tmp]);
            if (*rCursor.End() <= *pRedline->Start())
            {
                break;
            }
            if (*pRedline->End() <= *rCursor.Start())
            {
                ++tmp;
                continue;
            }
            if (pRedline->GetType() == RedlineType::Delete)
            {
                assert(*pRedline->Start() != *pRedline->End());
                // search in hidden layout can't overlap redlines
                assert(*rCursor.Start() <= *pRedline->Start() && *pRedline->End() <= *rCursor.End());
                SwPaM pam(*pRedline, nullptr);
                bReplaced &= rIDCO.DeleteAndJoin(pam);
            }
            else
            {
                ++tmp;
            }
        }
    }
    bReplaced &= rIDCO.ReplaceRange(rCursor, rReplacement, bRegExp);
#endif
    return bReplaced;
}

std::optional<OUString> ReplaceBackReferences(const i18nutil::SearchOptions2& rSearchOpt,
        SwPaM *const pPam, SwRootFrame const*const pLayout)
{
    std::optional<OUString> xRet;
    if( pPam && pPam->HasMark() &&
        SearchAlgorithms2::REGEXP == rSearchOpt.AlgorithmType2 )
    {
        SwContentNode const*const pTextNode = pPam->GetContentNode();
        SwContentNode const*const pMarkTextNode = pPam->GetContentNode(false);
        if (!pTextNode || !pTextNode->IsTextNode()
            || !pMarkTextNode || !pMarkTextNode->IsTextNode())
        {
            return xRet;
        }
        SwTextFrame const*const pFrame(pLayout
            ? static_cast<SwTextFrame const*>(pTextNode->getLayoutFrame(pLayout))
            : nullptr);
        const bool bParaEnd = rSearchOpt.searchString == "$" || rSearchOpt.searchString == "^$" || rSearchOpt.searchString == "$^";
        if (bParaEnd || (pLayout
                ? sw::FrameContainsNode(*pFrame, pPam->GetMark()->nNode.GetIndex())
                : pTextNode == pMarkTextNode))
        {
            utl::TextSearch aSText( utl::TextSearch::UpgradeToSearchOptions2( rSearchOpt) );
            SearchResult aResult;
            OUString aReplaceStr( rSearchOpt.replaceString );
            if (bParaEnd)
            {
                OUString const aStr("\\n");
                aResult.subRegExpressions = 1;
                aResult.startOffset.realloc(1);
                aResult.endOffset.realloc(1);
                aResult.startOffset[0] = 0;
                aResult.endOffset[0] = aStr.getLength();
                aSText.ReplaceBackReferences( aReplaceStr, aStr, aResult );
                xRet = aReplaceStr;
            }
            else
            {
                OUString const aStr(pLayout
                    ? pFrame->GetText()
                    : pTextNode->GetTextNode()->GetText());
                AmbiguousIndex nStart;
                AmbiguousIndex nEnd;
                if (pLayout)
                {
                    nStart.SetFrameIndex(pFrame->MapModelToViewPos(*pPam->Start()));
                    nEnd.SetFrameIndex(pFrame->MapModelToViewPos(*pPam->End()));
                }
                else
                {
                    nStart.SetModelIndex(pPam->Start()->nContent.GetIndex());
                    nEnd.SetModelIndex(pPam->End()->nContent.GetIndex());
                }
                if (aSText.SearchForward(aStr, &nStart.GetAnyIndex(), &nEnd.GetAnyIndex(), &aResult))
                {
                    aSText.ReplaceBackReferences( aReplaceStr, aStr, aResult );
                    xRet = aReplaceStr;
                }
            }
        }
    }
    return xRet;
}

} // namespace sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
