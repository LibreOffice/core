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

#include <math.h>

#include "psputil.hxx"
#include "glyphset.hxx"

#include "unx/printergfx.hxx"
#include "unx/fontmanager.hxx"
#include <unx/helper.hxx>

#include "osl/thread.h"

#include "sal/alloca.h"

using namespace psp ;

/*
 * implement text handling printer routines,
 */

void PrinterGfx::SetFont(
                    sal_Int32 nFontID,
                    sal_Int32 nHeight,
                    sal_Int32 nWidth,
                    sal_Int32 nAngle,
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
                           sal_GlyphId aGlyphId,
                           sal_Int32 nDelta)
{

    // draw the string
    // search for a glyph set matching the set font
    std::list< GlyphSet >::iterator aIter;
    for (aIter = maPS3Font.begin(); aIter != maPS3Font.end(); ++aIter)
        if ( ((*aIter).GetFontID()  == mnFontID)
             && ((*aIter).IsVertical() == mbTextVertical))
        {
            (*aIter).DrawGlyph (*this, rPoint, aGlyphId, nDelta);
            break;
        }

    // not found ? create a new one
    if (aIter == maPS3Font.end())
    {
        maPS3Font.push_back (GlyphSet(mnFontID, mbTextVertical));
        maPS3Font.back().DrawGlyph (*this, rPoint, aGlyphId, nDelta);
    }
}

void PrinterGfx::DrawGlyph(const Point& rPoint,
                           const GlyphItem& rGlyph,
                           sal_Int32 nDelta)
{
    // move and rotate the user coordinate system
    // avoid the gsave/grestore for the simple cases since it allows
    // reuse of the current font if it hasn't changed
    sal_Int32 nCurrentTextAngle = mnTextAngle;
    Point aPoint( rPoint );

    if (nCurrentTextAngle != 0)
    {
        PSGSave ();
        PSTranslate (rPoint);
        PSRotate (nCurrentTextAngle);
        mnTextAngle = 0;
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

        Point aRotPoint = Point( -nDescend*nTextWidth/nTextHeight, nAscend*nTextWidth/nTextHeight );

        // transform matrix to new individual direction
        PSGSave ();
        GraphicsStatus aSaveStatus = maVirtualStatus;
        // switch font aspect
        maVirtualStatus.mnTextWidth = nTextHeight;
        maVirtualStatus.mnTextHeight = nTextWidth;
        if( aPoint.X() || aPoint.Y() )
            PSTranslate( aPoint );
        PSRotate (900);
        // draw the rotated glyph
        drawGlyph(aRotPoint, rGlyph.maGlyphId, 0);

        // restore previous state
        maVirtualStatus = aSaveStatus;
        PSGRestore();
    }
    else
        drawGlyph(aPoint, rGlyph.maGlyphId, nDelta);

    // restore the user coordinate system
    if (nCurrentTextAngle != 0)
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
PrinterGfx::writeResources( osl::File* pFile, std::list< OString >& rSuppliedFonts )
{
    // write glyphsets and reencodings
    std::list< GlyphSet >::iterator aIter;
    for (aIter = maPS3Font.begin(); aIter != maPS3Font.end(); ++aIter)
    {
        aIter->PSUploadFont (*pFile, *this, mbUploadPS42Fonts, rSuppliedFonts );
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
