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

#include <sal/config.h>

#include <memory>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/linguistic2/LinguServiceManager.hpp>

#include <comphelper/processfactory.hxx>
#include <osl/file.h>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <tools/lineend.hxx>
#include <tools/debug.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/metric.hxx>
#include <vcl/textrectinfo.hxx>
#include <vcl/virdev.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/unohelp.hxx>

#include <config_fuzzers.h>
#include <ImplLayoutArgs.hxx>
#include <ImplOutDevData.hxx>
#include <drawmode.hxx>
#include <salgdi.hxx>
#include <svdata.hxx>
#include <textlayout.hxx>
#include <textlineinfo.hxx>
#include <impglyphitem.hxx>
#include <optional>
#include <font/PhysicalFontFace.hxx>

#define TEXT_DRAW_ELLIPSIS  (DrawTextFlags::EndEllipsis | DrawTextFlags::PathEllipsis | DrawTextFlags::NewsEllipsis)

void OutputDevice::SetLayoutMode( vcl::text::ComplexTextLayoutFlags nTextLayoutMode )
{
    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaLayoutModeAction( nTextLayoutMode ) );

    mnTextLayoutMode = nTextLayoutMode;

    if( mpAlphaVDev )
        mpAlphaVDev->SetLayoutMode( nTextLayoutMode );
}

void OutputDevice::SetDigitLanguage( LanguageType eTextLanguage )
{
    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextLanguageAction( eTextLanguage ) );

    meTextLanguage = eTextLanguage;

    if( mpAlphaVDev )
        mpAlphaVDev->SetDigitLanguage( eTextLanguage );
}

ImplMultiTextLineInfo::ImplMultiTextLineInfo()
{
}

ImplMultiTextLineInfo::~ImplMultiTextLineInfo()
{
}

void ImplMultiTextLineInfo::AddLine( const ImplTextLineInfo& rLine )
{
    mvLines.push_back(rLine);
}

void ImplMultiTextLineInfo::Clear()
{
    mvLines.clear();
}

void OutputDevice::ImplInitTextColor()
{
    DBG_TESTSOLARMUTEX();

    if ( mbInitTextColor )
    {
        mpGraphics->SetTextColor( GetTextColor() );
        mbInitTextColor = false;
    }
}

void OutputDevice::ImplDrawTextRect( tools::Long nBaseX, tools::Long nBaseY,
                                     tools::Long nDistX, tools::Long nDistY, tools::Long nWidth, tools::Long nHeight )
{
    tools::Long nX = nDistX;
    tools::Long nY = nDistY;

    Degree10 nOrientation = mpFontInstance->mnOrientation;
    if ( nOrientation )
    {
        // Rotate rect without rounding problems for 90 degree rotations
        if ( !(nOrientation % 900_deg10) )
        {
            if ( nOrientation == 900_deg10 )
            {
                tools::Long nTemp = nX;
                nX = nY;
                nY = -nTemp;
                nTemp = nWidth;
                nWidth = nHeight;
                nHeight = nTemp;
                nY -= nHeight;
            }
            else if ( nOrientation == 1800_deg10 )
            {
                nX = -nX;
                nY = -nY;
                nX -= nWidth;
                nY -= nHeight;
            }
            else /* ( nOrientation == 2700 ) */
            {
                tools::Long nTemp = nX;
                nX = -nY;
                nY = nTemp;
                nTemp = nWidth;
                nWidth = nHeight;
                nHeight = nTemp;
                nX -= nWidth;
            }
        }
        else
        {
            nX += nBaseX;
            nY += nBaseY;
            // inflate because polygons are drawn smaller
            tools::Rectangle aRect( Point( nX, nY ), Size( nWidth+1, nHeight+1 ) );
            tools::Polygon   aPoly( aRect );
            aPoly.Rotate( Point( nBaseX, nBaseY ), mpFontInstance->mnOrientation );
            ImplDrawPolygon( aPoly );
            return;
        }
    }

    nX += nBaseX;
    nY += nBaseY;
    mpGraphics->DrawRect( nX, nY, nWidth, nHeight, *this ); // original code

}

void OutputDevice::ImplDrawTextBackground( const SalLayout& rSalLayout )
{
    const tools::Long nWidth = rSalLayout.GetTextWidth() / rSalLayout.GetUnitsPerPixel();
    const Point aBase = rSalLayout.DrawBase();
    const tools::Long nX = aBase.X();
    const tools::Long nY = aBase.Y();

    if ( mbLineColor || mbInitLineColor )
    {
        mpGraphics->SetLineColor();
        mbInitLineColor = true;
    }
    mpGraphics->SetFillColor( GetTextFillColor() );
    mbInitFillColor = true;

    ImplDrawTextRect( nX, nY, 0, -(mpFontInstance->mxFontMetric->GetAscent() + mnEmphasisAscent),
                      nWidth,
                      mpFontInstance->mnLineHeight+mnEmphasisAscent+mnEmphasisDescent );
}

tools::Rectangle OutputDevice::ImplGetTextBoundRect( const SalLayout& rSalLayout ) const
{
    Point aPoint = rSalLayout.GetDrawPosition();
    tools::Long nX = aPoint.X();
    tools::Long nY = aPoint.Y();

    tools::Long nWidth = rSalLayout.GetTextWidth();
    tools::Long nHeight = mpFontInstance->mnLineHeight + mnEmphasisAscent + mnEmphasisDescent;

    nY -= mpFontInstance->mxFontMetric->GetAscent() + mnEmphasisAscent;

    if ( mpFontInstance->mnOrientation )
    {
        tools::Long nBaseX = nX, nBaseY = nY;
        if ( !(mpFontInstance->mnOrientation % 900_deg10) )
        {
            tools::Long nX2 = nX+nWidth;
            tools::Long nY2 = nY+nHeight;

            Point aBasePt( nBaseX, nBaseY );
            aBasePt.RotateAround( nX, nY, mpFontInstance->mnOrientation );
            aBasePt.RotateAround( nX2, nY2, mpFontInstance->mnOrientation );
            nWidth = nX2-nX;
            nHeight = nY2-nY;
        }
        else
        {
            // inflate by +1+1 because polygons are drawn smaller
            tools::Rectangle aRect( Point( nX, nY ), Size( nWidth+1, nHeight+1 ) );
            tools::Polygon   aPoly( aRect );
            aPoly.Rotate( Point( nBaseX, nBaseY ), mpFontInstance->mnOrientation );
            return aPoly.GetBoundRect();
        }
    }

    return tools::Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) );
}

bool OutputDevice::ImplDrawRotateText( SalLayout& rSalLayout )
{
    tools::Long nX = rSalLayout.DrawBase().X();
    tools::Long nY = rSalLayout.DrawBase().Y();

    tools::Rectangle aBoundRect;
    rSalLayout.DrawBase() = Point( 0, 0 );
    rSalLayout.DrawOffset() = Point( 0, 0 );
    if (!rSalLayout.GetBoundRect(aBoundRect))
    {
        // guess vertical text extents if GetBoundRect failed
        tools::Long nRight = rSalLayout.GetTextWidth();
        tools::Long nTop = mpFontInstance->mxFontMetric->GetAscent() + mnEmphasisAscent;
        tools::Long nHeight = mpFontInstance->mnLineHeight + mnEmphasisAscent + mnEmphasisDescent;
        aBoundRect = tools::Rectangle( 0, -nTop, nRight, nHeight - nTop );
    }

    // cache virtual device for rotation
    if (!mpOutDevData->mpRotateDev)
        mpOutDevData->mpRotateDev = VclPtr<VirtualDevice>::Create(*this);
    VirtualDevice* pVDev = mpOutDevData->mpRotateDev;

    // size it accordingly
    if( !pVDev->SetOutputSizePixel( aBoundRect.GetSize() ) )
        return false;

    const vcl::font::FontSelectPattern& rPattern = mpFontInstance->GetFontSelectPattern();
    vcl::Font aFont( GetFont() );
    aFont.SetOrientation( 0_deg10 );
    aFont.SetFontSize( Size( rPattern.mnWidth, rPattern.mnHeight ) );
    pVDev->SetFont( aFont );
    pVDev->SetTextColor( COL_BLACK );
    pVDev->SetTextFillColor();
    if (!pVDev->InitFont())
        return false;
    pVDev->ImplInitTextColor();

    // draw text into upper left corner
    rSalLayout.DrawBase() -= aBoundRect.TopLeft();
    rSalLayout.DrawText( *pVDev->mpGraphics );

    Bitmap aBmp = pVDev->GetBitmap( Point(), aBoundRect.GetSize() );
    if ( aBmp.IsEmpty() || !aBmp.Rotate( mpFontInstance->mnOwnOrientation, COL_WHITE ) )
        return false;

    // calculate rotation offset
    tools::Polygon aPoly( aBoundRect );
    aPoly.Rotate( Point(), mpFontInstance->mnOwnOrientation );
    Point aPoint = aPoly.GetBoundRect().TopLeft();
    aPoint += Point( nX, nY );

    // mask output with text colored bitmap
    GDIMetaFile* pOldMetaFile = mpMetaFile;
    tools::Long nOldOffX = mnOutOffX;
    tools::Long nOldOffY = mnOutOffY;
    bool bOldMap = mbMap;

    mnOutOffX   = 0;
    mnOutOffY   = 0;
    mpMetaFile  = nullptr;
    EnableMapMode( false );

    DrawMask( aPoint, aBmp, GetTextColor() );

    EnableMapMode( bOldMap );
    mnOutOffX   = nOldOffX;
    mnOutOffY   = nOldOffY;
    mpMetaFile  = pOldMetaFile;

    return true;
}

void OutputDevice::ImplDrawTextDirect( SalLayout& rSalLayout,
                                       bool bTextLines)
{
    if( mpFontInstance->mnOwnOrientation )
        if( ImplDrawRotateText( rSalLayout ) )
            return;

    tools::Long nOldX = rSalLayout.DrawBase().X();
    if( HasMirroredGraphics() )
    {
        tools::Long w = IsVirtual() ? mnOutWidth : mpGraphics->GetGraphicsWidth();
        tools::Long x = rSalLayout.DrawBase().X();
        rSalLayout.DrawBase().setX( w - 1 - x );
        if( !IsRTLEnabled() )
        {
            OutputDevice *pOutDevRef = this;
            // mirror this window back
            tools::Long devX = w-pOutDevRef->mnOutWidth-pOutDevRef->mnOutOffX;   // re-mirrored mnOutOffX
            rSalLayout.DrawBase().setX( devX + ( pOutDevRef->mnOutWidth - 1 - (rSalLayout.DrawBase().X() - devX) ) ) ;
        }
    }
    else if( IsRTLEnabled() )
    {
        OutputDevice *pOutDevRef = this;

        // mirror this window back
        tools::Long devX = pOutDevRef->mnOutOffX;   // re-mirrored mnOutOffX
        rSalLayout.DrawBase().setX( pOutDevRef->mnOutWidth - 1 - (rSalLayout.DrawBase().X() - devX) + devX );
    }

    rSalLayout.DrawText( *mpGraphics );
    rSalLayout.DrawBase().setX( nOldX );

    if( bTextLines )
        ImplDrawTextLines( rSalLayout,
            maFont.GetStrikeout(), maFont.GetUnderline(), maFont.GetOverline(),
            maFont.IsWordLineMode(), maFont.IsUnderlineAbove() );

    // emphasis marks
    if( maFont.GetEmphasisMark() & FontEmphasisMark::Style )
        ImplDrawEmphasisMarks( rSalLayout );
}

void OutputDevice::ImplDrawSpecialText( SalLayout& rSalLayout )
{
    Color       aOldColor           = GetTextColor();
    Color       aOldTextLineColor   = GetTextLineColor();
    Color       aOldOverlineColor   = GetOverlineColor();
    FontRelief  eRelief             = maFont.GetRelief();

    Point aOrigPos = rSalLayout.DrawBase();
    if ( eRelief != FontRelief::NONE )
    {
        Color   aReliefColor( COL_LIGHTGRAY );
        Color   aTextColor( aOldColor );

        Color   aTextLineColor( aOldTextLineColor );
        Color   aOverlineColor( aOldOverlineColor );

        // we don't have an automatic color, so black is always drawn on white
        if ( aTextColor == COL_BLACK )
            aTextColor = COL_WHITE;
        if ( aTextLineColor == COL_BLACK )
            aTextLineColor = COL_WHITE;
        if ( aOverlineColor == COL_BLACK )
            aOverlineColor = COL_WHITE;

        // relief-color is black for white text, in all other cases
        // we set this to LightGray
        // coverity[copy_paste_error: FALSE] - this is intentional
        if ( aTextColor == COL_WHITE )
            aReliefColor = COL_BLACK;
        SetTextLineColor( aReliefColor );
        SetOverlineColor( aReliefColor );
        SetTextColor( aReliefColor );
        ImplInitTextColor();

        // calculate offset - for high resolution printers the offset
        // should be greater so that the effect is visible
        tools::Long nOff = 1;
        nOff += mnDPIX/300;

        if ( eRelief == FontRelief::Engraved )
            nOff = -nOff;
        rSalLayout.DrawOffset() += Point( nOff, nOff);
        ImplDrawTextDirect( rSalLayout, mbTextLines );
        rSalLayout.DrawOffset() -= Point( nOff, nOff);

        SetTextLineColor( aTextLineColor );
        SetOverlineColor( aOverlineColor );
        SetTextColor( aTextColor );
        ImplInitTextColor();
        ImplDrawTextDirect( rSalLayout, mbTextLines );

        SetTextLineColor( aOldTextLineColor );
        SetOverlineColor( aOldOverlineColor );

        if ( aTextColor != aOldColor )
        {
            SetTextColor( aOldColor );
            ImplInitTextColor();
        }
    }
    else
    {
        if ( maFont.IsShadow() )
        {
            tools::Long nOff = 1 + ((mpFontInstance->mnLineHeight-24)/24);
            if ( maFont.IsOutline() )
                nOff++;
            SetTextLineColor();
            SetOverlineColor();
            if ( (GetTextColor() == COL_BLACK)
            ||   (GetTextColor().GetLuminance() < 8) )
                SetTextColor( COL_LIGHTGRAY );
            else
                SetTextColor( COL_BLACK );
            ImplInitTextColor();
            rSalLayout.DrawBase() += Point( nOff, nOff );
            ImplDrawTextDirect( rSalLayout, mbTextLines );
            rSalLayout.DrawBase() -= Point( nOff, nOff );
            SetTextColor( aOldColor );
            SetTextLineColor( aOldTextLineColor );
            SetOverlineColor( aOldOverlineColor );
            ImplInitTextColor();

            if ( !maFont.IsOutline() )
                ImplDrawTextDirect( rSalLayout, mbTextLines );
        }

        if ( maFont.IsOutline() )
        {
            rSalLayout.DrawBase() = aOrigPos + Point(-1,-1);
            ImplDrawTextDirect( rSalLayout, mbTextLines );
            rSalLayout.DrawBase() = aOrigPos + Point(+1,+1);
            ImplDrawTextDirect( rSalLayout, mbTextLines );
            rSalLayout.DrawBase() = aOrigPos + Point(-1,+0);
            ImplDrawTextDirect( rSalLayout, mbTextLines );
            rSalLayout.DrawBase() = aOrigPos + Point(-1,+1);
            ImplDrawTextDirect( rSalLayout, mbTextLines );
            rSalLayout.DrawBase() = aOrigPos + Point(+0,+1);
            ImplDrawTextDirect( rSalLayout, mbTextLines );
            rSalLayout.DrawBase() = aOrigPos + Point(+0,-1);
            ImplDrawTextDirect( rSalLayout, mbTextLines );
            rSalLayout.DrawBase() = aOrigPos + Point(+1,-1);
            ImplDrawTextDirect( rSalLayout, mbTextLines );
            rSalLayout.DrawBase() = aOrigPos + Point(+1,+0);
            ImplDrawTextDirect( rSalLayout, mbTextLines );
            rSalLayout.DrawBase() = aOrigPos;

            SetTextColor( COL_WHITE );
            SetTextLineColor( COL_WHITE );
            SetOverlineColor( COL_WHITE );
            ImplInitTextColor();
            ImplDrawTextDirect( rSalLayout, mbTextLines );
            SetTextColor( aOldColor );
            SetTextLineColor( aOldTextLineColor );
            SetOverlineColor( aOldOverlineColor );
            ImplInitTextColor();
        }
    }
}

void OutputDevice::ImplDrawText( SalLayout& rSalLayout )
{

    if( mbInitClipRegion )
        InitClipRegion();
    if( mbOutputClipped )
        return;
    if( mbInitTextColor )
        ImplInitTextColor();

    rSalLayout.DrawBase() += Point( mnTextOffX, mnTextOffY );

    if( IsTextFillColor() )
        ImplDrawTextBackground( rSalLayout );

    if( mbTextSpecial )
        ImplDrawSpecialText( rSalLayout );
    else
        ImplDrawTextDirect( rSalLayout, mbTextLines );
}

tools::Long OutputDevice::ImplGetTextLines( const tools::Rectangle& rRect, const tools::Long nTextHeight,
                                     ImplMultiTextLineInfo& rLineInfo,
                                     tools::Long nWidth, const OUString& rStr,
                                     DrawTextFlags nStyle, const vcl::ITextLayout& _rLayout )
{
    SAL_WARN_IF( nWidth <= 0, "vcl", "ImplGetTextLines: nWidth <= 0!" );

    if ( nWidth <= 0 )
        nWidth = 1;

    rLineInfo.Clear();
    if (rStr.isEmpty())
        return 0;

    const bool bClipping = (nStyle & DrawTextFlags::Clip) && !(nStyle & DrawTextFlags::EndEllipsis);

    tools::Long nMaxLineWidth  = 0;
    const bool bHyphenate = (nStyle & DrawTextFlags::WordBreakHyphenation) == DrawTextFlags::WordBreakHyphenation;
    css::uno::Reference< css::linguistic2::XHyphenator > xHyph;
    if (bHyphenate)
    {
        // get service provider
        css::uno::Reference<css::uno::XComponentContext> xContext(comphelper::getProcessComponentContext());
        css::uno::Reference<css::linguistic2::XLinguServiceManager2> xLinguMgr = css::linguistic2::LinguServiceManager::create(xContext);
        xHyph = xLinguMgr->getHyphenator();
    }

    css::uno::Reference<css::i18n::XBreakIterator> xBI;
    sal_Int32 nPos = 0;
    sal_Int32 nLen = rStr.getLength();
    sal_Int32 nCurrentTextY = 0;
    while ( nPos < nLen )
    {
        sal_Int32 nBreakPos = nPos;

        while ( ( nBreakPos < nLen ) && ( rStr[ nBreakPos ] != '\r' ) && ( rStr[ nBreakPos ] != '\n' ) )
            nBreakPos++;

        tools::Long nLineWidth = _rLayout.GetTextWidth( rStr, nPos, nBreakPos-nPos );
        if ( ( nLineWidth > nWidth ) && ( nStyle & DrawTextFlags::WordBreak ) )
        {
            if ( !xBI.is() )
                xBI = vcl::unohelper::CreateBreakIterator();

            if ( xBI.is() )
            {
                nBreakPos = ImplBreakLinesWithIterator(nWidth, rStr, _rLayout, xHyph, xBI, bHyphenate, nPos, nBreakPos);
                nLineWidth = _rLayout.GetTextWidth(rStr, nPos, nBreakPos - nPos);
            }
            else
                // fallback to something really simple
                nBreakPos = ImplBreakLinesSimple(nWidth, rStr, _rLayout, nPos, nBreakPos, nLineWidth);
        }

        if ( nLineWidth > nMaxLineWidth )
            nMaxLineWidth = nLineWidth;

        rLineInfo.AddLine( ImplTextLineInfo( nLineWidth, nPos, nBreakPos-nPos ) );

        if ( nBreakPos == nPos )
            nBreakPos++;
        nPos = nBreakPos;

        if ( nPos < nLen && ( ( rStr[ nPos ] == '\r' ) || ( rStr[ nPos ] == '\n' ) ) )
        {
            nPos++;
            // CR/LF?
            if ( ( nPos < nLen ) && ( rStr[ nPos ] == '\n' ) && ( rStr[ nPos-1 ] == '\r' ) )
                nPos++;
        }
        nCurrentTextY += nTextHeight;
        if (bClipping && nCurrentTextY > rRect.GetHeight())
            break;
    }

#ifdef DBG_UTIL
    for ( sal_Int32 nL = 0; nL < rLineInfo.Count(); nL++ )
    {
        ImplTextLineInfo& rLine = rLineInfo.GetLine( nL );
        OUString aLine = rStr.copy( rLine.GetIndex(), rLine.GetLen() );
        SAL_WARN_IF( aLine.indexOf( '\r' ) != -1, "vcl", "ImplGetTextLines - Found CR!" );
        SAL_WARN_IF( aLine.indexOf( '\n' ) != -1, "vcl", "ImplGetTextLines - Found LF!" );
    }
#endif

    return nMaxLineWidth;
}

sal_Int32 OutputDevice::ImplBreakLinesWithIterator(const tools::Long nWidth, const OUString& rStr, const vcl::ITextLayout& _rLayout,
                    const css::uno::Reference< css::linguistic2::XHyphenator >& xHyph,
                    const css::uno::Reference<css::i18n::XBreakIterator>& xBI,
                    const bool bHyphenate,
                    const sal_Int32 nPos, sal_Int32 nBreakPos)
{
    const css::lang::Locale& rDefLocale(Application::GetSettings().GetUILanguageTag().getLocale());
    sal_Int32 nSoftBreak = _rLayout.GetTextBreak( rStr, nWidth, nPos, nBreakPos - nPos );
    if (nSoftBreak == -1)
    {
        nSoftBreak = nPos;
    }
    SAL_WARN_IF( nSoftBreak >= nBreakPos, "vcl", "Break?!" );
    css::i18n::LineBreakHyphenationOptions aHyphOptions( xHyph, css::uno::Sequence <css::beans::PropertyValue>(), 1 );
    css::i18n::LineBreakUserOptions aUserOptions;
    css::i18n::LineBreakResults aLBR = xBI->getLineBreak( rStr, nSoftBreak, rDefLocale, nPos, aHyphOptions, aUserOptions );
    nBreakPos = aLBR.breakIndex;
    if ( nBreakPos <= nPos )
        nBreakPos = nSoftBreak;
    if ( !bHyphenate )
        return nBreakPos;

    // Whether hyphen or not: Put the word after the hyphen through
    // word boundary.

    // nMaxBreakPos the last char that fits into the line
    // nBreakPos is the word's start

    // We run into a problem if the doc is so narrow, that a word
    // is broken into more than two lines ...
    if ( !xHyph.is() )
        return nBreakPos;

    css::i18n::Boundary aBoundary = xBI->getWordBoundary( rStr, nBreakPos, rDefLocale, css::i18n::WordType::DICTIONARY_WORD, true );
    sal_Int32 nWordStart = nPos;
    sal_Int32 nWordEnd = aBoundary.endPos;
    SAL_WARN_IF( nWordEnd <= nWordStart, "vcl", "ImpBreakLine: Start >= End?" );

    sal_Int32 nWordLen = nWordEnd - nWordStart;
    if ( ( nWordEnd < nSoftBreak ) || ( nWordLen <= 3 ) )
        return nBreakPos;

    // #104415# May happen, because getLineBreak may differ from getWordBoundary with DICTIONARY_WORD
    // SAL_WARN_IF( nWordEnd < nMaxBreakPos, "vcl", "Hyph: Break?" );
    OUString aWord = rStr.copy( nWordStart, nWordLen );
    sal_Int32 nMinTrail = nWordEnd-nSoftBreak+1;  //+1: Before the "broken off" char
    css::uno::Reference< css::linguistic2::XHyphenatedWord > xHyphWord;
    if (xHyph.is())
        xHyphWord = xHyph->hyphenate( aWord, rDefLocale, aWord.getLength() - nMinTrail, css::uno::Sequence< css::beans::PropertyValue >() );
    if (!xHyphWord.is())
        return nBreakPos;

    bool bAlternate = xHyphWord->isAlternativeSpelling();
    sal_Int32 _nWordLen = 1 + xHyphWord->getHyphenPos();

    if ( ( _nWordLen < 2 ) || ( (nWordStart+_nWordLen) < 2 ) )
        return nBreakPos;

    if ( bAlternate )
    {
        nBreakPos = nWordStart + _nWordLen;
        return nBreakPos;
    }


    OUString aAlt( xHyphWord->getHyphenatedWord() );

    // We can have two cases:
    // 1) "packen" turns into "pak-ken"
    // 2) "Schiffahrt" turns into "Schiff-fahrt"

    // In case 1 we need to replace a char
    // In case 2 we add a char

    // Correct recognition is made harder by words such as
    // "Schiffahrtsbrennesseln", as the Hyphenator splits all
    // positions of the word and comes up with "Schifffahrtsbrennnesseln"
    // Thus, we cannot infer the aWord from the AlternativeWord's
    // index.
    // TODO: The whole junk will be made easier by a function in
    // the Hyphenator, as soon as AMA adds it.
    sal_Int32 nAltStart = _nWordLen - 1;
    sal_Int32 nTxtStart = nAltStart - (aAlt.getLength() - aWord.getLength());
    sal_Int32 nTxtEnd = nTxtStart;
    sal_Int32 nAltEnd = nAltStart;

    // The area between nStart and nEnd is the difference
    // between AlternativeString and OriginalString
    while( nTxtEnd < aWord.getLength() && nAltEnd < aAlt.getLength() &&
           aWord[nTxtEnd] != aAlt[nAltEnd] )
    {
        ++nTxtEnd;
        ++nAltEnd;
    }

    // If a char was added, we notice it now:
    if( nAltEnd > nTxtEnd && nAltStart == nAltEnd &&
        aWord[ nTxtEnd ] == aAlt[nAltEnd] )
    {
        ++nAltEnd;
        ++nTxtStart;
        ++nTxtEnd;
    }

    SAL_WARN_IF( ( nAltEnd - nAltStart ) != 1, "vcl", "Alternate: Wrong assumption!" );

    sal_Unicode cAlternateReplChar = 0;
    if ( nTxtEnd > nTxtStart )
        cAlternateReplChar = aAlt[ nAltStart ];

    nBreakPos = nWordStart + nTxtStart;
    if ( cAlternateReplChar )
        nBreakPos++;
    return nBreakPos;
}

sal_Int32 OutputDevice::ImplBreakLinesSimple( const tools::Long nWidth, const OUString& rStr,
                        const vcl::ITextLayout& _rLayout, const sal_Int32 nPos, sal_Int32 nBreakPos, tools::Long& nLineWidth )
{
    sal_Int32 nSpacePos = rStr.getLength();
    tools::Long nW = 0;
    do
    {
        nSpacePos = rStr.lastIndexOf( ' ', nSpacePos );
        if( nSpacePos != -1 )
        {
            if( nSpacePos > nPos )
                nSpacePos--;
            nW = _rLayout.GetTextWidth( rStr, nPos, nSpacePos-nPos );
        }
    } while( nW > nWidth );

    if( nSpacePos != -1 )
    {
        nBreakPos = nSpacePos;
        nLineWidth = _rLayout.GetTextWidth( rStr, nPos, nBreakPos-nPos );
        if( nBreakPos < rStr.getLength()-1 )
            nBreakPos++;
    }
    return nBreakPos;
}


void OutputDevice::SetTextColor( const Color& rColor )
{

    Color aColor(vcl::drawmode::GetTextColor(rColor, GetDrawMode(), GetSettings().GetStyleSettings()));

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextColorAction( aColor ) );

    if ( maTextColor != aColor )
    {
        maTextColor = aColor;
        mbInitTextColor = true;
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetTextColor( COL_ALPHA_OPAQUE );
}

void OutputDevice::SetTextFillColor()
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextFillColorAction( Color(), false ) );

    if ( maFont.GetColor() != COL_TRANSPARENT ) {
        maFont.SetFillColor( COL_TRANSPARENT );
    }
    if ( !maFont.IsTransparent() )
        maFont.SetTransparent( true );

    if( mpAlphaVDev )
        mpAlphaVDev->SetTextFillColor();
}

void OutputDevice::SetTextFillColor( const Color& rColor )
{
    Color aColor(vcl::drawmode::GetFillColor(rColor, GetDrawMode(), GetSettings().GetStyleSettings()));

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextFillColorAction( aColor, true ) );

    if ( maFont.GetFillColor() != aColor )
        maFont.SetFillColor( aColor );
    if ( maFont.IsTransparent() != rColor.IsTransparent() )
        maFont.SetTransparent( rColor.IsTransparent() );

    if( mpAlphaVDev )
        mpAlphaVDev->SetTextFillColor( COL_ALPHA_OPAQUE );
}

Color OutputDevice::GetTextFillColor() const
{
    if ( maFont.IsTransparent() )
        return COL_TRANSPARENT;
    else
        return maFont.GetFillColor();
}

void OutputDevice::SetTextAlign( TextAlign eAlign )
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextAlignAction( eAlign ) );

    if ( maFont.GetAlignment() != eAlign )
    {
        maFont.SetAlignment( eAlign );
        mbNewFont = true;
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetTextAlign( eAlign );
}

vcl::Region OutputDevice::GetOutputBoundsClipRegion() const
{
    return GetClipRegion();
}

#if !ENABLE_FUZZERS
const SalLayoutFlags eDefaultLayout = SalLayoutFlags::NONE;
#else
// ofz#23573 skip detecting bidi directions
const SalLayoutFlags eDefaultLayout = SalLayoutFlags::BiDiStrong;
#endif

void OutputDevice::DrawText( const Point& rStartPt, const OUString& rStr,
                             sal_Int32 nIndex, sal_Int32 nLen,
                             std::vector< tools::Rectangle >* pVector, OUString* pDisplayText,
                             const SalLayoutGlyphs* pLayoutCache
                             )
{
    assert(!is_double_buffered_window());

    if( (nLen < 0) || (nIndex + nLen >= rStr.getLength()))
    {
        nLen = rStr.getLength() - nIndex;
    }

    if (mpOutDevData->mpRecordLayout)
    {
        pVector = &mpOutDevData->mpRecordLayout->m_aUnicodeBoundRects;
        pDisplayText = &mpOutDevData->mpRecordLayout->m_aDisplayText;
    }

#if OSL_DEBUG_LEVEL > 2
    SAL_INFO("vcl.gdi", "OutputDevice::DrawText(\"" << rStr << "\")");
#endif

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextAction( rStartPt, rStr, nIndex, nLen ) );
    if( pVector )
    {
        vcl::Region aClip(GetOutputBoundsClipRegion());

        if (mpOutDevData->mpRecordLayout)
        {
            mpOutDevData->mpRecordLayout->m_aLineIndices.push_back( mpOutDevData->mpRecordLayout->m_aDisplayText.getLength() );
            aClip.Intersect( mpOutDevData->maRecordRect );
        }
        if( ! aClip.IsNull() )
        {
            std::vector< tools::Rectangle > aTmp;
            GetGlyphBoundRects( rStartPt, rStr, nIndex, nLen, aTmp );

            bool bInserted = false;
            for( std::vector< tools::Rectangle >::const_iterator it = aTmp.begin(); it != aTmp.end(); ++it, nIndex++ )
            {
                bool bAppend = false;

                if( aClip.Overlaps( *it ) )
                    bAppend = true;
                else if( rStr[ nIndex ] == ' ' && bInserted )
                {
                    std::vector< tools::Rectangle >::const_iterator next = it;
                    ++next;
                    if( next != aTmp.end() && aClip.Overlaps( *next ) )
                        bAppend = true;
                }

                if( bAppend )
                {
                    pVector->push_back( *it );
                    if( pDisplayText )
                        *pDisplayText += OUStringChar(rStr[ nIndex ]);
                    bInserted = true;
                }
            }
        }
        else
        {
            GetGlyphBoundRects( rStartPt, rStr, nIndex, nLen, *pVector );
            if( pDisplayText )
                *pDisplayText += rStr.subView( nIndex, nLen );
        }
    }

    if ( !IsDeviceOutputNecessary() || pVector )
        return;

    if(mpFontInstance)
        // do not use cache with modified string
        if(mpFontInstance->mpConversion)
            pLayoutCache = nullptr;

    std::unique_ptr<SalLayout> pSalLayout = ImplLayout(rStr, nIndex, nLen, rStartPt, 0, {}, eDefaultLayout, nullptr, pLayoutCache);
    if(pSalLayout)
    {
        ImplDrawText( *pSalLayout );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawText( rStartPt, rStr, nIndex, nLen, pVector, pDisplayText );
}

tools::Long OutputDevice::GetTextWidth( const OUString& rStr, sal_Int32 nIndex, sal_Int32 nLen,
     vcl::text::TextLayoutCache const*const pLayoutCache,
     SalLayoutGlyphs const*const pSalLayoutCache) const
{

    tools::Long nWidth = GetTextArray( rStr, nullptr, nIndex,
            nLen, pLayoutCache, pSalLayoutCache );

    return nWidth;
}

tools::Long OutputDevice::GetTextHeight() const
{
    if (!InitFont())
        return 0;

    tools::Long nHeight = mpFontInstance->mnLineHeight + mnEmphasisAscent + mnEmphasisDescent;

    if ( mbMap )
        nHeight = ImplDevicePixelToLogicHeight( nHeight );

    return nHeight;
}

float OutputDevice::approximate_char_width() const
{
    //note pango uses "The quick brown fox jumps over the lazy dog." for english
    //and has a bunch of per-language strings which corresponds somewhat with
    //makeRepresentativeText in include/svtools/sampletext.hxx
    return GetTextWidth("aemnnxEM") / 8.0;
}

float OutputDevice::approximate_digit_width() const
{
    return GetTextWidth("0123456789") / 10.0;
}

void OutputDevice::DrawTextArray( const Point& rStartPt, const OUString& rStr,
                                  o3tl::span<const sal_Int32> pDXAry,
                                  sal_Int32 nIndex, sal_Int32 nLen, SalLayoutFlags flags,
                                  const SalLayoutGlyphs* pSalLayoutCache )
{
    assert(!is_double_buffered_window());

    if( nLen < 0 || nIndex + nLen >= rStr.getLength() )
    {
        nLen = rStr.getLength() - nIndex;
    }
    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextArrayAction( rStartPt, rStr, pDXAry, nIndex, nLen ) );

    if ( !IsDeviceOutputNecessary() )
        return;
    if( !mpGraphics && !AcquireGraphics() )
        return;
    assert(mpGraphics);
    if( mbInitClipRegion )
        InitClipRegion();
    if( mbOutputClipped )
        return;

    std::unique_ptr<SalLayout> pSalLayout = ImplLayout(rStr, nIndex, nLen, rStartPt, 0, pDXAry, flags, nullptr, pSalLayoutCache);
    if( pSalLayout )
    {
        ImplDrawText( *pSalLayout );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawTextArray( rStartPt, rStr, pDXAry, nIndex, nLen, flags );
}

tools::Long OutputDevice::GetTextArray( const OUString& rStr, std::vector<sal_Int32>* pDXAry,
                                 sal_Int32 nIndex, sal_Int32 nLen,
                                 vcl::text::TextLayoutCache const*const pLayoutCache,
                                 SalLayoutGlyphs const*const pSalLayoutCache) const
{
    if( nIndex >= rStr.getLength() )
        return 0; // TODO: this looks like a buggy caller?

    if( nLen < 0 || nIndex + nLen >= rStr.getLength() )
    {
        nLen = rStr.getLength() - nIndex;
    }

    // do layout
    std::unique_ptr<SalLayout> pSalLayout = ImplLayout(rStr, nIndex, nLen,
            Point(0,0), 0, {}, eDefaultLayout, pLayoutCache, pSalLayoutCache);
    if( !pSalLayout )
    {
        // The caller expects this to init the elements of pDXAry.
        // Adapting all the callers to check that GetTextArray succeeded seems
        // too much work.
        // Init here to 0 only in the (rare) error case, so that any missing
        // element init in the happy case will still be found by tools,
        // and hope that is sufficient.
        if (pDXAry)
            std::fill(pDXAry->begin(), pDXAry->end(), 0);
        return 0;
    }

#if VCL_FLOAT_DEVICE_PIXEL
    std::unique_ptr<DeviceCoordinate[]> pDXPixelArray;
    if(pDXAry)
    {
        pDXPixelArray.reset(new DeviceCoordinate[nLen]);
    }
    DeviceCoordinate nWidth = pSalLayout->FillDXArray( pDXPixelArray.get() );
    int nWidthFactor = pSalLayout->GetUnitsPerPixel();

    // convert virtual char widths to virtual absolute positions
    if( pDXPixelArray )
    {
        for( int i = 1; i < nLen; ++i )
        {
            pDXPixelArray[ i ] += pDXPixelArray[ i-1 ];
        }
    }
    if( mbMap )
    {
        if( pDXPixelArray )
        {
            for( int i = 0; i < nLen; ++i )
            {
                pDXPixelArray[i] = ImplDevicePixelToLogicWidth( pDXPixelArray[i] );
            }
        }
        nWidth = ImplDevicePixelToLogicWidth( nWidth );
    }
    if( nWidthFactor > 1 )
    {
        if( pDXPixelArray )
        {
            for( int i = 0; i < nLen; ++i )
            {
                pDXPixelArray[i] /= nWidthFactor;
            }
        }
        nWidth /= nWidthFactor;
    }
    if(pDXAry)
    {
        pDXAry->resize(nLen);
        for( int i = 0; i < nLen; ++i )
        {
            (*pDXAry)[i] = basegfx::fround(pDXPixelArray[i]);
        }
    }
    return basegfx::fround(nWidth);

#else /* ! VCL_FLOAT_DEVICE_PIXEL */

    tools::Long nWidth = pSalLayout->FillDXArray( pDXAry );
    int nWidthFactor = pSalLayout->GetUnitsPerPixel();

    // convert virtual char widths to virtual absolute positions
    if( pDXAry )
        for( int i = 1; i < nLen; ++i )
            (*pDXAry)[ i ] += (*pDXAry)[ i-1 ];

    // convert from font units to logical units
    if( mbMap )
    {
        if( pDXAry )
            for( int i = 0; i < nLen; ++i )
                (*pDXAry)[i] = ImplDevicePixelToLogicWidth( (*pDXAry)[i] );
        nWidth = ImplDevicePixelToLogicWidth( nWidth );
    }

    if( nWidthFactor > 1 )
    {
        if( pDXAry )
            for( int i = 0; i < nLen; ++i )
                (*pDXAry)[i] /= nWidthFactor;
        nWidth /= nWidthFactor;
    }
    return nWidth;
#endif /* VCL_FLOAT_DEVICE_PIXEL */
}

void OutputDevice::GetCaretPositions( const OUString& rStr, sal_Int32* pCaretXArray,
                                      sal_Int32 nIndex, sal_Int32 nLen,
                                      const SalLayoutGlyphs* pGlyphs ) const
{

    if( nIndex >= rStr.getLength() )
        return;
    if( nIndex+nLen >= rStr.getLength() )
        nLen = rStr.getLength() - nIndex;

    // layout complex text
    std::unique_ptr<SalLayout> pSalLayout = ImplLayout(rStr, nIndex, nLen, Point(0, 0), 0, {},
                                                       eDefaultLayout, nullptr, pGlyphs);
    if( !pSalLayout )
        return;

    int nWidthFactor = pSalLayout->GetUnitsPerPixel();
    pSalLayout->GetCaretPositions( 2*nLen, pCaretXArray );
    tools::Long nWidth = pSalLayout->GetTextWidth();

    // fixup unknown caret positions
    int i;
    for( i = 0; i < 2 * nLen; ++i )
        if( pCaretXArray[ i ] >= 0 )
            break;
    tools::Long nXPos = pCaretXArray[ i ];
    for( i = 0; i < 2 * nLen; ++i )
    {
        if( pCaretXArray[ i ] >= 0 )
            nXPos = pCaretXArray[ i ];
        else
            pCaretXArray[ i ] = nXPos;
    }

    // handle window mirroring
    if( IsRTLEnabled() )
    {
        for( i = 0; i < 2 * nLen; ++i )
            pCaretXArray[i] = nWidth - pCaretXArray[i] - 1;
    }

    // convert from font units to logical units
    if( mbMap )
    {
        for( i = 0; i < 2*nLen; ++i )
            pCaretXArray[i] = ImplDevicePixelToLogicWidth( pCaretXArray[i] );
    }

    if( nWidthFactor != 1 )
    {
        for( i = 0; i < 2*nLen; ++i )
            pCaretXArray[i] /= nWidthFactor;
    }
}

void OutputDevice::DrawStretchText( const Point& rStartPt, sal_uLong nWidth,
                                    const OUString& rStr,
                                    sal_Int32 nIndex, sal_Int32 nLen)
{
    assert(!is_double_buffered_window());

    if( (nLen < 0) || (nIndex + nLen >= rStr.getLength()))
    {
        nLen = rStr.getLength() - nIndex;
    }

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaStretchTextAction( rStartPt, nWidth, rStr, nIndex, nLen ) );

    if ( !IsDeviceOutputNecessary() )
        return;

    std::unique_ptr<SalLayout> pSalLayout = ImplLayout(rStr, nIndex, nLen, rStartPt, nWidth);
    if( pSalLayout )
    {
        ImplDrawText( *pSalLayout );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawStretchText( rStartPt, nWidth, rStr, nIndex, nLen );
}

vcl::text::ImplLayoutArgs OutputDevice::ImplPrepareLayoutArgs( OUString& rStr,
                                                    const sal_Int32 nMinIndex, const sal_Int32 nLen,
                                                    DeviceCoordinate nPixelWidth, const DeviceCoordinate* pDXArray,
                                                    SalLayoutFlags nLayoutFlags,
         vcl::text::TextLayoutCache const*const pLayoutCache) const
{
    assert(nMinIndex >= 0);
    assert(nLen >= 0);

    // get string length for calculating extents
    sal_Int32 nEndIndex = rStr.getLength();
    if( nMinIndex + nLen < nEndIndex )
        nEndIndex = nMinIndex + nLen;

    // don't bother if there is nothing to do
    if( nEndIndex < nMinIndex )
        nEndIndex = nMinIndex;

    if( mnTextLayoutMode & vcl::text::ComplexTextLayoutFlags::BiDiRtl )
        nLayoutFlags |= SalLayoutFlags::BiDiRtl;
    if( mnTextLayoutMode & vcl::text::ComplexTextLayoutFlags::BiDiStrong )
        nLayoutFlags |= SalLayoutFlags::BiDiStrong;
    else if( !(mnTextLayoutMode & vcl::text::ComplexTextLayoutFlags::BiDiRtl) )
    {
        // Disable Bidi if no RTL hint and only known LTR codes used.
        bool bAllLtr = true;
        for (sal_Int32 i = nMinIndex; i < nEndIndex; i++)
        {
            // [0x0000, 0x052F] are Latin, Greek and Cyrillic.
            // [0x0370, 0x03FF] has a few holes as if Unicode 10.0.0, but
            //                  hopefully no RTL character will be encoded there.
            if (rStr[i] > 0x052F)
            {
                bAllLtr = false;
                break;
            }
        }
        if (bAllLtr)
            nLayoutFlags |= SalLayoutFlags::BiDiStrong;
    }

    if( !maFont.IsKerning() )
        nLayoutFlags |= SalLayoutFlags::DisableKerning;
    if( maFont.GetKerning() & FontKerning::Asian )
        nLayoutFlags |= SalLayoutFlags::KerningAsian;
    if( maFont.IsVertical() )
        nLayoutFlags |= SalLayoutFlags::Vertical;

    if( meTextLanguage ) //TODO: (mnTextLayoutMode & vcl::text::ComplexTextLayoutFlags::SubstituteDigits)
    {
        // disable character localization when no digits used
        const sal_Unicode* pBase = rStr.getStr();
        const sal_Unicode* pStr = pBase + nMinIndex;
        const sal_Unicode* pEnd = pBase + nEndIndex;
        std::optional<OUStringBuffer> xTmpStr;
        for( ; pStr < pEnd; ++pStr )
        {
            // TODO: are there non-digit localizations?
            if( (*pStr >= '0') && (*pStr <= '9') )
            {
                // translate characters to local preference
                sal_UCS4 cChar = GetLocalizedChar( *pStr, meTextLanguage );
                if( cChar != *pStr )
                {
                    if (!xTmpStr)
                        xTmpStr = OUStringBuffer(rStr);
                    // TODO: are the localized digit surrogates?
                    (*xTmpStr)[pStr - pBase] = cChar;
                }
            }
        }
        if (xTmpStr)
            rStr = (*xTmpStr).makeStringAndClear();
    }

    // right align for RTL text, DRAWPOS_REVERSED, RTL window style
    bool bRightAlign = bool(mnTextLayoutMode & vcl::text::ComplexTextLayoutFlags::BiDiRtl);
    if( mnTextLayoutMode & vcl::text::ComplexTextLayoutFlags::TextOriginLeft )
        bRightAlign = false;
    else if ( mnTextLayoutMode & vcl::text::ComplexTextLayoutFlags::TextOriginRight )
        bRightAlign = true;
    // SSA: hack for western office, ie text get right aligned
    //      for debugging purposes of mirrored UI
    bool bRTLWindow = IsRTLEnabled();
    bRightAlign ^= bRTLWindow;
    if( bRightAlign )
        nLayoutFlags |= SalLayoutFlags::RightAlign;

    // set layout options
    vcl::text::ImplLayoutArgs aLayoutArgs(rStr, nMinIndex, nEndIndex, nLayoutFlags, maFont.GetLanguageTag(), pLayoutCache);

    Degree10 nOrientation = mpFontInstance ? mpFontInstance->mnOrientation : 0_deg10;
    aLayoutArgs.SetOrientation( nOrientation );

    aLayoutArgs.SetLayoutWidth( nPixelWidth );
    aLayoutArgs.SetDXArray( pDXArray );

    return aLayoutArgs;
}

static OutputDevice::FontMappingUseData* fontMappingUseData = nullptr;

static inline bool IsTrackingFontMappingUse()
{
    return fontMappingUseData != nullptr;
}

static void TrackFontMappingUse( const vcl::Font& originalFont, const SalLayout* salLayout)
{
    assert(fontMappingUseData);
    OUString originalName = originalFont.GetStyleName().isEmpty()
        ? originalFont.GetFamilyName()
        : originalFont.GetFamilyName() + "/" + originalFont.GetStyleName();
    std::vector<OUString> usedFontNames;
    SalLayoutGlyphs glyphs = salLayout->GetGlyphs(); // includes all font fallbacks
    int level = 0;
    while( const SalLayoutGlyphsImpl* impl = glyphs.Impl(level++))
    {
        const vcl::font::PhysicalFontFace* face = impl->GetFont()->GetFontFace();
        OUString name = face->GetStyleName().isEmpty()
            ? face->GetFamilyName()
            : face->GetFamilyName() + "/" + face->GetStyleName();
        usedFontNames.push_back( name );
    }
    for( OutputDevice::FontMappingUseItem& item : *fontMappingUseData )
    {
        if( item.mOriginalFont == originalName && item.mUsedFonts == usedFontNames )
        {
            ++item.mCount;
            return;
        }
    }
    fontMappingUseData->push_back( { originalName, usedFontNames, 1 } );
}

void OutputDevice::StartTrackingFontMappingUse()
{
    delete fontMappingUseData;
    fontMappingUseData = new FontMappingUseData;
}

OutputDevice::FontMappingUseData OutputDevice::FinishTrackingFontMappingUse()
{
    if(!fontMappingUseData)
        return {};
    FontMappingUseData ret = std::move( *fontMappingUseData );
    delete fontMappingUseData;
    fontMappingUseData = nullptr;
    return ret;
}

std::unique_ptr<SalLayout> OutputDevice::ImplLayout(const OUString& rOrigStr,
                                    sal_Int32 nMinIndex, sal_Int32 nLen,
                                    const Point& rLogicalPos, tools::Long nLogicalWidth,
                                    o3tl::span<const sal_Int32> pDXArray, SalLayoutFlags flags,
         vcl::text::TextLayoutCache const* pLayoutCache,
         const SalLayoutGlyphs* pGlyphs) const
{
    if (pGlyphs && !pGlyphs->IsValid())
    {
        SAL_WARN("vcl", "Trying to setup invalid cached glyphs - falling back to relayout!");
        pGlyphs = nullptr;
    }

    if (!InitFont())
        return nullptr;

    // check string index and length
    if( -1 == nLen || nMinIndex + nLen > rOrigStr.getLength() )
    {
        const sal_Int32 nNewLen = rOrigStr.getLength() - nMinIndex;
        if( nNewLen <= 0 )
            return nullptr;
        nLen = nNewLen;
    }

    OUString aStr = rOrigStr;

    // convert from logical units to physical units
    // recode string if needed
    if( mpFontInstance->mpConversion ) {
        mpFontInstance->mpConversion->RecodeString( aStr, 0, aStr.getLength() );
        pLayoutCache = nullptr; // don't use cache with modified string!
        pGlyphs = nullptr;
    }

    DeviceCoordinate nPixelWidth = static_cast<DeviceCoordinate>(nLogicalWidth);
    if( nLogicalWidth && mbMap )
    {
        nPixelWidth = LogicWidthToDeviceCoordinate( nLogicalWidth );
    }

    std::unique_ptr<DeviceCoordinate[]> xDXPixelArray;
    DeviceCoordinate* pDXPixelArray(nullptr);
    if( !pDXArray.empty() )
    {
        if(mbMap)
        {
            // convert from logical units to font units using a temporary array
            xDXPixelArray.reset(new DeviceCoordinate[nLen]);
            pDXPixelArray = xDXPixelArray.get();
            // using base position for better rounding a.k.a. "dancing characters"
            DeviceCoordinate nPixelXOfs = LogicWidthToDeviceCoordinate( rLogicalPos.X() );
            for( int i = 0; i < nLen; ++i )
            {
                pDXPixelArray[i] = LogicWidthToDeviceCoordinate( rLogicalPos.X() + pDXArray[i] ) - nPixelXOfs;
            }
        }
        else
        {
#if VCL_FLOAT_DEVICE_PIXEL
            xDXPixelArray.reset(new DeviceCoordinate[nLen]);
            pDXPixelArray = xDXPixelArray.get();
            for( int i = 0; i < nLen; ++i )
            {
                pDXPixelArray[i] = (*pDXArray)[i];
            }
#else /* !VCL_FLOAT_DEVICE_PIXEL */
            pDXPixelArray = const_cast<DeviceCoordinate*>(pDXArray.data());
#endif /* !VCL_FLOAT_DEVICE_PIXEL */
        }
    }

    vcl::text::ImplLayoutArgs aLayoutArgs = ImplPrepareLayoutArgs( aStr, nMinIndex, nLen,
            nPixelWidth, pDXPixelArray, flags, pLayoutCache);

    // get matching layout object for base font
    std::unique_ptr<SalLayout> pSalLayout = mpGraphics->GetTextLayout(0);

    // layout text
    if( pSalLayout && !pSalLayout->LayoutText( aLayoutArgs, pGlyphs ? pGlyphs->Impl(0) : nullptr ) )
    {
        pSalLayout.reset();
    }

    if( !pSalLayout )
        return nullptr;

    // do glyph fallback if needed
    // #105768# avoid fallback for very small font sizes
    if (aLayoutArgs.HasFallbackRun() && mpFontInstance->GetFontSelectPattern().mnHeight >= 3)
        pSalLayout = ImplGlyphFallbackLayout(std::move(pSalLayout), aLayoutArgs, pGlyphs);

    if (flags & SalLayoutFlags::GlyphItemsOnly)
        // Return glyph items only after fallback handling. Otherwise they may
        // contain invalid glyph IDs.
        return pSalLayout;

    // position, justify, etc. the layout
    pSalLayout->AdjustLayout( aLayoutArgs );
    pSalLayout->DrawBase() = ImplLogicToDevicePixel( rLogicalPos );
    // adjust to right alignment if necessary
    if( aLayoutArgs.mnFlags & SalLayoutFlags::RightAlign )
    {
        DeviceCoordinate nRTLOffset;
        if( pDXPixelArray )
            nRTLOffset = pDXPixelArray[ nLen - 1 ];
        else if( nPixelWidth )
            nRTLOffset = nPixelWidth;
        else
            nRTLOffset = pSalLayout->GetTextWidth() / pSalLayout->GetUnitsPerPixel();
        pSalLayout->DrawOffset().setX( 1 - nRTLOffset );
    }

    if(IsTrackingFontMappingUse())
        TrackFontMappingUse(GetFont(), pSalLayout.get());

    return pSalLayout;
}

std::shared_ptr<vcl::text::TextLayoutCache> OutputDevice::CreateTextLayoutCache(
        OUString const& rString)
{
    return GenericSalLayout::CreateTextLayoutCache(rString);
}

bool OutputDevice::GetTextIsRTL( const OUString& rString, sal_Int32 nIndex, sal_Int32 nLen ) const
{
    OUString aStr( rString );
    vcl::text::ImplLayoutArgs aArgs = ImplPrepareLayoutArgs( aStr, nIndex, nLen, 0, nullptr );
    bool bRTL = false;
    int nCharPos = -1;
    if (!aArgs.GetNextPos(&nCharPos, &bRTL))
        return false;
    return (nCharPos != nIndex);
}

sal_Int32 OutputDevice::GetTextBreak( const OUString& rStr, tools::Long nTextWidth,
                                       sal_Int32 nIndex, sal_Int32 nLen,
                                       tools::Long nCharExtra,
         vcl::text::TextLayoutCache const*const pLayoutCache,
         const SalLayoutGlyphs* pGlyphs) const
{
    std::unique_ptr<SalLayout> pSalLayout = ImplLayout( rStr, nIndex, nLen,
            Point(0,0), 0, {}, eDefaultLayout, pLayoutCache, pGlyphs);
    sal_Int32 nRetVal = -1;
    if( pSalLayout )
    {
        // convert logical widths into layout units
        // NOTE: be very careful to avoid rounding errors for nCharExtra case
        // problem with rounding errors especially for small nCharExtras
        // TODO: remove when layout units have subpixel granularity
        tools::Long nWidthFactor = pSalLayout->GetUnitsPerPixel();
        tools::Long nSubPixelFactor = (nWidthFactor < 64 ) ? 64 : 1;
        nTextWidth *= nWidthFactor * nSubPixelFactor;
        DeviceCoordinate nTextPixelWidth = LogicWidthToDeviceCoordinate( nTextWidth );
        DeviceCoordinate nExtraPixelWidth = 0;
        if( nCharExtra != 0 )
        {
            nCharExtra *= nWidthFactor * nSubPixelFactor;
            nExtraPixelWidth = LogicWidthToDeviceCoordinate( nCharExtra );
        }
        nRetVal = pSalLayout->GetTextBreak( nTextPixelWidth, nExtraPixelWidth, nSubPixelFactor );
    }

    return nRetVal;
}

sal_Int32 OutputDevice::GetTextBreak( const OUString& rStr, tools::Long nTextWidth,
                                       sal_Unicode nHyphenChar, sal_Int32& rHyphenPos,
                                       sal_Int32 nIndex, sal_Int32 nLen,
                                       tools::Long nCharExtra,
         vcl::text::TextLayoutCache const*const pLayoutCache) const
{
    rHyphenPos = -1;

    std::unique_ptr<SalLayout> pSalLayout = ImplLayout( rStr, nIndex, nLen,
            Point(0,0), 0, {}, eDefaultLayout, pLayoutCache);
    sal_Int32 nRetVal = -1;
    if( pSalLayout )
    {
        // convert logical widths into layout units
        // NOTE: be very careful to avoid rounding errors for nCharExtra case
        // problem with rounding errors especially for small nCharExtras
        // TODO: remove when layout units have subpixel granularity
        tools::Long nWidthFactor = pSalLayout->GetUnitsPerPixel();
        tools::Long nSubPixelFactor = (nWidthFactor < 64 ) ? 64 : 1;

        nTextWidth *= nWidthFactor * nSubPixelFactor;
        DeviceCoordinate nTextPixelWidth = LogicWidthToDeviceCoordinate( nTextWidth );
        DeviceCoordinate nExtraPixelWidth = 0;
        if( nCharExtra != 0 )
        {
            nCharExtra *= nWidthFactor * nSubPixelFactor;
            nExtraPixelWidth = LogicWidthToDeviceCoordinate( nCharExtra );
        }

        // calculate un-hyphenated break position
        nRetVal = pSalLayout->GetTextBreak( nTextPixelWidth, nExtraPixelWidth, nSubPixelFactor );

        // calculate hyphenated break position
        OUString aHyphenStr(nHyphenChar);
        std::unique_ptr<SalLayout> pHyphenLayout = ImplLayout( aHyphenStr, 0, 1 );
        if( pHyphenLayout )
        {
            // calculate subpixel width of hyphenation character
            tools::Long nHyphenPixelWidth = pHyphenLayout->GetTextWidth() * nSubPixelFactor;

            // calculate hyphenated break position
            nTextPixelWidth -= nHyphenPixelWidth;
            if( nExtraPixelWidth > 0 )
                nTextPixelWidth -= nExtraPixelWidth;

            rHyphenPos = pSalLayout->GetTextBreak(nTextPixelWidth, nExtraPixelWidth, nSubPixelFactor);

            if( rHyphenPos > nRetVal )
                rHyphenPos = nRetVal;
        }
    }

    return nRetVal;
}

void OutputDevice::ImplDrawText( OutputDevice& rTargetDevice, const tools::Rectangle& rRect,
                                 const OUString& rOrigStr, DrawTextFlags nStyle,
                                 std::vector< tools::Rectangle >* pVector, OUString* pDisplayText,
                                 vcl::ITextLayout& _rLayout )
{

    Color aOldTextColor;
    Color aOldTextFillColor;
    bool  bRestoreFillColor = false;
    if ( (nStyle & DrawTextFlags::Disable) && ! pVector )
    {
        bool  bHighContrastBlack = false;
        bool  bHighContrastWhite = false;
        const StyleSettings& rStyleSettings( rTargetDevice.GetSettings().GetStyleSettings() );
        if( rStyleSettings.GetHighContrastMode() )
        {
            Color aCol;
            if( rTargetDevice.IsBackground() )
                aCol = rTargetDevice.GetBackground().GetColor();
            else
                // best guess is the face color here
                // but it may be totally wrong. the background color
                // was typically already reset
                aCol = rStyleSettings.GetFaceColor();

            bHighContrastBlack = aCol.IsDark();
            bHighContrastWhite = aCol.IsBright();
        }

        aOldTextColor = rTargetDevice.GetTextColor();
        if ( rTargetDevice.IsTextFillColor() )
        {
            bRestoreFillColor = true;
            aOldTextFillColor = rTargetDevice.GetTextFillColor();
        }
        if( bHighContrastBlack )
            rTargetDevice.SetTextColor( COL_GREEN );
        else if( bHighContrastWhite )
            rTargetDevice.SetTextColor( COL_LIGHTGREEN );
        else
        {
            // draw disabled text always without shadow
            // as it fits better with native look
            rTargetDevice.SetTextColor( rTargetDevice.GetSettings().GetStyleSettings().GetDisableColor() );
        }
    }

    tools::Long        nWidth          = rRect.GetWidth();
    tools::Long        nHeight         = rRect.GetHeight();

    if ( ((nWidth <= 0) || (nHeight <= 0)) && (nStyle & DrawTextFlags::Clip) )
        return;

    Point       aPos            = rRect.TopLeft();

    tools::Long        nTextHeight     = rTargetDevice.GetTextHeight();
    TextAlign   eAlign          = rTargetDevice.GetTextAlign();
    sal_Int32   nMnemonicPos    = -1;

    OUString aStr = rOrigStr;
    if ( nStyle & DrawTextFlags::Mnemonic )
        aStr = GetNonMnemonicString( aStr, nMnemonicPos );

    const bool bDrawMnemonics = !(rTargetDevice.GetSettings().GetStyleSettings().GetOptions() & StyleSettingsOptions::NoMnemonics) && !pVector;

    // We treat multiline text differently
    if ( nStyle & DrawTextFlags::MultiLine )
    {

        ImplMultiTextLineInfo   aMultiLineInfo;
        sal_Int32               i;
        sal_Int32               nFormatLines;

        if ( nTextHeight )
        {
            tools::Long nMaxTextWidth = ImplGetTextLines( rRect, nTextHeight, aMultiLineInfo, nWidth, aStr, nStyle, _rLayout );
            sal_Int32 nLines = static_cast<sal_Int32>(nHeight/nTextHeight);
            OUString aLastLine;
            nFormatLines = aMultiLineInfo.Count();
            if (nLines <= 0)
                nLines = 1;
            if ( nFormatLines > nLines )
            {
                if ( nStyle & DrawTextFlags::EndEllipsis )
                {
                    // Create last line and shorten it
                    nFormatLines = nLines-1;

                    ImplTextLineInfo& rLineInfo = aMultiLineInfo.GetLine( nFormatLines );
                    aLastLine = convertLineEnd(aStr.copy(rLineInfo.GetIndex()), LINEEND_LF);
                    // Replace all LineFeeds with Spaces
                    OUStringBuffer aLastLineBuffer(aLastLine);
                    sal_Int32 nLastLineLen = aLastLineBuffer.getLength();
                    for ( i = 0; i < nLastLineLen; i++ )
                    {
                        if ( aLastLineBuffer[ i ] == '\n' )
                            aLastLineBuffer[ i ] = ' ';
                    }
                    aLastLine = aLastLineBuffer.makeStringAndClear();
                    aLastLine = ImplGetEllipsisString( rTargetDevice, aLastLine, nWidth, nStyle, _rLayout );
                    nStyle &= ~DrawTextFlags(DrawTextFlags::VCenter | DrawTextFlags::Bottom);
                    nStyle |= DrawTextFlags::Top;
                }
            }
            else
            {
                if ( nMaxTextWidth <= nWidth )
                    nStyle &= ~DrawTextFlags::Clip;
            }

            // Do we need to clip the height?
            if ( nFormatLines*nTextHeight > nHeight )
                nStyle |= DrawTextFlags::Clip;

            // Set clipping
            if ( nStyle & DrawTextFlags::Clip )
            {
                rTargetDevice.Push( vcl::PushFlags::CLIPREGION );
                rTargetDevice.IntersectClipRegion( rRect );
            }

            // Vertical alignment
            if ( nStyle & DrawTextFlags::Bottom )
                aPos.AdjustY(nHeight-(nFormatLines*nTextHeight) );
            else if ( nStyle & DrawTextFlags::VCenter )
                aPos.AdjustY((nHeight-(nFormatLines*nTextHeight))/2 );

            // Font alignment
            if ( eAlign == ALIGN_BOTTOM )
                aPos.AdjustY(nTextHeight );
            else if ( eAlign == ALIGN_BASELINE )
                aPos.AdjustY(rTargetDevice.GetFontMetric().GetAscent() );

            // Output all lines except for the last one
            for ( i = 0; i < nFormatLines; i++ )
            {
                ImplTextLineInfo& rLineInfo = aMultiLineInfo.GetLine( i );
                if ( nStyle & DrawTextFlags::Right )
                    aPos.AdjustX(nWidth-rLineInfo.GetWidth() );
                else if ( nStyle & DrawTextFlags::Center )
                    aPos.AdjustX((nWidth-rLineInfo.GetWidth())/2 );
                sal_Int32 nIndex   = rLineInfo.GetIndex();
                sal_Int32 nLineLen = rLineInfo.GetLen();
                _rLayout.DrawText( aPos, aStr, nIndex, nLineLen, pVector, pDisplayText );
                if ( bDrawMnemonics )
                {
                    if ( (nMnemonicPos >= nIndex) && (nMnemonicPos < nIndex+nLineLen) )
                    {
                        tools::Long        nMnemonicX;
                        tools::Long        nMnemonicY;
                        DeviceCoordinate nMnemonicWidth;

                        std::unique_ptr<sal_Int32[]> const pCaretXArray(new sal_Int32[2 * nLineLen]);
                        /*sal_Bool bRet =*/ _rLayout.GetCaretPositions( aStr, pCaretXArray.get(),
                                                nIndex, nLineLen );
                        sal_Int32 lc_x1 = pCaretXArray[2*(nMnemonicPos - nIndex)];
                        sal_Int32 lc_x2 = pCaretXArray[2*(nMnemonicPos - nIndex)+1];
                        nMnemonicWidth = rTargetDevice.LogicWidthToDeviceCoordinate( std::abs(lc_x1 - lc_x2) );

                        Point       aTempPos = rTargetDevice.LogicToPixel( aPos );
                        nMnemonicX = rTargetDevice.GetOutOffXPixel() + aTempPos.X() + rTargetDevice.ImplLogicWidthToDevicePixel( std::min( lc_x1, lc_x2 ) );
                        nMnemonicY = rTargetDevice.GetOutOffYPixel() + aTempPos.Y() + rTargetDevice.ImplLogicWidthToDevicePixel( rTargetDevice.GetFontMetric().GetAscent() );
                        rTargetDevice.ImplDrawMnemonicLine( nMnemonicX, nMnemonicY, nMnemonicWidth );
                    }
                }
                aPos.AdjustY(nTextHeight );
                aPos.setX( rRect.Left() );
            }

            // If there still is a last line, we output it left-aligned as the line would be clipped
            if ( !aLastLine.isEmpty() )
                _rLayout.DrawText( aPos, aLastLine, 0, aLastLine.getLength(), pVector, pDisplayText );

            // Reset clipping
            if ( nStyle & DrawTextFlags::Clip )
                rTargetDevice.Pop();
        }
    }
    else
    {
        tools::Long nTextWidth = _rLayout.GetTextWidth( aStr, 0, -1 );

        // Clip text if needed
        if ( nTextWidth > nWidth )
        {
            if ( nStyle & TEXT_DRAW_ELLIPSIS )
            {
                aStr = ImplGetEllipsisString( rTargetDevice, aStr, nWidth, nStyle, _rLayout );
                nStyle &= ~DrawTextFlags(DrawTextFlags::Center | DrawTextFlags::Right);
                nStyle |= DrawTextFlags::Left;
                nTextWidth = _rLayout.GetTextWidth( aStr, 0, aStr.getLength() );
            }
        }
        else
        {
            if ( nTextHeight <= nHeight )
                nStyle &= ~DrawTextFlags::Clip;
        }

        // horizontal text alignment
        if ( nStyle & DrawTextFlags::Right )
            aPos.AdjustX(nWidth-nTextWidth );
        else if ( nStyle & DrawTextFlags::Center )
            aPos.AdjustX((nWidth-nTextWidth)/2 );

        // vertical font alignment
        if ( eAlign == ALIGN_BOTTOM )
            aPos.AdjustY(nTextHeight );
        else if ( eAlign == ALIGN_BASELINE )
            aPos.AdjustY(rTargetDevice.GetFontMetric().GetAscent() );

        if ( nStyle & DrawTextFlags::Bottom )
            aPos.AdjustY(nHeight-nTextHeight );
        else if ( nStyle & DrawTextFlags::VCenter )
            aPos.AdjustY((nHeight-nTextHeight)/2 );

        tools::Long nMnemonicX = 0;
        tools::Long nMnemonicY = 0;
        DeviceCoordinate nMnemonicWidth = 0;
        if ( nMnemonicPos != -1 )
        {
            std::unique_ptr<sal_Int32[]> const pCaretXArray(new sal_Int32[2 * aStr.getLength()]);
            /*sal_Bool bRet =*/ _rLayout.GetCaretPositions( aStr, pCaretXArray.get(), 0, aStr.getLength() );
            sal_Int32 lc_x1 = pCaretXArray[2*nMnemonicPos];
            sal_Int32 lc_x2 = pCaretXArray[2*nMnemonicPos+1];
            nMnemonicWidth = rTargetDevice.LogicWidthToDeviceCoordinate( std::abs(lc_x1 - lc_x2) );

            Point aTempPos = rTargetDevice.LogicToPixel( aPos );
            nMnemonicX = rTargetDevice.GetOutOffXPixel() + aTempPos.X() + rTargetDevice.ImplLogicWidthToDevicePixel( std::min(lc_x1, lc_x2) );
            nMnemonicY = rTargetDevice.GetOutOffYPixel() + aTempPos.Y() + rTargetDevice.ImplLogicWidthToDevicePixel( rTargetDevice.GetFontMetric().GetAscent() );
        }

        if ( nStyle & DrawTextFlags::Clip )
        {
            rTargetDevice.Push( vcl::PushFlags::CLIPREGION );
            rTargetDevice.IntersectClipRegion( rRect );
            _rLayout.DrawText( aPos, aStr, 0, aStr.getLength(), pVector, pDisplayText );
            if ( bDrawMnemonics && nMnemonicPos != -1 )
                rTargetDevice.ImplDrawMnemonicLine( nMnemonicX, nMnemonicY, nMnemonicWidth );
            rTargetDevice.Pop();
        }
        else
        {
            _rLayout.DrawText( aPos, aStr, 0, aStr.getLength(), pVector, pDisplayText );
            if ( bDrawMnemonics && nMnemonicPos != -1 )
                rTargetDevice.ImplDrawMnemonicLine( nMnemonicX, nMnemonicY, nMnemonicWidth );
        }
    }

    if ( nStyle & DrawTextFlags::Disable && !pVector )
    {
        rTargetDevice.SetTextColor( aOldTextColor );
        if ( bRestoreFillColor )
            rTargetDevice.SetTextFillColor( aOldTextFillColor );
    }
}

void OutputDevice::AddTextRectActions( const tools::Rectangle& rRect,
                                       const OUString&  rOrigStr,
                                       DrawTextFlags    nStyle,
                                       GDIMetaFile&     rMtf )
{

    if ( rOrigStr.isEmpty() || rRect.IsEmpty() )
        return;

    // we need a graphics
    if( !mpGraphics && !AcquireGraphics() )
        return;
    assert(mpGraphics);
    if( mbInitClipRegion )
        InitClipRegion();

    // temporarily swap in passed mtf for action generation, and
    // disable output generation.
    const bool bOutputEnabled( IsOutputEnabled() );
    GDIMetaFile* pMtf = mpMetaFile;

    mpMetaFile = &rMtf;
    EnableOutput( false );

    // #i47157# Factored out to ImplDrawTextRect(), to be shared
    // between us and DrawText()
    vcl::DefaultTextLayout aLayout( *this );
    ImplDrawText( *this, rRect, rOrigStr, nStyle, nullptr, nullptr, aLayout );

    // and restore again
    EnableOutput( bOutputEnabled );
    mpMetaFile = pMtf;
}

void OutputDevice::DrawText( const tools::Rectangle& rRect, const OUString& rOrigStr, DrawTextFlags nStyle,
                             std::vector< tools::Rectangle >* pVector, OUString* pDisplayText,
                             vcl::ITextLayout* _pTextLayout )
{
    assert(!is_double_buffered_window());

    if (mpOutDevData->mpRecordLayout)
    {
        pVector = &mpOutDevData->mpRecordLayout->m_aUnicodeBoundRects;
        pDisplayText = &mpOutDevData->mpRecordLayout->m_aDisplayText;
    }

    bool bDecomposeTextRectAction = ( _pTextLayout != nullptr ) && _pTextLayout->DecomposeTextRectAction();
    if ( mpMetaFile && !bDecomposeTextRectAction )
        mpMetaFile->AddAction( new MetaTextRectAction( rRect, rOrigStr, nStyle ) );

    if ( ( !IsDeviceOutputNecessary() && !pVector && !bDecomposeTextRectAction ) || rOrigStr.isEmpty() || rRect.IsEmpty() )
        return;

    // we need a graphics
    if( !mpGraphics && !AcquireGraphics() )
        return;
    assert(mpGraphics);
    if( mbInitClipRegion )
        InitClipRegion();
    if( mbOutputClipped && !bDecomposeTextRectAction )
        return;

    // temporarily disable mtf action generation (ImplDrawText _does_
    // create MetaActionType::TEXTs otherwise)
    GDIMetaFile* pMtf = mpMetaFile;
    if ( !bDecomposeTextRectAction )
        mpMetaFile = nullptr;

    // #i47157# Factored out to ImplDrawText(), to be used also
    // from AddTextRectActions()
    vcl::DefaultTextLayout aDefaultLayout( *this );
    ImplDrawText( *this, rRect, rOrigStr, nStyle, pVector, pDisplayText, _pTextLayout ? *_pTextLayout : aDefaultLayout );

    // and enable again
    mpMetaFile = pMtf;

    if( mpAlphaVDev )
        mpAlphaVDev->DrawText( rRect, rOrigStr, nStyle, pVector, pDisplayText );
}

tools::Rectangle OutputDevice::GetTextRect( const tools::Rectangle& rRect,
                                     const OUString& rStr, DrawTextFlags nStyle,
                                     TextRectInfo* pInfo,
                                     const vcl::ITextLayout* _pTextLayout ) const
{

    tools::Rectangle           aRect = rRect;
    sal_Int32           nLines;
    tools::Long                nWidth = rRect.GetWidth();
    tools::Long                nMaxWidth;
    tools::Long                nTextHeight = GetTextHeight();

    OUString aStr = rStr;
    if ( nStyle & DrawTextFlags::Mnemonic )
        aStr = GetNonMnemonicString( aStr );

    if ( nStyle & DrawTextFlags::MultiLine )
    {
        ImplMultiTextLineInfo   aMultiLineInfo;
        sal_Int32               nFormatLines;
        sal_Int32               i;

        nMaxWidth = 0;
        vcl::DefaultTextLayout aDefaultLayout( *const_cast< OutputDevice* >( this ) );
        ImplGetTextLines( rRect, nTextHeight, aMultiLineInfo, nWidth, aStr, nStyle, _pTextLayout ? *_pTextLayout : aDefaultLayout );
        nFormatLines = aMultiLineInfo.Count();
        if ( !nTextHeight )
            nTextHeight = 1;
        nLines = static_cast<sal_uInt16>(aRect.GetHeight()/nTextHeight);
        if ( pInfo )
            pInfo->mnLineCount = nFormatLines;
        if ( !nLines )
            nLines = 1;
        if ( nFormatLines <= nLines )
            nLines = nFormatLines;
        else
        {
            if ( !(nStyle & DrawTextFlags::EndEllipsis) )
                nLines = nFormatLines;
            else
            {
                if ( pInfo )
                    pInfo->mbEllipsis = true;
                nMaxWidth = nWidth;
            }
        }
        if ( pInfo )
        {
            bool bMaxWidth = nMaxWidth == 0;
            pInfo->mnMaxWidth = 0;
            for ( i = 0; i < nLines; i++ )
            {
                ImplTextLineInfo& rLineInfo = aMultiLineInfo.GetLine( i );
                if ( bMaxWidth && (rLineInfo.GetWidth() > nMaxWidth) )
                    nMaxWidth = rLineInfo.GetWidth();
                if ( rLineInfo.GetWidth() > pInfo->mnMaxWidth )
                    pInfo->mnMaxWidth = rLineInfo.GetWidth();
            }
        }
        else if ( !nMaxWidth )
        {
            for ( i = 0; i < nLines; i++ )
            {
                ImplTextLineInfo& rLineInfo = aMultiLineInfo.GetLine( i );
                if ( rLineInfo.GetWidth() > nMaxWidth )
                    nMaxWidth = rLineInfo.GetWidth();
            }
        }
    }
    else
    {
        nLines      = 1;
        nMaxWidth   = _pTextLayout ? _pTextLayout->GetTextWidth( aStr, 0, aStr.getLength() ) : GetTextWidth( aStr );

        if ( pInfo )
        {
            pInfo->mnLineCount  = 1;
            pInfo->mnMaxWidth   = nMaxWidth;
        }

        if ( (nMaxWidth > nWidth) && (nStyle & TEXT_DRAW_ELLIPSIS) )
        {
            if ( pInfo )
                pInfo->mbEllipsis = true;
            nMaxWidth = nWidth;
        }
    }

    if ( nStyle & DrawTextFlags::Right )
        aRect.SetLeft( aRect.Right()-nMaxWidth+1 );
    else if ( nStyle & DrawTextFlags::Center )
    {
        aRect.AdjustLeft((nWidth-nMaxWidth)/2 );
        aRect.SetRight( aRect.Left()+nMaxWidth-1 );
    }
    else
        aRect.SetRight( aRect.Left()+nMaxWidth-1 );

    if ( nStyle & DrawTextFlags::Bottom )
        aRect.SetTop( aRect.Bottom()-(nTextHeight*nLines)+1 );
    else if ( nStyle & DrawTextFlags::VCenter )
    {
        aRect.AdjustTop((aRect.GetHeight()-(nTextHeight*nLines))/2 );
        aRect.SetBottom( aRect.Top()+(nTextHeight*nLines)-1 );
    }
    else
        aRect.SetBottom( aRect.Top()+(nTextHeight*nLines)-1 );

    // #99188# get rid of rounding problems when using this rect later
    if (nStyle & DrawTextFlags::Right)
        aRect.AdjustLeft( -1 );
    else
        aRect.AdjustRight( 1 );
    return aRect;
}

static bool ImplIsCharIn( sal_Unicode c, const char* pStr )
{
    while ( *pStr )
    {
        if ( *pStr == c )
            return true;
        pStr++;
    }

    return false;
}

OUString OutputDevice::GetEllipsisString( const OUString& rOrigStr, tools::Long nMaxWidth,
                                        DrawTextFlags nStyle ) const
{
    vcl::DefaultTextLayout aTextLayout( *const_cast< OutputDevice* >( this ) );
    return ImplGetEllipsisString( *this, rOrigStr, nMaxWidth, nStyle, aTextLayout );
}

OUString OutputDevice::ImplGetEllipsisString( const OutputDevice& rTargetDevice, const OUString& rOrigStr, tools::Long nMaxWidth,
                                               DrawTextFlags nStyle, const vcl::ITextLayout& _rLayout )
{
    OUString aStr = rOrigStr;
    sal_Int32 nIndex = _rLayout.GetTextBreak( aStr, nMaxWidth, 0, aStr.getLength() );

    if ( nIndex != -1 )
    {
        if( (nStyle & DrawTextFlags::CenterEllipsis) == DrawTextFlags::CenterEllipsis )
        {
            OUStringBuffer aTmpStr( aStr );
            // speed it up by removing all but 1.33x as many as the break pos.
            sal_Int32 nEraseChars = std::max<sal_Int32>(4, aStr.getLength() - (nIndex*4)/3);
            while( nEraseChars < aStr.getLength() && _rLayout.GetTextWidth( aTmpStr.toString(), 0, aTmpStr.getLength() ) > nMaxWidth )
            {
                aTmpStr = aStr;
                sal_Int32 i = (aTmpStr.getLength() - nEraseChars)/2;
                aTmpStr.remove(i, nEraseChars++);
                aTmpStr.insert(i, "...");
            }
            aStr = aTmpStr.makeStringAndClear();
        }
        else if ( nStyle & DrawTextFlags::EndEllipsis )
        {
            aStr = aStr.copy(0, nIndex);
            if ( nIndex > 1 )
            {
                aStr += "...";
                while ( !aStr.isEmpty() && (_rLayout.GetTextWidth( aStr, 0, aStr.getLength() ) > nMaxWidth) )
                {
                    if ( (nIndex > 1) || (nIndex == aStr.getLength()) )
                        nIndex--;
                    aStr = aStr.replaceAt( nIndex, 1, u"");
                }
            }

            if ( aStr.isEmpty() && (nStyle & DrawTextFlags::Clip) )
                aStr += OUStringChar(rOrigStr[ 0 ]);
        }
        else if ( nStyle & DrawTextFlags::PathEllipsis )
        {
            OUString aPath( rOrigStr );
            OUString aAbbreviatedPath;
            osl_abbreviateSystemPath( aPath.pData, &aAbbreviatedPath.pData, nIndex, nullptr );
            aStr = aAbbreviatedPath;
        }
        else if ( nStyle & DrawTextFlags::NewsEllipsis )
        {
            static char const   pSepChars[] = ".";
            // Determine last section
            sal_Int32 nLastContent = aStr.getLength();
            while ( nLastContent )
            {
                nLastContent--;
                if ( ImplIsCharIn( aStr[ nLastContent ], pSepChars ) )
                    break;
            }
            while ( nLastContent &&
                    ImplIsCharIn( aStr[ nLastContent-1 ], pSepChars ) )
                nLastContent--;

            OUString aLastStr = aStr.copy(nLastContent);
            OUString aTempLastStr1 = "..." + aLastStr;
            if ( _rLayout.GetTextWidth( aTempLastStr1, 0, aTempLastStr1.getLength() ) > nMaxWidth )
                aStr = OutputDevice::ImplGetEllipsisString( rTargetDevice, aStr, nMaxWidth, nStyle | DrawTextFlags::EndEllipsis, _rLayout );
            else
            {
                sal_Int32 nFirstContent = 0;
                while ( nFirstContent < nLastContent )
                {
                    nFirstContent++;
                    if ( ImplIsCharIn( aStr[ nFirstContent ], pSepChars ) )
                        break;
                }
                while ( (nFirstContent < nLastContent) &&
                        ImplIsCharIn( aStr[ nFirstContent ], pSepChars ) )
                    nFirstContent++;
                // MEM continue here
                if ( nFirstContent >= nLastContent )
                    aStr = OutputDevice::ImplGetEllipsisString( rTargetDevice, aStr, nMaxWidth, nStyle | DrawTextFlags::EndEllipsis, _rLayout );
                else
                {
                    if ( nFirstContent > 4 )
                        nFirstContent = 4;
                    OUString aFirstStr = OUString::Concat(aStr.subView( 0, nFirstContent )) + "...";
                    OUString aTempStr = aFirstStr + aLastStr;
                    if ( _rLayout.GetTextWidth( aTempStr, 0, aTempStr.getLength() ) > nMaxWidth )
                        aStr = OutputDevice::ImplGetEllipsisString( rTargetDevice, aStr, nMaxWidth, nStyle | DrawTextFlags::EndEllipsis, _rLayout );
                    else
                    {
                        do
                        {
                            aStr = aTempStr;
                            if( nLastContent > aStr.getLength() )
                                nLastContent = aStr.getLength();
                            while ( nFirstContent < nLastContent )
                            {
                                nLastContent--;
                                if ( ImplIsCharIn( aStr[ nLastContent ], pSepChars ) )
                                    break;

                            }
                            while ( (nFirstContent < nLastContent) &&
                                    ImplIsCharIn( aStr[ nLastContent-1 ], pSepChars ) )
                                nLastContent--;

                            if ( nFirstContent < nLastContent )
                            {
                                OUString aTempLastStr = aStr.copy( nLastContent );
                                aTempStr = aFirstStr + aTempLastStr;

                                if ( _rLayout.GetTextWidth( aTempStr, 0, aTempStr.getLength() ) > nMaxWidth )
                                    break;
                            }
                        }
                        while ( nFirstContent < nLastContent );
                    }
                }
            }
        }
    }

    return aStr;
}

void OutputDevice::DrawCtrlText( const Point& rPos, const OUString& rStr,
                                 sal_Int32 nIndex, sal_Int32 nLen,
                                 DrawTextFlags nStyle, std::vector< tools::Rectangle >* pVector, OUString* pDisplayText,
                                 const SalLayoutGlyphs* pGlyphs )
{
    assert(!is_double_buffered_window());

    if( (nLen < 0) || (nIndex + nLen >= rStr.getLength()))
    {
        nLen = rStr.getLength() - nIndex;
    }

    if ( !IsDeviceOutputNecessary() || (nIndex >= rStr.getLength()) )
        return;

    // better get graphics here because ImplDrawMnemonicLine() will not
    // we need a graphics
    if( !mpGraphics && !AcquireGraphics() )
        return;
    assert(mpGraphics);
    if( mbInitClipRegion )
        InitClipRegion();
    if ( mbOutputClipped )
        return;

    if( nIndex >= rStr.getLength() )
        return;

    if( (nLen < 0) || (nIndex + nLen >= rStr.getLength()))
    {
        nLen = rStr.getLength() - nIndex;
    }
    OUString   aStr = rStr;
    sal_Int32  nMnemonicPos = -1;

    tools::Long        nMnemonicX = 0;
    tools::Long        nMnemonicY = 0;
    tools::Long        nMnemonicWidth = 0;
    if ( (nStyle & DrawTextFlags::Mnemonic) && nLen > 1 )
    {
        aStr = GetNonMnemonicString( aStr, nMnemonicPos );
        if ( nMnemonicPos != -1 )
        {
            if( nMnemonicPos < nIndex )
            {
                --nIndex;
            }
            else
            {
                if( nMnemonicPos < (nIndex+nLen) )
                    --nLen;
                SAL_WARN_IF( nMnemonicPos >= (nIndex+nLen), "vcl", "Mnemonic underline marker after last character" );
            }
            bool bInvalidPos = false;

            if( nMnemonicPos >= nLen )
            {
                // may occur in BiDi-Strings: the '~' is sometimes found behind the last char
                // due to some strange BiDi text editors
                // -> place the underline behind the string to indicate a failure
                bInvalidPos = true;
                nMnemonicPos = nLen-1;
            }

            std::unique_ptr<sal_Int32[]> const pCaretXArray(new sal_Int32[2 * nLen]);
            /*sal_Bool bRet =*/ GetCaretPositions( aStr, pCaretXArray.get(), nIndex, nLen, pGlyphs );
            sal_Int32 lc_x1 = pCaretXArray[ 2*(nMnemonicPos - nIndex) ];
            sal_Int32 lc_x2 = pCaretXArray[ 2*(nMnemonicPos - nIndex)+1 ];
            nMnemonicWidth = ::abs(static_cast<int>(lc_x1 - lc_x2));

            Point aTempPos( std::min(lc_x1,lc_x2), GetFontMetric().GetAscent() );
            if( bInvalidPos )  // #106952#, place behind the (last) character
                aTempPos = Point( std::max(lc_x1,lc_x2), GetFontMetric().GetAscent() );

            aTempPos += rPos;
            aTempPos = LogicToPixel( aTempPos );
            nMnemonicX = mnOutOffX + aTempPos.X();
            nMnemonicY = mnOutOffY + aTempPos.Y();
        }
    }

    bool autoacc = ImplGetSVData()->maNWFData.mbAutoAccel;

    if ( nStyle & DrawTextFlags::Disable && ! pVector )
    {
        Color aOldTextColor;
        Color aOldTextFillColor;
        bool  bRestoreFillColor;
        bool  bHighContrastBlack = false;
        bool  bHighContrastWhite = false;
        const StyleSettings& rStyleSettings( GetSettings().GetStyleSettings() );
        if( rStyleSettings.GetHighContrastMode() )
        {
            if( IsBackground() )
            {
                Wallpaper aWall = GetBackground();
                Color aCol = aWall.GetColor();
                bHighContrastBlack = aCol.IsDark();
                bHighContrastWhite = aCol.IsBright();
            }
        }

        aOldTextColor = GetTextColor();
        if ( IsTextFillColor() )
        {
            bRestoreFillColor = true;
            aOldTextFillColor = GetTextFillColor();
        }
        else
            bRestoreFillColor = false;

        if( bHighContrastBlack )
            SetTextColor( COL_GREEN );
        else if( bHighContrastWhite )
            SetTextColor( COL_LIGHTGREEN );
        else
            SetTextColor( GetSettings().GetStyleSettings().GetDisableColor() );

        DrawText( rPos, aStr, nIndex, nLen, pVector, pDisplayText );
        if (!(GetSettings().GetStyleSettings().GetOptions() & StyleSettingsOptions::NoMnemonics)
            && (!autoacc || !(nStyle & DrawTextFlags::HideMnemonic)) )
        {
            if ( nMnemonicPos != -1 )
                ImplDrawMnemonicLine( nMnemonicX, nMnemonicY, nMnemonicWidth );
        }
        SetTextColor( aOldTextColor );
        if ( bRestoreFillColor )
            SetTextFillColor( aOldTextFillColor );
    }
    else
    {
        DrawText( rPos, aStr, nIndex, nLen, pVector, pDisplayText, pGlyphs );
        if ( !(GetSettings().GetStyleSettings().GetOptions() & StyleSettingsOptions::NoMnemonics) && !pVector
            && (!autoacc || !(nStyle & DrawTextFlags::HideMnemonic)) )
        {
            if ( nMnemonicPos != -1 )
                ImplDrawMnemonicLine( nMnemonicX, nMnemonicY, nMnemonicWidth );
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawCtrlText( rPos, rStr, nIndex, nLen, nStyle, pVector, pDisplayText );
}

tools::Long OutputDevice::GetCtrlTextWidth( const OUString& rStr, const SalLayoutGlyphs* pGlyphs ) const
{
    sal_Int32 nLen = rStr.getLength();
    sal_Int32 nIndex = 0;

    sal_Int32  nMnemonicPos;
    OUString   aStr = GetNonMnemonicString( rStr, nMnemonicPos );
    if ( nMnemonicPos != -1 )
    {
        if ( nMnemonicPos < nIndex )
            nIndex--;
        else if (static_cast<sal_uLong>(nMnemonicPos) < static_cast<sal_uLong>(nIndex+nLen))
            nLen--;
    }
    return GetTextWidth( aStr, nIndex, nLen, nullptr, pGlyphs );
}

OUString OutputDevice::GetNonMnemonicString( const OUString& rStr, sal_Int32& rMnemonicPos )
{
    OUString   aStr    = rStr;
    sal_Int32  nLen    = aStr.getLength();
    sal_Int32  i       = 0;

    rMnemonicPos = -1;
    while ( i < nLen )
    {
        if ( aStr[ i ] == '~' )
        {
            if ( nLen <= i+1 )
                break;

            if ( aStr[ i+1 ] != '~' )
            {
                if ( rMnemonicPos == -1 )
                    rMnemonicPos = i;
                aStr = aStr.replaceAt( i, 1, u"" );
                nLen--;
            }
            else
            {
                aStr = aStr.replaceAt( i, 1, u"" );
                nLen--;
                i++;
            }
        }
        else
            i++;
    }

    return aStr;
}

bool OutputDevice::GetTextBoundRect( tools::Rectangle& rRect,
                                         const OUString& rStr, sal_Int32 nBase,
                                         sal_Int32 nIndex, sal_Int32 nLen,
                                         sal_uLong nLayoutWidth, o3tl::span<const sal_Int32> pDXAry,
                                         const SalLayoutGlyphs* pGlyphs ) const
{
    bool bRet = false;
    rRect.SetEmpty();

    std::unique_ptr<SalLayout> pSalLayout;
    const Point aPoint;
    // calculate offset when nBase!=nIndex
    tools::Long nXOffset = 0;
    if( nBase != nIndex )
    {
        sal_Int32 nStart = std::min( nBase, nIndex );
        sal_Int32 nOfsLen = std::max( nBase, nIndex ) - nStart;
        pSalLayout = ImplLayout( rStr, nStart, nOfsLen, aPoint, nLayoutWidth, pDXAry );
        if( pSalLayout )
        {
            nXOffset = pSalLayout->GetTextWidth();
            nXOffset /= pSalLayout->GetUnitsPerPixel();
            // TODO: fix offset calculation for Bidi case
            if( nBase < nIndex)
                nXOffset = -nXOffset;
        }
    }

    pSalLayout = ImplLayout(rStr, nIndex, nLen, aPoint, nLayoutWidth, pDXAry, eDefaultLayout,
                            nullptr, pGlyphs);
    if( pSalLayout )
    {
        tools::Rectangle aPixelRect;
        bRet = pSalLayout->GetBoundRect(aPixelRect);

        if( bRet )
        {
            int nWidthFactor = pSalLayout->GetUnitsPerPixel();

            if( nWidthFactor > 1 )
            {
                double fFactor = 1.0 / nWidthFactor;
                aPixelRect.SetLeft(
                    static_cast< tools::Long >(aPixelRect.Left() * fFactor) );
                aPixelRect.SetRight(
                    static_cast< tools::Long >(aPixelRect.Right() * fFactor) );
                aPixelRect.SetTop(
                    static_cast< tools::Long >(aPixelRect.Top() * fFactor) );
                aPixelRect.SetBottom(
                    static_cast< tools::Long >(aPixelRect.Bottom() * fFactor) );
            }

            Point aRotatedOfs( mnTextOffX, mnTextOffY );
            aRotatedOfs -= pSalLayout->GetDrawPosition( Point( nXOffset, 0 ) );
            aPixelRect += aRotatedOfs;
            rRect = PixelToLogic( aPixelRect );
            if( mbMap )
                rRect += Point( maMapRes.mnMapOfsX, maMapRes.mnMapOfsY );
        }
    }

    return bRet;
}

bool OutputDevice::GetTextOutlines( basegfx::B2DPolyPolygonVector& rVector,
                                        const OUString& rStr, sal_Int32 nBase,
                                        sal_Int32 nIndex, sal_Int32 nLen,
                                        sal_uLong nLayoutWidth, o3tl::span<const sal_Int32> pDXArray ) const
{
    if (!InitFont())
        return false;

    bool bRet = false;
    rVector.clear();
    if( nLen < 0 )
    {
        nLen = rStr.getLength() - nIndex;
    }
    rVector.reserve( nLen );

    // we want to get the Rectangle in logical units, so to
    // avoid rounding errors we just size the font in logical units
    bool bOldMap = mbMap;
    if( bOldMap )
    {
        const_cast<OutputDevice&>(*this).mbMap = false;
        const_cast<OutputDevice&>(*this).mbNewFont = true;
    }

    std::unique_ptr<SalLayout> pSalLayout;

    // calculate offset when nBase!=nIndex
    tools::Long nXOffset = 0;
    if( nBase != nIndex )
    {
        sal_Int32 nStart = std::min( nBase, nIndex );
        sal_Int32 nOfsLen = std::max( nBase, nIndex ) - nStart;
        pSalLayout = ImplLayout( rStr, nStart, nOfsLen, Point(0,0), nLayoutWidth, pDXArray );
        if( pSalLayout )
        {
            nXOffset = pSalLayout->GetTextWidth();
            pSalLayout.reset();
            // TODO: fix offset calculation for Bidi case
            if( nBase > nIndex)
                nXOffset = -nXOffset;
        }
    }

    pSalLayout = ImplLayout( rStr, nIndex, nLen, Point(0,0), nLayoutWidth, pDXArray );
    if( pSalLayout )
    {
        bRet = pSalLayout->GetOutline(rVector);
        if( bRet )
        {
            // transform polygon to pixel units
            basegfx::B2DHomMatrix aMatrix;

            int nWidthFactor = pSalLayout->GetUnitsPerPixel();
            if( nXOffset | mnTextOffX | mnTextOffY )
            {
                Point aRotatedOfs( mnTextOffX*nWidthFactor, mnTextOffY*nWidthFactor );
                aRotatedOfs -= pSalLayout->GetDrawPosition( Point( nXOffset, 0 ) );
                aMatrix.translate( aRotatedOfs.X(), aRotatedOfs.Y() );
            }

            if( nWidthFactor > 1 )
            {
                double fFactor = 1.0 / nWidthFactor;
                aMatrix.scale( fFactor, fFactor );
            }

            if( !aMatrix.isIdentity() )
            {
                for (auto & elem : rVector)
                    elem.transform( aMatrix );
            }
        }

        pSalLayout.reset();
    }

    if( bOldMap )
    {
        // restore original font size and map mode
        const_cast<OutputDevice&>(*this).mbMap = bOldMap;
        const_cast<OutputDevice&>(*this).mbNewFont = true;
    }

    return bRet;
}

bool OutputDevice::GetTextOutlines( PolyPolyVector& rResultVector,
                                        const OUString& rStr, sal_Int32 nBase,
                                        sal_Int32 nIndex, sal_Int32 nLen,
                                        sal_uLong nLayoutWidth, o3tl::span<const sal_Int32> pDXArray ) const
{
    rResultVector.clear();

    // get the basegfx polypolygon vector
    basegfx::B2DPolyPolygonVector aB2DPolyPolyVector;
    if( !GetTextOutlines( aB2DPolyPolyVector, rStr, nBase, nIndex, nLen,
                         nLayoutWidth, pDXArray ) )
        return false;

    // convert to a tool polypolygon vector
    rResultVector.reserve( aB2DPolyPolyVector.size() );
    for (auto const& elem : aB2DPolyPolyVector)
        rResultVector.emplace_back(elem); // #i76339#

    return true;
}

bool OutputDevice::GetTextOutline( tools::PolyPolygon& rPolyPoly, const OUString& rStr ) const
{
    rPolyPoly.Clear();

    // get the basegfx polypolygon vector
    basegfx::B2DPolyPolygonVector aB2DPolyPolyVector;
    if( !GetTextOutlines( aB2DPolyPolyVector, rStr, 0/*nBase*/, 0/*nIndex*/, /*nLen*/-1,
                         /*nLayoutWidth*/0, /*pDXArray*/{} ) )
        return false;

    // convert and merge into a tool polypolygon
    for (auto const& elem : aB2DPolyPolyVector)
        for(auto const& rB2DPolygon : elem)
            rPolyPoly.Insert(tools::Polygon(rB2DPolygon)); // #i76339#

    return true;
}

void OutputDevice::SetSystemTextColor(SystemTextColorFlags nFlags, bool bEnabled)
{
    if (nFlags & SystemTextColorFlags::Mono)
    {
        SetTextColor(COL_BLACK);
    }
    else
    {
        if (!bEnabled)
        {
            const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
            SetTextColor(rStyleSettings.GetDisableColor());
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
