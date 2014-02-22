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


#include "hintids.hxx"

#include <boost/scoped_ptr.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <editeng/lspcitem.hxx>
#include <txtflcnt.hxx>
#include <txtftn.hxx>
#include <flyfrms.hxx>
#include <fmtflcnt.hxx>
#include <fmtftn.hxx>
#include <ftninfo.hxx>
#include <charfmt.hxx>
#include <editeng/charrotateitem.hxx>
#include <layfrm.hxx>
#include <viewsh.hxx>
#include <viewopt.hxx>
#include <paratr.hxx>
#include <itrform2.hxx>
#include <porrst.hxx>
#include <portab.hxx>
#include <porfly.hxx>
#include <portox.hxx>
#include <porref.hxx>
#include <porfld.hxx>
#include <porftn.hxx>
#include <porhyph.hxx>
#include <pordrop.hxx>
#include <guess.hxx>
#include <blink.hxx>
#include <ftnfrm.hxx>
#include <redlnitr.hxx>
#include <pagefrm.hxx>
#include <pagedesc.hxx>
#include <tgrditem.hxx>
#include <doc.hxx>
#include <pormulti.hxx>
#include <unotools/charclass.hxx>

#include <vector>

#include <config_graphite.h>

#if OSL_DEBUG_LEVEL > 1
#include <ndtxt.hxx>
#endif

using namespace ::com::sun::star;

namespace {
    
    static long lcl_CalcOptRepaint( SwTxtFormatter &rThis,
                                    SwLineLayout &rCurr,
                                    const sal_Int32 nOldLineEnd,
                                    const std::vector<long> &rFlyStarts );
    
    static bool lcl_BuildHiddenPortion( const SwTxtSizeInfo& rInf, sal_Int32 &rPos );

    
    static bool lcl_HasSameBorder(const SwFont& rFirst, const SwFont& rSecond);
}

inline void ClearFly( SwTxtFormatInfo &rInf )
{
    delete rInf.GetFly();
    rInf.SetFly(0);
}

/*************************************************************************
 *                  SwTxtFormatter::CtorInitTxtFormatter()
 *************************************************************************/

void SwTxtFormatter::CtorInitTxtFormatter( SwTxtFrm *pNewFrm, SwTxtFormatInfo *pNewInf )
{
    CtorInitTxtPainter( pNewFrm, pNewInf );
    pInf = pNewInf;
    pDropFmt = GetInfo().GetDropFmt();
    pMulti = NULL;

    bOnceMore = false;
    bFlyInCntBase = false;
    bChanges = false;
    bTruncLines = false;
    nCntEndHyph = 0;
    nCntMidHyph = 0;
    nLeftScanIdx = COMPLETE_STRING;
    nRightScanIdx = 0;
    m_nHintEndIndex = 0;
    m_pFirstOfBorderMerge = 0;

    if( nStart > GetInfo().GetTxt().getLength() )
    {
        OSL_ENSURE( !this, "+SwTxtFormatter::CTOR: bad offset" );
        nStart = GetInfo().GetTxt().getLength();
    }

}

/*************************************************************************
 *                      SwTxtFormatter::DTOR
 *************************************************************************/

SwTxtFormatter::~SwTxtFormatter()
{
    
    
    if( GetInfo().GetRest() )
    {
        delete GetInfo().GetRest();
        GetInfo().SetRest(0);
    }
}

/*************************************************************************
 *                      SwTxtFormatter::Insert()
 *************************************************************************/

void SwTxtFormatter::Insert( SwLineLayout *pLay )
{
    
    if ( pCurr )
    {
        pLay->SetNext( pCurr->GetNext() );
        pCurr->SetNext( pLay );
    }
    else
        pCurr = pLay;
}

/*************************************************************************
 *                  SwTxtFormatter::GetFrmRstHeight()
 *************************************************************************/

KSHORT SwTxtFormatter::GetFrmRstHeight() const
{
    
    
    //
    
    
    const SwFrm *pPage = (const SwFrm*)pFrm->FindPageFrm();
    const SwTwips nHeight = pPage->Frm().Top()
                          + pPage->Prt().Top()
                          + pPage->Prt().Height() - Y();
    if( 0 > nHeight )
        return pCurr->Height();
    else
        return KSHORT( nHeight );
}

/*************************************************************************
 *                  SwTxtFormatter::UnderFlow()
 *************************************************************************/

SwLinePortion *SwTxtFormatter::UnderFlow( SwTxtFormatInfo &rInf )
{
    
    SwLinePortion *pUnderFlow = rInf.GetUnderFlow();
    if( !pUnderFlow )
        return 0;

    
    
    

    const sal_Int32 nSoftHyphPos = rInf.GetSoftHyphPos();
    const sal_Int32 nUnderScorePos = rInf.GetUnderScorePos();

    
    
    SwFlyPortion *pFly = rInf.GetFly();
    rInf.SetFly( 0 );

    FeedInf( rInf );
    rInf.SetLast( pCurr );
    
    
    rInf.SetUnderFlow(0);
    rInf.SetSoftHyphPos( nSoftHyphPos );
    rInf.SetUnderScorePos( nUnderScorePos );
    rInf.SetPaintOfst( GetLeftMargin() );

    
    SwLinePortion *pPor = pCurr->GetFirstPortion();
    if( pPor != pUnderFlow )
    {
        
        
        
        
        SwLinePortion *pTmpPrev = pPor;
        while( pPor && pPor != pUnderFlow )
        {
            if( !pPor->IsKernPortion() &&
                ( pPor->Width() || pPor->IsSoftHyphPortion() ) )
            {
                while( pTmpPrev != pPor )
                {
                    pTmpPrev->Move( rInf );
                    rInf.SetLast( pTmpPrev );
                    pTmpPrev = pTmpPrev->GetPortion();
                    OSL_ENSURE( pTmpPrev, "UnderFlow: Loosing control!" );
                };
            }
            pPor = pPor->GetPortion();
        }
        pPor = pTmpPrev;
        if( pPor && 
            ( pPor->IsFlyPortion() || pPor->IsDropPortion() ||
              pPor->IsFlyCntPortion() ) )
        {
            pPor->Move( rInf );
            rInf.SetLast( pPor );
            rInf.SetStopUnderFlow( true );
            pPor = pUnderFlow;
        }
    }

    
    OSL_ENSURE( pPor, "SwTxtFormatter::UnderFlow: overflow but underflow" );

    











    /*--------------------------------------------------
     * Snapshot
     * --------------------------------------------------*/
    if ( pPor==rInf.GetLast() )
    {
        
        
        
        rInf.SetFly( pFly );
        pPor->Truncate();
        return pPor; 
    }
    /*---------------------------------------------------
     * End the snapshot
     * --------------------------------------------------*/

    
    if( !pPor || !(rInf.X() + pPor->Width()) )
    {
        delete pFly;
        return 0;
    }

    
    
    SeekAndChg( rInf );

    
    
    rInf.Width( (sal_uInt16)(rInf.X() + (pPor->Width() ? pPor->Width() - 1 : 0)) );
    rInf.SetLen( pPor->GetLen() );
    rInf.SetFull( false );
    if( pFly )
    {
        
        
        
        
        rInf.SetFly( pFly );
        CalcFlyWidth( rInf );
    }
    rInf.GetLast()->SetPortion(0);

    
    
    
    if( rInf.GetLast() == pCurr )
    {
        if( pPor->InTxtGrp() && !pPor->InExpGrp() )
        {
            MSHORT nOldWhich = pCurr->GetWhichPor();
            *(SwLinePortion*)pCurr = *pPor;
            pCurr->SetPortion( pPor->GetPortion() );
            pCurr->SetWhichPor( nOldWhich );
            pPor->SetPortion( 0 );
            delete pPor;
            pPor = pCurr;
        }
    }
    pPor->Truncate();
    SwLinePortion *const pRest( rInf.GetRest() );
    if (pRest && pRest->InFldGrp() &&
        static_cast<SwFldPortion*>(pRest)->IsNoLength())
    {
        
        --m_nHintEndIndex;
    }
    delete pRest;
    rInf.SetRest(0);
    return pPor;
}

/*************************************************************************
 *                      SwTxtFormatter::InsertPortion()
 *************************************************************************/

void SwTxtFormatter::InsertPortion( SwTxtFormatInfo &rInf,
                                    SwLinePortion *pPor ) const
{
    
    
    if( pPor == pCurr )
    {
        if ( pCurr->GetPortion() )
        {
            pPor = pCurr->GetPortion();
        }

        
        rInf.SetOtherThanFtnInside( rInf.IsOtherThanFtnInside() || !pPor->IsFtnPortion() );
    }
    else
    {
        SwLinePortion *pLast = rInf.GetLast();
        if( pLast->GetPortion() )
        {
            while( pLast->GetPortion() )
                pLast = pLast->GetPortion();
            rInf.SetLast( pLast );
        }
        pLast->Insert( pPor );

        rInf.SetOtherThanFtnInside( rInf.IsOtherThanFtnInside() || !pPor->IsFtnPortion() );

        
        if( pCurr->Height() < pPor->Height() )
            pCurr->Height( pPor->Height() );
        if( pCurr->GetAscent() < pPor->GetAscent() )
            pCurr->SetAscent( pPor->GetAscent() );
    }

    
    rInf.SetLast( pPor );
    while( pPor )
    {
        pPor->Move( rInf );
        rInf.SetLast( pPor );
        pPor = pPor->GetPortion();
    }
}

/*************************************************************************
 *                      SwTxtFormatter::BuildPortion()
 *************************************************************************/

void SwTxtFormatter::BuildPortions( SwTxtFormatInfo &rInf )
{
    OSL_ENSURE( rInf.GetTxt().getLength() < COMPLETE_STRING,
            "SwTxtFormatter::BuildPortions: bad text length in info" );

    rInf.ChkNoHyph( CntEndHyph(), CntMidHyph() );

    
    
    
    rInf.SetLast( pCurr );
    rInf.ForcedLeftMargin( 0 );

    OSL_ENSURE( pCurr->FindLastPortion() == pCurr, "pLast supposed to equal pCurr" );

    if( !pCurr->GetAscent() && !pCurr->Height() )
        CalcAscent( rInf, pCurr );

    SeekAndChg( rInf );

    
    OSL_ENSURE( !rInf.X() || pMulti, "SwTxtFormatter::BuildPortion X=0?" );
    CalcFlyWidth( rInf );
    SwFlyPortion *pFly = rInf.GetFly();
    if( pFly )
    {
        if ( 0 < pFly->Fix() )
            ClearFly( rInf );
        else
            rInf.SetFull(true);
    }

    SwLinePortion *pPor = NewPortion( rInf );

    
    GETGRID( pFrm->FindPageFrm() )
    const bool bHasGrid = pGrid && rInf.SnapToGrid() &&
                              GRID_LINES_CHARS == pGrid->GetGridType();

    const SwDoc *pDoc = rInf.GetTxtFrm()->GetNode()->GetDoc();
    const sal_uInt16 nGridWidth = bHasGrid ?
                                GETGRIDWIDTH(pGrid,pDoc) : 0;   

    
    
    
    SwKernPortion* pGridKernPortion = 0;

    bool bFull = false;
    SwTwips nUnderLineStart = 0;
    rInf.Y( Y() );

    while( pPor && !rInf.IsStop() )
    {
        OSL_ENSURE( rInf.GetLen() < COMPLETE_STRING &&
                rInf.GetIdx() <= rInf.GetTxt().getLength(),
                "SwTxtFormatter::BuildPortions: bad length in info" );

        
        
        if( pPor->InFldGrp() )
            ((SwFldPortion*)pPor)->CheckScript( rInf );

        if( ! bHasGrid && rInf.HasScriptSpace() &&
            rInf.GetLast() && rInf.GetLast()->InTxtGrp() &&
            rInf.GetLast()->Width() && !rInf.GetLast()->InNumberGrp() )
        {
            sal_uInt8 nNxtActual = rInf.GetFont()->GetActual();
            sal_uInt8 nLstActual = nNxtActual;
            sal_uInt16 nLstHeight = (sal_uInt16)rInf.GetFont()->GetHeight();
            bool bAllowBefore = false;
            bool bAllowBehind = false;
            const CharClass& rCC = GetAppCharClass();

            
            
            if ( pPor->InFldGrp() )
            {
                OUString aAltTxt;
                if ( ((SwFldPortion*)pPor)->GetExpTxt( rInf, aAltTxt ) &&
                        !aAltTxt.isEmpty() )
                {
                    bAllowBehind = rCC.isLetterNumeric( aAltTxt, 0 );

                    const SwFont* pTmpFnt = ((SwFldPortion*)pPor)->GetFont();
                    if ( pTmpFnt )
                        nNxtActual = pTmpFnt->GetActual();
                }
            }
            else
            {
                const OUString& rTxt = rInf.GetTxt();
                sal_Int32 nIdx = rInf.GetIdx();
                bAllowBehind = nIdx < rTxt.getLength() ? rCC.isLetterNumeric(rTxt, nIdx) : false;
            }

            const SwLinePortion* pLast = rInf.GetLast();
            if ( bAllowBehind && pLast )
            {
                if ( pLast->InFldGrp() )
                {
                    OUString aAltTxt;
                    if ( ((SwFldPortion*)pLast)->GetExpTxt( rInf, aAltTxt ) &&
                         !aAltTxt.isEmpty() )
                    {
                        bAllowBefore = rCC.isLetterNumeric( aAltTxt, aAltTxt.getLength() - 1 );

                        const SwFont* pTmpFnt = ((SwFldPortion*)pLast)->GetFont();
                        if ( pTmpFnt )
                        {
                            nLstActual = pTmpFnt->GetActual();
                            nLstHeight = (sal_uInt16)pTmpFnt->GetHeight();
                        }
                    }
                }
                else if ( rInf.GetIdx() )
                {
                    bAllowBefore = rCC.isLetterNumeric( rInf.GetTxt(), rInf.GetIdx() - 1 );
                    
                    if ( bAllowBefore )
                        nLstActual = pScriptInfo->ScriptType( rInf.GetIdx() - 1 ) - 1;
                }

                nLstHeight /= 5;
                
                if( bAllowBefore && ( nLstActual != nNxtActual ) &&
                    nLstHeight && rInf.X() + nLstHeight <= rInf.Width() )
                {
                    SwKernPortion* pKrn =
                        new SwKernPortion( *rInf.GetLast(), nLstHeight,
                                           pLast->InFldGrp() && pPor->InFldGrp() );
                    rInf.GetLast()->SetPortion( NULL );
                    InsertPortion( rInf, pKrn );
                }
            }
        }
        else if ( bHasGrid && ! pGridKernPortion && ! pMulti )
        {
            
            if ( ! pGridKernPortion )
                pGridKernPortion = pPor->IsKernPortion() ?
                                   (SwKernPortion*)pPor :
                                   new SwKernPortion( *pCurr );

            
            
            const SwPageFrm* pPageFrm = pFrm->FindPageFrm();
            const SwLayoutFrm* pBody = pPageFrm->FindBodyCont();
            SWRECTFN( pPageFrm )

            const long nGridOrigin = pBody ?
                                    (pBody->*fnRect->fnGetPrtLeft)() :
                                    (pPageFrm->*fnRect->fnGetPrtLeft)();

            SwTwips nStartX = rInf.X() + GetLeftMargin();
            if ( bVert )
            {
                Point aPoint( nStartX, 0 );
                pFrm->SwitchHorizontalToVertical( aPoint );
                nStartX = aPoint.Y();
            }

            const SwTwips nOfst = nStartX - nGridOrigin;
            if ( nOfst )
            {
                const sal_uLong i = ( nOfst > 0 ) ?
                                ( ( nOfst - 1 ) / nGridWidth + 1 ) :
                                0;
                const SwTwips nKernWidth = i * nGridWidth - nOfst;
                const SwTwips nRestWidth = rInf.Width() - rInf.X();

                if ( nKernWidth <= nRestWidth )
                    pGridKernPortion->Width( (sal_uInt16)nKernWidth );
            }

            if ( pGridKernPortion != pPor )
                InsertPortion( rInf, pGridKernPortion );
        }

        if( pPor->IsDropPortion() )
            MergeCharacterBorder(*static_cast<SwDropPortion*>(pPor));

        
        if( pPor->IsMultiPortion() && ( !pMulti || pMulti->IsBidi() ) )
            bFull = BuildMultiPortion( rInf, *((SwMultiPortion*)pPor) );
        else
            bFull = pPor->Format( rInf );

        if( rInf.IsRuby() && !rInf.GetRest() )
            bFull = true;

        
        
        if ( UNDERLINE_NONE != pFnt->GetUnderline() && ! nUnderLineStart )
            nUnderLineStart = GetLeftMargin() + rInf.X();

        if ( pPor->IsFlyPortion() )
            pCurr->SetFly( true );
        
        
        
        
        
        
        
        
        else if ( ( ! rInf.GetPaintOfst() || nUnderLineStart < rInf.GetPaintOfst() ) &&
                  
                  nUnderLineStart &&
                     
                     
                  ( ( rInf.GetReformatStart() == rInf.GetIdx() &&
                      UNDERLINE_NONE == pFnt->GetUnderline()
                    ) ||
                     
                    ( rInf.GetReformatStart() >= rInf.GetIdx() &&
                      rInf.GetReformatStart() <= rInf.GetIdx() + pPor->GetLen() &&
                      UNDERLINE_NONE != pFnt->GetUnderline() ) ) )
            rInf.SetPaintOfst( nUnderLineStart );
        else if (  ! rInf.GetPaintOfst() &&
                   
                   ( ( pPor->InTabGrp() && !pPor->IsTabLeftPortion() ) ||
                   
                     ( pPor->IsMultiPortion() && ((SwMultiPortion*)pPor)->IsBidi() ) ||
                   
                     ( ( pPor->IsDropPortion() || pPor->IsMultiPortion() ) &&
                       rInf.GetReformatStart() >= rInf.GetIdx() &&
                       rInf.GetReformatStart() <= rInf.GetIdx() + pPor->GetLen() )
                   
                     || ( bHasGrid && SW_CJK != pFnt->GetActual() )
                   )
                )
            
            
            rInf.SetPaintOfst( GetLeftMargin() + rInf.X() );

        
        
        if ( IsUnderlineBreak( *pPor, *pFnt ) )
            nUnderLineStart = 0;

        if( pPor->IsFlyCntPortion() || ( pPor->IsMultiPortion() &&
            ((SwMultiPortion*)pPor)->HasFlyInCntnt() ) )
            SetFlyInCntBase();
        
        if ( !bFull )
        {
            rInf.ClrUnderFlow();
            if( ! bHasGrid && rInf.HasScriptSpace() && pPor->InTxtGrp() &&
                pPor->GetLen() && !pPor->InFldGrp() )
            {
                
                
                sal_Int32 nTmp = rInf.GetIdx() + pPor->GetLen();
                if( nTmp == pScriptInfo->NextScriptChg( nTmp - 1 ) &&
                    nTmp != rInf.GetTxt().getLength() )
                {
                    sal_uInt16 nDist = (sal_uInt16)(rInf.GetFont()->GetHeight()/5);

                    if( nDist )
                    {
                        
                        
                        const CharClass& rCC = GetAppCharClass();
                        if ( rCC.isLetterNumeric( rInf.GetTxt(), nTmp - 1 ) &&
                             rCC.isLetterNumeric( rInf.GetTxt(), nTmp ) )
                        {
                            
                            if ( rInf.X() + pPor->Width() + nDist <= rInf.Width() )
                                new SwKernPortion( *pPor, nDist );
                            else
                                bFull = true;
                        }
                    }
                }
            }
        }

        if ( bHasGrid && pPor != pGridKernPortion && ! pMulti )
        {
            sal_Int32 nTmp = rInf.GetIdx() + pPor->GetLen();
            const SwTwips nRestWidth = rInf.Width() - rInf.X() - pPor->Width();

            const sal_uInt8 nCurrScript = pFnt->GetActual(); 
            const sal_uInt8 nNextScript = nTmp >= rInf.GetTxt().getLength() ?
                                     SW_CJK :
                                     SwScriptInfo::WhichFont( nTmp, 0, pScriptInfo );

            
            
            
            
            if ( nRestWidth > 0 && SW_CJK != nCurrScript &&
                ! rInf.IsUnderFlow() && ( bFull || SW_CJK == nNextScript ) )
            {
                OSL_ENSURE( pGridKernPortion, "No GridKernPortion available" );

                
                SwLinePortion* pTmpPor = pGridKernPortion->GetPortion();
                sal_uInt16 nSumWidth = pPor->Width();
                while ( pTmpPor )
                {
                    nSumWidth = nSumWidth + pTmpPor->Width();
                    pTmpPor = pTmpPor->GetPortion();
                }

                const sal_uInt16 i = nSumWidth ?
                                 ( nSumWidth - 1 ) / nGridWidth + 1 :
                                 0;
                const SwTwips nTmpWidth = i * nGridWidth;
                const SwTwips nKernWidth = std::min( (SwTwips)(nTmpWidth - nSumWidth),
                                                nRestWidth );
                const sal_uInt16 nKernWidth_1 = (sal_uInt16)(nKernWidth / 2);

                OSL_ENSURE( nKernWidth <= nRestWidth,
                        "Not enough space left for adjusting non-asian text in grid mode" );

                pGridKernPortion->Width( pGridKernPortion->Width() + nKernWidth_1 );
                rInf.X( rInf.X() + nKernWidth_1 );

                if ( ! bFull )
                    new SwKernPortion( *pPor, (short)(nKernWidth - nKernWidth_1),
                                       false, true );

                pGridKernPortion = 0;
            }
            else if ( pPor->IsMultiPortion() || pPor->InFixMargGrp() ||
                      pPor->IsFlyCntPortion() || pPor->InNumberGrp() ||
                      pPor->InFldGrp() || nCurrScript != nNextScript )
                
                pGridKernPortion = 0;
        }

        rInf.SetFull( bFull );

        if( !pPor->IsDropPortion() )
            MergeCharacterBorder(*pPor, rInf);

        
        if ( !pPor->GetLen() && !pPor->IsFlyPortion()
            && !pPor->IsGrfNumPortion() && ! pPor->InNumberGrp()
            && !pPor->IsMultiPortion() )
            CalcAscent( rInf, pPor );

        InsertPortion( rInf, pPor );
        pPor = NewPortion( rInf );
    }

    if( !rInf.IsStop() )
    {
        
        SwTabPortion *pLastTab = rInf.GetLastTab();
        if( pLastTab )
            pLastTab->FormatEOL( rInf );
        else if( rInf.GetLast() && rInf.LastKernPortion() )
            rInf.GetLast()->FormatEOL( rInf );
    }
    if( pCurr->GetPortion() && pCurr->GetPortion()->InNumberGrp()
        && ((SwNumberPortion*)pCurr->GetPortion())->IsHide() )
        rInf.SetNumDone( false );

    
    ClearFly( rInf );

    
    rInf.SetTabOverflow( false );
}

/*************************************************************************
 *                 SwTxtFormatter::CalcAdjustLine()
 *************************************************************************/

void SwTxtFormatter::CalcAdjustLine( SwLineLayout *pCurrent )
{
    if( SVX_ADJUST_LEFT != GetAdjust() && !pMulti)
    {
        pCurrent->SetFormatAdj(true);
        if( IsFlyInCntBase() )
        {
            CalcAdjLine( pCurrent );
            
            
            UpdatePos( pCurrent, GetTopLeft(), GetStart(), true );
        }
    }
}

/*************************************************************************
 *                      SwTxtFormatter::CalcAscent()
 *************************************************************************/

void SwTxtFormatter::CalcAscent( SwTxtFormatInfo &rInf, SwLinePortion *pPor )
{
    bool bCalc = false;
    if ( pPor->InFldGrp() && ((SwFldPortion*)pPor)->GetFont() )
    {
        
        
        SwFont* pFldFnt = ((SwFldPortion*)pPor)->pFnt;
        SwFontSave aSave( rInf, pFldFnt );
        pPor->Height( rInf.GetTxtHeight() );
        pPor->SetAscent( rInf.GetAscent() );
        bCalc = true;
    }
    
    
    
    else if ( pPor->InTabGrp() && pPor->GetLen() == 0 &&
              rInf.GetLast() && rInf.GetLast()->InNumberGrp() &&
              static_cast<const SwNumberPortion*>(rInf.GetLast())->HasFont() )
    {
        const SwLinePortion* pLast = rInf.GetLast();
        pPor->Height( pLast->Height() );
        pPor->SetAscent( pLast->GetAscent() );
    }
    else
    {
        const SwLinePortion *pLast = rInf.GetLast();
        bool bChg = false;

        
        const bool bFirstPor = rInf.GetLineStart() == rInf.GetIdx();
        if ( pPor->IsQuoVadisPortion() )
            bChg = SeekStartAndChg( rInf, true );
        else
        {
            if( bFirstPor )
            {
                if( !rInf.GetTxt().isEmpty() )
                {
                    if ( pPor->GetLen() || !rInf.GetIdx()
                         || ( pCurr != pLast && !pLast->IsFlyPortion() )
                         || !pCurr->IsRest() ) 
                        bChg = SeekAndChg( rInf );
                    else
                        bChg = SeekAndChgBefore( rInf );
                }
                else if ( pMulti )
                    
                    
                    
                    
                    
                    bChg = false;
                else
                    bChg = SeekStartAndChg( rInf );
            }
            else
                bChg = SeekAndChg( rInf );
        }
        if( bChg || bFirstPor || !pPor->GetAscent()
            || !rInf.GetLast()->InTxtGrp() )
        {
            pPor->SetAscent( rInf.GetAscent()  );
            pPor->Height( rInf.GetTxtHeight() );
            bCalc = true;
        }
        else
        {
            pPor->Height( pLast->Height() );
            pPor->SetAscent( pLast->GetAscent() );
        }
    }

    if( pPor->InTxtGrp() && bCalc )
    {
        pPor->SetAscent(pPor->GetAscent() +
            rInf.GetFont()->GetTopBorderSpace());
        pPor->Height(pPor->Height() +
            rInf.GetFont()->GetTopBorderSpace() +
            rInf.GetFont()->GetBottomBorderSpace() );
    }
}

/*************************************************************************
 *                      class SwMetaPortion
 *************************************************************************/

class SwMetaPortion : public SwTxtPortion
{
public:
    inline  SwMetaPortion() { SetWhichPor( POR_META ); }
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;

};

/*************************************************************************
 *               virtual SwMetaPortion::Paint()
 *************************************************************************/

void SwMetaPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if ( Width() )
    {
        rInf.DrawViewOpt( *this, POR_META );
        SwTxtPortion::Paint( rInf );
    }
}


/*************************************************************************
 *                      SwTxtFormatter::WhichTxtPor()
 *************************************************************************/

SwTxtPortion *SwTxtFormatter::WhichTxtPor( SwTxtFormatInfo &rInf ) const
{
    SwTxtPortion *pPor = 0;
    if( GetFnt()->IsTox() )
    {
        pPor = new SwToxPortion;
    }
    else if ( GetFnt()->IsInputField() )
    {
        pPor = new SwTxtInputFldPortion();
    }
    else
    {
        if( GetFnt()->IsRef() )
            pPor = new SwRefPortion;
        else if (GetFnt()->IsMeta())
        {
            pPor = new SwMetaPortion;
        }
        else
        {
            
            
            
            if( rInf.GetLen() > 0 )
            {
                if( rInf.GetTxt()[rInf.GetIdx()]==CH_TXT_ATR_FIELDSTART )
                    pPor = new SwFieldMarkPortion();
                else if( rInf.GetTxt()[rInf.GetIdx()]==CH_TXT_ATR_FIELDEND )
                    pPor = new SwFieldMarkPortion();
                else if( rInf.GetTxt()[rInf.GetIdx()]==CH_TXT_ATR_FORMELEMENT )
                    pPor = new SwFieldFormPortion();
            }
            if( !pPor )
            {
                if( !rInf.X() && !pCurr->GetPortion() && !pCurr->GetLen() && !GetFnt()->IsURL() )
                    pPor = pCurr;
                else
                {
                    pPor = new SwTxtPortion;
                    if ( GetFnt()->IsURL() )
                    {
                        pPor->SetWhichPor( POR_URL );
                    }
                }
            }
        }
    }
    return pPor;
}

/*************************************************************************
 *                      SwTxtFormatter::NewTxtPortion()
 *************************************************************************/






SwTxtPortion *SwTxtFormatter::NewTxtPortion( SwTxtFormatInfo &rInf )
{
    
    
    Seek( rInf.GetIdx() );
    SwTxtPortion *pPor = WhichTxtPor( rInf );

    
    const sal_Int32 nNextAttr = GetNextAttr();
    sal_Int32 nNextChg = std::min( nNextAttr, rInf.GetTxt().getLength() );

    
    const sal_Int32 nNextScript = pScriptInfo->NextScriptChg( rInf.GetIdx() );
    nNextChg = std::min( nNextChg, nNextScript );

    
    const sal_Int32 nNextDir = pScriptInfo->NextDirChg( rInf.GetIdx() );
    nNextChg = std::min( nNextChg, nNextDir );

    
    
    
    
    //
    
    
    //
    
    
    
    

    pPor->SetLen(1);
    CalcAscent( rInf, pPor );

    const SwFont* pTmpFnt = rInf.GetFont();
    sal_Int32 nExpect = std::min( sal_Int32( ((Font *)pTmpFnt)->GetSize().Height() ),
                             sal_Int32( pPor->GetAscent() ) ) / 8;
    if ( !nExpect )
        nExpect = 1;
    nExpect = rInf.GetIdx() + ((rInf.Width() - rInf.X()) / nExpect);
    if( nExpect > rInf.GetIdx() && nNextChg > nExpect )
        nNextChg = std::min( nExpect, rInf.GetTxt().getLength() );

    
    
    
    if ( nLeftScanIdx <= rInf.GetIdx() && rInf.GetIdx() <= nRightScanIdx )
    {
        if ( nNextChg > nRightScanIdx )
            nNextChg = nRightScanIdx =
                rInf.ScanPortionEnd( nRightScanIdx, nNextChg );
    }
    else
    {
        nLeftScanIdx = rInf.GetIdx();
        nNextChg = nRightScanIdx =
                rInf.ScanPortionEnd( rInf.GetIdx(), nNextChg );
    }

    pPor->SetLen( nNextChg - rInf.GetIdx() );
    rInf.SetLen( pPor->GetLen() );
    return pPor;
}


/*************************************************************************
 *                 SwTxtFormatter::WhichFirstPortion()
 *************************************************************************/

SwLinePortion *SwTxtFormatter::WhichFirstPortion(SwTxtFormatInfo &rInf)
{
    SwLinePortion *pPor = 0;

    if( rInf.GetRest() )
    {
        
        if( '\0' != rInf.GetHookChar() )
            return 0;

        pPor = rInf.GetRest();
        if( pPor->IsErgoSumPortion() )
            rInf.SetErgoDone(true);
        else
            if( pPor->IsFtnNumPortion() )
                rInf.SetFtnDone(true);
            else
                if( pPor->InNumberGrp() )
                    rInf.SetNumDone(true);

        rInf.SetRest(0);
        pCurr->SetRest( true );
        return pPor;
    }

    
    
    if( rInf.GetIdx() )
    {
        

        
        if( !rInf.IsErgoDone() )
        {
            if( pFrm->IsInFtn() && !pFrm->GetIndPrev() )
                pPor = (SwLinePortion*)NewErgoSumPortion( rInf );
            rInf.SetErgoDone( true );
        }

        
        if( !pPor && !rInf.IsArrowDone() )
        {
            if( pFrm->GetOfst() && !pFrm->IsFollow() &&
                rInf.GetIdx() == pFrm->GetOfst() )
                pPor = new SwArrowPortion( *pCurr );
            rInf.SetArrowDone( true );
        }

        
        if ( ! pPor && ! pCurr->GetPortion() )
        {
            GETGRID( GetTxtFrm()->FindPageFrm() )
            if ( pGrid )
                pPor = new SwKernPortion( *pCurr );
        }

        
        if( !pPor )
        {
            pPor = rInf.GetRest();
            
            if( pPor )
            {
                pCurr->SetRest( true );
                rInf.SetRest(0);
            }
        }
    }
    else
    {
        
        if( !rInf.IsFtnDone() )
        {
            OSL_ENSURE( ( ! rInf.IsMulti() && ! pMulti ) || pMulti->HasRotation(),
                     "Rotated number portion trouble" );

            const bool bFtnNum = pFrm->IsFtnNumFrm();
            rInf.GetParaPortion()->SetFtnNum( bFtnNum );
            if( bFtnNum )
                pPor = (SwLinePortion*)NewFtnNumPortion( rInf );
            rInf.SetFtnDone( true );
        }

        
        
        if( !rInf.IsErgoDone() && !pPor && ! rInf.IsMulti() )
        {
            if( pFrm->IsInFtn() && !pFrm->GetIndPrev() )
                pPor = (SwLinePortion*)NewErgoSumPortion( rInf );
            rInf.SetErgoDone( true );
        }

        
        if( !rInf.IsNumDone() && !pPor )
        {
            OSL_ENSURE( ( ! rInf.IsMulti() && ! pMulti ) || pMulti->HasRotation(),
                     "Rotated number portion trouble" );

            
            if( GetTxtFrm()->GetTxtNode()->GetNumRule() )
                pPor = (SwLinePortion*)NewNumberPortion( rInf );
            rInf.SetNumDone( true );
        }
        
        if( !pPor && GetDropFmt() && ! rInf.IsMulti() )
            pPor = (SwLinePortion*)NewDropPortion( rInf );

        
        if ( !pPor && !pCurr->GetPortion() )
        {
            GETGRID( GetTxtFrm()->FindPageFrm() )
            if ( pGrid )
                pPor = new SwKernPortion( *pCurr );
        }
    }

        
        if ( !pPor && !pCurr->GetPortion() &&
             GetTxtFrm()->IsInTab() &&
             GetTxtFrm()->GetTxtNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::TAB_COMPAT) )
        {
            pPor = NewTabPortion( rInf, true );
        }

        
        if (!pPor)
        {
            pPor = TryNewNoLengthPortion(rInf);
        }

    return pPor;
}

static bool lcl_OldFieldRest( const SwLineLayout* pCurr )
{
    if( !pCurr->GetNext() )
        return false;
    const SwLinePortion *pPor = pCurr->GetNext()->GetPortion();
    bool bRet = false;
    while( pPor && !bRet )
    {
        bRet = (pPor->InFldGrp() && ((SwFldPortion*)pPor)->IsFollow()) ||
            (pPor->IsMultiPortion() && ((SwMultiPortion*)pPor)->IsFollowFld());
        if( !pPor->GetLen() )
            break;
        pPor = pPor->GetPortion();
    }
    return bRet;
}

/*************************************************************************
 *                      SwTxtFormatter::NewPortion()
 *************************************************************************/

/* NewPortion sets rInf.nLen
 * A SwTxtPortion is limited by a tab, break, txtatr or attr change
 * We can have three cases:
 * 1) The line is full and the wrap was not emulated
 *    -> return 0;
 * 2) The line is full and a wrap was emulated
 *    -> Reset width and return new FlyPortion
 * 3) We need to construct a new portion
 *    -> CalcFlyWidth emulates the width and return portion, if needed
 */

SwLinePortion *SwTxtFormatter::NewPortion( SwTxtFormatInfo &rInf )
{
    
    rInf.SetStopUnderFlow( false );
    if( rInf.GetUnderFlow() )
    {
        OSL_ENSURE( rInf.IsFull(), "SwTxtFormatter::NewPortion: underflow but not full" );
        return UnderFlow( rInf );
    }

    
    if( rInf.IsFull() )
    {
        
        
        if( rInf.IsNewLine() && (!rInf.GetFly() || !pCurr->IsDummy()) )
            return 0;

        
        
        
        
        
        if( rInf.GetFly() )
        {
            if( rInf.GetLast()->IsBreakPortion() )
            {
                delete rInf.GetFly();
                rInf.SetFly( 0 );
            }

            return rInf.GetFly();
        }
        
        
        
        
        if( rInf.GetRest() )
            rInf.SetNewLine( true );
        else
        {
            
            
            
            if( lcl_OldFieldRest( GetCurr() ) )
                rInf.SetNewLine( true );
            else
            {
                SwLinePortion *pFirst = WhichFirstPortion( rInf );
                if( pFirst )
                {
                    rInf.SetNewLine( true );
                    if( pFirst->InNumberGrp() )
                        rInf.SetNumDone( false) ;
                    delete pFirst;
                }
            }
        }

        return 0;
    }

    SwLinePortion *pPor = WhichFirstPortion( rInf );

    
    if ( !pPor )
    {
        sal_Int32 nEnd = rInf.GetIdx();
        if ( ::lcl_BuildHiddenPortion( rInf, nEnd ) )
            pPor = new SwHiddenTextPortion( nEnd - rInf.GetIdx() );
    }

    if( !pPor )
    {
        if( ( !pMulti || pMulti->IsBidi() ) &&
            
            
            ( !rInf.GetRest() || '\0' == rInf.GetHookChar() ) )
        {
            
            
            sal_Int32 nEnd = rInf.GetIdx();
            SwMultiCreator* pCreate = rInf.GetMultiCreator( nEnd, pMulti );
            if( pCreate )
            {
                SwMultiPortion* pTmp = NULL;

                if ( SW_MC_BIDI == pCreate->nId )
                    pTmp = new SwBidiPortion( nEnd, pCreate->nLevel );
                else if ( SW_MC_RUBY == pCreate->nId )
                {
                    Seek( rInf.GetIdx() );
                    bool bRubyTop = false;
                    bool* pRubyPos = 0;

                    if ( rInf.SnapToGrid() )
                    {
                        GETGRID( GetTxtFrm()->FindPageFrm() )
                        if ( pGrid )
                        {
                            bRubyTop = ! pGrid->GetRubyTextBelow();
                            pRubyPos = &bRubyTop;
                        }
                    }

                    pTmp = new SwRubyPortion( *pCreate, *rInf.GetFont(),
                                              *GetTxtFrm()->GetTxtNode()->getIDocumentSettingAccess(),
                                              nEnd, 0, pRubyPos );
                }
                else if( SW_MC_ROTATE == pCreate->nId )
                    pTmp = new SwRotatedPortion( *pCreate, nEnd,
                                                 GetTxtFrm()->IsRightToLeft() );
                else
                    pTmp = new SwDoubleLinePortion( *pCreate, nEnd );

                delete pCreate;
                CalcFlyWidth( rInf );

                return pTmp;
            }
        }
        
        sal_Unicode cChar = rInf.GetHookChar();

        if( cChar )
        {
            /* Wir holen uns nocheinmal cChar, um sicherzustellen, dass das
             * Tab jetzt wirklich ansteht und nicht auf die naechste Zeile
             * gewandert ist ( so geschehen hinter Rahmen ).
             * Wenn allerdings eine FldPortion im Rest wartet, muessen wir
             * das cChar natuerlich aus dem Feldinhalt holen, z.B. bei
             * DezimalTabs und Feldern (22615)
            */
            if( !rInf.GetRest() || !rInf.GetRest()->InFldGrp() )
                cChar = rInf.GetChar( rInf.GetIdx() );
            rInf.ClearHookChar();
        }
        else
        {
            if( rInf.GetIdx() >= rInf.GetTxt().getLength() )
            {
                rInf.SetFull(true);
                CalcFlyWidth( rInf );
                return pPor;
            }
            cChar = rInf.GetChar( rInf.GetIdx() );
        }

        switch( cChar )
        {
            case CH_TAB:
                pPor = NewTabPortion( rInf, false ); break;

            case CH_BREAK:
                pPor = new SwBreakPortion( *rInf.GetLast() ); break;

            case CHAR_SOFTHYPHEN:                   
                pPor = new SwSoftHyphPortion; break;

            case CHAR_HARDBLANK:                    
                pPor = new SwBlankPortion( ' ' ); break;

            case CHAR_HARDHYPHEN:               
                pPor = new SwBlankPortion( '-' ); break;

            case CHAR_ZWSP:                     
            case CHAR_ZWNBSP :                  
                pPor = new SwControlCharPortion( cChar ); break;

            case CH_TXTATR_BREAKWORD:
            case CH_TXTATR_INWORD:
                if( rInf.HasHint( rInf.GetIdx() ) )
                {
                    pPor = NewExtraPortion( rInf );
                    break;
                }
                
            default        :
                {
                SwTabPortion* pLastTabPortion = rInf.GetLastTab();
                if ( pLastTabPortion && cChar == rInf.GetTabDecimal() )
                {
                    
                    
                    
                    
                  if ( GetTxtFrm()->GetTxtNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::TAB_COMPAT) /*rInf.GetVsh()->IsTabCompat();*/ &&
                         POR_TABDECIMAL == pLastTabPortion->GetWhichPor() )
                    {
                        OSL_ENSURE( rInf.X() >= pLastTabPortion->Fix(), "Decimal tab stop position cannot be calculated" );
                        const sal_uInt16 nWidthOfPortionsUpToDecimalPosition = (sal_uInt16)(rInf.X() - pLastTabPortion->Fix() );
                        static_cast<SwTabDecimalPortion*>(pLastTabPortion)->SetWidthOfPortionsUpToDecimalPosition( nWidthOfPortionsUpToDecimalPosition );
                        rInf.SetTabDecimal( 0 );
                    }
                    else
                        rInf.SetFull( rInf.GetLastTab()->Format( rInf ) );
                }

                if( rInf.GetRest() )
                {
                    if( rInf.IsFull() )
                    {
                        rInf.SetNewLine(true);
                        return 0;
                    }
                    pPor = rInf.GetRest();
                    rInf.SetRest(0);
                }
                else
                {
                    if( rInf.IsFull() )
                        return 0;
                    pPor = NewTxtPortion( rInf );
                }
                break;
            }
        }

        
        
        if( pPor && rInf.GetRest() )
            pPor->SetLen( 0 );

        
        if( !pPor || rInf.IsStop() )
        {
            delete pPor;
            return 0;
        }
    }

    
    
    
    if ( pPor && ! pMulti )
    {
        if ( pPor->IsFtnPortion() )
        {
            const SwTxtFtn* pTxtFtn = ((SwFtnPortion*)pPor)->GetTxtFtn();

            if ( pTxtFtn )
            {
                SwFmtFtn& rFtn = (SwFmtFtn&)pTxtFtn->GetFtn();
                const SwDoc *pDoc = rInf.GetTxtFrm()->GetNode()->GetDoc();
                const SwEndNoteInfo* pInfo;
                if( rFtn.IsEndNote() )
                    pInfo = &pDoc->GetEndNoteInfo();
                else
                    pInfo = &pDoc->GetFtnInfo();
                const SwAttrSet& rSet = pInfo->GetAnchorCharFmt((SwDoc&)*pDoc)->GetAttrSet();

                const SfxPoolItem* pItem;
                sal_uInt16 nDir = 0;
                if( SFX_ITEM_SET == rSet.GetItemState( RES_CHRATR_ROTATE,
                    true, &pItem ))
                    nDir = ((SvxCharRotateItem*)pItem)->GetValue();

                if ( 0 != nDir )
                {
                    delete pPor;
                    pPor = new SwRotatedPortion( rInf.GetIdx() + 1, 900 == nDir ?
                                                    DIR_BOTTOM2TOP :
                                                    DIR_TOP2BOTTOM );
                }
            }
        }
        else if ( pPor->InNumberGrp() )
        {
            const SwFont* pNumFnt = ((SwFldPortion*)pPor)->GetFont();

            if ( pNumFnt )
            {
                sal_uInt16 nDir = pNumFnt->GetOrientation( rInf.GetTxtFrm()->IsVertical() );
                if ( 0 != nDir )
                {
                    delete pPor;
                    pPor = new SwRotatedPortion( 0, 900 == nDir ?
                                                    DIR_BOTTOM2TOP :
                                                    DIR_TOP2BOTTOM );

                    rInf.SetNumDone( false );
                    rInf.SetFtnDone( false );
                }
            }
        }
    }

    
    
    if( !pPor->GetAscent() && !pPor->Height() )
        CalcAscent( rInf, pPor );
    rInf.SetLen( pPor->GetLen() );

    
    CalcFlyWidth( rInf );

    
    
    if( !pCurr->Height() )
    {
        OSL_ENSURE( pCurr->Height(), "SwTxtFormatter::NewPortion: limbo dance" );
        pCurr->Height( pPor->Height() );
        pCurr->SetAscent( pPor->GetAscent() );
    }

    OSL_ENSURE( !pPor || pPor->Height(),
            "SwTxtFormatter::NewPortion: something went wrong");
    if( pPor->IsPostItsPortion() && rInf.X() >= rInf.Width() && rInf.GetFly() )
    {
        delete pPor;
        pPor = rInf.GetFly();
    }
    return pPor;
}

/*************************************************************************
 *                      SwTxtFormatter::FormatLine()
 *************************************************************************/

sal_Int32 SwTxtFormatter::FormatLine(const sal_Int32 nStartPos)
{
    OSL_ENSURE( ! pFrm->IsVertical() || pFrm->IsSwapped(),
            "SwTxtFormatter::FormatLine( nStartPos ) with unswapped frame" );

    
    SwHookOut aHook( GetInfo() );
    if( GetInfo().GetLen() < GetInfo().GetTxt().getLength() )
        GetInfo().SetLen( GetInfo().GetTxt().getLength() );

    bool bBuild = true;
    SetFlyInCntBase( false );
    GetInfo().SetLineHeight( 0 );
    GetInfo().SetLineNettoHeight( 0 );

    
    
    const KSHORT nOldHeight = pCurr->Height();
    const KSHORT nOldAscent = pCurr->GetAscent();

    pCurr->SetEndHyph( false );
    pCurr->SetMidHyph( false );

    
    
    SwLinePortion* pFld = GetInfo().GetRest();
    boost::scoped_ptr<SwFldPortion> xSaveFld;

    if ( pFld && pFld->InFldGrp() && !pFld->IsFtnPortion() )
        xSaveFld.reset(new SwFldPortion( *((SwFldPortion*)pFld) ));

    
    
    const bool bOptimizeRepaint = AllowRepaintOpt();
    const sal_Int32 nOldLineEnd = nStartPos + pCurr->GetLen();
    std::vector<long> flyStarts;

    
    if ( bOptimizeRepaint && pCurr->IsFly() )
    {
        SwLinePortion* pPor = pCurr->GetFirstPortion();
        long nPOfst = 0;
        while ( pPor )
        {
            if ( pPor->IsFlyPortion() )
                
                flyStarts.push_back( nPOfst );

            nPOfst += pPor->Width();
            pPor = pPor->GetPortion();
        }
    }

    
    while( bBuild )
    {
        GetInfo().SetFtnInside( false );
        GetInfo().SetOtherThanFtnInside( false );

        
        const bool bOldNumDone = GetInfo().IsNumDone();
        const bool bOldArrowDone = GetInfo().IsArrowDone();
        const bool bOldErgoDone = GetInfo().IsErgoDone();

        
        FormatReset( GetInfo() );

        GetInfo().SetNumDone( bOldNumDone );
        GetInfo().SetArrowDone( bOldArrowDone );
        GetInfo().SetErgoDone( bOldErgoDone );

        
        BuildPortions( GetInfo() );

        if( GetInfo().IsStop() )
        {
            pCurr->SetLen( 0 );
            pCurr->Height( GetFrmRstHeight() + 1 );
            pCurr->SetRealHeight( GetFrmRstHeight() + 1 );
            pCurr->Width(0);
            pCurr->Truncate();
            return nStartPos;
        }
        else if( GetInfo().IsDropInit() )
        {
            DropInit();
            GetInfo().SetDropInit( false );
        }

        pCurr->CalcLine( *this, GetInfo() );
        CalcRealHeight( GetInfo().IsNewLine() );

        
        
        SwLinePortion* pPorTmp = pCurr->GetPortion();
        if ( IsFlyInCntBase() && (!IsQuick() || (pPorTmp && pPorTmp->IsFlyCntPortion() && !pPorTmp->GetPortion() &&
            pCurr->Height() > pPorTmp->Height())))
        {
            KSHORT nTmpAscent, nTmpHeight;
            CalcAscentAndHeight( nTmpAscent, nTmpHeight );
            AlignFlyInCntBase( Y() + long( nTmpAscent ) );
            pCurr->CalcLine( *this, GetInfo() );
            CalcRealHeight();
        }

        
        if ( pCurr->GetRealHeight() <= GetInfo().GetLineHeight() )
        {
            pCurr->SetRealHeight( GetInfo().GetLineHeight() );
            bBuild = false;
        }
        else
        {
            bBuild = ( GetInfo().GetTxtFly()->IsOn() && ChkFlyUnderflow(GetInfo()) )
                     || GetInfo().CheckFtnPortion(pCurr);
            if( bBuild )
            {
                GetInfo().SetNumDone( bOldNumDone );
                GetInfo().ResetMaxWidthDiff();

                
                if ( GetInfo().GetRest() )
                {
                    delete GetInfo().GetRest();
                    GetInfo().SetRest( 0 );
                }

                
                if ( xSaveFld )
                    GetInfo().SetRest( new SwFldPortion( *xSaveFld ) );

                pCurr->SetLen( 0 );
                pCurr->Width(0);
                pCurr->Truncate();
            }
        }
    }

    
    
    
    if (GetInfo().GetTxtFrm()->GetTxtNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::TAB_OVER_MARGIN))
    {
        sal_uInt16 nSum = 0;
        SwLinePortion* pPor = pCurr->GetFirstPortion();

        while (pPor)
        {
            nSum += pPor->Width();
            pPor = pPor->GetPortion();
        }

        if (nSum > pCurr->Width())
            pCurr->Width(nSum);
    }

    
    if ( bOptimizeRepaint )
    {
        GetInfo().SetPaintOfst( ::lcl_CalcOptRepaint( *this, *pCurr, nOldLineEnd, flyStarts ) );
        flyStarts.clear();
    }
    else
        
        
        
        GetInfo().SetPaintOfst( 0 );

    
    
    
    GetInfo().GetParaPortion()->GetReformat()->LeftMove( GetInfo().GetIdx() );

    
    xSaveFld.reset();

    sal_Int32 nNewStart = nStartPos + pCurr->GetLen();

    
    if ( GetInfo().CompressLine() )
    {
        SwTwips nRepaintOfst = CalcKanaAdj( pCurr );

        
        if ( nRepaintOfst < GetInfo().GetPaintOfst() )
            GetInfo().SetPaintOfst( nRepaintOfst );
    }

    CalcAdjustLine( pCurr );

    if( nOldHeight != pCurr->Height() || nOldAscent != pCurr->GetAscent() )
    {
        SetFlyInCntBase();
        GetInfo().SetPaintOfst( 0 ); 
        
        
        GetInfo().SetShift( true );
    }

    if ( IsFlyInCntBase() && !IsQuick() )
        UpdatePos( pCurr, GetTopLeft(), GetStart() );

    return nNewStart;
}

/*************************************************************************
 *                      SwTxtFormatter::RecalcRealHeight()
 *************************************************************************/

void SwTxtFormatter::RecalcRealHeight()
{
    do
    {
        CalcRealHeight();
    } while (Next());
}

/*************************************************************************
 *                    SwTxtFormatter::CalcRealHeight()
 *************************************************************************/

void SwTxtFormatter::CalcRealHeight( bool bNewLine )
{
    KSHORT nLineHeight = pCurr->Height();
    pCurr->SetClipping( false );

    GETGRID( pFrm->FindPageFrm() )
    if ( pGrid && GetInfo().SnapToGrid() )
    {
        const sal_uInt16 nGridWidth = pGrid->GetBaseHeight();
        const sal_uInt16 nRubyHeight = pGrid->GetRubyHeight();
        const bool bRubyTop = ! pGrid->GetRubyTextBelow();

        nLineHeight = nGridWidth + nRubyHeight;
        sal_uInt16 nLineDist = nLineHeight;

        while ( pCurr->Height() > nLineHeight )
            nLineHeight = nLineHeight + nLineDist;

        KSHORT nAsc = pCurr->GetAscent() +
                      ( bRubyTop ?
                       ( nLineHeight - pCurr->Height() + nRubyHeight ) / 2 :
                       ( nLineHeight - pCurr->Height() - nRubyHeight ) / 2 );

        pCurr->Height( nLineHeight );
        pCurr->SetAscent( nAsc );
        pInf->GetParaPortion()->SetFixLineHeight();

        
        const SvxLineSpacingItem* pSpace = aLineInf.GetLineSpacing();
        if ( ! IsParaLine() && pSpace &&
             SVX_INTER_LINE_SPACE_PROP == pSpace->GetInterLineSpaceRule() )
        {
            sal_uLong nTmp = pSpace->GetPropLineSpace();

            if( nTmp < 100 )
                nTmp = 100;

            nTmp *= nLineHeight;
            nLineHeight = (sal_uInt16)(nTmp / 100);
        }

        pCurr->SetRealHeight( nLineHeight );
        return;
    }

    
    
    
    
    if( !pCurr->IsDummy() || ( !pCurr->GetNext() &&
        GetStart() >= GetTxtFrm()->GetTxt().getLength() && !bNewLine ) )
    {
        const SvxLineSpacingItem *pSpace = aLineInf.GetLineSpacing();
        if( pSpace )
        {
            switch( pSpace->GetLineSpaceRule() )
            {
                case SVX_LINE_SPACE_AUTO:
            if (pSpace->GetInterLineSpaceRule()==SVX_INTER_LINE_SPACE_PROP) {
                        long nTmp = pSpace->GetPropLineSpace();
                        if (nTmp<100) { 
                            nTmp *= nLineHeight;
                            nTmp /= 100;
                            if( !nTmp )
                                ++nTmp;
                            nLineHeight = (KSHORT)nTmp;
/*
                            
                            
                            KSHORT nAsc = ( 4 * nLineHeight ) / 5;  
                            if( nAsc < pCurr->GetAscent() ||
                                nLineHeight - nAsc < pCurr->Height() -
pCurr->GetAscent() )
                                pCurr->SetClipping( true );
                            pCurr->SetAscent( nAsc );
*/
                            pCurr->Height( nLineHeight );
                            pInf->GetParaPortion()->SetFixLineHeight();
                        }
                    }
                break;
                case SVX_LINE_SPACE_MIN:
                {
                    if( nLineHeight < KSHORT( pSpace->GetLineHeight() ) )
                        nLineHeight = pSpace->GetLineHeight();
                    break;
                }
                case SVX_LINE_SPACE_FIX:
                {
                    nLineHeight = pSpace->GetLineHeight();
                    KSHORT nAsc = ( 4 * nLineHeight ) / 5;  
                    if( nAsc < pCurr->GetAscent() ||
                        nLineHeight - nAsc < pCurr->Height() - pCurr->GetAscent() )
                        pCurr->SetClipping( true );
                    pCurr->Height( nLineHeight );
                    pCurr->SetAscent( nAsc );
                    pInf->GetParaPortion()->SetFixLineHeight();
                }
                break;
                default: OSL_FAIL( ": unknown LineSpaceRule" );
            }
            if( !IsParaLine() )
                switch( pSpace->GetInterLineSpaceRule() )
                {
                    case SVX_INTER_LINE_SPACE_OFF:
                    break;
                    case SVX_INTER_LINE_SPACE_PROP:
                    {
                        long nTmp = pSpace->GetPropLineSpace();
                        
                        
                        if( nTmp < 50 )
                            nTmp = nTmp ? 50 : 100;

                        nTmp *= nLineHeight;
                        nTmp /= 100;
                        if( !nTmp )
                            ++nTmp;
                        nLineHeight = (KSHORT)nTmp;
                        break;
                    }
                    case SVX_INTER_LINE_SPACE_FIX:
                    {
                        nLineHeight = nLineHeight + pSpace->GetInterLineSpace();
                        break;
                    }
                    default: OSL_FAIL( ": unknown InterLineSpaceRule" );
                }
        }
#if OSL_DEBUG_LEVEL > 1
        KSHORT nDummy = nLineHeight + 1;
        (void)nDummy;
#endif

        if( IsRegisterOn() )
        {
            SwTwips nTmpY = Y() + pCurr->GetAscent() + nLineHeight - pCurr->Height();
            SWRECTFN( pFrm )
            if ( bVert )
                nTmpY = pFrm->SwitchHorizontalToVertical( nTmpY );
            nTmpY = (*fnRect->fnYDiff)( nTmpY, RegStart() );
            KSHORT nDiff = KSHORT( nTmpY % RegDiff() );
            if( nDiff )
                nLineHeight += RegDiff() - nDiff;
        }
    }
    pCurr->SetRealHeight( nLineHeight );
}

/*************************************************************************
 *                      SwTxtFormatter::FeedInf()
 *************************************************************************/

void SwTxtFormatter::FeedInf( SwTxtFormatInfo &rInf ) const
{
    
    ClearFly( rInf );
    rInf.Init();

    rInf.ChkNoHyph( CntEndHyph(), CntMidHyph() );
    rInf.SetRoot( pCurr );
    rInf.SetLineStart( nStart );
    rInf.SetIdx( nStart );

    
    
    SwTwips nTmpLeft = Left();
    SwTwips nTmpRight = Right();
    SwTwips nTmpFirst = FirstLeft();

    if ( nTmpLeft > USHRT_MAX ||
         nTmpRight > USHRT_MAX ||
         nTmpFirst > USHRT_MAX )
    {
        SWRECTFN( rInf.GetTxtFrm() )
        nTmpLeft = (rInf.GetTxtFrm()->Frm().*fnRect->fnGetLeft)();
        nTmpRight = (rInf.GetTxtFrm()->Frm().*fnRect->fnGetRight)();
        nTmpFirst = nTmpLeft;
    }

    rInf.Left(  nTmpLeft  );
    rInf.Right( nTmpRight );
    rInf.First( nTmpFirst );

    rInf.RealWidth( KSHORT(rInf.Right()) - KSHORT(GetLeftMargin()) );
    rInf.Width( rInf.RealWidth() );
    if( ((SwTxtFormatter*)this)->GetRedln() )
    {
        ((SwTxtFormatter*)this)->GetRedln()->Clear( ((SwTxtFormatter*)this)->GetFnt() );
        ((SwTxtFormatter*)this)->GetRedln()->Reset();
    }
}

/*************************************************************************
 *                      SwTxtFormatter::FormatReset()
 *************************************************************************/

void SwTxtFormatter::FormatReset( SwTxtFormatInfo &rInf )
{
    pCurr->Truncate();
    pCurr->Init();
    if( pBlink && pCurr->IsBlinking() )
        pBlink->Delete( pCurr );

    
    pCurr->FinishSpaceAdd();
    pCurr->FinishKanaComp();
    pCurr->ResetFlags();
    FeedInf( rInf );
}

/*************************************************************************
 *                SwTxtFormatter::CalcOnceMore()
 *************************************************************************/

bool SwTxtFormatter::CalcOnceMore()
{
    if( pDropFmt )
    {
        const KSHORT nOldDrop = GetDropHeight();
        CalcDropHeight( pDropFmt->GetLines() );
        bOnceMore = nOldDrop != GetDropHeight();
    }
    else
        bOnceMore = false;
    return bOnceMore;
}

/*************************************************************************
 *                SwTxtFormatter::CalcBottomLine()
 *************************************************************************/

SwTwips SwTxtFormatter::CalcBottomLine() const
{
    SwTwips nRet = Y() + GetLineHeight();
    SwTwips nMin = GetInfo().GetTxtFly()->GetMinBottom();
    if( nMin && ++nMin > nRet )
    {
        SwTwips nDist = pFrm->Frm().Height() - pFrm->Prt().Height()
                        - pFrm->Prt().Top();
        if( nRet + nDist < nMin )
        {
            const bool bRepaint = HasTruncLines() &&
                GetInfo().GetParaPortion()->GetRepaint()->Bottom() == nRet-1;
            nRet = nMin - nDist;
            if( bRepaint )
            {
                ((SwRepaint*)GetInfo().GetParaPortion()
                    ->GetRepaint())->Bottom( nRet-1 );
                ((SwTxtFormatInfo&)GetInfo()).SetPaintOfst( 0 );
            }
        }
    }
    return nRet;
}

/*************************************************************************
 *                SwTxtFormatter::_CalcFitToContent()
 *
 * FME/OD: This routine does a limited text formatting.
 *************************************************************************/

SwTwips SwTxtFormatter::_CalcFitToContent()
{
    FormatReset( GetInfo() );
    BuildPortions( GetInfo() );
    pCurr->CalcLine( *this, GetInfo() );
    return pCurr->Width();
}

/*************************************************************************
 *                      SwTxtFormatter::AllowRepaintOpt()
 *
 * determines if the calculation of a repaint offset is allowed
 * otherwise each line is painted from 0 (this is a copy of the beginning
 * of the former SwTxtFormatter::Recycle() function
 *************************************************************************/
bool SwTxtFormatter::AllowRepaintOpt() const
{
    
    
    bool bOptimizeRepaint = nStart < GetInfo().GetReformatStart() &&
                                pCurr->GetLen();

    
    if ( bOptimizeRepaint )
    {
        switch( GetAdjust() )
        {
        case SVX_ADJUST_BLOCK:
        {
            if( IsLastBlock() || IsLastCenter() )
                bOptimizeRepaint = false;
            else
            {
                
                bOptimizeRepaint = 0 == pCurr->GetNext() && !pFrm->GetFollow();
                if ( bOptimizeRepaint )
                {
                    SwLinePortion *pPos = pCurr->GetFirstPortion();
                    while ( pPos && !pPos->IsFlyPortion() )
                        pPos = pPos->GetPortion();
                    bOptimizeRepaint = !pPos;
                }
            }
            break;
        }
        case SVX_ADJUST_CENTER:
        case SVX_ADJUST_RIGHT:
            bOptimizeRepaint = false;
            break;
        default: ;
        }
    }

    
    const sal_Int32 nReformat = GetInfo().GetReformatStart();
    if( bOptimizeRepaint && COMPLETE_STRING != nReformat )
    {
        const sal_Unicode cCh = nReformat >= GetInfo().GetTxt().getLength() ? 0 : GetInfo().GetTxt()[ nReformat ];
        bOptimizeRepaint = ( CH_TXTATR_BREAKWORD != cCh && CH_TXTATR_INWORD != cCh )
                            || ! GetInfo().HasHint( nReformat );
    }

    return bOptimizeRepaint;
}

void SwTxtFormatter::CalcUnclipped( SwTwips& rTop, SwTwips& rBottom )
{
    OSL_ENSURE( ! pFrm->IsVertical() || pFrm->IsSwapped(),
            "SwTxtFormatter::CalcUnclipped with unswapped frame" );

    long nFlyAsc, nFlyDesc;
    pCurr->MaxAscentDescent( rTop, rBottom, nFlyAsc, nFlyDesc );
    rTop = Y() + GetCurr()->GetAscent();
    rBottom = rTop + nFlyDesc;
    rTop -= nFlyAsc;
}


void SwTxtFormatter::UpdatePos( SwLineLayout *pCurrent, Point aStart,
    sal_Int32 nStartIdx, bool bAlways ) const
{
    OSL_ENSURE( ! pFrm->IsVertical() || pFrm->IsSwapped(),
            "SwTxtFormatter::UpdatePos with unswapped frame" );

    if( GetInfo().IsTest() )
        return;
    SwLinePortion *pFirst = pCurrent->GetFirstPortion();
    SwLinePortion *pPos = pFirst;
    SwTxtPaintInfo aTmpInf( GetInfo() );
    aTmpInf.SetpSpaceAdd( pCurrent->GetpLLSpaceAdd() );
    aTmpInf.ResetSpaceIdx();
    aTmpInf.SetKanaComp( pCurrent->GetpKanaComp() );
    aTmpInf.ResetKanaIdx();

    
    aTmpInf.SetIdx( nStartIdx );
    aTmpInf.SetPos( aStart );

    long nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc;
    pCurrent->MaxAscentDescent( nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc );

    KSHORT nTmpHeight = pCurrent->GetRealHeight();
    KSHORT nAscent = pCurrent->GetAscent() + nTmpHeight - pCurrent->Height();
    objectpositioning::AsCharFlags nFlags = AS_CHAR_ULSPACE;
    if( GetMulti() )
    {
        aTmpInf.SetDirection( GetMulti()->GetDirection() );
        if( GetMulti()->HasRotation() )
        {
            nFlags |= AS_CHAR_ROTATE;
            if( GetMulti()->IsRevers() )
            {
                nFlags |= AS_CHAR_REVERSE;
                aTmpInf.X( aTmpInf.X() - nAscent );
            }
            else
                aTmpInf.X( aTmpInf.X() + nAscent );
        }
        else
        {
            if ( GetMulti()->IsBidi() )
                nFlags |= AS_CHAR_BIDI;
            aTmpInf.Y( aTmpInf.Y() + nAscent );
        }
    }
    else
        aTmpInf.Y( aTmpInf.Y() + nAscent );

    while( pPos )
    {
        
        
        
        if( ( pPos->IsFlyCntPortion() || pPos->IsGrfNumPortion() )
            && ( bAlways || !IsQuick() ) )
        {
            pCurrent->MaxAscentDescent( nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc, pPos );

            if( pPos->IsGrfNumPortion() )
            {
                if( !nFlyAsc && !nFlyDesc )
                {
                    nTmpAscent = nAscent;
                    nFlyAsc = nAscent;
                    nTmpDescent = nTmpHeight - nAscent;
                    nFlyDesc = nTmpDescent;
                }
                ((SwGrfNumPortion*)pPos)->SetBase( nTmpAscent, nTmpDescent,
                                                   nFlyAsc, nFlyDesc );
            }
            else
            {
                Point aBase( aTmpInf.GetPos() );
                if ( GetInfo().GetTxtFrm()->IsVertical() )
                    GetInfo().GetTxtFrm()->SwitchHorizontalToVertical( aBase );

                ((SwFlyCntPortion*)pPos)->SetBase( *aTmpInf.GetTxtFrm(),
                    aBase, nTmpAscent, nTmpDescent, nFlyAsc,
                    nFlyDesc, nFlags );
            }
        }
        if( pPos->IsMultiPortion() && ((SwMultiPortion*)pPos)->HasFlyInCntnt() )
        {
            OSL_ENSURE( !GetMulti(), "Too much multi" );
            ((SwTxtFormatter*)this)->pMulti = (SwMultiPortion*)pPos;
            SwLineLayout *pLay = &GetMulti()->GetRoot();
            Point aSt( aTmpInf.X(), aStart.Y() );

            if ( GetMulti()->HasBrackets() )
            {
                OSL_ENSURE( GetMulti()->IsDouble(), "Brackets only for doubles");
                aSt.X() += ((SwDoubleLinePortion*)GetMulti())->PreWidth();
            }
            else if( GetMulti()->HasRotation() )
            {
                aSt.Y() += pCurrent->GetAscent() - GetMulti()->GetAscent();
                if( GetMulti()->IsRevers() )
                    aSt.X() += GetMulti()->Width();
                else
                    aSt.Y() += GetMulti()->Height();
               }
            else if ( GetMulti()->IsBidi() )
                
                aSt.X() += pLay->Width();

            sal_Int32 nStIdx = aTmpInf.GetIdx();
            do
            {
                UpdatePos( pLay, aSt, nStIdx, bAlways );
                nStIdx = nStIdx + pLay->GetLen();
                aSt.Y() += pLay->Height();
                pLay = pLay->GetNext();
            } while ( pLay );
            ((SwTxtFormatter*)this)->pMulti = NULL;
        }
        pPos->Move( aTmpInf );
        pPos = pPos->GetPortion();
    }
}


void SwTxtFormatter::AlignFlyInCntBase( long nBaseLine ) const
{
    OSL_ENSURE( ! pFrm->IsVertical() || pFrm->IsSwapped(),
            "SwTxtFormatter::AlignFlyInCntBase with unswapped frame" );

    if( GetInfo().IsTest() )
        return;
    SwLinePortion *pFirst = pCurr->GetFirstPortion();
    SwLinePortion *pPos = pFirst;
    objectpositioning::AsCharFlags nFlags = AS_CHAR_NOFLAG;
    if( GetMulti() && GetMulti()->HasRotation() )
    {
        nFlags |= AS_CHAR_ROTATE;
        if( GetMulti()->IsRevers() )
            nFlags |= AS_CHAR_REVERSE;
    }

    long nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc;

    while( pPos )
    {
        if( pPos->IsFlyCntPortion() || pPos->IsGrfNumPortion() )
        {
            pCurr->MaxAscentDescent( nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc, pPos );

            if( pPos->IsGrfNumPortion() )
                ((SwGrfNumPortion*)pPos)->SetBase( nTmpAscent, nTmpDescent,
                                                   nFlyAsc, nFlyDesc );
            else
            {
                Point aBase;
                if ( GetInfo().GetTxtFrm()->IsVertical() )
                {
                    nBaseLine = GetInfo().GetTxtFrm()->SwitchHorizontalToVertical( nBaseLine );
                    aBase = Point( nBaseLine, ((SwFlyCntPortion*)pPos)->GetRefPoint().Y() );
                }
                else
                    aBase = Point( ((SwFlyCntPortion*)pPos)->GetRefPoint().X(), nBaseLine );

                ((SwFlyCntPortion*)pPos)->SetBase( *GetInfo().GetTxtFrm(), aBase, nTmpAscent, nTmpDescent,
                    nFlyAsc, nFlyDesc, nFlags );
            }
        }
        pPos = pPos->GetPortion();
    }
}


bool SwTxtFormatter::ChkFlyUnderflow( SwTxtFormatInfo &rInf ) const
{
    OSL_ENSURE( rInf.GetTxtFly()->IsOn(), "SwTxtFormatter::ChkFlyUnderflow: why?" );
    if( GetCurr() )
    {
        
        
        const long nHeight = GetCurr()->GetRealHeight();
        SwRect aLine( GetLeftMargin(), Y(), rInf.RealWidth(), nHeight );

        SwRect aLineVert( aLine );
        if ( pFrm->IsVertical() )
            pFrm->SwitchHorizontalToVertical( aLineVert );
        SwRect aInter( rInf.GetTxtFly()->GetFrm( aLineVert ) );
        if ( pFrm->IsVertical() )
            pFrm->SwitchVerticalToHorizontal( aInter );

        if( !aInter.HasArea() )
            return false;

        
        
        const SwLinePortion *pPos = GetCurr()->GetFirstPortion();
        aLine.Pos().Y() = Y() + GetCurr()->GetRealHeight() - GetCurr()->Height();
        aLine.Height( GetCurr()->Height() );

        while( pPos )
        {
            aLine.Width( pPos->Width() );

            aLineVert = aLine;
            if ( pFrm->IsVertical() )
                pFrm->SwitchHorizontalToVertical( aLineVert );
            aInter = rInf.GetTxtFly()->GetFrm( aLineVert );
            if ( pFrm->IsVertical() )
                pFrm->SwitchVerticalToHorizontal( aInter );

            
            if( !pPos->IsFlyPortion() )
            {
                if( aInter.IsOver( aLine ) )
                {
                    aInter._Intersection( aLine );
                    if( aInter.HasArea() )
                    {
                        
                        
                        rInf.SetLineHeight( KSHORT(nHeight) );
                        
                        rInf.SetLineNettoHeight( KSHORT( pCurr->Height() ) );
                        return true;
                    }
                }
            }
            else
            {
                
                if ( ! aInter.IsOver( aLine ) )
                {
                    rInf.SetLineHeight( KSHORT(nHeight) );
                    rInf.SetLineNettoHeight( KSHORT( pCurr->Height() ) );
                    return true;
                }
                else
                {
                    aInter._Intersection( aLine );

                    
                    
                    
                    
                    if( ! aInter.HasArea() ||
                        ((SwFlyPortion*)pPos)->GetFixWidth() != aInter.Width() )
                    {
                        rInf.SetLineHeight( KSHORT(nHeight) );
                        rInf.SetLineNettoHeight( KSHORT( pCurr->Height() ) );
                        return true;
                    }
                }
            }

            aLine.Left( aLine.Left() + pPos->Width() );
            pPos = pPos->GetPortion();
        }
    }
    return false;
}

void SwTxtFormatter::CalcFlyWidth( SwTxtFormatInfo &rInf )
{
    if( GetMulti() || rInf.GetFly() )
        return;

    SwTxtFly *pTxtFly = rInf.GetTxtFly();
    if( !pTxtFly->IsOn() || rInf.IsIgnoreFly() )
        return;

    const SwLinePortion *pLast = rInf.GetLast();

    long nAscent;
    long nTop = Y();
    long nHeight;

    if( rInf.GetLineHeight() )
    {
        
        
        nAscent = pCurr->GetAscent();
        nHeight = rInf.GetLineNettoHeight();
        nTop += rInf.GetLineHeight() - nHeight;
    }
    else
    {
        nAscent = pLast->GetAscent();
        nHeight = pLast->Height();

        
        if ( ! pCurr->GetRealHeight() )
            CalcRealHeight();

        if ( pCurr->GetRealHeight() > nHeight )
            nTop += pCurr->GetRealHeight() - nHeight;
        else
            
            nHeight = pCurr->GetRealHeight();
    }

    const long nLeftMar = GetLeftMargin();
    const long nLeftMin = (rInf.X() || GetDropLeft()) ? nLeftMar : GetLeftMin();

    SwRect aLine( rInf.X() + nLeftMin, nTop, rInf.RealWidth() - rInf.X()
                  + nLeftMar - nLeftMin , nHeight );

    SwRect aLineVert( aLine );
    if ( pFrm->IsRightToLeft() )
        pFrm->SwitchLTRtoRTL( aLineVert );

    if ( pFrm->IsVertical() )
        pFrm->SwitchHorizontalToVertical( aLineVert );
    SwRect aInter( pTxtFly->GetFrm( aLineVert ) );

    if ( pFrm->IsRightToLeft() )
        pFrm->SwitchRTLtoLTR( aInter );

    if ( pFrm->IsVertical() )
        pFrm->SwitchVerticalToHorizontal( aInter );

    if( aInter.IsOver( aLine ) )
    {
        aLine.Left( rInf.X() + nLeftMar );
        bool bForced = false;
        if( aInter.Left() <= nLeftMin )
        {
            SwTwips nFrmLeft = GetTxtFrm()->Frm().Left();
            if( GetTxtFrm()->Prt().Left() < 0 )
                nFrmLeft += GetTxtFrm()->Prt().Left();
            if( aInter.Left() < nFrmLeft )
                aInter.Left( nFrmLeft );

            long nAddMar = 0;
            if ( pFrm->IsRightToLeft() )
            {
                nAddMar = pFrm->Frm().Right() - Right();
                if ( nAddMar < 0 )
                    nAddMar = 0;
            }
            else
                nAddMar = nLeftMar - nFrmLeft;

            aInter.Width( aInter.Width() + nAddMar );
            
            
            
            if( IsFirstTxtLine() && HasNegFirst() )
                bForced = true;
        }
        aInter.Intersection( aLine );
        if( !aInter.HasArea() )
            return;

        const bool bFullLine =  aLine.Left()  == aInter.Left() &&
                                aLine.Right() == aInter.Right();

        
        
        if( bFullLine && rInf.GetIdx() == rInf.GetTxt().getLength() )
        {
            rInf.SetNewLine( true );
            
            pCurr->SetDummy(true);
        }

        
        aInter.Pos().X() -= nLeftMar;
        SwFlyPortion *pFly = new SwFlyPortion( aInter );
        if( bForced )
        {
            pCurr->SetForcedLeftMargin( true );
            rInf.ForcedLeftMargin( (sal_uInt16)aInter.Width() );
        }

        if( bFullLine )
        {
            
            
            
            
            
            
            
            
            
            
            
            pFly->Height( KSHORT(aInter.Height()) );

            
            
            
            
            long nNextTop = pTxtFly->GetNextTop();
            if ( pFrm->IsVertical() )
                nNextTop = pFrm->SwitchVerticalToHorizontal( nNextTop );
            if( nNextTop > aInter.Bottom() )
            {
                SwTwips nH = nNextTop - aInter.Top();
                if( nH < KSHRT_MAX )
                    pFly->Height( KSHORT( nH ) );
            }
            if( nAscent < pFly->Height() )
                pFly->SetAscent( KSHORT(nAscent) );
            else
                pFly->SetAscent( pFly->Height() );
        }
        else
        {
            if( rInf.GetIdx() == rInf.GetTxt().getLength() )
            {
                
                pFly->Height( pLast->Height() );
                pFly->SetAscent( pLast->GetAscent() );
            }
            else
            {
                pFly->Height( KSHORT(aInter.Height()) );
                if( nAscent < pFly->Height() )
                    pFly->SetAscent( KSHORT(nAscent) );
                else
                    pFly->SetAscent( pFly->Height() );
            }
        }

        rInf.SetFly( pFly );

        if( pFly->Fix() < rInf.Width() )
            rInf.Width( pFly->Fix() );

        GETGRID( pFrm->FindPageFrm() )
        if ( pGrid )
        {
            const SwPageFrm* pPageFrm = pFrm->FindPageFrm();
            const SwLayoutFrm* pBody = pPageFrm->FindBodyCont();

            SWRECTFN( pPageFrm )

            const long nGridOrigin = pBody ?
                                    (pBody->*fnRect->fnGetPrtLeft)() :
                                    (pPageFrm->*fnRect->fnGetPrtLeft)();

            const SwDoc *pDoc = rInf.GetTxtFrm()->GetNode()->GetDoc();
            const sal_uInt16 nGridWidth = GETGRIDWIDTH( pGrid, pDoc);   

            SwTwips nStartX = GetLeftMargin();
            if ( bVert )
            {
                Point aPoint( nStartX, 0 );
                pFrm->SwitchHorizontalToVertical( aPoint );
                nStartX = aPoint.Y();
            }

            const SwTwips nOfst = nStartX - nGridOrigin;
            const SwTwips nTmpWidth = rInf.Width() + nOfst;

            const sal_uLong i = nTmpWidth / nGridWidth + 1;

            const long nNewWidth = ( i - 1 ) * nGridWidth - nOfst;
            if ( nNewWidth > 0 )
                rInf.Width( (sal_uInt16)nNewWidth );
            else
                rInf.Width( 0 );
        }
    }
}


SwFlyCntPortion *SwTxtFormatter::NewFlyCntPortion( SwTxtFormatInfo &rInf,
                                                   SwTxtAttr *pHint ) const
{
    SwFlyCntPortion *pRet = 0;
    const SwFrm *pFrame = (SwFrm*)pFrm;

    SwFlyInCntFrm *pFly;
    SwFrmFmt* pFrmFmt = ((SwTxtFlyCnt*)pHint)->GetFlyCnt().GetFrmFmt();
    if( RES_FLYFRMFMT == pFrmFmt->Which() )
        pFly = ((SwTxtFlyCnt*)pHint)->GetFlyFrm(pFrame);
    else
        pFly = NULL;
    
    
    

    long nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc;
    
    
    
    pCurr->MaxAscentDescent( nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc );

    
    
    
    
    KSHORT nAscent = 0;

    const bool bTxtFrmVertical = GetInfo().GetTxtFrm()->IsVertical();

    const bool bUseFlyAscent = pFly && pFly->GetValidPosFlag() &&
                               0 != ( bTxtFrmVertical ?
                                      pFly->GetRefPoint().X() :
                                      pFly->GetRefPoint().Y() );

    if ( bUseFlyAscent )
         nAscent = static_cast<sal_uInt16>( std::abs( int( bTxtFrmVertical ?
                                                  pFly->GetRelPos().X() :
                                                  pFly->GetRelPos().Y() ) ) );

    
    if ( IsQuick() ||
         !bUseFlyAscent ||
         nAscent < rInf.GetLast()->GetAscent() )
    {
        nAscent = rInf.GetLast()->GetAscent();
    }
    else if( nAscent > nFlyAsc )
        nFlyAsc = nAscent;

    Point aBase( GetLeftMargin() + rInf.X(), Y() + nAscent );
    objectpositioning::AsCharFlags nMode = IsQuick() ? AS_CHAR_QUICK : 0;
    if( GetMulti() && GetMulti()->HasRotation() )
    {
        nMode |= AS_CHAR_ROTATE;
        if( GetMulti()->IsRevers() )
            nMode |= AS_CHAR_REVERSE;
    }

    Point aTmpBase( aBase );
    if ( GetInfo().GetTxtFrm()->IsVertical() )
        GetInfo().GetTxtFrm()->SwitchHorizontalToVertical( aTmpBase );

    if( pFly )
    {
        pRet = new SwFlyCntPortion( *GetInfo().GetTxtFrm(), pFly, aTmpBase,
                                    nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc, nMode );
        
        
        
        
        
        rInf.SelectFont();
        if( pRet->GetAscent() > nAscent )
        {
            aBase.Y() = Y() + pRet->GetAscent();
            nMode |= AS_CHAR_ULSPACE;
            if( !rInf.IsTest() )
            {
                aTmpBase = aBase;
                if ( GetInfo().GetTxtFrm()->IsVertical() )
                    GetInfo().GetTxtFrm()->SwitchHorizontalToVertical( aTmpBase );

                pRet->SetBase( *rInf.GetTxtFrm(), aTmpBase, nTmpAscent,
                               nTmpDescent, nFlyAsc, nFlyDesc, nMode );
            }
        }
    }
    else
    {
        pRet = new SwFlyCntPortion( *rInf.GetTxtFrm(), (SwDrawContact*)pFrmFmt->FindContactObj(),
           aTmpBase, nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc, nMode );
    }
    return pRet;
}

/* Drop portion is a special case, because it has parts which aren't portions
   but we have handle them just like portions */
void SwTxtFormatter::MergeCharacterBorder( SwDropPortion& rPortion )
{
    if( rPortion.GetLines() > 1 )
    {
        SwDropPortionPart* pCurrPart = rPortion.GetPart();
        while( pCurrPart )
        {
            if( pCurrPart->GetFollow() &&
                ::lcl_HasSameBorder(pCurrPart->GetFont(), pCurrPart->GetFollow()->GetFont()) )
            {
                pCurrPart->SetJoinBorderWithNext(true);
                pCurrPart->GetFollow()->SetJoinBorderWithPrev(true);
            }
            pCurrPart = pCurrPart->GetFollow();
        }
    }
}

void SwTxtFormatter::MergeCharacterBorder( SwLinePortion& rPortion, SwTxtFormatInfo& rInf )
{
    const SwFont aCurFont = *rInf.GetFont();
    if( aCurFont.HasBorder() )
    {
        
        
        if( rInf.GetLast() && rInf.GetLast() != &rPortion && 
            rInf.GetLast()->GetJoinBorderWithNext() )
        {
            
            if( !rPortion.GetJoinBorderWithPrev() )
            {
                rPortion.SetJoinBorderWithPrev(true);
                if( rPortion.InTxtGrp() && rPortion.Width() > aCurFont.GetLeftBorderSpace() )
                    rPortion.Width(rPortion.Width() - aCurFont.GetLeftBorderSpace());
            }
        }
        else
        {
            rPortion.SetJoinBorderWithPrev(false);
            m_pFirstOfBorderMerge = &rPortion;
        }

        
        bool bSeek = false;
        if( !rInf.IsFull() && 
            rInf.GetIdx() + rPortion.GetLen() != rInf.GetTxt().getLength() ) 
            bSeek = Seek(rInf.GetIdx() + rPortion.GetLen());

        
        if( bSeek && GetFnt()->HasBorder() && ::lcl_HasSameBorder(aCurFont, *GetFnt()) )
        {
            
            if( !rPortion.GetJoinBorderWithNext() )
            {
                rPortion.SetJoinBorderWithNext(true);
                if( rPortion.InTxtGrp() && rPortion.Width() > aCurFont.GetRightBorderSpace() )
                    rPortion.Width(rPortion.Width() - aCurFont.GetRightBorderSpace());
            }
        }
        
        else
        {
            rPortion.SetJoinBorderWithNext(false);
            if( m_pFirstOfBorderMerge != &rPortion )
            {
                
                SwLinePortion* pActPor = m_pFirstOfBorderMerge;
                sal_uInt16 nMaxAscent = 0;
                sal_uInt16 nMaxHeight = 0;
                bool bReachCurrent = false;
                while( pActPor )
                {
                    if( nMaxHeight < pActPor->Height() )
                        nMaxHeight = pActPor->Height();
                    if( nMaxAscent < pActPor->GetAscent() )
                        nMaxAscent = pActPor->GetAscent();

                    pActPor = pActPor->GetPortion();
                    if( !pActPor && !bReachCurrent )
                    {
                        pActPor = &rPortion;
                        bReachCurrent = true;
                    }
                }

                
                pActPor = m_pFirstOfBorderMerge;
                bReachCurrent = false;
                while( pActPor )
                {
                    if( nMaxHeight > pActPor->Height() )
                        pActPor->Height(nMaxHeight);
                    if( nMaxAscent > pActPor->GetAscent() )
                        pActPor->SetAscent(nMaxAscent);

                    pActPor = pActPor->GetPortion();
                    if( !pActPor && !bReachCurrent )
                    {
                        pActPor = &rPortion;
                        bReachCurrent = true;
                    }
                }
                m_pFirstOfBorderMerge = 0;
            }
        }
        Seek(rInf.GetIdx());
    }
}

namespace {
    /*************************************************************************
    *                      ::CalcOptRepaint()
    *
    * calculates and sets optimal repaint offset for the current line
    *************************************************************************/
    long lcl_CalcOptRepaint( SwTxtFormatter &rThis,
                         SwLineLayout &rCurr,
                         const sal_Int32 nOldLineEnd,
                         const std::vector<long> &rFlyStarts )
    {
        SwTxtFormatInfo txtFmtInfo = rThis.GetInfo();
        if ( txtFmtInfo.GetIdx() < txtFmtInfo.GetReformatStart() )
        
        
            return 0;

        sal_Int32 nReformat = std::min<sal_Int32>( txtFmtInfo.GetReformatStart(), nOldLineEnd );

        
        
        if ( rFlyStarts.empty() && ! rCurr.IsFly() )
        {
            
            
            
            
            const long nFormatRepaint = txtFmtInfo.GetPaintOfst();

            if ( nReformat < txtFmtInfo.GetLineStart() + 3 )
                return 0;

            
            nReformat -= 2;

    #ifndef MACOSX
    #if ! ENABLE_GRAPHITE
            
            
            
            const SwScriptInfo& rSI = txtFmtInfo.GetParaPortion()->GetScriptInfo();
            sal_Int32 nMaxContext = 0;
            if( ::i18n::ScriptType::COMPLEX == rSI.ScriptType( nReformat ) )
                nMaxContext = 6;
    #else
            
            static const sal_Int32 nMaxContext = 10;
    #endif
    #else
            
            
            static const sal_Int32 nMaxContext = 8;
    #endif
            if( nMaxContext > 0 )
            {
                if ( nReformat > txtFmtInfo.GetLineStart() + nMaxContext )
                    nReformat = nReformat - nMaxContext;
                else
                    nReformat = txtFmtInfo.GetLineStart();
            }

            
            
            
            
            while ( nReformat > txtFmtInfo.GetLineStart() &&
                    CH_BLANK == txtFmtInfo.GetChar( nReformat ) )
                --nReformat;

            OSL_ENSURE( nReformat < txtFmtInfo.GetIdx(), "Reformat too small for me!" );
            SwRect aRect;

            
            
            bool bOldMulti = txtFmtInfo.IsMulti();
            rThis.GetCharRect( &aRect, nReformat );
            txtFmtInfo.SetMulti( bOldMulti );

            return nFormatRepaint ? std::min( aRect.Left(), nFormatRepaint ) :
                                    aRect.Left();
        }
        else
        {
            
            
            
            
            long nPOfst = 0;
            sal_uInt16 nCnt = 0;
            sal_uInt16 nX = 0;
            sal_uInt16 nIdx = rThis.GetInfo().GetLineStart();
            SwLinePortion* pPor = rCurr.GetFirstPortion();

            while ( pPor )
            {
                if ( pPor->IsFlyPortion() )
                {
                    
                    if (nCnt < rFlyStarts.size() &&
                        nX == rFlyStarts[ nCnt ] &&
                        nIdx < nReformat
                    )
                        
                        nPOfst = nX + pPor->Width();
                    else
                        break;

                    nCnt++;
                }
                nX = nX + pPor->Width();
                nIdx = nIdx + pPor->GetLen();
                pPor = pPor->GetPortion();
            }

            return nPOfst + rThis.GetLeftMargin();
        }
    }

    
    bool lcl_BuildHiddenPortion( const SwTxtSizeInfo& rInf, sal_Int32 &rPos )
    {
        
    
        const bool bShowInDocView = rInf.GetVsh() && rInf.GetVsh()->GetWin() && rInf.GetOpt().IsShowHiddenChar();
        const bool bShowForPrinting = rInf.GetOpt().IsShowHiddenChar( true ) && rInf.GetOpt().IsPrinting();
        if (bShowInDocView || bShowForPrinting)
            return false;

        const SwScriptInfo& rSI = rInf.GetParaPortion()->GetScriptInfo();
        sal_Int32 nHiddenStart;
        sal_Int32 nHiddenEnd;
        rSI.GetBoundsOfHiddenRange( rPos, nHiddenStart, nHiddenEnd );
        if ( nHiddenEnd )
        {
            rPos = nHiddenEnd;
            return true;
        }

        return false;
    }

    bool lcl_HasSameBorder(const SwFont& rFirst, const SwFont& rSecond)
    {
        return
            rFirst.GetTopBorder() == rSecond.GetTopBorder() &&
            rFirst.GetBottomBorder() == rSecond.GetBottomBorder() &&
            rFirst.GetLeftBorder() == rSecond.GetLeftBorder() &&
            rFirst.GetRightBorder() == rSecond.GetRightBorder() &&
            rFirst.GetTopBorderDist() == rSecond.GetTopBorderDist() &&
            rFirst.GetBottomBorderDist() == rSecond.GetBottomBorderDist() &&
            rFirst.GetLeftBorderDist() == rSecond.GetLeftBorderDist() &&
            rFirst.GetRightBorderDist() == rSecond.GetRightBorderDist() &&
            rFirst.GetOrientation() == rSecond.GetOrientation() &&
            rFirst.GetShadowColor() == rSecond.GetShadowColor() &&
            rFirst.GetShadowWidth() == rSecond.GetShadowWidth() &&
            rFirst.GetShadowLocation() == rSecond.GetShadowLocation();
    }

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
