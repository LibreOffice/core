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

#include <cassert>
#include <cmath>

#include <sal/types.h>

#include <basegfx/matrix/b2dhommatrix.hxx>

#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/linguistic2/LinguServiceManager.hpp>

#include <comphelper/processfactory.hxx>

#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/settings.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/unohelp.hxx>
#include <vcl/controllayout.hxx>

#include <outdata.hxx>
#include <outdev.h>
#include <salgdi.hxx>
#include <svdata.hxx>
#include <textlayout.hxx>

#include <config_graphite.h>
#if ENABLE_GRAPHITE
#include "graphite_features.hxx"
#endif


#define TEXT_DRAW_ELLIPSIS  (DrawTextFlags::EndEllipsis | DrawTextFlags::PathEllipsis | DrawTextFlags::NewsEllipsis)

ImplMultiTextLineInfo::ImplMultiTextLineInfo()
{
    mpLines = new ImplTextLineInfo*[MULTITEXTLINEINFO_RESIZE];
    mnLines = 0;
    mnSize  = MULTITEXTLINEINFO_RESIZE;
}

ImplMultiTextLineInfo::~ImplMultiTextLineInfo()
{
    for( sal_Int32 i = 0; i < mnLines; i++ )
        delete mpLines[i];
    delete [] mpLines;
}

void ImplMultiTextLineInfo::AddLine( ImplTextLineInfo* pLine )
{
    if ( mnSize == mnLines )
    {
        mnSize += MULTITEXTLINEINFO_RESIZE;
        ImplTextLineInfo** pNewLines = new ImplTextLineInfo*[mnSize];
        memcpy( pNewLines, mpLines, mnLines*sizeof(ImplTextLineInfo*) );
        mpLines = pNewLines;
    }

    mpLines[mnLines] = pLine;
    mnLines++;
}

void ImplMultiTextLineInfo::Clear()
{
    for( sal_Int32 i = 0; i < mnLines; i++ )
        delete mpLines[i];
    mnLines = 0;
}

void OutputDevice::ImplInitTextColor()
{
    DBG_TESTSOLARMUTEX();

    if ( mbInitTextColor )
    {
        mpGraphics->SetTextColor( ImplColorToSal( GetTextColor() ) );
        mbInitTextColor = false;
    }
}

void OutputDevice::ImplDrawTextRect( long nBaseX, long nBaseY,
                                     long nDistX, long nDistY, long nWidth, long nHeight )
{
    long nX = nDistX;
    long nY = nDistY;

    short nOrientation = mpFontEntry->mnOrientation;
    if ( nOrientation )
    {
        // Rotate rect without rounding problems for 90 degree rotations
        if ( !(nOrientation % 900) )
        {
            if ( nOrientation == 900 )
            {
                long nTemp = nX;
                nX = nY;
                nY = -nTemp;
                nTemp = nWidth;
                nWidth = nHeight;
                nHeight = nTemp;
                nY -= nHeight;
            }
            else if ( nOrientation == 1800 )
            {
                nX = -nX;
                nY = -nY;
                nX -= nWidth;
                nY -= nHeight;
            }
            else /* ( nOrientation == 2700 ) */
            {
                long nTemp = nX;
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
            Rectangle aRect( Point( nX, nY ), Size( nWidth+1, nHeight+1 ) );
            tools::Polygon   aPoly( aRect );
            aPoly.Rotate( Point( nBaseX, nBaseY ), mpFontEntry->mnOrientation );
            ImplDrawPolygon( aPoly );
            return;
        }
    }

    nX += nBaseX;
    nY += nBaseY;
    mpGraphics->DrawRect( nX, nY, nWidth, nHeight, this ); // original code

}

void OutputDevice::ImplDrawTextBackground( const SalLayout& rSalLayout )
{
    const long nWidth = rSalLayout.GetTextWidth() / rSalLayout.GetUnitsPerPixel();
    const Point aBase = rSalLayout.DrawBase();
    const long nX = aBase.X();
    const long nY = aBase.Y();

    if ( mbLineColor || mbInitLineColor )
    {
        mpGraphics->SetLineColor();
        mbInitLineColor = true;
    }
    mpGraphics->SetFillColor( ImplColorToSal( GetTextFillColor() ) );
    mbInitFillColor = true;

    ImplDrawTextRect( nX, nY, 0, -(mpFontEntry->maMetric.mnAscent + mnEmphasisAscent),
                      nWidth,
                      mpFontEntry->mnLineHeight+mnEmphasisAscent+mnEmphasisDescent );
}

Rectangle OutputDevice::ImplGetTextBoundRect( const SalLayout& rSalLayout )
{
    Point aPoint = rSalLayout.GetDrawPosition();
    long nX = aPoint.X();
    long nY = aPoint.Y();

    long nWidth = rSalLayout.GetTextWidth();
    long nHeight = mpFontEntry->mnLineHeight + mnEmphasisAscent + mnEmphasisDescent;

    nY -= mpFontEntry->maMetric.mnAscent + mnEmphasisAscent;

    if ( mpFontEntry->mnOrientation )
    {
        long nBaseX = nX, nBaseY = nY;
        if ( !(mpFontEntry->mnOrientation % 900) )
        {
            long nX2 = nX+nWidth;
            long nY2 = nY+nHeight;

            Point aBasePt( nBaseX, nBaseY );
            aBasePt.RotateAround( nX, nY, mpFontEntry->mnOrientation );
            aBasePt.RotateAround( nX2, nY2, mpFontEntry->mnOrientation );
            nWidth = nX2-nX;
            nHeight = nY2-nY;
        }
        else
        {
            // inflate by +1+1 because polygons are drawn smaller
            Rectangle aRect( Point( nX, nY ), Size( nWidth+1, nHeight+1 ) );
            tools::Polygon   aPoly( aRect );
            aPoly.Rotate( Point( nBaseX, nBaseY ), mpFontEntry->mnOrientation );
            return aPoly.GetBoundRect();
        }
    }

    return Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) );
}

bool OutputDevice::ImplDrawRotateText( SalLayout& rSalLayout )
{
    int nX = rSalLayout.DrawBase().X();
    int nY = rSalLayout.DrawBase().Y();

    Rectangle aBoundRect;
    rSalLayout.DrawBase() = Point( 0, 0 );
    rSalLayout.DrawOffset() = Point( 0, 0 );
    if( !rSalLayout.GetBoundRect( *mpGraphics, aBoundRect ) )
    {
        // guess vertical text extents if GetBoundRect failed
        int nRight = rSalLayout.GetTextWidth();
        int nTop = mpFontEntry->maMetric.mnAscent + mnEmphasisAscent;
        long nHeight = mpFontEntry->mnLineHeight + mnEmphasisAscent + mnEmphasisDescent;
        aBoundRect = Rectangle( 0, -nTop, nRight, nHeight - nTop );
    }

    // cache virtual device for rotation
    if (!mpOutDevData->mpRotateDev)
        mpOutDevData->mpRotateDev = VclPtr<VirtualDevice>::Create(*this, DeviceFormat::BITMASK);
    VirtualDevice* pVDev = mpOutDevData->mpRotateDev;

    // size it accordingly
    if( !pVDev->SetOutputSizePixel( aBoundRect.GetSize() ) )
        return false;

    vcl::Font aFont( GetFont() );
    aFont.SetOrientation( 0 );
    aFont.SetSize( Size( mpFontEntry->maFontSelData.mnWidth, mpFontEntry->maFontSelData.mnHeight ) );
    pVDev->SetFont( aFont );
    pVDev->SetTextColor( Color( COL_BLACK ) );
    pVDev->SetTextFillColor();
    pVDev->ImplNewFont();
    pVDev->InitFont();
    pVDev->ImplInitTextColor();

    // draw text into upper left corner
    rSalLayout.DrawBase() -= aBoundRect.TopLeft();
    rSalLayout.DrawText( *static_cast<OutputDevice*>(pVDev)->mpGraphics );

    Bitmap aBmp = pVDev->GetBitmap( Point(), aBoundRect.GetSize() );
    if ( !aBmp || !aBmp.Rotate( mpFontEntry->mnOwnOrientation, COL_WHITE ) )
        return false;

    // calculate rotation offset
    tools::Polygon aPoly( aBoundRect );
    aPoly.Rotate( Point(), mpFontEntry->mnOwnOrientation );
    Point aPoint = aPoly.GetBoundRect().TopLeft();
    aPoint += Point( nX, nY );

    // mask output with text colored bitmap
    GDIMetaFile* pOldMetaFile = mpMetaFile;
    long nOldOffX = mnOutOffX;
    long nOldOffY = mnOutOffY;
    bool bOldMap = mbMap;

    mnOutOffX   = 0L;
    mnOutOffY   = 0L;
    mpMetaFile  = nullptr;
    EnableMapMode( false );

    DrawMask( aPoint, aBmp, GetTextColor() );

    EnableMapMode( bOldMap );
    mnOutOffX   = nOldOffX;
    mnOutOffY   = nOldOffY;
    mpMetaFile  = pOldMetaFile;

    return true;
}

bool OutputDevice::ImplDrawTextDirect( SalLayout& rSalLayout,
                                       bool bTextLines,
                                       sal_uInt32 flags )
{
    if( mpFontEntry->mnOwnOrientation )
        if( ImplDrawRotateText( rSalLayout ) )
            return true;




    long nOldX = rSalLayout.DrawBase().X();
    if( HasMirroredGraphics() )
    {
        long w = meOutDevType == OUTDEV_VIRDEV ? mnOutWidth : mpGraphics->GetGraphicsWidth();
        long x = rSalLayout.DrawBase().X();
           rSalLayout.DrawBase().X() = w - 1 - x;
        if( !IsRTLEnabled() )
        {
            OutputDevice *pOutDevRef = this;
            // mirror this window back
            long devX = w-pOutDevRef->mnOutWidth-pOutDevRef->mnOutOffX;   // re-mirrored mnOutOffX
            rSalLayout.DrawBase().X() = devX + ( pOutDevRef->mnOutWidth - 1 - (rSalLayout.DrawBase().X() - devX) ) ;
        }
    }
    else if( IsRTLEnabled() )
    {
        OutputDevice *pOutDevRef = this;

        // mirror this window back
        long devX = pOutDevRef->mnOutOffX;   // re-mirrored mnOutOffX
        rSalLayout.DrawBase().X() = pOutDevRef->mnOutWidth - 1 - (rSalLayout.DrawBase().X() - devX) + devX;
    }

    if(flags)
    {
        if( ! rSalLayout.DrawTextSpecial( *mpGraphics, flags ))
        {
            rSalLayout.DrawBase().X() = nOldX;
            return false;
        }
    }
    else
    {
        rSalLayout.DrawText( *mpGraphics );
    }
    rSalLayout.DrawBase().X() = nOldX;

    if( bTextLines )
        ImplDrawTextLines( rSalLayout,
            maFont.GetStrikeout(), maFont.GetUnderline(), maFont.GetOverline(),
            maFont.IsWordLineMode(), ImplIsUnderlineAbove( maFont ) );


    // emphasis marks
    if( maFont.GetEmphasisMark() & EMPHASISMARK_STYLE )
        ImplDrawEmphasisMarks( rSalLayout );

    return true;
}

void OutputDevice::ImplDrawSpecialText( SalLayout& rSalLayout )
{
    Color       aOldColor           = GetTextColor();
    Color       aOldTextLineColor   = GetTextLineColor();
    Color       aOldOverlineColor   = GetOverlineColor();
    FontRelief  eRelief             = maFont.GetRelief();

    Point aOrigPos = rSalLayout.DrawBase();
    if ( eRelief != RELIEF_NONE )
    {
        Color   aReliefColor( COL_LIGHTGRAY );
        Color   aTextColor( aOldColor );

        Color   aTextLineColor( aOldTextLineColor );
        Color   aOverlineColor( aOldOverlineColor );

        // we don't have a automatic color, so black is always drawn on white
        if ( aTextColor.GetColor() == COL_BLACK )
            aTextColor = Color( COL_WHITE );
        if ( aTextLineColor.GetColor() == COL_BLACK )
            aTextLineColor = Color( COL_WHITE );
        if ( aOverlineColor.GetColor() == COL_BLACK )
            aOverlineColor = Color( COL_WHITE );

        // relief-color is black for white text, in all other cases
        // we set this to LightGray
        if ( aTextColor.GetColor() == COL_WHITE )
            aReliefColor = Color( COL_BLACK );
        SetTextLineColor( aReliefColor );
        SetOverlineColor( aReliefColor );
        SetTextColor( aReliefColor );
        ImplInitTextColor();

        // calculate offset - for high resolution printers the offset
        // should be greater so that the effect is visible
        long nOff = 1;
        nOff += mnDPIX/300;

        if ( eRelief == RELIEF_ENGRAVED )
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
            long nOff = 1 + ((mpFontEntry->mnLineHeight-24)/24);
            if ( maFont.IsOutline() )
                nOff++;
            SetTextLineColor();
            SetOverlineColor();
            if ( (GetTextColor().GetColor() == COL_BLACK)
            ||   (GetTextColor().GetLuminance() < 8) )
                SetTextColor( Color( COL_LIGHTGRAY ) );
            else
                SetTextColor( Color( COL_BLACK ) );
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
            if(! ImplDrawTextDirect( rSalLayout, mbTextLines, DRAWTEXT_F_OUTLINE))
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

                SetTextColor( Color( COL_WHITE ) );
                SetTextLineColor( Color( COL_WHITE ) );
                SetOverlineColor( Color( COL_WHITE ) );
                ImplInitTextColor();
                ImplDrawTextDirect( rSalLayout, mbTextLines );
                SetTextColor( aOldColor );
                SetTextLineColor( aOldTextLineColor );
                SetOverlineColor( aOldOverlineColor );
                ImplInitTextColor();
            }
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

long OutputDevice::ImplGetTextLines( ImplMultiTextLineInfo& rLineInfo,
                                     long nWidth, const OUString& rStr,
                                     DrawTextFlags nStyle, const vcl::ITextLayout& _rLayout )
{
    SAL_WARN_IF( nWidth <= 0, "vcl", "ImplGetTextLines: nWidth <= 0!" );

    if ( nWidth <= 0 )
        nWidth = 1;

    long nMaxLineWidth  = 0;
    rLineInfo.Clear();
    if ( !rStr.isEmpty() && (nWidth > 0) )
    {
        css::uno::Reference < css::i18n::XBreakIterator > xBI;
        // get service provider
        css::uno::Reference< css::uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );

        bool bHyphenate = (nStyle & DrawTextFlags::WordBreakHyphenation)
            == DrawTextFlags::WordBreakHyphenation;
        css::uno::Reference< css::linguistic2::XHyphenator > xHyph;
        if ( bHyphenate )
        {
            css::uno::Reference< css::linguistic2::XLinguServiceManager2> xLinguMgr = css::linguistic2::LinguServiceManager::create(xContext);
            xHyph = xLinguMgr->getHyphenator();
        }

        sal_Int32 nPos = 0;
        sal_Int32 nLen = rStr.getLength();
        while ( nPos < nLen )
        {
            sal_Int32 nBreakPos = nPos;

            while ( ( nBreakPos < nLen ) && ( rStr[ nBreakPos ] != '\r' ) && ( rStr[ nBreakPos ] != '\n' ) )
                nBreakPos++;

            long nLineWidth = _rLayout.GetTextWidth( rStr, nPos, nBreakPos-nPos );
            if ( ( nLineWidth > nWidth ) && ( nStyle & DrawTextFlags::WordBreak ) )
            {
                if ( !xBI.is() )
                    xBI = vcl::unohelper::CreateBreakIterator();

                if ( xBI.is() )
                {
                    const css::lang::Locale& rDefLocale(Application::GetSettings().GetUILanguageTag().getLocale());
                    sal_Int32 nSoftBreak = _rLayout.GetTextBreak( rStr, nWidth, nPos, nBreakPos - nPos );
                    if (nSoftBreak == -1)
                    {
                        nSoftBreak = nPos;
                    }
                    DBG_ASSERT( nSoftBreak < nBreakPos, "Break?!" );
                    css::i18n::LineBreakHyphenationOptions aHyphOptions( xHyph, css::uno::Sequence <css::beans::PropertyValue>(), 1 );
                    css::i18n::LineBreakUserOptions aUserOptions;
                    css::i18n::LineBreakResults aLBR = xBI->getLineBreak( rStr, nSoftBreak, rDefLocale, nPos, aHyphOptions, aUserOptions );
                    nBreakPos = aLBR.breakIndex;
                    if ( nBreakPos <= nPos )
                        nBreakPos = nSoftBreak;
                    if ( bHyphenate )
                    {
                        // Whether hyphen or not: Put the word after the hyphen through
                        // word boundary.

                        // nMaxBreakPos the last char that fits into the line
                        // nBreakPos is the word's start

                        // We run into a problem if the doc is so narrow, that a word
                        // is broken into more than two lines ...
                        if ( xHyph.is() )
                        {
                            sal_Unicode cAlternateReplChar = 0;
                            css::i18n::Boundary aBoundary = xBI->getWordBoundary( rStr, nBreakPos, rDefLocale, css::i18n::WordType::DICTIONARY_WORD, sal_True );
                            sal_Int32 nWordStart = nPos;
                            sal_Int32 nWordEnd = aBoundary.endPos;
                            DBG_ASSERT( nWordEnd > nWordStart, "ImpBreakLine: Start >= End?" );

                            sal_Int32 nWordLen = nWordEnd - nWordStart;
                            if ( ( nWordEnd >= nSoftBreak ) && ( nWordLen > 3 ) )
                            {
                                // #104415# May happen, because getLineBreak may differ from getWordBoudary with DICTIONARY_WORD
                                // DBG_ASSERT( nWordEnd >= nMaxBreakPos, "Hyph: Break?" );
                                OUString aWord = rStr.copy( nWordStart, nWordLen );
                                sal_Int32 nMinTrail = nWordEnd-nSoftBreak+1;  //+1: Before the "broken off" char
                                css::uno::Reference< css::linguistic2::XHyphenatedWord > xHyphWord;
                                if (xHyph.is())
                                    xHyphWord = xHyph->hyphenate( aWord, rDefLocale, aWord.getLength() - nMinTrail, css::uno::Sequence< css::beans::PropertyValue >() );
                                if (xHyphWord.is())
                                {
                                    bool bAlternate = xHyphWord->isAlternativeSpelling();
                                    sal_Int32 _nWordLen = 1 + xHyphWord->getHyphenPos();

                                    if ( ( _nWordLen >= 2 ) && ( (nWordStart+_nWordLen) >= ( 2 ) ) )
                                    {
                                        if ( !bAlternate )
                                        {
                                            nBreakPos = nWordStart + _nWordLen;
                                        }
                                        else
                                        {
                                            OUString aAlt( xHyphWord->getHyphenatedWord() );

                                            // We can have two cases:
                                            // 1) "packen" turns into "pak-ken"
                                            // 2) "Schiffahrt" turns into "Schiff-fahrt"

                                            // In case 1 we need to replace a char
                                            // In case 2 we add a char

                                            // Correct recognition is made harder by words such as
                                            // "Schiffahrtsbrennesseln", as the Hyphenator splits all
                                            // positions of the word and comes up with "Schifffahrtsbrennnesseln"
                                            // Thus, we cannot infer the aWord from the AlternativWord's
                                            // index.
                                            // TODO: The whole junk will be made easier by a function in
                                            // the Hyphenator, as soon as AMA adds it.
                                            sal_Int32 nAltStart = _nWordLen - 1;
                                            sal_Int32 nTxtStart = nAltStart - (aAlt.getLength() - aWord.getLength());
                                            sal_Int32 nTxtEnd = nTxtStart;
                                            sal_Int32 nAltEnd = nAltStart;

                                            // The area between nStart and nEnd is the difference
                                            // between AlternativString and OriginalString
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

                                            DBG_ASSERT( ( nAltEnd - nAltStart ) == 1, "Alternate: Wrong assumption!" );

                                            if ( nTxtEnd > nTxtStart )
                                                cAlternateReplChar = aAlt[ nAltStart ];

                                            nBreakPos = nWordStart + nTxtStart;
                                            if ( cAlternateReplChar )
                                                nBreakPos++;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    nLineWidth = _rLayout.GetTextWidth( rStr, nPos, nBreakPos-nPos );
                }
                else
                {
                    // fallback to something really simple
                    sal_Int32 nSpacePos = rStr.getLength();
                    long nW = 0;
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
                }
            }

            if ( nLineWidth > nMaxLineWidth )
                nMaxLineWidth = nLineWidth;

            rLineInfo.AddLine( new ImplTextLineInfo( nLineWidth, nPos, nBreakPos-nPos ) );

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
        }
    }
#ifdef DBG_UTIL
    for ( sal_Int32 nL = 0; nL < rLineInfo.Count(); nL++ )
    {
        ImplTextLineInfo* pLine = rLineInfo.GetLine( nL );
        OUString aLine = rStr.copy( pLine->GetIndex(), pLine->GetLen() );
        DBG_ASSERT( aLine.indexOf( '\r' ) == -1, "ImplGetTextLines - Found CR!" );
        DBG_ASSERT( aLine.indexOf( '\n' ) == -1, "ImplGetTextLines - Found LF!" );
    }
#endif

    return nMaxLineWidth;
}

void OutputDevice::SetTextColor( const Color& rColor )
{

    Color aColor( rColor );

    if ( mnDrawMode & ( DrawModeFlags::BlackText | DrawModeFlags::WhiteText |
                        DrawModeFlags::GrayText | DrawModeFlags::GhostedText |
                        DrawModeFlags::SettingsText ) )
    {
        if ( mnDrawMode & DrawModeFlags::BlackText )
            aColor = Color( COL_BLACK );
        else if ( mnDrawMode & DrawModeFlags::WhiteText )
            aColor = Color( COL_WHITE );
        else if ( mnDrawMode & DrawModeFlags::GrayText )
        {
            const sal_uInt8 cLum = aColor.GetLuminance();
            aColor = Color( cLum, cLum, cLum );
        }
        else if ( mnDrawMode & DrawModeFlags::SettingsText )
            aColor = GetSettings().GetStyleSettings().GetFontColor();

        if ( mnDrawMode & DrawModeFlags::GhostedText )
        {
            aColor = Color( (aColor.GetRed() >> 1) | 0x80,
                            (aColor.GetGreen() >> 1) | 0x80,
                            (aColor.GetBlue() >> 1) | 0x80 );
        }
    }

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextColorAction( aColor ) );

    if ( maTextColor != aColor )
    {
        maTextColor = aColor;
        mbInitTextColor = true;
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetTextColor( COL_BLACK );
}

void OutputDevice::SetTextFillColor()
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextFillColorAction( Color(), false ) );

    if ( maFont.GetColor() != Color( COL_TRANSPARENT ) ) {
        maFont.SetFillColor( Color( COL_TRANSPARENT ) );
    }
    if ( !maFont.IsTransparent() )
        maFont.SetTransparent( true );

    if( mpAlphaVDev )
        mpAlphaVDev->SetTextFillColor();
}

void OutputDevice::SetTextFillColor( const Color& rColor )
{

    Color aColor( rColor );
    bool bTransFill = ImplIsColorTransparent( aColor );

    if ( !bTransFill )
    {
        if ( mnDrawMode & ( DrawModeFlags::BlackFill | DrawModeFlags::WhiteFill |
                            DrawModeFlags::GrayFill | DrawModeFlags::NoFill |
                            DrawModeFlags::GhostedFill | DrawModeFlags::SettingsFill ) )
        {
            if ( mnDrawMode & DrawModeFlags::BlackFill )
                aColor = Color( COL_BLACK );
            else if ( mnDrawMode & DrawModeFlags::WhiteFill )
                aColor = Color( COL_WHITE );
            else if ( mnDrawMode & DrawModeFlags::GrayFill )
            {
                const sal_uInt8 cLum = aColor.GetLuminance();
                aColor = Color( cLum, cLum, cLum );
            }
            else if( mnDrawMode & DrawModeFlags::SettingsFill )
                aColor = GetSettings().GetStyleSettings().GetWindowColor();
            else if ( mnDrawMode & DrawModeFlags::NoFill )
            {
                aColor = Color( COL_TRANSPARENT );
                bTransFill = true;
            }

            if ( !bTransFill && (mnDrawMode & DrawModeFlags::GhostedFill) )
            {
                aColor = Color( (aColor.GetRed() >> 1) | 0x80,
                                (aColor.GetGreen() >> 1) | 0x80,
                                (aColor.GetBlue() >> 1) | 0x80 );
            }
        }
    }

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextFillColorAction( aColor, true ) );

    if ( maFont.GetFillColor() != aColor )
        maFont.SetFillColor( aColor );
    if ( maFont.IsTransparent() != bTransFill )
        maFont.SetTransparent( bTransFill );

    if( mpAlphaVDev )
        mpAlphaVDev->SetTextFillColor( COL_BLACK );
}

Color OutputDevice::GetTextFillColor() const
{
    if ( maFont.IsTransparent() )
        return Color( COL_TRANSPARENT );
    else
        return maFont.GetFillColor();
}

void OutputDevice::SetTextAlign( TextAlign eAlign )
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextAlignAction( eAlign ) );

    if ( maFont.GetAlign() != eAlign )
    {
        maFont.SetAlign( eAlign );
        mbNewFont = true;
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetTextAlign( eAlign );
}

void OutputDevice::DrawText( const Point& rStartPt, const OUString& rStr,
                             sal_Int32 nIndex, sal_Int32 nLen,
                             MetricVector* pVector, OUString* pDisplayText
                             )
{
    assert(!is_double_buffered_window());

    if(nLen == 0x0FFFF)
    {
        SAL_INFO("sal.rtl.xub",
                 "GetTextOutlines Suspicious arguments nLen:" << nLen);
    }
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
    fprintf( stderr, "   OutputDevice::DrawText(\"%s\")\n",
         OUStringToOString( rStr, RTL_TEXTENCODING_UTF8 ).getStr() );
#endif

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextAction( rStartPt, rStr, nIndex, nLen ) );
    if( pVector )
    {
        vcl::Region aClip( GetClipRegion() );
        if( meOutDevType == OUTDEV_WINDOW )
            aClip.Intersect( Rectangle( Point(), GetOutputSize() ) );
        if (mpOutDevData->mpRecordLayout)
        {
            mpOutDevData->mpRecordLayout->m_aLineIndices.push_back( mpOutDevData->mpRecordLayout->m_aDisplayText.getLength() );
            aClip.Intersect( mpOutDevData->maRecordRect );
        }
        if( ! aClip.IsNull() )
        {
            MetricVector aTmp;
            GetGlyphBoundRects( rStartPt, rStr, nIndex, nLen, nIndex, aTmp );

            bool bInserted = false;
            for( MetricVector::const_iterator it = aTmp.begin(); it != aTmp.end(); ++it, nIndex++ )
            {
                bool bAppend = false;

                if( aClip.IsOver( *it ) )
                    bAppend = true;
                else if( rStr[ nIndex ] == ' ' && bInserted )
                {
                    MetricVector::const_iterator next = it;
                    ++next;
                    if( next != aTmp.end() && aClip.IsOver( *next ) )
                        bAppend = true;
                }

                if( bAppend )
                {
                    pVector->push_back( *it );
                    if( pDisplayText )
                        *pDisplayText += OUString(rStr[ nIndex ]);
                    bInserted = true;
                }
            }
        }
        else
        {
            GetGlyphBoundRects( rStartPt, rStr, nIndex, nLen, nIndex, *pVector );
            if( pDisplayText )
                *pDisplayText += rStr.copy( nIndex, nLen );
        }
    }

    if ( !IsDeviceOutputNecessary() || pVector )
        return;

    SalLayout* pSalLayout = ImplLayout(rStr, nIndex, nLen, rStartPt);
    if( pSalLayout )
    {
        ImplDrawText( *pSalLayout );
        pSalLayout->Release();
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawText( rStartPt, rStr, nIndex, nLen, pVector, pDisplayText );
}

long OutputDevice::GetTextWidth( const OUString& rStr, sal_Int32 nIndex, sal_Int32 nLen,
     vcl::TextLayoutCache const*const pLayoutCache) const
{

    long nWidth = GetTextArray( rStr, nullptr, nIndex, nLen, pLayoutCache );

    return nWidth;
}

long OutputDevice::GetTextHeight() const
{

    if( mbNewFont )
        if( !ImplNewFont() )
            return 0;
    if( mbInitFont )
        if( !ImplNewFont() )
            return 0;

    long nHeight = mpFontEntry->mnLineHeight + mnEmphasisAscent + mnEmphasisDescent;

    if ( mbMap )
        nHeight = ImplDevicePixelToLogicHeight( nHeight );

    return nHeight;
}

float OutputDevice::approximate_char_width() const
{
    return GetTextWidth("aemnnxEM") / 8.0;
}

void OutputDevice::DrawTextArray( const Point& rStartPt, const OUString& rStr,
                                  const long* pDXAry,
                                  sal_Int32 nIndex, sal_Int32 nLen, SalLayoutFlags flags )
{
    assert(!is_double_buffered_window());

    if(nLen == 0x0FFFF)
    {
        SAL_INFO("sal.rtl.xub",
                 "DrawTextArray Suspicious arguments nLen:" << nLen);
    }
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
    if( mbInitClipRegion )
        InitClipRegion();
    if( mbOutputClipped )
        return;

    SalLayout* pSalLayout = ImplLayout(rStr, nIndex, nLen, rStartPt, 0, pDXAry, flags);
    if( pSalLayout )
    {
        ImplDrawText( *pSalLayout );
        pSalLayout->Release();
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawTextArray( rStartPt, rStr, pDXAry, nIndex, nLen, flags );
}

long OutputDevice::GetTextArray( const OUString& rStr, long* pDXAry,
                                 sal_Int32 nIndex, sal_Int32 nLen,
                                 vcl::TextLayoutCache const*const pLayoutCache) const
{
    if(nLen == 0x0FFFF)
    {
        SAL_INFO("sal.rtl.xub",
                 "GetTextArray Suspicious arguments nLen:" << nLen);
    }

    if( nIndex >= rStr.getLength() )
        return 0; // TODO: this looks like a buggy caller?

    if( nLen < 0 || nIndex + nLen >= rStr.getLength() )
    {
        nLen = rStr.getLength() - nIndex;
    }
    // do layout
    SalLayout *const pSalLayout = ImplLayout(rStr, nIndex, nLen,
            Point(0,0), 0, nullptr, SalLayoutFlags::NONE, pLayoutCache);
    if( !pSalLayout )
    {
        // The caller expects this to init the elements of pDXAry.
        // Adapting all the callers to check that GetTextArray succeeded seems
        // too much work.
        // Init here to 0 only in the (rare) error case, so that any missing
        // element init in the happy case will still be found by tools,
        // and hope that is sufficient.
        if (pDXAry)
        {
            memset(pDXAry, 0, nLen * sizeof(*pDXAry));
        }
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
    pSalLayout->Release();

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
        for( int i = 0; i < nLen; ++i )
        {
            pDXAry[i] = basegfx::fround(pDXPixelArray[i]);
        }
    }
    return basegfx::fround(nWidth);

#else /* ! VCL_FLOAT_DEVICE_PIXEL */

    long nWidth = pSalLayout->FillDXArray( pDXAry );
    int nWidthFactor = pSalLayout->GetUnitsPerPixel();
    pSalLayout->Release();

    // convert virtual char widths to virtual absolute positions
    if( pDXAry )
        for( int i = 1; i < nLen; ++i )
            pDXAry[ i ] += pDXAry[ i-1 ];

    // convert from font units to logical units
    if( mbMap )
    {
        if( pDXAry )
            for( int i = 0; i < nLen; ++i )
                pDXAry[i] = ImplDevicePixelToLogicWidth( pDXAry[i] );
        nWidth = ImplDevicePixelToLogicWidth( nWidth );
    }

    if( nWidthFactor > 1 )
    {
        if( pDXAry )
            for( int i = 0; i < nLen; ++i )
                pDXAry[i] /= nWidthFactor;
        nWidth /= nWidthFactor;
    }
    return nWidth;
#endif /* VCL_FLOAT_DEVICE_PIXEL */
}

bool OutputDevice::GetCaretPositions( const OUString& rStr, long* pCaretXArray,
                                      sal_Int32 nIndex, sal_Int32 nLen,
                                      long* pDXAry, long nLayoutWidth,
                                      bool bCellBreaking ) const
{

    if( nIndex >= rStr.getLength() )
        return false;
    if( nIndex+nLen >= rStr.getLength() )
        nLen = rStr.getLength() - nIndex;

    // layout complex text
    SalLayout* pSalLayout = ImplLayout( rStr, nIndex, nLen,
                                        Point(0,0), nLayoutWidth, pDXAry );
    if( !pSalLayout )
        return false;

    int nWidthFactor = pSalLayout->GetUnitsPerPixel();
    pSalLayout->GetCaretPositions( 2*nLen, pCaretXArray );
    long nWidth = pSalLayout->GetTextWidth();
    pSalLayout->Release();

    // fixup unknown caret positions
    int i;
    for( i = 0; i < 2 * nLen; ++i )
        if( pCaretXArray[ i ] >= 0 )
            break;
    long nXPos = pCaretXArray[ i ];
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

    // if requested move caret position to cell limits
    if( bCellBreaking )
    {
        ; // FIXME
    }

    return true;
}

void OutputDevice::DrawStretchText( const Point& rStartPt, sal_uLong nWidth,
                                    const OUString& rStr,
                                    sal_Int32 nIndex, sal_Int32 nLen)
{
    assert(!is_double_buffered_window());

    if(nIndex < 0 || nIndex == 0x0FFFF || nLen == 0x0FFFF)
    {
        SAL_INFO("sal.rtl.xub",
                 "DrawStretchText Suspicious arguments nIndex:" << nIndex << " nLen:" << nLen);
    }
    if( (nLen < 0) || (nIndex + nLen >= rStr.getLength()))
    {
        nLen = rStr.getLength() - nIndex;
    }

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaStretchTextAction( rStartPt, nWidth, rStr, nIndex, nLen ) );

    if ( !IsDeviceOutputNecessary() )
        return;

    SalLayout* pSalLayout = ImplLayout(rStr, nIndex, nLen, rStartPt, nWidth);
    if( pSalLayout )
    {
        ImplDrawText( *pSalLayout );
        pSalLayout->Release();
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawStretchText( rStartPt, nWidth, rStr, nIndex, nLen );
}

ImplLayoutArgs OutputDevice::ImplPrepareLayoutArgs( OUString& rStr,
                                                    const sal_Int32 nMinIndex, const sal_Int32 nLen,
                                                    DeviceCoordinate nPixelWidth, const DeviceCoordinate* pDXArray,
                                                    SalLayoutFlags nLayoutFlags,
         vcl::TextLayoutCache const*const pLayoutCache) const
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

    if( mnTextLayoutMode & TEXT_LAYOUT_BIDI_RTL )
        nLayoutFlags |= SalLayoutFlags::BiDiRtl;
    if( mnTextLayoutMode & TEXT_LAYOUT_BIDI_STRONG )
        nLayoutFlags |= SalLayoutFlags::BiDiStrong;
    else if( !(mnTextLayoutMode & TEXT_LAYOUT_BIDI_RTL) )
    {
        // disable Bidi if no RTL hint and no RTL codes used
        const sal_Unicode* pStr = rStr.getStr() + nMinIndex;
        const sal_Unicode* pEnd = rStr.getStr() + nEndIndex;
        for( ; pStr < pEnd; ++pStr )
            if( ((*pStr >= 0x0580) && (*pStr < 0x0800))   // middle eastern scripts
            ||  ((*pStr >= 0xFB18) && (*pStr < 0xFE00))   // hebrew + arabic A presentation forms
            ||  ((*pStr >= 0xFE70) && (*pStr < 0xFEFF)) ) // arabic presentation forms B
                break;
        if( pStr >= pEnd )
            nLayoutFlags |= SalLayoutFlags::BiDiStrong;
    }

    if( mbKerning )
        nLayoutFlags |= SalLayoutFlags::KerningPairs;
    if( maFont.GetKerning() & FontKerning::Asian )
        nLayoutFlags |= SalLayoutFlags::KerningAsian;
    if( maFont.IsVertical() )
        nLayoutFlags |= SalLayoutFlags::Vertical;

    if( mnTextLayoutMode & TEXT_LAYOUT_ENABLE_LIGATURES )
        nLayoutFlags |= SalLayoutFlags::EnableLigatures;
    else if( mnTextLayoutMode & TEXT_LAYOUT_COMPLEX_DISABLED )
        nLayoutFlags |= SalLayoutFlags::ComplexDisabled;
    else
    {
        // disable CTL for non-CTL text
        const sal_Unicode* pStr = rStr.getStr() + nMinIndex;
        const sal_Unicode* pEnd = rStr.getStr() + nEndIndex;
        for( ; pStr < pEnd; ++pStr )
            if( ((*pStr >= 0x0300) && (*pStr < 0x0370))   // diacritical marks
            ||  ((*pStr >= 0x0590) && (*pStr < 0x10A0))   // many CTL scripts
            ||  ((*pStr >= 0x1100) && (*pStr < 0x1200))   // hangul jamo
            ||  ((*pStr >= 0x1700) && (*pStr < 0x1900))   // many CTL scripts
            ||  ((*pStr >= 0xFB1D) && (*pStr < 0xFE00))   // middle east presentation
            ||  ((*pStr >= 0xFE70) && (*pStr < 0xFEFF))   // arabic presentation B
            ||  ((*pStr >= 0xFE00) && (*pStr < 0xFE10))   // variation selectors in BMP
            ||  ((pStr + 1 < pEnd) && (pStr[0] == 0xDB40) && (0xDD00 <= pStr[1]) && (pStr[1] < 0xDEF0)) // variation selector supplement
            )
                break;
        if( pStr >= pEnd )
            nLayoutFlags |= SalLayoutFlags::ComplexDisabled;
    }

    if( meTextLanguage ) //TODO: (mnTextLayoutMode & TEXT_LAYOUT_SUBSTITUTE_DIGITS)
    {
        // disable character localization when no digits used
        const sal_Unicode* pBase = rStr.getStr();
        const sal_Unicode* pStr = pBase + nMinIndex;
        const sal_Unicode* pEnd = pBase + nEndIndex;
        OUStringBuffer sTmpStr(rStr);
        for( ; pStr < pEnd; ++pStr )
        {
            // TODO: are there non-digit localizations?
            if( (*pStr >= '0') && (*pStr <= '9') )
            {
                // translate characters to local preference
                sal_UCS4 cChar = GetLocalizedChar( *pStr, meTextLanguage );
                if( cChar != *pStr )
                    // TODO: are the localized digit surrogates?
                    sTmpStr[pStr - pBase] = cChar;
            }
        }
        rStr = sTmpStr.makeStringAndClear();
    }

    // right align for RTL text, DRAWPOS_REVERSED, RTL window style
    bool bRightAlign = bool(mnTextLayoutMode & TEXT_LAYOUT_BIDI_RTL);
    if( mnTextLayoutMode & TEXT_LAYOUT_TEXTORIGIN_LEFT )
        bRightAlign = false;
    else if ( mnTextLayoutMode & TEXT_LAYOUT_TEXTORIGIN_RIGHT )
        bRightAlign = true;
    // SSA: hack for western office, ie text get right aligned
    //      for debugging purposes of mirrored UI
    bool bRTLWindow = IsRTLEnabled();
    bRightAlign ^= bRTLWindow;
    if( bRightAlign )
        nLayoutFlags |= SalLayoutFlags::RightAlign;

    // set layout options
    ImplLayoutArgs aLayoutArgs(rStr, nMinIndex, nEndIndex, nLayoutFlags, maFont.GetLanguageTag(), pLayoutCache);

    int nOrientation = mpFontEntry ? mpFontEntry->mnOrientation : 0;
    aLayoutArgs.SetOrientation( nOrientation );

    aLayoutArgs.SetLayoutWidth( nPixelWidth );
    aLayoutArgs.SetDXArray( pDXArray );

    return aLayoutArgs;
}

SalLayout* OutputDevice::ImplLayout(const OUString& rOrigStr,
                                    sal_Int32 nMinIndex, sal_Int32 nLen,
                                    const Point& rLogicalPos, long nLogicalWidth,
                                    const long* pDXArray, SalLayoutFlags flags,
         vcl::TextLayoutCache const* pLayoutCache) const
{
    // we need a graphics
    if( !mpGraphics )
        if( !AcquireGraphics() )
            return nullptr;

    // initialize font if needed
    if( mbNewFont )
        if( !ImplNewFont() )
            return nullptr;
    if( mbInitFont )
        InitFont();

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
    if( mpFontEntry->mpConversion ) {
        mpFontEntry->mpConversion->RecodeString( aStr, 0, aStr.getLength() );
        pLayoutCache = nullptr; // don't use cache with modified string!
    }
    DeviceCoordinate nPixelWidth = (DeviceCoordinate)nLogicalWidth;
    std::unique_ptr<DeviceCoordinate[]> xDXPixelArray;
    DeviceCoordinate* pDXPixelArray(nullptr);
    if( nLogicalWidth && mbMap )
    {
        nPixelWidth = LogicWidthToDeviceCoordinate( nLogicalWidth );
    }

    if( pDXArray)
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
                pDXPixelArray[i] = pDXArray[i];
            }
#else /* !VCL_FLOAT_DEVICE_PIXEL */
            pDXPixelArray = const_cast<DeviceCoordinate*>(pDXArray);
#endif /* !VCL_FLOAT_DEVICE_PIXEL */
        }
    }

    ImplLayoutArgs aLayoutArgs = ImplPrepareLayoutArgs( aStr, nMinIndex, nLen,
            nPixelWidth, pDXPixelArray, flags, pLayoutCache);

    // get matching layout object for base font
    SalLayout* pSalLayout = mpGraphics->GetTextLayout( aLayoutArgs, 0 );

    // layout text
    if( pSalLayout && !pSalLayout->LayoutText( aLayoutArgs ) )
    {
        pSalLayout->Release();
        pSalLayout = nullptr;
    }

    if( !pSalLayout )
        return nullptr;

    // do glyph fallback if needed
    // #105768# avoid fallback for very small font sizes
    if (aLayoutArgs.NeedFallback() && mpFontEntry->maFontSelData.mnHeight >= 3)
        pSalLayout = ImplGlyphFallbackLayout(pSalLayout, aLayoutArgs);

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
        pSalLayout->DrawOffset().X() = 1 - nRTLOffset;
    }

    return pSalLayout;
}

std::shared_ptr<vcl::TextLayoutCache> OutputDevice::CreateTextLayoutCache(
        OUString const& rString) const
{
    if (!mpGraphics) // can happen in e.g Insert Index/Table dialog
        return nullptr;
    OUString copyBecausePrepareModifiesIt(rString);
    ImplLayoutArgs aLayoutArgs = ImplPrepareLayoutArgs(copyBecausePrepareModifiesIt,
            0, rString.getLength(), 0, nullptr);

    SalLayout *const pSalLayout = mpGraphics->GetTextLayout( aLayoutArgs, 0 );
    if (!pSalLayout)
        return nullptr;
    std::shared_ptr<vcl::TextLayoutCache> const ret(
            pSalLayout->CreateTextLayoutCache(copyBecausePrepareModifiesIt));
    pSalLayout->Release();
    return ret;
}

bool OutputDevice::GetTextIsRTL( const OUString& rString, sal_Int32 nIndex, sal_Int32 nLen ) const
{
    OUString aStr( rString );
    ImplLayoutArgs aArgs = ImplPrepareLayoutArgs( aStr, nIndex, nLen, 0, nullptr );
    bool bRTL = false;
    int nCharPos = -1;
    if (!aArgs.GetNextPos(&nCharPos, &bRTL))
        return false;
    return (nCharPos != nIndex);
}

sal_Int32 OutputDevice::GetTextBreak( const OUString& rStr, long nTextWidth,
                                       sal_Int32 nIndex, sal_Int32 nLen,
                                       long nCharExtra,
         vcl::TextLayoutCache const*const pLayoutCache) const
{
    SalLayout *const pSalLayout = ImplLayout( rStr, nIndex, nLen,
            Point(0,0), 0, nullptr, SalLayoutFlags::NONE, pLayoutCache);
    sal_Int32 nRetVal = -1;
    if( pSalLayout )
    {
        // convert logical widths into layout units
        // NOTE: be very careful to avoid rounding errors for nCharExtra case
        // problem with rounding errors especially for small nCharExtras
        // TODO: remove when layout units have subpixel granularity
        long nWidthFactor = pSalLayout->GetUnitsPerPixel();
        long nSubPixelFactor = (nWidthFactor < 64 ) ? 64 : 1;
        nTextWidth *= nWidthFactor * nSubPixelFactor;
        DeviceCoordinate nTextPixelWidth = LogicWidthToDeviceCoordinate( nTextWidth );
        DeviceCoordinate nExtraPixelWidth = 0;
        if( nCharExtra != 0 )
        {
            nCharExtra *= nWidthFactor * nSubPixelFactor;
            nExtraPixelWidth = LogicWidthToDeviceCoordinate( nCharExtra );
        }
        nRetVal = pSalLayout->GetTextBreak( nTextPixelWidth, nExtraPixelWidth, nSubPixelFactor );

        pSalLayout->Release();
    }

    return nRetVal;
}

sal_Int32 OutputDevice::GetTextBreak( const OUString& rStr, long nTextWidth,
                                       sal_Unicode nHyphenChar, sal_Int32& rHyphenPos,
                                       sal_Int32 nIndex, sal_Int32 nLen,
                                       long nCharExtra,
         vcl::TextLayoutCache const*const pLayoutCache) const
{
    rHyphenPos = -1;

    SalLayout *const pSalLayout = ImplLayout( rStr, nIndex, nLen,
            Point(0,0), 0, nullptr, SalLayoutFlags::NONE, pLayoutCache);
    sal_Int32 nRetVal = -1;
    if( pSalLayout )
    {
        // convert logical widths into layout units
        // NOTE: be very careful to avoid rounding errors for nCharExtra case
        // problem with rounding errors especially for small nCharExtras
        // TODO: remove when layout units have subpixel granularity
        long nWidthFactor = pSalLayout->GetUnitsPerPixel();
        long nSubPixelFactor = (nWidthFactor < 64 ) ? 64 : 1;

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
        sal_Int32 nTempLen = 1;
        SalLayout* pHyphenLayout = ImplLayout( aHyphenStr, 0, nTempLen );
        if( pHyphenLayout )
        {
            // calculate subpixel width of hyphenation character
            long nHyphenPixelWidth = pHyphenLayout->GetTextWidth() * nSubPixelFactor;
            pHyphenLayout->Release();

            // calculate hyphenated break position
            nTextPixelWidth -= nHyphenPixelWidth;
            if( nExtraPixelWidth > 0 )
                nTextPixelWidth -= nExtraPixelWidth;

            rHyphenPos = pSalLayout->GetTextBreak(nTextPixelWidth, nExtraPixelWidth, nSubPixelFactor);

            if( rHyphenPos > nRetVal )
                rHyphenPos = nRetVal;
        }

        pSalLayout->Release();
    }

    return nRetVal;
}

void OutputDevice::ImplDrawText( OutputDevice& rTargetDevice, const Rectangle& rRect,
                                 const OUString& rOrigStr, DrawTextFlags nStyle,
                                 MetricVector* pVector, OUString* pDisplayText,
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

    long        nWidth          = rRect.GetWidth();
    long        nHeight         = rRect.GetHeight();

    if ( ((nWidth <= 0) || (nHeight <= 0)) && (nStyle & DrawTextFlags::Clip) )
        return;

    Point       aPos            = rRect.TopLeft();

    long        nTextHeight     = rTargetDevice.GetTextHeight();
    TextAlign   eAlign          = rTargetDevice.GetTextAlign();
    sal_Int32   nMnemonicPos    = -1;

    OUString aStr = rOrigStr;
    if ( nStyle & DrawTextFlags::Mnemonic )
        aStr = GetNonMnemonicString( aStr, nMnemonicPos );

    const bool bDrawMnemonics = !(rTargetDevice.GetSettings().GetStyleSettings().GetOptions() & StyleSettingsOptions::NoMnemonics) && !pVector;

    // We treat multiline text differently
    if ( nStyle & DrawTextFlags::MultiLine )
    {

        OUString                aLastLine;
        ImplMultiTextLineInfo   aMultiLineInfo;
        ImplTextLineInfo*       pLineInfo;
        sal_Int32               i;
        sal_Int32               nLines;
        sal_Int32               nFormatLines;

        if ( nTextHeight )
        {
            long nMaxTextWidth = ImplGetTextLines( aMultiLineInfo, nWidth, aStr, nStyle, _rLayout );
            nLines = (sal_Int32)(nHeight/nTextHeight);
            nFormatLines = aMultiLineInfo.Count();
            if (nLines <= 0)
                nLines = 1;
            if ( nFormatLines > nLines )
            {
                if ( nStyle & DrawTextFlags::EndEllipsis )
                {
                    // Create last line and shorten it
                    nFormatLines = nLines-1;

                    pLineInfo = aMultiLineInfo.GetLine( nFormatLines );
                    aLastLine = convertLineEnd(aStr.copy(pLineInfo->GetIndex()), LINEEND_LF);
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
                rTargetDevice.Push( PushFlags::CLIPREGION );
                rTargetDevice.IntersectClipRegion( rRect );
            }

            // Vertical alignment
            if ( nStyle & DrawTextFlags::Bottom )
                aPos.Y() += nHeight-(nFormatLines*nTextHeight);
            else if ( nStyle & DrawTextFlags::VCenter )
                aPos.Y() += (nHeight-(nFormatLines*nTextHeight))/2;

            // Font alignment
            if ( eAlign == ALIGN_BOTTOM )
                aPos.Y() += nTextHeight;
            else if ( eAlign == ALIGN_BASELINE )
                aPos.Y() += rTargetDevice.GetFontMetric().GetAscent();

            // Output all lines except for the last one
            for ( i = 0; i < nFormatLines; i++ )
            {
                pLineInfo = aMultiLineInfo.GetLine( i );
                if ( nStyle & DrawTextFlags::Right )
                    aPos.X() += nWidth-pLineInfo->GetWidth();
                else if ( nStyle & DrawTextFlags::Center )
                    aPos.X() += (nWidth-pLineInfo->GetWidth())/2;
                sal_Int32 nIndex   = pLineInfo->GetIndex();
                sal_Int32 nLineLen = pLineInfo->GetLen();
                _rLayout.DrawText( aPos, aStr, nIndex, nLineLen, pVector, pDisplayText );
                if ( bDrawMnemonics )
                {
                    if ( (nMnemonicPos >= nIndex) && (nMnemonicPos < nIndex+nLineLen) )
                    {
                        long        nMnemonicX;
                        long        nMnemonicY;
                        DeviceCoordinate nMnemonicWidth;

                        std::unique_ptr<long[]> const pCaretXArray(new long[2 * nLineLen]);
                        /*sal_Bool bRet =*/ _rLayout.GetCaretPositions( aStr, pCaretXArray.get(),
                                                nIndex, nLineLen );
                        long lc_x1 = pCaretXArray[2*(nMnemonicPos - nIndex)];
                        long lc_x2 = pCaretXArray[2*(nMnemonicPos - nIndex)+1];
                        nMnemonicWidth = rTargetDevice.LogicWidthToDeviceCoordinate( std::abs(lc_x1 - lc_x2) );

                        Point       aTempPos = rTargetDevice.LogicToPixel( aPos );
                        nMnemonicX = rTargetDevice.GetOutOffXPixel() + aTempPos.X() + rTargetDevice.ImplLogicWidthToDevicePixel( std::min( lc_x1, lc_x2 ) );
                        nMnemonicY = rTargetDevice.GetOutOffYPixel() + aTempPos.Y() + rTargetDevice.ImplLogicWidthToDevicePixel( rTargetDevice.GetFontMetric().GetAscent() );
                        rTargetDevice.ImplDrawMnemonicLine( nMnemonicX, nMnemonicY, nMnemonicWidth );
                    }
                }
                aPos.Y() += nTextHeight;
                aPos.X() = rRect.Left();
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
        long nTextWidth = _rLayout.GetTextWidth( aStr, 0, -1 );

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
            aPos.X() += nWidth-nTextWidth;
        else if ( nStyle & DrawTextFlags::Center )
            aPos.X() += (nWidth-nTextWidth)/2;

        // vertical font alignment
        if ( eAlign == ALIGN_BOTTOM )
            aPos.Y() += nTextHeight;
        else if ( eAlign == ALIGN_BASELINE )
            aPos.Y() += rTargetDevice.GetFontMetric().GetAscent();

        if ( nStyle & DrawTextFlags::Bottom )
            aPos.Y() += nHeight-nTextHeight;
        else if ( nStyle & DrawTextFlags::VCenter )
            aPos.Y() += (nHeight-nTextHeight)/2;

        long nMnemonicX = 0;
        long nMnemonicY = 0;
        DeviceCoordinate nMnemonicWidth = 0;
        if ( nMnemonicPos != -1 )
        {
            std::unique_ptr<long[]> const pCaretXArray(new long[2 * aStr.getLength()]);
            /*sal_Bool bRet =*/ _rLayout.GetCaretPositions( aStr, pCaretXArray.get(), 0, aStr.getLength() );
            long lc_x1 = pCaretXArray[2*(nMnemonicPos)];
            long lc_x2 = pCaretXArray[2*(nMnemonicPos)+1];
            nMnemonicWidth = rTargetDevice.LogicWidthToDeviceCoordinate( std::abs(lc_x1 - lc_x2) );

            Point aTempPos = rTargetDevice.LogicToPixel( aPos );
            nMnemonicX = rTargetDevice.GetOutOffXPixel() + aTempPos.X() + rTargetDevice.ImplLogicWidthToDevicePixel( std::min(lc_x1, lc_x2) );
            nMnemonicY = rTargetDevice.GetOutOffYPixel() + aTempPos.Y() + rTargetDevice.ImplLogicWidthToDevicePixel( rTargetDevice.GetFontMetric().GetAscent() );
        }

        if ( nStyle & DrawTextFlags::Clip )
        {
            rTargetDevice.Push( PushFlags::CLIPREGION );
            rTargetDevice.IntersectClipRegion( rRect );
            _rLayout.DrawText( aPos, aStr, 0, aStr.getLength(), pVector, pDisplayText );
            if ( bDrawMnemonics )
            {
                if ( nMnemonicPos != -1 )
                    rTargetDevice.ImplDrawMnemonicLine( nMnemonicX, nMnemonicY, nMnemonicWidth );
            }
            rTargetDevice.Pop();
        }
        else
        {
            _rLayout.DrawText( aPos, aStr, 0, aStr.getLength(), pVector, pDisplayText );
            if ( bDrawMnemonics )
            {
                if ( nMnemonicPos != -1 )
                    rTargetDevice.ImplDrawMnemonicLine( nMnemonicX, nMnemonicY, nMnemonicWidth );
            }
        }
    }

    if ( nStyle & DrawTextFlags::Disable && !pVector )
    {
        rTargetDevice.SetTextColor( aOldTextColor );
        if ( bRestoreFillColor )
            rTargetDevice.SetTextFillColor( aOldTextFillColor );
    }
}

void OutputDevice::AddTextRectActions( const Rectangle& rRect,
                                       const OUString&  rOrigStr,
                                       DrawTextFlags    nStyle,
                                       GDIMetaFile&     rMtf )
{

    if ( rOrigStr.isEmpty() || rRect.IsEmpty() )
        return;

    // we need a graphics
    if( !mpGraphics && !AcquireGraphics() )
        return;
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

void OutputDevice::DrawText( const Rectangle& rRect, const OUString& rOrigStr, DrawTextFlags nStyle,
                             MetricVector* pVector, OUString* pDisplayText,
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

Rectangle OutputDevice::GetTextRect( const Rectangle& rRect,
                                     const OUString& rStr, DrawTextFlags nStyle,
                                     TextRectInfo* pInfo,
                                     const vcl::ITextLayout* _pTextLayout ) const
{

    Rectangle           aRect = rRect;
    sal_Int32           nLines;
    long                nWidth = rRect.GetWidth();
    long                nMaxWidth;
    long                nTextHeight = GetTextHeight();

    OUString aStr = rStr;
    if ( nStyle & DrawTextFlags::Mnemonic )
        aStr = GetNonMnemonicString( aStr );

    if ( nStyle & DrawTextFlags::MultiLine )
    {
        ImplMultiTextLineInfo   aMultiLineInfo;
        ImplTextLineInfo*       pLineInfo;
        sal_Int32               nFormatLines;
        sal_Int32               i;

        nMaxWidth = 0;
        vcl::DefaultTextLayout aDefaultLayout( *const_cast< OutputDevice* >( this ) );
        ImplGetTextLines( aMultiLineInfo, nWidth, aStr, nStyle, _pTextLayout ? *_pTextLayout : aDefaultLayout );
        nFormatLines = aMultiLineInfo.Count();
        if ( !nTextHeight )
            nTextHeight = 1;
        nLines = (sal_uInt16)(aRect.GetHeight()/nTextHeight);
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
                pLineInfo = aMultiLineInfo.GetLine( i );
                if ( bMaxWidth && (pLineInfo->GetWidth() > nMaxWidth) )
                    nMaxWidth = pLineInfo->GetWidth();
                if ( pLineInfo->GetWidth() > pInfo->mnMaxWidth )
                    pInfo->mnMaxWidth = pLineInfo->GetWidth();
            }
        }
        else if ( !nMaxWidth )
        {
            for ( i = 0; i < nLines; i++ )
            {
                pLineInfo = aMultiLineInfo.GetLine( i );
                if ( pLineInfo->GetWidth() > nMaxWidth )
                    nMaxWidth = pLineInfo->GetWidth();
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
        aRect.Left() = aRect.Right()-nMaxWidth+1;
    else if ( nStyle & DrawTextFlags::Center )
    {
        aRect.Left() += (nWidth-nMaxWidth)/2;
        aRect.Right() = aRect.Left()+nMaxWidth-1;
    }
    else
        aRect.Right() = aRect.Left()+nMaxWidth-1;

    if ( nStyle & DrawTextFlags::Bottom )
        aRect.Top() = aRect.Bottom()-(nTextHeight*nLines)+1;
    else if ( nStyle & DrawTextFlags::VCenter )
    {
        aRect.Top()   += (aRect.GetHeight()-(nTextHeight*nLines))/2;
        aRect.Bottom() = aRect.Top()+(nTextHeight*nLines)-1;
    }
    else
        aRect.Bottom() = aRect.Top()+(nTextHeight*nLines)-1;

    // #99188# get rid of rounding problems when using this rect later
    if (nStyle & DrawTextFlags::Right)
        aRect.Left()--;
    else
        aRect.Right()++;
    return aRect;
}

static bool ImplIsCharIn( sal_Unicode c, const sal_Char* pStr )
{
    while ( *pStr )
    {
        if ( *pStr == c )
            return true;
        pStr++;
    }

    return false;
}

OUString OutputDevice::GetEllipsisString( const OUString& rOrigStr, long nMaxWidth,
                                        DrawTextFlags nStyle ) const
{
    vcl::DefaultTextLayout aTextLayout( *const_cast< OutputDevice* >( this ) );
    return ImplGetEllipsisString( *this, rOrigStr, nMaxWidth, nStyle, aTextLayout );
}

OUString OutputDevice::ImplGetEllipsisString( const OutputDevice& rTargetDevice, const OUString& rOrigStr, long nMaxWidth,
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
                aTmpStr = OUStringBuffer(aStr);
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
                    aStr = aStr.replaceAt( nIndex, 1, "");
                }
            }

            if ( aStr.isEmpty() && (nStyle & DrawTextFlags::Clip) )
                aStr += OUString(rOrigStr[ 0 ]);
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
            static sal_Char const   pSepChars[] = ".";
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
            OUString aTempLastStr1( "..." );
            aTempLastStr1 += aLastStr;
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
                    OUString aFirstStr = aStr.copy( 0, nFirstContent );
                    aFirstStr += "...";
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
                                 DrawTextFlags nStyle, MetricVector* pVector, OUString* pDisplayText )
{
    assert(!is_double_buffered_window());

    if(nLen == 0x0FFFF)
    {
        SAL_INFO("sal.rtl.xub",
                 "DrawCtrlText Suspicious arguments nLen:" << nLen);
    }
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

    long        nMnemonicX = 0;
    long        nMnemonicY = 0;
    long        nMnemonicWidth = 0;
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
                DBG_ASSERT( nMnemonicPos < (nIndex+nLen), "Mnemonic underline marker after last character" );
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

            std::unique_ptr<long[]> const pCaretXArray(new long[2 * nLen]);
            /*sal_Bool bRet =*/ GetCaretPositions( aStr, pCaretXArray.get(), nIndex, nLen );
            long lc_x1 = pCaretXArray[ 2*(nMnemonicPos - nIndex) ];
            long lc_x2 = pCaretXArray[ 2*(nMnemonicPos - nIndex)+1 ];
            nMnemonicWidth = ::abs((int)(lc_x1 - lc_x2));

            Point aTempPos( std::min(lc_x1,lc_x2), GetFontMetric().GetAscent() );
            if( bInvalidPos )  // #106952#, place behind the (last) character
                aTempPos = Point( std::max(lc_x1,lc_x2), GetFontMetric().GetAscent() );

            aTempPos += rPos;
            aTempPos = LogicToPixel( aTempPos );
            nMnemonicX = mnOutOffX + aTempPos.X();
            nMnemonicY = mnOutOffY + aTempPos.Y();
        }
    }

    bool accel = ImplGetSVData()->maNWFData.mbEnableAccel;
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
        if ( !(GetSettings().GetStyleSettings().GetOptions() & StyleSettingsOptions::NoMnemonics) && !pVector
            && accel && (!autoacc || !(nStyle & DrawTextFlags::HideMnemonic)) )
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
        DrawText( rPos, aStr, nIndex, nLen, pVector, pDisplayText );
        if ( !(GetSettings().GetStyleSettings().GetOptions() & StyleSettingsOptions::NoMnemonics) && !pVector
            && accel && (!autoacc || !(nStyle & DrawTextFlags::HideMnemonic)) )
        {
            if ( nMnemonicPos != -1 )
                ImplDrawMnemonicLine( nMnemonicX, nMnemonicY, nMnemonicWidth );
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawCtrlText( rPos, rStr, nIndex, nLen, nStyle, pVector, pDisplayText );
}

long OutputDevice::GetCtrlTextWidth( const OUString& rStr,
                                     sal_Int32 nIndex, sal_Int32 nLen,
                                     DrawTextFlags nStyle ) const
{
    if(nLen == 0x0FFFF)
    {
        SAL_INFO("sal.rtl.xub",
                 "GetCtrlTextWidth Suspicious arguments nLen:" << nLen);
    }
    /* defensive code */
    if( (nLen < 0) || (nIndex + nLen >= rStr.getLength()))
    {
        nLen = rStr.getLength() - nIndex;
    }

    if ( nStyle & DrawTextFlags::Mnemonic )
    {
        sal_Int32  nMnemonicPos;
        OUString   aStr = GetNonMnemonicString( rStr, nMnemonicPos );
        if ( nMnemonicPos != -1 )
        {
            if ( nMnemonicPos < nIndex )
                nIndex--;
            else if ( (nMnemonicPos >= nIndex) && ((sal_uLong)nMnemonicPos < (sal_uLong)(nIndex+nLen)) )
                nLen--;
        }
        return GetTextWidth( aStr, nIndex, nLen );
    }
    else
        return GetTextWidth( rStr, nIndex, nLen );
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
                aStr = aStr.replaceAt( i, 1, "" );
                nLen--;
            }
            else
            {
                aStr = aStr.replaceAt( i, 1, "" );
                nLen--;
                i++;
            }
        }
        else
            i++;
    }

    return aStr;
}

/** OutputDevice::GetSysTextLayoutData
 *
 * @param rStartPt Start point of the text
 * @param rStr Text string that will be transformed into layout of glyphs
 * @param nIndex Position in the string from where layout will be done
 * @param nLen Length of the string
 * @param pDXAry Custom layout adjustment data
 *
 * Export finalized glyph layout data as platform independent SystemTextLayoutData
 * (see vcl/inc/vcl/sysdata.hxx)
 *
 * Only parameters rStartPt and rStr are mandatory, the rest is optional
 * (default values will be used)
 *
 * @return SystemTextLayoutData
 **/
SystemTextLayoutData OutputDevice::GetSysTextLayoutData(const Point& rStartPt, const OUString& rStr, sal_Int32 nIndex, sal_Int32 nLen,
                                                        const long* pDXAry) const
{
    if(nLen == 0x0FFFF)
    {
        SAL_INFO("sal.rtl.xub",
                 "GetSysTextLayoutData Suspicious arguments nLen:" << nLen);
    }
    if( (nLen < 0) || (nIndex + nLen >= rStr.getLength()))
    {
        nLen = rStr.getLength() - nIndex;
    }

    SystemTextLayoutData aSysLayoutData;
    aSysLayoutData.nSize = sizeof(aSysLayoutData);
    aSysLayoutData.rGlyphData.reserve( 256 );
    aSysLayoutData.orientation = 0;

    if ( mpMetaFile )
    {
        if (pDXAry)
            mpMetaFile->AddAction( new MetaTextArrayAction( rStartPt, rStr, pDXAry, nIndex, nLen ) );
        else
            mpMetaFile->AddAction( new MetaTextAction( rStartPt, rStr, nIndex, nLen ) );
    }

    if ( !IsDeviceOutputNecessary() ) return aSysLayoutData;

    SalLayout* pLayout = ImplLayout(rStr, nIndex, nLen, rStartPt, 0, pDXAry);

    if ( !pLayout ) return aSysLayoutData;

    // setup glyphs
    Point aPos;
    sal_GlyphId aGlyphId;
    for( int nStart = 0; pLayout->GetNextGlyphs( 1, &aGlyphId, aPos, nStart ); )
    {
        // NOTE: Windows backend is producing unicode chars (ucs4), so on windows,
        //       ETO_GLYPH_INDEX is unusable, unless extra glyph conversion is made.

        SystemGlyphData aGlyph;
        aGlyph.index = static_cast<unsigned long> (aGlyphId & GF_IDXMASK);
        aGlyph.x = aPos.X();
        aGlyph.y = aPos.Y();
        int nLevel = (aGlyphId & GF_FONTMASK) >> GF_FONTSHIFT;
        aGlyph.fallbacklevel = nLevel < MAX_FALLBACK ? nLevel : 0;
        aSysLayoutData.rGlyphData.push_back(aGlyph);
    }

    // Get font data
    aSysLayoutData.orientation = pLayout->GetOrientation();

    pLayout->Release();

    return aSysLayoutData;
}

bool OutputDevice::GetTextBoundRect( Rectangle& rRect,
                                         const OUString& rStr, sal_Int32 nBase,
                                         sal_Int32 nIndex, sal_Int32 nLen,
                                         sal_uLong nLayoutWidth, const long* pDXAry ) const
{
    if(nLen == 0x0FFFF)
    {
        SAL_INFO("sal.rtl.xub",
                 "GetTextBoundRect Suspicious arguments nLen:" << nLen);
    }

    bool bRet = false;
    rRect.SetEmpty();

    SalLayout* pSalLayout = nullptr;
    const Point aPoint;
    // calculate offset when nBase!=nIndex
    long nXOffset = 0;
    if( nBase != nIndex )
    {
        sal_Int32 nStart = std::min( nBase, nIndex );
        sal_Int32 nOfsLen = std::max( nBase, nIndex ) - nStart;
        pSalLayout = ImplLayout( rStr, nStart, nOfsLen, aPoint, nLayoutWidth, pDXAry );
        if( pSalLayout )
        {
            nXOffset = pSalLayout->GetTextWidth();
            nXOffset /= pSalLayout->GetUnitsPerPixel();
            pSalLayout->Release();
            // TODO: fix offset calculation for Bidi case
            if( nBase < nIndex)
                nXOffset = -nXOffset;
        }
    }

    pSalLayout = ImplLayout( rStr, nIndex, nLen, aPoint, nLayoutWidth, pDXAry );
    Rectangle aPixelRect;
    if( pSalLayout )
    {
        bRet = pSalLayout->GetBoundRect( *mpGraphics, aPixelRect );

        if( bRet )
        {
            int nWidthFactor = pSalLayout->GetUnitsPerPixel();

            if( nWidthFactor > 1 )
            {
                double fFactor = 1.0 / nWidthFactor;
                aPixelRect.Left()
                    = static_cast< long >(aPixelRect.Left() * fFactor);
                aPixelRect.Right()
                    = static_cast< long >(aPixelRect.Right() * fFactor);
                aPixelRect.Top()
                    = static_cast< long >(aPixelRect.Top() * fFactor);
                aPixelRect.Bottom()
                    = static_cast< long >(aPixelRect.Bottom() * fFactor);
            }

            Point aRotatedOfs( mnTextOffX, mnTextOffY );
            aRotatedOfs -= pSalLayout->GetDrawPosition( Point( nXOffset, 0 ) );
            aPixelRect += aRotatedOfs;
            rRect = PixelToLogic( aPixelRect );
            if( mbMap )
                rRect += Point( maMapRes.mnMapOfsX, maMapRes.mnMapOfsY );
        }

        pSalLayout->Release();
    }

    if( bRet || (OUTDEV_PRINTER == meOutDevType) || !mpFontEntry )
        return bRet;

    // fall back to bitmap method to get the bounding rectangle,
    // so we need a monochrome virtual device with matching font
    ScopedVclPtrInstance< VirtualDevice > aVDev(DeviceFormat::BITMASK);
    vcl::Font aFont( GetFont() );
    aFont.SetShadow( false );
    aFont.SetOutline( false );
    aFont.SetRelief( RELIEF_NONE );
    aFont.SetOrientation( 0 );
    aFont.SetSize( Size( mpFontEntry->maFontSelData.mnWidth, mpFontEntry->maFontSelData.mnHeight ) );
    aVDev->SetFont( aFont );
    aVDev->SetTextAlign( ALIGN_TOP );

    // layout the text on the virtual device
    pSalLayout = aVDev->ImplLayout( rStr, nIndex, nLen, aPoint, nLayoutWidth, pDXAry );
    if( !pSalLayout )
        return false;

    // make the bitmap big enough
    // TODO: use factors when it would get too big
    long nWidth = pSalLayout->GetTextWidth();
    long nHeight = mpFontEntry->mnLineHeight + mnEmphasisAscent + mnEmphasisDescent;
    Point aOffset( nWidth/2, 8 );
    Size aOutSize( nWidth + 2*aOffset.X(), nHeight + 2*aOffset.Y() );
    if( !nWidth || !aVDev->SetOutputSizePixel( aOutSize ) )
    {
        pSalLayout->Release();
        return false;
    }

    // draw text in black
    pSalLayout->DrawBase() = aOffset;
    aVDev->SetTextColor( Color( COL_BLACK ) );
    aVDev->SetTextFillColor();
    aVDev->ImplInitTextColor();
    aVDev->ImplDrawText( *pSalLayout );
    pSalLayout->Release();

    // find extents using the bitmap
    Bitmap aBmp = aVDev->GetBitmap( Point(), aOutSize );
    BitmapReadAccess* pAcc = aBmp.AcquireReadAccess();
    if( !pAcc )
        return false;
    const BitmapColor aBlack( pAcc->GetBestMatchingColor( Color( COL_BLACK ) ) );
    const long nW = pAcc->Width();
    const long nH = pAcc->Height();
    long nLeft = 0;
    long nRight = 0;

    // find top left point
    long nTop = 0;
    for(; nTop < nH; ++nTop )
    {
        for( nLeft = 0; nLeft < nW; ++nLeft )
            if( pAcc->GetPixel( nTop, nLeft ) == aBlack )
                break;
        if( nLeft < nW )
            break;
    }

    // find bottom right point
    long nBottom = nH;
    while( --nBottom >= nTop )
    {
        for( nRight = nW; --nRight >= 0; )
            if( pAcc->GetPixel( nBottom, nRight ) == aBlack )
                break;
        if( nRight >= 0 )
            break;
    }
    if( nRight < nLeft )
    {
        long nX = nRight;
        nRight = nLeft;
        nLeft  = nX;
    }

    for( long nY = nTop; nY <= nBottom; ++nY )
    {
        // find leftmost point
        long nX;
        for( nX = 0; nX < nLeft; ++nX )
            if( pAcc->GetPixel( nY, nX ) == aBlack )
                break;
        nLeft = nX;

        // find rightmost point
        for( nX = nW; --nX > nRight; )
            if( pAcc->GetPixel( nY, nX ) == aBlack )
                break;
        nRight = nX;
    }

    Bitmap::ReleaseAccess( pAcc );

    if( nTop <= nBottom )
    {
        Size aSize( nRight - nLeft + 1, nBottom - nTop + 1 );
        Point aTopLeft( nLeft, nTop );
        aTopLeft -= aOffset;
        // adjust to text alignment
        aTopLeft.Y()+= mnTextOffY - (mpFontEntry->maMetric.mnAscent + mnEmphasisAscent);
        // convert to logical coordinates
        aSize = PixelToLogic( aSize );
        aTopLeft.X() = ImplDevicePixelToLogicWidth( aTopLeft.X() );
        aTopLeft.Y() = ImplDevicePixelToLogicHeight( aTopLeft.Y() );
        rRect = Rectangle( aTopLeft, aSize );
        return true;
    }

    return false;
}

bool OutputDevice::GetTextOutlines( ::basegfx::B2DPolyPolygonVector& rVector,
                                        const OUString& rStr, sal_Int32 nBase,
                                        sal_Int32 nIndex, sal_Int32 nLen,
                                        bool bOptimize, sal_uLong nLayoutWidth, const long* pDXArray ) const
{
    if(nLen == 0x0FFFF)
    {
        SAL_INFO("sal.rtl.xub",
                 "GetTextOutlines Suspicious arguments nLen:" << nLen);
    }
    // the fonts need to be initialized
    if( mbNewFont )
        ImplNewFont();
    if( mbInitFont )
        InitFont();
    if( !mpFontEntry )
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

    SalLayout* pSalLayout = nullptr;

    // calculate offset when nBase!=nIndex
    long nXOffset = 0;
    if( nBase != nIndex )
    {
        sal_Int32 nStart = std::min( nBase, nIndex );
        sal_Int32 nOfsLen = std::max( nBase, nIndex ) - nStart;
        pSalLayout = ImplLayout( rStr, nStart, nOfsLen, Point(0,0), nLayoutWidth, pDXArray );
        if( pSalLayout )
        {
            nXOffset = pSalLayout->GetTextWidth();
            pSalLayout->Release();
            // TODO: fix offset calculation for Bidi case
            if( nBase > nIndex)
                nXOffset = -nXOffset;
        }
    }

    pSalLayout = ImplLayout( rStr, nIndex, nLen, Point(0,0), nLayoutWidth, pDXArray );
    if( pSalLayout )
    {
        bRet = pSalLayout->GetOutline( *mpGraphics, rVector );
        if( bRet )
        {
            // transform polygon to pixel units
            ::basegfx::B2DHomMatrix aMatrix;

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
                ::basegfx::B2DPolyPolygonVector::iterator aIt = rVector.begin();
                for(; aIt != rVector.end(); ++aIt )
                    (*aIt).transform( aMatrix );
            }
        }

        pSalLayout->Release();
    }

    if( bOldMap )
    {
        // restore original font size and map mode
        const_cast<OutputDevice&>(*this).mbMap = bOldMap;
        const_cast<OutputDevice&>(*this).mbNewFont = true;
    }

    if( bRet || (OUTDEV_PRINTER == meOutDevType) || !mpFontEntry )
        return bRet;

    // reset work done (tdf#81876)
    rVector.clear();

    // fall back to bitmap conversion
    // Here, we can safely assume that the mapping between characters and glyphs
    // is one-to-one. This is most probably valid for the old bitmap fonts.
    // fall back to bitmap method to get the bounding rectangle,
    // so we need a monochrome virtual device with matching font
    pSalLayout = ImplLayout( rStr, nIndex, nLen, Point(0,0), nLayoutWidth, pDXArray );
    if (pSalLayout == nullptr)
        return false;
    long nOrgWidth = pSalLayout->GetTextWidth();
    long nOrgHeight = mpFontEntry->mnLineHeight + mnEmphasisAscent
        + mnEmphasisDescent;
    pSalLayout->Release();

    ScopedVclPtrInstance< VirtualDevice > aVDev(DeviceFormat::BITMASK);

    vcl::Font aFont(GetFont());
    aFont.SetShadow(false);
    aFont.SetOutline(false);
    aFont.SetRelief(RELIEF_NONE);
    aFont.SetOrientation(0);
    if( bOptimize )
    {
        aFont.SetSize( Size( 0, GLYPH_FONT_HEIGHT ) );
        aVDev->SetMapMode( MAP_PIXEL );
    }
    aVDev->SetFont( aFont );
    aVDev->SetTextAlign( ALIGN_TOP );
    aVDev->SetTextColor( Color(COL_BLACK) );
    aVDev->SetTextFillColor();

    pSalLayout = aVDev->ImplLayout( rStr, nIndex, nLen, Point(0,0), nLayoutWidth, pDXArray );
    if (pSalLayout == nullptr)
        return false;
    long nWidth = pSalLayout->GetTextWidth();
    long nHeight = aVDev->mpFontEntry->mnLineHeight + aVDev->mnEmphasisAscent +
                   aVDev->mnEmphasisDescent;
    pSalLayout->Release();

    if( !nWidth || !nHeight )
        return true;
    double fScaleX = static_cast< double >(nOrgWidth) / nWidth;
    double fScaleY = static_cast< double >(nOrgHeight) / nHeight;

    // calculate offset when nBase!=nIndex
    // TODO: fix offset calculation for Bidi case
    nXOffset = 0;
    if( nBase != nIndex )
    {
        sal_Int32 nStart  = ((nBase < nIndex) ? nBase : nIndex);
        sal_Int32 nLength = ((nBase > nIndex) ? nBase : nIndex) - nStart;
        pSalLayout = aVDev->ImplLayout( rStr, nStart, nLength, Point(0,0), nLayoutWidth, pDXArray );
        if( pSalLayout )
        {
            nXOffset = pSalLayout->GetTextWidth();
            pSalLayout->Release();
            if( nBase > nIndex)
                nXOffset = -nXOffset;
        }
    }

    bRet = true;
    bool bRTL = false;
    OUString aStr( rStr ); // prepare for e.g. localized digits
    sal_Int32 nIndex2 = nIndex; // only needed until nIndex is sal_Int32
    sal_Int32 nLen2 = nLen; // only needed until nLen is sal_Int32
    ImplLayoutArgs aLayoutArgs = ImplPrepareLayoutArgs( aStr, nIndex2, nLen2, 0, nullptr );
    for( int nCharPos = -1; aLayoutArgs.GetNextPos( &nCharPos, &bRTL);)
    {
        bool bSuccess = false;

        // draw character into virtual device
        pSalLayout = aVDev->ImplLayout( rStr, nCharPos, 1, Point(0,0), nLayoutWidth, pDXArray );
        if (pSalLayout == nullptr)
            return false;
        long nCharWidth = pSalLayout->GetTextWidth();

        Point aOffset(nCharWidth / 2, 8);
        Size aSize(nCharWidth + 2 * aOffset.X(), nHeight + 2 * aOffset.Y());
        bSuccess = (bool)aVDev->SetOutputSizePixel(aSize);
        if( bSuccess )
        {
            // draw glyph into virtual device
            aVDev->Erase();
            pSalLayout->DrawBase() += aOffset;
            pSalLayout->DrawBase() += Point( aVDev->mnTextOffX, aVDev->mnTextOffY );
            pSalLayout->DrawText( *aVDev->mpGraphics );
            pSalLayout->Release();

            // convert character image into outline
            Bitmap aBmp( aVDev->GetBitmap(Point(0, 0), aSize));

            tools::PolyPolygon aPolyPoly;
            bool bVectorized = aBmp.Vectorize(aPolyPoly, BmpVectorizeFlags::Outer | BmpVectorizeFlags::ReduceEdges);
            if( !bVectorized )
                bSuccess = false;
            else
            {
                // convert units to logical width
                for (sal_uInt16 j = 0; j < aPolyPoly.Count(); ++j)
                {
                    tools::Polygon& rPoly = aPolyPoly[j];
                    for (sal_uInt16 k = 0; k < rPoly.GetSize(); ++k)
                    {
                        Point& rPt = rPoly[k];
                        rPt -= aOffset;
                        int nPixelX = rPt.X() - static_cast<OutputDevice*>(aVDev.get())->mnTextOffX + nXOffset;
                        int nPixelY = rPt.Y() - static_cast<OutputDevice*>(aVDev.get())->mnTextOffY;
                        rPt.X() = ImplDevicePixelToLogicWidth( nPixelX );
                        rPt.Y() = ImplDevicePixelToLogicHeight( nPixelY );
                    }
                }

                // ignore "empty" glyphs:
                if( aPolyPoly.Count() > 0 )
                {
                    // convert  to B2DPolyPolygon
                    // TODO: get rid of intermediate tool's PolyPolygon
                    ::basegfx::B2DPolyPolygon aB2DPolyPoly = aPolyPoly.getB2DPolyPolygon();
                    ::basegfx::B2DHomMatrix aMatrix;
                    aMatrix.scale( fScaleX, fScaleY );
                    int nAngle = GetFont().GetOrientation();
                    if( nAngle )
                        aMatrix.rotate( nAngle * F_PI1800 );
                    aB2DPolyPoly.transform( aMatrix );
                    rVector.push_back( aB2DPolyPoly );
                }
            }
        }

        nXOffset += nCharWidth;
        bRet = bRet && bSuccess;
    }

    return bRet;
}

bool OutputDevice::GetTextOutlines( PolyPolyVector& rResultVector,
                                        const OUString& rStr, sal_Int32 nBase,
                                        sal_Int32 nIndex, sal_Int32 nLen, bool bOptimize,
                                        sal_uLong nTWidth, const long* pDXArray ) const
{
    if(nLen == 0x0FFFF)
    {
        SAL_INFO("sal.rtl.xub",
                 "GetTextOutlines Suspicious arguments  nLen:" << nLen);
    }

    rResultVector.clear();

    // get the basegfx polypolygon vector
    ::basegfx::B2DPolyPolygonVector aB2DPolyPolyVector;
    if( !GetTextOutlines( aB2DPolyPolyVector, rStr, nBase, nIndex, nLen,
                         bOptimize, nTWidth, pDXArray ) )
        return false;

    // convert to a tool polypolygon vector
    rResultVector.reserve( aB2DPolyPolyVector.size() );
    ::basegfx::B2DPolyPolygonVector::const_iterator aIt = aB2DPolyPolyVector.begin();
    for(; aIt != aB2DPolyPolyVector.end(); ++aIt )
        rResultVector.push_back(tools::PolyPolygon(*aIt)); // #i76339#

    return true;
}

bool OutputDevice::GetTextOutline( tools::PolyPolygon& rPolyPoly, const OUString& rStr,
                                       sal_Int32 nBase, sal_Int32 nIndex, sal_Int32 nLen,
                                       bool bOptimize, sal_uLong nTWidth, const long* pDXArray ) const
{
    if(nLen == 0x0FFFF)
    {
        SAL_INFO("sal.rtl.xub",
                 "GetTextOutline Suspicious arguments nLen:" << nLen);
    }
    rPolyPoly.Clear();

    // get the basegfx polypolygon vector
    ::basegfx::B2DPolyPolygonVector aB2DPolyPolyVector;
    if( !GetTextOutlines( aB2DPolyPolyVector, rStr, nBase, nIndex, nLen,
                         bOptimize, nTWidth, pDXArray ) )
        return false;

    // convert and merge into a tool polypolygon
    ::basegfx::B2DPolyPolygonVector::const_iterator aIt = aB2DPolyPolyVector.begin();
    for(; aIt != aB2DPolyPolyVector.end(); ++aIt )
        for( unsigned int i = 0; i < aIt->count(); ++i )
            rPolyPoly.Insert(tools::Polygon((*aIt).getB2DPolygon( i ))); // #i76339#

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
