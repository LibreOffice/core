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

#include <hintids.hxx>
#include <o3tl/make_unique.hxx>
#include <svl/whiter.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <swmodule.hxx>
#include <redline.hxx>
#include <txtatr.hxx>
#include <docary.hxx>
#include "itratr.hxx"
#include <ndtxt.hxx>
#include <doc.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>
#include <breakit.hxx>
#include <vcl/keycodes.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/settings.hxx>
#include <txtfrm.hxx>
#include <ftnfrm.hxx>
#include <vcl/svapp.hxx>
#include "redlnitr.hxx"
#include <extinput.hxx>
#include <sfx2/printer.hxx>
#include <vcl/window.hxx>

using namespace ::com::sun::star;

namespace sw {

std::unique_ptr<sw::MergedPara>
CheckParaRedlineMerge(SwTextFrame & rFrame, SwTextNode & rTextNode,
       FrameMode const eMode)
{
    IDocumentRedlineAccess const& rIDRA = rTextNode.getIDocumentRedlineAccess();
    if (!rFrame.getRootFrame()->IsHideRedlines())
    {
        return nullptr;
    }
    bool bHaveRedlines(false);
    std::vector<SwTextNode *> nodes{ &rTextNode };
    std::vector<sw::Extent> extents;
    OUStringBuffer mergedText;
    SwTextNode const* pParaPropsNode(nullptr);
    SwTextNode * pNode(&rTextNode);
    sal_Int32 nLastEnd(0);
    for (auto i = rIDRA.GetRedlinePos(rTextNode, USHRT_MAX);
         i < rIDRA.GetRedlineTable().size(); ++i)
    {
        SwRangeRedline const*const pRed = rIDRA.GetRedlineTable()[i];

        if (pNode->GetIndex() < pRed->Start()->nNode.GetIndex())
            break;

        if (pRed->GetType() != nsRedlineType_t::REDLINE_DELETE)
            continue;

        SwPosition const*const pStart(pRed->Start());
        SwPosition const*const pEnd(pRed->End());
        assert(*pStart != *pEnd); // empty delete allowed if shown ???
        bHaveRedlines = true;
        if (pStart->nContent != nLastEnd) // not 0 so we eliminate adjacent deletes
        {
            extents.emplace_back(pNode, nLastEnd, pStart->nContent.GetIndex());
            mergedText.append(pNode->GetText().copy(nLastEnd, pStart->nContent.GetIndex() - nLastEnd));
        }
        if (&pEnd->nNode.GetNode() != pNode)
        {
            if (pNode == &rTextNode)
            {
                pNode->SetRedlineMergeFlag(SwNode::Merge::First);
            } // else: was already set before
            for (sal_uLong j = pNode->GetIndex() + 1; j < pEnd->nNode.GetIndex(); ++j)
            {
                pNode->GetNodes()[j]->SetRedlineMergeFlag(SwNode::Merge::Hidden);
            }
            pNode = pEnd->nNode.GetNode().GetTextNode();
            assert(pNode);
            nodes.push_back(pNode);
            pNode->SetRedlineMergeFlag(SwNode::Merge::NonFirst);
        }
        nLastEnd = pEnd->nContent.GetIndex();
    }
    if (!bHaveRedlines)
    {
        return nullptr;
    }
    if (nLastEnd != pNode->Len())
    {
        extents.emplace_back(pNode, nLastEnd, pNode->Len());
        mergedText.append(pNode->GetText().copy(nLastEnd, pNode->Len() - nLastEnd));
    }
    if (extents.empty()) // there was no text anywhere
    {
        assert(mergedText.isEmpty());
        pParaPropsNode = &rTextNode; // if every node is empty, the first one wins
    }
    else
    {
        assert(!mergedText.isEmpty());
        pParaPropsNode = extents.begin()->pNode; // para props from first node that isn't empty
    }
    if (eMode == FrameMode::Existing)
    {
        // remove existing footnote frames for first node;
        // for non-first notes, DelFrames will remove all
        // (could possibly call lcl_ChangeFootnoteRef, not sure if worth it)
        // note: must be done *before* changing listeners!
        sal_Int32 nLast(0);
        std::vector<std::pair<sal_Int32, sal_Int32>> hidden;
        for (auto const& rExtent : extents)
        {
            if (rExtent.pNode != &rTextNode)
            {
                break;
            }
            if (rExtent.nStart != 0)
            {
                assert(rExtent.nStart != nLast);
                hidden.emplace_back(nLast, rExtent.nStart);
            }
            nLast = rExtent.nEnd;
        }
        if (nLast != rTextNode.Len())
        {
            hidden.emplace_back(nLast, rTextNode.Len());
        }
        sw::RemoveFootnotesForNode(rFrame, rTextNode, &hidden);
        // unfortunately DelFrames() must be done before StartListening too,
        // otherwise footnotes cannot be deleted by SwTextFootnote::DelFrames!
        for (auto iter = ++nodes.begin(); iter != nodes.end(); ++iter)
        {
            (**iter).DelFrames(rFrame.getRootFrame());
        }
    }
    auto pRet(o3tl::make_unique<sw::MergedPara>(rFrame, std::move(extents),
                mergedText.makeStringAndClear(), pParaPropsNode, &rTextNode));
    for (SwTextNode * pTmp : nodes)
    {
        pRet->listener.StartListening(pTmp);
    }
    rFrame.EndListeningAll();
    return pRet;
}

} // namespace sw

void SwAttrIter::InitFontAndAttrHandler(SwTextNode const& rTextNode,
        OUString const& rText,
        bool const*const pbVertLayout)
{
    // Build a font matching the default paragraph style:
    SwFontAccess aFontAccess( &rTextNode.GetAnyFormatColl(), m_pViewShell );
    // It is possible that Init is called more than once, e.g., in a
    // SwTextFrame::FormatOnceMore situation or (since sw_redlinehide)
    // from SwAttrIter::Seek(); in the latter case SwTextSizeInfo::m_pFnt
    // is an alias of m_pFont so it must not be deleted!
    if (m_pFont)
    {
        *m_pFont = aFontAccess.Get()->GetFont();
    }
    else
    {
        m_pFont = new SwFont( aFontAccess.Get()->GetFont() );
    }

    // set font to vertical if frame layout is vertical
    // if it's a re-init, the vert flag never changes
    if (pbVertLayout ? *pbVertLayout : m_aAttrHandler.IsVertLayout())
    {
        m_pFont->SetVertical( m_pFont->GetOrientation(), true );
    }

    // Initialize the default attribute of the attribute handler
    // based on the attribute array cached together with the font.
    // If any further attributes for the paragraph are given in pAttrSet
    // consider them during construction of the default array, and apply
    // them to the font
    m_aAttrHandler.Init(aFontAccess.Get()->GetDefault(), rTextNode.GetpSwAttrSet(),
           *rTextNode.getIDocumentSettingAccess(), m_pViewShell, *m_pFont,
           pbVertLayout ? *pbVertLayout : m_aAttrHandler.IsVertLayout() );

    m_aMagicNo[SwFontScript::Latin] = m_aMagicNo[SwFontScript::CJK] = m_aMagicNo[SwFontScript::CTL] = nullptr;

    assert(g_pBreakIt && g_pBreakIt->GetBreakIter().is());

    m_pFont->SetActual( m_pScriptInfo->WhichFont(TextFrameIndex(0)) );

    TextFrameIndex nChg(0);
    size_t nCnt = 0;

    do
    {
        if ( nCnt >= m_pScriptInfo->CountScriptChg() )
            break;
        nChg = m_pScriptInfo->GetScriptChg( nCnt );
        SwFontScript nTmp = SW_SCRIPTS;
        switch ( m_pScriptInfo->GetScriptType( nCnt++ ) ) {
            case i18n::ScriptType::ASIAN :
                if( !m_aMagicNo[SwFontScript::CJK] ) nTmp = SwFontScript::CJK;
                break;
            case i18n::ScriptType::COMPLEX :
                if( !m_aMagicNo[SwFontScript::CTL] ) nTmp = SwFontScript::CTL;
                break;
            default:
                if( !m_aMagicNo[SwFontScript::Latin ] ) nTmp = SwFontScript::Latin;
        }
        if( nTmp < SW_SCRIPTS )
        {
            m_pFont->ChkMagic( m_pViewShell, nTmp );
            m_pFont->GetMagic( m_aMagicNo[ nTmp ], m_aFontIdx[ nTmp ], nTmp );
        }
    }
    while (nChg < TextFrameIndex(rText.getLength()));
}

void SwAttrIter::CtorInitAttrIter(SwTextNode & rTextNode,
        SwScriptInfo & rScriptInfo, SwTextFrame const*const pFrame)
{
    // during HTML-Import it can happen, that no layout exists
    SwRootFrame* pRootFrame = rTextNode.getIDocumentLayoutAccess().GetCurrentLayout();
    m_pViewShell = pRootFrame ? pRootFrame->GetCurrShell() : nullptr;

    m_pScriptInfo = &rScriptInfo;

    // set font to vertical if frame layout is vertical
    bool bVertLayout = false;
    bool bRTL = false;
    if ( pFrame )
    {
        if ( pFrame->IsVertical() )
        {
            bVertLayout = true;
        }
        bRTL = pFrame->IsRightToLeft();
        m_pMergedPara = pFrame->GetMergedPara();
    }

    // determine script changes if not already done for current paragraph
    assert(m_pScriptInfo);
    if (m_pScriptInfo->GetInvalidityA() != TextFrameIndex(COMPLETE_STRING))
         m_pScriptInfo->InitScriptInfo(rTextNode, m_pMergedPara, bRTL);

    InitFontAndAttrHandler(rTextNode,
            m_pMergedPara ? m_pMergedPara->mergedText : rTextNode.GetText(),
            & bVertLayout);

    m_nStartIndex = m_nEndIndex = m_nPosition = m_nChgCnt = 0;
    m_nPropFont = 0;
    SwDoc* pDoc = rTextNode.GetDoc();
    const IDocumentRedlineAccess& rIDRA = rTextNode.getIDocumentRedlineAccess();

    // sw_redlinehide: this is a Ring - pExtInp is the first PaM that's inside
    // the node.  It's not clear whether there can be more than 1 PaM in the
    // Ring, and this code doesn't handle that case; neither did the old code.
    const SwExtTextInput* pExtInp = pDoc->GetExtTextInput( rTextNode );
    if (!pExtInp && m_pMergedPara)
    {
        SwTextNode const* pNode(&rTextNode);
        for (auto const& rExtent : m_pMergedPara->extents)
        {
            if (rExtent.pNode != pNode)
            {
                pNode = rExtent.pNode;
                pExtInp = pDoc->GetExtTextInput(*pNode);
                if (pExtInp)
                    break;
            }
        }
    }
    const bool bShow = IDocumentRedlineAccess::IsShowChanges( rIDRA.GetRedlineFlags() );
    if (pExtInp || m_pMergedPara || bShow)
    {
        SwRedlineTable::size_type nRedlPos = rIDRA.GetRedlinePos( rTextNode, USHRT_MAX );
        if (SwRedlineTable::npos == nRedlPos && m_pMergedPara)
        {
            SwTextNode const* pNode(&rTextNode);
            for (auto const& rExtent : m_pMergedPara->extents)
            {   // note: have to search because extents based only on Delete
                if (rExtent.pNode != pNode)
                {
                    pNode = rExtent.pNode;
                    nRedlPos = rIDRA.GetRedlinePos(*pNode, USHRT_MAX);
                    if (SwRedlineTable::npos != nRedlPos)
                        break;
                }
            }
            // TODO this is true initially but after delete ops it may be false... need to delete m_pMerged somewhere?
            // assert(SwRedlineTable::npos != nRedlPos);
            assert(SwRedlineTable::npos != nRedlPos || m_pMergedPara->extents.size() <= 1);
        }
        if (pExtInp || m_pMergedPara || SwRedlineTable::npos != nRedlPos)
        {
            const std::vector<ExtTextInputAttr> *pArr = nullptr;
            if( pExtInp )
            {
                pArr = &pExtInp->GetAttrs();
                Seek( TextFrameIndex(0) );
            }

            m_pRedline = new SwRedlineItr( rTextNode, *m_pFont, m_aAttrHandler, nRedlPos,
                            m_pMergedPara
                                ? SwRedlineItr::Mode::Hide
                                : bShow
                                    ? SwRedlineItr::Mode::Show
                                    : SwRedlineItr::Mode::Ignore,
                            pArr, pExtInp ? pExtInp->Start() : nullptr);

            if( m_pRedline->IsOn() )
                ++m_nChgCnt;
        }
    }
}

// The Redline-Iterator
// The following information/states exist in RedlineIterator:
//
// m_nFirst is the first index of RedlineTable, which overlaps with the paragraph.
//
// m_nAct is the currently active (if m_bOn is set) or the next possible index.
// m_nStart and m_nEnd give you the borders of the object within the paragraph.
//
// If m_bOn is set, the font has been manipulated according to it.
//
// If m_nAct is set to SwRedlineTable::npos (via Reset()), then currently no
// Redline is active, m_nStart and m_nEnd are invalid.
SwRedlineItr::SwRedlineItr( const SwTextNode& rTextNd, SwFont& rFnt,
                            SwAttrHandler& rAH, sal_Int32 nRed,
                            Mode const mode,
                            const std::vector<ExtTextInputAttr> *pArr,
                            SwPosition const*const pExtInputStart)
    : m_rDoc( *rTextNd.GetDoc() )
    , m_rAttrHandler( rAH )
    , m_nNdIdx( rTextNd.GetIndex() )
    , m_nFirst( nRed )
    , m_nAct( SwRedlineTable::npos )
    , m_bOn( false )
    , m_eMode( mode )
{
    if( pArr )
    {
        assert(pExtInputStart);
        m_pExt = new SwExtend(*pArr, pExtInputStart->nNode.GetIndex(),
                                     pExtInputStart->nContent.GetIndex());
    }
    else
        m_pExt = nullptr;
    assert(m_pExt || m_eMode != Mode::Ignore); // only create if necessary
    Seek(rFnt, m_nNdIdx, 0, COMPLETE_STRING);
}

SwRedlineItr::~SwRedlineItr() COVERITY_NOEXCEPT_FALSE
{
    Clear( nullptr );
    delete m_pExt;
}

// The return value of SwRedlineItr::Seek tells you if the current font
// has been manipulated by leaving (-1) or accessing (+1) of a section
short SwRedlineItr::Seek(SwFont& rFnt,
        sal_uLong const nNode, sal_Int32 const nNew, sal_Int32 const nOld)
{
    short nRet = 0;
    if( ExtOn() )
        return 0; // Abbreviation: if we're within an ExtendTextInputs
                  // there can't be other changes of attributes (not even by redlining)
    assert(m_eMode == Mode::Hide || m_nNdIdx == nNode);
    if (m_eMode == Mode::Show)
    {
        if (m_bOn)
        {
            if (nNew >= m_nEnd)
            {
                --nRet;
                Clear_( &rFnt );    // We go behind the current section
                ++m_nAct;             // and check the next one
            }
            else if (nNew < m_nStart)
            {
                --nRet;
                Clear_( &rFnt );    // We go in front of the current section
                if (m_nAct > m_nFirst)
                    m_nAct = m_nFirst;  // the test has to run from the beginning
                else
                    return nRet + EnterExtend(rFnt, nNode, nNew); // There's none prior to us
            }
            else
                return nRet + EnterExtend(rFnt, nNode, nNew); // We stayed in the same section
        }
        if (SwRedlineTable::npos == m_nAct || nOld > nNew)
            m_nAct = m_nFirst;

        m_nStart = COMPLETE_STRING;
        m_nEnd = COMPLETE_STRING;

        for ( ; m_nAct < m_rDoc.getIDocumentRedlineAccess().GetRedlineTable().size() ; ++m_nAct)
        {
            m_rDoc.getIDocumentRedlineAccess().GetRedlineTable()[ m_nAct ]->CalcStartEnd(m_nNdIdx, m_nStart, m_nEnd);

            if (nNew < m_nEnd)
            {
                if (nNew >= m_nStart) // only possible candidate
                {
                    m_bOn = true;
                    const SwRangeRedline *pRed = m_rDoc.getIDocumentRedlineAccess().GetRedlineTable()[ m_nAct ];

                    if (m_pSet)
                        m_pSet->ClearItem();
                    else
                    {
                        SwAttrPool& rPool =
                            const_cast<SwDoc&>(m_rDoc).GetAttrPool();
                        m_pSet = o3tl::make_unique<SfxItemSet>(rPool, svl::Items<RES_CHRATR_BEGIN, RES_CHRATR_END-1>{});
                    }

                    if( 1 < pRed->GetStackCount() )
                        FillHints( pRed->GetAuthor( 1 ), pRed->GetType( 1 ) );
                    FillHints( pRed->GetAuthor(), pRed->GetType() );

                    SfxWhichIter aIter( *m_pSet );
                    sal_uInt16 nWhich = aIter.FirstWhich();
                    while( nWhich )
                    {
                        const SfxPoolItem* pItem;
                        if( ( nWhich < RES_CHRATR_END ) &&
                            ( SfxItemState::SET == m_pSet->GetItemState( nWhich, true, &pItem ) ) )
                        {
                            SwTextAttr* pAttr = MakeRedlineTextAttr(
                                const_cast<SwDoc&>(m_rDoc),
                                *const_cast<SfxPoolItem*>(pItem) );
                            pAttr->SetPriorityAttr( true );
                            m_Hints.push_back(pAttr);
                            m_rAttrHandler.PushAndChg( *pAttr, rFnt );
                        }
                        nWhich = aIter.NextWhich();
                    }

                    ++nRet;
                }
                break;
            }
            m_nStart = COMPLETE_STRING;
            m_nEnd = COMPLETE_STRING;
        }
    }
    else if (m_eMode == Mode::Hide)
    {   // ... just iterate to update m_nAct for GetNextRedln();
        // there is no need to care about formatting in this mode
        if (m_nAct == SwRedlineTable::npos || nOld == COMPLETE_STRING)
        {   // reset, or move backward
            m_nAct = m_nFirst;
        }
        for ( ; m_nAct < m_rDoc.getIDocumentRedlineAccess().GetRedlineTable().size(); ++m_nAct)
        {   // only Start matters in this mode
            // Seeks until it finds a RL that starts at or behind the seek pos.
            // - then update m_nStart/m_nEnd to the intersection of it with the
            // current node (if any).
            // The only way to skip to a different node is if there is a Delete
            // RL, so if there is no intersection we'll never skip again.
            // Note: here, assume that delete can't nest inside delete!
            SwRangeRedline const*const pRedline(
                m_rDoc.getIDocumentRedlineAccess().GetRedlineTable()[m_nAct]);
            SwPosition const*const pStart(pRedline->Start());
            if (pRedline->GetType() == nsRedlineType_t::REDLINE_DELETE
                && (nNode < pStart->nNode.GetIndex()
                    || (nNode == pStart->nNode.GetIndex()
                        && nNew <= pStart->nContent.GetIndex())))
            {
                pRedline->CalcStartEnd(nNode, m_nStart, m_nEnd);
                break;
            }
            m_nStart = COMPLETE_STRING;
            m_nEnd = COMPLETE_STRING;
        }
    }
    return nRet + EnterExtend(rFnt, nNode, nNew);
}

void SwRedlineItr::FillHints( std::size_t nAuthor, RedlineType_t eType )
{
    switch ( eType )
    {
        case nsRedlineType_t::REDLINE_INSERT:
            SW_MOD()->GetInsertAuthorAttr(nAuthor, *m_pSet);
            break;
        case nsRedlineType_t::REDLINE_DELETE:
            SW_MOD()->GetDeletedAuthorAttr(nAuthor, *m_pSet);
            break;
        case nsRedlineType_t::REDLINE_FORMAT:
        case nsRedlineType_t::REDLINE_FMTCOLL:
            SW_MOD()->GetFormatAuthorAttr(nAuthor, *m_pSet);
            break;
        default:
            break;
    }
}

void SwRedlineItr::ChangeTextAttr( SwFont* pFnt, SwTextAttr const &rHt, bool bChg )
{
    OSL_ENSURE( IsOn(), "SwRedlineItr::ChangeTextAttr: Off?" );

    if (m_eMode != Mode::Show && !m_pExt)
        return;

    if( bChg )
    {
        if (m_pExt && m_pExt->IsOn())
            m_rAttrHandler.PushAndChg( rHt, *m_pExt->GetFont() );
        else
            m_rAttrHandler.PushAndChg( rHt, *pFnt );
    }
    else
    {
        OSL_ENSURE( ! m_pExt || ! m_pExt->IsOn(), "Pop of attribute during opened extension" );
        m_rAttrHandler.PopAndChg( rHt, *pFnt );
    }
}

void SwRedlineItr::Clear_( SwFont* pFnt )
{
    OSL_ENSURE( m_bOn, "SwRedlineItr::Clear: Off?" );
    m_bOn = false;
    for (auto const& hint : m_Hints)
    {
        if( pFnt )
            m_rAttrHandler.PopAndChg( *hint, *pFnt );
        else
            m_rAttrHandler.Pop( *hint );
        SwTextAttr::Destroy(hint, const_cast<SwDoc&>(m_rDoc).GetAttrPool() );
    }
    m_Hints.clear();
}

/// Ignore mode: does nothing.
/// Show mode: returns end of redline if currently in one, or start of next
/// Hide mode: returns start of next redline in current node, plus (if it's a
///            Delete) its end position and number of consecutive RLs
std::pair<sal_Int32, std::pair<SwRangeRedline const*, size_t>>
SwRedlineItr::GetNextRedln(sal_Int32 nNext, SwTextNode const*const pNode,
        SwRedlineTable::size_type & rAct)
{
    sal_Int32 nStart(m_nStart);
    sal_Int32 nEnd(m_nEnd);
    nNext = NextExtend(pNode->GetIndex(), nNext);
    if (m_eMode == Mode::Ignore || SwRedlineTable::npos == m_nFirst)
        return std::make_pair(nNext, std::make_pair(nullptr, 0));
    if (SwRedlineTable::npos == rAct)
    {
        rAct = m_nFirst;
    }
    if (rAct != m_nAct)
    {
        while (rAct < m_rDoc.getIDocumentRedlineAccess().GetRedlineTable().size())
        {
            SwRangeRedline const*const pRedline(
                    m_rDoc.getIDocumentRedlineAccess().GetRedlineTable()[rAct]);
            pRedline->CalcStartEnd(pNode->GetIndex(), nStart, nEnd);
            if (m_eMode != Mode::Hide
                || pRedline->GetType() == nsRedlineType_t::REDLINE_DELETE)
            {
                break;
            }
            ++rAct; // Hide mode: search a Delete RL
        }
    }
    if (rAct == m_rDoc.getIDocumentRedlineAccess().GetRedlineTable().size())
    {
        return std::make_pair(nNext, std::make_pair(nullptr, 0)); // no Delete here
    }
    if (m_bOn || (m_eMode == Mode::Show && nStart == 0))
    {   // in Ignore mode, the end of redlines isn't relevant, except as returned in the second in the pair!
        if (nEnd < nNext)
            nNext = nEnd;
    }
    else if (nStart <= nNext)
    {
        if (m_eMode == Mode::Show)
        {
            nNext = nStart;
        }
        else
        {
            assert(m_eMode == Mode::Hide);
            SwRangeRedline const* pRedline(
                    m_rDoc.getIDocumentRedlineAccess().GetRedlineTable()[rAct]);
            assert(pRedline->GetType() == nsRedlineType_t::REDLINE_DELETE); //?
            if (pRedline->GetType() == nsRedlineType_t::REDLINE_DELETE)
            {
                nNext = nStart;
                size_t nSkipped(1); // (consecutive) candidates to be skipped
                while (rAct + nSkipped <
                       m_rDoc.getIDocumentRedlineAccess().GetRedlineTable().size())
                {
                    SwRangeRedline const*const pNext =
                        m_rDoc.getIDocumentRedlineAccess().GetRedlineTable()[rAct + nSkipped];
                    if (pRedline->End() < pNext->Start())
                    {
                        break; // done for now
                    }
                    else if (pNext->Start() == pRedline->End() &&
                            pNext->GetType() == nsRedlineType_t::REDLINE_DELETE)
                    {
                        // consecutive delete - continue
                        pRedline = pNext;
                    }
                    ++nSkipped;
                }
                return std::make_pair(nNext, std::make_pair(pRedline, nSkipped));
            }
        }
    }
    return std::make_pair(nNext, std::make_pair(nullptr, 0));
}

bool SwRedlineItr::ChkSpecialUnderline_() const
{
    // If the underlining or the escapement is caused by redlining,
    // we always apply the SpecialUnderlining, i.e. the underlining
    // below the base line
    for (SwTextAttr* pHint : m_Hints)
    {
        const sal_uInt16 nWhich = pHint->Which();
        if( RES_CHRATR_UNDERLINE == nWhich ||
            RES_CHRATR_ESCAPEMENT == nWhich )
            return true;
    }
    return false;
}

bool SwRedlineItr::CheckLine(
        sal_uLong const nStartNode, sal_Int32 const nChkStart,
        sal_uLong const nEndNode, sal_Int32 nChkEnd)
{
    // note: previously this would return true in the (!m_bShow && m_pExt)
    // case, but surely that was a bug?
    if (m_nFirst == SwRedlineTable::npos || m_eMode != Mode::Show)
        return false;
    assert(nStartNode == nEndNode); (void) nStartNode; (void) nEndNode;
    if( nChkEnd == nChkStart ) // empty lines look one char further
        ++nChkEnd;
    sal_Int32 nOldStart = m_nStart;
    sal_Int32 nOldEnd = m_nEnd;
    SwRedlineTable::size_type const nOldAct = m_nAct;
    bool bRet = false;

    for (m_nAct = m_nFirst; m_nAct < m_rDoc.getIDocumentRedlineAccess().GetRedlineTable().size(); ++m_nAct)
    {
        m_rDoc.getIDocumentRedlineAccess().GetRedlineTable()[ m_nAct ]->CalcStartEnd( m_nNdIdx, m_nStart, m_nEnd );
        if (nChkEnd < m_nStart)
            break;
        if (nChkStart <= m_nEnd && (nChkEnd > m_nStart || COMPLETE_STRING == m_nEnd))
        {
            bRet = true;
            break;
        }
    }

    m_nStart = nOldStart;
    m_nEnd = nOldEnd;
    m_nAct = nOldAct;
    return bRet;
}

void SwExtend::ActualizeFont( SwFont &rFnt, ExtTextInputAttr nAttr )
{
    if ( nAttr & ExtTextInputAttr::Underline )
        rFnt.SetUnderline( LINESTYLE_SINGLE );
    else if ( nAttr & ExtTextInputAttr::BoldUnderline )
        rFnt.SetUnderline( LINESTYLE_BOLD );
    else if ( nAttr & ExtTextInputAttr::DottedUnderline )
        rFnt.SetUnderline( LINESTYLE_DOTTED );
    else if ( nAttr & ExtTextInputAttr::DashDotUnderline )
        rFnt.SetUnderline( LINESTYLE_DOTTED );

    if ( nAttr & ExtTextInputAttr::RedText )
        rFnt.SetColor( COL_RED );

    if ( nAttr & ExtTextInputAttr::Highlight )
    {
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
        rFnt.SetColor( rStyleSettings.GetHighlightTextColor() );
        rFnt.SetBackColor( new Color( rStyleSettings.GetHighlightColor() ) );
    }
    if ( nAttr & ExtTextInputAttr::GrayWaveline )
        rFnt.SetGreyWave( true );
}

short SwExtend::Enter(SwFont& rFnt, sal_uLong const nNode, sal_Int32 const nNew)
{
    OSL_ENSURE( !m_pFont, "SwExtend: Enter with Font" );
    if (nNode != m_nNode)
        return 0;
    OSL_ENSURE( !Inside(), "SwExtend: Enter without Leave" );
    m_nPos = nNew;
    if( Inside() )
    {
        m_pFont.reset( new SwFont(rFnt) );
        ActualizeFont( rFnt, m_rArr[m_nPos - m_nStart] );
        return 1;
    }
    return 0;
}

bool SwExtend::Leave_(SwFont& rFnt, sal_uLong const nNode, sal_Int32 const nNew)
{
    OSL_ENSURE(nNode == m_nNode && Inside(), "SwExtend: Leave without Enter");
    if (nNode != m_nNode)
        return true;
    const ExtTextInputAttr nOldAttr = m_rArr[m_nPos - m_nStart];
    m_nPos = nNew;
    if( Inside() )
    {   // We stayed within the ExtendText-section
        const ExtTextInputAttr nAttr = m_rArr[m_nPos - m_nStart];
        if( nOldAttr != nAttr ) // Is there an (inner) change of attributes?
        {
            rFnt = *m_pFont;
            ActualizeFont( rFnt, nAttr );
        }
    }
    else
    {
        rFnt = *m_pFont;
        m_pFont.reset();
        return true;
    }
    return false;
}

sal_Int32 SwExtend::Next(sal_uLong const nNode, sal_Int32 nNext)
{
    if (nNode != m_nNode)
        return nNext;
    if (m_nPos < m_nStart)
    {
        if (nNext > m_nStart)
            nNext = m_nStart;
    }
    else if (m_nPos < m_nEnd)
    {
        sal_Int32 nIdx = m_nPos - m_nStart;
        const ExtTextInputAttr nAttr = m_rArr[ nIdx ];
        while (static_cast<size_t>(++nIdx) < m_rArr.size() && nAttr == m_rArr[nIdx])
            ; //nothing
        nIdx = nIdx + m_nStart;
        if( nNext > nIdx )
            nNext = nIdx;
    }
    return nNext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
