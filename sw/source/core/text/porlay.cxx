/*************************************************************************
 *
 *  $RCSfile: porlay.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ama $ $Date: 2001-02-20 10:20:48 $
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
        }
        else
            pPos = 0;
    }
    return pLeft;
}

/*************************************************************************
 *                    SwLineLayout::CreateSpaceAdd()
 *************************************************************************/

void SwLineLayout::CreateSpaceAdd()
{
    MSHORT nNull = 0;
    pSpaceAdd = new SvShorts;
    pSpaceAdd->Insert( nNull, 0 );
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

void SwScriptInfo::InitScriptInfo( const String& rTxt )
{
    if( !pBreakIt->xBreak.is() )
        return;

    xub_StrLen nChg = nInvalidityPos;
    USHORT nCnt = 0;
    USHORT nScript;

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
    }
    else
        nScript = pBreakIt->xBreak->getScriptType( rTxt, 0 );

    aScriptChg.Remove( nCnt, aScriptChg.Count() - nCnt );
    aScriptType.Remove( nCnt, aScriptType.Count() - nCnt );

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
        nChg = pBreakIt->xBreak->endOfScript( rTxt, nChg, nScript );
        if( nChg < rTxt.Len() )
            nScript = pBreakIt->xBreak->getScriptType( rTxt, nChg );
    }
    do
    {
        nChg = pBreakIt->xBreak->endOfScript( rTxt, nChg, nScript );
        InsertScriptChg( nChg, nCnt );
        InsertScriptType( nScript, nCnt++ );
        if( nChg < rTxt.Len() )
            nScript = pBreakIt->xBreak->getScriptType( rTxt, nChg );
        else
            break;
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
    for( USHORT nX = 0; nX < CountScriptChg(); ++nX )
        if( nPos < GetScriptChg( nX ) )
            return GetScriptChg( nX );
    return STRING_LEN;
}

/*************************************************************************
 *                        SwScriptInfo::ScriptType(..)
 * returns the script of the character at the input position
 *************************************************************************/

USHORT SwScriptInfo::ScriptType( const xub_StrLen nPos ) const
{
    for( USHORT nX = 0; nX < CountScriptChg(); ++nX )
        if( nPos < GetScriptChg( nX ) )
            return GetScriptType( nX );
    return i18n::ScriptType::LATIN;
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

