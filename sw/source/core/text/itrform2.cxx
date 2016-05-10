/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "hintids.hxx"

#include <memory>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <editeng/lspcitem.hxx>
#include <dcontact.hxx>
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
#include <xmloff/odffields.hxx>
#include <IDocumentSettingAccess.hxx>

#include <vector>

#include <config_graphite.h>

#if OSL_DEBUG_LEVEL > 1
#include <ndtxt.hxx>
#endif

using namespace ::com::sun::star;

namespace {
    //! Calculates and sets optimal repaint offset for the current line
    static long lcl_CalcOptRepaint( SwTextFormatter &rThis,
                                    SwLineLayout &rCurr,
                                    const sal_Int32 nOldLineEnd,
                                    const std::vector<long> &rFlyStarts );
    //! Determine if we need to build hidden portions
    static bool lcl_BuildHiddenPortion( const SwTextSizeInfo& rInf, sal_Int32 &rPos );

    // Check whether the two font has the same border
    static bool lcl_HasSameBorder(const SwFont& rFirst, const SwFont& rSecond);
}

inline void ClearFly( SwTextFormatInfo &rInf )
{
    delete rInf.GetFly();
    rInf.SetFly(nullptr);
}

void SwTextFormatter::CtorInitTextFormatter( SwTextFrame *pNewFrame, SwTextFormatInfo *pNewInf )
{
    CtorInitTextPainter( pNewFrame, pNewInf );
    m_pInf = pNewInf;
    pDropFormat = GetInfo().GetDropFormat();
    pMulti = nullptr;

    bOnceMore = false;
    bFlyInCntBase = false;
    bChanges = false;
    bTruncLines = false;
    nCntEndHyph = 0;
    nCntMidHyph = 0;
    nLeftScanIdx = COMPLETE_STRING;
    nRightScanIdx = 0;
    m_nHintEndIndex = 0;
    m_pFirstOfBorderMerge = nullptr;

    if( m_nStart > GetInfo().GetText().getLength() )
    {
        OSL_ENSURE( false, "+SwTextFormatter::CTOR: bad offset" );
        m_nStart = GetInfo().GetText().getLength();
    }

}

SwTextFormatter::~SwTextFormatter()
{
    // Extremely unlikely, but still possible
    // e.g.: field splits up, widows start to matter
    if( GetInfo().GetRest() )
    {
        delete GetInfo().GetRest();
        GetInfo().SetRest(nullptr);
    }
}

void SwTextFormatter::Insert( SwLineLayout *pLay )
{
    // Insert BEHIND the current element
    if ( m_pCurr )
    {
        pLay->SetNext( m_pCurr->GetNext() );
        m_pCurr->SetNext( pLay );
    }
    else
        m_pCurr = pLay;
}

sal_uInt16 SwTextFormatter::GetFrameRstHeight() const
{
    // We want the rest height relative to the page.
    // If we're in a table, then pFrame->GetUpper() is not the page.

    // GetFrameRstHeight() is being called with Footnote.
    // Wrong: const SwFrame *pUpper = pFrame->GetUpper();
    const SwFrame *pPage = static_cast<const SwFrame*>(m_pFrame->FindPageFrame());
    const SwTwips nHeight = pPage->Frame().Top()
                          + pPage->Prt().Top()
                          + pPage->Prt().Height() - Y();
    if( 0 > nHeight )
        return m_pCurr->Height();
    else
        return sal_uInt16( nHeight );
}

SwLinePortion *SwTextFormatter::Underflow( SwTextFormatInfo &rInf )
{
    // Save values and initialize rInf
    SwLinePortion *pUnderflow = rInf.GetUnderflow();
    if( !pUnderflow )
        return nullptr;

    // We format backwards, i.e. attribute changes can happen the next
    // line again.
    // Can be seen in 8081.sdw, if you enter text in the first line

    const sal_Int32 nSoftHyphPos = rInf.GetSoftHyphPos();
    const sal_Int32 nUnderScorePos = rInf.GetUnderScorePos();

    // Save flys and set to 0, or else segmentation fault
    // Not ClearFly(rInf) !
    SwFlyPortion *pFly = rInf.GetFly();
    rInf.SetFly( nullptr );

    FeedInf( rInf );
    rInf.SetLast( m_pCurr );
    // pUnderflow does not need to be deleted, because it will drown in the following
    // Truncate()
    rInf.SetUnderflow(nullptr);
    rInf.SetSoftHyphPos( nSoftHyphPos );
    rInf.SetUnderScorePos( nUnderScorePos );
    rInf.SetPaintOfst( GetLeftMargin() );

    // We look for the portion with the under-flow position
    SwLinePortion *pPor = m_pCurr->GetFirstPortion();
    if( pPor != pUnderflow )
    {
        // pPrev will be the last portion before pUnderflow,
        // which still has a real width.
        // Exception: SoftHyphPortions must not be forgotten, of course!
        // Although they don't have a width.
        SwLinePortion *pTmpPrev = pPor;
        while( pPor && pPor != pUnderflow )
        {
            if( !pPor->IsKernPortion() &&
                ( pPor->Width() || pPor->IsSoftHyphPortion() ) )
            {
                while( pTmpPrev != pPor )
                {
                    pTmpPrev->Move( rInf );
                    rInf.SetLast( pTmpPrev );
                    pTmpPrev = pTmpPrev->GetPortion();
                    OSL_ENSURE( pTmpPrev, "Underflow: losing control!" );
                };
            }
            pPor = pPor->GetPortion();
        }
        pPor = pTmpPrev;
        if( pPor && // Flies + Initialen werden nicht beim Underflow mitgenommen
            ( pPor->IsFlyPortion() || pPor->IsDropPortion() ||
              pPor->IsFlyCntPortion() ) )
        {
            pPor->Move( rInf );
            rInf.SetLast( pPor );
            rInf.SetStopUnderflow( true );
            pPor = pUnderflow;
        }
    }

    // What? The under-flow portion is not in the portion chain?
    OSL_ENSURE( pPor, "SwTextFormatter::Underflow: overflow but underflow" );

    // OD 2004-05-26 #i29529# - correction: no delete of footnotes
//    if( rInf.IsFootnoteInside() && pPor && !rInf.IsQuick() )
//    {
//        SwLinePortion *pTmp = pPor->GetPortion();
//        while( pTmp )
//        {
//            if( pTmp->IsFootnotePortion() )
//                ((SwFootnotePortion*)pTmp)->ClearFootnote();
//            pTmp = pTmp->GetPortion();
//        }
//    }

    // Snapshot
    if ( pPor==rInf.GetLast() )
    {
        // We end up here, if the portion triggering the under-flow
        // spans over the whole line. E.g. if a word spans across
        // multiple lines and flows into a fly in the second line.
        rInf.SetFly( pFly );
        pPor->Truncate();
        return pPor; // Is that enough?
    }
    // End the snapshot

    // X + Width == 0 with SoftHyph > Line?!
    if( !pPor || !(rInf.X() + pPor->Width()) )
    {
        delete pFly;
        return nullptr;
    }

    // Preparing for Format()
    // We need to chip off the chain behind pLast, because we Insert after the Format()
    SeekAndChg( rInf );

    // line width is adjusted, so that pPor does not fit to current
    // line anymore
    rInf.Width( (sal_uInt16)(rInf.X() + (pPor->Width() ? pPor->Width() - 1 : 0)) );
    rInf.SetLen( pPor->GetLen() );
    rInf.SetFull( false );
    if( pFly )
    {
        // We need to recalculate the FlyPortion due to the following reason:
        // If the base line is lowered by a big font in the middle of the line,
        // causing overlapping with a fly, the FlyPortion has a wrong size/fixed
        // size.
        rInf.SetFly( pFly );
        CalcFlyWidth( rInf );
    }
    rInf.GetLast()->SetPortion(nullptr);

    // The SwLineLayout is an exception to this, which splits at the first
    // portion change.
    // Here only the other way around:
    if( rInf.GetLast() == m_pCurr )
    {
        if( pPor->InTextGrp() && !pPor->InExpGrp() )
        {
            const sal_uInt16 nOldWhich = m_pCurr->GetWhichPor();
            *static_cast<SwLinePortion*>(m_pCurr) = *pPor;
            m_pCurr->SetPortion( pPor->GetPortion() );
            m_pCurr->SetWhichPor( nOldWhich );
            pPor->SetPortion( nullptr );
            delete pPor;
            pPor = m_pCurr;
        }
    }
    pPor->Truncate();
    SwLinePortion *const pRest( rInf.GetRest() );
    if (pRest && pRest->InFieldGrp() &&
        static_cast<SwFieldPortion*>(pRest)->IsNoLength())
    {
        // HACK: decrement again, so we pick up the suffix in next line!
        --m_nHintEndIndex;
    }
    delete pRest;
    rInf.SetRest(nullptr);
    return pPor;
}

void SwTextFormatter::InsertPortion( SwTextFormatInfo &rInf,
                                    SwLinePortion *pPor ) const
{
    // The new portion is inserted, but everything's different for
    // LineLayout...
    if( pPor == m_pCurr )
    {
        if ( m_pCurr->GetPortion() )
        {
            pPor = m_pCurr->GetPortion();
        }

        // #i112181#
        rInf.SetOtherThanFootnoteInside( rInf.IsOtherThanFootnoteInside() || !pPor->IsFootnotePortion() );
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

        rInf.SetOtherThanFootnoteInside( rInf.IsOtherThanFootnoteInside() || !pPor->IsFootnotePortion() );

        // Adjust maxima
        if( m_pCurr->Height() < pPor->Height() )
            m_pCurr->Height( pPor->Height() );
        if( m_pCurr->GetAscent() < pPor->GetAscent() )
            m_pCurr->SetAscent( pPor->GetAscent() );
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

void SwTextFormatter::BuildPortions( SwTextFormatInfo &rInf )
{
    OSL_ENSURE( rInf.GetText().getLength() < COMPLETE_STRING,
            "SwTextFormatter::BuildPortions: bad text length in info" );

    rInf.ChkNoHyph( CntEndHyph(), CntMidHyph() );

    // First NewTextPortion() decides whether pCurr ends up in pPor.
    // We need to make sure that the font is being set in any case.
    // This is done automatically in CalcAscent.
    rInf.SetLast( m_pCurr );
    rInf.ForcedLeftMargin( 0 );

    OSL_ENSURE( m_pCurr->FindLastPortion() == m_pCurr, "pLast supposed to equal pCurr" );

    if( !m_pCurr->GetAscent() && !m_pCurr->Height() )
        CalcAscent( rInf, m_pCurr );

    SeekAndChg( rInf );

    // Width() is shortened in CalcFlyWidth if we have a FlyPortion
    OSL_ENSURE( !rInf.X() || pMulti, "SwTextFormatter::BuildPortion X=0?" );
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

    // Asian grid stuff
    SwTextGridItem const*const pGrid(GetGridItem(m_pFrame->FindPageFrame()));
    const bool bHasGrid = pGrid && rInf.SnapToGrid() &&
                              GRID_LINES_CHARS == pGrid->GetGridType();

    const SwDoc *pDoc = rInf.GetTextFrame()->GetNode()->GetDoc();
    const sal_uInt16 nGridWidth = (bHasGrid) ? GetGridWidth(*pGrid, *pDoc) : 0;

    // used for grid mode only:
    // the pointer is stored, because after formatting of non-asian text,
    // the width of the kerning portion has to be adjusted
    SwKernPortion* pGridKernPortion = nullptr;

    bool bFull = false;
    SwTwips nUnderLineStart = 0;
    rInf.Y( Y() );

    while( pPor && !rInf.IsStop() )
    {
        OSL_ENSURE( rInf.GetLen() < COMPLETE_STRING &&
                rInf.GetIdx() <= rInf.GetText().getLength(),
                "SwTextFormatter::BuildPortions: bad length in info" );

        // We have to check the script for fields in order to set the
        // correct nActual value for the font.
        if( pPor->InFieldGrp() )
            static_cast<SwFieldPortion*>(pPor)->CheckScript( rInf );

        if( ! bHasGrid && rInf.HasScriptSpace() &&
            rInf.GetLast() && rInf.GetLast()->InTextGrp() &&
            rInf.GetLast()->Width() && !rInf.GetLast()->InNumberGrp() )
        {
            sal_uInt8 nNxtActual = rInf.GetFont()->GetActual();
            sal_uInt8 nLstActual = nNxtActual;
            sal_uInt16 nLstHeight = (sal_uInt16)rInf.GetFont()->GetHeight();
            bool bAllowBehind = false;
            const CharClass& rCC = GetAppCharClass();

            // are there any punctuation characters on both sides
            // of the kerning portion?
            if ( pPor->InFieldGrp() )
            {
                OUString aAltText;
                if ( static_cast<SwFieldPortion*>(pPor)->GetExpText( rInf, aAltText ) &&
                        !aAltText.isEmpty() )
                {
                    bAllowBehind = rCC.isLetterNumeric( aAltText, 0 );

                    const SwFont* pTmpFnt = static_cast<SwFieldPortion*>(pPor)->GetFont();
                    if ( pTmpFnt )
                        nNxtActual = pTmpFnt->GetActual();
                }
            }
            else
            {
                const OUString& rText = rInf.GetText();
                sal_Int32 nIdx = rInf.GetIdx();
                bAllowBehind = nIdx < rText.getLength() && rCC.isLetterNumeric(rText, nIdx);
            }

            const SwLinePortion* pLast = rInf.GetLast();
            if ( bAllowBehind && pLast )
            {
                bool bAllowBefore = false;

                if ( pLast->InFieldGrp() )
                {
                    OUString aAltText;
                    if ( static_cast<const SwFieldPortion*>(pLast)->GetExpText( rInf, aAltText ) &&
                         !aAltText.isEmpty() )
                    {
                        bAllowBefore = rCC.isLetterNumeric( aAltText, aAltText.getLength() - 1 );

                        const SwFont* pTmpFnt = static_cast<const SwFieldPortion*>(pLast)->GetFont();
                        if ( pTmpFnt )
                        {
                            nLstActual = pTmpFnt->GetActual();
                            nLstHeight = (sal_uInt16)pTmpFnt->GetHeight();
                        }
                    }
                }
                else if ( rInf.GetIdx() )
                {
                    bAllowBefore = rCC.isLetterNumeric( rInf.GetText(), rInf.GetIdx() - 1 );
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
                                           pLast->InFieldGrp() && pPor->InFieldGrp() );
                    rInf.GetLast()->SetPortion( nullptr );
                    InsertPortion( rInf, pKrn );
                }
            }
        }
        else if ( bHasGrid && ! pGridKernPortion && ! pMulti )
        {
            // insert a grid kerning portion
            if ( ! pGridKernPortion )
                pGridKernPortion = pPor->IsKernPortion() ?
                                   static_cast<SwKernPortion*>(pPor) :
                                   new SwKernPortion( *m_pCurr );

            // if we have a new GridKernPortion, we initially calculate
            // its size so that its ends on the grid
            const SwPageFrame* pPageFrame = m_pFrame->FindPageFrame();
            const SwLayoutFrame* pBody = pPageFrame->FindBodyCont();
            SWRECTFN( pPageFrame )

            const long nGridOrigin = pBody ?
                                    (pBody->*fnRect->fnGetPrtLeft)() :
                                    (pPageFrame->*fnRect->fnGetPrtLeft)();

            SwTwips nStartX = rInf.X() + GetLeftMargin();
            if ( bVert )
            {
                Point aPoint( nStartX, 0 );
                m_pFrame->SwitchHorizontalToVertical( aPoint );
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

        // the multi-portion has it's own format function
        if( pPor->IsMultiPortion() && ( !pMulti || pMulti->IsBidi() ) )
            bFull = BuildMultiPortion( rInf, *static_cast<SwMultiPortion*>(pPor) );
        else
            bFull = pPor->Format( rInf );

        if( rInf.IsRuby() && !rInf.GetRest() )
            bFull = true;

        // if we are underlined, we store the beginning of this underlined
        // segment for repaint optimization
        if ( UNDERLINE_NONE != pFnt->GetUnderline() && ! nUnderLineStart )
            nUnderLineStart = GetLeftMargin() + rInf.X();

        if ( pPor->IsFlyPortion() )
            m_pCurr->SetFly( true );
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
                     ( pPor->IsMultiPortion() && static_cast<SwMultiPortion*>(pPor)->IsBidi() ) ||
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
            static_cast<SwMultiPortion*>(pPor)->HasFlyInContent() ) )
            SetFlyInCntBase();
        // bUnderflow needs to be reset or we wrap again at the next softhyphen
        if ( !bFull )
        {
            rInf.ClrUnderflow();
            if( ! bHasGrid && rInf.HasScriptSpace() && pPor->InTextGrp() &&
                pPor->GetLen() && !pPor->InFieldGrp() )
            {
                // The distance between two different scripts is set
                // to 20% of the fontheight.
                sal_Int32 nTmp = rInf.GetIdx() + pPor->GetLen();
                if( nTmp == pScriptInfo->NextScriptChg( nTmp - 1 ) &&
                    nTmp != rInf.GetText().getLength() )
                {
                    const sal_uInt16 nDist = (sal_uInt16)(rInf.GetFont()->GetHeight()/5);

                    if( nDist )
                    {
                        // we do not want a kerning portion if any end
                        // would be a punctuation character
                        const CharClass& rCC = GetAppCharClass();
                        if ( rCC.isLetterNumeric( rInf.GetText(), nTmp - 1 ) &&
                             rCC.isLetterNumeric( rInf.GetText(), nTmp ) )
                        {
                            // does the kerning portion still fit into the line?
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

            const sal_uInt8 nCurrScript = pFnt->GetActual(); // pScriptInfo->ScriptType( rInf.GetIdx() );
            const sal_uInt8 nNextScript = nTmp >= rInf.GetText().getLength() ?
                                     SW_CJK :
                                     SwScriptInfo::WhichFont( nTmp, nullptr, pScriptInfo );

            // snap non-asian text to grid if next portion is ASIAN or
            // there are no more portions in this line
            // be careful when handling an underflow event: the gridkernportion
            // could have been deleted
            if ( nRestWidth > 0 && SW_CJK != nCurrScript &&
                ! rInf.IsUnderflow() && ( bFull || SW_CJK == nNextScript ) )
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

                const SwTwips i = nSumWidth ?
                                 ( nSumWidth - 1 ) / nGridWidth + 1 :
                                 0;
                const SwTwips nTmpWidth = i * nGridWidth;
                const SwTwips nKernWidth = std::min(nTmpWidth - nSumWidth, nRestWidth);
                const sal_uInt16 nKernWidth_1 = (sal_uInt16)(nKernWidth / 2);

                OSL_ENSURE( nKernWidth <= nRestWidth,
                        "Not enough space left for adjusting non-asian text in grid mode" );

                pGridKernPortion->Width( pGridKernPortion->Width() + nKernWidth_1 );
                rInf.X( rInf.X() + nKernWidth_1 );

                if ( ! bFull )
                    new SwKernPortion( *pPor, (short)(nKernWidth - nKernWidth_1),
                                       false, true );

                pGridKernPortion = nullptr;
            }
            else if ( pPor->IsMultiPortion() || pPor->InFixMargGrp() ||
                      pPor->IsFlyCntPortion() || pPor->InNumberGrp() ||
                      pPor->InFieldGrp() || nCurrScript != nNextScript )
                // next portion should snap to grid
                pGridKernPortion = nullptr;
        }

        rInf.SetFull( bFull );

        if( !pPor->IsDropPortion() )
            MergeCharacterBorder(*pPor, rInf);

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
    if( m_pCurr->GetPortion() && m_pCurr->GetPortion()->InNumberGrp()
        && static_cast<SwNumberPortion*>(m_pCurr->GetPortion())->IsHide() )
        rInf.SetNumDone( false );

    // Delete fly in any case
    ClearFly( rInf );

    // Reinit the tab overflow flag after the line
    rInf.SetTabOverflow( false );
}

void SwTextFormatter::CalcAdjustLine( SwLineLayout *pCurrent )
{
    if( SVX_ADJUST_LEFT != GetAdjust() && !pMulti)
    {
        pCurrent->SetFormatAdj(true);
        if( IsFlyInCntBase() )
        {
            CalcAdjLine( pCurrent );
            // For e.g. centered fly we need to switch the RefPoint
            // That's why bAlways = true
            UpdatePos( pCurrent, GetTopLeft(), GetStart(), true );
        }
    }
}

void SwTextFormatter::CalcAscent( SwTextFormatInfo &rInf, SwLinePortion *pPor )
{
    bool bCalc = false;
    if ( pPor->InFieldGrp() && static_cast<SwFieldPortion*>(pPor)->GetFont() )
    {
        // Numbering + InterNetFields can keep an own font, then their size is
        // independent from hard attribute values
        SwFont* pFieldFnt = static_cast<SwFieldPortion*>(pPor)->pFnt;
        SwFontSave aSave( rInf, pFieldFnt );
        pPor->Height( rInf.GetTextHeight() );
        pPor->SetAscent( rInf.GetAscent() );
        bCalc = true;
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
        bool bChg = false;

        // In empty lines the attributes are switched on via SeekStart
        const bool bFirstPor = rInf.GetLineStart() == rInf.GetIdx();
        if ( pPor->IsQuoVadisPortion() )
            bChg = SeekStartAndChg( rInf, true );
        else
        {
            if( bFirstPor )
            {
                if( !rInf.GetText().isEmpty() )
                {
                    if ( pPor->GetLen() || !rInf.GetIdx()
                         || ( m_pCurr != pLast && !pLast->IsFlyPortion() )
                         || !m_pCurr->IsRest() ) // instead of !rInf.GetRest()
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
                    bChg = false;
                else
                    bChg = SeekStartAndChg( rInf );
            }
            else
                bChg = SeekAndChg( rInf );
        }
        if( bChg || bFirstPor || !pPor->GetAscent()
            || !rInf.GetLast()->InTextGrp() )
        {
            pPor->SetAscent( rInf.GetAscent()  );
            pPor->Height( rInf.GetTextHeight() );
            bCalc = true;
        }
        else
        {
            pPor->Height( pLast->Height() );
            pPor->SetAscent( pLast->GetAscent() );
        }
    }

    if( pPor->InTextGrp() && bCalc )
    {
        pPor->SetAscent(pPor->GetAscent() +
            rInf.GetFont()->GetTopBorderSpace());
        pPor->Height(pPor->Height() +
            rInf.GetFont()->GetTopBorderSpace() +
            rInf.GetFont()->GetBottomBorderSpace() );
    }
}

class SwMetaPortion : public SwTextPortion
{
public:
    inline  SwMetaPortion() { SetWhichPor( POR_META ); }
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
//    OUTPUT_OPERATOR
};

void SwMetaPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if ( Width() )
    {
        rInf.DrawViewOpt( *this, POR_META );
        SwTextPortion::Paint( rInf );
    }
}

namespace sw { namespace mark {
    OUString ExpandFieldmark(IFieldmark* pBM)
    {
        const IFieldmark::parameter_map_t* const pParameters = pBM->GetParameters();
        sal_Int32 nCurrentIdx = 0;
        const IFieldmark::parameter_map_t::const_iterator pResult = pParameters->find(OUString(ODF_FORMDROPDOWN_RESULT));
        if(pResult != pParameters->end())
            pResult->second >>= nCurrentIdx;

        const IFieldmark::parameter_map_t::const_iterator pListEntries = pParameters->find(OUString(ODF_FORMDROPDOWN_LISTENTRY));
        if (pListEntries != pParameters->end())
        {
            uno::Sequence< OUString > vListEntries;
            pListEntries->second >>= vListEntries;
            if (nCurrentIdx < vListEntries.getLength())
                return vListEntries[nCurrentIdx];
        }
        return OUString();
    }
} }

SwTextPortion *SwTextFormatter::WhichTextPor( SwTextFormatInfo &rInf ) const
{
    SwTextPortion *pPor = nullptr;
    if( GetFnt()->IsTox() )
    {
        pPor = new SwToxPortion;
    }
    else if ( GetFnt()->IsInputField() )
    {
        pPor = new SwTextInputFieldPortion();
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
            // Only at the End!
            // If pCurr does not have a width, it can however already have content.
            // E.g. for non-displayable characters
            if (rInf.GetText()[rInf.GetIdx()]==CH_TXT_ATR_FIELDSTART)
                pPor = new SwFieldMarkPortion();
            else if (rInf.GetText()[rInf.GetIdx()]==CH_TXT_ATR_FIELDEND)
                pPor = new SwFieldMarkPortion();
            else if (rInf.GetText()[rInf.GetIdx()]==CH_TXT_ATR_FORMELEMENT)
            {
                SwTextNode *pNd = rInf.GetTextFrame()->GetTextNode();
                const SwDoc *doc = pNd->GetDoc();
                SwIndex aIndex(pNd, rInf.GetIdx());
                SwPosition aPosition(*pNd, aIndex);
                sw::mark::IFieldmark *pBM = doc->getIDocumentMarkAccess()->getFieldmarkFor(aPosition);
                OSL_ENSURE(pBM != nullptr, "Where is my form field bookmark???");
                if (pBM != nullptr)
                {
                    if (pBM->GetFieldname( ) == ODF_FORMCHECKBOX)
                    {
                        pPor = new SwFieldFormCheckboxPortion();
                    }
                    else if (pBM->GetFieldname( ) == ODF_FORMDROPDOWN)
                    {
                        pPor = new SwFieldFormDropDownPortion(sw::mark::ExpandFieldmark(pBM));
                    }
                    /* we need to check for ODF_FORMTEXT for scenario having FormFields inside FORMTEXT.
                     * Otherwise file will crash on open.
                     */
                    else if (pBM->GetFieldname( ) == ODF_FORMTEXT)
                    {
                        pPor = new SwFieldMarkPortion();
                    }
                    else
                    {
                        assert( false );        // unknown type...
                    }
                }
            }
            if( !pPor )
            {
                if( !rInf.X() && !m_pCurr->GetPortion() && !m_pCurr->GetLen() && !GetFnt()->IsURL() )
                    pPor = m_pCurr;
                else
                {
                    pPor = new SwTextPortion;
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

// We calculate the length, the following portion limits are defined:
// 1) Tabs
// 2) Linebreaks
// 3) CH_TXTATR_BREAKWORD / CH_TXTATR_INWORD
// 4) next attribute change

SwTextPortion *SwTextFormatter::NewTextPortion( SwTextFormatInfo &rInf )
{
    // If we're at the line's beginning, we take pCurr
    // If pCurr is not derived from SwTextPortion, we need to duplicate
    Seek( rInf.GetIdx() );
    SwTextPortion *pPor = WhichTextPor( rInf );

    // until next attribute change:
    const sal_Int32 nNextAttr = GetNextAttr();
    sal_Int32 nNextChg = std::min( nNextAttr, rInf.GetText().getLength() );

    // end of script type:
    const sal_Int32 nNextScript = pScriptInfo->NextScriptChg( rInf.GetIdx() );
    nNextChg = std::min( nNextChg, nNextScript );

    // end of direction:
    const sal_Int32 nNextDir = pScriptInfo->NextDirChg( rInf.GetIdx() );
    nNextChg = std::min( nNextChg, nNextDir );

    // Turbo boost:
    // We assume that a font's characters are not larger than twice
    // as wide as heigh.
    // Very crazy: We need to take the ascent into account.

    // Mind the trap! GetSize() contains the wished-for height, the real height
    // is only known in CalcAscent!

    // The ratio is even crazier: a blank in Times New Roman has an ascent of
    // 182, a height of 200 and a width of 53!
    // It follows that a line with a lot of blanks is processed incorrectly.
    // Therefore we increase from factor 2 to 8 (due to negative kerning).

    pPor->SetLen(1);
    CalcAscent( rInf, pPor );

    const SwFont* pTmpFnt = rInf.GetFont();
    sal_Int32 nExpect = std::min( sal_Int32( pTmpFnt->GetHeight() ),
                             sal_Int32( pPor->GetAscent() ) ) / 8;
    if ( !nExpect )
        nExpect = 1;
    nExpect = rInf.GetIdx() + ((rInf.Width() - rInf.X()) / nExpect);
    if( nExpect > rInf.GetIdx() && nNextChg > nExpect )
        nNextChg = std::min( nExpect, rInf.GetText().getLength() );

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

SwLinePortion *SwTextFormatter::WhichFirstPortion(SwTextFormatInfo &rInf)
{
    SwLinePortion *pPor = nullptr;

    if( rInf.GetRest() )
    {
        // Tabs and fields
        if( '\0' != rInf.GetHookChar() )
            return nullptr;

        pPor = rInf.GetRest();
        if( pPor->IsErgoSumPortion() )
            rInf.SetErgoDone(true);
        else
            if( pPor->IsFootnoteNumPortion() )
                rInf.SetFootnoteDone(true);
            else
                if( pPor->InNumberGrp() )
                    rInf.SetNumDone(true);

        rInf.SetRest(nullptr);
        m_pCurr->SetRest( true );
        return pPor;
    }

    // We can stand in the follow, it's crucial that
    // pFrame->GetOfst() == 0!
    if( rInf.GetIdx() )
    {
        // We now too can elongate FootnotePortions and ErgoSumPortions

        // 1. The ErgoSumTexts
        if( !rInf.IsErgoDone() )
        {
            if( m_pFrame->IsInFootnote() && !m_pFrame->GetIndPrev() )
                pPor = static_cast<SwLinePortion*>(NewErgoSumPortion( rInf ));
            rInf.SetErgoDone( true );
        }

        // 2. Arrow portions
        if( !pPor && !rInf.IsArrowDone() )
        {
            if( m_pFrame->GetOfst() && !m_pFrame->IsFollow() &&
                rInf.GetIdx() == m_pFrame->GetOfst() )
                pPor = new SwArrowPortion( *m_pCurr );
            rInf.SetArrowDone( true );
        }

        // 3. Kerning portions at beginning of line in grid mode
        if ( ! pPor && ! m_pCurr->GetPortion() )
        {
            SwTextGridItem const*const pGrid(
                    GetGridItem(GetTextFrame()->FindPageFrame()));
            if ( pGrid )
                pPor = new SwKernPortion( *m_pCurr );
        }

        // 4. The line rests (multiline fields)
        if( !pPor )
        {
            pPor = rInf.GetRest();
            // Only for pPor of course
            if( pPor )
            {
                m_pCurr->SetRest( true );
                rInf.SetRest(nullptr);
            }
        }
    }
    else
    {
        // 5. The foot note count
        if( !rInf.IsFootnoteDone() )
        {
            OSL_ENSURE( ( ! rInf.IsMulti() && ! pMulti ) || pMulti->HasRotation(),
                     "Rotated number portion trouble" );

            const bool bFootnoteNum = m_pFrame->IsFootnoteNumFrame();
            rInf.GetParaPortion()->SetFootnoteNum( bFootnoteNum );
            if( bFootnoteNum )
                pPor = static_cast<SwLinePortion*>(NewFootnoteNumPortion( rInf ));
            rInf.SetFootnoteDone( true );
        }

        // 6. The ErgoSumTexts of course also exist in the TextMaster,
        // it's crucial whether the SwFootnoteFrame is aFollow
        if( !rInf.IsErgoDone() && !pPor && ! rInf.IsMulti() )
        {
            if( m_pFrame->IsInFootnote() && !m_pFrame->GetIndPrev() )
                pPor = static_cast<SwLinePortion*>(NewErgoSumPortion( rInf ));
            rInf.SetErgoDone( true );
        }

        // 7. The numbering
        if( !rInf.IsNumDone() && !pPor )
        {
            OSL_ENSURE( ( ! rInf.IsMulti() && ! pMulti ) || pMulti->HasRotation(),
                     "Rotated number portion trouble" );

            // If we're in the follow, then of course not
            if( GetTextFrame()->GetTextNode()->GetNumRule() )
                pPor = static_cast<SwLinePortion*>(NewNumberPortion( rInf ));
            rInf.SetNumDone( true );
        }
        // 8. The DropCaps
        if( !pPor && GetDropFormat() && ! rInf.IsMulti() )
            pPor = static_cast<SwLinePortion*>(NewDropPortion( rInf ));

        // 9. Kerning portions at beginning of line in grid mode
        if ( !pPor && !m_pCurr->GetPortion() )
        {
            SwTextGridItem const*const pGrid(
                    GetGridItem(GetTextFrame()->FindPageFrame()));
            if ( pGrid )
                pPor = new SwKernPortion( *m_pCurr );
        }
    }

        // 10. Decimal tab portion at the beginning of each line in table cells
        if ( !pPor && !m_pCurr->GetPortion() &&
             GetTextFrame()->IsInTab() &&
             GetTextFrame()->GetTextNode()->getIDocumentSettingAccess()->get(DocumentSettingId::TAB_COMPAT) )
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

static bool lcl_OldFieldRest( const SwLineLayout* pCurr )
{
    if( !pCurr->GetNext() )
        return false;
    const SwLinePortion *pPor = pCurr->GetNext()->GetPortion();
    bool bRet = false;
    while( pPor && !bRet )
    {
        bRet = (pPor->InFieldGrp() && static_cast<const SwFieldPortion*>(pPor)->IsFollow()) ||
            (pPor->IsMultiPortion() && static_cast<const SwMultiPortion*>(pPor)->IsFollowField());
        if( !pPor->GetLen() )
            break;
        pPor = pPor->GetPortion();
    }
    return bRet;
}

/* NewPortion sets rInf.nLen
 * A SwTextPortion is limited by a tab, break, txtatr or attr change
 * We can have three cases:
 * 1) The line is full and the wrap was not emulated
 *    -> return 0;
 * 2) The line is full and a wrap was emulated
 *    -> Reset width and return new FlyPortion
 * 3) We need to construct a new portion
 *    -> CalcFlyWidth emulates the width and return portion, if needed
 */

SwLinePortion *SwTextFormatter::NewPortion( SwTextFormatInfo &rInf )
{
    // Underflow takes precedence
    rInf.SetStopUnderflow( false );
    if( rInf.GetUnderflow() )
    {
        OSL_ENSURE( rInf.IsFull(), "SwTextFormatter::NewPortion: underflow but not full" );
        return Underflow( rInf );
    }

    // If the line is full, flys and Underflow portions could be waiting ...
    if( rInf.IsFull() )
    {
        // LineBreaks and Flys (bug05.sdw)
        // IsDummy()
        if( rInf.IsNewLine() && (!rInf.GetFly() || !m_pCurr->IsDummy()) )
            return nullptr;

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
                rInf.SetFly( nullptr );
            }

            return rInf.GetFly();
        }
        // Ein fieser Sonderfall: ein Rahmen ohne Umlauf kreuzt den
        // Footnote-Bereich. Wir muessen die Footnote-Portion als Zeilenrest
        // bekanntgeben, damit SwTextFrame::Format nicht abbricht
        // (die Textmasse wurde ja durchformatiert).
        if( rInf.GetRest() )
            rInf.SetNewLine( true );
        else
        {
            // Wenn die naechste Zeile mit einem Rest eines Feldes beginnt,
            // jetzt aber kein Rest mehr anliegt,
            // muss sie auf jeden Fall neu formatiert werden!
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

        return nullptr;
    }

    SwLinePortion *pPor = WhichFirstPortion( rInf );

    // Check for Hidden Portion:
    if ( !pPor )
    {
        sal_Int32 nEnd = rInf.GetIdx();
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
            sal_Int32 nEnd = rInf.GetIdx();
            SwMultiCreator* pCreate = rInf.GetMultiCreator( nEnd, pMulti );
            if( pCreate )
            {
                SwMultiPortion* pTmp = nullptr;

                if ( SW_MC_BIDI == pCreate->nId )
                    pTmp = new SwBidiPortion( nEnd, pCreate->nLevel );
                else if ( SW_MC_RUBY == pCreate->nId )
                {
                    Seek( rInf.GetIdx() );
                    bool bRubyTop = false;
                    bool* pRubyPos = nullptr;

                    if ( rInf.SnapToGrid() )
                    {
                        SwTextGridItem const*const pGrid(
                                GetGridItem(GetTextFrame()->FindPageFrame()));
                        if ( pGrid )
                        {
                            bRubyTop = ! pGrid->GetRubyTextBelow();
                            pRubyPos = &bRubyTop;
                        }
                    }

                    pTmp = new SwRubyPortion( *pCreate, *rInf.GetFont(),
                                              *GetTextFrame()->GetTextNode()->getIDocumentSettingAccess(),
                                              nEnd, 0, pRubyPos );
                }
                else if( SW_MC_ROTATE == pCreate->nId )
                    pTmp = new SwRotatedPortion( *pCreate, nEnd,
                                                 GetTextFrame()->IsRightToLeft() );
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
             * Wenn allerdings eine FieldPortion im Rest wartet, muessen wir
             * das cChar natuerlich aus dem Feldinhalt holen, z.B. bei
             * DezimalTabs und Feldern (22615)
            */
            if( !rInf.GetRest() || !rInf.GetRest()->InFieldGrp() )
                cChar = rInf.GetChar( rInf.GetIdx() );
            rInf.ClearHookChar();
        }
        else
        {
            if( rInf.GetIdx() >= rInf.GetText().getLength() )
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

            case CHAR_SOFTHYPHEN:                   // soft hyphen
                pPor = new SwSoftHyphPortion; break;

            case CHAR_HARDBLANK:                    // no-break space
                pPor = new SwBlankPortion( ' ' ); break;

            case CHAR_HARDHYPHEN:               // non-breaking hyphen
                pPor = new SwBlankPortion( '-' ); break;

            case CHAR_ZWSP:                     // zero width space
            case CHAR_ZWNBSP :                  // word joiner
                pPor = new SwControlCharPortion( cChar ); break;

            case CH_TXTATR_BREAKWORD:
            case CH_TXTATR_INWORD:
                if( rInf.HasHint( rInf.GetIdx() ) )
                {
                    pPor = NewExtraPortion( rInf );
                    break;
                }
                SAL_FALLTHROUGH;
            default        :
                {
                SwTabPortion* pLastTabPortion = rInf.GetLastTab();
                if ( pLastTabPortion && cChar == rInf.GetTabDecimal() )
                {
                    // #127428# Abandon dec. tab position if line is full
                    // We have a decimal tab portion in the line and the next character has to be
                    // aligned at the tab stop position. We store the width from the beginning of
                    // the tab stop portion up to the portion containing the decimal separator:
                  if ( GetTextFrame()->GetTextNode()->getIDocumentSettingAccess()->get(DocumentSettingId::TAB_COMPAT) /*rInf.GetVsh()->IsTabCompat();*/ &&
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
                        return nullptr;
                    }
                    pPor = rInf.GetRest();
                    rInf.SetRest(nullptr);
                }
                else
                {
                    if( rInf.IsFull() )
                        return nullptr;
                    pPor = NewTextPortion( rInf );
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
            return nullptr;
        }
    }

    // Special portions containing numbers (footnote anchor, footnote number,
    // numbering) can be contained in a rotated portion, if the user
    // choose a rotated character attribute.
    if ( pPor && ! pMulti )
    {
        if ( pPor->IsFootnotePortion() )
        {
            const SwTextFootnote* pTextFootnote = static_cast<SwFootnotePortion*>(pPor)->GetTextFootnote();

            if ( pTextFootnote )
            {
                SwFormatFootnote& rFootnote = (SwFormatFootnote&)pTextFootnote->GetFootnote();
                const SwDoc *pDoc = rInf.GetTextFrame()->GetNode()->GetDoc();
                const SwEndNoteInfo* pInfo;
                if( rFootnote.IsEndNote() )
                    pInfo = &pDoc->GetEndNoteInfo();
                else
                    pInfo = &pDoc->GetFootnoteInfo();
                const SwAttrSet& rSet = pInfo->GetAnchorCharFormat((SwDoc&)*pDoc)->GetAttrSet();

                const SfxPoolItem* pItem;
                sal_uInt16 nDir = 0;
                if( SfxItemState::SET == rSet.GetItemState( RES_CHRATR_ROTATE,
                    true, &pItem ))
                    nDir = static_cast<const SvxCharRotateItem*>(pItem)->GetValue();

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
            const SwFont* pNumFnt = static_cast<SwFieldPortion*>(pPor)->GetFont();

            if ( pNumFnt )
            {
                sal_uInt16 nDir = pNumFnt->GetOrientation( rInf.GetTextFrame()->IsVertical() );
                if ( 0 != nDir )
                {
                    delete pPor;
                    pPor = new SwRotatedPortion( 0, 900 == nDir ?
                                                    DIR_BOTTOM2TOP :
                                                    DIR_TOP2BOTTOM );

                    rInf.SetNumDone( false );
                    rInf.SetFootnoteDone( false );
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
    if( !m_pCurr->Height() )
    {
        OSL_ENSURE( m_pCurr->Height(), "SwTextFormatter::NewPortion: limbo dance" );
        m_pCurr->Height( pPor->Height() );
        m_pCurr->SetAscent( pPor->GetAscent() );
    }

    OSL_ENSURE( !pPor || pPor->Height(),
            "SwTextFormatter::NewPortion: something went wrong");
    if( pPor->IsPostItsPortion() && rInf.X() >= rInf.Width() && rInf.GetFly() )
    {
        delete pPor;
        pPor = rInf.GetFly();
    }
    return pPor;
}

sal_Int32 SwTextFormatter::FormatLine(const sal_Int32 nStartPos)
{
    OSL_ENSURE( ! m_pFrame->IsVertical() || m_pFrame->IsSwapped(),
            "SwTextFormatter::FormatLine( nStartPos ) with unswapped frame" );

    // For the formatting routines, we set pOut to the reference device.
    SwHookOut aHook( GetInfo() );
    if( GetInfo().GetLen() < GetInfo().GetText().getLength() )
        GetInfo().SetLen( GetInfo().GetText().getLength() );

    bool bBuild = true;
    SetFlyInCntBase( false );
    GetInfo().SetLineHeight( 0 );
    GetInfo().SetLineNetHeight( 0 );

    // Recycling muss bei geaenderter Zeilenhoehe unterdrueckt werden
    // und auch bei geaendertem Ascent (Absenken der Grundlinie).
    const sal_uInt16 nOldHeight = m_pCurr->Height();
    const sal_uInt16 nOldAscent = m_pCurr->GetAscent();

    m_pCurr->SetEndHyph( false );
    m_pCurr->SetMidHyph( false );

    // fly positioning can make it necessary format a line several times
    // for this, we have to keep a copy of our rest portion
    SwLinePortion* pField = GetInfo().GetRest();
    std::unique_ptr<SwFieldPortion> xSaveField;

    if ( pField && pField->InFieldGrp() && !pField->IsFootnotePortion() )
        xSaveField.reset(new SwFieldPortion( *static_cast<SwFieldPortion*>(pField) ));

    // for an optimal repaint rectangle, we want to compare fly portions
    // before and after the BuildPortions call
    const bool bOptimizeRepaint = AllowRepaintOpt();
    const sal_Int32 nOldLineEnd = nStartPos + m_pCurr->GetLen();
    std::vector<long> flyStarts;

    // these are the conditions for a fly position comparison
    if ( bOptimizeRepaint && m_pCurr->IsFly() )
    {
        SwLinePortion* pPor = m_pCurr->GetFirstPortion();
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
        GetInfo().SetFootnoteInside( false );
        GetInfo().SetOtherThanFootnoteInside( false );

        // These values must not be reset by FormatReset();
        const bool bOldNumDone = GetInfo().IsNumDone();
        const bool bOldArrowDone = GetInfo().IsArrowDone();
        const bool bOldErgoDone = GetInfo().IsErgoDone();

        // besides other things, this sets the repaint offset to 0
        FormatReset( GetInfo() );

        GetInfo().SetNumDone( bOldNumDone );
        GetInfo().SetArrowDone( bOldArrowDone );
        GetInfo().SetErgoDone( bOldErgoDone );

        // build new portions for this line
        BuildPortions( GetInfo() );

        if( GetInfo().IsStop() )
        {
            m_pCurr->SetLen( 0 );
            m_pCurr->Height( GetFrameRstHeight() + 1 );
            m_pCurr->SetRealHeight( GetFrameRstHeight() + 1 );
            m_pCurr->Width(0);
            m_pCurr->Truncate();
            return nStartPos;
        }
        else if( GetInfo().IsDropInit() )
        {
            DropInit();
            GetInfo().SetDropInit( false );
        }

        m_pCurr->CalcLine( *this, GetInfo() );
        CalcRealHeight( GetInfo().IsNewLine() );

        //#i120864# For Special case that at the first calculation couldn't get
        //correct height. And need to recalculate for the right height.
        SwLinePortion* pPorTmp = m_pCurr->GetPortion();
        if ( IsFlyInCntBase() && (!IsQuick() || (pPorTmp && pPorTmp->IsFlyCntPortion() && !pPorTmp->GetPortion() &&
            m_pCurr->Height() > pPorTmp->Height())))
        {
            sal_uInt16 nTmpAscent, nTmpHeight;
            CalcAscentAndHeight( nTmpAscent, nTmpHeight );
            AlignFlyInCntBase( Y() + long( nTmpAscent ) );
            m_pCurr->CalcLine( *this, GetInfo() );
            CalcRealHeight();
        }

        // bBuild entscheidet, ob noch eine Ehrenrunde gedreht wird
        if ( m_pCurr->GetRealHeight() <= GetInfo().GetLineHeight() )
        {
            m_pCurr->SetRealHeight( GetInfo().GetLineHeight() );
            bBuild = false;
        }
        else
        {
            bBuild = ( GetInfo().GetTextFly().IsOn() && ChkFlyUnderflow(GetInfo()) )
                     || GetInfo().CheckFootnotePortion(m_pCurr);
            if( bBuild )
            {
                GetInfo().SetNumDone( bOldNumDone );
                GetInfo().ResetMaxWidthDiff();

                // delete old rest
                if ( GetInfo().GetRest() )
                {
                    delete GetInfo().GetRest();
                    GetInfo().SetRest( nullptr );
                }

                // set original rest portion
                if ( xSaveField )
                    GetInfo().SetRest( new SwFieldPortion( *xSaveField ) );

                m_pCurr->SetLen( 0 );
                m_pCurr->Width(0);
                m_pCurr->Truncate();
            }
        }
    }

    // In case of compat mode, it's possible that a tab portion is wider after
    // formatting than before. If this is the case, we also have to make sure
    // the SwLineLayout is wider as well.
    if (GetInfo().GetTextFrame()->GetTextNode()->getIDocumentSettingAccess()->get(DocumentSettingId::TAB_OVER_MARGIN))
    {
        sal_uInt16 nSum = 0;
        SwLinePortion* pPor = m_pCurr->GetFirstPortion();

        while (pPor)
        {
            nSum += pPor->Width();
            pPor = pPor->GetPortion();
        }

        if (nSum > m_pCurr->Width())
            m_pCurr->Width(nSum);
    }

    // calculate optimal repaint rectangle
    if ( bOptimizeRepaint )
    {
        GetInfo().SetPaintOfst( ::lcl_CalcOptRepaint( *this, *m_pCurr, nOldLineEnd, flyStarts ) );
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
    GetInfo().GetParaPortion()->GetReformat().LeftMove( GetInfo().GetIdx() );

    // delete master copy of rest portion
    xSaveField.reset();

    sal_Int32 nNewStart = nStartPos + m_pCurr->GetLen();

    // adjust text if kana compression is enabled
    if ( GetInfo().CompressLine() )
    {
        SwTwips nRepaintOfst = CalcKanaAdj( m_pCurr );

        // adjust repaint offset
        if ( nRepaintOfst < GetInfo().GetPaintOfst() )
            GetInfo().SetPaintOfst( nRepaintOfst );
    }

    CalcAdjustLine( m_pCurr );

    if( nOldHeight != m_pCurr->Height() || nOldAscent != m_pCurr->GetAscent() )
    {
        SetFlyInCntBase();
        GetInfo().SetPaintOfst( 0 ); //geaenderte Zeilenhoehe => kein Recycling
        // alle weiteren Zeilen muessen gepaintet und, wenn Flys im Spiel sind
        // auch formatiert werden.
        GetInfo().SetShift( true );
    }

    if ( IsFlyInCntBase() && !IsQuick() )
        UpdatePos( m_pCurr, GetTopLeft(), GetStart() );

    return nNewStart;
}

void SwTextFormatter::RecalcRealHeight()
{
    do
    {
        CalcRealHeight();
    } while (Next());
}

void SwTextFormatter::CalcRealHeight( bool bNewLine )
{
    sal_uInt16 nLineHeight = m_pCurr->Height();
    m_pCurr->SetClipping( false );

    SwTextGridItem const*const pGrid(GetGridItem(m_pFrame->FindPageFrame()));
    if ( pGrid && GetInfo().SnapToGrid() )
    {
        const sal_uInt16 nGridWidth = pGrid->GetBaseHeight();
        const sal_uInt16 nRubyHeight = pGrid->GetRubyHeight();
        const bool bRubyTop = ! pGrid->GetRubyTextBelow();

        nLineHeight = nGridWidth + nRubyHeight;
        const sal_uInt16 nAmpRatio = (m_pCurr->Height() + nLineHeight - 1)/nLineHeight;
        nLineHeight *= nAmpRatio;

        const sal_uInt16 nAsc = m_pCurr->GetAscent() +
                      ( bRubyTop ?
                       ( nLineHeight - m_pCurr->Height() + nRubyHeight ) / 2 :
                       ( nLineHeight - m_pCurr->Height() - nRubyHeight ) / 2 );

        m_pCurr->Height( nLineHeight );
        m_pCurr->SetAscent( nAsc );
        m_pInf->GetParaPortion()->SetFixLineHeight();

        // we ignore any line spacing options except from ...
        const SvxLineSpacingItem* pSpace = m_aLineInf.GetLineSpacing();
        if ( ! IsParaLine() && pSpace &&
             SVX_INTER_LINE_SPACE_PROP == pSpace->GetInterLineSpaceRule() )
        {
            sal_uLong nTmp = pSpace->GetPropLineSpace();

            if( nTmp < 100 )
                nTmp = 100;

            nTmp *= nLineHeight;
            nLineHeight = (sal_uInt16)(nTmp / 100);
        }

        m_pCurr->SetRealHeight( nLineHeight );
        return;
    }

    // Das Dummyflag besitzen Zeilen, die nur Flyportions enthalten, diese
    // sollten kein Register etc. beachten. Dummerweise hat kann es eine leere
    // Zeile am Absatzende geben (bei leeren Abs?tzen oder nach einem
    // Shift-Return), die das Register durchaus beachten soll.
    if( !m_pCurr->IsDummy() || ( !m_pCurr->GetNext() &&
        GetStart() >= GetTextFrame()->GetText().getLength() && !bNewLine ) )
    {
        const SvxLineSpacingItem *pSpace = m_aLineInf.GetLineSpacing();
        if( pSpace )
        {
            switch( pSpace->GetLineSpaceRule() )
            {
                case SVX_LINE_SPACE_AUTO:
                    // shrink first line of paragraph too on spacing < 100%
                    if (IsParaLine() &&
                        pSpace->GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_PROP
                        && GetTextFrame()->GetTextNode()->getIDocumentSettingAccess()->get(DocumentSettingId::PROP_LINE_SPACING_SHRINKS_FIRST_LINE))
                    {
                        long nTmp = pSpace->GetPropLineSpace();
                        // Word will render < 50% too but it's just not readable
                        if( nTmp < 50 )
                            nTmp = nTmp ? 50 : 100;
                        if (nTmp<100) { // code adapted from fixed line height
                            nTmp *= nLineHeight;
                            nTmp /= 100;
                            if( !nTmp )
                                ++nTmp;
                            nLineHeight = (sal_uInt16)nTmp;
                            sal_uInt16 nAsc = ( 4 * nLineHeight ) / 5;  // 80%
#if 0
                            // could do clipping here (like Word does)
                            // but at 0.5 its unreadable either way...
                            if( nAsc < pCurr->GetAscent() ||
                                nLineHeight - nAsc < pCurr->Height() -
                                pCurr->GetAscent() )
                                pCurr->SetClipping( true );
#endif
                            m_pCurr->SetAscent( nAsc );
                            m_pCurr->Height( nLineHeight );
                            m_pInf->GetParaPortion()->SetFixLineHeight();
                        }
                    }
                break;
                case SVX_LINE_SPACE_MIN:
                {
                    if( nLineHeight < sal_uInt16( pSpace->GetLineHeight() ) )
                        nLineHeight = pSpace->GetLineHeight();
                    break;
                }
                case SVX_LINE_SPACE_FIX:
                {
                    nLineHeight = pSpace->GetLineHeight();
                    const sal_uInt16 nAsc = ( 4 * nLineHeight ) / 5;  // 80%
                    if( nAsc < m_pCurr->GetAscent() ||
                        nLineHeight - nAsc < m_pCurr->Height() - m_pCurr->GetAscent() )
                        m_pCurr->SetClipping( true );
                    m_pCurr->Height( nLineHeight );
                    m_pCurr->SetAscent( nAsc );
                    m_pInf->GetParaPortion()->SetFixLineHeight();
                }
                break;
                default: OSL_FAIL( ": unknown LineSpaceRule" );
            }
            // Note: for the _first_ line the line spacing of the previous
            // paragraph is applied in SwFlowFrame::CalcUpperSpace()
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
                        nLineHeight = (sal_uInt16)nTmp;
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
        sal_uInt16 nDummy = nLineHeight + 1;
        (void)nDummy;
#endif

        if( IsRegisterOn() )
        {
            SwTwips nTmpY = Y() + m_pCurr->GetAscent() + nLineHeight - m_pCurr->Height();
            SWRECTFN( m_pFrame )
            if ( bVert )
                nTmpY = m_pFrame->SwitchHorizontalToVertical( nTmpY );
            nTmpY = (*fnRect->fnYDiff)( nTmpY, RegStart() );
            const sal_uInt16 nDiff = sal_uInt16( nTmpY % RegDiff() );
            if( nDiff )
                nLineHeight += RegDiff() - nDiff;
        }
    }
    m_pCurr->SetRealHeight( nLineHeight );
}

void SwTextFormatter::FeedInf( SwTextFormatInfo &rInf ) const
{
    // 3260, 3860: Fly auf jeden Fall loeschen!
    ClearFly( rInf );
    rInf.Init();

    rInf.ChkNoHyph( CntEndHyph(), CntMidHyph() );
    rInf.SetRoot( m_pCurr );
    rInf.SetLineStart( m_nStart );
    rInf.SetIdx( m_nStart );

    // Handle overflows:
    // #i34348# Changed type from sal_uInt16 to SwTwips
    SwTwips nTmpLeft = Left();
    SwTwips nTmpRight = Right();
    SwTwips nTmpFirst = FirstLeft();

    if ( nTmpLeft > USHRT_MAX ||
         nTmpRight > USHRT_MAX ||
         nTmpFirst > USHRT_MAX )
    {
        SWRECTFN( rInf.GetTextFrame() )
        nTmpLeft = (rInf.GetTextFrame()->Frame().*fnRect->fnGetLeft)();
        nTmpRight = (rInf.GetTextFrame()->Frame().*fnRect->fnGetRight)();
        nTmpFirst = nTmpLeft;
    }

    rInf.Left(  nTmpLeft  );
    rInf.Right( nTmpRight );
    rInf.First( nTmpFirst );

    rInf.RealWidth( sal_uInt16(rInf.Right() - GetLeftMargin()) );
    rInf.Width( rInf.RealWidth() );
    if( const_cast<SwTextFormatter*>(this)->GetRedln() )
    {
        const_cast<SwTextFormatter*>(this)->GetRedln()->Clear( const_cast<SwTextFormatter*>(this)->GetFnt() );
        const_cast<SwTextFormatter*>(this)->GetRedln()->Reset();
    }
}

void SwTextFormatter::FormatReset( SwTextFormatInfo &rInf )
{
    m_pCurr->Truncate();
    m_pCurr->Init();
    if( pBlink && m_pCurr->IsBlinking() )
        pBlink->Delete( m_pCurr );

    // delete pSpaceAdd und pKanaComp
    m_pCurr->FinishSpaceAdd();
    m_pCurr->FinishKanaComp();
    m_pCurr->ResetFlags();
    FeedInf( rInf );
}

bool SwTextFormatter::CalcOnceMore()
{
    if( pDropFormat )
    {
        const sal_uInt16 nOldDrop = GetDropHeight();
        CalcDropHeight( pDropFormat->GetLines() );
        bOnceMore = nOldDrop != GetDropHeight();
    }
    else
        bOnceMore = false;
    return bOnceMore;
}

SwTwips SwTextFormatter::CalcBottomLine() const
{
    SwTwips nRet = Y() + GetLineHeight();
    SwTwips nMin = GetInfo().GetTextFly().GetMinBottom();
    if( nMin && ++nMin > nRet )
    {
        SwTwips nDist = m_pFrame->Frame().Height() - m_pFrame->Prt().Height()
                        - m_pFrame->Prt().Top();
        if( nRet + nDist < nMin )
        {
            const bool bRepaint = HasTruncLines() &&
                GetInfo().GetParaPortion()->GetRepaint().Bottom() == nRet-1;
            nRet = nMin - nDist;
            if( bRepaint )
            {
                ((SwRepaint&)GetInfo().GetParaPortion()
                    ->GetRepaint()).Bottom( nRet-1 );
                ((SwTextFormatInfo&)GetInfo()).SetPaintOfst( 0 );
            }
        }
    }
    return nRet;
}

// FME/OD: This routine does a limited text formatting.
SwTwips SwTextFormatter::_CalcFitToContent()
{
    FormatReset( GetInfo() );
    BuildPortions( GetInfo() );
    m_pCurr->CalcLine( *this, GetInfo() );
    return m_pCurr->Width();
}

// determines if the calculation of a repaint offset is allowed
// otherwise each line is painted from 0 (this is a copy of the beginning
// of the former SwTextFormatter::Recycle() function
bool SwTextFormatter::AllowRepaintOpt() const
{
    // reformat position in front of current line? Only in this case
    // we want to set the repaint offset
    bool bOptimizeRepaint = m_nStart < GetInfo().GetReformatStart() &&
                                m_pCurr->GetLen();

    // a special case is the last line of a block adjusted paragraph:
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
                // ????: Blank in der letzten Masterzeile (blocksat.sdw)
                bOptimizeRepaint = nullptr == m_pCurr->GetNext() && !m_pFrame->GetFollow();
                if ( bOptimizeRepaint )
                {
                    SwLinePortion *pPos = m_pCurr->GetFirstPortion();
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

    // Schon wieder ein Sonderfall: unsichtbare SoftHyphs
    const sal_Int32 nReformat = GetInfo().GetReformatStart();
    if( bOptimizeRepaint && COMPLETE_STRING != nReformat )
    {
        const sal_Unicode cCh = nReformat >= GetInfo().GetText().getLength() ? 0 : GetInfo().GetText()[ nReformat ];
        bOptimizeRepaint = ( CH_TXTATR_BREAKWORD != cCh && CH_TXTATR_INWORD != cCh )
                            || ! GetInfo().HasHint( nReformat );
    }

    return bOptimizeRepaint;
}

void SwTextFormatter::CalcUnclipped( SwTwips& rTop, SwTwips& rBottom )
{
    OSL_ENSURE( ! m_pFrame->IsVertical() || m_pFrame->IsSwapped(),
            "SwTextFormatter::CalcUnclipped with unswapped frame" );

    long nFlyAsc, nFlyDesc;
    m_pCurr->MaxAscentDescent( rTop, rBottom, nFlyAsc, nFlyDesc );
    rTop = Y() + GetCurr()->GetAscent();
    rBottom = rTop + nFlyDesc;
    rTop -= nFlyAsc;
}

void SwTextFormatter::UpdatePos( SwLineLayout *pCurrent, Point aStart,
    sal_Int32 nStartIdx, bool bAlways ) const
{
    OSL_ENSURE( ! m_pFrame->IsVertical() || m_pFrame->IsSwapped(),
            "SwTextFormatter::UpdatePos with unswapped frame" );

    if( GetInfo().IsTest() )
        return;
    SwLinePortion *pFirst = pCurrent->GetFirstPortion();
    SwLinePortion *pPos = pFirst;
    SwTextPaintInfo aTmpInf( GetInfo() );
    aTmpInf.SetpSpaceAdd( pCurrent->GetpLLSpaceAdd() );
    aTmpInf.ResetSpaceIdx();
    aTmpInf.SetKanaComp( pCurrent->GetpKanaComp() );
    aTmpInf.ResetKanaIdx();

    // The frame's size
    aTmpInf.SetIdx( nStartIdx );
    aTmpInf.SetPos( aStart );

    long nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc;
    pCurrent->MaxAscentDescent( nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc );

    const sal_uInt16 nTmpHeight = pCurrent->GetRealHeight();
    sal_uInt16 nAscent = pCurrent->GetAscent() + nTmpHeight - pCurrent->Height();
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
                static_cast<SwGrfNumPortion*>(pPos)->SetBase( nTmpAscent, nTmpDescent,
                                                   nFlyAsc, nFlyDesc );
            }
            else
            {
                Point aBase( aTmpInf.GetPos() );
                if ( GetInfo().GetTextFrame()->IsVertical() )
                    GetInfo().GetTextFrame()->SwitchHorizontalToVertical( aBase );

                static_cast<SwFlyCntPortion*>(pPos)->SetBase( *aTmpInf.GetTextFrame(),
                    aBase, nTmpAscent, nTmpDescent, nFlyAsc,
                    nFlyDesc, nFlags );
            }
        }
        if( pPos->IsMultiPortion() && static_cast<SwMultiPortion*>(pPos)->HasFlyInContent() )
        {
            OSL_ENSURE( !GetMulti(), "Too much multi" );
            const_cast<SwTextFormatter*>(static_cast<const SwTextFormatter*>(this))->pMulti = static_cast<SwMultiPortion*>(pPos);
            SwLineLayout *pLay = &GetMulti()->GetRoot();
            Point aSt( aTmpInf.X(), aStart.Y() );

            if ( GetMulti()->HasBrackets() )
            {
                OSL_ENSURE( GetMulti()->IsDouble(), "Brackets only for doubles");
                aSt.X() += static_cast<SwDoubleLinePortion*>(GetMulti())->PreWidth();
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

            sal_Int32 nStIdx = aTmpInf.GetIdx();
            do
            {
                UpdatePos( pLay, aSt, nStIdx, bAlways );
                nStIdx = nStIdx + pLay->GetLen();
                aSt.Y() += pLay->Height();
                pLay = pLay->GetNext();
            } while ( pLay );
            const_cast<SwTextFormatter*>(this)->pMulti = nullptr;
        }
        pPos->Move( aTmpInf );
        pPos = pPos->GetPortion();
    }
}

void SwTextFormatter::AlignFlyInCntBase( long nBaseLine ) const
{
    OSL_ENSURE( ! m_pFrame->IsVertical() || m_pFrame->IsSwapped(),
            "SwTextFormatter::AlignFlyInCntBase with unswapped frame" );

    if( GetInfo().IsTest() )
        return;
    SwLinePortion *pFirst = m_pCurr->GetFirstPortion();
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
            m_pCurr->MaxAscentDescent( nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc, pPos );

            if( pPos->IsGrfNumPortion() )
                static_cast<SwGrfNumPortion*>(pPos)->SetBase( nTmpAscent, nTmpDescent,
                                                   nFlyAsc, nFlyDesc );
            else
            {
                Point aBase;
                if ( GetInfo().GetTextFrame()->IsVertical() )
                {
                    nBaseLine = GetInfo().GetTextFrame()->SwitchHorizontalToVertical( nBaseLine );
                    aBase = Point( nBaseLine, static_cast<SwFlyCntPortion*>(pPos)->GetRefPoint().Y() );
                }
                else
                    aBase = Point( static_cast<SwFlyCntPortion*>(pPos)->GetRefPoint().X(), nBaseLine );

                static_cast<SwFlyCntPortion*>(pPos)->SetBase( *GetInfo().GetTextFrame(), aBase, nTmpAscent, nTmpDescent,
                    nFlyAsc, nFlyDesc, nFlags );
            }
        }
        pPos = pPos->GetPortion();
    }
}

bool SwTextFormatter::ChkFlyUnderflow( SwTextFormatInfo &rInf ) const
{
    OSL_ENSURE( rInf.GetTextFly().IsOn(), "SwTextFormatter::ChkFlyUnderflow: why?" );
    if( GetCurr() )
    {
        // First we check, whether a fly overlaps with the line.
        // = GetLineHeight()
        const sal_uInt16 nHeight = GetCurr()->GetRealHeight();
        SwRect aLine( GetLeftMargin(), Y(), rInf.RealWidth(), nHeight );

        SwRect aLineVert( aLine );
        if ( m_pFrame->IsVertical() )
            m_pFrame->SwitchHorizontalToVertical( aLineVert );
        SwRect aInter( rInf.GetTextFly().GetFrame( aLineVert ) );
        if ( m_pFrame->IsVertical() )
            m_pFrame->SwitchVerticalToHorizontal( aInter );

        if( !aInter.HasArea() )
            return false;

        // We now check every portion that could have lowered for overlapping
        // with the fly.
        const SwLinePortion *pPos = GetCurr()->GetFirstPortion();
        aLine.Pos().Y() = Y() + GetCurr()->GetRealHeight() - GetCurr()->Height();
        aLine.Height( GetCurr()->Height() );

        while( pPos )
        {
            aLine.Width( pPos->Width() );

            aLineVert = aLine;
            if ( m_pFrame->IsVertical() )
                m_pFrame->SwitchHorizontalToVertical( aLineVert );
            aInter = rInf.GetTextFly().GetFrame( aLineVert );
            if ( m_pFrame->IsVertical() )
                m_pFrame->SwitchVerticalToHorizontal( aInter );

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
                        rInf.SetLineHeight( nHeight );
                        // Height without extra spacing
                        rInf.SetLineNetHeight( m_pCurr->Height() );
                        return true;
                    }
                }
            }
            else
            {
                // The fly portion is not intersected by a fly anymore
                if ( ! aInter.IsOver( aLine ) )
                {
                    rInf.SetLineHeight( nHeight );
                    rInf.SetLineNetHeight( m_pCurr->Height() );
                    return true;
                }
                else
                {
                    aInter._Intersection( aLine );

                    // No area means a fly has become invalid because of
                    // lowering the line => reformat the line
                    // we also have to reformat the line, if the fly size
                    // differs from the intersection interval's size.
                    if( ! aInter.HasArea() ||
                        static_cast<const SwFlyPortion*>(pPos)->GetFixWidth() != aInter.Width() )
                    {
                        rInf.SetLineHeight( nHeight );
                        rInf.SetLineNetHeight( m_pCurr->Height() );
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

void SwTextFormatter::CalcFlyWidth( SwTextFormatInfo &rInf )
{
    if( GetMulti() || rInf.GetFly() )
        return;

    SwTextFly& rTextFly = rInf.GetTextFly();
    if( !rTextFly.IsOn() || rInf.IsIgnoreFly() )
        return;

    const SwLinePortion *pLast = rInf.GetLast();

    long nAscent;
    long nTop = Y();
    long nHeight;

    if( rInf.GetLineHeight() )
    {
        // Real line height has already been calculated, we only have to
        // search for intersections in the lower part of the strip
        nAscent = m_pCurr->GetAscent();
        nHeight = rInf.GetLineNetHeight();
        nTop += rInf.GetLineHeight() - nHeight;
    }
    else
    {
        nAscent = pLast->GetAscent();
        nHeight = pLast->Height();

        // We make a first guess for the lines real height
        if ( ! m_pCurr->GetRealHeight() )
            CalcRealHeight();

        if ( m_pCurr->GetRealHeight() > nHeight )
            nTop += m_pCurr->GetRealHeight() - nHeight;
        else
            // Important for fixed space between lines
            nHeight = m_pCurr->GetRealHeight();
    }

    const long nLeftMar = GetLeftMargin();
    const long nLeftMin = (rInf.X() || GetDropLeft()) ? nLeftMar : GetLeftMin();

    SwRect aLine( rInf.X() + nLeftMin, nTop, rInf.RealWidth() - rInf.X()
                  + nLeftMar - nLeftMin , nHeight );

    SwRect aLineVert( aLine );
    if ( m_pFrame->IsRightToLeft() )
        m_pFrame->SwitchLTRtoRTL( aLineVert );

    if ( m_pFrame->IsVertical() )
        m_pFrame->SwitchHorizontalToVertical( aLineVert );
    SwRect aInter( rTextFly.GetFrame( aLineVert ) );

    if ( m_pFrame->IsRightToLeft() )
        m_pFrame->SwitchRTLtoLTR( aInter );

    if ( m_pFrame->IsVertical() )
        m_pFrame->SwitchVerticalToHorizontal( aInter );

    if( aInter.IsOver( aLine ) )
    {
        aLine.Left( rInf.X() + nLeftMar );
        bool bForced = false;
        if( aInter.Left() <= nLeftMin )
        {
            SwTwips nFrameLeft = GetTextFrame()->Frame().Left();
            if( GetTextFrame()->Prt().Left() < 0 )
                nFrameLeft += GetTextFrame()->Prt().Left();
            if( aInter.Left() < nFrameLeft )
                aInter.Left( nFrameLeft );

            long nAddMar = 0;
            if ( m_pFrame->IsRightToLeft() )
            {
                nAddMar = m_pFrame->Frame().Right() - Right();
                if ( nAddMar < 0 )
                    nAddMar = 0;
            }
            else
                nAddMar = nLeftMar - nFrameLeft;

            aInter.Width( aInter.Width() + nAddMar );
            // For a negative first line indent, we set this flag to show
            // that the indentation/margin has been moved.
            // This needs to be respected by the DefaultTab at the zero position.
            if( IsFirstTextLine() && HasNegFirst() )
                bForced = true;
        }
        aInter.Intersection( aLine );
        if( !aInter.HasArea() )
            return;

        const bool bFullLine =  aLine.Left()  == aInter.Left() &&
                                aLine.Right() == aInter.Right();

        // Although no text is left, we need to format another line,
        // because also empty lines need to avoid a Fly with no wrapping.
        if( bFullLine && rInf.GetIdx() == rInf.GetText().getLength() )
        {
            rInf.SetNewLine( true );
            // 8221: We know that for dummies, it holds ascent == height
            m_pCurr->SetDummy(true);
        }

        // aInter becomes frame-local
        aInter.Pos().X() -= nLeftMar;
        SwFlyPortion *pFly = new SwFlyPortion( aInter );
        if( bForced )
        {
            m_pCurr->SetForcedLeftMargin();
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
            // IsDummy() is evaluated in IsFirstTextLine(), when moving lines
            // and in relation with DropCaps.
            pFly->Height( sal_uInt16(aInter.Height()) );

            // nNextTop now contains the margin's bottom edge, which we avoid
            // or the next margin's top edge, which we need to respect.
            // That means we can comfortably grow up to this value; that's how
            // we save a few empty lines.
            long nNextTop = rTextFly.GetNextTop();
            if ( m_pFrame->IsVertical() )
                nNextTop = m_pFrame->SwitchVerticalToHorizontal( nNextTop );
            if( nNextTop > aInter.Bottom() )
            {
                SwTwips nH = nNextTop - aInter.Top();
                if( nH < USHRT_MAX )
                    pFly->Height( sal_uInt16( nH ) );
            }
            if( nAscent < pFly->Height() )
                pFly->SetAscent( sal_uInt16(nAscent) );
            else
                pFly->SetAscent( pFly->Height() );
        }
        else
        {
            if( rInf.GetIdx() == rInf.GetText().getLength() )
            {
                // Don't use nHeight, or we have a huge descent
                pFly->Height( pLast->Height() );
                pFly->SetAscent( pLast->GetAscent() );
            }
            else
            {
                pFly->Height( sal_uInt16(aInter.Height()) );
                if( nAscent < pFly->Height() )
                    pFly->SetAscent( sal_uInt16(nAscent) );
                else
                    pFly->SetAscent( pFly->Height() );
            }
        }

        rInf.SetFly( pFly );

        if( pFly->Fix() < rInf.Width() )
            rInf.Width( pFly->Fix() );

        SwTextGridItem const*const pGrid(GetGridItem(m_pFrame->FindPageFrame()));
        if ( pGrid )
        {
            const SwPageFrame* pPageFrame = m_pFrame->FindPageFrame();
            const SwLayoutFrame* pBody = pPageFrame->FindBodyCont();

            SWRECTFN( pPageFrame )

            const long nGridOrigin = pBody ?
                                    (pBody->*fnRect->fnGetPrtLeft)() :
                                    (pPageFrame->*fnRect->fnGetPrtLeft)();

            const SwDoc *pDoc = rInf.GetTextFrame()->GetNode()->GetDoc();
            const sal_uInt16 nGridWidth = GetGridWidth(*pGrid, *pDoc);

            SwTwips nStartX = GetLeftMargin();
            if ( bVert )
            {
                Point aPoint( nStartX, 0 );
                m_pFrame->SwitchHorizontalToVertical( aPoint );
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

SwFlyCntPortion *SwTextFormatter::NewFlyCntPortion( SwTextFormatInfo &rInf,
                                                   SwTextAttr *pHint ) const
{
    SwFlyCntPortion *pRet = nullptr;
    const SwFrame *pFrame = static_cast<SwFrame*>(m_pFrame);

    SwFlyInContentFrame *pFly;
    SwFrameFormat* pFrameFormat = static_cast<SwTextFlyCnt*>(pHint)->GetFlyCnt().GetFrameFormat();
    if( RES_FLYFRMFMT == pFrameFormat->Which() )
        pFly = static_cast<SwTextFlyCnt*>(pHint)->GetFlyFrame(pFrame);
    else
        pFly = nullptr;
    // aBase is the document-global position, from which the new extra portion is placed
    // aBase.X() = Offset in the line after the current position
    // aBase.Y() = LineIter.Y() + Ascent of the current position

    long nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc;
    // OD 08.01.2004 #i11859# - use new method <SwLineLayout::MaxAscentDescent(..)>
    //SwLinePortion *pPos = pCurr->GetFirstPortion();
    //lcl_MaxAscDescent( pPos, nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc );
    m_pCurr->MaxAscentDescent( nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc );

    // If the ascent of the frame is larger than the ascent of the current position,
    // we use this one when calculating the base, or the frame would be positioned
    // too much to the top, sliding down after all causing a repaint in an area
    // he actually never was in.
    sal_uInt16 nAscent = 0;

    const bool bTextFrameVertical = GetInfo().GetTextFrame()->IsVertical();

    const bool bUseFlyAscent = pFly && pFly->GetValidPosFlag() &&
                               0 != ( bTextFrameVertical ?
                                      pFly->GetRefPoint().X() :
                                      pFly->GetRefPoint().Y() );

    if ( bUseFlyAscent )
         nAscent = static_cast<sal_uInt16>( std::abs( int( bTextFrameVertical ?
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
    if ( GetInfo().GetTextFrame()->IsVertical() )
        GetInfo().GetTextFrame()->SwitchHorizontalToVertical( aTmpBase );

    if( pFly )
    {
        pRet = new SwFlyCntPortion( *GetInfo().GetTextFrame(), pFly, aTmpBase,
                                    nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc, nMode );
        // We need to make sure that our font is set again in the OutputDevice
        // It could be that the FlyInCnt was added anew and GetFlyFrame() would
        // in turn cause, that it'd be created anew again.
        // This one's frames get formatted right away, which change the font and
        // we have a bug (3322).
        rInf.SelectFont();
        if( pRet->GetAscent() > nAscent )
        {
            aBase.Y() = Y() + pRet->GetAscent();
            nMode |= AS_CHAR_ULSPACE;
            if( !rInf.IsTest() )
            {
                aTmpBase = aBase;
                if ( GetInfo().GetTextFrame()->IsVertical() )
                    GetInfo().GetTextFrame()->SwitchHorizontalToVertical( aTmpBase );

                pRet->SetBase( *rInf.GetTextFrame(), aTmpBase, nTmpAscent,
                               nTmpDescent, nFlyAsc, nFlyDesc, nMode );
            }
        }
    }
    else
    {
        pRet = new SwFlyCntPortion( *rInf.GetTextFrame(), static_cast<SwDrawContact*>(pFrameFormat->FindContactObj()),
           aTmpBase, nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc, nMode );
    }
    return pRet;
}

/* Drop portion is a special case, because it has parts which aren't portions
   but we have handle them just like portions */
void SwTextFormatter::MergeCharacterBorder( SwDropPortion& rPortion )
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

void SwTextFormatter::MergeCharacterBorder( SwLinePortion& rPortion, SwTextFormatInfo& rInf )
{
    const SwFont aCurFont = *rInf.GetFont();
    if( aCurFont.HasBorder() )
    {
        // The current portion isn't inserted into the portion chain yet, so the info's
        // last portion will be the previous one
        if( rInf.GetLast() && rInf.GetLast() != &rPortion && // For para portion (special case)
            rInf.GetLast()->GetJoinBorderWithNext() )
        {
            // In some case border merge is called twice to the portion
            if( !rPortion.GetJoinBorderWithPrev() )
            {
                rPortion.SetJoinBorderWithPrev(true);
                if( rPortion.InTextGrp() && rPortion.Width() > aCurFont.GetLeftBorderSpace() )
                    rPortion.Width(rPortion.Width() - aCurFont.GetLeftBorderSpace());
            }
        }
        else
        {
            rPortion.SetJoinBorderWithPrev(false);
            m_pFirstOfBorderMerge = &rPortion;
        }

        // Get next portion's font
        bool bSeek = false;
        if( !rInf.IsFull() && // Not the last portion of the line (in case of line break)
            rInf.GetIdx() + rPortion.GetLen() != rInf.GetText().getLength() ) // Not the last portion of the paragraph
            bSeek = Seek(rInf.GetIdx() + rPortion.GetLen());

        // If next portion has the same border then merge
        if( bSeek && GetFnt()->HasBorder() && ::lcl_HasSameBorder(aCurFont, *GetFnt()) )
        {
            // In some case border merge is called twice to the portion
            if( !rPortion.GetJoinBorderWithNext() )
            {
                rPortion.SetJoinBorderWithNext(true);
                if( rPortion.InTextGrp() && rPortion.Width() > aCurFont.GetRightBorderSpace() )
                    rPortion.Width(rPortion.Width() - aCurFont.GetRightBorderSpace());
            }
        }
        // If this is the last portion of the merge group then make the real height merge
        else
        {
            rPortion.SetJoinBorderWithNext(false);
            if( m_pFirstOfBorderMerge != &rPortion )
            {
                // Calculate maximum height and ascent
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

                // Change all portion's height and ascent
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
                m_pFirstOfBorderMerge = nullptr;
            }
        }
        Seek(rInf.GetIdx());
    }
}

namespace {
    // calculates and sets optimal repaint offset for the current line
    long lcl_CalcOptRepaint( SwTextFormatter &rThis,
                         SwLineLayout &rCurr,
                         const sal_Int32 nOldLineEnd,
                         const std::vector<long> &rFlyStarts )
    {
        SwTextFormatInfo& txtFormatInfo = rThis.GetInfo();
        if ( txtFormatInfo.GetIdx() < txtFormatInfo.GetReformatStart() )
        // the reformat position is behind our new line, that means
        // something of our text has moved to the next line
            return 0;

        sal_Int32 nReformat = std::min<sal_Int32>( txtFormatInfo.GetReformatStart(), nOldLineEnd );

        // in case we do not have any fly in our line, our repaint position
        // is the changed position - 1
        if ( rFlyStarts.empty() && ! rCurr.IsFly() )
        {
            // this is the maximum repaint offset determined during formatting
            // for example: the beginning of the first right tab stop
            // if this value is 0, this means that we do not have an upper
            // limit for the repaint offset
            const long nFormatRepaint = txtFormatInfo.GetPaintOfst();

            if ( nReformat < txtFormatInfo.GetLineStart() + 3 )
                return 0;

            // step back two positions for smoother repaint
            nReformat -= 2;

    #ifndef MACOSX
    #if ! ENABLE_GRAPHITE
            // #i28795#, #i34607#, #i38388#
            // step back six(!) more characters for complex scripts
            // this is required e.g., for Khmer (thank you, Javier!)
            const SwScriptInfo& rSI = txtFormatInfo.GetParaPortion()->GetScriptInfo();
            sal_Int32 nMaxContext = 0;
            if( ::i18n::ScriptType::COMPLEX == rSI.ScriptType( nReformat ) )
                nMaxContext = 6;
    #else
            // Some Graphite fonts need context for scripts not marked as complex
            static const sal_Int32 nMaxContext = 10;
    #endif
    #else
            // some fonts like Quartz's Zapfino need more context
            // TODO: query FontInfo for maximum unicode context
            static const sal_Int32 nMaxContext = 8;
    #endif
            if( nMaxContext > 0 )
            {
                if ( nReformat > txtFormatInfo.GetLineStart() + nMaxContext )
                    nReformat = nReformat - nMaxContext;
                else
                {
                    nReformat = txtFormatInfo.GetLineStart();
                    //reset the margin flag - prevent loops
                    SwTextCursor::SetRightMargin(false);
                }
            }

            // Weird situation: Our line used to end with a hole portion
            // and we delete some characters at the end of our line. We have
            // to take care for repainting the blanks which are not anymore
            // covered by the hole portion
            while ( nReformat > txtFormatInfo.GetLineStart() &&
                    CH_BLANK == txtFormatInfo.GetChar( nReformat ) )
                --nReformat;

            OSL_ENSURE( nReformat < txtFormatInfo.GetIdx(), "Reformat too small for me!" );
            SwRect aRect;

            // Note: GetChareRect is not const. It definitely changes the
            // bMulti flag. We have to save and restore the old value.
            bool bOldMulti = txtFormatInfo.IsMulti();
            rThis.GetCharRect( &aRect, nReformat );
            txtFormatInfo.SetMulti( bOldMulti );

            return nFormatRepaint ? std::min( aRect.Left(), nFormatRepaint ) :
                                    aRect.Left();
        }
        else
        {
            // nReformat may be wrong, if something around flys has changed:
            // we compare the former and the new fly positions in this line
            // if anything has changed, we carefully have to adjust the right
            // repaint position
            long nPOfst = 0;
            size_t nCnt = 0;
            long nX = 0;
            sal_Int32 nIdx = rThis.GetInfo().GetLineStart();
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
    bool lcl_BuildHiddenPortion( const SwTextSizeInfo& rInf, sal_Int32 &rPos )
    {
        // Only if hidden text should not be shown:
    //    if ( rInf.GetVsh() && rInf.GetVsh()->GetWin() && rInf.GetOpt().IsShowHiddenChar() )
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

} //end unnamed namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
