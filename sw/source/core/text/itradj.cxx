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

#include <com/sun/star/i18n/ScriptType.hpp>
#include <vcl/outdev.hxx>
#include <IDocumentSettingAccess.hxx>

#include "frame.hxx"
#include "paratr.hxx"
#include "itrtxt.hxx"
#include "porglue.hxx"
#include "porlay.hxx"
#include "porfly.hxx"
#include "pordrop.hxx"
#include "pormulti.hxx"
#include <portab.hxx>

#define MIN_TAB_WIDTH 60

using namespace ::com::sun::star;

/*************************************************************************
 * SwTxtAdjuster::FormatBlock()
 *************************************************************************/

void SwTxtAdjuster::FormatBlock( )
{
    
    
    

    const SwLinePortion *pFly = 0;

    bool bSkip = !IsLastBlock() &&
        nStart + pCurr->GetLen() >= GetInfo().GetTxt().getLength();

    
    
    if( bSkip )
    {
        const SwLineLayout *pLay = pCurr->GetNext();
        while( pLay && !pLay->GetLen() )
        {
            const SwLinePortion *pPor = pCurr->GetFirstPortion();
            while( pPor && bSkip )
            {
                if( pPor->InTxtGrp() )
                    bSkip = false;
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

            
            const SwLinePortion *pPos = pCurr->GetFirstPortion();
            while( pPos )
            {
                
                if( pPos->IsFlyPortion() )
                    pTmpFly = pPos; 
                else if ( pTmpFly && pPos->InTxtGrp() )
                {
                    pFly = pTmpFly; 
                    pTmpFly = NULL;
                }
                pPos = pPos->GetPortion();
            }
            
            if( !pFly )
            {
                if( IsLastCenter() )
                    CalcFlyAdjust( pCurr );
                pCurr->FinishSpaceAdd();
                return;
            }
        }
    }

    const sal_Int32 nOldIdx = GetInfo().GetIdx();
    GetInfo().SetIdx( nStart );
    CalcNewBlock( pCurr, pFly );
    GetInfo().SetIdx( nOldIdx );
    GetInfo().GetParaPortion()->GetRepaint()->SetOfst(0);
}

/*************************************************************************
 * lcl_CheckKashidaPositions()
 *************************************************************************/
static bool lcl_CheckKashidaPositions( SwScriptInfo& rSI, SwTxtSizeInfo& rInf, SwTxtIter& rItr,
                                sal_Int32& rKashidas, sal_Int32& nGluePortion )
{
    
    sal_Int32 nIdx = rItr.GetStart();
    sal_Int32 nEnd = rItr.GetEnd();

    
    
    
    
    
    rKashidas = rSI.KashidaJustify ( 0, 0, rItr.GetStart(), rItr.GetLength(), 0 );

    if (rKashidas <= 0) 
        return true;

    
    
    sal_Int32* pKashidaPos = new sal_Int32[ rKashidas ];
    sal_Int32* pKashidaPosDropped = new sal_Int32[ rKashidas ];
    rSI.GetKashidaPositions ( nIdx, rItr.GetLength(), pKashidaPos );
    sal_Int32 nKashidaIdx = 0;
    while ( rKashidas && nIdx < nEnd )
    {
        rItr.SeekAndChgAttrIter( nIdx, rInf.GetOut() );
        sal_Int32 nNext = rItr.GetNextAttr();

        
        
        sal_Int32 nNextScript = rSI.NextScriptChg( nIdx );
        if( nNextScript < nNext )
            nNext = nNextScript;

        if ( nNext == COMPLETE_STRING || nNext > nEnd )
            nNext = nEnd;
        sal_Int32 nKashidasInAttr = rSI.KashidaJustify ( 0, 0, nIdx, nNext - nIdx );
        if (nKashidasInAttr > 0)
        {
            
            if ( rInf.GetOut()->GetMinKashida() <= 0 )
            {
                delete[] pKashidaPos;
                delete[] pKashidaPosDropped;
                return false;
            }

            sal_Int32 nKashidasDropped = 0;
            if ( !SwScriptInfo::IsArabicText( rInf.GetTxt(), nIdx, nNext - nIdx ) )
            {
                nKashidasDropped = nKashidasInAttr;
                rKashidas -= nKashidasDropped;
            }
            else
            {
                sal_uLong nOldLayout = rInf.GetOut()->GetLayoutMode();
                rInf.GetOut()->SetLayoutMode ( nOldLayout | TEXT_LAYOUT_BIDI_RTL );
                nKashidasDropped = rInf.GetOut()->ValidateKashidas ( rInf.GetTxt(), nIdx, nNext - nIdx,
                                               nKashidasInAttr, pKashidaPos + nKashidaIdx,
                                               pKashidaPosDropped );
                rInf.GetOut()->SetLayoutMode ( nOldLayout );
                if ( nKashidasDropped )
                {
                    rSI.MarkKashidasInvalid(nKashidasDropped, pKashidaPosDropped);
                    rKashidas -= nKashidasDropped;
                    nGluePortion -= nKashidasDropped;
                }
            }
            nKashidaIdx += nKashidasInAttr;
        }
        nIdx = nNext;
    }
    delete[] pKashidaPos;
    delete[] pKashidaPosDropped;

    
    return (rKashidas > 0);
}

/*************************************************************************
 * lcl_CheckKashidaWidth()
 *************************************************************************/
static bool lcl_CheckKashidaWidth ( SwScriptInfo& rSI, SwTxtSizeInfo& rInf, SwTxtIter& rItr, sal_Int32& rKashidas,
                             sal_Int32& nGluePortion, const long nGluePortionWidth, long& nSpaceAdd )
{
    
    
    
    bool bAddSpaceChanged;
    while (rKashidas)
    {
        bAddSpaceChanged = false;
        sal_Int32 nIdx = rItr.GetStart();
        sal_Int32 nEnd = rItr.GetEnd();
        while ( nIdx < nEnd )
        {
            rItr.SeekAndChgAttrIter( nIdx, rInf.GetOut() );
            sal_Int32 nNext = rItr.GetNextAttr();

            
            
            sal_Int32 nNextScript = rSI.NextScriptChg( nIdx );
            if( nNextScript < nNext )
               nNext = nNextScript;

            if ( nNext == COMPLETE_STRING || nNext > nEnd )
                nNext = nEnd;
            sal_Int32 nKashidasInAttr = rSI.KashidaJustify ( 0, 0, nIdx, nNext - nIdx );

            long nFontMinKashida = rInf.GetOut()->GetMinKashida();
            if ( nFontMinKashida && nKashidasInAttr > 0 && SwScriptInfo::IsArabicText( rInf.GetTxt(), nIdx, nNext - nIdx ) )
            {
                sal_Int32 nKashidasDropped = 0;
                while ( rKashidas && nGluePortion && nKashidasInAttr > 0 &&
                        nSpaceAdd / SPACING_PRECISION_FACTOR < nFontMinKashida )
                {
                    --nGluePortion;
                    --rKashidas;
                    --nKashidasInAttr;
                    ++nKashidasDropped;
                    if( !rKashidas || !nGluePortion ) 
                        return false;                 

                    nSpaceAdd = nGluePortionWidth / nGluePortion;
                    bAddSpaceChanged = true;
               }
               if( nKashidasDropped )
                   rSI.MarkKashidasInvalid( nKashidasDropped, nIdx, nNext - nIdx );
            }
            if ( bAddSpaceChanged )
                break; 
            nIdx = nNext;
        }
        if ( !bAddSpaceChanged )
            break; 
    }
   return true;
}

/*************************************************************************
 * SwTxtAdjuster::CalcNewBlock()
 *
 * CalcNewBlock() must only be called _after_ CalcLine()!
 * We always span between two RandPortions or FixPortions (Tabs and Flys).
 * We count the Glues and call ExpandBlock.
 *************************************************************************/

void SwTxtAdjuster::CalcNewBlock( SwLineLayout *pCurrent,
                                  const SwLinePortion *pStopAt, SwTwips nReal, bool bSkipKashida )
{
    OSL_ENSURE( GetInfo().IsMulti() || SVX_ADJUST_BLOCK == GetAdjust(),
            "CalcNewBlock: Why?" );
    OSL_ENSURE( pCurrent->Height(), "SwTxtAdjuster::CalcBlockAdjust: missing CalcLine()" );

    pCurrent->InitSpaceAdd();
    sal_Int32 nGluePortion = 0;
    sal_Int32 nCharCnt = 0;
    MSHORT nSpaceIdx = 0;

    
    SwScriptInfo& rSI = GetInfo().GetParaPortion()->GetScriptInfo();
    SwTxtSizeInfo aInf ( GetTxtFrm() );
    SwTxtIter aItr ( GetTxtFrm(), &aInf );

    if ( rSI.CountKashida() )
    {
        while (aItr.GetCurr() != pCurrent && aItr.GetNext())
           aItr.Next();

        if( bSkipKashida )
        {
            rSI.SetNoKashidaLine ( aItr.GetStart(), aItr.GetLength());
        }
        else
        {
            rSI.ClearKashidaInvalid ( aItr.GetStart(), aItr.GetLength() );
            rSI.ClearNoKashidaLine( aItr.GetStart(), aItr.GetLength() );
        }
    }

    
    if (!bSkipKashida)
        CalcRightMargin( pCurrent, nReal );

    
    const sal_Bool bDoNotJustifyLinesWithManualBreak =
                GetTxtFrm()->GetNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK);

    SwLinePortion *pPos = pCurrent->GetPortion();

    while( pPos )
    {
        if ( bDoNotJustifyLinesWithManualBreak &&
             pPos->IsBreakPortion() && !IsLastBlock() )
        {
           pCurrent->FinishSpaceAdd();
           break;
        }

        if ( pPos->InTxtGrp() )
            nGluePortion = nGluePortion + ((SwTxtPortion*)pPos)->GetSpaceCnt( GetInfo(), nCharCnt );
        else if( pPos->IsMultiPortion() )
        {
            SwMultiPortion* pMulti = (SwMultiPortion*)pPos;
            
            
            
            
            if( pMulti->HasTabulator() )
            {
                if ( nSpaceIdx == pCurrent->GetLLSpaceAddCount() )
                    pCurrent->SetLLSpaceAdd( 0, nSpaceIdx );

                nSpaceIdx++;
                nGluePortion = 0;
                nCharCnt = 0;
            }
            else if( pMulti->IsDouble() )
                nGluePortion = nGluePortion + ((SwDoubleLinePortion*)pMulti)->GetSpaceCnt();
            else if ( pMulti->IsBidi() )
                nGluePortion = nGluePortion + ((SwBidiPortion*)pMulti)->GetSpaceCnt( GetInfo() );  
        }

        if( pPos->InGlueGrp() )
        {
            if( pPos->InFixMargGrp() )
            {
                if ( nSpaceIdx == pCurrent->GetLLSpaceAddCount() )
                    pCurrent->SetLLSpaceAdd( 0, nSpaceIdx );

                const long nGluePortionWidth = static_cast<SwGluePortion*>(pPos)->GetPrtGlue() *
                                               SPACING_PRECISION_FACTOR;

                sal_Int32 nKashidas = 0;
                if( nGluePortion && rSI.CountKashida() && !bSkipKashida )
                {
                    
                    
                    if ( !lcl_CheckKashidaPositions ( rSI, aInf, aItr, nKashidas, nGluePortion ))
                    {
                        
                        
                        pCurrent->FinishSpaceAdd();
                        GetInfo().SetIdx( nStart );
                        CalcNewBlock( pCurrent, pStopAt, nReal, true );
                        return;
                    }
                }

                if( nGluePortion )
                {
                    long nSpaceAdd = nGluePortionWidth / nGluePortion;

                    
                    if( rSI.CountKashida() && !bSkipKashida )
                    {
                        if( !lcl_CheckKashidaWidth( rSI, aInf, aItr, nKashidas, nGluePortion, nGluePortionWidth, nSpaceAdd ))
                        {
                            
                            
                            pCurrent->FinishSpaceAdd();
                            GetInfo().SetIdx( nStart );
                            CalcNewBlock( pCurrent, pStopAt, nReal, true );
                            return;
                        }
                    }

                    pCurrent->SetLLSpaceAdd( nSpaceAdd , nSpaceIdx );
                    pPos->Width( ( (SwGluePortion*)pPos )->GetFixWidth() );
                }
                else if ( IsOneBlock() && nCharCnt > 1 )
                {
                    const long nSpaceAdd = - nGluePortionWidth / ( nCharCnt - 1 );
                    pCurrent->SetLLSpaceAdd( nSpaceAdd, nSpaceIdx );
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
            pCurrent->SetLLSpaceAdd( 0, nSpaceIdx );
            break;
        }
        pPos = pPos->GetPortion();
    }
}

/*************************************************************************
 * SwTxtAdjuster::CalcKanaAdj()
 *************************************************************************/

SwTwips SwTxtAdjuster::CalcKanaAdj( SwLineLayout* pCurrent )
{
    OSL_ENSURE( pCurrent->Height(), "SwTxtAdjuster::CalcBlockAdjust: missing CalcLine()" );
    OSL_ENSURE( !pCurrent->GetpKanaComp(), "pKanaComp already exists!!" );

    std::deque<sal_uInt16> *pNewKana = new std::deque<sal_uInt16>();
    pCurrent->SetKanaComp( pNewKana );

    const sal_uInt16 nNull = 0;
    MSHORT nKanaIdx = 0;
    long nKanaDiffSum = 0;
    SwTwips nRepaintOfst = 0;
    SwTwips nX = 0;
    sal_Bool bNoCompression = sal_False;

    
    CalcRightMargin( pCurrent, 0 );

    SwLinePortion* pPos = pCurrent->GetPortion();

    while( pPos )
    {
        if ( pPos->InTxtGrp() )
        {
            
            
            sal_uInt16 nMaxWidthDiff = GetInfo().GetMaxWidthDiff( (sal_uLong)pPos );

            
            if ( !nMaxWidthDiff && pPos == pCurrent->GetFirstPortion() )
                nMaxWidthDiff = GetInfo().GetMaxWidthDiff( (sal_uLong)pCurrent );

            
            nKanaDiffSum += nMaxWidthDiff;

            
            
            if ( nMaxWidthDiff && !nRepaintOfst )
                nRepaintOfst = nX + GetLeftMargin();
        }
        else if( pPos->InGlueGrp() && pPos->InFixMargGrp() )
        {
            if ( nKanaIdx == pCurrent->GetKanaComp().size() )
                pCurrent->GetKanaComp().push_back( nNull );

            sal_uInt16 nRest;

            if ( pPos->InTabGrp() )
            {
                nRest = ! bNoCompression &&
                        ( pPos->Width() > MIN_TAB_WIDTH ) ?
                        pPos->Width() - MIN_TAB_WIDTH :
                        0;

                
                
                
                bNoCompression = !pPos->IsTabLeftPortion();
            }
            else
            {
                nRest = ! bNoCompression ?
                        ((SwGluePortion*)pPos)->GetPrtGlue() :
                        0;

                bNoCompression = sal_False;
            }

            if( nKanaDiffSum )
            {
                sal_uLong nCompress = ( 10000 * nRest ) / nKanaDiffSum;

                if ( nCompress >= 10000 )
                    
                    
                    nCompress = 0;

                else
                    nCompress = 10000 - nCompress;

                ( pCurrent->GetKanaComp() )[ nKanaIdx ] = (sal_uInt16)nCompress;
                nKanaDiffSum = 0;
            }

            nKanaIdx++;
        }

        nX += pPos->Width();
        pPos = pPos->GetPortion();
    }

    
    nKanaIdx = 0;
    sal_uInt16 nCompress = ( pCurrent->GetKanaComp() )[ nKanaIdx ];
    pPos = pCurrent->GetPortion();
    long nDecompress = 0;
    nKanaDiffSum = 0;

    while( pPos )
    {
        if ( pPos->InTxtGrp() )
        {
            const sal_uInt16 nMinWidth = pPos->Width();

            
            
            sal_uInt16 nMaxWidthDiff = GetInfo().GetMaxWidthDiff( (sal_uLong)pPos );

            
            if ( !nMaxWidthDiff && pPos == pCurrent->GetFirstPortion() )
                nMaxWidthDiff = GetInfo().GetMaxWidthDiff( (sal_uLong)pCurrent );
            nKanaDiffSum += nMaxWidthDiff;
            pPos->Width( nMinWidth +
                       ( ( 10000 - nCompress ) * nMaxWidthDiff ) / 10000 );
            nDecompress += pPos->Width() - nMinWidth;
        }
        else if( pPos->InGlueGrp() && pPos->InFixMargGrp() )
        {
            if( nCompress )
            {
                nKanaDiffSum *= nCompress;
                nKanaDiffSum /= 10000;
            }

            pPos->Width( static_cast<sal_uInt16>(pPos->Width() - nDecompress) );

            if ( pPos->InTabGrp() )
                
                ((SwTabPortion*)pPos)->SetFixWidth( pPos->Width() );

            if ( ++nKanaIdx < pCurrent->GetKanaComp().size() )
                nCompress = ( pCurrent->GetKanaComp() )[ nKanaIdx ];

            nKanaDiffSum = 0;
            nDecompress = 0;
        }
        pPos = pPos->GetPortion();
    }

    return nRepaintOfst;
}

/*************************************************************************
 * SwTxtAdjuster::CalcRightMargin()
 *************************************************************************/

SwMarginPortion *SwTxtAdjuster::CalcRightMargin( SwLineLayout *pCurrent,
    SwTwips nReal )
{
    long nRealWidth;
    const sal_uInt16 nRealHeight = GetLineHeight();
    const sal_uInt16 nLineHeight = pCurrent->Height();

    KSHORT nPrtWidth = pCurrent->PrtWidth();
    SwLinePortion *pLast = pCurrent->FindLastPortion();

    if( GetInfo().IsMulti() )
        nRealWidth = nReal;
    else
    {
        nRealWidth = GetLineWidth();
        
        const long nLeftMar = GetLeftMargin();
        SwRect aCurrRect( nLeftMar + nPrtWidth, Y() + nRealHeight - nLineHeight,
                          nRealWidth - nPrtWidth, nLineHeight );

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
        delete pFly;
    }

    SwMarginPortion *pRight = new SwMarginPortion( 0 );
    pLast->Append( pRight );

    if( long( nPrtWidth )< nRealWidth )
        pRight->PrtWidth( KSHORT( nRealWidth - nPrtWidth ) );

    
    
    
    
    
    

    pCurrent->PrtWidth( KSHORT( nRealWidth ) );
    return pRight;
}

/*************************************************************************
 * SwTxtAdjuster::CalcFlyAdjust()
 *************************************************************************/

void SwTxtAdjuster::CalcFlyAdjust( SwLineLayout *pCurrent )
{
    
    SwMarginPortion *pLeft = pCurrent->CalcLeftMargin();
    SwGluePortion *pGlue = pLeft; 


    
    
    CalcRightMargin( pCurrent );

    SwLinePortion *pPos = pLeft->GetPortion();
    sal_Int32 nLen = 0;

    
    sal_Bool bComplete = 0 == nStart;
    const sal_Bool bTabCompat = GetTxtFrm()->GetNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::TAB_COMPAT);
    sal_Bool bMultiTab = sal_False;

    while( pPos )
    {
        if ( pPos->IsMultiPortion() && ((SwMultiPortion*)pPos)->HasTabulator() )
            bMultiTab = sal_True;
        else if( pPos->InFixMargGrp() &&
               ( bTabCompat ? ! pPos->InTabGrp() : ! bMultiTab ) )
        {
            
            
            
            if( SVX_ADJUST_RIGHT == GetAdjust() )
                ((SwGluePortion*)pPos)->MoveAllGlue( pGlue );
            else
            {
                
                
                
                
                if( bComplete && GetInfo().GetTxt().getLength() == nLen )
                    ((SwGluePortion*)pPos)->MoveHalfGlue( pGlue );
                else
                {
                    if ( ! bTabCompat )
                    {
                        if( pLeft == pGlue )
                        {
                            
                            
                            if( nLen + pPos->GetLen() >= pCurrent->GetLen() )
                                ((SwGluePortion*)pPos)->MoveHalfGlue( pGlue );
                            else
                                ((SwGluePortion*)pPos)->MoveAllGlue( pGlue );
                        }
                        else
                        {
                         
                         if( !pPos->IsMarginPortion() )
                              ((SwGluePortion*)pPos)->MoveHalfGlue( pGlue );
                         }
                     }
                     else
                        ((SwGluePortion*)pPos)->MoveHalfGlue( pGlue );
                }
            }

            pGlue = (SwFlyPortion*)pPos;
            bComplete = sal_False;
        }
        nLen = nLen + pPos->GetLen();
        pPos = pPos->GetPortion();
     }

     if( ! bTabCompat && ! bMultiTab && SVX_ADJUST_RIGHT == GetAdjust() )
        
        pLeft->AdjustRight( pCurrent );
}

/*************************************************************************
 * SwTxtAdjuster::CalcAdjLine()
 *************************************************************************/

void SwTxtAdjuster::CalcAdjLine( SwLineLayout *pCurrent )
{
    OSL_ENSURE( pCurrent->IsFormatAdj(), "CalcAdjLine: Why?" );

    pCurrent->SetFormatAdj(false);

    SwParaPortion* pPara = GetInfo().GetParaPortion();

    switch( GetAdjust() )
    {
        case SVX_ADJUST_RIGHT:
        case SVX_ADJUST_CENTER:
        {
            CalcFlyAdjust( pCurrent );
            pPara->GetRepaint()->SetOfst( 0 );
            break;
        }
        case SVX_ADJUST_BLOCK:
        {
            FormatBlock();
            break;
        }
        default : return;
    }
}

/*************************************************************************
 * SwTxtAdjuster::CalcFlyPortion()
 *
 * This is a quite complicated calculation: nCurrWidth is the width _before_
 * adding the word, that still fits onto the line! For this reason the FlyPortion's
 * width is still correct if we get a deadlock-situation of:
 * bFirstWord && !WORDFITS
 *************************************************************************/

SwFlyPortion *SwTxtAdjuster::CalcFlyPortion( const long nRealWidth,
                                             const SwRect &rCurrRect )
{
    SwTxtFly aTxtFly( GetTxtFrm() );

    const KSHORT nCurrWidth = pCurr->PrtWidth();
    SwFlyPortion *pFlyPortion = 0;

    SwRect aLineVert( rCurrRect );
    if ( GetTxtFrm()->IsRightToLeft() )
        GetTxtFrm()->SwitchLTRtoRTL( aLineVert );
    if ( GetTxtFrm()->IsVertical() )
        GetTxtFrm()->SwitchHorizontalToVertical( aLineVert );

    
    SwRect aFlyRect( aTxtFly.GetFrm( aLineVert ) );

    if ( GetTxtFrm()->IsRightToLeft() )
        GetTxtFrm()->SwitchRTLtoLTR( aFlyRect );
    if ( GetTxtFrm()->IsVertical() )
        GetTxtFrm()->SwitchVerticalToHorizontal( aFlyRect );

    
    if( aFlyRect.HasArea() )
    {
        
        SwRect aLocal( aFlyRect );
        aLocal.Pos( aLocal.Left() - GetLeftMargin(), aLocal.Top() );
        if( nCurrWidth > aLocal.Left() )
            aLocal.Left( nCurrWidth );

        
        KSHORT nLocalWidth = KSHORT( aLocal.Left() + aLocal.Width() );
        if( nRealWidth < long( nLocalWidth ) )
            aLocal.Width( nRealWidth - aLocal.Left() );
        GetInfo().GetParaPortion()->SetFly( true );
        pFlyPortion = new SwFlyPortion( aLocal );
        pFlyPortion->Height( KSHORT( rCurrRect.Height() ) );
        
        pFlyPortion->AdjFixWidth();
    }
    return pFlyPortion;
}

/*************************************************************************
 * SwTxtPainter::_CalcDropAdjust()
 * Drops and Adjustment
 * CalcDropAdjust is called at the end by Format() if needed
 *************************************************************************/

void SwTxtAdjuster::CalcDropAdjust()
{
    OSL_ENSURE( 1<GetDropLines() && SVX_ADJUST_LEFT!=GetAdjust() && SVX_ADJUST_BLOCK!=GetAdjust(),
            "CalcDropAdjust: No reason for DropAdjustment." );

    const MSHORT nLineNumber = GetLineNr();

    
    Top();

    if( !pCurr->IsDummy() || NextLine() )
    {
        
        GetAdjusted();

        SwLinePortion *pPor = pCurr->GetFirstPortion();

        
        
        if( pPor->InGlueGrp() && pPor->GetPortion()
              && pPor->GetPortion()->IsDropPortion() )
        {
            const SwLinePortion *pDropPor = (SwDropPortion*) pPor->GetPortion();
            SwGluePortion *pLeft = (SwGluePortion*) pPor;

            
            pPor = pPor->GetPortion();
            while( pPor && !pPor->InFixMargGrp() )
                pPor = pPor->GetPortion();

            SwGluePortion *pRight = ( pPor && pPor->InGlueGrp() ) ?
                                    (SwGluePortion*) pPor : 0;
            if( pRight && pRight != pLeft )
            {
                
                const KSHORT nDropLineStart =
                    KSHORT(GetLineStart()) + pLeft->Width() + pDropPor->Width();
                KSHORT nMinLeft = nDropLineStart;
                for( MSHORT i = 1; i < GetDropLines(); ++i )
                {
                    if( NextLine() )
                    {
                        
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

                
                if( nMinLeft < nDropLineStart )
                {
                    
                    
                    const short nGlue = nDropLineStart - nMinLeft;
                    if( !nMinLeft )
                        pLeft->MoveAllGlue( pRight );
                    else
                        pLeft->MoveGlue( pRight, nGlue );
                }
            }
        }
    }

    if( nLineNumber != GetLineNr() )
    {
        Top();
        while( nLineNumber != GetLineNr() && Next() )
            ;
    }
}

/*************************************************************************
 * SwTxtAdjuster::CalcDropRepaint()
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
