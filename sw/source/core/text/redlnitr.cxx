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

void SwAttrIter::CtorInitAttrIter( SwTextNode& rTextNode, SwScriptInfo& rScrInf, SwTextFrame const * pFrame )
{
    // during HTML-Import it can happen, that no layout exists
    SwRootFrame* pRootFrame = rTextNode.getIDocumentLayoutAccess().GetCurrentLayout();
    m_pViewShell = pRootFrame ? pRootFrame->GetCurrShell() : nullptr;

    m_pScriptInfo = &rScrInf;

    // attributes set at the whole paragraph
    m_pAttrSet = rTextNode.GetpSwAttrSet();
    // attribute array
    m_pHints = rTextNode.GetpSwpHints();

    // Build a font matching the default paragraph style:
    SwFontAccess aFontAccess( &rTextNode.GetAnyFormatColl(), m_pViewShell );
    delete m_pFont;
    m_pFont = new SwFont( aFontAccess.Get()->GetFont() );

    // set font to vertical if frame layout is vertical
    bool bVertLayout = false;
    bool bRTL = false;
    if ( pFrame )
    {
        if ( pFrame->IsVertical() )
        {
            bVertLayout = true;
            m_pFont->SetVertical( m_pFont->GetOrientation(), true );
        }
        bRTL = pFrame->IsRightToLeft();
    }

    // Initialize the default attribute of the attribute handler
    // based on the attribute array cached together with the font.
    // If any further attributes for the paragraph are given in pAttrSet
    // consider them during construction of the default array, and apply
    // them to the font
    m_aAttrHandler.Init( aFontAccess.Get()->GetDefault(), m_pAttrSet,
                       *rTextNode.getIDocumentSettingAccess(), m_pViewShell, *m_pFont, bVertLayout );

    m_aMagicNo[SwFontScript::Latin] = m_aMagicNo[SwFontScript::CJK] = m_aMagicNo[SwFontScript::CTL] = nullptr;

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

    const SwExtTextInput* pExtInp = pDoc->GetExtTextInput( rTextNode );
    const bool bShow = IDocumentRedlineAccess::IsShowChanges( rIDRA.GetRedlineFlags() );
    if( pExtInp || bShow )
    {
        const SwRedlineTable::size_type nRedlPos = rIDRA.GetRedlinePos( rTextNode, USHRT_MAX );
        if( pExtInp || SwRedlineTable::npos != nRedlPos )
        {
            const std::vector<ExtTextInputAttr> *pArr = nullptr;
            sal_Int32 nInputStt = 0;
            if( pExtInp )
            {
                pArr = &pExtInp->GetAttrs();
                nInputStt = pExtInp->Start()->nContent.GetIndex();
                Seek( 0 );
            }

            m_pRedline = new SwRedlineItr( rTextNode, *m_pFont, m_aAttrHandler, nRedlPos,
                                        bShow, pArr, nInputStt );

            if( m_pRedline->IsOn() )
                ++m_nChgCnt;
        }
    }
}

// The Redline-Iterator
// The following information/states exist in RedlineIterator:
//
// nFirst is the first index of RedlineTable, which overlaps with the paragraph.
//
// nAct is the currently active (if bOn is set) or the next possible index.
// nStart and nEnd give you the borders of the object within the paragraph.
//
// If bOn is set, the font has been manipulated according to it.
//
// If nAct is set to COMPLETE_STRING (via Reset()), then currently no
// Redline is active, nStart and nEnd are invalid.
SwRedlineItr::SwRedlineItr( const SwTextNode& rTextNd, SwFont& rFnt,
                            SwAttrHandler& rAH, sal_Int32 nRed, bool bShw,
                            const std::vector<ExtTextInputAttr> *pArr,
                            sal_Int32 nExtStart )
    : rDoc( *rTextNd.GetDoc() ), rAttrHandler( rAH ),
      nNdIdx( rTextNd.GetIndex() ), nFirst( nRed ),
      nAct( COMPLETE_STRING ), bOn( false ), bShow( bShw )
{
    if( pArr )
        pExt = new SwExtend( *pArr, nExtStart );
    else
        pExt = nullptr;
    Seek (rFnt, 0, COMPLETE_STRING);
}

SwRedlineItr::~SwRedlineItr() COVERITY_NOEXCEPT_FALSE
{
    Clear( nullptr );
    delete pExt;
}

// The return value of SwRedlineItr::Seek tells you if the current font
// has been manipulated by leaving (-1) or accessing (+1) of a section
short SwRedlineItr::Seek_(SwFont& rFnt, sal_Int32 nNew, sal_Int32 nOld)
{
    short nRet = 0;
    if( ExtOn() )
        return 0; // Abbreviation: if we're within an ExtendTextInputs
                  // there can't be other changes of attributes (not even by redlining)
    if( bShow )
    {
        if( bOn )
        {
            if( nNew >= nEnd )
            {
                --nRet;
                Clear_( &rFnt );    // We go behind the current section
                ++nAct;             // and check the next one
            }
            else if( nNew < nStart )
            {
                --nRet;
                Clear_( &rFnt );    // We go in front of the current section
                if( nAct > nFirst )
                    nAct = nFirst;  // the test has to run from the beginning
                else
                    return nRet + EnterExtend( rFnt, nNew ); // There's none prior to us
            }
            else
                return nRet + EnterExtend( rFnt, nNew ); // We stayed in the same section
        }
        if( COMPLETE_STRING == nAct || nOld > nNew )
            nAct = nFirst;

        nStart = COMPLETE_STRING;
        nEnd = COMPLETE_STRING;

        for( ; nAct < (sal_Int32)rDoc.getIDocumentRedlineAccess().GetRedlineTable().size() ; ++nAct )
        {
            rDoc.getIDocumentRedlineAccess().GetRedlineTable()[ nAct ]->CalcStartEnd( nNdIdx, nStart, nEnd );

            if( nNew < nEnd )
            {
                if( nNew >= nStart ) // only possible candidate
                {
                    bOn = true;
                    const SwRangeRedline *pRed = rDoc.getIDocumentRedlineAccess().GetRedlineTable()[ nAct ];

                    if (pSet)
                        pSet->ClearItem();
                    else
                    {
                        SwAttrPool& rPool =
                            const_cast<SwDoc&>(rDoc).GetAttrPool();
                        pSet = o3tl::make_unique<SfxItemSet>(rPool, svl::Items<RES_CHRATR_BEGIN, RES_CHRATR_END-1>{});
                    }

                    if( 1 < pRed->GetStackCount() )
                        FillHints( pRed->GetAuthor( 1 ), pRed->GetType( 1 ) );
                    FillHints( pRed->GetAuthor(), pRed->GetType() );

                    SfxWhichIter aIter( *pSet );
                    sal_uInt16 nWhich = aIter.FirstWhich();
                    while( nWhich )
                    {
                        const SfxPoolItem* pItem;
                        if( ( nWhich < RES_CHRATR_END ) &&
                            ( SfxItemState::SET == pSet->GetItemState( nWhich, true, &pItem ) ) )
                        {
                            SwTextAttr* pAttr = MakeRedlineTextAttr(
                                const_cast<SwDoc&>(rDoc),
                                *const_cast<SfxPoolItem*>(pItem) );
                            pAttr->SetPriorityAttr( true );
                            m_Hints.push_back(pAttr);
                            rAttrHandler.PushAndChg( *pAttr, rFnt );
                        }
                        nWhich = aIter.NextWhich();
                    }

                    ++nRet;
                }
                break;
            }
            nStart = COMPLETE_STRING;
            nEnd = COMPLETE_STRING;
        }
    }
    return nRet + EnterExtend( rFnt, nNew );
}

void SwRedlineItr::FillHints( std::size_t nAuthor, RedlineType_t eType )
{
    switch ( eType )
    {
        case nsRedlineType_t::REDLINE_INSERT:
            SW_MOD()->GetInsertAuthorAttr(nAuthor, *pSet);
            break;
        case nsRedlineType_t::REDLINE_DELETE:
            SW_MOD()->GetDeletedAuthorAttr(nAuthor, *pSet);
            break;
        case nsRedlineType_t::REDLINE_FORMAT:
        case nsRedlineType_t::REDLINE_FMTCOLL:
            SW_MOD()->GetFormatAuthorAttr(nAuthor, *pSet);
            break;
        default:
            break;
    }
}

void SwRedlineItr::ChangeTextAttr( SwFont* pFnt, SwTextAttr const &rHt, bool bChg )
{
    OSL_ENSURE( IsOn(), "SwRedlineItr::ChangeTextAttr: Off?" );

    if( !bShow && !pExt )
        return;

    if( bChg )
    {
        if ( pExt && pExt->IsOn() )
            rAttrHandler.PushAndChg( rHt, *pExt->GetFont() );
        else
            rAttrHandler.PushAndChg( rHt, *pFnt );
    }
    else
    {
        OSL_ENSURE( ! pExt || ! pExt->IsOn(), "Pop of attribute during opened extension" );
        rAttrHandler.PopAndChg( rHt, *pFnt );
    }
}

void SwRedlineItr::Clear_( SwFont* pFnt )
{
    OSL_ENSURE( bOn, "SwRedlineItr::Clear: Off?" );
    bOn = false;
    while (!m_Hints.empty())
    {
        SwTextAttr *pPos = m_Hints.front();
        m_Hints.pop_front();
        if( pFnt )
            rAttrHandler.PopAndChg( *pPos, *pFnt );
        else
            rAttrHandler.Pop( *pPos );
        SwTextAttr::Destroy(pPos, const_cast<SwDoc&>(rDoc).GetAttrPool() );
    }
}

sal_Int32 SwRedlineItr::GetNextRedln_( sal_Int32 nNext )
{
    nNext = NextExtend( nNext );
    if( !bShow || COMPLETE_STRING == nFirst )
        return nNext;
    if( COMPLETE_STRING == nAct )
    {
        nAct = nFirst;
        rDoc.getIDocumentRedlineAccess().GetRedlineTable()[ nAct ]->CalcStartEnd( nNdIdx, nStart, nEnd );
    }
    if( bOn || !nStart )
    {
        if( nEnd < nNext )
            nNext = nEnd;
    }
    else if( nStart < nNext )
        nNext = nStart;
    return nNext;
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
    if( nFirst == COMPLETE_STRING )
        return false;
    if( nChkEnd == nChkStart ) // empty lines look one char further
        ++nChkEnd;
    sal_Int32 nOldStart = nStart;
    sal_Int32 nOldEnd = nEnd;
    sal_Int32 nOldAct = nAct;
    bool bRet = false;

    for( nAct = nFirst; nAct < (sal_Int32)rDoc.getIDocumentRedlineAccess().GetRedlineTable().size() ; ++nAct )
    {
        rDoc.getIDocumentRedlineAccess().GetRedlineTable()[ nAct ]->CalcStartEnd( nNdIdx, nStart, nEnd );
        if( nChkEnd < nStart )
            break;
        if( nChkStart <= nEnd && ( nChkEnd > nStart || COMPLETE_STRING == nEnd ) )
        {
            bRet = true;
            break;
        }
    }

    nStart = nOldStart;
    nEnd = nOldEnd;
    nAct = nOldAct;
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
        rFnt.SetColor( Color( COL_RED ) );

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
    OSL_ENSURE( !pFnt, "SwExtend: Enter with Font" );
    nPos = nNew;
    if( Inside() )
    {
        pFnt.reset( new SwFont( rFnt ) );
        ActualizeFont( rFnt, rArr[ nPos - nStart ] );
        return 1;
    }
    return 0;
}

bool SwExtend::Leave_(SwFont& rFnt, sal_Int32 nNew)
{
    OSL_ENSURE( Inside(), "SwExtend: Leave without Enter" );
    const ExtTextInputAttr nOldAttr = rArr[ nPos - nStart ];
    nPos = nNew;
    if( Inside() )
    {   // We stayed within the ExtendText-section
        const ExtTextInputAttr nAttr = rArr[ nPos - nStart ];
        if( nOldAttr != nAttr ) // Is there an (inner) change of attributes?
        {
            rFnt = *pFnt;
            ActualizeFont( rFnt, nAttr );
        }
    }
    else
    {
        rFnt = *pFnt;
        pFnt.reset();
        return true;
    }
    return false;
}

sal_Int32 SwExtend::Next( sal_Int32 nNext )
{
    if( nPos < nStart )
    {
        if( nNext > nStart )
            nNext = nStart;
    }
    else if( nPos < nEnd )
    {
        sal_Int32 nIdx = nPos - nStart;
        const ExtTextInputAttr nAttr = rArr[ nIdx ];
        while( static_cast<size_t>(++nIdx) < rArr.size() && nAttr == rArr[ nIdx ] )
            ; //nothing
        nIdx = nIdx + nStart;
        if( nNext > nIdx )
            nNext = nIdx;
    }
    return nNext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
