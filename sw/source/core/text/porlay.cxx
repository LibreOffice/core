/*************************************************************************
 *
 *  $RCSfile: porlay.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: fme $ $Date: 2001-08-30 11:47:34 $
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

#include "errhdl.hxx"   // ASSERT

#include "txtcfg.hxx"
#include "porlay.hxx"
#include "itrform2.hxx"
#include "porglue.hxx"
#include "porexp.hxx"   // SwQuoVadisPortion
#include "blink.hxx"    // pBlink
#include "redlnitr.hxx" // SwRedlineItr
#include "porfly.hxx"   // SwFlyCntPortion
#ifndef _PORRST_HXX
#include <porrst.hxx>       // SwHangingPortion
#endif
#ifndef _PORMULTI_HXX
#include <pormulti.hxx>     // SwMultiPortion
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif
#ifndef _DRAWFONT_HXX
#include <drawfont.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif

using namespace ::com::sun::star;

/*************************************************************************
 *                 SwLineLayout::~SwLineLayout()
 *
 * class SwLineLayout: Das Layout einer einzelnen Zeile. Dazu
 * gehoeren vor allen Dingen die Dimension, die Anzahl der
 * Character und der Wortzwischenraeume in der Zeile.
 * Zeilenobjekte werden in einem eigenen Pool verwaltet, um zu
 * erreichen, dass sie im Speicher moeglichst beeinander liegen
 * (d.h. zusammen gepaged werden und den Speicher nicht
 * fragmentieren).
 *************************************************************************/

SwLineLayout::~SwLineLayout()
{
    Truncate();
    if( GetNext() )
        delete GetNext();
    if( pBlink )
        pBlink->Delete( this );
    delete pSpaceAdd;
    if ( pKanaComp )
        delete pKanaComp;
}

/*************************************************************************
 *                virtual SwLineLayout::Insert()
 *************************************************************************/

SwLinePortion *SwLineLayout::Insert( SwLinePortion *pIns )
{
    // Erster Attributwechsel, Masse und Laengen
    // aus *pCurr in die erste Textportion kopieren.
    if( !pPortion )
    {
        if( GetLen() )
        {
            pPortion = new SwTxtPortion( *(SwLinePortion*)this );
            if( IsBlinking() && pBlink )
            {
                SetBlinking( sal_False );
                pBlink->Replace( this, pPortion );
            }
        }
        else
        {
            SetPortion( pIns );
            return pIns;
        }
    }
    // mit Skope aufrufen, sonst Rekursion !
    return pPortion->SwLinePortion::Insert( pIns );
}

/*************************************************************************
 *                virtual SwLineLayout::Append()
 *************************************************************************/

SwLinePortion *SwLineLayout::Append( SwLinePortion *pIns )
{
    // Erster Attributwechsel, Masse und Laengen
    // aus *pCurr in die erste Textportion kopieren.
    if( !pPortion )
        pPortion = new SwTxtPortion( *(SwLinePortion*)this );
    // mit Skope aufrufen, sonst Rekursion !
    return pPortion->SwLinePortion::Append( pIns );
}

/*************************************************************************
 *                virtual SwLineLayout::Format()
 *************************************************************************/

// fuer die Sonderbehandlung bei leeren Zeilen

sal_Bool SwLineLayout::Format( SwTxtFormatInfo &rInf )
{
    if( GetLen() )
        return SwTxtPortion::Format( rInf );
    else
    {
        Height( rInf.GetTxtHeight() );
        return sal_True;
    }
}

/*************************************************************************
 *                    SwLineLayout::CalcLeftMargin()
 *
 * Wir sammeln alle FlyPortions am Anfang der Zeile zu einer MarginPortion.
 *************************************************************************/

SwMarginPortion *SwLineLayout::CalcLeftMargin()
{
    SwMarginPortion *pLeft = (GetPortion() && GetPortion()->IsMarginPortion()) ?
        (SwMarginPortion *)GetPortion() : 0;
    if( !GetPortion() )
         SetPortion( new SwTxtPortion( *(SwLinePortion*)this ) );
    if( !pLeft )
    {
        pLeft = new SwMarginPortion( 0 );
        pLeft->SetPortion( GetPortion() );
        SetPortion( pLeft );
    }
    else
    {
        pLeft->Height( 0 );
        pLeft->Width( 0 );
        pLeft->SetLen( 0 );
        pLeft->SetAscent( 0 );
        pLeft->SetPortion( NULL );
        pLeft->SetFixWidth(0);
    }

    SwLinePortion *pPos = pLeft->GetPortion();
    while( pPos )
    {
        DBG_LOOP;
        if( pPos->IsFlyPortion() )
        {
            // Die FlyPortion wird ausgesogen ...
            pLeft->Join( (SwGluePortion*)pPos );
            pPos = pLeft->GetPortion();
            if( GetpKanaComp() )
                GetKanaComp().Remove( 0, 1 );
        }
        else
            pPos = 0;
    }
    return pLeft;
}

/*************************************************************************
 *                    SwLineLayout::CreateSpaceAdd()
 *************************************************************************/

void SwLineLayout::CreateSpaceAdd( const short nInit )
{
    pSpaceAdd = new SvShorts;
    pSpaceAdd->Insert( nInit, 0 );
}

/*************************************************************************
 *                    SwLineLayout::CalcLine()
 *
 * Aus FormatLine() ausgelagert.
 *************************************************************************/

void SwLineLayout::CalcLine( SwTxtFormatter &rLine, SwTxtFormatInfo &rInf )
{
    const KSHORT nLineWidth = rInf.RealWidth();

    KSHORT nFlyAscent;
    KSHORT nFlyHeight;
    KSHORT nFlyDescent;
    sal_Bool bOnlyPostIts = sal_True;
    SetHanging( sal_False );

    sal_Bool bTmpDummy = ( 0 == GetLen() );
    SwFlyCntPortion* pFlyCnt = 0;
    if( bTmpDummy )
    {
        nFlyAscent = 0;
        nFlyHeight = 0;
        nFlyDescent = 0;
    }

    if( pPortion )
    {
        SetCntnt( sal_False );
        if( pPortion->IsBreakPortion() )
        {
            SetLen( pPortion->GetLen() );
            if( GetLen() )
                bTmpDummy = sal_False;
        }
        else
        {
            Init( GetPortion() );
            SwLinePortion *pPos = pPortion;
            SwLinePortion *pLast = this;
            KSHORT nMaxDescent = 0;

            //  Eine Gruppe ist ein Abschnitt in der Portion-Kette von
            //  pCurr oder einer Fix-Portion bis zum Ende bzw. zur naechsten
            //  Fix-Portion.
            while( pPos )
            {
                DBG_LOOP;
                ASSERT( POR_LIN != pPos->GetWhichPor(),
                        "SwLineLayout::CalcLine: don't use SwLinePortions !" );
                // Null-Portions werden eliminiert. Sie koennen entstehen,
                // wenn zwei FlyFrms ueberlappen.
                if( !pPos->Compress() )
                {
                    // 8110: Hoehe und Ascent nur uebernehmen, wenn sonst in der
                    // Zeile nichts mehr los ist.
                    if( !pPos->GetPortion() )
                    {
                        if( !Height() )
                            Height( pPos->Height() );
                        if( !GetAscent() )
                            SetAscent( pPos->GetAscent() );
                    }
                    delete pLast->Cut( pPos );
                    pPos = pLast->GetPortion();
                    continue;
                }

                // Es gab Attributwechsel: Laengen und Masse aufaddieren;
                // bzw.Maxima bilden.

                nLineLength += pPos->GetLen();
                KSHORT nPosHeight = pPos->Height();
                KSHORT nPosAscent = pPos->GetAscent();

                AddPrtWidth( pPos->Width() );

                ASSERT( nPosHeight >= nPosAscent,
                        "SwLineLayout::CalcLine: bad ascent or height" );
                if( pPos->IsHangingPortion() )
                {
                    SetHanging( sal_True );
                    rInf.GetParaPortion()->SetMargin( sal_True );
                }

                // Damit ein Paragraphende-Zeichen nicht durch ein Descent zu einer
                // geaenderten Zeilenhoehe und zum Umformatieren fuehrt.
                if ( !pPos->IsBreakPortion() || !Height() )
                {
                    bOnlyPostIts &= pPos->IsPostItsPortion();
                    if( bTmpDummy && !nLineLength )
                    {
                        if( pPos->IsFlyPortion() )
                        {
                            if( nFlyHeight < nPosHeight )
                                nFlyHeight = nPosHeight;
                            if( nFlyAscent < nPosAscent )
                                nFlyAscent = nPosAscent;
                            if( nFlyDescent < nPosHeight - nPosAscent )
                                nFlyDescent = nPosHeight - nPosAscent;
                        }
                        else
                        {
                            if( pPos->InNumberGrp() )
                            {
                                KSHORT nTmp = rInf.GetFont()->GetAscent(
                                                rInf.GetVsh(), rInf.GetOut() );
                                if( nTmp > nPosAscent )
                                {
                                    nPosHeight += nTmp - nPosAscent;
                                    nPosAscent = nTmp;
                                }
                                nTmp = rInf.GetFont()->GetHeight( rInf.GetVsh(),
                                                                rInf.GetOut() );
                                if( nTmp > nPosHeight )
                                    nPosHeight = nTmp;
                            }
                            Height( nPosHeight );
                            nAscent = nPosAscent;
                            nMaxDescent = nPosHeight - nPosAscent;
                        }
                    }
                    else if( !pPos->IsFlyPortion() )
                    {
                        if( Height() < nPosHeight )
                            Height( nPosHeight );
                        if( pPos->IsFlyCntPortion() || ( pPos->IsMultiPortion()
                            && ((SwMultiPortion*)pPos)->HasFlyInCntnt() ) )
                            rLine.SetFlyInCntBase();
                        if( pPos->IsFlyCntPortion() &&
                            ((SwFlyCntPortion*)pPos)->GetAlign() )
                        {
                            ((SwFlyCntPortion*)pPos)->SetMax( sal_False );
                            if( !pFlyCnt || pPos->Height() > pFlyCnt->Height() )
                                pFlyCnt = (SwFlyCntPortion*)pPos;
                        }
                        else
                        {
                            if( nAscent < nPosAscent )
                                nAscent = nPosAscent;
                            if( nMaxDescent < nPosHeight - nPosAscent )
                                nMaxDescent = nPosHeight - nPosAscent;
                        }
                    }
                }
                else if( pPos->GetLen() )
                    bTmpDummy = sal_False;
                if( !HasCntnt() && !pPos->InNumberGrp() )
                {
                    if ( pPos->InExpGrp() )
                    {
                        XubString aTxt;
                        if( pPos->GetExpTxt( rInf, aTxt ) && aTxt.Len() )
                            SetCntnt( sal_True );
                    }
                    else if( pPos->InTxtGrp() && pPos->GetLen() )
                        SetCntnt( sal_True );
                }
                bTmpDummy = bTmpDummy && !HasCntnt() &&
                            ( !pPos->Width() || pPos->IsFlyPortion() );

                pLast = pPos;
                pPos = pPos->GetPortion();
            }
            if( pFlyCnt )
            {
                if( pFlyCnt->Height() == Height() )
                {
                    pFlyCnt->SetMax( sal_True );
                    if( Height() > nMaxDescent + nAscent )
                    {
                        if( 3 == pFlyCnt->GetAlign() ) // Bottom
                            nAscent = Height() - nMaxDescent;
                        else if( 2 == pFlyCnt->GetAlign() ) // Center
                            nAscent = ( Height() + nAscent - nMaxDescent ) / 2;
                    }
                    pFlyCnt->SetAscent( nAscent );
                }
            }
            if( bTmpDummy && nFlyHeight )
            {
                nAscent = nFlyAscent;
                if( nFlyDescent > nFlyHeight - nFlyAscent )
                    Height( nFlyHeight + nFlyDescent );
                else
                    Height( nFlyHeight );
            }
            else if( nMaxDescent > Height() - nAscent )
                Height( nMaxDescent + nAscent );
            if( bOnlyPostIts )
            {
                Height( rInf.GetFont()->GetHeight( rInf.GetVsh(), rInf.GetOut() ) );
                nAscent = rInf.GetFont()->GetAscent( rInf.GetVsh(), rInf.GetOut() );
            }
        }
    }
    else
        SetCntnt( !bTmpDummy );
    // Robust:
    if( nLineWidth < Width() )
        Width( nLineWidth );
    ASSERT( nLineWidth >= Width(), "SwLineLayout::CalcLine: line is bursting" );
    SetDummy( bTmpDummy );
    SetRedline( rLine.GetRedln() &&
        rLine.GetRedln()->CheckLine( rLine.GetStart(), rLine.GetEnd() ) );
}

/*************************************************************************
 *                      class SwCharRange
 *************************************************************************/

SwCharRange &SwCharRange::operator+=(const SwCharRange &rRange)
{
    if(0 != rRange.nLen ) {
        if(0 == nLen) {
            nStart = rRange.nStart;
            nLen = rRange.nLen ;
        }
        else {
            if(rRange.nStart + rRange.nLen > nStart + nLen) {
                nLen = rRange.nStart + rRange.nLen - nStart;
            }
            if(rRange.nStart < nStart) {
                nLen += nStart - rRange.nStart;
                nStart = rRange.nStart;
            }
        }
    }
    return *this;
}

/*************************************************************************
 *                      SwScriptInfo::InitScriptInfo()
 *
 * searches for script changes in rTxt and stores them
 *************************************************************************/

void SwScriptInfo::InitScriptInfo( const SwTxtNode& rNode )
{

    if( !pBreakIt->xBreak.is() )
        return;

    xub_StrLen nChg = nInvalidityPos;
    USHORT nCnt = 0;
    USHORT nCntComp = 0;
    USHORT nScript;

    // compression type
    const String& rTxt = rNode.GetTxt();
    SwCharCompressType aCompEnum = rNode.GetDoc()->GetCharCompressType();

    // delete invalid data from arrays
    // if change position = 0 we do not use any data from the arrays
    // because by deleting all characters of the first group at the beginning
    // of a paragraph nScript is set to a wrong value
    if( nChg )
    {
        ASSERT( CountScriptChg(), "Where're my changes of script?" );
        while( nCnt < CountScriptChg() )
        {
            if ( nChg <= GetScriptChg( nCnt ) )
            {
                nScript = GetScriptType( nCnt );
                break;
            }
            else
                nCnt++;
        }
        if( CHARCOMPRESS_NONE != aCompEnum )
        {
            while( nCntComp < CountCompChg() )
            {
                if ( nChg <= GetCompStart( nCntComp ) )
                    break;
                else
                    nCntComp++;
            }
        }
    }
    else
        nScript = pBreakIt->xBreak->getScriptType( rTxt, 0 );

    USHORT nScriptRemove = aScriptChg.Count() - nCnt;
    aScriptChg.Remove( nCnt, nScriptRemove );
    aScriptType.Remove( nCnt, nScriptRemove );

    // new data for arrays, we start from changed position - 1
    if ( nChg )
         nChg--;
    // we know now that script at position nChg is nScript
    // we have to start at position - 1 to make sure that appending a
    // character cannot cause the occurence of two successing character
    // groups of the same type in the array

    // WEAK can only occur at the beginning of a paragraph
    // the first character not in the weak group determines the type of the
    // weak characters
    if( i18n::ScriptType::WEAK == nScript )
    {
        nChg = (xub_StrLen)pBreakIt->xBreak->endOfScript( rTxt, nChg, nScript );
        if( nChg < rTxt.Len() )
            nScript = pBreakIt->xBreak->getScriptType( rTxt, nChg );
        else
            nScript = i18n::ScriptType::LATIN;
    }
    USHORT nLastChg;
    if( nCntComp )
    {
        --nCntComp;
        nLastChg = GetCompStart( nCntComp );
        if( nChg >= nLastChg + GetCompLen( nCntComp ) )
        {
            nLastChg = nChg;
            ++nCntComp;
        }
    }
    else
        nLastChg = nChg;

    USHORT nCompRemove = aCompChg.Count() - nCntComp;
    aCompChg.Remove( nCntComp, nCompRemove );
    aCompLen.Remove( nCntComp, nCompRemove );
    aCompType.Remove( nCntComp, nCompRemove );

    do
    {
        nChg = (xub_StrLen)pBreakIt->xBreak->endOfScript( rTxt, nChg, nScript );
        aScriptChg.Insert( nChg, nCnt );
        aScriptType.Insert( nScript, nCnt++ );

        // if current script is asian, we search for compressable characters
        // in this range
        if ( CHARCOMPRESS_NONE != aCompEnum &&
             i18n::ScriptType::ASIAN == nScript )
        {
            USHORT ePrevState = NONE;
            USHORT eState;
            USHORT nPrevChg = nLastChg;

            while ( nLastChg < nChg )
            {
                xub_Unicode cChar = rTxt.GetChar( nLastChg );

                // examine current character
                switch ( cChar )
                {
                // Left punctuation found
                case 0x3008: case 0x300A: case 0x300C: case 0x300E:
                case 0x3010: case 0x3014: case 0x3016: case 0x3018:
                case 0x301A: case 0x301D:
                    eState = SPECIAL_LEFT;
                    break;
                // Right punctuation found
                case 0x3001: case 0x3002: case 0x3009: case 0x300B:
                case 0x300D: case 0x300F: case 0x3011: case 0x3015:
                case 0x3017: case 0x3019: case 0x301B: case 0x301E:
                case 0x301F:
                    eState = SPECIAL_RIGHT;
                    break;
                default:
                    eState = ( 0x3040 <= cChar && 0x3100 > cChar ) ?
                               KANA :
                               NONE;
                }

                // insert range of compressable characters
                if( ePrevState != eState )
                {
                    if ( ePrevState != NONE )
                    {
                        // insert start and type
                        if ( CHARCOMPRESS_PUNCTUATION_KANA == aCompEnum ||
                             ePrevState != KANA )
                        {
                            aCompChg.Insert( nPrevChg, nCntComp );
                            USHORT nTmpType = ePrevState;
                            aCompType.Insert( nTmpType, nCntComp );
                            aCompLen.Insert( nLastChg - nPrevChg, nCntComp++ );
                        }
                    }

                    ePrevState = eState;
                    nPrevChg = nLastChg;
                }

                nLastChg++;
            }

            // we still have to examine last entry
            if ( ePrevState != NONE )
            {
                // insert start and type
                if ( CHARCOMPRESS_PUNCTUATION_KANA == aCompEnum ||
                     ePrevState != KANA )
                {
                    aCompChg.Insert( nPrevChg, nCntComp );
                    USHORT nTmpType = ePrevState;
                    aCompType.Insert( nTmpType, nCntComp );
                    aCompLen.Insert( nLastChg - nPrevChg, nCntComp++ );
                }
            }
        }

        if( nChg < rTxt.Len() )
            nScript = pBreakIt->xBreak->getScriptType( rTxt, nChg );
        else
            break;
        nLastChg = nChg;
    } while( TRUE );

    // STRING_LEN means the data structure is up to date
    nInvalidityPos = STRING_LEN;
}

/*************************************************************************
 *                        SwScriptInfo::NextScriptChg(..)
 * returns the position of the next character which belongs to another script
 * than the character of the actual (input) position.
 * If there's no script change until the end of the paragraph, it will return
 * STRING_LEN.
 * Scripts are Asian (Chinese, Japanese, Korean),
 *             Latin ( English etc.)
 *         and Complex ( Hebrew, Arabian )
 *************************************************************************/

xub_StrLen SwScriptInfo::NextScriptChg( const xub_StrLen nPos )  const
{
    USHORT nEnd = CountScriptChg();
    for( USHORT nX = 0; nX < nEnd; ++nX )
    {
        if( nPos < GetScriptChg( nX ) )
            return GetScriptChg( nX );
    }

    return STRING_LEN;
}

/*************************************************************************
 *                        SwScriptInfo::ScriptType(..)
 * returns the script of the character at the input position
 *************************************************************************/

USHORT SwScriptInfo::ScriptType( const xub_StrLen nPos ) const
{
    USHORT nEnd = CountScriptChg();
    for( USHORT nX = 0; nX < nEnd; ++nX )
    {
        if( nPos < GetScriptChg( nX ) )
            return GetScriptType( nX );
    }

    return i18n::ScriptType::LATIN;
}

/*************************************************************************
 *                        SwScriptInfo::CompType(..)
 * returns the type of the compressed character
 *************************************************************************/

USHORT SwScriptInfo::CompType( const xub_StrLen nPos ) const
{
    USHORT nEnd = CountCompChg();
    for( USHORT nX = 0; nX < nEnd; ++nX )
    {
        xub_StrLen nChg = GetCompStart( nX );

        if ( nPos < nChg )
            return NONE;

        if( nPos < nChg + GetCompLen( nX ) )
            return GetCompType( nX );
    }
    return NONE;
}

/*************************************************************************
 *                      SwScriptInfo::HasKana()
 * returns, if there are compressable kanas or specials
 * betwenn nStart and nEnd
 *************************************************************************/

USHORT SwScriptInfo::HasKana( xub_StrLen nStart, const xub_StrLen nLen ) const
{
    USHORT nCnt = CountCompChg();
    xub_StrLen nEnd = nStart + nLen;

    for( USHORT nX = 0; nX < nCnt; ++nX )
    {
        xub_StrLen nKanaStart  = GetCompStart( nX );
        xub_StrLen nKanaEnd = nKanaStart + GetCompLen( nX );

        if ( nKanaStart >= nEnd )
            return USHRT_MAX;

        if ( nStart < nKanaEnd )
            return nX;
    }

    return USHRT_MAX;
}

/*************************************************************************
 *                      SwScriptInfo::Compress()
 *************************************************************************/

long SwScriptInfo::Compress( long* pKernArray, xub_StrLen nIdx, xub_StrLen nLen,
                             const USHORT nCompress, const USHORT nFontHeight,
                             Point* pPoint ) const
{
    ASSERT( nCompress, "Compression without compression?!" );
    ASSERT( nLen, "Compression without text?!" );
    USHORT nCompCount = CountCompChg();

    // In asian typography, there are full width and half width characters.
    // Full width punctuation characters can be compressed by 50 %
    // to determine this, we compare the font width with 75 % of its height
    USHORT nMinWidth = ( 3 * nFontHeight ) / 4;

    USHORT nCompIdx = HasKana( nIdx, nLen );

    if ( USHRT_MAX == nCompIdx )
        return 0;

    xub_StrLen nChg = GetCompStart( nCompIdx );
    xub_StrLen nCompLen = GetCompLen( nCompIdx );
    USHORT nI = 0;
    nLen += nIdx;

    if( nChg > nIdx )
    {
        nI = nChg - nIdx;
        nIdx = nChg;
    }
    else if( nIdx < nChg + nCompLen )
        nCompLen -= nIdx - nChg;

    if( nIdx > nLen || nCompIdx >= nCompCount )
        return 0;

    long nSub = 0;
    long nLast = nI ? pKernArray[ nI - 1 ] : 0;
    do
    {
        USHORT nType = GetCompType( nCompIdx );
        ASSERT( nType == CompType( nIdx ), "Gimme the right type!" );
        nCompLen += nIdx;
        if( nCompLen > nLen )
            nCompLen = nLen;

        // are we allowed to compress the character?
        if ( pKernArray[ nI ] - nLast < nMinWidth )
        {
            nIdx++; nI++;
        }
        else
        {
            while( nIdx < nCompLen )
            {
                ASSERT( SwScriptInfo::NONE != nType, "None compression?!" );

                // nLast is width of current character
                nLast -= pKernArray[ nI ];

                nLast *= nCompress;
                long nMove = 0;
                if( SwScriptInfo::KANA != nType )
                {
                    nLast /= 20000;
                    if( pPoint && SwScriptInfo::SPECIAL_LEFT == nType )
                    {
                        if( nI )
                            nMove = nLast;
                        else
                        {
                            pPoint->X() += nLast;
                            nLast = 0;
                        }
                    }
                }
                else
                    nLast /= 100000;
                nSub -= nLast;
                nLast = pKernArray[ nI ];
                if( nMove )
                    pKernArray[ nI - 1 ] += nMove;
                pKernArray[ nI++ ] -= nSub;
                ++nIdx;
            }
        }

        if( nIdx < nLen )
        {
            xub_StrLen nChg;
            if( ++nCompIdx < nCompCount )
            {
                nChg = GetCompStart( nCompIdx );
                if( nChg > nLen )
                    nChg = nLen;
                nCompLen = GetCompLen( nCompIdx );
            }
            else
                nChg = nLen;
            while( nIdx < nChg )
            {
                nLast = pKernArray[ nI ];
                pKernArray[ nI++ ] -= nSub;
                ++nIdx;
            }
        }
        else
            break;
    } while( nIdx < nLen );
    return nSub;
}

/*************************************************************************
 *                      class SwParaPortion
 *************************************************************************/

SwParaPortion::SwParaPortion()
{
    FormatReset();
    bFlys = bFtnNum = bMargin = sal_False;
    SetWhichPor( POR_PARA );
}

/*************************************************************************
 *                      SwParaPortion::GetParLen()
 *************************************************************************/

xub_StrLen SwParaPortion::GetParLen() const
{
    xub_StrLen nLen = 0;
    const SwLineLayout *pLay = this;
    while( pLay )
    {
        DBG_LOOP;
        nLen += pLay->GetLen();
        pLay = pLay->GetNext();
    }
    return nLen;
}

/*************************************************************************
 *                      SwParaPortion::FindDropPortion()
 *************************************************************************/

const SwDropPortion *SwParaPortion::FindDropPortion() const
{
    const SwLineLayout *pLay = this;
    while( pLay && pLay->IsDummy() )
        pLay = pLay->GetNext();
    while( pLay )
    {
        const SwLinePortion *pPos = pLay->GetPortion();
        while ( pPos && !pPos->GetLen() )
            pPos = pPos->GetPortion();
        if( pPos && pPos->IsDropPortion() )
            return (SwDropPortion *)pPos;
        pLay = pLay->GetLen() ? NULL : pLay->GetNext();
    }
    return NULL;
}

/*************************************************************************
 *                      SwLineLayout::Init()
 *************************************************************************/

void SwLineLayout::Init( SwLinePortion* pNextPortion )
{
    Height( 0 );
    Width( 0 );
    SetLen( 0 );
    SetAscent( 0 );
    SetRealHeight( 0 );
    SetPortion( pNextPortion );
}

/*-----------------16.11.00 11:04-------------------
 * HangingMargin()
 * looks for hanging punctuation portions in the paragraph
 * and return the maximum right offset of them.
 * If no such portion is found, the Margin/Hanging-flags will be atualized.
 * --------------------------------------------------*/

SwTwips SwLineLayout::_GetHangingMargin() const
{
    SwLinePortion* pPor = GetPortion();
    BOOL bFound = sal_False;
    SwTwips nDiff = 0;
    while( pPor)
    {
        if( pPor->IsHangingPortion() )
        {
            nDiff = ((SwHangingPortion*)pPor)->GetInnerWidth() - pPor->Width();
            if( nDiff )
                bFound = sal_True;
        }
        pPor = pPor->GetPortion();
    }
    if( !bFound ) // actualize the hanging-flag
        ((SwLineLayout*)this)->SetHanging( sal_False );
    return nDiff;
}

SwTwips SwTxtFrm::HangingMargin() const
{
    ASSERT( HasPara(), "Don't call me without a paraportion" );
    if( !GetPara()->IsMargin() )
        return 0;
    const SwLineLayout* pLine = GetPara();
    SwTwips nRet = 0;
    do
    {
        SwTwips nDiff = pLine->GetHangingMargin();
        if( nDiff > nRet )
            nRet = nDiff;
        pLine = pLine->GetNext();
    } while ( pLine );
    if( !nRet ) // actualize the margin-flag
        ((SwParaPortion*)GetPara())->SetMargin( sal_False );
    return nRet;
}

