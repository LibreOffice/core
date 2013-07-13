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


#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/metaact.hxx>
#include <vcl/gdimtf.hxx>

#include <editeng/adjustitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/flditem.hxx>
#include <impedit.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/txtrange.hxx>
#include <editeng/charsetcoloritem.hxx>
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

#include <editeng/forbiddencharacterstable.hxx>

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
#include <comphelper/string.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;

#define CH_HYPH     '-'

#define RESDIFF     10

#define WRONG_SHOW_MIN       5
#define WRONG_SHOW_SMALL    11
#define WRONG_SHOW_MEDIUM   15

struct TabInfo
{
    sal_Bool        bValid;

    SvxTabStop  aTabStop;
    xub_StrLen  nCharPos;
    sal_uInt16      nTabPortion;
    long        nStartPosX;
    long        nTabPos;

    TabInfo() { bValid = sal_False; }
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
    aRotatedPos.X() = (long)   ( nCos*aTranslatedPos.X() + nSin*aTranslatedPos.Y() );
    aRotatedPos.Y() = (long) - ( nSin*aTranslatedPos.X() - nCos*aTranslatedPos.Y() );
    aTranslatedPos = aRotatedPos;

    // Translation...
    aTranslatedPos += rOrigin;
    return aTranslatedPos;
}

sal_uInt8 GetCharTypeForCompression( sal_Unicode cChar )
{
    switch ( cChar )
    {
        case 0x3008: case 0x300A: case 0x300C: case 0x300E:
        case 0x3010: case 0x3014: case 0x3016: case 0x3018:
        case 0x301A: case 0x301D:
        {
            return CHAR_PUNCTUATIONRIGHT;
        }
        case 0x3001: case 0x3002: case 0x3009: case 0x300B:
        case 0x300D: case 0x300F: case 0x3011: case 0x3015:
        case 0x3017: case 0x3019: case 0x301B: case 0x301E:
        case 0x301F:
        {
            return CHAR_PUNCTUATIONLEFT;
        }
        default:
        {
            return ( ( 0x3040 <= cChar ) && ( 0x3100 > cChar ) ) ? CHAR_KANA : CHAR_NORMAL;
        }
    }
}

static void lcl_DrawRedLines(
    OutputDevice* pOutDev,
    long nFontHeight,
    const Point& rPnt,
    sal_uInt16 nIndex,
    sal_uInt16 nMaxEnd,
    const sal_Int32* pDXArray,
    WrongList* pWrongs,
    short nOrientation,
    const Point& rOrigin,
    sal_Bool bVertical,
    sal_Bool bIsRightToLeft )
{
    // But only if font is not too small ...
    long nHght = pOutDev->LogicToPixel( Size( 0, nFontHeight ) ).Height();
    if( WRONG_SHOW_MIN < nHght )
    {
        sal_uInt16 nStyle;
        if( WRONG_SHOW_MEDIUM < nHght )
            nStyle = WAVE_NORMAL;
        else if( WRONG_SHOW_SMALL < nHght )
            nStyle = WAVE_SMALL;
        else
            nStyle = WAVE_FLAT;

        sal_uInt16 nEnd, nStart = nIndex;
        sal_Bool bWrong = pWrongs->NextWrong( nStart, nEnd );
        while ( bWrong )
        {
            if ( nStart >= nMaxEnd )
                break;

            if ( nStart < nIndex )  // Corrected
                nStart = nIndex;
            if ( nEnd > nMaxEnd )
                nEnd = nMaxEnd;
            Point aPnt1( rPnt );
            if ( bVertical && ( nStyle != WAVE_FLAT ) )
            {
                // VCL doesn't know that the text is vertical, and is manipulating
                // the positions a little bit in y direction...
                long nOnePixel = pOutDev->PixelToLogic( Size( 0, 1 ) ).Height();
                long nCorrect = ( nStyle == WAVE_NORMAL ) ? 2*nOnePixel : nOnePixel;
                aPnt1.Y() -= nCorrect;
                aPnt1.X() -= nCorrect;
            }
            if ( nStart > nIndex )
            {
                if ( !bVertical )
                {
                    // since for RTL portions rPnt is on the visual right end of the portion
                    // (i.e. at the start of the first RTL char) we need to subtract the offset
                    // for RTL portions...
                    aPnt1.X() += (bIsRightToLeft ? -1 : 1) * pDXArray[ nStart - nIndex - 1 ];
                }
                else
                    aPnt1.Y() += pDXArray[ nStart - nIndex - 1 ];
            }
            Point aPnt2( rPnt );
            DBG_ASSERT( nEnd > nIndex, "RedLine: aPnt2?" );
            if ( !bVertical )
            {
                // since for RTL portions rPnt is on the visual right end of the portion
                // (i.e. at the start of the first RTL char) we need to subtract the offset
                // for RTL portions...
                aPnt2.X() += (bIsRightToLeft ? -1 : 1) * pDXArray[ nEnd - nIndex - 1 ];
            }
            else
                aPnt2.Y() += pDXArray[ nEnd - nIndex - 1 ];
            if ( nOrientation )
            {
                aPnt1 = Rotate( aPnt1, nOrientation, rOrigin );
                aPnt2 = Rotate( aPnt2, nOrientation, rOrigin );
            }

            pOutDev->DrawWaveLine( aPnt1, aPnt2, nStyle );

            nStart = nEnd+1;
            if ( nEnd < nMaxEnd )
                bWrong = pWrongs->NextWrong( nStart, nEnd );
            else
                bWrong = sal_False;
        }
    }
}

static Point lcl_ImplCalcRotatedPos( Point rPos, Point rOrigin, double nSin, double nCos )
{
    Point aRotatedPos;
    // Translation...
    Point aTranslatedPos( rPos);
    aTranslatedPos -= rOrigin;

    aRotatedPos.X() = (long)   ( nCos*aTranslatedPos.X() + nSin*aTranslatedPos.Y() );
    aRotatedPos.Y() = (long) - ( nSin*aTranslatedPos.X() - nCos*aTranslatedPos.Y() );
    aTranslatedPos = aRotatedPos;
    // Translation...
    aTranslatedPos += rOrigin;

    return aTranslatedPos;
}

static sal_Bool lcl_IsLigature( sal_Unicode cCh, sal_Unicode cNextCh ) // For Kashidas from sw/source/core/text/porlay.txt
{
            // Lam + Alef
    return ( 0x644 == cCh && 0x627 == cNextCh ) ||
            // Beh + Reh
           ( 0x628 == cCh && 0x631 == cNextCh );
}

static sal_Bool lcl_ConnectToPrev( sal_Unicode cCh, sal_Unicode cPrevCh )  // For Kashidas from sw/source/core/text/porlay.txt
{
    // Alef, Dal, Thal, Reh, Zain, and Waw do not connect to the left
    sal_Bool bRet = 0x627 != cPrevCh && 0x62F != cPrevCh && 0x630 != cPrevCh &&
                    0x631 != cPrevCh && 0x632 != cPrevCh && 0x648 != cPrevCh;

    // check for ligatures cPrevChar + cChar
    if ( bRet )
        bRet = ! lcl_IsLigature( cPrevCh, cCh );

    return bRet;
}


//  ----------------------------------------------------------------------
//  class ImpEditEngine
//  ----------------------------------------------------------------------
void ImpEditEngine::UpdateViews( EditView* pCurView )
{
    if ( !GetUpdateMode() || IsFormatting() || aInvalidRect.IsEmpty() )
        return;

    DBG_ASSERT( IsFormatted(), "UpdateViews: Doc not formatted!" );

    for (size_t nView = 0; nView < aEditViews.size(); ++nView)
    {
        EditView* pView = aEditViews[nView];
        DBG_CHKOBJ( pView, EditView, 0 );
        pView->HideCursor();

        Rectangle aClipRect( aInvalidRect );
        Rectangle aVisArea( pView->GetVisArea() );
        aClipRect.Intersection( aVisArea );

        if ( !aClipRect.IsEmpty() )
        {
            // convert to window coordinates ....
            aClipRect = pView->pImpEditView->GetWindowPos( aClipRect );

            if ( pView == pCurView )
                Paint( pView->pImpEditView, aClipRect, 0, sal_True );
            else
                pView->GetWindow()->Invalidate( aClipRect );
        }
    }

    if ( pCurView )
    {
        sal_Bool bGotoCursor = pCurView->pImpEditView->DoAutoScroll();
        pCurView->ShowCursor( bGotoCursor );
    }

    aInvalidRect = Rectangle();
    CallStatusHdl();
}

IMPL_LINK_NOARG(ImpEditEngine, OnlineSpellHdl)
{
    if ( !Application::AnyInput( VCL_INPUT_KEYBOARD ) && GetUpdateMode() && IsFormatted() )
        DoOnlineSpelling();
    else
        aOnlineSpellTimer.Start();

    return 0;
}

IMPL_LINK_NOARG_INLINE_START(ImpEditEngine, IdleFormatHdl)
{
    aIdleFormatter.ResetRestarts();

    // #i97146# check if that view is still available
    // else probably the idle format timer fired while we're already
    // downing
    EditView* pView = aIdleFormatter.GetView();
    for (size_t nView = 0; nView < aEditViews.size(); ++nView)
    {
        if( aEditViews[nView] == pView )
        {
            FormatAndUpdate( pView );
            break;
        }
    }
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(ImpEditEngine, IdleFormatHdl)

void ImpEditEngine::CheckIdleFormatter()
{
    aIdleFormatter.ForceTimeout();
    // If not idle, but still not formatted:
    if ( !IsFormatted() )
        FormatDoc();
}

void ImpEditEngine::FormatFullDoc()
{
    for ( sal_Int32 nPortion = 0; nPortion < GetParaPortions().Count(); nPortion++ )
        GetParaPortions()[nPortion]->MarkSelectionInvalid( 0, GetParaPortions()[nPortion]->GetNode()->Len() );
    FormatDoc();
}

void ImpEditEngine::FormatDoc()
{
    if ( !GetUpdateMode() || IsFormatting() )
        return;

    EnterBlockNotifications();

    bIsFormatting = true;

    // Then I can also start the spell-timer ...
    if ( GetStatus().DoOnlineSpelling() )
        StartOnlineSpellTimer();

    long nY = 0;
    sal_Bool bGrow = sal_False;

    Font aOldFont( GetRefDevice()->GetFont() );

    // Here already, so that not always in CreateLines...
    sal_Bool bMapChanged = ImpCheckRefMapMode();

    aInvalidRect = Rectangle();  // make empty
    for ( sal_Int32 nPara = 0; nPara < GetParaPortions().Count(); nPara++ )
    {
        ParaPortion* pParaPortion = GetParaPortions()[nPara];
        if ( pParaPortion->MustRepaint() || ( pParaPortion->IsInvalid() && pParaPortion->IsVisible() ) )
        {
            if ( pParaPortion->IsInvalid() )
            {
                sal_Bool bChangedByDerivedClass = GetEditEnginePtr()->FormattingParagraph( nPara );
                if ( bChangedByDerivedClass )
                {
                    pParaPortion->GetTextPortions().Reset();
                    pParaPortion->MarkSelectionInvalid( 0, pParaPortion->GetNode()->Len() );
                }
            }
            // No formatting should be necessary for MustRepaint()!
            if ( ( pParaPortion->MustRepaint() && !pParaPortion->IsInvalid() )
                    || CreateLines( nPara, nY ) )
            {
                if ( !bGrow && GetTextRanger() )
                {
                    // For a change in height all below must be reformatted ...
                    for ( sal_Int32 n = nPara+1; n < GetParaPortions().Count(); n++ )
                    {
                        ParaPortion* pPP = GetParaPortions()[n];
                        pPP->MarkSelectionInvalid( 0, pPP->GetNode()->Len() );
                        pPP->GetLines().Reset();
                    }
                }
                bGrow = sal_True;
                if ( IsCallParaInsertedOrDeleted() )
                    GetEditEnginePtr()->ParagraphHeightChanged( nPara );
                pParaPortion->SetMustRepaint( sal_False );
            }

            // InvalidRect set only once...
            if ( aInvalidRect.IsEmpty() )
            {
                // For Paperwidth 0 (AutoPageSize) it would otherwise be Empty()...
                long nWidth = std::max( (long)1, ( !IsVertical() ? aPaperSize.Width() : aPaperSize.Height() ) );
                Range aInvRange( GetInvalidYOffsets( pParaPortion ) );
                aInvalidRect = Rectangle( Point( 0, nY+aInvRange.Min() ),
                    Size( nWidth, aInvRange.Len() ) );
            }
            else
            {
                aInvalidRect.Bottom() = nY + pParaPortion->GetHeight();
            }
        }
        else if ( bGrow )
        {
            aInvalidRect.Bottom() = nY + pParaPortion->GetHeight();
        }
        nY += pParaPortion->GetHeight();
    }

    // One can also get into the formatting through UpdateMode ON=>OFF=>ON...
    // enable optimization first after Vobis delivery ...
    {
        sal_uInt32 nNewHeightNTP;
        sal_uInt32 nNewHeight = CalcTextHeight( &nNewHeightNTP );
        long nDiff = nNewHeight - nCurTextHeight;
        if ( nDiff )
            aStatus.GetStatusWord() |= !IsVertical() ? EE_STAT_TEXTHEIGHTCHANGED : EE_STAT_TEXTWIDTHCHANGED;
        if ( nNewHeight < nCurTextHeight )
        {
            aInvalidRect.Bottom() = (long)std::max( nNewHeight, nCurTextHeight );
            if ( aInvalidRect.IsEmpty() )
            {
                aInvalidRect.Top() = 0;
                // Left and Right are not evaluated, are however set due to IsEmpty.
                aInvalidRect.Left() = 0;
                aInvalidRect.Right() = !IsVertical() ? aPaperSize.Width() : aPaperSize.Height();
            }
        }

        nCurTextHeight = nNewHeight;
        nCurTextHeightNTP = nNewHeightNTP;

        if ( aStatus.AutoPageSize() )
            CheckAutoPageSize();
        else if ( nDiff )
        {
            for (size_t nView = 0; nView < aEditViews.size(); ++nView)
            {
                EditView* pView = aEditViews[nView];
                ImpEditView* pImpView = pView->pImpEditView;
                if ( pImpView->DoAutoHeight() )
                {
                    Size aSz( pImpView->GetOutputArea().GetWidth(), nCurTextHeight );
                    if ( aSz.Height() > aMaxAutoPaperSize.Height() )
                        aSz.Height() = aMaxAutoPaperSize.Height();
                    else if ( aSz.Height() < aMinAutoPaperSize.Height() )
                        aSz.Height() = aMinAutoPaperSize.Height();
                    pImpView->ResetOutputArea( Rectangle(
                        pImpView->GetOutputArea().TopLeft(), aSz ) );
                }
            }
        }
    }

    if ( aStatus.DoRestoreFont() )
        GetRefDevice()->SetFont( aOldFont );
    bIsFormatting = false;
    bFormatted = true;

    if ( bMapChanged )
        GetRefDevice()->Pop();

    CallStatusHdl();    // If Modified...

    LeaveBlockNotifications();
}

sal_Bool ImpEditEngine::ImpCheckRefMapMode()
{
    sal_Bool bChange = sal_False;

    if ( aStatus.DoFormat100() )
    {
        MapMode aMapMode( GetRefDevice()->GetMapMode() );
        if ( aMapMode.GetScaleX().GetNumerator() != aMapMode.GetScaleX().GetDenominator() )
            bChange = sal_True;
        else if ( aMapMode.GetScaleY().GetNumerator() != aMapMode.GetScaleY().GetDenominator() )
            bChange = sal_True;

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
        aPaperSize.Width() = (long) !IsVertical() ? CalcTextWidth( sal_True ) : GetTextHeight();
    if ( GetStatus().AutoPageHeight() )
        aPaperSize.Height() = (long) !IsVertical() ? GetTextHeight() : CalcTextWidth( sal_True );

    SetValidPaperSize( aPaperSize );    // consider Min, Max

    if ( aPaperSize != aPrevPaperSize )
    {
        if ( ( !IsVertical() && ( aPaperSize.Width() != aPrevPaperSize.Width() ) )
             || ( IsVertical() && ( aPaperSize.Height() != aPrevPaperSize.Height() ) ) )
        {
            // If ahead is centered / right or tabs ...
            aStatus.GetStatusWord() |= !IsVertical() ? EE_STAT_TEXTWIDTHCHANGED : EE_STAT_TEXTHEIGHTCHANGED;
            for ( sal_Int32 nPara = 0; nPara < GetParaPortions().Count(); nPara++ )
            {
                // Only paragraphs which are not aligned to the left need to be
                // reformatted, the height can not be changed here anymore.
                ParaPortion* pParaPortion = GetParaPortions()[nPara];
                ContentNode* pNode = pParaPortion->GetNode();
                SvxAdjust eJustification = GetJustification( nPara );
                if ( eJustification != SVX_ADJUST_LEFT )
                {
                    pParaPortion->MarkSelectionInvalid( 0, pNode->Len() );
                    CreateLines( nPara, 0 );  // 0: For AutoPageSize no TextRange!
                }
            }
        }

        Size aInvSize = aPaperSize;
        if ( aPaperSize.Width() < aPrevPaperSize.Width() )
            aInvSize.Width() = aPrevPaperSize.Width();
        if ( aPaperSize.Height() < aPrevPaperSize.Height() )
            aInvSize.Height() = aPrevPaperSize.Height();

        Size aSz( aInvSize );
        if ( IsVertical() )
        {
            aSz.Width() = aInvSize.Height();
            aSz.Height() = aInvSize.Width();
        }
        aInvalidRect = Rectangle( Point(), aSz );


        for (size_t nView = 0; nView < aEditViews.size(); ++nView)
        {
            EditView* pView = aEditViews[nView];
            pView->pImpEditView->RecalcOutputArea();
        }
    }
}

static sal_Int32 ImplCalculateFontIndependentLineSpacing( const sal_Int32 nFontHeight )
{
    return ( nFontHeight * 12 ) / 10;   // + 20%
}

sal_Bool ImpEditEngine::CreateLines( sal_Int32 nPara, sal_uInt32 nStartPosY )
{
    ParaPortion* pParaPortion = GetParaPortions()[nPara];

    // sal_Bool: Changes in the height of paragraph Yes / No - sal_True/sal_False
    DBG_ASSERT( pParaPortion->GetNode(), "Portion without Node in CreateLines" );
    DBG_ASSERT( pParaPortion->IsVisible(), "Invisible paragraphs not formatted!" );
    DBG_ASSERT( pParaPortion->IsInvalid(), "CreateLines: Portion not invalid!" );

    sal_Bool bProcessingEmptyLine = ( pParaPortion->GetNode()->Len() == 0 );
    sal_Bool bEmptyNodeWithPolygon = ( pParaPortion->GetNode()->Len() == 0 ) && GetTextRanger();

    // ---------------------------------------------------------------
    // Fast special treatment for empty paragraphs ...
    // ---------------------------------------------------------------
    if ( ( pParaPortion->GetNode()->Len() == 0 ) && !GetTextRanger() )
    {
        // fast special treatment ...
        if ( pParaPortion->GetTextPortions().Count() )
            pParaPortion->GetTextPortions().Reset();
        if ( pParaPortion->GetLines().Count() )
            pParaPortion->GetLines().Reset();
        CreateAndInsertEmptyLine( pParaPortion, nStartPosY );
        return FinishCreateLines( pParaPortion );
    }

    // ---------------------------------------------------------------
    // Initialization ......
    // ---------------------------------------------------------------

    // Always format for 100%:
    sal_Bool bMapChanged = ImpCheckRefMapMode();

    if ( pParaPortion->GetLines().Count() == 0 )
    {
        EditLine* pL = new EditLine;
        pParaPortion->GetLines().Append(pL);
    }

    // ---------------------------------------------------------------
    // Get Paragraph attributes  ......
    // ---------------------------------------------------------------
    ContentNode* const pNode = pParaPortion->GetNode();

    sal_Bool bRightToLeftPara = IsRightToLeft( nPara );

    SvxAdjust eJustification = GetJustification( nPara );
    sal_Bool bHyphenatePara = ((const SfxBoolItem&)pNode->GetContentAttribs().GetItem( EE_PARA_HYPHENATE )).GetValue();
    sal_Int32 nSpaceBefore      = 0;
    sal_Int32 nMinLabelWidth    = 0;
    sal_Int32 nSpaceBeforeAndMinLabelWidth = GetSpaceBeforeAndMinLabelWidth( pNode, &nSpaceBefore, &nMinLabelWidth );
    const SvxLRSpaceItem& rLRItem = GetLRSpaceItem( pNode );
    const SvxLineSpacingItem& rLSItem = (const SvxLineSpacingItem&) pNode->GetContentAttribs().GetItem( EE_PARA_SBL );
    const sal_Bool bScriptSpace = ((const SvxScriptSpaceItem&) pNode->GetContentAttribs().GetItem( EE_PARA_ASIANCJKSPACING )).GetValue();

    const short nInvalidDiff = pParaPortion->GetInvalidDiff();
    const sal_uInt16 nInvalidStart = pParaPortion->GetInvalidPosStart();
    const sal_uInt16 nInvalidEnd =  nInvalidStart + std::abs( nInvalidDiff );

    sal_Bool bQuickFormat = sal_False;
    if ( !bEmptyNodeWithPolygon && !HasScriptType( nPara, i18n::ScriptType::COMPLEX ) )
    {
        if ( ( pParaPortion->IsSimpleInvalid() ) && ( nInvalidDiff > 0 ) &&
             ( pNode->GetString().Search( CH_FEATURE, nInvalidStart ) > nInvalidEnd ) )
        {
            bQuickFormat = sal_True;
        }
        else if ( ( pParaPortion->IsSimpleInvalid() ) && ( nInvalidDiff < 0 ) )
        {
            // check if delete over the portion boundaries was done ...
            sal_uInt16 nStart = nInvalidStart;  // DOUBLE !!!!!!!!!!!!!!!
            sal_uInt16 nEnd = nStart - nInvalidDiff;  // negative
            bQuickFormat = sal_True;
            sal_uInt16 nPos = 0;
            sal_uInt16 nPortions = pParaPortion->GetTextPortions().Count();
            for ( sal_uInt16 nTP = 0; nTP < nPortions; nTP++ )
            {
                // There must be no start / end in the deleted area.
                TextPortion* const pTP = pParaPortion->GetTextPortions()[ nTP ];
                nPos = nPos + pTP->GetLen();
                if ( ( nPos > nStart ) && ( nPos < nEnd ) )
                {
                    bQuickFormat = sal_False;
                    break;
                }
            }
        }
    }

    // SW disables TEXT_LAYOUT_COMPLEX_DISABLED, so maybe I have to enable it...

    // Saving both layout mode and language (since I'm potentially changing both)
    GetRefDevice()->Push( PUSH_TEXTLAYOUTMODE|PUSH_TEXTLANGUAGE );

    ImplInitLayoutMode( GetRefDevice(), nPara, 0xFFFF );

    sal_uInt16 nRealInvalidStart = nInvalidStart;

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
    else    // nRealInvalidStart can be before InvalidStart, since Portions were deleted....
    {
        CreateTextPortions( pParaPortion, nRealInvalidStart );
    }


    // ---------------------------------------------------------------
    // Search for line with InvalidPos, start one line before
    // Flag the line => do not remove it !
    // ---------------------------------------------------------------

    sal_uInt16 nLine = pParaPortion->GetLines().Count()-1;
    for ( sal_uInt16 nL = 0; nL <= nLine; nL++ )
    {
        EditLine* pLine = pParaPortion->GetLines()[nL];
        if ( pLine->GetEnd() > nRealInvalidStart )  // not nInvalidStart!
        {
            nLine = nL;
            break;
        }
        pLine->SetValid();
    }
    // Begin one line before...
    // If it is typed at the end, the line in front cannot change.
    if ( nLine && ( !pParaPortion->IsSimpleInvalid() || ( nInvalidEnd < pNode->Len() ) || ( nInvalidDiff <= 0 ) ) )
        nLine--;

    EditLine* pLine = pParaPortion->GetLines()[nLine];

    static Rectangle aZeroArea = Rectangle( Point(), Point() );
    Rectangle aBulletArea( aZeroArea );
    if ( !nLine )
    {
        aBulletArea = GetEditEnginePtr()->GetBulletArea( GetParaPortions().GetPos( pParaPortion ) );
        if ( aBulletArea.Right() > 0 )
            pParaPortion->SetBulletX( (sal_uInt16) GetXValue( aBulletArea.Right() ) );
        else
            pParaPortion->SetBulletX( 0 ); // if Bullet is set incorrectly
    }

    // ---------------------------------------------------------------
    // Reformat all lines from here ...
    // ---------------------------------------------------------------
    sal_uInt16 nDelFromLine = 0xFFFF;
    sal_Bool bLineBreak = sal_False;

    sal_uInt16 nIndex = pLine->GetStart();
    EditLine aSaveLine( *pLine );
    SvxFont aTmpFont( pNode->GetCharAttribs().GetDefFont() );

    sal_Bool bCalcCharPositions = sal_True;
    sal_Int32* pBuf = new sal_Int32[ pNode->Len() ];

    sal_Bool bSameLineAgain = sal_False;    // For TextRanger, if the height changes.
    TabInfo aCurrentTab;

    sal_Bool bForceOneRun = bEmptyNodeWithPolygon;
    sal_Bool bCompressedChars = sal_False;

    while ( ( nIndex < pNode->Len() ) || bForceOneRun )
    {
        bForceOneRun = sal_False;

        sal_Bool bEOL = sal_False;
        sal_Bool bEOC = sal_False;
        sal_uInt16 nPortionStart = 0;
        sal_uInt16 nPortionEnd = 0;

        long nStartX = GetXValue( rLRItem.GetTxtLeft() + nSpaceBeforeAndMinLabelWidth );
        if ( nIndex == 0 )
        {
            long nFI = GetXValue( rLRItem.GetTxtFirstLineOfst() );
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
     // the positions unfortunately have to be redefined ...
        // Solution:
        // The line before can only become longer, not smaller
        // => ...
        if ( bCalcCharPositions )
            pLine->GetCharPosArray().clear();

        sal_uInt16 nTmpPos = nIndex;
        sal_uInt16 nTmpPortion = pLine->GetStartPortion();
        long nTmpWidth = 0;
        long nXWidth = nMaxLineWidth;
        if ( nXWidth <= nTmpWidth ) // while has to be looped once
            nXWidth = nTmpWidth+1;

        LongDqPtr pTextRanges = 0;
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
                    nTextLineHeight = ImplCalculateFontIndependentLineSpacing( aTmpFont.GetHeight() );
                else
                    nTextLineHeight = aTmpFont.GetPhysTxtSize( GetRefDevice(), String() ).Height();
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
                // Use the widest range ...
                // The widest range could be a bit confusing, so normally it
                // is the first one. Best with gaps.
                if ( pTextRanges->size() )
                {
                    sal_uInt16 n = 0;
                    long nA = pTextRanges->at(n++);
                    long nB = pTextRanges->at(n++);
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
                    nTextExtraYOffset += std::max( (long)(nTextLineHeight / 10), (long)1 );
                    if ( ( nTextY + nTextExtraYOffset  ) > GetTextRanger()->GetBoundRect().Bottom() )
                    {
                        nXWidth = !IsVertical() ? GetPaperSize().Width() : GetPaperSize().Height();
                        if ( !nXWidth ) // AutoPaperSize
                            nXWidth = 0x7FFFFFFF;
                    }
                }
            }
        }

        // search for Portion that no longer fits in line ....
        TextPortion* pPortion = 0;
        sal_Bool bBrokenLine = sal_False;
        bLineBreak = sal_False;
        const EditCharAttrib* pNextFeature = pNode->GetCharAttribs().FindFeature( pLine->GetStart() );
        while ( ( nTmpWidth < nXWidth ) && !bEOL && ( nTmpPortion < pParaPortion->GetTextPortions().Count() ) )
        {
            nPortionStart = nTmpPos;
            pPortion = pParaPortion->GetTextPortions()[nTmpPortion];
            if ( pPortion->GetKind() == PORTIONKIND_HYPHENATOR )
            {
                // Throw away a Portion, if necessary correct the one before,
                // if the Hyph portion has swallowed a character ...
                sal_uInt16 nTmpLen = pPortion->GetLen();
                pParaPortion->GetTextPortions().Remove( nTmpPortion );
                if (nTmpPortion && nTmpLen)
                {
                    nTmpPortion--;
                    TextPortion* pPrev = pParaPortion->GetTextPortions()[nTmpPortion];
                    DBG_ASSERT( pPrev->GetKind() == PORTIONKIND_TEXT, "Portion?!" );
                    nTmpWidth -= pPrev->GetSize().Width();
                    nTmpPos = nTmpPos - pPrev->GetLen();
                    pPrev->SetLen(pPrev->GetLen() + nTmpLen);
                    pPrev->GetSize().Width() = (-1);
                }

                DBG_ASSERT( nTmpPortion < pParaPortion->GetTextPortions().Count(), "No more Portions left!" );
                pPortion = pParaPortion->GetTextPortions()[nTmpPortion];
            }
            DBG_ASSERT( pPortion->GetKind() != PORTIONKIND_HYPHENATOR, "CreateLines: Hyphenator-Portion!" );
            DBG_ASSERT( pPortion->GetLen() || bProcessingEmptyLine, "Empty Portion in CreateLines ?!" );
            (void)bProcessingEmptyLine;
            if ( pNextFeature && ( pNextFeature->GetStart() == nTmpPos ) )
            {
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

                        short nAllSpaceBeforeText = static_cast< short >(rLRItem.GetTxtLeft()/* + rLRItem.GetTxtLeft()*/ + nSpaceBeforeAndMinLabelWidth);
                        aCurrentTab.aTabStop = pNode->GetContentAttribs().FindTabStop( nCurPos - nAllSpaceBeforeText /*rLRItem.GetTxtLeft()*/, aEditDoc.GetDefTab() );
                        aCurrentTab.nTabPos = GetXValue( (long) ( aCurrentTab.aTabStop.GetTabPos() + nAllSpaceBeforeText /*rLRItem.GetTxtLeft()*/ ) );
                        aCurrentTab.bValid = sal_False;

                        // Switch direction in R2L para...
                        if ( bRightToLeftPara )
                        {
                            if ( aCurrentTab.aTabStop.GetAdjustment() == SVX_TAB_ADJUST_RIGHT )
                                aCurrentTab.aTabStop.GetAdjustment() = SVX_TAB_ADJUST_LEFT;
                            else if ( aCurrentTab.aTabStop.GetAdjustment() == SVX_TAB_ADJUST_LEFT )
                                aCurrentTab.aTabStop.GetAdjustment() = SVX_TAB_ADJUST_RIGHT;
                        }

                        if ( ( aCurrentTab.aTabStop.GetAdjustment() == SVX_TAB_ADJUST_RIGHT ) ||
                             ( aCurrentTab.aTabStop.GetAdjustment() == SVX_TAB_ADJUST_CENTER ) ||
                             ( aCurrentTab.aTabStop.GetAdjustment() == SVX_TAB_ADJUST_DECIMAL ) )
                        {
                            // For LEFT / DEFAULT this tab is not considered.
                            aCurrentTab.bValid = sal_True;
                            aCurrentTab.nStartPosX = nTmpWidth;
                            aCurrentTab.nCharPos = nTmpPos;
                            aCurrentTab.nTabPortion = nTmpPortion;
                        }

                        pPortion->GetKind() = PORTIONKIND_TAB;
                        pPortion->SetExtraValue( aCurrentTab.aTabStop.GetFill() );
                        pPortion->GetSize().Width() = aCurrentTab.nTabPos - (nTmpWidth+nStartX);

                        // Height needed...
                        SeekCursor( pNode, nTmpPos+1, aTmpFont );
                        pPortion->GetSize().Height() = aTmpFont.QuickGetTextSize( GetRefDevice(), String(), 0, 0, NULL ).Height();

                        DBG_ASSERT( pPortion->GetSize().Width() >= 0, "Tab incorrectly calculated!" );

                        nTmpWidth = aCurrentTab.nTabPos-nStartX;

                        // If this is the first token on the line,
                        // and nTmpWidth > aPaperSize.Width, => infinite loop!
                        if ( ( nTmpWidth >= nXWidth ) && ( nTmpPortion == pLine->GetStartPortion() ) )
                        {
                            // What now?
                            // make the tab fitting
                            pPortion->GetSize().Width() = nXWidth-nOldTmpWidth;
                            nTmpWidth = nXWidth-1;
                            bEOL = sal_True;
                            bBrokenLine = sal_True;
                        }
                        EditLine::CharPosArrayType& rArray = pLine->GetCharPosArray();
                        size_t nPos = nTmpPos - pLine->GetStart();
                        rArray.insert(rArray.begin()+nPos, pPortion->GetSize().Width());
                        bCompressedChars = sal_False;
                    }
                    break;
                    case EE_FEATURE_LINEBR:
                    {
                        DBG_ASSERT( pPortion, "?!" );
                        pPortion->GetSize().Width() = 0;
                        bEOL = sal_True;
                        bLineBreak = sal_True;
                        pPortion->GetKind() = PORTIONKIND_LINEBREAK;
                        bCompressedChars = sal_False;
                        EditLine::CharPosArrayType& rArray = pLine->GetCharPosArray();
                        size_t nPos = nTmpPos - pLine->GetStart();
                        rArray.insert(rArray.begin()+nPos, pPortion->GetSize().Width());
                    }
                    break;
                    case EE_FEATURE_FIELD:
                    {
                        SeekCursor( pNode, nTmpPos+1, aTmpFont );
                        sal_Unicode cChar = 0;  // later: NBS?
                        aTmpFont.SetPhysFont( GetRefDevice() );
                        ImplInitDigitMode(GetRefDevice(), aTmpFont.GetLanguage());

                        OUString aFieldValue = cChar ? OUString(cChar) : ((EditCharAttribField*)pNextFeature)->GetFieldValue();
                        if ( bCalcCharPositions || !pPortion->HasValidSize() )
                        {
                            pPortion->GetSize() = aTmpFont.QuickGetTextSize( GetRefDevice(), aFieldValue, 0, aFieldValue.getLength(), 0 );
                            // So no scrolling for oversized fields
                            if ( pPortion->GetSize().Width() > nXWidth )
                            {
                                sal_Int32 nWidthOrg         = pPortion->GetSize().Width();
                                sal_Int32 nChars            = aFieldValue.getLength();
                                sal_Int32 nApproxWC         = nXWidth / ( nWidthOrg / nChars );
                                ExtraPortionInfo *pExtraInfo= pPortion->GetExtraInfos();
                                if( !nApproxWC ) nApproxWC++;
                                if( pExtraInfo == NULL )
                                {
                                    pExtraInfo = new ExtraPortionInfo();
                                    pExtraInfo->nOrgWidth = nXWidth;
                                    pPortion->SetExtraInfos( pExtraInfo );
                                }
                                else
                                {
                                    pExtraInfo->lineBreaksList.clear();
                                }

                                pPortion->GetSize().Width() = nXWidth;

                                while( nChars > 0 )
                                {
                                    pExtraInfo->lineBreaksList.push_back( aFieldValue.getLength() - nChars );
                                    nChars -= nApproxWC;
                                }
                            }
                        }
                        nTmpWidth += pPortion->GetSize().Width();
                        EditLine::CharPosArrayType& rArray = pLine->GetCharPosArray();
                        size_t nPos = nTmpPos - pLine->GetStart();
                        rArray.insert(rArray.begin()+nPos, pPortion->GetSize().Width());
                        pPortion->GetKind() = cChar ? PORTIONKIND_TEXT : PORTIONKIND_FIELD;
                        // If this is the first token on the line,
                        // and nTmpWidth > aPaperSize.Width, => infinite loop!
                        if ( ( nTmpWidth >= nXWidth ) && ( nTmpPortion == pLine->GetStartPortion() ) )
                        {
                            nTmpWidth = nXWidth-1;
                            bEOL = sal_True;
                            bBrokenLine = sal_True;
                        }
                        // Compression in Fields????
                        // I think this could be a little bit difficult and is not very useful
                        bCompressedChars = sal_False;
                    }
                    break;
                    default:    OSL_FAIL( "What feature?" );
                }
                pNextFeature = pNode->GetCharAttribs().FindFeature( pNextFeature->GetStart() + 1  );
            }
            else
            {
                DBG_ASSERT( pPortion->GetLen() || bProcessingEmptyLine, "Empty Portion - Extra Space?!" );
                (void)bProcessingEmptyLine;
                SeekCursor( pNode, nTmpPos+1, aTmpFont );
                aTmpFont.SetPhysFont( GetRefDevice() );
                ImplInitDigitMode(GetRefDevice(), aTmpFont.GetLanguage());

                if ( bCalcCharPositions || !pPortion->HasValidSize() )
                {
                    pPortion->GetSize() = aTmpFont.QuickGetTextSize( GetRefDevice(), pParaPortion->GetNode()->GetString(), nTmpPos, pPortion->GetLen(), pBuf );

                    // #i9050# Do Kerning also behind portions...
                    if ( ( aTmpFont.GetFixKerning() > 0 ) && ( ( nTmpPos + pPortion->GetLen() ) < pNode->Len() ) )
                        pPortion->GetSize().Width() += aTmpFont.GetFixKerning();
                    if ( IsFixedCellHeight() )
                        pPortion->GetSize().Height() = ImplCalculateFontIndependentLineSpacing( aTmpFont.GetHeight() );
                }
                if ( bCalcCharPositions )
                {
                    sal_uInt16 nLen = pPortion->GetLen();
                    // The array is  generally flattened at the beginning
                    // => Always simply quick inserts.
                    size_t nPos = nTmpPos - pLine->GetStart();
                    EditLine::CharPosArrayType& rArray = pLine->GetCharPosArray();
                    rArray.insert(rArray.begin()+nPos, pBuf, pBuf+nLen);
                }

                // And now check for Compression:
                if ( pPortion->GetLen() && GetAsianCompressionMode() )
                {
                    EditLine::CharPosArrayType& rArray = pLine->GetCharPosArray();
                    sal_Int32* pDXArray = &rArray[0] + nTmpPos - pLine->GetStart();
                    bCompressedChars |= ImplCalcAsianCompression(
                        pNode, pPortion, nTmpPos, pDXArray, 10000, false);
                }

                nTmpWidth += pPortion->GetSize().Width();

                pPortion->SetRightToLeft( GetRightToLeft( nPara, nTmpPos+1 ) );

                sal_uInt16 _nPortionEnd = nTmpPos + pPortion->GetLen();
                if( bScriptSpace && ( _nPortionEnd < pNode->Len() ) && ( nTmpWidth < nXWidth ) && IsScriptChange( EditPaM( pNode, _nPortionEnd ) ) )
                {
                    sal_Bool bAllow = sal_False;
                    sal_uInt16 nScriptTypeLeft = GetScriptType( EditPaM( pNode, _nPortionEnd ) );
                    sal_uInt16 nScriptTypeRight = GetScriptType( EditPaM( pNode, _nPortionEnd+1 ) );
                    if ( ( nScriptTypeLeft == i18n::ScriptType::ASIAN ) || ( nScriptTypeRight == i18n::ScriptType::ASIAN ) )
                        bAllow = sal_True;

                    // No spacing within L2R/R2L nesting
                    if ( bAllow )
                    {
                        long nExtraSpace = pPortion->GetSize().Height()/5;
                        nExtraSpace = GetXValue( nExtraSpace );
                        pPortion->GetSize().Width() += nExtraSpace;
                        nTmpWidth += nExtraSpace;
                    }
                }
            }

            if ( aCurrentTab.bValid && ( nTmpPortion != aCurrentTab.nTabPortion ) )
            {
                long nWidthAfterTab = 0;
                for ( sal_uInt16 n = aCurrentTab.nTabPortion+1; n <= nTmpPortion; n++  )
                {
                    const TextPortion* pTP = pParaPortion->GetTextPortions()[n];
                    nWidthAfterTab += pTP->GetSize().Width();
                }
                long nW = nWidthAfterTab;   // Length before tab position
                if ( aCurrentTab.aTabStop.GetAdjustment() == SVX_TAB_ADJUST_RIGHT )
                {
                }
                else if ( aCurrentTab.aTabStop.GetAdjustment() == SVX_TAB_ADJUST_CENTER )
                {
                    nW = nWidthAfterTab/2;
                }
                else if ( aCurrentTab.aTabStop.GetAdjustment() == SVX_TAB_ADJUST_DECIMAL )
                {
                    String aText = GetSelected( EditSelection(  EditPaM( pParaPortion->GetNode(), nTmpPos ),
                                                                EditPaM( pParaPortion->GetNode(), nTmpPos + pPortion->GetLen() ) ) );
                    sal_uInt16 nDecPos = aText.Search( aCurrentTab.aTabStop.GetDecimal() );
                    if ( nDecPos != STRING_NOTFOUND )
                    {
                        nW -= pParaPortion->GetTextPortions()[nTmpPortion]->GetSize().Width();
                        nW += aTmpFont.QuickGetTextSize( GetRefDevice(), pParaPortion->GetNode()->GetString(), nTmpPos, nDecPos, NULL ).Width();
                        aCurrentTab.bValid = sal_False;
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
                    aCurrentTab.bValid = sal_False;
                }
                TextPortion* const pTabPortion = pParaPortion->GetTextPortions()[aCurrentTab.nTabPortion];
                pTabPortion->GetSize().Width() = aCurrentTab.nTabPos - aCurrentTab.nStartPosX - nW - nStartX;
                nTmpWidth = aCurrentTab.nStartPosX + pTabPortion->GetSize().Width() + nWidthAfterTab;
            }

            nTmpPos = nTmpPos + pPortion->GetLen();
            nPortionEnd = nTmpPos;
            nTmpPortion++;
            if ( aStatus.OneCharPerLine() )
                bEOL = sal_True;
        }

        DBG_ASSERT( pPortion, "no portion!?" );

        aCurrentTab.bValid = sal_False;

        // this was possibly a portion too far:
        sal_Bool bFixedEnd = sal_False;
        if ( aStatus.OneCharPerLine() )
        {
            // State before Portion (apart from nTmpWidth):
            nPortionEnd = nTmpPos;
            nTmpPos -= pPortion ? pPortion->GetLen() : 0;
            nPortionStart = nTmpPos;
            nTmpPortion--;

            bEOL = sal_True;
            bEOC = sal_False;

            // And now just one character:
            nTmpPos++;
            nTmpPortion++;
            nPortionEnd = nTmpPortion;
            // one Non-Feature-Portion has to be wrapped
            if ( pPortion->GetLen() > 1 )
            {
                DBG_ASSERT( pPortion && (pPortion->GetKind() == PORTIONKIND_TEXT), "Len>1, but no TextPortion?" );
                nTmpWidth -= pPortion ? pPortion->GetSize().Width() : 0;
                sal_uInt16 nP = SplitTextPortion( pParaPortion, nTmpPos, pLine );
                const TextPortion* p = pParaPortion->GetTextPortions()[nP];
                DBG_ASSERT( p, "Portion ?!" );
                nTmpWidth += p->GetSize().Width();
            }
        }
        else if ( nTmpWidth >= nXWidth )
        {
            nPortionEnd = nTmpPos;
            nTmpPos -= pPortion ? pPortion->GetLen() : 0;
            nPortionStart = nTmpPos;
            nTmpPortion--;
            bEOL = sal_False;
            bEOC = sal_False;
            if( pPortion ) switch ( pPortion->GetKind() )
            {
                case PORTIONKIND_TEXT:
                {
                    nTmpWidth -= pPortion->GetSize().Width();
                }
                break;
                case PORTIONKIND_FIELD:
                case PORTIONKIND_TAB:
                {
                    nTmpWidth -= pPortion->GetSize().Width();
                    bEOL = sal_True;
                    bFixedEnd = sal_True;
                }
                break;
                default:
                {
                    //  A feature is not wrapped:
                    DBG_ASSERT( ( pPortion->GetKind() == PORTIONKIND_LINEBREAK ), "What Feature ?" );
                    bEOL = sal_True;
                    bFixedEnd = sal_True;
                }
            }
        }
        else
        {
            bEOL = sal_True;
            bEOC = sal_True;
            pLine->SetEnd( nPortionEnd );
            DBG_ASSERT( pParaPortion->GetTextPortions().Count(), "No TextPortions?" );
            pLine->SetEndPortion( (sal_uInt16)pParaPortion->GetTextPortions().Count() - 1 );
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
            bEOC = sal_False; // was set above, maybe change the sequence of the if's?
        }
        else if ( !bEOL )
        {
            DBG_ASSERT( pPortion && ((nPortionEnd-nPortionStart) == pPortion->GetLen()), "However, another portion?!" );
            long nRemainingWidth = nMaxLineWidth - nTmpWidth;
            sal_Bool bCanHyphenate = ( aTmpFont.GetCharSet() != RTL_TEXTENCODING_SYMBOL );
            if ( bCompressedChars && pPortion && ( pPortion->GetLen() > 1 ) && pPortion->GetExtraInfos() && pPortion->GetExtraInfos()->bCompressed )
            {
                // I need the manipulated DXArray for determining the break postion...
                sal_Int32* pDXArray = NULL;
                if (!pLine->GetCharPosArray().empty())
                    pDXArray = &pLine->GetCharPosArray()[0] + (nPortionStart - pLine->GetStart());
                ImplCalcAsianCompression(
                    pNode, pPortion, nPortionStart, pDXArray, 10000, true);
            }
            if( pPortion )
                ImpBreakLine( pParaPortion, pLine, pPortion, nPortionStart,
                                                nRemainingWidth, bCanHyphenate && bHyphenatePara );
        }

        // ------------------------------------------------------------------
        // Line finished => adjust
        // ------------------------------------------------------------------

        // CalcTextSize should be replaced by a continuous registering!
        Size aTextSize = pLine->CalcTextSize( *pParaPortion );

        if ( aTextSize.Height() == 0 )
        {
            SeekCursor( pNode, pLine->GetStart()+1, aTmpFont );
            aTmpFont.SetPhysFont( pRefDev );
            ImplInitDigitMode(pRefDev, aTmpFont.GetLanguage());

            if ( IsFixedCellHeight() )
                aTextSize.Height() = ImplCalculateFontIndependentLineSpacing( aTmpFont.GetHeight() );
            else
                aTextSize.Height() = aTmpFont.GetPhysTxtSize( pRefDev, String() ).Height();
            pLine->SetHeight( (sal_uInt16)aTextSize.Height() );
        }

        // The font metrics can not be calculated continuously, if the font is
        // set anyway, because a large font only after wrapping suddenly ends
        // up in the next line => Font metrics too big.
        FormatterFontMetric aFormatterMetrics;
        sal_uInt16 nTPos = pLine->GetStart();
        for ( sal_uInt16 nP = pLine->GetStartPortion(); nP <= pLine->GetEndPortion(); nP++ )
        {
            const TextPortion* pTP = pParaPortion->GetTextPortions()[nP];
            // problem with hard font height attribute, when everthing but the line break has this attribute
            if ( pTP->GetKind() != PORTIONKIND_LINEBREAK )
            {
                SeekCursor( pNode, nTPos+1, aTmpFont );
                aTmpFont.SetPhysFont( GetRefDevice() );
                ImplInitDigitMode(GetRefDevice(), aTmpFont.GetLanguage());
                RecalcFormatterFontMetrics( aFormatterMetrics, aTmpFont );
            }
            nTPos = nTPos + pTP->GetLen();
        }
        sal_uInt16 nLineHeight = aFormatterMetrics.GetHeight();
        if ( nLineHeight > pLine->GetHeight() )
            pLine->SetHeight( nLineHeight );
        pLine->SetMaxAscent( aFormatterMetrics.nMaxAscent );

        bSameLineAgain = sal_False;
        if ( GetTextRanger() && ( pLine->GetHeight() > nTextLineHeight ) )
        {
            // put down with the other size!
            bSameLineAgain = sal_True;
        }


        if ( !bSameLineAgain && !aStatus.IsOutliner() )
        {
            if ( rLSItem.GetLineSpaceRule() == SVX_LINE_SPACE_MIN )
            {
                sal_uInt16 nMinHeight = GetYValue( rLSItem.GetLineHeight() );
                sal_uInt16 nTxtHeight = pLine->GetHeight();
                if ( nTxtHeight < nMinHeight )
                {
                    // The Ascent has to be adjusted for the difference:
                    long nDiff = nMinHeight - nTxtHeight;
                    pLine->SetMaxAscent( (sal_uInt16)(pLine->GetMaxAscent() + nDiff) );
                    pLine->SetHeight( nMinHeight, nTxtHeight );
                }
            }
            else if ( rLSItem.GetLineSpaceRule() == SVX_LINE_SPACE_FIX )
            {
                sal_uInt16 nFixHeight = GetYValue( rLSItem.GetLineHeight() );
                sal_uInt16 nTxtHeight = pLine->GetHeight();
                pLine->SetMaxAscent( (sal_uInt16)(pLine->GetMaxAscent() + ( nFixHeight - nTxtHeight ) ) );
                pLine->SetHeight( nFixHeight, nTxtHeight );
            }
            else if ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_PROP )
            {
                if ( nPara || IsFixedCellHeight() || pLine->GetStartPortion() ) // Not the very first line
                {
                    // There are documents with PropLineSpace 0, why?
                    // (cmc: re above question :-) such documents can be seen by importing a .ppt
                    if ( rLSItem.GetPropLineSpace() && ( rLSItem.GetPropLineSpace() != 100 ) )
                    {
                        sal_uInt16 nTxtHeight = pLine->GetHeight();
                        sal_Int32 nH = nTxtHeight;
                        nH *= rLSItem.GetPropLineSpace();
                        nH /= 100;
                        // The Ascent has to be adjusted for the difference:
                        long nDiff = pLine->GetHeight() - nH;
                        if ( nDiff > pLine->GetMaxAscent() )
                            nDiff = pLine->GetMaxAscent();
                        pLine->SetMaxAscent( (sal_uInt16)(pLine->GetMaxAscent() - nDiff) );
                        pLine->SetHeight( (sal_uInt16)nH, nTxtHeight );
                    }
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
            TextPortion* const pTP = pParaPortion->GetTextPortions()[pLine->GetEndPortion()];
            sal_uInt16 nPosInArray = pLine->GetEnd()-1-pLine->GetStart();
            long nNewValue = ( nPosInArray ? pLine->GetCharPosArray()[ nPosInArray-1 ] : 0 ) + n;
            pLine->GetCharPosArray()[ nPosInArray ] = nNewValue;
            pTP->GetSize().Width() += n;
        }

        pLine->SetTextWidth( aTextSize.Width() );
        switch ( eJustification )
        {
            case SVX_ADJUST_CENTER:
            {
                long n = ( nMaxLineWidth - aTextSize.Width() ) / 2;
                n += nStartX;  // Indentation is kept.
                pLine->SetStartPosX( n );
            }
            break;
            case SVX_ADJUST_RIGHT:
            {
                // For automatically wrapped lines, which has a blank at the end
                // the blank must not be displayed!
                long n = nMaxLineWidth - aTextSize.Width();
                n += nStartX;  // Indentation is kept.
                pLine->SetStartPosX( n );
            }
            break;
            case SVX_ADJUST_BLOCK:
            {
                bool bDistLastLine = (GetJustifyMethod(nPara) == SVX_JUSTIFY_METHOD_DISTRIBUTE);
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

        // -----------------------------------------------------------------
        // Check whether the line must be re-issued ...
        // -----------------------------------------------------------------
        pLine->SetInvalid();

        // If a portion was wrapped there may be far too many positions in
        // CharPosArray:
        if ( bCalcCharPositions )
        {
            EditLine::CharPosArrayType& rArray = pLine->GetCharPosArray();
            size_t nLen = pLine->GetLen();
            if (rArray.size() > nLen)
                rArray.erase(rArray.begin()+nLen, rArray.end());
        }

        if ( GetTextRanger() )
        {
            if ( nTextXOffset )
                pLine->SetStartPosX( pLine->GetStartPosX() + nTextXOffset );
            if ( nTextExtraYOffset )
            {
                pLine->SetHeight( (sal_uInt16) ( pLine->GetHeight() + nTextExtraYOffset ), 0, pLine->GetHeight() );
                pLine->SetMaxAscent( (sal_uInt16) ( pLine->GetMaxAscent() + nTextExtraYOffset ) );
            }
        }

        // for <0 think over !
        if ( pParaPortion->IsSimpleInvalid() )
        {
            // Change through simple Text changes ...
            // Do mot cancel formatting since Portions possibly have to be split
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
                sal_uInt16 nStart = pLine->GetStart();
                sal_uInt16 nEnd = pLine->GetEnd();

                if ( nStart > nInvalidEnd )
                {
                    if ( ( ( nStart-nInvalidDiff ) == aSaveLine.GetStart() ) &&
                            ( ( nEnd-nInvalidDiff ) == aSaveLine.GetEnd() ) )
                    {
                        pLine->SetValid();
                        if ( bCalcCharPositions && bQuickFormat )
                        {
                            bCalcCharPositions = sal_False;
                            bLineBreak = sal_False;
                            pParaPortion->CorrectValuesBehindLastFormattedLine( nLine );
                            break;
                        }
                    }
                }
                else if ( bCalcCharPositions && bQuickFormat && ( nEnd > nInvalidEnd) )
                {
                    // If the invalid line ends so that the next begins on the
                    // 'same' passage as before, i.e. not wrapped differently,
                    //  then the text width does not have to be determined anew:
                    if ( nEnd == ( aSaveLine.GetEnd() + nInvalidDiff ) )
                    {
                        bCalcCharPositions = sal_False;
                        bLineBreak = sal_False;
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

            sal_uInt16 nEndPortion = pLine->GetEndPortion();

            // Next line or maybe a new line....
            pLine = 0;
            if ( nLine < pParaPortion->GetLines().Count()-1 )
                pLine = pParaPortion->GetLines()[++nLine];
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
                    // normaly CreateAndInsertEmptyLine would be called, but I want to use
                    // CreateLines, so I need Polygon code only here...
                    TextPortion* pDummyPortion = new TextPortion( 0 );
                    pParaPortion->GetTextPortions().Append(pDummyPortion);
                    pLine = new EditLine;
                    pParaPortion->GetLines().Insert(++nLine, pLine);
                    bForceOneRun = sal_True;
                    bProcessingEmptyLine = sal_True;
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

    if ( nDelFromLine != 0xFFFF )
        pParaPortion->GetLines().DeleteFromLine( nDelFromLine );

    DBG_ASSERT( pParaPortion->GetLines().Count(), "No line after CreateLines!" );

    if ( bLineBreak == sal_True )
        CreateAndInsertEmptyLine( pParaPortion, nStartPosY );

    delete[] pBuf;

    sal_Bool bHeightChanged = FinishCreateLines( pParaPortion );

    if ( bMapChanged )
        GetRefDevice()->Pop();

    GetRefDevice()->Pop();

    return bHeightChanged;
}

void ImpEditEngine::CreateAndInsertEmptyLine( ParaPortion* pParaPortion, sal_uInt32 )
{
    DBG_ASSERT( !GetTextRanger(), "Don't use CreateAndInsertEmptyLine with a polygon!" );

    EditLine* pTmpLine = new EditLine;
    pTmpLine->SetStart( pParaPortion->GetNode()->Len() );
    pTmpLine->SetEnd( pParaPortion->GetNode()->Len() );
    pParaPortion->GetLines().Append(pTmpLine);

    sal_Bool bLineBreak = pParaPortion->GetNode()->Len() ? sal_True : sal_False;
    sal_Int32 nSpaceBefore = 0;
    sal_Int32 nSpaceBeforeAndMinLabelWidth = GetSpaceBeforeAndMinLabelWidth( pParaPortion->GetNode(), &nSpaceBefore );
    const SvxLRSpaceItem& rLRItem = GetLRSpaceItem( pParaPortion->GetNode() );
    const SvxLineSpacingItem& rLSItem = (const SvxLineSpacingItem&)pParaPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_SBL );
    short nStartX = GetXValue( (short)(rLRItem.GetTxtLeft() + rLRItem.GetTxtFirstLineOfst() + nSpaceBefore));

    Rectangle aBulletArea = Rectangle( Point(), Point() );
    if ( bLineBreak == sal_True )
    {
        nStartX = (short)GetXValue( rLRItem.GetTxtLeft() + rLRItem.GetTxtFirstLineOfst() + nSpaceBeforeAndMinLabelWidth );
    }
    else
    {
        aBulletArea = GetEditEnginePtr()->GetBulletArea( GetParaPortions().GetPos( pParaPortion ) );
        if ( aBulletArea.Right() > 0 )
            pParaPortion->SetBulletX( (sal_uInt16) GetXValue( aBulletArea.Right() ) );
        else
            pParaPortion->SetBulletX( 0 ); // If Bullet set incorrectly.
        if ( pParaPortion->GetBulletX() > nStartX )
        {
            nStartX = (short)GetXValue( rLRItem.GetTxtLeft() + rLRItem.GetTxtFirstLineOfst() + nSpaceBeforeAndMinLabelWidth );
            if ( pParaPortion->GetBulletX() > nStartX )
                nStartX = pParaPortion->GetBulletX();
        }
    }

    SvxFont aTmpFont;
    SeekCursor( pParaPortion->GetNode(), bLineBreak ? pParaPortion->GetNode()->Len() : 0, aTmpFont );
    aTmpFont.SetPhysFont( pRefDev );

    TextPortion* pDummyPortion = new TextPortion( 0 );
    pDummyPortion->GetSize() = aTmpFont.GetPhysTxtSize( pRefDev, String() );
    if ( IsFixedCellHeight() )
        pDummyPortion->GetSize().Height() = ImplCalculateFontIndependentLineSpacing( aTmpFont.GetHeight() );
    pParaPortion->GetTextPortions().Append(pDummyPortion);
    FormatterFontMetric aFormatterMetrics;
    RecalcFormatterFontMetrics( aFormatterMetrics, aTmpFont );
    pTmpLine->SetMaxAscent( aFormatterMetrics.nMaxAscent );
    pTmpLine->SetHeight( (sal_uInt16) pDummyPortion->GetSize().Height() );
    sal_uInt16 nLineHeight = aFormatterMetrics.GetHeight();
    if ( nLineHeight > pTmpLine->GetHeight() )
        pTmpLine->SetHeight( nLineHeight );

    if ( !aStatus.IsOutliner() )
    {
        sal_Int32 nPara = GetParaPortions().GetPos( pParaPortion );
        SvxAdjust eJustification = GetJustification( nPara );
        long nMaxLineWidth = !IsVertical() ? aPaperSize.Width() : aPaperSize.Height();
        nMaxLineWidth -= GetXValue( rLRItem.GetRight() );
        long nTextXOffset = 0;
        if ( nMaxLineWidth < 0 )
            nMaxLineWidth = 1;
        if ( eJustification ==  SVX_ADJUST_CENTER )
            nStartX = sal::static_int_cast< short >(nMaxLineWidth / 2);
        else if ( eJustification ==  SVX_ADJUST_RIGHT )
            nStartX = sal::static_int_cast< short >(nMaxLineWidth);

        nStartX = sal::static_int_cast< short >(nStartX + nTextXOffset);
    }

    pTmpLine->SetStartPosX( nStartX );

    if ( !aStatus.IsOutliner() )
    {
        if ( rLSItem.GetLineSpaceRule() == SVX_LINE_SPACE_MIN )
        {
            sal_uInt16 nMinHeight = rLSItem.GetLineHeight();
            sal_uInt16 nTxtHeight = pTmpLine->GetHeight();
            if ( nTxtHeight < nMinHeight )
            {
                // The Ascent has to be adjusted for the difference:
                long nDiff = nMinHeight - nTxtHeight;
                pTmpLine->SetMaxAscent( (sal_uInt16)(pTmpLine->GetMaxAscent() + nDiff) );
                pTmpLine->SetHeight( nMinHeight, nTxtHeight );
            }
        }
        else if ( rLSItem.GetLineSpaceRule() == SVX_LINE_SPACE_FIX )
        {
            sal_uInt16 nFixHeight = rLSItem.GetLineHeight();
            sal_uInt16 nTxtHeight = pTmpLine->GetHeight();

            pTmpLine->SetMaxAscent( (sal_uInt16)(pTmpLine->GetMaxAscent() + ( nFixHeight - nTxtHeight ) ) );
            pTmpLine->SetHeight( nFixHeight, nTxtHeight );
        }
        else if ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_PROP )
        {
            sal_Int32 nPara = GetParaPortions().GetPos( pParaPortion );
            if ( nPara || IsFixedCellHeight() || pTmpLine->GetStartPortion() ) // Not the very first line
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
                    pTmpLine->SetMaxAscent( (sal_uInt16)(pTmpLine->GetMaxAscent() - nDiff) );
                    pTmpLine->SetHeight( (sal_uInt16)nH, nTxtHeight );
                }
            }
        }
    }

    if ( !bLineBreak )
    {
        long nMinHeight = aBulletArea.GetHeight();
        if ( nMinHeight > (long)pTmpLine->GetHeight() )
        {
            long nDiff = nMinHeight - (long)pTmpLine->GetHeight();
            // distribute nDiff upwards and downwards
            pTmpLine->SetMaxAscent( (sal_uInt16)(pTmpLine->GetMaxAscent() + nDiff/2) );
            pTmpLine->SetHeight( (sal_uInt16)nMinHeight );
        }
    }
    else
    {
        // -2: The new one is already inserted.
#ifdef DBG_UTIL
        EditLine* pLastLine = pParaPortion->GetLines()[pParaPortion->GetLines().Count()-2];
        DBG_ASSERT( pLastLine, "soft wrap no line?!" );
        DBG_ASSERT( pLastLine->GetEnd() == pParaPortion->GetNode()->Len(), "different anyway?" );
#endif
        sal_uInt16 nPos = (sal_uInt16) pParaPortion->GetTextPortions().Count() - 1 ;
        pTmpLine->SetStartPortion( nPos );
        pTmpLine->SetEndPortion( nPos );
    }
}

sal_Bool ImpEditEngine::FinishCreateLines( ParaPortion* pParaPortion )
{
//  CalcCharPositions( pParaPortion );
    pParaPortion->SetValid();
    long nOldHeight = pParaPortion->GetHeight();
    CalcHeight( pParaPortion );

    DBG_ASSERT( pParaPortion->GetTextPortions().Count(), "FinishCreateLines: No Text-Portion?" );
    sal_Bool bRet = ( pParaPortion->GetHeight() != nOldHeight );
    return bRet;
}

void ImpEditEngine::ImpBreakLine( ParaPortion* pParaPortion, EditLine* pLine, TextPortion* pPortion, sal_uInt16 nPortionStart, long nRemainingWidth, sal_Bool bCanHyphenate )
{
    ContentNode* const pNode = pParaPortion->GetNode();

    sal_uInt16 nBreakInLine = nPortionStart - pLine->GetStart();
    sal_uInt16 nMax = nBreakInLine + pPortion->GetLen();
    while ( ( nBreakInLine < nMax ) && ( pLine->GetCharPosArray()[nBreakInLine] < nRemainingWidth ) )
        nBreakInLine++;

    sal_uInt16 nMaxBreakPos = nBreakInLine + pLine->GetStart();
       sal_uInt16 nBreakPos = 0xFFFF;

    sal_Bool bCompressBlank = sal_False;
    sal_Bool bHyphenated = sal_False;
    sal_Bool bHangingPunctuation = sal_False;
    sal_Unicode cAlternateReplChar = 0;
    sal_Unicode cAlternateExtraChar = 0;
    sal_Bool bAltFullLeft = sal_False;
    sal_Bool bAltFullRight = sal_False;
    sal_uInt32 nAltDelChar = 0;

    if ( ( nMaxBreakPos < ( nMax + pLine->GetStart() ) ) && ( pNode->GetChar( nMaxBreakPos ) == ' ' ) )
    {
        // Break behind the blank, blank will be compressed...
        nBreakPos = nMaxBreakPos + 1;
        bCompressBlank = sal_True;
    }
    else
    {
        sal_uInt16 nMinBreakPos = pLine->GetStart();
        const CharAttribList::AttribsType& rAttrs = pNode->GetCharAttribs().GetAttribs();
        for (size_t nAttr = rAttrs.size(); nAttr; )
        {
            const EditCharAttrib& rAttr = rAttrs[--nAttr];
            if (rAttr.IsFeature() && rAttr.GetEnd() > nMinBreakPos && rAttr.GetEnd() <= nMaxBreakPos)
            {
                nMinBreakPos = rAttr.GetEnd();
                break;
            }
        }

        lang::Locale aLocale = GetLocale( EditPaM( pNode, nMaxBreakPos ) );

        Reference < i18n::XBreakIterator > _xBI( ImplGetBreakIterator() );
        Reference< XHyphenator > xHyph;
        if ( bCanHyphenate )
            xHyph = GetHyphenator();
        i18n::LineBreakHyphenationOptions aHyphOptions( xHyph, Sequence< PropertyValue >(), 1 );
        i18n::LineBreakUserOptions aUserOptions;

        const i18n::ForbiddenCharacters* pForbidden = GetForbiddenCharsTable()->GetForbiddenCharacters( LanguageTag::convertToLanguageType( aLocale ), sal_True );
        aUserOptions.forbiddenBeginCharacters = pForbidden->beginLine;
        aUserOptions.forbiddenEndCharacters = pForbidden->endLine;
        aUserOptions.applyForbiddenRules = ((const SfxBoolItem&)pNode->GetContentAttribs().GetItem( EE_PARA_FORBIDDENRULES )).GetValue();
        aUserOptions.allowPunctuationOutsideMargin = ((const SfxBoolItem&)pNode->GetContentAttribs().GetItem( EE_PARA_HANGINGPUNCTUATION )).GetValue();
        aUserOptions.allowHyphenateEnglish = sal_False;

        i18n::LineBreakResults aLBR = _xBI->getLineBreak(
            pNode->GetString(), nMaxBreakPos, aLocale, nMinBreakPos, aHyphOptions, aUserOptions );
        nBreakPos = (sal_uInt16)aLBR.breakIndex;

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

        // nBreakPos can never be outside the portion, even not with hangig punctuation
        if ( nBreakPos > nMaxBreakPos )
            nBreakPos = nMaxBreakPos;

        // BUG in I18N - the japanese dot is in the next line!
        // !!!  Test!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        if ( (nBreakPos + ( aUserOptions.allowPunctuationOutsideMargin ? 0 : 1 ) ) <= nMaxBreakPos )
        {
            sal_Unicode cFirstInNextLine = ( (nBreakPos+1) < pNode->Len() ) ? pNode->GetChar( nBreakPos ) : 0;
            if ( cFirstInNextLine == 12290 )
                nBreakPos++;
        }

        bHangingPunctuation = ( nBreakPos > nMaxBreakPos ) ? sal_True : sal_False;
        pLine->SetHangingPunctuation( bHangingPunctuation );

        // Whether a separator or not, push the word after the separator through
        // hyphenation ... NMaxBreakPos is the last character that fits into
        // the line, nBreakPos is the beginning of the word.
        // There is a problem if the Doc is so narrow that a word is broken
        // into more than two lines ...
        if ( !bHangingPunctuation && bCanHyphenate && GetHyphenator().is() )
        {
            i18n::Boundary aBoundary = _xBI->getWordBoundary(
                pNode->GetString(), nBreakPos, GetLocale( EditPaM( pNode, nBreakPos ) ), ::com::sun::star::i18n::WordType::DICTIONARY_WORD, true);
            sal_uInt16 nWordStart = nBreakPos;
            sal_uInt16 nWordEnd = (sal_uInt16) aBoundary.endPos;
            DBG_ASSERT( nWordEnd > nWordStart, "ImpBreakLine: Start >= End?" );

            sal_uInt16 nWordLen = nWordEnd - nWordStart;
            if ( ( nWordEnd >= nMaxBreakPos ) && ( nWordLen > 3 ) )
            {
                // May happen, because getLineBreak may differ from getWordBoudary with DICTIONARY_WORD
                String aWord(pNode->GetString(), nWordStart, nWordLen);
                sal_uInt16 nMinTrail = nWordEnd-nMaxBreakPos+1; //+1: Before the dickey letter
                Reference< XHyphenatedWord > xHyphWord;
                if (xHyphenator.is())
                    xHyphWord = xHyphenator->hyphenate( aWord, aLocale, aWord.Len() - nMinTrail, Sequence< PropertyValue >() );
                if (xHyphWord.is())
                {
                    sal_Bool bAlternate = xHyphWord->isAlternativeSpelling();
                    sal_uInt16 _nWordLen = 1 + xHyphWord->getHyphenPos();

                    if ( ( _nWordLen >= 2 ) && ( (nWordStart+_nWordLen) >= (pLine->GetStart() + 2 ) ) )
                    {
                        if ( !bAlternate )
                        {
                            bHyphenated = sal_True;
                            nBreakPos = nWordStart + _nWordLen;
                        }
                        else
                        {
                            // TODO: handle all alternative hyphenations (see hyphen-1.2.8/tests/unicode.*)
                            OUString aAlt( xHyphWord->getHyphenatedWord() );
                            OUString aWord2(aWord);
                            OUString aAltLeft(aAlt.copy(0, _nWordLen));
                            OUString aAltRight(aAlt.copy(_nWordLen));
                            bAltFullLeft = aWord2.startsWith(aAltLeft);
                            bAltFullRight = aWord2.endsWith(aAltRight);
                            nAltDelChar = aWord2.getLength() - aAlt.getLength() + (1 - bAltFullLeft) + (1 - bAltFullRight);

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
                            sal_uInt16 nAltStart = _nWordLen - 1;
                            sal_uInt16 nTxtStart = nAltStart - (aAlt.getLength() - aWord.Len());
                            sal_uInt16 nTxtEnd = nTxtStart;
                            sal_uInt16 nAltEnd = nAltStart;

                            // The regions between the nStart and nEnd is the
                            // difference between alternative and original string.
                            while( nTxtEnd < aWord.Len() && nAltEnd < aAlt.getLength() &&
                                   aWord.GetChar(nTxtEnd) != aAlt[nAltEnd] )
                            {
                                ++nTxtEnd;
                                ++nAltEnd;
                            }

                            // If a character is added, then we notice it now:
                            if( nAltEnd > nTxtEnd && nAltStart == nAltEnd &&
                                aWord.GetChar( nTxtEnd ) == aAlt[nAltEnd] )
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

                            bHyphenated = sal_True;
                            nBreakPos = nWordStart + nTxtStart;
                            if ( cAlternateReplChar || aAlt.getLength() < aWord2.getLength() || !bAltFullRight) // also for "oma-tje", "re-eel"
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

    sal_uInt16 nEndPortion = SplitTextPortion( pParaPortion, nBreakPos, pLine );

    if ( !bCompressBlank && !bHangingPunctuation )
    {
        // When justification is not SVX_ADJUST_LEFT, it's important to compress
        // the trailing space even if there is enough room for the space...
        // Don't check for SVX_ADJUST_LEFT, doesn't matter to compress in this case too...
        DBG_ASSERT( nBreakPos > pLine->GetStart(), "ImpBreakLines - BreakPos not expected!" );
        if ( pNode->GetChar( nBreakPos-1 ) == ' ' )
            bCompressBlank = sal_True;
    }

    if ( bCompressBlank || bHangingPunctuation )
    {
        TextPortion* const pTP = pParaPortion->GetTextPortions()[nEndPortion];
        DBG_ASSERT( pTP->GetKind() == PORTIONKIND_TEXT, "BlankRubber: No TextPortion!" );
        DBG_ASSERT( nBreakPos > pLine->GetStart(), "SplitTextPortion at the beginning of the line?" );
        sal_uInt16 nPosInArray = nBreakPos - 1 - pLine->GetStart();
        pTP->GetSize().Width() = ( nPosInArray && ( pTP->GetLen() > 1 ) ) ? pLine->GetCharPosArray()[ nPosInArray-1 ] : 0;
        pLine->GetCharPosArray()[ nPosInArray ] = pTP->GetSize().Width();
    }
    else if ( bHyphenated )
    {
        // A portion for inserting the separator ...
        TextPortion* pHyphPortion = new TextPortion( 0 );
        pHyphPortion->GetKind() = PORTIONKIND_HYPHENATOR;
        String aHyphText(OUString(CH_HYPH));
        if ( (cAlternateReplChar || cAlternateExtraChar) && bAltFullRight ) // alternation after the break doesn't supported
        {
            TextPortion* pPrev = pParaPortion->GetTextPortions()[nEndPortion];
            DBG_ASSERT( pPrev && pPrev->GetLen(), "Hyphenate: Prev portion?!" );
            pPrev->SetLen( pPrev->GetLen() - nAltDelChar );
            pHyphPortion->SetLen( nAltDelChar );
            if (cAlternateReplChar && !bAltFullLeft) pHyphPortion->SetExtraValue( cAlternateReplChar );
            // Correct width of the portion above:
            pPrev->GetSize().Width() =
                pLine->GetCharPosArray()[ nBreakPos-1 - pLine->GetStart() - nAltDelChar ];
        }

        // Determine the width of the Hyph-Portion:
        SvxFont aFont;
        SeekCursor( pParaPortion->GetNode(), nBreakPos, aFont );
        aFont.SetPhysFont( GetRefDevice() );
        pHyphPortion->GetSize().Height() = GetRefDevice()->GetTextHeight();
        pHyphPortion->GetSize().Width() = GetRefDevice()->GetTextWidth( aHyphText );

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

    const sal_uInt16 nFirstChar = pLine->GetStart();
    const sal_uInt16 nLastChar = pLine->GetEnd() -1;    // Last points behind
    ContentNode* pNode = pParaPortion->GetNode();

    DBG_ASSERT( nLastChar < pNode->Len(), "AdjustBlocks: Out of range!" );

    // Search blanks or Kashidas...
    std::vector<sal_uInt16> aPositions;
    sal_uInt16 nLastScript = i18n::ScriptType::LATIN;
    for ( sal_uInt16 nChar = nFirstChar; nChar <= nLastChar; nChar++ )
    {
        EditPaM aPaM( pNode, nChar+1 );
        LanguageType eLang = GetLanguage(aPaM);
        sal_uInt16 nScript = GetScriptType(aPaM);
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
        sal_uInt16 nPortionStart, nPortion;
        nPortion = pParaPortion->GetTextPortions().FindPortion( nLastChar+1, nPortionStart );
        TextPortion* pLastPortion = pParaPortion->GetTextPortions()[ nPortion ];
        long nRealWidth = pLine->GetCharPosArray()[nLastChar-nFirstChar];
        long nBlankWidth = nRealWidth;
        if ( nLastChar > nPortionStart )
            nBlankWidth -= pLine->GetCharPosArray()[nLastChar-nFirstChar-1];
        // Possibly the blank has already been deducted in ImpBreakLine:
        if ( nRealWidth == pLastPortion->GetSize().Width() )
        {
            // For the last character the portion must stop behind the blank
            // => Simplify correction:
            DBG_ASSERT( ( nPortionStart + pLastPortion->GetLen() ) == ( nLastChar+1 ), "Blank actually not at the end of the portion!?");
            pLastPortion->GetSize().Width() -= nBlankWidth;
            nRemainingSpace += nBlankWidth;
        }
        pLine->GetCharPosArray()[nLastChar-nFirstChar] -= nBlankWidth;
    }

    size_t nGaps = aPositions.size();
    const long nMore4Everyone = nRemainingSpace / nGaps;
    long nSomeExtraSpace = nRemainingSpace - nMore4Everyone*nGaps;

    DBG_ASSERT( nSomeExtraSpace < (long)nGaps, "AdjustBlocks: ExtraSpace too large" );
    DBG_ASSERT( nSomeExtraSpace >= 0, "AdjustBlocks: ExtraSpace < 0 " );

    // Correct the positions in the Array and the portion widths:
    // Last character won't be considered ...
    for ( std::vector<sal_uInt16>::const_iterator it(aPositions.begin()); it != aPositions.end(); ++it )
    {
        sal_uInt16 nChar = *it;
        if ( nChar < nLastChar )
        {
            sal_uInt16 nPortionStart, nPortion;
            nPortion = pParaPortion->GetTextPortions().FindPortion( nChar, nPortionStart, true );
            TextPortion* pLastPortion = pParaPortion->GetTextPortions()[ nPortion ];

            // The width of the portion:
            pLastPortion->GetSize().Width() += nMore4Everyone;
            if ( nSomeExtraSpace )
                pLastPortion->GetSize().Width()++;

            // Correct positions in array
            // Even for kashidas just change positions, VCL will then draw the kashida automaticly
            sal_uInt16 nPortionEnd = nPortionStart + pLastPortion->GetLen();
            for ( sal_uInt16 _n = nChar; _n < nPortionEnd; _n++ )
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

void ImpEditEngine::ImpFindKashidas( ContentNode* pNode, sal_uInt16 nStart, sal_uInt16 nEnd, std::vector<sal_uInt16>& rArray )
{
    // the search has to be performed on a per word base

    EditSelection aWordSel( EditPaM( pNode, nStart ) );
    aWordSel = SelectWord( aWordSel, ::com::sun::star::i18n::WordType::DICTIONARY_WORD );
    if ( aWordSel.Min().GetIndex() < nStart )
       aWordSel.Min().GetIndex() = nStart;

    while ( ( aWordSel.Min().GetNode() == pNode ) && ( aWordSel.Min().GetIndex() < nEnd ) )
    {
        sal_uInt16 nSavPos = aWordSel.Max().GetIndex();
        if ( aWordSel.Max().GetIndex() > nEnd )
           aWordSel.Max().GetIndex() = nEnd;

        String aWord = GetSelected( aWordSel );

        // restore selection for proper iteration at the end of the function
        aWordSel.Max().GetIndex() = nSavPos;

        xub_StrLen nIdx = 0;
        xub_StrLen nKashidaPos = STRING_LEN;
        sal_Unicode cCh;
        sal_Unicode cPrevCh = 0;

        while ( nIdx < aWord.Len() )
        {
            cCh = aWord.GetChar( nIdx );

            // 1. Priority:
            // after user inserted kashida
            if ( 0x640 == cCh )
            {
                nKashidaPos = aWordSel.Min().GetIndex() + nIdx;
                break;
            }

            // 2. Priority:
            // after a Seen or Sad
            if ( nIdx + 1 < aWord.Len() &&
                 ( 0x633 == cCh || 0x635 == cCh ) )
            {
                nKashidaPos = aWordSel.Min().GetIndex() + nIdx;
                break;
            }

            // 3. Priority:
            // before final form of the Marbuta, Hah, Dal
            // 4. Priority:
            // before final form of Alef, Lam or Kaf
            if ( nIdx && nIdx + 1 == aWord.Len() &&
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
            if ( nIdx && nIdx + 1 < aWord.Len() && 0x628 == cCh )
            {
                DBG_ASSERT( 0 != cPrevCh, "No previous character" );

                // check if next character is Reh, Yeh or Alef Maksura
                sal_Unicode cNextCh = aWord.GetChar( nIdx + 1 );

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
            if ( nIdx && nIdx + 1 == aWord.Len() &&
                 0x60C <= cCh && 0x6FE >= cCh )
            {
                DBG_ASSERT( 0 != cPrevCh, "No previous character" );

                // check if character is connectable to previous character,
                if ( lcl_ConnectToPrev( cCh, cPrevCh ) )
                {
                    // only choose this position if we did not find
                    // a better one:
                    if ( STRING_LEN == nKashidaPos )
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

        if ( STRING_LEN != nKashidaPos )
            rArray.push_back( nKashidaPos );

        aWordSel = WordRight( aWordSel.Max(), ::com::sun::star::i18n::WordType::DICTIONARY_WORD );
        aWordSel = SelectWord( aWordSel, ::com::sun::star::i18n::WordType::DICTIONARY_WORD );
    }
}

sal_uInt16 ImpEditEngine::SplitTextPortion( ParaPortion* pPortion, sal_uInt16 nPos, EditLine* pCurLine )
{
    DBG_ASSERT( pPortion, "SplitTextPortion: Which ?" );

    // The portion at nPos is split, if there is not a transition at nPos anyway
    if ( nPos == 0 )
        return 0;

    sal_uInt16 nSplitPortion;
    sal_uInt16 nTmpPos = 0;
    TextPortion* pTextPortion = NULL;
    sal_uInt16 nPortions = pPortion->GetTextPortions().Count();
    for ( nSplitPortion = 0; nSplitPortion < nPortions; nSplitPortion++ )
    {
        TextPortion* pTP = pPortion->GetTextPortions()[nSplitPortion];
        nTmpPos = nTmpPos + pTP->GetLen();
        if ( nTmpPos >= nPos )
        {
            if ( nTmpPos == nPos )  // then nothing needs to be split
            {
                return nSplitPortion;
            }
            pTextPortion = pTP;
            break;
        }
    }

    DBG_ASSERT( pTextPortion, "Position outside the area!" );
    DBG_ASSERT( pTextPortion->GetKind() == PORTIONKIND_TEXT, "SplitTextPortion: No TextPortion!" );

    sal_uInt16 nOverlapp = nTmpPos - nPos;
    pTextPortion->GetLen() = pTextPortion->GetLen() - nOverlapp;
    TextPortion* pNewPortion = new TextPortion( nOverlapp );
    pPortion->GetTextPortions().Insert(nSplitPortion+1, pNewPortion);
    // Set sizes
    if ( pCurLine )
    {
        // No new GetTextSize, instead use values from the Array:
        DBG_ASSERT( nPos > pCurLine->GetStart(), "SplitTextPortion at the beginning of the line?" );
        pTextPortion->GetSize().Width() = pCurLine->GetCharPosArray()[ nPos-pCurLine->GetStart()-1 ];

        if ( pTextPortion->GetExtraInfos() && pTextPortion->GetExtraInfos()->bCompressed )
        {
            // We need the original size from the portion
            sal_uInt16 nTxtPortionStart = pPortion->GetTextPortions().GetStartPos( nSplitPortion );
               SvxFont aTmpFont( pPortion->GetNode()->GetCharAttribs().GetDefFont() );
            SeekCursor( pPortion->GetNode(), nTxtPortionStart+1, aTmpFont );
            aTmpFont.SetPhysFont( GetRefDevice() );
            GetRefDevice()->Push( PUSH_TEXTLANGUAGE );
            ImplInitDigitMode(GetRefDevice(), aTmpFont.GetLanguage());
            Size aSz = aTmpFont.QuickGetTextSize( GetRefDevice(), pPortion->GetNode()->GetString(), nTxtPortionStart, pTextPortion->GetLen(), NULL );
            GetRefDevice()->Pop();
            pTextPortion->GetExtraInfos()->nOrgWidth = aSz.Width();
        }
    }
    else
        pTextPortion->GetSize().Width() = (-1);

    return nSplitPortion;
}

void ImpEditEngine::CreateTextPortions( ParaPortion* pParaPortion, sal_uInt16& rStart )
{
    sal_uInt16 nStartPos = rStart;
    ContentNode* pNode = pParaPortion->GetNode();
    DBG_ASSERT( pNode->Len(), "CreateTextPortions should not be used for empty paragraphs!" );

    ::std::set< sal_uInt32 > aPositions;
    aPositions.insert( 0 );

    sal_uInt16 nAttr = 0;
    EditCharAttrib* pAttrib = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
    while ( pAttrib )
    {
        // Insert Start and End into the Array...
        // The Insert method does not allow for duplicate values....
        aPositions.insert( pAttrib->GetStart() );
        aPositions.insert( pAttrib->GetEnd() );
        nAttr++;
        pAttrib = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
    }
    aPositions.insert( pNode->Len() );

    if ( pParaPortion->aScriptInfos.empty() )
        ((ImpEditEngine*)this)->InitScriptTypes( GetParaPortions().GetPos( pParaPortion ) );

    const ScriptTypePosInfos& rTypes = pParaPortion->aScriptInfos;
    for ( size_t nT = 0; nT < rTypes.size(); nT++ )
        aPositions.insert( rTypes[nT].nStartPos );

    const WritingDirectionInfos& rWritingDirections = pParaPortion->aWritingDirectionInfos;
    for ( size_t nD = 0; nD < rWritingDirections.size(); nD++ )
        aPositions.insert( rWritingDirections[nD].nStartPos );

    if ( mpIMEInfos && mpIMEInfos->nLen && mpIMEInfos->pAttribs && ( mpIMEInfos->aPos.GetNode() == pNode ) )
    {
        sal_uInt16 nLastAttr = 0xFFFF;
        for( sal_uInt16 n = 0; n < mpIMEInfos->nLen; n++ )
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
    sal_uInt16 nPortionStart = 0;
    sal_uInt16 nInvPortion = 0;
    sal_uInt16 nP;
    for ( nP = 0; nP < pParaPortion->GetTextPortions().Count(); nP++ )
    {
        const TextPortion* pTmpPortion = pParaPortion->GetTextPortions()[nP];
        nPortionStart = nPortionStart + pTmpPortion->GetLen();
        if ( nPortionStart >= nStartPos )
        {
            nPortionStart = nPortionStart - pTmpPortion->GetLen();
            rStart = nPortionStart;
            nInvPortion = nP;
            break;
        }
    }
    DBG_ASSERT( nP < pParaPortion->GetTextPortions().Count() || !pParaPortion->GetTextPortions().Count(), "Nothing to delete: CreateTextPortions" );
    if ( nInvPortion && ( nPortionStart+pParaPortion->GetTextPortions()[nInvPortion]->GetLen() > nStartPos ) )
    {
        // prefer one in front ...
        // But only if it was in the middle of the portion of, otherwise it
        // might be the only one in the row in front!
        nInvPortion--;
        nPortionStart = nPortionStart - pParaPortion->GetTextPortions()[nInvPortion]->GetLen();
    }
    pParaPortion->GetTextPortions().DeleteFromPortion( nInvPortion );

    // A portion may also have been formed by a line break:
    aPositions.insert( nPortionStart );

    ::std::set< sal_uInt32 >::iterator nInvPos = aPositions.find(  nPortionStart );
    DBG_ASSERT( (nInvPos != aPositions.end()), "InvPos ?!" );

    ::std::set< sal_uInt32 >::iterator i = nInvPos;
    ++i;
    while ( i != aPositions.end() )
    {
        TextPortion* pNew = new TextPortion( static_cast<sal_uInt16>(*i++) - static_cast<sal_uInt16>(*nInvPos++) );
        pParaPortion->GetTextPortions().Append(pNew);
    }

    DBG_ASSERT( pParaPortion->GetTextPortions().Count(), "No Portions?!" );
#if OSL_DEBUG_LEVEL > 2
    OSL_ENSURE( pParaPortion->DbgCheckTextPortions(), "Portion is broken?" );
#endif
}

void ImpEditEngine::RecalcTextPortion( ParaPortion* pParaPortion, sal_uInt16 nStartPos, short nNewChars )
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
            sal_uInt16 nNewPortionPos = 0;
            if ( nStartPos )
                nNewPortionPos = SplitTextPortion( pParaPortion, nStartPos ) + 1;

            // A blank portion may be here, if the paragraph was empty,
            // or if a line was created by a hard line break.
            if ( ( nNewPortionPos < pParaPortion->GetTextPortions().Count() ) &&
                    !pParaPortion->GetTextPortions()[nNewPortionPos]->GetLen() )
            {
                DBG_ASSERT( pParaPortion->GetTextPortions()[nNewPortionPos]->GetKind() == PORTIONKIND_TEXT, "the empty portion was no TextPortion!" );
                sal_uInt16 & r =
                    pParaPortion->GetTextPortions()[nNewPortionPos]->GetLen();
                r = r + nNewChars;
            }
            else
            {
                TextPortion* pNewPortion = new TextPortion( nNewChars );
                pParaPortion->GetTextPortions().Insert(nNewPortionPos, pNewPortion);
            }
        }
        else
        {
            sal_uInt16 nPortionStart;
            const sal_uInt16 nTP = pParaPortion->GetTextPortions().
                FindPortion( nStartPos, nPortionStart );
            TextPortion* const pTP = pParaPortion->GetTextPortions()[ nTP ];
            DBG_ASSERT( pTP, "RecalcTextPortion: Portion not found"  );
            pTP->GetLen() = pTP->GetLen() + nNewChars;
            pTP->GetSize().Width() = (-1);
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
        sal_uInt16 nPortion = 0;
        sal_uInt16 nPos = 0;
        sal_uInt16 nEnd = nStartPos-nNewChars;
        sal_uInt16 nPortions = pParaPortion->GetTextPortions().Count();
        TextPortion* pTP = 0;
        for ( nPortion = 0; nPortion < nPortions; nPortion++ )
        {
            pTP = pParaPortion->GetTextPortions()[ nPortion ];
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
            sal_uInt8 nType = pTP->GetKind();
            pParaPortion->GetTextPortions().Remove( nPortion );
            if ( nType == PORTIONKIND_LINEBREAK )
            {
                TextPortion* pNext = pParaPortion->GetTextPortions()[ nPortion ];
                if ( pNext && !pNext->GetLen() )
                {
                    // Remove dummy portion
                    pParaPortion->GetTextPortions().Remove( nPortion );
                }
            }
        }
        else
        {
            DBG_ASSERT( pTP->GetLen() > (-nNewChars), "Portion too small to shrink! ");
            pTP->GetLen() = pTP->GetLen() + nNewChars;
        }

        sal_uInt16 nPortionCount = pParaPortion->GetTextPortions().Count();
        assert( nPortionCount );
        if (nPortionCount)
        {
            // No HYPHENATOR portion is allowed to get stuck right at the end...
            sal_uInt16 nLastPortion = nPortionCount - 1;
            pTP = pParaPortion->GetTextPortions()[nLastPortion];
            if ( pTP->GetKind() == PORTIONKIND_HYPHENATOR )
            {
                // Discard portion; if possible, correct the ones before,
                // if the Hyphenator portion has swallowed one character...
                if ( nLastPortion && pTP->GetLen() )
                {
                    TextPortion* pPrev = pParaPortion->GetTextPortions()[nLastPortion - 1];
                    DBG_ASSERT( pPrev->GetKind() == PORTIONKIND_TEXT, "Portion?!" );
                    pPrev->SetLen( pPrev->GetLen() + pTP->GetLen() );
                    pPrev->GetSize().Width() = (-1);
                }
                pParaPortion->GetTextPortions().Remove( nLastPortion );
            }
        }
    }
#if OSL_DEBUG_LEVEL > 2
    OSL_ENSURE( pParaPortion->DbgCheckTextPortions(), "Portions are broken?" );
#endif
}

void ImpEditEngine::SetTextRanger( TextRanger* pRanger )
{
    if ( pTextRanger != pRanger )
    {
        delete pTextRanger;
        pTextRanger = pRanger;

        for ( sal_Int32 nPara = 0; nPara < GetParaPortions().Count(); nPara++ )
        {
            ParaPortion* pParaPortion = GetParaPortions()[nPara];
            pParaPortion->MarkSelectionInvalid( 0, pParaPortion->GetNode()->Len() );
            pParaPortion->GetLines().Reset();
        }

        FormatFullDoc();
        UpdateViews( GetActiveView() );
        if ( GetUpdateMode() && GetActiveView() )
            pActiveView->ShowCursor(false, false);
    }
}

void ImpEditEngine::SetVertical( sal_Bool bVertical )
{
    if ( IsVertical() != bVertical )
    {
        GetEditDoc().SetVertical( bVertical );
        sal_Bool bUseCharAttribs = ( aStatus.GetControlWord() & EE_CNTRL_USECHARATTRIBS ) ? sal_True : sal_False;
        GetEditDoc().CreateDefFont( bUseCharAttribs );
        if ( IsFormatted() )
        {
            FormatFullDoc();
            UpdateViews( GetActiveView() );
        }
    }
}

void ImpEditEngine::SetFixedCellHeight( sal_Bool bUseFixedCellHeight )
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

void ImpEditEngine::SeekCursor( ContentNode* pNode, sal_uInt16 nPos, SvxFont& rFont, OutputDevice* pOut, sal_uInt16 nIgnoreWhich )
{
    // It was planned, SeekCursor( nStartPos, nEndPos, ... ), so that it would
    // only be searched anew at the StartPosition.
    // Problem: There would be two lists to consider/handle:
    // OrderedByStart,OrderedByEnd.

    if ( nPos > pNode->Len() )
        nPos = pNode->Len();

    rFont = pNode->GetCharAttribs().GetDefFont();

    short nScriptType = GetScriptType( EditPaM( pNode, nPos ) );
    if ( ( nScriptType == i18n::ScriptType::ASIAN ) || ( nScriptType == i18n::ScriptType::COMPLEX ) )
    {
        const SvxFontItem& rFontItem = (const SvxFontItem&)pNode->GetContentAttribs().GetItem( GetScriptItemId( EE_CHAR_FONTINFO, nScriptType ) );
        rFont.SetName( rFontItem.GetFamilyName() );
        rFont.SetFamily( rFontItem.GetFamily() );
        rFont.SetPitch( rFontItem.GetPitch() );
        rFont.SetCharSet( rFontItem.GetCharSet() );
        Size aSz( rFont.GetSize() );
        aSz.Height() = ((const SvxFontHeightItem&)pNode->GetContentAttribs().GetItem( GetScriptItemId( EE_CHAR_FONTHEIGHT, nScriptType ) ) ).GetHeight();
        rFont.SetSize( aSz );
        rFont.SetWeight( ((const SvxWeightItem&)pNode->GetContentAttribs().GetItem( GetScriptItemId( EE_CHAR_WEIGHT, nScriptType ))).GetWeight() );
        rFont.SetItalic( ((const SvxPostureItem&)pNode->GetContentAttribs().GetItem( GetScriptItemId( EE_CHAR_ITALIC, nScriptType ))).GetPosture() );
        rFont.SetLanguage( ((const SvxLanguageItem&)pNode->GetContentAttribs().GetItem( GetScriptItemId( EE_CHAR_LANGUAGE, nScriptType ))).GetLanguage() );
    }

    sal_uInt16 nRelWidth = ((const SvxCharScaleWidthItem&)pNode->GetContentAttribs().GetItem( EE_CHAR_FONTWIDTH)).GetValue();

    if ( pOut )
    {
        const SvxUnderlineItem& rTextLineColor = (const SvxUnderlineItem&)pNode->GetContentAttribs().GetItem( EE_CHAR_UNDERLINE );
        if ( rTextLineColor.GetColor() != COL_TRANSPARENT )
            pOut->SetTextLineColor( rTextLineColor.GetColor() );
        else
            pOut->SetTextLineColor();
    }

    if ( pOut )
    {
        const SvxOverlineItem& rOverlineColor = (const SvxOverlineItem&)pNode->GetContentAttribs().GetItem( EE_CHAR_OVERLINE );
        if ( rOverlineColor.GetColor() != COL_TRANSPARENT )
            pOut->SetOverlineColor( rOverlineColor.GetColor() );
        else
            pOut->SetOverlineColor();
    }

    const SvxLanguageItem* pCJKLanguageItem = NULL;

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
            if ( ( pAttrib->Which() != nIgnoreWhich ) &&
                 ( ( ( pAttrib->GetStart() < nPos ) && ( pAttrib->GetEnd() >= nPos ) )
                     || ( !pNode->Len() ) ) )
            {
                DBG_ASSERT( ( pAttrib->Which() >= EE_CHAR_START ) && ( pAttrib->Which() <= EE_FEATURE_END ), "Invalid Attribute in Seek() " );
                if ( IsScriptItemValid( pAttrib->Which(), nScriptType ) )
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
                    nRelWidth = ((const SvxCharScaleWidthItem*)pAttrib->GetItem())->GetValue();
                if ( pAttrib->Which() == EE_CHAR_LANGUAGE_CJK )
                    pCJKLanguageItem = (const SvxLanguageItem*) pAttrib->GetItem();
            }
            pAttrib = GetAttrib( rAttribs, ++nAttr );
        }
    }

    if ( !pCJKLanguageItem )
        pCJKLanguageItem = (const SvxLanguageItem*) &pNode->GetContentAttribs().GetItem( EE_CHAR_LANGUAGE_CJK );

    rFont.SetCJKContextLanguage( pCJKLanguageItem->GetLanguage() );

    if ( rFont.GetKerning() && IsKernAsianPunctuation() && ( nScriptType == i18n::ScriptType::ASIAN ) )
        rFont.SetKerning( rFont.GetKerning() | KERNING_ASIAN );

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
        Size aRealSz( aMetric.GetSize() );
        rFont.SetPropr( 100 );

        if ( aStatus.DoStretch() )
        {
            if ( nStretchY != 100 )
            {
                aRealSz.Height() *= nStretchY;
                aRealSz.Height() /= 100;
            }
            if ( nStretchX != 100 )
            {
                if ( nStretchX == nStretchY &&
                     nRelWidth == 100 )
                {
                    aRealSz.Width() = 0;
                }
                else
                {
                    aRealSz.Width() *= nStretchX;
                    aRealSz.Width() /= 100;

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
                    rFont.SetFixKerning( (short)nKerning );
                }
            }
        }
        if ( nRelWidth != 100 )
        {
            aRealSz.Width() *= nRelWidth;
            aRealSz.Width() /= 100;
        }
        rFont.SetSize( aRealSz );
        // Font is not restored ...
    }

    if ( ( ( rFont.GetColor() == COL_AUTO ) || ( IsForceAutoColor() ) ) && pOut )
    {
        // #i75566# Do not use AutoColor when printing OR Pdf export
        const bool bPrinting(OUTDEV_PRINTER == pOut->GetOutDevType());
        const bool bPDFExporting(0 != pOut->GetPDFWriter());

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
        sal_uInt16 nAttr = mpIMEInfos->pAttribs[ nPos - mpIMEInfos->aPos.GetIndex() - 1 ];
        if ( nAttr & EXTTEXTINPUT_ATTR_UNDERLINE )
            rFont.SetUnderline( UNDERLINE_SINGLE );
        else if ( nAttr & EXTTEXTINPUT_ATTR_BOLDUNDERLINE )
            rFont.SetUnderline( UNDERLINE_BOLD );
        else if ( nAttr & EXTTEXTINPUT_ATTR_DOTTEDUNDERLINE )
            rFont.SetUnderline( UNDERLINE_DOTTED );
        else if ( nAttr & EXTTEXTINPUT_ATTR_DASHDOTUNDERLINE )
            rFont.SetUnderline( UNDERLINE_DOTTED );
        else if ( nAttr & EXTTEXTINPUT_ATTR_REDTEXT )
            rFont.SetColor( Color( COL_RED ) );
        else if ( nAttr & EXTTEXTINPUT_ATTR_HALFTONETEXT )
            rFont.SetColor( Color( COL_LIGHTGRAY ) );
        if ( nAttr & EXTTEXTINPUT_ATTR_HIGHLIGHT )
        {
            const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
            rFont.SetColor( rStyleSettings.GetHighlightTextColor() );
            rFont.SetFillColor( rStyleSettings.GetHighlightColor() );
            rFont.SetTransparent( sal_False );
        }
        else if ( nAttr & EXTTEXTINPUT_ATTR_GRAYWAVELINE )
        {
            rFont.SetUnderline( UNDERLINE_WAVE );
            if( pOut )
                pOut->SetTextLineColor( Color( COL_LIGHTGRAY ) );
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
    nAscent = (sal_uInt16)aMetric.GetAscent();
    if ( IsAddExtLeading() )
        nAscent = sal::static_int_cast< sal_uInt16 >(
            nAscent + aMetric.GetExtLeading() );
    nDescent = (sal_uInt16)aMetric.GetDescent();

    if ( IsFixedCellHeight() )
    {
        nAscent = sal::static_int_cast< sal_uInt16 >( rFont.GetHeight() );
        nDescent= sal::static_int_cast< sal_uInt16 >( ImplCalculateFontIndependentLineSpacing( rFont.GetHeight() ) - nAscent );
    }
    else
    {
        sal_uInt16 nIntLeading = ( aMetric.GetIntLeading() > 0 ) ? (sal_uInt16)aMetric.GetIntLeading() : 0;
        // Fonts without leading cause problems
        if ( ( nIntLeading == 0 ) && ( pRefDev->GetOutDevType() == OUTDEV_PRINTER ) )
        {
            // Lets see what Leading one gets on the screen
            VirtualDevice* pVDev = GetVirtualDevice( pRefDev->GetMapMode(), pRefDev->GetDrawMode() );
            rFont.SetPhysFont( pVDev );
            aMetric = pVDev->GetFontMetric();

            // This is so that the Leading does not count itself out again,
            // if the whole line has the font, nTmpLeading.
            nAscent = (sal_uInt16)aMetric.GetAscent();
            nDescent = (sal_uInt16)aMetric.GetDescent();
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
        short nDiff = (short)(rFont.GetSize().Height()*rFont.GetEscapement()/100L);
        if ( rFont.GetEscapement() > 0 )
        {
            nAscent = (sal_uInt16) (((long)nAscent)*nPropr/100 + nDiff);
            if ( nAscent > rCurMetrics.nMaxAscent )
                rCurMetrics.nMaxAscent = nAscent;
        }
        else    // has to be < 0
        {
            nDescent = (sal_uInt16) (((long)nDescent)*nPropr/100 - nDiff);
            if ( nDescent > rCurMetrics.nMaxDescent )
                rCurMetrics.nMaxDescent= nDescent;
        }
    }
}

void ImpEditEngine::Paint( OutputDevice* pOutDev, Rectangle aClipRect, Point aStartPos, sal_Bool bStripOnly, short nOrientation )
{
    if ( !GetUpdateMode() && !bStripOnly )
        return;

    if ( !IsFormatted() )
        FormatDoc();

    long nFirstVisXPos = - pOutDev->GetMapMode().GetOrigin().X();
    long nFirstVisYPos = - pOutDev->GetMapMode().GetOrigin().Y();

    const EditLine* pLine = NULL;
    Point aTmpPos;
    Point aRedLineTmpPos;
    DBG_ASSERT( GetParaPortions().Count(), "No ParaPortion?!" );
    SvxFont aTmpFont( GetParaPortions()[0]->GetNode()->GetCharAttribs().GetDefFont() );
    Font aOldFont( pOutDev->GetFont() );
    vcl::PDFExtOutDevData* pPDFExtOutDevData = PTR_CAST( vcl::PDFExtOutDevData, pOutDev->GetExtOutDevData() );

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
    const bool bMetafileValid( pMtf != NULL );

    long nVertLineSpacing = CalcVertLineSpacing(aStartPos);

    // --------------------------------------------------
    // Over all the paragraphs ...
    // --------------------------------------------------
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
        sal_uInt16 nIndex = 0;
        if ( pPortion->IsVisible() && (
                ( !IsVertical() && ( ( aStartPos.Y() + nParaHeight ) > aClipRect.Top() ) ) ||
                ( IsVertical() && ( ( aStartPos.X() - nParaHeight ) < aClipRect.Right() ) ) ) )

        {
            // --------------------------------------------------
            // Over the lines of the paragraph ...
            // --------------------------------------------------
            sal_uInt16 nLines = pPortion->GetLines().Count();
            sal_uInt16 nLastLine = nLines-1;

            // #108052#
            bool bEndOfParagraphWritten(false);

            if ( !IsVertical() )
                aStartPos.Y() += pPortion->GetFirstLineOffset();
            else
                aStartPos.X() -= pPortion->GetFirstLineOffset();

            Point aParaStart( aStartPos );

            const SvxLineSpacingItem& rLSItem = ((const SvxLineSpacingItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_SBL ));
            sal_uInt16 nSBL = ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_FIX )
                                ? GetYValue( rLSItem.GetInterLineSpace() ) : 0;
            bool bPaintBullet (false);

            for ( sal_uInt16 nLine = 0; nLine < nLines; nLine++ )
            {
                pLine = pPortion->GetLines()[nLine];
                nIndex = pLine->GetStart();
                DBG_ASSERT( pLine, "NULL-Pointer in the line iterator in UpdateViews" );
                aTmpPos = aStartPos;
                if ( !IsVertical() )
                {
                    aTmpPos.X() += pLine->GetStartPosX();
                    aTmpPos.Y() += pLine->GetMaxAscent();
                    aStartPos.Y() += pLine->GetHeight();
                    if (nLine != nLastLine)
                        aStartPos.Y() += nVertLineSpacing;
                }
                else
                {
                    aTmpPos.Y() += pLine->GetStartPosX();
                    aTmpPos.X() -= pLine->GetMaxAscent();
                    aStartPos.X() -= pLine->GetHeight();
                    if (nLine != nLastLine)
                        aStartPos.X() -= nVertLineSpacing;
                }

                if ( ( !IsVertical() && ( aStartPos.Y() > aClipRect.Top() ) )
                    || ( IsVertical() && aStartPos.X() < aClipRect.Right() ) )
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
                        const SfxBoolItem& rBulletState = static_cast<const SfxBoolItem&>(
                            pEditEngine->GetParaAttrib(n, EE_PARA_BULLETSTATE));
                        bPaintBullet = rBulletState.GetValue() ? true : false;
                    }

                    // --------------------------------------------------
                    // Over the Portions of the line ...
                    // --------------------------------------------------
                    bool bParsingFields = false;
                    ::std::vector< sal_Int32 >::iterator itSubLines;

                    for ( sal_uInt16 nPortion = pLine->GetStartPortion(); nPortion <= pLine->GetEndPortion(); nPortion++ )
                    {
                        DBG_ASSERT( pPortion->GetTextPortions().Count(), "Line without Textportion in Paint!" );
                        const TextPortion* pTextPortion = pPortion->GetTextPortions()[nPortion];
                        DBG_ASSERT( pTextPortion, "NULL-Pointer in Portion iterator in UpdateViews" );

                        long nPortionXOffset = GetPortionXOffset( pPortion, pLine, nPortion );
                        if ( !IsVertical() )
                        {
                            aTmpPos.X() = aStartPos.X() + nPortionXOffset;
                            if ( aTmpPos.X() > aClipRect.Right() )
                                break;  // No further output in line necessary
                        }
                        else
                        {
                            aTmpPos.Y() = aStartPos.Y() + nPortionXOffset;
                            if ( aTmpPos.Y() > aClipRect.Bottom() )
                                break;  // No further output in line necessary
                        }

                        switch ( pTextPortion->GetKind() )
                        {
                            case PORTIONKIND_TEXT:
                            case PORTIONKIND_FIELD:
                            case PORTIONKIND_HYPHENATOR:
                            {
                                SeekCursor( pPortion->GetNode(), nIndex+1, aTmpFont, pOutDev );

                                sal_Bool bDrawFrame = sal_False;

                                if ( ( pTextPortion->GetKind() == PORTIONKIND_FIELD ) && !aTmpFont.IsTransparent() &&
                                     ( GetBackgroundColor() != COL_AUTO ) && GetBackgroundColor().IsDark() &&
                                     ( IsAutoColorEnabled() && ( pOutDev->GetOutDevType() != OUTDEV_PRINTER ) ) )
                                {
                                    aTmpFont.SetTransparent( sal_True );
                                    pOutDev->SetFillColor();
                                    pOutDev->SetLineColor( GetAutoColor() );
                                    bDrawFrame = sal_True;
                                }

#if OSL_DEBUG_LEVEL > 2
                                if ( pTextPortion->GetKind() == PORTIONKIND_HYPHENATOR )
                                {
                                    aTmpFont.SetFillColor( COL_LIGHTGRAY );
                                    aTmpFont.SetTransparent( sal_False );
                                }
                                if ( pTextPortion->GetRightToLeft()  )
                                {
                                    aTmpFont.SetFillColor( COL_LIGHTGRAY );
                                    aTmpFont.SetTransparent( sal_False );
                                }
                                else if ( GetScriptType( EditPaM( pPortion->GetNode(), nIndex+1 ) ) == i18n::ScriptType::COMPLEX )
                                {
                                    aTmpFont.SetFillColor( COL_LIGHTCYAN );
                                    aTmpFont.SetTransparent( sal_False );
                                }
#endif
                                aTmpFont.SetPhysFont( pOutDev );

                                // #114278# Saving both layout mode and language (since I'm
                                // potentially changing both)
                                pOutDev->Push( PUSH_TEXTLAYOUTMODE|PUSH_TEXTLANGUAGE );
                                ImplInitLayoutMode( pOutDev, n, nIndex );
                                ImplInitDigitMode(pOutDev, aTmpFont.GetLanguage());

                                OUString aText;
                                sal_uInt16 nTextStart = 0;
                                sal_uInt16 nTextLen = 0;
                                const sal_Int32* pDXArray = 0;
                                sal_Int32* pTmpDXArray = 0;

                                if ( pTextPortion->GetKind() == PORTIONKIND_TEXT )
                                {
                                    aText = pPortion->GetNode()->GetString();
                                    nTextStart = nIndex;
                                    nTextLen = pTextPortion->GetLen();
                                    if (!pLine->GetCharPosArray().empty())
                                        pDXArray = &pLine->GetCharPosArray()[0]+( nIndex-pLine->GetStart() );

                                    // Paint control characters (#i55716#)
                                    if ( aStatus.MarkFields() )
                                    {
                                        xub_StrLen nTmpIdx;
                                        const xub_StrLen nTmpEnd = nTextStart + pTextPortion->GetLen();

                                        for ( nTmpIdx = nTextStart; nTmpIdx <= nTmpEnd ; ++nTmpIdx )
                                        {
                                            const sal_Unicode cChar = ( nTmpIdx != aText.getLength() && ( nTmpIdx != nTextStart || 0 == nTextStart ) ) ?
                                                                        aText[nTmpIdx] :
                                                                        0;

                                            if ( 0x200B == cChar || 0x2060 == cChar )
                                            {
                                                const OUString aBlank( ' ' );
                                                long nHalfBlankWidth = aTmpFont.QuickGetTextSize( pOutDev, aBlank, 0, 1, 0 ).Width() / 2;

                                                const long nAdvanceX = ( nTmpIdx == nTmpEnd ?
                                                                         pTextPortion->GetSize().Width() :
                                                                         pDXArray[ nTmpIdx - nTextStart ] ) - nHalfBlankWidth;
                                                const long nAdvanceY = -pLine->GetMaxAscent();

                                                Point aTopLeftRectPos( aTmpPos );
                                                if ( !IsVertical() )
                                                {
                                                    aTopLeftRectPos.X() += nAdvanceX;
                                                    aTopLeftRectPos.Y() += nAdvanceY;
                                                }
                                                else
                                                {
                                                    aTopLeftRectPos.Y() += nAdvanceX;
                                                    aTopLeftRectPos.X() -= nAdvanceY;
                                                }

                                                Point aBottomRightRectPos( aTopLeftRectPos );
                                                if ( !IsVertical() )
                                                {
                                                    aBottomRightRectPos.X() += 2 * nHalfBlankWidth;
                                                    aBottomRightRectPos.Y() += pLine->GetHeight();
                                                }
                                                else
                                                {
                                                    aBottomRightRectPos.X() -= pLine->GetHeight();
                                                    aBottomRightRectPos.Y() += 2 * nHalfBlankWidth;
                                                }

                                                pOutDev->Push( PUSH_FILLCOLOR );
                                                pOutDev->Push( PUSH_LINECOLOR );
                                                pOutDev->SetFillColor( COL_LIGHTGRAY );
                                                pOutDev->SetLineColor( COL_LIGHTGRAY );

                                                const Rectangle aBackRect( aTopLeftRectPos, aBottomRightRectPos );
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

                                                    const Size aSlashSize = aTmpFont.QuickGetTextSize( pOutDev, aSlash, 0, 1, 0 );
                                                    Point aSlashPos( aTmpPos );
                                                    const long nAddX = nHalfBlankWidth - aSlashSize.Width() / 2;
                                                    if ( !IsVertical() )
                                                    {
                                                        aSlashPos.X() = aTopLeftRectPos.X() + nAddX;
                                                    }
                                                    else
                                                    {
                                                        aSlashPos.Y() = aTopLeftRectPos.Y() + nAddX;
                                                    }

                                                    aTmpFont.QuickDrawText( pOutDev, aSlashPos, aSlash, 0, 1, 0 );

                                                    aTmpFont.SetEscapement( nOldEscapement );
                                                    aTmpFont.SetPropr( nOldPropr );
                                                    aTmpFont.SetPhysFont( pOutDev );
                                                }
                                            }
                                        }
                                    }
                                }
                                else if ( pTextPortion->GetKind() == PORTIONKIND_FIELD )
                                {
                                    const EditCharAttrib* pAttr = pPortion->GetNode()->GetCharAttribs().FindFeature(nIndex);
                                    DBG_ASSERT( pAttr, "Field not found");
                                    DBG_ASSERT( pAttr && pAttr->GetItem()->ISA( SvxFieldItem ), "Field of the wrong type! ");
                                    aText = ((EditCharAttribField*)pAttr)->GetFieldValue();
                                    nTextStart = 0;
                                    nTextLen = aText.getLength();
                                    ExtraPortionInfo *pExtraInfo = pTextPortion->GetExtraInfos();
                                    // Do not split the Fields into different lines while editing
                                    if( bStripOnly && !bParsingFields && pExtraInfo && pExtraInfo->lineBreaksList.size() )
                                    {
                                        bParsingFields = true;
                                        itSubLines = pExtraInfo->lineBreaksList.begin();
                                    }
                                    if( bParsingFields )
                                    {
                                        if( itSubLines != pExtraInfo->lineBreaksList.begin() )
                                        {
                                            if ( !IsVertical() )
                                            {
                                                aStartPos.Y() += pLine->GetMaxAscent();
                                                aTmpPos.Y() += pLine->GetHeight();
                                            }
                                            else
                                            {
                                                aTmpPos.X() -= pLine->GetMaxAscent();
                                                aStartPos.X() -= pLine->GetHeight();
                                            }
                                        }
                                        ::std::vector< sal_Int32 >::iterator curIt = itSubLines;
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

                                    pTmpDXArray = new sal_Int32[ aText.getLength() ];
                                    pDXArray = pTmpDXArray;
                                    Font _aOldFont( GetRefDevice()->GetFont() );
                                    aTmpFont.SetPhysFont( GetRefDevice() );
                                    aTmpFont.QuickGetTextSize( GetRefDevice(), aText, nTextStart, nTextLen, pTmpDXArray );
                                    if ( aStatus.DoRestoreFont() )
                                        GetRefDevice()->SetFont( _aOldFont );

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
                                else if ( pTextPortion->GetKind() == PORTIONKIND_HYPHENATOR )
                                {
                                    if ( pTextPortion->GetExtraValue() )
                                        aText = OUString(pTextPortion->GetExtraValue());
                                    aText += OUString(CH_HYPH);
                                    nTextStart = 0;
                                    nTextLen = aText.getLength();

                                    // crash when accessing 0 pointer in pDXArray
                                    pTmpDXArray = new sal_Int32[ aText.getLength() ];
                                    pDXArray = pTmpDXArray;
                                    Font _aOldFont( GetRefDevice()->GetFont() );
                                    aTmpFont.SetPhysFont( GetRefDevice() );
                                    aTmpFont.QuickGetTextSize( GetRefDevice(), aText, 0, aText.getLength(), pTmpDXArray );
                                    if ( aStatus.DoRestoreFont() )
                                        GetRefDevice()->SetFont( _aOldFont );
                                }

                                long nTxtWidth = pTextPortion->GetSize().Width();

                                Point aOutPos( aTmpPos );
                                aRedLineTmpPos = aTmpPos;
                                // In RTL portions spell markup pos should be at the start of the
                                // first chara as well. That is on the right end of the portion
                                if (pTextPortion->IsRightToLeft())
                                    aRedLineTmpPos.X() += pTextPortion->GetSize().Width();

                                if ( bStripOnly )
                                {
                                    EEngineData::WrongSpellVector aWrongSpellVector;

                                    if(GetStatus().DoOnlineSpelling() && pTextPortion->GetLen())
                                    {
                                        WrongList* pWrongs = pPortion->GetNode()->GetWrongList();

                                        if(pWrongs && !pWrongs->empty())
                                        {
                                            sal_uInt16 nStart(nIndex);
                                            sal_uInt16 nEnd(0);
                                            sal_Bool bWrong(pWrongs->NextWrong(nStart, nEnd));
                                            const sal_uInt16 nMaxEnd(nIndex + pTextPortion->GetLen());

                                            while(bWrong)
                                            {
                                                if(nStart >= nMaxEnd)
                                                {
                                                    break;
                                                }

                                                if(nStart < nIndex)
                                                {
                                                    nStart = nIndex;
                                                }

                                                if(nEnd > nMaxEnd)
                                                {
                                                    nEnd = nMaxEnd;
                                                }

                                                // add to vector
                                                aWrongSpellVector.push_back(EEngineData::WrongSpellClass(nStart, nEnd));

                                                // goto next index
                                                nStart = nEnd + 1;

                                                if(nEnd < nMaxEnd)
                                                {
                                                    bWrong = pWrongs->NextWrong(nStart, nEnd);
                                                }
                                                else
                                                {
                                                    bWrong = sal_False;
                                                }
                                            }
                                        }
                                    }

                                    const SvxFieldData* pFieldData = 0;

                                    if(PORTIONKIND_FIELD == pTextPortion->GetKind())
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
                                        aTmpFont, n, nIndex, pTextPortion->GetRightToLeft(),
                                        aWrongSpellVector.size() ? &aWrongSpellVector : 0,
                                        pFieldData,
                                        bEndOfLine, bEndOfParagraph, false, // support for EOL/EOP TEXT comments
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
                                            long nDiff = aTmpFont.GetSize().Height() * aTmpFont.GetEscapement() / 100L;
                                            if ( !IsVertical() )
                                                aOutPos.Y() -= nDiff;
                                            else
                                                aOutPos.X() += nDiff;
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
                                        if ( nEsc && ( ( aTmpFont.GetUnderline() != UNDERLINE_NONE ) ) )
                                        {
                                            // Paint the high/low without underline,
                                            // Display the Underline on the
                                            // base line of the original font height ...
                                            // But only if there was something underlined before!
                                            sal_Bool bSpecialUnderline = sal_False;
                                            EditCharAttrib* pPrev = pPortion->GetNode()->GetCharAttribs().FindAttrib( EE_CHAR_ESCAPEMENT, nIndex );
                                            if ( pPrev )
                                            {
                                                SvxFont aDummy;
                                                // Underscore in front?
                                                if ( pPrev->GetStart() )
                                                {
                                                    SeekCursor( pPortion->GetNode(), pPrev->GetStart(), aDummy );
                                                    if ( aDummy.GetUnderline() != UNDERLINE_NONE )
                                                        bSpecialUnderline = sal_True;
                                                }
                                                if ( !bSpecialUnderline && ( pPrev->GetEnd() < pPortion->GetNode()->Len() ) )
                                                {
                                                    SeekCursor( pPortion->GetNode(), pPrev->GetEnd()+1, aDummy );
                                                    if ( aDummy.GetUnderline() != UNDERLINE_NONE )
                                                        bSpecialUnderline = sal_True;
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
                                                comphelper::string::padToLength( aBlanks, (sal_Int32) nTextLen, ' ' );
                                                Point aUnderlinePos( aOutPos );
                                                if ( nOrientation )
                                                    aUnderlinePos = lcl_ImplCalcRotatedPos( aTmpPos, aOrigin, nSin, nCos );
                                                pOutDev->DrawStretchText( aUnderlinePos, aSz.Width(), aBlanks.makeStringAndClear(), 0, nTextLen );

                                                aTmpFont.SetUnderline( UNDERLINE_NONE );
                                                if ( !nOrientation )
                                                    aTmpFont.SetEscapement( nEsc );
                                                aTmpFont.SetPropr( nProp );
                                                aTmpFont.SetPhysFont( pOutDev );
                                            }
                                        }
                                        Point aRealOutPos( aOutPos );
                                        if ( ( pTextPortion->GetKind() == PORTIONKIND_TEXT )
                                               && pTextPortion->GetExtraInfos() && pTextPortion->GetExtraInfos()->bCompressed
                                               && pTextPortion->GetExtraInfos()->bFirstCharIsRightPunktuation )
                                        {
                                            aRealOutPos.X() += pTextPortion->GetExtraInfos()->nPortionOffsetX;
                                        }

                                        // RTL portions with (#i37132#)
                                        // compressed blank should not paint this blank:
                                        if ( pTextPortion->IsRightToLeft() && nTextLen >= 2 &&
                                             pDXArray[ nTextLen - 1 ] ==
                                             pDXArray[ nTextLen - 2 ] &&
                                             ' ' == aText[nTextStart + nTextLen - 1] )
                                            --nTextLen;

                                        // output directly
                                        aTmpFont.QuickDrawText( pOutDev, aRealOutPos, aText, nTextStart, nTextLen, pDXArray );

                                        if ( bDrawFrame )
                                        {
                                            Point aTopLeft( aTmpPos );
                                            aTopLeft.Y() -= pLine->GetMaxAscent();
                                            if ( nOrientation )
                                                aTopLeft = lcl_ImplCalcRotatedPos( aTopLeft, aOrigin, nSin, nCos );
                                            Rectangle aRect( aTopLeft, pTextPortion->GetSize() );
                                            pOutDev->DrawRect( aRect );
                                        }

                                        // PDF export:
                                        if ( pPDFExtOutDevData )
                                        {
                                            if ( pTextPortion->GetKind() == PORTIONKIND_FIELD )
                                            {
                                                const EditCharAttrib* pAttr = pPortion->GetNode()->GetCharAttribs().FindFeature(nIndex);
                                                const SvxFieldItem* pFieldItem = dynamic_cast<const SvxFieldItem*>(pAttr->GetItem());
                                                if( pFieldItem )
                                                {
                                                    const SvxFieldData* pFieldData = pFieldItem->GetField();
                                                    if ( pFieldData->ISA( SvxURLField ) )
                                                    {
                                                        Point aTopLeft( aTmpPos );
                                                        aTopLeft.Y() -= pLine->GetMaxAscent();

                                                        Rectangle aRect( aTopLeft, pTextPortion->GetSize() );
                                                        vcl::PDFExtOutDevBookmarkEntry aBookmark;
                                                        aBookmark.nLinkId = pPDFExtOutDevData->CreateLink( aRect );
                                                        aBookmark.aBookmark = ((SvxURLField*)pFieldData)->GetURL();
                                                        std::vector< vcl::PDFExtOutDevBookmarkEntry >& rBookmarks = pPDFExtOutDevData->GetBookmarks();
                                                        rBookmarks.push_back( aBookmark );
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    if ( GetStatus().DoOnlineSpelling() && !pPortion->GetNode()->GetWrongList()->empty() && pTextPortion->GetLen() )
                                    {
                                        {//#105750# adjust LinePos for superscript or subscript text
                                            short _nEsc = aTmpFont.GetEscapement();
                                            if( _nEsc )
                                            {
                                                long nShift = ((_nEsc*long(aTmpFont.GetSize().Height()))/ 100L);
                                                if( !IsVertical() )
                                                    aRedLineTmpPos.Y() -= nShift;
                                                else
                                                    aRedLineTmpPos.X() += nShift;
                                            }
                                        }
                                        Color aOldColor( pOutDev->GetLineColor() );
                                        pOutDev->SetLineColor( Color( GetColorConfig().GetColorValue( svtools::SPELL ).nColor ) );
                                        lcl_DrawRedLines( pOutDev, aTmpFont.GetSize().Height(), aRedLineTmpPos, nIndex, nIndex + pTextPortion->GetLen(), pDXArray, pPortion->GetNode()->GetWrongList(), nOrientation, aOrigin, IsVertical(), pTextPortion->IsRightToLeft() );
                                        pOutDev->SetLineColor( aOldColor );
                                    }
                                }

                                pOutDev->Pop();

                                //The C++ language guarantees that delete p will do nothing if p is equal to NULL.
                                delete[] pTmpDXArray;

                                if ( pTextPortion->GetKind() == PORTIONKIND_FIELD )
                                {
                                    const EditCharAttrib* pAttr = pPortion->GetNode()->GetCharAttribs().FindFeature(nIndex);
                                    DBG_ASSERT( pAttr, "Field not found" );
                                    DBG_ASSERT( pAttr && pAttr->GetItem()->ISA( SvxFieldItem ), "Wrong type of field!" );

                                    // add a meta file comment if we record to a metafile
                                    if( bMetafileValid )
                                    {
                                        const SvxFieldItem* pFieldItem = dynamic_cast<const SvxFieldItem*>(pAttr->GetItem());

                                        if( pFieldItem )
                                        {
                                            const SvxFieldData* pFieldData = pFieldItem->GetField();
                                            if( pFieldData )
                                                pMtf->AddAction( pFieldData->createEndComment() );
                                        }
                                    }

                                }

                            }
                            break;
                            case PORTIONKIND_TAB:
                            {
                                if ( pTextPortion->GetExtraValue() && ( pTextPortion->GetExtraValue() != ' ' ) )
                                {
                                    SeekCursor( pPortion->GetNode(), nIndex+1, aTmpFont, pOutDev );
                                    aTmpFont.SetTransparent( sal_False );
                                    aTmpFont.SetEscapement( 0 );
                                    aTmpFont.SetPhysFont( pOutDev );
                                    long nCharWidth = aTmpFont.QuickGetTextSize( pOutDev,
                                        OUString(pTextPortion->GetExtraValue()), 0, 1, NULL ).Width();
                                    sal_Int32 nChars = 2;
                                    if( nCharWidth )
                                        nChars = pTextPortion->GetSize().Width() / nCharWidth;
                                    if ( nChars < 2 )
                                        nChars = 2; // is compressed by DrawStretchText.
                                    else if ( nChars == 2 )
                                        nChars = 3; // looks better

                                    OUStringBuffer aBuf;
                                    comphelper::string::padToLength(aBuf, nChars, pTextPortion->GetExtraValue());
                                    OUString aText(aBuf.makeStringAndClear());
                                    aTmpFont.QuickDrawText( pOutDev, aTmpPos, aText, 0, aText.getLength(), NULL );
                                    pOutDev->DrawStretchText( aTmpPos, pTextPortion->GetSize().Width(), aText );

                                    if ( bStripOnly )
                                    {
                                        // create EOL and EOP bools
                                        const bool bEndOfLine(nPortion == pLine->GetEndPortion());
                                        const bool bEndOfParagraph(bEndOfLine && nLine + 1 == nLines);

                                        const Color aOverlineColor(pOutDev->GetOverlineColor());
                                        const Color aTextLineColor(pOutDev->GetTextLineColor());

                                        // StripPortions() data callback
                                        GetEditEnginePtr()->DrawingTab( aTmpPos,
                                            pTextPortion->GetSize().Width(),
                                            OUString(pTextPortion->GetExtraValue()),
                                            aTmpFont, n, nIndex, pTextPortion->GetRightToLeft(),
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
                                        aTmpPos, String(), 0, 0, 0,
                                        aTmpFont, n, nIndex, 0,
                                        0,
                                        0,
                                        bEndOfLine, bEndOfParagraph, false,
                                        0,
                                        aOverlineColor,
                                        aTextLineColor);
                                }
                            }
                            break;
                        }
                        if( bParsingFields )
                            nPortion--;
                        else
                            nIndex = nIndex + pTextPortion->GetLen();

                    }
                }

                if ( ( nLine != nLastLine ) && !aStatus.IsOutliner() )
                {
                    if ( !IsVertical() )
                        aStartPos.Y() += nSBL;
                    else
                        aStartPos.X() -= nSBL;
                }

                // no more visible actions?
                if ( !IsVertical() && ( aStartPos.Y() >= aClipRect.Bottom() ) )
                    break;
                else if ( IsVertical() && ( aStartPos.X() <= aClipRect.Left() ) )
                    break;
            }

            if ( !aStatus.IsOutliner() )
            {
                const SvxULSpaceItem& rULItem = (const SvxULSpaceItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_ULSPACE );
                long nUL = GetYValue( rULItem.GetLower() );
                if ( !IsVertical() )
                    aStartPos.Y() += nUL;
                else
                    aStartPos.X() -= nUL;
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
                    aTmpPos, String(), 0, 0, 0,
                    aTmpFont, n, nIndex, 0,
                    0,
                    0,
                    false, true, false, // support for EOL/EOP TEXT comments
                    0,
                    aOverlineColor,
                    aTextLineColor);
            }
        }
        else
        {
            if ( !IsVertical() )
                aStartPos.Y() += nParaHeight;
            else
                aStartPos.X() -= nParaHeight;
        }

        if ( pPDFExtOutDevData )
            pPDFExtOutDevData->EndStructureElement();

        // no more visible actions?
        if ( !IsVertical() && ( aStartPos.Y() > aClipRect.Bottom() ) )
            break;
        if ( IsVertical() && ( aStartPos.X() < aClipRect.Left() ) )
            break;
    }
    if ( aStatus.DoRestoreFont() )
        pOutDev->SetFont( aOldFont );
}

void ImpEditEngine::Paint( ImpEditView* pView, const Rectangle& rRect, OutputDevice* pTargetDevice, sal_Bool bUseVirtDev )
{
    DBG_ASSERT( pView, "No View - No Paint!" );
    DBG_CHKOBJ( GetEditEnginePtr(), EditEngine, 0 );

    if ( !GetUpdateMode() || IsInUndo() )
        return;

    // Intersection of paint area and output area.
    Rectangle aClipRect( pView->GetOutputArea() );
    aClipRect.Intersection( rRect );

    OutputDevice* pTarget = pTargetDevice ? pTargetDevice : pView->GetWindow();

    if ( bUseVirtDev )
    {
        Rectangle aClipRecPixel( pTarget->LogicToPixel( aClipRect ) );
        if ( !IsVertical() )
        {
            // etwas mehr, falls abgerundet!
            aClipRecPixel.Right() += 1;
            aClipRecPixel.Bottom() += 1;
        }
        else
        {
            aClipRecPixel.Left() -= 1;
            aClipRecPixel.Bottom() += 1;
        }

        // If aClipRecPixel > XXXX, then invalidate?!

        VirtualDevice* pVDev = GetVirtualDevice( pTarget->GetMapMode(), pTarget->GetDrawMode() );
        pVDev->SetDigitLanguage( GetRefDevice()->GetDigitLanguage() );

        {
            Color aBackgroundColor( pView->GetBackgroundColor() );
            // #i47161# Check if text is visible on background
            SvxFont aTmpFont;
            ContentNode* pNode = GetEditDoc().GetObject( 0 );
            SeekCursor( pNode, 1, aTmpFont );
            Color aFontColor( aTmpFont.GetColor() );
            if( (aFontColor == COL_AUTO) || IsForceAutoColor() )
                aFontColor = GetAutoColor();

            // #i69346# check for reverse color of input method attribute
            if( mpIMEInfos && (mpIMEInfos->aPos.GetNode() == pNode &&
                mpIMEInfos->pAttribs))
            {
                sal_uInt16 nAttr = mpIMEInfos->pAttribs[ 0 ];
                if ( nAttr & EXTTEXTINPUT_ATTR_HIGHLIGHT )
                {
                    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
                    aFontColor = rStyleSettings.GetHighlightColor() ;
                }
            }

            sal_uInt8 nColorDiff = aFontColor.GetColorError( aBackgroundColor );
            if( nColorDiff < 8 )
                aBackgroundColor = aFontColor.IsDark() ? COL_WHITE : COL_BLACK;
            pVDev->SetBackground( aBackgroundColor );
        }

        sal_Bool bVDevValid = sal_True;
        Size aOutSz( pVDev->GetOutputSizePixel() );
        if ( (  aOutSz.Width() < aClipRecPixel.GetWidth() ) ||
             (  aOutSz.Height() < aClipRecPixel.GetHeight() ) )
        {
            bVDevValid = pVDev->SetOutputSizePixel( aClipRecPixel.GetSize() );
        }
        else
        {
            // The VirtDev can become very big during a Resize =>
            // eventually make it smaller!
            if ( ( aOutSz.Height() > ( aClipRecPixel.GetHeight() + RESDIFF ) ) ||
                 ( aOutSz.Width() > ( aClipRecPixel.GetWidth() + RESDIFF ) ) )
            {
                bVDevValid = pVDev->SetOutputSizePixel( aClipRecPixel.GetSize() );
            }
            else
            {
                pVDev->Erase();
            }
        }
        DBG_ASSERT( bVDevValid, "VDef could not be enlarged!" );
        if ( !bVDevValid )
        {
            Paint( pView, rRect, 0, sal_False /* ohne VDev */ );
            return;
        }

        // PaintRect for VDev not with aligned size,
        // Otherwise, the line below must also be printed out:
        Rectangle aTmpRect( Point( 0, 0 ), aClipRect.GetSize() );

        aClipRect = pTarget->PixelToLogic( aClipRecPixel );
        Point aStartPos;
        if ( !IsVertical() )
        {
            aStartPos = aClipRect.TopLeft();
            aStartPos = pView->GetDocPos( aStartPos );
            aStartPos.X() *= (-1);
            aStartPos.Y() *= (-1);
        }
        else
        {
            aStartPos = aClipRect.TopRight();
            Point aDocPos( pView->GetDocPos( aStartPos ) );
            aStartPos.X() = aClipRect.GetSize().Width() + aDocPos.Y();
            aStartPos.Y() = -aDocPos.X();
        }

        Paint( pVDev, aTmpRect, aStartPos );

        sal_Bool bClipRegion = sal_False;
        Region aOldRegion;
        MapMode aOldMapMode;
        if ( GetTextRanger() )
        {
            // Some problems here with push/pop, why?!
//          pTarget->Push( PUSH_CLIPREGION|PUSH_MAPMODE );
            bClipRegion = pTarget->IsClipRegion();
            aOldRegion = pTarget->GetClipRegion();
            // How do I get the polygon to the right place??
            // The polygon is based on the view, not the Window
            // => reset origin...
            aOldMapMode = pTarget->GetMapMode();
            Point aOrigin = aOldMapMode.GetOrigin();
            Point aViewPos = pView->GetOutputArea().TopLeft();
            aOrigin.Move( aViewPos.X(), aViewPos.Y() );
            aClipRect.Move( -aViewPos.X(), -aViewPos.Y() );
            MapMode aNewMapMode( aOldMapMode );
            aNewMapMode.SetOrigin( aOrigin );
            pTarget->SetMapMode( aNewMapMode );
            pTarget->SetClipRegion( Region( GetTextRanger()->GetPolyPolygon() ) );
        }

        pTarget->DrawOutDev( aClipRect.TopLeft(), aClipRect.GetSize(),
                            Point(0,0), aClipRect.GetSize(), *pVDev );

        if ( GetTextRanger() )
        {
//          pTarget->Pop();
            if ( bClipRegion )
                pTarget->SetClipRegion( aOldRegion );
            else
                pTarget->SetClipRegion();
            pTarget->SetMapMode( aOldMapMode );
        }


        pView->DrawSelection(pView->GetEditSelection(), 0, pTarget);
    }
    else
    {
        Point aStartPos;
        if ( !IsVertical() )
        {
            aStartPos = pView->GetOutputArea().TopLeft();
            aStartPos.X() -= pView->GetVisDocLeft();
            aStartPos.Y() -= pView->GetVisDocTop();
        }
        else
        {
            aStartPos = pView->GetOutputArea().TopRight();
            aStartPos.X() += pView->GetVisDocTop();
            aStartPos.Y() -= pView->GetVisDocLeft();
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
                aClipRect.Right() = nMaxX;
        }

        sal_Bool bClipRegion = pTarget->IsClipRegion();
        Region aOldRegion = pTarget->GetClipRegion();
        pTarget->IntersectClipRegion( aClipRect );

        Paint( pTarget, aClipRect, aStartPos );

        if ( bClipRegion )
            pTarget->SetClipRegion( aOldRegion );
        else
            pTarget->SetClipRegion();

        pView->DrawSelection(pView->GetEditSelection(), 0, pTarget);
    }

}

void ImpEditEngine::InsertContent( ContentNode* pNode, sal_Int32 nPos )
{
    DBG_ASSERT( pNode, "NULL-Pointer in InsertContent! " );
    DBG_ASSERT( IsInUndo(), "InsertContent only for Undo()!" );
    ParaPortion* pNew = new ParaPortion( pNode );
    GetParaPortions().Insert(nPos, pNew);
    aEditDoc.Insert(nPos, pNode);
    if ( IsCallParaInsertedOrDeleted() )
        GetEditEnginePtr()->ParagraphInserted( nPos );
}

EditPaM ImpEditEngine::SplitContent( sal_Int32 nNode, sal_uInt16 nSepPos )
{
    ContentNode* pNode = aEditDoc.GetObject( nNode );
    DBG_ASSERT( pNode, "Invalid Node in SplitContent" );
    DBG_ASSERT( IsInUndo(), "SplitContent only for Undo()!" );
    DBG_ASSERT( nSepPos <= pNode->Len(), "Index out of range: SplitContent" );
    EditPaM aPaM( pNode, nSepPos );
    return ImpInsertParaBreak( aPaM );
}

EditPaM ImpEditEngine::ConnectContents( sal_Int32 nLeftNode, sal_Bool bBackward )
{
    ContentNode* pLeftNode = aEditDoc.GetObject( nLeftNode );
    ContentNode* pRightNode = aEditDoc.GetObject( nLeftNode+1 );
    DBG_ASSERT( pLeftNode, "Invalid left node in ConnectContents ");
    DBG_ASSERT( pRightNode, "Invalid right node in ConnectContents ");
    DBG_ASSERT( IsInUndo(), "ConnectContent only for Undo()!" );
    return ImpConnectParagraphs( pLeftNode, pRightNode, bBackward );
}

void ImpEditEngine::SetUpdateMode( bool bUp, EditView* pCurView, sal_Bool bForceUpdate )
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
            DeletedNodeInfo* pDelInfo = new DeletedNodeInfo( (sal_uIntPtr)pPPortion->GetNode(), nParagraph );
            aDeletedNodes.push_back(pDelInfo);
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
                    Font aOldFont( GetRefDevice()->GetFont() );
                    CreateLines( nParagraph, 0 );   // 0: No TextRanger
                    if ( aStatus.DoRestoreFont() )
                        GetRefDevice()->SetFont( aOldFont );
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

        pPPortion->SetMustRepaint( sal_True );
        if ( GetUpdateMode() && !IsInUndo() && !GetTextRanger() )
        {
            aInvalidRect = Rectangle(    Point( 0, GetParaPortions().GetYOffset( pPPortion ) ),
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
    if ( pCurView && ( GetUpdateMode() == sal_True ) )
    {
        // in this case one can redraw directly without invalidating the
        // Portions
        sal_Int32 nFirstPortion = std::min( static_cast<sal_Int32>(aOldPositions.Min()), nNewPos );
        sal_Int32 nLastPortion = std::max( static_cast<sal_Int32>(aOldPositions.Max()), nNewPos );

        ParaPortion* pUpperPortion = GetParaPortions().SafeGetObject( nFirstPortion );
        ParaPortion* pLowerPortion = GetParaPortions().SafeGetObject( nLastPortion );

        aInvalidRect = Rectangle();  // make empty
        aInvalidRect.Left() = 0;
        aInvalidRect.Right() = aPaperSize.Width();
        aInvalidRect.Top() = GetParaPortions().GetYOffset( pUpperPortion );
        aInvalidRect.Bottom() = GetParaPortions().GetYOffset( pLowerPortion ) + pLowerPortion->GetHeight();

        UpdateViews( pCurView );
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
        pTmpPortion->MarkSelectionInvalid( 0, pTmpPortion->GetNode()->Len() );
    }
    pTmpPortion->ResetHeight();
}

IMPL_LINK_NOARG_INLINE_START(ImpEditEngine, StatusTimerHdl)
{
    CallStatusHdl();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(ImpEditEngine, StatusTimerHdl)

void ImpEditEngine::CallStatusHdl()
{
    if ( aStatusHdlLink.IsSet() && aStatus.GetStatusWord() )
    {
        // The Status has to be reset before the Call,
        // since other Flags might be set in the handler...
        EditStatus aTmpStatus( aStatus );
        aStatus.Clear();
        aStatusHdlLink.Call( &aTmpStatus );
        aStatusTimer.Stop();    // If called by hand ...
    }
}

ContentNode* ImpEditEngine::GetPrevVisNode( ContentNode* pCurNode )
{
    const ParaPortion* pPortion = FindParaPortion( pCurNode );
    DBG_ASSERT( pPortion, "GetPrevVisibleNode: No matching portion!" );
    pPortion = GetPrevVisPortion( pPortion );
    if ( pPortion )
        return pPortion->GetNode();
    return 0;
}

ContentNode* ImpEditEngine::GetNextVisNode( ContentNode* pCurNode )
{
    const ParaPortion* pPortion = FindParaPortion( pCurNode );
    DBG_ASSERT( pPortion, "GetNextVisibleNode: No matching portion!" );
    pPortion = GetNextVisPortion( pPortion );
    if ( pPortion )
        return pPortion->GetNode();
    return 0;
}

const ParaPortion* ImpEditEngine::GetPrevVisPortion( const ParaPortion* pCurPortion ) const
{
    sal_Int32 nPara = GetParaPortions().GetPos( pCurPortion );
    DBG_ASSERT( nPara < GetParaPortions().Count() , "Portion not found: GetPrevVisPortion" );
    const ParaPortion* pPortion = nPara ? GetParaPortions()[--nPara] : 0;
    while ( pPortion && !pPortion->IsVisible() )
        pPortion = nPara ? GetParaPortions()[--nPara] : 0;

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
    sal_uInt16 nTotalLineCount = 0;
    const ParaPortionList& rParaPortions = GetParaPortions();
    sal_Int32 nParaCount = rParaPortions.Count();

    for (sal_Int32 i = 0; i < nParaCount; ++i)
    {
        if (GetVerJustification(i) != SVX_VER_JUSTIFY_BLOCK)
            // All paragraphs must have the block justification set.
            return 0;

        const ParaPortion* pPortion = rParaPortions[i];
        nTotalOccupiedHeight += pPortion->GetFirstLineOffset();

        const SvxLineSpacingItem& rLSItem = (const SvxLineSpacingItem&)pPortion->GetNode()->GetContentAttribs().GetItem(EE_PARA_SBL);
        sal_uInt16 nSBL = ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_FIX )
                            ? GetYValue( rLSItem.GetInterLineSpace() ) : 0;

        const SvxULSpaceItem& rULItem = (const SvxULSpaceItem&)pPortion->GetNode()->GetContentAttribs().GetItem(EE_PARA_ULSPACE);
        long nUL = GetYValue( rULItem.GetLower() );

        const EditLineList& rLines = pPortion->GetLines();
        sal_uInt16 nLineCount = rLines.Count();
        nTotalLineCount += nLineCount;
        for (sal_uInt16 j = 0; j < nLineCount; ++j)
        {
            const EditLine* pLine = rLines[j];
            nTotalOccupiedHeight += pLine->GetHeight();
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
        // Shift the text to the right for the asian layout mode.
        rStartPos.X() += nTotalSpace;

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
        DBG_ASSERT( pNode, "Not a single paragraph in InsertParagraph ?" );
        aPaM = EditPaM( pNode, pNode->Len() );
    }
    else
    {
        ContentNode* pNode = GetEditDoc().GetObject( 0 );
        aPaM = EditPaM( pNode, 0 );
    }

    return ImpInsertParaBreak( aPaM );
}

EditSelection* ImpEditEngine::SelectParagraph( sal_Int32 nPara )
{
    EditSelection* pSel = 0;
    ContentNode* pNode = GetEditDoc().GetObject( nPara );
    DBG_ASSERTWARNING( pNode, "Paragraph does not exist: SelectParagraph" );
    if ( pNode )
        pSel = new EditSelection( EditPaM( pNode, 0 ), EditPaM( pNode, pNode->Len() ) );

    return pSel;
}

void ImpEditEngine::FormatAndUpdate( EditView* pCurView )
{
    if ( bDowning )
        return ;

    if ( IsInUndo() )
        IdleFormatAndUpdate( pCurView );
    else
    {
        FormatDoc();
        UpdateViews( pCurView );
    }
}

void ImpEditEngine::SetFlatMode( sal_Bool bFlat )
{
    if ( bFlat != aStatus.UseCharAttribs() )
        return;

    if ( !bFlat )
        aStatus.TurnOnFlags( EE_CNTRL_USECHARATTRIBS );
    else
        aStatus.TurnOffFlags( EE_CNTRL_USECHARATTRIBS );

    aEditDoc.CreateDefFont( !bFlat );

    FormatFullDoc();
    UpdateViews( (EditView*) 0);
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
        aInvalidRect=Rectangle(0,0,1000000,1000000);
        UpdateViews( GetActiveView() );
    }
}

const SvxNumberFormat* ImpEditEngine::GetNumberFormat( const ContentNode *pNode ) const
{
    const SvxNumberFormat *pRes = 0;

    if (pNode)
    {
        // get index of paragraph
        sal_Int32 nPara = GetEditDoc().GetPos( const_cast< ContentNode * >(pNode) );
        DBG_ASSERT( nPara < EE_PARA_NOT_FOUND, "node not found in array" );
        if (nPara < EE_PARA_NOT_FOUND)
        {
            // the called function may be overloaded by an OutlinerEditEng object to provide
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
    // nSpaceBefore     matches the ODF attribut text:space-before
    // nMinLabelWidth   matches the ODF attribut text:min-label-width

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
    return (const SvxLRSpaceItem&)pNode->GetContentAttribs().GetItem( aStatus.IsOutliner() ? EE_PARA_OUTLLRSPACE : EE_PARA_LRSPACE );
}

// select a representative text language for the digit type according to the
// text numeral setting:
LanguageType ImpEditEngine::ImplCalcDigitLang(LanguageType eCurLang) const
{
    // #114278# Also setting up digit language from Svt options
    // (cannot reliably inherit the outdev's setting)
    if( !pCTLOptions )
        pCTLOptions = new SvtCTLOptions;

    LanguageType eLang = eCurLang;
    const SvtCTLOptions::TextNumerals nCTLTextNumerals = pCTLOptions->GetCTLTextNumerals();

    if ( SvtCTLOptions::NUMERALS_HINDI == nCTLTextNumerals )
        eLang = LANGUAGE_ARABIC_SAUDI_ARABIA;
    else if ( SvtCTLOptions::NUMERALS_ARABIC == nCTLTextNumerals )
        eLang = LANGUAGE_ENGLISH;
    else if ( SvtCTLOptions::NUMERALS_SYSTEM == nCTLTextNumerals )
        eLang = (LanguageType) Application::GetSettings().GetLanguageTag().getLanguageType();

    return eLang;
}

OUString ImpEditEngine::convertDigits(const OUString &rString, sal_Int32 nStt, sal_Int32 nLen, LanguageType eDigitLang) const
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
    assert(pOutDev); //persumably there isn't any case where pOutDev should be NULL ?
    if (pOutDev)
        pOutDev->SetDigitLanguage(ImplCalcDigitLang(eCurLang));
}

void ImpEditEngine::ImplInitLayoutMode( OutputDevice* pOutDev, sal_Int32 nPara, sal_uInt16 nIndex )
{
    sal_Bool bCTL = sal_False;
    sal_Bool bR2L = sal_False;
    if ( nIndex == 0xFFFF )
    {
        bCTL = HasScriptType( nPara, i18n::ScriptType::COMPLEX );
        bR2L = IsRightToLeft( nPara );
    }
    else
    {
        ContentNode* pNode = GetEditDoc().GetObject( nPara );
        short nScriptType = GetScriptType( EditPaM( pNode, nIndex+1 ) );
        bCTL = nScriptType == i18n::ScriptType::COMPLEX;
        // this change was discussed in issue 37190
        bR2L = GetRightToLeft( nPara, nIndex + 1) % 2 ? sal_True : sal_False;
        // it also works for issue 55927
    }

    sal_uLong nLayoutMode = pOutDev->GetLayoutMode();

    // We always use the left postion for DrawText()
    nLayoutMode &= ~(TEXT_LAYOUT_BIDI_RTL);

    if ( !bCTL && !bR2L)
    {
        // No CTL/Bidi checking necessary
        nLayoutMode |= ( TEXT_LAYOUT_COMPLEX_DISABLED | TEXT_LAYOUT_BIDI_STRONG );
    }
    else
    {
        // CTL/Bidi checking neccessary
        // Don't use BIDI_STRONG, VCL must do some checks.
        nLayoutMode &= ~( TEXT_LAYOUT_COMPLEX_DISABLED | TEXT_LAYOUT_BIDI_STRONG );

        if ( bR2L )
            nLayoutMode |= TEXT_LAYOUT_BIDI_RTL|TEXT_LAYOUT_TEXTORIGIN_LEFT;
    }

    pOutDev->SetLayoutMode( nLayoutMode );

    // #114278# Also setting up digit language from Svt options
    // (cannot reliably inherit the outdev's setting)
    LanguageType eLang = (LanguageType) Application::GetSettings().GetLanguageTag().getLanguageType();
    ImplInitDigitMode( pOutDev, eLang );
}

Reference < i18n::XBreakIterator > ImpEditEngine::ImplGetBreakIterator() const
{
    if ( !xBI.is() )
    {
        Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
        xBI = i18n::BreakIterator::create( xContext );
    }
    return xBI;
}

Reference < i18n::XExtendedInputSequenceChecker > ImpEditEngine::ImplGetInputSequenceChecker() const
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
    Color aColor = const_cast<ImpEditEngine*>(this)->GetColorConfig().GetColorValue( svtools::FONTCOLOR ).nColor;

    if ( GetBackgroundColor() != COL_AUTO )
    {
        if ( GetBackgroundColor().IsDark() && aColor.IsDark() )
            aColor = COL_WHITE;
        else if ( GetBackgroundColor().IsBright() && aColor.IsBright() )
            aColor = COL_BLACK;
    }

    return aColor;
}


sal_Bool ImpEditEngine::ImplCalcAsianCompression( ContentNode* pNode, TextPortion* pTextPortion, sal_uInt16 nStartPos, sal_Int32* pDXArray, sal_uInt16 n100thPercentFromMax, sal_Bool bManipulateDXArray )
{
    DBG_ASSERT( GetAsianCompressionMode(), "ImplCalcAsianCompression - Why?" );
    DBG_ASSERT( pTextPortion->GetLen(), "ImplCalcAsianCompression - Empty Portion?" );

    // Percent is 1/100 Percent...
    if ( n100thPercentFromMax == 10000 )
        pTextPortion->SetExtraInfos( NULL );

    sal_Bool bCompressed = sal_False;

    if ( GetScriptType( EditPaM( pNode, nStartPos+1 ) ) == i18n::ScriptType::ASIAN )
    {
        long nNewPortionWidth = pTextPortion->GetSize().Width();
        sal_uInt16 nPortionLen = pTextPortion->GetLen();
        for ( sal_uInt16 n = 0; n < nPortionLen; n++ )
        {
            sal_uInt8 nType = GetCharTypeForCompression( pNode->GetChar( n+nStartPos ) );

            sal_Bool bCompressPunctuation = ( nType == CHAR_PUNCTUATIONLEFT ) || ( nType == CHAR_PUNCTUATIONRIGHT );
            sal_Bool bCompressKana = ( nType == CHAR_KANA ) && ( GetAsianCompressionMode() == text::CharacterCompressionType::PUNCTUATION_AND_KANA );

            // create Extra infos only if needed...
            if ( bCompressPunctuation || bCompressKana )
            {
                if ( !pTextPortion->GetExtraInfos() )
                {
                    ExtraPortionInfo* pExtraInfos = new ExtraPortionInfo;
                    pTextPortion->SetExtraInfos( pExtraInfos );
                    pExtraInfos->nOrgWidth = pTextPortion->GetSize().Width();
                    pExtraInfos->nAsianCompressionTypes = CHAR_NORMAL;
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
                    bCompressed = sal_True;
                    nNewPortionWidth -= nCompress;
                    pTextPortion->GetExtraInfos()->bCompressed = sal_True;


                    // Special handling for rightpunctuation: For the 'compression' we must
                    // start the output before the normal char position....
                    if ( bManipulateDXArray && ( pTextPortion->GetLen() > 1 ) )
                    {
                        if ( !pTextPortion->GetExtraInfos()->pOrgDXArray )
                            pTextPortion->GetExtraInfos()->SaveOrgDXArray( pDXArray, pTextPortion->GetLen()-1 );

                        if ( nType == CHAR_PUNCTUATIONRIGHT )
                        {
                            // If it's the first char, I must handle it in Paint()...
                            if ( n )
                            {
                                // -1: No entry for the last character
                                for ( sal_uInt16 i = n-1; i < (nPortionLen-1); i++ )
                                    pDXArray[i] -= nCompress;
                            }
                            else
                            {
                                pTextPortion->GetExtraInfos()->bFirstCharIsRightPunktuation = sal_True;
                                pTextPortion->GetExtraInfos()->nPortionOffsetX = -nCompress;
                            }
                        }
                        else
                        {
                            // -1: No entry for the last character
                            for ( sal_uInt16 i = n; i < (nPortionLen-1); i++ )
                                pDXArray[i] -= nCompress;
                        }
                    }
                }
            }
        }

        if ( bCompressed && ( n100thPercentFromMax == 10000 ) )
            pTextPortion->GetExtraInfos()->nWidthFullCompression = nNewPortionWidth;

        pTextPortion->GetSize().Width() = nNewPortionWidth;

        if ( pTextPortion->GetExtraInfos() && ( n100thPercentFromMax != 10000 ) )
        {
            // Maybe rounding errors in nNewPortionWidth, assure that width not bigger than expected
            long nShrink = pTextPortion->GetExtraInfos()->nOrgWidth - pTextPortion->GetExtraInfos()->nWidthFullCompression;
            nShrink *= n100thPercentFromMax;
            nShrink /= 10000;
            long nNewWidth = pTextPortion->GetExtraInfos()->nOrgWidth - nShrink;
            if ( nNewWidth < pTextPortion->GetSize().Width() )
            pTextPortion->GetSize().Width() = nNewWidth;
        }
    }
    return bCompressed;
}


void ImpEditEngine::ImplExpandCompressedPortions( EditLine* pLine, ParaPortion* pParaPortion, long nRemainingWidth )
{
    sal_Bool bFoundCompressedPortion = sal_False;
    long nCompressed = 0;
    std::vector<TextPortion*> aCompressedPortions;

    sal_uInt16 nPortion = pLine->GetEndPortion();
    TextPortion* pTP = pParaPortion->GetTextPortions()[ nPortion ];
    while ( pTP && ( pTP->GetKind() == PORTIONKIND_TEXT ) )
    {
        if ( pTP->GetExtraInfos() && pTP->GetExtraInfos()->bCompressed )
        {
            bFoundCompressedPortion = sal_True;
            nCompressed += pTP->GetExtraInfos()->nOrgWidth - pTP->GetSize().Width();
            aCompressedPortions.push_back(pTP);
        }
        pTP = ( nPortion > pLine->GetStartPortion() ) ? pParaPortion->GetTextPortions()[ --nPortion ] : NULL;
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

        for (size_t i = 0, n = aCompressedPortions.size(); i < n; ++i)
        {
            pTP = aCompressedPortions[i];
            pTP->GetExtraInfos()->bCompressed = sal_False;
            pTP->GetSize().Width() = pTP->GetExtraInfos()->nOrgWidth;
            if ( nCompressPercent )
            {
                size_t nTxtPortion = pParaPortion->GetTextPortions().GetPos( pTP );
                sal_uInt16 nTxtPortionStart = pParaPortion->GetTextPortions().GetStartPos( nTxtPortion );
                DBG_ASSERT( nTxtPortionStart >= pLine->GetStart(), "Portion doesn't belong to the line!!!" );
                sal_Int32* pDXArray = NULL;
                if (!pLine->GetCharPosArray().empty())
                    pDXArray = &pLine->GetCharPosArray()[0]+( nTxtPortionStart-pLine->GetStart() );
                if ( pTP->GetExtraInfos()->pOrgDXArray )
                    memcpy( pDXArray, pTP->GetExtraInfos()->pOrgDXArray, (pTP->GetLen()-1)*sizeof(sal_Int32) );
                ImplCalcAsianCompression( pParaPortion->GetNode(), pTP, nTxtPortionStart, pDXArray, (sal_uInt16)nCompressPercent, sal_True );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
