/*************************************************************************
 *
 *  $RCSfile: pormulti.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ama $ $Date: 2000-10-30 09:58:10 $
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

#ifndef _TXATBASE_HXX //autogen
#include <txatbase.hxx>
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
    delete pBracket;
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
        Height( Height() + pLay->Height() );
        SetAscent( GetAscent() + pLay->GetAscent() );
        if( Width() < pLay->Width() )
            Width( pLay->Width() );
        pLay = pLay->GetNext();
    } while ( pLay );
}

/*-----------------25.10.00 09:51-------------------
 * SwMultiPortion::PaintBracket paints the wished bracket,
 * if the multiportion has surrounding brackets.
 * The X-position of the SwTxtPaintInfo will be modified:
 * the open bracket sets position behind itself,
 * the close bracket in front of itself.
 * --------------------------------------------------*/

void SwMultiPortion::PaintBracket( const SwTxtPaintInfo &rInf,
    sal_Bool bOpen ) const
{
    sal_Unicode cCh = bOpen ? pBracket->cPre : pBracket->cPost;
    if( !cCh )
        return;
    KSHORT nChWidth = bOpen ? PreWidth() : PostWidth();
    if( !nChWidth )
        return;
    SwBlankPortion aBlank( cCh, sal_True );
    aBlank.SetAscent( pBracket->nAscent );
    aBlank.Width( nChWidth );
    aBlank.Height( pBracket->nHeight );
    SwFont* pTmpFnt = new SwFont( *rInf.GetFont() );
    pTmpFnt->SetProportion( 100 );
    SwFontSave aSave( rInf, pTmpFnt );
    aBlank.Paint( rInf );
}

/*-----------------25.10.00 16:26-------------------
 * SwMultiPortion::SetBrackets creates the bracket-structur
 * and fills it, if not both characters are 0x00.
 * --------------------------------------------------*/

void SwMultiPortion::SetBrackets( sal_Unicode cPre, sal_Unicode cPost )
{
    if( cPre || cPost )
    {
        pBracket = new SwBracket;
        pBracket->cPre = cPre;
        pBracket->cPost = cPost;
    }
}

/*-----------------25.10.00 16:29-------------------
 * SwMultiPortion::FormatBrackets
 * calculates the size of the brackets => pBracket,
 * reduces the nMaxWidth-parameter ( minus bracket-width )
 * and moves the rInf-x-position behind the opening bracket.
 * --------------------------------------------------*/

void SwMultiPortion::FormatBrackets( SwTxtFormatInfo &rInf, SwTwips& nMaxWidth )
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
 * SwMultiPortion::CalcBlanks
 * calculates the number of blanks in each line and
 * the difference of the width of the two lines.
 * These results are used from the text adjustment.
 * --------------------------------------------------*/

void SwMultiPortion::CalcBlanks( SwTxtFormatInfo &rInf )
{
    SwLinePortion* pPor = aRoot.GetFirstPortion();
    xub_StrLen nNull = 0;
    xub_StrLen nStart = rInf.GetIdx();
    bTabulator = sal_False;
    for( nBlank1 = 0; pPor; pPor = pPor->GetPortion() )
    {
        if( pPor->InTxtGrp() )
            nBlank1 += ((SwTxtPortion*)pPor)->GetSpaceCnt( rInf, nNull );
        rInf.SetIdx( rInf.GetIdx() + pPor->GetLen() );
        if( pPor->InTabGrp() )
            bTabulator = sal_True;
    }
    nLineDiff = aRoot.Width();
    if( aRoot.GetNext() )
    {
        pPor = aRoot.GetNext()->GetFirstPortion();
        nLineDiff -= aRoot.GetNext()->Width();
    }
    for( nBlank2 = 0; pPor; pPor = pPor->GetPortion() )
    {
        if( pPor->InTxtGrp() )
            nBlank2 += ((SwTxtPortion*)pPor)->GetSpaceCnt( rInf, nNull );
        rInf.SetIdx( rInf.GetIdx() + pPor->GetLen() );
        if( pPor->InTabGrp() )
            bTabulator = sal_True;
    }
    rInf.SetIdx( nStart );
}

long SwMultiPortion::CalcSpacing( short nSpaceAdd, const SwTxtSizeInfo &rInf ) const
{
    return HasTabulator() ? 0 : GetSpaceCnt() * nSpaceAdd;
}

sal_Bool SwMultiPortion::ChangeSpaceAdd( SwLineLayout* pCurr, short nSpaceAdd )
{
    sal_Bool bRet = sal_False;
    if( !HasTabulator() && nSpaceAdd > 0 )
    {
        if( pCurr->IsNoSpaceAdd() )
        {
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

void SwMultiPortion::ResetSpaceAdd( SwLineLayout* pCurr )
{
    pCurr->GetSpaceAdd().Remove(0);
    if( !pCurr->GetSpaceAdd().Count() )
        pCurr->FinishSpaceAdd();
}


/*-----------------13.10.00 16:22-------------------
 * If we're inside a two-line-attribute,
 * the attribute will be returned,
 * otherwise the function returns zero.
 * --------------------------------------------------*/

const SwTxtAttr* SwTxtSizeInfo::GetTwoLines( const xub_StrLen nPos ) const
{
    const SwpHints *pHints = pFrm->GetTxtNode()->GetpSwpHints();
    if( !pHints )
        return NULL;
    for( MSHORT i = 0; i < pHints->Count(); ++i )
    {
        const SwTxtAttr *pRet = (*pHints)[i];
        xub_StrLen nStart = *pRet->GetStart();
        if( nPos < nStart )
            break;
        if( RES_TXTATR_TWO_LINES == pRet->Which()
#ifdef FOR_YOUR_OWN_RISK
            || RES_CHRATR_UNDERLINE == pRet->Which()
#endif
            )
        {
            if( nPos == nStart || *pRet->GetEnd() > nPos )
                return pRet;
        }
    }
    return NULL;
}

/*-----------------13.10.00 16:24-------------------
 * SwTxtPainter::PaintMultiPortion manages the paint for a SwMultiPortion.
 * External, for the calling function, it seems to be a normal Paint-function,
 * internal it is like a SwTxtFrm::Paint with multiple DrawTextLines
 * --------------------------------------------------*/

void SwTxtPainter::PaintMultiPortion( const SwRect &rPaint,
    SwMultiPortion& rMulti )
{
    // old values must be saved and restored at the end
    xub_StrLen nOldLen = GetInfo().GetLen();
    KSHORT nOldX = GetInfo().X();
    KSHORT nOldY = GetInfo().Y();
    xub_StrLen nOldIdx = GetInfo().GetIdx();
    SvShorts *pOldSpaceAdd = GetInfo().GetpSpaceAdd();
    MSHORT nOldSpIdx = GetInfo().GetSpaceIdx();
    sal_Bool bSpaceChg = sal_False;
    short nSpaceAdd = ( pOldSpaceAdd && !rMulti.HasTabulator() ) ?
                      GetInfo().GetSpaceAdd() : 0;
    if( rMulti.GetRoot().GetpSpaceAdd() )
    {
        GetInfo().SetSpaceAdd( rMulti.GetRoot().GetpSpaceAdd() );
        GetInfo().ResetSpaceIdx();
        bSpaceChg = rMulti.ChangeSpaceAdd( &rMulti.GetRoot(), nSpaceAdd );
    }
    else if( rMulti.HasTabulator() )
        GetInfo().SetSpaceAdd( NULL );
    if( rMulti.HasBrackets() )
    {
        SeekAndChg( GetInfo() );
        rMulti.PaintBracket( GetInfo(), sal_True );
        GetInfo().X( GetInfo().X() + rMulti.PreWidth() );
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
        if( ( bRest && pPor->InFldGrp() && !pPor->GetLen() ) )
            SeekAndChgBefore( GetInfo() );
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
            if( bSpaceChg )
            {
                GetInfo().GetpSpaceAdd()->Remove( 0 );
                bSpaceChg = sal_False;
            }
            if( pLay->GetpSpaceAdd() )
            {
                GetInfo().SetSpaceAdd( pLay->GetpSpaceAdd() );
                GetInfo().ResetSpaceIdx();
                bSpaceChg = rMulti.ChangeSpaceAdd( pLay, nSpaceAdd );
            }
            else
            {
                GetInfo().SetSpaceAdd( rMulti.HasTabulator() ? 0:pOldSpaceAdd );
                GetInfo().SetSpaceIdx( nOldSpIdx);
            }
            // We switch to the baseline of the next inner line
            GetInfo().Y( GetInfo().Y() + rMulti.GetRoot().Height()
                - rMulti.GetRoot().GetAscent() + pLay->GetAscent() );
        }
    } while( pPor );

    if( bSpaceChg )
    {
        GetInfo().GetpSpaceAdd()->Remove( 0 );
        bSpaceChg = sal_False;
    }
    GetInfo().SetIdx( nOldIdx );
    GetInfo().Y( nOldY );

    if( rMulti.HasBrackets() )
    {
        SeekAndChg( GetInfo() );
        GetInfo().X( nOldX + rMulti.Width() - rMulti.PostWidth() +
            ( nSpaceAdd > 0 ? rMulti.CalcSpacing( nSpaceAdd, GetInfo() ) :0 ) );
        rMulti.PaintBracket( GetInfo(), sal_False );
    }
    // Restore the saved values
    GetInfo().X( nOldX );
    GetInfo().SetLen( nOldLen );
    GetInfo().SetSpaceAdd( pOldSpaceAdd );
    GetInfo().SetSpaceIdx( nOldSpIdx);
}

/*-----------------13.10.00 16:46-------------------
 * SwTxtFormatter::BuildMultiPortion manages the formatting of a SwMultiPortion.
 * External, for the calling function, it seems to be a normal Format-function,
 * internal it is like a SwTxtFrm::_Format with multiple BuildPortions
 * --------------------------------------------------*/

BOOL SwTxtFormatter::BuildMultiPortion( SwTxtFormatInfo &rInf,
    SwMultiPortion& rMulti )
{
    SwTwips nMaxWidth = rInf.Width();
    SeekAndChg( rInf );
    if( rMulti.HasBrackets() )
        rMulti.FormatBrackets( rInf, nMaxWidth );

    SwTwips nTmpX = rInf.X();

    pMulti = &rMulti;
    SwLineLayout *pOldCurr = pCurr;
    xub_StrLen nOldStart = GetStart();
    SwTwips nMinWidth = nTmpX + 1;
    SwTwips nActWidth = nMaxWidth;
    SwTxtFormatInfo aInf( rInf, rMulti.GetRoot(), nActWidth );
    xub_StrLen nStartIdx = aInf.GetIdx();
    xub_StrLen nMultiLen = rMulti.GetLen() - rInf.GetIdx();
    SwLinePortion *pRest = NULL;
    BOOL bRet = FALSE;
    do
    {
        pCurr = &rMulti.GetRoot();
        nStart = nStartIdx;
        bRet = FALSE;
        FormatReset( aInf );
        aInf.X( nTmpX );
        aInf.Width( nActWidth );
        if( rMulti.GetFldRest() )
        {
            SwFldPortion *pFld =
                rMulti.GetFldRest()->Clone( rMulti.GetFldRest()->GetExp() );
            pFld->SetFollow( sal_True );
            aInf.SetRest( pFld );
        }
        BuildPortions( aInf );
        rMulti.CalcSize( *this );
        pCurr->SetRealHeight( pCurr->Height() );
        if( pCurr->GetLen() < nMultiLen || aInf.GetRest() )
        {
            xub_StrLen nFirstLen = pCurr->GetLen();
            pCurr->SetNext( new SwLineLayout() );
            pCurr = pCurr->GetNext();
            nStart = aInf.GetIdx();
            aInf.X( nTmpX );
            SwTxtFormatInfo aTmp( aInf, *pCurr, nActWidth );
            aTmp.SetRest( aInf.GetRest() );
            aInf.SetRest( NULL );
            BuildPortions( aTmp );
            rMulti.CalcSize( *this );
            pCurr->SetRealHeight( pCurr->Height() );
            pRest = aTmp.GetRest();
            if( nFirstLen + pCurr->GetLen() < nMultiLen || pRest )
                bRet = TRUE;
        }
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
        delete pRest;
        pRest = NULL;
    } while ( TRUE );
    pMulti = NULL;
    pCurr = pOldCurr;
    nStart = nOldStart;
    rMulti.SetLen( rMulti.GetRoot().GetLen() + ( rMulti.GetRoot().GetNext() ?
        rMulti.GetRoot().GetNext()->GetLen() : 0 ) );
    rMulti.CalcBlanks( rInf );
    if( rMulti.GetLineDiff() )
    {
        SwLineLayout* pLine = &rMulti.GetRoot();
        if( rMulti.GetLineDiff() > 0 )
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
    if( rMulti.HasBrackets() )
        rMulti.Width( rMulti.Width() + rMulti.BracketWidth() );

    if( bRet )
    {
        SwMultiPortion *pTmp = new SwMultiPortion( nMultiLen + rInf.GetIdx() );
        if( rMulti.HasBrackets() )
        {
            pTmp->SetBrackets( rMulti );
            // An empty multiportion needs no brackets.
            // Notice: GetLen() might be zero, if the multiportion contains
            // the second part of a field and the width might be zero, if
            // it contains a note only. In this cases the brackets are okay.
            // But if the length and the width are both zero, the multiportion
            // is really empty.
            if( !rMulti.GetLen() && rMulti.Width() == rMulti.BracketWidth() )
                rMulti.ClearBrackets();
        }
        ASSERT( !pRest || pRest->InFldGrp(),
            "BuildMultiPortion: Surprising restportion, field exspected" );
        pTmp->SetFldRest( (SwFldPortion*) pRest );
        rInf.SetRest( pTmp );
    }
    return bRet;
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
    bSpaceChg = pMulti->ChangeSpaceAdd( pTxtCursor->pCurr, nSpaceAdd );
    nWidth = pTxtCursor->pCurr->Width();
    if( nSpaceAdd > 0 && !pMulti->HasTabulator() )
        pTxtCursor->pCurr->Width( nWidth + nSpaceAdd * pMulti->GetSpaceCnt() );
}

SwTxtCursorSave::~SwTxtCursorSave()
{
    if( bSpaceChg )
        SwMultiPortion::ResetSpaceAdd( pTxtCrsr->pCurr );
    pTxtCrsr->pCurr->Width( nWidth );
    pTxtCrsr->pCurr = pCurr;
    pTxtCrsr->nStart = nStart;
}

