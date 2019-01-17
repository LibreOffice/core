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
#include <editeng/unolingu.hxx>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <viewopt.hxx>
#include <viewsh.hxx>
#include <SwPortionHandler.hxx>
#include "porhyph.hxx"
#include "inftxt.hxx"
#include "itrform2.hxx"
#include "guess.hxx"
#include <splargs.hxx>
#include <rootfrm.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::i18n;

Reference< XHyphenatedWord >  SwTextFormatInfo::HyphWord(
                                const OUString &rText, const sal_Int32 nMinTrail )
{
    if( rText.getLength() < 4 || m_pFnt->IsSymbol(m_pVsh) )
        return nullptr;
    Reference< XHyphenator >  xHyph = ::GetHyphenator();
    Reference< XHyphenatedWord > xHyphWord;

    if( xHyph.is() )
        xHyphWord = xHyph->hyphenate( rText,
                            g_pBreakIt->GetLocale( m_pFnt->GetLanguage() ),
                            rText.getLength() - nMinTrail, GetHyphValues() );
    return xHyphWord;

}

/**
 * We format a row for interactive hyphenation
 */
bool SwTextFrame::Hyphenate(SwInterHyphInfoTextFrame & rHyphInf)
{
    vcl::RenderContext* pRenderContext = getRootFrame()->GetCurrShell()->GetOut();
    OSL_ENSURE( ! IsVertical() || ! IsSwapped(),"swapped frame at SwTextFrame::Hyphenate" );

    assert(g_pBreakIt && g_pBreakIt->GetBreakIter().is());

    // We lock it, to start with
    OSL_ENSURE( !IsLocked(), "SwTextFrame::Hyphenate: this is locked" );

    // The frame::Frame must have a valid SSize!
    Calc(pRenderContext);
    GetFormatted();

    bool bRet = false;
    if( !IsEmpty() )
    {
        // We always need to enable hyphenation
        // Don't be afraid: the SwTextIter saves the old row in the hyphenate
        TextFrameLockGuard aLock( this );

        if ( IsVertical() )
            SwapWidthAndHeight();

        SwTextFormatInfo aInf( getRootFrame()->GetCurrShell()->GetOut(), this, true ); // true for interactive hyph!
        SwTextFormatter aLine( this, &aInf );
        aLine.CharToLine( rHyphInf.m_nStart );

        // If we're within the first word of a row, it could've been hyphenated
        // in the row earlier.
        // That's why we go one row back.
        if( aLine.Prev() )
        {
            SwLinePortion *pPor = aLine.GetCurr()->GetFirstPortion();
            while( pPor->GetNextPortion() )
                pPor = pPor->GetNextPortion();
            if( pPor->GetWhichPor() == PortionType::SoftHyphen ||
                pPor->GetWhichPor() == PortionType::SoftHyphenStr )
                aLine.Next();
        }

        const TextFrameIndex nEnd = rHyphInf.m_nEnd;
        while( !bRet && aLine.GetStart() < nEnd )
        {
            bRet = aLine.Hyphenate( rHyphInf );
            if( !aLine.Next() )
                break;
        }

        if ( IsVertical() )
            SwapWidthAndHeight();
    }
    return bRet;
}

/**
 * We format a row for interactive hyphenation
 * We can assume that we've already formatted.
 * We just reformat the row, the hyphenator will be prepared like
 * the UI expects it to be.
 * TODO: We can of course optimize this a lot.
 */
void SetParaPortion( SwTextInfo *pInf, SwParaPortion *pRoot )
{
    OSL_ENSURE( pRoot, "SetParaPortion: no root anymore" );
    pInf->m_pPara = pRoot;
}

bool SwTextFormatter::Hyphenate(SwInterHyphInfoTextFrame & rHyphInf)
{
    SwTextFormatInfo &rInf = GetInfo();

    // We never need to hyphenate anything in the last row
    // Except for, if it contains a FlyPortion or if it's the
    // last row of the Master
    if( !GetNext() && !rInf.GetTextFly().IsOn() && !m_pFrame->GetFollow() )
        return false;

    TextFrameIndex nWrdStart = m_nStart;

    // We need to retain the old row
    // E.g.: The attribute for hyphenation was not set, but
    // it's always set in SwTextFrame::Hyphenate, because we want
    // to set breakpoints.
    SwLineLayout *pOldCurr = m_pCurr;

    InitCntHyph();

    // 5298: IsParaLine() (ex.IsFirstLine) calls GetParaPortion().
    // We have to create the same conditions: in the first line
    // we format SwParaPortions...
    if( pOldCurr->IsParaPortion() )
    {
        SwParaPortion *pPara = new SwParaPortion();
        SetParaPortion( &rInf, pPara );
        m_pCurr = pPara;
        OSL_ENSURE( IsParaLine(), "SwTextFormatter::Hyphenate: not the first" );
    }
    else
        m_pCurr = new SwLineLayout();

    nWrdStart = FormatLine( nWrdStart );

    // You always should keep in mind that for example there are fields
    // which can be hyphenated
    if( m_pCurr->PrtWidth() && m_pCurr->GetLen() )
    {
        // We must be prepared that there are FlyFrames in the line,
        // at which line breaking is possible. So we search for the first
        // HyphPortion in the specified range.

        SwLinePortion *pPos = m_pCurr->GetNextPortion();
        const TextFrameIndex nPamStart = rHyphInf.m_nStart;
        nWrdStart = m_nStart;
        const TextFrameIndex nEnd = rHyphInf.m_nEnd;
        while( pPos )
        {
            // Either we are above or we are running into a HyphPortion
            // at the end of line or before a Fly.
            if( nWrdStart >= nEnd )
            {
                nWrdStart = TextFrameIndex(0);
                break;
            }

            if( nWrdStart >= nPamStart && pPos->InHyphGrp()
                && ( !pPos->IsSoftHyphPortion()
                     || static_cast<SwSoftHyphPortion*>(pPos)->IsExpand() ) )
            {
                nWrdStart = nWrdStart + pPos->GetLen();
                break;
            }

            nWrdStart = nWrdStart + pPos->GetLen();
            pPos = pPos->GetNextPortion();
        }
        // When pPos is null, no hyphen position was found.
        if( !pPos )
            nWrdStart = TextFrameIndex(0);
    }
    else
        // In case the whole line is zero-length, that's the same situation as
        // above when the portion iteration ends without explicitly breaking
        // from the loop.
        nWrdStart = TextFrameIndex(0);

    // the old LineLayout is set again ...
    delete m_pCurr;
    m_pCurr = pOldCurr;

    if( pOldCurr->IsParaPortion() )
    {
        SetParaPortion( &rInf, static_cast<SwParaPortion*>(pOldCurr) );
        OSL_ENSURE( IsParaLine(), "SwTextFormatter::Hyphenate: even not the first" );
    }

    if (nWrdStart == TextFrameIndex(0))
        return false;

    // nWrdStart contains the position in string that should be hyphenated
    rHyphInf.m_nWordStart = nWrdStart;

    TextFrameIndex nLen(0);
    const TextFrameIndex nEnd = nWrdStart;

    // we search forwards
    Reference< XHyphenatedWord > xHyphWord;

    Boundary const aBound = g_pBreakIt->GetBreakIter()->getWordBoundary(
        rInf.GetText(), sal_Int32(nWrdStart),
        g_pBreakIt->GetLocale( rInf.GetFont()->GetLanguage() ), WordType::DICTIONARY_WORD, true );
    nWrdStart = TextFrameIndex(aBound.startPos);
    nLen = TextFrameIndex(aBound.endPos) - nWrdStart;
    if (nLen == TextFrameIndex(0))
        return false;

    OUString const aSelText(rInf.GetText().copy(sal_Int32(nWrdStart), sal_Int32(nLen)));
    const sal_Int32 nMinTrail = (nWrdStart + nLen > nEnd)
            ? sal_Int32(nWrdStart + nLen - nEnd) - 1
            : 0;

    //!! rHyphInf.SetHyphWord( ... ) must done here
    xHyphWord = rInf.HyphWord( aSelText, nMinTrail );
    if ( xHyphWord.is() )
    {
        rHyphInf.SetHyphWord( xHyphWord );
        rHyphInf.m_nWordStart = nWrdStart;
        rHyphInf.m_nWordLen = nLen;
        return true;
    }

    return false;
}

bool SwTextPortion::CreateHyphen( SwTextFormatInfo &rInf, SwTextGuess const &rGuess )
{
    const Reference< XHyphenatedWord >&  xHyphWord = rGuess.HyphWord();

    OSL_ENSURE( !mpNextPortion, "SwTextPortion::CreateHyphen(): another portion, another planet..." );
    OSL_ENSURE( xHyphWord.is(), "SwTextPortion::CreateHyphen(): You are lucky! The code is robust here." );

    if( rInf.IsHyphForbud() ||
        mpNextPortion || // robust
        !xHyphWord.is() || // more robust
        // multi-line fields can't be hyphenated interactively
        ( rInf.IsInterHyph() && InFieldGrp() ) )
        return false;

    std::unique_ptr<SwHyphPortion> pHyphPor;
    TextFrameIndex nPorEnd;
    SwTextSizeInfo aInf( rInf );

    // first case: hyphenated word has alternative spelling
    if ( xHyphWord->isAlternativeSpelling() )
    {
        SvxAlternativeSpelling aAltSpell;
        aAltSpell = SvxGetAltSpelling( xHyphWord );
        OSL_ENSURE( aAltSpell.bIsAltSpelling, "no alternative spelling" );

        OUString aAltText = aAltSpell.aReplacement;
        nPorEnd = TextFrameIndex(aAltSpell.nChangedPos) + rGuess.BreakStart() - rGuess.FieldDiff();
        sal_Int32 nTmpLen = 0;

        // soft hyphen at alternative spelling position?
        if( rInf.GetText()[sal_Int32(rInf.GetSoftHyphPos())] == CHAR_SOFTHYPHEN )
        {
            pHyphPor.reset(new SwSoftHyphStrPortion( aAltText ));
            nTmpLen = 1;
        }
        else {
            pHyphPor.reset(new SwHyphStrPortion( aAltText ));
        }

        // length of pHyphPor is adjusted
        pHyphPor->SetLen( TextFrameIndex(aAltText.getLength() + 1) );
        static_cast<SwPosSize&>(*pHyphPor) = pHyphPor->GetTextSize( rInf );
        pHyphPor->SetLen( TextFrameIndex(aAltSpell.nChangedLength + nTmpLen) );
    }
    else
    {
        // second case: no alternative spelling
        pHyphPor.reset(new SwHyphPortion);
        pHyphPor->SetLen(TextFrameIndex(1));

        static const void* nLastFontCacheId = nullptr;
        static sal_uInt16 aMiniCacheH = 0, aMiniCacheW = 0;
        const void* nTmpFontCacheId;
        sal_uInt16 nFntIdx;
        rInf.GetFont()->GetFontCacheId( nTmpFontCacheId, nFntIdx, rInf.GetFont()->GetActual() );
        if( !nLastFontCacheId || nLastFontCacheId != nTmpFontCacheId ) {
            nLastFontCacheId = nTmpFontCacheId;
            static_cast<SwPosSize&>(*pHyphPor) = pHyphPor->GetTextSize( rInf );
            aMiniCacheH = pHyphPor->Height();
            aMiniCacheW = pHyphPor->Width();
        } else {
            pHyphPor->Height( aMiniCacheH );
            pHyphPor->Width( aMiniCacheW );
        }
        pHyphPor->SetLen(TextFrameIndex(0));

        // values required for this
        nPorEnd = TextFrameIndex(xHyphWord->getHyphenPos() + 1)
                + rGuess.BreakStart() - rGuess.FieldDiff();
    }

    // portion end must be in front of us
    // we do not put hyphens at start of line
    if ( nPorEnd > rInf.GetIdx() ||
         ( nPorEnd == rInf.GetIdx() && rInf.GetLineStart() != rInf.GetIdx() ) )
    {
        aInf.SetLen( nPorEnd - rInf.GetIdx() );
        pHyphPor->SetAscent( GetAscent() );
        SetLen( aInf.GetLen() );
        CalcTextSize( aInf );

        Insert( pHyphPor.release() );

        short nKern = rInf.GetFont()->CheckKerning();
        if( nKern )
            new SwKernPortion( *this, nKern );

        return true;
    }

    // last exit for the lost
    pHyphPor.reset();
    BreakCut( rInf, rGuess );
    return false;
}

bool SwHyphPortion::GetExpText( const SwTextSizeInfo &/*rInf*/, OUString &rText ) const
{
    rText = "-";
    return true;
}

void SwHyphPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Special( GetLen(), OUString('-'), GetWhichPor() );
}

bool SwHyphPortion::Format( SwTextFormatInfo &rInf )
{
    const SwLinePortion *pLast = rInf.GetLast();
    Height( pLast->Height() );
    SetAscent( pLast->GetAscent() );
    OUString aText;

    if( !GetExpText( rInf, aText ) )
        return false;

    PrtWidth( rInf.GetTextSize( aText ).Width() );
    const bool bFull = rInf.Width() <= rInf.X() + PrtWidth();
    if( bFull && !rInf.IsUnderflow() ) {
        Truncate();
        rInf.SetUnderflow( this );
    }

    return bFull;
}

bool SwHyphStrPortion::GetExpText( const SwTextSizeInfo &, OUString &rText ) const
{
    rText = aExpand;
    return true;
}

void SwHyphStrPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Special( GetLen(), aExpand, GetWhichPor() );
}

SwLinePortion *SwSoftHyphPortion::Compress() { return this; }

SwSoftHyphPortion::SwSoftHyphPortion() :
    bExpand(false), nViewWidth(0)
{
    SetLen(TextFrameIndex(1));
    SetWhichPor( PortionType::SoftHyphen );
}

sal_uInt16 SwSoftHyphPortion::GetViewWidth( const SwTextSizeInfo &rInf ) const
{
    // Although we're in the const, nViewWidth should be calculated at
    // the last possible moment
    if( !Width() && rInf.OnWin() && rInf.GetOpt().IsSoftHyph() && !IsExpand() )
    {
        if( !nViewWidth )
            const_cast<SwSoftHyphPortion*>(this)->nViewWidth
                = rInf.GetTextSize(OUString('-')).Width();
    }
    else
        const_cast<SwSoftHyphPortion*>(this)->nViewWidth = 0;
    return nViewWidth;
}

/**
 * Cases:
 *
 *  1) SoftHyph is in the line, ViewOpt off
 *     -> invisible, neighbors unchanged
 *  2) SoftHyph is in the line, ViewOpt on
 *     -> visible, neighbors unchanged
 *  3) SoftHyph is at the end of the line, ViewOpt on or off
 *     -> always visible, neighbors unchanged
 */
void SwSoftHyphPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if( Width() )
    {
        rInf.DrawViewOpt( *this, PortionType::SoftHyphen );
        SwExpandPortion::Paint( rInf );
    }
}

/**
 * We get the final width from the FormatEOL()
 *
 * During the underflow-phase we determine, whether or not
 * there's an alternative spelling at all ...
 *
 * Case 1: "Au-to"
 * 1) {Au}{-}{to}, {to} does not fit anymore => underflow
 * 2) {-} calls hyphenate => no alternative
 * 3) FormatEOL() and bFull = true
 *
 * Case 2: "Zuc-ker"
 * 1) {Zuc}{-}{ker}, {ker} does not fit anymore => underflow
 * 2) {-} calls hyphenate => alternative!
 * 3) Underflow() and bFull = true
 * 4) {Zuc} calls hyphenate => {Zuk}{-}{ker}
 */
bool SwSoftHyphPortion::Format( SwTextFormatInfo &rInf )
{
    bool bFull = true;

    // special case for old German spelling
    if( rInf.IsUnderflow()  )
    {
        if( rInf.GetSoftHyphPos() )
            return true;

        const bool bHyph = rInf.ChgHyph( true );
        if( rInf.IsHyphenate() )
        {
            rInf.SetSoftHyphPos( rInf.GetIdx() );
            Width(0);
            // if the soft hyphened word has an alternative spelling
            // when hyphenated (old German spelling), the soft hyphen
            // portion has to trigger an underflow
            SwTextGuess aGuess;
            bFull = rInf.IsInterHyph() ||
                    !aGuess.AlternativeSpelling(rInf, rInf.GetIdx() - TextFrameIndex(1));
        }
        rInf.ChgHyph( bHyph );

        if( bFull && !rInf.IsHyphForbud() )
        {
            rInf.SetSoftHyphPos(TextFrameIndex(0));
            FormatEOL( rInf );
            if ( rInf.GetFly() )
                rInf.GetRoot()->SetMidHyph( true );
            else
                rInf.GetRoot()->SetEndHyph( true );
        }
        else
        {
            rInf.SetSoftHyphPos( rInf.GetIdx() );
            Truncate();
            rInf.SetUnderflow( this );
        }
        return true;
    }

    rInf.SetSoftHyphPos(TextFrameIndex(0));
    SetExpand( true );
    bFull = SwHyphPortion::Format( rInf );
    SetExpand( false );
    if( !bFull )
    {
        // By default, we do not have a width, but we do have a height
        Width(0);
    }
    return bFull;
}

/**
 * Format End of Line
 */
void SwSoftHyphPortion::FormatEOL( SwTextFormatInfo &rInf )
{
    if( !IsExpand() )
    {
        SetExpand( true );
        if( rInf.GetLast() == this )
            rInf.SetLast( FindPrevPortion( rInf.GetRoot() ) );

        // We need to reset the old values
        const SwTwips nOldX  = rInf.X();
        TextFrameIndex const nOldIdx = rInf.GetIdx();
        rInf.X( rInf.X() - PrtWidth() );
        rInf.SetIdx( rInf.GetIdx() - GetLen() );
        const bool bFull = SwHyphPortion::Format( rInf );

        // Shady business: We're allowed to get wider, but a Fly is also
        // being processed, which needs a correct X position
        if( bFull || !rInf.GetFly() )
            rInf.X( nOldX );
        else
            rInf.X( nOldX + Width() );
        rInf.SetIdx( nOldIdx );
    }
}

/**
 * We're expanding:
 * - if the special characters should be visible
 * - if we're at the end of the line
 * - if we're before a (real/emulated) line break
 */
bool SwSoftHyphPortion::GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const
{
    if( IsExpand() || ( rInf.OnWin() && rInf.GetOpt().IsSoftHyph() ) ||
        ( GetNextPortion() && ( GetNextPortion()->InFixGrp() ||
          GetNextPortion()->IsDropPortion() || GetNextPortion()->IsLayPortion() ||
          GetNextPortion()->IsParaPortion() || GetNextPortion()->IsBreakPortion() ) ) )
    {
        return SwHyphPortion::GetExpText( rInf, rText );
    }
    return false;
}

void SwSoftHyphPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    const PortionType nWhich = ! Width() ?
                          PortionType::SoftHyphenComp :
                          GetWhichPor();
    rPH.Special( GetLen(), OUString('-'), nWhich );
}

void SwSoftHyphStrPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    // Bug or feature?:
    // {Zu}{k-}{ker}, {k-} will be gray instead of {-}
    rInf.DrawViewOpt( *this, PortionType::SoftHyphen );
    SwHyphStrPortion::Paint( rInf );
}

SwSoftHyphStrPortion::SwSoftHyphStrPortion( const OUString &rStr )
    : SwHyphStrPortion( rStr )
{
    SetLen(TextFrameIndex(1));
    SetWhichPor( PortionType::SoftHyphenStr );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
