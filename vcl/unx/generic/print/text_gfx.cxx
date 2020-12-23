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

#include "psputil.hxx"
#include "glyphset.hxx"

#include <unx/printergfx.hxx>
#include <unx/fontmanager.hxx>

using namespace psp ;

/*
 * implement text handling printer routines,
 */

void PrinterGfx::SetFont(
                    sal_Int32 nFontID,
                    sal_Int32 nHeight,
                    sal_Int32 nWidth,
                    Degree10 nAngle,
                    bool bVertical,
                    bool bArtItalic,
                    bool bArtBold
                    )
{
    // font and encoding will be set by drawText again immediately
    // before PSShowText
    mnFontID                          = nFontID;
    maVirtualStatus.maFont.clear();
    maVirtualStatus.maEncoding        = RTL_TEXTENCODING_DONTKNOW;
    maVirtualStatus.mnTextHeight      = nHeight;
    maVirtualStatus.mnTextWidth       = nWidth;
    maVirtualStatus.mbArtItalic       = bArtItalic;
    maVirtualStatus.mbArtBold         = bArtBold;
    mnTextAngle                       = nAngle;
    mbTextVertical                    = bVertical;
}

void PrinterGfx::drawGlyph(const Point& rPoint,
                           sal_GlyphId aGlyphId)
{

    // draw the string
    // search for a glyph set matching the set font
    bool bGlyphFound = false;
    for (auto & elem : maPS3Font)
        if ( (elem.GetFontID()  == mnFontID)
             && (elem.IsVertical() == mbTextVertical))
        {
            elem.DrawGlyph (*this, rPoint, aGlyphId);
            bGlyphFound = true;
            break;
        }

    // not found ? create a new one
    if (!bGlyphFound)
    {
        maPS3Font.emplace_back(mnFontID, mbTextVertical);
        maPS3Font.back().DrawGlyph (*this, rPoint, aGlyphId);
    }
}

void PrinterGfx::DrawGlyph(const Point& rPoint,
                           const GlyphItem& rGlyph)
{
    // move and rotate the user coordinate system
    // avoid the gsave/grestore for the simple cases since it allows
    // reuse of the current font if it hasn't changed
    Degree10 nCurrentTextAngle = mnTextAngle;
    Point aPoint( rPoint );

    if (nCurrentTextAngle)
    {
        PSGSave ();
        PSTranslate (rPoint);
        PSRotate (nCurrentTextAngle);
        mnTextAngle = 0_deg10;
        aPoint = Point( 0, 0 );
    }

    if (mbTextVertical && rGlyph.IsVertical())
    {
        sal_Int32 nTextHeight = maVirtualStatus.mnTextHeight;
        sal_Int32 nTextWidth  = maVirtualStatus.mnTextWidth ? maVirtualStatus.mnTextWidth : maVirtualStatus.mnTextHeight;
        sal_Int32 nAscend = mrFontMgr.getFontAscend( mnFontID );
        sal_Int32 nDescend = mrFontMgr.getFontDescend( mnFontID );

        nDescend = nDescend * nTextHeight / 1000;
        nAscend = nAscend * nTextHeight / 1000;

        Point aRotPoint( -nDescend*nTextWidth/nTextHeight, nAscend*nTextWidth/nTextHeight );

        // transform matrix to new individual direction
        PSGSave ();
        GraphicsStatus aSaveStatus = maVirtualStatus;
        // switch font aspect
        maVirtualStatus.mnTextWidth = nTextHeight;
        maVirtualStatus.mnTextHeight = nTextWidth;
        if( aPoint.X() || aPoint.Y() )
            PSTranslate( aPoint );
        PSRotate (900_deg10);
        // draw the rotated glyph
        drawGlyph(aRotPoint, rGlyph.glyphId());

        // restore previous state
        maVirtualStatus = aSaveStatus;
        PSGRestore();
    }
    else
        drawGlyph(aPoint, rGlyph.glyphId());

    // restore the user coordinate system
    if (nCurrentTextAngle)
    {
        PSGRestore ();
        mnTextAngle = nCurrentTextAngle;
    }
}

/*
 * spool the converted truetype fonts to the page header after the page body is
 * complete
 * for Type1 fonts spool additional reencoding vectors that are necessary to access the
 * whole font
 */

void
PrinterGfx::OnEndJob ()
{
    maPS3Font.clear();
}

void
PrinterGfx::writeResources( osl::File* pFile, std::vector< OString >& rSuppliedFonts )
{
    // write glyphsets and reencodings
    for (auto & PS3Font : maPS3Font)
    {
        PS3Font.PSUploadFont (*pFile, *this, mbUploadPS42Fonts, rSuppliedFonts );
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
