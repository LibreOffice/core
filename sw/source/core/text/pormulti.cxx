/*************************************************************************
 *
 *  $RCSfile: pormulti.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: ama $ $Date: 2001-02-16 15:27:47 $
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

#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif
#ifndef _SVX_TWOLINESITEM_HXX
#include <svx/twolinesitem.hxx>
#endif
#ifndef _SVX_CHARROTATEITEM_HXX
#include <svx/charrotateitem.hxx>
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
#ifndef _LAYFRM_HXX
#include <layfrm.hxx>       // GetUpper()
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
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif

using namespace ::com::sun::star;

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

/*-----------------16.02.01 12:07-------------------
 * SwRotatedPortion::SwRotatedPortion(..)
 * --------------------------------------------------*/

SwRotatedPortion::SwRotatedPortion( const SwMultiCreator& rCreate,
    xub_StrLen nEnd ) : SwMultiPortion( nEnd )
{
    const SvxCharRotateItem* pRot = (SvxCharRotateItem*)rCreate.pItem;
    if( !pRot )
    {
        const SwTxtAttr& rAttr = *rCreate.pAttr;
        if( RES_CHRATR_ROTATE == rAttr.Which() )
            pRot = &rAttr.GetCharRotate();
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
                    GetItemState( RES_CHRATR_ROTATE, TRUE, &pItem ) )
                    pRot = (SvxCharRotateItem*)pItem;
            }
        }
    }
    if( pRot )
        SetDirection( pRot->IsBottomToTop() ? 1 : 3 );
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

        if( RES_CHRATR_TWO_LINES == rAttr.Which() )
            pTwo = &rAttr.Get2Lines();
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
                    pTwo = (SvxTwoLinesItem*)pItem;
            }
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
    BYTE nTmp = SW_SCRIPTS;
    if( pBracket->cPre )
    {
        String aTxt( pBracket->cPre );
        USHORT nScript = pBreakIt->xBreak->getScriptType( aTxt, 0 );
        switch ( nScript ) {
            case i18n::ScriptType::LATIN : nTmp = SW_LATIN; break;
            case i18n::ScriptType::ASIAN : nTmp = SW_CJK; break;
            case i18n::ScriptType::COMPLEX : nTmp = SW_CTL; break;
        }
    }
    pBracket->nPreScript = nTmp;
    nTmp = SW_SCRIPTS;
    if( pBracket->cPost )
    {
        String aTxt( pBracket->cPost );
        USHORT nScript = pBreakIt->xBreak->getScriptType( aTxt, 0 );
        switch ( nScript ) {
            case i18n::ScriptType::LATIN : nTmp = SW_LATIN; break;
            case i18n::ScriptType::ASIAN : nTmp = SW_CJK; break;
            case i18n::ScriptType::COMPLEX : nTmp = SW_CTL; break;
        }
    }
    pBracket->nPostScript = nTmp;

    if( !pBracket->cPre && !pBracket->cPost )
    {
        delete pBracket;
        pBracket = 0;
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
        BYTE nAct = bOpen ? pBracket->nPreScript : pBracket->nPostScript;
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

/*-----------------25.10.00 16:26-------------------
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
    pBracket->nAscent = 0;
    pBracket->nHeight = 0;
    if( pBracket->cPre )
    {
        String aStr( pBracket->cPre );
        BYTE nActualScr = pTmpFnt->GetActual();
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
        String aStr( pBracket->cPost );
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

SwRubyPortion::SwRubyPortion( const SwMultiCreator& rCreate, const SwFont& rFnt,
    xub_StrLen nEnd, xub_StrLen nOffs ) : SwMultiPortion( nEnd )
{
    SetRuby();
    ASSERT( SW_MC_RUBY == rCreate.nId, "Ruby exspected" );
    ASSERT( RES_TXTATR_CJK_RUBY == rCreate.pAttr->Which(), "Wrong attribute" );
    const SwFmtRuby& rRuby = rCreate.pAttr->GetRuby();
    nAdjustment = rRuby.GetAdjustment();
    nRubyOffset = nOffs;
    SetTop( !rRuby.GetPosition() );
    const SwCharFmt* pFmt = ((SwTxtRuby*)rCreate.pAttr)->GetCharFmt();
    SwFont *pRubyFont;
    if( pFmt )
    {
        const SwAttrSet& rSet = pFmt->GetAttrSet();
         pRubyFont = new SwFont( rFnt );
        pRubyFont->SetDiffFnt( &rSet );
    }
    else
        pRubyFont = NULL;
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

/*-----------------13.11.00 15:38-------------------
 * lcl_Has2Lines(..)
 * is a little help function for GetMultiCreator(..)
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
        rValue = rAttr.Get2Lines().GetValue();
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
    }
    return sal_False;
}

SwMultiCreator* SwTxtSizeInfo::GetMultiCreator( xub_StrLen &rPos ) const
{
    const SvxTwoLinesItem* p2Lines = NULL;
    const SvxCharRotateItem* pRotate = NULL;
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == pFrm->GetTxtNode()->GetSwAttrSet().
        GetItemState( RES_CHRATR_TWO_LINES, TRUE, &pItem ) &&
        ((SvxTwoLinesItem*)pItem)->GetValue() )
        p2Lines = (SvxTwoLinesItem*)pItem;
    else
        pItem = NULL;

    const SwpHints *pHints = pFrm->GetTxtNode()->GetpSwpHints();
    if( !pHints && !p2Lines )
        return NULL;
    const SwTxtAttr *pRuby = NULL;
    sal_Bool bTwo = sal_False;
    USHORT n2Lines = USHRT_MAX;
    USHORT nCount = pHints ? pHints->Count() : 0;
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
                pRuby = pTmp;
            else if( RES_CHRATR_ROTATE == pTmp->Which() )
                pRuby = pTmp;
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
    if( pRuby )
    {   // The winner is ... a ruby attribute and so
        // the end of the multiportion is the end of the ruby attribute.
        rPos = *pRuby->GetEnd();
        SwMultiCreator *pRet = new SwMultiCreator;
        pRet->pItem = NULL;
        pRet->pAttr = pRuby;
        pRet->nId = ( RES_TXTATR_CJK_RUBY == pRuby->Which() ) ?
                    SW_MC_RUBY : SW_MC_ROTATE;
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
            aEnd.Insert( *pRet->pAttr->GetEnd(), 0 );
            if( pItem )
            {
                aEnd[ 0 ] = GetTxt().Len();
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
            aEnd.Insert( GetTxt().Len(), 0 );
        }
        pRet->nId = SW_MC_DOUBLE;
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
        return pRet;
    }
    return NULL;
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
    sal_Bool bSpaceChg  : 1;
    sal_uInt8 nOldDir   : 2;
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
    nOldDir = rInfo.GetDirection();
    rInfo.SetDirection( rMulti.GetDirection() );
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
    rInfo.SetDirection( nOldDir );
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
    KSHORT nOldX = KSHORT(GetInfo().X());
    KSHORT nOldY = KSHORT(GetInfo().Y());
    xub_StrLen nOldIdx = GetInfo().GetIdx();

    SwSpaceManipulator aManip( GetInfo(), rMulti );

    SwFontSave *pFontSave;
    SwFont* pTmpFnt;

    if( rMulti.IsDouble() || rMulti.HasRotation() )
    {
        pTmpFnt = new SwFont( *GetInfo().GetFont() );
        if( rMulti.IsDouble() )
        {
            SetPropFont( 50 );
            pTmpFnt->SetProportion( GetPropFont() );
        }
        pTmpFnt->SetVertical( rMulti.GetFontRotation() );
        pFontSave = new SwFontSave( GetInfo(), pTmpFnt, this );
    }
    else
    {
        pFontSave = NULL;
        pTmpFnt = NULL;
    }

    if( rMulti.HasBrackets() )
    {
        xub_StrLen nOldIdx = GetInfo().GetIdx();
        GetInfo().SetIdx(((SwDoubleLinePortion&)rMulti).GetBrackets()->nStart);
        SeekAndChg( GetInfo() );
        ((SwDoubleLinePortion&)rMulti).PaintBracket( GetInfo(), 0, sal_True );
        GetInfo().SetIdx( nOldIdx );
    }

    KSHORT nTmpX = KSHORT(GetInfo().X());

    SwLineLayout* pLay = &rMulti.GetRoot();// the first line of the multiportion
    SwLinePortion* pPor = pLay->GetFirstPortion();//first portion of these line

    // GetInfo().Y() is the baseline from the surrounding line. We must switch
    // this temporary to the baseline of the inner lines of the multiportion.
    if( rMulti.HasRotation() )
    {
        if( rMulti.IsRevers() )
        {
            GetInfo().Y( nOldY - rMulti.GetAscent() );
            GetInfo().X( nTmpX - pLay->GetAscent() + rMulti.Width() );
        }
        else
        {
            GetInfo().Y( nOldY - rMulti.GetAscent() + rMulti.Height() );
            GetInfo().X( nTmpX + pLay->GetAscent() );
        }
    }
    else
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
            aManip.SecondLine();
            if( rMulti.HasRotation() )
            {
                if( rMulti.IsRevers() )
                {
                    GetInfo().X( nTmpX + pLay->Height() - pLay->GetAscent() );
                    GetInfo().Y( nOldY - rMulti.GetAscent() );
                }
                else
                {
                    GetInfo().X( nTmpX + rMulti.Width()
                                 - pLay->Height() + pLay->GetAscent() );
                    GetInfo().Y( nOldY - rMulti.GetAscent() + rMulti.Height() );
                }
            }
            else
            {
                GetInfo().X( nTmpX );
                // We switch to the baseline of the next inner line
                GetInfo().Y( GetInfo().Y() + rMulti.GetRoot().Height()
                    - rMulti.GetRoot().GetAscent() + pLay->GetAscent() );
            }
        }
    } while( pPor );

    GetInfo().SetIdx( nOldIdx );
    GetInfo().Y( nOldY );

    if( rMulti.HasBrackets() )
    {
        xub_StrLen nOldIdx = GetInfo().GetIdx();
        GetInfo().SetIdx(((SwDoubleLinePortion&)rMulti).GetBrackets()->nStart);
        SeekAndChg( GetInfo() );
        GetInfo().X( nOldX );
        ((SwDoubleLinePortion&)rMulti).PaintBracket( GetInfo(),
            aManip.GetSpaceAdd(), sal_False );
        GetInfo().SetIdx( nOldIdx );
    }
    // Restore the saved values
    GetInfo().X( nOldX );
    GetInfo().SetLen( nOldLen );
    delete pFontSave;
    delete pTmpFnt;
    SetPropFont( 0 );
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

    if( rMulti.HasBrackets() )
    {
        xub_StrLen nOldIdx = rInf.GetIdx();
        rInf.SetIdx( ((SwDoubleLinePortion&)rMulti).GetBrackets()->nStart );
        SeekAndChg( rInf );
        ((SwDoubleLinePortion&)rMulti).FormatBrackets( rInf, nMaxWidth );
        rInf.SetIdx( nOldIdx );
    }

    SeekAndChg( rInf );
    SwFontSave *pFontSave;
    if( rMulti.IsDouble() || rMulti.HasRotation() )
    {
        SwFont* pTmpFnt = new SwFont( *rInf.GetFont() );
        if( rMulti.HasRotation() )
        {
            pTmpFnt->SetVertical( rMulti.GetFontRotation() );
            nMaxWidth = USHRT_MAX;
        }
        if( rMulti.IsDouble() )
        {
            SetPropFont( 50 );
            pTmpFnt->SetProportion( GetPropFont() );
        }
        pFontSave = new SwFontSave( rInf, pTmpFnt, this );
    }
    else
        pFontSave = NULL;

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
        aInf.Width( KSHORT(nActWidth) );
        aInf.RealWidth( KSHORT(nActWidth) );
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
        rMulti.CalcSize( *this, aInf );
        pCurr->SetRealHeight( pCurr->Height() );
        if( rMulti.HasRotation() && !rMulti.IsDouble() )
            break;
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
            rMulti.CalcSize( *this, aInf );
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
        if( nTmpX + rMulti.Width() > rInf.Width() )
        {
            bRet = sal_True;
            rMulti.GetRoot().Truncate();
            rMulti.GetRoot().SetLen(0);
            rMulti.GetRoot().Width(0);
            rMulti.CalcSize( *this, aInf );
            rMulti.SetLen(0);
            rInf.SetIdx( nStartIdx );
        }
    }

    if( bRet )
    {
        ASSERT( !pNextFirst || pNextFirst->InFldGrp(),
            "BuildMultiPortion: Surprising restportion, field exspected" );
        SwMultiPortion *pTmp;
        if( rMulti.IsDouble() )
            pTmp = new SwDoubleLinePortion( ((SwDoubleLinePortion&)rMulti),
                                            nMultiLen + rInf.GetIdx() );
        else if( rMulti.IsRuby() )
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
        else if( rMulti.HasRotation() )
            pTmp = new SwRotatedPortion( nMultiLen + rInf.GetIdx(),
                                         rMulti.GetDirection() );
        else
            pTmp = NULL;
        if( pNextFirst && pTmp )
        {
            pTmp->SetFollowFld();
            pTmp->GetRoot().SetPortion( pNextFirst );
        }
        rInf.SetRest( pTmp );
    }
    rInf.SetTxt( *pOldTxt );
    SeekAndChg( rInf );
    delete pFirstRest;
    delete pSecondRest;
    delete pFontSave;
    SetPropFont( 0 );
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
    if( pFld )
    {
        const SwTxtAttr *pHint = GetAttr( nPos - 1 );
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
    SwMultiCreator* pCreate = GetInfo().GetMultiCreator( nMultiPos );
    ASSERT( pCreate, "Multiportion without attribut?" );

    if( pRest || nMultiPos > nPos || ( pMulti->IsRuby() &&
        ((SwRubyPortion*)pMulti)->GetRubyOffset() < STRING_LEN ) )
    {
        SwMultiPortion* pTmp;
        if( pMulti->IsDouble() )
            pTmp = new SwDoubleLinePortion( *pCreate, nMultiPos );
        else if( pMulti->IsRuby() )
            pTmp = new SwRubyPortion( *pCreate, *GetInfo().GetFont(), nMultiPos,
                                ((SwRubyPortion*)pMulti)->GetRubyOffset() );
        else if( pMulti->GetDirection() )
            pTmp = new SwRotatedPortion( nMultiPos, pMulti->GetDirection() );
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
    nOldProp = pTxtCursor->GetPropFont();
    if( pMulti->IsDouble() )
    {
        pTxtCursor->SetPropFont( 50 );
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
    pTxtCrsr->pCurr->Width( KSHORT(nWidth) );
    pTxtCrsr->pCurr = pCurr;
    pTxtCrsr->nStart = nStart;
    pTxtCrsr->SetPropFont( nOldProp );
}

