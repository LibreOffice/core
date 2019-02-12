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
#include <txtatr.hxx>
#include <svl/itemiter.hxx>
#include <sfx2/printer.hxx>
#include <svx/svdobj.hxx>
#include <vcl/svapp.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <fmtornt.hxx>
#include <fmtflcnt.hxx>
#include <fmtcntnt.hxx>
#include <fmtftn.hxx>
#include <frmatr.hxx>
#include <frmfmt.hxx>
#include <fmtfld.hxx>
#include <doc.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <viewsh.hxx>
#include <rootfrm.hxx>
#include <docary.hxx>
#include <ndtxt.hxx>
#include <dcontact.hxx>
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
#include <editeng/rsiditem.hxx>
#include <calbck.hxx>

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
    std::pair<SwTextNode const*, sal_Int32> const pos( m_pMergedPara
        ? sw::MapViewToModel(*m_pMergedPara, nNewPos)
        : std::make_pair(m_pTextNode, sal_Int32(nNewPos)));
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

    if ( m_nStartIndex ) // If attributes have been opened at all ...
    {
        // Close attributes that are currently open, but stop at nNewPos+1

        // As long as we've not yet reached the end of EndArray and the
        // TextAttribute ends before or at the new position ...
        while ((m_nEndIndex < pHints->Count()) &&
               (*(pTextAttr = pHints->GetSortedByEnd(m_nEndIndex))->GetAnyEnd() <= nNewPos))
        {
            // Close the TextAttributes, whose StartPos were before or at
            // the old nPos and are currently open
            if (pTextAttr->GetStart() <= nOldPos)  Rst( pTextAttr );
            m_nEndIndex++;
        }
    }
    else // skip the not opened ends
    {
        while ((m_nEndIndex < pHints->Count()) &&
               (*pHints->GetSortedByEnd(m_nEndIndex)->GetAnyEnd() <= nNewPos))
        {
            m_nEndIndex++;
        }
    }

    // As long as we've not yet reached the end of EndArray and the
    // TextAttribute ends before or at the new position...
    while ((m_nStartIndex < pHints->Count()) &&
            ((pTextAttr = pHints->Get(m_nStartIndex))->GetStart() <= nNewPos))
    {

        // open the TextAttributes, whose ends lie behind the new position
        if ( *pTextAttr->GetAnyEnd() > nNewPos )  Chg( pTextAttr );
        m_nStartIndex++;
    }

}

bool SwAttrIter::Seek(TextFrameIndex const nNewPos)
{
    // note: nNewPos isn't necessarily a index returned from GetNextAttr
    std::pair<SwTextNode const*, sal_Int32> const newPos( m_pMergedPara
        ? sw::MapViewToModel(*m_pMergedPara, nNewPos)
        : std::make_pair(m_pTextNode, sal_Int32(nNewPos)));

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
        assert(m_nChgCnt == 0); // should have reset it all? there cannot be ExtOn() inside of a Delete redline, surely?
        // Unapply current para items:
        // the SwAttrHandler doesn't appear to be capable of *unapplying*
        // items at all; it can only apply a previously effective item.
        // So do this by recreating the font from scratch.
        // Apply new para items:
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
        if (m_pMergedPara || m_pTextNode->GetpSwpHints())
        {
            if( m_pRedline )
                m_pRedline->Clear( nullptr );

            // reset font to its original state
            m_aAttrHandler.Reset();
            m_aAttrHandler.ResetFont( *m_pFont );

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
    for (SfxPoolItem const* pItem = iter.FirstItem(); pItem; pItem = iter.NextItem())
    {
        auto const nWhich(pItem->Which());
        if (isCHRATR(nWhich) && RES_CHRATR_RSID != nWhich)
        {
            pAttrs[nWhich - RES_CHRATR_BEGIN] = pItem;
        }
        else if (nWhich == RES_TXTATR_UNKNOWN_CONTAINER)
        {
            pAttrs[RES_CHRATR_END] = pItem;
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
    if (!pRLEnd->nNode.GetNode().IsTextNode() // if fully deleted...
        || pRLEnd->nContent == pRLEnd->nNode.GetNode().GetTextNode()->Len())
    {
        // shortcut: nothing follows redline
        // current state is end state
        return false;
    }
    std::vector<SwTextAttr*> activeCharFmts;
    // can't compare the SwFont that's stored somewhere, it doesn't have compare
    // operator, so try to recreate the situation with some temp arrays here
    SfxPoolItem const* activeCharAttrsStart[RES_CHRATR_END - RES_CHRATR_BEGIN + 1] = { nullptr, };
    if (&rStartNode != &pRLEnd->nNode.GetNode())
    {   // nodes' attributes are only needed if there are different nodes
        InsertCharAttrs(activeCharAttrsStart, rStartNode.GetSwAttrSet());
    }
    if (SwpHints const*const pStartHints = rStartNode.GetpSwpHints())
    {
        // check hint ends of hints that start before and end within
        sal_Int32 const nRedlineEnd(&rStartNode == &pRLEnd->nNode.GetNode()
                ? pRLEnd->nContent.GetIndex()
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
                            : *pAttr->GetAutoFormat().GetStyleHandle().get());
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
            pRLEnd->nContent.GetIndex() < *pStartHints->GetSortedByEnd(nEndIndex)->GetAnyEnd());
    }

    if (&rStartNode != &pRLEnd->nNode.GetNode())
    {
        nStartIndex = 0;
        nEndIndex = 0;
    }

    // treat para properties as text properties
        // ... with the FormatToTextAttr we get autofmts that correspond to the *effective* attr set difference
        // effective attr set: para props + charfmts + autofmt *in that order*
        // ... and the charfmt must be *nominally* the same

    SfxPoolItem const* activeCharAttrsEnd[RES_CHRATR_END - RES_CHRATR_BEGIN + 1] = { nullptr, };
    if (&rStartNode != &pRLEnd->nNode.GetNode())
    {   // nodes' attributes are only needed if there are different nodes
        InsertCharAttrs(activeCharAttrsEnd,
                pRLEnd->nNode.GetNode().GetTextNode()->GetSwAttrSet());
    }

    if (SwpHints *const pEndHints = pRLEnd->nNode.GetNode().GetTextNode()->GetpSwpHints())
    {
        // check hint starts of hints that start within and end after
#ifndef NDEBUG
        sal_Int32 const nRedlineStart(&rStartNode == &pRLEnd->nNode.GetNode()
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
            if (pRLEnd->nContent.GetIndex() < pAttr->GetStart())
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
            if (*pAttr->End() <= pRLEnd->nContent.GetIndex())
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
                            : *pAttr->GetAutoFormat().GetStyleHandle().get());
                        InsertCharAttrs(activeCharAttrsEnd, rSet);

                    }
                    break;
                // SwTextNode::SetAttr puts it into AUTOFMT which is quite
                // sensible so it doesn't actually exist as a hint
                case RES_TXTATR_UNKNOWN_CONTAINER:
                default: assert(false);
            }
        }
        if (&rStartNode != &pRLEnd->nNode.GetNode())
        {
            // need to iterate the nEndIndex forward too so the loop in the
            // caller can look for the right ends in the next iteration
            for (nEndIndex = 0; nEndIndex < pEndHints->Count(); ++nEndIndex)
            {
                SwTextAttr *const pAttr(pEndHints->GetSortedByEnd(nEndIndex));
                if (!pAttr->End())
                    continue;
                if (pRLEnd->nContent.GetIndex() < *pAttr->End())
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
        // all of these are poolable
//        assert(!activeCharAttrsStart[i] || activeCharAttrsStart[i]->GetItemPool()->IsItemPoolable(*activeCharAttrsStart[i]));
        if (activeCharAttrsStart[i] != activeCharAttrsEnd[i])
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
                sal_Int32 const nNextEnd = *pAttr->GetAnyEnd();
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
        if (aChar < CH_TXT_ATR_FORMELEMENT
            || aChar > CH_TXT_ATR_FIELDEND)
        {
            ++p;
        }
        else
        {
            break;
        }
    }
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
                assert(redline.second.first->End()->nNode.GetIndex() <= m_pMergedPara->pLastNode->GetIndex()
                    || !redline.second.first->End()->nNode.GetNode().IsTextNode());
                if (CanSkipOverRedline(*pTextNode, redline.first, *redline.second.first,
                        nStartIndex, nEndIndex, m_nPosition == redline.first))
                {   // if current position is start of the redline, must skip!
                    nActRedline += redline.second.second;
                    if (&redline.second.first->End()->nNode.GetNode() != pTextNode)
                    {
                        pTextNode = redline.second.first->End()->nNode.GetNode().GetTextNode();
                        nPosition = redline.second.first->End()->nContent.GetIndex();
                    }
                    else
                    {
                        nPosition = redline.second.first->End()->nContent.GetIndex();
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

class SwMinMaxArgs
{
public:
    VclPtr<OutputDevice> pOut;
    SwViewShell const * pSh;
    sal_uLong &rMin;
    sal_uLong &rAbsMin;
    long nRowWidth;
    long nWordWidth;
    long nWordAdd;
    sal_Int32 nNoLineBreak;
    SwMinMaxArgs( OutputDevice* pOutI, SwViewShell const * pShI, sal_uLong& rMinI, sal_uLong &rAbsI )
        : pOut( pOutI ), pSh( pShI ), rMin( rMinI ), rAbsMin( rAbsI )
        { nRowWidth = nWordWidth = nWordAdd = 0; nNoLineBreak = COMPLETE_STRING; }
    void Minimum( long nNew ) const { if( static_cast<long>(rMin) < nNew ) rMin = nNew; }
    void NewWord() { nWordAdd = nWordWidth = 0; }
};

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
        if( nIdx <= aBndry.startPos && nIdx && nIdx-1 != rArg.nNoLineBreak )
            rArg.NewWord();
        if( nStop == nIdx )
            ++nStop;
        if( nStop > nEnd )
            nStop = nEnd;

        SwDrawTextInfo aDrawInf(rArg.pSh, *rArg.pOut, rText, nIdx, nStop - nIdx);
        long nCurrentWidth = pFnt->GetTextSize_( aDrawInf ).Width();
        rArg.nRowWidth += nCurrentWidth;
        if( bClear )
            rArg.NewWord();
        else
        {
            rArg.nWordWidth += nCurrentWidth;
            if( static_cast<long>(rArg.rAbsMin) < rArg.nWordWidth )
                rArg.rAbsMin = rArg.nWordWidth;
            rArg.Minimum( rArg.nWordWidth + rArg.nWordAdd );
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

class SwMinMaxNodeArgs
{
public:
    sal_uLong nMaxWidth;    // sum of all frame widths
    long nMinWidth;         // biggest frame
    long nLeftRest;         // space not already covered by frames in the left margin
    long nRightRest;        // space not already covered by frames in the right margin
    long nLeftDiff;         // Min/Max-difference of the frame in the left margin
    long nRightDiff;        // Min/Max-difference of the frame in the right margin
    sal_uLong nIndx;        // index of the node
    void Minimum( long nNew ) { if( nNew > nMinWidth ) nMinWidth = nNew; }
};

static void lcl_MinMaxNode( SwFrameFormat* pNd, SwMinMaxNodeArgs* pIn )
{
    const SwFormatAnchor& rFormatA = pNd->GetAnchor();

    if ((RndStdIds::FLY_AT_PARA != rFormatA.GetAnchorId()) &&
        (RndStdIds::FLY_AT_CHAR != rFormatA.GetAnchorId()))
    {
        return;
    }

    const SwPosition *pPos = rFormatA.GetContentAnchor();
    OSL_ENSURE(pPos && pIn, "Unexpected NULL arguments");
    if (!pPos || !pIn || pIn->nIndx != pPos->nNode.GetIndex())
        return;

    long nMin, nMax;
    SwHTMLTableLayout *pLayout = nullptr;
    const bool bIsDrawFrameFormat = pNd->Which()==RES_DRAWFRMFMT;
    if( !bIsDrawFrameFormat )
    {
        // Does the frame contain a table at the start or the end?
        const SwNodes& rNodes = pNd->GetDoc()->GetNodes();
        const SwFormatContent& rFlyContent = pNd->GetContent();
        sal_uLong nStt = rFlyContent.GetContentIdx()->GetIndex();
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

    long nDiff;
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
    nMin += rLR.GetLeft();
    nMin += rLR.GetRight();
    nMax += rLR.GetLeft();
    nMax += rLR.GetRight();

    if( css::text::WrapTextMode_THROUGH == pNd->GetSurround().GetSurround() )
    {
        pIn->Minimum( nMin );
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
                pIn->nRightRest -= pIn->nRightDiff;
                pIn->nRightDiff = nDiff;
            }
            if( text::RelOrientation::FRAME != rOrient.GetRelationOrient() )
            {
                if( pIn->nRightRest > 0 )
                    pIn->nRightRest = 0;
            }
            pIn->nRightRest -= nMin;
            break;
        }
        case text::HoriOrientation::LEFT:
        {
            if( nDiff )
            {
                pIn->nLeftRest -= pIn->nLeftDiff;
                pIn->nLeftDiff = nDiff;
            }
            if( text::RelOrientation::FRAME != rOrient.GetRelationOrient() &&
                pIn->nLeftRest < 0 )
                pIn->nLeftRest = 0;
            pIn->nLeftRest -= nMin;
            break;
        }
        default:
        {
            pIn->nMaxWidth += nMax;
            pIn->Minimum( nMin );
        }
    }
}

#define FLYINCNT_MIN_WIDTH 284

/**
 * Changing this method very likely requires changing of GetScalingOfSelectedText
 * This one is called exclusively from import filters, so there is no layout.
 */
void SwTextNode::GetMinMaxSize( sal_uLong nIndex, sal_uLong& rMin, sal_uLong &rMax,
                               sal_uLong& rAbsMin ) const
{
    SwViewShell const * pSh = GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    OutputDevice* pOut = nullptr;
    if( pSh )
        pOut = pSh->GetWin();
    if( !pOut )
        pOut = Application::GetDefaultDevice();

    MapMode aOldMap( pOut->GetMapMode() );
    pOut->SetMapMode( MapMode( MapUnit::MapTwip ) );

    rMin = 0;
    rMax = 0;
    rAbsMin = 0;

    const SvxLRSpaceItem &rSpace = GetSwAttrSet().GetLRSpace();
    long nLROffset = rSpace.GetTextLeft() + GetLeftMarginWithNum( true );
    short nFLOffs;
    // For enumerations a negative first line indentation is probably filled already
    if( !GetFirstLineOfsWithNum( nFLOffs ) || nFLOffs > nLROffset )
        nLROffset = nFLOffs;

    SwMinMaxNodeArgs aNodeArgs;
    aNodeArgs.nMinWidth = 0;
    aNodeArgs.nMaxWidth = 0;
    aNodeArgs.nLeftRest = nLROffset;
    aNodeArgs.nRightRest = rSpace.GetRight();
    aNodeArgs.nLeftDiff = 0;
    aNodeArgs.nRightDiff = 0;
    if( nIndex )
    {
        SwFrameFormats* pTmp = const_cast<SwFrameFormats*>(GetDoc()->GetSpzFrameFormats());
        if( pTmp )
        {
            aNodeArgs.nIndx = nIndex;
            for( SwFrameFormat *pFormat : *pTmp )
                lcl_MinMaxNode( pFormat, &aNodeArgs );
        }
    }
    if( aNodeArgs.nLeftRest < 0 )
        aNodeArgs.Minimum( nLROffset - aNodeArgs.nLeftRest );
    aNodeArgs.nLeftRest -= aNodeArgs.nLeftDiff;
    if( aNodeArgs.nLeftRest < 0 )
        aNodeArgs.nMaxWidth -= aNodeArgs.nLeftRest;

    if( aNodeArgs.nRightRest < 0 )
        aNodeArgs.Minimum( rSpace.GetRight() - aNodeArgs.nRightRest );
    aNodeArgs.nRightRest -= aNodeArgs.nRightDiff;
    if( aNodeArgs.nRightRest < 0 )
        aNodeArgs.nMaxWidth -= aNodeArgs.nRightRest;

    SwScriptInfo aScriptInfo;
    SwAttrIter aIter( *const_cast<SwTextNode*>(this), aScriptInfo );
    TextFrameIndex nIdx(0);
    aIter.SeekAndChgAttrIter( nIdx, pOut );
    TextFrameIndex nLen(m_Text.getLength());
    long nCurrentWidth = 0;
    long nAdd = 0;
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
               !pHint )
        {
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
                if( static_cast<long>(rMax) < aArg.nRowWidth )
                    rMax = aArg.nRowWidth;
                aArg.nRowWidth = 0;
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
                aArg.nWordWidth += nCurrentWidth;
                aArg.nRowWidth += nCurrentWidth;
                if( static_cast<long>(rAbsMin) < aArg.nWordWidth )
                    rAbsMin = aArg.nWordWidth;
                aArg.Minimum( aArg.nWordWidth + aArg.nWordAdd );
                aArg.nNoLineBreak = sal_Int32(nIdx++);
            }
            break;
            case CH_TXTATR_BREAKWORD:
            case CH_TXTATR_INWORD:
            {
                if( !pHint )
                    break;
                long nOldWidth = aArg.nWordWidth;
                long nOldAdd = aArg.nWordAdd;
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
                        nCurrentWidth += rLR.GetLeft();
                        nCurrentWidth += rLR.GetRight();
                        aArg.nWordAdd = nOldWidth + nOldAdd;
                        aArg.nWordWidth = nCurrentWidth;
                        aArg.nRowWidth += nCurrentWidth;
                        if( static_cast<long>(rAbsMin) < aArg.nWordWidth )
                            rAbsMin = aArg.nWordWidth;
                        aArg.Minimum( aArg.nWordWidth + aArg.nWordAdd );
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
                    default: aArg.nWordWidth = nOldWidth;
                        aArg.nWordAdd = nOldAdd;

                }
                aIter.SeekAndChgAttrIter( ++nIdx, pOut );
            }
            break;
        }
    }
    if( static_cast<long>(rMax) < aArg.nRowWidth )
        rMax = aArg.nRowWidth;

    nLROffset += rSpace.GetRight();

    rAbsMin += nLROffset;
    rAbsMin += nAdd;
    rMin += nLROffset;
    rMin += nAdd;
    if( static_cast<long>(rMin) < aNodeArgs.nMinWidth )
        rMin = aNodeArgs.nMinWidth;
    if( static_cast<long>(rAbsMin) < aNodeArgs.nMinWidth )
        rAbsMin = aNodeArgs.nMinWidth;
    rMax += aNodeArgs.nMaxWidth;
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
    assert(GetOfst() <= nStart && (!GetFollow() || nStart < GetFollow()->GetOfst()));
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
    SwTextAttrEnd aAttr( aItem, 0, COMPLETE_STRING );
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
            OUString sTmp( CH_BLANK );
            SwDrawTextInfo aDrawInf(pSh, *pOut, sTmp, 0, 1);
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
        } // end of while
    }

    nWidth = std::max( nWidth, nProWidth );

    // search for the line containing nStart
    if (HasPara())
    {
        SwTextInfo aInf(this);
        SwTextIter aLine(this, &aInf);
        aLine.CharToLine( nStart );
        pOut->SetMapMode( aOldMap );
        return static_cast<sal_uInt16>( nWidth ?
            ( ( 100 * aLine.GetCurr()->Height() ) / nWidth ) : 0 );
    }
    // no frame or no paragraph, we take the height of the character
    // at nStart as line height

    aIter.SeekAndChgAttrIter( nStart, pOut );
    pOut->SetMapMode( aOldMap );

    SwDrawTextInfo aDrawInf(pSh, *pOut, GetText(), sal_Int32(nStart), 1);
    return static_cast<sal_uInt16>( nWidth ? ((100 * aIter.GetFnt()->GetTextSize_( aDrawInf ).Height()) / nWidth ) : 0 );
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
        SwPosition aPos( *this );
        aPos.nContent += nIdx;

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
