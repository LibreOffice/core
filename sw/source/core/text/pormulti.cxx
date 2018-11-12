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

#include <deque>
#include <memory>

#include <hintids.hxx>

#include <editeng/twolinesitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <vcl/outdev.hxx>
#include <fmtfld.hxx>
#include <fldbas.hxx>
#include <txatbase.hxx>
#include <fmtruby.hxx>
#include <txtatr.hxx>
#include <charfmt.hxx>
#include <txtinet.hxx>
#include <fchrfmt.hxx>
#include <layfrm.hxx>
#include <SwPortionHandler.hxx>
#include "pormulti.hxx"
#include "inftxt.hxx"
#include "itrpaint.hxx"
#include <viewopt.hxx>
#include "itrform2.hxx"
#include "porfld.hxx"
#include "porglue.hxx"
#include <breakit.hxx>
#include <pagefrm.hxx>
#include <rowfrm.hxx>
#include <pagedesc.hxx>
#include <tgrditem.hxx>
#include <swtable.hxx>
#include <fmtfsize.hxx>

using namespace ::com::sun::star;

// A SwMultiPortion is not a simple portion,
// it's a container, which contains almost a SwLineLayoutPortion.
// This SwLineLayout could be followed by other textportions via pPortion
// and by another SwLineLayout via pNext to realize a doubleline portion.
SwMultiPortion::~SwMultiPortion()
{
}

void SwMultiPortion::Paint( const SwTextPaintInfo & ) const
{
    OSL_FAIL( "Don't try SwMultiPortion::Paint, try SwTextPainter::PaintMultiPortion" );
}

// Summarize the internal lines to calculate the (external) size.
// The internal line has to calculate first.
void SwMultiPortion::CalcSize( SwTextFormatter& rLine, SwTextFormatInfo &rInf )
{
    Width( 0 );
    Height( 0 );
    SetAscent( 0 );
    SetFlyInContent( false );
    SwLineLayout *pLay = &GetRoot();
    do
    {
        pLay->CalcLine( rLine, rInf );
        if( rLine.IsFlyInCntBase() )
            SetFlyInContent( true );
        if( IsRuby() && ( OnTop() == ( pLay == &GetRoot() ) ) )
        {
            // An empty phonetic line don't need an ascent or a height.
            if( !pLay->Width() )
            {
                pLay->SetAscent( 0 );
                pLay->Height( 0 );
            }
            if( OnTop() )
                SetAscent( GetAscent() + pLay->Height() );
        }
        else
            SetAscent( GetAscent() + pLay->GetAscent() );

        // Increase the line height, except for ruby text on the right.
        if ( !IsRuby() || !OnRight() || pLay == &GetRoot() )
            Height( Height() + pLay->Height() );
        else
        {
            // We already added the width after building the portion,
            // so no need to add it twice.
            break;
        }

        if( Width() < pLay->Width() )
            Width( pLay->Width() );
        pLay = pLay->GetNext();
    } while ( pLay );
    if( HasBrackets() )
    {
        sal_uInt16 nTmp = static_cast<SwDoubleLinePortion*>(this)->GetBrackets()->nHeight;
        if( nTmp > Height() )
        {
            const sal_uInt16 nAdd = ( nTmp - Height() ) / 2;
            GetRoot().SetAscent( GetRoot().GetAscent() + nAdd );
            GetRoot().Height( GetRoot().Height() + nAdd );
            Height( nTmp );
        }
        nTmp = static_cast<SwDoubleLinePortion*>(this)->GetBrackets()->nAscent;
        if( nTmp > GetAscent() )
            SetAscent( nTmp );
    }
}

long SwMultiPortion::CalcSpacing( long , const SwTextSizeInfo & ) const
{
    return 0;
}

bool SwMultiPortion::ChgSpaceAdd( SwLineLayout*, long ) const
{
    return false;
}

void SwMultiPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Text( GetLen(), GetWhichPor() );
}

// sets the tabulator-flag, if there's any tabulator-portion inside.
void SwMultiPortion::ActualizeTabulator()
{
    SwLinePortion* pPor = GetRoot().GetFirstPortion();
    // First line
    for( bTab1 = bTab2 = false; pPor; pPor = pPor->GetNextPortion() )
        if( pPor->InTabGrp() )
            SetTab1( true );
    if( GetRoot().GetNext() )
    {
        // Second line
        pPor = GetRoot().GetNext()->GetFirstPortion();
        do
        {
            if( pPor->InTabGrp() )
                SetTab2( true );
            pPor = pPor->GetNextPortion();
        } while ( pPor );
    }
}

SwRotatedPortion::SwRotatedPortion( const SwMultiCreator& rCreate,
        TextFrameIndex const nEnd, bool bRTL )
    : SwMultiPortion( nEnd )
{
    const SvxCharRotateItem* pRot = static_cast<const SvxCharRotateItem*>(rCreate.pItem);
    if( !pRot )
    {
        const SwTextAttr& rAttr = *rCreate.pAttr;
        const SfxPoolItem *const pItem =
                CharFormat::GetItem(rAttr, RES_CHRATR_ROTATE);
        if ( pItem )
        {
            pRot = static_cast<const SvxCharRotateItem*>(pItem);
        }
    }
    if( pRot )
    {
        sal_uInt8 nDir;
        if ( bRTL )
            nDir = pRot->IsBottomToTop() ? 3 : 1;
        else
            nDir = pRot->IsBottomToTop() ? 1 : 3;

        SetDirection( nDir );
    }
}

SwBidiPortion::SwBidiPortion(TextFrameIndex const nEnd, sal_uInt8 nLv)
    : SwMultiPortion( nEnd ), nLevel( nLv )
{
    SetBidi();

    if ( nLevel % 2 )
        SetDirection( DIR_RIGHT2LEFT );
    else
        SetDirection( DIR_LEFT2RIGHT );
}

long SwBidiPortion::CalcSpacing( long nSpaceAdd, const SwTextSizeInfo& rInf ) const
{
    return HasTabulator() ? 0 : sal_Int32(GetSpaceCnt(rInf)) * nSpaceAdd / SPACING_PRECISION_FACTOR;
}

bool SwBidiPortion::ChgSpaceAdd( SwLineLayout* pCurr, long nSpaceAdd ) const
{
    if( !HasTabulator() && nSpaceAdd > 0 && !pCurr->IsSpaceAdd() )
    {
        pCurr->CreateSpaceAdd();
        pCurr->SetLLSpaceAdd( nSpaceAdd, 0 );
        return true;
    }

    return false;
}

TextFrameIndex SwBidiPortion::GetSpaceCnt(const SwTextSizeInfo &rInf) const
{
    // Calculate number of blanks for justified alignment
    TextFrameIndex nTmpStart = rInf.GetIdx();
    TextFrameIndex nNull(0);
    TextFrameIndex nBlanks(0);

    for (SwLinePortion* pPor = GetRoot().GetFirstPortion(); pPor; pPor = pPor->GetNextPortion())
    {
        if( pPor->InTextGrp() )
            nBlanks = nBlanks + static_cast<SwTextPortion*>(pPor)->GetSpaceCnt( rInf, nNull );
        else if ( pPor->IsMultiPortion() &&
                 static_cast<SwMultiPortion*>(pPor)->IsBidi() )
            nBlanks = nBlanks + static_cast<SwBidiPortion*>(pPor)->GetSpaceCnt( rInf );

        const_cast<SwTextSizeInfo &>(rInf).SetIdx( rInf.GetIdx() + pPor->GetLen() );
    }
    const_cast<SwTextSizeInfo &>(rInf).SetIdx( nTmpStart );
    return nBlanks;
}

// This constructor is for the continuation of a doubleline portion
// in the next line.
// It takes the same brackets and if the original has no content except
// brackets, these will be deleted.
SwDoubleLinePortion::SwDoubleLinePortion(
        SwDoubleLinePortion& rDouble, TextFrameIndex const nEnd)
    : SwMultiPortion(nEnd)
    , nLineDiff(0)
    , nBlank1(0)
    , nBlank2(0)
{
    SetDirection( rDouble.GetDirection() );
    SetDouble();
    if( rDouble.GetBrackets() )
    {
        SetBrackets( rDouble );
        // An empty multiportion needs no brackets.
        // Notice: GetLen() might be zero, if the multiportion contains
        // the second part of a field and the width might be zero, if
        // it contains a note only. In this cases the brackets are okay.
        // But if the length and the width are both zero, the portion
        // is really empty.
        if( rDouble.Width() ==  rDouble.BracketWidth() )
            rDouble.ClearBrackets();
    }
}

// This constructor uses the textattribute to get the right brackets.
// The textattribute could be a 2-line-attribute or a character- or
// internet style, which contains the 2-line-attribute.
SwDoubleLinePortion::SwDoubleLinePortion(
        const SwMultiCreator& rCreate, TextFrameIndex const nEnd)
    : SwMultiPortion(nEnd)
    , pBracket(new SwBracket)
    , nLineDiff(0)
    , nBlank1(0)
    , nBlank2(0)
{
    pBracket->nAscent = 0;
    pBracket->nHeight = 0;
    pBracket->nPreWidth = 0;
    pBracket->nPostWidth = 0;

    SetDouble();
    const SvxTwoLinesItem* pTwo = static_cast<const SvxTwoLinesItem*>(rCreate.pItem);
    if( pTwo )
        pBracket->nStart = TextFrameIndex(0);
    else
    {
        const SwTextAttr& rAttr = *rCreate.pAttr;
        pBracket->nStart = rCreate.nStartOfAttr;

        const SfxPoolItem * const pItem =
            CharFormat::GetItem( rAttr, RES_CHRATR_TWO_LINES );
        if ( pItem )
        {
            pTwo = static_cast<const SvxTwoLinesItem*>(pItem);
        }
    }
    if( pTwo )
    {
        pBracket->cPre = pTwo->GetStartBracket();
        pBracket->cPost = pTwo->GetEndBracket();
    }
    else
    {
        pBracket->cPre = 0;
        pBracket->cPost = 0;
    }
    SwFontScript nTmp = SW_SCRIPTS;
    if( pBracket->cPre > 255 )
    {
        OUString aText = OUString(pBracket->cPre);
        nTmp = SwScriptInfo::WhichFont(0, aText);
    }
    pBracket->nPreScript = nTmp;
    nTmp = SW_SCRIPTS;
    if( pBracket->cPost > 255 )
    {
        OUString aText = OUString(pBracket->cPost);
        nTmp = SwScriptInfo::WhichFont(0, aText);
    }
    pBracket->nPostScript = nTmp;

    if( !pBracket->cPre && !pBracket->cPost )
    {
        pBracket.reset();
    }

    // double line portions have the same direction as the frame directions
    if ( rCreate.nLevel % 2 )
        SetDirection( DIR_RIGHT2LEFT );
    else
        SetDirection( DIR_LEFT2RIGHT );
}

// paints the wished bracket,
// if the multiportion has surrounding brackets.
// The X-position of the SwTextPaintInfo will be modified:
// the open bracket sets position behind itself,
// the close bracket in front of itself.
void SwDoubleLinePortion::PaintBracket( SwTextPaintInfo &rInf,
                                        long nSpaceAdd,
                                        bool bOpen ) const
{
    sal_Unicode cCh = bOpen ? pBracket->cPre : pBracket->cPost;
    if( !cCh )
        return;
    const sal_uInt16 nChWidth = bOpen ? PreWidth() : PostWidth();
    if( !nChWidth )
        return;
    if( !bOpen )
        rInf.X( rInf.X() + Width() - PostWidth() +
            ( nSpaceAdd > 0 ? CalcSpacing( nSpaceAdd, rInf ) : 0 ) );

    SwBlankPortion aBlank( cCh, true );
    aBlank.SetAscent( pBracket->nAscent );
    aBlank.Width( nChWidth );
    aBlank.Height( pBracket->nHeight );
    {
        std::unique_ptr<SwFont> pTmpFnt( new SwFont( *rInf.GetFont() ) );
        SwFontScript nAct = bOpen ? pBracket->nPreScript : pBracket->nPostScript;
        if( SW_SCRIPTS > nAct )
            pTmpFnt->SetActual( nAct );
        pTmpFnt->SetProportion( 100 );
        SwFontSave aSave( rInf, pTmpFnt.get() );
        aBlank.Paint( rInf );
    }
    if( bOpen )
        rInf.X( rInf.X() + PreWidth() );
}

// creates the bracket-structure
// and fills it, if not both characters are 0x00.
void SwDoubleLinePortion::SetBrackets( const SwDoubleLinePortion& rDouble )
{
    if( rDouble.pBracket )
    {
        pBracket.reset( new SwBracket );
        pBracket->cPre = rDouble.pBracket->cPre;
        pBracket->cPost = rDouble.pBracket->cPost;
        pBracket->nPreScript = rDouble.pBracket->nPreScript;
        pBracket->nPostScript = rDouble.pBracket->nPostScript;
        pBracket->nStart = rDouble.pBracket->nStart;
    }
}

// calculates the size of the brackets => pBracket,
// reduces the nMaxWidth-parameter ( minus bracket-width )
// and moves the rInf-x-position behind the opening bracket.
void SwDoubleLinePortion::FormatBrackets( SwTextFormatInfo &rInf, SwTwips& nMaxWidth )
{
    nMaxWidth -= rInf.X();
    std::unique_ptr<SwFont> pTmpFnt( new SwFont( *rInf.GetFont() ) );
    pTmpFnt->SetProportion( 100 );
    pBracket->nAscent = 0;
    pBracket->nHeight = 0;
    if( pBracket->cPre )
    {
        OUString aStr( pBracket->cPre );
        SwFontScript nActualScr = pTmpFnt->GetActual();
        if( SW_SCRIPTS > pBracket->nPreScript )
            pTmpFnt->SetActual( pBracket->nPreScript );
        SwFontSave aSave( rInf, pTmpFnt.get() );
        SwPosSize aSize = rInf.GetTextSize( aStr );
        pBracket->nAscent = rInf.GetAscent();
        pBracket->nHeight = aSize.Height();
        pTmpFnt->SetActual( nActualScr );
        if( nMaxWidth > aSize.Width() )
        {
            pBracket->nPreWidth = aSize.Width();
            nMaxWidth -= aSize.Width();
            rInf.X( rInf.X() + aSize.Width() );
        }
        else
        {
            pBracket->nPreWidth = 0;
            nMaxWidth = 0;
        }
    }
    else
        pBracket->nPreWidth = 0;
    if( pBracket->cPost )
    {
        OUString aStr( pBracket->cPost );
        if( SW_SCRIPTS > pBracket->nPostScript )
            pTmpFnt->SetActual( pBracket->nPostScript );
        SwFontSave aSave( rInf, pTmpFnt.get() );
        SwPosSize aSize = rInf.GetTextSize( aStr );
        const sal_uInt16 nTmpAsc = rInf.GetAscent();
        if( nTmpAsc > pBracket->nAscent )
        {
            pBracket->nHeight += nTmpAsc - pBracket->nAscent;
            pBracket->nAscent = nTmpAsc;
        }
        if( aSize.Height() > pBracket->nHeight )
            pBracket->nHeight = aSize.Height();
        if( nMaxWidth > aSize.Width() )
        {
            pBracket->nPostWidth = aSize.Width();
            nMaxWidth -= aSize.Width();
        }
        else
        {
            pBracket->nPostWidth = 0;
            nMaxWidth = 0;
        }
    }
    else
        pBracket->nPostWidth = 0;
    nMaxWidth += rInf.X();
}

// calculates the number of blanks in each line and
// the difference of the width of the two lines.
// These results are used from the text adjustment.
void SwDoubleLinePortion::CalcBlanks( SwTextFormatInfo &rInf )
{
    SwLinePortion* pPor = GetRoot().GetFirstPortion();
    TextFrameIndex nNull(0);
    TextFrameIndex nStart = rInf.GetIdx();
    SetTab1( false );
    SetTab2( false );
    for (nBlank1 = TextFrameIndex(0); pPor; pPor = pPor->GetNextPortion())
    {
        if( pPor->InTextGrp() )
            nBlank1 = nBlank1 + static_cast<SwTextPortion*>(pPor)->GetSpaceCnt( rInf, nNull );
        rInf.SetIdx( rInf.GetIdx() + pPor->GetLen() );
        if( pPor->InTabGrp() )
            SetTab1( true );
    }
    nLineDiff = GetRoot().Width();
    if( GetRoot().GetNext() )
    {
        pPor = GetRoot().GetNext()->GetFirstPortion();
        nLineDiff -= GetRoot().GetNext()->Width();
    }
    for (nBlank2 = TextFrameIndex(0); pPor; pPor = pPor->GetNextPortion())
    {
        if( pPor->InTextGrp() )
            nBlank2 = nBlank2 + static_cast<SwTextPortion*>(pPor)->GetSpaceCnt( rInf, nNull );
        rInf.SetIdx( rInf.GetIdx() + pPor->GetLen() );
        if( pPor->InTabGrp() )
            SetTab2( true );
    }
    rInf.SetIdx( nStart );
}

long SwDoubleLinePortion::CalcSpacing( long nSpaceAdd, const SwTextSizeInfo & ) const
{
    return HasTabulator() ? 0 : sal_Int32(GetSpaceCnt()) * nSpaceAdd / SPACING_PRECISION_FACTOR;
}

// Merges the spaces for text adjustment from the inner and outer part.
// Inside the doubleline portion the wider line has no spaceadd-array, the
// smaller line has such an array to reach width of the wider line.
// If the surrounding line has text adjustment and the doubleline portion
// contains no tabulator, it is necessary to create/manipulate the inner
// space arrays.
bool SwDoubleLinePortion::ChgSpaceAdd( SwLineLayout* pCurr,
                                           long nSpaceAdd ) const
{
    bool bRet = false;
    if( !HasTabulator() && nSpaceAdd > 0 )
    {
        if( !pCurr->IsSpaceAdd() )
        {
            // The wider line gets the spaceadd from the surrounding line direct
            pCurr->CreateSpaceAdd();
            pCurr->SetLLSpaceAdd( nSpaceAdd, 0 );
            bRet = true;
        }
        else
        {
            sal_Int32 const nMyBlank = sal_Int32(GetSmallerSpaceCnt());
            sal_Int32 const nOther = sal_Int32(GetSpaceCnt());
            SwTwips nMultiSpace = pCurr->GetLLSpaceAdd( 0 ) * nMyBlank + nOther * nSpaceAdd;

            if( nMyBlank )
                nMultiSpace /= sal_Int32(nMyBlank);

//            pCurr->SetLLSpaceAdd( nMultiSpace, 0 );
            // #i65711# SetLLSpaceAdd replaces the first value,
            // instead we want to insert a new first value:
            std::vector<long>* pVec = pCurr->GetpLLSpaceAdd();
            pVec->insert( pVec->begin(), nMultiSpace );
            bRet = true;
        }
    }
    return bRet;
}
// cancels the manipulation from SwDoubleLinePortion::ChangeSpaceAdd(..)
void SwDoubleLinePortion::ResetSpaceAdd( SwLineLayout* pCurr )
{
    pCurr->RemoveFirstLLSpaceAdd();
    if( !pCurr->GetLLSpaceAddCount() )
        pCurr->FinishSpaceAdd();
}

SwDoubleLinePortion::~SwDoubleLinePortion()
{
}

// constructs a ruby portion, i.e. an additional text is displayed
// beside the main text, e.g. phonetic characters.
SwRubyPortion::SwRubyPortion(const SwRubyPortion& rRuby, TextFrameIndex const nEnd)
    : SwMultiPortion( nEnd )
    , nRubyOffset( rRuby.GetRubyOffset() )
    , nAdjustment( rRuby.GetAdjustment() )
{
    SetDirection( rRuby.GetDirection() );
    SetRubyPosition( rRuby.GetRubyPosition() );
    SetRuby();
}

// constructs a ruby portion, i.e. an additional text is displayed
// beside the main text, e.g. phonetic characters.
SwRubyPortion::SwRubyPortion( const SwMultiCreator& rCreate, const SwFont& rFnt,
                              const IDocumentSettingAccess& rIDocumentSettingAccess,
                              TextFrameIndex const nEnd, TextFrameIndex const nOffs,
                              const SwTextSizeInfo &rInf )
     : SwMultiPortion( nEnd )
{
    SetRuby();
    OSL_ENSURE( SwMultiCreatorId::Ruby == rCreate.nId, "Ruby expected" );
    OSL_ENSURE( RES_TXTATR_CJK_RUBY == rCreate.pAttr->Which(), "Wrong attribute" );
    const SwFormatRuby& rRuby = rCreate.pAttr->GetRuby();
    nAdjustment = rRuby.GetAdjustment();
    nRubyOffset = nOffs;

    const SwTextFrame *pFrame = rInf.GetTextFrame();
    RubyPosition ePos = static_cast<RubyPosition>( rRuby.GetPosition() );

    // RIGHT is designed for horizontal writing mode only.
    if ( ePos == RubyPosition::RIGHT && pFrame->IsVertical() )
        ePos = RubyPosition::ABOVE;

    // In grid mode we force the ruby text to the upper or lower line
    if ( rInf.SnapToGrid() )
    {
        SwTextGridItem const*const pGrid( GetGridItem(pFrame->FindPageFrame()) );
        if ( pGrid )
            ePos = pGrid->GetRubyTextBelow() ? RubyPosition::BELOW : RubyPosition::ABOVE;
    }

    SetRubyPosition( ePos );

    const SwCharFormat *const pFormat =
        static_txtattr_cast<SwTextRuby const*>(rCreate.pAttr)->GetCharFormat();
    std::unique_ptr<SwFont> pRubyFont;
    if( pFormat )
    {
        const SwAttrSet& rSet = pFormat->GetAttrSet();
        pRubyFont.reset(new SwFont( rFnt ));
        pRubyFont->SetDiffFnt( &rSet, &rIDocumentSettingAccess );

        // we do not allow a vertical font for the ruby text
        pRubyFont->SetVertical( rFnt.GetOrientation() , OnRight() );
    }

    OUString aStr = rRuby.GetText().copy( sal_Int32(nOffs) );
    SwFieldPortion *pField = new SwFieldPortion( aStr, std::move(pRubyFont) );
    pField->SetNextOffset( nOffs );
    pField->SetFollow( true );

    if( OnTop() )
        GetRoot().SetNextPortion( pField );
    else
    {
        GetRoot().SetNext( new SwLineLayout() );
        GetRoot().GetNext()->SetNextPortion( pField );
    }

    // ruby portions have the same direction as the frame directions
    if ( rCreate.nLevel % 2 )
    {
        // switch right and left ruby adjustment in rtl environment
        if ( css::text::RubyAdjust_LEFT == nAdjustment )
            nAdjustment = css::text::RubyAdjust_RIGHT;
        else if ( css::text::RubyAdjust_RIGHT == nAdjustment )
            nAdjustment = css::text::RubyAdjust_LEFT;

        SetDirection( DIR_RIGHT2LEFT );
    }
    else
        SetDirection( DIR_LEFT2RIGHT );
}

// In ruby portion there are different alignments for
// the ruby text and the main text.
// Left, right, centered and two possibilities of block adjustment
// The block adjustment is realized by spacing between the characters,
// either with a half space or no space in front of the first letter and
// a half space at the end of the last letter.
// Notice: the smaller line will be manipulated, normally it's the ruby line,
// but it could be the main text, too.
// If there is a tabulator in smaller line, no adjustment is possible.
void SwRubyPortion::Adjust_( SwTextFormatInfo &rInf )
{
    SwTwips nLineDiff = GetRoot().Width() - GetRoot().GetNext()->Width();
    TextFrameIndex const nOldIdx = rInf.GetIdx();
    if( !nLineDiff )
        return;
    SwLineLayout *pCurr;
    if( nLineDiff < 0 )
    {   // The first line has to be adjusted.
        if( GetTab1() )
            return;
        pCurr = &GetRoot();
        nLineDiff = -nLineDiff;
    }
    else
    {   // The second line has to be adjusted.
        if( GetTab2() )
            return;
        pCurr = GetRoot().GetNext();
        rInf.SetIdx( nOldIdx + GetRoot().GetLen() );
    }
    sal_uInt16 nLeft = 0;   // the space in front of the first letter
    sal_uInt16 nRight = 0;  // the space at the end of the last letter
    TextFrameIndex nSub(0);
    switch ( nAdjustment )
    {
        case css::text::RubyAdjust_CENTER: nRight = static_cast<sal_uInt16>(nLineDiff / 2);
            [[fallthrough]];
        case css::text::RubyAdjust_RIGHT: nLeft  = static_cast<sal_uInt16>(nLineDiff - nRight); break;
        case css::text::RubyAdjust_BLOCK: nSub   = TextFrameIndex(1);
            [[fallthrough]];
        case css::text::RubyAdjust_INDENT_BLOCK:
        {
            TextFrameIndex nCharCnt(0);
            SwLinePortion *pPor;
            for( pPor = pCurr->GetFirstPortion(); pPor; pPor = pPor->GetNextPortion() )
            {
                if( pPor->InTextGrp() )
                    static_cast<SwTextPortion*>(pPor)->GetSpaceCnt( rInf, nCharCnt );
                rInf.SetIdx( rInf.GetIdx() + pPor->GetLen() );
            }
            if( nCharCnt > nSub )
            {
                SwTwips nCalc = nLineDiff / sal_Int32(nCharCnt - nSub);
                short nTmp;
                if( nCalc < SHRT_MAX )
                    nTmp = -short(nCalc);
                else
                    nTmp = SHRT_MIN;

                pCurr->CreateSpaceAdd( SPACING_PRECISION_FACTOR * nTmp );
                nLineDiff -= nCalc * (sal_Int32(nCharCnt) - 1);
            }
            if( nLineDiff > 1 )
            {
                nRight = static_cast<sal_uInt16>(nLineDiff / 2);
                nLeft  = static_cast<sal_uInt16>(nLineDiff - nRight);
            }
            break;
        }
        default: OSL_FAIL( "New ruby adjustment" );
    }
    if( nLeft || nRight )
    {
        if( !pCurr->GetNextPortion() )
            pCurr->SetNextPortion(SwTextPortion::CopyLinePortion(*pCurr));
        if( nLeft )
        {
            SwMarginPortion *pMarg = new SwMarginPortion;
            pMarg->AddPrtWidth( nLeft );
            pMarg->SetNextPortion( pCurr->GetNextPortion() );
            pCurr->SetNextPortion( pMarg );
        }
        if( nRight )
        {
            SwMarginPortion *pMarg = new SwMarginPortion;
            pMarg->AddPrtWidth( nRight );
            pCurr->FindLastPortion()->Append( pMarg );
        }
    }

    pCurr->Width( Width() );
    rInf.SetIdx( nOldIdx );
}

// has to change the nRubyOffset, if there's a fieldportion
// in the phonetic line.
// The nRubyOffset is the position in the rubystring, where the
// next SwRubyPortion has start the displaying of the phonetics.
void SwRubyPortion::CalcRubyOffset()
{
    const SwLineLayout *pCurr = &GetRoot();
    if( !OnTop() )
    {
        pCurr = pCurr->GetNext();
        if( !pCurr )
            return;
    }
    const SwLinePortion *pPor = pCurr->GetFirstPortion();
    const SwFieldPortion *pField = nullptr;
    while( pPor )
    {
        if( pPor->InFieldGrp() )
            pField = static_cast<const SwFieldPortion*>(pPor);
        pPor = pPor->GetNextPortion();
    }
    if( pField )
    {
        if( pField->HasFollow() )
            nRubyOffset = pField->GetNextOffset();
        else
            nRubyOffset = TextFrameIndex(COMPLETE_STRING);
    }
}

// A little helper function for GetMultiCreator(..)
// It extracts the 2-line-format from a 2-line-attribute or a character style.
// The rValue is set to true, if the 2-line-attribute's value is set and
// no 2-line-format reference is passed. If there is a 2-line-format reference,
// then the rValue is set only, if the 2-line-attribute's value is set _and_
// the 2-line-formats has the same brackets.
static bool lcl_Check2Lines(const SfxPoolItem *const pItem,
        const SvxTwoLinesItem* &rpRef, bool &rValue)
{
    if( pItem )
    {
        rValue = static_cast<const SvxTwoLinesItem*>(pItem)->GetValue();
        if( !rpRef )
            rpRef = static_cast<const SvxTwoLinesItem*>(pItem);
        else if( static_cast<const SvxTwoLinesItem*>(pItem)->GetEndBracket() !=
                    rpRef->GetEndBracket() ||
                    static_cast<const SvxTwoLinesItem*>(pItem)->GetStartBracket() !=
                    rpRef->GetStartBracket() )
            rValue = false;
        return true;
    }
    return false;
}

static bool lcl_Has2Lines(const SwTextAttr& rAttr,
        const SvxTwoLinesItem* &rpRef, bool &rValue)
{
    const SfxPoolItem* pItem = CharFormat::GetItem(rAttr, RES_CHRATR_TWO_LINES);
    return lcl_Check2Lines(pItem, rpRef, rValue);
}

// is a little help function for GetMultiCreator(..)
// It extracts the charrotation from a charrotate-attribute or a character style.
// The rValue is set to true, if the charrotate-attribute's value is set and
// no charrotate-format reference is passed.
// If there is a charrotate-format reference, then the rValue is set only,
// if the charrotate-attribute's value is set _and_ identical
// to the charrotate-format's value.
static bool lcl_CheckRotation(const SfxPoolItem *const pItem,
        const SvxCharRotateItem* &rpRef, bool &rValue)
{
    if ( pItem )
    {
        rValue = static_cast<const SvxCharRotateItem*>(pItem)->GetValue();
        if( !rpRef )
            rpRef = static_cast<const SvxCharRotateItem*>(pItem);
        else if( static_cast<const SvxCharRotateItem*>(pItem)->GetValue() !=
                    rpRef->GetValue() )
            rValue = false;
        return true;
    }

    return false;
}

static bool lcl_HasRotation(const SwTextAttr& rAttr,
        const SvxCharRotateItem* &rpRef, bool &rValue)
{
    const SfxPoolItem* pItem = CharFormat::GetItem( rAttr, RES_CHRATR_ROTATE );
    return lcl_CheckRotation(pItem, rpRef, rValue);
}

namespace sw {

    // need to use a very special attribute iterator here that returns
    // both the hints and the nodes, so that GetMultiCreator() can handle
    // items in the nodes' set properly
    class MergedAttrIterMulti
        : public MergedAttrIterBase
    {
    private:
        bool m_First = true;
    public:
        MergedAttrIterMulti(SwTextFrame const& rFrame) : MergedAttrIterBase(rFrame) {}
        SwTextAttr const* NextAttr(SwTextNode const*& rpNode);
        // can't have operator= because m_pMerged/m_pNode const
        void Assign(MergedAttrIterMulti const& rOther)
        {
            assert(m_pMerged == rOther.m_pMerged);
            assert(m_pNode == rOther.m_pNode);
            m_CurrentExtent = rOther.m_CurrentExtent;
            m_CurrentHint = rOther.m_CurrentHint;
            m_First = rOther.m_First;
        }
    };

    SwTextAttr const* MergedAttrIterMulti::NextAttr(SwTextNode const*& rpNode)
    {
        if (m_First)
        {
            m_First = false;
            rpNode = m_pMerged
                ? !m_pMerged->extents.empty()
                    ? m_pMerged->extents[0].pNode
                    : m_pMerged->pFirstNode
                : m_pNode;
            return nullptr;
        }
        if (m_pMerged)
        {
            while (m_CurrentExtent < m_pMerged->extents.size())
            {
                sw::Extent const& rExtent(m_pMerged->extents[m_CurrentExtent]);
                if (SwpHints const*const pHints = rExtent.pNode->GetpSwpHints())
                {
                    while (m_CurrentHint < pHints->Count())
                    {
                        SwTextAttr const*const pHint(pHints->Get(m_CurrentHint));
                        if (rExtent.nEnd < pHint->GetStart())
                        {
                            break;
                        }
                        ++m_CurrentHint;
                        if (rExtent.nStart <= pHint->GetStart())
                        {
                            rpNode = rExtent.pNode;
                            return pHint;
                        }
                    }
                }
                ++m_CurrentExtent;
                if (m_CurrentExtent < m_pMerged->extents.size() &&
                    rExtent.pNode != m_pMerged->extents[m_CurrentExtent].pNode)
                {
                    m_CurrentHint = 0; // reset
                    rpNode = m_pMerged->extents[m_CurrentExtent].pNode;
                    return nullptr;
                }
            }
            return nullptr;
        }
        else
        {
            SwpHints const*const pHints(m_pNode->GetpSwpHints());
            if (pHints)
            {
                if (m_CurrentHint < pHints->Count())
                {
                    SwTextAttr const*const pHint(pHints->Get(m_CurrentHint));
                    ++m_CurrentHint;
                    rpNode = m_pNode;
                    return pHint;
                }
            }
            return nullptr;
        }
    }
}

// If we (e.g. the position rPos) are inside a two-line-attribute or
// a ruby-attribute, the attribute will be returned in a SwMultiCreator-struct,
// otherwise the function returns zero.
// The rPos parameter is set to the end of the multiportion,
// normally this is the end of the attribute,
// but sometimes it is the start of another attribute, which finished or
// interrupts the first attribute.
// E.g. a ruby portion interrupts a 2-line-attribute, a 2-line-attribute
// with different brackets interrupts another 2-line-attribute.
std::unique_ptr<SwMultiCreator> SwTextSizeInfo::GetMultiCreator(TextFrameIndex &rPos,
                                                SwMultiPortion const * pMulti ) const
{
    SwScriptInfo& rSI = const_cast<SwParaPortion*>(GetParaPortion())->GetScriptInfo();

    // get the last embedding level
    sal_uInt8 nCurrLevel;
    if ( pMulti )
    {
        OSL_ENSURE( pMulti->IsBidi(), "Nested MultiPortion is not BidiPortion" );
        // level associated with bidi-portion;
        nCurrLevel = static_cast<SwBidiPortion const *>(pMulti)->GetLevel();
    }
    else
        // no nested bidi portion required
        nCurrLevel = GetTextFrame()->IsRightToLeft() ? 1 : 0;

    // check if there is a field at rPos:
    sal_uInt8 nNextLevel = nCurrLevel;
    bool bFieldBidi = false;

    if (rPos < TextFrameIndex(GetText().getLength()) && CH_TXTATR_BREAKWORD == GetChar(rPos))
    {
        bFieldBidi = true;
    }
    else
        nNextLevel = rSI.DirType( rPos );

    if (TextFrameIndex(GetText().getLength()) != rPos && nNextLevel > nCurrLevel)
    {
        rPos = bFieldBidi ? rPos + TextFrameIndex(1) : rSI.NextDirChg(rPos, &nCurrLevel);
        if (TextFrameIndex(COMPLETE_STRING) == rPos)
            return nullptr;
        std::unique_ptr<SwMultiCreator> pRet(new SwMultiCreator);
        pRet->pItem = nullptr;
        pRet->pAttr = nullptr;
        pRet->nStartOfAttr = TextFrameIndex(-1);
        pRet->nId = SwMultiCreatorId::Bidi;
        pRet->nLevel = nCurrLevel + 1;
        return pRet;
    }

    // a bidi portion can only contain other bidi portions
    if ( pMulti )
        return nullptr;

    // need the node that contains input rPos
    std::pair<SwTextNode const*, sal_Int32> startPos(m_pFrame->MapViewToModel(rPos));
    const SvxCharRotateItem* pActiveRotateItem(nullptr);
    const SfxPoolItem* pNodeRotateItem(nullptr);
    const SvxTwoLinesItem* pActiveTwoLinesItem(nullptr);
    const SfxPoolItem* pNodeTwoLinesItem(nullptr);
    SwTextAttr const* pActiveTwoLinesHint(nullptr);
    SwTextAttr const* pActiveRotateHint(nullptr);
    const SwTextAttr *pRuby = nullptr;
    sw::MergedAttrIterMulti iterAtStartOfNode(*m_pFrame);
    bool bTwo = false;
    bool bRot = false;

    for (sw::MergedAttrIterMulti iter = *m_pFrame; ; )
    {
        SwTextNode const* pNode(nullptr);
        SwTextAttr const*const pAttr = iter.NextAttr(pNode);
        if (!pNode)
        {
            break;
        }
        if (pAttr)
        {
            assert(pNode->GetIndex() <= startPos.first->GetIndex()); // should break earlier
            if (startPos.first->GetIndex() <= pNode->GetIndex())
            {
                if (startPos.first->GetIndex() != pNode->GetIndex()
                    || startPos.second < pAttr->GetStart())
                {
                    break;
                }
                if (startPos.second < *pAttr->GetAnyEnd())
                {
                    // sw_redlinehide: ruby *always* splits
                    if (RES_TXTATR_CJK_RUBY == pAttr->Which())
                        pRuby = pAttr;
                    else
                    {
                        const SvxCharRotateItem* pRoTmp = nullptr;
                        if (lcl_HasRotation( *pAttr, pRoTmp, bRot ))
                        {
                            pActiveRotateHint = bRot ? pAttr : nullptr;
                            pActiveRotateItem = pRoTmp;
                        }
                        const SvxTwoLinesItem* p2Tmp = nullptr;
                        if (lcl_Has2Lines( *pAttr, p2Tmp, bTwo ))
                        {
                            pActiveTwoLinesHint = bTwo ? pAttr : nullptr;
                            pActiveTwoLinesItem = p2Tmp;
                        }
                    }
                }
            }
        }
        else if (pNode) // !pAttr && pNode means the node changed
        {
            if (startPos.first->GetIndex() < pNode->GetIndex())
            {
                break; // only one node initially
            }
            if (startPos.first->GetIndex() == pNode->GetIndex())
            {
                iterAtStartOfNode.Assign(iter);
                if (SfxItemState::SET == pNode->GetSwAttrSet().GetItemState(
                            RES_CHRATR_ROTATE, true, &pNodeRotateItem) &&
                    static_cast<const SvxCharRotateItem*>(pNodeRotateItem)->GetValue())
                {
                    pActiveRotateItem = static_cast<const SvxCharRotateItem*>(pNodeRotateItem);
                }
                else
                {
                    pNodeRotateItem = nullptr;
                }
                if (SfxItemState::SET == startPos.first->GetSwAttrSet().GetItemState(
                            RES_CHRATR_TWO_LINES, true, &pNodeTwoLinesItem) &&
                    static_cast<const SvxTwoLinesItem*>(pNodeTwoLinesItem)->GetValue())
                {
                    pActiveTwoLinesItem = static_cast<const SvxTwoLinesItem*>(pNodeTwoLinesItem);
                }
                else
                {
                    pNodeTwoLinesItem = nullptr;
                }
            }
        }
    }
    if (!pRuby && !pActiveTwoLinesItem && !pActiveRotateItem)
        return nullptr;

    if( pRuby )
    {   // The winner is ... a ruby attribute and so
        // the end of the multiportion is the end of the ruby attribute.
        rPos = m_pFrame->MapModelToView(startPos.first, *pRuby->End());
        std::unique_ptr<SwMultiCreator> pRet(new SwMultiCreator);
        pRet->pItem = nullptr;
        pRet->pAttr = pRuby;
        pRet->nStartOfAttr = m_pFrame->MapModelToView(startPos.first, pRet->pAttr->GetStart());
        pRet->nId = SwMultiCreatorId::Ruby;
        pRet->nLevel = GetTextFrame()->IsRightToLeft() ? 1 : 0;
        return pRet;
    }
    if (pActiveTwoLinesHint ||
        (pNodeTwoLinesItem && pNodeTwoLinesItem == pActiveTwoLinesItem &&
         rPos < TextFrameIndex(GetText().getLength())))
    {   // The winner is a 2-line-attribute,
        // the end of the multiportion depends on the following attributes...
        std::unique_ptr<SwMultiCreator> pRet(new SwMultiCreator);

        // We note the endpositions of the 2-line attributes in aEnd as stack
        std::deque<TextFrameIndex> aEnd;

        // The bOn flag signs the state of the last 2-line attribute in the
        // aEnd-stack, it is compatible with the winner-attribute or
        // it interrupts the other attribute.
        bool bOn = true;

        if (pActiveTwoLinesHint)
        {
            pRet->pItem = nullptr;
            pRet->pAttr = pActiveTwoLinesHint;
            pRet->nStartOfAttr = m_pFrame->MapModelToView(startPos.first, pRet->pAttr->GetStart());
            if (pNodeTwoLinesItem)
            {
                aEnd.push_front(m_pFrame->MapModelToView(startPos.first, startPos.first->Len()));
                bOn = static_cast<const SvxTwoLinesItem*>(pNodeTwoLinesItem)->GetEndBracket() ==
                        pActiveTwoLinesItem->GetEndBracket() &&
                      static_cast<const SvxTwoLinesItem*>(pNodeTwoLinesItem)->GetStartBracket() ==
                        pActiveTwoLinesItem->GetStartBracket();
            }
            else
            {
                aEnd.push_front(m_pFrame->MapModelToView(startPos.first, *pRet->pAttr->End()));
            }
        }
        else
        {
            pRet->pItem = pNodeTwoLinesItem;
            pRet->pAttr = nullptr;
            pRet->nStartOfAttr = TextFrameIndex(-1);
            aEnd.push_front(m_pFrame->MapModelToView(startPos.first, startPos.first->Len()));
        }
        pRet->nId = SwMultiCreatorId::Double;
        pRet->nLevel = GetTextFrame()->IsRightToLeft() ? 1 : 0;

        // pActiveTwoLinesHint is the last 2-line-attribute, which contains
        // the actual position.

        // At this moment we know that at position rPos the "winner"-attribute
        // causes a 2-line-portion. The end of the attribute is the end of the
        // portion, if there's no interrupting attribute.
        // There are two kinds of interrupters:
        // - ruby attributes stops the 2-line-attribute, the end of the
        //   multiline is the start of the ruby attribute
        // - 2-line-attributes with value "Off" or with different brackets,
        //   these attributes may interrupt the winner, but they could be
        //   neutralized by another 2-line-attribute starting at the same
        //   position with the same brackets as the winner-attribute.

        // In the following loop rPos is the critical position and it will be
        // evaluated, if at rPos starts a interrupting or a maintaining
        // continuity attribute.

        // iterAtStartOfNode is positioned to the first hint of the node
        // (if any); the node item itself has already been handled above
        for (sw::MergedAttrIterMulti iter = iterAtStartOfNode; ; )
        {
            SwTextNode const* pNode(nullptr);
            SwTextAttr const*const pTmp = iter.NextAttr(pNode);
            if (!pNode)
            {
                break;
            }
            assert(startPos.first->GetIndex() <= pNode->GetIndex());
            TextFrameIndex nTmpStart;
            TextFrameIndex nTmpEnd;
            if (pTmp)
            {
                nTmpEnd = m_pFrame->MapModelToView(pNode, *pTmp->GetAnyEnd());
                if (nTmpEnd <= rPos)
                    continue;
                nTmpStart = m_pFrame->MapModelToView(pNode, pTmp->GetStart());
            }
            else
            {
                pNodeTwoLinesItem = nullptr;
                pNode->GetSwAttrSet().GetItemState(
                            RES_CHRATR_TWO_LINES, true, &pNodeTwoLinesItem);
                nTmpStart = m_pFrame->MapModelToView(pNode, 0);
                nTmpEnd = m_pFrame->MapModelToView(pNode, pNode->Len());
                assert(rPos <= nTmpEnd); // next node must not have smaller index
            }

            if (rPos < nTmpStart)
            {
                // If bOn is false and the next attribute starts later than rPos
                // the winner attribute is interrupted at rPos.
                // If the start of the next attribute is behind the end of
                // the last attribute on the aEnd-stack, this is the endposition
                // on the stack is the end of the 2-line portion.
                if (!bOn || aEnd.back() < nTmpStart)
                    break;
                // At this moment, bOn is true and the next attribute starts
                // behind rPos, so we could move rPos to the next startpoint
                rPos = nTmpStart;
                // We clean up the aEnd-stack, endpositions equal to rPos are
                // superfluous.
                while( !aEnd.empty() && aEnd.back() <= rPos )
                {
                    bOn = !bOn;
                    aEnd.pop_back();
                }
                // If the endstack is empty, we simulate an attribute with
                // state true and endposition rPos
                if( aEnd.empty() )
                {
                    aEnd.push_front( rPos );
                    bOn = true;
                }
            }
            // A ruby attribute stops the 2-line immediately
            if (pTmp && RES_TXTATR_CJK_RUBY == pTmp->Which())
                return pRet;
            if (pTmp ? lcl_Has2Lines(*pTmp, pActiveTwoLinesItem, bTwo)
                     : lcl_Check2Lines(pNodeTwoLinesItem, pActiveTwoLinesItem, bTwo))
            {   // We have an interesting attribute..
                if( bTwo == bOn )
                {   // .. with the same state, so the last attribute could
                    // be continued.
                    if (aEnd.back() < nTmpEnd)
                        aEnd.back() = nTmpEnd;
                }
                else
                {   // .. with a different state.
                    bOn = bTwo;
                    // If this is smaller than the last on the stack, we put
                    // it on the stack. If it has the same endposition, the last
                    // could be removed.
                    if (nTmpEnd < aEnd.back())
                        aEnd.push_back( nTmpEnd );
                    else if( aEnd.size() > 1 )
                        aEnd.pop_back();
                    else
                        aEnd.back() = nTmpEnd;
                }
            }
        }
        if( bOn && !aEnd.empty() )
            rPos = aEnd.back();
        return pRet;
    }
    if (pActiveRotateHint ||
        (pNodeRotateItem && pNodeRotateItem == pActiveRotateItem &&
         rPos < TextFrameIndex(GetText().getLength())))
    {   // The winner is a rotate-attribute,
        // the end of the multiportion depends on the following attributes...
        std::unique_ptr<SwMultiCreator> pRet(new SwMultiCreator);
        pRet->nId = SwMultiCreatorId::Rotate;

        // We note the endpositions of the 2-line attributes in aEnd as stack
        std::deque<TextFrameIndex> aEnd;

        // The bOn flag signs the state of the last 2-line attribute in the
        // aEnd-stack, which could interrupts the winning rotation attribute.
        bool bOn = pNodeTwoLinesItem != nullptr;
        aEnd.push_front(TextFrameIndex(GetText().getLength()));

        // first, search for the start position of the next TWOLINE portion
        // because the ROTATE portion must end there at the latest
        TextFrameIndex n2Start = rPos;
        for (sw::MergedAttrIterMulti iter = iterAtStartOfNode; ; )
        {
            SwTextNode const* pNode(nullptr);
            SwTextAttr const*const pTmp = iter.NextAttr(pNode);
            if (!pNode)
            {
                break;
            }
            assert(startPos.first->GetIndex() <= pNode->GetIndex());
            TextFrameIndex nTmpStart;
            TextFrameIndex nTmpEnd;
            if (pTmp)
            {
                nTmpEnd = m_pFrame->MapModelToView(pNode, *pTmp->GetAnyEnd());
                if (nTmpEnd <= n2Start)
                    continue;
                nTmpStart = m_pFrame->MapModelToView(pNode, pTmp->GetStart());
            }
            else
            {
                pNodeTwoLinesItem = nullptr;
                pNode->GetSwAttrSet().GetItemState(
                            RES_CHRATR_TWO_LINES, true, &pNodeTwoLinesItem);
                nTmpStart = m_pFrame->MapModelToView(pNode, 0);
                nTmpEnd = m_pFrame->MapModelToView(pNode, pNode->Len());
                assert(n2Start <= nTmpEnd); // next node must not have smaller index
            }

            if (n2Start < nTmpStart)
            {
                if (bOn || aEnd.back() < nTmpStart)
                    break;
                n2Start = nTmpStart;
                while( !aEnd.empty() && aEnd.back() <= n2Start )
                {
                    bOn = !bOn;
                    aEnd.pop_back();
                }
                if( aEnd.empty() )
                {
                    aEnd.push_front( n2Start );
                    bOn = false;
                }
            }
            // A ruby attribute stops immediately
            if (pTmp && RES_TXTATR_CJK_RUBY == pTmp->Which())
            {
                bOn = true;
                break;
            }
            const SvxTwoLinesItem* p2Lines = nullptr;
            if (pTmp ? lcl_Has2Lines(*pTmp, p2Lines, bTwo)
                     : lcl_Check2Lines(pNodeTwoLinesItem, p2Lines, bTwo))
            {
                if( bTwo == bOn )
                {
                    if (aEnd.back() < nTmpEnd)
                        aEnd.back() = nTmpEnd;
                }
                else
                {
                    bOn = bTwo;
                    if (nTmpEnd < aEnd.back())
                        aEnd.push_back( nTmpEnd );
                    else if( aEnd.size() > 1 )
                        aEnd.pop_back();
                    else
                        aEnd.back() = nTmpEnd;
                }
            }
        }
        if( !bOn && !aEnd.empty() )
            n2Start = aEnd.back();

        aEnd.clear();

        // now, search for the end of the ROTATE portion, similar to above
        bOn = true;
        if (pActiveRotateHint)
        {
            pRet->pItem = nullptr;
            pRet->pAttr = pActiveRotateHint;
            pRet->nStartOfAttr = m_pFrame->MapModelToView(startPos.first, pRet->pAttr->GetStart());
            if (pNodeRotateItem)
            {
                aEnd.push_front(m_pFrame->MapModelToView(startPos.first, startPos.first->Len()));
                bOn = static_cast<const SvxCharRotateItem*>(pNodeRotateItem)->GetValue() ==
                        pActiveRotateItem->GetValue();
            }
            else
            {
                aEnd.push_front(m_pFrame->MapModelToView(startPos.first, *pRet->pAttr->End()));
            }
        }
        else
        {
            pRet->pItem = pNodeRotateItem;
            pRet->pAttr = nullptr;
            pRet->nStartOfAttr = TextFrameIndex(-1);
            aEnd.push_front(m_pFrame->MapModelToView(startPos.first, startPos.first->Len()));
        }
        for (sw::MergedAttrIterMulti iter = iterAtStartOfNode; ; )
        {
            SwTextNode const* pNode(nullptr);
            SwTextAttr const*const pTmp = iter.NextAttr(pNode);
            if (!pNode)
            {
                break;
            }
            assert(startPos.first->GetIndex() <= pNode->GetIndex());
            TextFrameIndex nTmpStart;
            TextFrameIndex nTmpEnd;
            if (pTmp)
            {
                nTmpEnd = m_pFrame->MapModelToView(pNode, *pTmp->GetAnyEnd());
                if (nTmpEnd <= rPos)
                    continue;
                nTmpStart = m_pFrame->MapModelToView(pNode, pTmp->GetStart());
            }
            else
            {
                pNodeRotateItem = nullptr;
                pNode->GetSwAttrSet().GetItemState(
                            RES_CHRATR_ROTATE, true, &pNodeRotateItem);
                nTmpStart = m_pFrame->MapModelToView(pNode, 0);
                nTmpEnd = m_pFrame->MapModelToView(pNode, pNode->Len());
                assert(rPos <= nTmpEnd); // next node must not have smaller index
            }

            if (rPos < nTmpStart)
            {
                if (!bOn || aEnd.back() < nTmpStart)
                    break;
                rPos = nTmpStart;
                while( !aEnd.empty() && aEnd.back() <= rPos )
                {
                    bOn = !bOn;
                    aEnd.pop_back();
                }
                if( aEnd.empty() )
                {
                    aEnd.push_front( rPos );
                    bOn = true;
                }
            }
            if (pTmp && RES_TXTATR_CJK_RUBY == pTmp->Which())
            {
                bOn = false;
                break;
            }
            // TODO why does this use bTwo, not bRot ???
            if (pTmp ? lcl_HasRotation(*pTmp, pActiveRotateItem, bTwo)
                     : lcl_CheckRotation(pNodeRotateItem, pActiveRotateItem, bTwo))
            {
                if( bTwo == bOn )
                {
                    if (aEnd.back() < nTmpEnd)
                        aEnd.back() = nTmpEnd;
                }
                else
                {
                    bOn = bTwo;
                    if (nTmpEnd < aEnd.back())
                        aEnd.push_back( nTmpEnd );
                    else if( aEnd.size() > 1 )
                        aEnd.pop_back();
                    else
                        aEnd.back() = nTmpEnd;
                }
            }
        }
        if( bOn && !aEnd.empty() )
            rPos = aEnd.back();
        if( rPos > n2Start )
            rPos = n2Start;
        return pRet;
    }
    return nullptr;
}

// A little helper class to manage the spaceadd-arrays of the text adjustment
// during a PaintMultiPortion.
// The constructor prepares the array for the first line of multiportion,
// the SecondLine-function restores the values for the first line and prepares
// the second line.
// The destructor restores the values of the last manipulation.
class SwSpaceManipulator
{
    SwTextPaintInfo& rInfo;
    SwMultiPortion& rMulti;
    std::vector<long>* pOldSpaceAdd;
    sal_uInt16 nOldSpIdx;
    long nSpaceAdd;
    bool bSpaceChg;
    sal_uInt8 nOldDir;
public:
    SwSpaceManipulator( SwTextPaintInfo& rInf, SwMultiPortion& rMult );
    ~SwSpaceManipulator();
    void SecondLine();
    long GetSpaceAdd() const { return nSpaceAdd; }
};

SwSpaceManipulator::SwSpaceManipulator( SwTextPaintInfo& rInf,
                                        SwMultiPortion& rMult )
    : rInfo(rInf)
    , rMulti(rMult)
    , nSpaceAdd(0)
{
    pOldSpaceAdd = rInfo.GetpSpaceAdd();
    nOldSpIdx = rInfo.GetSpaceIdx();
    nOldDir = rInfo.GetDirection();
    rInfo.SetDirection( rMulti.GetDirection() );
    bSpaceChg = false;

    if( rMulti.IsDouble() )
    {
        nSpaceAdd = ( pOldSpaceAdd && !rMulti.HasTabulator() ) ?
                      rInfo.GetSpaceAdd() : 0;
        if( rMulti.GetRoot().IsSpaceAdd() )
        {
            rInfo.SetpSpaceAdd( rMulti.GetRoot().GetpLLSpaceAdd() );
            rInfo.ResetSpaceIdx();
            bSpaceChg = rMulti.ChgSpaceAdd( &rMulti.GetRoot(), nSpaceAdd );
        }
        else if( rMulti.HasTabulator() )
            rInfo.SetpSpaceAdd( nullptr );
    }
    else if ( ! rMulti.IsBidi() )
    {
        rInfo.SetpSpaceAdd( rMulti.GetRoot().GetpLLSpaceAdd() );
        rInfo.ResetSpaceIdx();
    }
}

void SwSpaceManipulator::SecondLine()
{
    if( bSpaceChg )
    {
        rInfo.RemoveFirstSpaceAdd();
        bSpaceChg = false;
    }
    SwLineLayout *pLay = rMulti.GetRoot().GetNext();
    if( pLay->IsSpaceAdd() )
    {
        rInfo.SetpSpaceAdd( pLay->GetpLLSpaceAdd() );
        rInfo.ResetSpaceIdx();
        bSpaceChg = rMulti.ChgSpaceAdd( pLay, nSpaceAdd );
    }
    else
    {
        rInfo.SetpSpaceAdd( (!rMulti.IsDouble() || rMulti.HasTabulator() ) ?
                                nullptr : pOldSpaceAdd );
        rInfo.SetSpaceIdx( nOldSpIdx);
    }
}

SwSpaceManipulator::~SwSpaceManipulator()
{
    if( bSpaceChg )
    {
        rInfo.RemoveFirstSpaceAdd();
        bSpaceChg = false;
    }
    rInfo.SetpSpaceAdd( pOldSpaceAdd );
    rInfo.SetSpaceIdx( nOldSpIdx);
    rInfo.SetDirection( nOldDir );
}

// Manages the paint for a SwMultiPortion.
// External, for the calling function, it seems to be a normal Paint-function,
// internal it is like a SwTextFrame::PaintSwFrame with multiple DrawTextLines
void SwTextPainter::PaintMultiPortion( const SwRect &rPaint,
    SwMultiPortion& rMulti, const SwMultiPortion* pEnvPor )
{
    SwTextGridItem const*const pGrid(GetGridItem(m_pFrame->FindPageFrame()));
    const bool bHasGrid = pGrid && GetInfo().SnapToGrid();
    sal_uInt16 nRubyHeight = 0;
    bool bRubyTop = true;

    if ( bHasGrid && pGrid->IsSquaredMode() )
    {
        nRubyHeight = pGrid->GetRubyHeight();
        bRubyTop = ! pGrid->GetRubyTextBelow();
    }

    // do not allow grid mode for first line in ruby portion
    const bool bRubyInGrid = bHasGrid && rMulti.IsRuby();

    const sal_uInt16 nOldHeight = rMulti.Height();
    const bool bOldGridModeAllowed = GetInfo().SnapToGrid();

    if ( bRubyInGrid )
    {
        GetInfo().SetSnapToGrid( ! bRubyTop );
        if (pGrid->IsSquaredMode())
            rMulti.Height( m_pCurr->Height() );
    }

    SwLayoutModeModifier aLayoutModeModifier( *GetInfo().GetOut() );
    bool bEnvDir = false;
    bool bThisDir = false;
    bool bFrameDir = false;
    if ( rMulti.IsBidi() )
    {
        // these values are needed for the calculation of the x coordinate
        // and the layout mode
        OSL_ENSURE( ! pEnvPor || pEnvPor->IsBidi(),
                "Oh no, I expected a BidiPortion" );
        bFrameDir = GetInfo().GetTextFrame()->IsRightToLeft();
        bEnvDir = pEnvPor ? ((static_cast<const SwBidiPortion*>(pEnvPor)->GetLevel() % 2) != 0) : bFrameDir;
        bThisDir = (static_cast<SwBidiPortion&>(rMulti).GetLevel() % 2) != 0;
    }

#if OSL_DEBUG_LEVEL > 1
    // only paint first level bidi portions
    if( rMulti.Width() > 1 && ! pEnvPor )
        GetInfo().DrawViewOpt( rMulti, PortionType::Field );
#endif

    if ( bRubyInGrid && pGrid->IsSquaredMode() )
        rMulti.Height( nOldHeight );

    // do we have to repaint a post it portion?
    if( GetInfo().OnWin() && rMulti.GetNextPortion() &&
        ! rMulti.GetNextPortion()->Width() )
        rMulti.GetNextPortion()->PrePaint( GetInfo(), &rMulti );

    // old values must be saved and restored at the end
    TextFrameIndex const nOldLen = GetInfo().GetLen();
    const SwTwips nOldX = GetInfo().X();
    const SwTwips nOldY = GetInfo().Y();
    TextFrameIndex const nOldIdx = GetInfo().GetIdx();

    SwSpaceManipulator aManip( GetInfo(), rMulti );

    std::unique_ptr<SwFontSave> pFontSave;
    std::unique_ptr<SwFont> pTmpFnt;

    if( rMulti.IsDouble() )
    {
        pTmpFnt.reset(new SwFont( *GetInfo().GetFont() ));
        if( rMulti.IsDouble() )
        {
            SetPropFont( 50 );
            pTmpFnt->SetProportion( GetPropFont() );
        }
        pFontSave.reset(new SwFontSave( GetInfo(), pTmpFnt.get(), this ));
    }
    else
    {
        pFontSave = nullptr;
        pTmpFnt = nullptr;
    }

    if( rMulti.HasBrackets() )
    {
        TextFrameIndex const nTmpOldIdx = GetInfo().GetIdx();
        GetInfo().SetIdx(static_cast<SwDoubleLinePortion&>(rMulti).GetBrackets()->nStart);
        SeekAndChg( GetInfo() );
        static_cast<SwDoubleLinePortion&>(rMulti).PaintBracket( GetInfo(), 0, true );
        GetInfo().SetIdx( nTmpOldIdx );
    }

    const SwTwips nTmpX = GetInfo().X();

    SwLineLayout* pLay = &rMulti.GetRoot();// the first line of the multiportion
    SwLinePortion* pPor = pLay->GetFirstPortion();//first portion of these line
    SwTwips nOfst = 0;

    // GetInfo().Y() is the baseline from the surrounding line. We must switch
    // this temporary to the baseline of the inner lines of the multiportion.
    if( rMulti.HasRotation() )
    {
        if( rMulti.IsRevers() )
        {
            GetInfo().Y( nOldY - rMulti.GetAscent() );
            nOfst = nTmpX + rMulti.Width();
        }
        else
        {
            GetInfo().Y( nOldY - rMulti.GetAscent() + rMulti.Height() );
            nOfst = nTmpX;
        }
    }
    else if ( rMulti.IsBidi() )
    {
        // does the current bidi portion has the same direction
        // as its environment?
        if ( bEnvDir != bThisDir )
        {
            // different directions, we have to adjust the x coordinate
            SwTwips nMultiWidth = rMulti.Width() +
                    rMulti.CalcSpacing( GetInfo().GetSpaceAdd(), GetInfo() );

            if ( bFrameDir == bThisDir )
                GetInfo().X( GetInfo().X() - nMultiWidth );
            else
                GetInfo().X( GetInfo().X() + nMultiWidth );
        }

        nOfst = nOldY - rMulti.GetAscent();

        // set layout mode
        aLayoutModeModifier.Modify( bThisDir );
    }
    else
        nOfst = nOldY - rMulti.GetAscent();

    bool bRest = pLay->IsRest();
    bool bFirst = true;

    OSL_ENSURE( nullptr == GetInfo().GetUnderFnt() || rMulti.IsBidi(),
            " Only BiDi portions are allowed to use the common underlining font" );

    if ( rMulti.IsRuby() )
        GetInfo().SetRuby( rMulti.OnTop() );

    do
    {
        if ( bHasGrid && pGrid->IsSquaredMode() )
        {
            if( rMulti.HasRotation() )
            {
                const sal_uInt16 nAdjustment = ( pLay->Height() - pPor->Height() ) / 2 +
                                            pPor->GetAscent();
                if( rMulti.IsRevers() )
                    GetInfo().X( nOfst - nAdjustment );
                else
                    GetInfo().X( nOfst + nAdjustment );
            }
            else
            {
                // special treatment for ruby portions in grid mode
                SwTwips nAdjustment = 0;
                if ( rMulti.IsRuby() )
                {
                    if ( bRubyTop != ( pLay == &rMulti.GetRoot() ) )
                        // adjust base text
                        nAdjustment = ( m_pCurr->Height() - nRubyHeight - pPor->Height() ) / 2;
                    else if ( bRubyTop )
                        // adjust upper ruby text
                        nAdjustment = nRubyHeight - pPor->Height();
                    // else adjust lower ruby text
                }

                GetInfo().Y( nOfst + nAdjustment + pPor->GetAscent() );
            }
        }
        else if( rMulti.HasRotation() )
        {
            if( rMulti.IsRevers() )
                GetInfo().X( nOfst - AdjustBaseLine( *pLay, pPor, 0, 0, true ) );
            else
                GetInfo().X( nOfst + AdjustBaseLine( *pLay, pPor ) );
        }
        else if ( rMulti.IsRuby() && rMulti.OnRight() && GetInfo().IsRuby() )
        {
            SwTwips nLineDiff = std::max(( rMulti.GetRoot().Height() - pPor->Width() ) / 2, 0 );
            GetInfo().Y( nOfst + nLineDiff );
            // Draw the ruby text on top of the preserved space.
            GetInfo().X( GetInfo().X() - pPor->Height() );
        }
        else
            GetInfo().Y( nOfst + AdjustBaseLine( *pLay, pPor ) );

        bool bSeeked = true;
        GetInfo().SetLen( pPor->GetLen() );

        if( bRest && pPor->InFieldGrp() && !pPor->GetLen() )
        {
            if( static_cast<SwFieldPortion*>(pPor)->HasFont() )
                 bSeeked = false;
            else
                SeekAndChgBefore( GetInfo() );
        }
        else if( pPor->InTextGrp() || pPor->InFieldGrp() || pPor->InTabGrp() )
            SeekAndChg( GetInfo() );
        else if ( !bFirst && pPor->IsBreakPortion() && GetInfo().GetOpt().IsParagraph() )
        {
            if( GetRedln() )
                SeekAndChg( GetInfo() );
            else
                SeekAndChgBefore( GetInfo() );
        }
        else
            bSeeked = false;

        SwLinePortion *pNext = pPor->GetNextPortion();
        if(GetInfo().OnWin() && pNext && !pNext->Width() )
        {
            if ( !bSeeked )
                SeekAndChg( GetInfo() );
            pNext->PrePaint( GetInfo(), pPor );
        }

        CheckSpecialUnderline( pPor );
        SwUnderlineFont* pUnderLineFnt = GetInfo().GetUnderFnt();
        if ( pUnderLineFnt )
        {
            if ( rMulti.IsDouble() )
                pUnderLineFnt->GetFont().SetProportion( 50 );
            pUnderLineFnt->SetPos( GetInfo().GetPos() );
        }

        if ( rMulti.IsBidi() )
        {
            // we do not allow any rotation inside a bidi portion
            SwFont* pTmpFont = GetInfo().GetFont();
            pTmpFont->SetVertical( 0, GetInfo().GetTextFrame()->IsVertical() );
        }

        if( pPor->IsMultiPortion() && static_cast<SwMultiPortion*>(pPor)->IsBidi() )
        {
            // but we do allow nested bidi portions
            OSL_ENSURE( rMulti.IsBidi(), "Only nesting of bidi portions is allowed" );
            PaintMultiPortion( rPaint, static_cast<SwMultiPortion&>(*pPor), &rMulti );
        }
        else
            pPor->Paint( GetInfo() );

        bFirst &= !pPor->GetLen();
        if( pNext || !pPor->IsMarginPortion() )
            pPor->Move( GetInfo() );

        pPor = pNext;

        // If there's no portion left, we go to the next line
        if( !pPor && pLay->GetNext() )
        {
            pLay = pLay->GetNext();
            pPor = pLay->GetFirstPortion();
            bRest = pLay->IsRest();
            aManip.SecondLine();

            // delete underline font
            delete GetInfo().GetUnderFnt();
            GetInfo().SetUnderFnt( nullptr );

            if( rMulti.HasRotation() )
            {
                if( rMulti.IsRevers() )
                {
                    nOfst += pLay->Height();
                    GetInfo().Y( nOldY - rMulti.GetAscent() );
                }
                else
                {
                    nOfst -= pLay->Height();
                    GetInfo().Y( nOldY - rMulti.GetAscent() + rMulti.Height() );
                }
            }
            else if ( bHasGrid && rMulti.IsRuby() )
            {
                GetInfo().SetSnapToGrid( bRubyTop );
                GetInfo().X( nTmpX );
                if (pGrid->IsSquaredMode() )
                {
                    if ( bRubyTop )
                        nOfst += nRubyHeight;
                    else
                        nOfst += m_pCurr->Height() - nRubyHeight;
                }
                else
                {
                    nOfst += rMulti.GetRoot().Height();
                }
            }
            else if ( rMulti.IsRuby() && rMulti.OnRight() )
            {
                GetInfo().SetDirection( DIR_TOP2BOTTOM );
                GetInfo().SetRuby( true );
            } else
            {
                GetInfo().X( nTmpX );
                // We switch to the baseline of the next inner line
                nOfst += rMulti.GetRoot().Height();
            }
        }
    } while( pPor );

    if ( bRubyInGrid )
        GetInfo().SetSnapToGrid( bOldGridModeAllowed );

    // delete underline font
    if ( ! rMulti.IsBidi() )
    {
        delete GetInfo().GetUnderFnt();
        GetInfo().SetUnderFnt( nullptr );
    }

    GetInfo().SetIdx( nOldIdx );
    GetInfo().Y( nOldY );

    if( rMulti.HasBrackets() )
    {
        TextFrameIndex const nTmpOldIdx = GetInfo().GetIdx();
        GetInfo().SetIdx(static_cast<SwDoubleLinePortion&>(rMulti).GetBrackets()->nStart);
        SeekAndChg( GetInfo() );
        GetInfo().X( nOldX );
        static_cast<SwDoubleLinePortion&>(rMulti).PaintBracket( GetInfo(),
            aManip.GetSpaceAdd(), false );
        GetInfo().SetIdx( nTmpOldIdx );
    }
    // Restore the saved values
    GetInfo().X( nOldX );
    GetInfo().SetLen( nOldLen );
    pFontSave.reset();
    pTmpFnt.reset();
    SetPropFont( 0 );
}

static bool lcl_ExtractFieldFollow( SwLineLayout* pLine, SwLinePortion* &rpField )
{
    SwLinePortion* pLast = pLine;
    rpField = pLine->GetNextPortion();
    while( rpField && !rpField->InFieldGrp() )
    {
        pLast = rpField;
        rpField = rpField->GetNextPortion();
    }
    bool bRet = rpField != nullptr;
    if( bRet )
    {
        if( static_cast<SwFieldPortion*>(rpField)->IsFollow() )
        {
            rpField->Truncate();
            pLast->SetNextPortion( nullptr );
        }
        else
            rpField = nullptr;
    }
    pLine->Truncate();
    return bRet;
}

// If a multi portion completely has to go to the
// next line, this function is called to truncate
// the rest of the remaining multi portion
static void lcl_TruncateMultiPortion( SwMultiPortion& rMulti, SwTextFormatInfo& rInf,
           TextFrameIndex const nStartIdx)
{
    rMulti.GetRoot().Truncate();
    rMulti.GetRoot().SetLen(TextFrameIndex(0));
    rMulti.GetRoot().Width(0);
//  rMulti.CalcSize( *this, aInf );
    if ( rMulti.GetRoot().GetNext() )
    {
        rMulti.GetRoot().GetNext()->Truncate();
        rMulti.GetRoot().GetNext()->SetLen(TextFrameIndex(0));
        rMulti.GetRoot().GetNext()->Width( 0 );
    }
    rMulti.Width( 0 );
    rMulti.SetLen(TextFrameIndex(0));
    rInf.SetIdx( nStartIdx );
}

// Manages the formatting of a SwMultiPortion. External, for the calling
// function, it seems to be a normal Format-function, internal it is like a
// SwTextFrame::Format_ with multiple BuildPortions
bool SwTextFormatter::BuildMultiPortion( SwTextFormatInfo &rInf,
    SwMultiPortion& rMulti )
{
    SwTwips nMaxWidth = rInf.Width();
    SwTwips nOldX = 0;

    if( rMulti.HasBrackets() )
    {
        TextFrameIndex const nOldIdx = rInf.GetIdx();
        rInf.SetIdx( static_cast<SwDoubleLinePortion&>(rMulti).GetBrackets()->nStart );
        SeekAndChg( rInf );
        nOldX = GetInfo().X();
        static_cast<SwDoubleLinePortion&>(rMulti).FormatBrackets( rInf, nMaxWidth );
        rInf.SetIdx( nOldIdx );
    }

    SeekAndChg( rInf );
    std::unique_ptr<SwFontSave> xFontSave;
    if( rMulti.IsDouble() )
    {
        SwFont* pTmpFnt = new SwFont( *rInf.GetFont() );
        if( rMulti.IsDouble() )
        {
            SetPropFont( 50 );
            pTmpFnt->SetProportion( GetPropFont() );
        }
        xFontSave.reset(new SwFontSave(rInf, pTmpFnt, this));
    }

    SwLayoutModeModifier aLayoutModeModifier( *GetInfo().GetOut() );
    if ( rMulti.IsBidi() )
    {
        // set layout mode
        aLayoutModeModifier.Modify( ! rInf.GetTextFrame()->IsRightToLeft() );
    }

    SwTwips nTmpX = 0;

    if( rMulti.HasRotation() )
    {
        // For nMaxWidth we take the height of the body frame.
        // #i25067#: If the current frame is inside a table, we restrict
        // nMaxWidth to the current frame height, unless the frame size
        // attribute is set to variable size:

        // We set nTmpX (which is used for portion calculating) to the
        // current Y value
        const SwPageFrame* pPage = m_pFrame->FindPageFrame();
        OSL_ENSURE( pPage, "No page in frame!");
        const SwLayoutFrame* pUpperFrame = pPage;

        if ( m_pFrame->IsInTab() )
        {
            pUpperFrame = m_pFrame->GetUpper();
            while ( pUpperFrame && !pUpperFrame->IsCellFrame() )
                pUpperFrame = pUpperFrame->GetUpper();
            assert(pUpperFrame); //pFrame is in table but does not have an upper cell frame
            if (!pUpperFrame)
                return false;
            const SwTableLine* pLine = static_cast<const SwRowFrame*>(pUpperFrame->GetUpper())->GetTabLine();
            const SwFormatFrameSize& rFrameFormatSize = pLine->GetFrameFormat()->GetFrameSize();
            if ( ATT_VAR_SIZE == rFrameFormatSize.GetHeightSizeType() )
                pUpperFrame = pPage;
        }
        if ( pUpperFrame == pPage && !m_pFrame->IsInFootnote() )
            pUpperFrame = pPage->FindBodyCont();

        nMaxWidth = pUpperFrame ?
                    ( rInf.GetTextFrame()->IsVertical() ?
                      pUpperFrame->getFramePrintArea().Width() :
                      pUpperFrame->getFramePrintArea().Height() ) :
                    USHRT_MAX;
    }
    else
        nTmpX = rInf.X();

    SwMultiPortion* pOldMulti = pMulti;

    pMulti = &rMulti;
    SwLineLayout *pOldCurr = m_pCurr;
    TextFrameIndex const nOldStart = GetStart();
    SwTwips nMinWidth = nTmpX + 1;
    SwTwips nActWidth = nMaxWidth;
    const TextFrameIndex nStartIdx = rInf.GetIdx();
    TextFrameIndex nMultiLen = rMulti.GetLen();

    SwLinePortion *pFirstRest;
    SwLinePortion *pSecondRest;
    if( rMulti.IsFormatted() )
    {
        if( !lcl_ExtractFieldFollow( &rMulti.GetRoot(), pFirstRest )
            && rMulti.IsDouble() && rMulti.GetRoot().GetNext() )
            lcl_ExtractFieldFollow( rMulti.GetRoot().GetNext(), pFirstRest );
        if( !rMulti.IsDouble() && rMulti.GetRoot().GetNext() )
            lcl_ExtractFieldFollow( rMulti.GetRoot().GetNext(), pSecondRest );
        else
            pSecondRest = nullptr;
    }
    else
    {
        pFirstRest = rMulti.GetRoot().GetNextPortion();
        pSecondRest = rMulti.GetRoot().GetNext() ?
                      rMulti.GetRoot().GetNext()->GetNextPortion() : nullptr;
        if( pFirstRest )
            rMulti.GetRoot().SetNextPortion( nullptr );
        if( pSecondRest )
            rMulti.GetRoot().GetNext()->SetNextPortion( nullptr );
        rMulti.SetFormatted();
        nMultiLen = nMultiLen - rInf.GetIdx();
    }

    // save some values
    const OUString* pOldText = &(rInf.GetText());
    const SwTwips nOldPaintOfst = rInf.GetPaintOfst();
    std::shared_ptr<vcl::TextLayoutCache> const pOldCachedVclData(rInf.GetCachedVclData());
    rInf.SetCachedVclData(nullptr);

    OUString const aMultiStr( rInf.GetText().copy(0, sal_Int32(nMultiLen + rInf.GetIdx())) );
    rInf.SetText( aMultiStr );
    SwTextFormatInfo aInf( rInf, rMulti.GetRoot(), nActWidth );
    // Do we allow break cuts? The FirstMulti-Flag is evaluated during
    // line break determination.
    bool bFirstMulti = rInf.GetIdx() != rInf.GetLineStart();

    SwLinePortion *pNextFirst = nullptr;
    SwLinePortion *pNextSecond = nullptr;
    bool bRet = false;

    SwTextGridItem const*const pGrid(GetGridItem(m_pFrame->FindPageFrame()));
    const bool bHasGrid = pGrid && GRID_LINES_CHARS == pGrid->GetGridType();

    bool bRubyTop = false;

    if ( bHasGrid )
        bRubyTop = ! pGrid->GetRubyTextBelow();

    do
    {
        m_pCurr = &rMulti.GetRoot();
        m_nStart = nStartIdx;
        bRet = false;
        FormatReset( aInf );
        aInf.X( nTmpX );
        aInf.Width( sal_uInt16(nActWidth) );
        aInf.RealWidth( sal_uInt16(nActWidth) );
        aInf.SetFirstMulti( bFirstMulti );
        aInf.SetNumDone( rInf.IsNumDone() );
        aInf.SetFootnoteDone( rInf.IsFootnoteDone() );

        if( pFirstRest )
        {
            OSL_ENSURE( pFirstRest->InFieldGrp(), "BuildMulti: Fieldrest expected");
            SwFieldPortion *pField =
                static_cast<SwFieldPortion*>(pFirstRest)->Clone(
                    static_cast<SwFieldPortion*>(pFirstRest)->GetExp() );
            pField->SetFollow( true );
            aInf.SetRest( pField );
        }
        aInf.SetRuby( rMulti.IsRuby() && rMulti.OnTop() );

        // in grid mode we temporarily have to disable the grid for the ruby line
        const bool bOldGridModeAllowed = GetInfo().SnapToGrid();
        if ( bHasGrid && aInf.IsRuby() && bRubyTop )
            aInf.SetSnapToGrid( false );

        // If there's no more rubytext, then buildportion is forbidden
        if( pFirstRest || !aInf.IsRuby() )
            BuildPortions( aInf );

        aInf.SetSnapToGrid( bOldGridModeAllowed );

        rMulti.CalcSize( *this, aInf );
        m_pCurr->SetRealHeight( m_pCurr->Height() );

        if( rMulti.IsBidi() )
        {
            pNextFirst = aInf.GetRest();
            break;
        }

        if( rMulti.HasRotation() && !rMulti.IsDouble() )
            break;
        // second line has to be formatted
        else if( m_pCurr->GetLen()<nMultiLen || rMulti.IsRuby() || aInf.GetRest())
        {
            TextFrameIndex const nFirstLen = m_pCurr->GetLen();
            delete m_pCurr->GetNext();
            m_pCurr->SetNext( new SwLineLayout() );
            m_pCurr = m_pCurr->GetNext();
            m_nStart = aInf.GetIdx();
            aInf.X( nTmpX );
            SwTextFormatInfo aTmp( aInf, *m_pCurr, nActWidth );
            if( rMulti.IsRuby() )
            {
                aTmp.SetRuby( !rMulti.OnTop() );
                pNextFirst = aInf.GetRest();
                if( pSecondRest )
                {
                    OSL_ENSURE( pSecondRest->InFieldGrp(), "Fieldrest expected");
                    SwFieldPortion *pField = static_cast<SwFieldPortion*>(pSecondRest)->Clone(
                                    static_cast<SwFieldPortion*>(pSecondRest)->GetExp() );
                    pField->SetFollow( true );
                    aTmp.SetRest( pField );
                }
                if( !rMulti.OnTop() && nFirstLen < nMultiLen )
                    bRet = true;
            }
            else
                aTmp.SetRest( aInf.GetRest() );
            aInf.SetRest( nullptr );

            // in grid mode we temporarily have to disable the grid for the ruby line
            if ( bHasGrid && aTmp.IsRuby() && ! bRubyTop )
                aTmp.SetSnapToGrid( false );

            BuildPortions( aTmp );

            const SwLinePortion *pRightPortion = rMulti.OnRight() ?
                                                 rMulti.GetRoot().GetNext()->GetNextPortion() : nullptr;
            if (pRightPortion)
            {
                // The ruby text on the right is vertical.
                // The width and the height are swapped.
                SwTwips nHeight = pRightPortion->Height();
                // Keep room for the ruby text.
                rMulti.GetRoot().FindLastPortion()->AddPrtWidth( nHeight );
            }

            aTmp.SetSnapToGrid( bOldGridModeAllowed );

            rMulti.CalcSize( *this, aInf );
            rMulti.GetRoot().SetRealHeight( rMulti.GetRoot().Height() );
            m_pCurr->SetRealHeight( m_pCurr->Height() );
            if( rMulti.IsRuby() )
            {
                pNextSecond = aTmp.GetRest();
                if( pNextFirst )
                    bRet = true;
            }
            else
                pNextFirst = aTmp.GetRest();
            if( ( !aTmp.IsRuby() && nFirstLen + m_pCurr->GetLen() < nMultiLen )
                || aTmp.GetRest() )
                // our guess for width of multiportion was too small,
                // text did not fit into multiportion
                bRet = true;
        }
        if( rMulti.IsRuby() )
            break;
        if( bRet )
        {
            // our guess for multiportion width was too small,
            // we set min to act
            nMinWidth = nActWidth;
            nActWidth = ( 3 * nMaxWidth + nMinWidth + 3 ) / 4;
            if ( nActWidth == nMaxWidth && rInf.GetLineStart() == rInf.GetIdx() )
            // we have too less space, we must allow break cuts
            // ( the first multi flag is considered during TextPortion::Format_() )
                bFirstMulti = false;
            if( nActWidth <= nMinWidth )
                break;
        }
        else
        {
            // For Solaris, this optimization can causes trouble:
            // Setting this to the portion width ( = rMulti.Width() )
            // can make GetTextBreak inside SwTextGuess::Guess return to small
            // values. Therefore we add some extra twips.
            if( nActWidth > nTmpX + rMulti.Width() + 6 )
                nActWidth = nTmpX + rMulti.Width() + 6;
            nMaxWidth = nActWidth;
            nActWidth = ( 3 * nMaxWidth + nMinWidth + 3 ) / 4;
            if( nActWidth >= nMaxWidth )
                break;
            // we do not allow break cuts during formatting
            bFirstMulti = true;
        }
        delete pNextFirst;
        pNextFirst = nullptr;
    } while ( true );

    pMulti = pOldMulti;

    m_pCurr = pOldCurr;
    m_nStart = nOldStart;
    SetPropFont( 0 );

    rMulti.SetLen( rMulti.GetRoot().GetLen() + ( rMulti.GetRoot().GetNext() ?
        rMulti.GetRoot().GetNext()->GetLen() : TextFrameIndex(0) ) );

    if( rMulti.IsDouble() )
    {
        static_cast<SwDoubleLinePortion&>(rMulti).CalcBlanks( rInf );
        if( static_cast<SwDoubleLinePortion&>(rMulti).GetLineDiff() )
        {
            SwLineLayout* pLine = &rMulti.GetRoot();
            if( static_cast<SwDoubleLinePortion&>(rMulti).GetLineDiff() > 0 )
            {
                rInf.SetIdx( nStartIdx + pLine->GetLen() );
                pLine = pLine->GetNext();
            }
            if( pLine )
            {
                GetInfo().SetMulti( true );

                // If the fourth element bSkipKashida of function CalcNewBlock is true, multiportion will be showed in justification.
                // Kashida (Persian) is a type of justification used in some cursive scripts, particularly Arabic.
                // In contrast to white-space justification, which increases the length of a line of text by expanding spaces between words or individual letters,
                // kashida justification is accomplished by elongating characters at certain chosen points.
                // Kashida justification can be combined with white-space justification to various extents.
                // The default value of bSkipKashida (the 4th parameter passed to 'CalcNewBlock') is false.
                // Only when Adjust is SvxAdjust::Block ( alignment is justify ), multiportion will be showed in justification in new code.
                CalcNewBlock( pLine, nullptr, rMulti.Width(), GetAdjust() != SvxAdjust::Block );

                GetInfo().SetMulti( false );
            }
            rInf.SetIdx( nStartIdx );
        }
        if( static_cast<SwDoubleLinePortion&>(rMulti).GetBrackets() )
        {
            rMulti.Width( rMulti.Width() +
                    static_cast<SwDoubleLinePortion&>(rMulti).BracketWidth() );
            GetInfo().X( nOldX );
        }
    }
    else
    {
        rMulti.ActualizeTabulator();
        if( rMulti.IsRuby() )
        {
            static_cast<SwRubyPortion&>(rMulti).Adjust( rInf );
            static_cast<SwRubyPortion&>(rMulti).CalcRubyOffset();
        }
    }
    if( rMulti.HasRotation() )
    {
        SwTwips nH = rMulti.Width();
        SwTwips nAsc = rMulti.GetAscent() + ( nH - rMulti.Height() )/2;
        if( nAsc > nH )
            nAsc = nH;
        else if( nAsc < 0 )
            nAsc = 0;
        rMulti.Width( rMulti.Height() );
        rMulti.Height( sal_uInt16(nH) );
        rMulti.SetAscent( sal_uInt16(nAsc) );
        bRet = ( rInf.GetPos().X() + rMulti.Width() > rInf.Width() ) &&
                 nStartIdx != rInf.GetLineStart();
    }
    else if ( rMulti.IsBidi() )
    {
        bRet = rMulti.GetLen() < nMultiLen || pNextFirst;
    }

    // line break has to be performed!
    if( bRet )
    {
        OSL_ENSURE( !pNextFirst || pNextFirst->InFieldGrp(),
            "BuildMultiPortion: Surprising restportion, field expected" );
        SwMultiPortion *pTmp;
        if( rMulti.IsDouble() )
            pTmp = new SwDoubleLinePortion( static_cast<SwDoubleLinePortion&>(rMulti),
                                            nMultiLen + rInf.GetIdx() );
        else if( rMulti.IsRuby() )
        {
            OSL_ENSURE( !pNextSecond || pNextSecond->InFieldGrp(),
                "BuildMultiPortion: Surprising restportion, field expected" );

            if ( rInf.GetIdx() == rInf.GetLineStart() )
            {
                // the ruby portion has to be split in two portions
                pTmp = new SwRubyPortion( static_cast<SwRubyPortion&>(rMulti),
                                          nMultiLen + rInf.GetIdx() );

                if( pNextSecond )
                {
                    pTmp->GetRoot().SetNext( new SwLineLayout() );
                    pTmp->GetRoot().GetNext()->SetNextPortion( pNextSecond );
                }
                pTmp->SetFollowField();
            }
            else
            {
                // we try to keep our ruby portion together
                lcl_TruncateMultiPortion( rMulti, rInf, nStartIdx );
                pTmp = nullptr;
            }
        }
        else if( rMulti.HasRotation() )
        {
            // we try to keep our rotated portion together
            lcl_TruncateMultiPortion( rMulti, rInf, nStartIdx );
            pTmp = new SwRotatedPortion( nMultiLen + rInf.GetIdx(),
                                         rMulti.GetDirection() );
        }
        // during a recursion of BuildMultiPortions we may not build
        // a new SwBidiPortion, this would cause a memory leak
        else if( rMulti.IsBidi() && ! pMulti )
        {
            if ( ! rMulti.GetLen() )
                lcl_TruncateMultiPortion( rMulti, rInf, nStartIdx );

            // If there is a HolePortion at the end of the bidi portion,
            // it has to be moved behind the bidi portion. Otherwise
            // the visual cursor travelling gets into trouble.
            SwLineLayout& aRoot = rMulti.GetRoot();
            SwLinePortion* pPor = aRoot.GetFirstPortion();
            while ( pPor )
            {
                if ( pPor->GetNextPortion() && pPor->GetNextPortion()->IsHolePortion() )
                {
                    SwLinePortion* pHolePor = pPor->GetNextPortion();
                    pPor->SetNextPortion( nullptr );
                    aRoot.SetLen( aRoot.GetLen() - pHolePor->GetLen() );
                    rMulti.SetLen( rMulti.GetLen() - pHolePor->GetLen() );
                    rMulti.SetNextPortion( pHolePor );
                    break;
                }
                pPor = pPor->GetNextPortion();
            }

            pTmp = new SwBidiPortion( nMultiLen + rInf.GetIdx(),
                                    static_cast<SwBidiPortion&>(rMulti).GetLevel() );
        }
        else
            pTmp = nullptr;

        if ( ! rMulti.GetLen() && rInf.GetLast() )
        {
            SeekAndChgBefore( rInf );
            rInf.GetLast()->FormatEOL( rInf );
        }

        if( pNextFirst && pTmp )
        {
            pTmp->SetFollowField();
            pTmp->GetRoot().SetNextPortion( pNextFirst );
        }
        else
            // A follow field portion is still waiting. If nobody wants it,
            // we delete it.
            delete pNextFirst;

        rInf.SetRest( pTmp );
    }

    rInf.SetCachedVclData(pOldCachedVclData);
    rInf.SetText( *pOldText );
    rInf.SetPaintOfst( nOldPaintOfst );
    rInf.SetStop( aInf.IsStop() );
    rInf.SetNumDone( true );
    rInf.SetFootnoteDone( true );
    SeekAndChg( rInf );
    delete pFirstRest;
    delete pSecondRest;
    xFontSave.reset();
    return bRet;
}

// When a fieldportion at the end of line breaks and needs a following
// fieldportion in the next line, then the "restportion" of the formatinfo
// has to be set. Normally this happens during the formatting of the first
// part of the fieldportion.
// But sometimes the formatting starts at the line with the following part,
// especially when the following part is on the next page.
// In this case the MakeRestPortion-function has to create the following part.
// The first parameter is the line that contains possibly a first part
// of a field. When the function finds such field part, it creates the right
// restportion. This may be a multiportion, e.g. if the field is surrounded by
// a doubleline- or ruby-portion.
// The second parameter is the start index of the line.
SwLinePortion* SwTextFormatter::MakeRestPortion( const SwLineLayout* pLine,
    TextFrameIndex nPosition)
{
    if( !nPosition )
        return nullptr;
    TextFrameIndex nMultiPos = nPosition - pLine->GetLen();
    const SwMultiPortion *pTmpMulti = nullptr;
    const SwMultiPortion *pHelpMulti = nullptr;
    const SwLinePortion* pPor = pLine->GetFirstPortion();
    SwFieldPortion *pField = nullptr;
    while( pPor )
    {
        if( pPor->GetLen() && !pHelpMulti )
        {
            nMultiPos = nMultiPos + pPor->GetLen();
            pTmpMulti = nullptr;
        }
        if( pPor->InFieldGrp() )
        {
            if( !pHelpMulti )
                pTmpMulti = nullptr;
            pField = const_cast<SwFieldPortion*>(static_cast<const SwFieldPortion*>(pPor));
        }
        else if( pPor->IsMultiPortion() )
        {
            OSL_ENSURE( !pHelpMulti || pHelpMulti->IsBidi(),
                    "Nested multiportions are forbidden." );

            pField = nullptr;
            pTmpMulti = static_cast<const SwMultiPortion*>(pPor);
        }
        pPor = pPor->GetNextPortion();
        // If the last portion is a multi-portion, we enter it
        // and look for a field portion inside.
        // If we are already in a multiportion, we could change to the
        // next line
        if( !pPor && pTmpMulti )
        {
            if( pHelpMulti )
            {   // We're already inside the multiportion, let's take the second
                // line, if we are in a double line portion
                if( !pHelpMulti->IsRuby() )
                    pPor = pHelpMulti->GetRoot().GetNext();
                pTmpMulti = nullptr;
            }
            else
            {   // Now we enter a multiportion, in a ruby portion we take the
                // main line, not the phonetic line, in a doublelineportion we
                // starts with the first line.
                pHelpMulti = pTmpMulti;
                nMultiPos = nMultiPos - pHelpMulti->GetLen();
                if( pHelpMulti->IsRuby() && pHelpMulti->OnTop() )
                    pPor = pHelpMulti->GetRoot().GetNext();
                else
                    pPor = pHelpMulti->GetRoot().GetFirstPortion();
            }
        }
    }
    if( pField && !pField->HasFollow() )
        pField = nullptr;

    SwLinePortion *pRest = nullptr;
    if( pField )
    {
        const SwTextAttr *pHint = GetAttr(nPosition - TextFrameIndex(1));
        if ( pHint
             && ( pHint->Which() == RES_TXTATR_FIELD
                  || pHint->Which() == RES_TXTATR_ANNOTATION ) )
        {
            pRest = NewFieldPortion( GetInfo(), pHint );
            if( pRest->InFieldGrp() )
                static_cast<SwFieldPortion*>(pRest)->TakeNextOffset( pField );
            else
            {
                delete pRest;
                pRest = nullptr;
            }
        }
    }
    if( !pHelpMulti )
        return pRest;

    nPosition = nMultiPos + pHelpMulti->GetLen();
    std::unique_ptr<SwMultiCreator> pCreate = GetInfo().GetMultiCreator( nMultiPos, nullptr );

    if ( !pCreate )
    {
        OSL_ENSURE( !pHelpMulti->GetLen(), "Multiportion without attribute?" );
        if ( nMultiPos )
            --nMultiPos;
        pCreate = GetInfo().GetMultiCreator( --nMultiPos, nullptr );
    }

    if (!pCreate)
        return pRest;

    if( pRest || nMultiPos > nPosition || ( pHelpMulti->IsRuby() &&
        static_cast<const SwRubyPortion*>(pHelpMulti)->GetRubyOffset() < TextFrameIndex(COMPLETE_STRING)))
    {
        SwMultiPortion* pTmp;
        if( pHelpMulti->IsDouble() )
            pTmp = new SwDoubleLinePortion( *pCreate, nMultiPos );
        else if( pHelpMulti->IsBidi() )
            pTmp = new SwBidiPortion( nMultiPos, pCreate->nLevel );
        else if( pHelpMulti->IsRuby() )
        {
            pTmp = new SwRubyPortion( *pCreate, *GetInfo().GetFont(),
                                       m_pFrame->GetDoc().getIDocumentSettingAccess(),
                                       nMultiPos, static_cast<const SwRubyPortion*>(pHelpMulti)->GetRubyOffset(),
                                       GetInfo() );
        }
        else if( pHelpMulti->HasRotation() )
            pTmp = new SwRotatedPortion( nMultiPos, pHelpMulti->GetDirection() );
        else
        {
            return pRest;
        }
        pCreate.reset();
        pTmp->SetFollowField();
        if( pRest )
        {
            SwLineLayout *pLay = &pTmp->GetRoot();
            if( pTmp->IsRuby() && pTmp->OnTop() )
            {
                pLay->SetNext( new SwLineLayout() );
                pLay = pLay->GetNext();
            }
            pLay->SetNextPortion( pRest );
        }
        return pTmp;
    }
    return pRest;
}

// SwTextCursorSave notes the start and current line of a SwTextCursor,
// sets them to the values for GetCursorOfst inside a multiportion
// and restores them in the destructor.
SwTextCursorSave::SwTextCursorSave( SwTextCursor* pCursor,
                                  SwMultiPortion* pMulti,
                                  SwTwips nY,
                                  sal_uInt16& nX,
                                  TextFrameIndex const nCurrStart,
                                  long nSpaceAdd )
{
    pTextCursor = pCursor;
    nStart = pCursor->m_nStart;
    pCursor->m_nStart = nCurrStart;
    pCurr = pCursor->m_pCurr;
    pCursor->m_pCurr = &pMulti->GetRoot();
    while( pCursor->Y() + pCursor->GetLineHeight() < nY &&
        pCursor->Next() )
        ; // nothing
    nWidth = pCursor->m_pCurr->Width();
    nOldProp = pCursor->GetPropFont();

    if ( pMulti->IsDouble() || pMulti->IsBidi() )
    {
        bSpaceChg = pMulti->ChgSpaceAdd( pCursor->m_pCurr, nSpaceAdd );

        TextFrameIndex nSpaceCnt;
        if ( pMulti->IsDouble() )
        {
            pCursor->SetPropFont( 50 );
            nSpaceCnt = static_cast<SwDoubleLinePortion*>(pMulti)->GetSpaceCnt();
        }
        else
        {
            TextFrameIndex const nOldIdx = pCursor->GetInfo().GetIdx();
            pCursor->GetInfo().SetIdx ( nCurrStart );
            nSpaceCnt = static_cast<SwBidiPortion*>(pMulti)->GetSpaceCnt(pCursor->GetInfo());
            pCursor->GetInfo().SetIdx ( nOldIdx );
        }

        if( nSpaceAdd > 0 && !pMulti->HasTabulator() )
            pCursor->m_pCurr->Width( static_cast<sal_uInt16>(nWidth + nSpaceAdd * sal_Int32(nSpaceCnt) / SPACING_PRECISION_FACTOR) );

        // For a BidiPortion we have to calculate the offset from the
        // end of the portion
        if ( nX && pMulti->IsBidi() )
            nX = pCursor->m_pCurr->Width() - nX;
    }
    else
        bSpaceChg = false;
}

SwTextCursorSave::~SwTextCursorSave()
{
    if( bSpaceChg )
        SwDoubleLinePortion::ResetSpaceAdd( pTextCursor->m_pCurr );
    pTextCursor->m_pCurr->Width( nWidth );
    pTextCursor->m_pCurr = pCurr;
    pTextCursor->m_nStart = nStart;
    pTextCursor->SetPropFont( nOldProp );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
