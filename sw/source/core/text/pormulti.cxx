/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <hintids.hxx>

#include <com/sun/star/i18n/ScriptType.hpp>
#include <editeng/twolinesitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <vcl/outdev.hxx>
#include <fmtfld.hxx>
#include <fldbas.hxx>      // SwField
#include <txatbase.hxx>
#include <fmtruby.hxx>  // SwFmtRuby
#include <txtatr.hxx>   // SwTxtRuby
#include <charfmt.hxx>
#include <txtinet.hxx>
#include <fchrfmt.hxx>
#include <layfrm.hxx>       // GetUpper()
#include <SwPortionHandler.hxx>
#include <pormulti.hxx>     // SwMultiPortion
#include <inftxt.hxx>       // SwTxtSizeInfo
#include <itrpaint.hxx>     // SwTxtPainter
#include <viewopt.hxx>      // SwViewOptions
#include <itrform2.hxx>     // SwTxtFormatter
#include <porfld.hxx>       // SwFldPortion
#include <porglue.hxx>
#include <breakit.hxx>
#include <pagefrm.hxx>
#include <rowfrm.hxx>
#include <pagedesc.hxx> // SwPageDesc
#include <tgrditem.hxx>
#include <swtable.hxx>
#include <fmtfsize.hxx>

using namespace ::com::sun::star;
extern sal_Bool IsUnderlineBreak( const SwLinePortion& rPor, const SwFont& rFnt );

/*--------------------------------------------------
 *  class SwMultiPortion
 *
 * A SwMultiPortion is not a simple portion,
 * it's a container, which contains almost a SwLineLayoutPortion.
 * This SwLineLayout could be followed by other textportions via pPortion
 * and by another SwLineLayout via pNext to realize a doubleline portion.
 * --------------------------------------------------*/

SwMultiPortion::~SwMultiPortion()
{
    delete pFldRest;
}

void SwMultiPortion::Paint( const SwTxtPaintInfo & ) const
{
    OSL_FAIL( "Don't try SwMultiPortion::Paint, try SwTxtPainter::PaintMultiPortion" );
}

/*--------------------------------------------------
 * Summarize the internal lines to calculate the (external) size.
 * The internal line has to calculate first.
 * --------------------------------------------------*/

void SwMultiPortion::CalcSize( SwTxtFormatter& rLine, SwTxtFormatInfo &rInf )
{
    Width( 0 );
    Height( 0 );
    SetAscent( 0 );
    SetFlyInCntnt( sal_False );
    SwLineLayout *pLay = &GetRoot();
    do
    {
        pLay->CalcLine( rLine, rInf );
        if( rLine.IsFlyInCntBase() )
            SetFlyInCntnt( sal_True );
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
        Height( Height() + pLay->Height() );
        if( Width() < pLay->Width() )
            Width( pLay->Width() );
        pLay = pLay->GetNext();
    } while ( pLay );
    if( HasBrackets() )
    {
        KSHORT nTmp = ((SwDoubleLinePortion*)this)->GetBrackets()->nHeight;
        if( nTmp > Height() )
        {
            KSHORT nAdd = ( nTmp - Height() ) / 2;
            GetRoot().SetAscent( GetRoot().GetAscent() + nAdd );
            GetRoot().Height( GetRoot().Height() + nAdd );
            Height( nTmp );
        }
        nTmp = ((SwDoubleLinePortion*)this)->GetBrackets()->nAscent;
        if( nTmp > GetAscent() )
            SetAscent( nTmp );
    }
}

long SwMultiPortion::CalcSpacing( long , const SwTxtSizeInfo & ) const
{
    return 0;
}

sal_Bool SwMultiPortion::ChgSpaceAdd( SwLineLayout*, long ) const
{
    return sal_False;
}

/*************************************************************************
 *              virtual SwMultiPortion::HandlePortion()
 *************************************************************************/

void SwMultiPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Text( GetLen(), GetWhichPor() );
}

/*--------------------------------------------------
 * SwMultiPortion::ActualizeTabulator()
 * sets the tabulator-flag, if there's any tabulator-portion inside.
 * --------------------------------------------------*/

void SwMultiPortion::ActualizeTabulator()
{
    SwLinePortion* pPor = GetRoot().GetFirstPortion();
    // First line
    for( bTab1 = bTab2 = sal_False; pPor; pPor = pPor->GetPortion() )
        if( pPor->InTabGrp() )
            SetTab1( sal_True );
    if( GetRoot().GetNext() )
    {
        // Second line
        pPor = GetRoot().GetNext()->GetFirstPortion();
        do
        {
            if( pPor->InTabGrp() )
                SetTab2( sal_True );
            pPor = pPor->GetPortion();
        } while ( pPor );
    }
}

/*--------------------------------------------------
 * SwRotatedPortion::SwRotatedPortion(..)
 * --------------------------------------------------*/

SwRotatedPortion::SwRotatedPortion( const SwMultiCreator& rCreate,
    xub_StrLen nEnd, sal_Bool bRTL ) : SwMultiPortion( nEnd )
{
    const SvxCharRotateItem* pRot = (SvxCharRotateItem*)rCreate.pItem;
    if( !pRot )
    {
        const SwTxtAttr& rAttr = *rCreate.pAttr;
        const SfxPoolItem *const pItem =
                CharFmt::GetItem(rAttr, RES_CHRATR_ROTATE);
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

/*---------------------------------------------------
 * SwBidiPortion::SwBidiPortion(..)
 * --------------------------------------------------*/

SwBidiPortion::SwBidiPortion( xub_StrLen nEnd, sal_uInt8 nLv )
    : SwMultiPortion( nEnd ), nLevel( nLv )
{
    SetBidi();

    if ( nLevel % 2 )
        SetDirection( DIR_RIGHT2LEFT );
    else
        SetDirection( DIR_LEFT2RIGHT );
}


long SwBidiPortion::CalcSpacing( long nSpaceAdd, const SwTxtSizeInfo& rInf ) const
{
    return HasTabulator() ? 0 : GetSpaceCnt(rInf) * nSpaceAdd / SPACING_PRECISION_FACTOR;
}

sal_Bool SwBidiPortion::ChgSpaceAdd( SwLineLayout* pCurr, long nSpaceAdd ) const
{
    sal_Bool bRet = sal_False;
    if( !HasTabulator() && nSpaceAdd > 0 && !pCurr->IsSpaceAdd() )
    {
        pCurr->CreateSpaceAdd();
        pCurr->SetLLSpaceAdd( nSpaceAdd, 0 );
        bRet = sal_True;
    }

    return bRet;
}

xub_StrLen SwBidiPortion::GetSpaceCnt( const SwTxtSizeInfo &rInf ) const
{
    // Calculate number of blanks for justified alignment
    SwLinePortion* pPor = GetRoot().GetFirstPortion();
    xub_StrLen nTmpStart = rInf.GetIdx();
    xub_StrLen nNull = 0;
    xub_StrLen nBlanks;

    for( nBlanks = 0; pPor; pPor = pPor->GetPortion() )
    {
        if( pPor->InTxtGrp() )
            nBlanks = nBlanks + ((SwTxtPortion*)pPor)->GetSpaceCnt( rInf, nNull );
        else if ( pPor->IsMultiPortion() &&
                 ((SwMultiPortion*)pPor)->IsBidi() )
            nBlanks = nBlanks + ((SwBidiPortion*)pPor)->GetSpaceCnt( rInf );

        ((SwTxtSizeInfo &)rInf).SetIdx( rInf.GetIdx() + pPor->GetLen() );
    }
    ((SwTxtSizeInfo &)rInf).SetIdx( nTmpStart );
    return nBlanks;
}

/*--------------------------------------------------
 * SwDoubleLinePortion::SwDoubleLinePortion(..)
 * This constructor is for the continuation of a doubleline portion
 * in the next line.
 * It takes the same brackets and if the original has no content except
 * brackets, these will be deleted.
 * --------------------------------------------------*/

SwDoubleLinePortion::SwDoubleLinePortion( SwDoubleLinePortion& rDouble,
                                          xub_StrLen nEnd ) :
    SwMultiPortion( nEnd ),
    pBracket( 0 )
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

/*--------------------------------------------------
 * SwDoubleLinePortion::SwDoubleLinePortion(..)
 * This constructor uses the textattribut to get the right brackets.
 * The textattribut could be a 2-line-attribute or a character- or
 * internetstyle, which contains the 2-line-attribute.
 * --------------------------------------------------*/

SwDoubleLinePortion::SwDoubleLinePortion( const SwMultiCreator& rCreate,
    xub_StrLen nEnd ) : SwMultiPortion( nEnd ), pBracket( new SwBracket() )
{
    SetDouble();
    const SvxTwoLinesItem* pTwo = (SvxTwoLinesItem*)rCreate.pItem;
    if( pTwo )
        pBracket->nStart = 0;
    else
    {
        const SwTxtAttr& rAttr = *rCreate.pAttr;
        pBracket->nStart = *rAttr.GetStart();

        const SfxPoolItem * const pItem =
            CharFmt::GetItem( rAttr, RES_CHRATR_TWO_LINES );
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
    sal_uInt8 nTmp = SW_SCRIPTS;
    if( pBracket->cPre > 255 )
    {
        String aTxt = OUString(pBracket->cPre);
        nTmp = SwScriptInfo::WhichFont( 0, &aTxt, 0 );
    }
    pBracket->nPreScript = nTmp;
    nTmp = SW_SCRIPTS;
    if( pBracket->cPost > 255 )
    {
        String aTxt = OUString(pBracket->cPost);
        nTmp = SwScriptInfo::WhichFont( 0, &aTxt, 0 );
    }
    pBracket->nPostScript = nTmp;

    if( !pBracket->cPre && !pBracket->cPost )
    {
        delete pBracket;
        pBracket = 0;
    }

    // double line portions have the same direction as the frame directions
    if ( rCreate.nLevel % 2 )
        SetDirection( DIR_RIGHT2LEFT );
    else
        SetDirection( DIR_LEFT2RIGHT );
}


/*--------------------------------------------------
 * SwMultiPortion::PaintBracket paints the wished bracket,
 * if the multiportion has surrounding brackets.
 * The X-position of the SwTxtPaintInfo will be modified:
 * the open bracket sets position behind itself,
 * the close bracket in front of itself.
 * --------------------------------------------------*/

void SwDoubleLinePortion::PaintBracket( SwTxtPaintInfo &rInf,
                                        long nSpaceAdd,
                                        sal_Bool bOpen ) const
{
    sal_Unicode cCh = bOpen ? pBracket->cPre : pBracket->cPost;
    if( !cCh )
        return;
    KSHORT nChWidth = bOpen ? PreWidth() : PostWidth();
    if( !nChWidth )
        return;
    if( !bOpen )
        rInf.X( rInf.X() + Width() - PostWidth() +
            ( nSpaceAdd > 0 ? CalcSpacing( nSpaceAdd, rInf ) : 0 ) );

    SwBlankPortion aBlank( cCh, sal_True );
    aBlank.SetAscent( pBracket->nAscent );
    aBlank.Width( nChWidth );
    aBlank.Height( pBracket->nHeight );
    {
        SwFont* pTmpFnt = new SwFont( *rInf.GetFont() );
        sal_uInt8 nAct = bOpen ? pBracket->nPreScript : pBracket->nPostScript;
        if( SW_SCRIPTS > nAct )
            pTmpFnt->SetActual( nAct );
        pTmpFnt->SetProportion( 100 );
        SwFontSave aSave( rInf, pTmpFnt );
        aBlank.Paint( rInf );
        delete pTmpFnt;
    }
    if( bOpen )
        rInf.X( rInf.X() + PreWidth() );
}

/*--------------------------------------------------
 * SwDoubleLinePortion::SetBrackets creates the bracket-structur
 * and fills it, if not both characters are 0x00.
 * --------------------------------------------------*/

void SwDoubleLinePortion::SetBrackets( const SwDoubleLinePortion& rDouble )
{
    if( rDouble.pBracket )
    {
        pBracket = new SwBracket;
        pBracket->cPre = rDouble.pBracket->cPre;
        pBracket->cPost = rDouble.pBracket->cPost;
        pBracket->nPreScript = rDouble.pBracket->nPreScript;
        pBracket->nPostScript = rDouble.pBracket->nPostScript;
        pBracket->nStart = rDouble.pBracket->nStart;
    }
}

/*--------------------------------------------------
 * SwDoubleLinePortion::FormatBrackets
 * calculates the size of the brackets => pBracket,
 * reduces the nMaxWidth-parameter ( minus bracket-width )
 * and moves the rInf-x-position behind the opening bracket.
 * --------------------------------------------------*/

void SwDoubleLinePortion::FormatBrackets( SwTxtFormatInfo &rInf, SwTwips& nMaxWidth )
{
    nMaxWidth -= rInf.X();
    SwFont* pTmpFnt = new SwFont( *rInf.GetFont() );
    pTmpFnt->SetProportion( 100 );
    pBracket->nAscent = 0;
    pBracket->nHeight = 0;
    if( pBracket->cPre )
    {
        OUString aStr( pBracket->cPre );
        sal_uInt8 nActualScr = pTmpFnt->GetActual();
        if( SW_SCRIPTS > pBracket->nPreScript )
            pTmpFnt->SetActual( pBracket->nPreScript );
        SwFontSave aSave( rInf, pTmpFnt );
        SwPosSize aSize = rInf.GetTxtSize( aStr );
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
        SwFontSave aSave( rInf, pTmpFnt );
        SwPosSize aSize = rInf.GetTxtSize( aStr );
        KSHORT nTmpAsc = rInf.GetAscent();
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

/*--------------------------------------------------
 * SwDoubleLinePortion::CalcBlanks
 * calculates the number of blanks in each line and
 * the difference of the width of the two lines.
 * These results are used from the text adjustment.
 * --------------------------------------------------*/

void SwDoubleLinePortion::CalcBlanks( SwTxtFormatInfo &rInf )
{
    SwLinePortion* pPor = GetRoot().GetFirstPortion();
    xub_StrLen nNull = 0;
    xub_StrLen nStart = rInf.GetIdx();
    SetTab1( sal_False );
    SetTab2( sal_False );
    for( nBlank1 = 0; pPor; pPor = pPor->GetPortion() )
    {
        if( pPor->InTxtGrp() )
            nBlank1 = nBlank1 + ((SwTxtPortion*)pPor)->GetSpaceCnt( rInf, nNull );
        rInf.SetIdx( rInf.GetIdx() + pPor->GetLen() );
        if( pPor->InTabGrp() )
            SetTab1( sal_True );
    }
    nLineDiff = GetRoot().Width();
    if( GetRoot().GetNext() )
    {
        pPor = GetRoot().GetNext()->GetFirstPortion();
        nLineDiff -= GetRoot().GetNext()->Width();
    }
    for( nBlank2 = 0; pPor; pPor = pPor->GetPortion() )
    {
        if( pPor->InTxtGrp() )
            nBlank2 = nBlank2 + ((SwTxtPortion*)pPor)->GetSpaceCnt( rInf, nNull );
        rInf.SetIdx( rInf.GetIdx() + pPor->GetLen() );
        if( pPor->InTabGrp() )
            SetTab2( sal_True );
    }
    rInf.SetIdx( nStart );
}

long SwDoubleLinePortion::CalcSpacing( long nSpaceAdd, const SwTxtSizeInfo & ) const
{
    return HasTabulator() ? 0 : GetSpaceCnt() * nSpaceAdd / SPACING_PRECISION_FACTOR;
}

/*--------------------------------------------------
 * SwDoubleLinePortion::ChangeSpaceAdd(..)
 * merges the spaces for text adjustment from the inner and outer part.
 * Inside the doubleline portion the wider line has no spaceadd-array, the
 * smaller line has such an array to reach width of the wider line.
 * If the surrounding line has text adjustment and the doubleline portion
 * contains no tabulator, it is necessary to create/manipulate the inner
 * space arrays.
 * --------------------------------------------------*/

sal_Bool SwDoubleLinePortion::ChgSpaceAdd( SwLineLayout* pCurr,
                                           long nSpaceAdd ) const
{
    sal_Bool bRet = sal_False;
    if( !HasTabulator() && nSpaceAdd > 0 )
    {
        if( !pCurr->IsSpaceAdd() )
        {
            // The wider line gets the spaceadd from the surrounding line direct
            pCurr->CreateSpaceAdd();
            pCurr->SetLLSpaceAdd( nSpaceAdd, 0 );
            bRet = sal_True;
        }
        else
        {
            xub_StrLen nMyBlank = GetSmallerSpaceCnt();
            xub_StrLen nOther = GetSpaceCnt();
            SwTwips nMultiSpace = pCurr->GetLLSpaceAdd( 0 ) * nMyBlank + nOther * nSpaceAdd;

            if( nMyBlank )
                nMultiSpace /= nMyBlank;

            if( nMultiSpace < KSHRT_MAX * SPACING_PRECISION_FACTOR )
            {
//                pCurr->SetLLSpaceAdd( nMultiSpace, 0 );
                // #i65711# SetLLSpaceAdd replaces the first value,
                // instead we want to insert a new first value:
                std::vector<long>* pVec = pCurr->GetpLLSpaceAdd();
                pVec->insert( pVec->begin(), nMultiSpace );
                bRet = sal_True;
            }
        }
    }
    return bRet;
}
/*--------------------------------------------------
 * SwDoubleLinePortion::ResetSpaceAdd(..)
 * cancels the manipulation from SwDoubleLinePortion::ChangeSpaceAdd(..)
 * --------------------------------------------------*/

void SwDoubleLinePortion::ResetSpaceAdd( SwLineLayout* pCurr )
{
    pCurr->RemoveFirstLLSpaceAdd();;
    if( !pCurr->GetLLSpaceAddCount() )
        pCurr->FinishSpaceAdd();
}

SwDoubleLinePortion::~SwDoubleLinePortion()
{
    delete pBracket;
}

/*--------------------------------------------------
 * SwRubyPortion::SwRubyPortion(..)
 * constructs a ruby portion, i.e. an additional text is displayed
 * beside the main text, e.g. phonetic characters.
 * --------------------------------------------------*/


SwRubyPortion::SwRubyPortion( const SwRubyPortion& rRuby, xub_StrLen nEnd ) :
    SwMultiPortion( nEnd ),
    nRubyOffset( rRuby.GetRubyOffset() ),
    nAdjustment( rRuby.GetAdjustment() )
{
    SetDirection( rRuby.GetDirection() ),
    SetTop( rRuby.OnTop() );
    SetRuby();
}

/*--------------------------------------------------
 * SwRubyPortion::SwRubyPortion(..)
 * constructs a ruby portion, i.e. an additional text is displayed
 * beside the main text, e.g. phonetic characters.
 * --------------------------------------------------*/

SwRubyPortion::SwRubyPortion( const SwMultiCreator& rCreate, const SwFont& rFnt,
                              const IDocumentSettingAccess& rIDocumentSettingAccess,
                              xub_StrLen nEnd, xub_StrLen nOffs,
                              const sal_Bool* pForceRubyPos )
     : SwMultiPortion( nEnd )
{
    SetRuby();
    OSL_ENSURE( SW_MC_RUBY == rCreate.nId, "Ruby expected" );
    OSL_ENSURE( RES_TXTATR_CJK_RUBY == rCreate.pAttr->Which(), "Wrong attribute" );
    const SwFmtRuby& rRuby = rCreate.pAttr->GetRuby();
    nAdjustment = rRuby.GetAdjustment();
    nRubyOffset = nOffs;

    // in grid mode we force the ruby text to the upper or lower line
    if ( pForceRubyPos )
        SetTop( *pForceRubyPos );
    else
        SetTop( ! rRuby.GetPosition() );

    const SwCharFmt* pFmt = ((SwTxtRuby*)rCreate.pAttr)->GetCharFmt();
    SwFont *pRubyFont;
    if( pFmt )
    {
        const SwAttrSet& rSet = pFmt->GetAttrSet();
         pRubyFont = new SwFont( rFnt );
        pRubyFont->SetDiffFnt( &rSet, &rIDocumentSettingAccess );

        // we do not allow a vertical font for the ruby text
        pRubyFont->SetVertical( rFnt.GetOrientation() );
    }
    else
        pRubyFont = NULL;

    String aStr( rRuby.GetText(), nOffs, STRING_LEN );
    SwFldPortion *pFld = new SwFldPortion( aStr, pRubyFont );
    pFld->SetNextOffset( nOffs );
    pFld->SetFollow( sal_True );

    if( OnTop() )
        GetRoot().SetPortion( pFld );
    else
    {
        GetRoot().SetNext( new SwLineLayout() );
        GetRoot().GetNext()->SetPortion( pFld );
    }

    // ruby portions have the same direction as the frame directions
    if ( rCreate.nLevel % 2 )
    {
        // switch right and left ruby adjustment in rtl environment
        if ( 0 == nAdjustment )
            nAdjustment = 2;
        else if ( 2 == nAdjustment )
            nAdjustment = 0;

        SetDirection( DIR_RIGHT2LEFT );
    }
    else
        SetDirection( DIR_LEFT2RIGHT );
}

/*--------------------------------------------------
 * SwRubyPortion::_Adjust(..)
 * In ruby portion there are different alignments for
 * the ruby text and the main text.
 * Left, right, centered and two possibilities of block adjustment
 * The block adjustment is realized by spacing between the characteres,
 * either with a half space or no space in front of the first letter and
 * a half space at the end of the last letter.
 * Notice: the smaller line will be manipulated, normally it's the ruby line,
 * but it could be the main text, too.
 * If there is a tabulator in smaller line, no adjustment is possible.
 * --------------------------------------------------*/

void SwRubyPortion::_Adjust( SwTxtFormatInfo &rInf )
{
    SwTwips nLineDiff = GetRoot().Width() - GetRoot().GetNext()->Width();
    xub_StrLen nOldIdx = rInf.GetIdx();
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
    KSHORT nLeft = 0;   // the space in front of the first letter
    KSHORT nRight = 0;  // the space at the end of the last letter
    sal_uInt16 nSub = 0;
    switch ( nAdjustment )
    {
        case 1: nRight = static_cast<sal_uInt16>(nLineDiff / 2);    // no break
        case 2: nLeft  = static_cast<sal_uInt16>(nLineDiff - nRight); break;
        case 3: nSub   = 1; // no break
        case 4:
        {
            xub_StrLen nCharCnt = 0;
            SwLinePortion *pPor;
            for( pPor = pCurr->GetFirstPortion(); pPor; pPor = pPor->GetPortion() )
            {
                if( pPor->InTxtGrp() )
                    ((SwTxtPortion*)pPor)->GetSpaceCnt( rInf, nCharCnt );
                rInf.SetIdx( rInf.GetIdx() + pPor->GetLen() );
            }
            if( nCharCnt > nSub )
            {
                SwTwips nCalc = nLineDiff / ( nCharCnt - nSub );
                short nTmp;
                if( nCalc < SHRT_MAX )
                    nTmp = -short(nCalc);
                else
                    nTmp = SHRT_MIN;

                pCurr->CreateSpaceAdd( SPACING_PRECISION_FACTOR * nTmp );
                nLineDiff -= nCalc * ( nCharCnt - 1 );
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
        if( !pCurr->GetPortion() )
            pCurr->SetPortion( new SwTxtPortion( *pCurr ) );
        SwMarginPortion *pMarg = new SwMarginPortion( 0 );
        if( nLeft )
        {
            pMarg->AddPrtWidth( nLeft );
            pMarg->SetPortion( pCurr->GetPortion() );
            pCurr->SetPortion( pMarg );
        }
        if( nRight )
        {
            pMarg = new SwMarginPortion( 0 );
            pMarg->AddPrtWidth( nRight );
            pCurr->FindLastPortion()->Append( pMarg );
        }
    }

    pCurr->Width( Width() );
    rInf.SetIdx( nOldIdx );
}

/*--------------------------------------------------
 * CalcRubyOffset()
 * has to change the nRubyOffset, if there's a fieldportion
 * in the phonetic line.
 * The nRubyOffset is the position in the rubystring, where the
 * next SwRubyPortion has start the displaying of the phonetics.
 * --------------------------------------------------*/

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
    const SwFldPortion *pFld = NULL;
    while( pPor )
    {
        if( pPor->InFldGrp() )
            pFld = (SwFldPortion*)pPor;
        pPor = pPor->GetPortion();
    }
    if( pFld )
    {
        if( pFld->HasFollow() )
            nRubyOffset = pFld->GetNextOffset();
        else
            nRubyOffset = STRING_LEN;
    }
}

/*--------------------------------------------------
 * SwTxtSizeInfo::GetMultiCreator(..)
 * If we (e.g. the position rPos) are inside a two-line-attribute or
 * a ruby-attribute, the attribute will be returned in a SwMultiCreator-struct,
 * otherwise the function returns zero.
 * The rPos parameter is set to the end of the multiportion,
 * normally this is the end of the attribute,
 * but sometimes it is the start of another attribute, which finished or
 * interrupts the first attribute.
 * E.g. a ruby portion interrupts a 2-line-attribute, a 2-line-attribute
 * with different brackets interrupts another 2-line-attribute.
 * --------------------------------------------------*/

/*--------------------------------------------------
 * lcl_Has2Lines(..)
 * is a little help function for GetMultiCreator(..)
 * It extracts the 2-line-format from a 2-line-attribute or a character style.
 * The rValue is set to sal_True, if the 2-line-attribute's value is set and
 * no 2-line-format reference is passed. If there is a 2-line-format reference,
 * then the rValue is set only, if the 2-line-attribute's value is set _and_
 * the 2-line-formats has the same brackets.
 * --------------------------------------------------*/

static sal_Bool lcl_Has2Lines( const SwTxtAttr& rAttr, const SvxTwoLinesItem* &rpRef,
    sal_Bool &rValue )
{
    const SfxPoolItem* pItem = CharFmt::GetItem( rAttr, RES_CHRATR_TWO_LINES );
    if( pItem )
    {
        rValue = ((SvxTwoLinesItem*)pItem)->GetValue();
        if( !rpRef )
            rpRef = (SvxTwoLinesItem*)pItem;
        else if( ((SvxTwoLinesItem*)pItem)->GetEndBracket() !=
                    rpRef->GetEndBracket() ||
                    ((SvxTwoLinesItem*)pItem)->GetStartBracket() !=
                    rpRef->GetStartBracket() )
            rValue = sal_False;
        return sal_True;
    }
    return sal_False;
}

/*--------------------------------------------------
 * lcl_HasRotation(..)
 * is a little help function for GetMultiCreator(..)
 * It extracts the charrotation from a charrotate-attribute or a character style.
 * The rValue is set to sal_True, if the charrotate-attribute's value is set and
 * no charrotate-format reference is passed.
 * If there is a charrotate-format reference, then the rValue is set only,
 * if the charrotate-attribute's value is set _and_ identical
 * to the charrotate-format's value.
 * --------------------------------------------------*/

static sal_Bool lcl_HasRotation( const SwTxtAttr& rAttr,
    const SvxCharRotateItem* &rpRef, sal_Bool &rValue )
{
    const SfxPoolItem* pItem = CharFmt::GetItem( rAttr, RES_CHRATR_ROTATE );
    if ( pItem )
    {
        rValue = 0 != ((SvxCharRotateItem*)pItem)->GetValue();
        if( !rpRef )
            rpRef = (SvxCharRotateItem*)pItem;
        else if( ((SvxCharRotateItem*)pItem)->GetValue() !=
                    rpRef->GetValue() )
            rValue = sal_False;
        return sal_True;
    }

    return sal_False;
}

SwMultiCreator* SwTxtSizeInfo::GetMultiCreator( xub_StrLen &rPos,
                                                SwMultiPortion* pMulti ) const
{
    SwScriptInfo& rSI = ((SwParaPortion*)GetParaPortion())->GetScriptInfo();

    // get the last embedding level
    sal_uInt8 nCurrLevel;
    if ( pMulti )
    {
        OSL_ENSURE( pMulti->IsBidi(), "Nested MultiPortion is not BidiPortion" );
        // level associated with bidi-portion;
        nCurrLevel = ((SwBidiPortion*)pMulti)->GetLevel();
    }
    else
        // no nested bidi portion required
        nCurrLevel = GetTxtFrm()->IsRightToLeft() ? 1 : 0;

    // check if there is a field at rPos:
    sal_uInt8 nNextLevel = nCurrLevel;
    sal_Bool bFldBidi = sal_False;

    if ( CH_TXTATR_BREAKWORD == GetChar( rPos ) )
    {
        bFldBidi = sal_True;
/*
        // examining the script of the field text should be sufficient
        // for 99% of all cases
        XubString aTxt = GetTxtFrm()->GetTxtNode()->GetExpandTxt( rPos, 1 );

        if ( pBreakIt->GetBreakIter().is() && aTxt.Len() )
        {
            sal_Bool bFldDir = ( i18n::ScriptType::COMPLEX ==
                                 pBreakIt->GetRealScriptOfText( aTxt, 0 ) );
            sal_Bool bCurrDir = ( 0 != ( nCurrLevel % 2 ) );
            if ( bFldDir != bCurrDir )
            {
                nNextLevel = nCurrLevel + 1;
                bFldBidi = sal_True;
            }
        }*/
    }
    else
        nNextLevel = rSI.DirType( rPos );

    if ( GetTxt().Len() != rPos && nNextLevel > nCurrLevel )
    {
        rPos = bFldBidi ? rPos + 1 : rSI.NextDirChg( rPos, &nCurrLevel );
        if ( STRING_LEN == rPos )
            return NULL;
        SwMultiCreator *pRet = new SwMultiCreator;
        pRet->pItem = NULL;
        pRet->pAttr = NULL;
        pRet->nId = SW_MC_BIDI;
        pRet->nLevel = nCurrLevel + 1;
        return pRet;
    }

    // a bidi portion can only contain other bidi portions
    if ( pMulti )
        return NULL;

    const SvxCharRotateItem* pRotate = NULL;
    const SfxPoolItem* pRotItem;
    if( SFX_ITEM_SET == pFrm->GetTxtNode()->GetSwAttrSet().
        GetItemState( RES_CHRATR_ROTATE, sal_True, &pRotItem ) &&
        ((SvxCharRotateItem*)pRotItem)->GetValue() )
        pRotate = (SvxCharRotateItem*)pRotItem;
    else
        pRotItem = NULL;
    const SvxTwoLinesItem* p2Lines = NULL;
    const SwTxtNode *pLclTxtNode = pFrm->GetTxtNode();
    if( !pLclTxtNode )
        return NULL;
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == pLclTxtNode->GetSwAttrSet().
        GetItemState( RES_CHRATR_TWO_LINES, sal_True, &pItem ) &&
        ((SvxTwoLinesItem*)pItem)->GetValue() )
        p2Lines = (SvxTwoLinesItem*)pItem;
    else
        pItem = NULL;

    const SwpHints *pHints = pLclTxtNode->GetpSwpHints();
    if( !pHints && !p2Lines && !pRotate )
        return NULL;
    const SwTxtAttr *pRuby = NULL;
    sal_Bool bTwo = sal_False;
    sal_Bool bRot = sal_False;
    sal_uInt16 n2Lines = USHRT_MAX;
    sal_uInt16 nRotate = USHRT_MAX;
    sal_uInt16 nCount = pHints ? pHints->Count() : 0;
    sal_uInt16 i;
    for( i = 0; i < nCount; ++i )
    {
        const SwTxtAttr *pTmp = (*pHints)[i];
        xub_StrLen nStart = *pTmp->GetStart();
        if( rPos < nStart )
            break;
        if( *pTmp->GetAnyEnd() > rPos )
        {
            if( RES_TXTATR_CJK_RUBY == pTmp->Which() )
                pRuby = pTmp;
            else
            {
                const SvxCharRotateItem* pRoTmp = NULL;
                if( lcl_HasRotation( *pTmp, pRoTmp, bRot ) )
                {
                    nRotate = bRot ? i : nCount;
                    pRotate = pRoTmp;
                }
                const SvxTwoLinesItem* p2Tmp = NULL;
                if( lcl_Has2Lines( *pTmp, p2Tmp, bTwo ) )
                {
                    n2Lines = bTwo ? i : nCount;
                    p2Lines = p2Tmp;
                }
            }
        }
    }
    if( pRuby )
    {   // The winner is ... a ruby attribute and so
        // the end of the multiportion is the end of the ruby attribute.
        rPos = *pRuby->GetEnd();
        SwMultiCreator *pRet = new SwMultiCreator;
        pRet->pItem = NULL;
        pRet->pAttr = pRuby;
        pRet->nId = SW_MC_RUBY;
        pRet->nLevel = GetTxtFrm()->IsRightToLeft() ? 1 : 0;
        return pRet;
    }
    if( n2Lines < nCount || ( pItem && pItem == p2Lines &&
        rPos < GetTxt().Len() ) )
    {   // The winner is a 2-line-attribute,
        // the end of the multiportion depends on the following attributes...
        SwMultiCreator *pRet = new SwMultiCreator;

        // We note the endpositions of the 2-line attributes in aEnd as stack
        SvXub_StrLens aEnd;

        // The bOn flag signs the state of the last 2-line attribute in the
        // aEnd-stack, it is compatible with the winner-attribute or
        // it interrupts the other attribute.
        sal_Bool bOn = sal_True;

        if( n2Lines < nCount )
        {
            pRet->pItem = NULL;
            pRet->pAttr = (*pHints)[n2Lines];
            aEnd.push_front( *pRet->pAttr->GetEnd() );
            if( pItem )
            {
                aEnd.front() = GetTxt().Len();
                bOn = ((SvxTwoLinesItem*)pItem)->GetEndBracket() ==
                        p2Lines->GetEndBracket() &&
                      ((SvxTwoLinesItem*)pItem)->GetStartBracket() ==
                        p2Lines->GetStartBracket();
            }
        }
        else
        {
            pRet->pItem = pItem;
            pRet->pAttr = NULL;
            aEnd.push_front( GetTxt().Len() );
        }
        pRet->nId = SW_MC_DOUBLE;
        pRet->nLevel = GetTxtFrm()->IsRightToLeft() ? 1 : 0;

        // n2Lines is the index of the last 2-line-attribute, which contains
        // the actual position.
        i = 0;
        // At this moment we know that at position rPos the "winner"-attribute
        // causes a 2-line-portion. The end of the attribute is the end of the
        // portion, if there's no interrupting attribute.
        // There are two kinds of interruptors:
        // - ruby attributes stops the 2-line-attribute, the end of the
        //   multiline is the start of the ruby attribute
        // - 2-line-attributes with value "Off" or with different brackets,
        //   these attributes may interrupt the winner, but they could be
        //   neutralized by another 2-line-attribute starting at the same
        //   position with the same brackets as the winner-attribute.

        // In the following loop rPos is the critical position and it will be
        // evaluated, if at rPos starts a interrupting or a maintaining
        // continuity attribute.
        while( i < nCount )
        {
            const SwTxtAttr *pTmp = (*pHints)[i++];
            if( *pTmp->GetAnyEnd() <= rPos )
                continue;
            if( rPos < *pTmp->GetStart() )
            {
                // If bOn is sal_False and the next attribute starts later than rPos
                // the winner attribute is interrupted at rPos.
                // If the start of the next atribute is behind the end of
                // the last attribute on the aEnd-stack, this is the endposition
                // on the stack is the end of the 2-line portion.
                if( !bOn || aEnd.back() < *pTmp->GetStart() )
                    break;
                // At this moment, bOn is sal_True and the next attribute starts
                // behind rPos, so we could move rPos to the next startpoint
                rPos = *pTmp->GetStart();
                // We clean up the aEnd-stack, endpositions equal to rPos are
                // superfluous.
                while( !aEnd.empty() && aEnd.back() <= rPos )
                {
                    bOn = !bOn;
                    aEnd.pop_back();
                }
                // If the endstack is empty, we simulate an attribute with
                // state sal_True and endposition rPos
                if( aEnd.empty() )
                {
                    aEnd.push_front( rPos );
                    bOn = sal_True;
                }
            }
            // A ruby attribute stops the 2-line immediately
            if( RES_TXTATR_CJK_RUBY == pTmp->Which() )
                return pRet;
            if( lcl_Has2Lines( *pTmp, p2Lines, bTwo ) )
            {   // We have an interesting attribute..
                if( bTwo == bOn )
                {   // .. with the same state, so the last attribute could
                    // be continued.
                    if( aEnd.back() < *pTmp->GetEnd() )
                        aEnd.back() = *pTmp->GetEnd();
                }
                else
                {   // .. with a different state.
                    bOn = bTwo;
                    // If this is smaller than the last on the stack, we put
                    // it on the stack. If it has the same endposition, the last
                    // could be removed.
                    if( aEnd.back() > *pTmp->GetEnd() )
                        aEnd.push_back( *pTmp->GetEnd() );
                    else if( aEnd.size() > 1 )
                        aEnd.pop_back();
                    else
                        aEnd.back() = *pTmp->GetEnd();
                }
            }
        }
        if( bOn && !aEnd.empty() )
            rPos = aEnd.back();
        return pRet;
    }
    if( nRotate < nCount || ( pRotItem && pRotItem == pRotate &&
        rPos < GetTxt().Len() ) )
    {   // The winner is a rotate-attribute,
        // the end of the multiportion depends on the following attributes...
        SwMultiCreator *pRet = new SwMultiCreator;
        pRet->nId = SW_MC_ROTATE;

        // We note the endpositions of the 2-line attributes in aEnd as stack
        SvXub_StrLens aEnd;

        // The bOn flag signs the state of the last 2-line attribute in the
        // aEnd-stack, which could interrupts the winning rotation attribute.
        sal_Bool bOn = pItem ? sal_True : sal_False;
        aEnd.push_front( GetTxt().Len() );
        // n2Lines is the index of the last 2-line-attribute, which contains
        // the actual position.
        i = 0;
        xub_StrLen n2Start = rPos;
        while( i < nCount )
        {
            const SwTxtAttr *pTmp = (*pHints)[i++];
            if( *pTmp->GetAnyEnd() <= n2Start )
                continue;
            if( n2Start < *pTmp->GetStart() )
            {
                if( bOn || aEnd.back() < *pTmp->GetStart() )
                    break;
                n2Start = *pTmp->GetStart();
                while( !aEnd.empty() && aEnd.back() <= n2Start )
                {
                    bOn = !bOn;
                    aEnd.pop_back();
                }
                if( aEnd.empty() )
                {
                    aEnd.push_front( n2Start );
                    bOn = sal_False;
                }
            }
            // A ruby attribute stops immediately
            if( RES_TXTATR_CJK_RUBY == pTmp->Which() )
            {
                bOn = sal_True;
                break;
            }
            p2Lines = NULL;
            if( lcl_Has2Lines( *pTmp, p2Lines, bTwo ) )
            {
                if( bTwo == bOn )
                {
                    if( aEnd.back() < *pTmp->GetEnd() )
                        aEnd.back() = *pTmp->GetEnd();
                }
                else
                {
                    bOn = bTwo;
                    if( aEnd.back() > *pTmp->GetEnd() )
                        aEnd.push_back( *pTmp->GetEnd() );
                    else if( aEnd.size() > 1 )
                        aEnd.pop_back();
                    else
                        aEnd.back() = *pTmp->GetEnd();
                }
            }
        }
        if( !bOn && !aEnd.empty() )
            n2Start = aEnd.back();

        if( !aEnd.empty() )
            aEnd.clear();

        bOn = sal_True;
        if( nRotate < nCount )
        {
            pRet->pItem = NULL;
            pRet->pAttr = (*pHints)[nRotate];
            aEnd.push_front( *pRet->pAttr->GetEnd() );
            if( pRotItem )
            {
                aEnd.front() = GetTxt().Len();
                bOn = ((SvxCharRotateItem*)pRotItem)->GetValue() ==
                        pRotate->GetValue();
            }
        }
        else
        {
            pRet->pItem = pRotItem;
            pRet->pAttr = NULL;
            aEnd.push_front( GetTxt().Len() );
        }
        i = 0;
        while( i < nCount )
        {
            const SwTxtAttr *pTmp = (*pHints)[i++];
            if( *pTmp->GetAnyEnd() <= rPos )
                continue;
            if( rPos < *pTmp->GetStart() )
            {
                if( !bOn || aEnd.back() < *pTmp->GetStart() )
                    break;
                rPos = *pTmp->GetStart();
                while( !aEnd.empty() && aEnd.back() <= rPos )
                {
                    bOn = !bOn;
                    aEnd.pop_back();
                }
                if( aEnd.empty() )
                {
                    aEnd.push_front( rPos );
                    bOn = sal_True;
                }
            }
            if( RES_TXTATR_CJK_RUBY == pTmp->Which() )
            {
                bOn = sal_False;
                break;
            }
            if( lcl_HasRotation( *pTmp, pRotate, bTwo ) )
            {
                if( bTwo == bOn )
                {
                    if( aEnd.back() < *pTmp->GetEnd() )
                        aEnd.back() = *pTmp->GetEnd();
                }
                else
                {
                    bOn = bTwo;
                    if( aEnd.back() > *pTmp->GetEnd() )
                        aEnd.push_back( *pTmp->GetEnd() );
                    else if( aEnd.size() > 1 )
                        aEnd.pop_back();
                    else
                        aEnd.back() = *pTmp->GetEnd();
                }
            }
        }
        if( bOn && !aEnd.empty() )
            rPos = aEnd.back();
        if( rPos > n2Start )
            rPos = n2Start;
        return pRet;
    }
    return NULL;
}

/*--------------------------------------------------
 * SwSpaceManipulator
 * is a little helper class to manage the spaceadd-arrays of the text adjustment
 * during a PaintMultiPortion.
 * The constructor prepares the array for the first line of multiportion,
 * the SecondLine-function restores the values for the first line and prepares
 * the second line.
 * The destructor restores the values of the last manipulation.
 * --------------------------------------------------*/

class SwSpaceManipulator
{
    SwTxtPaintInfo& rInfo;
    SwMultiPortion& rMulti;
    std::vector<long>* pOldSpaceAdd;
    MSHORT nOldSpIdx;
    long nSpaceAdd;
    sal_Bool bSpaceChg  : 1;
    sal_uInt8 nOldDir   : 2;
public:
    SwSpaceManipulator( SwTxtPaintInfo& rInf, SwMultiPortion& rMult );
    ~SwSpaceManipulator();
    void SecondLine();
    inline long GetSpaceAdd() const { return nSpaceAdd; }
};

SwSpaceManipulator::SwSpaceManipulator( SwTxtPaintInfo& rInf,
                                        SwMultiPortion& rMult ) :
         rInfo( rInf ), rMulti( rMult )
{
    pOldSpaceAdd = rInfo.GetpSpaceAdd();
    nOldSpIdx = rInfo.GetSpaceIdx();
    nOldDir = rInfo.GetDirection();
    rInfo.SetDirection( rMulti.GetDirection() );
    bSpaceChg = sal_False;

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
            rInfo.SetpSpaceAdd( NULL );
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
        bSpaceChg = sal_False;
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
                                0 : pOldSpaceAdd );
        rInfo.SetSpaceIdx( nOldSpIdx);
    }
}

SwSpaceManipulator::~SwSpaceManipulator()
{
    if( bSpaceChg )
    {
        rInfo.RemoveFirstSpaceAdd();
        bSpaceChg = sal_False;
    }
    rInfo.SetpSpaceAdd( pOldSpaceAdd );
    rInfo.SetSpaceIdx( nOldSpIdx);
    rInfo.SetDirection( nOldDir );
}

/*--------------------------------------------------
 * SwTxtPainter::PaintMultiPortion manages the paint for a SwMultiPortion.
 * External, for the calling function, it seems to be a normal Paint-function,
 * internal it is like a SwTxtFrm::Paint with multiple DrawTextLines
 * --------------------------------------------------*/

void SwTxtPainter::PaintMultiPortion( const SwRect &rPaint,
    SwMultiPortion& rMulti, const SwMultiPortion* pEnvPor )
{
    GETGRID( pFrm->FindPageFrm() )
    const sal_Bool bHasGrid = pGrid && GetInfo().SnapToGrid();
    sal_uInt16 nRubyHeight = 0;
    sal_Bool bRubyTop = sal_False;

    if ( bHasGrid )
    {
        nRubyHeight = pGrid->GetRubyHeight();
        bRubyTop = ! pGrid->GetRubyTextBelow();
    }

    // do not allow grid mode for first line in ruby portion
    const sal_Bool bRubyInGrid = bHasGrid && rMulti.IsRuby();

    const sal_uInt16 nOldHeight = rMulti.Height();
    const sal_Bool bOldGridModeAllowed = GetInfo().SnapToGrid();

    if ( bRubyInGrid )
    {
        GetInfo().SetSnapToGrid( ! bRubyTop );
        rMulti.Height( pCurr->Height() );
    }

    SwLayoutModeModifier aLayoutModeModifier( *GetInfo().GetOut() );
    sal_uInt8 nEnvDir = 0;
    sal_uInt8 nThisDir = 0;
    sal_uInt8 nFrmDir = 0;
    if ( rMulti.IsBidi() )
    {
        // these values are needed for the calculation of the x coordinate
        // and the layout mode
        OSL_ENSURE( ! pEnvPor || pEnvPor->IsBidi(),
                "Oh no, I expected a BidiPortion" );
        nFrmDir = GetInfo().GetTxtFrm()->IsRightToLeft() ? 1 : 0;
        nEnvDir = pEnvPor ? ((SwBidiPortion*)pEnvPor)->GetLevel() % 2 : nFrmDir;
        nThisDir = ((SwBidiPortion&)rMulti).GetLevel() % 2;
    }

#if OSL_DEBUG_LEVEL > 1
    // only paint first level bidi portions
    if( rMulti.Width() > 1 && ! pEnvPor )
        GetInfo().DrawViewOpt( rMulti, POR_FLD );
#endif

    if ( bRubyInGrid )
        rMulti.Height( nOldHeight );

    // do we have to repaint a post it portion?
    if( GetInfo().OnWin() && rMulti.GetPortion() &&
        ! rMulti.GetPortion()->Width() )
        rMulti.GetPortion()->PrePaint( GetInfo(), &rMulti );

    // old values must be saved and restored at the end
    xub_StrLen nOldLen = GetInfo().GetLen();
    KSHORT nOldX = KSHORT(GetInfo().X());
    long nOldY = GetInfo().Y();
    xub_StrLen nOldIdx = GetInfo().GetIdx();

    SwSpaceManipulator aManip( GetInfo(), rMulti );

    SwFontSave *pFontSave;
    SwFont* pTmpFnt;

    if( rMulti.IsDouble() )
    {
        pTmpFnt = new SwFont( *GetInfo().GetFont() );
        if( rMulti.IsDouble() )
        {
            SetPropFont( 50 );
            pTmpFnt->SetProportion( GetPropFont() );
        }
        pFontSave = new SwFontSave( GetInfo(), pTmpFnt, this );
    }
    else
    {
        pFontSave = NULL;
        pTmpFnt = NULL;
    }

    if( rMulti.HasBrackets() )
    {
        xub_StrLen nTmpOldIdx = GetInfo().GetIdx();
        GetInfo().SetIdx(((SwDoubleLinePortion&)rMulti).GetBrackets()->nStart);
        SeekAndChg( GetInfo() );
        ((SwDoubleLinePortion&)rMulti).PaintBracket( GetInfo(), 0, sal_True );
        GetInfo().SetIdx( nTmpOldIdx );
    }

    KSHORT nTmpX = KSHORT(GetInfo().X());

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
        if ( nEnvDir != nThisDir )
        {
            // different directions, we have to adjust the x coordinate
            SwTwips nMultiWidth = rMulti.Width() +
                    rMulti.CalcSpacing( GetInfo().GetSpaceAdd(), GetInfo() );

            if ( nFrmDir == nThisDir )
                GetInfo().X( GetInfo().X() - nMultiWidth );
            else
                GetInfo().X( GetInfo().X() + nMultiWidth );
        }

        nOfst = nOldY - rMulti.GetAscent();

        // set layout mode
        aLayoutModeModifier.Modify( nThisDir );
    }
    else
        nOfst = nOldY - rMulti.GetAscent();

    sal_Bool bRest = pLay->IsRest();
    sal_Bool bFirst = sal_True;

    OSL_ENSURE( 0 == GetInfo().GetUnderFnt() || rMulti.IsBidi(),
            " Only BiDi portions are allowed to use the common underlining font" );

    do
    {
        if ( bHasGrid )
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
                        nAdjustment = ( pCurr->Height() - nRubyHeight - pPor->Height() ) / 2;
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
                GetInfo().X( nOfst - AdjustBaseLine( *pLay, pPor, 0, 0, sal_True ) );
            else
                GetInfo().X( nOfst + AdjustBaseLine( *pLay, pPor ) );
        }
        else
            GetInfo().Y( nOfst + AdjustBaseLine( *pLay, pPor ) );

        sal_Bool bSeeked = sal_True;
        GetInfo().SetLen( pPor->GetLen() );

        if( bRest && pPor->InFldGrp() && !pPor->GetLen() )
        {
            if( ((SwFldPortion*)pPor)->HasFont() )
                 bSeeked = sal_False;
            else
                SeekAndChgBefore( GetInfo() );
        }
        else if( pPor->InTxtGrp() || pPor->InFldGrp() || pPor->InTabGrp() )
            SeekAndChg( GetInfo() );
        else if ( !bFirst && pPor->IsBreakPortion() && GetInfo().GetOpt().IsParagraph() )
        {
            if( GetRedln() )
                SeekAndChg( GetInfo() );
            else
                SeekAndChgBefore( GetInfo() );
        }
        else
            bSeeked = sal_False;

        SwLinePortion *pNext = pPor->GetPortion();
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
            pTmpFont->SetVertical( 0, GetInfo().GetTxtFrm()->IsVertical() );
        }

        if( pPor->IsMultiPortion() && ((SwMultiPortion*)pPor)->IsBidi() )
        {
            // but we do allow nested bidi portions
            OSL_ENSURE( rMulti.IsBidi(), "Only nesting of bidi portions is allowed" );
            PaintMultiPortion( rPaint, (SwMultiPortion&)*pPor, &rMulti );
        }
        else
            pPor->Paint( GetInfo() );

        if( GetFnt()->IsURL() && pPor->InTxtGrp() )
            GetInfo().NotifyURL( *pPor );

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
            GetInfo().SetUnderFnt( 0 );

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
                GetInfo().X( nTmpX );
                if ( bRubyTop )
                {
                    nOfst += nRubyHeight;
                    GetInfo().SetSnapToGrid( sal_True );
                }
                else
                {
                    nOfst += pCurr->Height() - nRubyHeight;
                    GetInfo().SetSnapToGrid( sal_False );
                }
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
        GetInfo().SetUnderFnt( 0 );
    }

    GetInfo().SetIdx( nOldIdx );
    GetInfo().Y( nOldY );

    if( rMulti.HasBrackets() )
    {
        xub_StrLen nTmpOldIdx = GetInfo().GetIdx();
        GetInfo().SetIdx(((SwDoubleLinePortion&)rMulti).GetBrackets()->nStart);
        SeekAndChg( GetInfo() );
        GetInfo().X( nOldX );
        ((SwDoubleLinePortion&)rMulti).PaintBracket( GetInfo(),
            aManip.GetSpaceAdd(), sal_False );
        GetInfo().SetIdx( nTmpOldIdx );
    }
    // Restore the saved values
    GetInfo().X( nOldX );
    GetInfo().SetLen( nOldLen );
    delete pFontSave;
    delete pTmpFnt;
    SetPropFont( 0 );
}

static sal_Bool lcl_ExtractFieldFollow( SwLineLayout* pLine, SwLinePortion* &rpFld )
{
    SwLinePortion* pLast = pLine;
    rpFld = pLine->GetPortion();
    while( rpFld && !rpFld->InFldGrp() )
    {
        pLast = rpFld;
        rpFld = rpFld->GetPortion();
    }
    sal_Bool bRet = rpFld != 0;
    if( bRet )
    {
        if( ((SwFldPortion*)rpFld)->IsFollow() )
        {
            rpFld->Truncate();
            pLast->SetPortion( NULL );
        }
        else
            rpFld = NULL;
    }
    pLine->Truncate();
    return bRet;
}

/*----------------------------------------------------
 *              lcl_TruncateMultiPortion
 * If a multi portion completely has to go to the
 * next line, this function is called to trunctate
 * the rest of the remaining multi portion
 * --------------------------------------------------*/

static void lcl_TruncateMultiPortion( SwMultiPortion& rMulti, SwTxtFormatInfo& rInf,
                               xub_StrLen nStartIdx )
{
    rMulti.GetRoot().Truncate();
    rMulti.GetRoot().SetLen(0);
    rMulti.GetRoot().Width(0);
//  rMulti.CalcSize( *this, aInf );
    if ( rMulti.GetRoot().GetNext() )
    {
        rMulti.GetRoot().GetNext()->Truncate();
        rMulti.GetRoot().GetNext()->SetLen( 0 );
        rMulti.GetRoot().GetNext()->Width( 0 );
    }
    rMulti.Width( 0 );
    rMulti.SetLen(0);
    rInf.SetIdx( nStartIdx );
}

/*-----------------------------------------------------------------------------
 *              SwTxtFormatter::BuildMultiPortion
 * manages the formatting of a SwMultiPortion. External, for the calling
 * function, it seems to be a normal Format-function, internal it is like a
 * SwTxtFrm::_Format with multiple BuildPortions
 *---------------------------------------------------------------------------*/

sal_Bool SwTxtFormatter::BuildMultiPortion( SwTxtFormatInfo &rInf,
    SwMultiPortion& rMulti )
{
    SwTwips nMaxWidth = rInf.Width();
    KSHORT nOldX = 0;

    if( rMulti.HasBrackets() )
    {
        xub_StrLen nOldIdx = rInf.GetIdx();
        rInf.SetIdx( ((SwDoubleLinePortion&)rMulti).GetBrackets()->nStart );
        SeekAndChg( rInf );
        nOldX = KSHORT(GetInfo().X());
        ((SwDoubleLinePortion&)rMulti).FormatBrackets( rInf, nMaxWidth );
        rInf.SetIdx( nOldIdx );
    }

    SeekAndChg( rInf );
    SwFontSave *pFontSave;
    if( rMulti.IsDouble() )
    {
        SwFont* pTmpFnt = new SwFont( *rInf.GetFont() );
        if( rMulti.IsDouble() )
        {
            SetPropFont( 50 );
            pTmpFnt->SetProportion( GetPropFont() );
        }
        pFontSave = new SwFontSave( rInf, pTmpFnt, this );
    }
    else
        pFontSave = NULL;

    SwLayoutModeModifier aLayoutModeModifier( *GetInfo().GetOut() );
    if ( rMulti.IsBidi() )
    {
        // set layout mode
        aLayoutModeModifier.Modify( ! rInf.GetTxtFrm()->IsRightToLeft() );
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
        const SwPageFrm* pPage = pFrm->FindPageFrm();
        OSL_ENSURE( pPage, "No page in frame!");
        const SwLayoutFrm* pUpperFrm = pPage;

        if ( pFrm->IsInTab() )
        {
            pUpperFrm = pFrm->GetUpper();
            while ( pUpperFrm && !pUpperFrm->IsCellFrm() )
                pUpperFrm = pUpperFrm->GetUpper();
            OSL_ENSURE( pUpperFrm, "pFrm is in table but does not have an upper cell frame" );
            const SwTableLine* pLine = ((SwRowFrm*)pUpperFrm->GetUpper())->GetTabLine();
            const SwFmtFrmSize& rFrmFmtSize = pLine->GetFrmFmt()->GetFrmSize();
            if ( ATT_VAR_SIZE == rFrmFmtSize.GetHeightSizeType() )
                pUpperFrm = pPage;
        }
        if ( pUpperFrm == pPage && !pFrm->IsInFtn() )
            pUpperFrm = pPage->FindBodyCont();

        nMaxWidth = pUpperFrm ?
                    ( rInf.GetTxtFrm()->IsVertical() ?
                      pUpperFrm->Prt().Width() :
                      pUpperFrm->Prt().Height() ) :
                    USHRT_MAX;
    }
    else
        nTmpX = rInf.X();

    SwMultiPortion* pOldMulti = pMulti;

    pMulti = &rMulti;
    SwLineLayout *pOldCurr = pCurr;
    xub_StrLen nOldStart = GetStart();
    SwTwips nMinWidth = nTmpX + 1;
    SwTwips nActWidth = nMaxWidth;
    const xub_StrLen nStartIdx = rInf.GetIdx();
    xub_StrLen nMultiLen = rMulti.GetLen();

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
            pSecondRest = NULL;
    }
    else
    {
        pFirstRest = rMulti.GetRoot().GetPortion();
        pSecondRest = rMulti.GetRoot().GetNext() ?
                      rMulti.GetRoot().GetNext()->GetPortion() : NULL;
        if( pFirstRest )
            rMulti.GetRoot().SetPortion( NULL );
        if( pSecondRest )
            rMulti.GetRoot().GetNext()->SetPortion( NULL );
        rMulti.SetFormatted();
        nMultiLen = nMultiLen - rInf.GetIdx();
    }

    // save some values
    const XubString* pOldTxt = &(rInf.GetTxt());
    const SwTwips nOldPaintOfst = rInf.GetPaintOfst();

    XubString aMultiStr( rInf.GetTxt(), 0, nMultiLen + rInf.GetIdx() );
    rInf.SetTxt( aMultiStr );
    SwTxtFormatInfo aInf( rInf, rMulti.GetRoot(), nActWidth );
    // Do we allow break cuts? The FirstMulti-Flag is evaluated during
    // line break determination.
    sal_Bool bFirstMulti = rInf.GetIdx() != rInf.GetLineStart();

    SwLinePortion *pNextFirst = NULL;
    SwLinePortion *pNextSecond = NULL;
    sal_Bool bRet = sal_False;

    GETGRID( pFrm->FindPageFrm() )
    const sal_Bool bHasGrid = pGrid && GRID_LINES_CHARS == pGrid->GetGridType();

    sal_Bool bRubyTop = sal_False;

    if ( bHasGrid )
        bRubyTop = ! pGrid->GetRubyTextBelow();

    do
    {
        pCurr = &rMulti.GetRoot();
        nStart = nStartIdx;
        bRet = sal_False;
        FormatReset( aInf );
        aInf.X( nTmpX );
        aInf.Width( KSHORT(nActWidth) );
        aInf.RealWidth( KSHORT(nActWidth) );
        aInf.SetFirstMulti( bFirstMulti );
        aInf.SetNumDone( rInf.IsNumDone() );
        aInf.SetFtnDone( rInf.IsFtnDone() );

        if( pFirstRest )
        {
            OSL_ENSURE( pFirstRest->InFldGrp(), "BuildMulti: Fieldrest expected");
            SwFldPortion *pFld =
                ((SwFldPortion*)pFirstRest)->Clone(
                    ((SwFldPortion*)pFirstRest)->GetExp() );
            pFld->SetFollow( sal_True );
            aInf.SetRest( pFld );
        }
        aInf.SetRuby( rMulti.IsRuby() && rMulti.OnTop() );

        // in grid mode we temporarily have to disable the grid for the ruby line
        const sal_Bool bOldGridModeAllowed = GetInfo().SnapToGrid();
        if ( bHasGrid && aInf.IsRuby() && bRubyTop )
            aInf.SetSnapToGrid( sal_False );

        // If there's no more rubytext, then buildportion is forbidden
        if( pFirstRest || !aInf.IsRuby() )
            BuildPortions( aInf );

        aInf.SetSnapToGrid( bOldGridModeAllowed );

        rMulti.CalcSize( *this, aInf );
        pCurr->SetRealHeight( pCurr->Height() );

        if( rMulti.IsBidi() )
        {
            pNextFirst = aInf.GetRest();
            break;
        }

        if( rMulti.HasRotation() && !rMulti.IsDouble() )
            break;
        // second line has to be formatted
        else if( pCurr->GetLen()<nMultiLen || rMulti.IsRuby() || aInf.GetRest())
        {
            xub_StrLen nFirstLen = pCurr->GetLen();
            delete pCurr->GetNext();
            pCurr->SetNext( new SwLineLayout() );
            pCurr = pCurr->GetNext();
            nStart = aInf.GetIdx();
            aInf.X( nTmpX );
            SwTxtFormatInfo aTmp( aInf, *pCurr, nActWidth );
            if( rMulti.IsRuby() )
            {
                aTmp.SetRuby( !rMulti.OnTop() );
                pNextFirst = aInf.GetRest();
                if( pSecondRest )
                {
                    OSL_ENSURE( pSecondRest->InFldGrp(), "Fieldrest expected");
                    SwFldPortion *pFld = ((SwFldPortion*)pSecondRest)->Clone(
                                    ((SwFldPortion*)pSecondRest)->GetExp() );
                    pFld->SetFollow( sal_True );
                    aTmp.SetRest( pFld );
                }
                if( !rMulti.OnTop() && nFirstLen < nMultiLen )
                    bRet = sal_True;
            }
            else
                aTmp.SetRest( aInf.GetRest() );
            aInf.SetRest( NULL );

            // in grid mode we temporarily have to disable the grid for the ruby line
            if ( bHasGrid && aTmp.IsRuby() && ! bRubyTop )
                aTmp.SetSnapToGrid( sal_False );

            BuildPortions( aTmp );

            aTmp.SetSnapToGrid( bOldGridModeAllowed );

            rMulti.CalcSize( *this, aInf );
            rMulti.GetRoot().SetRealHeight( rMulti.GetRoot().Height() );
            pCurr->SetRealHeight( pCurr->Height() );
            if( rMulti.IsRuby() )
            {
                pNextSecond = aTmp.GetRest();
                if( pNextFirst )
                    bRet = sal_True;
            }
            else
                pNextFirst = aTmp.GetRest();
            if( ( !aTmp.IsRuby() && nFirstLen + pCurr->GetLen() < nMultiLen )
                || aTmp.GetRest() )
                // our guess for width of multiportion was too small,
                // text did not fit into multiportion
                bRet = sal_True;
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
            // ( the first multi flag is considered during TxtPortion::_Format() )
                bFirstMulti = sal_False;
            if( nActWidth <= nMinWidth )
                break;
        }
        else
        {
            // For Solaris, this optimisation can causes trouble:
            // Setting this to the portion width ( = rMulti.Width() )
            // can make GetTextBreak inside SwTxtGuess::Guess return to small
            // values. Therefore we add some extra twips.
            if( nActWidth > nTmpX + rMulti.Width() + 6 )
                nActWidth = nTmpX + rMulti.Width() + 6;
            nMaxWidth = nActWidth;
            nActWidth = ( 3 * nMaxWidth + nMinWidth + 3 ) / 4;
            if( nActWidth >= nMaxWidth )
                break;
            // we do not allow break cuts during formatting
            bFirstMulti = sal_True;
        }
        delete pNextFirst;
        pNextFirst = NULL;
    } while ( sal_True );

    pMulti = pOldMulti;

    pCurr = pOldCurr;
    nStart = nOldStart;
      SetPropFont( 0 );

    rMulti.SetLen( rMulti.GetRoot().GetLen() + ( rMulti.GetRoot().GetNext() ?
        rMulti.GetRoot().GetNext()->GetLen() : 0 ) );

    if( rMulti.IsDouble() )
    {
        ((SwDoubleLinePortion&)rMulti).CalcBlanks( rInf );
        if( ((SwDoubleLinePortion&)rMulti).GetLineDiff() )
        {
            SwLineLayout* pLine = &rMulti.GetRoot();
            if( ((SwDoubleLinePortion&)rMulti).GetLineDiff() > 0 )
            {
                rInf.SetIdx( nStartIdx + pLine->GetLen() );
                pLine = pLine->GetNext();
            }
            if( pLine )
            {
                GetInfo().SetMulti( sal_True );
                CalcNewBlock( pLine, NULL, rMulti.Width() );
                GetInfo().SetMulti( sal_False );
            }
            rInf.SetIdx( nStartIdx );
        }
        if( ((SwDoubleLinePortion&)rMulti).GetBrackets() )
        {
            rMulti.Width( rMulti.Width() +
                    ((SwDoubleLinePortion&)rMulti).BracketWidth() );
            GetInfo().X( nOldX );
        }
    }
    else
    {
        rMulti.ActualizeTabulator();
        if( rMulti.IsRuby() )
        {
            ((SwRubyPortion&)rMulti).Adjust( rInf );
            ((SwRubyPortion&)rMulti).CalcRubyOffset();
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
        rMulti.Height( KSHORT(nH) );
        rMulti.SetAscent( KSHORT(nAsc) );
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
        OSL_ENSURE( !pNextFirst || pNextFirst->InFldGrp(),
            "BuildMultiPortion: Surprising restportion, field expected" );
        SwMultiPortion *pTmp;
        if( rMulti.IsDouble() )
            pTmp = new SwDoubleLinePortion( ((SwDoubleLinePortion&)rMulti),
                                            nMultiLen + rInf.GetIdx() );
        else if( rMulti.IsRuby() )
        {
            OSL_ENSURE( !pNextSecond || pNextSecond->InFldGrp(),
                "BuildMultiPortion: Surprising restportion, field expected" );

            if ( rInf.GetIdx() == rInf.GetLineStart() )
            {
                // the ruby portion has to be split in two portions
                pTmp = new SwRubyPortion( ((SwRubyPortion&)rMulti),
                                          nMultiLen + rInf.GetIdx() );

                if( pNextSecond )
                {
                    pTmp->GetRoot().SetNext( new SwLineLayout() );
                    pTmp->GetRoot().GetNext()->SetPortion( pNextSecond );
                }
                pTmp->SetFollowFld();
            }
            else
            {
                // we try to keep our ruby portion together
                lcl_TruncateMultiPortion( rMulti, rInf, nStartIdx );
                pTmp = 0;
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
                if ( pPor->GetPortion() && pPor->GetPortion()->IsHolePortion() )
                {
                    SwLinePortion* pHolePor = pPor->GetPortion();
                    pPor->SetPortion( NULL );
                    aRoot.SetLen( aRoot.GetLen() - pHolePor->GetLen() );
                    rMulti.SetLen( rMulti.GetLen() - pHolePor->GetLen() );
                    rMulti.SetPortion( pHolePor );
                    break;
                }
                pPor = pPor->GetPortion();
            }

            pTmp = new SwBidiPortion( nMultiLen + rInf.GetIdx(),
                                    ((SwBidiPortion&)rMulti).GetLevel() );
        }
        else
            pTmp = NULL;

        if ( ! rMulti.GetLen() && rInf.GetLast() )
        {
            SeekAndChgBefore( rInf );
            rInf.GetLast()->FormatEOL( rInf );
        }

        if( pNextFirst && pTmp )
        {
            pTmp->SetFollowFld();
            pTmp->GetRoot().SetPortion( pNextFirst );
        }
        else
            // A follow field portion is still waiting. If nobody wants it,
            // we delete it.
            delete pNextFirst;

        rInf.SetRest( pTmp );
    }

    rInf.SetTxt( *pOldTxt );
    rInf.SetPaintOfst( nOldPaintOfst );
    rInf.SetStop( aInf.IsStop() );
    rInf.SetNumDone( sal_True );
    rInf.SetFtnDone( sal_True );
    SeekAndChg( rInf );
    delete pFirstRest;
    delete pSecondRest;
    delete pFontSave;
    return bRet;
}

/*--------------------------------------------------
 * SwTxtFormatter::MakeRestPortion(..)
 * When a fieldportion at the end of line breaks and needs a following
 * fieldportion in the next line, then the "restportion" of the formatinfo
 * has to be set. Normally this happens during the formatting of the first
 * part of the fieldportion.
 * But sometimes the formatting starts at the line with the following part,
 * especially when the following part is on the next page.
 * In this case the MakeRestPortion-function has to create the following part.
 * The first parameter is the line that contains possibly a first part
 * of a field. When the function finds such field part, it creates the right
 * restportion. This may be a multiportion, e.g. if the field is surrounded by
 * a doubleline- or ruby-portion.
 * The second parameter is the start index of the line.
 * --------------------------------------------------*/

SwLinePortion* SwTxtFormatter::MakeRestPortion( const SwLineLayout* pLine,
    xub_StrLen nPosition )
{
    if( !nPosition )
        return NULL;
    xub_StrLen nMultiPos = nPosition - pLine->GetLen();
    const SwMultiPortion *pTmpMulti = NULL;
    const SwMultiPortion *pHelpMulti = NULL;
    const SwLinePortion* pPor = pLine->GetFirstPortion();
    SwFldPortion *pFld = NULL;
    while( pPor )
    {
        if( pPor->GetLen() )
        {
            if( !pHelpMulti )
            {
                nMultiPos = nMultiPos + pPor->GetLen();
                pTmpMulti = NULL;
            }
        }
        if( pPor->InFldGrp() )
        {
            if( !pHelpMulti )
                pTmpMulti = NULL;
            pFld = (SwFldPortion*)pPor;
        }
        else if( pPor->IsMultiPortion() )
        {
            OSL_ENSURE( !pHelpMulti || pHelpMulti->IsBidi(),
                    "Nested multiportions are forbidden." );

            pFld = NULL;
            pTmpMulti = (SwMultiPortion*)pPor;
        }
        pPor = pPor->GetPortion();
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
                pTmpMulti = NULL;
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
    if( pFld && !pFld->HasFollow() )
        pFld = NULL;

    SwLinePortion *pRest = NULL;
    if( pFld )
    {
        const SwTxtAttr *pHint = GetAttr( nPosition - 1 );
        if( pHint && pHint->Which() == RES_TXTATR_FIELD )
        {
            pRest = NewFldPortion( GetInfo(), pHint );
            if( pRest->InFldGrp() )
                ((SwFldPortion*)pRest)->TakeNextOffset( pFld );
            else
            {
                delete pRest;
                pRest = NULL;
            }
        }
    }
    if( !pHelpMulti )
        return pRest;

    nPosition = nMultiPos + pHelpMulti->GetLen();
    SwMultiCreator* pCreate = GetInfo().GetMultiCreator( nMultiPos, 0 );

    if ( !pCreate )
    {
        OSL_ENSURE( !pHelpMulti->GetLen(), "Multiportion without attribut?" );
        if ( nMultiPos )
            --nMultiPos;
        pCreate = GetInfo().GetMultiCreator( --nMultiPos, 0 );
    }

    if (!pCreate)
        return pRest;

    if( pRest || nMultiPos > nPosition || ( pHelpMulti->IsRuby() &&
        ((SwRubyPortion*)pHelpMulti)->GetRubyOffset() < STRING_LEN ) )
    {
        SwMultiPortion* pTmp;
        if( pHelpMulti->IsDouble() )
            pTmp = new SwDoubleLinePortion( *pCreate, nMultiPos );
        else if( pHelpMulti->IsBidi() )
            pTmp = new SwBidiPortion( nMultiPos, pCreate->nLevel );
        else if( pHelpMulti->IsRuby() )
        {
            sal_Bool bRubyTop;
            sal_Bool* pRubyPos = 0;

            if ( GetInfo().SnapToGrid() )
            {
                GETGRID( pFrm->FindPageFrm() )
                if ( pGrid )
                {
                    bRubyTop = ! pGrid->GetRubyTextBelow();
                    pRubyPos = &bRubyTop;
                }
            }

            pTmp = new SwRubyPortion( *pCreate, *GetInfo().GetFont(),
                                      *pFrm->GetTxtNode()->getIDocumentSettingAccess(),
                                       nMultiPos, ((SwRubyPortion*)pHelpMulti)->GetRubyOffset(),
                                       pRubyPos );
        }
        else if( pHelpMulti->HasRotation() )
            pTmp = new SwRotatedPortion( nMultiPos, pHelpMulti->GetDirection() );
        else
        {
            delete pCreate;
            return pRest;
        }
        delete pCreate;
        pTmp->SetFollowFld();
        if( pRest )
        {
            SwLineLayout *pLay = &pTmp->GetRoot();
            if( pTmp->IsRuby() && pTmp->OnTop() )
            {
                pLay->SetNext( new SwLineLayout() );
                pLay = pLay->GetNext();
            }
            pLay->SetPortion( pRest );
        }
        return pTmp;
    }
    return pRest;
}



/*--------------------------------------------------
 * SwTxtCursorSave notes the start and current line of a SwTxtCursor,
 * sets them to the values for GetCrsrOfst inside a multiportion
 * and restores them in the destructor.
 * --------------------------------------------------*/

SwTxtCursorSave::SwTxtCursorSave( SwTxtCursor* pTxtCursor,
                                  SwMultiPortion* pMulti,
                                  SwTwips nY,
                                  sal_uInt16& nX,
                                  xub_StrLen nCurrStart,
                                  long nSpaceAdd )
{
    pTxtCrsr = pTxtCursor;
    nStart = pTxtCursor->nStart;
    pTxtCursor->nStart = nCurrStart;
    pCurr = pTxtCursor->pCurr;
    pTxtCursor->pCurr = &pMulti->GetRoot();
    while( pTxtCursor->Y() + pTxtCursor->GetLineHeight() < nY &&
        pTxtCursor->Next() )
        ; // nothing
    nWidth = pTxtCursor->pCurr->Width();
    nOldProp = pTxtCursor->GetPropFont();

    if ( pMulti->IsDouble() || pMulti->IsBidi() )
    {
        bSpaceChg = pMulti->ChgSpaceAdd( pTxtCursor->pCurr, nSpaceAdd );

        sal_uInt16 nSpaceCnt;
        if ( pMulti->IsDouble() )
        {
            pTxtCursor->SetPropFont( 50 );
            nSpaceCnt = ((SwDoubleLinePortion*)pMulti)->GetSpaceCnt();
        }
        else
        {
            const xub_StrLen nOldIdx = pTxtCursor->GetInfo().GetIdx();
            pTxtCursor->GetInfo().SetIdx ( nCurrStart );
            nSpaceCnt = ((SwBidiPortion*)pMulti)->GetSpaceCnt(pTxtCursor->GetInfo());
            pTxtCursor->GetInfo().SetIdx ( nOldIdx );
        }

        if( nSpaceAdd > 0 && !pMulti->HasTabulator() )
            pTxtCursor->pCurr->Width( static_cast<sal_uInt16>(nWidth + nSpaceAdd * nSpaceCnt / SPACING_PRECISION_FACTOR ) );

        // For a BidiPortion we have to calculate the offset from the
        // end of the portion
        if ( nX && pMulti->IsBidi() )
            nX = pTxtCursor->pCurr->Width() - nX;
    }
    else
        bSpaceChg = sal_False;
}

SwTxtCursorSave::~SwTxtCursorSave()
{
    if( bSpaceChg )
        SwDoubleLinePortion::ResetSpaceAdd( pTxtCrsr->pCurr );
    pTxtCrsr->pCurr->Width( KSHORT(nWidth) );
    pTxtCrsr->pCurr = pCurr;
    pTxtCrsr->nStart = nStart;
    pTxtCrsr->SetPropFont( nOldProp );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
