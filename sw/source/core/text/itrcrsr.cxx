/*************************************************************************
 *
 *  $RCSfile: itrcrsr.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: ama $ $Date: 2000-11-28 14:28:24 $
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

#include "hintids.hxx"
#include "errhdl.hxx"
#include "ndtxt.hxx"
#include "frmfmt.hxx"
#include "paratr.hxx"
#include "flyfrm.hxx"

#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_LSPCITEM_HXX //autogen
#include <svx/lspcitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif

#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#include "txtcfg.hxx"
#include "itrtxt.hxx"

#include "txtfrm.hxx"
#include "flyfrms.hxx"
#include "porglue.hxx"      // SwFlyCnt
#include "porfld.hxx"       // SwFldPortion::IsFollow()
#include "porfly.hxx"       // GetFlyCrsrOfst()
#include "pordrop.hxx"
#include "crstate.hxx"      // SwCrsrMoveState
#ifndef _PORMULTI_HXX
#include <pormulti.hxx>     // SwMultiPortion
#endif

// Nicht reentrant !!!
// wird in GetCharRect gesetzt und im UnitUp/Down ausgewertet.
sal_Bool SwTxtCursor::bRightMargin = sal_False;

/*************************************************************************
 *                SwTxtMargin::CtorInit()
 *************************************************************************/
void SwTxtMargin::CtorInit( SwTxtFrm *pFrm, SwTxtSizeInfo *pNewInf )
{
    SwTxtIter::CtorInit( pFrm, pNewInf );

    pInf = pNewInf;
    GetInfo().SetFont( GetFnt() );
    SwTxtNode *pNode = pFrm->GetTxtNode();

    const SvxLRSpaceItem &rSpace =
        pFrm->GetTxtNode()->GetSwAttrSet().GetLRSpace();
    nRight = pFrm->Frm().Left() + pFrm->Prt().Left() + pFrm->Prt().Width();
    nLeft = Max( long( rSpace.GetTxtLeft() + pNode->GetLeftMarginWithNum(sal_True)),
                 pFrm->Prt().Left() ) + pFrm->Frm().Left();
    if( nLeft >= nRight )
        nLeft = pFrm->Prt().Left() + pFrm->Frm().Left();
    if( nLeft >= nRight ) // z.B. bei grossen Absatzeinzuegen in schmalen Tabellenspalten
        nRight = nLeft + 1; // einen goennen wir uns immer
    if( pFrm->IsFollow() && pFrm->GetOfst() )
        nFirst = nLeft;
    else
    {
        short nFLOfst;
        long nFirstLineOfs;
        if( !pNode->GetFirstLineOfsWithNum( nFLOfst ) &&
            rSpace.IsAutoFirst() )
        {
            nFirstLineOfs = GetFnt()->GetSize( GetFnt()->GetActual() ).Height();
            const SvxLineSpacingItem *pSpace = aLineInf.GetLineSpacing();
            if( pSpace )
            {
                switch( pSpace->GetLineSpaceRule() )
                {
                    case SVX_LINE_SPACE_AUTO:
                    break;
                    case SVX_LINE_SPACE_MIN:
                    {
                        if( nFirstLineOfs < KSHORT( pSpace->GetLineHeight() ) )
                            nFirstLineOfs = pSpace->GetLineHeight();
                        break;
                    }
                    case SVX_LINE_SPACE_FIX:
                        nFirstLineOfs = pSpace->GetLineHeight();
                    break;
                    default: ASSERT( sal_False, ": unknown LineSpaceRule" );
                }
                switch( pSpace->GetInterLineSpaceRule() )
                {
                    case SVX_INTER_LINE_SPACE_OFF:
                    break;
                    case SVX_INTER_LINE_SPACE_PROP:
                    {
                        long nTmp = pSpace->GetPropLineSpace();
                        // 50% ist das Minimum, bei 0% schalten wir auf
                        // den Defaultwert 100% um ...
                        if( nTmp < 50 )
                            nTmp = nTmp ? 50 : 100;

                        nTmp *= nFirstLineOfs;
                        nTmp /= 100;
                        if( !nTmp )
                            ++nTmp;
                        nFirstLineOfs = (KSHORT)nTmp;
                        break;
                    }
                    case SVX_INTER_LINE_SPACE_FIX:
                    {
                        nFirstLineOfs += pSpace->GetInterLineSpace();
                        break;
                    }
                    default: ASSERT( sal_False, ": unknown InterLineSpaceRule" );
                }
            }
        }
        else
            nFirstLineOfs = nFLOfst;
        nFirst = Max( rSpace.GetTxtLeft() + pNode->GetLeftMarginWithNum( sal_True )
            + nFirstLineOfs, pFrm->Prt().Left() ) + pFrm->Frm().Left();
        if( nFirst >= nRight )
            nFirst = nRight - 1;
    }
    const SvxAdjustItem& rAdjust = pFrm->GetTxtNode()->GetSwAttrSet().GetAdjust();
    nAdjust = rAdjust.GetAdjust();
    bOneBlock = rAdjust.GetOneWord() == SVX_ADJUST_BLOCK;
    bLastBlock = rAdjust.GetLastBlock() == SVX_ADJUST_BLOCK;
    bLastCenter = rAdjust.GetLastBlock() == SVX_ADJUST_CENTER;
#ifdef DEBUG
    static sal_Bool bOne = sal_False;
    static sal_Bool bLast = sal_False;
    static sal_Bool bCenter = sal_False;
    bOneBlock |= bOne;
    bLastBlock |= bLast;
    bLastCenter |= bCenter;
#endif
    DropInit();
}

/*************************************************************************
 *                SwTxtMargin::DropInit()
 *************************************************************************/
void SwTxtMargin::DropInit()
{
    nDropLeft = nDropLines = nDropHeight = nDropDescent = 0;
    const SwParaPortion *pPara = GetInfo().GetParaPortion();
    if( pPara )
    {
        const SwDropPortion *pPorDrop = pPara->FindDropPortion();
        if ( pPorDrop )
        {
            nDropLeft = pPorDrop->GetDropLeft();
            nDropLines = pPorDrop->GetLines();
            nDropHeight = pPorDrop->GetDropHeight();
            nDropDescent = pPorDrop->GetDropDescent();
        }
    }
}

/*************************************************************************
 *                SwTxtMargin::GetLineStart()
 *************************************************************************/

// Unter Beruecksichtigung des Erstzeileneinzuges und der angebenen Breite.
SwTwips SwTxtMargin::GetLineStart() const
{
    SwTwips nRet = GetLeftMargin();
    if( GetAdjust() != SVX_ADJUST_LEFT &&
        !pCurr->GetFirstPortion()->IsMarginPortion() )
    {
        // Wenn die erste Portion ein Margin ist, dann wird das
        // Adjustment durch die Portions ausgedrueckt.
        if( GetAdjust() == SVX_ADJUST_RIGHT )
            nRet = Right() - CurrWidth();
        else if( GetAdjust() == SVX_ADJUST_CENTER )
            nRet += (GetLineWidth() - CurrWidth()) / 2;
    }
    return nRet;
}

/*************************************************************************
 *                      SwTxtCursor::CtorInit()
 *************************************************************************/
void SwTxtCursor::CtorInit( SwTxtFrm *pFrm, SwTxtSizeInfo *pInf )
{
    SwTxtMargin::CtorInit( pFrm, pInf );
    // 6096: Vorsicht, die Iteratoren sind abgeleitet!
    // GetInfo().SetOut( GetInfo().GetWin() );
}

/*************************************************************************
 *                      SwTxtCursor::GetEndCharRect()
 *************************************************************************/

// 1170: Antikbug: Shift-Ende vergisst das letzte Zeichen ...

sal_Bool SwTxtCursor::GetEndCharRect( SwRect* pOrig, const xub_StrLen nOfst,
                                  SwCrsrMoveState* pCMS, const long nMax )
{
    // 1170: Mehrdeutigkeit von Dokumentpositionen
    bRightMargin = sal_True;
    CharCrsrToLine(nOfst);

    // Etwas verdreht: nOfst bezeichnet die Position hinter dem letzten
    // Zeichen der letzten Zeile == Position vor dem ersten Zeichen der
    // Zeile in der wir gerade stehen:
    if( nOfst != GetStart() || !pCurr->GetLen() )
    {
        // 8810: Masterzeile RightMargin, danach LeftMargin
        const sal_Bool bRet = GetCharRect( pOrig, nOfst, pCMS, nMax );
        bRightMargin = nOfst >= GetEnd() && nOfst < GetInfo().GetTxt().Len();
        return bRet;
    }

    if( !GetPrev() || !GetPrev()->GetLen() || !PrevLine() )
        return GetCharRect( pOrig, nOfst, pCMS, nMax );

    // Adjustierung ggf. nachholen
    GetAdjusted();

    KSHORT nX = 0;
    KSHORT nLast = 0;
    SwLinePortion *pPor = pCurr->GetFirstPortion();

    KSHORT nTmpHeight, nTmpAscent;
    CalcAscentAndHeight( nTmpAscent, nTmpHeight );
    KSHORT nPorHeight = nTmpHeight;
    KSHORT nPorAscent = nTmpAscent;

    // Die letzte Text/EndPortion der Zeile suchen
    while( pPor )
    {
        nX += pPor->Width();
        if( pPor->InTxtGrp() || ( pPor->GetLen() && !pPor->IsFlyPortion()
            && !pPor->IsHolePortion() ) || pPor->IsBreakPortion() )
        {
            nLast = nX;
            nPorHeight = pPor->Height();
            nPorAscent = pPor->GetAscent();
        }
        pPor = pPor->GetPortion();
    }

    const Size aCharSize( 1, nTmpHeight );
    pOrig->Pos( GetTopLeft() );
    pOrig->SSize( aCharSize );
    pOrig->Pos().X() += nLast;
    const SwTwips nRight = Right() - 1;
    if( pOrig->Left() > nRight )
        pOrig->Pos().X() = nRight;

    if ( pCMS && pCMS->bRealHeight )
    {
        if ( nTmpAscent > nPorAscent )
            pCMS->aRealHeight.X() = nTmpAscent - nPorAscent;
        else
            pCMS->aRealHeight.X() = 0;
        ASSERT( nPorHeight, "GetCharRect: Missing Portion-Height" );
        pCMS->aRealHeight.Y() = nPorHeight;
    }

    return sal_True;
}

/*************************************************************************
 *                      SwTxtCursor::GetCharRect()
 *
 * Tabs: Ist das aktuelle Zeichen das Tabzeichen, dann liefert
 * GetCharRect() das SwRect hinter dem letzten Character in
 * der vorhergehenden Portion. Ist das aktuelle Zeichen das
 * Zeichen nach dem Tabzeichen, so wird das SwRect am Anfang
 * der Portion-Box geliefert, weil das Tabzeichen selber
 * unsichtbar ist. Oder andersherum: Wenn eine Portion mit einem
 * Tabzeichen beginnt, dann darf dieses Tabzeichen bei der
 * Berechnung des x-Offset der Characterbox innerhalb der Portion
 * nicht beruecksichtigt werden.
 *
 *************************************************************************/
sal_Bool SwTxtCursor::GetCharRect( SwRect* pOrig, const xub_StrLen nOfst,
                               SwCrsrMoveState* pCMS, const long nMax )
{
    CharCrsrToLine(nOfst);

    // Adjustierung ggf. nachholen
    GetAdjusted();

    const XubString &rText = GetInfo().GetTxt();
    SwTxtSizeInfo aInf( GetInfo(), rText, nStart );
    KSHORT nTmpAscent, nTmpHeight;  // Zeilenhoehe
    CalcAscentAndHeight( nTmpAscent, nTmpHeight );
    const Size  aCharSize( 1, nTmpHeight );
    const Point aCharPos( GetTopLeft() );
    pOrig->Pos( aCharPos );
    pOrig->SSize( aCharSize );
    sal_Bool bRet = sal_True;
    sal_Bool bWidth = sal_True;//pCMS && pCMS->bRealWidth;
    if( !pCurr->GetLen() && !pCurr->Width() )
    {
        if ( pCMS && pCMS->bRealHeight )
        {
            pCMS->aRealHeight.X() = 0;
            pCMS->aRealHeight.Y() = nTmpHeight;
        }
    }
    else
    {
        KSHORT nPorHeight = nTmpHeight;
        KSHORT nPorAscent = nTmpAscent;
        SwTwips nX = 0;
        SwTwips nFirst = 0;
        SwLinePortion *pPor = pCurr->GetFirstPortion();
        SvShorts *pSpaceAdd = pCurr->GetpSpaceAdd();
        MSHORT nSpaceIdx = 0;
        short nSpaceAdd = pSpaceAdd ? (*pSpaceAdd)[0] : 0;
        sal_Bool bNoTxt = sal_True;

        // Zuerst werden alle Portions ohne Len am Zeilenanfang uebersprungen.
        // Ausnahme bilden die fiesen Spezialportions aus WhichFirstPortion:
        // Num, ErgoSum, FtnNum, FeldReste
        // 8477: aber auch die einzige Textportion einer leeren Zeile mit
        // Right/Center-Adjustment! Also nicht nur pPor->GetExpandPortion() ...

        while( pPor && !pPor->GetLen() )
        {
            nX += pPor->Width();
            if ( pPor->InSpaceGrp() && nSpaceAdd )
                nX += pPor->CalcSpacing( nSpaceAdd, aInf );
            if( bNoTxt )
                nFirst = nX;
            // 8670: EndPortions zaehlen hier einmal als TxtPortions.
            if( pPor->InTxtGrp() || pPor->IsBreakPortion() )
            {
                bNoTxt = sal_False;
                nFirst = nX;
            }
            if( pPor->IsMultiPortion() && pSpaceAdd &&
                ((SwMultiPortion*)pPor)->HasTabulator() )
            {
                if ( ++nSpaceIdx < pSpaceAdd->Count() )
                    nSpaceAdd = (*pSpaceAdd)[nSpaceIdx];
                else
                    nSpaceAdd = 0;
            }
            if( pPor->InFixMargGrp() )
            {
                if( pPor->IsMarginPortion() )
                    bNoTxt = sal_False;
                if( pSpaceAdd )
                {
                    if( nSpaceAdd < 0 )
                        nX += nSpaceAdd;
                    if ( ++nSpaceIdx < pSpaceAdd->Count() )
                        nSpaceAdd = (*pSpaceAdd)[nSpaceIdx];
                    else
                        nSpaceAdd = 0;
                }
            }
            pPor = pPor->GetPortion();
        }

        if( !pPor )
        {
            // Es sind nur Spezialportions unterwegs.
            nX = nFirst;
        }
        else
        {
            if( !pPor->IsMarginPortion() && !pPor->IsPostItsPortion() &&
                (!pPor->InFldGrp() || pPor->GetAscent() ) )
            {
                nPorHeight = pPor->Height();
                nPorAscent = pPor->GetAscent();
            }
            while( pPor && aInf.GetIdx() < nOfst && !pPor->IsBreakPortion() )
            {
                if( !pPor->IsMarginPortion() && !pPor->IsPostItsPortion() &&
                    (!pPor->InFldGrp() || pPor->GetAscent() ) )
                {
                    nPorHeight = pPor->Height();
                    nPorAscent = pPor->GetAscent();
                }
                if ( aInf.GetIdx() + pPor->GetLen() <= nOfst )
                {
                    if ( pPor->InSpaceGrp() && nSpaceAdd )
                        nX += pPor->PrtWidth() +
                              pPor->CalcSpacing( nSpaceAdd, aInf );
                    else
                    {
                        if( pPor->InFixMargGrp() && pSpaceAdd )
                        {
                            if( nSpaceAdd < 0 )
                                nX += nSpaceAdd;
                            if ( ++nSpaceIdx < pSpaceAdd->Count() )
                                nSpaceAdd = (*pSpaceAdd)[nSpaceIdx];
                            else
                                nSpaceAdd = 0;
                        }
                        if ( !pPor->IsFlyPortion() || ( pPor->GetPortion() &&
                                !pPor->GetPortion()->IsMarginPortion() ) )
                            nX += pPor->PrtWidth();
                    }
                    if( pPor->IsMultiPortion() && pSpaceAdd &&
                        ((SwMultiPortion*)pPor)->HasTabulator() )
                    {
                        if ( ++nSpaceIdx < pSpaceAdd->Count() )
                            nSpaceAdd = (*pSpaceAdd)[nSpaceIdx];
                        else
                            nSpaceAdd = 0;
                    }

                    aInf.SetIdx( aInf.GetIdx() + pPor->GetLen() );
                    pPor = pPor->GetPortion();
                }
                else
                {
                    if( pPor->IsMultiPortion() )
                    {
                        // In a multi-portion we use GetCharRect()-function
                        // rekursively and must add the x-position
                        // of the multi-portion.
                        xub_StrLen nOldStart = nStart;
                        nStart = aInf.GetIdx();
                        SwLineLayout* pOldCurr = pCurr;
                        pCurr = &((SwMultiPortion*)pPor)->GetRoot();
                        if( nStart + pCurr->GetLen() <= nOfst )
                            Next();
                        sal_Bool bSpaceChg = ((SwMultiPortion*)pPor)->
                                                ChgSpaceAdd( pCurr, nSpaceAdd );
                        bRet = GetCharRect( pOrig, nOfst, pCMS, nMax );
                        pOrig->Pos().X() += nX;
                        if( ((SwMultiPortion*)pPor)->HasBrackets() )
                            pOrig->Pos().X() +=
                                ((SwDoubleLinePortion*)pPor)->PreWidth();
                        if( bSpaceChg )
                            SwDoubleLinePortion::ResetSpaceAdd( pCurr );
                        pCurr = pOldCurr;
                        nStart = nOldStart;
                        bPrev = sal_False;
                        return bRet;
                    }
                    if ( pPor->PrtWidth() )
                    {
                        xub_StrLen nOldLen = pPor->GetLen();
                        pPor->SetLen( nOfst - aInf.GetIdx() );
                        aInf.SetLen( pPor->GetLen() );
                        if( nX || !pPor->InNumberGrp() )
                        {
                            SeekAndChg( aInf );
                            const sal_Bool bOldOnWin = aInf.OnWin();
                            aInf.SetOnWin( sal_False ); // keine BULLETs!
                            SwTwips nTmp = nX;
                            nX += pPor->GetTxtSize( aInf ).Width();
                            aInf.SetOnWin( bOldOnWin );
                            if ( pPor->InSpaceGrp() && nSpaceAdd )
                                nX += pPor->CalcSpacing( nSpaceAdd, aInf );
                            if( bWidth )
                            {
                                pPor->SetLen( pPor->GetLen() + 1 );
                                aInf.SetLen( pPor->GetLen() );
                                aInf.SetOnWin( sal_False ); // keine BULLETs!
                                nTmp += pPor->GetTxtSize( aInf ).Width();
                                aInf.SetOnWin( bOldOnWin );
                                if ( pPor->InSpaceGrp() && nSpaceAdd )
                                    nTmp += pPor->CalcSpacing(nSpaceAdd, aInf);
                                pOrig->Width( nTmp - nX );
                            }
                        }
                        pPor->SetLen( nOldLen );
                    }
                    bWidth = sal_False;
                    break;
                }
            }
        }

        if( pPor )
        {
            // Bei manchen Portions gibts es kein links (Number, DropCap)
            if( pPor->InNumberGrp() )
            {
                nX += pPor->Width();
                if( pPor->GetPortion() && pPor->GetPortion()->IsMarginPortion() )
                {
                    nX += pPor->GetPortion()->Width();
                    nPorHeight = pPor->GetPortion()->Height();
                    nPorAscent = pPor->GetPortion()->GetAscent();
                }
                else
                    bRet = sal_False;
            }
            else
            {
                sal_Bool bEmptyFld = sal_False;
                if( pPor->InFldGrp() && pPor->GetLen() )
                {
                    SwFldPortion *pTmp = (SwFldPortion*)pPor;
                    while( pTmp->HasFollow() && !pTmp->GetExp().Len() )
                    {
                        KSHORT nAddX = pTmp->Width();
                        SwLinePortion *pNext = pTmp->GetPortion();
                        while( pNext && !pNext->InFldGrp() )
                        {
                            if( pNext->GetLen() )
                                pNext = NULL; // Gibt's das ueberhaupt?
                            else
                            {
                                nAddX += pNext->Width();
                                pNext = pNext->GetPortion();
                            }
                        }
                        if( !pNext )
                            break;
                        pTmp = (SwFldPortion*)pNext;
                        nPorHeight = pTmp->Height();
                        nPorAscent = pTmp->GetAscent();
                        nX += nAddX;
                        bEmptyFld = sal_True;
                    }
                }
                // 8513: Felder im Blocksatz, ueberspringen
                while( pPor && !pPor->GetLen() && ( pPor->IsFlyPortion() ||
                       pPor->IsKernPortion() ||
                       ( !bEmptyFld && pPor->InFldGrp() ) ) )
                {
                    if ( pPor->InSpaceGrp() && nSpaceAdd )
                        nX += pPor->PrtWidth() +
                              pPor->CalcSpacing( nSpaceAdd, aInf );
                    else
                    {
                        if( pPor->InFixMargGrp() && pSpaceAdd )
                        {
                            if( nSpaceAdd < 0 )
                                nX += nSpaceAdd;
                            if ( ++nSpaceIdx < pSpaceAdd->Count() )
                                nSpaceAdd = (*pSpaceAdd)[nSpaceIdx];
                            else
                                nSpaceAdd = 0;
                        }
                        if ( !pPor->IsFlyPortion() || ( pPor->GetPortion() &&
                                !pPor->GetPortion()->IsMarginPortion() ) )
                            nX += pPor->PrtWidth();
                    }
                    if( pPor->IsMultiPortion() && pSpaceAdd &&
                        ((SwMultiPortion*)pPor)->HasTabulator() )
                    {
                        if ( ++nSpaceIdx < pSpaceAdd->Count() )
                            nSpaceAdd = (*pSpaceAdd)[nSpaceIdx];
                        else
                            nSpaceAdd = 0;
                    }
                    if( !pPor->IsFlyPortion() )
                    {
                        nPorHeight = pPor->Height();
                        nPorAscent = pPor->GetAscent();
                    }
                    pPor = pPor->GetPortion();
                }
            }

            if( aInf.GetIdx() == nOfst && pPor && pPor->InHyphGrp() &&
                pPor->GetPortion() && pPor->GetPortion()->InFixGrp() )
            {
                // Alle Sonderportions muessen uebersprungen werden
                // Beispiel: zu-[FLY]sammen, 'u' == 19, 's' == 20; Right()
                // Ohne den Ausgleich landen wir vor '-' mit dem
                // Ausgleich vor 's'.
                while( pPor && !pPor->GetLen() )
                {
                    DBG_LOOP;
                    nX += pPor->Width();
                    if( !pPor->IsMarginPortion() )
                    {
                        nPorHeight = pPor->Height();
                        nPorAscent = pPor->GetAscent();
                    }
                    pPor = pPor->GetPortion();
                }
            }
            if( pPor && pCMS )
            {
                if( pCMS->bFieldInfo && pPor->InFldGrp() && pPor->Width() )
                    pOrig->Width( pPor->Width() );
                if( pPor->IsDropPortion() )
                {
                    nPorAscent = ((SwDropPortion*)pPor)->GetDropHeight();
                    nPorHeight = nPorAscent;
                    pOrig->Height( nPorHeight +
                        ((SwDropPortion*)pPor)->GetDropDescent() );
                    if( nTmpHeight < pOrig->Height() )
                    {
                        nTmpAscent = nPorAscent;
                        nTmpHeight = pOrig->Height();
                    }
                }
                if( bWidth && pPor->PrtWidth() && pPor->GetLen() &&
                    aInf.GetIdx() == nOfst )
                {
                    if( !pPor->IsFlyPortion() && pPor->Height() &&
                        pPor->GetAscent() )
                    {
                        nPorHeight = pPor->Height();
                        nPorAscent = pPor->GetAscent();
                    }
                    SwTwips nTmp;
                    if( 2 > pPor->GetLen() )
                    {
                        nTmp = pPor->Width();
                        if ( pPor->InSpaceGrp() && nSpaceAdd )
                            nTmp += pPor->CalcSpacing( nSpaceAdd, aInf );
                    }
                    else
                    {
                        const sal_Bool bOldOnWin = aInf.OnWin();
                        xub_StrLen nOldLen = pPor->GetLen();
                        pPor->SetLen( 1 );
                        aInf.SetLen( pPor->GetLen() );
                        SeekAndChg( aInf );
                        aInf.SetOnWin( sal_False ); // keine BULLETs!
                        nTmp = pPor->GetTxtSize( aInf ).Width();
                        aInf.SetOnWin( bOldOnWin );
                        if ( pPor->InSpaceGrp() && nSpaceAdd )
                            nTmp += pPor->CalcSpacing( nSpaceAdd, aInf );
                        pPor->SetLen( nOldLen );
                    }
                    pOrig->Width( nTmp );
                }
            }
        }
        // Es darf nicht vorzeitig returnt werden.
        // 6798: nicht auf pOrig->Right() rumhuehnern.
        pOrig->Pos().X() += nX;

        // Hier muesste eigentlich "-1 LogicToPixel" stehen, dies erscheint
        // mir aber zu teuer, deshalb ein Wert (-12), mit dem es sich hoffentlich
        // leben laesst?
        const SwTwips nRight = Right() - 12;

        if( pOrig->Left() > nRight )
            pOrig->Pos().X() = nRight;

        if ( pCMS && pCMS->bRealHeight )
        {
            if ( nTmpAscent > nPorAscent )
                pCMS->aRealHeight.X() = nTmpAscent - nPorAscent;
            else
                pCMS->aRealHeight.X() = 0;
            ASSERT( nPorHeight, "GetCharRect: Missing Portion-Height" );
            if ( nTmpHeight > nPorHeight )
                pCMS->aRealHeight.Y() = nPorHeight;
            else
                pCMS->aRealHeight.Y() = nTmpHeight;
        }
    }
    if( nMax )
    {
        if( pOrig->Bottom() > nMax )
        {
            if( pOrig->Top() > nMax )
                pOrig->Top( nMax );
            pOrig->Bottom( nMax );
        }
        if ( pCMS && pCMS->bRealHeight )
        {
            long nTmp = pCMS->aRealHeight.X() + pOrig->Top();
            if( nTmp >= nMax )
            {
                pCMS->aRealHeight.X() = nMax - pOrig->Top();
                pCMS->aRealHeight.Y() = 0;
            }
            else if( nTmp + pCMS->aRealHeight.Y() > nMax )
                pCMS->aRealHeight.Y() = nMax - nTmp;
        }
    }
    long nOut = pOrig->Right() - GetTxtFrm()->Frm().Right();
    if( nOut > 0 )
    {
        if( GetTxtFrm()->Frm().Width() < GetTxtFrm()->Prt().Left()
                                   + GetTxtFrm()->Prt().Width() )
            nOut += GetTxtFrm()->Frm().Width() - GetTxtFrm()->Prt().Left()
                    - GetTxtFrm()->Prt().Width();
        if( nOut > 0 )
            pOrig->Pos().X() -= nOut + 10;
    }
    return bRet;
}

/*************************************************************************
 *                      SwTxtCursor::GetCrsrOfst()
 *
 * Return: Offset im String
 *************************************************************************/
xub_StrLen SwTxtCursor::GetCrsrOfst( SwPosition *pPos, const Point &rPoint,
                     const MSHORT nChgNode, const SwCrsrMoveState* pCMS ) const
{
    // Adjustierung ggf. nachholen
    GetAdjusted();

    const XubString &rText = GetInfo().GetTxt();
    xub_StrLen nOffset = 0;

    // x ist der horizontale Offset innerhalb der Zeile.
    SwTwips x = rPoint.X();
    CONST SwTwips nLeftMargin  = GetLineStart();
    SwTwips nRightMargin = GetLineEnd();
    if( nRightMargin == nLeftMargin )
        nRightMargin += 30;

    if(x < nLeftMargin)
        x = nLeftMargin;
    sal_Bool bRightOver = x > nRightMargin;
    if( bRightOver )
        x = nRightMargin;

    sal_Bool bRightAllowed = pCMS && ( pCMS->eState == MV_NONE );

    // Bis hierher in Dokumentkoordinaten.
    x -= nLeftMargin;

    KSHORT nX = KSHORT( x );

    // Wenn es in der Zeile Attributwechsel gibt, den Abschnitt
    // suchen, in dem nX liegt.
    SwLinePortion *pPor = pCurr->GetFirstPortion();
    xub_StrLen nCurrStart  = nStart;
    sal_Bool bLastPortion;
    sal_Bool bHolePortion = sal_False;
    sal_Bool bLastHyph = sal_False;

    SvShorts *pSpaceAdd = pCurr->GetpSpaceAdd();
    xub_StrLen nOldIdx = GetInfo().GetIdx();
    MSHORT nSpaceIdx = 0;
    short nSpaceAdd = pSpaceAdd ? (*pSpaceAdd)[0] : 0;

    // nWidth ist die Breite der Zeile, oder die Breite des
    // Abschnitts mit dem Fontwechsel, in dem nX liegt.

    KSHORT nWidth = pPor->Width();
    if ( pSpaceAdd )
    {
        if ( pPor->InSpaceGrp() && nSpaceAdd )
        {
            ((SwTxtSizeInfo&)GetInfo()).SetIdx( nCurrStart );
            nWidth += pPor->CalcSpacing( nSpaceAdd, GetInfo() );
        }
        if( pPor->InFixMargGrp() ||
            (pPor->IsMultiPortion() && ((SwMultiPortion*)pPor)->HasTabulator()))
        {
            if ( ++nSpaceIdx < pSpaceAdd->Count() )
                nSpaceAdd = (*pSpaceAdd)[nSpaceIdx];
            else
                nSpaceAdd = 0;
        }
    }

    KSHORT nWidth30 = !nWidth && pPor->GetLen() && pPor->InToxRefOrFldGrp()
        ? 30 : nWidth;

    while(!(bLastPortion = (0 == pPor->GetPortion())) && nWidth30 < nX &&
        !pPor->IsBreakPortion() )
    {
        if ( pPor->IsPostItsPortion() && pPor->GetViewWidth( GetInfo() )/2 > nX )
            break;

        nX -= nWidth;
        nCurrStart += pPor->GetLen();
        bHolePortion = pPor->IsHolePortion();
        pPor = pPor->GetPortion();
        nWidth = pPor->Width();
        if ( pSpaceAdd )
        {
            if ( pPor->InSpaceGrp() && nSpaceAdd )
            {
                ((SwTxtSizeInfo&)GetInfo()).SetIdx( nCurrStart );
                nWidth += pPor->CalcSpacing( nSpaceAdd, GetInfo() );
            }
            if( pPor->InFixMargGrp() || ( pPor->IsMultiPortion() &&
                ((SwMultiPortion*)pPor)->HasTabulator() ) )
            {
                if ( ++nSpaceIdx < pSpaceAdd->Count() )
                    nSpaceAdd = (*pSpaceAdd)[nSpaceIdx];
                else
                    nSpaceAdd = 0;
            }
        }
        nWidth30 = !nWidth && pPor->GetLen() && ( pPor->InToxRefOrFldGrp() ||
                    pPor->IsPostItsPortion() ) ? 30 : nWidth;
        if( !pPor->IsFlyPortion() && !pPor->IsMarginPortion() )
            bLastHyph = pPor->InHyphGrp();
    }

    if( nX==nWidth )
    {
        SwLinePortion *pNextPor = pPor->GetPortion();
        while( pNextPor && pNextPor->InFldGrp() && !pNextPor->Width() )
        {
            nCurrStart += pPor->GetLen();
            pPor = pNextPor;
            if( !pPor->IsFlyPortion() && !pPor->IsMarginPortion() )
                bLastHyph = pPor->InHyphGrp();
            pNextPor = pPor->GetPortion();
        }
    }

    ((SwTxtSizeInfo&)GetInfo()).SetIdx( nOldIdx );

    xub_StrLen nLength = pPor->GetLen();

    sal_Bool bFieldInfo = pCMS && pCMS->bFieldInfo;

    if( bFieldInfo && ( nWidth30 < nX || bRightOver ||
        ( pPor->InNumberGrp() && !pPor->IsFtnNumPortion() ) ||
        ( pPor->IsMarginPortion() && nWidth > nX + 30 ) ) )
        ((SwCrsrMoveState*)pCMS)->bPosCorr = sal_True;

    // 7684: Wir sind genau auf der HyphPortion angelangt und muessen dafuer
    // sorgen, dass wir in dem String landen.
    // 7993: Wenn die Laenge 0 ist muessen wir raus...
    if( !nLength )
    {
        if( pCMS )
        {
            if( pPor->IsFlyPortion() && bFieldInfo )
                ((SwCrsrMoveState*)pCMS)->bPosCorr = sal_True;

            if( pPor->IsFtnNumPortion() && !bRightOver && nX )
                ((SwCrsrMoveState*)pCMS)->bFtnNoInfo = sal_True;
        }
        if( !nCurrStart )
            return 0;

         // 7849, 7816: auf pPor->GetHyphPortion kann nicht verzichtet werden!
        if( bHolePortion || ( !bRightAllowed && bLastHyph ) ||
            ( pPor->IsMarginPortion() && !pPor->GetPortion() &&
            // 46598: In der letzten Zeile eines zentrierten Absatzes wollen
            // wir auch mal hinter dem letzten Zeichen landen.
              nCurrStart < rText.Len() ) )
            --nCurrStart;
        else if( pPor->InFldGrp() && ((SwFldPortion*)pPor)->IsFollow()
                 && nWidth > nX )
        {
            if( bFieldInfo )
                --nCurrStart;
            else
            {
                KSHORT nHeight = pPor->Height();
                if ( !nHeight || nHeight > nWidth )
                    nHeight = nWidth;
                if( nChgNode && nWidth - nHeight/2 > nX )
                    --nCurrStart;
            }
        }
        return nCurrStart;
    }
    if ( 1 == nLength )
    {
        if ( nWidth )
        {
            // Sonst kommen wir nicht mehr in zeichengeb. Rahmen hinein...
            if( !( nChgNode && pPos && pPor->IsFlyCntPortion() ) )
            {
                if ( pPor->InFldGrp() )
                {
                    KSHORT nHeight = 0;
                    if( !bFieldInfo )
                    {
                        nHeight = pPor->Height();
                        if ( !nHeight || nHeight > nWidth )
                            nHeight = nWidth;
                    }
                    if( !((SwFldPortion*)pPor)->HasFollow() &&
                        nWidth - nHeight/2 <= nX )
                        ++nCurrStart;
                }
                else if( ( !pPor->IsFlyPortion() || ( pPor->GetPortion() &&
                    !pPor->GetPortion()->IsMarginPortion() &&
                    !pPor->GetPortion()->IsHolePortion() ) )
                         && ( nWidth/2 < nX ) && !bFieldInfo
                         && ( bRightAllowed || !bLastHyph ))
                    ++nCurrStart;
                return nCurrStart;
            }
        }
        else
        {
            if ( pPor->IsPostItsPortion() || pPor->IsBreakPortion() ||
                 pPor->InToxRefGrp() )
                return nCurrStart;
            if ( pPor->InFldGrp() )
            {
                if( bRightOver && !((SwFldPortion*)pPor)->HasFollow() )
                    ++nCurrStart;
                return nCurrStart;
            }
        }
    }

    if( bLastPortion && (pCurr->GetNext() || pFrm->GetFollow() ) )
        --nLength;

    if( nWidth > nX )
    {
        if( pPor->IsMultiPortion() )
        {
            // In a multi-portion we use GetCrsrOfst()-funtion rekursively
            SwTxtCursorSave aSave( (SwTxtCursor*)this, (SwMultiPortion*)pPor,
                rPoint.Y(), nCurrStart, nSpaceAdd );
            if( ((SwMultiPortion*)pPor)->HasBrackets() )
                nX -= ((SwDoubleLinePortion*)pPor)->PreWidth();
            return GetCrsrOfst( pPos, Point( nLeftMargin + nX, rPoint.Y() ),
                                nChgNode, pCMS );
        }
        if( pPor->InTxtGrp() )
        {
            SwTxtSizeInfo aSizeInf( GetInfo(), rText, nCurrStart );
            ((SwTxtCursor*)this)->SeekAndChg( aSizeInf );
            SwTxtSlot aDiffTxt( &aSizeInf, ((SwTxtPortion*)pPor) );
            SwFontSave aSave( aSizeInf, pPor->IsDropPortion() ?
                    ((SwDropPortion*)pPor)->GetFnt() : NULL );
            nLength = aSizeInf.GetFont()->_GetCrsrOfst( aSizeInf.GetVsh(),
                                    aSizeInf.GetOut(),
                                    aSizeInf.GetTxt(), nX, aSizeInf.GetIdx(),
                                    pPor->GetLen(), nSpaceAdd );
            if( bFieldInfo && nLength == pPor->GetLen() )
                --nLength;
        }
        else
        {
            if( nChgNode && pPos && pPor->IsFlyCntPortion()
                && !( (SwFlyCntPortion*)pPor )->IsDraw() )
            {
                // JP 24.11.94: liegt die Pos nicht im Fly, dann
                //              darf nicht mit STRING_LEN returnt werden!
                //              (BugId: 9692 + Aenderung in feshview)
                SwFlyInCntFrm *pTmp = ( (SwFlyCntPortion*)pPor )->GetFlyFrm();
                sal_Bool bChgNode = 1 < nChgNode;
                if( !bChgNode )
                {
                    SwFrm* pLower = pTmp->GetLower();
                    if( pLower && (pLower->IsTxtFrm() || pLower->IsLayoutFrm()) )
                        bChgNode = sal_True;
                }
                if( bChgNode && pTmp->Frm().IsInside( rPoint ) &&
                    !( pTmp->IsProtected() ) )
                {
                    nLength = ((SwFlyCntPortion*)pPor)->
                              GetFlyCrsrOfst( nX, rPoint, pPos, pCMS );
                    // Sobald der Frame gewechselt wird, muessen wir aufpassen, dass
                    // unser Font wieder im OutputDevice steht.
                    // vgl. Paint und new SwFlyCntPortion !
                    ((SwTxtSizeInfo*)pInf)->SelectFont();

                    // 6776: Das pIter->GetCrsrOfst returnt
                    // aus einer Verschachtelung mit STRING_LEN.
                    return STRING_LEN;
                }
            }
            else
                nLength = pPor->GetCrsrOfst( nX );
        }
    }
    nOffset = nCurrStart + nLength;

    // 7684: Wir sind vor der HyphPortion angelangt und muessen dafuer
    // sorgen, dass wir in dem String landen.
    // Bei Zeilenenden vor FlyFrms muessen ebenso behandelt werden.

    if( nOffset && pPor->GetLen() == nLength && pPor->GetPortion() &&
        !pPor->GetPortion()->GetLen() && pPor->GetPortion()->InHyphGrp() )
        --nOffset;

    return nOffset;
}

