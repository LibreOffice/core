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
#include <vcl/metric.hxx>
#include <vcl/svapp.hxx>
#include <paratr.hxx>
#include <txtfrm.hxx>
#include <charfmt.hxx>
#include <viewopt.hxx>
#include <viewsh.hxx>
#include "pordrop.hxx"
#include "itrform2.hxx"
#include "txtpaint.hxx"
#include <blink.hxx>
#include <breakit.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <editeng/langitem.hxx>
#include <charatr.hxx>
#include <editeng/fhgtitem.hxx>
#include <calbck.hxx>
#include <doc.hxx>

using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star;

/**
 * Calculates if a drop caps portion intersects with a fly
 * The width and height of the drop caps portion are passed as arguments,
 * the position is calculated from the values in rInf
 */
static bool lcl_IsDropFlyInter( const SwTextFormatInfo &rInf,
                             sal_uInt16 nWidth, sal_uInt16 nHeight )
{
    const SwTextFly& rTextFly = rInf.GetTextFly();
    if( rTextFly.IsOn() )
    {
        SwRect aRect( rInf.GetTextFrame()->getFrameArea().Pos(), Size( nWidth, nHeight) );
        aRect.Pos() += rInf.GetTextFrame()->getFramePrintArea().Pos();
        aRect.Pos().AdjustX(rInf.X() );
        aRect.Pos().setY( rInf.Y() );
        aRect = rTextFly.GetFrame( aRect );
        return aRect.HasArea();
    }

    return false;
}

class SwDropSave
{
    SwTextPaintInfo* pInf;
    sal_Int32 const nIdx;
    sal_Int32 const nLen;
    long const nX;
    long const nY;

public:
    explicit SwDropSave( const SwTextPaintInfo &rInf );
    ~SwDropSave();
};

SwDropSave::SwDropSave( const SwTextPaintInfo &rInf ) :
        pInf( const_cast<SwTextPaintInfo*>(&rInf) ), nIdx( rInf.GetIdx() ),
        nLen( rInf.GetLen() ), nX( rInf.X() ), nY( rInf.Y() )
{
}

SwDropSave::~SwDropSave()
{
    pInf->SetIdx(TextFrameIndex(nIdx));
    pInf->SetLen(TextFrameIndex(nLen));
    pInf->X( nX );
    pInf->Y( nY );
}

/// SwDropPortionPart DTor
SwDropPortionPart::~SwDropPortionPart()
{
    pFollow.reset();
    pFnt.reset();
}

/// SwDropPortion CTor, DTor
SwDropPortion::SwDropPortion( const sal_uInt16 nLineCnt,
                              const sal_uInt16 nDrpHeight,
                              const sal_uInt16 nDrpDescent,
                              const sal_uInt16 nDist )
  : nLines( nLineCnt ),
    nDropHeight(nDrpHeight),
    nDropDescent(nDrpDescent),
    nDistance(nDist),
    nFix(0),
    nY(0)
{
    SetWhichPor( PortionType::Drop );
}

SwDropPortion::~SwDropPortion()
{
    pPart.reset();
    if( pBlink )
        pBlink->Delete( this );
}

/// nWishLen = 0 indicates that we want a whole word
sal_Int32 SwTextNode::GetDropLen( sal_Int32 nWishLen ) const
{
    sal_Int32 nEnd = GetText().getLength();
    if( nWishLen && nWishLen < nEnd )
        nEnd = nWishLen;

    if (! nWishLen)
    {
        // find first word
        const SwAttrSet& rAttrSet = GetSwAttrSet();
        const sal_uInt16 nTextScript = g_pBreakIt->GetRealScriptOfText( GetText(), 0 );

        LanguageType eLanguage;

        switch ( nTextScript )
        {
        case i18n::ScriptType::ASIAN :
            eLanguage = rAttrSet.GetCJKLanguage().GetLanguage();
            break;
        case i18n::ScriptType::COMPLEX :
            eLanguage = rAttrSet.GetCTLLanguage().GetLanguage();
            break;
        default :
            eLanguage = rAttrSet.GetLanguage().GetLanguage();
            break;
        }

        Boundary aBound =
            g_pBreakIt->GetBreakIter()->getWordBoundary( GetText(), 0,
            g_pBreakIt->GetLocale( eLanguage ), WordType::DICTIONARY_WORD, true );

        nEnd = aBound.endPos;
    }

    sal_Int32 i = 0;
    for( ; i < nEnd; ++i )
    {
        sal_Unicode const cChar = GetText()[i];
        if( CH_TAB == cChar || CH_BREAK == cChar ||
            (( CH_TXTATR_BREAKWORD == cChar || CH_TXTATR_INWORD == cChar )
                && GetTextAttrForCharAt(i)) )
            break;
    }
    return i;
}

/// nWishLen = 0 indicates that we want a whole word
TextFrameIndex SwTextFrame::GetDropLen(TextFrameIndex const nWishLen) const
{
    TextFrameIndex nEnd(GetText().getLength());
    if (nWishLen && nWishLen < nEnd)
        nEnd = nWishLen;

    if (! nWishLen)
    {
        // find first word
        const SwAttrSet& rAttrSet = GetTextNodeForParaProps()->GetSwAttrSet();
        const sal_uInt16 nTextScript = g_pBreakIt->GetRealScriptOfText(GetText(), 0);

        LanguageType eLanguage;

        switch ( nTextScript )
        {
        case i18n::ScriptType::ASIAN :
            eLanguage = rAttrSet.GetCJKLanguage().GetLanguage();
            break;
        case i18n::ScriptType::COMPLEX :
            eLanguage = rAttrSet.GetCTLLanguage().GetLanguage();
            break;
        default :
            eLanguage = rAttrSet.GetLanguage().GetLanguage();
            break;
        }

        Boundary aBound = g_pBreakIt->GetBreakIter()->getWordBoundary(
            GetText(), 0, g_pBreakIt->GetLocale(eLanguage),
            WordType::DICTIONARY_WORD, true );

        nEnd = TextFrameIndex(aBound.endPos);
    }

    TextFrameIndex i(0);
    for ( ; i < nEnd; ++i)
    {
        sal_Unicode const cChar = GetText()[sal_Int32(i)];
        if (CH_TAB == cChar || CH_BREAK == cChar ||
            CH_TXTATR_BREAKWORD == cChar || CH_TXTATR_INWORD == cChar)
        {
#ifndef NDEBUG
            if (CH_TXTATR_BREAKWORD == cChar || CH_TXTATR_INWORD == cChar)
            {
                std::pair<SwTextNode const*, sal_Int32> const pos(MapViewToModel(i));
                assert(pos.first->GetTextAttrForCharAt(pos.second) != nullptr);
            }
#endif
            break;
        }
    }
    return i;
}

/**
 * If a dropcap is found the return value is true otherwise false. The
 * drop cap sizes passed back by reference are font height, drop height
 * and drop descent.
 */
bool SwTextNode::GetDropSize(int& rFontHeight, int& rDropHeight, int& rDropDescent) const
{
    rFontHeight = 0;
    rDropHeight = 0;
    rDropDescent =0;

    const SwAttrSet& rSet = GetSwAttrSet();
    const SwFormatDrop& rDrop = rSet.GetDrop();

    // Return (0,0) if there is no drop cap at this paragraph
    if( 1 >= rDrop.GetLines() ||
        ( !rDrop.GetChars() && !rDrop.GetWholeWord() ) )
    {
        return false;
    }

    // get text frame
    SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIter(*this);
    for( SwTextFrame* pLastFrame = aIter.First(); pLastFrame; pLastFrame = aIter.Next() )
    {
        // Only (master-) text frames can have a drop cap.
        if (!pLastFrame->IsFollow() &&
            pLastFrame->GetTextNodeForFirstText() == this)
        {

            if( !pLastFrame->HasPara() )
                pLastFrame->GetFormatted();

            if ( !pLastFrame->IsEmpty() )
            {
                const SwParaPortion* pPara = pLastFrame->GetPara();
                OSL_ENSURE( pPara, "GetDropSize could not find the ParaPortion, I'll guess the drop cap size" );

                if ( pPara )
                {
                    const SwLinePortion* pFirstPor = pPara->GetFirstPortion();
                    if (pFirstPor && pFirstPor->IsDropPortion())
                    {
                        const SwDropPortion* pDrop = static_cast<const SwDropPortion*>(pFirstPor);
                        rDropHeight = pDrop->GetDropHeight();
                        rDropDescent = pDrop->GetDropDescent();
                        if (const SwFont *pFont = pDrop->GetFnt())
                            rFontHeight = pFont->GetSize(pFont->GetActual()).Height();
                        else
                        {
                            const SvxFontHeightItem& rItem = rSet.Get(RES_CHRATR_FONTSIZE);
                            rFontHeight = rItem.GetHeight();
                        }
                    }
                }
            }
            break;
        }
    }

    if (rFontHeight==0 && rDropHeight==0 && rDropDescent==0)
    {
        const sal_uInt16 nLines = rDrop.GetLines();

        const SvxFontHeightItem& rItem = rSet.Get( RES_CHRATR_FONTSIZE );
        rFontHeight = rItem.GetHeight();
        rDropHeight = nLines * rFontHeight;
        rDropDescent = rFontHeight / 5;
        return false;
    }

    return true;
}

/// Manipulate the width, otherwise the chars are being stretched
void SwDropPortion::PaintText( const SwTextPaintInfo &rInf ) const
{
    OSL_ENSURE( nDropHeight && pPart && nLines != 1, "Drop Portion painted twice" );

    const SwDropPortionPart* pCurrPart = GetPart();
    const TextFrameIndex nOldLen = GetLen();
    const sal_uInt16 nOldWidth = Width();
    const sal_uInt16 nOldAscent = GetAscent();

    const SwTwips nBasePosY  = rInf.Y();
    const_cast<SwTextPaintInfo&>(rInf).Y( nBasePosY + nY );
    const_cast<SwDropPortion*>(this)->SetAscent( nOldAscent + nY );
    SwDropSave aSave( rInf );
    // for text inside drop portions we let vcl handle the text directions
    SwLayoutModeModifier aLayoutModeModifier( *rInf.GetOut() );
    aLayoutModeModifier.SetAuto();

    while ( pCurrPart )
    {
        const_cast<SwDropPortion*>(this)->SetLen( pCurrPart->GetLen() );
        const_cast<SwDropPortion*>(this)->Width( pCurrPart->GetWidth() );
        const_cast<SwTextPaintInfo&>(rInf).SetLen( pCurrPart->GetLen() );
        SwFontSave aFontSave( rInf, &pCurrPart->GetFont() );
        const_cast<SwDropPortion*>(this)->SetJoinBorderWithNext(pCurrPart->GetJoinBorderWithNext());
        const_cast<SwDropPortion*>(this)->SetJoinBorderWithPrev(pCurrPart->GetJoinBorderWithPrev());

        if ( rInf.OnWin() &&
            !rInf.GetOpt().IsPagePreview() && !rInf.GetOpt().IsReadonly() && SwViewOption::IsFieldShadings() &&
            (!pCurrPart->GetFont().GetBackColor() || *pCurrPart->GetFont().GetBackColor() == COL_TRANSPARENT) )
        {
            rInf.DrawBackground( *this );
        }

        SwTextPortion::Paint( rInf );

        const_cast<SwTextPaintInfo&>(rInf).SetIdx( rInf.GetIdx() + pCurrPart->GetLen() );
        const_cast<SwTextPaintInfo&>(rInf).X( rInf.X() + pCurrPart->GetWidth() );
        pCurrPart = pCurrPart->GetFollow();
    }

    const_cast<SwTextPaintInfo&>(rInf).Y( nBasePosY );
    const_cast<SwDropPortion*>(this)->Width( nOldWidth );
    const_cast<SwDropPortion*>(this)->SetLen( nOldLen );
    const_cast<SwDropPortion*>(this)->SetAscent( nOldAscent );
    const_cast<SwDropPortion*>(this)->SetJoinBorderWithNext(false);
    const_cast<SwDropPortion*>(this)->SetJoinBorderWithPrev(false);
}

void SwDropPortion::PaintDrop( const SwTextPaintInfo &rInf ) const
{
    // normal output is being done during the normal painting
    if( ! nDropHeight || ! pPart || nLines == 1 )
        return;

    // set the lying values
    const sal_uInt16 nOldHeight = Height();
    const sal_uInt16 nOldWidth  = Width();
    const sal_uInt16 nOldAscent = GetAscent();
    const SwTwips nOldPosY  = rInf.Y();
    const SwTwips nOldPosX  = rInf.X();
    const SwParaPortion *pPara = rInf.GetParaPortion();
    const Point aOutPos( nOldPosX, nOldPosY - pPara->GetAscent()
                         - pPara->GetRealHeight() + pPara->Height() );
    // make good for retouching

    // Set baseline
    const_cast<SwTextPaintInfo&>(rInf).Y( aOutPos.Y() + nDropHeight );

    // for background
    const_cast<SwDropPortion*>(this)->Height( nDropHeight + nDropDescent );
    const_cast<SwDropPortion*>(this)->SetAscent( nDropHeight );

    // Always adapt Clipregion to us, never set it off using the existing ClipRect
    // as that could be set for the line
    SwRect aClipRect;
    if ( rInf.OnWin() )
    {
        aClipRect = SwRect( aOutPos, SvLSize() );
        aClipRect.Intersection( rInf.GetPaintRect() );
    }
    SwSaveClip aClip( const_cast<OutputDevice*>(rInf.GetOut()) );
    aClip.ChgClip( aClipRect, rInf.GetTextFrame() );

    // Just do, what we always do ...
    PaintText( rInf );

    // save old values
    const_cast<SwDropPortion*>(this)->Height( nOldHeight );
    const_cast<SwDropPortion*>(this)->Width( nOldWidth );
    const_cast<SwDropPortion*>(this)->SetAscent( nOldAscent );
    const_cast<SwTextPaintInfo&>(rInf).Y( nOldPosY );
}

void SwDropPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    // normal output is being done here
    if( ! nDropHeight || ! pPart || 1 == nLines )
    {
        if ( rInf.OnWin() &&
            !rInf.GetOpt().IsPagePreview() && !rInf.GetOpt().IsReadonly() && SwViewOption::IsFieldShadings()       )
            rInf.DrawBackground( *this );

        // make sure that font is not rotated
        std::unique_ptr<SwFont> pTmpFont;
        if ( rInf.GetFont()->GetOrientation( rInf.GetTextFrame()->IsVertical() ) )
        {
            pTmpFont.reset(new SwFont( *rInf.GetFont() ));
            pTmpFont->SetVertical( 0, rInf.GetTextFrame()->IsVertical() );
        }

        SwFontSave aFontSave( rInf, pTmpFont.get() );
        // for text inside drop portions we let vcl handle the text directions
        SwLayoutModeModifier aLayoutModeModifier( *rInf.GetOut() );
        aLayoutModeModifier.SetAuto();

        SwTextPortion::Paint( rInf );
    }
}

bool SwDropPortion::FormatText( SwTextFormatInfo &rInf )
{
    const TextFrameIndex nOldLen = GetLen();
    const TextFrameIndex nOldInfLen = rInf.GetLen();
    if (!SwTextPortion::Format( rInf ))
        return false;

    // looks like shit, but what can we do?
    rInf.SetUnderflow( nullptr );
    Truncate();
    SetLen( nOldLen );
    rInf.SetLen( nOldInfLen );

    return true;
}

SwPosSize SwDropPortion::GetTextSize( const SwTextSizeInfo &rInf ) const
{
    sal_uInt16 nMyX = 0;
    TextFrameIndex nIdx(0);

    const SwDropPortionPart* pCurrPart = GetPart();

    // skip parts
    while ( pCurrPart && nIdx + pCurrPart->GetLen() < rInf.GetLen() )
    {
        nMyX = nMyX + pCurrPart->GetWidth();
        nIdx = nIdx + pCurrPart->GetLen();
        pCurrPart = pCurrPart->GetFollow();
    }

    TextFrameIndex const nOldIdx = rInf.GetIdx();
    TextFrameIndex const nOldLen = rInf.GetLen();

    const_cast<SwTextSizeInfo&>(rInf).SetIdx( nIdx );
    const_cast<SwTextSizeInfo&>(rInf).SetLen( rInf.GetLen() - nIdx );

    if( pCurrPart )
    {
        const_cast<SwDropPortion*>(this)->SetJoinBorderWithNext(pCurrPart->GetJoinBorderWithNext());
        const_cast<SwDropPortion*>(this)->SetJoinBorderWithPrev(pCurrPart->GetJoinBorderWithPrev());
    }

    // robust
    SwFontSave aFontSave( rInf, pCurrPart ? &pCurrPart->GetFont() : nullptr );
    SwPosSize aPosSize( SwTextPortion::GetTextSize( rInf ) );
    aPosSize.Width( aPosSize.Width() + nMyX );

    const_cast<SwTextSizeInfo&>(rInf).SetIdx( nOldIdx );
    const_cast<SwTextSizeInfo&>(rInf).SetLen( nOldLen );
    if( pCurrPart )
    {
        const_cast<SwDropPortion*>(this)->SetJoinBorderWithNext(false);
        const_cast<SwDropPortion*>(this)->SetJoinBorderWithPrev(false);
    }

    return aPosSize;
}

TextFrameIndex SwDropPortion::GetCursorOfst(const sal_uInt16) const
{
    return TextFrameIndex(0);
}

void SwTextFormatter::CalcDropHeight( const sal_uInt16 nLines )
{
    const SwLinePortion *const pOldCurr = GetCurr();
    sal_uInt16 nDropHght = 0;
    sal_uInt16 nAscent = 0;
    sal_uInt16 nHeight = 0;
    sal_uInt16 nDropLns = 0;
    const bool bRegisterOld = IsRegisterOn();
    m_bRegisterOn = false;

    Top();

    while( GetCurr()->IsDummy() )
    {
        if ( !Next() )
            break;
    }

    // If we have only one line we return 0
    if( GetNext() || GetDropLines() == 1 )
    {
        for( ; nDropLns < nLines; nDropLns++ )
        {
            if ( GetCurr()->IsDummy() )
                break;
            else
            {
                CalcAscentAndHeight( nAscent, nHeight );
                nDropHght = nDropHght + nHeight;
                m_bRegisterOn = bRegisterOld;
            }
            if ( !Next() )
            {
                nDropLns++;
                break;
            }
        }

        // We hit the line ascent when reaching the last line!
        nDropHght = nDropHght - nHeight;
        nDropHght = nDropHght + nAscent;
        Top();
    }
    m_bRegisterOn = bRegisterOld;
    SetDropDescent( nHeight - nAscent );
    SetDropHeight( nDropHght );
    SetDropLines( nDropLns );
    // Find old position!
    while( pOldCurr != GetCurr() )
    {
        if( !Next() )
        {
            OSL_ENSURE( false, "SwTextFormatter::_CalcDropHeight: left Toulouse" );
            break;
        }
    }
}

/**
 * We assume that the font height doesn't change and that at first there
 * are at least as many lines, as the DropCap-setting claims
 */
void SwTextFormatter::GuessDropHeight( const sal_uInt16 nLines )
{
    OSL_ENSURE( nLines, "GuessDropHeight: Give me more Lines!" );
    sal_uInt16 nAscent = 0;
    sal_uInt16 nHeight = 0;
    SetDropLines( nLines );
    if ( GetDropLines() > 1 )
    {
        CalcRealHeight();
        CalcAscentAndHeight( nAscent, nHeight );
    }
    SetDropDescent( nHeight - nAscent );
    SetDropHeight( nHeight * nLines - GetDropDescent() );
}

SwDropPortion *SwTextFormatter::NewDropPortion( SwTextFormatInfo &rInf )
{
    if( !pDropFormat )
        return nullptr;

    TextFrameIndex nPorLen(pDropFormat->GetWholeWord() ? 0 : pDropFormat->GetChars());
    nPorLen = m_pFrame->GetDropLen( nPorLen );
    if( !nPorLen )
    {
        ClearDropFormat();
        return nullptr;
    }

    SwDropPortion *pDropPor = nullptr;

    // first or second round?
    if ( !( GetDropHeight() || IsOnceMore() ) )
    {
        if ( GetNext() )
            CalcDropHeight( pDropFormat->GetLines() );
        else
            GuessDropHeight( pDropFormat->GetLines() );
    }

    // the DropPortion
    if( GetDropHeight() )
        pDropPor = new SwDropPortion( GetDropLines(), GetDropHeight(),
                                      GetDropDescent(), pDropFormat->GetDistance() );
    else
       pDropPor = new SwDropPortion( 0,0,0,pDropFormat->GetDistance() );

    pDropPor->SetLen( nPorLen );

    // If it was not possible to create a proper drop cap portion
    // due to avoiding endless loops. We return a drop cap portion
    // with an empty SwDropCapPart. For these portions the current
    // font is used.
    if ( GetDropLines() < 2 )
    {
        SetPaintDrop( true );
        return pDropPor;
    }

    // build DropPortionParts:
    OSL_ENSURE( ! rInf.GetIdx(), "Drop Portion not at 0 position!" );
    TextFrameIndex nNextChg(0);
    const SwCharFormat* pFormat = pDropFormat->GetCharFormat();
    SwDropPortionPart* pCurrPart = nullptr;

    while ( nNextChg  < nPorLen )
    {
        // check for attribute changes and if the portion has to split:
        Seek( nNextChg );

        // the font is deleted in the destructor of the drop portion part
        SwFont* pTmpFnt = new SwFont( *rInf.GetFont() );
        if ( pFormat )
        {
            const SwAttrSet& rSet = pFormat->GetAttrSet();
            pTmpFnt->SetDiffFnt(&rSet, &m_pFrame->GetDoc().getIDocumentSettingAccess());
        }

        // we do not allow a vertical font for the drop portion
        pTmpFnt->SetVertical( 0, rInf.GetTextFrame()->IsVertical() );

        // find next attribute change / script change
        const TextFrameIndex nTmpIdx = nNextChg;
        TextFrameIndex nNextAttr = GetNextAttr();
        nNextChg = m_pScriptInfo->NextScriptChg( nTmpIdx );
        if( nNextChg > nNextAttr )
            nNextChg = nNextAttr;
        if ( nNextChg > nPorLen )
            nNextChg = nPorLen;

        std::unique_ptr<SwDropPortionPart> pPart(
                new SwDropPortionPart( *pTmpFnt, nNextChg - nTmpIdx ) );
        auto pPartTemp = pPart.get();

        if ( ! pCurrPart )
            pDropPor->SetPart( std::move(pPart) );
        else
            pCurrPart->SetFollow( std::move(pPart) );

        pCurrPart = pPartTemp;
    }

    SetPaintDrop( true );
    return pDropPor;
}

void SwTextPainter::PaintDropPortion()
{
    const SwDropPortion *pDrop = GetInfo().GetParaPortion()->FindDropPortion();
    OSL_ENSURE( pDrop, "DrapCop-Portion not available." );
    if( !pDrop )
        return;

    const SwTwips nOldY = GetInfo().Y();

    Top();

    GetInfo().SetpSpaceAdd( m_pCurr->GetpLLSpaceAdd() );
    GetInfo().ResetSpaceIdx();
    GetInfo().SetKanaComp( m_pCurr->GetpKanaComp() );
    GetInfo().ResetKanaIdx();

    // 8047: Drops and Dummies
    while( !m_pCurr->GetLen() && Next() )
        ;

    // MarginPortion and Adjustment!
    const SwLinePortion *pPor = m_pCurr->GetFirstPortion();
    long nX = 0;
    while( pPor && !pPor->IsDropPortion() )
    {
        nX = nX + pPor->Width();
        pPor = pPor->GetNextPortion();
    }
    Point aLineOrigin( GetTopLeft() );

    aLineOrigin.AdjustX(nX );
    sal_uInt16 nTmpAscent, nTmpHeight;
    CalcAscentAndHeight( nTmpAscent, nTmpHeight );
    aLineOrigin.AdjustY(nTmpAscent );
    GetInfo().SetIdx( GetStart() );
    GetInfo().SetPos( aLineOrigin );
    GetInfo().SetLen( pDrop->GetLen() );

    pDrop->PaintDrop( GetInfo() );

    GetInfo().Y( nOldY );
}

// Since the calculation of the font size is expensive, this is being
// channeled through a DropCapCache
#define DROP_CACHE_SIZE 10

class SwDropCapCache
{
    const void* aFontCacheId[ DROP_CACHE_SIZE ];
    OUString aText[ DROP_CACHE_SIZE ];
    sal_uInt16 aFactor[ DROP_CACHE_SIZE ];
    sal_uInt16 aWishedHeight[ DROP_CACHE_SIZE ];
    short aDescent[ DROP_CACHE_SIZE ];
    sal_uInt16 nIndex;
public:
    SwDropCapCache();
    void CalcFontSize( SwDropPortion* pDrop, SwTextFormatInfo &rInf );
};

// SwDropCapCache Ctor / Dtor
SwDropCapCache::SwDropCapCache() : nIndex( 0 )
{
    memset( &aFontCacheId, 0, sizeof(aFontCacheId) );
    memset( &aWishedHeight, 0, sizeof(aWishedHeight) );
}

void SwDropPortion::DeleteDropCapCache()
{
    delete pDropCapCache;
}

void SwDropCapCache::CalcFontSize( SwDropPortion* pDrop, SwTextFormatInfo &rInf )
{
    const void* nFntCacheId = nullptr;
    sal_uInt16 nTmpIdx = 0;

    OSL_ENSURE( pDrop->GetPart(),"DropPortion without part during font calculation");

    SwDropPortionPart* pCurrPart = pDrop->GetPart();
    const bool bUseCache = ! pCurrPart->GetFollow() && !pCurrPart->GetFont().HasBorder();
    TextFrameIndex nIdx = rInf.GetIdx();
    OUString aStr(rInf.GetText().copy(sal_Int32(nIdx), sal_Int32(pCurrPart->GetLen())));

    long nDescent = 0;
    long nFactor = -1;

    if ( bUseCache )
    {
        SwFont& rFnt = pCurrPart->GetFont();
        rFnt.CheckFontCacheId( rInf.GetVsh(), rFnt.GetActual() );
        rFnt.GetFontCacheId( nFntCacheId, nTmpIdx, rFnt.GetActual() );

        nTmpIdx = 0;

        while( nTmpIdx < DROP_CACHE_SIZE &&
            ( aText[ nTmpIdx ] != aStr || aFontCacheId[ nTmpIdx ] != nFntCacheId ||
            aWishedHeight[ nTmpIdx ] != pDrop->GetDropHeight() ) )
            ++nTmpIdx;
    }

    // we have to calculate a new font scaling factor if
    // 1. we did not find a scaling factor in the cache or
    // 2. we are not allowed to use the cache because the drop portion
    //    consists of more than one part
    if( nTmpIdx >= DROP_CACHE_SIZE || ! bUseCache )
    {
        ++nIndex;
        nIndex %= DROP_CACHE_SIZE;
        nTmpIdx = nIndex;

        long nWishedHeight = pDrop->GetDropHeight();
        long nAscent = 0;

        // find out biggest font size for initial scaling factor
        long nMaxFontHeight = 1;
        while ( pCurrPart )
        {
            const SwFont& rFnt = pCurrPart->GetFont();
            const long nCurrHeight = rFnt.GetHeight( rFnt.GetActual() );
            if ( nCurrHeight > nMaxFontHeight )
                nMaxFontHeight = nCurrHeight;

            pCurrPart = pCurrPart->GetFollow();
        }

        nFactor = ( 1000 * nWishedHeight ) / nMaxFontHeight;

        if ( bUseCache )
        {
            // save keys for cache
            aFontCacheId[ nTmpIdx ] = nFntCacheId;
            aText[ nTmpIdx ] = aStr;
            aWishedHeight[ nTmpIdx ] = sal_uInt16(nWishedHeight);
            // save initial scaling factor
            aFactor[ nTmpIdx ] = static_cast<sal_uInt16>(nFactor);
        }

        bool bGrow = (pDrop->GetLen() != TextFrameIndex(0));

        // for growing control
        long nMax = USHRT_MAX;
        long nMin = 0;
#if OSL_DEBUG_LEVEL > 1
        long nGrow = 0;
#endif

        bool bWinUsed = false;
        vcl::Font aOldFnt;
        MapMode aOldMap( MapUnit::MapTwip );
        OutputDevice* pOut = rInf.GetOut();
        OutputDevice* pWin;
        if( rInf.GetVsh() && rInf.GetVsh()->GetWin() )
            pWin = rInf.GetVsh()->GetWin();
        else
            pWin = Application::GetDefaultDevice();

        while( bGrow )
        {
            // reset pCurrPart to first part
            pCurrPart = pDrop->GetPart();
            bool bFirstGlyphRect = true;
            tools::Rectangle aCommonRect, aRect;

            while ( pCurrPart )
            {
                // current font
                SwFont& rFnt = pCurrPart->GetFont();

                // Get height including proportion
                const long nCurrHeight = rFnt.GetHeight( rFnt.GetActual() );

                // Get without proportion
                const sal_uInt8 nOldProp = rFnt.GetPropr();
                rFnt.SetProportion( 100 );
                Size aOldSize( 0, rFnt.GetHeight( rFnt.GetActual() ) );

                Size aNewSize( 0, ( nFactor * nCurrHeight ) / 1000 );
                rFnt.SetSize( aNewSize, rFnt.GetActual() );
                rFnt.ChgPhysFnt( rInf.GetVsh(), *pOut );

                nAscent = rFnt.GetAscent( rInf.GetVsh(), *pOut );

                // we get the rectangle that covers all chars
                bool bHaveGlyphRect = pOut->GetTextBoundRect( aRect, rInf.GetText(), 0,
                            sal_Int32(nIdx), sal_Int32(pCurrPart->GetLen()))
                    && ! aRect.IsEmpty();

                if ( ! bHaveGlyphRect )
                {
                    // getting glyph boundaries failed for some reason,
                    // we take the window for calculating sizes
                    if ( pWin )
                    {
                        if ( ! bWinUsed )
                        {
                            bWinUsed = true;
                            aOldMap = pWin->GetMapMode( );
                            pWin->SetMapMode( MapMode( MapUnit::MapTwip ) );
                            aOldFnt = pWin->GetFont();
                        }
                        pWin->SetFont( rFnt.GetActualFont() );

                        bHaveGlyphRect = pWin->GetTextBoundRect( aRect, rInf.GetText(), 0,
                                sal_Int32(nIdx), sal_Int32(pCurrPart->GetLen()))
                            && ! aRect.IsEmpty();
                    }
                    if (!bHaveGlyphRect)
                    {
                        // We do not have a window or our window could not
                        // give us glyph boundaries.
                        aRect = tools::Rectangle( Point( 0, 0 ), Size( 0, nAscent ) );
                    }
                }

                // Now we (hopefully) have a bounding rectangle for the
                // glyphs of the current portion and the ascent of the current
                // font

                // reset font size and proportion
                rFnt.SetSize( aOldSize, rFnt.GetActual() );
                rFnt.SetProportion( nOldProp );

                // Modify the bounding rectangle with the borders
                // Robust: If the padding is so big as drop cap letter has no enough space than
                // remove all padding.
                if( rFnt.GetTopBorderSpace() + rFnt.GetBottomBorderSpace() >= nWishedHeight )
                {
                    rFnt.SetTopBorderDist(0);
                    rFnt.SetBottomBorderDist(0);
                    rFnt.SetRightBorderDist(0);
                    rFnt.SetLeftBorderDist(0);
                }

                if( rFnt.GetTopBorder() )
                {
                    aRect.setHeight(aRect.GetHeight() + rFnt.GetTopBorderSpace());
                    aRect.setY(aRect.getY() - rFnt.GetTopBorderSpace());
                }

                if( rFnt.GetBottomBorder() )
                {
                    aRect.setHeight(aRect.GetHeight() + rFnt.GetBottomBorderSpace());
                }

                if ( bFirstGlyphRect )
                {
                    aCommonRect = aRect;
                    bFirstGlyphRect = false;
                }
                else
                    aCommonRect.Union( aRect );

                nIdx = nIdx + pCurrPart->GetLen();
                pCurrPart = pCurrPart->GetFollow();
            }

            // now we have a union ( aCommonRect ) of all glyphs with
            // respect to a common baseline : 0

            // get descent and ascent from union
            if ( rInf.GetTextFrame()->IsVertical() )
            {
                nDescent = aCommonRect.Left();
                nAscent = aCommonRect.Right();

                if ( nDescent < 0 )
                    nDescent = -nDescent;
            }
            else
            {
                nDescent = aCommonRect.Bottom();
                nAscent = aCommonRect.Top();
            }
            if ( nAscent < 0 )
                nAscent = -nAscent;

            const long nHght = nAscent + nDescent;
            if ( nHght )
            {
                if ( nHght > nWishedHeight )
                    nMax = nFactor;
                else
                {
                    if ( bUseCache )
                        aFactor[ nTmpIdx ] = static_cast<sal_uInt16>(nFactor);
                    nMin = nFactor;
                }

                nFactor = ( nFactor * nWishedHeight ) / nHght;
                bGrow = ( nFactor > nMin ) && ( nFactor < nMax );
#if OSL_DEBUG_LEVEL > 1
                if ( bGrow )
                    nGrow++;
#endif
                nIdx = rInf.GetIdx();
            }
            else
                bGrow = false;
        }

        if ( bWinUsed )
        {
            // reset window if it has been used
            pWin->SetMapMode( aOldMap );
            pWin->SetFont( aOldFnt );
        }

        if ( bUseCache )
            aDescent[ nTmpIdx ] = -short( nDescent );
    }

    pCurrPart = pDrop->GetPart();

    // did made any new calculations or did we use the cache?
    if ( -1 == nFactor )
    {
        nFactor = aFactor[ nTmpIdx ];
        nDescent = aDescent[ nTmpIdx ];
    }
    else
        nDescent = -nDescent;

    while ( pCurrPart )
    {
        // scale current font
        SwFont& rFnt = pCurrPart->GetFont();
        Size aNewSize( 0, ( nFactor * rFnt.GetHeight( rFnt.GetActual() ) ) / 1000 );

        const sal_uInt8 nOldProp = rFnt.GetPropr();
        rFnt.SetProportion( 100 );
        rFnt.SetSize( aNewSize, rFnt.GetActual() );
        rFnt.SetProportion( nOldProp );

        pCurrPart = pCurrPart->GetFollow();
    }
    pDrop->SetY( static_cast<short>(nDescent) );
}

bool SwDropPortion::Format( SwTextFormatInfo &rInf )
{
    bool bFull = false;
    nFix = static_cast<sal_uInt16>(rInf.X());

    SwLayoutModeModifier aLayoutModeModifier( *rInf.GetOut() );
    aLayoutModeModifier.SetAuto();

    if( nDropHeight && pPart && nLines!=1 )
    {
        if( !pDropCapCache )
            pDropCapCache = new SwDropCapCache();

        // adjust font sizes to fit into the rectangle
        pDropCapCache->CalcFontSize( this, rInf );

        const long nOldX = rInf.X();
        {
            SwDropSave aSave( rInf );
            SwDropPortionPart* pCurrPart = pPart.get();

            while ( pCurrPart )
            {
                rInf.SetLen( pCurrPart->GetLen() );
                SwFont& rFnt = pCurrPart->GetFont();
                {
                    SwFontSave aFontSave( rInf, &rFnt );
                    SetJoinBorderWithNext(pCurrPart->GetJoinBorderWithNext());
                    SetJoinBorderWithPrev(pCurrPart->GetJoinBorderWithPrev());
                    bFull = FormatText( rInf );

                    if ( bFull )
                        break;
                }

                const SwTwips nTmpWidth =
                        ( InSpaceGrp() && rInf.GetSpaceAdd() ) ?
                        Width() + CalcSpacing( rInf.GetSpaceAdd(), rInf ) :
                        Width();

                // set values
                pCurrPart->SetWidth( static_cast<sal_uInt16>(nTmpWidth) );

                // Move
                rInf.SetIdx( rInf.GetIdx() + pCurrPart->GetLen() );
                rInf.X( rInf.X() + nTmpWidth );
                pCurrPart = pCurrPart->GetFollow();
            }
            SetJoinBorderWithNext(false);
            SetJoinBorderWithPrev(false);
            Width( static_cast<sal_uInt16>(rInf.X() - nOldX) );
        }

        // reset my length
        SetLen( rInf.GetLen() );

        // Quit when Flys are overlapping
        if( ! bFull )
            bFull = lcl_IsDropFlyInter( rInf, Width(), nDropHeight );

        if( bFull )
        {
            // FormatText could have caused nHeight to be 0
            if ( !Height() )
                Height( rInf.GetTextHeight() );

            // And now for another round
            nDropHeight = nLines = 0;
            pPart.reset();

            // Meanwhile use normal formatting
            bFull = SwTextPortion::Format( rInf );
        }
        else
            rInf.SetDropInit( true );

        Height( rInf.GetTextHeight() );
        SetAscent( rInf.GetAscent() );
    }
    else
        bFull = SwTextPortion::Format( rInf );

    if( bFull )
        nDistance = 0;
    else
    {
        const sal_uInt16 nWant = Width() + GetDistance();
        const sal_uInt16 nRest = static_cast<sal_uInt16>(rInf.Width() - rInf.X());
        if( ( nWant > nRest ) ||
            lcl_IsDropFlyInter( rInf, Width() + GetDistance(), nDropHeight ) )
            nDistance = 0;

        Width( Width() + nDistance );
    }
    return bFull;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
