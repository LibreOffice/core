/*************************************************************************
 *
 *  $RCSfile: txtdrop.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:26 $
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

#ifndef _SV_METRIC_HXX //autogen
#include <vcl/metric.hxx>
#endif
#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>   // Format()
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>  // SwViewOption
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>   // ViewShell
#endif
#ifndef _PORDROP_HXX
#include <pordrop.hxx>
#endif
#ifndef _ITRFORM2_HXX
#include <itrform2.hxx>
#endif
#ifndef _SWFONT_HXX
#include <swfont.hxx>
#endif
#ifndef _TXTPAINT_HXX
#include <txtpaint.hxx> // SwSaveClip
#endif
#ifndef _TXTFLY_HXX
#include <txtfly.hxx>   // Format()
#endif
#ifndef _BLINK_HXX
#include <blink.hxx>    // pBlink
#endif
#ifndef _TXATBASE_HXX
#include <txatbase.hxx>
#endif

/*************************************************************************
 *                SwDropPortion CTor, DTor
 *************************************************************************/

SwDropPortion::SwDropPortion( SwFont *pF, const MSHORT nLineCnt,
                                          const KSHORT nDropHeight,
                                          const KSHORT nDropDescent,
                                          const KSHORT nDistance )
  : pFnt( pF ),
    nLines( nLineCnt ),
    nDropHeight(nDropHeight),
    nDropDescent(nDropDescent),
    nDistance(nDistance),
    nX(0),
    nY(0),
    nFix(0)
{
    SetWhichPor( POR_DROP );
}



SwDropPortion::~SwDropPortion()
{
    delete pFnt;
    if( pBlink )
        pBlink->Delete( this );
}

sal_Bool SwTxtSizeInfo::_HasHint( const SwTxtNode* pTxtNode, xub_StrLen nPos )
{
    const SwpHints *pHints = pTxtNode->GetpSwpHints();
    if( !pHints )
        return sal_False;
    for( MSHORT i = 0; i < pHints->Count(); ++i )
    {
        const SwTxtAttr *pPos = (*pHints)[i];
        xub_StrLen nStart = *pPos->GetStart();
        if( nPos < nStart )
            return sal_False;
        if( nPos == nStart && !pPos->GetEnd() )
            return sal_True;
    }
    return sal_False;
}

/*************************************************************************
 *                    SwTxtNode::GetDropLen()
 *************************************************************************/

MSHORT SwTxtNode::GetDropLen( MSHORT nWishLen ) const
{
    xub_StrLen nEnd = GetTxt().Len();
    if( nWishLen && nWishLen < nEnd )
        nEnd = nWishLen;
    xub_StrLen i = 0;
    for( ; i < nEnd; ++i )
    {
        xub_Unicode cChar = GetTxt().GetChar( i );
        if( CH_TAB == cChar || CH_BREAK == cChar ||
            ( !nWishLen && CH_BLANK == cChar ) ||
            (( CH_TXTATR_BREAKWORD == cChar || CH_TXTATR_INWORD == cChar )
                && SwTxtSizeInfo::_HasHint( this, i ) ) )
            break;
    }
    return i;
}

/*************************************************************************
 *                    SwDropPortion::PaintTxt()
 *************************************************************************/

// Die Breite manipulieren, sonst werden die Buchstaben gestretcht



void SwDropPortion::PaintTxt( const SwTxtPaintInfo &rInf /*, const sal_Bool bBack*/ )
    const
{
    if ( rInf.OnWin() )
    {
        if( rInf.GetOpt().IsField() )
            rInf.DrawBackground( *this );
//      else if ( bBack )
//          rInf.DrawRect( *this );
    }
    ((SwDropPortion*)this)->Width( Width() - nDistance );
    SwTxtPortion::Paint( rInf );
    ((SwDropPortion*)this)->Width( Width() + nDistance );
}

/*************************************************************************
 *                   SwDropPortion::Paint()
 *************************************************************************/


void SwDropPortion::PaintDrop( const SwTxtPaintInfo &rInf ) const
{
    // ganz normale Ausgabe
    if( !nDropHeight || !pFnt || nLines == 1 )
    {
        SwFontSave aTmp( rInf, pFnt );
        PaintTxt( rInf /*, sal_False */);
        return;
    }

    // Luegenwerte einstellen!
    const KSHORT nOldHeight = Height();
    const KSHORT nOldWidth  = Width();
    const KSHORT nOldAscent = GetAscent();
    const SwTwips nOldPosY  = rInf.Y();
    const KSHORT nOldPosX   = rInf.X();
    const SwParaPortion *pPara = rInf.GetParaPortion();
    const Point aOutPos( nOldPosX + nX, nOldPosY - pPara->GetAscent()
                         - pPara->GetRealHeight() + pPara->Height() );
    // Retusche nachholen.

    ((SwTxtPaintInfo&)rInf).Y( aOutPos.Y() + nDropHeight + nY );
    ((SwDropPortion*)this)->Height( nDropHeight+nDropDescent );
    ((SwDropPortion*)this)->Width( Width() - nX );
    ((SwDropPortion*)this)->SetAscent( nDropHeight + nY );

    // Clipregion auf uns einstellen!
    // Und zwar immer, und nie mit dem bestehenden ClipRect
    // verrechnen, weil dies auf die Zeile eingestellt sein koennte.

    SwRect aClipRect;
    if ( rInf.OnWin() )
    {
        aClipRect = SwRect( aOutPos, SvLSize() );
        aClipRect.Intersection( rInf.GetPaintRect() );
    }
    SwSaveClip aClip( (OutputDevice*)rInf.GetOut() );
    aClip.ChgClip( aClipRect );

    // Das machen, was man sonst nur macht ...
    SwFontSave aSave( rInf, pFnt );
    PaintTxt( rInf /*, sal_True */);

    // Alte Werte sichern
    ((SwDropPortion*)this)->Height( nOldHeight );
    ((SwDropPortion*)this)->Width( nOldWidth );
    ((SwDropPortion*)this)->SetAscent( nOldAscent );
    ((SwTxtPaintInfo&)rInf).Y( nOldPosY );
    ((SwTxtPaintInfo&)rInf).X( nOldPosX );

}

/*************************************************************************
 *              virtual SwDropPortion::Paint()
 *************************************************************************/


void SwDropPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    // ganz normale Ausgabe wird hier erledigt.
    if( !nDropHeight || !pFnt )
        PaintTxt( rInf /*, sal_False */);
}

/*************************************************************************
 *                virtual Format()
 *************************************************************************/


sal_Bool SwDropPortion::FormatTxt( SwTxtFormatInfo &rInf )
{
    const xub_StrLen nOldLen = GetLen();
    const xub_StrLen nOldInfLen = rInf.GetLen();
    const sal_Bool bFull = SwTxtPortion::Format( rInf );
    if( bFull )
    {
        // sieht zwar Scheisse aus, aber was soll man schon machen?
        rInf.SetUnderFlow( 0 );
        Truncate();
        SetLen( nOldLen );
        rInf.SetLen( nOldInfLen );
    }
    return bFull;
}

/*************************************************************************
 *                virtual GetTxtSize()
 *************************************************************************/


SwPosSize SwDropPortion::GetTxtSize( const SwTxtSizeInfo &rInf ) const
{
    SwFontSave aSave( rInf, pFnt );
    return SwTxtPortion::GetTxtSize( rInf );
}

/*************************************************************************
 *                virtual GetCrsrOfst()
 *************************************************************************/

xub_StrLen SwDropPortion::GetCrsrOfst( const KSHORT nOfst ) const
{
    return 0;
}

#ifdef OLDRECYCLE
/*************************************************************************
 *                virtual SwDropPortion::MayRecycle()
 *************************************************************************/


sal_Bool SwDropPortion::MayRecycle() const
{
    return sal_False;
}
#endif

/*************************************************************************
 *                SwTxtFormatter::CalcDropHeight()
 *************************************************************************/



void SwTxtFormatter::CalcDropHeight( const MSHORT nLines )
{
    const SwLinePortion *const pOldCurr = GetCurr();
    KSHORT nDropHght = 0;
    KSHORT nAscent = 0;
    KSHORT nHeight = 0;
    KSHORT nDropLns = 0;
    sal_Bool bRegisterOld = IsRegisterOn();
    bRegisterOn = sal_False;

    Top();

    while( GetCurr()->IsDummy() )
    {
        if ( !Next() )
            break;
    }

    // Wenn wir nur eine Zeile haben returnen wir 0
    if( GetNext() || GetDropLines() == 1 )
    {
        for( ; nDropLns < nLines; nDropLns++ )
        {
            if ( GetCurr()->IsDummy() )
                break;
            else
            {
                CalcAscentAndHeight( nAscent, nHeight );
                nDropHght += nHeight;
                bRegisterOn = bRegisterOld;
            }
            if ( !Next() )
            {
                nDropLns++; // Fix: 11356
                break;
            }
        }

        // In der letzten Zeile plumpsen wir auf den Zeilenascent!
        nDropHght -= nHeight;
        nDropHght += nAscent;
        Top();
    }
    bRegisterOn = bRegisterOld;
    SetDropDescent( nHeight - nAscent );
    SetDropHeight( nDropHght );
    SetDropLines( nDropLns );
    // Alte Stelle wiederfinden!
    while( pOldCurr != GetCurr() )
    {
        if( !Next() )
        {
            ASSERT( !this, "SwTxtFormatter::_CalcDropHeight: left Toulouse" );
            break;
        }
    }
}

/*************************************************************************
 *                SwTxtFormatter::GuessDropHeight()
 *
 *  Wir schaetzen mal, dass die Fonthoehe sich nicht aendert und dass
 *  erst mindestens soviele Zeilen gibt, wie die DropCap-Einstellung angibt.
 *
 *************************************************************************/



void SwTxtFormatter::GuessDropHeight( const MSHORT nLines )
{
    ASSERT( nLines, "GuessDropHeight: Give me more Lines!" );
    KSHORT nAscent = 0;
    KSHORT nHeight = 0;
    SetDropLines( nLines );
    if ( GetDropLines() > 1 )
    {
        CalcRealHeight();
        CalcAscentAndHeight( nAscent, nHeight );
    }
    SetDropDescent( nHeight - nAscent );
    SetDropHeight( nHeight * nLines - GetDropDescent() );
}

/*************************************************************************
 *                SwTxtFormatter::NewDropPortion
 *************************************************************************/


SwDropPortion *SwTxtFormatter::NewDropPortion( SwTxtFormatInfo &rInf ) const
{
    if( !pDropFmt )
        return 0;

    xub_StrLen nPorLen = pDropFmt->GetWholeWord() ? 0 : pDropFmt->GetChars();
    nPorLen = pFrm->GetTxtNode()->GetDropLen( nPorLen );
    if( !nPorLen )
    {
        ((SwTxtFormatter*)this)->ClearDropFmt();
        return 0;
    }

    SwDropPortion *pDropPor = 0;

    // erste oder zweite Runde?
    if ( !( GetDropHeight() || IsOnceMore() ) )
    {
        if ( GetNext() )
            ((SwTxtFormatter*)this)->CalcDropHeight( pDropFmt->GetLines() );
        else
            ((SwTxtFormatter*)this)->GuessDropHeight( pDropFmt->GetLines() );
    }
    SwFont *pTmpFnt = new SwFont( pDropFmt->GetCharFmt()
                                 ? &pDropFmt->GetCharFmt()->GetAttrSet()
                                 : &rInf.GetCharAttr() );
    if( GetDropHeight() )
        pDropPor = new SwDropPortion( pTmpFnt, GetDropLines(),
                                 GetDropHeight(), GetDropDescent(),
                                 pDropFmt->GetDistance() );
    else
        pDropPor = new SwDropPortion( pTmpFnt,0,0,0,pDropFmt->GetDistance() );

    pDropPor->SetLen( nPorLen );
    ((SwTxtFormatter*)this)->SetPaintDrop( sal_True );
    return pDropPor;
}

/*************************************************************************
 *                SwTxtPainter::PaintDropPortion()
 *************************************************************************/



void SwTxtPainter::PaintDropPortion()
{
    const SwDropPortion *pDrop = GetInfo().GetParaPortion()->FindDropPortion();
    ASSERT( pDrop, "DrapCop-Portion not available." );
    if( !pDrop )
        return;

    const SwTwips nOldY = GetInfo().Y();

    Top();

    GetInfo().SetSpaceAdd( pCurr->GetpSpaceAdd() );
    GetInfo().ResetSpaceIdx();

    // 8047: Drops und Dummies
    while( !pCurr->GetLen() && Next() )
        ;

    // MarginPortion und Adjustment!
    const SwLinePortion *pPor = pCurr->GetFirstPortion();
    KSHORT nX = 0;
    while( pPor && !pPor->IsDropPortion() )
    {
        nX += pPor->Width();
        pPor = pPor->GetPortion();
    }
    Point aLineOrigin( GetTopLeft() );

#ifdef NIE
    // Retusche nachholen...
    if( nX )
    {
        const Point aPoint( Left(), Y() );
        const Size  aSize( nX - 1, GetDropHeight()+GetDropDescent() );
        SwRect aRetouche( aPoint, aSize );
        GetInfo().DrawRect( aRetouche );
    }
#endif

    aLineOrigin.X() += nX;
    KSHORT nTmpAscent, nTmpHeight;
    CalcAscentAndHeight( nTmpAscent, nTmpHeight );
    aLineOrigin.Y() += nTmpAscent;
    GetInfo().SetIdx( GetStart() );
    GetInfo().SetPos( aLineOrigin );
    GetInfo().SetLen( pDrop->GetLen() );

    pDrop->PaintDrop( GetInfo() );

    GetInfo().Y( nOldY );
}

/*************************************************************************
 * Da die Berechnung der Fontgroesse der Initialen ein teures Geschaeft ist,
 * wird dies durch einen DropCapCache geschleust.
 *************************************************************************/

#define DROP_CACHE_SIZE 10

class SwDropCapCache
{
    long aMagicNo[ DROP_CACHE_SIZE ];
    XubString aTxt[ DROP_CACHE_SIZE ];
    Size aSize[ DROP_CACHE_SIZE ];
    KSHORT aWishedHeight[ DROP_CACHE_SIZE ];
    short aDescent[ DROP_CACHE_SIZE ];
    MSHORT nIndex;
public:
    SwDropCapCache();
    ~SwDropCapCache(){}
    void CalcFontSize( SwDropPortion* pDrop, SwTxtFormatInfo &rInf );
};



SwDropCapCache::SwDropCapCache() : nIndex( 0 )
{
    memset( &aMagicNo, 0, sizeof(aMagicNo) );
    memset( &aWishedHeight, 0, sizeof(aWishedHeight) );
}



void SwDropCapCache::CalcFontSize( SwDropPortion* pDrop, SwTxtFormatInfo &rInf )
{
    const void* pFntNo;
    MSHORT nTmpIdx;
    SwFont *pFnt = pDrop->pFnt;
    pFnt->GetMagic( pFntNo, nTmpIdx, pFnt->GetActual() );
    XubString aStr( rInf.GetTxt(), rInf.GetIdx(), pDrop->GetLen() );

    nTmpIdx = 0;
    while( nTmpIdx < DROP_CACHE_SIZE &&
        ( aTxt[ nTmpIdx ] != aStr || aMagicNo[ nTmpIdx ] != long(pFntNo) ||
          aWishedHeight[ nTmpIdx ] != pDrop->GetDropHeight() ) )
        ++nTmpIdx;
    if( nTmpIdx >= DROP_CACHE_SIZE )
    {
        ++nIndex;
        nIndex %= DROP_CACHE_SIZE;
        nTmpIdx = nIndex;

        aMagicNo[ nTmpIdx ] = long(pFntNo);
        aTxt[ nTmpIdx ] = aStr;
        long nWishedHeight = pDrop->GetDropHeight();

        Size aNewSize = Size( 0, nWishedHeight );

        aSize[ nTmpIdx ] = aNewSize;

        long nAscent = 0;
        long nDescent = 0;

        OutputDevice *pOut = rInf.GetOut();

        xub_StrLen nLen = aStr.Len();

        sal_Bool bGrow = ( nLen != 0 );
#ifdef DEBUG
        long nGrow = 0;
#endif
        long nMin, nMax = KSHRT_MAX;
        nMin = aNewSize.Height()/2;

        sal_Bool bWinUsed = sal_False;
        Font aOldFnt;
        MapMode aOldMap( MAP_TWIP );
        OutputDevice *pWin;
        if( rInf.GetVsh() && rInf.GetVsh()->GetWin() )
            pWin = rInf.GetVsh()->GetWin();
        else
            pWin = GetpApp()->GetDefaultDevice();
        while( bGrow )
        {
            pFnt->SetSize( aNewSize, pFnt->GetActual() );
            pFnt->ChgPhysFnt( rInf.GetVsh(), pOut );
            nAscent = pFnt->GetAscent( rInf.GetVsh(), pOut );

            // Wir besorgen uns das alle Buchstaben umfassende Rechteck:
            Rectangle aRect, aTmp;
            sal_Bool bTakeRect = sal_False;
            for ( xub_StrLen i = 0; i < nLen; i++ )
            {
                if( pOut->GetGlyphBoundRect( aStr.GetChar(i), aTmp, sal_False )
                    && !aTmp.IsEmpty() )
                {
                    if ( bTakeRect )
                        aRect.Union( aTmp );
                    else
                    {
                        aRect = aTmp;
                        bTakeRect = sal_True;
                    }
                }
            }
            if ( bTakeRect )
            {
                // Der tiefste Punkt der Buchstaben
                nDescent = aRect.Bottom()-nAscent;
                // Der hoechste Punkt der Buchstaben
                nAscent = nAscent-aRect.Top();
            }
            // Wenn kein Rectangle ermittelt werden konnte, nehmen wir weiterhin
            // einfach den Ascent mit all den bekannten Folgen (Q,g etc.)
            else
            {
                if ( pWin )
                {
                    if ( bWinUsed )
                        pWin->SetFont( pFnt->GetActualFont() );
                    else
                    {
                        bWinUsed = sal_True;
                        aOldMap = pWin->GetMapMode( );
                        pWin->SetMapMode( MapMode( MAP_TWIP ) );
                        aOldFnt = pWin->GetFont();
                        pWin->SetFont( pFnt->GetActualFont() );
                    }
                    for ( xub_StrLen i = 0; i < nLen; i++ )
                    {
                        if( pWin->GetGlyphBoundRect( aStr.GetChar(i), aTmp, sal_False )
                            && !aTmp.IsEmpty() )
                        {
                            if ( bTakeRect )
                                aRect.Union( aTmp );
                            else
                            {
                                aRect = aTmp;
                                bTakeRect = sal_True;
                            }
                        }
                    }
                }
                if ( bTakeRect )
                {
                    FontMetric aWinMet( pWin->GetFontMetric() );
                    nAscent = (KSHORT) aWinMet.GetAscent();
                    // Der tiefste Punkt der Buchstaben
                    nDescent = aRect.Bottom()-nAscent;
                    // Der hoechste Punkt der Buchstaben
                    nAscent = nAscent-aRect.Top();
                }
                else
                    nDescent = 0; // nAscent stimmt eh noch.
            }
            const long nHght = nAscent + nDescent;
            if ( nHght )
            {
                if ( nHght > nWishedHeight )
                    nMax = aNewSize.Height();
                else
                {
                    aSize[ nTmpIdx ] = aNewSize;
                    nMin = aNewSize.Height();
                }
                const long nNewHeight = ( aNewSize.Height() * nWishedHeight )
                                        / nHght;
                bGrow = ( nNewHeight > nMin ) && ( nNewHeight < nMax );
#ifdef DEBUG
                if ( bGrow )
                    nGrow++;
#endif
                aNewSize.Height() = KSHORT( nNewHeight );
            }
            else
                bGrow = sal_False;
        }
        if ( bWinUsed )
        {
            nDescent += pFnt->GetLeading( rInf.GetVsh(), pWin );
            pWin->SetMapMode( aOldMap );
            pWin->SetFont( aOldFnt );
        }
        else
            nDescent += pFnt->GetLeading( rInf.GetVsh(), pOut );

        aDescent[ nTmpIdx ] = -short( nDescent );
        aWishedHeight[ nTmpIdx ] = KSHORT(nWishedHeight);
    }
    pDrop->nY = aDescent[ nTmpIdx ];
    pFnt->SetSize( aSize[ nTmpIdx ], pFnt->GetActual() );
}



void SwDropPortion::DeleteDropCapCache()
{
    delete pDropCapCache;
}

/*************************************************************************
 *                virtual Format()
 *************************************************************************/


sal_Bool SwDropPortion::Format( SwTxtFormatInfo &rInf )
{
    sal_Bool bFull = sal_False;
    Fix( rInf.X() );
    if( nDropHeight && pFnt && nLines!=1 )
    {
        pFnt->ChkMagic( rInf.GetVsh(), pFnt->GetActual() );
        if( !pDropCapCache )
            pDropCapCache = new SwDropCapCache();
        pDropCapCache->CalcFontSize( this, rInf );
        {
            SwFontSave aSave( rInf, pFnt );
            bFull = FormatTxt( rInf );
            if( !bFull )
            {
                // 7631, 7633: bei Ueberlappungen mit Flys ist Schluss.
                const SwTxtFly *pTxtFly = rInf.GetTxtFly();
                if( pTxtFly && pTxtFly->IsOn() )
                {
                    SwRect aRect( rInf.GetTxtFrm()->Frm().Pos(), SvLSize() );
                    aRect.Height( nDropHeight );
                    aRect.Pos() += rInf.GetTxtFrm()->Prt().Pos();
                    aRect.Pos().X() += rInf.X();
                    aRect.Pos().Y() = rInf.Y();
                    aRect = pTxtFly->GetFrm( aRect );
                    bFull = aRect.HasArea();
                }
            }
        }
        if( bFull )
        {
            // Durch FormatTxt kann nHeight auf 0 gesetzt worden sein
            if ( !Height() )
                Height( rInf.GetTxtHeight() );

            bFull = SwTxtPortion::Format( rInf );
            if ( !bFull )
            {
                // Jetzt noch einmal der ganze Spass
                nDropHeight = nLines = 0;
                delete pFnt;
                pFnt = NULL;
            }
//          bFull = FormatTxt( rInf );
        }
        else
            rInf.SetDropInit( sal_True );

        Height( rInf.GetTxtHeight() );
        SetAscent( rInf.GetAscent() );
    }
    else
        bFull = SwTxtPortion::Format( rInf );
//      bFull = FormatTxt( rInf );

    if( bFull )
        nDistance = 0;
    else
    {
        const KSHORT nWant = Width() + GetDistance();
        const KSHORT nRest = rInf.Width() - rInf.X();
        if( nWant > nRest )
        //  Robust: Kann die gewuenschte Distance nicht eingehalten werden,
        //          gibt es gar keine!
        //  nDistance = nWant - nRest;
            nDistance = 0;

        Width( Width() + nDistance );
    }
    return bFull;
}




