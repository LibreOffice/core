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
#include <vcl/helper.hxx>

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

void PrinterGfx::drawGlyphs(
                            const Point& rPoint,
                            sal_GlyphId* pGlyphIds,
                            sal_Unicode* pUnicodes,
                            sal_Int16 nLen,
                            sal_Int32* pDeltaArray
                            )
{

    // draw the string
    // search for a glyph set matching the set font
    std::list< GlyphSet >::iterator aIter;
    for (aIter = maPS3Font.begin(); aIter != maPS3Font.end(); ++aIter)
        if ( ((*aIter).GetFontID()  == mnFontID)
             && ((*aIter).IsVertical() == mbTextVertical))
        {
            (*aIter).DrawGlyphs (*this, rPoint, pGlyphIds, pUnicodes, nLen, pDeltaArray);
            break;
        }

    // not found ? create a new one
    if (aIter == maPS3Font.end())
    {
        maPS3Font.push_back (GlyphSet(mnFontID, mbTextVertical));
        maPS3Font.back().DrawGlyphs (*this, rPoint, pGlyphIds, pUnicodes, nLen, pDeltaArray);
    }
}

void PrinterGfx::DrawGlyphs(
                            const Point& rPoint,
                            sal_GlyphId* pGlyphIds,
                            sal_Unicode* pUnicodes,
                            sal_Int16 nLen,
                            sal_Int32* pDeltaArray
                            )
{
    if( nLen <= 0 )
        return;

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

    if( mbTextVertical )
    {
        // vertical glyphs can have an additional rotation ... sigh.
        // so break up text in chunks of normal glyphs and print out
        // specially rotated glyphs extra
        sal_GlyphId* pTempGlyphIds = static_cast<sal_GlyphId*>(alloca(sizeof(sal_Int32)*nLen));
        sal_Int32* pTempDelta = static_cast<sal_Int32*>(alloca(sizeof(sal_Int32)*nLen));
        sal_Unicode* pTempUnicodes = static_cast<sal_Unicode*>(alloca(sizeof(sal_Unicode)*nLen));
        sal_Int16 nTempLen = 0;
        sal_Int32 nTempFirstDelta = 0;
        Point aRotPoint;
        sal_Int32 nTextHeight = maVirtualStatus.mnTextHeight;
        sal_Int32 nTextWidth  = maVirtualStatus.mnTextWidth ? maVirtualStatus.mnTextWidth : maVirtualStatus.mnTextHeight;
        sal_Int32 nAscend = mrFontMgr.getFontAscend( mnFontID );
        sal_Int32 nDescend = mrFontMgr.getFontDescend( mnFontID );

        nDescend = nDescend * nTextHeight / 1000;
        nAscend = nAscend * nTextHeight / 1000;

        for( sal_Int16 i = 0; i < nLen; i++ )
        {
            const sal_GlyphId nRot = pGlyphIds[i] & GF_ROTMASK;
            if( nRot == GF_NONE )
            {
                pTempUnicodes[nTempLen] = pUnicodes[i];
                pTempGlyphIds[nTempLen] = pGlyphIds[i];
                if( nTempLen > 0 )
                    pTempDelta[nTempLen-1]  = pDeltaArray[i-1]-nTempFirstDelta;
                else
                {
                    // the first element in pDeltaArray shows
                    // the offset of the second character
                    // so if the first glyph is normal
                    // then we do not need to move the delta indices
                    // else we have to move them down by one and
                    // recalculate aPoint and all deltas
                    if( i != 0 )
                        nTempFirstDelta = pDeltaArray[ i-1 ];
                }
                nTempLen++;
            }
            else
            {
                sal_Int32 nOffset = i > 0 ? pDeltaArray[i-1] : 0;
                sal_Int32 nRotAngle = 0;
                switch( nRot )
                {
                    case GF_ROTR:
                        nRotAngle = 2700;
                        aRotPoint = Point( -nAscend*nTextWidth/nTextHeight, -nDescend*nTextWidth/nTextHeight - nOffset );
                        break;
                    case GF_ROTL:
                        nRotAngle = 900;
                        aRotPoint = Point( -nDescend*nTextWidth/nTextHeight, nOffset + nAscend*nTextWidth/nTextHeight );
                        break;
                }
                sal_GlyphId nRotGlyphId     = pGlyphIds[i];
                sal_Unicode nRotUnicode     = pUnicodes[i];
                sal_Int32 nRotDelta         = 0;

                // transform matrix to new individual direction
                PSGSave ();
                GraphicsStatus aSaveStatus = maVirtualStatus;
                if( nRot != 2 ) // switch font aspect
                {
                    maVirtualStatus.mnTextWidth = nTextHeight;
                    maVirtualStatus.mnTextHeight = nTextWidth;
                }
                if( aPoint.X() || aPoint.Y() )
                    PSTranslate( aPoint );
                PSRotate (nRotAngle);
                // draw the rotated glyph
                drawGlyphs( aRotPoint, &nRotGlyphId, &nRotUnicode, 1, &nRotDelta );

                // restore previous state
                maVirtualStatus = aSaveStatus;
                PSGRestore();
            }
        }

        pGlyphIds = pTempGlyphIds;
        pUnicodes = pTempUnicodes;
        pDeltaArray = pTempDelta;
        nLen = nTempLen;

        aPoint.X() += nTempFirstDelta;
    }

    if( nLen > 0 )
        drawGlyphs( aPoint, pGlyphIds, pUnicodes, nLen, pDeltaArray );

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
