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


#include <vcl/svapp.hxx>
#include <vcl/metaact.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/settings.hxx>

#include <editeng/adjustitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/forbiddenruleitem.hxx>
#include "impedit.hxx"
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/txtrange.hxx>
#include <editeng/colritem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/scriptspaceitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/justifyitem.hxx>

#include <svtools/colorcfg.hxx>
#include <svl/ctloptions.hxx>
#include <svl/asiancfg.hxx>

#include <editeng/hngpnctitem.hxx>
#include <editeng/forbiddencharacterstable.hxx>

#include <unotools/configmgr.hxx>
#include <unotools/localedatawrapper.hxx>

#include <editeng/unolingu.hxx>

#include <set>
#include <math.h>
#include <vcl/metric.hxx>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/InputSequenceChecker.hpp>
#include <com/sun/star/text/CharacterCompressionType.hpp>
#include <vcl/pdfextoutdevdata.hxx>
#include <i18nlangtag/mslangid.hxx>

#include <comphelper/processfactory.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <comphelper/string.hxx>
#include <comphelper/lok.hxx>
#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;

#define CH_HYPH     '-'

#define WRONG_SHOW_MIN       5

struct TabInfo
{
    bool        bValid;

    SvxTabStop  aTabStop;
    sal_Int32   nTabPortion;
    long        nStartPosX;
    long        nTabPos;

    TabInfo()
        : bValid(false)
        , nTabPortion(0)
        , nStartPosX(0)
        , nTabPos(0)
        { }

};

Point Rotate( const Point& rPoint, short nOrientation, const Point& rOrigin )
{
    double nRealOrientation = nOrientation*F_PI1800;
    double nCos = cos( nRealOrientation );
    double nSin = sin( nRealOrientation );

    Point aRotatedPos;
    Point aTranslatedPos( rPoint );

    // Translation
    aTranslatedPos -= rOrigin;

    // Rotation...
    aRotatedPos.setX( static_cast<long>( nCos*aTranslatedPos.X() + nSin*aTranslatedPos.Y() ) );
    aRotatedPos.setY( static_cast<long>(- ( nSin*aTranslatedPos.X() - nCos*aTranslatedPos.Y() )) );
    aTranslatedPos = aRotatedPos;

    // Translation...
    aTranslatedPos += rOrigin;
    return aTranslatedPos;
}

AsianCompressionFlags GetCharTypeForCompression( sal_Unicode cChar )
{
    switch ( cChar )
    {
        case 0x3008: case 0x300A: case 0x300C: case 0x300E:
        case 0x3010: case 0x3014: case 0x3016: case 0x3018:
        case 0x301A: case 0x301D:
        {
            return AsianCompressionFlags::PunctuationRight;
        }
        case 0x3001: case 0x3002: case 0x3009: case 0x300B:
        case 0x300D: case 0x300F: case 0x3011: case 0x3015:
        case 0x3017: case 0x3019: case 0x301B: case 0x301E:
        case 0x301F:
        {
            return AsianCompressionFlags::PunctuationLeft;
        }
        default:
        {
            return ( ( 0x3040 <= cChar ) && ( 0x3100 > cChar ) ) ? AsianCompressionFlags::Kana : AsianCompressionFlags::Normal;
        }
    }
}

static void lcl_DrawRedLines( OutputDevice* pOutDev,
                              long nFontHeight,
                              const Point& rPnt,
                              size_t nIndex,
                              size_t nMaxEnd,
                              const long* pDXArray,
                              WrongList const * pWrongs,
                              short nOrientation,
                              const Point& rOrigin,
                              bool bVertical,
                              bool bIsRightToLeft )
{
    // But only if font is not too small...
    long nHght = pOutDev->LogicToPixel( Size( 0, nFontHeight ) ).Height();
    if( WRONG_SHOW_MIN < nHght )
    {
        size_t nEnd, nStart = nIndex;
        bool bWrong = pWrongs->NextWrong( nStart, nEnd );
        while ( bWrong )
        {
            if ( nStart >= nMaxEnd )
                break;

            if ( nStart < nIndex )  // Corrected
                nStart = nIndex;
            if ( nEnd > nMaxEnd )
                nEnd = nMaxEnd;
            Point aPnt1( rPnt );
            if ( bVertical )
            {
                // VCL doesn't know that the text is vertical, and is manipulating
                // the positions a little bit in y direction...
                long nOnePixel = pOutDev->PixelToLogic( Size( 0, 1 ) ).Height();
                long nCorrect = 2*nOnePixel;
                aPnt1.AdjustY( -nCorrect );
                aPnt1.AdjustX( -nCorrect );
            }
            if ( nStart > nIndex )
            {
                if ( !bVertical )
                {
                    // since for RTL portions rPnt is on the visual right end of the portion
                    // (i.e. at the start of the first RTL char) we need to subtract the offset
                    // for RTL portions...
                    aPnt1.AdjustX((bIsRightToLeft ? -1 : 1) * pDXArray[ nStart - nIndex - 1 ] );
                }
                else
                    aPnt1.AdjustY(pDXArray[ nStart - nIndex - 1 ] );
            }
            Point aPnt2( rPnt );
            DBG_ASSERT( nEnd > nIndex, "RedLine: aPnt2?" );
            if ( !bVertical )
            {
                // since for RTL portions rPnt is on the visual right end of the portion
                // (i.e. at the start of the first RTL char) we need to subtract the offset
                // for RTL portions...
                aPnt2.AdjustX((bIsRightToLeft ? -1 : 1) * pDXArray[ nEnd - nIndex - 1 ] );
            }
            else
                aPnt2.AdjustY(pDXArray[ nEnd - nIndex - 1 ] );
            if ( nOrientation )
            {
                aPnt1 = Rotate( aPnt1, nOrientation, rOrigin );
                aPnt2 = Rotate( aPnt2, nOrientation, rOrigin );
            }

            pOutDev->DrawWaveLine( aPnt1, aPnt2 );

            nStart = nEnd+1;
            if ( nEnd < nMaxEnd )
                bWrong = pWrongs->NextWrong( nStart, nEnd );
            else
                bWrong = false;
        }
    }
}

static Point lcl_ImplCalcRotatedPos( Point rPos, Point rOrigin, double nSin, double nCos )
{
    Point aRotatedPos;
    // Translation...
    Point aTranslatedPos( rPos);
    aTranslatedPos -= rOrigin;

    aRotatedPos.setX( static_cast<long>( nCos*aTranslatedPos.X() + nSin*aTranslatedPos.Y() ) );
    aRotatedPos.setY( static_cast<long>(- ( nSin*aTranslatedPos.X() - nCos*aTranslatedPos.Y() )) );
    aTranslatedPos = aRotatedPos;
    // Translation...
    aTranslatedPos += rOrigin;

    return aTranslatedPos;
}

static bool lcl_IsLigature( sal_Unicode cCh, sal_Unicode cNextCh ) // For Kashidas from sw/source/core/text/porlay.txt
{
            // Lam + Alef
    return ( 0x644 == cCh && 0x627 == cNextCh ) ||
            // Beh + Reh
           ( 0x628 == cCh && 0x631 == cNextCh );
}

static bool lcl_ConnectToPrev( sal_Unicode cCh, sal_Unicode cPrevCh )  // For Kashidas from sw/source/core/text/porlay.txt
{
    // Alef, Dal, Thal, Reh, Zain, and Waw do not connect to the left
    bool bRet = 0x627 != cPrevCh && 0x62F != cPrevCh && 0x630 != cPrevCh &&
                0x631 != cPrevCh && 0x632 != cPrevCh && 0x648 != cPrevCh;

    // check for ligatures cPrevChar + cChar
    if ( bRet )
        bRet = ! lcl_IsLigature( cPrevCh, cCh );

    return bRet;
}


//  class ImpEditEngine

void ImpEditEngine::UpdateViews( EditView* pCurView )
{
    if ( !GetUpdateMode() || IsFormatting() || aInvalidRect.IsEmpty() )
        return;

    DBG_ASSERT( IsFormatted(), "UpdateViews: Doc not formatted!" );

    for (EditView* pView : aEditViews)
    {
        pView->HideCursor();

        tools::Rectangle aClipRect( aInvalidRect );
        tools::Rectangle aVisArea( pView->GetVisArea() );
        aClipRect.Intersection( aVisArea );

        if ( !aClipRect.IsEmpty() )
        {
            // convert to window coordinates...
            aClipRect = pView->pImpEditView->GetWindowPos( aClipRect );

            // moved to one executing method to allow finer control
            pView->InvalidateWindow(aClipRect);

            pView->InvalidateOtherViewWindows( aClipRect );
        }
    }

    if ( pCurView )
    {
        bool bGotoCursor = pCurView->pImpEditView->DoAutoScroll();
        pCurView->ShowCursor( bGotoCursor );
    }

    aInvalidRect = tools::Rectangle();
    CallStatusHdl();
}

IMPL_LINK_NOARG(ImpEditEngine, OnlineSpellHdl, Timer *, void)
{
    if ( !Application::AnyInput( VclInputFlags::KEYBOARD ) && GetUpdateMode() && IsFormatted() )
        DoOnlineSpelling();
    else
        aOnlineSpellTimer.Start();
}

IMPL_LINK_NOARG(ImpEditEngine, IdleFormatHdl, Timer *, void)
{
    aIdleFormatter.ResetRestarts();

    // #i97146# check if that view is still available
    // else probably the idle format timer fired while we're already
    // downing
    EditView* pView = aIdleFormatter.GetView();
    for (EditView* aEditView : aEditViews)
    {
        if( aEditView == pView )
        {
            FormatAndUpdate( pView );
            break;
        }
    }
}

void ImpEditEngine::CheckIdleFormatter()
{
    aIdleFormatter.ForceTimeout();
    // If not idle, but still not formatted:
    if ( !IsFormatted() )
        FormatDoc();
}

bool ImpEditEngine::IsPageOverflow( ) const
{
    return mbNeedsChainingHandling;
}


void ImpEditEngine::FormatFullDoc()
{
    for ( sal_Int32 nPortion = 0; nPortion < GetParaPortions().Count(); nPortion++ )
        GetParaPortions()[nPortion]->MarkSelectionInvalid( 0 );
    FormatDoc();
}

void ImpEditEngine::FormatDoc()
{
    if (!GetUpdateMode() || IsFormatting())
        return;

    bIsFormatting = true;

    // Then I can also start the spell-timer...
    if ( GetStatus().DoOnlineSpelling() )
        StartOnlineSpellTimer();

    long nY = 0;
    bool bGrow = false;

    vcl::Font aOldFont( GetRefDevice()->GetFont() );

    // Here already, so that not always in CreateLines...
    bool bMapChanged = ImpCheckRefMapMode();

    aInvalidRect = tools::Rectangle();  // make empty
    for ( sal_Int32 nPara = 0; nPara < GetParaPortions().Count(); nPara++ )
    {
        ParaPortion* pParaPortion = GetParaPortions()[nPara];
        if ( pParaPortion->MustRepaint() || ( pParaPortion->IsInvalid() && pParaPortion->IsVisible() ) )
        {
            // No formatting should be necessary for MustRepaint()!
            if ( ( pParaPortion->MustRepaint() && !pParaPortion->IsInvalid() )
                    || CreateLines( nPara, nY ) )
            {
                if ( !bGrow && GetTextRanger() )
                {
                    // For a change in height all below must be reformatted...
                    for ( sal_Int32 n = nPara+1; n < GetParaPortions().Count(); n++ )
                    {
                        ParaPortion* pPP = GetParaPortions()[n];
                        pPP->MarkSelectionInvalid( 0 );
                        pPP->GetLines().Reset();
                    }
                }
                bGrow = true;
                if ( IsCallParaInsertedOrDeleted() )
                    GetEditEnginePtr()->ParagraphHeightChanged( nPara );
                pParaPortion->SetMustRepaint( false );
            }

            // InvalidRect set only once...
            if ( aInvalidRect.IsEmpty() )
            {
                // For Paperwidth 0 (AutoPageSize) it would otherwise be Empty()...
                long nWidth = std::max( long(1), ( !IsVertical() ? aPaperSize.Width() : aPaperSize.Height() ) );
                Range aInvRange( GetInvalidYOffsets( pParaPortion ) );
                aInvalidRect = tools::Rectangle( Point( 0, nY+aInvRange.Min() ),
                    Size( nWidth, aInvRange.Len() ) );
            }
            else
            {
                aInvalidRect.SetBottom( nY + pParaPortion->GetHeight() );
            }
        }
        else if ( bGrow )
        {
            aInvalidRect.SetBottom( nY + pParaPortion->GetHeight() );
        }
        nY += pParaPortion->GetHeight();
    }

    // One can also get into the formatting through UpdateMode ON=>OFF=>ON...
    // enable optimization first after Vobis delivery...
    {
        sal_uInt32 nNewHeightNTP;
        sal_uInt32 nNewHeight = CalcTextHeight( &nNewHeightNTP );
        long nDiff = nNewHeight - nCurTextHeight;
        if ( nDiff )
            aStatus.GetStatusWord() |= !IsVertical() ? EditStatusFlags::TextHeightChanged : EditStatusFlags::TEXTWIDTHCHANGED;
        if ( nNewHeight < nCurTextHeight )
        {
            aInvalidRect.SetBottom( static_cast<long>(std::max( nNewHeight, nCurTextHeight )) );
            if ( aInvalidRect.IsEmpty() )
            {
                aInvalidRect.SetTop( 0 );
                // Left and Right are not evaluated, are however set due to IsEmpty.
                aInvalidRect.SetLeft( 0 );
                aInvalidRect.SetRight( !IsVertical() ? aPaperSize.Width() : aPaperSize.Height() );
            }
        }

        nCurTextHeight = nNewHeight;
        nCurTextHeightNTP = nNewHeightNTP;

        if ( aStatus.AutoPageSize() )
            CheckAutoPageSize();
        else if ( nDiff )
        {
            for (EditView* pView : aEditViews)
            {
                ImpEditView* pImpView = pView->pImpEditView.get();
                if ( pImpView->DoAutoHeight() )
                {
                    Size aSz( pImpView->GetOutputArea().GetWidth(), nCurTextHeight );
                    if ( aSz.Height() > aMaxAutoPaperSize.Height() )
                        aSz.setHeight( aMaxAutoPaperSize.Height() );
                    else if ( aSz.Height() < aMinAutoPaperSize.Height() )
                        aSz.setHeight( aMinAutoPaperSize.Height() );
                    pImpView->ResetOutputArea( tools::Rectangle(
                        pImpView->GetOutputArea().TopLeft(), aSz ) );
                }
            }
        }
    }

    bIsFormatting = false;
    bFormatted = true;

    if ( bMapChanged )
        GetRefDevice()->Pop();

    CallStatusHdl();    // If Modified...
}

bool ImpEditEngine::ImpCheckRefMapMode()
{
    bool bChange = false;

    if ( aStatus.DoFormat100() )
    {
        MapMode aMapMode( GetRefDevice()->GetMapMode() );
        if ( aMapMode.GetScaleX().GetNumerator() != aMapMode.GetScaleX().GetDenominator() )
            bChange = true;
        else if ( aMapMode.GetScaleY().GetNumerator() != aMapMode.GetScaleY().GetDenominator() )
            bChange = true;

        if ( bChange )
        {
            Fraction Scale1( 1, 1 );
            aMapMode.SetScaleX( Scale1 );
            aMapMode.SetScaleY( Scale1 );
            GetRefDevice()->Push();
            GetRefDevice()->SetMapMode( aMapMode );
        }
    }

    return bChange;
}

void ImpEditEngine::CheckAutoPageSize()
{
    Size aPrevPaperSize( GetPaperSize() );
    if ( GetStatus().AutoPageWidth() )
        aPaperSize.setWidth( !IsVertical() ? CalcTextWidth( true ) : GetTextHeight() );
    if ( GetStatus().AutoPageHeight() )
        aPaperSize.setHeight( !IsVertical() ? GetTextHeight() : CalcTextWidth( true ) );

    SetValidPaperSize( aPaperSize );    // consider Min, Max

    if ( aPaperSize != aPrevPaperSize )
    {
        if ( ( !IsVertical() && ( aPaperSize.Width() != aPrevPaperSize.Width() ) )
             || ( IsVertical() && ( aPaperSize.Height() != aPrevPaperSize.Height() ) ) )
        {
            // If ahead is centered / right or tabs...
            aStatus.GetStatusWord() |= !IsVertical() ? EditStatusFlags::TEXTWIDTHCHANGED : EditStatusFlags::TextHeightChanged;
            for ( sal_Int32 nPara = 0; nPara < GetParaPortions().Count(); nPara++ )
            {
                // Only paragraphs which are not aligned to the left need to be
                // reformatted, the height can not be changed here anymore.
                ParaPortion* pParaPortion = GetParaPortions()[nPara];
                SvxAdjust eJustification = GetJustification( nPara );
                if ( eJustification != SvxAdjust::Left )
                {
                    pParaPortion->MarkSelectionInvalid( 0 );
                    CreateLines( nPara, 0 );  // 0: For AutoPageSize no TextRange!
                }
            }
        }

        Size aInvSize = aPaperSize;
        if ( aPaperSize.Width() < aPrevPaperSize.Width() )
            aInvSize.setWidth( aPrevPaperSize.Width() );
        if ( aPaperSize.Height() < aPrevPaperSize.Height() )
            aInvSize.setHeight( aPrevPaperSize.Height() );

        Size aSz( aInvSize );
        if ( IsVertical() )
        {
            aSz.setWidth( aInvSize.Height() );
            aSz.setHeight( aInvSize.Width() );
        }
        aInvalidRect = tools::Rectangle( Point(), aSz );


        for (EditView* pView : aEditViews)
        {
            pView->pImpEditView->RecalcOutputArea();
        }
    }
}

void ImpEditEngine::CheckPageOverflow()
{
    SAL_INFO("editeng.chaining", "[CONTROL_STATUS] AutoPageSize is " << (( aStatus.GetControlWord() & EEControlBits::AUTOPAGESIZE ) ? "ON" : "OFF") );

    sal_uInt32 nBoxHeight = GetMaxAutoPaperSize().Height();
    SAL_INFO("editeng.chaining", "[OVERFLOW-CHECK] Current MaxAutoPaperHeight is " << nBoxHeight);

    sal_uInt32 nTxtHeight = CalcTextHeight(nullptr);
    SAL_INFO("editeng.chaining", "[OVERFLOW-CHECK] Current Text Height is " << nTxtHeight);

    sal_uInt32 nParaCount = GetParaPortions().Count();
    sal_uInt32 nFirstLineCount = GetLineCount(0);
    bool bOnlyOneEmptyPara = (nParaCount == 1) &&
                            (nFirstLineCount == 1) &&
                            (GetLineLen(0,0) == 0);

    if (nTxtHeight > nBoxHeight && !bOnlyOneEmptyPara)
    {
        // which paragraph is the first to cause higher size of the box?
        ImplUpdateOverflowingParaNum( nBoxHeight); // XXX: currently only for horizontal text
        //aStatus.SetPageOverflow(true);
        mbNeedsChainingHandling = true;
    } else
    {
        // No overflow if within box boundaries
        //aStatus.SetPageOverflow(false);
        mbNeedsChainingHandling = false;
    }

}

static sal_Int32 ImplCalculateFontIndependentLineSpacing( const sal_Int32 nFontHeight )
{
    return ( nFontHeight * 12 ) / 10;   // + 20%
}

bool ImpEditEngine::CreateLines( sal_Int32 nPara, sal_uInt32 nStartPosY )
{
    ParaPortion* pParaPortion = GetParaPortions()[nPara];

    // sal_Bool: Changes in the height of paragraph Yes / No - sal_True/sal_False
    DBG_ASSERT( pParaPortion->GetNode(), "Portion without Node in CreateLines" );
    DBG_ASSERT( pParaPortion->IsVisible(), "Invisible paragraphs not formatted!" );
    DBG_ASSERT( pParaPortion->IsInvalid(), "CreateLines: Portion not invalid!" );

    bool bProcessingEmptyLine = ( pParaPortion->GetNode()->Len() == 0 );
    bool bEmptyNodeWithPolygon = ( pParaPortion->GetNode()->Len() == 0 ) && GetTextRanger();


    // Fast special treatment for empty paragraphs...

    if ( ( pParaPortion->GetNode()->Len() == 0 ) && !GetTextRanger() )
    {
        // fast special treatment...
        if ( pParaPortion->GetTextPortions().Count() )
            pParaPortion->GetTextPortions().Reset();
        if ( pParaPortion->GetLines().Count() )
            pParaPortion->GetLines().Reset();
        CreateAndInsertEmptyLine( pParaPortion );
        return FinishCreateLines( pParaPortion );
    }


    // Initialization...


    // Always format for 100%:
    bool bMapChanged = ImpCheckRefMapMode();

    if ( pParaPortion->GetLines().Count() == 0 )
    {
        EditLine* pL = new EditLine;
        pParaPortion->GetLines().Append(pL);
    }


    // Get Paragraph attributes...

    ContentNode* const pNode = pParaPortion->GetNode();

    bool bRightToLeftPara = IsRightToLeft( nPara );

    SvxAdjust eJustification = GetJustification( nPara );
    bool bHyphenatePara = pNode->GetContentAttribs().GetItem( EE_PARA_HYPHENATE ).GetValue();
    sal_Int32 nSpaceBefore      = 0;
    sal_Int32 nMinLabelWidth    = 0;
    sal_Int32 nSpaceBeforeAndMinLabelWidth = GetSpaceBeforeAndMinLabelWidth( pNode, &nSpaceBefore, &nMinLabelWidth );
    const SvxLRSpaceItem& rLRItem = GetLRSpaceItem( pNode );
    const SvxLineSpacingItem& rLSItem = pNode->GetContentAttribs().GetItem( EE_PARA_SBL );
    const bool bScriptSpace = pNode->GetContentAttribs().GetItem( EE_PARA_ASIANCJKSPACING ).GetValue();

    const short nInvalidDiff = pParaPortion->GetInvalidDiff();
    const sal_Int32 nInvalidStart = pParaPortion->GetInvalidPosStart();
    const sal_Int32 nInvalidEnd =  nInvalidStart + std::abs( nInvalidDiff );

    bool bQuickFormat = false;
    if ( !bEmptyNodeWithPolygon && !HasScriptType( nPara, i18n::ScriptType::COMPLEX ) )
    {
        if ( ( pParaPortion->IsSimpleInvalid() ) && ( nInvalidDiff > 0 ) &&
             ( pNode->GetString().indexOf( CH_FEATURE, nInvalidStart ) > nInvalidEnd ) )
        {
            bQuickFormat = true;
        }
        else if ( ( pParaPortion->IsSimpleInvalid() ) && ( nInvalidDiff < 0 ) )
        {
            // check if delete over the portion boundaries was done...
            sal_Int32 nStart = nInvalidStart;  // DOUBLE !!!!!!!!!!!!!!!
            sal_Int32 nEnd = nStart - nInvalidDiff;  // negative
            bQuickFormat = true;
            sal_Int32 nPos = 0;
            sal_Int32 nPortions = pParaPortion->GetTextPortions().Count();
            for ( sal_Int32 nTP = 0; nTP < nPortions; nTP++ )
            {
                // There must be no start / end in the deleted area.
                const TextPortion& rTP = pParaPortion->GetTextPortions()[ nTP ];
                nPos = nPos + rTP.GetLen();
                if ( ( nPos > nStart ) && ( nPos < nEnd ) )
                {
                    bQuickFormat = false;
                    break;
                }
            }
        }
    }

    // Saving both layout mode and language (since I'm potentially changing both)
    GetRefDevice()->Push( PushFlags::TEXTLAYOUTMODE|PushFlags::TEXTLANGUAGE );

    ImplInitLayoutMode( GetRefDevice(), nPara, -1 );

    sal_Int32 nRealInvalidStart = nInvalidStart;

    if ( bEmptyNodeWithPolygon )
    {
        TextPortion* pDummyPortion = new TextPortion( 0 );
        pParaPortion->GetTextPortions().Reset();
        pParaPortion->GetTextPortions().Append(pDummyPortion);
    }
    else if ( bQuickFormat )
    {
        // faster Method:
        RecalcTextPortion( pParaPortion, nInvalidStart, nInvalidDiff );
    }
    else    // nRealInvalidStart can be before InvalidStart, since Portions were deleted...
    {
        CreateTextPortions( pParaPortion, nRealInvalidStart );
    }


    // Search for line with InvalidPos, start one line before
    // Flag the line => do not remove it !


    sal_Int32 nLine = pParaPortion->GetLines().Count()-1;
    for ( sal_Int32 nL = 0; nL <= nLine; nL++ )
    {
        EditLine& rLine = pParaPortion->GetLines()[nL];
        if ( rLine.GetEnd() > nRealInvalidStart )  // not nInvalidStart!
        {
            nLine = nL;
            break;
        }
        rLine.SetValid();
    }
    // Begin one line before...
    // If it is typed at the end, the line in front cannot change.
    if ( nLine && ( !pParaPortion->IsSimpleInvalid() || ( nInvalidEnd < pNode->Len() ) || ( nInvalidDiff <= 0 ) ) )
        nLine--;

    EditLine* pLine = &pParaPortion->GetLines()[nLine];

    static tools::Rectangle aZeroArea = tools::Rectangle( Point(), Point() );
    tools::Rectangle aBulletArea( aZeroArea );
    if ( !nLine )
    {
        aBulletArea = GetEditEnginePtr()->GetBulletArea( GetParaPortions().GetPos( pParaPortion ) );
        if ( aBulletArea.Right() > 0 )
            pParaPortion->SetBulletX( static_cast<sal_Int32>(GetXValue( aBulletArea.Right() )) );
        else
            pParaPortion->SetBulletX( 0 ); // if Bullet is set incorrectly
    }


    // Reformat all lines from here...

    sal_Int32 nDelFromLine = -1;
    bool bLineBreak = false;

    sal_Int32 nIndex = pLine->GetStart();
    EditLine aSaveLine( *pLine );
    SvxFont aTmpFont( pNode->GetCharAttribs().GetDefFont() );

    ImplInitLayoutMode( GetRefDevice(), nPara, nIndex );

    std::unique_ptr<long[]> pBuf(new long[ pNode->Len() ]);

    bool bSameLineAgain = false;    // For TextRanger, if the height changes.
    TabInfo aCurrentTab;

    bool bForceOneRun = bEmptyNodeWithPolygon;
    bool bCompressedChars = false;

    while ( ( nIndex < pNode->Len() ) || bForceOneRun )
    {
        bForceOneRun = false;

        bool bEOL = false;
        bool bEOC = false;
        sal_Int32 nPortionStart = 0;
        sal_Int32 nPortionEnd = 0;

        long nStartX = GetXValue( rLRItem.GetTextLeft() + nSpaceBeforeAndMinLabelWidth );
        if ( nIndex == 0 )
        {
            long nFI = GetXValue( rLRItem.GetTextFirstLineOfst() );
            nStartX += nFI;

            if ( !nLine && ( pParaPortion->GetBulletX() > nStartX ) )
            {
                    nStartX = pParaPortion->GetBulletX();
            }
        }

        long nMaxLineWidth;
        if ( !IsVertical() )
            nMaxLineWidth = aStatus.AutoPageWidth() ? aMaxAutoPaperSize.Width() : aPaperSize.Width();
        else
            nMaxLineWidth = aStatus.AutoPageHeight() ? aMaxAutoPaperSize.Height() : aPaperSize.Height();

        nMaxLineWidth -= GetXValue( rLRItem.GetRight() );
        nMaxLineWidth -= nStartX;

        // If PaperSize == long_max, one cannot take away any negative
        // first line indent. (Overflow)
        if ( ( nMaxLineWidth < 0 ) && ( nStartX < 0 ) )
            nMaxLineWidth = ( !IsVertical() ? aPaperSize.Width() : aPaperSize.Height() ) - GetXValue( rLRItem.GetRight() );

        // If still less than 0, it may be just the right edge.
        if ( nMaxLineWidth <= 0 )
            nMaxLineWidth = 1;

        // Problem:
        // Since formatting starts a line _before_ the invalid position,
     // the positions unfortunately have to be redefined...
        // Solution:
        // The line before can only become longer, not smaller
        // =>...
        pLine->GetCharPosArray().clear();

        sal_Int32 nTmpPos = nIndex;
        sal_Int32 nTmpPortion = pLine->GetStartPortion();
        long nTmpWidth = 0;
        long nXWidth = nMaxLineWidth;

        LongDqPtr pTextRanges = nullptr;
        long nTextExtraYOffset = 0;
        long nTextXOffset = 0;
        long nTextLineHeight = 0;
        if ( GetTextRanger() )
        {
            GetTextRanger()->SetVertical( IsVertical() );

            long nTextY = nStartPosY + GetEditCursor( pParaPortion, pLine->GetStart() ).Top();
            if ( !bSameLineAgain )
            {
                SeekCursor( pNode, nTmpPos+1, aTmpFont );
                aTmpFont.SetPhysFont( GetRefDevice() );
                ImplInitDigitMode(GetRefDevice(), aTmpFont.GetLanguage());

                if ( IsFixedCellHeight() )
                    nTextLineHeight = ImplCalculateFontIndependentLineSpacing( aTmpFont.GetFontHeight() );
                else
                    nTextLineHeight = aTmpFont.GetPhysTxtSize( GetRefDevice() ).Height();
                // Metrics can be greater
                FormatterFontMetric aTempFormatterMetrics;
                RecalcFormatterFontMetrics( aTempFormatterMetrics, aTmpFont );
                sal_uInt16 nLineHeight = aTempFormatterMetrics.GetHeight();
                if ( nLineHeight > nTextLineHeight )
                    nTextLineHeight = nLineHeight;
            }
            else
                nTextLineHeight = pLine->GetHeight();

            nXWidth = 0;
            while ( !nXWidth )
            {
                long nYOff = nTextY + nTextExtraYOffset;
                long nYDiff = nTextLineHeight;
                if ( IsVertical() )
                {
                    long nMaxPolygonX = GetTextRanger()->GetBoundRect().Right();
                    nYOff = nMaxPolygonX-nYOff;
                    nYDiff = -nTextLineHeight;
                }
                pTextRanges = GetTextRanger()->GetTextRanges( Range( nYOff, nYOff + nYDiff ) );
                DBG_ASSERT( pTextRanges, "GetTextRanges?!" );
                long nMaxRangeWidth = 0;
                // Use the widest range...
                // The widest range could be a bit confusing, so normally it
                // is the first one. Best with gaps.
                assert(pTextRanges->size() % 2 == 0 && "textranges are always in pairs");
                if (!pTextRanges->empty())
                {
                    long nA = pTextRanges->at(0);
                    long nB = pTextRanges->at(1);
                    DBG_ASSERT( nA <= nB, "TextRange distorted?" );
                    long nW = nB - nA;
                    if ( nW > nMaxRangeWidth )
                    {
                        nMaxRangeWidth = nW;
                        nTextXOffset = nA;
                    }
                }
                nXWidth = nMaxRangeWidth;
                if ( nXWidth )
                    nMaxLineWidth = nXWidth - nStartX - GetXValue( rLRItem.GetRight() );
                else
                {
                    // Try further down in the polygon.
                    // Below the polygon use the Paper Width.
                    nTextExtraYOffset += std::max( static_cast<long>(nTextLineHeight / 10), long(1) );
                    if ( ( nTextY + nTextExtraYOffset  ) > GetTextRanger()->GetBoundRect().Bottom() )
                    {
                        nXWidth = !IsVertical() ? GetPaperSize().Width() : GetPaperSize().Height();
                        if ( !nXWidth ) // AutoPaperSize
                            nXWidth = 0x7FFFFFFF;
                    }
                }
            }
        }

        // search for Portion that no longer fits in line...
        TextPortion* pPortion = nullptr;
        sal_Int32 nPortionLen = 0;
        bool bContinueLastPortion = false;
        bool bBrokenLine = false;
        bLineBreak = false;
        const EditCharAttrib* pNextFeature = pNode->GetCharAttribs().FindFeature( pLine->GetStart() );
        while ( ( nTmpWidth < nXWidth ) && !bEOL )
        {
            const sal_Int32 nTextPortions = pParaPortion->GetTextPortions().Count();
            assert(nTextPortions > 0);
            bContinueLastPortion = (nTmpPortion >= nTextPortions);
            if (bContinueLastPortion)
            {
                if (nTmpPos >= pNode->Len())
                    break;  // while

                // Continue with remainder. This only to have *some* valid
                // X-values and not endlessly create new lines until DOOM..
                // Happened in the scenario of tdf#104152 where inserting a
                // paragraph lead to a11y attempting to format the doc to
                // obtain content when notified.
                nTmpPortion = nTextPortions - 1;
                SAL_WARN("editeng","ImpEditEngine::CreateLines - continuation of a broken portion");
            }

            nPortionStart = nTmpPos;
            pPortion = &pParaPortion->GetTextPortions()[nTmpPortion];
            if ( !bContinueLastPortion && pPortion->GetKind() == PortionKind::HYPHENATOR )
            {
                // Throw away a Portion, if necessary correct the one before,
                // if the Hyph portion has swallowed a character...
                sal_Int32 nTmpLen = pPortion->GetLen();
                pParaPortion->GetTextPortions().Remove( nTmpPortion );
                if (nTmpPortion && nTmpLen)
                {
                    nTmpPortion--;
                    TextPortion& rPrev = pParaPortion->GetTextPortions()[nTmpPortion];
                    DBG_ASSERT( rPrev.GetKind() == PortionKind::TEXT, "Portion?!" );
                    nTmpWidth -= rPrev.GetSize().Width();
                    nTmpPos = nTmpPos - rPrev.GetLen();
                    rPrev.SetLen(rPrev.GetLen() + nTmpLen);
                    rPrev.GetSize().setWidth( -1 );
                }

                DBG_ASSERT( nTmpPortion < pParaPortion->GetTextPortions().Count(), "No more Portions left!" );
                pPortion = &pParaPortion->GetTextPortions()[nTmpPortion];
            }

            if (bContinueLastPortion)
            {
                // Note that this may point behind the portion and is only to
                // be used with the node's string offsets to generate X-values.
                nPortionLen = pNode->Len() - nPortionStart;
            }
            else
            {
                nPortionLen = pPortion->GetLen();
            }

            DBG_ASSERT( pPortion->GetKind() != PortionKind::HYPHENATOR, "CreateLines: Hyphenator-Portion!" );
            DBG_ASSERT( nPortionLen || bProcessingEmptyLine, "Empty Portion in CreateLines ?!" );
            if ( pNextFeature && ( pNextFeature->GetStart() == nTmpPos ) )
            {
                SAL_WARN_IF( bContinueLastPortion,
                        "editeng","ImpEditEngine::CreateLines - feature in continued portion will be wrong");
                sal_uInt16 nWhich = pNextFeature->GetItem()->Which();
                switch ( nWhich )
                {
                    case EE_FEATURE_TAB:
                    {
                        long nOldTmpWidth = nTmpWidth;

                        // Search for Tab-Pos...
                        long nCurPos = nTmpWidth+nStartX;
                        // consider scaling
                        if ( aStatus.DoStretch() && ( nStretchX != 100 ) )
                            nCurPos = nCurPos*100/std::max(static_cast<sal_Int32>(nStretchX), static_cast<sal_Int32>(1));

                        short nAllSpaceBeforeText = static_cast< short >(rLRItem.GetTextLeft()/* + rLRItem.GetTextLeft()*/ + nSpaceBeforeAndMinLabelWidth);
                        aCurrentTab.aTabStop = pNode->GetContentAttribs().FindTabStop( nCurPos - nAllSpaceBeforeText /*rLRItem.GetTextLeft()*/, aEditDoc.GetDefTab() );
                        aCurrentTab.nTabPos = GetXValue( static_cast<long>( aCurrentTab.aTabStop.GetTabPos() + nAllSpaceBeforeText /*rLRItem.GetTextLeft()*/ ) );
                        aCurrentTab.bValid = false;

                        // Switch direction in R2L para...
                        if ( bRightToLeftPara )
                        {
                            if ( aCurrentTab.aTabStop.GetAdjustment() == SvxTabAdjust::Right )
                                aCurrentTab.aTabStop.GetAdjustment() = SvxTabAdjust::Left;
                            else if ( aCurrentTab.aTabStop.GetAdjustment() == SvxTabAdjust::Left )
                                aCurrentTab.aTabStop.GetAdjustment() = SvxTabAdjust::Right;
                        }

                        if ( ( aCurrentTab.aTabStop.GetAdjustment() == SvxTabAdjust::Right ) ||
                             ( aCurrentTab.aTabStop.GetAdjustment() == SvxTabAdjust::Center ) ||
                             ( aCurrentTab.aTabStop.GetAdjustment() == SvxTabAdjust::Decimal ) )
                        {
                            // For LEFT / DEFAULT this tab is not considered.
                            aCurrentTab.bValid = true;
                            aCurrentTab.nStartPosX = nTmpWidth;
                            aCurrentTab.nTabPortion = nTmpPortion;
                        }

                        pPortion->SetKind(PortionKind::TAB);
                        pPortion->SetExtraValue( aCurrentTab.aTabStop.GetFill() );
                        pPortion->GetSize().setWidth( aCurrentTab.nTabPos - (nTmpWidth+nStartX) );

                        // Height needed...
                        SeekCursor( pNode, nTmpPos+1, aTmpFont );
                        pPortion->GetSize().setHeight( aTmpFont.QuickGetTextSize( GetRefDevice(), OUString(), 0, 0 ).Height() );

                        DBG_ASSERT( pPortion->GetSize().Width() >= 0, "Tab incorrectly calculated!" );

                        nTmpWidth = aCurrentTab.nTabPos-nStartX;

                        // If this is the first token on the line,
                        // and nTmpWidth > aPaperSize.Width, => infinite loop!
                        if ( ( nTmpWidth >= nXWidth ) && ( nTmpPortion == pLine->GetStartPortion() ) )
                        {
                            // What now?
                            // make the tab fitting
                            pPortion->GetSize().setWidth( nXWidth-nOldTmpWidth );
                            nTmpWidth = nXWidth-1;
                            bEOL = true;
                            bBrokenLine = true;
                        }
                        EditLine::CharPosArrayType& rArray = pLine->GetCharPosArray();
                        size_t nPos = nTmpPos - pLine->GetStart();
                        rArray.insert(rArray.begin()+nPos, pPortion->GetSize().Width());
                        bCompressedChars = false;
                    }
                    break;
                    case EE_FEATURE_LINEBR:
                    {
                        DBG_ASSERT( pPortion, "?!" );
                        pPortion->GetSize().setWidth( 0 );
                        bEOL = true;
                        bLineBreak = true;
                        pPortion->SetKind( PortionKind::LINEBREAK );
                        bCompressedChars = false;
                        EditLine::CharPosArrayType& rArray = pLine->GetCharPosArray();
                        size_t nPos = nTmpPos - pLine->GetStart();
                        rArray.insert(rArray.begin()+nPos, pPortion->GetSize().Width());
                    }
                    break;
                    case EE_FEATURE_FIELD:
                    {
                        SeekCursor( pNode, nTmpPos+1, aTmpFont );
                        aTmpFont.SetPhysFont( GetRefDevice() );
                        ImplInitDigitMode(GetRefDevice(), aTmpFont.GetLanguage());

                        OUString aFieldValue = static_cast<const EditCharAttribField*>(pNextFeature)->GetFieldValue();
                        // get size, but also DXArray to allow length information in line breaking below
                        const sal_Int32 nLength(aFieldValue.getLength());
                        std::unique_ptr<long[]> pTmpDXArray(new long[nLength]);
                        pPortion->GetSize() = aTmpFont.QuickGetTextSize(GetRefDevice(), aFieldValue, 0, aFieldValue.getLength(), pTmpDXArray.get());

                        // So no scrolling for oversized fields
                        if ( pPortion->GetSize().Width() > nXWidth )
                        {
                            // create ExtraPortionInfo on-demand, flush lineBreaksList
                            ExtraPortionInfo *pExtraInfo = pPortion->GetExtraInfos();

                            if(nullptr == pExtraInfo)
                            {
                                pExtraInfo = new ExtraPortionInfo();
                                pExtraInfo->nOrgWidth = nXWidth;
                                pPortion->SetExtraInfos(pExtraInfo);
                            }
                            else
                            {
                                pExtraInfo->lineBreaksList.clear();
                            }

                            // iterate over CellBreaks using XBreakIterator to be on the
                            // safe side with international texts/charSets
                            Reference < i18n::XBreakIterator > xBreakIterator(ImplGetBreakIterator());
                            const sal_Int32 nTextLength(aFieldValue.getLength());
                            const lang::Locale aLocale(GetLocale(EditPaM(pNode, nPortionStart)));
                            sal_Int32 nDone(0);
                            sal_Int32 nNextCellBreak(
                                xBreakIterator->nextCharacters(
                                        aFieldValue,
                                        0,
                                        aLocale,
                                        css::i18n::CharacterIteratorMode::SKIPCELL,
                                        0,
                                        nDone));
                            sal_Int32 nLastCellBreak(0);
                            sal_Int32 nLineStartX(0);

                            // always add 1st line break (safe, we already know we are larger than nXWidth)
                            pExtraInfo->lineBreaksList.push_back(0);

                            for(sal_Int32 a(0); a < nTextLength; a++)
                            {
                                if(a == nNextCellBreak)
                                {
                                    // check width
                                    if(pTmpDXArray[a] - nLineStartX > nXWidth)
                                    {
                                        // new CellBreak does not fit in current line, need to
                                        // create a break at LastCellBreak - but do not add 1st
                                        // line break twice for very tall frames
                                        if(0 != a)
                                        {
                                            pExtraInfo->lineBreaksList.push_back(a);
                                        }

                                        // moveLineStart forward in X
                                        nLineStartX = pTmpDXArray[nLastCellBreak];
                                    }

                                    // update CellBreak iteration values
                                    nLastCellBreak = a;
                                    nNextCellBreak = xBreakIterator->nextCharacters(
                                        aFieldValue,
                                        a,
                                        aLocale,
                                        css::i18n::CharacterIteratorMode::SKIPCELL,
                                        1,
                                        nDone);
                                }
                            }
                        }
                        nTmpWidth += pPortion->GetSize().Width();
                        EditLine::CharPosArrayType& rArray = pLine->GetCharPosArray();
                        size_t nPos = nTmpPos - pLine->GetStart();
                        rArray.insert(rArray.begin()+nPos, pPortion->GetSize().Width());
                        pPortion->SetKind(PortionKind::FIELD);
                        // If this is the first token on the line,
                        // and nTmpWidth > aPaperSize.Width, => infinite loop!
                        if ( ( nTmpWidth >= nXWidth ) && ( nTmpPortion == pLine->GetStartPortion() ) )
                        {
                            nTmpWidth = nXWidth-1;
                            bEOL = true;
                            bBrokenLine = true;
                        }
                        // Compression in Fields????
                        // I think this could be a little bit difficult and is not very useful
                        bCompressedChars = false;
                    }
                    break;
                    default:    OSL_FAIL( "What feature?" );
                }
                pNextFeature = pNode->GetCharAttribs().FindFeature( pNextFeature->GetStart() + 1  );
            }
            else
            {
                DBG_ASSERT( nPortionLen || bProcessingEmptyLine, "Empty Portion - Extra Space?!" );
                SeekCursor( pNode, nTmpPos+1, aTmpFont );
                aTmpFont.SetPhysFont( GetRefDevice() );
                ImplInitDigitMode(GetRefDevice(), aTmpFont.GetLanguage());

                if (!bContinueLastPortion)
                    pPortion->SetRightToLeftLevel( GetRightToLeft( nPara, nTmpPos+1 ) );

                if (bContinueLastPortion)
                {
                     Size aSize( aTmpFont.QuickGetTextSize( GetRefDevice(),
                            pParaPortion->GetNode()->GetString(), nTmpPos, nPortionLen, pBuf.get() ));
                     pPortion->GetSize().AdjustWidth(aSize.Width() );
                     if (pPortion->GetSize().Height() < aSize.Height())
                         pPortion->GetSize().setHeight( aSize.Height() );
                }
                else
                {
                    pPortion->GetSize() = aTmpFont.QuickGetTextSize( GetRefDevice(),
                            pParaPortion->GetNode()->GetString(), nTmpPos, nPortionLen, pBuf.get() );
                }

                // #i9050# Do Kerning also behind portions...
                if ( ( aTmpFont.GetFixKerning() > 0 ) && ( ( nTmpPos + nPortionLen ) < pNode->Len() ) )
                    pPortion->GetSize().AdjustWidth(aTmpFont.GetFixKerning() );
                if ( IsFixedCellHeight() )
                    pPortion->GetSize().setHeight( ImplCalculateFontIndependentLineSpacing( aTmpFont.GetFontHeight() ) );
                // The array is  generally flattened at the beginning
                // => Always simply quick inserts.
                size_t nPos = nTmpPos - pLine->GetStart();
                EditLine::CharPosArrayType& rArray = pLine->GetCharPosArray();
                rArray.insert( rArray.begin() + nPos, pBuf.get(), pBuf.get() + nPortionLen);

                // And now check for Compression:
                if ( !bContinueLastPortion && nPortionLen && GetAsianCompressionMode() != CharCompressType::NONE )
                {
                    long* pDXArray = rArray.data() + nTmpPos - pLine->GetStart();
                    bCompressedChars |= ImplCalcAsianCompression(
                        pNode, pPortion, nTmpPos, pDXArray, 10000, false);
                }

                nTmpWidth += pPortion->GetSize().Width();

                sal_Int32 _nPortionEnd = nTmpPos + nPortionLen;
                if( bScriptSpace && ( _nPortionEnd < pNode->Len() ) && ( nTmpWidth < nXWidth ) && IsScriptChange( EditPaM( pNode, _nPortionEnd ) ) )
                {
                    bool bAllow = false;
                    sal_uInt16 nScriptTypeLeft = GetI18NScriptType( EditPaM( pNode, _nPortionEnd ) );
                    sal_uInt16 nScriptTypeRight = GetI18NScriptType( EditPaM( pNode, _nPortionEnd+1 ) );
                    if ( ( nScriptTypeLeft == i18n::ScriptType::ASIAN ) || ( nScriptTypeRight == i18n::ScriptType::ASIAN ) )
                        bAllow = true;

                    // No spacing within L2R/R2L nesting
                    if ( bAllow )
                    {
                        long nExtraSpace = pPortion->GetSize().Height()/5;
                        nExtraSpace = GetXValue( nExtraSpace );
                        pPortion->GetSize().AdjustWidth(nExtraSpace );
                        nTmpWidth += nExtraSpace;
                    }
                }
            }

            if ( aCurrentTab.bValid && ( nTmpPortion != aCurrentTab.nTabPortion ) )
            {
                long nWidthAfterTab = 0;
                for ( sal_Int32 n = aCurrentTab.nTabPortion+1; n <= nTmpPortion; n++  )
                {
                    const TextPortion& rTP = pParaPortion->GetTextPortions()[n];
                    nWidthAfterTab += rTP.GetSize().Width();
                }
                long nW = nWidthAfterTab;   // Length before tab position
                if ( aCurrentTab.aTabStop.GetAdjustment() == SvxTabAdjust::Right )
                {
                }
                else if ( aCurrentTab.aTabStop.GetAdjustment() == SvxTabAdjust::Center )
                {
                    nW = nWidthAfterTab/2;
                }
                else if ( aCurrentTab.aTabStop.GetAdjustment() == SvxTabAdjust::Decimal )
                {
                    OUString aText = GetSelected( EditSelection(  EditPaM( pParaPortion->GetNode(), nTmpPos ),
                                                                EditPaM( pParaPortion->GetNode(), nTmpPos + nPortionLen ) ) );
                    sal_Int32 nDecPos = aText.indexOf( aCurrentTab.aTabStop.GetDecimal() );
                    if ( nDecPos != -1 )
                    {
                        nW -= pParaPortion->GetTextPortions()[nTmpPortion].GetSize().Width();
                        nW += aTmpFont.QuickGetTextSize( GetRefDevice(), pParaPortion->GetNode()->GetString(), nTmpPos, nDecPos ).Width();
                        aCurrentTab.bValid = false;
                    }
                }
                else
                {
                    OSL_FAIL( "CreateLines: Tab not handled!" );
                }
                long nMaxW = aCurrentTab.nTabPos - aCurrentTab.nStartPosX - nStartX;
                if ( nW >= nMaxW )
                {
                    nW = nMaxW;
                    aCurrentTab.bValid = false;
                }
                TextPortion& rTabPortion = pParaPortion->GetTextPortions()[aCurrentTab.nTabPortion];
                rTabPortion.GetSize().setWidth( aCurrentTab.nTabPos - aCurrentTab.nStartPosX - nW - nStartX );
                nTmpWidth = aCurrentTab.nStartPosX + rTabPortion.GetSize().Width() + nWidthAfterTab;
            }

            nTmpPos = nTmpPos + nPortionLen;
            nPortionEnd = nTmpPos;
            nTmpPortion++;
            if ( aStatus.OneCharPerLine() )
                bEOL = true;
        }

        DBG_ASSERT( pPortion, "no portion!?" );

        aCurrentTab.bValid = false;

        // this was possibly a portion too far:
        bool bFixedEnd = false;
        if ( aStatus.OneCharPerLine() )
        {
            // State before Portion (apart from nTmpWidth):
            nTmpPos -= pPortion ? nPortionLen : 0;
            nPortionStart = nTmpPos;
            nTmpPortion--;

            bEOL = true;
            bEOC = false;

            // And now just one character:
            nTmpPos++;
            nTmpPortion++;
            nPortionEnd = nTmpPortion;
            // one Non-Feature-Portion has to be wrapped
            if ( pPortion && nPortionLen > 1 )
            {
                DBG_ASSERT( pPortion->GetKind() == PortionKind::TEXT, "Len>1, but no TextPortion?" );
                nTmpWidth -= pPortion->GetSize().Width();
                sal_Int32 nP = SplitTextPortion( pParaPortion, nTmpPos, pLine );
                nTmpWidth += pParaPortion->GetTextPortions()[nP].GetSize().Width();
            }
        }
        else if ( nTmpWidth >= nXWidth )
        {
            nPortionEnd = nTmpPos;
            nTmpPos -= pPortion ? nPortionLen : 0;
            nPortionStart = nTmpPos;
            nTmpPortion--;
            bEOL = false;
            bEOC = false;
            if( pPortion ) switch ( pPortion->GetKind() )
            {
                case PortionKind::TEXT:
                {
                    nTmpWidth -= pPortion->GetSize().Width();
                }
                break;
                case PortionKind::FIELD:
                case PortionKind::TAB:
                {
                    nTmpWidth -= pPortion->GetSize().Width();
                    bEOL = true;
                    bFixedEnd = true;
                }
                break;
                default:
                {
                    //  A feature is not wrapped:
                    DBG_ASSERT( ( pPortion->GetKind() == PortionKind::LINEBREAK ), "What Feature ?" );
                    bEOL = true;
                    bFixedEnd = true;
                }
            }
        }
        else
        {
            bEOL = true;
            bEOC = true;
            pLine->SetEnd( nPortionEnd );
            DBG_ASSERT( pParaPortion->GetTextPortions().Count(), "No TextPortions?" );
            pLine->SetEndPortion( pParaPortion->GetTextPortions().Count() - 1 );
        }

        if ( aStatus.OneCharPerLine() )
        {
            pLine->SetEnd( nPortionEnd );
            pLine->SetEndPortion( nTmpPortion-1 );
        }
        else if ( bFixedEnd )
        {
            pLine->SetEnd( nPortionStart );
            pLine->SetEndPortion( nTmpPortion-1 );
        }
        else if ( bLineBreak || bBrokenLine )
        {
            pLine->SetEnd( nPortionStart+1 );
            pLine->SetEndPortion( nTmpPortion-1 );
            bEOC = false; // was set above, maybe change the sequence of the if's?
        }
        else if ( !bEOL && !bContinueLastPortion )
        {
            DBG_ASSERT( pPortion && ((nPortionEnd-nPortionStart) == pPortion->GetLen()), "However, another portion?!" );
            long nRemainingWidth = nMaxLineWidth - nTmpWidth;
            bool bCanHyphenate = ( aTmpFont.GetCharSet() != RTL_TEXTENCODING_SYMBOL );
            if ( bCompressedChars && pPortion && ( pPortion->GetLen() > 1 ) && pPortion->GetExtraInfos() && pPortion->GetExtraInfos()->bCompressed )
            {
                // I need the manipulated DXArray for determining the break position...
                long* pDXArray = pLine->GetCharPosArray().data() + (nPortionStart - pLine->GetStart());
                ImplCalcAsianCompression(
                    pNode, pPortion, nPortionStart, pDXArray, 10000, true);
            }
            if( pPortion )
                ImpBreakLine( pParaPortion, pLine, pPortion, nPortionStart,
                                                nRemainingWidth, bCanHyphenate && bHyphenatePara );
        }


        // Line finished => adjust


        // CalcTextSize should be replaced by a continuous registering!
        Size aTextSize = pLine->CalcTextSize( *pParaPortion );

        if ( aTextSize.Height() == 0 )
        {
            SeekCursor( pNode, pLine->GetStart()+1, aTmpFont );
            aTmpFont.SetPhysFont( pRefDev );
            ImplInitDigitMode(pRefDev, aTmpFont.GetLanguage());

            if ( IsFixedCellHeight() )
                aTextSize.setHeight( ImplCalculateFontIndependentLineSpacing( aTmpFont.GetFontHeight() ) );
            else
                aTextSize.setHeight( aTmpFont.GetPhysTxtSize( pRefDev ).Height() );
            pLine->SetHeight( static_cast<sal_uInt16>(aTextSize.Height()) );
        }

        // The font metrics can not be calculated continuously, if the font is
        // set anyway, because a large font only after wrapping suddenly ends
        // up in the next line => Font metrics too big.
        FormatterFontMetric aFormatterMetrics;
        sal_Int32 nTPos = pLine->GetStart();
        for ( sal_Int32 nP = pLine->GetStartPortion(); nP <= pLine->GetEndPortion(); nP++ )
        {
            const TextPortion& rTP = pParaPortion->GetTextPortions()[nP];
            // problem with hard font height attribute, when everything but the line break has this attribute
            if ( rTP.GetKind() != PortionKind::LINEBREAK )
            {
                SeekCursor( pNode, nTPos+1, aTmpFont );
                aTmpFont.SetPhysFont( GetRefDevice() );
                ImplInitDigitMode(GetRefDevice(), aTmpFont.GetLanguage());
                RecalcFormatterFontMetrics( aFormatterMetrics, aTmpFont );
            }
            nTPos = nTPos + rTP.GetLen();
        }
        sal_uInt16 nLineHeight = aFormatterMetrics.GetHeight();
        if ( nLineHeight > pLine->GetHeight() )
            pLine->SetHeight( nLineHeight );
        pLine->SetMaxAscent( aFormatterMetrics.nMaxAscent );

        bSameLineAgain = false;
        if ( GetTextRanger() && ( pLine->GetHeight() > nTextLineHeight ) )
        {
            // put down with the other size!
            bSameLineAgain = true;
        }


        if ( !bSameLineAgain && !aStatus.IsOutliner() )
        {
            if ( rLSItem.GetLineSpaceRule() == SvxLineSpaceRule::Min )
            {
                sal_uInt16 nMinHeight = GetYValue( rLSItem.GetLineHeight() );
                sal_uInt16 nTxtHeight = pLine->GetHeight();
                if ( nTxtHeight < nMinHeight )
                {
                    // The Ascent has to be adjusted for the difference:
                    long nDiff = nMinHeight - nTxtHeight;
                    pLine->SetMaxAscent( static_cast<sal_uInt16>(pLine->GetMaxAscent() + nDiff) );
                    pLine->SetHeight( nMinHeight, nTxtHeight );
                }
            }
            else if ( rLSItem.GetLineSpaceRule() == SvxLineSpaceRule::Fix )
            {
                sal_uInt16 nFixHeight = GetYValue( rLSItem.GetLineHeight() );
                sal_uInt16 nTxtHeight = pLine->GetHeight();
                pLine->SetMaxAscent( static_cast<sal_uInt16>(pLine->GetMaxAscent() + ( nFixHeight - nTxtHeight ) ) );
                pLine->SetHeight( nFixHeight, nTxtHeight );
            }
            else if ( rLSItem.GetInterLineSpaceRule() == SvxInterLineSpaceRule::Prop )
            {
                // There are documents with PropLineSpace 0, why?
                // (cmc: re above question :-) such documents can be seen by importing a .ppt
                if ( rLSItem.GetPropLineSpace() && ( rLSItem.GetPropLineSpace() < 100 ) )
                {
                    // Adapted code from sw/source/core/text/itrform2.cxx
                    sal_uInt16 nPropLineSpace = rLSItem.GetPropLineSpace();
                    sal_uInt16 nAscent = pLine->GetMaxAscent();
                    sal_uInt16 nNewAscent = pLine->GetTxtHeight() * nPropLineSpace / 100 * 4 / 5; // 80%
                    if ( !nAscent || nAscent > nNewAscent )
                    {
                        pLine->SetMaxAscent( nNewAscent );
                    }
                    sal_uInt16 nHeight = pLine->GetHeight() * nPropLineSpace / 100;
                    pLine->SetHeight( nHeight, pLine->GetTxtHeight() );
                }
                else if ( rLSItem.GetPropLineSpace() && ( rLSItem.GetPropLineSpace() != 100 ) )
                {
                    sal_uInt16 nTxtHeight = pLine->GetHeight();
                    sal_Int32 nPropTextHeight = nTxtHeight * rLSItem.GetPropLineSpace() / 100;
                    // The Ascent has to be adjusted for the difference:
                    long nDiff = pLine->GetHeight() - nPropTextHeight;
                    pLine->SetMaxAscent( static_cast<sal_uInt16>( pLine->GetMaxAscent() - nDiff ) );
                    pLine->SetHeight( static_cast<sal_uInt16>( nPropTextHeight ), nTxtHeight );
                }
            }
        }

        if ( ( !IsVertical() && aStatus.AutoPageWidth() ) ||
             ( IsVertical() && aStatus.AutoPageHeight() ) )
        {
            // If the row fits within the current paper width, then this width
            // has to be used for the Alignment. If it does not fit or if it
            // will change the paper width, it will be formatted again for
            // Justification! = LEFT anyway.
            long nMaxLineWidthFix = ( !IsVertical() ? aPaperSize.Width() : aPaperSize.Height() )
                                        - GetXValue( rLRItem.GetRight() ) - nStartX;
            if ( aTextSize.Width() < nMaxLineWidthFix )
                nMaxLineWidth = nMaxLineWidthFix;
        }

        if ( bCompressedChars )
        {
            long nRemainingWidth = nMaxLineWidth - aTextSize.Width();
            if ( nRemainingWidth > 0 )
            {
                ImplExpandCompressedPortions( pLine, pParaPortion, nRemainingWidth );
                aTextSize = pLine->CalcTextSize( *pParaPortion );
            }
        }

        if ( pLine->IsHangingPunctuation() )
        {
            // Width from HangingPunctuation was set to 0 in ImpBreakLine,
            // check for rel width now, maybe create compression...
            long n = nMaxLineWidth - aTextSize.Width();
            TextPortion& rTP = pParaPortion->GetTextPortions()[pLine->GetEndPortion()];
            sal_Int32 nPosInArray = pLine->GetEnd()-1-pLine->GetStart();
            long nNewValue = ( nPosInArray ? pLine->GetCharPosArray()[ nPosInArray-1 ] : 0 ) + n;
            pLine->GetCharPosArray()[ nPosInArray ] = nNewValue;
            rTP.GetSize().AdjustWidth(n );
        }

        pLine->SetTextWidth( aTextSize.Width() );
        switch ( eJustification )
        {
            case SvxAdjust::Center:
            {
                long n = ( nMaxLineWidth - aTextSize.Width() ) / 2;
                n += nStartX;  // Indentation is kept.
                pLine->SetStartPosX( n );
            }
            break;
            case SvxAdjust::Right:
            {
                // For automatically wrapped lines, which has a blank at the end
                // the blank must not be displayed!
                long n = nMaxLineWidth - aTextSize.Width();
                n += nStartX;  // Indentation is kept.
                pLine->SetStartPosX( n );
            }
            break;
            case SvxAdjust::Block:
            {
                bool bDistLastLine = (GetJustifyMethod(nPara) == SvxCellJustifyMethod::Distribute);
                long nRemainingSpace = nMaxLineWidth - aTextSize.Width();
                pLine->SetStartPosX( nStartX );
                if ( nRemainingSpace > 0 && (!bEOC || bDistLastLine) )
                    ImpAdjustBlocks( pParaPortion, pLine, nRemainingSpace );
            }
            break;
            default:
            {
                pLine->SetStartPosX( nStartX ); // FI, LI
            }
            break;
        }


        // Check whether the line must be re-issued...

        pLine->SetInvalid();

        // If a portion was wrapped there may be far too many positions in
        // CharPosArray:
        EditLine::CharPosArrayType& rArray = pLine->GetCharPosArray();
        size_t nLen = pLine->GetLen();
        if (rArray.size() > nLen)
            rArray.erase(rArray.begin()+nLen, rArray.end());

        if ( GetTextRanger() )
        {
            if ( nTextXOffset )
                pLine->SetStartPosX( pLine->GetStartPosX() + nTextXOffset );
            if ( nTextExtraYOffset )
            {
                pLine->SetHeight( static_cast<sal_uInt16>( pLine->GetHeight() + nTextExtraYOffset ), 0 );
                pLine->SetMaxAscent( static_cast<sal_uInt16>( pLine->GetMaxAscent() + nTextExtraYOffset ) );
            }
        }

        // for <0 think over !
        if ( pParaPortion->IsSimpleInvalid() )
        {
            // Change through simple Text changes...
            // Do not cancel formatting since Portions possibly have to be split
            // again! If at some point cancelable, then validate the following
            // line! But if applicable, mark as valid, so there is less output...
            if ( pLine->GetEnd() < nInvalidStart )
            {
                if ( *pLine == aSaveLine )
                {
                    pLine->SetValid();
                }
            }
            else
            {
                sal_Int32 nStart = pLine->GetStart();
                sal_Int32 nEnd = pLine->GetEnd();

                if ( nStart > nInvalidEnd )
                {
                    if ( ( ( nStart-nInvalidDiff ) == aSaveLine.GetStart() ) &&
                            ( ( nEnd-nInvalidDiff ) == aSaveLine.GetEnd() ) )
                    {
                        pLine->SetValid();
                        if (bQuickFormat)
                        {
                            bLineBreak = false;
                            pParaPortion->CorrectValuesBehindLastFormattedLine( nLine );
                            break;
                        }
                    }
                }
                else if (bQuickFormat && (nEnd > nInvalidEnd))
                {
                    // If the invalid line ends so that the next begins on the
                    // 'same' passage as before, i.e. not wrapped differently,
                    //  then the text width does not have to be determined anew:
                    if ( nEnd == ( aSaveLine.GetEnd() + nInvalidDiff ) )
                    {
                        bLineBreak = false;
                        pParaPortion->CorrectValuesBehindLastFormattedLine( nLine );
                        break;
                    }
                }
            }
        }

        if ( !bSameLineAgain )
        {
            nIndex = pLine->GetEnd();   // next line start = last line end
                                        // as nEnd points to the last character!

            sal_Int32 nEndPortion = pLine->GetEndPortion();

            // Next line or maybe a new line...
            pLine = nullptr;
            if ( nLine < pParaPortion->GetLines().Count()-1 )
                pLine = &pParaPortion->GetLines()[++nLine];
            if ( pLine && ( nIndex >= pNode->Len() ) )
            {
                nDelFromLine = nLine;
                break;
            }
            if ( !pLine )
            {
                if ( nIndex < pNode->Len() )
                {
                    pLine = new EditLine;
                    pParaPortion->GetLines().Insert(++nLine, pLine);
                }
                else if ( nIndex && bLineBreak && GetTextRanger() )
                {
                    // normally CreateAndInsertEmptyLine would be called, but I want to use
                    // CreateLines, so I need Polygon code only here...
                    TextPortion* pDummyPortion = new TextPortion( 0 );
                    pParaPortion->GetTextPortions().Append(pDummyPortion);
                    pLine = new EditLine;
                    pParaPortion->GetLines().Insert(++nLine, pLine);
                    bForceOneRun = true;
                    bProcessingEmptyLine = true;
                }
            }
            if ( pLine )
            {
                aSaveLine = *pLine;
                pLine->SetStart( nIndex );
                pLine->SetEnd( nIndex );
                pLine->SetStartPortion( nEndPortion+1 );
                pLine->SetEndPortion( nEndPortion+1 );
            }
        }
    }   // while ( Index < Len )

    if ( nDelFromLine >= 0 )
        pParaPortion->GetLines().DeleteFromLine( nDelFromLine );

    DBG_ASSERT( pParaPortion->GetLines().Count(), "No line after CreateLines!" );

    if ( bLineBreak )
        CreateAndInsertEmptyLine( pParaPortion );

    pBuf.reset();

    bool bHeightChanged = FinishCreateLines( pParaPortion );

    if ( bMapChanged )
        GetRefDevice()->Pop();

    GetRefDevice()->Pop();

    return bHeightChanged;
}

void ImpEditEngine::CreateAndInsertEmptyLine( ParaPortion* pParaPortion )
{
    DBG_ASSERT( !GetTextRanger(), "Don't use CreateAndInsertEmptyLine with a polygon!" );

    EditLine* pTmpLine = new EditLine;
    pTmpLine->SetStart( pParaPortion->GetNode()->Len() );
    pTmpLine->SetEnd( pParaPortion->GetNode()->Len() );
    pParaPortion->GetLines().Append(pTmpLine);

    bool bLineBreak = pParaPortion->GetNode()->Len() > 0;
    sal_Int32 nSpaceBefore = 0;
    sal_Int32 nSpaceBeforeAndMinLabelWidth = GetSpaceBeforeAndMinLabelWidth( pParaPortion->GetNode(), &nSpaceBefore );
    const SvxLRSpaceItem& rLRItem = GetLRSpaceItem( pParaPortion->GetNode() );
    const SvxLineSpacingItem& rLSItem = pParaPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_SBL );
    long nStartX = GetXValue( rLRItem.GetTextLeft() + rLRItem.GetTextFirstLineOfst() + nSpaceBefore );

    tools::Rectangle aBulletArea = tools::Rectangle( Point(), Point() );
    if ( bLineBreak )
    {
        nStartX = GetXValue( rLRItem.GetTextLeft() + rLRItem.GetTextFirstLineOfst() + nSpaceBeforeAndMinLabelWidth );
    }
    else
    {
        aBulletArea = GetEditEnginePtr()->GetBulletArea( GetParaPortions().GetPos( pParaPortion ) );
        if ( aBulletArea.Right() > 0 )
            pParaPortion->SetBulletX( static_cast<sal_Int32>(GetXValue( aBulletArea.Right() )) );
        else
            pParaPortion->SetBulletX( 0 ); // If Bullet set incorrectly.
        if ( pParaPortion->GetBulletX() > nStartX )
        {
            nStartX = GetXValue( rLRItem.GetTextLeft() + rLRItem.GetTextFirstLineOfst() + nSpaceBeforeAndMinLabelWidth );
            if ( pParaPortion->GetBulletX() > nStartX )
                nStartX = pParaPortion->GetBulletX();
        }
    }

    SvxFont aTmpFont;
    SeekCursor( pParaPortion->GetNode(), bLineBreak ? pParaPortion->GetNode()->Len() : 0, aTmpFont );
    aTmpFont.SetPhysFont( pRefDev );

    TextPortion* pDummyPortion = new TextPortion( 0 );
    pDummyPortion->GetSize() = aTmpFont.GetPhysTxtSize( pRefDev );
    if ( IsFixedCellHeight() )
        pDummyPortion->GetSize().setHeight( ImplCalculateFontIndependentLineSpacing( aTmpFont.GetFontHeight() ) );
    pParaPortion->GetTextPortions().Append(pDummyPortion);
    FormatterFontMetric aFormatterMetrics;
    RecalcFormatterFontMetrics( aFormatterMetrics, aTmpFont );
    pTmpLine->SetMaxAscent( aFormatterMetrics.nMaxAscent );
    pTmpLine->SetHeight( static_cast<sal_uInt16>(pDummyPortion->GetSize().Height()) );
    sal_uInt16 nLineHeight = aFormatterMetrics.GetHeight();
    if ( nLineHeight > pTmpLine->GetHeight() )
        pTmpLine->SetHeight( nLineHeight );

    if ( !aStatus.IsOutliner() )
    {
        sal_Int32 nPara = GetParaPortions().GetPos( pParaPortion );
        SvxAdjust eJustification = GetJustification( nPara );
        long nMaxLineWidth = !IsVertical() ? aPaperSize.Width() : aPaperSize.Height();
        nMaxLineWidth -= GetXValue( rLRItem.GetRight() );
        if ( nMaxLineWidth < 0 )
            nMaxLineWidth = 1;
        if ( eJustification ==  SvxAdjust::Center )
            nStartX = nMaxLineWidth / 2;
        else if ( eJustification ==  SvxAdjust::Right )
            nStartX = nMaxLineWidth;
    }

    pTmpLine->SetStartPosX( nStartX );

    if ( !aStatus.IsOutliner() )
    {
        if ( rLSItem.GetLineSpaceRule() == SvxLineSpaceRule::Min )
        {
            sal_uInt16 nMinHeight = rLSItem.GetLineHeight();
            sal_uInt16 nTxtHeight = pTmpLine->GetHeight();
            if ( nTxtHeight < nMinHeight )
            {
                // The Ascent has to be adjusted for the difference:
                long nDiff = nMinHeight - nTxtHeight;
                pTmpLine->SetMaxAscent( static_cast<sal_uInt16>(pTmpLine->GetMaxAscent() + nDiff) );
                pTmpLine->SetHeight( nMinHeight, nTxtHeight );
            }
        }
        else if ( rLSItem.GetLineSpaceRule() == SvxLineSpaceRule::Fix )
        {
            sal_uInt16 nFixHeight = rLSItem.GetLineHeight();
            sal_uInt16 nTxtHeight = pTmpLine->GetHeight();

            pTmpLine->SetMaxAscent( static_cast<sal_uInt16>(pTmpLine->GetMaxAscent() + ( nFixHeight - nTxtHeight ) ) );
            pTmpLine->SetHeight( nFixHeight, nTxtHeight );
        }
        else if ( rLSItem.GetInterLineSpaceRule() == SvxInterLineSpaceRule::Prop )
        {
            sal_Int32 nPara = GetParaPortions().GetPos( pParaPortion );
            if ( nPara || pTmpLine->GetStartPortion() ) // Not the very first line
            {
                // There are documents with PropLineSpace 0, why?
                // (cmc: re above question :-) such documents can be seen by importing a .ppt
                if ( rLSItem.GetPropLineSpace() && ( rLSItem.GetPropLineSpace() != 100 ) )
                {
                    sal_uInt16 nTxtHeight = pTmpLine->GetHeight();
                    sal_Int32 nH = nTxtHeight;
                    nH *= rLSItem.GetPropLineSpace();
                    nH /= 100;
                    // The Ascent has to be adjusted for the difference:
                    long nDiff = pTmpLine->GetHeight() - nH;
                    if ( nDiff > pTmpLine->GetMaxAscent() )
                        nDiff = pTmpLine->GetMaxAscent();
                    pTmpLine->SetMaxAscent( static_cast<sal_uInt16>(pTmpLine->GetMaxAscent() - nDiff) );
                    pTmpLine->SetHeight( static_cast<sal_uInt16>(nH), nTxtHeight );
                }
            }
        }
    }

    if ( !bLineBreak )
    {
        long nMinHeight = aBulletArea.GetHeight();
        if ( nMinHeight > static_cast<long>(pTmpLine->GetHeight()) )
        {
            long nDiff = nMinHeight - static_cast<long>(pTmpLine->GetHeight());
            // distribute nDiff upwards and downwards
            pTmpLine->SetMaxAscent( static_cast<sal_uInt16>(pTmpLine->GetMaxAscent() + nDiff/2) );
            pTmpLine->SetHeight( static_cast<sal_uInt16>(nMinHeight) );
        }
    }
    else
    {
        // -2: The new one is already inserted.
#ifdef DBG_UTIL
        EditLine& rLastLine = pParaPortion->GetLines()[pParaPortion->GetLines().Count()-2];
        DBG_ASSERT( rLastLine.GetEnd() == pParaPortion->GetNode()->Len(), "different anyway?" );
#endif
        sal_Int32 nPos = pParaPortion->GetTextPortions().Count() - 1 ;
        pTmpLine->SetStartPortion( nPos );
        pTmpLine->SetEndPortion( nPos );
    }
}

bool ImpEditEngine::FinishCreateLines( ParaPortion* pParaPortion )
{
//  CalcCharPositions( pParaPortion );
    pParaPortion->SetValid();
    long nOldHeight = pParaPortion->GetHeight();
    CalcHeight( pParaPortion );

    DBG_ASSERT( pParaPortion->GetTextPortions().Count(), "FinishCreateLines: No Text-Portion?" );
    bool bRet = ( pParaPortion->GetHeight() != nOldHeight );
    return bRet;
}

void ImpEditEngine::ImpBreakLine( ParaPortion* pParaPortion, EditLine* pLine, TextPortion const * pPortion, sal_Int32 nPortionStart, long nRemainingWidth, bool bCanHyphenate )
{
    ContentNode* const pNode = pParaPortion->GetNode();

    sal_Int32 nBreakInLine = nPortionStart - pLine->GetStart();
    sal_Int32 nMax = nBreakInLine + pPortion->GetLen();
    while ( ( nBreakInLine < nMax ) && ( pLine->GetCharPosArray()[nBreakInLine] < nRemainingWidth ) )
        nBreakInLine++;

    sal_Int32 nMaxBreakPos = nBreakInLine + pLine->GetStart();
    sal_Int32 nBreakPos = SAL_MAX_INT32;

    bool bCompressBlank = false;
    bool bHyphenated = false;
    bool bHangingPunctuation = false;
    sal_Unicode cAlternateReplChar = 0;
    sal_Unicode cAlternateExtraChar = 0;
    bool bAltFullLeft = false;
    bool bAltFullRight = false;
    sal_uInt32 nAltDelChar = 0;

    if ( ( nMaxBreakPos < ( nMax + pLine->GetStart() ) ) && ( pNode->GetChar( nMaxBreakPos ) == ' ' ) )
    {
        // Break behind the blank, blank will be compressed...
        nBreakPos = nMaxBreakPos + 1;
        bCompressBlank = true;
    }
    else
    {
        sal_Int32 nMinBreakPos = pLine->GetStart();
        const CharAttribList::AttribsType& rAttrs = pNode->GetCharAttribs().GetAttribs();
        for (size_t nAttr = rAttrs.size(); nAttr; )
        {
            const EditCharAttrib& rAttr = *rAttrs[--nAttr].get();
            if (rAttr.IsFeature() && rAttr.GetEnd() > nMinBreakPos && rAttr.GetEnd() <= nMaxBreakPos)
            {
                nMinBreakPos = rAttr.GetEnd();
                break;
            }
        }
        assert(nMinBreakPos <= nMaxBreakPos);

        lang::Locale aLocale = GetLocale( EditPaM( pNode, nMaxBreakPos ) );

        Reference < i18n::XBreakIterator > _xBI( ImplGetBreakIterator() );
        const bool bAllowPunctuationOutsideMargin = static_cast<const SfxBoolItem&>(
                pNode->GetContentAttribs().GetItem( EE_PARA_HANGINGPUNCTUATION )).GetValue();

        if (nMinBreakPos == nMaxBreakPos)
        {
            nBreakPos = nMinBreakPos;
        }
        else
        {
            Reference< XHyphenator > xHyph;
            if ( bCanHyphenate )
                xHyph = GetHyphenator();
            i18n::LineBreakHyphenationOptions aHyphOptions( xHyph, Sequence< PropertyValue >(), 1 );
            i18n::LineBreakUserOptions aUserOptions;

            const i18n::ForbiddenCharacters* pForbidden = GetForbiddenCharsTable()->GetForbiddenCharacters( LanguageTag::convertToLanguageType( aLocale ), true );
            aUserOptions.forbiddenBeginCharacters = pForbidden->beginLine;
            aUserOptions.forbiddenEndCharacters = pForbidden->endLine;
            aUserOptions.applyForbiddenRules = static_cast<const SfxBoolItem&>(pNode->GetContentAttribs().GetItem( EE_PARA_FORBIDDENRULES )).GetValue();
            aUserOptions.allowPunctuationOutsideMargin = bAllowPunctuationOutsideMargin;
            aUserOptions.allowHyphenateEnglish = false;

            i18n::LineBreakResults aLBR = _xBI->getLineBreak(
                pNode->GetString(), nMaxBreakPos, aLocale, nMinBreakPos, aHyphOptions, aUserOptions );
            nBreakPos = aLBR.breakIndex;

            // BUG in I18N - under special condition (break behind field, #87327#) breakIndex is < nMinBreakPos
            if ( nBreakPos < nMinBreakPos )
            {
                nBreakPos = nMinBreakPos;
            }
            else if ( ( nBreakPos > nMaxBreakPos ) && !aUserOptions.allowPunctuationOutsideMargin )
            {
                OSL_FAIL( "I18N: XBreakIterator::getLineBreak returns position > Max" );
                nBreakPos = nMaxBreakPos;
            }

            // nBreakPos can never be outside the portion, even not with hanging punctuation
            if ( nBreakPos > nMaxBreakPos )
                nBreakPos = nMaxBreakPos;
        }

        // BUG in I18N - the japanese dot is in the next line!
        // !!!  Test!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        if ( (nBreakPos + ( bAllowPunctuationOutsideMargin ? 0 : 1 ) ) <= nMaxBreakPos )
        {
            sal_Unicode cFirstInNextLine = ( (nBreakPos+1) < pNode->Len() ) ? pNode->GetChar( nBreakPos ) : 0;
            if ( cFirstInNextLine == 12290 )
                nBreakPos++;
        }

        bHangingPunctuation = nBreakPos > nMaxBreakPos;
        pLine->SetHangingPunctuation( bHangingPunctuation );

        // Whether a separator or not, push the word after the separator through
        // hyphenation... NMaxBreakPos is the last character that fits into
        // the line, nBreakPos is the beginning of the word.
        // There is a problem if the Doc is so narrow that a word is broken
        // into more than two lines...
        if ( !bHangingPunctuation && bCanHyphenate && GetHyphenator().is() )
        {
            i18n::Boundary aBoundary = _xBI->getWordBoundary(
                pNode->GetString(), nBreakPos, GetLocale( EditPaM( pNode, nBreakPos ) ), css::i18n::WordType::DICTIONARY_WORD, true);
            sal_Int32 nWordStart = nBreakPos;
            sal_Int32 nWordEnd = aBoundary.endPos;
            DBG_ASSERT( nWordEnd >= nWordStart, "Start >= End?" );

            sal_Int32 nWordLen = nWordEnd - nWordStart;
            if ( ( nWordEnd >= nMaxBreakPos ) && ( nWordLen > 3 ) )
            {
                // May happen, because getLineBreak may differ from getWordBoudary with DICTIONARY_WORD
                const OUString aWord = pNode->GetString().copy(nWordStart, nWordLen);
                sal_Int32 nMinTrail = nWordEnd-nMaxBreakPos+1; //+1: Before the dickey letter
                Reference< XHyphenatedWord > xHyphWord;
                if (xHyphenator.is())
                    xHyphWord = xHyphenator->hyphenate( aWord, aLocale, aWord.getLength() - nMinTrail, Sequence< PropertyValue >() );
                if (xHyphWord.is())
                {
                    bool bAlternate = xHyphWord->isAlternativeSpelling();
                    sal_Int32 _nWordLen = 1 + xHyphWord->getHyphenPos();

                    if ( ( _nWordLen >= 2 ) && ( (nWordStart+_nWordLen) >= (pLine->GetStart() + 2 ) ) )
                    {
                        if ( !bAlternate )
                        {
                            bHyphenated = true;
                            nBreakPos = nWordStart + _nWordLen;
                        }
                        else
                        {
                            // TODO: handle all alternative hyphenations (see hyphen-1.2.8/tests/unicode.*)
                            OUString aAlt( xHyphWord->getHyphenatedWord() );
                            OUString aAltLeft(aAlt.copy(0, _nWordLen));
                            OUString aAltRight(aAlt.copy(_nWordLen));
                            bAltFullLeft = aWord.startsWith(aAltLeft);
                            bAltFullRight = aWord.endsWith(aAltRight);
                            nAltDelChar = aWord.getLength() - aAlt.getLength() + static_cast<int>(!bAltFullLeft) + static_cast<int>(!bAltFullRight);

                            // NOTE: improved for other cases, see fdo#63711

                            // We expect[ed] the two cases:
                            // 1) packen becomes pak-ken
                            // 2) Schiffahrt becomes Schiff-fahrt
                            // In case 1, a character has to be replaced
                            // in case 2 a character is added.
                            // The identification is complicated by long
                            // compound words because the Hyphenator separates
                            // all position of the word. [This is not true for libhyphen.]
                            // "Schiffahrtsbrennesseln" -> "Schifffahrtsbrennnesseln"
                            // We can thus actually not directly connect the index of the
                            // AlternativeWord to aWord. The whole issue will be simplified
                            // by a function in the  Hyphenator as soon as AMA builds this in...
                            sal_Int32 nAltStart = _nWordLen - 1;
                            sal_Int32 nTxtStart = nAltStart - (aAlt.getLength() - aWord.getLength());
                            sal_Int32 nTxtEnd = nTxtStart;
                            sal_Int32 nAltEnd = nAltStart;

                            // The regions between the nStart and nEnd is the
                            // difference between alternative and original string.
                            while( nTxtEnd < aWord.getLength() && nAltEnd < aAlt.getLength() &&
                                   aWord[nTxtEnd] != aAlt[nAltEnd] )
                            {
                                ++nTxtEnd;
                                ++nAltEnd;
                            }

                            // If a character is added, then we notice it now:
                            if( nAltEnd > nTxtEnd && nAltStart == nAltEnd &&
                                aWord[ nTxtEnd ] == aAlt[nAltEnd] )
                            {
                                ++nAltEnd;
                                ++nTxtStart;
                                ++nTxtEnd;
                            }

                            DBG_ASSERT( ( nAltEnd - nAltStart ) == 1, "Alternate: Wrong assumption!" );

                            if ( nTxtEnd > nTxtStart )
                                cAlternateReplChar = aAlt[nAltStart];
                            else
                                cAlternateExtraChar = aAlt[nAltStart];

                            bHyphenated = true;
                            nBreakPos = nWordStart + nTxtStart;
                            if ( cAlternateReplChar || aAlt.getLength() < aWord.getLength() || !bAltFullRight) // also for "oma-tje", "re-eel"
                                nBreakPos++;
                        }
                    }
                }
            }
        }

        if ( nBreakPos <= pLine->GetStart() )
        {
            // No separator in line => Chop!
            nBreakPos = nMaxBreakPos;
            // I18N nextCharacters !
            if ( nBreakPos <= pLine->GetStart() )
                nBreakPos = pLine->GetStart() + 1;  // Otherwise infinite loop!
        }
    }

    // the dickey portion is the end portion
    pLine->SetEnd( nBreakPos );

    sal_Int32 nEndPortion = SplitTextPortion( pParaPortion, nBreakPos, pLine );

    if ( !bCompressBlank && !bHangingPunctuation )
    {
        // When justification is not SvxAdjust::Left, it's important to compress
        // the trailing space even if there is enough room for the space...
        // Don't check for SvxAdjust::Left, doesn't matter to compress in this case too...
        DBG_ASSERT( nBreakPos > pLine->GetStart(), "ImpBreakLines - BreakPos not expected!" );
        if ( pNode->GetChar( nBreakPos-1 ) == ' ' )
            bCompressBlank = true;
    }

    if ( bCompressBlank || bHangingPunctuation )
    {
        TextPortion& rTP = pParaPortion->GetTextPortions()[nEndPortion];
        DBG_ASSERT( rTP.GetKind() == PortionKind::TEXT, "BlankRubber: No TextPortion!" );
        DBG_ASSERT( nBreakPos > pLine->GetStart(), "SplitTextPortion at the beginning of the line?" );
        sal_Int32 nPosInArray = nBreakPos - 1 - pLine->GetStart();
        rTP.GetSize().setWidth( ( nPosInArray && ( rTP.GetLen() > 1 ) ) ? pLine->GetCharPosArray()[ nPosInArray-1 ] : 0 );
        pLine->GetCharPosArray()[ nPosInArray ] = rTP.GetSize().Width();
    }
    else if ( bHyphenated )
    {
        // A portion for inserting the separator...
        TextPortion* pHyphPortion = new TextPortion( 0 );
        pHyphPortion->SetKind( PortionKind::HYPHENATOR );
        OUString aHyphText(CH_HYPH);
        if ( (cAlternateReplChar || cAlternateExtraChar) && bAltFullRight ) // alternation after the break doesn't supported
        {
            TextPortion& rPrev = pParaPortion->GetTextPortions()[nEndPortion];
            DBG_ASSERT( rPrev.GetLen(), "Hyphenate: Prev portion?!" );
            rPrev.SetLen( rPrev.GetLen() - nAltDelChar );
            pHyphPortion->SetLen( nAltDelChar );
            if (cAlternateReplChar && !bAltFullLeft) pHyphPortion->SetExtraValue( cAlternateReplChar );
            // Correct width of the portion above:
            rPrev.GetSize().setWidth(
                pLine->GetCharPosArray()[ nBreakPos-1 - pLine->GetStart() - nAltDelChar ] );
        }

        // Determine the width of the Hyph-Portion:
        SvxFont aFont;
        SeekCursor( pParaPortion->GetNode(), nBreakPos, aFont );
        aFont.SetPhysFont( GetRefDevice() );
        pHyphPortion->GetSize().setHeight( GetRefDevice()->GetTextHeight() );
        pHyphPortion->GetSize().setWidth( GetRefDevice()->GetTextWidth( aHyphText ) );

        pParaPortion->GetTextPortions().Insert(++nEndPortion, pHyphPortion);
    }
    pLine->SetEndPortion( nEndPortion );
}

void ImpEditEngine::ImpAdjustBlocks( ParaPortion* pParaPortion, EditLine* pLine, long nRemainingSpace )
{
    DBG_ASSERT( nRemainingSpace > 0, "AdjustBlocks: Somewhat too little..." );
    DBG_ASSERT( pLine, "AdjustBlocks: Line ?!" );
    if ( ( nRemainingSpace < 0 ) || pLine->IsEmpty() )
        return ;

    const sal_Int32 nFirstChar = pLine->GetStart();
    const sal_Int32 nLastChar = pLine->GetEnd() -1;    // Last points behind
    ContentNode* pNode = pParaPortion->GetNode();

    DBG_ASSERT( nLastChar < pNode->Len(), "AdjustBlocks: Out of range!" );

    // Search blanks or Kashidas...
    std::vector<sal_Int32> aPositions;
    sal_uInt16 nLastScript = i18n::ScriptType::LATIN;
    for ( sal_Int32 nChar = nFirstChar; nChar <= nLastChar; nChar++ )
    {
        EditPaM aPaM( pNode, nChar+1 );
        LanguageType eLang = GetLanguage(aPaM);
        sal_uInt16 nScript = GetI18NScriptType(aPaM);
        if ( MsLangId::getPrimaryLanguage( eLang) == LANGUAGE_ARABIC_PRIMARY_ONLY )
            // Arabic script is handled later.
            continue;

        if ( pNode->GetChar(nChar) == ' ' )
        {
            // Normal latin script.
            aPositions.push_back( nChar );
        }
        else if (nChar > nFirstChar)
        {
            if (nLastScript == i18n::ScriptType::ASIAN)
            {
                // Set break position between this and the last character if
                // the last character is asian script.
                aPositions.push_back( nChar-1 );
            }
            else if (nScript == i18n::ScriptType::ASIAN)
            {
                // Set break position between a latin script and asian script.
                aPositions.push_back( nChar-1 );
            }
        }

        nLastScript = nScript;
    }

    // Kashidas ?
    ImpFindKashidas( pNode, nFirstChar, nLastChar, aPositions );

    if ( aPositions.empty() )
        return;

    // If the last character is a blank, it is rejected!
    // The width must be distributed to the blockers in front...
    // But not if it is the only one.
    if ( ( pNode->GetChar( nLastChar ) == ' ' ) && ( aPositions.size() > 1 ) &&
         ( MsLangId::getPrimaryLanguage( GetLanguage( EditPaM( pNode, nLastChar ) ) ) != LANGUAGE_ARABIC_PRIMARY_ONLY ) )
    {
        aPositions.pop_back();
        sal_Int32 nPortionStart, nPortion;
        nPortion = pParaPortion->GetTextPortions().FindPortion( nLastChar+1, nPortionStart );
        TextPortion& rLastPortion = pParaPortion->GetTextPortions()[ nPortion ];
        long nRealWidth = pLine->GetCharPosArray()[nLastChar-nFirstChar];
        long nBlankWidth = nRealWidth;
        if ( nLastChar > nPortionStart )
            nBlankWidth -= pLine->GetCharPosArray()[nLastChar-nFirstChar-1];
        // Possibly the blank has already been deducted in ImpBreakLine:
        if ( nRealWidth == rLastPortion.GetSize().Width() )
        {
            // For the last character the portion must stop behind the blank
            // => Simplify correction:
            DBG_ASSERT( ( nPortionStart + rLastPortion.GetLen() ) == ( nLastChar+1 ), "Blank actually not at the end of the portion!?");
            rLastPortion.GetSize().AdjustWidth( -nBlankWidth );
            nRemainingSpace += nBlankWidth;
        }
        pLine->GetCharPosArray()[nLastChar-nFirstChar] -= nBlankWidth;
    }

    size_t nGaps = aPositions.size();
    const long nMore4Everyone = nRemainingSpace / nGaps;
    long nSomeExtraSpace = nRemainingSpace - nMore4Everyone*nGaps;

    DBG_ASSERT( nSomeExtraSpace < static_cast<long>(nGaps), "AdjustBlocks: ExtraSpace too large" );
    DBG_ASSERT( nSomeExtraSpace >= 0, "AdjustBlocks: ExtraSpace < 0 " );

    // Correct the positions in the Array and the portion widths:
    // Last character won't be considered...
    for (auto const& nChar : aPositions)
    {
        if ( nChar < nLastChar )
        {
            sal_Int32 nPortionStart, nPortion;
            nPortion = pParaPortion->GetTextPortions().FindPortion( nChar, nPortionStart, true );
            TextPortion& rLastPortion = pParaPortion->GetTextPortions()[ nPortion ];

            // The width of the portion:
            rLastPortion.GetSize().AdjustWidth(nMore4Everyone );
            if ( nSomeExtraSpace )
                rLastPortion.GetSize().AdjustWidth( 1 );

            // Correct positions in array
            // Even for kashidas just change positions, VCL will then draw the kashida automatically
            sal_Int32 nPortionEnd = nPortionStart + rLastPortion.GetLen();
            for ( sal_Int32 _n = nChar; _n < nPortionEnd; _n++ )
            {
                pLine->GetCharPosArray()[_n-nFirstChar] += nMore4Everyone;
                if ( nSomeExtraSpace )
                    pLine->GetCharPosArray()[_n-nFirstChar]++;
            }

            if ( nSomeExtraSpace )
                nSomeExtraSpace--;
        }
    }

    // Now the text width contains the extra width...
    pLine->SetTextWidth( pLine->GetTextWidth() + nRemainingSpace );
}

void ImpEditEngine::ImpFindKashidas( ContentNode* pNode, sal_Int32 nStart, sal_Int32 nEnd, std::vector<sal_Int32>& rArray )
{
    // the search has to be performed on a per word base

    EditSelection aWordSel( EditPaM( pNode, nStart ) );
    aWordSel = SelectWord( aWordSel, css::i18n::WordType::DICTIONARY_WORD );
    if ( aWordSel.Min().GetIndex() < nStart )
       aWordSel.Min().SetIndex( nStart );

    while ( ( aWordSel.Min().GetNode() == pNode ) && ( aWordSel.Min().GetIndex() < nEnd ) )
    {
        const sal_Int32 nSavPos = aWordSel.Max().GetIndex();
        if ( aWordSel.Max().GetIndex() > nEnd )
           aWordSel.Max().SetIndex( nEnd );

        OUString aWord = GetSelected( aWordSel );

        // restore selection for proper iteration at the end of the function
        aWordSel.Max().SetIndex( nSavPos );

        sal_Int32 nIdx = 0;
        sal_Int32 nKashidaPos = -1;
        sal_Unicode cCh;
        sal_Unicode cPrevCh = 0;

        while ( nIdx < aWord.getLength() )
        {
            cCh = aWord[ nIdx ];

            // 1. Priority:
            // after user inserted kashida
            if ( 0x640 == cCh )
            {
                nKashidaPos = aWordSel.Min().GetIndex() + nIdx;
                break;
            }

            // 2. Priority:
            // after a Seen or Sad
            if ( nIdx + 1 < aWord.getLength() &&
                 ( 0x633 == cCh || 0x635 == cCh ) )
            {
                nKashidaPos = aWordSel.Min().GetIndex() + nIdx;
                break;
            }

            // 3. Priority:
            // before final form of the Marbuta, Hah, Dal
            // 4. Priority:
            // before final form of Alef, Lam or Kaf
            if ( nIdx && nIdx + 1 == aWord.getLength() &&
                 ( 0x629 == cCh || 0x62D == cCh || 0x62F == cCh ||
                   0x627 == cCh || 0x644 == cCh || 0x643 == cCh ) )
            {
                DBG_ASSERT( 0 != cPrevCh, "No previous character" );

                // check if character is connectable to previous character,
                if ( lcl_ConnectToPrev( cCh, cPrevCh ) )
                {
                    nKashidaPos = aWordSel.Min().GetIndex() + nIdx - 1;
                    break;
                }
            }

            // 5. Priority:
            // before media Bah
            if ( nIdx && nIdx + 1 < aWord.getLength() && 0x628 == cCh )
            {
                DBG_ASSERT( 0 != cPrevCh, "No previous character" );

                // check if next character is Reh, Yeh or Alef Maksura
                sal_Unicode cNextCh = aWord[ nIdx + 1 ];

                if ( 0x631 == cNextCh || 0x64A == cNextCh ||
                     0x649 == cNextCh )
                {
                    // check if character is connectable to previous character,
                    if ( lcl_ConnectToPrev( cCh, cPrevCh ) )
                        nKashidaPos = aWordSel.Min().GetIndex() + nIdx - 1;
                }
            }

            // 6. Priority:
            // other connecting possibilities
            if ( nIdx && nIdx + 1 == aWord.getLength() &&
                 0x60C <= cCh && 0x6FE >= cCh )
            {
                DBG_ASSERT( 0 != cPrevCh, "No previous character" );

                // check if character is connectable to previous character,
                if ( lcl_ConnectToPrev( cCh, cPrevCh ) )
                {
                    // only choose this position if we did not find
                    // a better one:
                    if ( nKashidaPos<0 )
                        nKashidaPos = aWordSel.Min().GetIndex() + nIdx - 1;
                    break;
                }
            }

            // Do not consider Fathatan, Dammatan, Kasratan, Fatha,
            // Damma, Kasra, Shadda and Sukun when checking if
            // a character can be connected to previous character.
            if ( cCh < 0x64B || cCh > 0x652 )
                cPrevCh = cCh;

            ++nIdx;
        } // end of current word

        if ( nKashidaPos>=0 )
            rArray.push_back( nKashidaPos );

        aWordSel = WordRight( aWordSel.Max(), css::i18n::WordType::DICTIONARY_WORD );
        aWordSel = SelectWord( aWordSel, css::i18n::WordType::DICTIONARY_WORD );
    }
}

sal_Int32 ImpEditEngine::SplitTextPortion( ParaPortion* pPortion, sal_Int32 nPos, EditLine* pCurLine )
{
    DBG_ASSERT( pPortion, "SplitTextPortion: Which ?" );

    // The portion at nPos is split, if there is not a transition at nPos anyway
    if ( nPos == 0 )
        return 0;

    sal_Int32 nSplitPortion;
    sal_Int32 nTmpPos = 0;
    TextPortion* pTextPortion = nullptr;
    sal_Int32 nPortions = pPortion->GetTextPortions().Count();
    for ( nSplitPortion = 0; nSplitPortion < nPortions; nSplitPortion++ )
    {
        TextPortion& rTP = pPortion->GetTextPortions()[nSplitPortion];
        nTmpPos = nTmpPos + rTP.GetLen();
        if ( nTmpPos >= nPos )
        {
            if ( nTmpPos == nPos )  // then nothing needs to be split
            {
                return nSplitPortion;
            }
            pTextPortion = &rTP;
            break;
        }
    }

    DBG_ASSERT( pTextPortion, "Position outside the area!" );

    if (!pTextPortion)
        return 0;

    DBG_ASSERT( pTextPortion->GetKind() == PortionKind::TEXT, "SplitTextPortion: No TextPortion!" );

    sal_Int32 nOverlapp = nTmpPos - nPos;
    pTextPortion->SetLen( pTextPortion->GetLen() - nOverlapp );
    TextPortion* pNewPortion = new TextPortion( nOverlapp );
    pPortion->GetTextPortions().Insert(nSplitPortion+1, pNewPortion);
    // Set sizes
    if ( pCurLine )
    {
        // No new GetTextSize, instead use values from the Array:
        DBG_ASSERT( nPos > pCurLine->GetStart(), "SplitTextPortion at the beginning of the line?" );
        pTextPortion->GetSize().setWidth( pCurLine->GetCharPosArray()[ nPos-pCurLine->GetStart()-1 ] );

        if ( pTextPortion->GetExtraInfos() && pTextPortion->GetExtraInfos()->bCompressed )
        {
            // We need the original size from the portion
            sal_Int32 nTxtPortionStart = pPortion->GetTextPortions().GetStartPos( nSplitPortion );
            SvxFont aTmpFont( pPortion->GetNode()->GetCharAttribs().GetDefFont() );
            SeekCursor( pPortion->GetNode(), nTxtPortionStart+1, aTmpFont );
            aTmpFont.SetPhysFont( GetRefDevice() );
            GetRefDevice()->Push( PushFlags::TEXTLANGUAGE );
            ImplInitDigitMode(GetRefDevice(), aTmpFont.GetLanguage());
            Size aSz = aTmpFont.QuickGetTextSize( GetRefDevice(), pPortion->GetNode()->GetString(), nTxtPortionStart, pTextPortion->GetLen() );
            GetRefDevice()->Pop();
            pTextPortion->GetExtraInfos()->nOrgWidth = aSz.Width();
        }
    }
    else
        pTextPortion->GetSize().setWidth( -1 );

    return nSplitPortion;
}

void ImpEditEngine::CreateTextPortions( ParaPortion* pParaPortion, sal_Int32& rStart )
{
    sal_Int32 nStartPos = rStart;
    ContentNode* pNode = pParaPortion->GetNode();
    DBG_ASSERT( pNode->Len(), "CreateTextPortions should not be used for empty paragraphs!" );

    std::set< sal_Int32 > aPositions;
    aPositions.insert( 0 );

    sal_uInt16 nAttr = 0;
    EditCharAttrib* pAttrib = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
    while ( pAttrib )
    {
        // Insert Start and End into the Array...
        // The Insert method does not allow for duplicate values...
        aPositions.insert( pAttrib->GetStart() );
        aPositions.insert( pAttrib->GetEnd() );
        nAttr++;
        pAttrib = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
    }
    aPositions.insert( pNode->Len() );

    if ( pParaPortion->aScriptInfos.empty() )
        InitScriptTypes( GetParaPortions().GetPos( pParaPortion ) );

    const ScriptTypePosInfos& rTypes = pParaPortion->aScriptInfos;
    for (const ScriptTypePosInfo& rType : rTypes)
        aPositions.insert( rType.nStartPos );

    const WritingDirectionInfos& rWritingDirections = pParaPortion->aWritingDirectionInfos;
    for (const WritingDirectionInfo & rWritingDirection : rWritingDirections)
        aPositions.insert( rWritingDirection.nStartPos );

    if ( mpIMEInfos && mpIMEInfos->nLen && mpIMEInfos->pAttribs && ( mpIMEInfos->aPos.GetNode() == pNode ) )
    {
        ExtTextInputAttr nLastAttr = ExtTextInputAttr(0xFFFF);
        for( sal_Int32 n = 0; n < mpIMEInfos->nLen; n++ )
        {
            if ( mpIMEInfos->pAttribs[n] != nLastAttr )
            {
                aPositions.insert( mpIMEInfos->aPos.GetIndex() + n );
                nLastAttr = mpIMEInfos->pAttribs[n];
            }
        }
        aPositions.insert( mpIMEInfos->aPos.GetIndex() + mpIMEInfos->nLen );
    }

    // From ... Delete:
    // Unfortunately, the number of text portions does not have to match
    // aPositions.Count(), since there might be line breaks...
    sal_Int32 nPortionStart = 0;
    sal_Int32 nInvPortion = 0;
    sal_Int32 nP;
    for ( nP = 0; nP < pParaPortion->GetTextPortions().Count(); nP++ )
    {
        const TextPortion& rTmpPortion = pParaPortion->GetTextPortions()[nP];
        nPortionStart = nPortionStart + rTmpPortion.GetLen();
        if ( nPortionStart >= nStartPos )
        {
            nPortionStart = nPortionStart - rTmpPortion.GetLen();
            rStart = nPortionStart;
            nInvPortion = nP;
            break;
        }
    }
    DBG_ASSERT( nP < pParaPortion->GetTextPortions().Count() || !pParaPortion->GetTextPortions().Count(), "Nothing to delete: CreateTextPortions" );
    if ( nInvPortion && ( nPortionStart+pParaPortion->GetTextPortions()[nInvPortion].GetLen() > nStartPos ) )
    {
        // prefer one in front...
        // But only if it was in the middle of the portion of, otherwise it
        // might be the only one in the row in front!
        nInvPortion--;
        nPortionStart = nPortionStart - pParaPortion->GetTextPortions()[nInvPortion].GetLen();
    }
    pParaPortion->GetTextPortions().DeleteFromPortion( nInvPortion );

    // A portion may also have been formed by a line break:
    aPositions.insert( nPortionStart );

    std::set< sal_Int32 >::iterator nInvPos = aPositions.find(  nPortionStart );
    DBG_ASSERT( (nInvPos != aPositions.end()), "InvPos ?!" );

    std::set< sal_Int32 >::iterator i = nInvPos;
    ++i;
    while ( i != aPositions.end() )
    {
        TextPortion* pNew = new TextPortion( (*i++) - *nInvPos++ );
        pParaPortion->GetTextPortions().Append(pNew);
    }

    DBG_ASSERT( pParaPortion->GetTextPortions().Count(), "No Portions?!" );
#if OSL_DEBUG_LEVEL > 0
    OSL_ENSURE( ParaPortion::DbgCheckTextPortions(*pParaPortion), "Portion is broken?" );
#endif
}

void ImpEditEngine::RecalcTextPortion( ParaPortion* pParaPortion, sal_Int32 nStartPos, sal_Int32 nNewChars )
{
    DBG_ASSERT( pParaPortion->GetTextPortions().Count(), "No Portions!" );
    DBG_ASSERT( nNewChars, "RecalcTextPortion with Diff == 0" );

    ContentNode* const pNode = pParaPortion->GetNode();
    if ( nNewChars > 0 )
    {
        // If an Attribute begins/ends at nStartPos, then a new portion starts
        // otherwise the portion is extended at nStartPos.
        if ( pNode->GetCharAttribs().HasBoundingAttrib( nStartPos ) || IsScriptChange( EditPaM( pNode, nStartPos ) ) )
        {
            sal_Int32 nNewPortionPos = 0;
            if ( nStartPos )
                nNewPortionPos = SplitTextPortion( pParaPortion, nStartPos ) + 1;

            // A blank portion may be here, if the paragraph was empty,
            // or if a line was created by a hard line break.
            if ( ( nNewPortionPos < pParaPortion->GetTextPortions().Count() ) &&
                    !pParaPortion->GetTextPortions()[nNewPortionPos].GetLen() )
            {
                TextPortion& rTP = pParaPortion->GetTextPortions()[nNewPortionPos];
                DBG_ASSERT( rTP.GetKind() == PortionKind::TEXT, "the empty portion was no TextPortion!" );
                rTP.SetLen( rTP.GetLen() + nNewChars );
            }
            else
            {
                TextPortion* pNewPortion = new TextPortion( nNewChars );
                pParaPortion->GetTextPortions().Insert(nNewPortionPos, pNewPortion);
            }
        }
        else
        {
            sal_Int32 nPortionStart;
            const sal_Int32 nTP = pParaPortion->GetTextPortions().
                FindPortion( nStartPos, nPortionStart );
            TextPortion& rTP = pParaPortion->GetTextPortions()[ nTP ];
            rTP.SetLen( rTP.GetLen() + nNewChars );
            rTP.GetSize().setWidth( -1 );
        }
    }
    else
    {
        // Shrink or remove portion if necessary.
        // Before calling this method it must be ensured that no portions were
        // in the deleted area!

        // There must be no portions extending into the area or portions starting in
        // the area, so it must be:
        //    nStartPos <= nPos <= nStartPos - nNewChars(neg.)
        sal_Int32 nPortion = 0;
        sal_Int32 nPos = 0;
        sal_Int32 nEnd = nStartPos-nNewChars;
        sal_Int32 nPortions = pParaPortion->GetTextPortions().Count();
        TextPortion* pTP = nullptr;
        for ( nPortion = 0; nPortion < nPortions; nPortion++ )
        {
            pTP = &pParaPortion->GetTextPortions()[ nPortion ];
            if ( ( nPos+pTP->GetLen() ) > nStartPos )
            {
                DBG_ASSERT( nPos <= nStartPos, "Wrong Start!" );
                DBG_ASSERT( nPos+pTP->GetLen() >= nEnd, "Wrong End!" );
                break;
            }
            nPos = nPos + pTP->GetLen();
        }
        DBG_ASSERT( pTP, "RecalcTextPortion: Portion not found" );
        if ( ( nPos == nStartPos ) && ( (nPos+pTP->GetLen()) == nEnd ) )
        {
            // Remove portion;
            PortionKind nType = pTP->GetKind();
            pParaPortion->GetTextPortions().Remove( nPortion );
            if ( nType == PortionKind::LINEBREAK )
            {
                TextPortion& rNext = pParaPortion->GetTextPortions()[ nPortion ];
                if ( !rNext.GetLen() )
                {
                    // Remove dummy portion
                    pParaPortion->GetTextPortions().Remove( nPortion );
                }
            }
        }
        else
        {
            DBG_ASSERT( pTP->GetLen() > (-nNewChars), "Portion too small to shrink! ");
            pTP->SetLen( pTP->GetLen() + nNewChars );
        }

        sal_Int32 nPortionCount = pParaPortion->GetTextPortions().Count();
        assert( nPortionCount );
        if (nPortionCount)
        {
            // No HYPHENATOR portion is allowed to get stuck right at the end...
            sal_Int32 nLastPortion = nPortionCount - 1;
            pTP = &pParaPortion->GetTextPortions()[nLastPortion];
            if ( pTP->GetKind() == PortionKind::HYPHENATOR )
            {
                // Discard portion; if possible, correct the ones before,
                // if the Hyphenator portion has swallowed one character...
                if ( nLastPortion && pTP->GetLen() )
                {
                    TextPortion& rPrev = pParaPortion->GetTextPortions()[nLastPortion - 1];
                    DBG_ASSERT( rPrev.GetKind() == PortionKind::TEXT, "Portion?!" );
                    rPrev.SetLen( rPrev.GetLen() + pTP->GetLen() );
                    rPrev.GetSize().setWidth( -1 );
                }
                pParaPortion->GetTextPortions().Remove( nLastPortion );
            }
        }
    }
#if OSL_DEBUG_LEVEL > 0
    OSL_ENSURE( ParaPortion::DbgCheckTextPortions(*pParaPortion), "Portions are broken?" );
#endif
}

void ImpEditEngine::SetTextRanger( std::unique_ptr<TextRanger> pRanger )
{
    pTextRanger = std::move(pRanger);

    for ( sal_Int32 nPara = 0; nPara < GetParaPortions().Count(); nPara++ )
    {
        ParaPortion* pParaPortion = GetParaPortions()[nPara];
        pParaPortion->MarkSelectionInvalid( 0 );
        pParaPortion->GetLines().Reset();
    }

    FormatFullDoc();
    UpdateViews( GetActiveView() );
    if ( GetUpdateMode() && GetActiveView() )
        pActiveView->ShowCursor(false, false);
}

void ImpEditEngine::SetVertical( bool bVertical, bool bTopToBottom)
{
    if ( IsVertical() != bVertical || IsTopToBottom() != (bVertical && bTopToBottom))
    {
        GetEditDoc().SetVertical( bVertical, bTopToBottom);
        bool bUseCharAttribs = bool(aStatus.GetControlWord() & EEControlBits::USECHARATTRIBS);
        GetEditDoc().CreateDefFont( bUseCharAttribs );
        if ( IsFormatted() )
        {
            FormatFullDoc();
            UpdateViews( GetActiveView() );
        }
    }
}

void ImpEditEngine::SetFixedCellHeight( bool bUseFixedCellHeight )
{
    if ( IsFixedCellHeight() != bUseFixedCellHeight )
    {
        GetEditDoc().SetFixedCellHeight( bUseFixedCellHeight );
        if ( IsFormatted() )
        {
            FormatFullDoc();
            UpdateViews( GetActiveView() );
        }
    }
}

void ImpEditEngine::SeekCursor( ContentNode* pNode, sal_Int32 nPos, SvxFont& rFont, OutputDevice* pOut )
{
    // It was planned, SeekCursor( nStartPos, nEndPos,... ), so that it would
    // only be searched anew at the StartPosition.
    // Problem: There would be two lists to consider/handle:
    // OrderedByStart,OrderedByEnd.

    if ( nPos > pNode->Len() )
        nPos = pNode->Len();

    rFont = pNode->GetCharAttribs().GetDefFont();

    /*
     * Set attributes for script types Asian and Complex
    */
    short nScriptTypeI18N = GetI18NScriptType( EditPaM( pNode, nPos ) );
    SvtScriptType nScriptType = SvtLanguageOptions::FromI18NToSvtScriptType(nScriptTypeI18N);
    if ( ( nScriptTypeI18N == i18n::ScriptType::ASIAN ) || ( nScriptTypeI18N == i18n::ScriptType::COMPLEX ) )
    {
        const SvxFontItem& rFontItem = static_cast<const SvxFontItem&>(pNode->GetContentAttribs().GetItem( GetScriptItemId( EE_CHAR_FONTINFO, nScriptType ) ));
        rFont.SetFamilyName( rFontItem.GetFamilyName() );
        rFont.SetFamily( rFontItem.GetFamily() );
        rFont.SetPitch( rFontItem.GetPitch() );
        rFont.SetCharSet( rFontItem.GetCharSet() );
        Size aSz( rFont.GetFontSize() );
        aSz.setHeight( static_cast<const SvxFontHeightItem&>(pNode->GetContentAttribs().GetItem( GetScriptItemId( EE_CHAR_FONTHEIGHT, nScriptType ) ) ).GetHeight() );
        rFont.SetFontSize( aSz );
        rFont.SetWeight( static_cast<const SvxWeightItem&>(pNode->GetContentAttribs().GetItem( GetScriptItemId( EE_CHAR_WEIGHT, nScriptType ))).GetWeight() );
        rFont.SetItalic( static_cast<const SvxPostureItem&>(pNode->GetContentAttribs().GetItem( GetScriptItemId( EE_CHAR_ITALIC, nScriptType ))).GetPosture() );
        rFont.SetLanguage( static_cast<const SvxLanguageItem&>(pNode->GetContentAttribs().GetItem( GetScriptItemId( EE_CHAR_LANGUAGE, nScriptType ))).GetLanguage() );
    }

    sal_uInt16 nRelWidth = pNode->GetContentAttribs().GetItem( EE_CHAR_FONTWIDTH).GetValue();

    /*
     * Set output device's line and overline colors
    */
    if ( pOut )
    {
        const SvxUnderlineItem& rTextLineColor = pNode->GetContentAttribs().GetItem( EE_CHAR_UNDERLINE );
        if ( rTextLineColor.GetColor() != COL_TRANSPARENT )
            pOut->SetTextLineColor( rTextLineColor.GetColor() );
        else
            pOut->SetTextLineColor();

        const SvxOverlineItem& rOverlineColor = pNode->GetContentAttribs().GetItem( EE_CHAR_OVERLINE );
        if ( rOverlineColor.GetColor() != COL_TRANSPARENT )
            pOut->SetOverlineColor( rOverlineColor.GetColor() );
        else
            pOut->SetOverlineColor();
    }

    const SvxLanguageItem* pCJKLanguageItem = nullptr;

    /*
     * Scan through char attributes of pNode
    */
    if ( aStatus.UseCharAttribs() )
    {
        CharAttribList::AttribsType& rAttribs = pNode->GetCharAttribs().GetAttribs();
        size_t nAttr = 0;
        EditCharAttrib* pAttrib = GetAttrib(rAttribs, nAttr);
        while ( pAttrib && ( pAttrib->GetStart() <= nPos ) )
        {
            // when seeking, ignore attributes which start there! Empty attributes
            // are considered (used) as these are just set. But do not use empty
            // attributes: When just set and empty => no effect on font
            // In a blank paragraph, set characters take effect immediately.
            if ( ( pAttrib->Which() != 0 ) &&
                 ( ( ( pAttrib->GetStart() < nPos ) && ( pAttrib->GetEnd() >= nPos ) )
                     || ( !pNode->Len() ) ) )
            {
                DBG_ASSERT( ( pAttrib->Which() >= EE_CHAR_START ) && ( pAttrib->Which() <= EE_FEATURE_END ), "Invalid Attribute in Seek() " );
                if ( IsScriptItemValid( pAttrib->Which(), nScriptTypeI18N ) )
                {
                    pAttrib->SetFont( rFont, pOut );
                    // #i1550# hard color attrib should win over text color from field
                    if ( pAttrib->Which() == EE_FEATURE_FIELD )
                    {
                        EditCharAttrib* pColorAttr = pNode->GetCharAttribs().FindAttrib( EE_CHAR_COLOR, nPos );
                        if ( pColorAttr )
                            pColorAttr->SetFont( rFont, pOut );
                    }
                }
                if ( pAttrib->Which() == EE_CHAR_FONTWIDTH )
                    nRelWidth = static_cast<const SvxCharScaleWidthItem*>(pAttrib->GetItem())->GetValue();
                if ( pAttrib->Which() == EE_CHAR_LANGUAGE_CJK )
                    pCJKLanguageItem = static_cast<const SvxLanguageItem*>( pAttrib->GetItem() );
            }
            pAttrib = GetAttrib( rAttribs, ++nAttr );
        }
    }

    if ( !pCJKLanguageItem )
        pCJKLanguageItem = &pNode->GetContentAttribs().GetItem( EE_CHAR_LANGUAGE_CJK );

    rFont.SetCJKContextLanguage( pCJKLanguageItem->GetLanguage() );

    if ( (rFont.GetKerning() != FontKerning::NONE) && IsKernAsianPunctuation() && ( nScriptTypeI18N == i18n::ScriptType::ASIAN ) )
        rFont.SetKerning( rFont.GetKerning() | FontKerning::Asian );

    if ( aStatus.DoNotUseColors() )
    {
        rFont.SetColor( /* rColorItem.GetValue() */ COL_BLACK );
    }

    if ( aStatus.DoStretch() || ( nRelWidth != 100 ) )
    {
        // For the current Output device, because otherwise if RefDev=Printer its looks
        // ugly on the screen!
        OutputDevice* pDev = pOut ? pOut : GetRefDevice();
        rFont.SetPhysFont( pDev );
        FontMetric aMetric( pDev->GetFontMetric() );

        // Set the font as we want it to look like & reset the Propr attribute
        // so that it is not counted twice.
        Size aRealSz( aMetric.GetFontSize() );
        rFont.SetPropr( 100 );

        if ( aStatus.DoStretch() )
        {
            if ( nStretchY != 100 )
            {
                aRealSz.setHeight( aRealSz.Height() * nStretchY );
                aRealSz.setHeight( aRealSz.Height() / 100 );
            }
            if ( nStretchX != 100 )
            {
                if ( nStretchX == nStretchY &&
                     nRelWidth == 100 )
                {
                    aRealSz.setWidth( 0 );
                }
                else
                {
                    aRealSz.setWidth( aRealSz.Width() * nStretchX );
                    aRealSz.setWidth( aRealSz.Width() / 100 );

                    // Also the Kerning: (long due to handle Interim results)
                    long nKerning = rFont.GetFixKerning();
/*
  The consideration was: If negative kerning, but StretchX = 200
  => Do not double the kerning, thus pull the letters closer together
  ---------------------------
  Kern  StretchX    =>Kern
  ---------------------------
  >0        <100        < (Proportional)
  <0        <100        < (Proportional)
  >0        >100        > (Proportional)
  <0        >100        < (The amount, thus disproportional)
*/
                    if ( ( nKerning < 0  ) && ( nStretchX > 100 ) )
                    {
                        // disproportional
                        nKerning *= 100;
                        nKerning /= nStretchX;
                    }
                    else if ( nKerning )
                    {
                        // Proportional
                        nKerning *= nStretchX;
                        nKerning /= 100;
                    }
                    rFont.SetFixKerning( static_cast<short>(nKerning) );
                }
            }
        }
        if ( nRelWidth != 100 )
        {
            aRealSz.setWidth( aRealSz.Width() * nRelWidth );
            aRealSz.setWidth( aRealSz.Width() / 100 );
        }
        rFont.SetFontSize( aRealSz );
        // Font is not restored...
    }

    if ( ( ( rFont.GetColor() == COL_AUTO ) || ( IsForceAutoColor() ) ) && pOut )
    {
        // #i75566# Do not use AutoColor when printing OR Pdf export
        const bool bPrinting(OUTDEV_PRINTER == pOut->GetOutDevType());
        const bool bPDFExporting(OUTDEV_PDF == pOut->GetOutDevType());

        if ( IsAutoColorEnabled() && !bPrinting && !bPDFExporting)
        {
            // Never use WindowTextColor on the printer
            rFont.SetColor( GetAutoColor() );
        }
        else
        {
            if ( ( GetBackgroundColor() != COL_AUTO ) && GetBackgroundColor().IsDark() )
                rFont.SetColor( COL_WHITE );
            else
                rFont.SetColor( COL_BLACK );
        }
    }

    if ( mpIMEInfos && mpIMEInfos->pAttribs && ( mpIMEInfos->aPos.GetNode() == pNode ) &&
        ( nPos > mpIMEInfos->aPos.GetIndex() ) && ( nPos <= ( mpIMEInfos->aPos.GetIndex() + mpIMEInfos->nLen ) ) )
    {
        ExtTextInputAttr nAttr = mpIMEInfos->pAttribs[ nPos - mpIMEInfos->aPos.GetIndex() - 1 ];
        if ( nAttr & ExtTextInputAttr::Underline )
            rFont.SetUnderline( LINESTYLE_SINGLE );
        else if ( nAttr & ExtTextInputAttr::BoldUnderline )
            rFont.SetUnderline( LINESTYLE_BOLD );
        else if ( nAttr & ExtTextInputAttr::DottedUnderline )
            rFont.SetUnderline( LINESTYLE_DOTTED );
        else if ( nAttr & ExtTextInputAttr::DashDotUnderline )
            rFont.SetUnderline( LINESTYLE_DOTTED );
        else if ( nAttr & ExtTextInputAttr::RedText )
            rFont.SetColor( COL_RED );
        else if ( nAttr & ExtTextInputAttr::HalfToneText )
            rFont.SetColor( COL_LIGHTGRAY );
        if ( nAttr & ExtTextInputAttr::Highlight )
        {
            const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
            rFont.SetColor( rStyleSettings.GetHighlightTextColor() );
            rFont.SetFillColor( rStyleSettings.GetHighlightColor() );
            rFont.SetTransparent( false );
        }
        else if ( nAttr & ExtTextInputAttr::GrayWaveline )
        {
            rFont.SetUnderline( LINESTYLE_WAVE );
            if( pOut )
                pOut->SetTextLineColor( COL_LIGHTGRAY );
        }
    }
}

void ImpEditEngine::RecalcFormatterFontMetrics( FormatterFontMetric& rCurMetrics, SvxFont& rFont )
{
    // for line height at high / low first without Propr!
    sal_uInt16 nPropr = rFont.GetPropr();
    DBG_ASSERT( ( nPropr == 100 ) || rFont.GetEscapement(), "Propr without Escape?!" );
    if ( nPropr != 100 )
    {
        rFont.SetPropr( 100 );
        rFont.SetPhysFont( pRefDev );
    }
    sal_uInt16 nAscent, nDescent;

    FontMetric aMetric( pRefDev->GetFontMetric() );
    nAscent = static_cast<sal_uInt16>(aMetric.GetAscent());
    if ( IsAddExtLeading() )
        nAscent = sal::static_int_cast< sal_uInt16 >(
            nAscent + aMetric.GetExternalLeading() );
    nDescent = static_cast<sal_uInt16>(aMetric.GetDescent());

    if ( IsFixedCellHeight() )
    {
        nAscent = sal::static_int_cast< sal_uInt16 >( rFont.GetFontHeight() );
        nDescent= sal::static_int_cast< sal_uInt16 >( ImplCalculateFontIndependentLineSpacing( rFont.GetFontHeight() ) - nAscent );
    }
    else
    {
        sal_uInt16 nIntLeading = ( aMetric.GetInternalLeading() > 0 ) ? static_cast<sal_uInt16>(aMetric.GetInternalLeading()) : 0;
        // Fonts without leading cause problems
        if ( ( nIntLeading == 0 ) && ( pRefDev->GetOutDevType() == OUTDEV_PRINTER ) )
        {
            // Lets see what Leading one gets on the screen
            VclPtr<VirtualDevice> pVDev = GetVirtualDevice( pRefDev->GetMapMode(), pRefDev->GetDrawMode() );
            rFont.SetPhysFont( pVDev );
            aMetric = pVDev->GetFontMetric();

            // This is so that the Leading does not count itself out again,
            // if the whole line has the font, nTmpLeading.
            nAscent = static_cast<sal_uInt16>(aMetric.GetAscent());
            nDescent = static_cast<sal_uInt16>(aMetric.GetDescent());
        }
    }
    if ( nAscent > rCurMetrics.nMaxAscent )
        rCurMetrics.nMaxAscent = nAscent;
    if ( nDescent > rCurMetrics.nMaxDescent )
        rCurMetrics.nMaxDescent= nDescent;
    // Special treatment of high/low:
    if ( rFont.GetEscapement() )
    {
        // Now in consideration of Escape/Propr
        // possibly enlarge Ascent or Descent
        short nDiff = static_cast<short>(rFont.GetFontSize().Height()*rFont.GetEscapement()/100L);
        if ( rFont.GetEscapement() > 0 )
        {
            nAscent = static_cast<sal_uInt16>(static_cast<long>(nAscent)*nPropr/100 + nDiff);
            if ( nAscent > rCurMetrics.nMaxAscent )
                rCurMetrics.nMaxAscent = nAscent;
        }
        else    // has to be < 0
        {
            nDescent = static_cast<sal_uInt16>(static_cast<long>(nDescent)*nPropr/100 - nDiff);
            if ( nDescent > rCurMetrics.nMaxDescent )
                rCurMetrics.nMaxDescent= nDescent;
        }
    }
}

void ImpEditEngine::Paint( OutputDevice* pOutDev, tools::Rectangle aClipRect, Point aStartPos, bool bStripOnly, short nOrientation )
{
    if ( !GetUpdateMode() && !bStripOnly )
        return;

    if ( !IsFormatted() )
        FormatDoc();

    long nFirstVisXPos = - pOutDev->GetMapMode().GetOrigin().X();
    long nFirstVisYPos = - pOutDev->GetMapMode().GetOrigin().Y();

    const EditLine* pLine = nullptr;
    Point aTmpPos;
    Point aRedLineTmpPos;
    DBG_ASSERT( GetParaPortions().Count(), "No ParaPortion?!" );
    SvxFont aTmpFont( GetParaPortions()[0]->GetNode()->GetCharAttribs().GetDefFont() );
    vcl::Font aOldFont( pOutDev->GetFont() );
    vcl::PDFExtOutDevData* pPDFExtOutDevData = dynamic_cast< vcl::PDFExtOutDevData* >( pOutDev->GetExtOutDevData() );

    // In the case of rotated text is aStartPos considered TopLeft because
    // other information is missing, and since the whole object is shown anyway
    // un-scrolled.
    // The rectangle is infinite.
    Point aOrigin( aStartPos );
    double nCos = 0.0, nSin = 0.0;
    if ( nOrientation )
    {
        double nRealOrientation = nOrientation*F_PI1800;
        nCos = cos( nRealOrientation );
        nSin = sin( nRealOrientation );
    }

    // #110496# Added some more optional metafile comments. This
    // change: factored out some duplicated code.
    GDIMetaFile* pMtf = pOutDev->GetConnectMetaFile();
    const bool bMetafileValid( pMtf != nullptr );

    long nVertLineSpacing = CalcVertLineSpacing(aStartPos);


    // Over all the paragraphs...

    for ( sal_Int32 n = 0; n < GetParaPortions().Count(); n++ )
    {
        const ParaPortion* pPortion = GetParaPortions()[n];
        DBG_ASSERT( pPortion, "NULL-Pointer in TokenList in Paint" );
        // if when typing idle formatting,  asynchronous Paint.
        // Invisible Portions may be invalid.
        if ( pPortion->IsVisible() && pPortion->IsInvalid() )
            return;

        if ( pPDFExtOutDevData )
            pPDFExtOutDevData->BeginStructureElement( vcl::PDFWriter::Paragraph );

        long nParaHeight = pPortion->GetHeight();
        sal_Int32 nIndex = 0;
        if ( pPortion->IsVisible() && (
                ( !IsVertical() && ( ( aStartPos.Y() + nParaHeight ) > aClipRect.Top() ) ) ||
                ( IsVertical() && IsTopToBottom() && ( ( aStartPos.X() - nParaHeight ) < aClipRect.Right() ) ) ||
                ( IsVertical() && !IsTopToBottom() && ( ( aStartPos.X() + nParaHeight ) > aClipRect.Left() ) ) ) )

        {

            // Over the lines of the paragraph...

            sal_Int32 nLines = pPortion->GetLines().Count();
            sal_Int32 nLastLine = nLines-1;

            bool bEndOfParagraphWritten(false);

            if ( !IsVertical() )
                aStartPos.AdjustY(pPortion->GetFirstLineOffset() );
            else
            {
                if( IsTopToBottom() )
                    aStartPos.AdjustX( -(pPortion->GetFirstLineOffset()) );
                else
                    aStartPos.AdjustX(pPortion->GetFirstLineOffset() );
            }

            Point aParaStart( aStartPos );

            const SvxLineSpacingItem& rLSItem = pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_SBL );
            sal_uInt16 nSBL = ( rLSItem.GetInterLineSpaceRule() == SvxInterLineSpaceRule::Fix )
                                ? GetYValue( rLSItem.GetInterLineSpace() ) : 0;
            bool bPaintBullet (false);

            for ( sal_Int32 nLine = 0; nLine < nLines; nLine++ )
            {
                pLine = &pPortion->GetLines()[nLine];
                nIndex = pLine->GetStart();
                DBG_ASSERT( pLine, "NULL-Pointer in the line iterator in UpdateViews" );
                aTmpPos = aStartPos;
                if ( !IsVertical() )
                {
                    aTmpPos.AdjustX(pLine->GetStartPosX() );
                    aTmpPos.AdjustY(pLine->GetMaxAscent() );
                    aStartPos.AdjustY(pLine->GetHeight() );
                    if (nLine != nLastLine)
                        aStartPos.AdjustY(nVertLineSpacing );
                }
                else
                {
                    if ( IsTopToBottom() )
                    {
                        aTmpPos.AdjustY(pLine->GetStartPosX() );
                        aTmpPos.AdjustX( -(pLine->GetMaxAscent()) );
                        aStartPos.AdjustX( -(pLine->GetHeight()) );
                        if (nLine != nLastLine)
                            aStartPos.AdjustX( -nVertLineSpacing );
                    }
                    else
                    {
                        aTmpPos.AdjustY( -(pLine->GetStartPosX()) );
                        aTmpPos.AdjustX(pLine->GetMaxAscent() );
                        aStartPos.AdjustX(pLine->GetHeight() );
                        if (nLine != nLastLine)
                            aStartPos.AdjustX(nVertLineSpacing );
                    }
                }

                if ( ( !IsVertical() && ( aStartPos.Y() > aClipRect.Top() ) )
                    || ( IsVertical() && IsTopToBottom() && aStartPos.X() < aClipRect.Right() )
                    || ( IsVertical() && !IsTopToBottom() && aStartPos.X() > aClipRect.Left() ) )
                {
                    bPaintBullet = false;

                    // Why not just also call when stripping portions? This will give the correct values
                    // and needs no position corrections in OutlinerEditEng::DrawingText which tries to call
                    // PaintBullet correctly; exactly what GetEditEnginePtr()->PaintingFirstLine
                    // does, too. No change for not-layouting (painting).
                    if(0 == nLine) // && !bStripOnly)
                    {
                        GetEditEnginePtr()->PaintingFirstLine( n, aParaStart, aTmpPos.Y(), aOrigin, nOrientation, pOutDev );

                        // Remember whether a bullet was painted.
                        const SfxBoolItem& rBulletState = pEditEngine->GetParaAttrib(n, EE_PARA_BULLETSTATE);
                        bPaintBullet = rBulletState.GetValue();
                    }


                    // Over the Portions of the line...

                    bool bParsingFields = false;
                    std::vector< sal_Int32 >::iterator itSubLines;

                    for ( sal_Int32 nPortion = pLine->GetStartPortion(); nPortion <= pLine->GetEndPortion(); nPortion++ )
                    {
                        DBG_ASSERT( pPortion->GetTextPortions().Count(), "Line without Textportion in Paint!" );
                        const TextPortion& rTextPortion = pPortion->GetTextPortions()[nPortion];

                        long nPortionXOffset = GetPortionXOffset( pPortion, pLine, nPortion );
                        if ( !IsVertical() )
                        {
                            aTmpPos.setX( aStartPos.X() + nPortionXOffset );
                            if ( aTmpPos.X() > aClipRect.Right() )
                                break;  // No further output in line necessary
                        }
                        else
                        {
                            if( IsTopToBottom() )
                            {
                                aTmpPos.setY( aStartPos.Y() + nPortionXOffset );
                                if ( aTmpPos.Y() > aClipRect.Bottom() )
                                    break;  // No further output in line necessary
                            }
                            else
                            {
                                aTmpPos.setY( aStartPos.Y() - nPortionXOffset );
                                if (aTmpPos.Y() < aClipRect.Top())
                                    break;  // No further output in line necessary
                            }
                        }

                        switch ( rTextPortion.GetKind() )
                        {
                            case PortionKind::TEXT:
                            case PortionKind::FIELD:
                            case PortionKind::HYPHENATOR:
                            {
                                SeekCursor( pPortion->GetNode(), nIndex+1, aTmpFont, pOutDev );

                                bool bDrawFrame = false;

                                if ( ( rTextPortion.GetKind() == PortionKind::FIELD ) && !aTmpFont.IsTransparent() &&
                                     ( GetBackgroundColor() != COL_AUTO ) && GetBackgroundColor().IsDark() &&
                                     ( IsAutoColorEnabled() && ( pOutDev->GetOutDevType() != OUTDEV_PRINTER ) ) )
                                {
                                    aTmpFont.SetTransparent( true );
                                    pOutDev->SetFillColor();
                                    pOutDev->SetLineColor( GetAutoColor() );
                                    bDrawFrame = true;
                                }

#if OSL_DEBUG_LEVEL > 2
                                // Do we really need this if statement?
                                if ( rTextPortion.GetKind() == PortionKind::HYPHENATOR )
                                {
                                    aTmpFont.SetFillColor( COL_LIGHTGRAY );
                                    aTmpFont.SetTransparent( sal_False );
                                }
                                if ( rTextPortion.IsRightToLeft()  )
                                {
                                    aTmpFont.SetFillColor( COL_LIGHTGRAY );
                                    aTmpFont.SetTransparent( sal_False );
                                }
                                else if ( GetI18NScriptType( EditPaM( pPortion->GetNode(), nIndex+1 ) ) == i18n::ScriptType::COMPLEX )
                                {
                                    aTmpFont.SetFillColor( COL_LIGHTCYAN );
                                    aTmpFont.SetTransparent( sal_False );
                                }
#endif
                                aTmpFont.SetPhysFont( pOutDev );

                                // #114278# Saving both layout mode and language (since I'm
                                // potentially changing both)
                                pOutDev->Push( PushFlags::TEXTLAYOUTMODE|PushFlags::TEXTLANGUAGE );
                                ImplInitLayoutMode( pOutDev, n, nIndex );
                                ImplInitDigitMode(pOutDev, aTmpFont.GetLanguage());

                                OUString aText;
                                sal_Int32 nTextStart = 0;
                                sal_Int32 nTextLen = 0;
                                const long* pDXArray = nullptr;
                                std::unique_ptr<long[]> pTmpDXArray;

                                if ( rTextPortion.GetKind() == PortionKind::TEXT )
                                {
                                    aText = pPortion->GetNode()->GetString();
                                    nTextStart = nIndex;
                                    nTextLen = rTextPortion.GetLen();
                                    pDXArray = pLine->GetCharPosArray().data() + (nIndex - pLine->GetStart());

                                    // Paint control characters (#i55716#)
                                    /* XXX: Given that there's special handling
                                     * only for some specific characters
                                     * (U+200B ZERO WIDTH SPACE and U+2060 WORD
                                     * JOINER) it is assumed to be not relevant
                                     * for MarkUrlFields(). */
                                    if ( aStatus.MarkNonUrlFields() )
                                    {
                                        sal_Int32 nTmpIdx;
                                        const sal_Int32 nTmpEnd = nTextStart + rTextPortion.GetLen();

                                        for ( nTmpIdx = nTextStart; nTmpIdx <= nTmpEnd ; ++nTmpIdx )
                                        {
                                            const sal_Unicode cChar = ( nTmpIdx != aText.getLength() && ( nTmpIdx != nTextStart || 0 == nTextStart ) ) ?
                                                                        aText[nTmpIdx] :
                                                                        0;

                                            if ( 0x200B == cChar || 0x2060 == cChar )
                                            {
                                                const OUString aBlank( ' ' );
                                                long nHalfBlankWidth = aTmpFont.QuickGetTextSize( pOutDev, aBlank, 0, 1 ).Width() / 2;

                                                const long nAdvanceX = ( nTmpIdx == nTmpEnd ?
                                                                         rTextPortion.GetSize().Width() :
                                                                         pDXArray[ nTmpIdx - nTextStart ] ) - nHalfBlankWidth;
                                                const long nAdvanceY = -pLine->GetMaxAscent();

                                                Point aTopLeftRectPos( aTmpPos );
                                                if ( !IsVertical() )
                                                {
                                                    aTopLeftRectPos.AdjustX(nAdvanceX );
                                                    aTopLeftRectPos.AdjustY(nAdvanceY );
                                                }
                                                else
                                                {
                                                    if( IsTopToBottom() )
                                                    {
                                                        aTopLeftRectPos.AdjustY( -nAdvanceX );
                                                        aTopLeftRectPos.AdjustX(nAdvanceY );
                                                    }
                                                    else
                                                    {
                                                        aTopLeftRectPos.AdjustY(nAdvanceX );
                                                        aTopLeftRectPos.AdjustX( -nAdvanceY );
                                                    }
                                                }

                                                Point aBottomRightRectPos( aTopLeftRectPos );
                                                if ( !IsVertical() )
                                                {
                                                    aBottomRightRectPos.AdjustX(2 * nHalfBlankWidth );
                                                    aBottomRightRectPos.AdjustY(pLine->GetHeight() );
                                                }
                                                else
                                                {
                                                    if (IsTopToBottom())
                                                    {
                                                        aBottomRightRectPos.AdjustX(pLine->GetHeight() );
                                                        aBottomRightRectPos.AdjustY( -(2 * nHalfBlankWidth) );
                                                    }
                                                    else
                                                    {
                                                        aBottomRightRectPos.AdjustX( -(pLine->GetHeight()) );
                                                        aBottomRightRectPos.AdjustY(2 * nHalfBlankWidth );
                                                    }
                                                }

                                                pOutDev->Push( PushFlags::FILLCOLOR );
                                                pOutDev->Push( PushFlags::LINECOLOR );
                                                pOutDev->SetFillColor( COL_LIGHTGRAY );
                                                pOutDev->SetLineColor( COL_LIGHTGRAY );

                                                const tools::Rectangle aBackRect( aTopLeftRectPos, aBottomRightRectPos );
                                                pOutDev->DrawRect( aBackRect );

                                                pOutDev->Pop();
                                                pOutDev->Pop();

                                                if ( 0x200B == cChar )
                                                {
                                                    const OUString aSlash( '/' );
                                                    const short nOldEscapement = aTmpFont.GetEscapement();
                                                    const sal_uInt8 nOldPropr = aTmpFont.GetPropr();

                                                    aTmpFont.SetEscapement( -20 );
                                                    aTmpFont.SetPropr( 25 );
                                                    aTmpFont.SetPhysFont( pOutDev );

                                                    const Size aSlashSize = aTmpFont.QuickGetTextSize( pOutDev, aSlash, 0, 1 );
                                                    Point aSlashPos( aTmpPos );
                                                    const long nAddX = nHalfBlankWidth - aSlashSize.Width() / 2;
                                                    if ( !IsVertical() )
                                                    {
                                                        aSlashPos.setX( aTopLeftRectPos.X() + nAddX );
                                                    }
                                                    else
                                                    {
                                                        if (IsTopToBottom())
                                                            aSlashPos.setY( aTopLeftRectPos.Y() + nAddX );
                                                        else
                                                            aSlashPos.setY( aTopLeftRectPos.Y() - nAddX );
                                                    }

                                                    aTmpFont.QuickDrawText( pOutDev, aSlashPos, aSlash, 0, 1 );

                                                    aTmpFont.SetEscapement( nOldEscapement );
                                                    aTmpFont.SetPropr( nOldPropr );
                                                    aTmpFont.SetPhysFont( pOutDev );
                                                }
                                            }
                                        }
                                    }
                                }
                                else if ( rTextPortion.GetKind() == PortionKind::FIELD )
                                {
                                    const EditCharAttrib* pAttr = pPortion->GetNode()->GetCharAttribs().FindFeature(nIndex);
                                    DBG_ASSERT( pAttr, "Field not found");
                                    DBG_ASSERT( pAttr && dynamic_cast< const SvxFieldItem* >( pAttr->GetItem() ) !=  nullptr, "Field of the wrong type! ");
                                    aText = static_cast<const EditCharAttribField*>(pAttr)->GetFieldValue();
                                    nTextStart = 0;
                                    nTextLen = aText.getLength();
                                    ExtraPortionInfo *pExtraInfo = rTextPortion.GetExtraInfos();
                                    // Do not split the Fields into different lines while editing
                                    // With EditView on Overlay bStripOnly is now set for stripping to
                                    // primitives. To stay compatible in EditMode use pActiveView to detect
                                    // when we are in EditMode. For whatever reason URLs are drawn as single
                                    // line in edit mode, originally clipped against edit area (which is no
                                    // longer done in Overlay mode and allows to *read* the URL).
                                    // It would be difficult to change this due to needed adaptations in
                                    // EditEngine (look for lineBreaksList creation)
                                    if( nullptr == pActiveView && bStripOnly && !bParsingFields && pExtraInfo && !pExtraInfo->lineBreaksList.empty() )
                                    {
                                        bParsingFields = true;
                                        itSubLines = pExtraInfo->lineBreaksList.begin();
                                    }

                                    if( bParsingFields )
                                    {
                                        if( itSubLines != pExtraInfo->lineBreaksList.begin() )
                                        {
                                            // only use GetMaxAscent(), pLine->GetHeight() will not
                                            // proceed as needed (see PortionKind::TEXT above and nAdvanceY)
                                            // what will lead to a compressed look with multiple lines
                                            const sal_uInt16 nMaxAscent(pLine->GetMaxAscent());

                                            if ( !IsVertical() )
                                            {
                                                aStartPos.AdjustY(nMaxAscent );
                                                aTmpPos.AdjustY(nMaxAscent );
                                            }
                                            else
                                            {
                                                if (IsTopToBottom())
                                                {
                                                    aTmpPos.AdjustX( -nMaxAscent );
                                                    aStartPos.AdjustX( -nMaxAscent );
                                                }
                                                else
                                                {
                                                    aTmpPos.AdjustX(nMaxAscent );
                                                    aStartPos.AdjustX(nMaxAscent );
                                                }
                                            }
                                        }
                                        std::vector< sal_Int32 >::iterator curIt = itSubLines;
                                        ++itSubLines;
                                        if( itSubLines != pExtraInfo->lineBreaksList.end() )
                                        {
                                            nTextStart = *curIt;
                                            nTextLen = *itSubLines - nTextStart;
                                        }
                                        else
                                        {
                                            nTextStart = *curIt;
                                            nTextLen = nTextLen - nTextStart;
                                            bParsingFields = false;
                                        }
                                    }

                                    pTmpDXArray.reset(new long[ aText.getLength() ]);
                                    pDXArray = pTmpDXArray.get();
                                    vcl::Font _aOldFont( GetRefDevice()->GetFont() );
                                    aTmpFont.SetPhysFont( GetRefDevice() );
                                    aTmpFont.QuickGetTextSize( GetRefDevice(), aText, nTextStart, nTextLen, pTmpDXArray.get() );

                                    // add a meta file comment if we record to a metafile
                                    if( bMetafileValid )
                                    {
                                        const SvxFieldItem* pFieldItem = dynamic_cast<const SvxFieldItem*>(pAttr->GetItem());
                                        if( pFieldItem )
                                        {
                                            const SvxFieldData* pFieldData = pFieldItem->GetField();
                                            if( pFieldData )
                                                pMtf->AddAction( pFieldData->createBeginComment() );
                                        }
                                    }

                                }
                                else if ( rTextPortion.GetKind() == PortionKind::HYPHENATOR )
                                {
                                    if ( rTextPortion.GetExtraValue() )
                                        aText = OUString(rTextPortion.GetExtraValue());
                                    aText += OUStringLiteral1(CH_HYPH);
                                    nTextStart = 0;
                                    nTextLen = aText.getLength();

                                    // crash when accessing 0 pointer in pDXArray
                                    pTmpDXArray.reset(new long[ aText.getLength() ]);
                                    pDXArray = pTmpDXArray.get();
                                    vcl::Font _aOldFont( GetRefDevice()->GetFont() );
                                    aTmpFont.SetPhysFont( GetRefDevice() );
                                    aTmpFont.QuickGetTextSize( GetRefDevice(), aText, 0, aText.getLength(), pTmpDXArray.get() );
                                }

                                long nTxtWidth = rTextPortion.GetSize().Width();

                                Point aOutPos( aTmpPos );
                                aRedLineTmpPos = aTmpPos;
                                // In RTL portions spell markup pos should be at the start of the
                                // first chara as well. That is on the right end of the portion
                                if (rTextPortion.IsRightToLeft())
                                    aRedLineTmpPos.AdjustX(rTextPortion.GetSize().Width() );

                                if ( bStripOnly )
                                {
                                    EEngineData::WrongSpellVector aWrongSpellVector;

                                    if(GetStatus().DoOnlineSpelling() && rTextPortion.GetLen())
                                    {
                                        WrongList* pWrongs = pPortion->GetNode()->GetWrongList();

                                        if(pWrongs && !pWrongs->empty())
                                        {
                                            size_t nStart = nIndex, nEnd = 0;
                                            bool bWrong = pWrongs->NextWrong(nStart, nEnd);
                                            const size_t nMaxEnd(nIndex + rTextPortion.GetLen());

                                            while(bWrong)
                                            {
                                                if(nStart >= nMaxEnd)
                                                {
                                                    break;
                                                }

                                                if(nStart < static_cast<size_t>(nIndex))
                                                {
                                                    nStart = nIndex;
                                                }

                                                if(nEnd > nMaxEnd)
                                                {
                                                    nEnd = nMaxEnd;
                                                }

                                                // add to vector
                                                aWrongSpellVector.emplace_back(nStart, nEnd);

                                                // goto next index
                                                nStart = nEnd + 1;

                                                if(nEnd < nMaxEnd)
                                                {
                                                    bWrong = pWrongs->NextWrong(nStart, nEnd);
                                                }
                                                else
                                                {
                                                    bWrong = false;
                                                }
                                            }
                                        }
                                    }

                                    const SvxFieldData* pFieldData = nullptr;

                                    if(PortionKind::FIELD == rTextPortion.GetKind())
                                    {
                                        const EditCharAttrib* pAttr = pPortion->GetNode()->GetCharAttribs().FindFeature(nIndex);
                                        const SvxFieldItem* pFieldItem = dynamic_cast<const SvxFieldItem*>(pAttr->GetItem());

                                        if(pFieldItem)
                                        {
                                            pFieldData = pFieldItem->GetField();
                                        }
                                    }

                                    // support for EOC, EOW, EOS TEXT comments. To support that,
                                    // the locale is needed. With the locale and a XBreakIterator it is
                                    // possible to re-create the text marking info on primitive level
                                    const lang::Locale aLocale(GetLocale(EditPaM(pPortion->GetNode(), nIndex + 1)));

                                    // create EOL and EOP bools
                                    const bool bEndOfLine(nPortion == pLine->GetEndPortion());
                                    const bool bEndOfParagraph(bEndOfLine && nLine + 1 == nLines);

                                    // get Overline color (from ((const SvxOverlineItem*)GetItem())->GetColor() in
                                    // consequence, but also already set at pOutDev)
                                    const Color aOverlineColor(pOutDev->GetOverlineColor());

                                    // get TextLine color (from ((const SvxUnderlineItem*)GetItem())->GetColor() in
                                    // consequence, but also already set at pOutDev)
                                    const Color aTextLineColor(pOutDev->GetTextLineColor());

                                    // Unicode code points conversion according to ctl text numeral setting
                                    aText = convertDigits(aText, nTextStart, nTextLen,
                                        ImplCalcDigitLang(aTmpFont.GetLanguage()));

                                    // StripPortions() data callback
                                    GetEditEnginePtr()->DrawingText( aOutPos, aText, nTextStart, nTextLen, pDXArray,
                                        aTmpFont, n, rTextPortion.GetRightToLeftLevel(),
                                        !aWrongSpellVector.empty() ? &aWrongSpellVector : nullptr,
                                        pFieldData,
                                        bEndOfLine, bEndOfParagraph, // support for EOL/EOP TEXT comments
                                        &aLocale,
                                        aOverlineColor,
                                        aTextLineColor);

                                    // #108052# remember that EOP is written already for this ParaPortion
                                    if(bEndOfParagraph)
                                    {
                                        bEndOfParagraphWritten = true;
                                    }
                                }
                                else
                                {
                                    short nEsc = aTmpFont.GetEscapement();
                                    if ( nOrientation )
                                    {
                                        // In case of high/low do it yourself:
                                        if ( aTmpFont.GetEscapement() )
                                        {
                                            long nDiff = aTmpFont.GetFontSize().Height() * aTmpFont.GetEscapement() / 100L;
                                            if ( !IsVertical() )
                                                aOutPos.AdjustY( -nDiff );
                                            else
                                            {
                                                if (IsTopToBottom())
                                                    aOutPos.AdjustX(nDiff );
                                                else
                                                    aOutPos.AdjustX( -nDiff );
                                            }
                                            aRedLineTmpPos = aOutPos;
                                            aTmpFont.SetEscapement( 0 );
                                        }

                                        aOutPos = lcl_ImplCalcRotatedPos( aOutPos, aOrigin, nSin, nCos );
                                        aTmpFont.SetOrientation( aTmpFont.GetOrientation()+nOrientation );
                                        aTmpFont.SetPhysFont( pOutDev );

                                    }

                                    // Take only what begins in the visible range:
                                    // Important, because of a bug in some graphic cards
                                    // when transparent font, output when negative
                                    if ( nOrientation || ( !IsVertical() && ( ( aTmpPos.X() + nTxtWidth ) >= nFirstVisXPos ) )
                                            || ( IsVertical() && ( ( aTmpPos.Y() + nTxtWidth ) >= nFirstVisYPos ) ) )
                                    {
                                        if ( nEsc && ( aTmpFont.GetUnderline() != LINESTYLE_NONE ) )
                                        {
                                            // Paint the high/low without underline,
                                            // Display the Underline on the
                                            // base line of the original font height...
                                            // But only if there was something underlined before!
                                            bool bSpecialUnderline = false;
                                            EditCharAttrib* pPrev = pPortion->GetNode()->GetCharAttribs().FindAttrib( EE_CHAR_ESCAPEMENT, nIndex );
                                            if ( pPrev )
                                            {
                                                SvxFont aDummy;
                                                // Underscore in front?
                                                if ( pPrev->GetStart() )
                                                {
                                                    SeekCursor( pPortion->GetNode(), pPrev->GetStart(), aDummy );
                                                    if ( aDummy.GetUnderline() != LINESTYLE_NONE )
                                                        bSpecialUnderline = true;
                                                }
                                                if ( !bSpecialUnderline && ( pPrev->GetEnd() < pPortion->GetNode()->Len() ) )
                                                {
                                                    SeekCursor( pPortion->GetNode(), pPrev->GetEnd()+1, aDummy );
                                                    if ( aDummy.GetUnderline() != LINESTYLE_NONE )
                                                        bSpecialUnderline = true;
                                                }
                                            }
                                            if ( bSpecialUnderline )
                                            {
                                                Size aSz = aTmpFont.GetPhysTxtSize( pOutDev, aText, nTextStart, nTextLen );
                                                sal_uInt8 nProp = aTmpFont.GetPropr();
                                                aTmpFont.SetEscapement( 0 );
                                                aTmpFont.SetPropr( 100 );
                                                aTmpFont.SetPhysFont( pOutDev );
                                                OUStringBuffer aBlanks;
                                                comphelper::string::padToLength( aBlanks, nTextLen, ' ' );
                                                Point aUnderlinePos( aOutPos );
                                                if ( nOrientation )
                                                    aUnderlinePos = lcl_ImplCalcRotatedPos( aTmpPos, aOrigin, nSin, nCos );
                                                pOutDev->DrawStretchText( aUnderlinePos, aSz.Width(), aBlanks.makeStringAndClear(), 0, nTextLen );

                                                aTmpFont.SetUnderline( LINESTYLE_NONE );
                                                if ( !nOrientation )
                                                    aTmpFont.SetEscapement( nEsc );
                                                aTmpFont.SetPropr( nProp );
                                                aTmpFont.SetPhysFont( pOutDev );
                                            }
                                        }
                                        Point aRealOutPos( aOutPos );
                                        if ( ( rTextPortion.GetKind() == PortionKind::TEXT )
                                               && rTextPortion.GetExtraInfos() && rTextPortion.GetExtraInfos()->bCompressed
                                               && rTextPortion.GetExtraInfos()->bFirstCharIsRightPunktuation )
                                        {
                                            aRealOutPos.AdjustX(rTextPortion.GetExtraInfos()->nPortionOffsetX );
                                        }

                                        // RTL portions with (#i37132#)
                                        // compressed blank should not paint this blank:
                                        if ( rTextPortion.IsRightToLeft() && nTextLen >= 2 &&
                                             pDXArray[ nTextLen - 1 ] ==
                                             pDXArray[ nTextLen - 2 ] &&
                                             ' ' == aText[nTextStart + nTextLen - 1] )
                                            --nTextLen;

                                        // output directly
                                        aTmpFont.QuickDrawText( pOutDev, aRealOutPos, aText, nTextStart, nTextLen, pDXArray );

                                        if ( bDrawFrame )
                                        {
                                            Point aTopLeft( aTmpPos );
                                            aTopLeft.AdjustY( -(pLine->GetMaxAscent()) );
                                            if ( nOrientation )
                                                aTopLeft = lcl_ImplCalcRotatedPos( aTopLeft, aOrigin, nSin, nCos );
                                            tools::Rectangle aRect( aTopLeft, rTextPortion.GetSize() );
                                            pOutDev->DrawRect( aRect );
                                        }

                                        // PDF export:
                                        if ( pPDFExtOutDevData )
                                        {
                                            if ( rTextPortion.GetKind() == PortionKind::FIELD )
                                            {
                                                const EditCharAttrib* pAttr = pPortion->GetNode()->GetCharAttribs().FindFeature(nIndex);
                                                const SvxFieldItem* pFieldItem = dynamic_cast<const SvxFieldItem*>(pAttr->GetItem());
                                                if( pFieldItem )
                                                {
                                                    const SvxFieldData* pFieldData = pFieldItem->GetField();
                                                    if ( auto pUrlField = dynamic_cast< const SvxURLField* >( pFieldData ) )
                                                    {
                                                        Point aTopLeft( aTmpPos );
                                                        aTopLeft.AdjustY( -(pLine->GetMaxAscent()) );

                                                        tools::Rectangle aRect( aTopLeft, rTextPortion.GetSize() );
                                                        vcl::PDFExtOutDevBookmarkEntry aBookmark;
                                                        aBookmark.nLinkId = pPDFExtOutDevData->CreateLink( aRect );
                                                        aBookmark.aBookmark = pUrlField->GetURL();
                                                        std::vector< vcl::PDFExtOutDevBookmarkEntry >& rBookmarks = pPDFExtOutDevData->GetBookmarks();
                                                        rBookmarks.push_back( aBookmark );
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    const WrongList* const pWrongList = pPortion->GetNode()->GetWrongList();
                                    if ( GetStatus().DoOnlineSpelling() && pWrongList && !pWrongList->empty() && rTextPortion.GetLen() )
                                    {
                                        {//#105750# adjust LinePos for superscript or subscript text
                                            short _nEsc = aTmpFont.GetEscapement();
                                            if( _nEsc )
                                            {
                                                long nShift = (_nEsc * aTmpFont.GetFontSize().Height()) / 100L;
                                                if( !IsVertical() )
                                                    aRedLineTmpPos.AdjustY( -nShift );
                                                else
                                                    if (IsTopToBottom())
                                                        aRedLineTmpPos.AdjustX(nShift );
                                                    else
                                                        aRedLineTmpPos.AdjustX( -nShift );
                                            }
                                        }
                                        Color aOldColor( pOutDev->GetLineColor() );
                                        pOutDev->SetLineColor( GetColorConfig().GetColorValue( svtools::SPELL ).nColor );
                                        lcl_DrawRedLines( pOutDev, aTmpFont.GetFontSize().Height(), aRedLineTmpPos, static_cast<size_t>(nIndex), static_cast<size_t>(nIndex) + rTextPortion.GetLen(), pDXArray, pPortion->GetNode()->GetWrongList(), nOrientation, aOrigin, IsVertical(), rTextPortion.IsRightToLeft() );
                                        pOutDev->SetLineColor( aOldColor );
                                    }
                                }

                                pOutDev->Pop();

                                pTmpDXArray.reset();

                                if ( rTextPortion.GetKind() == PortionKind::FIELD )
                                {
                                    const EditCharAttrib* pAttr = pPortion->GetNode()->GetCharAttribs().FindFeature(nIndex);
                                    DBG_ASSERT( pAttr, "Field not found" );
                                    DBG_ASSERT( pAttr && dynamic_cast< const SvxFieldItem* >( pAttr->GetItem() ) !=  nullptr, "Wrong type of field!" );

                                    // add a meta file comment if we record to a metafile
                                    if( bMetafileValid )
                                    {
                                        const SvxFieldItem* pFieldItem = dynamic_cast<const SvxFieldItem*>(pAttr->GetItem());

                                        if( pFieldItem )
                                        {
                                            const SvxFieldData* pFieldData = pFieldItem->GetField();
                                            if( pFieldData )
                                                pMtf->AddAction( SvxFieldData::createEndComment() );
                                        }
                                    }

                                }

                            }
                            break;
                            case PortionKind::TAB:
                            {
                                if ( rTextPortion.GetExtraValue() && ( rTextPortion.GetExtraValue() != ' ' ) )
                                {
                                    SeekCursor( pPortion->GetNode(), nIndex+1, aTmpFont, pOutDev );
                                    aTmpFont.SetTransparent( false );
                                    aTmpFont.SetEscapement( 0 );
                                    aTmpFont.SetPhysFont( pOutDev );
                                    long nCharWidth = aTmpFont.QuickGetTextSize( pOutDev,
                                        OUString(rTextPortion.GetExtraValue()), 0, 1 ).Width();
                                    sal_Int32 nChars = 2;
                                    if( nCharWidth )
                                        nChars = rTextPortion.GetSize().Width() / nCharWidth;
                                    if ( nChars < 2 )
                                        nChars = 2; // is compressed by DrawStretchText.
                                    else if ( nChars == 2 )
                                        nChars = 3; // looks better

                                    OUStringBuffer aBuf;
                                    comphelper::string::padToLength(aBuf, nChars, rTextPortion.GetExtraValue());
                                    OUString aText(aBuf.makeStringAndClear());
                                    aTmpFont.QuickDrawText( pOutDev, aTmpPos, aText, 0, aText.getLength() );
                                    pOutDev->DrawStretchText( aTmpPos, rTextPortion.GetSize().Width(), aText );

                                    if ( bStripOnly )
                                    {
                                        // create EOL and EOP bools
                                        const bool bEndOfLine(nPortion == pLine->GetEndPortion());
                                        const bool bEndOfParagraph(bEndOfLine && nLine + 1 == nLines);

                                        const Color aOverlineColor(pOutDev->GetOverlineColor());
                                        const Color aTextLineColor(pOutDev->GetTextLineColor());

                                        // StripPortions() data callback
                                        GetEditEnginePtr()->DrawingTab( aTmpPos,
                                            rTextPortion.GetSize().Width(),
                                            OUString(rTextPortion.GetExtraValue()),
                                            aTmpFont, n, rTextPortion.GetRightToLeftLevel(),
                                            bEndOfLine, bEndOfParagraph,
                                            aOverlineColor, aTextLineColor);
                                    }
                                }
                                else if ( bStripOnly )
                                {
                                    // #i108052# When stripping, a callback for _empty_ paragraphs is also needed.
                                    // This was optimized away (by not rendering the space-only tab portion), so do
                                    // it manually here.
                                    const bool bEndOfLine(nPortion == pLine->GetEndPortion());
                                    const bool bEndOfParagraph(bEndOfLine && nLine + 1 == nLines);

                                    const Color aOverlineColor(pOutDev->GetOverlineColor());
                                    const Color aTextLineColor(pOutDev->GetTextLineColor());

                                    GetEditEnginePtr()->DrawingText(
                                        aTmpPos, OUString(), 0, 0, nullptr,
                                        aTmpFont, n, 0,
                                        nullptr,
                                        nullptr,
                                        bEndOfLine, bEndOfParagraph,
                                        nullptr,
                                        aOverlineColor,
                                        aTextLineColor);
                                }
                            }
                            break;
                            case PortionKind::LINEBREAK: break;
                        }
                        if( bParsingFields )
                            nPortion--;
                        else
                            nIndex = nIndex + rTextPortion.GetLen();

                    }
                }

                if ( ( nLine != nLastLine ) && !aStatus.IsOutliner() )
                {
                    if ( !IsVertical() )
                        aStartPos.AdjustY(nSBL );
                    else
                    {
                        if( IsTopToBottom() )
                            aStartPos.AdjustX( -nSBL );
                        else
                            aStartPos.AdjustX(nSBL );
                    }
                }

                // no more visible actions?
                if ( !IsVertical() && ( aStartPos.Y() >= aClipRect.Bottom() ) )
                    break;
                else if ( IsVertical() && IsTopToBottom() && ( aStartPos.X() <= aClipRect.Left() ) )
                    break;
                else if (IsVertical() && !IsTopToBottom() && (aStartPos.X() >= aClipRect.Right()))
                    break;
            }

            if ( !aStatus.IsOutliner() )
            {
                const SvxULSpaceItem& rULItem = pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_ULSPACE );
                long nUL = GetYValue( rULItem.GetLower() );
                if ( !IsVertical() )
                    aStartPos.AdjustY(nUL );
                else
                {
                    if (IsTopToBottom())
                        aStartPos.AdjustX( -nUL );
                    else
                        aStartPos.AdjustX(nUL );
                }
            }

            // #108052# Safer way for #i108052# and #i118881#: If for the current ParaPortion
            // EOP is not written, do it now. This will be safer than before. It has shown
            // that the reason for #i108052# was fixed/removed again, so this is a try to fix
            // the number of paragraphs (and counting empty ones) now independent from the
            // changes in EditEngine behaviour.
            if(!bEndOfParagraphWritten && !bPaintBullet && bStripOnly)
            {
                const Color aOverlineColor(pOutDev->GetOverlineColor());
                const Color aTextLineColor(pOutDev->GetTextLineColor());

                GetEditEnginePtr()->DrawingText(
                    aTmpPos, OUString(), 0, 0, nullptr,
                    aTmpFont, n, 0,
                    nullptr,
                    nullptr,
                    false, true, // support for EOL/EOP TEXT comments
                    nullptr,
                    aOverlineColor,
                    aTextLineColor);
            }
        }
        else
        {
            if ( !IsVertical() )
                aStartPos.AdjustY(nParaHeight );
            else
            {
                if (IsTopToBottom())
                    aStartPos.AdjustX( -nParaHeight );
                else
                    aStartPos.AdjustX(nParaHeight );
            }
        }

        if ( pPDFExtOutDevData )
            pPDFExtOutDevData->EndStructureElement();

        // no more visible actions?
        if ( !IsVertical() && ( aStartPos.Y() > aClipRect.Bottom() ) )
            break;
        if ( IsVertical() && IsTopToBottom() && ( aStartPos.X() < aClipRect.Left() ) )
            break;
        if (IsVertical() && !IsTopToBottom() && ( aStartPos.X() > aClipRect.Right() ) )
            break;
    }
}

void ImpEditEngine::Paint( ImpEditView* pView, const tools::Rectangle& rRect, OutputDevice* pTargetDevice )
{
    DBG_ASSERT( pView, "No View - No Paint!" );

    if ( !GetUpdateMode() || IsInUndo() )
        return;

    // Intersection of paint area and output area.
    tools::Rectangle aClipRect( pView->GetOutputArea() );
    aClipRect.Intersection( rRect );

    OutputDevice* pTarget = pTargetDevice ? pTargetDevice : pView->GetWindow();

    Point aStartPos;
    if ( !IsVertical() )
    {
        aStartPos = pView->GetOutputArea().TopLeft();
        aStartPos.AdjustX( -(pView->GetVisDocLeft()) );
        aStartPos.AdjustY( -(pView->GetVisDocTop()) );
    }
    else
    {
        if( IsTopToBottom() )
        {
            aStartPos = pView->GetOutputArea().TopRight();
            aStartPos.AdjustX(pView->GetVisDocTop() );
            aStartPos.AdjustY( -(pView->GetVisDocLeft()) );
        }
        else
        {
            aStartPos = pView->GetOutputArea().BottomLeft();
            aStartPos.AdjustX( -(pView->GetVisDocTop()) );
            aStartPos.AdjustY(pView->GetVisDocLeft() );
        }
    }

    // If Doc-width < Output Area,Width and not wrapped fields,
    // the fields usually protrude if > line.
    // (Not at the top, since there the Doc-width from formatting is already
    // there)
    if ( !IsVertical() && ( pView->GetOutputArea().GetWidth() > GetPaperSize().Width() ) )
    {
        long nMaxX = pView->GetOutputArea().Left() + GetPaperSize().Width();
        if ( aClipRect.Left() > nMaxX )
            return;
        if ( aClipRect.Right() > nMaxX )
            aClipRect.SetRight( nMaxX );
    }

    bool bClipRegion = pTarget->IsClipRegion();
    vcl::Region aOldRegion = pTarget->GetClipRegion();
    pTarget->IntersectClipRegion( aClipRect );

    Paint( pTarget, aClipRect, aStartPos );

    if ( bClipRegion )
        pTarget->SetClipRegion( aOldRegion );
    else
        pTarget->SetClipRegion();

    // In case of tiled rendering pass a region to DrawSelectionXOR(), so that
    // selection callbacks are not emitted during every repaint.
    vcl::Region aRegion;
    pView->DrawSelectionXOR(pView->GetEditSelection(), comphelper::LibreOfficeKit::isActive() ? &aRegion : nullptr, pTarget);
}

void ImpEditEngine::InsertContent( ContentNode* pNode, sal_Int32 nPos )
{
    DBG_ASSERT( pNode, "NULL-Pointer in InsertContent! " );
    DBG_ASSERT( IsInUndo(), "InsertContent only for Undo()!" );
    GetParaPortions().Insert(nPos, std::make_unique<ParaPortion>( pNode ));
    aEditDoc.Insert(nPos, pNode);
    if ( IsCallParaInsertedOrDeleted() )
        GetEditEnginePtr()->ParagraphInserted( nPos );
}

EditPaM ImpEditEngine::SplitContent( sal_Int32 nNode, sal_Int32 nSepPos )
{
    ContentNode* pNode = aEditDoc.GetObject( nNode );
    DBG_ASSERT( pNode, "Invalid Node in SplitContent" );
    DBG_ASSERT( IsInUndo(), "SplitContent only for Undo()!" );
    DBG_ASSERT( nSepPos <= pNode->Len(), "Index out of range: SplitContent" );
    EditPaM aPaM( pNode, nSepPos );
    return ImpInsertParaBreak( aPaM );
}

EditPaM ImpEditEngine::ConnectContents( sal_Int32 nLeftNode, bool bBackward )
{
    ContentNode* pLeftNode = aEditDoc.GetObject( nLeftNode );
    ContentNode* pRightNode = aEditDoc.GetObject( nLeftNode+1 );
    DBG_ASSERT( pLeftNode, "Invalid left node in ConnectContents ");
    DBG_ASSERT( pRightNode, "Invalid right node in ConnectContents ");
    DBG_ASSERT( IsInUndo(), "ConnectContent only for Undo()!" );
    return ImpConnectParagraphs( pLeftNode, pRightNode, bBackward );
}

void ImpEditEngine::SetUpdateMode( bool bUp, EditView* pCurView, bool bForceUpdate )
{
    bool bChanged = ( GetUpdateMode() != bUp );

    // When switching from sal_True to sal_False, all selections were visible,
    // => paint over
    // the other hand, were all invisible => paint
    // If !bFormatted, e.g. after SetText, then if UpdateMode=sal_True
    // formatting is not needed immediately, probably because more text is coming.
    // At latest it is formatted at a Paint/CalcTextWidth.
    bUpdate = bUp;
    if ( bUpdate && ( bChanged || bForceUpdate ) )
        FormatAndUpdate( pCurView );
}

void ImpEditEngine::ShowParagraph( sal_Int32 nParagraph, bool bShow )
{
    ParaPortion* pPPortion = GetParaPortions().SafeGetObject( nParagraph );
    DBG_ASSERT( pPPortion, "ShowParagraph: Paragraph does not exist! ");
    if ( pPPortion && ( pPPortion->IsVisible() != bShow ) )
    {
        pPPortion->SetVisible( bShow );

        if ( !bShow )
        {
            // Mark as deleted, so that no selection will end or begin at
            // this paragraph...
            aDeletedNodes.push_back(std::make_unique<DeletedNodeInfo>( pPPortion->GetNode(), nParagraph ));
            UpdateSelections();
            // The region below will not be invalidated if UpdateMode = sal_False!
            // If anyway, then save as sal_False before SetVisible !
        }

        if ( bShow && ( pPPortion->IsInvalid() || !pPPortion->nHeight ) )
        {
            if ( !GetTextRanger() )
            {
                if ( pPPortion->IsInvalid() )
                {
                    vcl::Font aOldFont( GetRefDevice()->GetFont() );
                    CreateLines( nParagraph, 0 );   // 0: No TextRanger
                }
                else
                {
                    CalcHeight( pPPortion );
                }
                nCurTextHeight += pPPortion->GetHeight();
            }
            else
            {
                nCurTextHeight = 0x7fffffff;
            }
        }

        pPPortion->SetMustRepaint( true );
        if ( GetUpdateMode() && !IsInUndo() && !GetTextRanger() )
        {
            aInvalidRect = tools::Rectangle(    Point( 0, GetParaPortions().GetYOffset( pPPortion ) ),
                                        Point( GetPaperSize().Width(), nCurTextHeight ) );
            UpdateViews( GetActiveView() );
        }
    }
}

EditSelection ImpEditEngine::MoveParagraphs( Range aOldPositions, sal_Int32 nNewPos, EditView* pCurView )
{
    DBG_ASSERT( GetParaPortions().Count() != 0, "No paragraphs found: MoveParagraphs" );
    if ( GetParaPortions().Count() == 0 )
        return EditSelection();
    aOldPositions.Justify();

    EditSelection aSel( ImpMoveParagraphs( aOldPositions, nNewPos ) );

    if ( nNewPos >= GetParaPortions().Count() )
        nNewPos = GetParaPortions().Count() - 1;

    // Where the paragraph was inserted it has to be properly redrawn:
    // Where the paragraph was removed it has to be properly redrawn:
    // ( and correspondingly in between as well...)
    if ( pCurView && GetUpdateMode() )
    {
        // in this case one can redraw directly without invalidating the
        // Portions
        sal_Int32 nFirstPortion = std::min( static_cast<sal_Int32>(aOldPositions.Min()), nNewPos );
        sal_Int32 nLastPortion = std::max( static_cast<sal_Int32>(aOldPositions.Max()), nNewPos );

        ParaPortion* pUpperPortion = GetParaPortions().SafeGetObject( nFirstPortion );
        ParaPortion* pLowerPortion = GetParaPortions().SafeGetObject( nLastPortion );
        if (pUpperPortion && pLowerPortion)
        {
            aInvalidRect = tools::Rectangle();  // make empty
            aInvalidRect.SetLeft( 0 );
            aInvalidRect.SetRight( aPaperSize.Width() );
            aInvalidRect.SetTop( GetParaPortions().GetYOffset( pUpperPortion ) );
            aInvalidRect.SetBottom( GetParaPortions().GetYOffset( pLowerPortion ) + pLowerPortion->GetHeight() );

            UpdateViews( pCurView );
        }
    }
    else
    {
        // redraw from the upper invalid position
        sal_Int32 nFirstInvPara = std::min( static_cast<sal_Int32>(aOldPositions.Min()), nNewPos );
        InvalidateFromParagraph( nFirstInvPara );
    }
    return aSel;
}

void ImpEditEngine::InvalidateFromParagraph( sal_Int32 nFirstInvPara )
{
    // The following paragraphs are not invalidated, since ResetHeight()
    // => size change => all the following are re-issued anyway.
    ParaPortion* pTmpPortion;
    if ( nFirstInvPara != 0 )
    {
        pTmpPortion = GetParaPortions()[nFirstInvPara-1];
        pTmpPortion->MarkInvalid( pTmpPortion->GetNode()->Len(), 0 );
    }
    else
    {
        pTmpPortion = GetParaPortions()[0];
        pTmpPortion->MarkSelectionInvalid( 0 );
    }
    pTmpPortion->ResetHeight();
}

IMPL_LINK_NOARG(ImpEditEngine, StatusTimerHdl, Timer *, void)
{
    CallStatusHdl();
}

void ImpEditEngine::CallStatusHdl()
{
    if ( aStatusHdlLink.IsSet() && bool(aStatus.GetStatusWord()) )
    {
        // The Status has to be reset before the Call,
        // since other Flags might be set in the handler...
        EditStatus aTmpStatus( aStatus );
        aStatus.Clear();
        aStatusHdlLink.Call( aTmpStatus );
        aStatusTimer.Stop();    // If called by hand...
    }
}

ContentNode* ImpEditEngine::GetPrevVisNode( ContentNode const * pCurNode )
{
    const ParaPortion* pPortion = FindParaPortion( pCurNode );
    DBG_ASSERT( pPortion, "GetPrevVisibleNode: No matching portion!" );
    pPortion = GetPrevVisPortion( pPortion );
    if ( pPortion )
        return pPortion->GetNode();
    return nullptr;
}

ContentNode* ImpEditEngine::GetNextVisNode( ContentNode const * pCurNode )
{
    const ParaPortion* pPortion = FindParaPortion( pCurNode );
    DBG_ASSERT( pPortion, "GetNextVisibleNode: No matching portion!" );
    pPortion = GetNextVisPortion( pPortion );
    if ( pPortion )
        return pPortion->GetNode();
    return nullptr;
}

const ParaPortion* ImpEditEngine::GetPrevVisPortion( const ParaPortion* pCurPortion ) const
{
    sal_Int32 nPara = GetParaPortions().GetPos( pCurPortion );
    DBG_ASSERT( nPara < GetParaPortions().Count() , "Portion not found: GetPrevVisPortion" );
    const ParaPortion* pPortion = nPara ? GetParaPortions()[--nPara] : nullptr;
    while ( pPortion && !pPortion->IsVisible() )
        pPortion = nPara ? GetParaPortions()[--nPara] : nullptr;

    return pPortion;
}

const ParaPortion* ImpEditEngine::GetNextVisPortion( const ParaPortion* pCurPortion ) const
{
    sal_Int32 nPara = GetParaPortions().GetPos( pCurPortion );
    DBG_ASSERT( nPara < GetParaPortions().Count() , "Portion not found: GetPrevVisNode" );
    const ParaPortion* pPortion = GetParaPortions().SafeGetObject( ++nPara );
    while ( pPortion && !pPortion->IsVisible() )
        pPortion = GetParaPortions().SafeGetObject( ++nPara );

    return pPortion;
}

long ImpEditEngine::CalcVertLineSpacing(Point& rStartPos) const
{
    long nTotalOccupiedHeight = 0;
    sal_Int32 nTotalLineCount = 0;
    const ParaPortionList& rParaPortions = GetParaPortions();
    sal_Int32 nParaCount = rParaPortions.Count();

    for (sal_Int32 i = 0; i < nParaCount; ++i)
    {
        if (GetVerJustification(i) != SvxCellVerJustify::Block)
            // All paragraphs must have the block justification set.
            return 0;

        const ParaPortion* pPortion = rParaPortions[i];
        nTotalOccupiedHeight += pPortion->GetFirstLineOffset();

        const SvxLineSpacingItem& rLSItem = pPortion->GetNode()->GetContentAttribs().GetItem(EE_PARA_SBL);
        sal_uInt16 nSBL = ( rLSItem.GetInterLineSpaceRule() == SvxInterLineSpaceRule::Fix )
                            ? GetYValue( rLSItem.GetInterLineSpace() ) : 0;

        const SvxULSpaceItem& rULItem = pPortion->GetNode()->GetContentAttribs().GetItem(EE_PARA_ULSPACE);
        long nUL = GetYValue( rULItem.GetLower() );

        const EditLineList& rLines = pPortion->GetLines();
        sal_Int32 nLineCount = rLines.Count();
        nTotalLineCount += nLineCount;
        for (sal_Int32 j = 0; j < nLineCount; ++j)
        {
            const EditLine& rLine = rLines[j];
            nTotalOccupiedHeight += rLine.GetHeight();
            if (j < nLineCount-1)
                nTotalOccupiedHeight += nSBL;
            nTotalOccupiedHeight += nUL;
        }
    }

    long nTotalSpace = IsVertical() ? aPaperSize.Width() : aPaperSize.Height();
    nTotalSpace -= nTotalOccupiedHeight;
    if (nTotalSpace <= 0 || nTotalLineCount <= 1)
        return 0;

    if (IsVertical())
    {
        if( IsTopToBottom() )
            // Shift the text to the right for the asian layout mode.
            rStartPos.AdjustX(nTotalSpace );
        else
            rStartPos.AdjustX( -nTotalSpace );
    }

    return nTotalSpace / (nTotalLineCount-1);
}

EditPaM ImpEditEngine::InsertParagraph( sal_Int32 nPara )
{
    EditPaM aPaM;
    if ( nPara != 0 )
    {
        ContentNode* pNode = GetEditDoc().GetObject( nPara-1 );
        if ( !pNode )
            pNode = GetEditDoc().GetObject( GetEditDoc().Count() - 1 );
        assert(pNode && "Not a single paragraph in InsertParagraph ?");
        aPaM = EditPaM( pNode, pNode->Len() );
    }
    else
    {
        ContentNode* pNode = GetEditDoc().GetObject( 0 );
        aPaM = EditPaM( pNode, 0 );
    }

    return ImpInsertParaBreak( aPaM );
}

std::unique_ptr<EditSelection> ImpEditEngine::SelectParagraph( sal_Int32 nPara )
{
    std::unique_ptr<EditSelection> pSel;
    ContentNode* pNode = GetEditDoc().GetObject( nPara );
    SAL_WARN_IF( !pNode, "editeng", "Paragraph does not exist: SelectParagraph" );
    if ( pNode )
        pSel.reset(new EditSelection( EditPaM( pNode, 0 ), EditPaM( pNode, pNode->Len() ) ));

    return pSel;
}

void ImpEditEngine::FormatAndUpdate( EditView* pCurView, bool bCalledFromUndo )
{
    if ( bDowning )
        return ;

    if ( IsInUndo() )
        IdleFormatAndUpdate( pCurView );
    else
    {
        if (bCalledFromUndo)
            // in order to make bullet points that have had their styles changed, redraw themselves
            for ( sal_Int32 nPortion = 0; nPortion < GetParaPortions().Count(); nPortion++ )
                GetParaPortions()[nPortion]->MarkInvalid( 0, 0 );
        FormatDoc();
        UpdateViews( pCurView );
    }

    EENotify aNotify(EE_NOTIFY_PROCESSNOTIFICATIONS);
    GetNotifyHdl().Call(aNotify);
}

void ImpEditEngine::SetFlatMode( bool bFlat )
{
    if ( bFlat != aStatus.UseCharAttribs() )
        return;

    if ( !bFlat )
        aStatus.TurnOnFlags( EEControlBits::USECHARATTRIBS );
    else
        aStatus.TurnOffFlags( EEControlBits::USECHARATTRIBS );

    aEditDoc.CreateDefFont( !bFlat );

    FormatFullDoc();
    UpdateViews();
    if ( pActiveView )
        pActiveView->ShowCursor();
}

void ImpEditEngine::SetCharStretching( sal_uInt16 nX, sal_uInt16 nY )
{
    bool bChanged(false);
    if ( !IsVertical() )
    {
        bChanged = nStretchX!=nX || nStretchY!=nY;
        nStretchX = nX;
        nStretchY = nY;
    }
    else
    {
        bChanged = nStretchX!=nY || nStretchY!=nX;
        nStretchX = nY;
        nStretchY = nX;
    }

    if (bChanged && aStatus.DoStretch())
    {
        FormatFullDoc();
        // (potentially) need everything redrawn
        aInvalidRect=tools::Rectangle(0,0,1000000,1000000);
        UpdateViews( GetActiveView() );
    }
}

const SvxNumberFormat* ImpEditEngine::GetNumberFormat( const ContentNode *pNode ) const
{
    const SvxNumberFormat *pRes = nullptr;

    if (pNode)
    {
        // get index of paragraph
        sal_Int32 nPara = GetEditDoc().GetPos( pNode );
        DBG_ASSERT( nPara < EE_PARA_NOT_FOUND, "node not found in array" );
        if (nPara < EE_PARA_NOT_FOUND)
        {
            // the called function may be overridden by an OutlinerEditEng
            // object to provide
            // access to the SvxNumberFormat of the Outliner.
            // The EditEngine implementation will just return 0.
            pRes = pEditEngine->GetNumberFormat( nPara );
        }
    }

    return pRes;
}

sal_Int32 ImpEditEngine::GetSpaceBeforeAndMinLabelWidth(
    const ContentNode *pNode,
    sal_Int32 *pnSpaceBefore, sal_Int32 *pnMinLabelWidth ) const
{
    // nSpaceBefore     matches the ODF attribute text:space-before
    // nMinLabelWidth   matches the ODF attribute text:min-label-width

    const SvxNumberFormat *pNumFmt = GetNumberFormat( pNode );

    // if no number format was found we have no Outliner or the numbering level
    // within the Outliner is -1 which means no number format should be applied.
    // Thus the default values to be returned are 0.
    sal_Int32 nSpaceBefore   = 0;
    sal_Int32 nMinLabelWidth = 0;

    if (pNumFmt)
    {
        nMinLabelWidth = -pNumFmt->GetFirstLineOffset();
        nSpaceBefore   = pNumFmt->GetAbsLSpace() - nMinLabelWidth;
        DBG_ASSERT( nMinLabelWidth >= 0, "ImpEditEngine::GetSpaceBeforeAndMinLabelWidth: min-label-width < 0 encountered" );
    }
    if (pnSpaceBefore)
        *pnSpaceBefore      = nSpaceBefore;
    if (pnMinLabelWidth)
        *pnMinLabelWidth    = nMinLabelWidth;

    return nSpaceBefore + nMinLabelWidth;
}

const SvxLRSpaceItem& ImpEditEngine::GetLRSpaceItem( ContentNode* pNode )
{
    return pNode->GetContentAttribs().GetItem( aStatus.IsOutliner() ? EE_PARA_OUTLLRSPACE : EE_PARA_LRSPACE );
}

// select a representative text language for the digit type according to the
// text numeral setting:
LanguageType ImpEditEngine::ImplCalcDigitLang(LanguageType eCurLang) const
{
    if (utl::ConfigManager::IsFuzzing())
        return LANGUAGE_ENGLISH_US;

    // #114278# Also setting up digit language from Svt options
    // (cannot reliably inherit the outdev's setting)
    if( !pCTLOptions )
        pCTLOptions.reset( new SvtCTLOptions );

    LanguageType eLang = eCurLang;
    const SvtCTLOptions::TextNumerals nCTLTextNumerals = pCTLOptions->GetCTLTextNumerals();

    if ( SvtCTLOptions::NUMERALS_HINDI == nCTLTextNumerals )
        eLang = LANGUAGE_ARABIC_SAUDI_ARABIA;
    else if ( SvtCTLOptions::NUMERALS_ARABIC == nCTLTextNumerals )
        eLang = LANGUAGE_ENGLISH;
    else if ( SvtCTLOptions::NUMERALS_SYSTEM == nCTLTextNumerals )
        eLang = Application::GetSettings().GetLanguageTag().getLanguageType();

    return eLang;
}

OUString ImpEditEngine::convertDigits(const OUString &rString, sal_Int32 nStt, sal_Int32 nLen, LanguageType eDigitLang)
{
    OUStringBuffer aBuf(rString);
    for (sal_Int32 nIdx = nStt, nEnd = nStt + nLen; nIdx < nEnd; ++nIdx)
    {
        sal_Unicode cChar = aBuf[nIdx];
        if (cChar >= '0' && cChar <= '9')
            aBuf[nIdx] = GetLocalizedChar(cChar, eDigitLang);
    }
    return aBuf.makeStringAndClear();
}

// Either sets the digit mode at the output device
void ImpEditEngine::ImplInitDigitMode(OutputDevice* pOutDev, LanguageType eCurLang)
{
    assert(pOutDev); //presumably there isn't any case where pOutDev should be NULL?
    if (pOutDev)
        pOutDev->SetDigitLanguage(ImplCalcDigitLang(eCurLang));
}

void ImpEditEngine::ImplInitLayoutMode( OutputDevice* pOutDev, sal_Int32 nPara, sal_Int32 nIndex )
{
    bool bCTL = false;
    bool bR2L = false;
    if ( nIndex == -1 )
    {
        bCTL = HasScriptType( nPara, i18n::ScriptType::COMPLEX );
        bR2L = IsRightToLeft( nPara );
    }
    else
    {
        ContentNode* pNode = GetEditDoc().GetObject( nPara );
        short nScriptType = GetI18NScriptType( EditPaM( pNode, nIndex+1 ) );
        bCTL = nScriptType == i18n::ScriptType::COMPLEX;
        // this change was discussed in issue 37190
        bR2L = (GetRightToLeft( nPara, nIndex + 1) % 2) != 0;
        // it also works for issue 55927
    }

    ComplexTextLayoutFlags nLayoutMode = pOutDev->GetLayoutMode();

    // We always use the left position for DrawText()
    nLayoutMode &= ~ComplexTextLayoutFlags::BiDiRtl;

    if ( !bCTL && !bR2L)
    {
        // No Bidi checking necessary
        nLayoutMode |= ComplexTextLayoutFlags::BiDiStrong;
    }
    else
    {
        // Bidi checking necessary
        // Don't use BIDI_STRONG, VCL must do some checks.
        nLayoutMode &= ~ComplexTextLayoutFlags::BiDiStrong;

        if ( bR2L )
            nLayoutMode |= ComplexTextLayoutFlags::BiDiRtl|ComplexTextLayoutFlags::TextOriginLeft;
    }

    pOutDev->SetLayoutMode( nLayoutMode );

    // #114278# Also setting up digit language from Svt options
    // (cannot reliably inherit the outdev's setting)
    LanguageType eLang = Application::GetSettings().GetLanguageTag().getLanguageType();
    ImplInitDigitMode( pOutDev, eLang );
}

Reference < i18n::XBreakIterator > const & ImpEditEngine::ImplGetBreakIterator() const
{
    if ( !xBI.is() )
    {
        Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
        xBI = i18n::BreakIterator::create( xContext );
    }
    return xBI;
}

Reference < i18n::XExtendedInputSequenceChecker > const & ImpEditEngine::ImplGetInputSequenceChecker() const
{
    if ( !xISC.is() )
    {
        Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
        xISC = i18n::InputSequenceChecker::create( xContext );
    }
    return xISC;
}

Color ImpEditEngine::GetAutoColor() const
{
    Color aColor = GetColorConfig().GetColorValue(svtools::FONTCOLOR).nColor;

    if ( GetBackgroundColor() != COL_AUTO )
    {
        if ( GetBackgroundColor().IsDark() && aColor.IsDark() )
            aColor = COL_WHITE;
        else if ( GetBackgroundColor().IsBright() && aColor.IsBright() )
            aColor = COL_BLACK;
    }

    return aColor;
}

bool ImpEditEngine::ImplCalcAsianCompression(ContentNode* pNode,
                                             TextPortion* pTextPortion, sal_Int32 nStartPos,
                                             long* pDXArray, sal_uInt16 n100thPercentFromMax,
                                             bool bManipulateDXArray)
{
    DBG_ASSERT( GetAsianCompressionMode() != CharCompressType::NONE, "ImplCalcAsianCompression - Why?" );
    DBG_ASSERT( pTextPortion->GetLen(), "ImplCalcAsianCompression - Empty Portion?" );

    // Percent is 1/100 Percent...
    if ( n100thPercentFromMax == 10000 )
        pTextPortion->SetExtraInfos( nullptr );

    bool bCompressed = false;

    if ( GetI18NScriptType( EditPaM( pNode, nStartPos+1 ) ) == i18n::ScriptType::ASIAN )
    {
        long nNewPortionWidth = pTextPortion->GetSize().Width();
        sal_Int32 nPortionLen = pTextPortion->GetLen();
        for ( sal_Int32 n = 0; n < nPortionLen; n++ )
        {
            AsianCompressionFlags nType = GetCharTypeForCompression( pNode->GetChar( n+nStartPos ) );

            bool bCompressPunctuation = ( nType == AsianCompressionFlags::PunctuationLeft ) || ( nType == AsianCompressionFlags::PunctuationRight );
            bool bCompressKana = ( nType == AsianCompressionFlags::Kana ) && ( GetAsianCompressionMode() == CharCompressType::PunctuationAndKana );

            // create Extra infos only if needed...
            if ( bCompressPunctuation || bCompressKana )
            {
                if ( !pTextPortion->GetExtraInfos() )
                {
                    ExtraPortionInfo* pExtraInfos = new ExtraPortionInfo;
                    pTextPortion->SetExtraInfos( pExtraInfos );
                    pExtraInfos->nOrgWidth = pTextPortion->GetSize().Width();
                    pExtraInfos->nAsianCompressionTypes = AsianCompressionFlags::Normal;
                }
                pTextPortion->GetExtraInfos()->nMaxCompression100thPercent = n100thPercentFromMax;
                pTextPortion->GetExtraInfos()->nAsianCompressionTypes |= nType;

                long nOldCharWidth;
                if ( (n+1) < nPortionLen )
                {
                    nOldCharWidth = pDXArray[n];
                }
                else
                {
                    if ( bManipulateDXArray )
                        nOldCharWidth = nNewPortionWidth - pTextPortion->GetExtraInfos()->nPortionOffsetX;
                    else
                        nOldCharWidth = pTextPortion->GetExtraInfos()->nOrgWidth;
                }
                nOldCharWidth -= ( n ? pDXArray[n-1] : 0 );

                long nCompress = 0;

                if ( bCompressPunctuation )
                {
                    nCompress = nOldCharWidth / 2;
                }
                else // Kana
                {
                    nCompress = nOldCharWidth / 10;
                }

                if ( n100thPercentFromMax != 10000 )
                {
                    nCompress *= n100thPercentFromMax;
                    nCompress /= 10000;
                }

                if ( nCompress )
                {
                    bCompressed = true;
                    nNewPortionWidth -= nCompress;
                    pTextPortion->GetExtraInfos()->bCompressed = true;


                    // Special handling for rightpunctuation: For the 'compression' we must
                    // start the output before the normal char position...
                    if ( bManipulateDXArray && ( pTextPortion->GetLen() > 1 ) )
                    {
                        if ( !pTextPortion->GetExtraInfos()->pOrgDXArray )
                            pTextPortion->GetExtraInfos()->SaveOrgDXArray( pDXArray, pTextPortion->GetLen()-1 );

                        if ( nType == AsianCompressionFlags::PunctuationRight )
                        {
                            // If it's the first char, I must handle it in Paint()...
                            if ( n )
                            {
                                // -1: No entry for the last character
                                for ( sal_Int32 i = n-1; i < (nPortionLen-1); i++ )
                                    pDXArray[i] -= nCompress;
                            }
                            else
                            {
                                pTextPortion->GetExtraInfos()->bFirstCharIsRightPunktuation = true;
                                pTextPortion->GetExtraInfos()->nPortionOffsetX = -nCompress;
                            }
                        }
                        else
                        {
                            // -1: No entry for the last character
                            for ( sal_Int32 i = n; i < (nPortionLen-1); i++ )
                                pDXArray[i] -= nCompress;
                        }
                    }
                }
            }
        }

        if ( bCompressed && ( n100thPercentFromMax == 10000 ) )
            pTextPortion->GetExtraInfos()->nWidthFullCompression = nNewPortionWidth;

        pTextPortion->GetSize().setWidth( nNewPortionWidth );

        if ( pTextPortion->GetExtraInfos() && ( n100thPercentFromMax != 10000 ) )
        {
            // Maybe rounding errors in nNewPortionWidth, assure that width not bigger than expected
            long nShrink = pTextPortion->GetExtraInfos()->nOrgWidth - pTextPortion->GetExtraInfos()->nWidthFullCompression;
            nShrink *= n100thPercentFromMax;
            nShrink /= 10000;
            long nNewWidth = pTextPortion->GetExtraInfos()->nOrgWidth - nShrink;
            if ( nNewWidth < pTextPortion->GetSize().Width() )
            pTextPortion->GetSize().setWidth( nNewWidth );
        }
    }
    return bCompressed;
}


void ImpEditEngine::ImplExpandCompressedPortions( EditLine* pLine, ParaPortion* pParaPortion, long nRemainingWidth )
{
    bool bFoundCompressedPortion = false;
    long nCompressed = 0;
    std::vector<TextPortion*> aCompressedPortions;

    sal_Int32 nPortion = pLine->GetEndPortion();
    TextPortion* pTP = &pParaPortion->GetTextPortions()[ nPortion ];
    while ( pTP && ( pTP->GetKind() == PortionKind::TEXT ) )
    {
        if ( pTP->GetExtraInfos() && pTP->GetExtraInfos()->bCompressed )
        {
            bFoundCompressedPortion = true;
            nCompressed += pTP->GetExtraInfos()->nOrgWidth - pTP->GetSize().Width();
            aCompressedPortions.push_back(pTP);
        }
        pTP = ( nPortion > pLine->GetStartPortion() ) ? &pParaPortion->GetTextPortions()[ --nPortion ] : nullptr;
    }

    if ( bFoundCompressedPortion )
    {
        long nCompressPercent = 0;
        if ( nCompressed > nRemainingWidth )
        {
            nCompressPercent = nCompressed - nRemainingWidth;
            DBG_ASSERT( nCompressPercent < 200000, "ImplExpandCompressedPortions - Overflow!" );
            nCompressPercent *= 10000;
            nCompressPercent /= nCompressed;
        }

        for (TextPortion* pTP2 : aCompressedPortions)
        {
            pTP = pTP2;
            pTP->GetExtraInfos()->bCompressed = false;
            pTP->GetSize().setWidth( pTP->GetExtraInfos()->nOrgWidth );
            if ( nCompressPercent )
            {
                sal_Int32 nTxtPortion = pParaPortion->GetTextPortions().GetPos( pTP );
                sal_Int32 nTxtPortionStart = pParaPortion->GetTextPortions().GetStartPos( nTxtPortion );
                DBG_ASSERT( nTxtPortionStart >= pLine->GetStart(), "Portion doesn't belong to the line!!!" );
                long* pDXArray = pLine->GetCharPosArray().data() + (nTxtPortionStart - pLine->GetStart());
                if ( pTP->GetExtraInfos()->pOrgDXArray )
                    memcpy( pDXArray, pTP->GetExtraInfos()->pOrgDXArray.get(), (pTP->GetLen()-1)*sizeof(sal_Int32) );
                ImplCalcAsianCompression( pParaPortion->GetNode(), pTP, nTxtPortionStart, pDXArray, static_cast<sal_uInt16>(nCompressPercent), true );
            }
        }
    }
}

void ImpEditEngine::ImplUpdateOverflowingParaNum(sal_uInt32 nPaperHeight)
{
    sal_uInt32 nY = 0;
    sal_uInt32 nPH;

    for ( sal_Int32 nPara = 0; nPara < GetParaPortions().Count(); nPara++ ) {
        ParaPortion* pPara = GetParaPortions()[nPara];
        nPH = pPara->GetHeight();
        nY += nPH;
        if ( nY > nPaperHeight /*nCurTextHeight*/ ) // found first paragraph overflowing
        {
            mnOverflowingPara = nPara;
            SAL_INFO("editeng.chaining", "[CHAINING] Setting first overflowing #Para#: " << nPara);
            ImplUpdateOverflowingLineNum( nPaperHeight, nPara, nY-nPH);
            return;
        }
    }
}

void ImpEditEngine::ImplUpdateOverflowingLineNum(sal_uInt32 nPaperHeight,
                                             sal_uInt32 nOverflowingPara,
                                             sal_uInt32 nHeightBeforeOverflowingPara)
{
    sal_uInt32 nY = nHeightBeforeOverflowingPara;
    sal_uInt32 nLH;

    ParaPortion *pPara = GetParaPortions()[nOverflowingPara];

    // Like UpdateOverflowingParaNum but for each line in the first
    //  overflowing paragraph.
    for ( sal_Int32 nLine = 0; nLine < pPara->GetLines().Count(); nLine++ ) {
        // XXX: We must use a reference here because the copy constructor resets the height
        EditLine &aLine = pPara->GetLines()[nLine];
        nLH = aLine.GetHeight();
        nY += nLH;

        // Debugging output
        if (nLine == 0) {
            SAL_INFO("editeng.chaining", "[CHAINING] First line has height " << nLH);
        }

        if ( nY > nPaperHeight ) // found first line overflowing
        {
            mnOverflowingLine = nLine;
            SAL_INFO("editeng.chaining", "[CHAINING] Setting first overflowing -Line- to: " << nLine);
            return;
        }
    }

    assert(false && "You should never get here");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
