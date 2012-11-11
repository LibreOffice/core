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


#include "hintids.hxx"

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
#include <layfrm.hxx>       // GetFrmRstHeight, etc
#include <viewsh.hxx>
#include <viewopt.hxx>      // SwViewOptions
#include <paratr.hxx>       // SwFmtDrop
#include <itrform2.hxx>
#include <porrst.hxx>
#include <portab.hxx>       // pLastTab->
#include <porfly.hxx>       // CalcFlyWidth
#include <portox.hxx>       // WhichTxtPortion
#include <porref.hxx>       // WhichTxtPortion
#include <porfld.hxx>       // SwNumberPortion for CalcAscent()
#include <porftn.hxx>       // SwFtnPortion
#include <porhyph.hxx>
#include <guess.hxx>
#include <blink.hxx>        // pBlink
#include <ftnfrm.hxx>       // WhichFirstPortion() -> move it
#include <redlnitr.hxx>     // SwRedlineItr
#include <pagefrm.hxx>
#include <pagedesc.hxx>     // SwPageDesc
#include <tgrditem.hxx>
#include <doc.hxx>          // SwDoc
#include <pormulti.hxx>     // SwMultiPortion
#include <unotools/charclass.hxx>

#include <vector>

#if OSL_DEBUG_LEVEL > 1
#include <ndtxt.hxx>        // pSwpHints, output operator
#endif

using namespace ::com::sun::star;

extern sal_Bool IsUnderlineBreak( const SwLinePortion& rPor, const SwFont& rFnt );

namespace {
    //! Calculates and sets optimal repaint offset for the current line
    static long lcl_CalcOptRepaint( SwTxtFormatter &rThis,
                                    SwLineLayout &rCurr,
                                    const xub_StrLen nOldLineEnd,
                                    const std::vector<long> &rFlyStarts );
    //! Determine if we need to build hidden portions
    static bool lcl_BuildHiddenPortion( const SwTxtSizeInfo& rInf, xub_StrLen &rPos );
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

    bOnceMore = sal_False;
    bFlyInCntBase = sal_False;
    bChanges = sal_False;
    bTruncLines = sal_False;
    nCntEndHyph = 0;
    nCntMidHyph = 0;
    nLeftScanIdx = STRING_LEN;
    nRightScanIdx = 0;
    m_nHintEndIndex = 0;

    if( nStart > GetInfo().GetTxt().Len() )
    {
        OSL_ENSURE( !this, "+SwTxtFormatter::CTOR: bad offset" );
        nStart = GetInfo().GetTxt().Len();
    }

}

/*************************************************************************
 *                      SwTxtFormatter::DTOR
 *************************************************************************/

SwTxtFormatter::~SwTxtFormatter()
{
    // Extremly unlikely, but still possible
    // e.g.: field splits up, widows start to matter
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
    // Insert BEHIND the current element
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
    // We want the rest height relative to the page.
    // If we're in a table, then pFrm->GetUpper() is not the page.
    //
    // GetFrmRstHeight() is being called with Ftn.
    // Wrong: const SwFrm *pUpper = pFrm->GetUpper();
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
    // Save values and initialize rInf
    SwLinePortion *pUnderFlow = rInf.GetUnderFlow();
    if( !pUnderFlow )
        return 0;

    // We format backwards, i.e. attribute changes can happen the next
    // line again.
    // Can be seen in 8081.sdw, if you enter text in the first line

    const xub_StrLen nSoftHyphPos = rInf.GetSoftHyphPos();
    const xub_StrLen nUnderScorePos = rInf.GetUnderScorePos();

    // Save flys and set to 0, or else segmentation fault
    // Not ClearFly(rInf) !
    SwFlyPortion *pFly = rInf.GetFly();
    rInf.SetFly( 0 );

    FeedInf( rInf );
    rInf.SetLast( pCurr );
    // pUnderFlow does not need to be deleted, because it will drown in the following
    // Truncate()
    rInf.SetUnderFlow(0);
    rInf.SetSoftHyphPos( nSoftHyphPos );
    rInf.SetUnderScorePos( nUnderScorePos );
    rInf.SetPaintOfst( GetLeftMargin() );

    // We look for the portion with the under-flow position
    SwLinePortion *pPor = pCurr->GetFirstPortion();
    if( pPor != pUnderFlow )
    {
        // pPrev will be the last portion before pUnderFlow,
        // which still has a real width.
        // Exception: SoftHyphPortions must not be forgotten, of course!
        // Although they don't have a width.
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
        if( pPor && // Flies + Initialen werden nicht beim UnderFlow mitgenommen
            ( pPor->IsFlyPortion() || pPor->IsDropPortion() ||
              pPor->IsFlyCntPortion() ) )
        {
            pPor->Move( rInf );
            rInf.SetLast( pPor );
            rInf.SetStopUnderFlow( sal_True );
            pPor = pUnderFlow;
        }
    }

    // What? The under-flow portion is not in the portion chain?
    OSL_ENSURE( pPor, "SwTxtFormatter::UnderFlow: overflow but underflow" );

    // OD 2004-05-26 #i29529# - correction: no delete of footnotes
//    if( rInf.IsFtnInside() && pPor && !rInf.IsQuick() )
//    {
//        SwLinePortion *pTmp = pPor->GetPortion();
//        while( pTmp )
//        {
//            if( pTmp->IsFtnPortion() )
//                ((SwFtnPortion*)pTmp)->ClearFtn();
//            pTmp = pTmp->GetPortion();
//        }
//    }

    /*--------------------------------------------------
     * Snapshot
     * --------------------------------------------------*/
    if ( pPor==rInf.GetLast() )
    {
        // We end up here, if the portion triggering the under-flow
        // spans over the whole line. E.g. if a word spans across
        // multiple lines and flows into a fly in the second line.
        rInf.SetFly( pFly );
        pPor->Truncate();
        return pPor; // Is that enough?
    }
    /*---------------------------------------------------
     * End the snapshot
     * --------------------------------------------------*/

    // X + Width == 0 with SoftHyph > Line?!
    if( !pPor || !(rInf.X() + pPor->Width()) )
    {
        delete pFly;
        return 0;
    }

    // Preparing for Format()
    // We need to chip off the chain behind pLast, because we Insert after the Format()
    SeekAndChg( rInf );

    // line width is adjusted, so that pPor does not fit to current
    // line anymore
    rInf.Width( (sal_uInt16)(rInf.X() + (pPor->Width() ? pPor->Width() - 1 : 0)) );
    rInf.SetLen( pPor->GetLen() );
    rInf.SetFull( sal_False );
    if( pFly )
    {
        // We need to recalculate the FlyPortion due to the following reason:
        // If the base line is lowered by a big font in the middle of the line,
        // causing overlapping with a fly, the FlyPortion has a wrong size/fixed
        // size.
        rInf.SetFly( pFly );
        CalcFlyWidth( rInf );
    }
    rInf.GetLast()->SetPortion(0);

    // The SwLineLayout is an exception to this, which splits at the first
    // portion change.
    // Here inly the other way around:
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
        // HACK: decrement again, so we pick up the suffix in next line!
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
    // The new portion is inserted, but everything's different for
    // LineLayout ...
    if( pPor == pCurr )
    {
        if ( pCurr->GetPortion() )
        {
            pPor = pCurr->GetPortion();
        }

        // #i112181#
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

        // Adjust maxima
        if( pCurr->Height() < pPor->Height() )
            pCurr->Height( pPor->Height() );
        if( pCurr->GetAscent() < pPor->GetAscent() )
            pCurr->SetAscent( pPor->GetAscent() );
    }

    // Sometimes chains are constructed (e.g. by hyphenate)
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
    OSL_ENSURE( rInf.GetTxt().Len() < STRING_LEN,
            "SwTxtFormatter::BuildPortions: bad text length in info" );

    rInf.ChkNoHyph( CntEndHyph(), CntMidHyph() );

    // First NewTxtPortion() decides whether pCurr ends up in pPor.
    // We need to make sure that the font is being set in any case.
    // This is done automatically in CalcAscent.
    rInf.SetLast( pCurr );
    rInf.ForcedLeftMargin( 0 );

    OSL_ENSURE( pCurr->FindLastPortion() == pCurr, "pLast supposed to equal pCurr" );

    if( !pCurr->GetAscent() && !pCurr->Height() )
        CalcAscent( rInf, pCurr );

    SeekAndChg( rInf );

    // Width() is shortened in CalcFlyWidth if we have a FlyPortion
    OSL_ENSURE( !rInf.X() || pMulti, "SwTxtFormatter::BuildPortion X=0?" );
    CalcFlyWidth( rInf );
    SwFlyPortion *pFly = rInf.GetFly();
    if( pFly )
    {
        if ( 0 < pFly->Fix() )
            ClearFly( rInf );
        else
            rInf.SetFull(sal_True);
    }

    SwLinePortion *pPor = NewPortion( rInf );

    // Asian grid stuff
    GETGRID( pFrm->FindPageFrm() )
    const sal_Bool bHasGrid = pGrid && rInf.SnapToGrid() &&
                              GRID_LINES_CHARS == pGrid->GetGridType();

    const SwDoc *pDoc = rInf.GetTxtFrm()->GetNode()->GetDoc();
    const sal_uInt16 nGridWidth = bHasGrid ?
                                GETGRIDWIDTH(pGrid,pDoc) : 0;   //for textgrid refactor

    // used for grid mode only:
    // the pointer is stored, because after formatting of non-asian text,
    // the width of the kerning portion has to be adjusted
    SwKernPortion* pGridKernPortion = 0;

    sal_Bool bFull;
    SwTwips nUnderLineStart = 0;
    rInf.Y( Y() );

    while( pPor && !rInf.IsStop() )
    {
        OSL_ENSURE( rInf.GetLen() < STRING_LEN &&
                rInf.GetIdx() <= rInf.GetTxt().Len(),
                "SwTxtFormatter::BuildPortions: bad length in info" );

        // We have to check the script for fields in order to set the
        // correct nActual value for the font.
        if( pPor->InFldGrp() )
            ((SwFldPortion*)pPor)->CheckScript( rInf );

        if( ! bHasGrid && rInf.HasScriptSpace() &&
            rInf.GetLast() && rInf.GetLast()->InTxtGrp() &&
            rInf.GetLast()->Width() && !rInf.GetLast()->InNumberGrp() )
        {
            sal_uInt8 nNxtActual = rInf.GetFont()->GetActual();
            sal_uInt8 nLstActual = nNxtActual;
            sal_uInt16 nLstHeight = (sal_uInt16)rInf.GetFont()->GetHeight();
            sal_Bool bAllowBefore = sal_False;
            sal_Bool bAllowBehind = sal_False;
            const CharClass& rCC = GetAppCharClass();

            // are there any punctuation characters on both sides
            // of the kerning portion?
            if ( pPor->InFldGrp() )
            {
                XubString aAltTxt;
                if ( ((SwFldPortion*)pPor)->GetExpTxt( rInf, aAltTxt ) &&
                        aAltTxt.Len() )
                {
                    bAllowBehind = rCC.isLetterNumeric( aAltTxt, 0 );

                    const SwFont* pTmpFnt = ((SwFldPortion*)pPor)->GetFont();
                    if ( pTmpFnt )
                        nNxtActual = pTmpFnt->GetActual();
                }
            }
            else
                bAllowBehind = rCC.isLetterNumeric( rInf.GetTxt(), rInf.GetIdx() );

            const SwLinePortion* pLast = rInf.GetLast();
            if ( bAllowBehind && pLast )
            {
                if ( pLast->InFldGrp() )
                {
                    XubString aAltTxt;
                    if ( ((SwFldPortion*)pLast)->GetExpTxt( rInf, aAltTxt ) &&
                         aAltTxt.Len() )
                    {
                        bAllowBefore = rCC.isLetterNumeric( aAltTxt, aAltTxt.Len() - 1 );

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
                    // Note: ScriptType returns values in [1,4]
                    if ( bAllowBefore )
                        nLstActual = pScriptInfo->ScriptType( rInf.GetIdx() - 1 ) - 1;
                }

                nLstHeight /= 5;
                // does the kerning portion still fit into the line?
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
            // insert a grid kerning portion
            if ( ! pGridKernPortion )
                pGridKernPortion = pPor->IsKernPortion() ?
                                   (SwKernPortion*)pPor :
                                   new SwKernPortion( *pCurr );

            // if we have a new GridKernPortion, we initially calculate
            // its size so that its ends on the grid
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

        // the multi-portion has it's own format function
        if( pPor->IsMultiPortion() && ( !pMulti || pMulti->IsBidi() ) )
            bFull = BuildMultiPortion( rInf, *((SwMultiPortion*)pPor) );
        else
            bFull = pPor->Format( rInf );

        if( rInf.IsRuby() && !rInf.GetRest() )
            bFull = sal_True;

        // if we are underlined, we store the beginning of this underlined
        // segment for repaint optimization
        if ( UNDERLINE_NONE != pFnt->GetUnderline() && ! nUnderLineStart )
            nUnderLineStart = GetLeftMargin() + rInf.X();

        if ( pPor->IsFlyPortion() )
            pCurr->SetFly( sal_True );
        // some special cases, where we have to take care for the repaint
        // offset:
        // 1. Underlined portions due to special underline feature
        // 2. Right Tab
        // 3. BidiPortions
        // 4. other Multiportions
        // 5. DropCaps
        // 6. Grid Mode
        else if ( ( ! rInf.GetPaintOfst() || nUnderLineStart < rInf.GetPaintOfst() ) &&
                  // 1. Underlined portions
                  nUnderLineStart &&
                     // reformat is at end of an underlined portion and next portion
                     // is not underlined
                  ( ( rInf.GetReformatStart() == rInf.GetIdx() &&
                      UNDERLINE_NONE == pFnt->GetUnderline()
                    ) ||
                     // reformat is inside portion and portion is underlined
                    ( rInf.GetReformatStart() >= rInf.GetIdx() &&
                      rInf.GetReformatStart() <= rInf.GetIdx() + pPor->GetLen() &&
                      UNDERLINE_NONE != pFnt->GetUnderline() ) ) )
            rInf.SetPaintOfst( nUnderLineStart );
        else if (  ! rInf.GetPaintOfst() &&
                   // 2. Right Tab
                   ( ( pPor->InTabGrp() && !pPor->IsTabLeftPortion() ) ||
                   // 3. BidiPortions
                     ( pPor->IsMultiPortion() && ((SwMultiPortion*)pPor)->IsBidi() ) ||
                   // 4. Multi Portion and 5. Drop Caps
                     ( ( pPor->IsDropPortion() || pPor->IsMultiPortion() ) &&
                       rInf.GetReformatStart() >= rInf.GetIdx() &&
                       rInf.GetReformatStart() <= rInf.GetIdx() + pPor->GetLen() )
                   // 6. Grid Mode
                     || ( bHasGrid && SW_CJK != pFnt->GetActual() )
                   )
                )
            // we store the beginning of the critical portion as our
            // paint offset
            rInf.SetPaintOfst( GetLeftMargin() + rInf.X() );

        // under one of these conditions we are allowed to delete the
        // start of the underline portion
        if ( IsUnderlineBreak( *pPor, *pFnt ) )
            nUnderLineStart = 0;

        if( pPor->IsFlyCntPortion() || ( pPor->IsMultiPortion() &&
            ((SwMultiPortion*)pPor)->HasFlyInCntnt() ) )
            SetFlyInCntBase();
        // bUnderFlow needs to be reset or we wrap again at the next softhyphen
        if ( !bFull )
        {
            rInf.ClrUnderFlow();
            if( ! bHasGrid && rInf.HasScriptSpace() && pPor->InTxtGrp() &&
                pPor->GetLen() && !pPor->InFldGrp() )
            {
                // The distance between two different scripts is set
                // to 20% of the fontheight.
                xub_StrLen nTmp = rInf.GetIdx() + pPor->GetLen();
                if( nTmp == pScriptInfo->NextScriptChg( nTmp - 1 ) &&
                    nTmp != rInf.GetTxt().Len() )
                {
                    sal_uInt16 nDist = (sal_uInt16)(rInf.GetFont()->GetHeight()/5);

                    if( nDist )
                    {
                        // we do not want a kerning portion if any end
                        // would be a punctuation character
                        const CharClass& rCC = GetAppCharClass();
                        if ( rCC.isLetterNumeric( rInf.GetTxt(), nTmp - 1 ) &&
                             rCC.isLetterNumeric( rInf.GetTxt(), nTmp ) )
                        {
                            // does the kerning portion still fit into the line?
                            if ( rInf.X() + pPor->Width() + nDist <= rInf.Width() )
                                new SwKernPortion( *pPor, nDist );
                            else
                                bFull = sal_True;
                        }
                    }
                }
            }
        }

        if ( bHasGrid && pPor != pGridKernPortion && ! pMulti )
        {
            xub_StrLen nTmp = rInf.GetIdx() + pPor->GetLen();
            const SwTwips nRestWidth = rInf.Width() - rInf.X() - pPor->Width();

            const sal_uInt8 nCurrScript = pFnt->GetActual(); // pScriptInfo->ScriptType( rInf.GetIdx() );
            const sal_uInt8 nNextScript = nTmp >= rInf.GetTxt().Len() ?
                                     SW_CJK :
                                     SwScriptInfo::WhichFont( nTmp, 0, pScriptInfo );

            // snap non-asian text to grid if next portion is ASIAN or
            // there are no more portions in this line
            // be careful when handling an underflow event: the gridkernportion
            // could have been deleted
            if ( nRestWidth > 0 && SW_CJK != nCurrScript &&
                ! rInf.IsUnderFlow() && ( bFull || SW_CJK == nNextScript ) )
            {
                OSL_ENSURE( pGridKernPortion, "No GridKernPortion available" );

                // calculate size
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
                const SwTwips nKernWidth = Min( (SwTwips)(nTmpWidth - nSumWidth),
                                                nRestWidth );
                const sal_uInt16 nKernWidth_1 = (sal_uInt16)(nKernWidth / 2);

                OSL_ENSURE( nKernWidth <= nRestWidth,
                        "Not enough space left for adjusting non-asian text in grid mode" );

                pGridKernPortion->Width( pGridKernPortion->Width() + nKernWidth_1 );
                rInf.X( rInf.X() + nKernWidth_1 );

                if ( ! bFull )
                    new SwKernPortion( *pPor, (short)(nKernWidth - nKernWidth_1),
                                       sal_False, sal_True );

                pGridKernPortion = 0;
            }
            else if ( pPor->IsMultiPortion() || pPor->InFixMargGrp() ||
                      pPor->IsFlyCntPortion() || pPor->InNumberGrp() ||
                      pPor->InFldGrp() || nCurrScript != nNextScript )
                // next portion should snap to grid
                pGridKernPortion = 0;
        }

        rInf.SetFull( bFull );

        // Restportions from fields with multiple lines don't yet have the right ascent
        if ( !pPor->GetLen() && !pPor->IsFlyPortion()
            && !pPor->IsGrfNumPortion() && ! pPor->InNumberGrp()
            && !pPor->IsMultiPortion() )
            CalcAscent( rInf, pPor );

        InsertPortion( rInf, pPor );
        pPor = NewPortion( rInf );
    }

    if( !rInf.IsStop() )
    {
        // The last right centered, decimal tab
        SwTabPortion *pLastTab = rInf.GetLastTab();
        if( pLastTab )
            pLastTab->FormatEOL( rInf );
        else if( rInf.GetLast() && rInf.LastKernPortion() )
            rInf.GetLast()->FormatEOL( rInf );
    }
    if( pCurr->GetPortion() && pCurr->GetPortion()->InNumberGrp()
        && ((SwNumberPortion*)pCurr->GetPortion())->IsHide() )
        rInf.SetNumDone( sal_False );

    // Delete fly in any case
    ClearFly( rInf );

    // Reinit the tab overflow flag after the line
    rInf.SetTabOverflow( sal_False );
}

/*************************************************************************
 *                 SwTxtFormatter::CalcAdjustLine()
 *************************************************************************/

void SwTxtFormatter::CalcAdjustLine( SwLineLayout *pCurrent )
{
    if( SVX_ADJUST_LEFT != GetAdjust() && !pMulti)
    {
        pCurrent->SetFormatAdj(sal_True);
        if( IsFlyInCntBase() )
        {
            CalcAdjLine( pCurrent );
            // For e.g. centered fly we need to switch the RefPoint
            // That's why bAllWays = sal_True
            UpdatePos( pCurrent, GetTopLeft(), GetStart(), sal_True );
        }
    }
}

/*************************************************************************
 *                      SwTxtFormatter::CalcAscent()
 *************************************************************************/

void SwTxtFormatter::CalcAscent( SwTxtFormatInfo &rInf, SwLinePortion *pPor )
{
    if ( pPor->InFldGrp() && ((SwFldPortion*)pPor)->GetFont() )
    {
        // Numbering + InterNetFlds can keep an own font, then their size is
        // independent from hard attribute values
        SwFont* pFldFnt = ((SwFldPortion*)pPor)->pFnt;
        SwFontSave aSave( rInf, pFldFnt );
        ((SwFldPortion*)pPor)->Height( pFldFnt->GetHeight( rInf.GetVsh(), *rInf.GetOut() ) );
        ((SwFldPortion*)pPor)->SetAscent( pFldFnt->GetAscent( rInf.GetVsh(), *rInf.GetOut() ) );
    }
    // #i89179#
    // tab portion representing the list tab of a list label gets the
    // same height and ascent as the corresponding number portion
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
        sal_Bool bChg;

        // In empty lines the attributes are switched on via SeekStart
        const sal_Bool bFirstPor = rInf.GetLineStart() == rInf.GetIdx();
        if ( pPor->IsQuoVadisPortion() )
            bChg = SeekStartAndChg( rInf, sal_True );
        else
        {
            if( bFirstPor )
            {
                if( rInf.GetTxt().Len() )
                {
                    if ( pPor->GetLen() || !rInf.GetIdx()
                         || ( pCurr != pLast && !pLast->IsFlyPortion() )
                         || !pCurr->IsRest() ) // instead of !rInf.GetRest()
                        bChg = SeekAndChg( rInf );
                    else
                        bChg = SeekAndChgBefore( rInf );
                }
                else if ( pMulti )
                    // do not open attributes starting at 0 in empty multi
                    // portions (rotated numbering followed by a footnote
                    // can cause trouble, because the footnote attribute
                    // starts at 0, but if we open it, the attribute handler
                    // cannot handle it.
                    bChg = sal_False;
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
        }
        else
        {
            pPor->Height( pLast->Height() );
            pPor->SetAscent( pLast->GetAscent() );
        }
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
//    OUTPUT_OPERATOR
};

//CLASSIO( SwMetaPortion )

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
        pPor = new SwToxPortion;
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
            // Only at the End!
            // If pCurr does not have a width, it can however aready have content.
            // E.g. for non-displayable characters
            if( rInf.GetLen() > 0 )
            {
                if( rInf.GetTxt().GetChar(rInf.GetIdx())==CH_TXT_ATR_FIELDSTART )
                    pPor = new SwFieldMarkPortion();
                else if( rInf.GetTxt().GetChar(rInf.GetIdx())==CH_TXT_ATR_FIELDEND )
                    pPor = new SwFieldMarkPortion();
                else if( rInf.GetTxt().GetChar(rInf.GetIdx())==CH_TXT_ATR_FORMELEMENT )
                    pPor = new SwFieldFormPortion();
            }
            if( !pPor )
            {
                if( !rInf.X() && !pCurr->GetPortion() && !pCurr->GetLen() && !GetFnt()->IsURL() )
                    pPor = pCurr;
                else
                {
                    pPor = new SwTxtPortion;
                    if( GetFnt()->IsURL() )
                        pPor->SetWhichPor( POR_URL );
                }
            }
        }
    }
    return pPor;
}

/*************************************************************************
 *                      SwTxtFormatter::NewTxtPortion()
 *************************************************************************/
// We calculate the length, the following portion limits are defined:
// 1) Tabs
// 2) Linebreaks
// 3) CH_TXTATR_BREAKWORD / CH_TXTATR_INWORD
// 4) next attribute change

SwTxtPortion *SwTxtFormatter::NewTxtPortion( SwTxtFormatInfo &rInf )
{
    // If we're at the line's beginning, we take pCurr
    // If pCurr is not derived from SwTxtPortion, we need to duplicate
    Seek( rInf.GetIdx() );
    SwTxtPortion *pPor = WhichTxtPor( rInf );

    // until next attribute change:
    const xub_StrLen nNextAttr = GetNextAttr();
    xub_StrLen nNextChg = Min( nNextAttr, rInf.GetTxt().Len() );

    // end of script type:
    const xub_StrLen nNextScript = pScriptInfo->NextScriptChg( rInf.GetIdx() );
    nNextChg = Min( nNextChg, nNextScript );

    // end of direction:
    const xub_StrLen nNextDir = pScriptInfo->NextDirChg( rInf.GetIdx() );
    nNextChg = Min( nNextChg, nNextDir );

    // Turbo boost:
    // We assume that a font's characters are not larger than twice
    // as wide as heigh.
    // Very crazy: We need to take the ascent into account.
    //
    // Mind the trap! GetSize() contains the wished-for height, the real height
    // is only known in CalcAscent!
    //
    // The ratio is even crazier: a blank in Times New Roman has an ascent of
    // 182, a height of 200 and a width of 53!
    // It follows that a line with a lot of blanks is processed incorrectly.
    // Therefore we increase from factor 2 to 8 (due to negative kerning).

    pPor->SetLen(1);
    CalcAscent( rInf, pPor );

    const SwFont* pTmpFnt = rInf.GetFont();
    KSHORT nExpect = Min( KSHORT( ((Font *)pTmpFnt)->GetSize().Height() ),
                          KSHORT( pPor->GetAscent() ) ) / 8;
    if ( !nExpect )
        nExpect = 1;
    nExpect = (sal_uInt16)(rInf.GetIdx() + ((rInf.Width() - rInf.X()) / nExpect));
    if( nExpect > rInf.GetIdx() && nNextChg > nExpect )
        nNextChg = Min( nExpect, rInf.GetTxt().Len() );

    // we keep an invariant during method calls:
    // there are no portion ending characters like hard spaces
    // or tabs in [ nLeftScanIdx, nRightScanIdx ]
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
        // Tabs and fields
        if( '\0' != rInf.GetHookChar() )
            return 0;

        pPor = rInf.GetRest();
        if( pPor->IsErgoSumPortion() )
            rInf.SetErgoDone(sal_True);
        else
            if( pPor->IsFtnNumPortion() )
                rInf.SetFtnDone(sal_True);
            else
                if( pPor->InNumberGrp() )
                    rInf.SetNumDone(sal_True);

        rInf.SetRest(0);
        pCurr->SetRest( sal_True );
        return pPor;
    }

    // We can stand in the follow, it's crucial that
    // pFrm->GetOfst() == 0!
    if( rInf.GetIdx() )
    {
        // We now too can elongate FtnPortions and ErgoSumPortions

        // 1. The ErgoSumTexts
        if( !rInf.IsErgoDone() )
        {
            if( pFrm->IsInFtn() && !pFrm->GetIndPrev() )
                pPor = (SwLinePortion*)NewErgoSumPortion( rInf );
            rInf.SetErgoDone( sal_True );
        }

        // 2. Arrow portions
        if( !pPor && !rInf.IsArrowDone() )
        {
            if( pFrm->GetOfst() && !pFrm->IsFollow() &&
                rInf.GetIdx() == pFrm->GetOfst() )
                pPor = new SwArrowPortion( *pCurr );
            rInf.SetArrowDone( sal_True );
        }

        // 3. Kerning portions at beginning of line in grid mode
        if ( ! pPor && ! pCurr->GetPortion() )
        {
            GETGRID( GetTxtFrm()->FindPageFrm() )
            if ( pGrid )
                pPor = new SwKernPortion( *pCurr );
        }

        // 4. The line rests (multiline fields)
        if( !pPor )
        {
            pPor = rInf.GetRest();
            // Only for pPor of course
            if( pPor )
            {
                pCurr->SetRest( sal_True );
                rInf.SetRest(0);
            }
        }
    }
    else
    {
        // 5. The foot note count
        if( !rInf.IsFtnDone() )
        {
            OSL_ENSURE( ( ! rInf.IsMulti() && ! pMulti ) || pMulti->HasRotation(),
                     "Rotated number portion trouble" );

            sal_Bool bFtnNum = pFrm->IsFtnNumFrm();
            rInf.GetParaPortion()->SetFtnNum( bFtnNum );
            if( bFtnNum )
                pPor = (SwLinePortion*)NewFtnNumPortion( rInf );
            rInf.SetFtnDone( sal_True );
        }

        // 6. The ErgoSumTexts of course also exist in the TextMaster,
        // it's crucial whether the SwFtnFrm is aFollow
        if( !rInf.IsErgoDone() && !pPor && ! rInf.IsMulti() )
        {
            if( pFrm->IsInFtn() && !pFrm->GetIndPrev() )
                pPor = (SwLinePortion*)NewErgoSumPortion( rInf );
            rInf.SetErgoDone( sal_True );
        }

        // 7. The numbering
        if( !rInf.IsNumDone() && !pPor )
        {
            OSL_ENSURE( ( ! rInf.IsMulti() && ! pMulti ) || pMulti->HasRotation(),
                     "Rotated number portion trouble" );

            // If we're in the follow, then of course not
            if( GetTxtFrm()->GetTxtNode()->GetNumRule() )
                pPor = (SwLinePortion*)NewNumberPortion( rInf );
            rInf.SetNumDone( sal_True );
        }
        // 8. The DropCaps
        if( !pPor && GetDropFmt() && ! rInf.IsMulti() )
            pPor = (SwLinePortion*)NewDropPortion( rInf );

        // 9. Kerning portions at beginning of line in grid mode
        if ( !pPor && !pCurr->GetPortion() )
        {
            GETGRID( GetTxtFrm()->FindPageFrm() )
            if ( pGrid )
                pPor = new SwKernPortion( *pCurr );
        }
    }

        // 10. Decimal tab portion at the beginning of each line in table cells
        if ( !pPor && !pCurr->GetPortion() &&
             GetTxtFrm()->IsInTab() &&
             GetTxtFrm()->GetTxtNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::TAB_COMPAT) )
        {
            pPor = NewTabPortion( rInf, true );
        }

        // 11. suffix of meta-field
        if (!pPor)
        {
            pPor = TryNewNoLengthPortion(rInf);
        }

    return pPor;
}

static sal_Bool lcl_OldFieldRest( const SwLineLayout* pCurr )
{
    if( !pCurr->GetNext() )
        return sal_False;
    const SwLinePortion *pPor = pCurr->GetNext()->GetPortion();
    sal_Bool bRet = sal_False;
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
    // Underflow takes precedence
    rInf.SetStopUnderFlow( sal_False );
    if( rInf.GetUnderFlow() )
    {
        OSL_ENSURE( rInf.IsFull(), "SwTxtFormatter::NewPortion: underflow but not full" );
        return UnderFlow( rInf );
    }

    // If the line is full, flys and UnderFlow portions could be waiting ...
    if( rInf.IsFull() )
    {
        // LineBreaks and Flys (bug05.sdw)
        // IsDummy()
        if( rInf.IsNewLine() && (!rInf.GetFly() || !pCurr->IsDummy()) )
            return 0;

        // Wenn der Text an den Fly gestossen ist, oder wenn
        // der Fly als erstes drankommt, weil er ueber dem linken
        // Rand haengt, wird GetFly() returnt.
        // Wenn IsFull() und kein GetFly() vorhanden ist, gibt's
        // naturgemaesz eine 0.
        if( rInf.GetFly() )
        {
            if( rInf.GetLast()->IsBreakPortion() )
            {
                delete rInf.GetFly();
                rInf.SetFly( 0 );
            }

            return rInf.GetFly();
        }
        // Ein fieser Sonderfall: ein Rahmen ohne Umlauf kreuzt den
        // Ftn-Bereich. Wir muessen die Ftn-Portion als Zeilenrest
        // bekanntgeben, damit SwTxtFrm::Format nicht abbricht
        // (die Textmasse wurde ja durchformatiert).
        if( rInf.GetRest() )
            rInf.SetNewLine( sal_True );
        else
        {
            // Wenn die naechste Zeile mit einem Rest eines Feldes beginnt,
            // jetzt aber kein Rest mehr anliegt,
            // muss sie auf jeden Fall neu formatiert werden!
            if( lcl_OldFieldRest( GetCurr() ) )
                rInf.SetNewLine( sal_True );
            else
            {
                SwLinePortion *pFirst = WhichFirstPortion( rInf );
                if( pFirst )
                {
                    rInf.SetNewLine( sal_True );
                    if( pFirst->InNumberGrp() )
                        rInf.SetNumDone( sal_False) ;
                    delete pFirst;
                }
            }
        }

        return 0;
    }

    SwLinePortion *pPor = WhichFirstPortion( rInf );

    // Check for Hidden Portion:
    if ( !pPor )
    {
        xub_StrLen nEnd = rInf.GetIdx();
        if ( ::lcl_BuildHiddenPortion( rInf, nEnd ) )
            pPor = new SwHiddenTextPortion( nEnd - rInf.GetIdx() );
    }

    if( !pPor )
    {
        if( ( !pMulti || pMulti->IsBidi() ) &&
            // #i42734#
            // No multi portion if there is a hook character waiting:
            ( !rInf.GetRest() || '\0' == rInf.GetHookChar() ) )
        {
            // We open a multiportion part, if we enter a multi-line part
            // of the paragraph.
            xub_StrLen nEnd = rInf.GetIdx();
            SwMultiCreator* pCreate = rInf.GetMultiCreator( nEnd, pMulti );
            if( pCreate )
            {
                SwMultiPortion* pTmp = NULL;

                if ( SW_MC_BIDI == pCreate->nId )
                    pTmp = new SwBidiPortion( nEnd, pCreate->nLevel );
                else if ( SW_MC_RUBY == pCreate->nId )
                {
                    Seek( rInf.GetIdx() );
                    sal_Bool bRubyTop;
                    sal_Bool* pRubyPos = 0;

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
        // 5010: Tabs und Felder
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
            if( rInf.GetIdx() >= rInf.GetTxt().Len() )
            {
                rInf.SetFull(sal_True);
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

            case CHAR_SOFTHYPHEN:                   // soft hyphen
                pPor = new SwSoftHyphPortion; break;

            case CHAR_HARDBLANK:                    // no-break space
                pPor = new SwBlankPortion( ' ' ); break;

            case CHAR_HARDHYPHEN:               // non-breaking hyphen
                pPor = new SwBlankPortion( '-' ); break;

            case CHAR_ZWSP:                     // zero width space
            case CHAR_ZWNBSP :                  // word joiner
//            case CHAR_RLM :                     // right to left mark
//            case CHAR_LRM :                     // left to right mark
                pPor = new SwControlCharPortion( cChar ); break;

            case CH_TXTATR_BREAKWORD:
            case CH_TXTATR_INWORD:
                            if( rInf.HasHint( rInf.GetIdx() ) )
                            {
                                pPor = NewExtraPortion( rInf );
                                break;
                            }
                            // No break
            default        :
            {
                SwTabPortion* pLastTabPortion = rInf.GetLastTab();
                if ( pLastTabPortion && cChar == rInf.GetTabDecimal() )
                {
                    // #127428# Abandon dec. tab position if line is full
                    // We have a decimal tab portion in the line and the next character has to be
                    // aligned at the tab stop position. We store the width from the beginning of
                    // the tab stop portion up to the portion containint the decimal separator:
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
                        rInf.SetNewLine(sal_True);
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

        // if a portion is created despite there being a pending RestPortion,
        // then it is a field which has been split (e.g. because it contains a Tab)
        if( pPor && rInf.GetRest() )
            pPor->SetLen( 0 );

        // robust:
        if( !pPor || rInf.IsStop() )
        {
            delete pPor;
            return 0;
        }
    }

    // Special portions containing numbers (footnote anchor, footnote number,
    // numbering) can be contained in a rotated portion, if the user
    // choose a rotated character attribute.
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
                    sal_True, &pItem ))
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

                    rInf.SetNumDone( sal_False );
                    rInf.SetFtnDone( sal_False );
                }
            }
        }
    }

    // Der Font wird im Outputdevice eingestellt,
    // der Ascent und die Hoehe werden berechnet.
    if( !pPor->GetAscent() && !pPor->Height() )
        CalcAscent( rInf, pPor );
    rInf.SetLen( pPor->GetLen() );

    // In CalcFlyWidth wird Width() verkuerzt, wenn eine FlyPortion vorliegt.
    CalcFlyWidth( rInf );

    // Man darf nicht vergessen, dass pCurr als GetLast() vernuenftige
    // Werte bereithalten muss:
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

xub_StrLen SwTxtFormatter::FormatLine( const xub_StrLen nStartPos )
{
    OSL_ENSURE( ! pFrm->IsVertical() || pFrm->IsSwapped(),
            "SwTxtFormatter::FormatLine( nStartPos ) with unswapped frame" );

    // For the formatting routines, we set pOut to the reference device.
    SwHookOut aHook( GetInfo() );
    if( GetInfo().GetLen() < GetInfo().GetTxt().Len() )
        GetInfo().SetLen( GetInfo().GetTxt().Len() );

    sal_Bool bBuild = sal_True;
    SetFlyInCntBase( sal_False );
    GetInfo().SetLineHeight( 0 );
    GetInfo().SetLineNettoHeight( 0 );

    // Recycling muss bei geaenderter Zeilenhoehe unterdrueckt werden
    // und auch bei geaendertem Ascent (Absenken der Grundlinie).
    const KSHORT nOldHeight = pCurr->Height();
    const KSHORT nOldAscent = pCurr->GetAscent();

    pCurr->SetEndHyph( sal_False );
    pCurr->SetMidHyph( sal_False );

    // fly positioning can make it necessary format a line several times
    // for this, we have to keep a copy of our rest portion
    SwLinePortion* pFld = GetInfo().GetRest();
    SwFldPortion* pSaveFld = 0;

    if ( pFld && pFld->InFldGrp() && !pFld->IsFtnPortion() )
        pSaveFld = new SwFldPortion( *((SwFldPortion*)pFld) );

    // for an optimal repaint rectangle, we want to compare fly portions
    // before and after the BuildPortions call
    const sal_Bool bOptimizeRepaint = AllowRepaintOpt();
    const xub_StrLen nOldLineEnd = nStartPos + pCurr->GetLen();
    std::vector<long> flyStarts;

    // these are the conditions for a fly position comparison
    if ( bOptimizeRepaint && pCurr->IsFly() )
    {
        SwLinePortion* pPor = pCurr->GetFirstPortion();
        long nPOfst = 0;
        while ( pPor )
        {
            if ( pPor->IsFlyPortion() )
                // insert start value of fly portion
                flyStarts.push_back( nPOfst );

            nPOfst += pPor->Width();
            pPor = pPor->GetPortion();
        }
    }

    // Hier folgt bald die Unterlaufpruefung.
    while( bBuild )
    {
        GetInfo().SetFtnInside( sal_False );
        GetInfo().SetOtherThanFtnInside( sal_False );

        // These values must not be reset by FormatReset();
        sal_Bool bOldNumDone = GetInfo().IsNumDone();
        sal_Bool bOldArrowDone = GetInfo().IsArrowDone();
        sal_Bool bOldErgoDone = GetInfo().IsErgoDone();

        // besides other things, this sets the repaint offset to 0
        FormatReset( GetInfo() );

        GetInfo().SetNumDone( bOldNumDone );
        GetInfo().SetArrowDone( bOldArrowDone );
        GetInfo().SetErgoDone( bOldErgoDone );

        // build new portions for this line
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
            GetInfo().SetDropInit( sal_False );
        }

        pCurr->CalcLine( *this, GetInfo() );
        CalcRealHeight( GetInfo().IsNewLine() );

        if ( IsFlyInCntBase() && !IsQuick() )
        {
            KSHORT nTmpAscent, nTmpHeight;
            CalcAscentAndHeight( nTmpAscent, nTmpHeight );
            AlignFlyInCntBase( Y() + long( nTmpAscent ) );
            pCurr->CalcLine( *this, GetInfo() );
            CalcRealHeight();
        }

        // bBuild entscheidet, ob noch eine Ehrenrunde gedreht wird
        if ( pCurr->GetRealHeight() <= GetInfo().GetLineHeight() )
        {
            pCurr->SetRealHeight( GetInfo().GetLineHeight() );
            bBuild = sal_False;
        }
        else
        {
            bBuild = ( GetInfo().GetTxtFly()->IsOn() && ChkFlyUnderflow(GetInfo()) )
                     || GetInfo().CheckFtnPortion(pCurr);
            if( bBuild )
            {
                GetInfo().SetNumDone( bOldNumDone );
                GetInfo().ResetMaxWidthDiff();

                // delete old rest
                if ( GetInfo().GetRest() )
                {
                    delete GetInfo().GetRest();
                    GetInfo().SetRest( 0 );
                }

                // set original rest portion
                if ( pSaveFld )
                    GetInfo().SetRest( new SwFldPortion( *pSaveFld ) );

                pCurr->SetLen( 0 );
                pCurr->Width(0);
                pCurr->Truncate();
            }
        }
    }

    // calculate optimal repaint rectangle
    if ( bOptimizeRepaint )
    {
        GetInfo().SetPaintOfst( ::lcl_CalcOptRepaint( *this, *pCurr, nOldLineEnd, flyStarts ) );
        flyStarts.clear();
    }
    else
        // Special case: We do not allow an optimitation of the repaint
        // area, but during formatting the repaint offset is set to indicate
        // a maximum value for the offset. This value has to be reset:
        GetInfo().SetPaintOfst( 0 );

    // This corrects the start of the reformat range if something has
    // moved to the next line. Otherwise IsFirstReformat in AllowRepaintOpt
    // will give us a wrong result if we have to reformat another line
    GetInfo().GetParaPortion()->GetReformat()->LeftMove( GetInfo().GetIdx() );

    // delete master copy of rest portion
    delete pSaveFld;

    xub_StrLen nNewStart = nStartPos + pCurr->GetLen();

    // adjust text if kana compression is enabled
    if ( GetInfo().CompressLine() )
    {
        SwTwips nRepaintOfst = CalcKanaAdj( pCurr );

        // adjust repaint offset
        if ( nRepaintOfst < GetInfo().GetPaintOfst() )
            GetInfo().SetPaintOfst( nRepaintOfst );
    }

    CalcAdjustLine( pCurr );

    if( nOldHeight != pCurr->Height() || nOldAscent != pCurr->GetAscent() )
    {
        SetFlyInCntBase();
        GetInfo().SetPaintOfst( 0 ); //geaenderte Zeilenhoehe => kein Recycling
        // alle weiteren Zeilen muessen gepaintet und, wenn Flys im Spiel sind
        // auch formatiert werden.
        GetInfo().SetShift( sal_True );
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
    sal_Bool bMore = sal_True;
    while(bMore)
    {
        CalcRealHeight();
        bMore = Next() != 0;
    }
}

/*************************************************************************
 *                    SwTxtFormatter::CalcRealHeight()
 *************************************************************************/

void SwTxtFormatter::CalcRealHeight( sal_Bool bNewLine )
{
    KSHORT nLineHeight = pCurr->Height();
    pCurr->SetClipping( sal_False );

    GETGRID( pFrm->FindPageFrm() )
    if ( pGrid && GetInfo().SnapToGrid() )
    {
        const sal_uInt16 nGridWidth = pGrid->GetBaseHeight();
        const sal_uInt16 nRubyHeight = pGrid->GetRubyHeight();
        const sal_Bool bRubyTop = ! pGrid->GetRubyTextBelow();

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

        // we ignore any line spacing options except from ...
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

    // Das Dummyflag besitzen Zeilen, die nur Flyportions enthalten, diese
    // sollten kein Register etc. beachten. Dummerweise hat kann es eine leere
    // Zeile am Absatzende geben (bei leeren Abs?tzen oder nach einem
    // Shift-Return), die das Register durchaus beachten soll.
    if( !pCurr->IsDummy() || ( !pCurr->GetNext() &&
        GetStart() >= GetTxtFrm()->GetTxt().Len() && !bNewLine ) )
    {
        const SvxLineSpacingItem *pSpace = aLineInf.GetLineSpacing();
        if( pSpace )
        {
            switch( pSpace->GetLineSpaceRule() )
            {
                case SVX_LINE_SPACE_AUTO:
            if (pSpace->GetInterLineSpaceRule()==SVX_INTER_LINE_SPACE_PROP) {
                        long nTmp = pSpace->GetPropLineSpace();
                        if (nTmp<100) { // code adaped from fixed line height
                            nTmp *= nLineHeight;
                            nTmp /= 100;
                            if( !nTmp )
                                ++nTmp;
                            nLineHeight = (KSHORT)nTmp;
/*
                            //@TODO figure out how WW maps ascent and descent
                            //in case of prop  line spacing <100%
                            KSHORT nAsc = ( 4 * nLineHeight ) / 5;  // 80%
                            if( nAsc < pCurr->GetAscent() ||
                                nLineHeight - nAsc < pCurr->Height() -
pCurr->GetAscent() )
                                pCurr->SetClipping( sal_True );
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
                    KSHORT nAsc = ( 4 * nLineHeight ) / 5;  // 80%
                    if( nAsc < pCurr->GetAscent() ||
                        nLineHeight - nAsc < pCurr->Height() - pCurr->GetAscent() )
                        pCurr->SetClipping( sal_True );
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
                        // 50% ist das Minimum, bei 0% schalten wir auf
                        // den Defaultwert 100% um ...
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
    // 3260, 3860: Fly auf jeden Fall loeschen!
    ClearFly( rInf );
    rInf.Init();

    rInf.ChkNoHyph( CntEndHyph(), CntMidHyph() );
    rInf.SetRoot( pCurr );
    rInf.SetLineStart( nStart );
    rInf.SetIdx( nStart );

    // Handle overflows:
    // #i34348# Changed type from sal_uInt16 to SwTwips
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

    // delete pSpaceAdd und pKanaComp
    pCurr->FinishSpaceAdd();
    pCurr->FinishKanaComp();
    pCurr->ResetFlags();
    FeedInf( rInf );
}

/*************************************************************************
 *                SwTxtFormatter::CalcOnceMore()
 *************************************************************************/

sal_Bool SwTxtFormatter::CalcOnceMore()
{
    if( pDropFmt )
    {
        const KSHORT nOldDrop = GetDropHeight();
        CalcDropHeight( pDropFmt->GetLines() );
        bOnceMore = nOldDrop != GetDropHeight();
    }
    else
        bOnceMore = sal_False;
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
            sal_Bool bRepaint = HasTruncLines() &&
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
sal_Bool SwTxtFormatter::AllowRepaintOpt() const
{
    // reformat position in front of current line? Only in this case
    // we want to set the repaint offset
    sal_Bool bOptimizeRepaint = nStart < GetInfo().GetReformatStart() &&
                                pCurr->GetLen();

    // a special case is the last line of a block adjusted paragraph:
    if ( bOptimizeRepaint )
    {
        switch( GetAdjust() )
        {
        case SVX_ADJUST_BLOCK:
        {
            if( IsLastBlock() || IsLastCenter() )
                bOptimizeRepaint = sal_False;
            else
            {
                // ????: Blank in der letzten Masterzeile (blocksat.sdw)
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
            bOptimizeRepaint = sal_False;
            break;
        default: ;
        }
    }

    // Schon wieder ein Sonderfall: unsichtbare SoftHyphs
    const xub_StrLen nReformat = GetInfo().GetReformatStart();
    if( bOptimizeRepaint && STRING_LEN != nReformat )
    {
        const sal_Unicode cCh = GetInfo().GetTxt().GetChar( nReformat );
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
    xub_StrLen nStartIdx, sal_Bool bAllWays ) const
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

    // The frame's size
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
        // We only know one case where changing the position (caused by the
        // adjustment) could be relevant for a portion: We need to SetRefPoint
        // for FlyCntPortions.
        if( ( pPos->IsFlyCntPortion() || pPos->IsGrfNumPortion() )
            && ( bAllWays || !IsQuick() ) )
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
                // jump to end of the bidi portion
                aSt.X() += pLay->Width();

            xub_StrLen nStIdx = aTmpInf.GetIdx();
            do
            {
                UpdatePos( pLay, aSt, nStIdx, bAllWays );
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


sal_Bool SwTxtFormatter::ChkFlyUnderflow( SwTxtFormatInfo &rInf ) const
{
    OSL_ENSURE( rInf.GetTxtFly()->IsOn(), "SwTxtFormatter::ChkFlyUnderflow: why?" );
    if( GetCurr() )
    {
        // First we check, whether a fly overlaps with the line.
        // = GetLineHeight()
        const long nHeight = GetCurr()->GetRealHeight();
        SwRect aLine( GetLeftMargin(), Y(), rInf.RealWidth(), nHeight );

        SwRect aLineVert( aLine );
        if ( pFrm->IsVertical() )
            pFrm->SwitchHorizontalToVertical( aLineVert );
        SwRect aInter( rInf.GetTxtFly()->GetFrm( aLineVert ) );
        if ( pFrm->IsVertical() )
            pFrm->SwitchVerticalToHorizontal( aInter );

        if( !aInter.HasArea() )
            return sal_False;

        // We now check every portion that could have lowered for overlapping
        // with the fly.
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

            // New flys from below?
            if( !pPos->IsFlyPortion() )
            {
                if( aInter.IsOver( aLine ) )
                {
                    aInter._Intersection( aLine );
                    if( aInter.HasArea() )
                    {
                        // To be evaluated during reformat of this line:
                        // RealHeight including spacing
                        rInf.SetLineHeight( KSHORT(nHeight) );
                        // Height without extra spacing
                        rInf.SetLineNettoHeight( KSHORT( pCurr->Height() ) );
                        return sal_True;
                    }
                }
            }
            else
            {
                // The fly portion is not intersected by a fly anymore
                if ( ! aInter.IsOver( aLine ) )
                {
                    rInf.SetLineHeight( KSHORT(nHeight) );
                    rInf.SetLineNettoHeight( KSHORT( pCurr->Height() ) );
                    return sal_True;
                }
                else
                {
                    aInter._Intersection( aLine );

                    // No area means a fly has become invalid because of
                    // lowering the line => reformat the line
                    // we also have to reformat the line, if the fly size
                    // differs from the intersection interval's size.
                    if( ! aInter.HasArea() ||
                        ((SwFlyPortion*)pPos)->GetFixWidth() != aInter.Width() )
                    {
                        rInf.SetLineHeight( KSHORT(nHeight) );
                        rInf.SetLineNettoHeight( KSHORT( pCurr->Height() ) );
                        return sal_True;
                    }
                }
            }

            aLine.Left( aLine.Left() + pPos->Width() );
            pPos = pPos->GetPortion();
        }
    }
    return sal_False;
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
        // Real line height has already been calculated, we only have to
        // search for intersections in the lower part of the strip
        nAscent = pCurr->GetAscent();
        nHeight = rInf.GetLineNettoHeight();
        nTop += rInf.GetLineHeight() - nHeight;
    }
    else
    {
        nAscent = pLast->GetAscent();
        nHeight = pLast->Height();

        // We make a first guess for the lines real height
        if ( ! pCurr->GetRealHeight() )
            CalcRealHeight();

        if ( pCurr->GetRealHeight() > nHeight )
            nTop += pCurr->GetRealHeight() - nHeight;
        else
            // Important for fixed space between lines
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
        sal_Bool bForced = sal_False;
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
            // For a negative first line indent, we set this flag to show
            // that the indentation/margin has been moved.
            // This needs to be respected by the DefaultTab at the zero position.
            if( IsFirstTxtLine() && HasNegFirst() )
                bForced = sal_True;
        }
        aInter.Intersection( aLine );
        if( !aInter.HasArea() )
            return;

        const sal_Bool bFullLine =  aLine.Left()  == aInter.Left() &&
                                aLine.Right() == aInter.Right();

        // Although no text is left, we need to format another line,
        // because also empty lines need to avoid a Fly with no wrapping.
        if( bFullLine && rInf.GetIdx() == rInf.GetTxt().Len() )
        {
            rInf.SetNewLine( sal_True );
            // 8221: We know that for dummies, it holds ascent == height
            pCurr->SetDummy(sal_True);
        }

        // aInter becomes frame-local
        aInter.Pos().X() -= nLeftMar;
        SwFlyPortion *pFly = new SwFlyPortion( aInter );
        if( bForced )
        {
            pCurr->SetForcedLeftMargin( sal_True );
            rInf.ForcedLeftMargin( (sal_uInt16)aInter.Width() );
        }

        if( bFullLine )
        {
            // 8110: In order to properly flow around Flys with different
            // wrapping attributes, we need to increase by units of line height.
            // The last avoiding line should be adjusted in height, so that
            // we don't get a frame spacing effect.
            // 8221: It is important that ascent == height, because the FlyPortion
            // values are transferred to pCurr in CalcLine and IsDummy() relies
            // on this behaviour.
            // To my knowledge we only have two places where DummyLines can be
            // created: here and in MakeFlyDummies.
            // IsDummy() is evaluated in IsFirstTxtLine(), when moving lines
            // and in relation with DropCaps.
            pFly->Height( KSHORT(aInter.Height()) );

            // nNextTop now contains the margin's bottom edge, which we avoid
            // or the next margin's top edge, which we need to respect.
            // That means we can comfortably grow up to this value; that's how
            // we save a few empty lines.
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
            if( rInf.GetIdx() == rInf.GetTxt().Len() )
            {
                // Don't use nHeight, or we have a huge descent
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
            const sal_uInt16 nGridWidth = GETGRIDWIDTH( pGrid, pDoc);   //For textgrid refactor

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
    // aBase is the document-global position, from which the new extra portion is placed
    // aBase.X() = Offset in in the line after the current position
    // aBase.Y() = LineIter.Y() + Ascent of the current position

    long nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc;
    // OD 08.01.2004 #i11859# - use new method <SwLineLayout::MaxAscentDescent(..)>
    //SwLinePortion *pPos = pCurr->GetFirstPortion();
    //lcl_MaxAscDescent( pPos, nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc );
    pCurr->MaxAscentDescent( nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc );

    // If the ascent of the frame is larger than the ascent of the current position,
    // we use this one when calculating the base, or the frame would be positioned
    // too much to the top, sliding down after all causing a repaint in an area
    // he actually never was in.
    KSHORT nAscent = 0;

    const bool bTxtFrmVertical = GetInfo().GetTxtFrm()->IsVertical();

    const bool bUseFlyAscent = pFly && pFly->GetValidPosFlag() &&
                               0 != ( bTxtFrmVertical ?
                                      pFly->GetRefPoint().X() :
                                      pFly->GetRefPoint().Y() );

    if ( bUseFlyAscent )
         nAscent = static_cast<sal_uInt16>( Abs( int( bTxtFrmVertical ?
                                                  pFly->GetRelPos().X() :
                                                  pFly->GetRelPos().Y() ) ) );

    // Check if be prefer to use the ascent of the last portion:
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
        // We need to make sure that our font is set again in the OutputDevice
        // It could be that the FlyInCnt was added anew and GetFlyFrm() would
        // in turn cause, that it'd be created anew again.
        // This one's frames get formatted right away, which change the font and
        // we have a bug (3322).
        rInf.SelectFont();
        if( pRet->GetAscent() > nAscent )
        {
            aBase.Y() = Y() + pRet->GetAscent();
            nMode |= AS_CHAR_ULSPACE;
            if( !rInf.IsTest() )
                aTmpBase = aBase;
                if ( GetInfo().GetTxtFrm()->IsVertical() )
                    GetInfo().GetTxtFrm()->SwitchHorizontalToVertical( aTmpBase );

                pRet->SetBase( *rInf.GetTxtFrm(), aTmpBase, nTmpAscent,
                               nTmpDescent, nFlyAsc, nFlyDesc, nMode );
        }
    }
    else
    {
        pRet = new SwFlyCntPortion( *rInf.GetTxtFrm(), (SwDrawContact*)pFrmFmt->FindContactObj(),
           aTmpBase, nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc, nMode );
    }
    return pRet;
}

namespace {
    /*************************************************************************
    *                      ::CalcOptRepaint()
    *
    * calculates and sets optimal repaint offset for the current line
    *************************************************************************/
    long lcl_CalcOptRepaint( SwTxtFormatter &rThis,
                         SwLineLayout &rCurr,
                         const xub_StrLen nOldLineEnd,
                         const std::vector<long> &rFlyStarts )
    {
        SwTxtFormatInfo txtFmtInfo = rThis.GetInfo();
        if ( txtFmtInfo.GetIdx() < txtFmtInfo.GetReformatStart() )
        // the reformat position is behind our new line, that means
        // something of our text has moved to the next line
            return 0;

        xub_StrLen nReformat = Min( txtFmtInfo.GetReformatStart(), nOldLineEnd );

        // in case we do not have any fly in our line, our repaint position
        // is the changed position - 1
        if ( rFlyStarts.empty() && ! rCurr.IsFly() )
        {
            // this is the maximum repaint offset determined during formatting
            // for example: the beginning of the first right tab stop
            // if this value is 0, this means that we do not have an upper
            // limit for the repaint offset
            const long nFormatRepaint = txtFmtInfo.GetPaintOfst();

            if ( nReformat < txtFmtInfo.GetLineStart() + 3 )
                return 0;

            // step back two positions for smoother repaint
            nReformat -= 2;

    #ifndef QUARTZ
    #ifndef ENABLE_GRAPHITE
            // #i28795#, #i34607#, #i38388#
            // step back six(!) more characters for complex scripts
            // this is required e.g., for Khmer (thank you, Javier!)
            const SwScriptInfo& rSI = txtFmtInfo.GetParaPortion()->GetScriptInfo();
            xub_StrLen nMaxContext = 0;
            if( ::i18n::ScriptType::COMPLEX == rSI.ScriptType( nReformat ) )
                nMaxContext = 6;
    #else
            // Some Graphite fonts need context for scripts not marked as complex
            static const xub_StrLen nMaxContext = 10;
    #endif
    #else
            // some fonts like Quartz's Zapfino need more context
            // TODO: query FontInfo for maximum unicode context
            static const xub_StrLen nMaxContext = 8;
    #endif
            if( nMaxContext > 0 )
            {
                if ( nReformat > txtFmtInfo.GetLineStart() + nMaxContext )
                    nReformat = nReformat - nMaxContext;
                else
                    nReformat = txtFmtInfo.GetLineStart();
            }

            // Weird situation: Our line used to end with a hole portion
            // and we delete some characters at the end of our line. We have
            // to take care for repainting the blanks which are not anymore
            // covered by the hole portion
            while ( nReformat > txtFmtInfo.GetLineStart() &&
                    CH_BLANK == txtFmtInfo.GetChar( nReformat ) )
                --nReformat;

            OSL_ENSURE( nReformat < txtFmtInfo.GetIdx(), "Reformat too small for me!" );
            SwRect aRect;

            // Note: GetChareRect is not const. It definitely changes the
            // bMulti flag. We have to save and resore the old value.
            sal_Bool bOldMulti = txtFmtInfo.IsMulti();
            rThis.GetCharRect( &aRect, nReformat );
            txtFmtInfo.SetMulti( bOldMulti );

            return nFormatRepaint ? Min( aRect.Left(), nFormatRepaint ) :
                                    aRect.Left();
        }
        else
        {
            // nReformat may be wrong, if something around flys has changed:
            // we compare the former and the new fly positions in this line
            // if anything has changed, we carefully have to adjust the right
            // repaint position
            long nPOfst = 0;
            sal_uInt16 nCnt = 0;
            sal_uInt16 nX = 0;
            sal_uInt16 nIdx = rThis.GetInfo().GetLineStart();
            SwLinePortion* pPor = rCurr.GetFirstPortion();

            while ( pPor )
            {
                if ( pPor->IsFlyPortion() )
                {
                    // compare start of fly with former start of fly
                    if (nCnt < rFlyStarts.size() &&
                        nX == rFlyStarts[ nCnt ] &&
                        nIdx < nReformat
                    )
                        // found fix position, nothing has changed left from nX
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

    // Determine if we need to build hidden portions
    bool lcl_BuildHiddenPortion( const SwTxtSizeInfo& rInf, xub_StrLen &rPos )
    {
        // Only if hidden text should not be shown:
    //    if ( rInf.GetVsh() && rInf.GetVsh()->GetWin() && rInf.GetOpt().IsShowHiddenChar() )
        const bool bShowInDocView = rInf.GetVsh() && rInf.GetVsh()->GetWin() && rInf.GetOpt().IsShowHiddenChar();
        const bool bShowForPrinting = rInf.GetOpt().IsShowHiddenChar( sal_True ) && rInf.GetOpt().IsPrinting();
        if (bShowInDocView || bShowForPrinting)
            return false;

        const SwScriptInfo& rSI = rInf.GetParaPortion()->GetScriptInfo();
        xub_StrLen nHiddenStart;
        xub_StrLen nHiddenEnd;
        rSI.GetBoundsOfHiddenRange( rPos, nHiddenStart, nHiddenEnd );
        if ( nHiddenEnd )
        {
            rPos = nHiddenEnd;
            return true;
        }

        return false;
    }

} //end unnamed namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
