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
#include <vcl/svapp.hxx>
#include "redlnitr.hxx"
#include <extinput.hxx>
#include <sfx2/printer.hxx>
#include <vcl/window.hxx>

using namespace ::com::sun::star;

namespace sw {

std::unique_ptr<sw::MergedPara>
CheckParaRedlineMerge(SwTextFrame & rFrame, SwTextNode & rTextNode)
{
    IDocumentRedlineAccess const& rIDRA = rTextNode.getIDocumentRedlineAccess();
    if (!rFrame.getRootFrame()->IsHideRedlines())
    {
        return nullptr;
    }
    bool bHaveRedlines(false);
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
            pNode = pEnd->nNode.GetNode().GetTextNode();
            assert(pNode);
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
    return o3tl::make_unique<sw::MergedPara>(std::move(extents),
            mergedText.makeStringAndClear(), pParaPropsNode, &rTextNode);
}

} // namespace sw

void SwAttrIter::CtorInitAttrIter(SwTextNode & rTextNode,
        SwScriptInfo & rScriptInfo, SwTextFrame *const pFrame)
{
    // during HTML-Import it can happen, that no layout exists
    SwRootFrame* pRootFrame = rTextNode.getIDocumentLayoutAccess().GetCurrentLayout();
    m_pViewShell = pRootFrame ? pRootFrame->GetCurrShell() : nullptr;

    m_pScriptInfo = &rScriptInfo;

    // attribute array
    m_pHints = rTextNode.GetpSwpHints();

    // Build a font matching the default paragraph style:
    SwFontAccess aFontAccess( &rTextNode.GetAnyFormatColl(), m_pViewShell );
    delete m_pFont;
    m_pFont = new SwFont( aFontAccess.Get()->GetFont() );

    // set font to vertical if frame layout is vertical
    bool bVertLayout = false;
    bool bRTL = false;
    sw::MergedPara const* pMerged(nullptr);
    if ( pFrame )
    {
        if ( pFrame->IsVertical() )
        {
            bVertLayout = true;
            m_pFont->SetVertical( m_pFont->GetOrientation(), true );
        }
        bRTL = pFrame->IsRightToLeft();
        pMerged = pFrame->GetMergedPara();
    }

    // Initialize the default attribute of the attribute handler
    // based on the attribute array cached together with the font.
    // If any further attributes for the paragraph are given in pAttrSet
    // consider them during construction of the default array, and apply
    // them to the font
    m_aAttrHandler.Init(aFontAccess.Get()->GetDefault(), rTextNode.GetpSwAttrSet(),
                       *rTextNode.getIDocumentSettingAccess(), m_pViewShell, *m_pFont, bVertLayout );

    m_aMagicNo[SwFontScript::Latin] = m_aMagicNo[SwFontScript::CJK] = m_aMagicNo[SwFontScript::CTL] = nullptr;

    // TODO must init m_pRedline before this
    // determine script changes if not already done for current paragraph
    assert(m_pScriptInfo);
    if ( m_pScriptInfo->GetInvalidityA() != COMPLETE_STRING )
         m_pScriptInfo->InitScriptInfo( rTextNode, bRTL );

    assert(g_pBreakIt && g_pBreakIt->GetBreakIter().is());

    m_pFont->SetActual( SwScriptInfo::WhichFont( 0, nullptr, m_pScriptInfo ) );

    sal_Int32 nChg = 0;
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
    } while (nChg < rTextNode.GetText().getLength());

    m_nStartIndex = m_nEndIndex = m_nPosition = m_nChgCnt = 0;
    m_nPropFont = 0;
    SwDoc* pDoc = rTextNode.GetDoc();
    const IDocumentRedlineAccess& rIDRA = rTextNode.getIDocumentRedlineAccess();

    // sw_redlinehide: this is a Ring - pExtInp is the first PaM that's inside
    // the node.  It's not clear whether there can be more than 1 PaM in the
    // Ring, and this code doesn't handle that case; neither did the old code.
    const SwExtTextInput* pExtInp = pDoc->GetExtTextInput( rTextNode );
    if (!pExtInp && pMerged)
    {
        SwTextNode const* pNode(&rTextNode);
        for (auto const& rExtent : pMerged->extents)
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
    if (pExtInp || pMerged || bShow)
    {
        const SwRedlineTable::size_type nRedlPos = rIDRA.GetRedlinePos( rTextNode, USHRT_MAX );
        if (pExtInp || pMerged || SwRedlineTable::npos != nRedlPos)
        {
            const std::vector<ExtTextInputAttr> *pArr = nullptr;
            if( pExtInp )
            {
                pArr = &pExtInp->GetAttrs();
                Seek( 0 );
            }

            m_pRedline = new SwRedlineItr( rTextNode, *m_pFont, m_aAttrHandler, nRedlPos,
                            bShow, pArr, pExtInp ? pExtInp->Start() : nullptr);

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
                            SwAttrHandler& rAH, sal_Int32 nRed, bool bShow,
                            const std::vector<ExtTextInputAttr> *pArr,
                            SwPosition const*const pExtInputStart)
    : m_rDoc( *rTextNd.GetDoc() )
    , m_rAttrHandler( rAH )
    , m_nNdIdx( rTextNd.GetIndex() )
    , m_nFirst( nRed )
    , m_nAct( SwRedlineTable::npos )
    , m_bOn( false )
    , m_bShow( bShow )
{
    if( pArr )
    {
        assert(pExtInputStart);
        m_pExt = new SwExtend(*pArr, pExtInputStart->nNode.GetIndex(),
                                     pExtInputStart->nContent.GetIndex());
    }
    else
        m_pExt = nullptr;
    Seek (rFnt, 0, COMPLETE_STRING);
}

SwRedlineItr::~SwRedlineItr() COVERITY_NOEXCEPT_FALSE
{
    Clear( nullptr );
    delete m_pExt;
}

// The return value of SwRedlineItr::Seek tells you if the current font
// has been manipulated by leaving (-1) or accessing (+1) of a section
short SwRedlineItr::Seek_(SwFont& rFnt, sal_Int32 nNew, sal_Int32 nOld)
{
    short nRet = 0;
    if( ExtOn() )
        return 0; // Abbreviation: if we're within an ExtendTextInputs
                  // there can't be other changes of attributes (not even by redlining)
    if (m_bShow)
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
                    return nRet + EnterExtend( rFnt, nNew ); // There's none prior to us
            }
            else
                return nRet + EnterExtend( rFnt, nNew ); // We stayed in the same section
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
    return nRet + EnterExtend( rFnt, nNew );
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

    if (!m_bShow && !m_pExt)
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

// TODO this must be ITERABLE pass in members as parameter
std::pair<sal_Int32, SwRangeRedline const*>
SwRedlineItr::GetNextRedln(sal_Int32 nNext, SwTextNode const*const pNode, SwRedlineTable::size_type & rAct)
{
    sal_Int32 nStart(m_nStart);
    sal_Int32 nEnd(m_nEnd);
    nNext = NextExtend( nNext );
    if (!m_bShow || SwRedlineTable::npos == m_nFirst)
        return std::make_pair(nNext, nullptr);
    if (SwRedlineTable::npos == rAct)
    {
        rAct = m_nFirst; // TODO???
        m_rDoc.getIDocumentRedlineAccess().GetRedlineTable()[rAct]->CalcStartEnd(pNode->GetIndex(), nStart, nEnd);
    }
    if (rAct != m_nAct)
    {
        m_rDoc.getIDocumentRedlineAccess().GetRedlineTable()[rAct]->CalcStartEnd(pNode->GetIndex(), nStart, nEnd);
    }
    if (m_bOn || !nStart)
    {
        if (nEnd < nNext)
            nNext = nEnd;
    }
    else if (nStart <= nNext)
    {
        nNext = nStart;
        if (!m_bShow)
        {
            SwRangeRedline const* pRedline = m_rDoc.getIDocumentRedlineAccess().GetRedlineTable()[rAct];
            if (pRedline->GetType() == nsRedlineType_t::REDLINE_DELETE)
            {
                ++rAct;
                while (rAct < m_rDoc.getIDocumentRedlineAccess().GetRedlineTable().size())
                {
                    SwRangeRedline *const pNext = m_rDoc.getIDocumentRedlineAccess().GetRedlineTable()[rAct];
                    if (pRedline->End() < pNext->Start())
                    {
                        break; // done for now
                    }
                    else if (pNext->Start() == pRedline->End() && pNext->GetType() == nsRedlineType_t::REDLINE_DELETE)
                    {
                        // consecutive delete - continue
                        pRedline = pNext;
                    }
                    ++rAct;
                }
                return std::make_pair(nNext, pRedline);
            }
        }
    }
    return std::make_pair(nNext, nullptr);
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

bool SwRedlineItr::CheckLine( sal_Int32 nChkStart, sal_Int32 nChkEnd )
{
    if (m_nFirst == SwRedlineTable::npos)
        return false;
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

short SwExtend::Enter(SwFont& rFnt, sal_Int32 nNew)
{
    OSL_ENSURE( !Inside(), "SwExtend: Enter without Leave" );
    OSL_ENSURE( !m_pFont, "SwExtend: Enter with Font" );
    m_nPos = nNew;
    if( Inside() )
    {
        m_pFont.reset( new SwFont(rFnt) );
        ActualizeFont( rFnt, m_rArr[m_nPos - m_nStart] );
        return 1;
    }
    return 0;
}

bool SwExtend::Leave_(SwFont& rFnt, sal_Int32 nNew)
{
    OSL_ENSURE( Inside(), "SwExtend: Leave without Enter" );
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

sal_Int32 SwExtend::Next( sal_Int32 nNext )
{
    (void) m_nNode; // TODO use it here
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
