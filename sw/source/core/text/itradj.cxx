/*************************************************************************
 *
 *  $RCSfile: itradj.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ama $ $Date: 2000-11-06 09:13:59 $
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

#include "frame.hxx"       // CalcFlyAdjust()
#include "paratr.hxx"
#ifdef DEBUG
# include "ndtxt.hxx"        // pSwpHints, Ausgabeoperator
#endif
#include "segmentc.hxx"

#include "txtcfg.hxx"
#include "itrtxt.hxx"
#include "porglue.hxx"
#include "porlay.hxx"
#include "porfly.hxx"       // CalcFlyAdjust()
#include "pordrop.hxx"       // CalcFlyAdjust()
#include "pormulti.hxx"


/*************************************************************************
 *                    SwTxtAdjuster::FormatBlock()
 *************************************************************************/

void SwTxtAdjuster::FormatBlock( )
{
    // In der letzten Zeile gibt's keinen Blocksatz.
    // Und bei Tabulatoren aus Tradition auch nicht.
    // 7701: wenn Flys im Spiel sind, geht's weiter

    const SwLinePortion *pFly = 0;

    sal_Bool bSkip = !IsLastBlock() &&
        nStart + pCurr->GetLen() >= GetInfo().GetTxt().Len();

    // ????: mehrzeilige Felder sind fies: wir muessen kontrollieren,
    // ob es noch andere Textportions im Absatz gibt.
    if( bSkip )
    {
        const SwLineLayout *pLay = pCurr->GetNext();
        while( pLay && !pLay->GetLen() )
        {
            const SwLinePortion *pPor = pCurr->GetFirstPortion();
            while( pPor && bSkip )
            {
                if( pPor->InTxtGrp() )
                    bSkip = sal_False;
                pPor = pPor->GetPortion();
            }
            pLay = bSkip ? pLay->GetNext() : 0;
        }
    }

    if( bSkip )
    {
        if( !GetInfo().GetParaPortion()->HasFly() )
        {
            if( IsLastCenter() )
                CalcFlyAdjust( pCurr );
            pCurr->FinishSpaceAdd();
            return;
        }
        else
        {
            const SwLinePortion *pTmpFly = NULL;

            // 7701: beim letzten Fly soll Schluss sein
            const SwLinePortion *pPos = pCurr->GetFirstPortion();
            while( pPos )
            {
                // Ich suche jetzt den letzten Fly, hinter dem noch Text ist:
                if( pPos->IsFlyPortion() )
                    pTmpFly = pPos; // Ein Fly wurde gefunden
                else if ( pTmpFly && pPos->InTxtGrp() )
                {
                    pFly = pTmpFly; // Ein Fly mit nachfolgendem Text!
                    pTmpFly = NULL;
                }
                pPos = pPos->GetPortion();
            }
            // 8494: Wenn keiner gefunden wurde, ist sofort Schluss!
            if( !pFly )
            {
                if( IsLastCenter() )
                    CalcFlyAdjust( pCurr );
                pCurr->FinishSpaceAdd();
                return;
            }
        }
    }

    const nOldIdx = GetInfo().GetIdx();
    GetInfo().SetIdx( nStart );
    CalcNewBlock( pCurr, pFly );
    GetInfo().SetIdx( nOldIdx );
    GetInfo().GetParaPortion()->GetRepaint()->SetOfst(0);
}

/*************************************************************************
 *                    SwTxtAdjuster::CalcBlockAdjust()
 *
 * CalcBlockAdjust() darf erst nach CalcLine() gerufen werden !
 * Aufgespannt wird immer zwischen zwei RandPortions oder FixPortions
 * (Tabs und Flys). Dabei werden die Glues gezaehlt und ExpandBlock gerufen.
 *************************************************************************/

void SwTxtAdjuster::CalcNewBlock( SwLineLayout *pCurr,
                                  const SwLinePortion *pStopAt, SwTwips nReal )
{
    ASSERT( GetInfo().IsMulti() || SVX_ADJUST_BLOCK == GetAdjust(),
            "CalcNewBlock: Why?" );
    ASSERT( pCurr->Height(), "SwTxtAdjuster::CalcBlockAdjust: missing CalcLine()" );

    pCurr->InitSpaceAdd();
    MSHORT nNull = 0;
    xub_StrLen nGluePortion = 0;
    xub_StrLen nCharCnt = 0;
    MSHORT nSpaceIdx = 0;

    // Nicht vergessen:
    // CalcRightMargin() setzt pCurr->Width() auf die Zeilenbreite !
    CalcRightMargin( pCurr, nReal );

    SwLinePortion *pPos = pCurr->GetPortion();

    while( pPos )
    {
        if ( pPos->IsBreakPortion() && !IsLastBlock() )
        {
            pCurr->FinishSpaceAdd();
            break;
        }
        if ( pPos->InTxtGrp() )
            nGluePortion += ((SwTxtPortion*)pPos)->GetSpaceCnt( GetInfo(), nCharCnt );
        else if( pPos->IsMultiPortion() )
        {
            SwMultiPortion* pMulti = (SwMultiPortion*)pPos;
            // a multiportion with a tabulator inside breaks the text adjustment
            // a ruby portion will not be stretched by text adjustment
            // a double line portion takes additional space for each blank
            // in the wider line
            if( pMulti->HasTabulator() )
            {
                if ( nSpaceIdx == pCurr->GetSpaceAdd().Count() )
                    pCurr->GetSpaceAdd().Insert( nNull, nSpaceIdx );
                nSpaceIdx++;
                nGluePortion = 0;
                nCharCnt = 0;
            }
            else if( pMulti->IsDouble() )
                nGluePortion += ((SwDoubleLinePortion*)pMulti)->GetSpaceCnt();
        }

        if( pPos->InGlueGrp() )
        {
            if( pPos->InFixMargGrp() )
            {
                if ( nSpaceIdx == pCurr->GetSpaceAdd().Count() )
                    pCurr->GetSpaceAdd().Insert( nNull, nSpaceIdx );
                if( nGluePortion )
                {
                    ( pCurr->GetSpaceAdd() )[nSpaceIdx] =
                         ( (SwGluePortion*)pPos )->GetPrtGlue() / nGluePortion;
                    pPos->Width( ( (SwGluePortion*)pPos )->GetFixWidth() );
                }
                else if ( IsOneBlock() && nCharCnt > 1 )
                {
                    ( pCurr->GetSpaceAdd() )[nSpaceIdx] =
                        - ( (SwGluePortion*)pPos )->GetPrtGlue() / (nCharCnt-1);
                    pPos->Width( ( (SwGluePortion*)pPos )->GetFixWidth() );
                }
                nSpaceIdx++;
                nGluePortion = 0;
                nCharCnt = 0;
            }
            else
                ++nGluePortion;
        }
        GetInfo().SetIdx( GetInfo().GetIdx() + pPos->GetLen() );
        if ( pPos == pStopAt )
        {
            if ( nSpaceIdx == pCurr->GetSpaceAdd().Count() )
                pCurr->GetSpaceAdd().Insert( nNull, nSpaceIdx );
            else
                pCurr->GetSpaceAdd()[nSpaceIdx] = 0;
            break;
        }
        pPos = pPos->GetPortion();
    }
}

/*************************************************************************
 *                    SwTxtAdjuster::CalcRightMargin()
 *************************************************************************/

SwMarginPortion *SwTxtAdjuster::CalcRightMargin( SwLineLayout *pCurr,
    SwTwips nReal )
{
    long nRealWidth;
    const long nHeight = GetLineHeight();
    KSHORT nPrtWidth = pCurr->PrtWidth();
    SwLinePortion *pLast = pCurr->FindLastPortion();

    if( GetInfo().IsMulti() )
        nRealWidth = nReal;
    else
    {
        nRealWidth = GetLineWidth();
        // Fuer jeden FlyFrm, der in den rechten Rand hineinragt,
        // wird eine FlyPortion angelegt.
        const long nLeftMar = GetLeftMargin();
        SwRect aCurrRect( nLeftMar + nPrtWidth, Y(),
                          nRealWidth - nPrtWidth, nHeight );

        SwFlyPortion *pFly = CalcFlyPortion( nRealWidth, aCurrRect );
        while( pFly && long( nPrtWidth )< nRealWidth )
        {
            pLast->Append( pFly );
            pLast = pFly;
            if( pFly->Fix() > nPrtWidth )
                pFly->Width( ( pFly->Fix() - nPrtWidth) + pFly->Width() + 1);
            nPrtWidth += pFly->Width() + 1;
            aCurrRect.Left( nLeftMar + nPrtWidth );
            pFly = CalcFlyPortion( nRealWidth, aCurrRect );
        }
        if( pFly )
            delete pFly;
    }

    SwMarginPortion *pRight = new SwMarginPortion( 0 );
    pLast->Append( pRight );

    if( long( nPrtWidth )< nRealWidth )
        pRight->PrtWidth( KSHORT( nRealWidth - nPrtWidth ) );

    // pCurr->Width() wird auf die reale Groesse gesetzt,
    // da jetzt die MarginPortions eingehaengt sind.
    // Dieser Trick hat wundersame Auswirkungen.
    // Wenn pCurr->Width() == nRealWidth ist, dann wird das gesamte
    // Adjustment implizit ausgecontert. GetLeftMarginAdjust() und
    // IsBlocksatz() sind der Meinung, sie haetten eine mit Zeichen
    // gefuellte Zeile.

    pCurr->PrtWidth( KSHORT( nRealWidth ) );
    return pRight;
}

/*************************************************************************
 *                    SwTxtAdjuster::CalcFlyAdjust()
 *************************************************************************/

void SwTxtAdjuster::CalcFlyAdjust( SwLineLayout *pCurr )
{
    // 1) Es wird ein linker Rand eingefuegt:
    SwMarginPortion *pLeft = pCurr->CalcLeftMargin();
    SwGluePortion *pGlue = pLeft;       // die letzte GluePortion


    // 2) Es wird ein rechter Rand angehaengt:
    // CalcRightMargin berechnet auch eventuelle Ueberlappungen mit
    // FlyFrms.
    CalcRightMargin( pCurr );

    SwLinePortion *pPos = pLeft->GetPortion();
    xub_StrLen nLen = 0;

    // Wenn wir nur eine Zeile vorliegen haben und die Textportion zusammen
    // haengend ist und wenn zentriert wird, dann ...

    sal_Bool bComplete = 0 == nStart;

    while( pPos )
    {
        if( pPos->InFixMargGrp() )
        {
            if( SVX_ADJUST_RIGHT == GetAdjust() )
            {
                ((SwGluePortion*)pPos)->MoveAllGlue( pGlue );
            }
            else
            {
                // Eine schlaue Idee von MA:
                // Fuer die erste Textportion wird rechtsbuendig eingestellt,
                // fuer die letzte linksbuendig.

                // Die erste Textportion kriegt den ganzen Glue
                // Aber nur, wenn wir mehr als eine Zeile besitzen.
                if( bComplete && GetInfo().GetTxt().Len() == nLen )
                {
                    ((SwGluePortion*)pPos)->MoveHalfGlue( pGlue );
                }
                else
                {
                    if( pLeft == pGlue )
                    {
                        // Wenn es nur einen linken und rechten Rand gibt,
                        // dann teilen sich die Raender den Glue.
                        if( nLen + pPos->GetLen() >= pCurr->GetLen() )
                            ((SwGluePortion*)pPos)->MoveHalfGlue( pGlue );
                        else
                            ((SwGluePortion*)pPos)->MoveAllGlue( pGlue );
                    }
                    else
                    {
                        // Die letzte Textportion behaelt sein Glue
                        if( !pPos->IsMarginPortion() )
                            ((SwGluePortion*)pPos)->MoveHalfGlue( pGlue );
                    }
                }
            }
            pGlue = (SwFlyPortion*)pPos;
            bComplete = sal_False;
        }
        nLen += pPos->GetLen();
        pPos = pPos->GetPortion();
    }

    if( SVX_ADJUST_RIGHT == GetAdjust() )
        pLeft->AdjustRight();
}

/*************************************************************************
 *                  SwTxtAdjuster::CalcAdjLine()
 *************************************************************************/

void SwTxtAdjuster::CalcAdjLine( SwLineLayout *pCurr )
{
    ASSERT( pCurr->IsFormatAdj(), "CalcAdjLine: Why?" );

    pCurr->SetFormatAdj(sal_False);
    switch( GetAdjust() )
    {
        case SVX_ADJUST_RIGHT:
        case SVX_ADJUST_CENTER:
        {
            CalcFlyAdjust( pCurr );
            GetInfo().GetParaPortion()->GetRepaint()->SetOfst( 0 );
            break;
        }
        case SVX_ADJUST_BLOCK:
        {
            // 8311: In Zeilen mit LineBreaks gibt es keinen Blocksatz!
            if( pCurr->GetLen() &&
                CH_BREAK == GetInfo().GetChar( nStart + pCurr->GetLen() - 1 ) &&
                !IsLastBlock() )
            {
                if( IsLastCenter() )
                {
                    CalcFlyAdjust( pCurr );
                    GetInfo().GetParaPortion()->GetRepaint()->SetOfst( 0 );
                    break;
                }
                return;
            }
            FormatBlock();
            break;
        }
        default : return;
    }

#ifdef DEBUG
/*
    if( OPTDBG( *pInf ) )
    {
        pCurr->DebugPortions( aDbstream, pInf->GetTxt(), nStart );
        if( GetHints() )
        {
            const SwpHints &rHt = *GetHints();
            aDbstream << rHt;
            SwAttrIter::Dump( aDbstream );
        }
    }
 */
#endif
}

/*************************************************************************
 *                    SwTxtAdjuster::CalcFlyPortion()
 *
 * Die Berechnung hat es in sich: nCurrWidth geibt die Breite _vor_ dem
 * aufaddieren des Wortes das noch auf die Zeile passt! Aus diesem Grund
 * stimmt die Breite der FlyPortion auch, wenn die Blockierungssituation
 * bFirstWord && !WORDFITS eintritt.
 *************************************************************************/

SwFlyPortion *SwTxtAdjuster::CalcFlyPortion( const long nRealWidth,
                                             const SwRect &rCurrRect )
{
    SwTxtFly aTxtFly( (SwCntntFrm*)GetTxtFrm() );

    const KSHORT nCurrWidth = pCurr->PrtWidth();
    SwFlyPortion *pFlyPortion = 0;
    // aFlyRect ist dokumentglobal !

    SwRect aFlyRect( aTxtFly.GetFrm( rCurrRect ) );

    // Wenn ein Frame ueberlappt, wird eine Portion eroeffnet.
    if( aFlyRect.HasArea() )
    {
        // aLocal ist framelokal
        SwRect aLocal( aFlyRect );
        aLocal.Pos( aLocal.Left() - GetLeftMargin(), aLocal.Top() );
        if( nCurrWidth > KSHORT( aLocal.Left() ) )
            aLocal.Left( nCurrWidth );

        // Wenn das Rechteck breiter als die Zeile ist, stutzen
        // wir es ebenfalls zurecht.
        KSHORT nLocalWidth = KSHORT( aLocal.Left() + aLocal.Width() );
        if( nRealWidth < long( nLocalWidth ) )
            aLocal.Width( nRealWidth - aLocal.Left() );
        GetInfo().GetParaPortion()->SetFly( sal_True );
        pFlyPortion = new SwFlyPortion( aLocal );
        pFlyPortion->Height( KSHORT( rCurrRect.Height() ) );
        // Die Width koennte kleiner sein als die FixWidth, daher:
        pFlyPortion->AdjFixWidth();
    }
    return pFlyPortion;
}

/*************************************************************************
 *                SwTxtPainter::_CalcDropAdjust()
 *************************************************************************/

// 6721: Drops und Adjustment
// CalcDropAdjust wird ggf. am Ende von Format() gerufen.

void SwTxtAdjuster::CalcDropAdjust()
{
    ASSERT( 1<GetDropLines() && SVX_ADJUST_LEFT!=GetAdjust() && SVX_ADJUST_BLOCK!=GetAdjust(),
            "CalcDropAdjust: No reason for DropAdjustment." )

    const MSHORT nLineNr = GetLineNr();

    // 1) Dummies ueberspringen
    Top();

    if( !pCurr->IsDummy() || NextLine() )
    {
        // Erst adjustieren.
        GetAdjusted();

        SwLinePortion *pPor = pCurr->GetFirstPortion();

        // 2) Sicherstellen, dass die DropPortion dabei ist.
        // 3) pLeft: Die GluePor vor der DropPor
        if( pPor->InGlueGrp() && pPor->GetPortion()
              && pPor->GetPortion()->IsDropPortion() )
        {
            const SwLinePortion *pDropPor = (SwDropPortion*) pPor->GetPortion();
            SwGluePortion *pLeft = (SwGluePortion*) pPor;

            // 4) pRight: Die GluePor hinter der DropPor suchen
            pPor = pPor->GetPortion();
            while( pPor && !pPor->InFixMargGrp() )
                pPor = pPor->GetPortion();

            SwGluePortion *pRight = ( pPor && pPor->InGlueGrp() ) ?
                                    (SwGluePortion*) pPor : 0;
            if( pRight && pRight != pLeft )
            {
                // 5) nMinLeft berechnen. Wer steht am weitesten links?
                const KSHORT nDropLineStart =
                    KSHORT(GetLineStart()) + pLeft->Width() + pDropPor->Width();
                KSHORT nMinLeft = nDropLineStart;
                for( MSHORT i = 1; i < GetDropLines(); ++i )
                {
                    if( NextLine() )
                    {
                        // Erst adjustieren.
                        GetAdjusted();

                        pPor = pCurr->GetFirstPortion();
                        const SwMarginPortion *pMar = pPor->IsMarginPortion() ?
                                                      (SwMarginPortion*)pPor : 0;
                        if( !pMar )
                            nMinLeft = 0;
                        else
                        {
                            const KSHORT nLineStart =
                                KSHORT(GetLineStart()) + pMar->Width();
                            if( nMinLeft > nLineStart )
                                nMinLeft = nLineStart;
                        }
                    }
                }

                // 6) Den Glue zwischen pLeft und pRight neu verteilen.
                if( nMinLeft < nDropLineStart )
                {
                    // Glue wird immer von pLeft nach pRight abgegeben,
                    // damit der Text nach links wandert.
                    const short nGlue = nDropLineStart - nMinLeft;
                    if( !nMinLeft )
                        pLeft->MoveAllGlue( pRight );
                    else
                        pLeft->MoveGlue( pRight, nGlue );
#ifdef DBGTXT
                    aDbstream << "Drop adjusted: " << nGlue << endl;
#endif
                }
            }
        }
    }

    if( nLineNr != GetLineNr() )
    {
        Top();
        while( nLineNr != GetLineNr() && Next() )
            ;
    }
}

/*************************************************************************
 *                SwTxtAdjuster::CalcDropRepaint()
 *************************************************************************/

void SwTxtAdjuster::CalcDropRepaint()
{
    Top();
    SwRepaint &rRepaint = *GetInfo().GetParaPortion()->GetRepaint();
    if( rRepaint.Top() > Y() )
        rRepaint.Top( Y() );
    for( MSHORT i = 1; i < GetDropLines(); ++i )
        NextLine();
    const SwTwips nBottom = Y() + GetLineHeight() - 1;
    if( rRepaint.Bottom() < nBottom )
        rRepaint.Bottom( nBottom );
}


