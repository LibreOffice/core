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

#include <sal/config.h>

#include <algorithm>

#include <hintids.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/cmapitem.hxx>
#include <svl/itemiter.hxx>
#include <svx/svdobj.hxx>
#include <vcl/svapp.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <fmtornt.hxx>
#include <fmtflcnt.hxx>
#include <fmtcntnt.hxx>
#include <fmtftn.hxx>
#include <fmtpdsc.hxx>
#include <frmatr.hxx>
#include <frmfmt.hxx>
#include <fmtfld.hxx>
#include <doc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentSettingAccess.hxx>
#include <txatbase.hxx>
#include <viewsh.hxx>
#include <rootfrm.hxx>
#include <docary.hxx>
#include <ndtxt.hxx>
#include <fldbas.hxx>
#include <pam.hxx>
#include "itratr.hxx"
#include <htmltbl.hxx>
#include <swtable.hxx>
#include "redlnitr.hxx"
#include <redline.hxx>
#include <fmtsrnd.hxx>
#include "itrtxt.hxx"
#include <breakit.hxx>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <editeng/lrspitem.hxx>
#include <calbck.hxx>
#include <frameformats.hxx>
#include <sortedobjs.hxx>
#include <anchoredobject.hxx>
#include <flyfrm.hxx>
#include <flyfrms.hxx>

using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star;

static sal_Int32 GetNextAttrImpl(SwTextNode const* pTextNode,
        size_t nStartIndex, size_t nEndIndex, sal_Int32 nPosition);

SwAttrIter::SwAttrIter(SwTextNode const * pTextNode)
    : m_pViewShell(nullptr)
    , m_pFont(nullptr)
    , m_pScriptInfo(nullptr)
    , m_pLastOut(nullptr)
    , m_nChgCnt(0)
    , m_nStartIndex(0)
    , m_nEndIndex(0)
    , m_nPosition(0)
    , m_nPropFont(0)
    , m_pTextNode(pTextNode)
    , m_pMergedPara(nullptr)
{
    m_aFontCacheIds[SwFontScript::Latin] = m_aFontCacheIds[SwFontScript::CJK] = m_aFontCacheIds[SwFontScript::CTL] = nullptr;
}

SwAttrIter::SwAttrIter(SwTextNode& rTextNode, SwScriptInfo& rScrInf, SwTextFrame const*const pFrame)
    : m_pViewShell(nullptr)
    , m_pFont(nullptr)
    , m_pScriptInfo(nullptr)
    , m_pLastOut(nullptr)
    , m_nChgCnt(0)
    , m_nPropFont(0)
    , m_pTextNode(&rTextNode)
    , m_pMergedPara(nullptr)
{
    CtorInitAttrIter(rTextNode, rScrInf, pFrame);
}

void SwAttrIter::Chg( SwTextAttr const *pHt )
{
    assert(pHt && m_pFont && "No attribute of font available for change");
    if( m_pRedline && m_pRedline->IsOn() )
        m_pRedline->ChangeTextAttr( m_pFont, *pHt, true );
    else
        m_aAttrHandler.PushAndChg( *pHt, *m_pFont );
    m_nChgCnt++;
}

void SwAttrIter::Rst( SwTextAttr const *pHt )
{
    assert(pHt && m_pFont && "No attribute of font available for reset");
    // get top from stack after removing pHt
    if( m_pRedline && m_pRedline->IsOn() )
        m_pRedline->ChangeTextAttr( m_pFont, *pHt, false );
    else
        m_aAttrHandler.PopAndChg( *pHt, *m_pFont );
    m_nChgCnt--;
}

SwAttrIter::~SwAttrIter()
{
    m_pRedline.reset();
    delete m_pFont;
}

bool SwAttrIter::MaybeHasHints() const
{
    return nullptr != m_pTextNode->GetpSwpHints() || nullptr != m_pMergedPara;
}

/**
 * Returns the attribute for a position
 *
 * Only if the attribute is exactly at the position @param nPos and
 * does not have an EndIndex
 *
 * We need this function for attributes which should alter formatting without
 * changing the content of the string.
 * Such "degenerated" attributes are e.g.: fields which retain expanded text and
 * line-bound Frames.
 * In order to avoid ambiguities between different such attributes, we insert a
 * special character at the start of the string, when creating such an attribute.
 * The Formatter later on encounters such a special character and retrieves the
 * degenerate attribute via GetAttr().
 */
SwTextAttr *SwAttrIter::GetAttr(TextFrameIndex const nPosition) const
{
    std::pair<SwTextNode const*, sal_Int32> const pos( m_pMergedPara
        ? sw::MapViewToModel(*m_pMergedPara, nPosition)
        : std::make_pair(m_pTextNode, sal_Int32(nPosition)));
    return pos.first->GetTextAttrForCharAt(pos.second);
}

bool SwAttrIter::SeekAndChgAttrIter(TextFrameIndex const nNewPos, OutputDevice* pOut)
{
    std::pair<SwTextNode const*, sal_Int32> const pos{SeekNewPos(nNewPos, nullptr)};
    bool bChg = m_nStartIndex && pos.first == m_pTextNode && pos.second == m_nPosition
        ? m_pFont->IsFntChg()
        : Seek( nNewPos );
    if ( m_pLastOut.get() != pOut )
    {
        m_pLastOut = pOut;
        m_pFont->SetFntChg( true );
        bChg = true;
    }
    if( bChg )
    {
        // if the change counter is zero, we know the cache id of the wanted font
        if ( !m_nChgCnt && !m_nPropFont )
            m_pFont->SetFontCacheId( m_aFontCacheIds[ m_pFont->GetActual() ],
                m_aFontIdx[ m_pFont->GetActual() ], m_pFont->GetActual() );
        m_pFont->ChgPhysFnt( m_pViewShell, *pOut );
    }

    return bChg;
}

bool SwAttrIter::IsSymbol(TextFrameIndex const nNewPos)
{
    Seek( nNewPos );
    if ( !m_nChgCnt && !m_nPropFont )
        m_pFont->SetFontCacheId( m_aFontCacheIds[ m_pFont->GetActual() ],
            m_aFontIdx[ m_pFont->GetActual() ], m_pFont->GetActual() );
    return m_pFont->IsSymbol( m_pViewShell );
}

bool SwTextFrame::IsSymbolAt(TextFrameIndex const nPos) const
{
    SwTextInfo info(const_cast<SwTextFrame*>(this));
    SwTextIter iter(const_cast<SwTextFrame*>(this), &info);
    return iter.IsSymbol(nPos);
}

bool SwAttrIter::SeekStartAndChgAttrIter( OutputDevice* pOut, const bool bParaFont )
{
    SwTextNode const*const pFirstTextNode(m_pMergedPara ? m_pMergedPara->pFirstNode : m_pTextNode);
    if ( m_pRedline && m_pRedline->ExtOn() )
        m_pRedline->LeaveExtend(*m_pFont, pFirstTextNode->GetIndex(), 0);

    if (m_pTextNode != pFirstTextNode)
    {
        assert(m_pMergedPara);
        m_pTextNode = m_pMergedPara->pFirstNode;
        InitFontAndAttrHandler(*m_pMergedPara->pParaPropsNode, *m_pTextNode,
                               m_pMergedPara->mergedText, nullptr, nullptr);
    }

    // reset font to its original state
    m_aAttrHandler.Reset();
    m_aAttrHandler.ResetFont( *m_pFont );

    m_nStartIndex = 0;
    m_nEndIndex = 0;
    m_nPosition = 0;
    m_nChgCnt = 0;
    if( m_nPropFont )
        m_pFont->SetProportion( m_nPropFont );
    if( m_pRedline )
    {
        m_pRedline->Clear( m_pFont );
        if( !bParaFont )
            m_nChgCnt = m_nChgCnt + m_pRedline->Seek(*m_pFont, pFirstTextNode->GetIndex(), 0, COMPLETE_STRING);
        else
            m_pRedline->Reset();
    }

    SwpHints const*const pHints(m_pTextNode->GetpSwpHints());
    if (pHints && !bParaFont)
    {
        SwTextAttr *pTextAttr;
        // While we've not reached the end of the StartArray && the TextAttribute starts at position 0...
        while ((m_nStartIndex < pHints->Count()) &&
               !((pTextAttr = pHints->Get(m_nStartIndex))->GetStart()))
        {
            // open the TextAttributes
            Chg( pTextAttr );
            m_nStartIndex++;
        }
    }

    bool bChg = m_pFont->IsFntChg();
    if ( m_pLastOut.get() != pOut )
    {
        m_pLastOut = pOut;
        m_pFont->SetFntChg( true );
        bChg = true;
    }
    if( bChg )
    {
        // if the application counter is zero, we know the cache id of the wanted font
        if ( !m_nChgCnt && !m_nPropFont )
            m_pFont->SetFontCacheId( m_aFontCacheIds[ m_pFont->GetActual() ],
                m_aFontIdx[ m_pFont->GetActual() ], m_pFont->GetActual() );
        m_pFont->ChgPhysFnt( m_pViewShell, *pOut );
    }
    return bChg;
}

// AMA: New AttrIter Nov 94
void SwAttrIter::SeekFwd(const sal_Int32 nOldPos, const sal_Int32 nNewPos)
{
    SwpHints const*const pHints(m_pTextNode->GetpSwpHints());
    SwTextAttr *pTextAttr;
    const auto nHintsCount = pHints->Count();

    if ( m_nStartIndex ) // If attributes have been opened at all ...
    {
        // Close attributes that are currently open, but stop at nNewPos+1

        // As long as we've not yet reached the end of EndArray and the
        // TextAttribute ends before or at the new position ...
        while ((m_nEndIndex < nHintsCount) &&
               ((pTextAttr = pHints->GetSortedByEnd(m_nEndIndex))->GetAnyEnd() <= nNewPos))
        {
            // Close the TextAttributes, whose StartPos were before or at
            // the old nPos and are currently open
            if (pTextAttr->GetStart() <= nOldPos)  Rst( pTextAttr );
            m_nEndIndex++;
        }
    }
    else // skip the not opened ends
    {
        while ((m_nEndIndex < nHintsCount) &&
               (pHints->GetSortedByEnd(m_nEndIndex)->GetAnyEnd() <= nNewPos))
        {
            m_nEndIndex++;
        }
    }

    // As long as we've not yet reached the end of EndArray and the
    // TextAttribute ends before or at the new position...
    while ((m_nStartIndex < nHintsCount) &&
            ((pTextAttr = pHints->Get(m_nStartIndex))->GetStart() <= nNewPos))
    {

        // open the TextAttributes, whose ends lie behind the new position
        if ( pTextAttr->GetAnyEnd() > nNewPos )  Chg( pTextAttr );
        m_nStartIndex++;
    }
}

void SwAttrIter::SeekToEnd()
{
    if (m_pTextNode->GetDoc().getIDocumentSettingAccess().get(
            DocumentSettingId::APPLY_PARAGRAPH_MARK_FORMAT_TO_EMPTY_LINE_AT_END_OF_PARAGRAPH))
    {
        SfxItemPool & rPool{const_cast<SwAttrPool&>(m_pTextNode->GetDoc().GetAttrPool())};
        SwFormatAutoFormat const& rListAutoFormat{m_pTextNode->GetAttr(RES_PARATR_LIST_AUTOFMT)};
        std::shared_ptr<SfxItemSet> const pSet{rListAutoFormat.GetStyleHandle()};
        if (!pSet)
        {
            return;
        }
        if (pSet->HasItem(RES_TXTATR_CHARFMT))
        {
            SwFormatCharFormat const& rCharFormat{pSet->Get(RES_TXTATR_CHARFMT)};
            m_pEndCharFormatAttr.reset(new SwTextAttrEnd{
                    SfxPoolItemHolder{rPool, &rCharFormat}, -1, -1});
            Chg(m_pEndCharFormatAttr.get());
        }
        // note: RES_TXTATR_CHARFMT should be cleared here but it looks like
        // SwAttrHandler only looks at RES_CHRATR_* anyway
        m_pEndAutoFormatAttr.reset(new SwTextAttrEnd{
                SfxPoolItemHolder{rPool, &rListAutoFormat}, -1, -1});
        Chg(m_pEndAutoFormatAttr.get());
    }
}

std::pair<SwTextNode const*, sal_Int32>
SwAttrIter::SeekNewPos(TextFrameIndex const nNewPos, bool *const o_pIsToEnd)
{
    std::pair<SwTextNode const*, sal_Int32> newPos{ m_pMergedPara
        ? sw::MapViewToModel(*m_pMergedPara, nNewPos)
        : std::make_pair(m_pTextNode, sal_Int32(nNewPos))};

    bool isToEnd{false};
    if (m_pMergedPara)
    {
        if (m_pMergedPara->extents.empty())
        {
            isToEnd = true;
            assert(m_pMergedPara->pLastNode == newPos.first);
        }
        else
        {
            auto const& rLast{m_pMergedPara->extents.back()};
            isToEnd = rLast.pNode == newPos.first && rLast.nEnd == newPos.second;
            // for text formatting: use *last* node if all text is hidden
            if (isToEnd
                && m_pMergedPara->pLastNode != newPos.first // implies there is hidden text
                && m_pViewShell->GetLayout()->GetParagraphBreakMode() == sw::ParagraphBreakMode::Hidden
                && m_pTextNode->GetDoc().getIDocumentSettingAccess().get(
                    DocumentSettingId::APPLY_PARAGRAPH_MARK_FORMAT_TO_EMPTY_LINE_AT_END_OF_PARAGRAPH))
            {
                TextFrameIndex nHiddenStart(COMPLETE_STRING);
                TextFrameIndex nHiddenEnd(0);
                m_pScriptInfo->GetBoundsOfHiddenRange(TextFrameIndex(0), nHiddenStart, nHiddenEnd);
                if (TextFrameIndex(0) == nHiddenStart
                    && TextFrameIndex(m_pMergedPara->mergedText.getLength()) <= nHiddenEnd)
                {
                    newPos.first = m_pMergedPara->pLastNode;
                    newPos.second = m_pMergedPara->pLastNode->Len();
                }
            }
        }
    }
    else
    {
        isToEnd = newPos.second == m_pTextNode->Len();
    }
    if (o_pIsToEnd)
    {
        *o_pIsToEnd = isToEnd;
    }

    return newPos;
}

bool SwAttrIter::Seek(TextFrameIndex const nNewPos)
{
    // note: nNewPos isn't necessarily an index returned from GetNextAttr
    bool isToEnd{false};
    std::pair<SwTextNode const*, sal_Int32> const newPos{SeekNewPos(nNewPos, &isToEnd)};

    if ( m_pRedline && m_pRedline->ExtOn() )
        m_pRedline->LeaveExtend(*m_pFont, newPos.first->GetIndex(), newPos.second);
    if (m_pTextNode->GetIndex() < newPos.first->GetIndex())
    {
        // Skipping to a different node - first seek until the end of this node
        // to get rid of all hint items
        if (m_pTextNode->GetpSwpHints())
        {
            sal_Int32 nPos(m_nPosition);
            do
            {
                sal_Int32 const nOldPos(nPos);
                nPos = GetNextAttrImpl(m_pTextNode, m_nStartIndex, m_nEndIndex, nPos);
                if (nPos <= m_pTextNode->Len())
                {
                    SeekFwd(nOldPos, nPos);
                }
                else
                {
                    SeekFwd(nOldPos, m_pTextNode->Len());
                }
            }
            while (nPos < m_pTextNode->Len());
        }
        // Unapply current para items:
        // the SwAttrHandler doesn't appear to be capable of *unapplying*
        // items at all; it can only apply a previously effective item.
        // So do this by recreating the font from scratch.
        // Apply new para items:
        assert(m_pMergedPara);
        InitFontAndAttrHandler(*m_pMergedPara->pParaPropsNode, *newPos.first,
                               m_pMergedPara->mergedText, nullptr, nullptr);
        // reset to next
        m_pTextNode = newPos.first;
        m_nStartIndex = 0;
        m_nEndIndex = 0;
        m_nPosition = 0;
        assert(m_pRedline);
    }

    // sw_redlinehide: Seek(0) must move before the first character, which
    // has a special case where the first node starts with delete redline.
    if ((!nNewPos && !m_pMergedPara)
        || newPos.first != m_pTextNode
        || newPos.second < m_nPosition)
    {
        if (m_pMergedPara)
        {
            if (m_pTextNode != newPos.first)
            {
                m_pTextNode = newPos.first;
                // sw_redlinehide: hope it's okay to use the current text node
                // here; the AttrHandler shouldn't care about non-char items
                InitFontAndAttrHandler(*m_pMergedPara->pParaPropsNode, *m_pTextNode,
                                       m_pMergedPara->mergedText, nullptr, nullptr);
            }
        }
        // also reset it if the RES_PARATR_LIST_AUTOFMT has been applied!
        if (m_pMergedPara || m_pTextNode->GetpSwpHints() || m_pEndAutoFormatAttr)
        {
            if( m_pRedline )
                m_pRedline->Clear( nullptr );

            // reset font to its original state
            m_aAttrHandler.Reset();
            m_aAttrHandler.ResetFont( *m_pFont );
            m_pEndCharFormatAttr.reset();
            m_pEndAutoFormatAttr.reset();

            if( m_nPropFont )
                m_pFont->SetProportion( m_nPropFont );
            m_nStartIndex = 0;
            m_nEndIndex = 0;
            m_nPosition = 0;
            m_nChgCnt = 0;

            // Attention!
            // resetting the font here makes it necessary to apply any
            // changes for extended input directly to the font
            if ( m_pRedline && m_pRedline->ExtOn() )
            {
                m_pRedline->UpdateExtFont( *m_pFont );
                ++m_nChgCnt;
            }
        }
    }

    if (m_pTextNode->GetpSwpHints())
    {
        if (m_pMergedPara)
        {
            // iterate hint by hint: SeekFwd does not mix ends and starts,
            // it always applies all the starts last, so it must be called once
            // per position where hints start/end!
            sal_Int32 nPos(m_nPosition);
            do
            {
                sal_Int32 const nOldPos(nPos);
                nPos = GetNextAttrImpl(m_pTextNode, m_nStartIndex, m_nEndIndex, nPos);
                if (nPos <= newPos.second)
                {
                    SeekFwd(nOldPos, nPos);
                }
                else
                {
                    SeekFwd(nOldPos, newPos.second);
                }
            }
            while (nPos < newPos.second);
        }
        else
        {
            SeekFwd(m_nPosition, newPos.second);
        }
    }

    if (isToEnd && !m_pEndAutoFormatAttr)
    {
        SeekToEnd();
    }

    m_pFont->SetActual( m_pScriptInfo->WhichFont(nNewPos) );

    if( m_pRedline )
        m_nChgCnt = m_nChgCnt + m_pRedline->Seek(*m_pFont, m_pTextNode->GetIndex(), newPos.second, m_nPosition);
    m_nPosition = newPos.second;

    if( m_nPropFont )
        m_pFont->SetProportion( m_nPropFont );

    return m_pFont->IsFntChg();
}

static void InsertCharAttrs(SfxPoolItem const** pAttrs, SfxItemSet const& rItems)
{
    SfxItemIter iter(rItems);
    for (SfxPoolItem const* pItem = iter.GetCurItem(); pItem; pItem = iter.NextItem())
    {
        auto const nWhich(pItem->Which());
        if (isCHRATR(nWhich) && RES_CHRATR_RSID != nWhich)
        {
            pAttrs[nWhich - RES_CHRATR_BEGIN] = pItem;
        }
        else if (nWhich == RES_TXTATR_UNKNOWN_CONTAINER)
        {
            pAttrs[RES_CHRATR_END - RES_CHRATR_BEGIN] = pItem;
        }
    }
}

// if return false: portion ends at start of redline, indexes unchanged
// if return true: portion end not known (past end of redline), indexes point to first hint past end of redline
static bool CanSkipOverRedline(
        SwTextNode const& rStartNode, sal_Int32 const nStartRedline,
        SwRangeRedline const& rRedline,
        size_t & rStartIndex, size_t & rEndIndex,
        bool const isTheAnswerYes)
{
    size_t nStartIndex(rStartIndex);
    size_t nEndIndex(rEndIndex);
    SwPosition const*const pRLEnd(rRedline.End());
    if (!pRLEnd->GetNode().IsTextNode() // if fully deleted...
        || pRLEnd->GetContentIndex() == pRLEnd->GetNode().GetTextNode()->Len())
    {
        // shortcut: nothing follows redline
        // current state is end state
        return false;
    }
    std::vector<SwTextAttr*> activeCharFmts;
    // can't compare the SwFont that's stored somewhere, it doesn't have compare
    // operator, so try to recreate the situation with some temp arrays here
    SfxPoolItem const* activeCharAttrsStart[RES_CHRATR_END - RES_CHRATR_BEGIN + 1] = { nullptr, };
    if (rStartNode != pRLEnd->GetNode())
    {   // nodes' attributes are only needed if there are different nodes
        InsertCharAttrs(activeCharAttrsStart, rStartNode.GetSwAttrSet());
    }
    if (SwpHints const*const pStartHints = rStartNode.GetpSwpHints())
    {
        // check hint ends of hints that start before and end within
        sal_Int32 const nRedlineEnd(rStartNode == pRLEnd->GetNode()
                ? pRLEnd->GetContentIndex()
                : rStartNode.Len());
        for ( ; nEndIndex < pStartHints->Count(); ++nEndIndex)
        {
            SwTextAttr *const pAttr(pStartHints->GetSortedByEnd(nEndIndex));
            if (!pAttr->End())
            {
                continue;
            }
            if (nRedlineEnd < *pAttr->End())
            {
                break;
            }
            if (nStartRedline <= pAttr->GetStart())
            {
                continue;
            }
            if (pAttr->IsFormatIgnoreEnd())
            {
                continue;
            }
            switch (pAttr->Which())
            {
                // if any of these ends inside RL then we need a new portion
                case RES_TXTATR_REFMARK:
                case RES_TXTATR_TOXMARK:
                case RES_TXTATR_META: // actually these 2 aren't allowed to overlap ???
                case RES_TXTATR_METAFIELD:
                case RES_TXTATR_INETFMT:
                case RES_TXTATR_CJK_RUBY:
                case RES_TXTATR_INPUTFIELD:
                case RES_TXTATR_CONTENTCONTROL:
                    {
                        if (!isTheAnswerYes) return false; // always break
                    }
                    break;
                // these are guaranteed not to overlap
                // and come in order of application
                case RES_TXTATR_AUTOFMT:
                case RES_TXTATR_CHARFMT:
                    {
                        if (pAttr->Which() == RES_TXTATR_CHARFMT)
                        {
                            activeCharFmts.push_back(pAttr);
                        }
                        // pure formatting hints may end inside the redline &
                        // start again inside the redline, which must not cause
                        // a new text portion if they have the same items - so
                        // store the effective items & compare all at the end
                        SfxItemSet const& rSet((pAttr->Which() == RES_TXTATR_CHARFMT)
                            ? static_cast<SfxItemSet const&>(pAttr->GetCharFormat().GetCharFormat()->GetAttrSet())
                            : *pAttr->GetAutoFormat().GetStyleHandle());
                        InsertCharAttrs(activeCharAttrsStart, rSet);
                    }
                    break;
                // SwTextNode::SetAttr puts it into AUTOFMT which is quite
                // sensible so it doesn't actually exist as a hint
                case RES_TXTATR_UNKNOWN_CONTAINER:
                default: assert(false);
            }
        }
        assert(nEndIndex == pStartHints->Count() ||
            pRLEnd->GetContentIndex() < pStartHints->GetSortedByEnd(nEndIndex)->GetAnyEnd());
    }

    if (rStartNode != pRLEnd->GetNode())
    {
        nStartIndex = 0;
        nEndIndex = 0;
    }

    // treat para properties as text properties
        // ... with the FormatToTextAttr we get autofmts that correspond to the *effective* attr set difference
        // effective attr set: para props + charfmts + autofmt *in that order*
        // ... and the charfmt must be *nominally* the same

    SfxPoolItem const* activeCharAttrsEnd[RES_CHRATR_END - RES_CHRATR_BEGIN + 1] = { nullptr, };
    if (rStartNode != pRLEnd->GetNode())
    {   // nodes' attributes are only needed if there are different nodes
        InsertCharAttrs(activeCharAttrsEnd,
                pRLEnd->GetNode().GetTextNode()->GetSwAttrSet());
    }

    if (SwpHints *const pEndHints = pRLEnd->GetNode().GetTextNode()->GetpSwpHints())
    {
        // check hint starts of hints that start within and end after
#ifndef NDEBUG
        sal_Int32 const nRedlineStart(rStartNode == pRLEnd->GetNode()
                ? nStartRedline
                : 0);
#endif
        for ( ; nStartIndex < pEndHints->Count(); ++nStartIndex)
        {
            SwTextAttr *const pAttr(pEndHints->Get(nStartIndex));
            // compare with < here, not <=, to get the effective formatting
            // of the 1st char after the redline; should not cause problems
            // with consecutive delete redlines because those are handed by
            // GetNextRedln() and here we have the last end pos.
            if (pRLEnd->GetContentIndex() < pAttr->GetStart())
            {
                break;
            }
            if (!pAttr->End())
                continue;
            if (pAttr->IsFormatIgnoreStart())
            {
                continue;
            }
            assert(nRedlineStart <= pAttr->GetStart()); // we wouldn't be here otherwise?
            if (*pAttr->End() <= pRLEnd->GetContentIndex())
            {
                continue;
            }
            switch (pAttr->Which())
            {
                case RES_TXTATR_REFMARK:
                case RES_TXTATR_TOXMARK:
                case RES_TXTATR_META: // actually these 2 aren't allowed to overlap ???
                case RES_TXTATR_METAFIELD:
                case RES_TXTATR_INETFMT:
                case RES_TXTATR_CJK_RUBY:
                case RES_TXTATR_INPUTFIELD:
                case RES_TXTATR_CONTENTCONTROL:
                    {
                        if (!isTheAnswerYes) return false;
                    }
                    break;
                case RES_TXTATR_AUTOFMT:
                case RES_TXTATR_CHARFMT:
                    {
                        // char formats must be *nominally* the same
                        if (pAttr->Which() == RES_TXTATR_CHARFMT)
                        {
                            auto iter = std::find_if(activeCharFmts.begin(), activeCharFmts.end(),
                                [&pAttr](const SwTextAttr* pCharFmt) { return *pCharFmt == *pAttr; });
                            if (iter != activeCharFmts.end())
                                activeCharFmts.erase(iter);
                            else if (!isTheAnswerYes)
                                return false;
                        }
                        SfxItemSet const& rSet((pAttr->Which() == RES_TXTATR_CHARFMT)
                            ? static_cast<SfxItemSet const&>(pAttr->GetCharFormat().GetCharFormat()->GetAttrSet())
                            : *pAttr->GetAutoFormat().GetStyleHandle());
                        InsertCharAttrs(activeCharAttrsEnd, rSet);

                    }
                    break;
                // SwTextNode::SetAttr puts it into AUTOFMT which is quite
                // sensible so it doesn't actually exist as a hint
                case RES_TXTATR_UNKNOWN_CONTAINER:
                default: assert(false);
            }
        }
        if (rStartNode != pRLEnd->GetNode())
        {
            // need to iterate the nEndIndex forward too so the loop in the
            // caller can look for the right ends in the next iteration
            for (nEndIndex = 0; nEndIndex < pEndHints->Count(); ++nEndIndex)
            {
                SwTextAttr *const pAttr(pEndHints->GetSortedByEnd(nEndIndex));
                if (!pAttr->End())
                    continue;
                if (pRLEnd->GetContentIndex() < *pAttr->End())
                {
                    break;
                }
            }
        }
    }

    // if we didn't find a matching start for any end, then it really ends inside
    if (!activeCharFmts.empty())
    {
        if (!isTheAnswerYes) return false;
    }
    for (size_t i = 0; i < SAL_N_ELEMENTS(activeCharAttrsStart); ++i)
    {
        // all of these should be shareable (but we have no SfxItemPool to check it here)
        // assert(!activeCharAttrsStart[i] || activeCharAttrsStart[i]->GetItemPool()->Shareable(*activeCharAttrsStart[i]));
        if (!SfxPoolItem::areSame(activeCharAttrsStart[i], activeCharAttrsEnd[i]))
        {
            if (!isTheAnswerYes) return false;
        }
    }
    rStartIndex = nStartIndex;
    rEndIndex = nEndIndex;
    return true;
}

static sal_Int32 GetNextAttrImpl(SwTextNode const*const pTextNode,
        size_t const nStartIndex, size_t const nEndIndex,
        sal_Int32 const nPosition)
{
    // note: this used to be COMPLETE_STRING, but was set to Len() + 1 below,
    // which is rather silly, so set it to Len() instead
    sal_Int32 nNext = pTextNode->Len();
    if (SwpHints const*const pHints = pTextNode->GetpSwpHints())
    {
        // are there attribute starts left?
        for (size_t i = nStartIndex; i < pHints->Count(); ++i)
        {
            SwTextAttr *const pAttr(pHints->Get(i));
            if (!pAttr->IsFormatIgnoreStart())
            {
                nNext = pAttr->GetStart();
                break;
            }
        }
        // are there attribute ends left?
        for (size_t i = nEndIndex; i < pHints->Count(); ++i)
        {
            SwTextAttr *const pAttr(pHints->GetSortedByEnd(i));
            if (!pAttr->IsFormatIgnoreEnd())
            {
                sal_Int32 const nNextEnd = pAttr->GetAnyEnd();
                nNext = std::min(nNext, nNextEnd); // pick nearest one
                break;
            }
        }
    }
    // TODO: maybe use hints like FieldHints for this instead of looking at the text...
    const sal_Int32 l = std::min(nNext, pTextNode->Len());
    sal_Int32 p = nPosition;
    const sal_Unicode* pStr = pTextNode->GetText().getStr();
    while (p < l)
    {
        sal_Unicode aChar = pStr[p];
        switch (aChar)
        {
            case CH_TXT_ATR_FORMELEMENT:
            case CH_TXT_ATR_FIELDSTART:
            case CH_TXT_ATR_FIELDSEP:
            case CH_TXT_ATR_FIELDEND:
                goto break_; // sigh...
            default:
                ++p;
        }
    }
break_:
    assert(p <= nNext);
    if (p < l)
    {
        // found a CH_TXT_ATR_FIELD*: if it's same as current position,
        // skip behind it so that both before- and after-positions are returned
        nNext = (nPosition < p) ? p : p + 1;
    }
    return nNext;
}

TextFrameIndex SwAttrIter::GetNextAttr() const
{
    size_t nStartIndex(m_nStartIndex);
    size_t nEndIndex(m_nEndIndex);
    size_t nPosition(m_nPosition);
    SwTextNode const* pTextNode(m_pTextNode);
    SwRedlineTable::size_type nActRedline(m_pRedline ? m_pRedline->GetAct() : SwRedlineTable::npos);

    while (true)
    {
        sal_Int32 nNext = GetNextAttrImpl(pTextNode, nStartIndex, nEndIndex, nPosition);
        if( m_pRedline )
        {
            std::pair<sal_Int32, std::pair<SwRangeRedline const*, size_t>> const redline(
                    m_pRedline->GetNextRedln(nNext, pTextNode, nActRedline));
            if (redline.second.first)
            {
                assert(m_pMergedPara);
                assert(redline.second.first->End()->GetNodeIndex() <= m_pMergedPara->pLastNode->GetIndex()
                    || !redline.second.first->End()->GetNode().IsTextNode());
                if (CanSkipOverRedline(*pTextNode, redline.first, *redline.second.first,
                        nStartIndex, nEndIndex, m_nPosition == redline.first))
                {   // if current position is start of the redline, must skip!
                    nActRedline += redline.second.second;
                    if (&redline.second.first->End()->GetNode() != pTextNode)
                    {
                        pTextNode = redline.second.first->End()->GetNode().GetTextNode();
                        nPosition = redline.second.first->End()->GetContentIndex();
                    }
                    else
                    {
                        nPosition = redline.second.first->End()->GetContentIndex();
                    }
                }
                else
                {
                    return sw::MapModelToView(*m_pMergedPara, pTextNode, redline.first);
                }
            }
            else
            {
                return m_pMergedPara
                    ? sw::MapModelToView(*m_pMergedPara, pTextNode, redline.first)
                    : TextFrameIndex(redline.first);
            }
        }
        else
        {
            return TextFrameIndex(nNext);
        }
    }
}

namespace
{
class FormatBreakTracker
{
private:
    std::optional<SvxCaseMap> m_nCaseMap;

    bool m_bNeedsBreak = false;

    void SetCaseMap(SvxCaseMap nValue)
    {
        if (m_nCaseMap != nValue)
            m_bNeedsBreak = true;

        m_nCaseMap = nValue;
    }

public:
    void HandleItemSet(const SfxItemSet& rSet)
    {
        if (const SvxCaseMapItem* pItem = rSet.GetItem(RES_CHRATR_CASEMAP))
            SetCaseMap(pItem->GetCaseMap());
    }

    void Reset() { m_bNeedsBreak = false; }

    bool NeedsBreak() const { return m_bNeedsBreak; }
};

bool HasFormatBreakAttribute(FormatBreakTracker* pTracker, const SwTextAttr* pAttr)
{
    pTracker->Reset();

    switch (pAttr->Which())
    {
        case RES_TXTATR_AUTOFMT:
        case RES_TXTATR_CHARFMT:
        {
            const SfxItemSet& rSet((pAttr->Which() == RES_TXTATR_CHARFMT)
                                       ? static_cast<SfxItemSet const&>(
                                             pAttr->GetCharFormat().GetCharFormat()->GetAttrSet())
                                       : *pAttr->GetAutoFormat().GetStyleHandle());

            pTracker->HandleItemSet(rSet);
        }
        break;
    }

    if (pAttr->IsFormatIgnoreStart() || pAttr->IsFormatIgnoreEnd())
        pTracker->Reset();

    return pTracker->NeedsBreak();
}
}

TextFrameIndex SwAttrIter::GetNextLayoutBreakAttr() const
{
    size_t nStartIndex(m_nStartIndex);
    SwTextNode const* pTextNode(m_pTextNode);

    sal_Int32 nNext = std::numeric_limits<sal_Int32>::max();

    auto* pHints = pTextNode->GetpSwpHints();
    if (!pHints)
    {
        return TextFrameIndex{ nNext };
    }

    FormatBreakTracker stTracker;
    stTracker.HandleItemSet(pTextNode->GetSwAttrSet());

    for (size_t i = 0; i < pHints->Count(); ++i)
    {
        SwTextAttr* const pAttr(pHints->Get(i));
        if (HasFormatBreakAttribute(&stTracker, pAttr))
        {
            if (i >= nStartIndex)
            {
                nNext = pAttr->GetStart();
                break;
            }
        }
    }

    return TextFrameIndex{ nNext };
}

namespace {

class SwMinMaxArgs
{
public:
    VclPtr<OutputDevice> m_pOut;
    SwViewShell const* m_pSh;
    sal_uLong& m_rMin;
    sal_uLong& m_rAbsMin;
    tools::Long m_nRowWidth;
    tools::Long m_nWordWidth;
    tools::Long m_nWordAdd;
    sal_Int32 m_nNoLineBreak;
    SwMinMaxArgs(OutputDevice* pOutI, SwViewShell const* pShI, sal_uLong& rMinI, sal_uLong& rAbsI)
        : m_pOut(pOutI)
        , m_pSh(pShI)
        , m_rMin(rMinI)
        , m_rAbsMin(rAbsI)
        , m_nRowWidth(0)
        , m_nWordWidth(0)
        , m_nWordAdd(0)
        , m_nNoLineBreak(COMPLETE_STRING)
    { }
    void Minimum( tools::Long nNew ) const {
        if (static_cast<tools::Long>(m_rMin) < nNew)
            m_rMin = nNew;
    }
    void NewWord() { m_nWordAdd = m_nWordWidth = 0; }
};

}

static bool lcl_MinMaxString( SwMinMaxArgs& rArg, SwFont* pFnt, const OUString &rText,
    sal_Int32 nIdx, sal_Int32 nEnd )
{
    bool bRet = false;
    while( nIdx < nEnd )
    {
        sal_Int32 nStop = nIdx;
        LanguageType eLang = pFnt->GetLanguage();
        assert(g_pBreakIt && g_pBreakIt->GetBreakIter().is());

        bool bClear = CH_BLANK == rText[ nStop ];
        Boundary aBndry( g_pBreakIt->GetBreakIter()->getWordBoundary( rText, nIdx,
                         g_pBreakIt->GetLocale( eLang ),
                         WordType::DICTIONARY_WORD, true ) );
        nStop = aBndry.endPos;
        if (nIdx <= aBndry.startPos && nIdx && nIdx - 1 != rArg.m_nNoLineBreak)
            rArg.NewWord();
        if( nStop == nIdx )
            ++nStop;
        if( nStop > nEnd )
            nStop = nEnd;

        SwDrawTextInfo aDrawInf(rArg.m_pSh, *rArg.m_pOut, rText, nIdx, nStop - nIdx);
        tools::Long nCurrentWidth = pFnt->GetTextSize_( aDrawInf ).Width();
        rArg.m_nRowWidth += nCurrentWidth;
        if( bClear )
            rArg.NewWord();
        else
        {
            rArg.m_nWordWidth += nCurrentWidth;
            if (static_cast<tools::Long>(rArg.m_rAbsMin) < rArg.m_nWordWidth)
                rArg.m_rAbsMin = rArg.m_nWordWidth;
            rArg.Minimum(rArg.m_nWordWidth + rArg.m_nWordAdd);
            bRet = true;
        }
        nIdx = nStop;
    }
    return bRet;
}

bool SwTextNode::IsSymbolAt(const sal_Int32 nBegin) const
{
    SwScriptInfo aScriptInfo;
    SwAttrIter aIter( *const_cast<SwTextNode*>(this), aScriptInfo );
    aIter.Seek( TextFrameIndex(nBegin) );
    return aIter.GetFnt()->IsSymbol( getIDocumentLayoutAccess().GetCurrentViewShell() );
}

namespace {

class SwMinMaxNodeArgs
{
public:
    sal_uLong m_nMaxWidth; // sum of all frame widths
    tools::Long m_nMinWidth; // biggest frame
    tools::Long m_nLeftRest; // space not already covered by frames in the left margin
    tools::Long m_nRightRest; // space not already covered by frames in the right margin
    tools::Long m_nLeftDiff; // Min/Max-difference of the frame in the left margin
    tools::Long m_nRightDiff; // Min/Max-difference of the frame in the right margin
    SwNodeOffset m_nIndex; // index of the node
    void Minimum( tools::Long nNew ) {
        if (nNew > m_nMinWidth)
            m_nMinWidth = nNew;
    }
};

}

static void lcl_MinMaxNode(SwFrameFormat* pNd, SwMinMaxNodeArgs& rIn)
{
    const SwFormatAnchor& rFormatA = pNd->GetAnchor();

    if ((RndStdIds::FLY_AT_PARA != rFormatA.GetAnchorId()) &&
        (RndStdIds::FLY_AT_CHAR != rFormatA.GetAnchorId()))
    {
        return;
    }

    const SwNode *pAnchorNode = rFormatA.GetAnchorNode();
    OSL_ENSURE(pAnchorNode, "Unexpected NULL arguments");
    if (!pAnchorNode || rIn.m_nIndex != pAnchorNode->GetIndex())
        return;

    tools::Long nMin, nMax;
    SwHTMLTableLayout *pLayout = nullptr;
    const bool bIsDrawFrameFormat = pNd->Which()==RES_DRAWFRMFMT;
    if( !bIsDrawFrameFormat )
    {
        // Does the frame contain a table at the start or the end?
        const SwNodes& rNodes = pNd->GetDoc().GetNodes();
        const SwFormatContent& rFlyContent = pNd->GetContent();
        SwNodeOffset nStt = rFlyContent.GetContentIdx()->GetIndex();
        SwTableNode* pTableNd = rNodes[nStt+1]->GetTableNode();
        if( !pTableNd )
        {
            SwNode *pNd2 = rNodes[nStt];
            pNd2 = rNodes[pNd2->EndOfSectionIndex()-1];
            if( pNd2->IsEndNode() )
                pTableNd = pNd2->StartOfSectionNode()->GetTableNode();
        }

        if( pTableNd )
            pLayout = pTableNd->GetTable().GetHTMLTableLayout();
    }

    const SwFormatHoriOrient& rOrient = pNd->GetHoriOrient();
    sal_Int16 eHoriOri = rOrient.GetHoriOrient();

    tools::Long nDiff;
    if( pLayout )
    {
        nMin = pLayout->GetMin();
        nMax = pLayout->GetMax();
        nDiff = nMax - nMin;
    }
    else
    {
        if( bIsDrawFrameFormat )
        {
            const SdrObject* pSObj = pNd->FindSdrObject();
            if( pSObj )
                nMin = pSObj->GetCurrentBoundRect().GetWidth();
            else
                nMin = 0;

        }
        else
        {
            const SwFormatFrameSize &rSz = pNd->GetFrameSize();
            nMin = rSz.GetWidth();
        }
        nMax = nMin;
        nDiff = 0;
    }

    const SvxLRSpaceItem &rLR = pNd->GetLRSpace();
    nMin += rLR.ResolveLeft({});
    nMin += rLR.ResolveRight({});
    nMax += rLR.ResolveLeft({});
    nMax += rLR.ResolveRight({});

    if( css::text::WrapTextMode_THROUGH == pNd->GetSurround().GetSurround() )
    {
        rIn.Minimum( nMin );
        return;
    }

    // Frames, which are left- or right-aligned are only party considered
    // when calculating the maximum, since the border is already being considered.
    // Only if the frame extends into the text body, this part is being added
    switch( eHoriOri )
    {
        case text::HoriOrientation::RIGHT:
        {
            if( nDiff )
            {
                rIn.m_nRightRest -= rIn.m_nRightDiff;
                rIn.m_nRightDiff = nDiff;
            }
            if( text::RelOrientation::FRAME != rOrient.GetRelationOrient() )
            {
                if (rIn.m_nRightRest > 0)
                    rIn.m_nRightRest = 0;
            }
            rIn.m_nRightRest -= nMin;
            break;
        }
        case text::HoriOrientation::LEFT:
        {
            if( nDiff )
            {
                rIn.m_nLeftRest -= rIn.m_nLeftDiff;
                rIn.m_nLeftDiff = nDiff;
            }
            if (text::RelOrientation::FRAME != rOrient.GetRelationOrient() && rIn.m_nLeftRest < 0)
                rIn.m_nLeftRest = 0;
            rIn.m_nLeftRest -= nMin;
            break;
        }
        default:
        {
            rIn.m_nMaxWidth += nMax;
            rIn.Minimum(nMin);
        }
    }
}

#define FLYINCNT_MIN_WIDTH 284

/**
 * Changing this method very likely requires changing of GetScalingOfSelectedText
 * This one is called exclusively from import filters, so there is no layout.
 */
void SwTextNode::GetMinMaxSize( SwNodeOffset nIndex, sal_uLong& rMin, sal_uLong &rMax,
                               sal_uLong& rAbsMin ) const
{
    SwViewShell const * pSh = GetDoc().getIDocumentLayoutAccess().GetCurrentViewShell();
    OutputDevice* pOut = nullptr;
    if( pSh )
        pOut = pSh->GetWin()->GetOutDev();
    if( !pOut )
        pOut = Application::GetDefaultDevice();

    MapMode aOldMap( pOut->GetMapMode() );
    pOut->SetMapMode( MapMode( MapUnit::MapTwip ) );

    rMin = 0;
    rMax = 0;
    rAbsMin = 0;

    SvxTextLeftMarginItem const& rTextLeftMargin(GetSwAttrSet().GetTextLeftMargin());
    SvxRightMarginItem const& rRightMargin(GetSwAttrSet().GetRightMargin());
    tools::Long nLROffset = rTextLeftMargin.ResolveTextLeft({}) + GetLeftMarginWithNum(true);
    short nFLOffs;
    // For enumerations a negative first line indentation is probably filled already
    if (!GetFirstLineOfsWithNum(nFLOffs, {}) || nFLOffs > nLROffset)
        nLROffset = nFLOffs;

    SwMinMaxNodeArgs aNodeArgs;
    aNodeArgs.m_nMinWidth = 0;
    aNodeArgs.m_nMaxWidth = 0;
    aNodeArgs.m_nLeftRest = nLROffset;
    aNodeArgs.m_nRightRest = rRightMargin.ResolveRight({});
    aNodeArgs.m_nLeftDiff = 0;
    aNodeArgs.m_nRightDiff = 0;
    if( nIndex )
    {
        sw::SpzFrameFormats* pSpzs = const_cast<sw::SpzFrameFormats*>(GetDoc().GetSpzFrameFormats());
        if(pSpzs)
        {
            aNodeArgs.m_nIndex = nIndex;
            for(auto pFormat: *pSpzs)
                lcl_MinMaxNode(pFormat, aNodeArgs);
        }
    }
    if (aNodeArgs.m_nLeftRest < 0)
        aNodeArgs.Minimum(nLROffset - aNodeArgs.m_nLeftRest);
    aNodeArgs.m_nLeftRest -= aNodeArgs.m_nLeftDiff;
    if (aNodeArgs.m_nLeftRest < 0)
        aNodeArgs.m_nMaxWidth -= aNodeArgs.m_nLeftRest;

    if (aNodeArgs.m_nRightRest < 0)
        aNodeArgs.Minimum(rRightMargin.ResolveRight({}) - aNodeArgs.m_nRightRest);
    aNodeArgs.m_nRightRest -= aNodeArgs.m_nRightDiff;
    if (aNodeArgs.m_nRightRest < 0)
        aNodeArgs.m_nMaxWidth -= aNodeArgs.m_nRightRest;

    SwScriptInfo aScriptInfo;
    SwAttrIter aIter( *const_cast<SwTextNode*>(this), aScriptInfo );
    TextFrameIndex nIdx(0);
    aIter.SeekAndChgAttrIter( nIdx, pOut );
    TextFrameIndex nLen(m_Text.getLength());
    tools::Long nCurrentWidth = 0;
    tools::Long nAdd = 0;
    SwMinMaxArgs aArg( pOut, pSh, rMin, rAbsMin );
    while( nIdx < nLen )
    {
        TextFrameIndex nNextChg = aIter.GetNextAttr();
        TextFrameIndex nStop = aScriptInfo.NextScriptChg( nIdx );
        if( nNextChg > nStop )
            nNextChg = nStop;
        SwTextAttr *pHint = nullptr;
        sal_Unicode cChar = CH_BLANK;
        nStop = nIdx;
        while( nStop < nLen && nStop < nNextChg &&
               CH_TAB != (cChar = m_Text[sal_Int32(nStop)]) &&
               CH_BREAK != cChar && CHAR_HARDBLANK != cChar &&
               CHAR_HARDHYPHEN != cChar && CHAR_SOFTHYPHEN != cChar &&
               CH_TXT_ATR_INPUTFIELDSTART != cChar &&
               CH_TXT_ATR_INPUTFIELDEND != cChar &&
               CH_TXT_ATR_FORMELEMENT != cChar &&
               CH_TXT_ATR_FIELDSTART != cChar &&
               CH_TXT_ATR_FIELDSEP != cChar &&
               CH_TXT_ATR_FIELDEND != cChar &&
               !pHint )
        {
            // this looks like some defensive programming to handle dummy char
            // with missing hint? but it's rather silly because it may pass the
            // dummy char to lcl_MinMaxString in that case...
            if( ( CH_TXTATR_BREAKWORD != cChar && CH_TXTATR_INWORD != cChar )
                || ( nullptr == ( pHint = aIter.GetAttr( nStop ) ) ) )
                ++nStop;
        }
        if (lcl_MinMaxString(aArg, aIter.GetFnt(), m_Text, sal_Int32(nIdx), sal_Int32(nStop)))
        {
            nAdd = 20;
        }
        nIdx = nStop;
        aIter.SeekAndChgAttrIter( nIdx, pOut );
        switch( cChar )
        {
            case CH_BREAK  :
            {
                if (static_cast<tools::Long>(rMax) < aArg.m_nRowWidth)
                    rMax = aArg.m_nRowWidth;
                aArg.m_nRowWidth = 0;
                aArg.NewWord();
                aIter.SeekAndChgAttrIter( ++nIdx, pOut );
            }
            break;
            case CH_TAB    :
            {
                aArg.NewWord();
                aIter.SeekAndChgAttrIter( ++nIdx, pOut );
            }
            break;
            case CHAR_SOFTHYPHEN:
                ++nIdx;
            break;
            case CHAR_HARDBLANK:
            case CHAR_HARDHYPHEN:
            {
                OUString sTmp( cChar );
                SwDrawTextInfo aDrawInf( pSh,
                    *pOut, sTmp, 0, 1, 0, false );
                nCurrentWidth = aIter.GetFnt()->GetTextSize_( aDrawInf ).Width();
                aArg.m_nWordWidth += nCurrentWidth;
                aArg.m_nRowWidth += nCurrentWidth;
                if (static_cast<tools::Long>(rAbsMin) < aArg.m_nWordWidth)
                    rAbsMin = aArg.m_nWordWidth;
                aArg.Minimum(aArg.m_nWordWidth + aArg.m_nWordAdd);
                aArg.m_nNoLineBreak = sal_Int32(nIdx++);
            }
            break;
            case CH_TXTATR_BREAKWORD:
            case CH_TXTATR_INWORD:
            {
                if( !pHint )
                    break;
                tools::Long nOldWidth = aArg.m_nWordWidth;
                tools::Long nOldAdd = aArg.m_nWordAdd;
                aArg.NewWord();

                switch( pHint->Which() )
                {
                    case RES_TXTATR_FLYCNT :
                    {
                        SwFrameFormat *pFrameFormat = pHint->GetFlyCnt().GetFrameFormat();
                        const SvxLRSpaceItem &rLR = pFrameFormat->GetLRSpace();
                        if( RES_DRAWFRMFMT == pFrameFormat->Which() )
                        {
                            const SdrObject* pSObj = pFrameFormat->FindSdrObject();
                            if( pSObj )
                                nCurrentWidth = pSObj->GetCurrentBoundRect().GetWidth();
                            else
                                nCurrentWidth = 0;
                        }
                        else
                        {
                            const SwFormatFrameSize& rTmpSize = pFrameFormat->GetFrameSize();
                            if( RES_FLYFRMFMT == pFrameFormat->Which()
                                && rTmpSize.GetWidthPercent() )
                            {
                                // This is a hack for the following situation: In the paragraph there's a
                                // text frame with relative size. Then let's take 0.5 cm as minimum width
                                // and USHRT_MAX as maximum width
                                // It were cleaner and maybe necessary later on to iterate over the content
                                // of the text frame and call GetMinMaxSize recursively
                                nCurrentWidth = FLYINCNT_MIN_WIDTH; // 0.5 cm
                                rMax = std::max(rMax, sal_uLong(USHRT_MAX));
                            }
                            else
                                nCurrentWidth = pFrameFormat->GetFrameSize().GetWidth();
                        }
                        nCurrentWidth += rLR.ResolveLeft({});
                        nCurrentWidth += rLR.ResolveRight({});
                        aArg.m_nWordAdd = nOldWidth + nOldAdd;
                        aArg.m_nWordWidth = nCurrentWidth;
                        aArg.m_nRowWidth += nCurrentWidth;
                        if (static_cast<tools::Long>(rAbsMin) < aArg.m_nWordWidth)
                            rAbsMin = aArg.m_nWordWidth;
                        aArg.Minimum(aArg.m_nWordWidth + aArg.m_nWordAdd);
                        break;
                    }
                    case RES_TXTATR_FTN :
                    {
                        const OUString aText = pHint->GetFootnote().GetNumStr();
                        if( lcl_MinMaxString( aArg, aIter.GetFnt(), aText, 0,
                            aText.getLength() ) )
                            nAdd = 20;
                        break;
                    }

                    case RES_TXTATR_FIELD :
                    case RES_TXTATR_ANNOTATION :
                        {
                            SwField *pField = const_cast<SwField*>(pHint->GetFormatField().GetField());
                            const OUString aText = pField->ExpandField(true, nullptr);
                            if( lcl_MinMaxString( aArg, aIter.GetFnt(), aText, 0,
                                aText.getLength() ) )
                                nAdd = 20;
                            break;
                        }
                    default:
                        aArg.m_nWordWidth = nOldWidth;
                        aArg.m_nWordAdd = nOldAdd;
                }
                aIter.SeekAndChgAttrIter( ++nIdx, pOut );
            }
            break;
            case CH_TXT_ATR_INPUTFIELDSTART:
            case CH_TXT_ATR_INPUTFIELDEND:
            case CH_TXT_ATR_FORMELEMENT:
            case CH_TXT_ATR_FIELDSTART:
            case CH_TXT_ATR_FIELDSEP:
            case CH_TXT_ATR_FIELDEND:
            {   // just skip it and continue with the content...
                aIter.SeekAndChgAttrIter( ++nIdx, pOut );
            }
            break;
        }
    }
    if (static_cast<tools::Long>(rMax) < aArg.m_nRowWidth)
        rMax = aArg.m_nRowWidth;

    nLROffset += rRightMargin.ResolveRight({});

    rAbsMin += nLROffset;
    rAbsMin += nAdd;
    rMin += nLROffset;
    rMin += nAdd;
    if (static_cast<tools::Long>(rMin) < aNodeArgs.m_nMinWidth)
        rMin = aNodeArgs.m_nMinWidth;
    if (static_cast<tools::Long>(rAbsMin) < aNodeArgs.m_nMinWidth)
        rAbsMin = aNodeArgs.m_nMinWidth;
    rMax += aNodeArgs.m_nMaxWidth;
    rMax += nLROffset;
    rMax += nAdd;
    if( rMax < rMin ) // e.g. Frames with flow through only contribute to the minimum
        rMax = rMin;
    pOut->SetMapMode( aOldMap );
}

/**
 * Calculates the width of the text part specified by nStart and nEnd,
 * the height of the line containing nStart is divided by this width,
 * indicating the scaling factor, if the text part is rotated.
 * Having CH_BREAKs in the text part, this method returns the scaling
 * factor for the longest of the text parts separated by the CH_BREAK
 *
 * Changing this method very likely requires changing of "GetMinMaxSize"
 */
sal_uInt16 SwTextFrame::GetScalingOfSelectedText(
        TextFrameIndex nStart, TextFrameIndex nEnd)
{
    assert(GetOffset() <= nStart && (!GetFollow() || nStart < GetFollow()->GetOffset()));
    SwViewShell const*const pSh = getRootFrame()->GetCurrShell();
    assert(pSh);
    OutputDevice *const pOut = &pSh->GetRefDev();
    assert(pOut);

    MapMode aOldMap( pOut->GetMapMode() );
    pOut->SetMapMode( MapMode( MapUnit::MapTwip ) );

    if (nStart == nEnd)
    {
        assert(g_pBreakIt && g_pBreakIt->GetBreakIter().is());

        SwScriptInfo aScriptInfo;
        SwAttrIter aIter(*GetTextNodeFirst(), aScriptInfo, this);
        aIter.SeekAndChgAttrIter( nStart, pOut );

        Boundary aBound = g_pBreakIt->GetBreakIter()->getWordBoundary(
            GetText(), sal_Int32(nStart),
            g_pBreakIt->GetLocale( aIter.GetFnt()->GetLanguage() ),
            WordType::DICTIONARY_WORD, true );

        if (sal_Int32(nStart) == aBound.startPos)
        {
            // cursor is at left or right border of word
            pOut->SetMapMode( aOldMap );
            return 100;
        }

        nStart = TextFrameIndex(aBound.startPos);
        nEnd = TextFrameIndex(aBound.endPos);

        if (nStart == nEnd)
        {
            pOut->SetMapMode( aOldMap );
            return 100;
        }
    }

    SwScriptInfo aScriptInfo;
    SwAttrIter aIter(*GetTextNodeFirst(), aScriptInfo, this);

    // We do not want scaling attributes to be considered during this
    // calculation. For this, we push a temporary scaling attribute with
    // scaling value 100 and priority flag on top of the scaling stack
    SwAttrHandler& rAH = aIter.GetAttrHandler();
    SvxCharScaleWidthItem aItem(100, RES_CHRATR_SCALEW);
    SwTextAttrEnd aAttr( SfxPoolItemHolder(getRootFrame()->GetCurrShell()->GetAttrPool(), &aItem), 0, COMPLETE_STRING );
    aAttr.SetPriorityAttr( true );
    rAH.PushAndChg( aAttr, *(aIter.GetFnt()) );

    TextFrameIndex nIdx = nStart;

    sal_uLong nWidth = 0;
    sal_uLong nProWidth = 0;

    while( nIdx < nEnd )
    {
        aIter.SeekAndChgAttrIter( nIdx, pOut );

        // scan for end of portion
        TextFrameIndex const nNextChg = std::min(aIter.GetNextAttr(), aScriptInfo.NextScriptChg(nIdx));

        TextFrameIndex nStop = nIdx;
        sal_Unicode cChar = CH_BLANK;
        SwTextAttr* pHint = nullptr;

        // stop at special characters in [ nIdx, nNextChg ]
        while( nStop < nEnd && nStop < nNextChg )
        {
            cChar = GetText()[sal_Int32(nStop)];
            if (
                CH_TAB == cChar ||
                CH_BREAK == cChar ||
                CHAR_HARDBLANK == cChar ||
                CHAR_HARDHYPHEN == cChar ||
                CHAR_SOFTHYPHEN == cChar ||
                CH_TXT_ATR_INPUTFIELDSTART == cChar ||
                CH_TXT_ATR_INPUTFIELDEND == cChar ||
                CH_TXT_ATR_FORMELEMENT == cChar ||
                CH_TXT_ATR_FIELDSTART == cChar ||
                CH_TXT_ATR_FIELDSEP == cChar ||
                CH_TXT_ATR_FIELDEND == cChar ||
                (
                  (CH_TXTATR_BREAKWORD == cChar || CH_TXTATR_INWORD == cChar) &&
                  (nullptr == (pHint = aIter.GetAttr(nStop)))
                )
               )
            {
                break;
            }
            else
                ++nStop;
        }

        // calculate text widths up to cChar
        if ( nStop > nIdx )
        {
            SwDrawTextInfo aDrawInf(pSh, *pOut, GetText(), sal_Int32(nIdx), sal_Int32(nStop - nIdx));
            nProWidth += aIter.GetFnt()->GetTextSize_( aDrawInf ).Width();
        }

        nIdx = nStop;
        aIter.SeekAndChgAttrIter( nIdx, pOut );

        if ( cChar == CH_BREAK )
        {
            nWidth = std::max( nWidth, nProWidth );
            nProWidth = 0;
            nIdx++;
        }
        else if ( cChar == CH_TAB )
        {
            // tab receives width of one space
            SwDrawTextInfo aDrawInf(pSh, *pOut, OUStringChar(CH_BLANK), 0, 1);
            nProWidth += aIter.GetFnt()->GetTextSize_( aDrawInf ).Width();
            nIdx++;
        }
        else if ( cChar == CHAR_SOFTHYPHEN )
            ++nIdx;
        else if ( cChar == CHAR_HARDBLANK || cChar == CHAR_HARDHYPHEN )
        {
            OUString sTmp( cChar );
            SwDrawTextInfo aDrawInf(pSh, *pOut, sTmp, 0, 1);
            nProWidth += aIter.GetFnt()->GetTextSize_( aDrawInf ).Width();
            nIdx++;
        }
        else if ( pHint && ( cChar == CH_TXTATR_BREAKWORD || cChar == CH_TXTATR_INWORD ) )
        {
            switch( pHint->Which() )
            {
            case RES_TXTATR_FTN :
                {
                    const OUString aText = pHint->GetFootnote().GetNumStr();
                    SwDrawTextInfo aDrawInf(pSh, *pOut, aText, 0, aText.getLength());

                    nProWidth += aIter.GetFnt()->GetTextSize_( aDrawInf ).Width();
                    break;
                }

            case RES_TXTATR_FIELD :
            case RES_TXTATR_ANNOTATION :
                {
                    SwField *pField = const_cast<SwField*>(pHint->GetFormatField().GetField());
                    OUString const aText = pField->ExpandField(true, getRootFrame());
                    SwDrawTextInfo aDrawInf(pSh, *pOut, aText, 0, aText.getLength());

                    nProWidth += aIter.GetFnt()->GetTextSize_( aDrawInf ).Width();
                    break;
                }

            default:
                {
                    // any suggestions for a default action?
                }
            } // end of switch
            nIdx++;
        }
        else if (CH_TXT_ATR_INPUTFIELDSTART == cChar ||
                 CH_TXT_ATR_INPUTFIELDEND == cChar ||
                 CH_TXT_ATR_FORMELEMENT == cChar ||
                 CH_TXT_ATR_FIELDSTART == cChar ||
                 CH_TXT_ATR_FIELDSEP == cChar ||
                 CH_TXT_ATR_FIELDEND == cChar)
        {   // just skip it and continue with the content...
            ++nIdx;
        }
    } // end of while

    nWidth = std::max( nWidth, nProWidth );

    // search for the line containing nStart
    if (HasPara())
    {
        SwTextInfo aInf(this);
        SwTextIter aLine(this, &aInf);
        aLine.CharToLine( nStart );
        pOut->SetMapMode( aOldMap );
        return o3tl::narrowing<sal_uInt16>( nWidth ?
            ( ( 100 * aLine.GetCurr()->Height() ) / nWidth ) : 0 );
    }
    // no frame or no paragraph, we take the height of the character
    // at nStart as line height

    aIter.SeekAndChgAttrIter( nStart, pOut );
    pOut->SetMapMode( aOldMap );

    SwDrawTextInfo aDrawInf(pSh, *pOut, GetText(), sal_Int32(nStart), 1);
    return o3tl::narrowing<sal_uInt16>( nWidth ? ((100 * aIter.GetFnt()->GetTextSize_( aDrawInf ).Height()) / nWidth ) : 0 );
}

std::vector<SwFlyAtContentFrame*> SwTextFrame::GetSplitFlyDrawObjs() const
{
    std::vector<SwFlyAtContentFrame*> aObjs;
    const SwSortedObjs* pSortedObjs = GetDrawObjs();
    if (!pSortedObjs)
    {
        return aObjs;
    }

    for (const auto& pSortedObj : *pSortedObjs)
    {
        SwFlyFrame* pFlyFrame = pSortedObj->DynCastFlyFrame();
        if (!pFlyFrame)
        {
            continue;
        }

        if (!pFlyFrame->IsFlySplitAllowed())
        {
            continue;
        }

        aObjs.push_back(static_cast<SwFlyAtContentFrame*>(pFlyFrame));
    }

    return aObjs;
}

bool SwTextFrame::HasSplitFlyDrawObjs() const
{
    return !GetSplitFlyDrawObjs().empty();
}

SwFlyAtContentFrame* SwTextFrame::HasNonLastSplitFlyDrawObj() const
{
    const SwTextFrame* pFollow = GetFollow();
    if (!pFollow)
    {
        return nullptr;
    }

    if (mnOffset != pFollow->GetOffset())
    {
        return nullptr;
    }

    // At this point we know what we're part of a chain that is an anchor for split fly frames, but
    // we're not the last one. See if we have a matching fly.

    // Look up the master of the anchor.
    const SwTextFrame* pAnchor = this;
    while (pAnchor->IsFollow())
    {
        pAnchor = pAnchor->FindMaster();
    }
    for (const auto& pFly : pAnchor->GetSplitFlyDrawObjs())
    {
        // Nominally all flys are anchored in the master; see if this fly is effectively anchored in
        // us.
        SwTextFrame* pFlyAnchor = pFly->FindAnchorCharFrame();
        if (pFlyAnchor != this)
        {
            continue;
        }
        if (pFly->GetFollow())
        {
            return pFly;
        }
    }

    return nullptr;
}

bool SwTextFrame::IsEmptyMasterWithSplitFly() const
{
    if (!IsEmptyMaster())
    {
        return false;
    }

    if (!m_pDrawObjs || m_pDrawObjs->size() != 1)
    {
        return false;
    }

    SwFlyFrame* pFlyFrame = (*m_pDrawObjs)[0]->DynCastFlyFrame();
    if (!pFlyFrame || !pFlyFrame->IsFlySplitAllowed())
    {
        return false;
    }

    if (mnOffset != GetFollow()->GetOffset())
    {
        return false;
    }

    return true;
}

bool SwTextFrame::IsEmptyWithSplitFly() const
{
    if (IsFollow())
    {
        return false;
    }

    if (SvxBreak const eBreak = GetBreakItem().GetBreak();
           eBreak == SvxBreak::ColumnBefore || eBreak == SvxBreak::ColumnBoth
        || eBreak == SvxBreak::PageBefore || eBreak == SvxBreak::PageBoth
        || GetPageDescItem().GetPageDesc() != nullptr)
    {
        return false;
    }

    SwRectFnSet fnUpper(GetUpper());
    if (fnUpper.YDiff(fnUpper.GetBottom(getFrameArea()), fnUpper.GetPrtBottom(*GetUpper())) <= 0)
    {
        return false;
    }

    // This is a master that doesn't fit the current parent.
    if (!m_pDrawObjs || m_pDrawObjs->size() != 1)
    {
        return false;
    }

    SwFlyFrame* pFlyFrame = (*m_pDrawObjs)[0]->DynCastFlyFrame();
    if (!pFlyFrame || !pFlyFrame->IsFlySplitAllowed())
    {
        return false;
    }

    // It has a split fly anchored to it.
    if (pFlyFrame->GetFrameFormat()->GetVertOrient().GetPos() >= 0)
    {
        return false;
    }

    // Negative vertical offset means that visually it already may have a first line.
    // Consider that, we may need to split the frame, so the fly frame is on one page and the empty
    // paragraph's frame is on a next page.
    return true;
}

SwTwips SwTextNode::GetWidthOfLeadingTabs() const
{
    SwTwips nRet = 0;

    sal_Int32 nIdx = 0;

    while ( nIdx < GetText().getLength() )
    {
        const sal_Unicode cCh = GetText()[nIdx];
        if ( cCh!='\t' && cCh!=' ' )
        {
            break;
        }
        ++nIdx;
    }

    if ( nIdx > 0 )
    {
        SwPosition aPos( *this, nIdx );

        // Find the non-follow text frame:
        SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIter(*this);
        for( SwTextFrame* pFrame = aIter.First(); pFrame; pFrame = aIter.Next() )
        {
            // Only consider master frames:
            if (!pFrame->IsFollow() &&
                pFrame->GetTextNodeForFirstText() == this)
            {
                SwRectFnSet aRectFnSet(pFrame);
                SwRect aRect;
                pFrame->GetCharRect( aRect, aPos );
                nRet = pFrame->IsRightToLeft() ?
                            aRectFnSet.GetPrtRight(*pFrame) - aRectFnSet.GetRight(aRect) :
                            aRectFnSet.GetLeft(aRect) - aRectFnSet.GetPrtLeft(*pFrame);
                break;
            }
        }
    }

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
