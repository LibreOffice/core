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
#include <cstring>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <osl/endian.h>
#include <osl/file.hxx>
#include <sal/types.h>

#include <vcl/sysdata.hxx>

#include "fontsubset.hxx"
#include "quartz/salbmp.h"
#ifdef MACOSX
#include "quartz/salgdi.h"
#endif
#include "quartz/utils.h"
#ifdef IOS
#include "saldatabasic.hxx"
#endif
#include "sft.hxx"


#if defined(IOS) && defined(DBG_UTIL)

// Variables in TiledView.m
extern int DBG_DRAW_ROUNDS, DBG_DRAW_COUNTER, DBG_DRAW_DEPTH;

#define DBG_DRAW_OPERATION(s,v) \
    do { \
        if (DBG_DRAW_ROUNDS >= 0) { \
            if (DBG_DRAW_COUNTER++ > DBG_DRAW_ROUNDS) \
                return v; \
            SAL_DEBUG("===> " << s << " " << DBG_DRAW_COUNTER); \
        } \
    } while (false)

#define DBG_DRAW_OPERATION_EXIT(s) \
    do { \
        if (DBG_DRAW_ROUNDS >= 0) \
            SAL_DEBUG("<=== " << s << " " << DBG_DRAW_COUNTER); \
    } while (false)

#define DBG_DRAW_OPERATION_EXIT_EARLY(s) DBG_DRAW_OPERATION_EXIT(s << " exit early " << __LINE__)

#else

#define DBG_DRAW_OPERATION(s,v) /* empty */
#define DBG_DRAW_OPERATION_EXIT(s) /* empty */
#define DBG_DRAW_OPERATION_EXIT_EARLY(s) /* empty */

#endif

using namespace vcl;


static const basegfx::B2DPoint aHalfPointOfs ( 0.5, 0.5 );

static void AddPolygonToPath( CGMutablePathRef xPath,
                              const basegfx::B2DPolygon& rPolygon,
                              bool bClosePath, bool bPixelSnap, bool bLineDraw )
{
    // short circuit if there is nothing to do
    const int nPointCount = rPolygon.count();
    if( nPointCount <= 0 )
    {
        return;
    }
    (void)bPixelSnap; // TODO

    const bool bHasCurves = rPolygon.areControlPointsUsed();
    for( int nPointIdx = 0, nPrevIdx = 0;; nPrevIdx = nPointIdx++ )
    {
        int nClosedIdx = nPointIdx;
        if( nPointIdx >= nPointCount )
        {
            // prepare to close last curve segment if needed
            if( bClosePath && (nPointIdx == nPointCount) )
            {
                nClosedIdx = 0;
            }
            else
            {
                break;
            }
        }

        basegfx::B2DPoint aPoint = rPolygon.getB2DPoint( nClosedIdx );

        if( bPixelSnap)
        {
            // snap device coordinates to full pixels
            aPoint.setX( basegfx::fround( aPoint.getX() ) );
            aPoint.setY( basegfx::fround( aPoint.getY() ) );
        }

        if( bLineDraw )
        {
            aPoint += aHalfPointOfs;
        }
        if( !nPointIdx )
        {
            // first point => just move there
            SAL_INFO( "vcl.cg", "CGPathMoveToPoint(" << xPath << ",NULL," << aPoint.getX() << "," << aPoint.getY() << ")");
            CGPathMoveToPoint( xPath, nullptr, aPoint.getX(), aPoint.getY() );
            continue;
        }

        bool bPendingCurve = false;
        if( bHasCurves )
        {
            bPendingCurve = rPolygon.isNextControlPointUsed( nPrevIdx );
            bPendingCurve |= rPolygon.isPrevControlPointUsed( nClosedIdx );
        }

        if( !bPendingCurve )    // line segment
        {
            SAL_INFO( "vcl.cg", "CGPathAddLineToPoint(" << xPath << ",NULL," << aPoint.getX() << "," << aPoint.getY() << ")");
            CGPathAddLineToPoint( xPath, nullptr, aPoint.getX(), aPoint.getY() );
        }
        else                        // cubic bezier segment
        {
            basegfx::B2DPoint aCP1 = rPolygon.getNextControlPoint( nPrevIdx );
            basegfx::B2DPoint aCP2 = rPolygon.getPrevControlPoint( nClosedIdx );
            if( bLineDraw )
            {
                aCP1 += aHalfPointOfs;
                aCP2 += aHalfPointOfs;
            }
            SAL_INFO( "vcl.cg", "CGPathAddCurveToPoint(" << xPath << ",NULL," << aCP1.getX() << "," << aCP1.getY() << "," <<
                      aCP2.getX() << "," << aCP2.getY() << "," << aPoint.getX() << "," << aPoint.getY() << ")" );
            CGPathAddCurveToPoint( xPath, nullptr, aCP1.getX(), aCP1.getY(),
                                    aCP2.getX(), aCP2.getY(), aPoint.getX(), aPoint.getY() );
        }
    }

    if( bClosePath )
    {
        SAL_INFO( "vcl.cg", "CGPathCloseSubpath(" << xPath << ")" );
        CGPathCloseSubpath( xPath );
    }
}

static void AddPolyPolygonToPath( CGMutablePathRef xPath,
                                  const basegfx::B2DPolyPolygon& rPolyPoly,
                                  bool bPixelSnap, bool bLineDraw )
{
    // short circuit if there is nothing to do
    const int nPolyCount = rPolyPoly.count();
    if( nPolyCount <= 0 )
    {
        return;
    }
    for( int nPolyIdx = 0; nPolyIdx < nPolyCount; ++nPolyIdx )
    {
        const basegfx::B2DPolygon rPolygon = rPolyPoly.getB2DPolygon( nPolyIdx );
        AddPolygonToPath( xPath, rPolygon, true, bPixelSnap, bLineDraw );
    }
}

bool AquaSalGraphics::CreateFontSubset( const OUString& rToFile,
                                        const PhysicalFontFace* pFontData,
                                        const sal_GlyphId* pGlyphIds, const sal_uInt8* pEncoding,
                                        sal_Int32* pGlyphWidths, int nGlyphCount,
                                        FontSubsetInfo& rInfo )
{
    // TODO: move more of the functionality here into the generic subsetter code

    // prepare the requested file name for writing the font-subset file
    OUString aSysPath;
    if( osl_File_E_None != osl_getSystemPathFromFileURL( rToFile.pData, &aSysPath.pData ) )
    {
        return false;
    }

    // get the raw-bytes from the font to be subset
    std::vector<unsigned char> aBuffer;
    bool bCffOnly = false;
    if( !GetRawFontData( pFontData, aBuffer, &bCffOnly ) )
    {
        return false;
    }
    const OString aToFile( OUStringToOString( aSysPath,
                                              osl_getThreadTextEncoding()));

    // handle CFF-subsetting
    if( bCffOnly )
    {
        // provide the raw-CFF data to the subsetter
        ByteCount nCffLen = aBuffer.size();
        rInfo.LoadFont( FontType::CFF_FONT, &aBuffer[0], nCffLen );

        // NOTE: assuming that all glyphids requested on Aqua are fully translated

        // make the subsetter provide the requested subset
        FILE* pOutFile = fopen( aToFile.getStr(), "wb" );
        bool bRC = rInfo.CreateFontSubset( FontType::TYPE1_PFB, pOutFile, nullptr,
                                           pGlyphIds, pEncoding, nGlyphCount, pGlyphWidths );
        fclose( pOutFile );
        return bRC;
    }

    // TODO: modernize psprint's horrible fontsubset C-API
    // this probably only makes sense after the switch to another SCM
    // that can preserve change history after file renames

    // prepare data for psprint's font subsetter
    TrueTypeFont* pSftFont = nullptr;
    int nRC = ::OpenTTFontBuffer( static_cast<void*>(&aBuffer[0]), aBuffer.size(), 0, &pSftFont);
    if( nRC != SF_OK )
    {
        return false;
    }
    // get details about the subsetted font
    TTGlobalFontInfo aTTInfo;
    ::GetTTGlobalFontInfo( pSftFont, &aTTInfo );
    rInfo.m_nFontType = FontType::SFNT_TTF;
    rInfo.m_aPSName = OUString( aTTInfo.psname, std::strlen(aTTInfo.psname),
                                RTL_TEXTENCODING_UTF8 );
    rInfo.m_aFontBBox = Rectangle( Point( aTTInfo.xMin, aTTInfo.yMin ),
                                   Point( aTTInfo.xMax, aTTInfo.yMax ) );
    rInfo.m_nCapHeight = aTTInfo.yMax; // Well ...
    rInfo.m_nAscent = aTTInfo.winAscent;
    rInfo.m_nDescent = aTTInfo.winDescent;
    // mac fonts usually do not have an OS2-table
    // => get valid ascent/descent values from other tables
    if( !rInfo.m_nAscent )
    {
        rInfo.m_nAscent = +aTTInfo.typoAscender;
    }
    if( !rInfo.m_nAscent )
    {
        rInfo.m_nAscent = +aTTInfo.ascender;
    }
    if( !rInfo.m_nDescent )
    {
        rInfo.m_nDescent = +aTTInfo.typoDescender;
    }
    if( !rInfo.m_nDescent )
    {
        rInfo.m_nDescent = -aTTInfo.descender;
    }

    // subset glyphs and get their properties
    // take care that subset fonts require the NotDef glyph in pos 0
    int nOrigCount = nGlyphCount;
    sal_uInt16 aShortIDs[ 257 ];
    sal_uInt8 aTempEncs[ 257 ];
    int nNotDef = -1;

    assert( (nGlyphCount <= 256 && "too many glyphs for subsetting" ));

    for( int i = 0; i < nGlyphCount; ++i )
    {
        aTempEncs[i] = pEncoding[i];

        sal_GlyphId aGlyphId(pGlyphIds[i]);
        aShortIDs[i] = static_cast<sal_uInt16>( aGlyphId );
        if( !aGlyphId && nNotDef < 0 )
        {
            nNotDef = i; // first NotDef glyph found
        }
    }

    if( nNotDef != 0 )
    {
        // add fake NotDef glyph if needed
        if( nNotDef < 0 )
        {
            nNotDef = nGlyphCount++;
        }
        // NotDef glyph must be in pos 0 => swap glyphids
        aShortIDs[ nNotDef ] = aShortIDs[0];
        aTempEncs[ nNotDef ] = aTempEncs[0];
        aShortIDs[0] = 0;
        aTempEncs[0] = 0;
    }

    // TODO: where to get bVertical?
    const bool bVertical = false;

    // fill the pGlyphWidths array
    // while making sure that the NotDef glyph is at index==0
    TTSimpleGlyphMetrics* pGlyphMetrics = ::GetTTSimpleGlyphMetrics( pSftFont, aShortIDs,
                                                                     nGlyphCount, bVertical );
    if( !pGlyphMetrics )
    {
        return false;
    }

    sal_uInt16 nNotDefAdv = pGlyphMetrics[0].adv;
    pGlyphMetrics[0].adv = pGlyphMetrics[nNotDef].adv;
    pGlyphMetrics[nNotDef].adv  = nNotDefAdv;
    for( int i = 0; i < nOrigCount; ++i )
    {
        pGlyphWidths[i] = pGlyphMetrics[i].adv;
    }
    free( pGlyphMetrics );

    // write subset into destination file
    nRC = ::CreateTTFromTTGlyphs( pSftFont, aToFile.getStr(), aShortIDs,
                                  aTempEncs, nGlyphCount, 0, nullptr );
    ::CloseTTFont(pSftFont);
    return (nRC == SF_OK);
}

static inline void alignLinePoint( const SalPoint* i_pIn, float& o_fX, float& o_fY )
{
    o_fX = static_cast<float>(i_pIn->mnX ) + 0.5;
    o_fY = static_cast<float>(i_pIn->mnY ) + 0.5;
}

void AquaSalGraphics::copyBits( const SalTwoRect& rPosAry, SalGraphics *pSrcGraphics )
{

    if( !pSrcGraphics )
    {
        pSrcGraphics = this;
    }
    //from unix salgdi2.cxx
    //[FIXME] find a better way to prevent calc from crashing when width and height are negative
    if( rPosAry.mnSrcWidth <= 0 ||
        rPosAry.mnSrcHeight <= 0 ||
        rPosAry.mnDestWidth <= 0 ||
        rPosAry.mnDestHeight <= 0 )
    {
        return;
    }

#ifdef IOS
    // If called from idle layout, mrContext is NULL, no idea what to do
    if (!mrContext)
        return;
#endif

    // accelerate trivial operations
    /*const*/ AquaSalGraphics* pSrc = static_cast<AquaSalGraphics*>(pSrcGraphics);
    const bool bSameGraphics = (this == pSrc)
#ifdef MACOSX
        || (mbWindow && mpFrame && pSrc->mbWindow && (mpFrame == pSrc->mpFrame))
#endif
        ;

    if( bSameGraphics &&
        (rPosAry.mnSrcWidth == rPosAry.mnDestWidth) &&
        (rPosAry.mnSrcHeight == rPosAry.mnDestHeight))
    {
        // short circuit if there is nothing to do
        if( (rPosAry.mnSrcX == rPosAry.mnDestX) &&
            (rPosAry.mnSrcY == rPosAry.mnDestY))
        {
            return;
        }
        // use copyArea() if source and destination context are identical
        copyArea( rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnSrcX, rPosAry.mnSrcY,
                  rPosAry.mnSrcWidth, rPosAry.mnSrcHeight, false/*bWindowInvalidate*/ );
        return;
    }

    ApplyXorContext();
    pSrc->ApplyXorContext();

    SAL_WARN_IF( !pSrc->mxLayer, "vcl.quartz",
                 "AquaSalGraphics::copyBits() from non-layered graphics this=" << this );

    const CGPoint aDstPoint = CGPointMake(+rPosAry.mnDestX - rPosAry.mnSrcX, rPosAry.mnDestY - rPosAry.mnSrcY);
    if( (rPosAry.mnSrcWidth == rPosAry.mnDestWidth &&
         rPosAry.mnSrcHeight == rPosAry.mnDestHeight) &&
        (!mnBitmapDepth || (aDstPoint.x + pSrc->mnWidth) <= mnWidth)
        && pSrc->mxLayer ) // workaround for a Quartz crash
    {
        // in XOR mode the drawing context is redirected to the XOR mask
        // if source and target are identical then copyBits() paints onto the target context though
        CGContextRef xCopyContext = mrContext;
        if( mpXorEmulation && mpXorEmulation->IsEnabled() )
        {
            if( pSrcGraphics == this )
            {
                xCopyContext = mpXorEmulation->GetTargetContext();
            }
        }
        SAL_INFO( "vcl.cg", "CGContextSaveGState(" << xCopyContext << ")" );
        CGContextSaveGState( xCopyContext );

        const CGRect aDstRect = CGRectMake(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth, rPosAry.mnDestHeight);
        SAL_INFO( "vcl.cg", "CGContextClipToRect(" << xCopyContext << "," << aDstRect << ")" );
        CGContextClipToRect( xCopyContext, aDstRect );

        // draw at new destination
        // NOTE: flipped drawing gets disabled for this, else the subimage would be drawn upside down
        if( pSrc->IsFlipped() )
        {
            SAL_INFO( "vcl.cg", "CGContextTranslateCTM(" << xCopyContext << ",0," << mnHeight << ")" );
            CGContextTranslateCTM( xCopyContext, 0, +mnHeight );
            SAL_INFO( "vcl.cg", "CGContextScaleCTM(" << xCopyContext << ",+1,-1)" );
            CGContextScaleCTM( xCopyContext, +1, -1 );
        }

        // TODO: pSrc->size() != this->size()
        SAL_INFO( "vcl.cg", "CGContextDrawLayerAtPoint(" << xCopyContext << "," << aDstPoint << "," << pSrc->mxLayer << ")" );
        CGContextDrawLayerAtPoint( xCopyContext, aDstPoint, pSrc->mxLayer );

        SAL_INFO( "vcl.cg", "CGContextRestoreGState(" << xCopyContext << ")" );
        CGContextRestoreGState( xCopyContext );
        // mark the destination rectangle as updated
        RefreshRect( aDstRect );
    }
    else
    {
        SalBitmap* pBitmap = pSrc->getBitmap( rPosAry.mnSrcX, rPosAry.mnSrcY,
                                              rPosAry.mnSrcWidth, rPosAry.mnSrcHeight );
        if( pBitmap )
        {
            SalTwoRect aPosAry( rPosAry );
            aPosAry.mnSrcX = 0;
            aPosAry.mnSrcY = 0;
            drawBitmap( aPosAry, *pBitmap );
            delete pBitmap;
        }
    }
}

static void DrawPattern50( void*, CGContextRef rContext )
{
    static const CGRect aRects[2] = { { {0,0}, { 2, 2 } }, { { 2, 2 }, { 2, 2 } } };
    SAL_INFO( "vcl.cg", "CGContextAddRects(" << rContext << ",aRects,2 )" );
    CGContextAddRects( rContext, aRects, 2 );
    SAL_INFO( "vcl.cg", "CGContextFillPath(" << rContext << ")" );
    CGContextFillPath( rContext );
}

static void getBoundRect( sal_uInt32 nPoints, const SalPoint *pPtAry,
                          long &rX, long& rY, long& rWidth, long& rHeight )
{
    long nX1 = pPtAry->mnX;
    long nX2 = nX1;
    long nY1 = pPtAry->mnY;
    long nY2 = nY1;

    for( sal_uInt32 n = 1; n < nPoints; n++ )
    {
        if( pPtAry[n].mnX < nX1 )
        {
            nX1 = pPtAry[n].mnX;
        }
        else if( pPtAry[n].mnX > nX2 )
        {
            nX2 = pPtAry[n].mnX;
        }
        if( pPtAry[n].mnY < nY1 )
        {
            nY1 = pPtAry[n].mnY;
        }
        else if( pPtAry[n].mnY > nY2 )
        {
            nY2 = pPtAry[n].mnY;
        }
    }
    rX = nX1;
    rY = nY1;
    rWidth = nX2 - nX1 + 1;
    rHeight = nY2 - nY1 + 1;
}

static SalColor ImplGetROPSalColor( SalROPColor nROPColor )
{
    SalColor nSalColor;
    if ( nROPColor == SalROPColor::N0 )
    {
        nSalColor = MAKE_SALCOLOR( 0, 0, 0 );
    }
    else
    {
        nSalColor = MAKE_SALCOLOR( 255, 255, 255 );
    }
    return nSalColor;
}

// apply the XOR mask to the target context if active and dirty
void AquaSalGraphics::ApplyXorContext()
{
    if( !mpXorEmulation )
    {
        return;
    }
    if( mpXorEmulation->UpdateTarget() )
    {
        RefreshRect( 0, 0, mnWidth, mnHeight ); // TODO: refresh minimal changerect
    }
}

void AquaSalGraphics::copyArea( long nDstX, long nDstY,long nSrcX, long nSrcY,
                                long nSrcWidth, long nSrcHeight, bool /*bWindowInvalidate*/ )
{
    SAL_WARN_IF( !mxLayer, "vcl.quartz",
                 "AquaSalGraphics::copyArea() for non-layered graphics this=" << this );

#ifdef IOS
    if( !mxLayer )
        return;
#endif

    ApplyXorContext();

    // in XOR mode the drawing context is redirected to the XOR mask
    // copyArea() always works on the target context though
    CGContextRef xCopyContext = mrContext;
    if( mpXorEmulation && mpXorEmulation->IsEnabled() )
    {
        xCopyContext = mpXorEmulation->GetTargetContext();
    }
    // drawing a layer onto its own context causes trouble on OSX => copy it first
    // TODO: is it possible to get rid of this unneeded copy more often?
    //       e.g. on OSX>=10.5 only this situation causes problems:
    //          mnBitmapDepth && (aDstPoint.x + pSrc->mnWidth) > mnWidth

    CGLayerRef xSrcLayer = mxLayer;
    // TODO: if( mnBitmapDepth > 0 )
    {
        const CGSize aSrcSize = CGSizeMake(nSrcWidth, nSrcHeight);
        xSrcLayer = CGLayerCreateWithContext( xCopyContext, aSrcSize, nullptr );
        SAL_INFO( "vcl.cg", "CGLayerCreateWithContext(" << xCopyContext << "," << aSrcSize << ",NULL) = " << xSrcLayer );

        const CGContextRef xSrcContext = CGLayerGetContext( xSrcLayer );
        SAL_INFO( "vcl.cg", "CGLayerGetContext(" << xSrcLayer << ") = " << xSrcContext );

        CGPoint aSrcPoint = CGPointMake(-nSrcX, -nSrcY);
        if( IsFlipped() )
        {
            SAL_INFO( "vcl.cg", "CGContextTranslateCTM(" << xSrcContext << ",0," << nSrcHeight << ")" );
            CGContextTranslateCTM( xSrcContext, 0, +nSrcHeight );
            SAL_INFO( "vcl.cg", "CGContextScaleCTM(" << xSrcContext << ",+1,-1)" );
            CGContextScaleCTM( xSrcContext, +1, -1 );
            aSrcPoint.y = (nSrcY + nSrcHeight) - mnHeight;
        }
        SAL_INFO( "vcl.cg", "CGContextDrawLayerAtPoint(" << xSrcContext << "," << aSrcPoint << "," << mxLayer << ")" );
        CGContextDrawLayerAtPoint( xSrcContext, aSrcPoint, mxLayer );
    }

    // draw at new destination
    const CGPoint aDstPoint = CGPointMake(+nDstX, +nDstY);
    SAL_INFO( "vcl.cg", "CGContextDrawLayerAtPoint(" << xCopyContext << "," << aDstPoint << "," << xSrcLayer << ")" );
    CGContextDrawLayerAtPoint( xCopyContext, aDstPoint, xSrcLayer );

    // cleanup
    if( xSrcLayer != mxLayer )
    {
        SAL_INFO( "vcl.cg", "CGLayerRelease(" << xSrcLayer << ")" );
        CGLayerRelease( xSrcLayer );
    }
    // mark the destination rectangle as updated
    RefreshRect( nDstX, nDstY, nSrcWidth, nSrcHeight );
}

#ifndef IOS

void AquaSalGraphics::copyResolution( AquaSalGraphics& rGraphics )
{
    if( !rGraphics.mnRealDPIY && rGraphics.mbWindow && rGraphics.mpFrame )
    {
        rGraphics.initResolution( rGraphics.mpFrame->getNSWindow() );
    }
    mnRealDPIX = rGraphics.mnRealDPIX;
    mnRealDPIY = rGraphics.mnRealDPIY;
}

#endif

bool AquaSalGraphics::blendBitmap( const SalTwoRect&,
                                   const SalBitmap& )
{
    return false;
}

bool AquaSalGraphics::blendAlphaBitmap( const SalTwoRect&,
                                        const SalBitmap&,
                                        const SalBitmap&,
                                        const SalBitmap& )
{
    return false;
}

bool AquaSalGraphics::drawAlphaBitmap( const SalTwoRect& rTR,
                                       const SalBitmap& rSrcBitmap,
                                       const SalBitmap& rAlphaBmp )
{
    DBG_DRAW_OPERATION("drawAlphaBitmap", true);

    if (rTR.mnSrcWidth != rTR.mnDestWidth || rTR.mnSrcHeight != rTR.mnDestHeight)
    {
        // TODO - would be better to scale it by the native code
        return false;
    }

    // An image mask can't have a depth > 8 bits (should be 1 to 8 bits)
    if( rAlphaBmp.GetBitCount() > 8 )
    {
        DBG_DRAW_OPERATION_EXIT_EARLY("drawAlphaBitmap");
        return false;
    }

    // are these two tests really necessary? (see vcl/unx/source/gdi/salgdi2.cxx)
    // horizontal/vertical mirroring not implemented yet
    if( rTR.mnDestWidth < 0 || rTR.mnDestHeight < 0 )
    {
        DBG_DRAW_OPERATION_EXIT_EARLY("drawAlphaBitmap");
        return false;
    }

    const QuartzSalBitmap& rSrcSalBmp = static_cast<const QuartzSalBitmap&>(rSrcBitmap);
    const QuartzSalBitmap& rMaskSalBmp = static_cast<const QuartzSalBitmap&>(rAlphaBmp);
    CGImageRef xMaskedImage = rSrcSalBmp.CreateWithMask( rMaskSalBmp, rTR.mnSrcX,
                                                         rTR.mnSrcY, rTR.mnSrcWidth,
                                                         rTR.mnSrcHeight );
    if( !xMaskedImage )
    {
        DBG_DRAW_OPERATION_EXIT_EARLY("drawAlphaBitmap");
        return false;
    }

    if ( CheckContext() )
    {
        const CGRect aDstRect = CGRectMake( rTR.mnDestX, rTR.mnDestY, rTR.mnDestWidth, rTR.mnDestHeight);
        SAL_INFO( "vcl.cg", "CGContextDrawImage(" << mrContext << "," << aDstRect << "," << xMaskedImage << ")" );
        CGContextDrawImage( mrContext, aDstRect, xMaskedImage );
        RefreshRect( aDstRect );
    }

    SAL_INFO( "vcl.cg", "CGImageRelease(" << xMaskedImage << ")");
    CGImageRelease(xMaskedImage);

    DBG_DRAW_OPERATION_EXIT("drawAlphaBitmap");
    return true;
}

bool AquaSalGraphics::drawTransformedBitmap(
    const basegfx::B2DPoint& rNull, const basegfx::B2DPoint& rX, const basegfx::B2DPoint& rY,
    const SalBitmap& rSrcBitmap, const SalBitmap* pAlphaBmp )
{
    DBG_DRAW_OPERATION("drawTransformedBitmap", true);

    if( !CheckContext() )
    {
        DBG_DRAW_OPERATION_EXIT_EARLY("drawTransformedBitmap");
        return true;
    }

    // get the Quartz image
    CGImageRef xImage = nullptr;
    const Size aSize = rSrcBitmap.GetSize();
    const QuartzSalBitmap& rSrcSalBmp = static_cast<const QuartzSalBitmap&>(rSrcBitmap);
    const QuartzSalBitmap* pMaskSalBmp = static_cast<const QuartzSalBitmap*>(pAlphaBmp);

    if( !pMaskSalBmp)
        xImage = rSrcSalBmp.CreateCroppedImage( 0, 0, (int)aSize.Width(), (int)aSize.Height() );
    else
        xImage = rSrcSalBmp.CreateWithMask( *pMaskSalBmp, 0, 0, (int)aSize.Width(), (int)aSize.Height() );
    if( !xImage )
    {
        DBG_DRAW_OPERATION_EXIT_EARLY("drawTransformedBitmap");
        return false;
    }

    // setup the image transformation
    // using the rNull,rX,rY points as destinations for the (0,0),(0,Width),(Height,0) source points
    SAL_INFO( "vcl.cg", "CGContextSaveGState(" << mrContext << ") " << ++mnContextStackDepth );
    CGContextSaveGState( mrContext );
    const basegfx::B2DVector aXRel = rX - rNull;
    const basegfx::B2DVector aYRel = rY - rNull;
    const CGAffineTransform aCGMat = CGAffineTransformMake(
        aXRel.getX()/aSize.Width(), aXRel.getY()/aSize.Width(),
        aYRel.getX()/aSize.Height(), aYRel.getY()/aSize.Height(),
        rNull.getX(), rNull.getY());

    SAL_INFO( "vcl.cg", "CGContextConcatCTM(" << mrContext << "," << aCGMat << ")" );
    CGContextConcatCTM( mrContext, aCGMat );

    // draw the transformed image
    const CGRect aSrcRect = CGRectMake(0, 0, aSize.Width(), aSize.Height());
    SAL_INFO( "vcl.cg", "CGContextDrawImage(" << mrContext << "," << aSrcRect << "," << xImage << ")" );
    CGContextDrawImage( mrContext, aSrcRect, xImage );

    SAL_INFO( "vcl.cg", "CGImageRelease(" << xImage << ")" );
    CGImageRelease( xImage );
    // restore the Quartz graphics state
    SAL_INFO( "vcl.cg", "CGContextRestoreGState(" << mrContext << ") " << mnContextStackDepth--);
    CGContextRestoreGState(mrContext);

    // mark the destination as painted
    const CGRect aDstRect = CGRectApplyAffineTransform( aSrcRect, aCGMat );
    RefreshRect( aDstRect );

    DBG_DRAW_OPERATION_EXIT("drawTransformedBitmap");
    return true;
}

bool AquaSalGraphics::drawAlphaRect( long nX, long nY, long nWidth,
                                     long nHeight, sal_uInt8 nTransparency )
{
    DBG_DRAW_OPERATION("drawAlphaRect", true);

    if( !CheckContext() )
    {
        DBG_DRAW_OPERATION_EXIT_EARLY("drawAlphaRect");
        return true;
    }

    // save the current state
    SAL_INFO( "vcl.cg", "CGContextSaveGState(" << mrContext << ") " << ++mnContextStackDepth );
    CGContextSaveGState( mrContext );
    SAL_INFO( "vcl.cg", "CGContextSetAlpha(" << mrContext << "," << (100-nTransparency) * (1.0/100) << ")" );
    CGContextSetAlpha( mrContext, (100-nTransparency) * (1.0/100) );

    CGRect aRect = CGRectMake(nX, nY, nWidth-1, nHeight-1);
    if( IsPenVisible() )
    {
        aRect.origin.x += 0.5;
        aRect.origin.y += 0.5;
    }

    SAL_INFO( "vcl.cg", "CGContextBeginPath(" << mrContext << ")" );
    CGContextBeginPath( mrContext );
    SAL_INFO( "vcl.cg", "CGContextAddRect(" << mrContext << "," << aRect << ")" );
    CGContextAddRect( mrContext, aRect );
    SAL_INFO( "vcl.cg", "CGContextDrawPath(" << mrContext << ",kCGPathFill)" );
    CGContextDrawPath( mrContext, kCGPathFill );

    // restore state
    SAL_INFO( "vcl.cg", "CGContextRestoreGState(" << mrContext << ") " << mnContextStackDepth--);
    CGContextRestoreGState(mrContext);
    RefreshRect( aRect );

    DBG_DRAW_OPERATION_EXIT("drawAlphaRect");
    return true;
}

void AquaSalGraphics::drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap )
{
    DBG_DRAW_OPERATION("drawBitmap",);

    if( !CheckContext() )
    {
        DBG_DRAW_OPERATION_EXIT_EARLY("drawBitmap");
        return;
    }

    const QuartzSalBitmap& rBitmap = static_cast<const QuartzSalBitmap&>(rSalBitmap);
    CGImageRef xImage = rBitmap.CreateCroppedImage( (int)rPosAry.mnSrcX, (int)rPosAry.mnSrcY,
                                                    (int)rPosAry.mnSrcWidth, (int)rPosAry.mnSrcHeight );
    if( !xImage )
    {
        DBG_DRAW_OPERATION_EXIT_EARLY("drawBitmap");
        return;
    }

    const CGRect aDstRect = CGRectMake(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth, rPosAry.mnDestHeight);
    SAL_INFO( "vcl.cg", "CGContextDrawImage(" << mrContext << "," << aDstRect << "," << xImage << ")" );
    CGContextDrawImage( mrContext, aDstRect, xImage );

    SAL_INFO( "vcl.cg", "CGImageRelease(" << xImage << ")" );
    CGImageRelease( xImage );
    RefreshRect( aDstRect );

    DBG_DRAW_OPERATION_EXIT("drawBitmap");
}

void AquaSalGraphics::drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap,
                                  const SalBitmap& rTransparentBitmap )
{
    DBG_DRAW_OPERATION("drawBitmap",);

    if( !CheckContext() )
    {
        DBG_DRAW_OPERATION_EXIT_EARLY("drawBitmap");
        return;
    }

    const QuartzSalBitmap& rBitmap = static_cast<const QuartzSalBitmap&>(rSalBitmap);
    const QuartzSalBitmap& rMask = static_cast<const QuartzSalBitmap&>(rTransparentBitmap);
    CGImageRef xMaskedImage( rBitmap.CreateWithMask( rMask, rPosAry.mnSrcX, rPosAry.mnSrcY,
                                                     rPosAry.mnSrcWidth, rPosAry.mnSrcHeight ) );
    if( !xMaskedImage )
    {
        DBG_DRAW_OPERATION_EXIT_EARLY("drawBitmap");
        return;
    }

    const CGRect aDstRect = CGRectMake(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth, rPosAry.mnDestHeight);
    SAL_INFO( "vcl.cg", "CGContextDrawImage(" << mrContext << "," << aDstRect << "," << xMaskedImage << ")" );
    CGContextDrawImage( mrContext, aDstRect, xMaskedImage );
    SAL_INFO( "vcl.cg", "CGImageRelease(" << xMaskedImage << ")" );
    CGImageRelease( xMaskedImage );
    RefreshRect( aDstRect );

    DBG_DRAW_OPERATION_EXIT("drawBitmap");
}

#ifndef IOS

bool AquaSalGraphics::drawEPS( long nX, long nY, long nWidth, long nHeight,
                                   void* pEpsData, sal_uLong nByteCount )
{
    // convert the raw data to an NSImageRef
    NSData* xNSData = [NSData dataWithBytes:pEpsData length:(int)nByteCount];
    NSImageRep* xEpsImage = [NSEPSImageRep imageRepWithData: xNSData];
    if( !xEpsImage )
    {
        return false;
    }
    // get the target context
    if( !CheckContext() )
    {
        return false;
    }
    // NOTE: flip drawing, else the nsimage would be drawn upside down
    SAL_INFO( "vcl.cg", "CGContextSaveGState(" << mrContext << ") " << ++mnContextStackDepth );
    CGContextSaveGState( mrContext );
//  CGContextTranslateCTM( mrContext, 0, +mnHeight );
    SAL_INFO( "vcl.cg", "CGContextScaleCTM(" << mrContext << ",+1,-1)" );
    CGContextScaleCTM( mrContext, +1, -1 );
    nY = /*mnHeight*/ - (nY + nHeight);

    // prepare the target context
    NSGraphicsContext* pOrigNSCtx = [NSGraphicsContext currentContext];
    [pOrigNSCtx retain];

    // create new context
    NSGraphicsContext* pDrawNSCtx = [NSGraphicsContext graphicsContextWithGraphicsPort: mrContext flipped: IsFlipped()];
    // set it, setCurrentContext also releases the prviously set one
    [NSGraphicsContext setCurrentContext: pDrawNSCtx];

    // draw the EPS
    const NSRect aDstRect = NSMakeRect( nX, nY, nWidth, nHeight);
    const BOOL bOK = [xEpsImage drawInRect: aDstRect];

    // restore the NSGraphicsContext
    [NSGraphicsContext setCurrentContext: pOrigNSCtx];
    [pOrigNSCtx release]; // restore the original retain count

    SAL_INFO( "vcl.cg", "CGContextRestoreGState(" << mrContext << ") " << mnContextStackDepth--);
    CGContextRestoreGState( mrContext );
    // mark the destination rectangle as updated
    RefreshRect( aDstRect );

    return bOK;
}

#endif

void AquaSalGraphics::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
    DBG_DRAW_OPERATION("drawLine",);

    if( nX1 == nX2 && nY1 == nY2 )
    {
        // #i109453# platform independent code expects at least one pixel to be drawn
        drawPixel( nX1, nY1 );

        DBG_DRAW_OPERATION_EXIT_EARLY("drawLine");
        return;
    }

    if( !CheckContext() )
    {
        DBG_DRAW_OPERATION_EXIT_EARLY("drawLine");
        return;
    }

    SAL_INFO( "vcl.cg", "CGContextBeginPath(" << mrContext << ")" );
    CGContextBeginPath( mrContext );
    SAL_INFO( "vcl.cg", "CGContextMoveToPoint(" << mrContext << "," << static_cast<float>(nX1)+0.5 << "," << static_cast<float>(nY1)+0.5 << ")" );
    CGContextMoveToPoint( mrContext, static_cast<float>(nX1)+0.5, static_cast<float>(nY1)+0.5 );
    SAL_INFO( "vcl.cg", "CGContextAddLineToPoint(" << mrContext << "," << static_cast<float>(nX2)+0.5 << "," << static_cast<float>(nY2)+0.5 << ")" );
    CGContextAddLineToPoint( mrContext, static_cast<float>(nX2)+0.5, static_cast<float>(nY2)+0.5 );
    SAL_INFO( "vcl.cg", "CGContextDrawPath(" << mrContext << ",kCGPathStroke)" );
    CGContextDrawPath( mrContext, kCGPathStroke );

    Rectangle aRefreshRect( nX1, nY1, nX2, nY2 );
    (void) aRefreshRect;
    // Is a call to RefreshRect( aRefreshRect ) missing here?

    DBG_DRAW_OPERATION_EXIT("drawLine");
}

void AquaSalGraphics::drawMask( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap, SalColor nMaskColor )
{
    DBG_DRAW_OPERATION("drawMask",);

    if( !CheckContext() )
    {
        DBG_DRAW_OPERATION_EXIT_EARLY("drawMask");
        return;
    }

    const QuartzSalBitmap& rBitmap = static_cast<const QuartzSalBitmap&>(rSalBitmap);
    CGImageRef xImage = rBitmap.CreateColorMask( rPosAry.mnSrcX, rPosAry.mnSrcY,
                                                 rPosAry.mnSrcWidth, rPosAry.mnSrcHeight,
                                                 nMaskColor );
    if( !xImage )
    {
        DBG_DRAW_OPERATION_EXIT_EARLY("drawMask");
        return;
    }

    const CGRect aDstRect = CGRectMake(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth, rPosAry.mnDestHeight);
    SAL_INFO( "vcl.cg", "CGContextDrawImage(" << mrContext << "," << aDstRect << "," << xImage << ")" );
    CGContextDrawImage( mrContext, aDstRect, xImage );
    SAL_INFO( "vcl.cg", "CGImageRelease(" << xImage << ")" );
    CGImageRelease( xImage );
    RefreshRect( aDstRect );

    DBG_DRAW_OPERATION_EXIT("drawMask");
}

void AquaSalGraphics::drawPixel( long nX, long nY )
{
    // draw pixel with current line color
    ImplDrawPixel( nX, nY, maLineColor );
}

void AquaSalGraphics::drawPixel( long nX, long nY, SalColor nSalColor )
{
    const RGBAColor aPixelColor( nSalColor );
    ImplDrawPixel( nX, nY, aPixelColor );
}

bool AquaSalGraphics::drawPolyLine( const basegfx::B2DPolygon& rPolyLine,
                                    double fTransparency,
                                    const basegfx::B2DVector& rLineWidths,
                                    basegfx::B2DLineJoin eLineJoin,
                                    css::drawing::LineCap eLineCap,
                                    double fMiterMinimumAngle)
{
    DBG_DRAW_OPERATION("drawPolyLine", true);

    // short circuit if there is nothing to do
    const int nPointCount = rPolyLine.count();
    if( nPointCount <= 0 )
    {
        DBG_DRAW_OPERATION_EXIT_EARLY("drawPolyLine");
        return true;
    }

#ifdef IOS
    if( !CheckContext() )
    {
        DBG_DRAW_OPERATION_EXIT_EARLY("drawPolyLine");
        return false;
    }
#endif

    // #i101491# Aqua does not support B2DLineJoin::NONE; return false to use
    // the fallback (own geometry preparation)
    // #i104886# linejoin-mode and thus the above only applies to "fat" lines
    if( (basegfx::B2DLineJoin::NONE == eLineJoin) &&
        (rLineWidths.getX() > 1.3) )
    {
        DBG_DRAW_OPERATION_EXIT_EARLY("drawPolyLine");
        return false;
    }

    // setup line attributes
    CGLineJoin aCGLineJoin = kCGLineJoinMiter;
    switch( eLineJoin )
    {
    case basegfx::B2DLineJoin::NONE: aCGLineJoin = /*TODO?*/kCGLineJoinMiter; break;
    case basegfx::B2DLineJoin::Bevel: aCGLineJoin = kCGLineJoinBevel; break;
    case basegfx::B2DLineJoin::Miter: aCGLineJoin = kCGLineJoinMiter; break;
    case basegfx::B2DLineJoin::Round: aCGLineJoin = kCGLineJoinRound; break;
    }
    // convert miter minimum angle to miter limit
    CGFloat fCGMiterLimit = 1.0 / sin(fMiterMinimumAngle / 2.0);
    // setup cap attribute
    CGLineCap aCGLineCap(kCGLineCapButt);

    switch(eLineCap)
    {
        default: // css::drawing::LineCap_BUTT:
        {
            aCGLineCap = kCGLineCapButt;
            break;
        }
        case css::drawing::LineCap_ROUND:
        {
            aCGLineCap = kCGLineCapRound;
            break;
        }
        case css::drawing::LineCap_SQUARE:
        {
            aCGLineCap = kCGLineCapSquare;
            break;
        }
    }

    // setup poly-polygon path
    CGMutablePathRef xPath = CGPathCreateMutable();
    SAL_INFO( "vcl.cg", "CGPathCreateMutable() = " << xPath );
    AddPolygonToPath( xPath, rPolyLine, rPolyLine.isClosed(), !getAntiAliasB2DDraw(), true );

    const CGRect aRefreshRect = CGPathGetBoundingBox( xPath );
    SAL_INFO( "vcl.cg", "CGPathGetBoundingBox(" << xPath << ") = " << aRefreshRect );
    // #i97317# workaround for Quartz having problems with drawing small polygons
    if( ! ((aRefreshRect.size.width <= 0.125) && (aRefreshRect.size.height <= 0.125)) )
    {
        // use the path to prepare the graphics context
        SAL_INFO( "vcl.cg", "CGContextSaveGState(" << mrContext << ") " << ++mnContextStackDepth );
        CGContextSaveGState( mrContext );
        SAL_INFO( "vcl.cg", "CGContextBeginPath(" << mrContext << ")" );
        CGContextBeginPath( mrContext );
        SAL_INFO( "vcl.cg", "CGContextAddPath(" << mrContext << "," << xPath << ")" );
        CGContextAddPath( mrContext, xPath );
        // draw path with antialiased line
        CGContextSetShouldAntialias( mrContext, true );
        SAL_INFO( "vcl.cg", "CGContextSetAlpha(" << mrContext << "," << 1.0 - fTransparency << ")" );
        CGContextSetAlpha( mrContext, 1.0 - fTransparency );
        CGContextSetLineJoin( mrContext, aCGLineJoin );
        CGContextSetLineCap( mrContext, aCGLineCap );
        CGContextSetLineWidth( mrContext, rLineWidths.getX() );
        CGContextSetMiterLimit(mrContext, fCGMiterLimit);
        SAL_INFO( "vcl.cg", "CGContextDrawPath(" << mrContext << ",kCGPathStroke)" );
        CGContextDrawPath( mrContext, kCGPathStroke );
        SAL_INFO( "vcl.cg", "CGContextRestoreGState(" << mrContext << ") " << mnContextStackDepth-- );
        CGContextRestoreGState( mrContext );

        // mark modified rectangle as updated
        RefreshRect( aRefreshRect );
    }

    SAL_INFO( "vcl.cg", "CGPathRelease(" << xPath << ")" );
    CGPathRelease( xPath );

    DBG_DRAW_OPERATION_EXIT("drawPolyLine");
    return true;
}

bool AquaSalGraphics::drawPolyLineBezier( sal_uInt32, const SalPoint*, const PolyFlags* )
{
    return false;
}

bool AquaSalGraphics::drawPolyPolygon( const basegfx::B2DPolyPolygon& rPolyPoly,
                                       double fTransparency )
{
    DBG_DRAW_OPERATION("drawPolyPolygon", true);

    // short circuit if there is nothing to do
    const int nPolyCount = rPolyPoly.count();
    if( nPolyCount <= 0 )
    {
        DBG_DRAW_OPERATION_EXIT_EARLY("drawPolyPolygon");
        return true;
    }

    // ignore invisible polygons
    if( (fTransparency >= 1.0) || (fTransparency < 0) )
    {
        DBG_DRAW_OPERATION_EXIT_EARLY("drawPolyPolygon");
        return true;
    }

    // setup poly-polygon path
    CGMutablePathRef xPath = CGPathCreateMutable();
    SAL_INFO( "vcl.cg", "CGPathCreateMutable() = " << xPath );
    for( int nPolyIdx = 0; nPolyIdx < nPolyCount; ++nPolyIdx )
    {
        const basegfx::B2DPolygon rPolygon = rPolyPoly.getB2DPolygon( nPolyIdx );
        AddPolygonToPath( xPath, rPolygon, true, !getAntiAliasB2DDraw(), IsPenVisible() );
    }

    const CGRect aRefreshRect = CGPathGetBoundingBox( xPath );
    SAL_INFO( "vcl.cg", "CGPathGetBoundingBox(" << xPath << ") = " << aRefreshRect );
    // #i97317# workaround for Quartz having problems with drawing small polygons
    if( ! ((aRefreshRect.size.width <= 0.125) && (aRefreshRect.size.height <= 0.125)) )
    {
        // prepare drawing mode
        CGPathDrawingMode eMode;
        if( IsBrushVisible() && IsPenVisible() )
        {
            eMode = kCGPathEOFillStroke;
        }
        else if( IsPenVisible() )
        {
            eMode = kCGPathStroke;
        }
        else if( IsBrushVisible() )
        {
            eMode = kCGPathEOFill;
        }
        else
        {
            SAL_WARN( "vcl.quartz", "Neither pen nor brush visible" );
            SAL_INFO( "vcl.cg", "CGPathRelease(" << xPath << ")" );
            CGPathRelease( xPath );
            DBG_DRAW_OPERATION_EXIT_EARLY("drawPolyPolygon");
            return true;
        }

        // use the path to prepare the graphics context
        SAL_INFO( "vcl.cg", "CGContextSaveGState(" << mrContext << ") " << ++mnContextStackDepth );
        CGContextSaveGState( mrContext );
        SAL_INFO( "vcl.cg", "CGContextBeginPath(" << mrContext << ")" );
        CGContextBeginPath( mrContext );
        SAL_INFO( "vcl.cg", "CGContextAddPath(" << mrContext << "," << xPath << ")" );
        CGContextAddPath( mrContext, xPath );

        // draw path with antialiased polygon
        CGContextSetShouldAntialias( mrContext, true );
        SAL_INFO( "vcl.cg", "CGContextSetAlpha(" << mrContext << "," << 1.0 - fTransparency << ")" );
        CGContextSetAlpha( mrContext, 1.0 - fTransparency );
        SAL_INFO( "vcl.cg", "CGContextDrawPath(" << mrContext << "," << eMode << ")" );
        CGContextDrawPath( mrContext, eMode );
        SAL_INFO( "vcl.cg", "CGContextRestoreGState(" << mrContext << ") " << mnContextStackDepth-- );
        CGContextRestoreGState( mrContext );

        // mark modified rectangle as updated
        RefreshRect( aRefreshRect );
    }

    SAL_INFO( "vcl.cg", "CGPathRelease(" << xPath << ")" );
    CGPathRelease( xPath );

    DBG_DRAW_OPERATION_EXIT("drawPolyPolygon");
    return true;
}

void AquaSalGraphics::drawPolyPolygon( sal_uInt32 nPolyCount, const sal_uInt32 *pPoints, PCONSTSALPOINT  *ppPtAry )
{
    DBG_DRAW_OPERATION("drawPolyPolygon",);

    if( nPolyCount <= 0 )
    {
        DBG_DRAW_OPERATION_EXIT_EARLY("drawPolyPolygon");
        return;
    }

    if( !CheckContext() )
    {
        DBG_DRAW_OPERATION_EXIT_EARLY("drawPolyPolygon");
        return;
    }

    // find bound rect
    long leftX = 0, topY = 0, maxWidth = 0, maxHeight = 0;
    getBoundRect( pPoints[0], ppPtAry[0], leftX, topY, maxWidth, maxHeight );

    for( sal_uInt32 n = 1; n < nPolyCount; n++ )
    {
        long nX = leftX, nY = topY, nW = maxWidth, nH = maxHeight;
        getBoundRect( pPoints[n], ppPtAry[n], nX, nY, nW, nH );
        if( nX < leftX )
        {
            maxWidth += leftX - nX;
            leftX = nX;
        }
        if( nY < topY )
        {
            maxHeight += topY - nY;
            topY = nY;
        }
        if( nX + nW > leftX + maxWidth )
        {
            maxWidth = nX + nW - leftX;
        }
        if( nY + nH > topY + maxHeight )
        {
            maxHeight = nY + nH - topY;
        }
    }

    // prepare drawing mode
    CGPathDrawingMode eMode;
    if( IsBrushVisible() && IsPenVisible() )
    {
        eMode = kCGPathEOFillStroke;
    }
    else if( IsPenVisible() )
    {
        eMode = kCGPathStroke;
    }
    else if( IsBrushVisible() )
    {
        eMode = kCGPathEOFill;
    }
    else
    {
        SAL_WARN( "vcl.quartz", "Neither pen nor brush visible" );
        DBG_DRAW_OPERATION_EXIT_EARLY("drawPolyPolygon");
        return;
    }

    // convert to CGPath
    SAL_INFO( "vcl.cg", "CGContextBeginPath(" << mrContext << ")" );
    CGContextBeginPath( mrContext );
    if( IsPenVisible() )
    {
        for( sal_uInt32 nPoly = 0; nPoly < nPolyCount; nPoly++ )
        {
            const sal_uInt32 nPoints = pPoints[nPoly];
            if( nPoints > 1 )
            {
                const SalPoint *pPtAry = ppPtAry[nPoly];
                float fX, fY;

                alignLinePoint( pPtAry, fX, fY );
                SAL_INFO( "vcl.cg", "CGContextMoveToPoint(" << mrContext << "," << fX << "," << fY << ")" );
                CGContextMoveToPoint( mrContext, fX, fY );
                pPtAry++;

                for( sal_uInt32 nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++ )
                {
                    alignLinePoint( pPtAry, fX, fY );
                    SAL_INFO( "vcl.cg", "CGContextAddLineToPoint(" << mrContext << "," << fX << "," << fY << ")" );
                    CGContextAddLineToPoint( mrContext, fX, fY );
                }
                SAL_INFO( "vcl.cg", "CGContextClosePath(" << mrContext << ")");
                CGContextClosePath(mrContext);
            }
        }
    }
    else
    {
        for( sal_uInt32 nPoly = 0; nPoly < nPolyCount; nPoly++ )
        {
            const sal_uInt32 nPoints = pPoints[nPoly];
            if( nPoints > 1 )
            {
                const SalPoint *pPtAry = ppPtAry[nPoly];
                SAL_INFO( "vcl.cg", "CGContextMoveToPoint(" << mrContext << "," << pPtAry->mnX << "," << pPtAry->mnY << ")" );
                CGContextMoveToPoint( mrContext, pPtAry->mnX, pPtAry->mnY );
                pPtAry++;
                for( sal_uInt32 nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++ )
                {
                    SAL_INFO( "vcl.cg", "CGContextAddLineToPoint(" << mrContext << "," << pPtAry->mnX << "," << pPtAry->mnY << ")" );
                    CGContextAddLineToPoint( mrContext, pPtAry->mnX, pPtAry->mnY );
                }
                SAL_INFO( "vcl.cg", "CGContextClosePath(" << mrContext << ")");
                CGContextClosePath(mrContext);
            }
        }
    }

    SAL_INFO( "vcl.cg", "CGContextDrawPath(" << mrContext << "," <<
              (eMode == kCGPathFill ? "kCGPathFill" :
               (eMode == kCGPathEOFill ? "kCGPathEOFill" :
                (eMode == kCGPathFillStroke ? "kCGPathFillStroke" :
                 (eMode == kCGPathEOFillStroke ? "kCGPathEOFillStroke" :
                  "???"))))
              << ")" );
    CGContextDrawPath( mrContext, eMode );

    RefreshRect( leftX, topY, maxWidth, maxHeight );

    DBG_DRAW_OPERATION_EXIT("drawPolyPolygon");
}

void AquaSalGraphics::drawPolygon( sal_uInt32 nPoints, const SalPoint *pPtAry )
{
    DBG_DRAW_OPERATION("drawPolygon",);

    if( nPoints <= 1 )
    {
        DBG_DRAW_OPERATION_EXIT_EARLY("drawPolygon");
        return;
    }

    if( !CheckContext() )
    {
        DBG_DRAW_OPERATION_EXIT_EARLY("drawPolygon");
        return;
    }

    long nX = 0, nY = 0, nWidth = 0, nHeight = 0;
    getBoundRect( nPoints, pPtAry, nX, nY, nWidth, nHeight );

    CGPathDrawingMode eMode;
    if( IsBrushVisible() && IsPenVisible() )
    {
        eMode = kCGPathEOFillStroke;
    }
    else if( IsPenVisible() )
    {
        eMode = kCGPathStroke;
    }
    else if( IsBrushVisible() )
    {
        eMode = kCGPathEOFill;
    }
    else
    {
        SAL_WARN( "vcl.quartz", "Neither pen nor brush visible" );
        return;
    }

    SAL_INFO( "vcl.cg", "CGContextBeginPath(" << mrContext << ")" );
    CGContextBeginPath( mrContext );

    if( IsPenVisible() )
    {
        float fX, fY;
        alignLinePoint( pPtAry, fX, fY );
        SAL_INFO( "vcl.cg", "CGContextMoveToPoint(" << mrContext << "," << fX << "," << fY << ")" );
        CGContextMoveToPoint( mrContext, fX, fY );
        pPtAry++;
        for( sal_uInt32 nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++ )
        {
            alignLinePoint( pPtAry, fX, fY );
            SAL_INFO( "vcl.cg", "CGContextAddLineToPoint(" << mrContext << "," << fX << "," << fY << ")" );
            CGContextAddLineToPoint( mrContext, fX, fY );
        }
    }
    else
    {
        SAL_INFO( "vcl.cg", "CGContextMoveToPoint(" << mrContext << "," << pPtAry->mnX << "," << pPtAry->mnY << ")" );
        CGContextMoveToPoint( mrContext, pPtAry->mnX, pPtAry->mnY );
        pPtAry++;
        for( sal_uInt32 nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++ )
        {
            SAL_INFO( "vcl.cg", "CGContextAddLineToPoint(" << mrContext << "," << pPtAry->mnX << "," << pPtAry->mnY << ")" );
            CGContextAddLineToPoint( mrContext, pPtAry->mnX, pPtAry->mnY );
        }
    }

    SAL_INFO( "vcl.cg", "CGContextClosePath(" << mrContext << ")");
    CGContextClosePath( mrContext );
    SAL_INFO( "vcl.cg", "CGContextDrawPath(" << mrContext << "," << eMode << ")" );
    CGContextDrawPath( mrContext, eMode );
    RefreshRect( nX, nY, nWidth, nHeight );

    DBG_DRAW_OPERATION_EXIT("drawPolygon");
}

bool AquaSalGraphics::drawPolygonBezier( sal_uInt32, const SalPoint*, const PolyFlags* )
{
    return false;
}

bool AquaSalGraphics::drawPolyPolygonBezier( sal_uInt32, const sal_uInt32*,
                                             const SalPoint* const*, const PolyFlags* const* )
{
    return false;
}

void AquaSalGraphics::drawRect( long nX, long nY, long nWidth, long nHeight )
{
    DBG_DRAW_OPERATION("drawRect",);

    if( !CheckContext() )
    {
        DBG_DRAW_OPERATION_EXIT_EARLY("drawRect");
        return;
    }

    CGRect aRect( CGRectMake(nX, nY, nWidth, nHeight) );
    if( IsPenVisible() )
    {
        aRect.origin.x      += 0.5;
        aRect.origin.y      += 0.5;
        aRect.size.width    -= 1;
        aRect.size.height -= 1;
    }

    if( IsBrushVisible() )
    {
        SAL_INFO( "vcl.cg", "CGContextFillRect(" << mrContext << "," << aRect << ")" );
        CGContextFillRect( mrContext, aRect );
    }
    if( IsPenVisible() )
    {
        SAL_INFO( "vcl.cg", "CGContextStrokeRect(" << mrContext << "," << aRect << ")" );
        CGContextStrokeRect( mrContext, aRect );
    }
    RefreshRect( nX, nY, nWidth, nHeight );

    DBG_DRAW_OPERATION_EXIT("drawRect");
}

void AquaSalGraphics::drawPolyLine( sal_uInt32 nPoints, const SalPoint *pPtAry )
{
    DBG_DRAW_OPERATION("drawPolyLine",);

    if( nPoints < 1 )
    {
        DBG_DRAW_OPERATION_EXIT_EARLY("drawPolyLine");
        return;
    }

    if( !CheckContext() )
    {
        DBG_DRAW_OPERATION_EXIT_EARLY("drawPolyLine");
        return;
    }

    long nX = 0, nY = 0, nWidth = 0, nHeight = 0;
    getBoundRect( nPoints, pPtAry, nX, nY, nWidth, nHeight );

    float fX, fY;
    SAL_INFO( "vcl.cg", "CGContextBeginPath(" << mrContext << ")" );
    CGContextBeginPath( mrContext );
    alignLinePoint( pPtAry, fX, fY );
    SAL_INFO( "vcl.cg", "CGContextMoveToPoint(" << mrContext << "," << fX << "," << fY << ")" );
    CGContextMoveToPoint( mrContext, fX, fY );
    pPtAry++;

    for( sal_uInt32 nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++ )
    {
        alignLinePoint( pPtAry, fX, fY );
        SAL_INFO( "vcl.cg", "CGContextAddLineToPoint(" << mrContext << "," << fX << "," << fY << ")" );
        CGContextAddLineToPoint( mrContext, fX, fY );
    }
    SAL_INFO( "vcl.cg", "CGContextStrokePath(" << mrContext << ")" );
    CGContextStrokePath(mrContext);

    RefreshRect( nX, nY, nWidth, nHeight );

    DBG_DRAW_OPERATION_EXIT("drawPolyLine");
}

sal_uInt16 AquaSalGraphics::GetBitCount() const
{
    sal_uInt16 nBits = mnBitmapDepth ? mnBitmapDepth : 32;//24;
    return nBits;
}

SalBitmap* AquaSalGraphics::getBitmap( long  nX, long  nY, long  nDX, long  nDY )
{
    SAL_WARN_IF( !mxLayer, "vcl.quartz", "AquaSalGraphics::getBitmap() with no layer this=" << this );

    ApplyXorContext();

    QuartzSalBitmap* pBitmap = new QuartzSalBitmap;
    if( !pBitmap->Create( mxLayer, mnBitmapDepth, nX, nY, nDX, nDY, IsFlipped()) )
    {
        delete pBitmap;
        pBitmap = nullptr;
    }
    return pBitmap;
}

SystemGraphicsData AquaSalGraphics::GetGraphicsData() const
{
    SystemGraphicsData aRes;
    aRes.nSize = sizeof(aRes);
    aRes.rCGContext = mrContext;
    return aRes;
}

long AquaSalGraphics::GetGraphicsWidth() const
{
    long w = 0;
    if( mrContext && (
#ifndef IOS
                      mbWindow ||
#endif
                      mbVirDev) )
    {
        w = mnWidth;
    }

#ifndef IOS
    if( w == 0 )
    {
        if( mbWindow && mpFrame )
        {
            w = mpFrame->maGeometry.nWidth;
        }
    }
#endif
    return w;
}

SalColor AquaSalGraphics::getPixel( long nX, long nY )
{
    // return default value on printers or when out of bounds
    if( !mxLayer || (nX < 0) || (nX >= mnWidth) ||
        (nY < 0) || (nY >= mnHeight))
    {
        return COL_BLACK;
    }
    // prepare creation of matching a CGBitmapContext
#if defined OSL_BIGENDIAN
    struct{ unsigned char b, g, r, a; } aPixel;
#else
    struct{ unsigned char a, r, g, b; } aPixel;
#endif

    // create a one-pixel bitmap context
    // TODO: is it worth to cache it?
    CGContextRef xOnePixelContext =
        CGBitmapContextCreate( &aPixel, 1, 1, 8, 32,
                               GetSalData()->mxRGBSpace,
                               kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Big );

    SAL_INFO( "vcl.cg", "CGBitmapContextCreate(1x1x8) = " << xOnePixelContext );

    // update this graphics layer
    ApplyXorContext();

    // copy the requested pixel into the bitmap context
    if( IsFlipped() )
    {
        nY = mnHeight - nY;
    }
    const CGPoint aCGPoint = CGPointMake(-nX, -nY);
    SAL_INFO( "vcl.cg", "CGContextDrawLayerAtPoint(" <<
              xOnePixelContext << "," << aCGPoint << "," << mxLayer << ")" );
    CGContextDrawLayerAtPoint( xOnePixelContext, aCGPoint, mxLayer );

    SAL_INFO( "vcl.cg", "CGContextRelease(" << xOnePixelContext << ")" );
    CGContextRelease( xOnePixelContext );

    SalColor nSalColor = MAKE_SALCOLOR( aPixel.r, aPixel.g, aPixel.b );
    return nSalColor;
}

void AquaSalGraphics::GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY )
{
#ifndef IOS
    if( !mnRealDPIY )
    {
        initResolution( (mbWindow && mpFrame) ? mpFrame->getNSWindow() : nil );
    }

    rDPIX = mnRealDPIX;
    rDPIY = mnRealDPIY;
#else
    rDPIX = rDPIY = 200; // FIXME
#endif
}

void AquaSalGraphics::ImplDrawPixel( long nX, long nY, const RGBAColor& rColor )
{
    if( !CheckContext() )
    {
        return;
    }
    // overwrite the fill color
    SAL_INFO( "vcl.cg", "CGContextSetFillColor(" << mrContext << "," << rColor << ")" );
    CGContextSetFillColor( mrContext, rColor.AsArray() );
    // draw 1x1 rect, there is no pixel drawing in Quartz
    const CGRect aDstRect = CGRectMake(nX, nY, 1, 1);
    SAL_INFO( "vcl.cg", "CGContextFillRect(" << mrContext << "," << aDstRect << ")" );
    CGContextFillRect( mrContext, aDstRect );
    RefreshRect( aDstRect );
    // reset the fill color
    SAL_INFO( "vcl.cg", "CGContextSetFillColor(" << mrContext << "," << maFillColor << ")" );
    CGContextSetFillColor( mrContext, maFillColor.AsArray() );
}

#ifndef IOS

void AquaSalGraphics::initResolution( NSWindow* )
{
    // #i100617# read DPI only once; there is some kind of weird caching going on
    // if the main screen changes
    // FIXME: this is really unfortunate and needs to be investigated

    SalData* pSalData = GetSalData();
    if( pSalData->mnDPIX == 0 || pSalData->mnDPIY == 0 )
    {
        NSScreen* pScreen = nil;

        /* #i91301#
        many woes went into the try to have different resolutions
        on different screens. The result of these trials is that OOo is not ready
        for that yet, vcl and applications would need to be adapted.

        Unfortunately this is not possible in the 3.0 timeframe.
        So let's stay with one resolution for all Windows and VirtualDevices
        which is the resolution of the main screen

        This of course also means that measurements are exact only on the main screen.
        For activating different resolutions again just comment out the two lines below.

        if( pWin )
        pScreen = [pWin screen];
        */
        if( pScreen == nil )
        {
            NSArray* pScreens = [NSScreen screens];
            if( pScreens && [pScreens count] > 0)
            {
                pScreen = [pScreens objectAtIndex: 0];
            }
        }

        mnRealDPIX = mnRealDPIY = 96;
        if( pScreen )
        {
            NSDictionary* pDev = [pScreen deviceDescription];
            if( pDev )
            {
                NSNumber* pVal = [pDev objectForKey: @"NSScreenNumber"];
                if( pVal )
                {
                    // FIXME: casting a long to CGDirectDisplayID is evil, but
                    // Apple suggest to do it this way
                    const CGDirectDisplayID nDisplayID = (CGDirectDisplayID)[pVal longValue];
                    const CGSize aSize = CGDisplayScreenSize( nDisplayID ); // => result is in millimeters
                    mnRealDPIX = static_cast<long>((CGDisplayPixelsWide( nDisplayID ) * 25.4) / aSize.width);
                    mnRealDPIY = static_cast<long>((CGDisplayPixelsHigh( nDisplayID ) * 25.4) / aSize.height);
                }
                else
                {
                    OSL_FAIL( "no resolution found in device description" );
                }
            }
            else
            {
                OSL_FAIL( "no device description" );
            }
        }
        else
        {
            OSL_FAIL( "no screen found" );
        }

        // #i107076# maintaining size-WYSIWYG-ness causes many problems for
        //           low-DPI, high-DPI or for mis-reporting devices
        //           => it is better to limit the calculation result then
        static const int nMinDPI = 72;
        if( (mnRealDPIX < nMinDPI) || (mnRealDPIY < nMinDPI) )
        {
            mnRealDPIX = mnRealDPIY = nMinDPI;
        }
        // Note that on a Retina display, the "mnRealDPIX" as
        // calculated above is not the true resolution of the display,
        // but the "logical" one, or whatever the correct terminology
        // is. (For instance on a 5K 27in iMac, it's 108.)  So at
        // least currently, it won't be over 200. I don't know whether
        // this test is a "sanity check", or whether there is some
        // real reason to limit this to 200.
        static const int nMaxDPI = 200;
        if( (mnRealDPIX > nMaxDPI) || (mnRealDPIY > nMaxDPI) )
        {
            mnRealDPIX = mnRealDPIY = nMaxDPI;
        }
        // for OSX any anisotropy reported for the display resolution is best ignored (e.g. TripleHead2Go)
        mnRealDPIX = mnRealDPIY = (mnRealDPIX + mnRealDPIY + 1) / 2;

        pSalData->mnDPIX = mnRealDPIX;
        pSalData->mnDPIY = mnRealDPIY;
    }
    else
    {
        mnRealDPIX = pSalData->mnDPIX;
        mnRealDPIY = pSalData->mnDPIY;
    }
}

#endif

void AquaSalGraphics::invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags )
{
    if ( CheckContext() )
    {
        CGRect aCGRect = CGRectMake( nX, nY, nWidth, nHeight);
        SAL_INFO( "vcl.cg", "CGContextSaveGState(" << mrContext << ") " << ++mnContextStackDepth);
        CGContextSaveGState(mrContext);

        if ( nFlags & SalInvert::TrackFrame )
        {
            const CGFloat dashLengths[2]  = { 4.0, 4.0 };     // for drawing dashed line
            CGContextSetBlendMode( mrContext, kCGBlendModeDifference );
            SAL_INFO( "vcl.cg", "CGContextSetRGBStrokeColor(" << mrContext << ",{1,1,1,1})" );
            CGContextSetRGBStrokeColor ( mrContext, 1.0, 1.0, 1.0, 1.0 );
            CGContextSetLineDash ( mrContext, 0, dashLengths, 2 );
            CGContextSetLineWidth( mrContext, 2.0);
            SAL_INFO( "vcl.cg", "CGContextStrokeRect(" << mrContext << "," << aCGRect << ")" );
            CGContextStrokeRect ( mrContext, aCGRect );
        }
        else if ( nFlags & SalInvert::N50 )
        {
            //CGContextSetAllowsAntialiasing( mrContext, false );
            CGContextSetBlendMode(mrContext, kCGBlendModeDifference);
            CGContextAddRect( mrContext, aCGRect );
            Pattern50Fill();
        }
        else // just invert
        {
            CGContextSetBlendMode(mrContext, kCGBlendModeDifference);
            SAL_INFO( "vcl.cg", "CGContextSetRGBFillColor(" << mrContext << ",{1,1,1,1})" );
            CGContextSetRGBFillColor ( mrContext,1.0, 1.0, 1.0 , 1.0 );
            SAL_INFO( "vcl.cg", "CGContextFillRect(" << mrContext << "," << aCGRect << ")" );
            CGContextFillRect ( mrContext, aCGRect );
        }
        SAL_INFO( "vcl.cg", "CGContextRestoreGState(" << mrContext << ") " << mnContextStackDepth-- );
        CGContextRestoreGState( mrContext);
        RefreshRect( aCGRect );
    }
}

namespace {

CGPoint* makeCGptArray(sal_uInt32 nPoints, const SalPoint* pPtAry)
{
    CGPoint *CGpoints = new CGPoint[nPoints];
    if ( CGpoints )
    {
        for(sal_uLong i=0;i<nPoints;i++)
        {
            CGpoints[i].x = pPtAry[i].mnX;
            CGpoints[i].y = pPtAry[i].mnY;
        }
    }
    return CGpoints;
}

}

void AquaSalGraphics::invert( sal_uInt32 nPoints, const SalPoint*  pPtAry, SalInvert nSalFlags )
{
    if ( CheckContext() )
    {
        SAL_INFO( "vcl.cg", "CGContextSaveGState(" << mrContext << ") " << ++mnContextStackDepth);
        CGContextSaveGState(mrContext);
        CGPoint* CGpoints = makeCGptArray(nPoints,pPtAry);
        CGContextAddLines ( mrContext, CGpoints, nPoints );
        if ( nSalFlags & SalInvert::TrackFrame )
        {
            const CGFloat dashLengths[2]  = { 4.0, 4.0 };     // for drawing dashed line
            CGContextSetBlendMode( mrContext, kCGBlendModeDifference );
            SAL_INFO( "vcl.cg", "CGContextSetRGBStrokeColor(" << mrContext << ",{1,1,1,1})" );
            CGContextSetRGBStrokeColor ( mrContext, 1.0, 1.0, 1.0, 1.0 );
            CGContextSetLineDash ( mrContext, 0, dashLengths, 2 );
            CGContextSetLineWidth( mrContext, 2.0);
            SAL_INFO( "vcl.cg", "CGContextStrokePath(" << mrContext << ")" );
            CGContextStrokePath ( mrContext );
        }
        else if ( nSalFlags & SalInvert::N50 )
        {
            CGContextSetBlendMode(mrContext, kCGBlendModeDifference);
            Pattern50Fill();
        }
        else // just invert
        {
            CGContextSetBlendMode( mrContext, kCGBlendModeDifference );
            SAL_INFO( "vcl.cg", "CGContextSetRGBFillColor(" << mrContext << ",{1,1,1,1})" );
            CGContextSetRGBFillColor( mrContext, 1.0, 1.0, 1.0, 1.0 );
            SAL_INFO( "vcl.cg", "CGContextFillPath(" << mrContext << ")" );
            CGContextFillPath( mrContext );
        }
        const CGRect aRefreshRect = CGContextGetClipBoundingBox(mrContext);
        SAL_INFO( "vcl.cg", "CGContextRestoreGState(" << mrContext << ") " << mnContextStackDepth-- );
        CGContextRestoreGState( mrContext);
        delete []  CGpoints;
        RefreshRect( aRefreshRect );
    }
}

void AquaSalGraphics::Pattern50Fill()
{
    static const CGFloat aFillCol[4] = { 1,1,1,1 };
    static const CGPatternCallbacks aCallback = { 0, &DrawPattern50, nullptr };
    static const CGColorSpaceRef mxP50Space = CGColorSpaceCreatePattern( GetSalData()->mxRGBSpace );
    static const CGPatternRef mxP50Pattern = CGPatternCreate( nullptr, CGRectMake( 0, 0, 4, 4 ),
                                                              CGAffineTransformIdentity, 4, 4,
                                                              kCGPatternTilingConstantSpacing,
                                                              false, &aCallback );
    SAL_WARN_IF( !mrContext, "vcl.quartz", "mrContext is NULL" );
    SAL_INFO( "vcl.cg", "CGContextSetFillColorSpace(" << mrContext << "," << mxP50Space << ")" );
    CGContextSetFillColorSpace( mrContext, mxP50Space );
    SAL_INFO( "vcl.cg", "CGContextSetFillPattern(" << mrContext << "," << mxP50Pattern << ",{1,1,1,1})" );
    CGContextSetFillPattern( mrContext, mxP50Pattern, aFillCol );
    SAL_INFO( "vcl.cg", "CGContextFillPath(" << mrContext << ")" );
    CGContextFillPath( mrContext );
}

void AquaSalGraphics::ResetClipRegion()
{
    // release old path and indicate no clipping
    if( mxClipPath )
    {
        SAL_INFO( "vcl.cg", "CGPathRelease(" << mxClipPath << ")" );
        CGPathRelease( mxClipPath );
        mxClipPath = nullptr;
    }
    if( CheckContext() )
    {
        SetState();
    }
}

void AquaSalGraphics::SetState()
{
    SAL_INFO( "vcl.cg", "CGContextRestoreGState(" << mrContext << ") " << mnContextStackDepth--);
    CGContextRestoreGState( mrContext );
    SAL_INFO( "vcl.cg", "CGContextSaveGState(" << mrContext << ") " << ++mnContextStackDepth );
    CGContextSaveGState( mrContext );

    // setup clipping
    if( mxClipPath )
    {
        SAL_INFO( "vcl.cg", "CGContextBeginPath(" << mrContext << ")" );
        CGContextBeginPath( mrContext );            // discard any existing path
        SAL_INFO( "vcl.cg", "CGContextAddPath(" << mrContext << "," << mxClipPath << ")" );
        CGContextAddPath( mrContext, mxClipPath );  // set the current path to the clipping path
        SAL_INFO( "vcl.cg", "CGContextClip(" << mrContext << ")" );
        CGContextClip( mrContext );                 // use it for clipping
    }

    // set RGB colorspace and line and fill colors
    SAL_INFO( "vcl.cg", "CGContextSetFillColor(" << mrContext << "," << maFillColor << ")" );
    CGContextSetFillColor( mrContext, maFillColor.AsArray() );

    SAL_INFO( "vcl.cg", "CGContextSetStrokeColor(" << mrContext << "," << maLineColor << ")" );
    CGContextSetStrokeColor( mrContext, maLineColor.AsArray() );
    CGContextSetShouldAntialias( mrContext, false );
    if( mnXorMode == 2 )
    {
        CGContextSetBlendMode( mrContext, kCGBlendModeDifference );
    }
}

void AquaSalGraphics::SetLineColor()
{
    maLineColor.SetAlpha( 0.0 );   // transparent
    if( CheckContext() )
    {
        SAL_INFO( "vcl.cg", "CGContextSetRGBStrokeColor(" << mrContext << "," << maLineColor << ")" );
        CGContextSetRGBStrokeColor( mrContext, maLineColor.GetRed(), maLineColor.GetGreen(),
                                    maLineColor.GetBlue(), maLineColor.GetAlpha() );
    }
}

void AquaSalGraphics::SetLineColor( SalColor nSalColor )
{
    maLineColor = RGBAColor( nSalColor );
    if( CheckContext() )
    {
        SAL_INFO( "vcl.cg", "CGContextSetRGBStrokeColor(" << mrContext << "," << maLineColor << ")" );
        CGContextSetRGBStrokeColor( mrContext, maLineColor.GetRed(), maLineColor.GetGreen(),
                                   maLineColor.GetBlue(), maLineColor.GetAlpha() );
    }
}

void AquaSalGraphics::SetFillColor()
{
    maFillColor.SetAlpha( 0.0 );   // transparent
    if( CheckContext() )
    {
        SAL_INFO( "vcl.cg", "CGContextSetRGBFillColor(" << mrContext << "," << maFillColor << ")" );
        CGContextSetRGBFillColor( mrContext, maFillColor.GetRed(), maFillColor.GetGreen(),
                                  maFillColor.GetBlue(), maFillColor.GetAlpha() );
    }
}

void AquaSalGraphics::SetFillColor( SalColor nSalColor )
{
    maFillColor = RGBAColor( nSalColor );
    if( CheckContext() )
    {
        SAL_INFO( "vcl.cg", "CGContextSetRGBFillColor(" << mrContext << "," << maFillColor << ")" );
        CGContextSetRGBFillColor( mrContext, maFillColor.GetRed(), maFillColor.GetGreen(),
                                 maFillColor.GetBlue(), maFillColor.GetAlpha() );
    }
}

bool AquaSalGraphics::supportsOperation( OutDevSupportType eType ) const
{
    bool bRet = false;
    switch( eType )
    {
    case OutDevSupportType::TransparentRect:
    case OutDevSupportType::B2DDraw:
        bRet = true;
        break;
    default:
        break;
    }
    return bRet;
}

bool AquaSalGraphics::setClipRegion( const vcl::Region& i_rClip )
{
    // release old clip path
    if( mxClipPath )
    {
        SAL_INFO( "vcl.cg", "CGPathRelease(" << mxClipPath << ")" );
        CGPathRelease( mxClipPath );
        mxClipPath = nullptr;
    }
    mxClipPath = CGPathCreateMutable();
    SAL_INFO( "vcl.cg", "CGPathCreateMutable() = " << mxClipPath );

    // set current path, either as polypolgon or sequence of rectangles
    if(i_rClip.HasPolyPolygonOrB2DPolyPolygon())
    {
        const basegfx::B2DPolyPolygon aClip(i_rClip.GetAsB2DPolyPolygon());

        AddPolyPolygonToPath( mxClipPath, aClip, !getAntiAliasB2DDraw(), false );
    }
    else
    {
        RectangleVector aRectangles;
        i_rClip.GetRegionRectangles(aRectangles);

        for(RectangleVector::const_iterator aRectIter(aRectangles.begin());
            aRectIter != aRectangles.end(); ++aRectIter)
        {
            const long nW(aRectIter->Right() - aRectIter->Left() + 1); // uses +1 logic in original

            if(nW)
            {
                const long nH(aRectIter->Bottom() - aRectIter->Top() + 1); // uses +1 logic in original

                if(nH)
                {
                    const CGRect aRect = CGRectMake( aRectIter->Left(), aRectIter->Top(), nW, nH);
                    SAL_INFO( "vcl.cg", "CGPathAddRect(" << mxClipPath << ",NULL," << aRect << ")" );
                    CGPathAddRect( mxClipPath, nullptr, aRect );
                }
            }
        }
    }
    // set the current path as clip region
    if( CheckContext() )
    {
        SetState();
    }
    return true;
}

void AquaSalGraphics::SetROPFillColor( SalROPColor nROPColor )
{
    if( ! mbPrinter )
    {
        SetFillColor( ImplGetROPSalColor( nROPColor ) );
    }
}

void AquaSalGraphics::SetROPLineColor( SalROPColor nROPColor )
{
    if( ! mbPrinter )
    {
        SetLineColor( ImplGetROPSalColor( nROPColor ) );
    }
}

void AquaSalGraphics::SetXORMode( bool bSet )
{
    // return early if XOR mode remains unchanged
    if( mbPrinter )
    {
        return;
    }
    if( ! bSet && mnXorMode == 2 )
    {
        CGContextSetBlendMode( mrContext, kCGBlendModeNormal );
        mnXorMode = 0;
        return;
    }
    else if( bSet && mnXorMode == 0)
    {
        CGContextSetBlendMode( mrContext, kCGBlendModeDifference );
        mnXorMode = 2;
        return;
    }

    if( (mpXorEmulation == nullptr) && !bSet )
    {
        return;
    }
    if( (mpXorEmulation != nullptr) && (bSet == mpXorEmulation->IsEnabled()) )
    {
        return;
    }
    if( !CheckContext() )
    {
         return;
    }
    // prepare XOR emulation
    if( !mpXorEmulation )
    {
        mpXorEmulation = new XorEmulation();
        mpXorEmulation->SetTarget( mnWidth, mnHeight, mnBitmapDepth, mrContext, mxLayer );
    }

    // change the XOR mode
    if( bSet )
    {
        mpXorEmulation->Enable();
        mrContext = mpXorEmulation->GetMaskContext();
        mnXorMode = 1;
    }
    else
    {
        mpXorEmulation->UpdateTarget();
        mpXorEmulation->Disable();
        mrContext = mpXorEmulation->GetTargetContext();
        mnXorMode = 0;
    }
}

#ifndef IOS

void AquaSalGraphics::updateResolution()
{
    SAL_WARN_IF( !mbWindow, "vcl", "updateResolution on inappropriate graphics" );

    initResolution( (mbWindow && mpFrame) ? mpFrame->getNSWindow() : nil );
}

#endif

XorEmulation::XorEmulation()
  : m_xTargetLayer( nullptr )
  , m_xTargetContext( nullptr )
  , m_xMaskContext( nullptr )
  , m_xTempContext( nullptr )
  , m_pMaskBuffer( nullptr )
  , m_pTempBuffer( nullptr )
  , m_nBufferLongs( 0 )
  , m_bIsEnabled( false )
{
    SAL_INFO( "vcl.quartz", "XorEmulation::XorEmulation() this=" << this );
}

XorEmulation::~XorEmulation()
{
    SAL_INFO( "vcl.quartz", "XorEmulation::~XorEmulation() this=" << this );
    Disable();
    SetTarget( 0, 0, 0, nullptr, nullptr );
}

void XorEmulation::SetTarget( int nWidth, int nHeight, int nTargetDepth,
                              CGContextRef xTargetContext, CGLayerRef xTargetLayer )
{
    SAL_INFO( "vcl.quartz", "XorEmulation::SetTarget() this=" << this <<
              " (" << nWidth << "x" << nHeight << ") depth=" << nTargetDepth <<
              " context=" << xTargetContext << " layer=" << xTargetLayer );

    // prepare to replace old mask+temp context
    if( m_xMaskContext )
    {
        // cleanup the mask context
        SAL_INFO( "vcl.cg", "CGContextRelease(" << m_xMaskContext << ")" );
        CGContextRelease( m_xMaskContext );
        delete[] m_pMaskBuffer;
        m_xMaskContext = nullptr;
        m_pMaskBuffer = nullptr;

        // cleanup the temp context if needed
        if( m_xTempContext )
        {
            SAL_INFO( "vcl.cg", "CGContextRelease(" << m_xTempContext << ")" );
            CGContextRelease( m_xTempContext );
            delete[] m_pTempBuffer;
            m_xTempContext = nullptr;
            m_pTempBuffer = nullptr;
        }
    }

    // return early if there is nothing more to do
    if( !xTargetContext )
    {
        return;
    }
    // retarget drawing operations to the XOR mask
    m_xTargetLayer = xTargetLayer;
    m_xTargetContext = xTargetContext;

    // prepare creation of matching CGBitmaps
    CGColorSpaceRef aCGColorSpace = GetSalData()->mxRGBSpace;
    CGBitmapInfo aCGBmpInfo = kCGImageAlphaNoneSkipFirst;
    int nBitDepth = nTargetDepth;
    if( !nBitDepth )
    {
        nBitDepth = 32;
    }
    int nBytesPerRow = (nBitDepth == 16) ? 2 : 4;
    const size_t nBitsPerComponent = (nBitDepth == 16) ? 5 : 8;
    if( nBitDepth <= 8 )
    {
        aCGColorSpace = GetSalData()->mxGraySpace;
        aCGBmpInfo = kCGImageAlphaNone;
        nBytesPerRow = 1;
    }
    nBytesPerRow *= nWidth;
    m_nBufferLongs = (nHeight * nBytesPerRow + sizeof(sal_uLong)-1) / sizeof(sal_uLong);

    // create a XorMask context
    m_pMaskBuffer = new sal_uLong[ m_nBufferLongs ];
    m_xMaskContext = CGBitmapContextCreate( m_pMaskBuffer,
                                            nWidth, nHeight,
                                            nBitsPerComponent, nBytesPerRow,
                                            aCGColorSpace, aCGBmpInfo );
    SAL_WARN_IF( !m_xMaskContext, "vcl.quartz", "mask context creation failed" );
    SAL_INFO( "vcl.cg", "CGBitmapContextCreate(" << nWidth << "x" << nHeight << ") = " << m_xMaskContext );

    // reset the XOR mask to black
    memset( m_pMaskBuffer, 0, m_nBufferLongs * sizeof(sal_uLong) );

    // a bitmap context will be needed for manual XORing
    // create one unless the target context is a bitmap context
    if( nTargetDepth )
    {
        m_pTempBuffer = static_cast<sal_uLong*>(CGBitmapContextGetData( m_xTargetContext ));
    }
    if( !m_pTempBuffer )
    {
        // create a bitmap context matching to the target context
        m_pTempBuffer = new sal_uLong[ m_nBufferLongs ];
        m_xTempContext = CGBitmapContextCreate( m_pTempBuffer,
                                                nWidth, nHeight,
                                                nBitsPerComponent, nBytesPerRow,
                                                aCGColorSpace, aCGBmpInfo );
        SAL_WARN_IF( !m_xTempContext, "vcl.quartz", "temp context creation failed" );
        SAL_INFO( "vcl.cg", "CGBitmapContextCreate(" << nWidth << "x" << nHeight << ") = " << m_xTempContext );
    }

    // initialize XOR mask context for drawing
    CGContextSetFillColorSpace( m_xMaskContext, aCGColorSpace );
    CGContextSetStrokeColorSpace( m_xMaskContext, aCGColorSpace );
    CGContextSetShouldAntialias( m_xMaskContext, false );

    // improve the XorMask's XOR emulation a litte
    // NOTE: currently only enabled for monochrome contexts
    if( aCGColorSpace == GetSalData()->mxGraySpace )
    {
        CGContextSetBlendMode( m_xMaskContext, kCGBlendModeDifference );
    }
    // intialize the transformation matrix to the drawing target
    const CGAffineTransform aCTM = CGContextGetCTM( xTargetContext );
    CGContextConcatCTM( m_xMaskContext, aCTM );
    if( m_xTempContext )
    {
        CGContextConcatCTM( m_xTempContext, aCTM );
    }
    // initialize the default XorMask graphics state
    CGContextSaveGState( m_xMaskContext );
}

bool XorEmulation::UpdateTarget()
{
    SAL_INFO( "vcl.quartz", "XorEmulation::UpdateTarget() this=" << this );

    if( !IsEnabled() )
    {
        return false;
    }
    // update the temp bitmap buffer if needed
    if( m_xTempContext )
    {
        SAL_WARN_IF( m_xTargetContext == nullptr, "vcl.quartz", "Target layer is NULL");
        SAL_INFO( "vcl.cg", "CGContextDrawLayerAtPoint(" << m_xTempContext << "," << CGPointZero << "," << m_xTargetLayer << ")" );
        CGContextDrawLayerAtPoint( m_xTempContext, CGPointZero, m_xTargetLayer );
    }
    // do a manual XOR with the XorMask
    // this approach suffices for simple color manipulations
    // and also the complex-clipping-XOR-trick used in metafiles
    const sal_uLong* pSrc = m_pMaskBuffer;
    sal_uLong* pDst = m_pTempBuffer;
    for( int i = m_nBufferLongs; --i >= 0;)
    {
        *(pDst++) ^= *(pSrc++);
    }
    // write back the XOR results to the target context
    if( m_xTempContext )
    {
        CGImageRef xXorImage = CGBitmapContextCreateImage( m_xTempContext );
        SAL_INFO( "vcl.cg", "CGBitmapContextCreateImage(" << m_xTempContext << ") = " << xXorImage );
        const int nWidth  = (int)CGImageGetWidth( xXorImage );
        const int nHeight = (int)CGImageGetHeight( xXorImage );
        // TODO: update minimal changerect
        const CGRect aFullRect = CGRectMake(0, 0, nWidth, nHeight);
        SAL_INFO( "vcl.cg", "CGContextDrawImage(" << m_xTargetContext << "," << aFullRect << "," << xXorImage << ")" );
        CGContextDrawImage( m_xTargetContext, aFullRect, xXorImage );
        SAL_INFO( "vcl.cg", "CGImageRelease(" << xXorImage << ")" );
        CGImageRelease( xXorImage );
    }

    // reset the XorMask to black again
    // TODO: not needed for last update
    memset( m_pMaskBuffer, 0, m_nBufferLongs * sizeof(sal_uLong) );

    // TODO: return FALSE if target was not changed
    return true;
}

void AquaSalGraphics::SetVirDevGraphics( CGLayerRef xLayer, CGContextRef xContext,
    int nBitmapDepth )
{
    SAL_INFO( "vcl.quartz", "SetVirDevGraphics() this=" << this << " layer=" << xLayer << " context=" << xContext );

#ifndef IOS
    mbWindow = false;
#endif
    mbPrinter = false;
    mbVirDev = true;

#ifdef IOS
    (void) nBitmapDepth;

    if( !xContext )
    {
        // We will return early a few lines lower.
        // Undo the "stack initialization" done at the initial call of
        // this method, see end.
        SAL_INFO( "vcl.cg", "CGContextRestoreGState(" << mrContext << ") " << mnContextStackDepth--);
        CGContextRestoreGState( mrContext );
    }
#endif

    // set graphics properties
    mxLayer = xLayer;
    mrContext = xContext;

#ifndef IOS
    mnBitmapDepth = nBitmapDepth;
#endif

#ifdef IOS
    mbForeignContext = xContext != NULL;
#endif

    // return early if the virdev is being destroyed
    if( !xContext )
        return;

    // get new graphics properties
    if( !mxLayer )
    {
        mnWidth = CGBitmapContextGetWidth( mrContext );
        mnHeight = CGBitmapContextGetHeight( mrContext );
        SAL_INFO( "vcl.cg", "CGBitmapContextGetWidth&Height(" << mrContext <<
                  ") = " << mnWidth << "x" << mnHeight );
    }
    else
    {
        const CGSize aSize = CGLayerGetSize( mxLayer );
        mnWidth = static_cast<int>(aSize.width);
        mnHeight = static_cast<int>(aSize.height);
        SAL_INFO( "vcl.cg", "CGLayerGetSize(" << mxLayer << ") = " << aSize );
    }

    // prepare graphics for drawing
    const CGColorSpaceRef aCGColorSpace = GetSalData()->mxRGBSpace;
    CGContextSetFillColorSpace( mrContext, aCGColorSpace );
    CGContextSetStrokeColorSpace( mrContext, aCGColorSpace );

    // re-enable XorEmulation for the new context
    if( mpXorEmulation )
    {
        mpXorEmulation->SetTarget( mnWidth, mnHeight, mnBitmapDepth, mrContext, mxLayer );
        if( mpXorEmulation->IsEnabled() )
        {
            mrContext = mpXorEmulation->GetMaskContext();
        }
    }

    // initialize stack of CGContext states
    SAL_INFO( "vcl.cg", "CGContextSaveGState(" << mrContext << ") " << ++mnContextStackDepth );
    CGContextSaveGState( mrContext );
    SetState();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
