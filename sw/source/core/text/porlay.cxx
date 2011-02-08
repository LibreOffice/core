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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include "errhdl.hxx"   // ASSERT

#include "txtcfg.hxx"
#include "porlay.hxx"
#include "itrform2.hxx"
#include "porglue.hxx"
#include "porexp.hxx"   // SwQuoVadisPortion
#include "blink.hxx"    // pBlink
#include "redlnitr.hxx" // SwRedlineItr
#include "porfly.hxx"   // SwFlyCntPortion
#include <porrst.hxx>       // SwHangingPortion
#include <pormulti.hxx>     // SwMultiPortion
#include <breakit.hxx>
#include <unicode/uchar.h>
#include <com/sun/star/i18n/ScriptType.hdl>
#include <com/sun/star/i18n/CTLScriptType.hdl>
#include <com/sun/star/i18n/WordType.hdl>
#include <paratr.hxx>
#include <editeng/adjitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/charhiddenitem.hxx>
#include <vcl/outdev.hxx>
#include <editeng/blnkitem.hxx>
#include <tools/multisel.hxx>
#include <unotools/charclass.hxx>
#include <i18npool/mslangid.hxx>
#include <charfmt.hxx>
#include <fchrfmt.hxx>
#include <docary.hxx>       // SwRedlineTbl
#include <redline.hxx>      // SwRedline

// --> FME 2004-06-08 #i12836# enhanced pdf export
#include <section.hxx>
// <--

#include <IDocumentRedlineAccess.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentContentOperations.hxx>

using namespace ::com::sun::star;
using namespace i18n::ScriptType;

#include <unicode/ubidi.h>
#include <i18nutil/unicode.hxx>  //unicode::getUnicodeScriptType

sal_Bool isAlefChar ( xub_Unicode cCh )
{
   return ( cCh == 0x622 || cCh == 0x623 || cCh == 0x625 || cCh == 0x627 ||
           cCh == 0x622 || cCh == 0x671 || cCh == 0x672 || cCh == 0x673 || cCh == 0x675 );
}

sal_Bool isWawChar ( xub_Unicode cCh )
{
   return ( cCh == 0x624 || cCh == 0x648 || cCh == 0x676 || cCh == 0x677 ||
           ( cCh >= 0x6C4 &&  cCh <= 0x6CB ) || cCh == 0x6CF );
}

sal_Bool isDalChar ( xub_Unicode cCh )
{
   return ( cCh == 0x62F || cCh == 0x630 || cCh == 0x688 || cCh == 0x689 || cCh == 0x690 );
}

sal_Bool isRehChar ( xub_Unicode cCh )
{
   return ( cCh == 0x631 || cCh == 0x632 || ( cCh >= 0x691 && cCh <= 0x699 ));
}

sal_Bool isTehMarbutaChar ( xub_Unicode cCh )
{
   return ( cCh == 0x629 || cCh == 0x6C0 );
}

sal_Bool isBaaChar ( xub_Unicode cCh )
{
   return ( cCh == 0x628 || cCh == 0x62A || cCh == 0x62B || cCh == 0x679 || cCh == 0x680 );
}

sal_Bool isYehChar ( xub_Unicode cCh )
{
   return ( cCh == 0x626 || cCh == 0x649 || cCh == 0x64A || cCh == 0x678 || cCh == 0x6CC ||
       cCh == 0x6CE || cCh == 0x6D0 || cCh == 0x6D1 );
}

sal_Bool isSeenOrSadChar ( xub_Unicode cCh )
{
   return ( ( cCh >= 0x633 && cCh <= 0x636 ) || ( cCh >= 0x69A && cCh <= 0x69E )
           || cCh == 0x6FA || cCh == 0x6FB );
}

sal_Bool isHahChar ( xub_Unicode cCh )
{
   return ( ( cCh >= 0x62C && cCh <= 0x62E ) || ( cCh >= 0x681 && cCh <= 0x687 )
           || cCh == 0x6BF );
}

sal_Bool isAinChar ( xub_Unicode cCh )
{
   return ( cCh == 0x639 || cCh == 0x63A || cCh == 0x6A0 || cCh == 0x6FC );
}

sal_Bool isKafChar ( xub_Unicode cCh )
{
   return ( cCh == 0x643 || ( cCh >= 0x6AC && cCh <= 0x6AE ) );
}

sal_Bool isLamChar ( xub_Unicode cCh )
{
   return ( cCh == 0x644 || ( cCh >= 0x6B5 && cCh <= 0x6B8 ) );
}

sal_Bool isGafChar ( xub_Unicode cCh )
{
   return ( cCh == 0x6A9 || cCh == 0x6AB ||( cCh >= 0x6AF && cCh <= 0x6B4 ) );
}

sal_Bool isQafChar ( xub_Unicode cCh )
{
   return ( cCh == 0x642 || cCh == 0x6A7 || cCh == 0x6A8  );
}

sal_Bool isFeChar ( xub_Unicode cCh )
{
   return ( cCh == 0x641 || ( cCh >= 0x6A1 && cCh <= 0x6A6 ) );
}
sal_Bool isTransparentChar ( xub_Unicode cCh )
{
    return ( ( cCh >= 0x610 && cCh <= 0x61A ) ||
            ( cCh >= 0x64B && cCh <= 0x65E ) ||
            ( cCh == 0x670 ) ||
            ( cCh >= 0x6D6 && cCh <= 0x6DC ) ||
            ( cCh >= 0x6DF && cCh <= 0x6E4 ) ||
            ( cCh >= 0x6E7 && cCh <= 0x6E8 ) ||
            ( cCh >= 0x6EA && cCh <= 0x6ED ));
}

/*************************************************************************
 *                 lcl_IsLigature
 *
 * Checks if cCh + cNectCh builds a ligature (used for Kashidas)
 *************************************************************************/

sal_Bool lcl_IsLigature( xub_Unicode cCh, xub_Unicode cNextCh )
{
            // Lam + Alef
    return ( isLamChar ( cCh ) && isAlefChar ( cNextCh ));
}

/*************************************************************************
 *                 lcl_ConnectToPrev
 *
 * Checks if cCh is connectable to cPrevCh (used for Kashidas)
 *************************************************************************/

sal_Bool lcl_ConnectToPrev( xub_Unicode cCh, xub_Unicode cPrevCh )
{
    // Alef, Dal, Thal, Reh, Zain, and Waw do not connect to the left
    // Uh, there seem to be some more characters that are not connectable
    // to the left. So we look for the characters that are actually connectable
    // to the left. Here is the complete list of WH:

    // (hennerdrewes):
    // added lam forms 0x06B5..0x06B8
    // added 0x6FA..0x6FC, according to unicode documentation, although not present in my fonts
    // added heh goal 0x6C1
    sal_Bool bRet = 0x628 == cPrevCh ||
                    ( 0x62A <= cPrevCh && cPrevCh <= 0x62E ) ||
                  ( 0x633 <= cPrevCh && cPrevCh <= 0x647 ) ||
                      0x649 == cPrevCh || // Alef Maksura does connect !!!
                      0x64A == cPrevCh ||
                    ( 0x678 <= cPrevCh && cPrevCh <= 0x687 ) ||
                  ( 0x69A <= cPrevCh && cPrevCh <= 0x6C1 ) ||
                  ( 0x6C3 <= cPrevCh && cPrevCh <= 0x6D3 ) ||
                  ( 0x6FA <= cPrevCh && cPrevCh <= 0x6FC )  ;

    // check for ligatures cPrevChar + cChar
    if( bRet )
        bRet = !lcl_IsLigature( cPrevCh, cCh );
    return bRet;
}

/*************************************************************************
 *                 lcl_HasStrongLTR
 *************************************************************************/
 bool lcl_HasStrongLTR ( const String& rTxt, xub_StrLen nStart, xub_StrLen nEnd )
 {
     for ( xub_StrLen nCharIdx = nStart; nCharIdx < nEnd; ++nCharIdx )
     {
         const UCharDirection nCharDir = u_charDirection ( rTxt.GetChar ( nCharIdx ));
         if ( nCharDir == U_LEFT_TO_RIGHT ||
              nCharDir == U_LEFT_TO_RIGHT_EMBEDDING ||
              nCharDir == U_LEFT_TO_RIGHT_OVERRIDE )
             return true;
     }
     return false;
 }

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
    delete pLLSpaceAdd;
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
 *                    SwLineLayout::InitSpaceAdd()
 *************************************************************************/

void SwLineLayout::InitSpaceAdd()
{
    if ( !pLLSpaceAdd )
        CreateSpaceAdd();
    else
        SetLLSpaceAdd( 0, 0 );
}

/*************************************************************************
 *                    SwLineLayout::CreateSpaceAdd()
 *************************************************************************/

void SwLineLayout::CreateSpaceAdd( const long nInit )
{
    pLLSpaceAdd = new std::vector<long>;
    SetLLSpaceAdd( nInit, 0 );
}

/*************************************************************************
 * Local helper function. Returns true if there are only blanks
 * in [nStt, nEnd[
 *************************************************************************/

bool lcl_HasOnlyBlanks( const XubString& rTxt, xub_StrLen nStt, xub_StrLen nEnd )
{
    bool bBlankOnly = true;
    while ( nStt < nEnd )
    {
        const xub_Unicode cChar = rTxt.GetChar( nStt++ );
        if ( ' ' != cChar && 0x3000 != cChar )
        {
            bBlankOnly = false;
            break;
        }
    }
    return bBlankOnly;
}

/*************************************************************************
 *                    SwLineLayout::CalcLine()
 *
 * Aus FormatLine() ausgelagert.
 *************************************************************************/

void SwLineLayout::CalcLine( SwTxtFormatter &rLine, SwTxtFormatInfo &rInf )
{
    const KSHORT nLineWidth = rInf.RealWidth();

    KSHORT nFlyAscent = 0;
    KSHORT nFlyHeight = 0;
    KSHORT nFlyDescent = 0;
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

    // --> FME 2006-03-01 #i3952#
    const bool bIgnoreBlanksAndTabsForLineHeightCalculation =
            rInf.GetTxtFrm()->GetNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION);

    bool bHasBlankPortion = false;
    bool bHasOnlyBlankPortions = true;
    // <--

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
                OSL_ENSURE( POR_LIN != pPos->GetWhichPor(),
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

                const xub_StrLen nPorSttIdx = rInf.GetLineStart() + nLineLength;
                nLineLength = nLineLength + pPos->GetLen();
                AddPrtWidth( pPos->Width() );

                // --> FME 2006-03-01 #i3952#
                if ( bIgnoreBlanksAndTabsForLineHeightCalculation )
                {
                    if ( pPos->InTabGrp() || pPos->IsHolePortion() ||
                            ( pPos->IsTextPortion() &&
                              lcl_HasOnlyBlanks( rInf.GetTxt(), nPorSttIdx, nPorSttIdx + pPos->GetLen() ) ) )
                    {
                        pLast = pPos;
                        pPos = pPos->GetPortion();
                        bHasBlankPortion = true;
                        continue;
                    }
                }
                // <--

                bHasOnlyBlankPortions = false;

                // Es gab Attributwechsel: Laengen und Masse aufaddieren;
                // bzw.Maxima bilden.

                KSHORT nPosHeight = pPos->Height();
                KSHORT nPosAscent = pPos->GetAscent();

                OSL_ENSURE( nPosHeight >= nPosAscent,
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
                                                rInf.GetVsh(), *rInf.GetOut() );
                                if( nTmp > nPosAscent )
                                {
                                    nPosHeight += nTmp - nPosAscent;
                                    nPosAscent = nTmp;
                                }
                                nTmp = rInf.GetFont()->GetHeight( rInf.GetVsh(),
                                                                 *rInf.GetOut() );
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
                    else if( ( pPos->InTxtGrp() || pPos->IsMultiPortion() ) &&
                             pPos->GetLen() )
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

            if( bOnlyPostIts && !( bHasBlankPortion && bHasOnlyBlankPortions ) )
            {
                Height( rInf.GetFont()->GetHeight( rInf.GetVsh(), *rInf.GetOut() ) );
                nAscent = rInf.GetFont()->GetAscent( rInf.GetVsh(), *rInf.GetOut() );
            }
        }
    }
    else
    {
        SetCntnt( !bTmpDummy );

        // --> FME 2006-03-01 #i3952#
        if ( bIgnoreBlanksAndTabsForLineHeightCalculation &&
             lcl_HasOnlyBlanks( rInf.GetTxt(), rInf.GetLineStart(), rInf.GetLineStart() + GetLen() ) )
        {
            bHasBlankPortion = true;
        }
        // <--
    }

    // --> FME 2006-03-01 #i3952#
    if ( bHasBlankPortion && bHasOnlyBlankPortions )
    {
        USHORT nTmpAscent = GetAscent();
        USHORT nTmpHeight = Height();
        rLine.GetAttrHandler().GetDefaultAscentAndHeight( rInf.GetVsh(), *rInf.GetOut(), nTmpAscent, nTmpHeight );
        SetAscent( nTmpAscent );
        Height( nTmpHeight );
    }
    // <--

    // Robust:
    if( nLineWidth < Width() )
        Width( nLineWidth );
    OSL_ENSURE( nLineWidth >= Width(), "SwLineLayout::CalcLine: line is bursting" );
    SetDummy( bTmpDummy );
    SetRedline( rLine.GetRedln() &&
        rLine.GetRedln()->CheckLine( rLine.GetStart(), rLine.GetEnd() ) );
}

// --> OD 2005-05-20 #i47162# - add optional parameter <_bNoFlyCntPorAndLinePor>
// to control, if the fly content portions and line portion are considered.
void SwLineLayout::MaxAscentDescent( SwTwips& _orAscent,
                                     SwTwips& _orDescent,
                                     SwTwips& _orObjAscent,
                                     SwTwips& _orObjDescent,
                                     const SwLinePortion* _pDontConsiderPortion,
                                     const bool _bNoFlyCntPorAndLinePor ) const
{
    _orAscent = 0;
    _orDescent = 0;
    _orObjAscent = 0;
    _orObjDescent = 0;

    const SwLinePortion* pTmpPortion = this;
    if ( !pTmpPortion->GetLen() && pTmpPortion->GetPortion() )
    {
        pTmpPortion = pTmpPortion->GetPortion();
    }

    while ( pTmpPortion )
    {
        if ( !pTmpPortion->IsBreakPortion() && !pTmpPortion->IsFlyPortion() &&
             ( !_bNoFlyCntPorAndLinePor ||
               ( !pTmpPortion->IsFlyCntPortion() &&
                 !(pTmpPortion == this && pTmpPortion->GetPortion() ) ) ) )
        {
            SwTwips nPortionAsc = static_cast<SwTwips>(pTmpPortion->GetAscent());
            SwTwips nPortionDesc = static_cast<SwTwips>(pTmpPortion->Height()) -
                                   nPortionAsc;

            const sal_Bool bFlyCmp = pTmpPortion->IsFlyCntPortion() ?
                                     static_cast<const SwFlyCntPortion*>(pTmpPortion)->IsMax() :
                                     !( pTmpPortion == _pDontConsiderPortion );

            if ( bFlyCmp )
            {
                _orObjAscent = Max( _orObjAscent, nPortionAsc );
                _orObjDescent = Max( _orObjDescent, nPortionDesc );
            }

            if ( !pTmpPortion->IsFlyCntPortion() && !pTmpPortion->IsGrfNumPortion() )
            {
                _orAscent = Max( _orAscent, nPortionAsc );
                _orDescent = Max( _orDescent, nPortionDesc );
            }
        }
        pTmpPortion = pTmpPortion->GetPortion();
    }
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
 *                      SwScriptInfo::SwScriptInfo()
 *************************************************************************/
SwScriptInfo::SwScriptInfo() :
    nInvalidityPos( 0 ),
    nDefaultDir( 0 )
{
};

/*************************************************************************
 *                      SwScriptInfo::~SwScriptInfo()
 *************************************************************************/
SwScriptInfo::~SwScriptInfo()
{
}

/*************************************************************************
 *                     SwScriptInfo::WhichFont()
 *
 * Converts i18n Script Type (LATIN, ASIAN, COMPLEX, WEAK) to
 * Sw Script Types (SW_LATIN, SW_CJK, SW_CTL), used to identify the font
 *************************************************************************/
BYTE SwScriptInfo::WhichFont( xub_StrLen nIdx, const String* pTxt, const SwScriptInfo* pSI )
{
    OSL_ENSURE( pTxt || pSI,"How should I determine the script type?" );
    USHORT nScript;

    // First we try to use our SwScriptInfo
    if ( pSI )
        nScript = pSI->ScriptType( nIdx );
    else
        // Ok, we have to ask the break iterator
        nScript = pBreakIt->GetRealScriptOfText( *pTxt, nIdx );

    switch ( nScript ) {
        case i18n::ScriptType::LATIN : return SW_LATIN;
        case i18n::ScriptType::ASIAN : return SW_CJK;
        case i18n::ScriptType::COMPLEX : return SW_CTL;
    }

    OSL_ENSURE( sal_False, "Somebody tells lies about the script type!" );
    return SW_LATIN;
}

/*************************************************************************
 *                      SwScriptInfo::InitScriptInfo()
 *
 * searches for script changes in rTxt and stores them
 *************************************************************************/

void SwScriptInfo::InitScriptInfo( const SwTxtNode& rNode )
{
    InitScriptInfo( rNode, nDefaultDir == UBIDI_RTL );
}

void SwScriptInfo::InitScriptInfo( const SwTxtNode& rNode, sal_Bool bRTL )
{
    if( !pBreakIt->GetBreakIter().is() )
        return;

    const String& rTxt = rNode.GetTxt();

    //
    // HIDDEN TEXT INFORMATION
    //
    Range aRange( 0, rTxt.Len() ? rTxt.Len() - 1 : 0 );
    MultiSelection aHiddenMulti( aRange );
    CalcHiddenRanges( rNode, aHiddenMulti );

    aHiddenChg.Remove( 0, aHiddenChg.Count() );
    USHORT nHiddenIdx = 0;
    USHORT i = 0;
    for( i = 0; i < aHiddenMulti.GetRangeCount(); ++i )
    {
        const Range& rRange = aHiddenMulti.GetRange( i );
        const xub_StrLen nStart = (xub_StrLen)rRange.Min();
        const xub_StrLen nEnd = (xub_StrLen)rRange.Max() + 1;

        aHiddenChg.Insert( nStart, nHiddenIdx++ );
        aHiddenChg.Insert( nEnd, nHiddenIdx++ );
    }

    //
    // SCRIPT AND SCRIPT RELATED INFORMATION
    //

    xub_StrLen nChg = nInvalidityPos;

    // STRING_LEN means the data structure is up to date
    nInvalidityPos = STRING_LEN;

    // this is the default direction
    nDefaultDir = static_cast<BYTE>(bRTL ? UBIDI_RTL : UBIDI_LTR);

    // counter for script info arrays
    USHORT nCnt = 0;
    // counter for compression information arrays
    USHORT nCntComp = 0;
    // counter for kashida array
    USHORT nCntKash = 0;

    BYTE nScript = i18n::ScriptType::LATIN;

    // compression type
    const SwCharCompressType aCompEnum = rNode.getIDocumentSettingAccess()->getCharacterCompressionType();

    // justification type
    const sal_Bool bAdjustBlock = SVX_ADJUST_BLOCK ==
                                  rNode.GetSwAttrSet().GetAdjust().GetAdjust();

    //
    // FIND INVALID RANGES IN SCRIPT INFO ARRAYS:
    //

    if( nChg )
    {
        // if change position = 0 we do not use any data from the arrays
        // because by deleting all characters of the first group at the beginning
        // of a paragraph nScript is set to a wrong value
        OSL_ENSURE( CountScriptChg(), "Where're my changes of script?" );
        while( nCnt < CountScriptChg() )
        {
            if ( nChg > GetScriptChg( nCnt ) )
                nCnt++;
            else
            {
                nScript = GetScriptType( nCnt );
                break;
            }
        }
        if( CHARCOMPRESS_NONE != aCompEnum )
        {
            while( nCntComp < CountCompChg() )
            {
                if ( nChg > GetCompStart( nCntComp ) )
                    nCntComp++;
                else
                    break;
            }
        }
        if ( bAdjustBlock )
        {
            while( nCntKash < CountKashida() )
            {
                if ( nChg > GetKashida( nCntKash ) )
                    nCntKash++;
                else
                    break;
            }
        }
    }

    //
    // ADJUST nChg VALUE:
    //

    // by stepping back one position we know that we are inside a group
    // declared as an nScript group
    if ( nChg )
        --nChg;

    const xub_StrLen nGrpStart = nCnt ? GetScriptChg( nCnt - 1 ) : 0;

    // we go back in our group until we reach the first character of
    // type nScript
    while ( nChg > nGrpStart &&
            nScript != pBreakIt->GetBreakIter()->getScriptType( rTxt, nChg ) )
        --nChg;

    // If we are at the start of a group, we do not trust nScript,
    // we better get nScript from the breakiterator:
    if ( nChg == nGrpStart )
        nScript = (BYTE)pBreakIt->GetBreakIter()->getScriptType( rTxt, nChg );

    //
    // INVALID DATA FROM THE SCRIPT INFO ARRAYS HAS TO BE DELETED:
    //

    // remove invalid entries from script information arrays
    aScriptChanges.erase( aScriptChanges.begin() + nCnt, aScriptChanges.end() );

    // get the start of the last compression group
    USHORT nLastCompression = nChg;
    if( nCntComp )
    {
        --nCntComp;
        nLastCompression = GetCompStart( nCntComp );
        if( nChg >= nLastCompression + GetCompLen( nCntComp ) )
        {
            nLastCompression = nChg;
            ++nCntComp;
        }
    }

    // remove invalid entries from compression information arrays
    const USHORT nCompRemove = aCompChg.Count() - nCntComp;
    aCompChg.Remove( nCntComp, nCompRemove );
    aCompLen.Remove( nCntComp, nCompRemove );
    aCompType.Remove( nCntComp, nCompRemove );

    // get the start of the last kashida group
    USHORT nLastKashida = nChg;
    if( nCntKash && i18n::ScriptType::COMPLEX == nScript )
    {
        --nCntKash;
        nLastKashida = GetKashida( nCntKash );
    }

    // remove invalid entries from kashida array
    aKashida.Remove( nCntKash, aKashida.Count() - nCntKash );

    //
    // TAKE CARE OF WEAK CHARACTERS: WE MUST FIND AN APPROPRIATE
    // SCRIPT FOR WEAK CHARACTERS AT THE BEGINNING OF A PARAGRAPH
    //

    if( WEAK == pBreakIt->GetBreakIter()->getScriptType( rTxt, nChg ) )
    {
        // If the beginning of the current group is weak, this means that
        // all of the characters in this grounp are weak. We have to assign
        // the scripts to these characters depending on the fonts which are
        // set for these characters to display them.
        xub_StrLen nEnd =
                (xub_StrLen)pBreakIt->GetBreakIter()->endOfScript( rTxt, nChg, WEAK );

        if( nEnd > rTxt.Len() )
            nEnd = rTxt.Len();

        nScript = (BYTE)GetI18NScriptTypeOfLanguage( (USHORT)GetAppLanguage() );

        OSL_ENSURE( i18n::ScriptType::LATIN == nScript ||
                i18n::ScriptType::ASIAN == nScript ||
                i18n::ScriptType::COMPLEX == nScript, "Wrong default language" );

        nChg = nEnd;

        // Get next script type or set to weak in order to exit
        BYTE nNextScript = ( nEnd < rTxt.Len() ) ?
           (BYTE)pBreakIt->GetBreakIter()->getScriptType( rTxt, nEnd ) :
           (BYTE)WEAK;

        if ( nScript != nNextScript )
        {
            aScriptChanges.push_back( ScriptChangeInfo(nEnd, nScript) );
            nCnt++;
            nScript = nNextScript;
        }
    }

    //
    // UPDATE THE SCRIPT INFO ARRAYS:
    //

    while ( nChg < rTxt.Len() || ( !aScriptChanges.empty() && !rTxt.Len() ) )
    {
        OSL_ENSURE( i18n::ScriptType::WEAK != nScript,
                "Inserting WEAK into SwScriptInfo structure" );
        OSL_ENSURE( STRING_LEN != nChg, "65K? Strange length of script section" );

        xub_StrLen nSearchStt = nChg;
        nChg = (xub_StrLen)pBreakIt->GetBreakIter()->endOfScript( rTxt, nSearchStt, nScript );

        if ( nChg > rTxt.Len() )
            nChg = rTxt.Len();

        // --> FME 2008-09-17 #i28203#
        // for 'complex' portions, we make sure that a portion does not contain more
        // than one script:
        if( i18n::ScriptType::COMPLEX == nScript && pBreakIt->GetScriptTypeDetector().is() )
        {
            const short nScriptType = pBreakIt->GetScriptTypeDetector()->getCTLScriptType( rTxt, nSearchStt );
            xub_StrLen nNextCTLScriptStart = nSearchStt;
            short nCurrentScriptType = nScriptType;
            while( com::sun::star::i18n::CTLScriptType::CTL_UNKNOWN == nCurrentScriptType || nScriptType == nCurrentScriptType )
            {
                nNextCTLScriptStart = (xub_StrLen)pBreakIt->GetScriptTypeDetector()->endOfCTLScriptType( rTxt, nNextCTLScriptStart );
                if( nNextCTLScriptStart < rTxt.Len() && nNextCTLScriptStart < nChg )
                    nCurrentScriptType = pBreakIt->GetScriptTypeDetector()->getCTLScriptType( rTxt, nNextCTLScriptStart );
                else
                    break;
            }
            nChg = Min( nChg, nNextCTLScriptStart );
        }
        // <--

        // special case for dotted circle since it can be used with complex
        // before a mark, so we want it associated with the mark's script
        if (nChg < rTxt.Len() && nChg > 0 && (i18n::ScriptType::WEAK ==
            pBreakIt->GetBreakIter()->getScriptType(rTxt,nChg - 1)))
        {
            int8_t nType = u_charType(rTxt.GetChar(nChg) );
            if (nType == U_NON_SPACING_MARK || nType == U_ENCLOSING_MARK ||
                nType == U_COMBINING_SPACING_MARK )
            {
                aScriptChanges.push_back( ScriptChangeInfo(nChg-1, nScript) );
            }
            else
            {
                aScriptChanges.push_back( ScriptChangeInfo(nChg, nScript) );
            }
        }
        else
        {
            aScriptChanges.push_back( ScriptChangeInfo(nChg, nScript) );
        }
        ++nCnt;

        // if current script is asian, we search for compressable characters
        // in this range
        if ( CHARCOMPRESS_NONE != aCompEnum &&
             i18n::ScriptType::ASIAN == nScript )
        {
            BYTE ePrevState = NONE;
            BYTE eState;
            USHORT nPrevChg = nLastCompression;

            while ( nLastCompression < nChg )
            {
                xub_Unicode cChar = rTxt.GetChar( nLastCompression );

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
                    eState = static_cast<BYTE>( ( 0x3040 <= cChar && 0x3100 > cChar ) ? KANA : NONE );
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
                            BYTE nTmpType = ePrevState;
                            aCompType.Insert( nTmpType, nCntComp );
                            aCompLen.Insert( nLastCompression - nPrevChg, nCntComp++ );
                        }
                    }

                    ePrevState = eState;
                    nPrevChg = nLastCompression;
                }

                nLastCompression++;
            }

            // we still have to examine last entry
            if ( ePrevState != NONE )
            {
                // insert start and type
                if ( CHARCOMPRESS_PUNCTUATION_KANA == aCompEnum ||
                     ePrevState != KANA )
                {
                    aCompChg.Insert( nPrevChg, nCntComp );
                    BYTE nTmpType = ePrevState;
                    aCompType.Insert( nTmpType, nCntComp );
                    aCompLen.Insert( nLastCompression - nPrevChg, nCntComp++ );
                }
            }
        }

        // we search for connecting opportunities (kashida)
        else if ( bAdjustBlock && i18n::ScriptType::COMPLEX == nScript )
        {
            SwScanner aScanner( rNode, rNode.GetTxt(), 0, 0,
                                i18n::WordType::DICTIONARY_WORD,
                                nLastKashida, nChg );

            // the search has to be performed on a per word base
            while ( aScanner.NextWord() )
            {
                const XubString& rWord = aScanner.GetWord();

                xub_StrLen nIdx = 0;
                xub_StrLen nKashidaPos = STRING_LEN;
                xub_Unicode cCh;
                xub_Unicode cPrevCh = 0;

                USHORT nPriorityLevel = 7; // 0..6 = level found
                                           // 7 not found

                xub_StrLen nWordLen = rWord.Len();

                // ignore trailing vowel chars
                while( nWordLen && isTransparentChar( rWord.GetChar( nWordLen - 1 )))
                    --nWordLen;

                while (nIdx < nWordLen)
                {
                    cCh = rWord.GetChar( nIdx );

                    // 1. Priority:
                    // after user inserted kashida
                    if ( 0x640 == cCh )
                    {
                        nKashidaPos = aScanner.GetBegin() + nIdx;
                        nPriorityLevel = 0;
                    }

                    // 2. Priority:
                    // after a Seen or Sad
                    if (nPriorityLevel >= 1 && nIdx < nWordLen - 1)
                    {
                        if( isSeenOrSadChar( cCh )
                         && (rWord.GetChar( nIdx+1 ) != 0x200C) ) // #i98410#: prevent ZWNJ expansion
                        {
                            nKashidaPos  = aScanner.GetBegin() + nIdx;
                            nPriorityLevel = 1;
                        }
                    }

                    // 3. Priority:
                    // before final form of Teh Marbuta, Hah, Dal
                    if ( nPriorityLevel >= 2 && nIdx > 0 )
                    {
                        if ( isTehMarbutaChar ( cCh ) || // Teh Marbuta (right joining)
                             isDalChar ( cCh ) ||        // Dal (right joining) final form may appear in the middle of word
                             ( isHahChar ( cCh ) && nIdx == nWordLen - 1))  // Hah (dual joining) only at end of word
                        {

                            OSL_ENSURE( 0 != cPrevCh, "No previous character" );
                            // check if character is connectable to previous character,
                            if ( lcl_ConnectToPrev( cCh, cPrevCh ) )
                            {
                                nKashidaPos = aScanner.GetBegin() + nIdx - 1;
                                nPriorityLevel = 2;
                            }
                        }
                    }

                    // 4. Priority:
                    // before final form of Alef, Lam or Kaf
                    if ( nPriorityLevel >= 3 && nIdx > 0 )
                    {
                        if ( isAlefChar ( cCh ) ||   // Alef (right joining) final form may appear in the middle of word
                             (( isLamChar ( cCh ) || // Lam
                              isKafChar ( cCh )   || // Kaf (both dual joining)
                              isGafChar ( cCh ) )
                              && nIdx == nWordLen - 1))  // only at end of word
                        {
                            OSL_ENSURE( 0 != cPrevCh, "No previous character" );
                            // check if character is connectable to previous character,
                            if ( lcl_ConnectToPrev( cCh, cPrevCh ) )
                            {
                                nKashidaPos = aScanner.GetBegin() + nIdx - 1;
                                nPriorityLevel = 3;
                            }
                        }
                    }

                    // 5. Priority:
                    // before media Bah
                    if ( nPriorityLevel >= 4 && nIdx > 0 && nIdx < nWordLen - 1 )
                    {
                        if ( isBaaChar ( cCh )) // Bah
                        {
                            // check if next character is Reh, Yeh or Alef Maksura
                            xub_Unicode cNextCh = rWord.GetChar( nIdx + 1 );
                            if ( isRehChar ( cNextCh ) || isYehChar ( cNextCh ))
                           {
                                OSL_ENSURE( 0 != cPrevCh, "No previous character" );
                                // check if character is connectable to previous character,
                                if ( lcl_ConnectToPrev( cCh, cPrevCh ) )
                                {
                                    nKashidaPos = aScanner.GetBegin() + nIdx - 1;
                                    nPriorityLevel = 4;
                                }
                            }
                        }
                    }

                    // 6. Priority:
                    // before the final form of Waw, Ain, Qaf and Fa
                    if ( nPriorityLevel >= 5 && nIdx > 0 )
                    {
                        if ( isWawChar ( cCh )   || // Wav (right joining)
                                                    // final form may appear in the middle of word
                             (( isAinChar ( cCh ) ||  // Ain (dual joining)
                                isQafChar ( cCh ) ||  // Qaf (dual joining)
                                isFeChar  ( cCh ) )   // Feh (dual joining)
                                && nIdx == nWordLen - 1))  // only at end of word
                        {
                            OSL_ENSURE( 0 != cPrevCh, "No previous character" );
                            // check if character is connectable to previous character,
                            if ( lcl_ConnectToPrev( cCh, cPrevCh ) )
                            {
                                nKashidaPos = aScanner.GetBegin() + nIdx - 1;
                                nPriorityLevel = 5;
                            }
                        }
                    }

                    // other connecting possibilities
                    if ( nPriorityLevel >= 6 && nIdx > 0 )
                    {
                        // remaining right joiners
                        // Reh, Zain, Thal,
                        if ( isRehChar ( cCh ) ||   // Reh Zain (right joining)
                                                    // final form may appear in the middle of word
                             ( 0x60C <= cCh && 0x6FE >= cCh // all others
                              && nIdx == nWordLen - 1))   // only at end of word
                        {
                            OSL_ENSURE( 0 != cPrevCh, "No previous character" );
                            // check if character is connectable to previous character,
                            if ( lcl_ConnectToPrev( cCh, cPrevCh ) )
                            {
                                nKashidaPos = aScanner.GetBegin() + nIdx - 1;
                                nPriorityLevel = 6;
                            }
                        }
                    }

                    // Do not consider Fathatan, Dammatan, Kasratan, Fatha,
                    // Damma, Kasra, Shadda and Sukun when checking if
                    // a character can be connected to previous character.
                    if ( !isTransparentChar ( cCh) )
                        cPrevCh = cCh;

                   ++nIdx;
                } // end of current word

                if ( STRING_LEN != nKashidaPos )
                    aKashida.Insert( nKashidaPos, nCntKash++ );
            } // end of kashida search
        }

        if ( nChg < rTxt.Len() )
            nScript = (BYTE)pBreakIt->GetBreakIter()->getScriptType( rTxt, nChg );

        nLastCompression = nChg;
        nLastKashida = nChg;
    };

#if OSL_DEBUG_LEVEL > 1
    // check kashida data
    long nTmpKashidaPos = -1;
    sal_Bool bWrongKash = sal_False;
    for (i = 0; i < aKashida.Count(); ++i )
    {
        long nCurrKashidaPos = GetKashida( i );
        if ( nCurrKashidaPos <= nTmpKashidaPos )
        {
            bWrongKash = sal_True;
            break;
        }
        nTmpKashidaPos = nCurrKashidaPos;
    }
    OSL_ENSURE( ! bWrongKash, "Kashida array contains wrong data" );
#endif

    // remove invalid entries from direction information arrays
    aDirectionChanges.clear();

    // Perform Unicode Bidi Algorithm for text direction information
    bool bPerformUBA = UBIDI_LTR != nDefaultDir;
    nCnt = 0;
    while( !bPerformUBA && nCnt < CountScriptChg() )
    {
        if ( i18n::ScriptType::COMPLEX == GetScriptType( nCnt++ ) )
            bPerformUBA = true;
    }

    // do not call the unicode bidi algorithm if not required
    if ( bPerformUBA )
    {
        UpdateBidiInfo( rTxt );

        // #i16354# Change script type for RTL text to CTL:
        // 1. All text in RTL runs will use the CTL font
        // #i89825# change the script type also to CTL (hennerdrewes)
        // 2. Text in embedded LTR runs that does not have any strong LTR characters (numbers!)
        for ( USHORT nDirIdx = 0; nDirIdx < aDirectionChanges.size(); ++nDirIdx )
        {
            const BYTE nCurrDirType = GetDirType( nDirIdx );
                // nStart ist start of RTL run:
                const xub_StrLen nStart = nDirIdx > 0 ? GetDirChg( nDirIdx - 1 ) : 0;
                // nEnd is end of RTL run:
                const xub_StrLen nEnd = GetDirChg( nDirIdx );

            if ( nCurrDirType % 2 == UBIDI_RTL  || // text in RTL run
                ( nCurrDirType > UBIDI_LTR && !lcl_HasStrongLTR( rTxt, nStart, nEnd ) ) ) // non-strong text in embedded LTR run
            {
                // nScriptIdx points into the ScriptArrays:
                USHORT nScriptIdx = 0;

                // Skip entries in ScriptArray which are not inside the RTL run:
                // Make nScriptIdx become the index of the script group with
                // 1. nStartPosOfGroup <= nStart and
                // 2. nEndPosOfGroup > nStart
                while ( GetScriptChg( nScriptIdx ) <= nStart )
                    ++nScriptIdx;

                const xub_StrLen nStartPosOfGroup = nScriptIdx ? GetScriptChg( nScriptIdx - 1 ) : 0;
                const BYTE nScriptTypeOfGroup = GetScriptType( nScriptIdx );

                OSL_ENSURE( nStartPosOfGroup <= nStart && GetScriptChg( nScriptIdx ) > nStart,
                        "Script override with CTL font trouble" );

                // Check if we have to insert a new script change at
                // position nStart. If nStartPosOfGroup < nStart,
                // we have to insert a new script change:
                if ( nStart > 0 && nStartPosOfGroup < nStart )
                {
                    aScriptChanges.insert(aScriptChanges.begin() + nScriptIdx,
                                          ScriptChangeInfo(nStart, nScriptTypeOfGroup) );
                    ++nScriptIdx;
                }

                // Remove entries in ScriptArray which end inside the RTL run:
                while ( nScriptIdx < aScriptChanges.size() && GetScriptChg( nScriptIdx ) <= nEnd )
                {
                    aScriptChanges.erase(aScriptChanges.begin() + nScriptIdx);
                }

                // Insert a new entry in ScriptArray for the end of the RTL run:
                aScriptChanges.insert(aScriptChanges.begin() + nScriptIdx,
                                      ScriptChangeInfo(nEnd, i18n::ScriptType::COMPLEX) );

#if OSL_DEBUG_LEVEL > 1
                // Check that ScriptChangeInfos are in increasing order of
                // position and that we don't have "empty" changes.
                BYTE nLastTyp = i18n::ScriptType::WEAK;
                xub_StrLen nLastPos = 0;
                for (aScriptChanges::const_iterator i2 = aScriptChanges.begin(); i2 < aScriptChanges.end(); ++i2)
                {
                    OSL_ENSURE( nLastTyp != i2->type &&
                            nLastPos < i2->position,
                            "Heavy InitScriptType() confusion" );
                    nLastPos = i2->position;
                    nLastTyp = i2->type;
                }
#endif
            }
        }
    }
}

void SwScriptInfo::UpdateBidiInfo( const String& rTxt )
{
    // remove invalid entries from direction information arrays
    aDirectionChanges.clear();
    //
    // Bidi functions from icu 2.0
    //
    UErrorCode nError = U_ZERO_ERROR;
    UBiDi* pBidi = ubidi_openSized( rTxt.Len(), 0, &nError );
    nError = U_ZERO_ERROR;

    ubidi_setPara( pBidi, reinterpret_cast<const UChar *>(rTxt.GetBuffer()), rTxt.Len(),    // UChar != sal_Unicode in MinGW
                   nDefaultDir, NULL, &nError );
    nError = U_ZERO_ERROR;
    long nCount = ubidi_countRuns( pBidi, &nError );
    int32_t nStart = 0;
    int32_t nEnd;
    UBiDiLevel nCurrDir;
    for ( USHORT nIdx = 0; nIdx < nCount; ++nIdx )
    {
        ubidi_getLogicalRun( pBidi, nStart, &nEnd, &nCurrDir );
        aDirectionChanges.push_back( DirectionChangeInfo(nEnd, nCurrDir) );
        nStart = nEnd;
    }

    ubidi_close( pBidi );
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

BYTE SwScriptInfo::ScriptType( const xub_StrLen nPos ) const
{
    USHORT nEnd = CountScriptChg();
    for( USHORT nX = 0; nX < nEnd; ++nX )
    {
        if( nPos < GetScriptChg( nX ) )
            return GetScriptType( nX );
    }

    // the default is the application language script
    return (BYTE)GetI18NScriptTypeOfLanguage( (USHORT)GetAppLanguage() );
}

xub_StrLen SwScriptInfo::NextDirChg( const xub_StrLen nPos,
                                     const BYTE* pLevel )  const
{
    BYTE nCurrDir = pLevel ? *pLevel : 62;
    USHORT nEnd = CountDirChg();
    for( USHORT nX = 0; nX < nEnd; ++nX )
    {
        if( nPos < GetDirChg( nX ) &&
            ( nX + 1 == nEnd || GetDirType( nX + 1 ) <= nCurrDir ) )
            return GetDirChg( nX );
    }

    return STRING_LEN;
}

BYTE SwScriptInfo::DirType( const xub_StrLen nPos ) const
{
    USHORT nEnd = CountDirChg();
    for( USHORT nX = 0; nX < nEnd; ++nX )
    {
        if( nPos < GetDirChg( nX ) )
            return GetDirType( nX );
    }

    return 0;
}

/*************************************************************************
 *                        SwScriptInfo::MaskHiddenRanges(..)
 * Takes a string and replaced the hidden ranges with cChar.
 **************************************************************************/

USHORT SwScriptInfo::MaskHiddenRanges( const SwTxtNode& rNode, XubString& rText,
                                       const xub_StrLen nStt, const xub_StrLen nEnd,
                                       const xub_Unicode cChar )
{
    OSL_ENSURE( rNode.GetTxt().Len() == rText.Len(), "MaskHiddenRanges, string len mismatch" );

    PositionList aList;
    xub_StrLen nHiddenStart;
    xub_StrLen nHiddenEnd;
    USHORT nNumOfHiddenChars = 0;
    GetBoundsOfHiddenRange( rNode, 0, nHiddenStart, nHiddenEnd, &aList );
    PositionList::const_reverse_iterator rFirst( aList.end() );
    PositionList::const_reverse_iterator rLast( aList.begin() );
    while ( rFirst != rLast )
    {
        nHiddenEnd = *(rFirst++);
        nHiddenStart = *(rFirst++);

        if ( nHiddenEnd < nStt || nHiddenStart > nEnd )
            continue;

        while ( nHiddenStart < nHiddenEnd && nHiddenStart < nEnd )
        {
            if ( nHiddenStart >= nStt && nHiddenStart < nEnd )
            {
                rText.SetChar( nHiddenStart, cChar );
                ++nNumOfHiddenChars;
            }
            ++nHiddenStart;
        }
    }

    return nNumOfHiddenChars;
}

/*************************************************************************
 *                        SwScriptInfo::DeleteHiddenRanges(..)
 * Takes a SwTxtNode and deletes the hidden ranges from the node.
 **************************************************************************/

void SwScriptInfo::DeleteHiddenRanges( SwTxtNode& rNode )
{
    PositionList aList;
    xub_StrLen nHiddenStart;
    xub_StrLen nHiddenEnd;
    GetBoundsOfHiddenRange( rNode, 0, nHiddenStart, nHiddenEnd, &aList );
    PositionList::const_reverse_iterator rFirst( aList.end() );
    PositionList::const_reverse_iterator rLast( aList.begin() );
    while ( rFirst != rLast )
    {
        nHiddenEnd = *(rFirst++);
        nHiddenStart = *(rFirst++);

        SwPaM aPam( rNode, nHiddenStart, rNode, nHiddenEnd );
        rNode.getIDocumentContentOperations()->DeleteRange( aPam );
    }
}

/*************************************************************************
 *                        SwScriptInfo::GetBoundsOfHiddenRange(..)
 * static version
 **************************************************************************/

bool SwScriptInfo::GetBoundsOfHiddenRange( const SwTxtNode& rNode, xub_StrLen nPos,
                                           xub_StrLen& rnStartPos, xub_StrLen& rnEndPos,
                                           PositionList* pList )
{
    rnStartPos = STRING_LEN;
    rnEndPos = 0;

    bool bNewContainsHiddenChars = false;

    //
    // Optimization: First examine the flags at the text node:
    //
    if ( !rNode.IsCalcHiddenCharFlags() )
    {
        bool bWholePara = rNode.HasHiddenCharAttribute( true );
        bool bContainsHiddenChars = rNode.HasHiddenCharAttribute( false );
        if ( !bContainsHiddenChars )
            return false;

        if ( bWholePara )
        {
            if ( pList )
            {
                pList->push_back( 0 );
                pList->push_back( rNode.GetTxt().Len() );
            }

            rnStartPos = 0;
            rnEndPos = rNode.GetTxt().Len();
            return true;
        }
    }

    const SwScriptInfo* pSI = SwScriptInfo::GetScriptInfo( rNode );
    if ( pSI )
    {
        //
        // Check first, if we have a valid SwScriptInfo object for this text node:
        //
        bNewContainsHiddenChars = pSI->GetBoundsOfHiddenRange( nPos, rnStartPos, rnEndPos, pList );
        const bool bNewHiddenCharsHidePara = ( rnStartPos == 0 && rnEndPos >= rNode.GetTxt().Len() );
        rNode.SetHiddenCharAttribute( bNewHiddenCharsHidePara, bNewContainsHiddenChars );
    }
    else
    {
        //
        // No valid SwScriptInfo Object, we have to do it the hard way:
        //
        Range aRange( 0, rNode.GetTxt().Len() ? rNode.GetTxt().Len() - 1 : 0 );
        MultiSelection aHiddenMulti( aRange );
        SwScriptInfo::CalcHiddenRanges( rNode, aHiddenMulti );
        for( USHORT i = 0; i < aHiddenMulti.GetRangeCount(); ++i )
        {
            const Range& rRange = aHiddenMulti.GetRange( i );
            const xub_StrLen nHiddenStart = (xub_StrLen)rRange.Min();
            const xub_StrLen nHiddenEnd = (xub_StrLen)rRange.Max() + 1;

            if ( nHiddenStart > nPos )
                break;
            else if ( nHiddenStart <= nPos && nPos < nHiddenEnd )
            {
                rnStartPos = nHiddenStart;
                rnEndPos   = Min( nHiddenEnd, rNode.GetTxt().Len() );
                break;
            }
        }

        if ( pList )
        {
            for( USHORT i = 0; i < aHiddenMulti.GetRangeCount(); ++i )
            {
                const Range& rRange = aHiddenMulti.GetRange( i );
                pList->push_back( (xub_StrLen)rRange.Min() );
                pList->push_back( (xub_StrLen)rRange.Max() + 1 );
            }
        }

        bNewContainsHiddenChars = aHiddenMulti.GetRangeCount() > 0;
    }

    return bNewContainsHiddenChars;
}

/*************************************************************************
 *                        SwScriptInfo::GetBoundsOfHiddenRange(..)
 * non-static version
 **************************************************************************/

bool SwScriptInfo::GetBoundsOfHiddenRange( xub_StrLen nPos, xub_StrLen& rnStartPos,
                                           xub_StrLen& rnEndPos, PositionList* pList ) const
{
    rnStartPos = STRING_LEN;
    rnEndPos = 0;

    USHORT nEnd = CountHiddenChg();
    for( USHORT nX = 0; nX < nEnd; ++nX )
    {
        const xub_StrLen nHiddenStart = GetHiddenChg( nX++ );
        const xub_StrLen nHiddenEnd = GetHiddenChg( nX );

        if ( nHiddenStart > nPos )
            break;
        else if ( nHiddenStart <= nPos && nPos < nHiddenEnd )
        {
            rnStartPos = nHiddenStart;
            rnEndPos   = nHiddenEnd;
            break;
        }
    }

    if ( pList )
    {
        for( USHORT nX = 0; nX < nEnd; ++nX )
        {
            pList->push_back( GetHiddenChg( nX++ ) );
            pList->push_back( GetHiddenChg( nX ) );
        }
    }

    return CountHiddenChg() > 0;
}

/*************************************************************************
 *                        SwScriptInfo::IsInHiddenRange()
 **************************************************************************/

bool SwScriptInfo::IsInHiddenRange( const SwTxtNode& rNode, xub_StrLen nPos )
{
    xub_StrLen nStartPos;
    xub_StrLen nEndPos;
    SwScriptInfo::GetBoundsOfHiddenRange( rNode, nPos, nStartPos, nEndPos );
    return nStartPos != STRING_LEN;
}


#if OSL_DEBUG_LEVEL > 1
/*************************************************************************
 *                        SwScriptInfo::CompType(..)
 * returns the type of the compressed character
 *************************************************************************/

BYTE SwScriptInfo::CompType( const xub_StrLen nPos ) const
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
#endif

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

long SwScriptInfo::Compress( sal_Int32* pKernArray, xub_StrLen nIdx, xub_StrLen nLen,
                             const USHORT nCompress, const USHORT nFontHeight,
                             Point* pPoint ) const
{
    OSL_ENSURE( nCompress, "Compression without compression?!" );
    OSL_ENSURE( nLen, "Compression without text?!" );
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
    nLen = nLen + nIdx;

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
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( nType == CompType( nIdx ), "Gimme the right type!" );
#endif
        nCompLen = nCompLen + nIdx;
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
                OSL_ENSURE( SwScriptInfo::NONE != nType, "None compression?!" );

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
            xub_StrLen nTmpChg;
            if( ++nCompIdx < nCompCount )
            {
                nTmpChg = GetCompStart( nCompIdx );
                if( nTmpChg > nLen )
                    nTmpChg = nLen;
                nCompLen = GetCompLen( nCompIdx );
            }
            else
                nTmpChg = nLen;
            while( nIdx < nTmpChg )
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
 *                      SwScriptInfo::KashidaJustify()
 *************************************************************************/

// Note on calling KashidaJustify():
// Kashida positions may be marked as invalid. Therefore KashidaJustify may return the clean
// total number of kashida positions, or the number of kashida positions after some positions
// have been dropped, depending on the state of the aKashidaInvalid array.

USHORT SwScriptInfo::KashidaJustify( sal_Int32* pKernArray,
                                    sal_Int32* pScrArray,
                                    xub_StrLen nStt,
                                    xub_StrLen nLen,
                                    long nSpaceAdd ) const
{
    OSL_ENSURE( nLen, "Kashida justification without text?!" );

    if( !IsKashidaLine(nStt))
        return STRING_LEN;

    // evaluate kashida informatin in collected in SwScriptInfo

    USHORT nCntKash = 0;
    while( nCntKash < CountKashida() )
    {
        if ( nStt <= GetKashida( nCntKash ) )
            break;
        else
            nCntKash++;
    }

    const xub_StrLen nEnd = nStt + nLen;

    USHORT nCntKashEnd = nCntKash;
    while ( nCntKashEnd < CountKashida() )
    {
       if ( nEnd <= GetKashida( nCntKashEnd ) )
            break;
        else
            nCntKashEnd++;
    }

    USHORT nActualKashCount = nCntKashEnd - nCntKash;
    for ( USHORT i = nCntKash; i < nCntKashEnd; ++i )
    {
        if ( nActualKashCount && !IsKashidaValid ( i ) )
            --nActualKashCount;
    }

    if ( !pKernArray )
        return nActualKashCount;

    // do nothing if there is no more kashida
    if ( nCntKash < CountKashida() )
    {
        // skip any invalid kashidas
        while ( ! IsKashidaValid ( nCntKash ) && nCntKash < nCntKashEnd )
            ++nCntKash;

        xub_StrLen nKashidaPos = GetKashida( nCntKash );
        xub_StrLen nIdx = nKashidaPos;
        long nKashAdd = nSpaceAdd;

        while ( nIdx < nEnd )
        {
            USHORT nArrayPos = nIdx - nStt;

            // next kashida position
            ++nCntKash;
            while ( ! IsKashidaValid ( nCntKash ) && nCntKash < nCntKashEnd )
                ++nCntKash;

            nIdx = nCntKash < CountKashida() && IsKashidaValid ( nCntKash ) ? GetKashida( nCntKash ) : nEnd;
            if ( nIdx > nEnd )
                nIdx = nEnd;

            const USHORT nArrayEnd = nIdx - nStt;

            while ( nArrayPos < nArrayEnd )
            {
                pKernArray[ nArrayPos ] += nKashAdd;
                if ( pScrArray )
                    pScrArray[ nArrayPos ] += nKashAdd;
                ++nArrayPos;
            }
            nKashAdd += nSpaceAdd;
        }
    }

    return 0;
}

/*************************************************************************
 *                      SwScriptInfo::IsArabicText()
 *
 * Checks if the current text is 'Arabic' text. Note that only the first
 * character has to be checked because a ctl portion only contains one
 * script, see NewTxtPortion
 *************************************************************************/
sal_Bool SwScriptInfo::IsArabicText( const XubString& rTxt, xub_StrLen nStt, xub_StrLen nLen )
{
    using namespace ::com::sun::star::i18n;
    static ScriptTypeList typeList[] = {
        { UnicodeScript_kArabic, UnicodeScript_kArabic, UnicodeScript_kArabic },        // 11,
        { UnicodeScript_kScriptCount, UnicodeScript_kScriptCount, UnicodeScript_kScriptCount }    // 88
    };

    // go forward if current position does not hold a regular character:
    const CharClass& rCC = GetAppCharClass();
    sal_Int32 nIdx = nStt;
    const xub_StrLen nEnd = nStt + nLen;
    while ( nIdx < nEnd && !rCC.isLetterNumeric( rTxt, (xub_StrLen)nIdx ) )
    {
        ++nIdx;
    }

    if( nIdx == nEnd )
    {
        // no regular character found in this portion. Go backward:
        --nIdx;
        while ( nIdx >= 0 && !rCC.isLetterNumeric( rTxt, (xub_StrLen)nIdx ) )
        {
            --nIdx;
        }
    }

    if( nIdx >= 0 )
    {
        const xub_Unicode cCh = rTxt.GetChar( (xub_StrLen)nIdx );
        const sal_Int16 type = unicode::getUnicodeScriptType( cCh, typeList, UnicodeScript_kScriptCount );
        return type == UnicodeScript_kArabic;
    }
    return sal_False;
}

/*************************************************************************
 *                      SwScriptInfo::IsKashidaValid()
 *************************************************************************/

sal_Bool SwScriptInfo::IsKashidaValid ( xub_StrLen nKashPos ) const
{
    for ( xub_StrLen i = 0; i < aKashidaInvalid.Count(); ++i )
    {
        if ( aKashidaInvalid [ i ] == nKashPos )
            return false;
    }
    return true;
}

/*************************************************************************
 *                      SwScriptInfo::ClearKashidaInvalid()
 *************************************************************************/

void SwScriptInfo::ClearKashidaInvalid (  xub_StrLen nKashPos )
{
    for ( xub_StrLen i = 0; i < aKashidaInvalid.Count(); ++i )
    {
        if ( aKashidaInvalid [ i ] == nKashPos )
        {
           aKashidaInvalid.Remove (i, 1);
           return;
        }
    }
}

/*************************************************************************
 *                      SwScriptInfo::MarkOrClearKashidaInvalid()
 *************************************************************************/
// bMark == true:
// marks the first valid kashida in the given text range as invalid

// bMark == false:
// clears all kashida invalid flags in the given text range

bool SwScriptInfo::MarkOrClearKashidaInvalid ( xub_StrLen nStt, xub_StrLen nLen, bool bMark, xub_StrLen nMarkCount )
{
    USHORT nCntKash = 0;
    while( nCntKash < CountKashida() )
    {
        if ( nStt <= GetKashida( nCntKash ) )
            break;
        else
            nCntKash++;
    }

    const xub_StrLen nEnd = nStt + nLen;

    while ( nCntKash < CountKashida() )
    {
        if ( nEnd <= GetKashida( nCntKash ) )
            break;
        else
        {
            if(bMark)
            {
                if ( IsKashidaValid ( nCntKash ) )
                {
                    MarkKashidaInvalid ( nCntKash );
                    --nMarkCount;
                    if(!nMarkCount)
                       return true;
                }
            }
            else
            {
                ClearKashidaInvalid ( nCntKash );
            }
            nCntKash++;
        }
    }
    return false;
}

void SwScriptInfo::MarkKashidaInvalid ( xub_StrLen nKashPos )
{
    aKashidaInvalid.Insert( nKashPos, aKashidaInvalid.Count() );
}

/*************************************************************************
 *                      SwScriptInfo::GetKashidaPositions()
 *************************************************************************/
// retrieve the kashida positions in the given text range
USHORT SwScriptInfo::GetKashidaPositions ( xub_StrLen nStt, xub_StrLen nLen,
                                           xub_StrLen* pKashidaPosition )
{
    USHORT nCntKash = 0;
    while( nCntKash < CountKashida() )
    {
        if ( nStt <= GetKashida( nCntKash ) )
            break;
        else
            nCntKash++;
    }

    const xub_StrLen nEnd = nStt + nLen;

    USHORT nCntKashEnd = nCntKash;
    while ( nCntKashEnd < CountKashida() )
    {
       if ( nEnd <= GetKashida( nCntKashEnd ) )
            break;
        else
        {
            pKashidaPosition [ nCntKashEnd - nCntKash ] = GetKashida ( nCntKashEnd );
            nCntKashEnd++;
        }
    }
    return nCntKashEnd - nCntKash;
}

void SwScriptInfo::SetNoKashidaLine ( xub_StrLen nStt, xub_StrLen nLen )
{
    aNoKashidaLine.Insert( nStt, aNoKashidaLine.Count());
    aNoKashidaLineEnd.Insert( nStt+nLen, aNoKashidaLineEnd.Count());
}

/*************************************************************************
 *                      SwScriptInfo::IsKashidaLine()
 *************************************************************************/
// determines if the line uses kashida justification

bool SwScriptInfo::IsKashidaLine ( xub_StrLen nCharIdx ) const
{
   for( xub_StrLen i = 0; i < aNoKashidaLine.Count(); ++i )
    {
       if( nCharIdx >= aNoKashidaLine[ i ] && nCharIdx < aNoKashidaLineEnd[ i ])
           return false;
    }
   return true;
}
/*************************************************************************
 *                      SwScriptInfo::ClearKashidaLine()
 *************************************************************************/

void SwScriptInfo::ClearNoKashidaLine ( xub_StrLen nStt, xub_StrLen nLen )
{
   xub_StrLen i = 0;
   while( i < aNoKashidaLine.Count())
   {
       if( nStt + nLen >= aNoKashidaLine[ i ] && nStt < aNoKashidaLineEnd [ i ] )
       {
           aNoKashidaLine.Remove(i, 1);
           aNoKashidaLineEnd.Remove(i, 1);
       }
       else
           ++i;
   }
}

/*************************************************************************
 *                      SwScriptInfo::MarkKashidasInvalid()
 *************************************************************************/
// mark the given character indices as invalid kashida positions
bool SwScriptInfo::MarkKashidasInvalid ( xub_StrLen nCnt, xub_StrLen* pKashidaPositions )
{
   OSL_ENSURE( pKashidaPositions && nCnt > 0, "Where are kashidas?" );

   USHORT nCntKash = 0;
   xub_StrLen nKashidaPosIdx = 0;

    while ( nCntKash < CountKashida() && nKashidaPosIdx < nCnt )
    {
       if ( pKashidaPositions [nKashidaPosIdx] > GetKashida( nCntKash ) )
       {
           nCntKash++;
           continue;
       }

        if ( pKashidaPositions [nKashidaPosIdx] == GetKashida( nCntKash ) && IsKashidaValid ( nCntKash ) )
       {
            MarkKashidaInvalid ( nCntKash );
       }
       else
           return false; // something is wrong
       nKashidaPosIdx++;
   }
   return true;
}

/*************************************************************************
 *                      SwScriptInfo::ThaiJustify()
 *************************************************************************/

USHORT SwScriptInfo::ThaiJustify( const XubString& rTxt, sal_Int32* pKernArray,
                                  sal_Int32* pScrArray, xub_StrLen nStt,
                                  xub_StrLen nLen, xub_StrLen nNumberOfBlanks,
                                  long nSpaceAdd )
{
    OSL_ENSURE( nStt + nLen <= rTxt.Len(), "String in ThaiJustify too small" );

    SwTwips nNumOfTwipsToDistribute = nSpaceAdd * nNumberOfBlanks /
                                      SPACING_PRECISION_FACTOR;

    long nSpaceSum = 0;
    USHORT nCnt = 0;

    for ( USHORT nI = 0; nI < nLen; ++nI )
    {
        const xub_Unicode cCh = rTxt.GetChar( nStt + nI );

        // check if character is not above or below base
        if ( ( 0xE34 > cCh || cCh > 0xE3A ) &&
             ( 0xE47 > cCh || cCh > 0xE4E ) && cCh != 0xE31 )
        {
            if ( nNumberOfBlanks > 0 )
            {
                nSpaceAdd = nNumOfTwipsToDistribute / nNumberOfBlanks;
                --nNumberOfBlanks;
                nNumOfTwipsToDistribute -= nSpaceAdd;
            }
            nSpaceSum += nSpaceAdd;
            ++nCnt;
        }

        if ( pKernArray ) pKernArray[ nI ] += nSpaceSum;
        if ( pScrArray ) pScrArray[ nI ] += nSpaceSum;
    }

    return nCnt;
}

/*************************************************************************
 *                      SwScriptInfo::GetScriptInfo()
 *************************************************************************/

SwScriptInfo* SwScriptInfo::GetScriptInfo( const SwTxtNode& rTNd,
                                           sal_Bool bAllowInvalid )
{
    SwClientIter aClientIter( (SwTxtNode&)rTNd );
    SwClient* pLast = aClientIter.GoStart();
    SwScriptInfo* pScriptInfo = 0;

    while( pLast )
    {
        if ( pLast->ISA( SwTxtFrm ) )
        {
            pScriptInfo = (SwScriptInfo*)((SwTxtFrm*)pLast)->GetScriptInfo();
            if ( pScriptInfo )
            {
                if ( !bAllowInvalid && STRING_LEN != pScriptInfo->GetInvalidity() )
                    pScriptInfo = 0;
                else break;
            }
        }
        pLast = ++aClientIter;
    }

    return pScriptInfo;
}

/*************************************************************************
 *                      SwParaPortion::SwParaPortion()
 *************************************************************************/
SwParaPortion::SwParaPortion()
{
    FormatReset();
    bFlys = bFtnNum = bMargin = sal_False;
    SetWhichPor( POR_PARA );
}

/*************************************************************************
 *                      SwParaPortion::~SwParaPortion()
 *************************************************************************/
SwParaPortion::~SwParaPortion()
{
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
        nLen = nLen + pLay->GetLen();
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

/*--------------------------------------------------
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
        // the last post its portion
        else if ( pPor->IsPostItsPortion() && ! pPor->GetPortion() )
            nDiff = nAscent;

        pPor = pPor->GetPortion();
    }
    if( !bFound ) // actualize the hanging-flag
        ((SwLineLayout*)this)->SetHanging( sal_False );
    return nDiff;
}

SwTwips SwTxtFrm::HangingMargin() const
{
    OSL_ENSURE( HasPara(), "Don't call me without a paraportion" );
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


/*************************************************************************
 *                      SwScriptInfo::CalcHiddenRanges()
 *
 * Returns a MultiSection indicating the hidden ranges.
 *************************************************************************/

void SwScriptInfo::CalcHiddenRanges( const SwTxtNode& rNode, MultiSelection& rHiddenMulti )
{
    const SfxPoolItem* pItem = 0;
    if( SFX_ITEM_SET == rNode.GetSwAttrSet().GetItemState( RES_CHRATR_HIDDEN, TRUE, &pItem ) &&
        ((SvxCharHiddenItem*)pItem)->GetValue() )
    {
        rHiddenMulti.SelectAll();
    }

    const SwpHints* pHints = rNode.GetpSwpHints();
    const SwTxtAttr* pTxtAttr = 0;

    if( pHints )
    {
        MSHORT nTmp = 0;

        while( nTmp < pHints->GetStartCount() )
        {
            pTxtAttr = pHints->GetStart( nTmp++ );
            const SvxCharHiddenItem* pHiddenItem = static_cast<const SvxCharHiddenItem*>( CharFmt::GetItem( *pTxtAttr, RES_CHRATR_HIDDEN ) );
            if( pHiddenItem )
            {
                xub_StrLen nSt = *pTxtAttr->GetStart();
                xub_StrLen nEnd = *pTxtAttr->GetEnd();
                if( nEnd > nSt )
                {
                    Range aTmp( nSt, nEnd - 1 );
                    rHiddenMulti.Select( aTmp, pHiddenItem->GetValue() );
                }
            }
        }
    }

    // If there are any hidden ranges in the current text node, we have
    // to unhide the redlining ranges:
    const IDocumentRedlineAccess& rIDRA = *rNode.getIDocumentRedlineAccess();
    if ( rHiddenMulti.GetRangeCount() && IDocumentRedlineAccess::IsShowChanges( rIDRA.GetRedlineMode() ) )
    {
        USHORT nAct = rIDRA.GetRedlinePos( rNode, USHRT_MAX );

        for ( ; nAct < rIDRA.GetRedlineTbl().Count(); nAct++ )
        {
            const SwRedline* pRed = rIDRA.GetRedlineTbl()[ nAct ];

            if ( pRed->Start()->nNode > rNode.GetIndex() )
                break;

            xub_StrLen nRedlStart;
            xub_StrLen nRedlnEnd;
            pRed->CalcStartEnd( rNode.GetIndex(), nRedlStart, nRedlnEnd );
            if ( nRedlnEnd > nRedlStart )
            {
                Range aTmp( nRedlStart, nRedlnEnd - 1 );
                rHiddenMulti.Select( aTmp, false );
            }
        }
    }

    //
    // We calculated a lot of stuff. Finally we can update the flags at the text node.
    //
    const bool bNewContainsHiddenChars = rHiddenMulti.GetRangeCount() > 0;
    bool bNewHiddenCharsHidePara = false;
    if ( bNewContainsHiddenChars )
    {
        const Range& rRange = rHiddenMulti.GetRange( 0 );
        const xub_StrLen nHiddenStart = (xub_StrLen)rRange.Min();
        const xub_StrLen nHiddenEnd = (xub_StrLen)rRange.Max() + 1;
        bNewHiddenCharsHidePara = ( nHiddenStart == 0 && nHiddenEnd >= rNode.GetTxt().Len() );
    }
    rNode.SetHiddenCharAttribute( bNewHiddenCharsHidePara, bNewContainsHiddenChars );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
