/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <deque>

#include <hintids.hxx>

#include <com/sun/star/i18n/ScriptType.hpp>
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
#include <pormulti.hxx>
#include <inftxt.hxx>
#include <itrpaint.hxx>
#include <viewopt.hxx>
#include <itrform2.hxx>
#include <porfld.hxx>
#include <porglue.hxx>
#include <breakit.hxx>
#include <pagefrm.hxx>
#include <rowfrm.hxx>
#include <pagedesc.hxx>
#include <tgrditem.hxx>
#include <swtable.hxx>
#include <fmtfsize.hxx>

using namespace ::com::sun::star;

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
    SetFlyInCntnt( false );
    SwLineLayout *pLay = &GetRoot();
    do
    {
        pLay->CalcLine( rLine, rInf );
        if( rLine.IsFlyInCntBase() )
            SetFlyInCntnt( true );
        if( IsRuby() && ( OnTop() == ( pLay == &GetRoot() ) ) )
        {
            
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

bool SwMultiPortion::ChgSpaceAdd( SwLineLayout*, long ) const
{
    return false;
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
    
    for( bTab1 = bTab2 = false; pPor; pPor = pPor->GetPortion() )
        if( pPor->InTabGrp() )
            SetTab1( true );
    if( GetRoot().GetNext() )
    {
        
        pPor = GetRoot().GetNext()->GetFirstPortion();
        do
        {
            if( pPor->InTabGrp() )
                SetTab2( true );
            pPor = pPor->GetPortion();
        } while ( pPor );
    }
}

/*--------------------------------------------------
 * SwRotatedPortion::SwRotatedPortion(..)
 * --------------------------------------------------*/

SwRotatedPortion::SwRotatedPortion( const SwMultiCreator& rCreate,
    sal_Int32 nEnd, bool bRTL ) : SwMultiPortion( nEnd )
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

SwBidiPortion::SwBidiPortion( sal_Int32 nEnd, sal_uInt8 nLv )
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

sal_Int32 SwBidiPortion::GetSpaceCnt( const SwTxtSizeInfo &rInf ) const
{
    
    SwLinePortion* pPor = GetRoot().GetFirstPortion();
    sal_Int32 nTmpStart = rInf.GetIdx();
    sal_Int32 nNull = 0;
    sal_Int32 nBlanks;

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

SwDoubleLinePortion::SwDoubleLinePortion(SwDoubleLinePortion& rDouble, sal_Int32 nEnd)
    : SwMultiPortion(nEnd)
    , pBracket(0)
    , nLineDiff(0)
    , nBlank1(0)
    , nBlank2(0)
{
    SetDirection( rDouble.GetDirection() );
    SetDouble();
    if( rDouble.GetBrackets() )
    {
        SetBrackets( rDouble );
        
        
        
        
        
        
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

SwDoubleLinePortion::SwDoubleLinePortion(const SwMultiCreator& rCreate, sal_Int32 nEnd)
    : SwMultiPortion(nEnd)
    , pBracket(new SwBracket())
    , nLineDiff(0)
    , nBlank1(0)
    , nBlank2(0)
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
        OUString aTxt = OUString(pBracket->cPre);
        nTmp = SwScriptInfo::WhichFont( 0, &aTxt, 0 );
    }
    pBracket->nPreScript = nTmp;
    nTmp = SW_SCRIPTS;
    if( pBracket->cPost > 255 )
    {
        OUString aTxt = OUString(pBracket->cPost);
        nTmp = SwScriptInfo::WhichFont( 0, &aTxt, 0 );
    }
    pBracket->nPostScript = nTmp;

    if( !pBracket->cPre && !pBracket->cPost )
    {
        delete pBracket;
        pBracket = 0;
    }

    
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
                                        bool bOpen ) const
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

    SwBlankPortion aBlank( cCh, true );
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
    delete(pTmpFnt);
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
    sal_Int32 nNull = 0;
    sal_Int32 nStart = rInf.GetIdx();
    SetTab1( false );
    SetTab2( false );
    for( nBlank1 = 0; pPor; pPor = pPor->GetPortion() )
    {
        if( pPor->InTxtGrp() )
            nBlank1 = nBlank1 + ((SwTxtPortion*)pPor)->GetSpaceCnt( rInf, nNull );
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
    for( nBlank2 = 0; pPor; pPor = pPor->GetPortion() )
    {
        if( pPor->InTxtGrp() )
            nBlank2 = nBlank2 + ((SwTxtPortion*)pPor)->GetSpaceCnt( rInf, nNull );
        rInf.SetIdx( rInf.GetIdx() + pPor->GetLen() );
        if( pPor->InTabGrp() )
            SetTab2( true );
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

bool SwDoubleLinePortion::ChgSpaceAdd( SwLineLayout* pCurr,
                                           long nSpaceAdd ) const
{
    bool bRet = false;
    if( !HasTabulator() && nSpaceAdd > 0 )
    {
        if( !pCurr->IsSpaceAdd() )
        {
            
            pCurr->CreateSpaceAdd();
            pCurr->SetLLSpaceAdd( nSpaceAdd, 0 );
            bRet = true;
        }
        else
        {
            sal_Int32 nMyBlank = GetSmallerSpaceCnt();
            sal_Int32 nOther = GetSpaceCnt();
            SwTwips nMultiSpace = pCurr->GetLLSpaceAdd( 0 ) * nMyBlank + nOther * nSpaceAdd;

            if( nMyBlank )
                nMultiSpace /= nMyBlank;

            if( nMultiSpace < KSHRT_MAX * SPACING_PRECISION_FACTOR )
            {

                
                
                std::vector<long>* pVec = pCurr->GetpLLSpaceAdd();
                pVec->insert( pVec->begin(), nMultiSpace );
                bRet = true;
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


SwRubyPortion::SwRubyPortion( const SwRubyPortion& rRuby, sal_Int32 nEnd ) :
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
                              sal_Int32 nEnd, sal_Int32 nOffs,
                              const bool* pForceRubyPos )
     : SwMultiPortion( nEnd )
{
    SetRuby();
    OSL_ENSURE( SW_MC_RUBY == rCreate.nId, "Ruby expected" );
    OSL_ENSURE( RES_TXTATR_CJK_RUBY == rCreate.pAttr->Which(), "Wrong attribute" );
    const SwFmtRuby& rRuby = rCreate.pAttr->GetRuby();
    nAdjustment = rRuby.GetAdjustment();
    nRubyOffset = nOffs;

    
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

        
        pRubyFont->SetVertical( rFnt.GetOrientation() );
    }
    else
        pRubyFont = NULL;

    OUString aStr = rRuby.GetText().copy( nOffs );
    SwFldPortion *pFld = new SwFldPortion( aStr, pRubyFont );
    pFld->SetNextOffset( nOffs );
    pFld->SetFollow( true );

    if( OnTop() )
        GetRoot().SetPortion( pFld );
    else
    {
        GetRoot().SetNext( new SwLineLayout() );
        GetRoot().GetNext()->SetPortion( pFld );
    }

    
    if ( rCreate.nLevel % 2 )
    {
        
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
    sal_Int32 nOldIdx = rInf.GetIdx();
    if( !nLineDiff )
        return;
    SwLineLayout *pCurr;
    if( nLineDiff < 0 )
    {   
        if( GetTab1() )
            return;
        pCurr = &GetRoot();
        nLineDiff = -nLineDiff;
    }
    else
    {   
        if( GetTab2() )
            return;
        pCurr = GetRoot().GetNext();
        rInf.SetIdx( nOldIdx + GetRoot().GetLen() );
    }
    KSHORT nLeft = 0;   
    KSHORT nRight = 0;  
    sal_uInt16 nSub = 0;
    switch ( nAdjustment )
    {
        case 1: nRight = static_cast<sal_uInt16>(nLineDiff / 2);    
        case 2: nLeft  = static_cast<sal_uInt16>(nLineDiff - nRight); break;
        case 3: nSub   = 1; 
        case 4:
        {
            sal_Int32 nCharCnt = 0;
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
        if( nLeft )
        {
            SwMarginPortion *pMarg = new SwMarginPortion( 0 );
            pMarg->AddPrtWidth( nLeft );
            pMarg->SetPortion( pCurr->GetPortion() );
            pCurr->SetPortion( pMarg );
        }
        if( nRight )
        {
            SwMarginPortion *pMarg = new SwMarginPortion( 0 );
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
            nRubyOffset = COMPLETE_STRING;
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
 * The rValue is set to true, if the 2-line-attribute's value is set and
 * no 2-line-format reference is passed. If there is a 2-line-format reference,
 * then the rValue is set only, if the 2-line-attribute's value is set _and_
 * the 2-line-formats has the same brackets.
 * --------------------------------------------------*/

static bool lcl_Has2Lines( const SwTxtAttr& rAttr, const SvxTwoLinesItem* &rpRef,
    bool &rValue )
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
            rValue = false;
        return true;
    }
    return false;
}

/*--------------------------------------------------
 * lcl_HasRotation(..)
 * is a little help function for GetMultiCreator(..)
 * It extracts the charrotation from a charrotate-attribute or a character style.
 * The rValue is set to true, if the charrotate-attribute's value is set and
 * no charrotate-format reference is passed.
 * If there is a charrotate-format reference, then the rValue is set only,
 * if the charrotate-attribute's value is set _and_ identical
 * to the charrotate-format's value.
 * --------------------------------------------------*/

static bool lcl_HasRotation( const SwTxtAttr& rAttr,
    const SvxCharRotateItem* &rpRef, bool &rValue )
{
    const SfxPoolItem* pItem = CharFmt::GetItem( rAttr, RES_CHRATR_ROTATE );
    if ( pItem )
    {
        rValue = ((SvxCharRotateItem*)pItem)->GetValue();
        if( !rpRef )
            rpRef = (SvxCharRotateItem*)pItem;
        else if( ((SvxCharRotateItem*)pItem)->GetValue() !=
                    rpRef->GetValue() )
            rValue = false;
        return true;
    }

    return false;
}

SwMultiCreator* SwTxtSizeInfo::GetMultiCreator( sal_Int32 &rPos,
                                                SwMultiPortion* pMulti ) const
{
    SwScriptInfo& rSI = ((SwParaPortion*)GetParaPortion())->GetScriptInfo();

    
    sal_uInt8 nCurrLevel;
    if ( pMulti )
    {
        OSL_ENSURE( pMulti->IsBidi(), "Nested MultiPortion is not BidiPortion" );
        
        nCurrLevel = ((SwBidiPortion*)pMulti)->GetLevel();
    }
    else
        
        nCurrLevel = GetTxtFrm()->IsRightToLeft() ? 1 : 0;

    
    sal_uInt8 nNextLevel = nCurrLevel;
    bool bFldBidi = false;

    if ( rPos < GetTxt().getLength() && CH_TXTATR_BREAKWORD == GetChar( rPos ) )
    {
        bFldBidi = true;
    }
    else
        nNextLevel = rSI.DirType( rPos );

    if ( GetTxt().getLength() != rPos && nNextLevel > nCurrLevel )
    {
        rPos = bFldBidi ? rPos + 1 : rSI.NextDirChg( rPos, &nCurrLevel );
        if ( COMPLETE_STRING == rPos )
            return NULL;
        SwMultiCreator *pRet = new SwMultiCreator;
        pRet->pItem = NULL;
        pRet->pAttr = NULL;
        pRet->nId = SW_MC_BIDI;
        pRet->nLevel = nCurrLevel + 1;
        return pRet;
    }

    
    if ( pMulti )
        return NULL;

    const SvxCharRotateItem* pRotate = NULL;
    const SfxPoolItem* pRotItem;
    if( SFX_ITEM_SET == m_pFrm->GetTxtNode()->GetSwAttrSet().
        GetItemState( RES_CHRATR_ROTATE, true, &pRotItem ) &&
        ((SvxCharRotateItem*)pRotItem)->GetValue() )
        pRotate = (SvxCharRotateItem*)pRotItem;
    else
        pRotItem = NULL;
    const SvxTwoLinesItem* p2Lines = NULL;
    const SwTxtNode *pLclTxtNode = m_pFrm->GetTxtNode();
    if( !pLclTxtNode )
        return NULL;
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == pLclTxtNode->GetSwAttrSet().
        GetItemState( RES_CHRATR_TWO_LINES, true, &pItem ) &&
        ((SvxTwoLinesItem*)pItem)->GetValue() )
        p2Lines = (SvxTwoLinesItem*)pItem;
    else
        pItem = NULL;

    const SwpHints *pHints = pLclTxtNode->GetpSwpHints();
    if( !pHints && !p2Lines && !pRotate )
        return NULL;
    const SwTxtAttr *pRuby = NULL;
    bool bTwo = false;
    bool bRot = false;
    sal_uInt16 n2Lines = USHRT_MAX;
    sal_uInt16 nRotate = USHRT_MAX;
    sal_uInt16 nCount = pHints ? pHints->Count() : 0;
    sal_uInt16 i;
    for( i = 0; i < nCount; ++i )
    {
        const SwTxtAttr *pTmp = (*pHints)[i];
        sal_Int32 nStart = *pTmp->GetStart();
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
    {   
        
        rPos = *pRuby->End();
        SwMultiCreator *pRet = new SwMultiCreator;
        pRet->pItem = NULL;
        pRet->pAttr = pRuby;
        pRet->nId = SW_MC_RUBY;
        pRet->nLevel = GetTxtFrm()->IsRightToLeft() ? 1 : 0;
        return pRet;
    }
    if( n2Lines < nCount || ( pItem && pItem == p2Lines &&
        rPos < GetTxt().getLength() ) )
    {   
        
        SwMultiCreator *pRet = new SwMultiCreator;

        
        std::deque< sal_Int32 > aEnd;

        
        
        
        bool bOn = true;

        if( n2Lines < nCount )
        {
            pRet->pItem = NULL;
            pRet->pAttr = (*pHints)[n2Lines];
            aEnd.push_front( *pRet->pAttr->End() );
            if( pItem )
            {
                aEnd.front() = GetTxt().getLength();
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
            aEnd.push_front( GetTxt().getLength() );
        }
        pRet->nId = SW_MC_DOUBLE;
        pRet->nLevel = GetTxtFrm()->IsRightToLeft() ? 1 : 0;

        
        
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
                    bOn = true;
                }
            }
            
            if( RES_TXTATR_CJK_RUBY == pTmp->Which() )
                return pRet;
            if( lcl_Has2Lines( *pTmp, p2Lines, bTwo ) )
            {   
                if( bTwo == bOn )
                {   
                    
                    if( aEnd.back() < *pTmp->End() )
                        aEnd.back() = *pTmp->End();
                }
                else
                {   
                    bOn = bTwo;
                    
                    
                    
                    if( aEnd.back() > *pTmp->End() )
                        aEnd.push_back( *pTmp->End() );
                    else if( aEnd.size() > 1 )
                        aEnd.pop_back();
                    else
                        aEnd.back() = *pTmp->End();
                }
            }
        }
        if( bOn && !aEnd.empty() )
            rPos = aEnd.back();
        return pRet;
    }
    if( nRotate < nCount || ( pRotItem && pRotItem == pRotate &&
        rPos < GetTxt().getLength() ) )
    {   
        
        SwMultiCreator *pRet = new SwMultiCreator;
        pRet->nId = SW_MC_ROTATE;

        
        std::deque< sal_Int32 > aEnd;

        
        
        bool bOn = pItem;
        aEnd.push_front( GetTxt().getLength() );
        
        
        i = 0;
        sal_Int32 n2Start = rPos;
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
                    bOn = false;
                }
            }
            
            if( RES_TXTATR_CJK_RUBY == pTmp->Which() )
            {
                bOn = true;
                break;
            }
            p2Lines = NULL;
            if( lcl_Has2Lines( *pTmp, p2Lines, bTwo ) )
            {
                if( bTwo == bOn )
                {
                    if( aEnd.back() < *pTmp->End() )
                        aEnd.back() = *pTmp->End();
                }
                else
                {
                    bOn = bTwo;
                    if( aEnd.back() > *pTmp->End() )
                        aEnd.push_back( *pTmp->End() );
                    else if( aEnd.size() > 1 )
                        aEnd.pop_back();
                    else
                        aEnd.back() = *pTmp->End();
                }
            }
        }
        if( !bOn && !aEnd.empty() )
            n2Start = aEnd.back();

        if( !aEnd.empty() )
            aEnd.clear();

        bOn = true;
        if( nRotate < nCount )
        {
            pRet->pItem = NULL;
            pRet->pAttr = (*pHints)[nRotate];
            aEnd.push_front( *pRet->pAttr->End() );
            if( pRotItem )
            {
                aEnd.front() = GetTxt().getLength();
                bOn = ((SvxCharRotateItem*)pRotItem)->GetValue() ==
                        pRotate->GetValue();
            }
        }
        else
        {
            pRet->pItem = pRotItem;
            pRet->pAttr = NULL;
            aEnd.push_front( GetTxt().getLength() );
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
                    bOn = true;
                }
            }
            if( RES_TXTATR_CJK_RUBY == pTmp->Which() )
            {
                bOn = false;
                break;
            }
            if( lcl_HasRotation( *pTmp, pRotate, bTwo ) )
            {
                if( bTwo == bOn )
                {
                    if( aEnd.back() < *pTmp->End() )
                        aEnd.back() = *pTmp->End();
                }
                else
                {
                    bOn = bTwo;
                    if( aEnd.back() > *pTmp->End() )
                        aEnd.push_back( *pTmp->End() );
                    else if( aEnd.size() > 1 )
                        aEnd.pop_back();
                    else
                        aEnd.back() = *pTmp->End();
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
    bool bSpaceChg;
    sal_uInt8 nOldDir;
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
                                0 : pOldSpaceAdd );
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

/*--------------------------------------------------
 * SwTxtPainter::PaintMultiPortion manages the paint for a SwMultiPortion.
 * External, for the calling function, it seems to be a normal Paint-function,
 * internal it is like a SwTxtFrm::Paint with multiple DrawTextLines
 * --------------------------------------------------*/

void SwTxtPainter::PaintMultiPortion( const SwRect &rPaint,
    SwMultiPortion& rMulti, const SwMultiPortion* pEnvPor )
{
    GETGRID( pFrm->FindPageFrm() )
    const bool bHasGrid = pGrid && GetInfo().SnapToGrid();
    sal_uInt16 nRubyHeight = 0;
    bool bRubyTop = false;

    if ( bHasGrid )
    {
        nRubyHeight = pGrid->GetRubyHeight();
        bRubyTop = ! pGrid->GetRubyTextBelow();
    }

    
    const bool bRubyInGrid = bHasGrid && rMulti.IsRuby();

    const sal_uInt16 nOldHeight = rMulti.Height();
    const bool bOldGridModeAllowed = GetInfo().SnapToGrid();

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
        
        
        OSL_ENSURE( ! pEnvPor || pEnvPor->IsBidi(),
                "Oh no, I expected a BidiPortion" );
        nFrmDir = GetInfo().GetTxtFrm()->IsRightToLeft() ? 1 : 0;
        nEnvDir = pEnvPor ? ((SwBidiPortion*)pEnvPor)->GetLevel() % 2 : nFrmDir;
        nThisDir = ((SwBidiPortion&)rMulti).GetLevel() % 2;
    }

#if OSL_DEBUG_LEVEL > 1
    
    if( rMulti.Width() > 1 && ! pEnvPor )
        GetInfo().DrawViewOpt( rMulti, POR_FLD );
#endif

    if ( bRubyInGrid )
        rMulti.Height( nOldHeight );

    
    if( GetInfo().OnWin() && rMulti.GetPortion() &&
        ! rMulti.GetPortion()->Width() )
        rMulti.GetPortion()->PrePaint( GetInfo(), &rMulti );

    
    sal_Int32 nOldLen = GetInfo().GetLen();
    KSHORT nOldX = KSHORT(GetInfo().X());
    long nOldY = GetInfo().Y();
    sal_Int32 nOldIdx = GetInfo().GetIdx();

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
        sal_Int32 nTmpOldIdx = GetInfo().GetIdx();
        GetInfo().SetIdx(((SwDoubleLinePortion&)rMulti).GetBrackets()->nStart);
        SeekAndChg( GetInfo() );
        ((SwDoubleLinePortion&)rMulti).PaintBracket( GetInfo(), 0, true );
        GetInfo().SetIdx( nTmpOldIdx );
    }

    KSHORT nTmpX = KSHORT(GetInfo().X());

    SwLineLayout* pLay = &rMulti.GetRoot();
    SwLinePortion* pPor = pLay->GetFirstPortion();
    SwTwips nOfst = 0;

    
    
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
        
        
        if ( nEnvDir != nThisDir )
        {
            
            SwTwips nMultiWidth = rMulti.Width() +
                    rMulti.CalcSpacing( GetInfo().GetSpaceAdd(), GetInfo() );

            if ( nFrmDir == nThisDir )
                GetInfo().X( GetInfo().X() - nMultiWidth );
            else
                GetInfo().X( GetInfo().X() + nMultiWidth );
        }

        nOfst = nOldY - rMulti.GetAscent();

        
        aLayoutModeModifier.Modify( nThisDir );
    }
    else
        nOfst = nOldY - rMulti.GetAscent();

    bool bRest = pLay->IsRest();
    bool bFirst = true;

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
                
                SwTwips nAdjustment = 0;
                if ( rMulti.IsRuby() )
                {
                    if ( bRubyTop != ( pLay == &rMulti.GetRoot() ) )
                        
                        nAdjustment = ( pCurr->Height() - nRubyHeight - pPor->Height() ) / 2;
                    else if ( bRubyTop )
                        
                        nAdjustment = nRubyHeight - pPor->Height();
                    
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
        else
            GetInfo().Y( nOfst + AdjustBaseLine( *pLay, pPor ) );

        bool bSeeked = true;
        GetInfo().SetLen( pPor->GetLen() );

        if( bRest && pPor->InFldGrp() && !pPor->GetLen() )
        {
            if( ((SwFldPortion*)pPor)->HasFont() )
                 bSeeked = false;
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
            bSeeked = false;

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
            
            SwFont* pTmpFont = GetInfo().GetFont();
            pTmpFont->SetVertical( 0, GetInfo().GetTxtFrm()->IsVertical() );
        }

        if( pPor->IsMultiPortion() && ((SwMultiPortion*)pPor)->IsBidi() )
        {
            
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

        
        if( !pPor && pLay->GetNext() )
        {
            pLay = pLay->GetNext();
            pPor = pLay->GetFirstPortion();
            bRest = pLay->IsRest();
            aManip.SecondLine();

            
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
                    GetInfo().SetSnapToGrid( true );
                }
                else
                {
                    nOfst += pCurr->Height() - nRubyHeight;
                    GetInfo().SetSnapToGrid( false );
                }
            } else
            {
                GetInfo().X( nTmpX );
                
                nOfst += rMulti.GetRoot().Height();
            }
        }
    } while( pPor );

    if ( bRubyInGrid )
        GetInfo().SetSnapToGrid( bOldGridModeAllowed );

    
    if ( ! rMulti.IsBidi() )
    {
        delete GetInfo().GetUnderFnt();
        GetInfo().SetUnderFnt( 0 );
    }

    GetInfo().SetIdx( nOldIdx );
    GetInfo().Y( nOldY );

    if( rMulti.HasBrackets() )
    {
        sal_Int32 nTmpOldIdx = GetInfo().GetIdx();
        GetInfo().SetIdx(((SwDoubleLinePortion&)rMulti).GetBrackets()->nStart);
        SeekAndChg( GetInfo() );
        GetInfo().X( nOldX );
        ((SwDoubleLinePortion&)rMulti).PaintBracket( GetInfo(),
            aManip.GetSpaceAdd(), false );
        GetInfo().SetIdx( nTmpOldIdx );
    }
    
    GetInfo().X( nOldX );
    GetInfo().SetLen( nOldLen );
    delete pFontSave;
    delete pTmpFnt;
    SetPropFont( 0 );
}

static bool lcl_ExtractFieldFollow( SwLineLayout* pLine, SwLinePortion* &rpFld )
{
    SwLinePortion* pLast = pLine;
    rpFld = pLine->GetPortion();
    while( rpFld && !rpFld->InFldGrp() )
    {
        pLast = rpFld;
        rpFld = rpFld->GetPortion();
    }
    bool bRet = rpFld != 0;
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
                               sal_Int32 nStartIdx )
{
    rMulti.GetRoot().Truncate();
    rMulti.GetRoot().SetLen(0);
    rMulti.GetRoot().Width(0);

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

bool SwTxtFormatter::BuildMultiPortion( SwTxtFormatInfo &rInf,
    SwMultiPortion& rMulti )
{
    SwTwips nMaxWidth = rInf.Width();
    KSHORT nOldX = 0;

    if( rMulti.HasBrackets() )
    {
        sal_Int32 nOldIdx = rInf.GetIdx();
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
        
        aLayoutModeModifier.Modify( ! rInf.GetTxtFrm()->IsRightToLeft() );
    }

    SwTwips nTmpX = 0;

    if( rMulti.HasRotation() )
    {
        
        
        
        

        
        
        const SwPageFrm* pPage = pFrm->FindPageFrm();
        OSL_ENSURE( pPage, "No page in frame!");
        const SwLayoutFrm* pUpperFrm = pPage;

        if ( pFrm->IsInTab() )
        {
            pUpperFrm = pFrm->GetUpper();
            while ( pUpperFrm && !pUpperFrm->IsCellFrm() )
                pUpperFrm = pUpperFrm->GetUpper();
            assert(pUpperFrm); 
            if (!pUpperFrm)
                return false;
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
    sal_Int32 nOldStart = GetStart();
    SwTwips nMinWidth = nTmpX + 1;
    SwTwips nActWidth = nMaxWidth;
    const sal_Int32 nStartIdx = rInf.GetIdx();
    sal_Int32 nMultiLen = rMulti.GetLen();

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

    
    const OUString* pOldTxt = &(rInf.GetTxt());
    const SwTwips nOldPaintOfst = rInf.GetPaintOfst();

    OUString const aMultiStr( rInf.GetTxt().copy(0, nMultiLen + rInf.GetIdx()) );
    rInf.SetTxt( aMultiStr );
    SwTxtFormatInfo aInf( rInf, rMulti.GetRoot(), nActWidth );
    
    
    bool bFirstMulti = rInf.GetIdx() != rInf.GetLineStart();

    SwLinePortion *pNextFirst = NULL;
    SwLinePortion *pNextSecond = NULL;
    bool bRet = false;

    GETGRID( pFrm->FindPageFrm() )
    const bool bHasGrid = pGrid && GRID_LINES_CHARS == pGrid->GetGridType();

    bool bRubyTop = false;

    if ( bHasGrid )
        bRubyTop = ! pGrid->GetRubyTextBelow();

    do
    {
        pCurr = &rMulti.GetRoot();
        nStart = nStartIdx;
        bRet = false;
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
            pFld->SetFollow( true );
            aInf.SetRest( pFld );
        }
        aInf.SetRuby( rMulti.IsRuby() && rMulti.OnTop() );

        
        const bool bOldGridModeAllowed = GetInfo().SnapToGrid();
        if ( bHasGrid && aInf.IsRuby() && bRubyTop )
            aInf.SetSnapToGrid( false );

        
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
        
        else if( pCurr->GetLen()<nMultiLen || rMulti.IsRuby() || aInf.GetRest())
        {
            sal_Int32 nFirstLen = pCurr->GetLen();
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
                    pFld->SetFollow( true );
                    aTmp.SetRest( pFld );
                }
                if( !rMulti.OnTop() && nFirstLen < nMultiLen )
                    bRet = true;
            }
            else
                aTmp.SetRest( aInf.GetRest() );
            aInf.SetRest( NULL );

            
            if ( bHasGrid && aTmp.IsRuby() && ! bRubyTop )
                aTmp.SetSnapToGrid( false );

            BuildPortions( aTmp );

            aTmp.SetSnapToGrid( bOldGridModeAllowed );

            rMulti.CalcSize( *this, aInf );
            rMulti.GetRoot().SetRealHeight( rMulti.GetRoot().Height() );
            pCurr->SetRealHeight( pCurr->Height() );
            if( rMulti.IsRuby() )
            {
                pNextSecond = aTmp.GetRest();
                if( pNextFirst )
                    bRet = true;
            }
            else
                pNextFirst = aTmp.GetRest();
            if( ( !aTmp.IsRuby() && nFirstLen + pCurr->GetLen() < nMultiLen )
                || aTmp.GetRest() )
                
                
                bRet = true;
        }
        if( rMulti.IsRuby() )
            break;
        if( bRet )
        {
            
            
            nMinWidth = nActWidth;
            nActWidth = ( 3 * nMaxWidth + nMinWidth + 3 ) / 4;
            if ( nActWidth == nMaxWidth && rInf.GetLineStart() == rInf.GetIdx() )
            
            
                bFirstMulti = false;
            if( nActWidth <= nMinWidth )
                break;
        }
        else
        {
            
            
            
            
            if( nActWidth > nTmpX + rMulti.Width() + 6 )
                nActWidth = nTmpX + rMulti.Width() + 6;
            nMaxWidth = nActWidth;
            nActWidth = ( 3 * nMaxWidth + nMinWidth + 3 ) / 4;
            if( nActWidth >= nMaxWidth )
                break;
            
            bFirstMulti = true;
        }
        delete pNextFirst;
        pNextFirst = NULL;
    } while ( true );

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
                GetInfo().SetMulti( true );

                
                
                
                
                
                
                
                CalcNewBlock( pLine, NULL, rMulti.Width(), GetAdjust() != SVX_ADJUST_BLOCK );

                GetInfo().SetMulti( false );
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
                
                lcl_TruncateMultiPortion( rMulti, rInf, nStartIdx );
                pTmp = 0;
            }
        }
        else if( rMulti.HasRotation() )
        {
            
            lcl_TruncateMultiPortion( rMulti, rInf, nStartIdx );
            pTmp = new SwRotatedPortion( nMultiLen + rInf.GetIdx(),
                                         rMulti.GetDirection() );
        }
        
        
        else if( rMulti.IsBidi() && ! pMulti )
        {
            if ( ! rMulti.GetLen() )
                lcl_TruncateMultiPortion( rMulti, rInf, nStartIdx );

            
            
            
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
            
            
            delete pNextFirst;

        rInf.SetRest( pTmp );
    }

    rInf.SetTxt( *pOldTxt );
    rInf.SetPaintOfst( nOldPaintOfst );
    rInf.SetStop( aInf.IsStop() );
    rInf.SetNumDone( true );
    rInf.SetFtnDone( true );
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
    sal_Int32 nPosition )
{
    if( !nPosition )
        return NULL;
    sal_Int32 nMultiPos = nPosition - pLine->GetLen();
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
        
        
        
        
        if( !pPor && pTmpMulti )
        {
            if( pHelpMulti )
            {   
                
                if( !pHelpMulti->IsRuby() )
                    pPor = pHelpMulti->GetRoot().GetNext();
                pTmpMulti = NULL;
            }
            else
            {   
                
                
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
        if ( pHint
             && ( pHint->Which() == RES_TXTATR_FIELD
                  || pHint->Which() == RES_TXTATR_ANNOTATION ) )
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
        ((SwRubyPortion*)pHelpMulti)->GetRubyOffset() < COMPLETE_STRING ) )
    {
        SwMultiPortion* pTmp;
        if( pHelpMulti->IsDouble() )
            pTmp = new SwDoubleLinePortion( *pCreate, nMultiPos );
        else if( pHelpMulti->IsBidi() )
            pTmp = new SwBidiPortion( nMultiPos, pCreate->nLevel );
        else if( pHelpMulti->IsRuby() )
        {
            bool bRubyTop;
            bool* pRubyPos = 0;

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
    delete (pCreate);
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
                                  sal_Int32 nCurrStart,
                                  long nSpaceAdd )
{
    pTxtCrsr = pTxtCursor;
    nStart = pTxtCursor->nStart;
    pTxtCursor->nStart = nCurrStart;
    pCurr = pTxtCursor->pCurr;
    pTxtCursor->pCurr = &pMulti->GetRoot();
    while( pTxtCursor->Y() + pTxtCursor->GetLineHeight() < nY &&
        pTxtCursor->Next() )
        ; 
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
            const sal_Int32 nOldIdx = pTxtCursor->GetInfo().GetIdx();
            pTxtCursor->GetInfo().SetIdx ( nCurrStart );
            nSpaceCnt = ((SwBidiPortion*)pMulti)->GetSpaceCnt(pTxtCursor->GetInfo());
            pTxtCursor->GetInfo().SetIdx ( nOldIdx );
        }

        if( nSpaceAdd > 0 && !pMulti->HasTabulator() )
            pTxtCursor->pCurr->Width( static_cast<sal_uInt16>(nWidth + nSpaceAdd * nSpaceCnt / SPACING_PRECISION_FACTOR ) );

        
        
        if ( nX && pMulti->IsBidi() )
            nX = pTxtCursor->pCurr->Width() - nX;
    }
    else
        bSpaceChg = false;
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
