/*************************************************************************
 *
 *  $RCSfile: pormulti.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-20 09:35:44 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVX_TWOLINESITEM_HXX
#include <svx/twolinesitem.hxx>
#endif
#ifndef _TXATBASE_HXX //autogen
#include <txatbase.hxx>
#endif
#ifndef _FMTRUBY_HXX
#include <fmtruby.hxx>  // SwFmtRuby
#endif
#ifndef _TXTATR_HXX
#include <txtatr.hxx>   // SwTxtRuby
#endif
#ifndef _CHARFMT_HXX    // SwCharFmt
#include <charfmt.hxx>
#endif
#ifndef _TXTINET_HXX    // SwTxtINetFmt
#include <txtinet.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _PORMULTI_HXX
#include <pormulti.hxx>     // SwMultiPortion
#endif
#ifndef _INFTXT_HXX
#include <inftxt.hxx>       // SwTxtSizeInfo
#endif
#ifndef _ITRPAINT_HXX
#include <itrpaint.hxx>     // SwTxtPainter
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>      // SwViewOptions
#endif
#ifndef _ITRFORM2_HXX
#include <itrform2.hxx>     // SwTxtFormatter
#endif
#ifndef _PORFLD_HXX
#include <porfld.hxx>       // SwFldPortion
#endif
#ifndef _PORGLUE_HXX
#include <porglue.hxx>
#endif

/*-----------------10.10.00 15:23-------------------
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

void SwMultiPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    ASSERT( FALSE,
    "Don't try SwMultiPortion::Paint, try SwTxtPainter::PaintMultiPortion" );
}

/*-----------------13.10.00 16:21-------------------
 * Summarize the internal lines to calculate the (external) size.
 * The internal line has to calculate first.
 * --------------------------------------------------*/

void SwMultiPortion::CalcSize( SwTxtFormatter& rLine )
{
    Width( 0 );
    Height( 0 );
    SetAscent( 0 );
    SwLineLayout *pLay = &GetRoot();
    do
    {
        pLay->CalcLine( rLine );
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
}

long SwMultiPortion::CalcSpacing( short nSpaceAdd, const SwTxtSizeInfo &rInf )
    const
{
    return 0;
}

/*-----------------01.11.00 14:21-------------------
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

/*-----------------01.11.00 14:22-------------------
 * SwDoubleLinePortion::SwDoubleLinePortion(..)
 * This constructor is for the continuation of a doubleline portion
 * in the next line.
 * It takes the same brackets and if the original has no content except
 * brackets, these will be deleted.
 * --------------------------------------------------*/

SwDoubleLinePortion::SwDoubleLinePortion( SwDoubleLinePortion& rDouble,
    xub_StrLen nEnd ) : SwMultiPortion( nEnd ), pBracket( 0 )
{
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

/*-----------------01.11.00 14:22-------------------
 * SwDoubleLinePortion::SwDoubleLinePortion(..)
 * This constructor uses the textattribut to get the right brackets.
 * The textattribut could be a 2-line-attribute or a character- or
 * internetstyle, which contains the 2-line-attribute.
 * --------------------------------------------------*/

SwDoubleLinePortion::SwDoubleLinePortion( const SwTxtAttr& rAttr,
    xub_StrLen nEnd ) : SwMultiPortion( nEnd ), pBracket( new SwBracket() )
{
    SetDouble();
    if( RES_CHRATR_TWO_LINES == rAttr.Which() )
    {
        pBracket->cPre = rAttr.Get2Lines().GetStartBracket();
        pBracket->cPost = rAttr.Get2Lines().GetEndBracket();
    }
    else
    {
        SwCharFmt* pFmt = NULL;
        if( RES_TXTATR_INETFMT == rAttr.Which() )
            pFmt = ((SwTxtINetFmt&)rAttr).GetCharFmt();
        else if( RES_TXTATR_CHARFMT == rAttr.Which() )
            pFmt = rAttr.GetCharFmt().GetCharFmt();
        if ( pFmt )
        {
            const SfxPoolItem* pItem;
            if( SFX_ITEM_SET == pFmt->GetAttrSet().
                GetItemState( RES_CHRATR_TWO_LINES, TRUE, &pItem ) )
            {
                pBracket->cPre = ((SvxTwoLinesItem*)pItem)->GetStartBracket();
                pBracket->cPost = ((SvxTwoLinesItem*)pItem)->GetEndBracket();
            }
        }
        else
        {
            pBracket->cPre = 0;
            pBracket->cPost = 0;
         }
    }
}


/*-----------------25.10.00 09:51-------------------
 * SwMultiPortion::PaintBracket paints the wished bracket,
 * if the multiportion has surrounding brackets.
 * The X-position of the SwTxtPaintInfo will be modified:
 * the open bracket sets position behind itself,
 * the close bracket in front of itself.
 * --------------------------------------------------*/

void SwDoubleLinePortion::PaintBracket( SwTxtPaintInfo &rInf,
    short nSpaceAdd, sal_Bool bOpen ) const
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
        pTmpFnt->SetProportion( 100 );
        SwFontSave aSave( rInf, pTmpFnt );
        aBlank.Paint( rInf );
        delete pTmpFnt;
    }
    if( bOpen )
        rInf.X( rInf.X() + PreWidth() );
}

/*-----------------25.10.00 16:26-------------------
 * SwDoubleLinePortion::SetBrackets creates the bracket-structur
 * and fills it, if not both characters are 0x00.
 * --------------------------------------------------*/

void SwDoubleLinePortion::SetBrackets( sal_Unicode cPre, sal_Unicode cPost )
{
    if( cPre || cPost )
    {
        pBracket = new SwBracket;
        pBracket->cPre = cPre;
        pBracket->cPost = cPost;
    }
}

/*-----------------25.10.00 16:29-------------------
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
    SwFontSave aSave( rInf, pTmpFnt );
    pBracket->nAscent = rInf.GetAscent();
    if( pBracket->cPre )
    {
        String aStr( pBracket->cPre );
        SwPosSize aSize = rInf.GetTxtSize( aStr );
        pBracket->nHeight = aSize.Height();
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
        String aStr( pBracket->cPost );
        SwPosSize aSize = rInf.GetTxtSize( aStr );
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

/*-----------------26.10.00 10:36-------------------
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
            nBlank1 += ((SwTxtPortion*)pPor)->GetSpaceCnt( rInf, nNull );
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
            nBlank2 += ((SwTxtPortion*)pPor)->GetSpaceCnt( rInf, nNull );
        rInf.SetIdx( rInf.GetIdx() + pPor->GetLen() );
        if( pPor->InTabGrp() )
            SetTab2( sal_True );
    }
    rInf.SetIdx( nStart );
}

long SwDoubleLinePortion::CalcSpacing( short nSpaceAdd, const SwTxtSizeInfo &rInf ) const
{
    return HasTabulator() ? 0 : GetSpaceCnt() * nSpaceAdd;
}

/*-----------------01.11.00 14:29-------------------
 * SwDoubleLinePortion::ChangeSpaceAdd(..)
 * merges the spaces for text adjustment from the inner and outer part.
 * Inside the doubleline portion the wider line has no spaceadd-array, the
 * smaller line has such an array to reach width of the wider line.
 * If the surrounding line has text adjustment and the doubleline portion
 * contains no tabulator, it is necessary to create/manipulate the inner
 * space arrays.
 * --------------------------------------------------*/

sal_Bool SwDoubleLinePortion::ChangeSpaceAdd( SwLineLayout* pCurr, short nSpaceAdd )
{
    sal_Bool bRet = sal_False;
    if( !HasTabulator() && nSpaceAdd > 0 )
    {
        if( pCurr->IsNoSpaceAdd() )
        {   // The wider line gets the spaceadd from the surrounding line direct
            pCurr->CreateSpaceAdd();
            ( pCurr->GetSpaceAdd() )[0] = nSpaceAdd;
            bRet = sal_True;
        }
        else
        {
            xub_StrLen nMyBlank = GetSmallerSpaceCnt();
            xub_StrLen nOther = GetSpaceCnt();
            SwTwips nMultiSpace = pCurr->GetSpaceAdd()[0] * nMyBlank
                                  + nOther * nSpaceAdd;
            if( nMyBlank )
                nMultiSpace /= nMyBlank;
            if( nMultiSpace < KSHRT_MAX )
            {
                pCurr->GetpSpaceAdd()->Insert(KSHORT(nMultiSpace),0);
                bRet = sal_True;
            }
        }
    }
    return bRet;
}
/*-----------------01.11.00 14:29-------------------
 * SwDoubleLinePortion::ResetSpaceAdd(..)
 * cancels the manipulation from SwDoubleLinePortion::ChangeSpaceAdd(..)
 * --------------------------------------------------*/

void SwDoubleLinePortion::ResetSpaceAdd( SwLineLayout* pCurr )
{
    pCurr->GetSpaceAdd().Remove(0);
    if( !pCurr->GetSpaceAdd().Count() )
        pCurr->FinishSpaceAdd();
}

SwDoubleLinePortion::~SwDoubleLinePortion()
{
    delete pBracket;
}

/*-----------------13.11.00 14:50-------------------
 * SwRubyPortion::SwRubyPortion(..)
 * constructs a ruby portion, i.e. an additional text is displayed
 * beside the main text, e.g. phonetic characters.
 * --------------------------------------------------*/

SwRubyPortion::SwRubyPortion( const SwTxtAttr& rAttr,  const SwFont& rFnt,
    xub_StrLen nEnd, xub_StrLen nOffs ) : SwMultiPortion( nEnd )
{
    SetRuby();
    ASSERT( RES_TXTATR_CJK_RUBY == rAttr.Which(), "Wrong attribute" );
    const SwFmtRuby& rRuby = rAttr.GetRuby();
    nAdjustment = rRuby.GetAdjustment();
    nRubyOffset = nOffs;
    SetTop( !rRuby.GetPosition() );
    const SwAttrSet& rSet = ((SwTxtRuby&)rAttr).GetCharFmt()->GetAttrSet();
    SwFont *pRubyFont = new SwFont( rFnt );
    pRubyFont->SetDiffFnt( &rSet );
    String aStr( rRuby.GetText(), nOffs, STRING_LEN );
    SwFldPortion *pFld = new SwFldPortion( aStr, pRubyFont );
    pFld->SetFollow( sal_True );
    if( !rRuby.GetPosition() )
        GetRoot().SetPortion( pFld );
    else
    {
        GetRoot().SetNext( new SwLineLayout() );
        GetRoot().GetNext()->SetPortion( pFld );
    }
}

/*-----------------13.11.00 14:56-------------------
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
    USHORT nSub = 0;
    switch ( nAdjustment )
    {
        case 1: nRight = nLineDiff/2;    // no break
        case 2: nLeft = nLineDiff - nRight; break;
        case 3: nSub = 1; // no break
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
                pCurr->CreateSpaceAdd();
                pCurr->GetSpaceAdd().Insert( nTmp, 0 );
                nLineDiff -= nCalc * ( nCharCnt - 1 );
            }
            if( nLineDiff > 1 )
            {
                nRight = nLineDiff/2;
                nLeft = nLineDiff - nRight;
            }
            break;
        }
        default: ASSERT( sal_False, "New ruby adjustment" );
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
            if( pCurr->GetpSpaceAdd() )
                pCurr->GetSpaceAdd().Insert( short(0), 0 );
        }
    }
    rInf.SetIdx( nOldIdx );
}

/*-----------------08.11.00 14:14-------------------
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

/*-----------------13.10.00 16:22-------------------
 * SwTxtSizeInfo::GetMultiAttr(..)
 * If we (e.g. the position rPos) are inside a two-line-attribute or
 * a ruby-attribute, the attribute will be returned,
 * otherwise the function returns zero.
 * The rPos parameter is set to the end of the multiportion,
 * normally this is the end of the attribute,
 * but sometimes it is the start of another attribute, which finished or
 * interrupts the first attribute.
 * E.g. a ruby portion interrupts a 2-line-attribute, a 2-line-attribute
 * with different brackets interrupts another 2-line-attribute.
 * --------------------------------------------------*/

/*-----------------13.11.00 15:38-------------------
 * lcl_Has2Lines(..)
 * is a little help function for GetMultiAttr(..)
 * It extracts the 2-line-format from a 2-line-attribute or a character style.
 * The rValue is set to TRUE, if the 2-line-attribute's value is set and
 * no 2-line-format reference is passed. If there is a 2-line-format reference,
 * then the rValue is set only, if the 2-line-attribute's value is set _and_
 * the 2-line-formats has the same brackets.
 * --------------------------------------------------*/

sal_Bool lcl_Has2Lines( const SwTxtAttr& rAttr, const SvxTwoLinesItem* &rpRef,
    sal_Bool &rValue )
{
    if( RES_CHRATR_TWO_LINES == rAttr.Which() )
    {
        rValue = /* rAttr.Get2Lines().GetValue() */ sal_True;
        if( !rpRef )
            rpRef = &rAttr.Get2Lines();
        else if( rAttr.Get2Lines().GetEndBracket() != rpRef->GetEndBracket() ||
            rAttr.Get2Lines().GetStartBracket() != rpRef->GetStartBracket() )
            rValue = sal_False;
        return sal_True;
    }
    SwCharFmt* pFmt = NULL;
    if( RES_TXTATR_INETFMT == rAttr.Which() )
        pFmt = ((SwTxtINetFmt&)rAttr).GetCharFmt();
    else if( RES_TXTATR_CHARFMT == rAttr.Which() )
        pFmt = rAttr.GetCharFmt().GetCharFmt();
    if ( pFmt )
    {
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == pFmt->GetAttrSet().
            GetItemState( RES_CHRATR_TWO_LINES, TRUE, &pItem ) )
        {
            rValue = /* ((SvxTwoLinesItem*)pItem)->GetValue(); */ sal_True;
            if( !rpRef )
                rpRef = (SvxTwoLinesItem*)pItem;
            else if( ((SvxTwoLinesItem*)pItem)->GetEndBracket() !=
                        rpRef->GetEndBracket() ||
                        ((SvxTwoLinesItem*)pItem)->GetStartBracket() !=
                        rpRef->GetStartBracket() )
                rValue = sal_False;
            return sal_True;
        }
    }
    return sal_False;
}

const SwTxtAttr* SwTxtSizeInfo::GetMultiAttr( xub_StrLen &rPos ) const
{
    const SwpHints *pHints = pFrm->GetTxtNode()->GetpSwpHints();
    if( !pHints )
        return NULL;
    const SwTxtAttr *pRet = NULL;
    const SvxTwoLinesItem* p2Lines = NULL;
    sal_Bool bTwo = sal_False;
    USHORT n2Lines = USHRT_MAX;
    USHORT nCount = pHints->Count();
    USHORT i;
    for( i = 0; i < nCount; ++i )
    {
        const SwTxtAttr *pTmp = (*pHints)[i];
        xub_StrLen nStart = *pTmp->GetStart();
        if( rPos < nStart )
            break;
        if( *pTmp->GetAnyEnd() > rPos )
        {
            if( RES_TXTATR_CJK_RUBY == pTmp->Which() )
                pRet = pTmp;
            else
            {
                const SvxTwoLinesItem* p2Tmp = NULL;
                if( lcl_Has2Lines( *pTmp, p2Tmp, bTwo ) )
                {
                    n2Lines = bTwo ? i : nCount;
                    p2Lines = p2Tmp;
                }
            }
        }
    }
    if( pRet )
    {   // The winner is ... a ruby attribute and so
        // the end of the multiportion is the end of the ruby attribute.
        rPos = *pRet->GetEnd();
        return pRet;
    }
    if( n2Lines < nCount )
    {   // The winner is a 2-line-attribute,
        // the end of the multiportion depends on the following attributes...
        pRet = (*pHints)[n2Lines];
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

        // We note the endpositions of the 2-line attributes in aEnd as stack
        SvXub_StrLens aEnd;
        aEnd.Insert( *pRet->GetEnd(), 0 );

        // The bOn flag signs the state of the last 2-line attribute in the
        // aEnd-stack, it is compatible with the winner-attribute or
        // it interrupts the other attribute.
        sal_Bool bOn = sal_True;

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
                // If bOn is FALSE and the next attribute starts later than rPos
                // the winner attribute is interrupted at rPos.
                // If the start of the next atribute is behind the end of
                // the last attribute on the aEnd-stack, this is the endposition
                // on the stack is the end of the 2-line portion.
                if( !bOn || aEnd[ aEnd.Count()-1 ] < *pTmp->GetStart() )
                    break;
                // At this moment, bOn is TRUE and the next attribute starts
                // behind rPos, so we could move rPos to the next startpoint
                rPos = *pTmp->GetStart();
                // We clean up the aEnd-stack, endpositions equal to rPos are
                // superfluous.
                while( aEnd.Count() && aEnd[ aEnd.Count()-1 ] <= rPos )
                {
                    bOn = !bOn;
                    aEnd.Remove( aEnd.Count()-1, 1 );
                }
                // If the endstack is empty, we simulate an attribute with
                // state TRUE and endposition rPos
                if( !aEnd.Count() )
                {
                    aEnd.Insert( rPos, 0 );
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
                    if( aEnd[ aEnd.Count()-1 ] < *pTmp->GetEnd() )
                        aEnd[ aEnd.Count()-1 ] = *pTmp->GetEnd();
                }
                else
                {   // .. with a different state.
                    bOn = bTwo;
                    // If this is smaller than the last on the stack, we put
                    // it on the stack. If it has the same endposition, the last
                    // could be removed.
                    if( aEnd[ aEnd.Count()-1 ] > *pTmp->GetEnd() )
                        aEnd.Insert( *pTmp->GetEnd(), aEnd.Count() );
                    else if( aEnd.Count() > 1 )
                        aEnd.Remove( aEnd.Count()-1, 1 );
                    else
                        aEnd[ aEnd.Count()-1 ] = *pTmp->GetEnd();
                }
            }
        }
        if( bOn && aEnd.Count() )
            rPos = aEnd[ aEnd.Count()-1 ];
    }
    return pRet;
}

/*-----------------01.11.00 14:52-------------------
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
    SvShorts *pOldSpaceAdd;
    MSHORT nOldSpIdx;
    short nSpaceAdd;
    sal_Bool bSpaceChg;
public:
    SwSpaceManipulator( SwTxtPaintInfo& rInf, SwMultiPortion& rMult );
    ~SwSpaceManipulator();
    void SecondLine();
    inline short GetSpaceAdd() const { return nSpaceAdd; }
};

SwSpaceManipulator::SwSpaceManipulator( SwTxtPaintInfo& rInf,
    SwMultiPortion& rMult ) : rInfo( rInf ), rMulti( rMult )
{
    pOldSpaceAdd = rInfo.GetpSpaceAdd();
    nOldSpIdx = rInfo.GetSpaceIdx();
    bSpaceChg = sal_False;
    if( rMulti.IsDouble() )
    {
        nSpaceAdd = ( pOldSpaceAdd && !rMulti.HasTabulator() ) ?
                      rInfo.GetSpaceAdd() : 0;
        if( rMulti.GetRoot().GetpSpaceAdd() )
        {
            rInfo.SetSpaceAdd( rMulti.GetRoot().GetpSpaceAdd() );
            rInfo.ResetSpaceIdx();
            bSpaceChg = rMulti.ChgSpaceAdd( &rMulti.GetRoot(), nSpaceAdd );
        }
        else if( rMulti.HasTabulator() )
            rInfo.SetSpaceAdd( NULL );
    }
    else
    {
        rInfo.SetSpaceAdd( rMulti.GetRoot().GetpSpaceAdd() );
        rInfo.ResetSpaceIdx();
    }
}

void SwSpaceManipulator::SecondLine()
{
    if( bSpaceChg )
    {
        rInfo.GetpSpaceAdd()->Remove( 0 );
        bSpaceChg = sal_False;
    }
    SwLineLayout *pLay = rMulti.GetRoot().GetNext();
    if( pLay->GetpSpaceAdd() )
    {
        rInfo.SetSpaceAdd( pLay->GetpSpaceAdd() );
        rInfo.ResetSpaceIdx();
        bSpaceChg = rMulti.ChgSpaceAdd( pLay, nSpaceAdd );
    }
    else
    {
        rInfo.SetSpaceAdd( (!rMulti.IsDouble() || rMulti.HasTabulator() ) ?
                                0 : pOldSpaceAdd );
        rInfo.SetSpaceIdx( nOldSpIdx);
    }
}

SwSpaceManipulator::~SwSpaceManipulator()
{
    if( bSpaceChg )
    {
        rInfo.GetpSpaceAdd()->Remove( 0 );
        bSpaceChg = sal_False;
    }
    rInfo.SetSpaceAdd( pOldSpaceAdd );
    rInfo.SetSpaceIdx( nOldSpIdx);
}

/*-----------------13.10.00 16:24-------------------
 * SwTxtPainter::PaintMultiPortion manages the paint for a SwMultiPortion.
 * External, for the calling function, it seems to be a normal Paint-function,
 * internal it is like a SwTxtFrm::Paint with multiple DrawTextLines
 * --------------------------------------------------*/

void SwTxtPainter::PaintMultiPortion( const SwRect &rPaint,
    SwMultiPortion& rMulti )
{
    if( rMulti.Width() > 1 )
        GetInfo().DrawViewOpt( rMulti, POR_FLD );

    // old values must be saved and restored at the end
    xub_StrLen nOldLen = GetInfo().GetLen();
    KSHORT nOldX = GetInfo().X();
    KSHORT nOldY = GetInfo().Y();
    xub_StrLen nOldIdx = GetInfo().GetIdx();

    SwSpaceManipulator aManip( GetInfo(), rMulti );

    if( rMulti.HasBrackets() )
    {
        SeekAndChg( GetInfo() );
        ((SwDoubleLinePortion&)rMulti).PaintBracket( GetInfo(), 0, sal_True );
    }

    KSHORT nTmpX = GetInfo().X();

    SwLineLayout* pLay = &rMulti.GetRoot();// the first line of the multiportion
    SwLinePortion* pPor = pLay->GetFirstPortion();//first portion of these line

    // GetInfo().Y() is the baseline from the surrounding line. We must switch
    // this temporary to the baseline of the inner lines of the multiportion.
    GetInfo().Y( nOldY - rMulti.GetAscent() + pLay->GetAscent() );
    sal_Bool bRest = pLay->IsRest();
    sal_Bool bFirst = sal_True;
    do
    {
        sal_Bool bSeeked = sal_True;
        GetInfo().SetLen( pPor->GetLen() );
        GetInfo().SetSpecialUnderline( sal_False );
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

        if( pFnt->GetEscapement() && UNDERLINE_NONE != pFnt->GetUnderline() )
            CheckSpecialUnderline();

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
            GetInfo().X( nTmpX );
            aManip.SecondLine();
            // We switch to the baseline of the next inner line
            GetInfo().Y( GetInfo().Y() + rMulti.GetRoot().Height()
                - rMulti.GetRoot().GetAscent() + pLay->GetAscent() );
        }
    } while( pPor );

    GetInfo().SetIdx( nOldIdx );
    GetInfo().Y( nOldY );

    if( rMulti.HasBrackets() )
    {
        SeekAndChg( GetInfo() );
        GetInfo().X( nOldX );
        ((SwDoubleLinePortion&)rMulti).PaintBracket( GetInfo(),
            aManip.GetSpaceAdd(), sal_False );
    }
    // Restore the saved values
    GetInfo().X( nOldX );
    GetInfo().SetLen( nOldLen );
}

/*-----------------13.10.00 16:46-------------------
 * SwTxtFormatter::BuildMultiPortion manages the formatting of a SwMultiPortion.
 * External, for the calling function, it seems to be a normal Format-function,
 * internal it is like a SwTxtFrm::_Format with multiple BuildPortions
 * --------------------------------------------------*/

sal_Bool lcl_ExtractFieldFollow( SwLineLayout* pLine, SwLinePortion* &rpFld )
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

BOOL SwTxtFormatter::BuildMultiPortion( SwTxtFormatInfo &rInf,
    SwMultiPortion& rMulti )
{
    SwTwips nMaxWidth = rInf.Width();
    SeekAndChg( rInf );
    if( rMulti.HasBrackets() )
        ((SwDoubleLinePortion&)rMulti).FormatBrackets( rInf, nMaxWidth );

    SwTwips nTmpX = rInf.X();

    pMulti = &rMulti;
    SwLineLayout *pOldCurr = pCurr;
    xub_StrLen nOldStart = GetStart();
    SwTwips nMinWidth = nTmpX + 1;
    SwTwips nActWidth = nMaxWidth;
    xub_StrLen nStartIdx = rInf.GetIdx();
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
        nMultiLen -= rInf.GetIdx();
    }

    const XubString* pOldTxt = &(rInf.GetTxt());
    XubString aMultiStr( rInf.GetTxt(), 0, nMultiLen + rInf.GetIdx() );
    rInf.SetTxt( aMultiStr );
    SwTxtFormatInfo aInf( rInf, rMulti.GetRoot(), nActWidth );

    SwLinePortion *pNextFirst = NULL;
    SwLinePortion *pNextSecond = NULL;
    BOOL bRet = FALSE;
    do
    {
        pCurr = &rMulti.GetRoot();
        nStart = nStartIdx;
        bRet = FALSE;
        FormatReset( aInf );
        aInf.X( nTmpX );
        aInf.Width( nActWidth );
        if( pFirstRest )
        {
            ASSERT( pFirstRest->InFldGrp(), "BuildMulti: Fieldrest exspected");
            SwFldPortion *pFld =
                ((SwFldPortion*)pFirstRest)->Clone(
                    ((SwFldPortion*)pFirstRest)->GetExp() );
            pFld->SetFollow( sal_True );
            aInf.SetRest( pFld );
        }
        aInf.SetRuby( rMulti.IsRuby() && rMulti.OnTop() );
        // If there's no more rubytext, then buildportion is forbidden
        if( pFirstRest || !aInf.IsRuby() )
            BuildPortions( aInf );
        rMulti.CalcSize( *this );
        pCurr->SetRealHeight( pCurr->Height() );
        if( pCurr->GetLen() < nMultiLen || rMulti.IsRuby() || aInf.GetRest() )
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
                    ASSERT( pSecondRest->InFldGrp(), "Fieldrest exspected");
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
            BuildPortions( aTmp );
            rMulti.CalcSize( *this );
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
                bRet = sal_True;
        }
        if( rMulti.IsRuby() )
            break;
        if( bRet )
        {
            nMinWidth = nActWidth;
            nActWidth = ( 3 * nMaxWidth + nMinWidth + 3 ) / 4;
            if( nActWidth <= nMinWidth )
                break;
        }
        else
        {
            if( nActWidth > nTmpX + rMulti.Width() + 1)
                nActWidth = nTmpX + rMulti.Width() + 1;
            nMaxWidth = nActWidth;
            nActWidth = ( 3 * nMaxWidth + nMinWidth + 3 ) / 4;
            if( nActWidth >= nMaxWidth )
                break;
        }
        delete pNextFirst;
        pNextFirst = NULL;
    } while ( TRUE );
    pMulti = NULL;
    pCurr = pOldCurr;
    nStart = nOldStart;
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
            rMulti.Width( rMulti.Width() +
                ((SwDoubleLinePortion&)rMulti).BracketWidth() );
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

    if( bRet )
    {
        SwMultiPortion *pTmp;
        if( rMulti.IsDouble() )
            pTmp = new SwDoubleLinePortion( ((SwDoubleLinePortion&)rMulti),
                                            nMultiLen + rInf.GetIdx() );
        ASSERT( !pNextFirst || pNextFirst->InFldGrp(),
            "BuildMultiPortion: Surprising restportion, field exspected" );
        if( rMulti.IsRuby() )
        {
            ASSERT( !pNextSecond || pNextSecond->InFldGrp(),
                "BuildMultiPortion: Surprising restportion, field exspected" );
            pTmp = new SwRubyPortion( nMultiLen + rInf.GetIdx(),
                    ((SwRubyPortion&)rMulti).GetAdjustment(), !rMulti.OnTop(),
                    ((SwRubyPortion&)rMulti).GetRubyOffset() );
            if( pNextSecond )
            {
                pTmp->GetRoot().SetNext( new SwLineLayout() );
                pTmp->GetRoot().GetNext()->SetPortion( pNextSecond );
            }
            pTmp->SetFollowFld();
        }
        if( pNextFirst )
        {
            pTmp->SetFollowFld();
            pTmp->GetRoot().SetPortion( pNextFirst );
        }
        rInf.SetRest( pTmp );
    }
    rInf.SetTxt( *pOldTxt );
    delete pFirstRest;
    delete pSecondRest;
    return bRet;
}

/*-----------------08.11.00 09:29-------------------
 * SwTxtFormatter::MakeRestPortion(..)
 * When a fieldportion at the end of line breaks and needs a following
 * fieldportion in the next line, then the "restportion" of the formatinfo
 * has to be set. Normally this happens during the formatting of the first
 * part of the fieldportion.
 * But sometimes the formatting starts at the line with the following part,
 * exspecally when the following part is on the next page.
 * In this case the MakeRestPortion-function has to create the following part.
 * The first parameter is the line that contains possibly a first part
 * of a field. When the function finds such field part, it creates the right
 * restportion. This may be a multiportion, e.g. if the field is surrounded by
 * a doubleline- or ruby-portion.
 * The second parameter is the start index of the line.
 * --------------------------------------------------*/

SwLinePortion* SwTxtFormatter::MakeRestPortion( const SwLineLayout* pLine,
    xub_StrLen nPos )
{
    if( !nPos )
        return NULL;
    xub_StrLen nMultiPos = nPos - pLine->GetLen();
    const SwMultiPortion *pTmpMulti = NULL;
    const SwMultiPortion *pMulti = NULL;
    const SwLinePortion* pPor = pLine->GetFirstPortion();
    SwFldPortion *pFld = NULL;
    while( pPor )
    {
        if( pPor->GetLen() )
        {
            if( !pMulti )
            {
                nMultiPos += pPor->GetLen();
                pTmpMulti = NULL;
            }
        }
        if( pPor->InFldGrp() )
        {
            if( !pMulti )
                pTmpMulti = NULL;
            pFld = (SwFldPortion*)pPor;
        }
        else if( pPor->IsMultiPortion() )
        {
            ASSERT( !pMulti, "Nested multiportions are forbidden." );
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
            if( pMulti )
            {   // We're already inside the multiportion, let's take the second
                // line, if we are in a double line portion
                if( !pMulti->IsRuby() )
                    pPor = pMulti->GetRoot().GetNext();
                pTmpMulti = NULL;
            }
            else
            {   // Now we enter a multiportion, in a ruby portion we take the
                // main line, not the phonetic line, in a doublelineportion we
                // starts with the first line.
                pMulti = pTmpMulti;
                nMultiPos -= pMulti->GetLen();
                if( pMulti->IsRuby() && pMulti->OnTop() )
                    pPor = pMulti->GetRoot().GetNext();
                else
                    pPor = pMulti->GetRoot().GetFirstPortion();
            }
        }
    }
    if( pFld && !pFld->HasFollow() )
        pFld = NULL;

    SwLinePortion *pRest = NULL;
    const SwTxtAttr *pHint;
    if( pFld )
    {
        pHint = GetAttr( nPos - 1 );
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
    if( !pMulti )
        return pRest;

    nPos = nMultiPos + pMulti->GetLen();
    pHint = GetInfo().GetMultiAttr( nMultiPos );
    ASSERT( pHint, "Multiportion without attribut?" );

    if( pRest || nMultiPos > nPos || ( pMulti->IsRuby() &&
        ((SwRubyPortion*)pMulti)->GetRubyOffset() < STRING_LEN ) )
    {
        SwMultiPortion* pTmp;
        if( pMulti->IsDouble() )
            pTmp = new SwDoubleLinePortion( *pHint, nMultiPos );
        else if( pMulti->IsRuby() )
            pTmp = new SwRubyPortion( *pHint, *GetInfo().GetFont(), nMultiPos,
                                ((SwRubyPortion*)pMulti)->GetRubyOffset() );
        else
            return pRest;
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



/*-----------------23.10.00 10:47-------------------
 * SwTxtCursorSave notes the start and current line of a SwTxtCursor,
 * sets them to the values for GetCrsrOfst inside a multiportion
 * and restores them in the destructor.
 * --------------------------------------------------*/

SwTxtCursorSave::SwTxtCursorSave( SwTxtCursor* pTxtCursor,
    SwMultiPortion* pMulti, SwTwips nY, xub_StrLen nCurrStart, short nSpaceAdd )
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
    if( pMulti->IsDouble() )
    {
        bSpaceChg = pMulti->ChgSpaceAdd( pTxtCursor->pCurr, nSpaceAdd );
        if( nSpaceAdd > 0 && !pMulti->HasTabulator() )
            pTxtCursor->pCurr->Width( nWidth + nSpaceAdd *
            ((SwDoubleLinePortion*)pMulti)->GetSpaceCnt() );
    }
    else
        bSpaceChg = sal_False;
}

SwTxtCursorSave::~SwTxtCursorSave()
{
    if( bSpaceChg )
        SwDoubleLinePortion::ResetSpaceAdd( pTxtCrsr->pCurr );
    pTxtCrsr->pCurr->Width( nWidth );
    pTxtCrsr->pCurr = pCurr;
    pTxtCrsr->nStart = nStart;
}

