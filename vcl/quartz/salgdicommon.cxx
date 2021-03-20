/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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
#include <sal/log.hxx>

#include <cassert>
#include <cstring>
#include <numeric>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <osl/endian.h>
#include <osl/file.hxx>
#include <sal/types.h>
#include <tools/long.hxx>
#include <vcl/sysdata.hxx>

#include <fontsubset.hxx>
#include <quartz/salbmp.h>
#ifdef MACOSX
#include <quartz/salgdi.h>
#endif
#include <quartz/utils.h>
#ifdef IOS
#include "saldatabasic.hxx"
#endif
#include <sft.hxx>

using namespace vcl;

const basegfx::B2DPoint aHalfPointOfs ( 0.5, 0.5 );

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
            CGPathAddCurveToPoint( xPath, nullptr, aCP1.getX(), aCP1.getY(),
                                    aCP2.getX(), aCP2.getY(), aPoint.getX(), aPoint.getY() );
        }
    }

    if( bClosePath )
    {
        CGPathCloseSubpath( xPath );
    }
}

bool AquaSalGraphics::CreateFontSubset( const OUString& rToFile,
                                        const PhysicalFontFace* pFontData,
                                        const sal_GlyphId* pGlyphIds, const sal_uInt8* pEncoding,
                                        sal_Int32* pGlyphWidths, const int nGlyphCount,
                                        FontSubsetInfo& rInfo )
{
    // TODO: move more of the functionality here into the generic subsetter code

    // prepare the requested file name for writing the font-subset file
    OUString aSysPath;
    if( osl_File_E_None != osl_getSystemPathFromFileURL( rToFile.pData, &aSysPath.pData ) )
        return false;

    // get the raw-bytes from the font to be subset
    std::vector<unsigned char> aBuffer;
    bool bCffOnly = false;
    if( !GetRawFontData( pFontData, aBuffer, &bCffOnly ) )
        return false;
    const OString aToFile( OUStringToOString( aSysPath,
                                              osl_getThreadTextEncoding()));

    // handle CFF-subsetting
    // NOTE: assuming that all glyphids requested on Aqua are fully translated
    if (bCffOnly)
        return SalGraphics::CreateCFFfontSubset(aBuffer.data(), aBuffer.size(), aToFile, pGlyphIds,
                                                pEncoding, pGlyphWidths, nGlyphCount, rInfo);

    // TODO: modernize psprint's horrible fontsubset C-API
    // this probably only makes sense after the switch to another SCM
    // that can preserve change history after file renames

    // prepare data for psprint's font subsetter
    TrueTypeFont* pSftFont = nullptr;
    if (::OpenTTFontBuffer( static_cast<void*>(aBuffer.data()), aBuffer.size(), 0, &pSftFont)
            != SFErrCodes::Ok)
        return false;

    // get details about the subsetted font
    TTGlobalFontInfo aTTInfo;
    ::GetTTGlobalFontInfo( pSftFont, &aTTInfo );
    OUString aPSName(aTTInfo.psname, std::strlen(aTTInfo.psname), RTL_TEXTENCODING_UTF8);
    FillFontSubsetInfo(aTTInfo, aPSName, rInfo);

    // write subset into destination file
    bool bRet
        = SalGraphics::CreateTTFfontSubset(*pSftFont, aToFile, false /* use FontSelectPattern? */,
                                           pGlyphIds, pEncoding, pGlyphWidths, nGlyphCount);
    ::CloseTTFont(pSftFont);
    return bRet;
}

static void alignLinePoint( const Point* i_pIn, float& o_fX, float& o_fY )
{
    o_fX = static_cast<float>(i_pIn->getX() ) + 0.5;
    o_fY = static_cast<float>(i_pIn->getY() ) + 0.5;
}

static void DrawPattern50( void*, CGContextRef rContext )
{
    static const CGRect aRects[2] = { { {0,0}, { 2, 2 } }, { { 2, 2 }, { 2, 2 } } };
    CGContextAddRects( rContext, aRects, 2 );
    CGContextFillPath( rContext );
}

static void getBoundRect( sal_uInt32 nPoints, const Point *pPtAry,
                          tools::Long &rX, tools::Long& rY, tools::Long& rWidth,
                          tools::Long& rHeight )
{
    tools::Long nX1 = pPtAry->getX();
    tools::Long nX2 = nX1;
    tools::Long nY1 = pPtAry->getY();
    tools::Long nY2 = nY1;

    for( sal_uInt32 n = 1; n < nPoints; n++ )
    {
        if( pPtAry[n].getX() < nX1 )
        {
            nX1 = pPtAry[n].getX();
        }
        else if( pPtAry[n].getX() > nX2 )
        {
            nX2 = pPtAry[n].getX();
        }
        if( pPtAry[n].getY() < nY1 )
        {
            nY1 = pPtAry[n].getY();
        }
        else if( pPtAry[n].getY() > nY2 )
        {
            nY2 = pPtAry[n].getY();
        }
    }
    rX = nX1;
    rY = nY1;
    rWidth = nX2 - nX1 + 1;
    rHeight = nY2 - nY1 + 1;
}

static Color ImplGetROPColor( SalROPColor nROPColor )
{
    Color nColor;
    if ( nROPColor == SalROPColor::N0 )
    {
        nColor = Color( 0, 0, 0 );
    }
    else
    {
        nColor = Color( 255, 255, 255 );
    }
    return nColor;
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
    // An image mask can't have a depth > 8 bits (should be 1 to 8 bits)
    if( rAlphaBmp.GetBitCount() > 8 )
        return false;

    // are these two tests really necessary? (see vcl/unx/source/gdi/salgdi2.cxx)
    // horizontal/vertical mirroring not implemented yet
    if( rTR.mnDestWidth < 0 || rTR.mnDestHeight < 0 )
        return false;

    const QuartzSalBitmap& rSrcSalBmp = static_cast<const QuartzSalBitmap&>(rSrcBitmap);
    const QuartzSalBitmap& rMaskSalBmp = static_cast<const QuartzSalBitmap&>(rAlphaBmp);
    CGImageRef xMaskedImage = rSrcSalBmp.CreateWithMask( rMaskSalBmp, rTR.mnSrcX,
                                                         rTR.mnSrcY, rTR.mnSrcWidth,
                                                         rTR.mnSrcHeight );
    if( !xMaskedImage )
        return false;

    if ( CheckContext() )
    {
        const CGRect aDstRect = CGRectMake( rTR.mnDestX, rTR.mnDestY, rTR.mnDestWidth, rTR.mnDestHeight);
        CGContextDrawImage( maContextHolder.get(), aDstRect, xMaskedImage );
        RefreshRect( aDstRect );
    }

    CGImageRelease(xMaskedImage);

    return true;
}

bool AquaSalGraphics::drawTransformedBitmap(
    const basegfx::B2DPoint& rNull, const basegfx::B2DPoint& rX, const basegfx::B2DPoint& rY,
    const SalBitmap& rSrcBitmap, const SalBitmap* pAlphaBmp, double fAlpha )
{
    if( !CheckContext() )
        return true;

    if( fAlpha != 1.0 )
        return false;

    // get the Quartz image
    CGImageRef xImage = nullptr;
    const Size aSize = rSrcBitmap.GetSize();
    const QuartzSalBitmap& rSrcSalBmp = static_cast<const QuartzSalBitmap&>(rSrcBitmap);
    const QuartzSalBitmap* pMaskSalBmp = static_cast<const QuartzSalBitmap*>(pAlphaBmp);

    if( !pMaskSalBmp)
        xImage = rSrcSalBmp.CreateCroppedImage( 0, 0, static_cast<int>(aSize.Width()), static_cast<int>(aSize.Height()) );
    else
        xImage = rSrcSalBmp.CreateWithMask( *pMaskSalBmp, 0, 0, static_cast<int>(aSize.Width()), static_cast<int>(aSize.Height()) );
    if( !xImage )
        return false;

    // setup the image transformation
    // using the rNull,rX,rY points as destinations for the (0,0),(0,Width),(Height,0) source points
    maContextHolder.saveState();
    const basegfx::B2DVector aXRel = rX - rNull;
    const basegfx::B2DVector aYRel = rY - rNull;
    const CGAffineTransform aCGMat = CGAffineTransformMake(
        aXRel.getX()/aSize.Width(), aXRel.getY()/aSize.Width(),
        aYRel.getX()/aSize.Height(), aYRel.getY()/aSize.Height(),
        rNull.getX(), rNull.getY());

    CGContextConcatCTM( maContextHolder.get(), aCGMat );

    // draw the transformed image
    const CGRect aSrcRect = CGRectMake(0, 0, aSize.Width(), aSize.Height());
    CGContextDrawImage( maContextHolder.get(), aSrcRect, xImage );

    CGImageRelease( xImage );
    // restore the Quartz graphics state
    maContextHolder.restoreState();

    // mark the destination as painted
    const CGRect aDstRect = CGRectApplyAffineTransform( aSrcRect, aCGMat );
    RefreshRect( aDstRect );

    return true;
}

bool AquaSalGraphics::hasFastDrawTransformedBitmap() const
{
    return false;
}

bool AquaSalGraphics::drawAlphaRect( tools::Long nX, tools::Long nY, tools::Long nWidth,
                                     tools::Long nHeight, sal_uInt8 nTransparency )
{
    if( !CheckContext() )
        return true;

    // save the current state
    maContextHolder.saveState();
    CGContextSetAlpha( maContextHolder.get(), (100-nTransparency) * (1.0/100) );

    CGRect aRect = CGRectMake(nX, nY, nWidth-1, nHeight-1);
    if( IsPenVisible() )
    {
        aRect.origin.x += 0.5;
        aRect.origin.y += 0.5;
    }

    CGContextBeginPath( maContextHolder.get() );
    CGContextAddRect( maContextHolder.get(), aRect );
    CGContextDrawPath( maContextHolder.get(), kCGPathFill );

    maContextHolder.restoreState();
    RefreshRect( aRect );

    return true;
}

void AquaSalGraphics::drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap )
{
    if( !CheckContext() )
        return;

    const QuartzSalBitmap& rBitmap = static_cast<const QuartzSalBitmap&>(rSalBitmap);
    CGImageRef xImage = rBitmap.CreateCroppedImage( static_cast<int>(rPosAry.mnSrcX), static_cast<int>(rPosAry.mnSrcY),
                                                    static_cast<int>(rPosAry.mnSrcWidth), static_cast<int>(rPosAry.mnSrcHeight) );
    if( !xImage )
        return;

    const CGRect aDstRect = CGRectMake(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth, rPosAry.mnDestHeight);
    CGContextDrawImage( maContextHolder.get(), aDstRect, xImage );

    CGImageRelease( xImage );
    RefreshRect( aDstRect );
}

void AquaSalGraphics::drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap,
                                  const SalBitmap& rTransparentBitmap )
{
    if( !CheckContext() )
        return;

    const QuartzSalBitmap& rBitmap = static_cast<const QuartzSalBitmap&>(rSalBitmap);
    const QuartzSalBitmap& rMask = static_cast<const QuartzSalBitmap&>(rTransparentBitmap);
    CGImageRef xMaskedImage( rBitmap.CreateWithMask( rMask, rPosAry.mnSrcX, rPosAry.mnSrcY,
                                                     rPosAry.mnSrcWidth, rPosAry.mnSrcHeight ) );
    if( !xMaskedImage )
        return;

    const CGRect aDstRect = CGRectMake(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth, rPosAry.mnDestHeight);
    CGContextDrawImage( maContextHolder.get(), aDstRect, xMaskedImage );
    CGImageRelease( xMaskedImage );
    RefreshRect( aDstRect );
}

#ifndef IOS

bool AquaSalGraphics::drawEPS(
    tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight,
    void* pEpsData, sal_uInt32 nByteCount )
{
    // convert the raw data to an NSImageRef
    NSData* xNSData = [NSData dataWithBytes:pEpsData length:static_cast<int>(nByteCount)];
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
    maContextHolder.saveState();
//  CGContextTranslateCTM( maContextHolder.get(), 0, +mnHeight );
    CGContextScaleCTM( maContextHolder.get(), +1, -1 );
    nY = /*mnHeight*/ - (nY + nHeight);

    // prepare the target context
    NSGraphicsContext* pOrigNSCtx = [NSGraphicsContext currentContext];
    [pOrigNSCtx retain];

    // create new context
    NSGraphicsContext* pDrawNSCtx = [NSGraphicsContext graphicsContextWithCGContext: maContextHolder.get() flipped: IsFlipped()];
    // set it, setCurrentContext also releases the previously set one
    [NSGraphicsContext setCurrentContext: pDrawNSCtx];

    // draw the EPS
    const NSRect aDstRect = NSMakeRect( nX, nY, nWidth, nHeight);
    const bool bOK = [xEpsImage drawInRect: aDstRect];

    // restore the NSGraphicsContext
    [NSGraphicsContext setCurrentContext: pOrigNSCtx];
    [pOrigNSCtx release]; // restore the original retain count

    maContextHolder.restoreState();
    // mark the destination rectangle as updated
    RefreshRect( aDstRect );

    return bOK;
}

#endif

void AquaSalGraphics::drawLine( tools::Long nX1, tools::Long nY1, tools::Long nX2, tools::Long nY2 )
{
    if( nX1 == nX2 && nY1 == nY2 )
    {
        // #i109453# platform independent code expects at least one pixel to be drawn
        drawPixel( nX1, nY1 );

        return;
    }

    if( !CheckContext() )
        return;

    CGContextBeginPath( maContextHolder.get() );
    CGContextMoveToPoint( maContextHolder.get(), static_cast<float>(nX1)+0.5, static_cast<float>(nY1)+0.5 );
    CGContextAddLineToPoint( maContextHolder.get(), static_cast<float>(nX2)+0.5, static_cast<float>(nY2)+0.5 );
    CGContextDrawPath( maContextHolder.get(), kCGPathStroke );

    tools::Rectangle aRefreshRect( nX1, nY1, nX2, nY2 );
    (void) aRefreshRect;
    // Is a call to RefreshRect( aRefreshRect ) missing here?
}

void AquaSalGraphics::drawMask( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap, Color nMaskColor )
{
    if( !CheckContext() )
        return;

    const QuartzSalBitmap& rBitmap = static_cast<const QuartzSalBitmap&>(rSalBitmap);
    CGImageRef xImage = rBitmap.CreateColorMask( rPosAry.mnSrcX, rPosAry.mnSrcY,
                                                 rPosAry.mnSrcWidth, rPosAry.mnSrcHeight,
                                                 nMaskColor );
    if( !xImage )
        return;

    const CGRect aDstRect = CGRectMake(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth, rPosAry.mnDestHeight);
    CGContextDrawImage( maContextHolder.get(), aDstRect, xImage );
    CGImageRelease( xImage );
    RefreshRect( aDstRect );
}

void AquaSalGraphics::drawPixel( tools::Long nX, tools::Long nY )
{
    // draw pixel with current line color
    ImplDrawPixel( nX, nY, maLineColor );
}

void AquaSalGraphics::drawPixel( tools::Long nX, tools::Long nY, Color nColor )
{
    const RGBAColor aPixelColor( nColor );
    ImplDrawPixel( nX, nY, aPixelColor );
}

bool AquaSalGraphics::drawPolyLine(
    const basegfx::B2DHomMatrix& rObjectToDevice,
    const basegfx::B2DPolygon& rPolyLine,
    double fTransparency,
    double fLineWidth,
    const std::vector< double >* pStroke, // MM01
    basegfx::B2DLineJoin eLineJoin,
    css::drawing::LineCap eLineCap,
    double fMiterMinimumAngle,
    bool bPixelSnapHairline)
{
    // MM01 check done for simple reasons
    if(!rPolyLine.count() || fTransparency < 0.0 || fTransparency > 1.0)
    {
        return true;
    }

#ifdef IOS
    if( !CheckContext() )
        return false;
#endif

    // tdf#124848 get correct LineWidth in discrete coordinates,
    if(fLineWidth == 0) // hairline
        fLineWidth = 1.0;
    else // Adjust line width for object-to-device scale.
        fLineWidth = (rObjectToDevice * basegfx::B2DVector(fLineWidth, 0)).getLength();

    // #i101491# Aqua does not support B2DLineJoin::NONE; return false to use
    // the fallback (own geometry preparation)
    // #i104886# linejoin-mode and thus the above only applies to "fat" lines
    if( (basegfx::B2DLineJoin::NONE == eLineJoin) && (fLineWidth > 1.3) )
        return false;

    // MM01 need to do line dashing as fallback stuff here now
    const double fDotDashLength(nullptr != pStroke ? std::accumulate(pStroke->begin(), pStroke->end(), 0.0) : 0.0);
    const bool bStrokeUsed(0.0 != fDotDashLength);
    assert(!bStrokeUsed || (bStrokeUsed && pStroke));
    basegfx::B2DPolyPolygon aPolyPolygonLine;

    if(bStrokeUsed)
    {
        // apply LineStyle
        basegfx::utils::applyLineDashing(
            rPolyLine, // source
            *pStroke, // pattern
            &aPolyPolygonLine, // target for lines
            nullptr, // target for gaps
            fDotDashLength); // full length if available
    }
    else
    {
        // no line dashing, just copy
        aPolyPolygonLine.append(rPolyLine);
    }

    // Transform to DeviceCoordinates, get DeviceLineWidth, execute PixelSnapHairline
    aPolyPolygonLine.transform(rObjectToDevice);
    if(bPixelSnapHairline) { aPolyPolygonLine = basegfx::utils::snapPointsOfHorizontalOrVerticalEdges(aPolyPolygonLine); }

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

    // MM01 todo - I assume that this is OKAY to be done in one run for quartz
    // but this NEEDS to be checked/verified
    for(sal_uInt32 a(0); a < aPolyPolygonLine.count(); a++)
    {
        const basegfx::B2DPolygon aPolyLine(aPolyPolygonLine.getB2DPolygon(a));
        AddPolygonToPath(
            xPath,
            aPolyLine,
            aPolyLine.isClosed(),
            !getAntiAlias(),
            true);
    }

    const CGRect aRefreshRect = CGPathGetBoundingBox( xPath );
    // #i97317# workaround for Quartz having problems with drawing small polygons
    if( (aRefreshRect.size.width > 0.125) || (aRefreshRect.size.height > 0.125) )
    {
        // use the path to prepare the graphics context
        maContextHolder.saveState();
        CGContextBeginPath( maContextHolder.get() );
        CGContextAddPath( maContextHolder.get(), xPath );
        // draw path with antialiased line
        CGContextSetShouldAntialias( maContextHolder.get(), getAntiAlias() );
        CGContextSetAlpha( maContextHolder.get(), 1.0 - fTransparency );
        CGContextSetLineJoin( maContextHolder.get(), aCGLineJoin );
        CGContextSetLineCap( maContextHolder.get(), aCGLineCap );
        CGContextSetLineWidth( maContextHolder.get(), fLineWidth );
        CGContextSetMiterLimit(maContextHolder.get(), fCGMiterLimit);
        CGContextDrawPath( maContextHolder.get(), kCGPathStroke );
        maContextHolder.restoreState();

        // mark modified rectangle as updated
        RefreshRect( aRefreshRect );
    }

    CGPathRelease( xPath );

    return true;
}

bool AquaSalGraphics::drawPolyLineBezier( sal_uInt32, const Point*, const PolyFlags* )
{
    return false;
}

bool AquaSalGraphics::drawPolyPolygon(
    const basegfx::B2DHomMatrix& rObjectToDevice,
    const basegfx::B2DPolyPolygon& rPolyPolygon,
    double fTransparency)
{
#ifdef IOS
    if (!maContextHolder.isSet())
        return true;
#endif

    // short circuit if there is nothing to do
    if( rPolyPolygon.count() == 0 )
        return true;

    // ignore invisible polygons
    if( (fTransparency >= 1.0) || (fTransparency < 0) )
        return true;

    // Fallback: Transform to DeviceCoordinates
    basegfx::B2DPolyPolygon aPolyPolygon(rPolyPolygon);
    aPolyPolygon.transform(rObjectToDevice);

    // setup poly-polygon path
    CGMutablePathRef xPath = CGPathCreateMutable();
    // tdf#120252 Use the correct, already transformed PolyPolygon (as long as
    // the transformation is not used here...)
    for(auto const& rPolygon : aPolyPolygon)
    {
        AddPolygonToPath( xPath, rPolygon, true, !getAntiAlias(), IsPenVisible() );
    }

    const CGRect aRefreshRect = CGPathGetBoundingBox( xPath );
    // #i97317# workaround for Quartz having problems with drawing small polygons
    if( (aRefreshRect.size.width > 0.125) || (aRefreshRect.size.height > 0.125) )
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
            CGPathRelease( xPath );
            return true;
        }

        // use the path to prepare the graphics context
        maContextHolder.saveState();
        CGContextBeginPath( maContextHolder.get() );
        CGContextAddPath( maContextHolder.get(), xPath );

        // draw path with antialiased polygon
        CGContextSetShouldAntialias( maContextHolder.get(), getAntiAlias() );
        CGContextSetAlpha( maContextHolder.get(), 1.0 - fTransparency );
        CGContextDrawPath( maContextHolder.get(), eMode );
        maContextHolder.restoreState();

        // mark modified rectangle as updated
        RefreshRect( aRefreshRect );
    }

    CGPathRelease( xPath );

    return true;
}

void AquaSalGraphics::drawPolyPolygon( sal_uInt32 nPolyCount, const sal_uInt32 *pPoints, const Point*  *ppPtAry )
{
    if( nPolyCount <= 0 )
        return;

    if( !CheckContext() )
        return;

    // find bound rect
    tools::Long leftX = 0, topY = 0, maxWidth = 0, maxHeight = 0;
    getBoundRect( pPoints[0], ppPtAry[0], leftX, topY, maxWidth, maxHeight );

    for( sal_uInt32 n = 1; n < nPolyCount; n++ )
    {
        tools::Long nX = leftX, nY = topY, nW = maxWidth, nH = maxHeight;
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
        return;
    }

    // convert to CGPath
    CGContextBeginPath( maContextHolder.get() );
    if( IsPenVisible() )
    {
        for( sal_uInt32 nPoly = 0; nPoly < nPolyCount; nPoly++ )
        {
            const sal_uInt32 nPoints = pPoints[nPoly];
            if( nPoints > 1 )
            {
                const Point *pPtAry = ppPtAry[nPoly];
                float fX, fY;

                alignLinePoint( pPtAry, fX, fY );
                CGContextMoveToPoint( maContextHolder.get(), fX, fY );
                pPtAry++;

                for( sal_uInt32 nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++ )
                {
                    alignLinePoint( pPtAry, fX, fY );
                    CGContextAddLineToPoint( maContextHolder.get(), fX, fY );
                }
                CGContextClosePath(maContextHolder.get());
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
                const Point *pPtAry = ppPtAry[nPoly];
                CGContextMoveToPoint( maContextHolder.get(), pPtAry->getX(), pPtAry->getY() );
                pPtAry++;
                for( sal_uInt32 nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++ )
                {
                    CGContextAddLineToPoint( maContextHolder.get(), pPtAry->getX(), pPtAry->getY() );
                }
                CGContextClosePath(maContextHolder.get());
            }
        }
    }

    CGContextDrawPath( maContextHolder.get(), eMode );

    RefreshRect( leftX, topY, maxWidth, maxHeight );
}

void AquaSalGraphics::drawPolygon( sal_uInt32 nPoints, const Point *pPtAry )
{
    if( nPoints <= 1 )
        return;

    if( !CheckContext() )
        return;

    tools::Long nX = 0, nY = 0, nWidth = 0, nHeight = 0;
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

    CGContextBeginPath( maContextHolder.get() );

    if( IsPenVisible() )
    {
        float fX, fY;
        alignLinePoint( pPtAry, fX, fY );
        CGContextMoveToPoint( maContextHolder.get(), fX, fY );
        pPtAry++;
        for( sal_uInt32 nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++ )
        {
            alignLinePoint( pPtAry, fX, fY );
            CGContextAddLineToPoint( maContextHolder.get(), fX, fY );
        }
    }
    else
    {
        CGContextMoveToPoint( maContextHolder.get(), pPtAry->getX(), pPtAry->getY() );
        pPtAry++;
        for( sal_uInt32 nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++ )
        {
            CGContextAddLineToPoint( maContextHolder.get(), pPtAry->getX(), pPtAry->getY() );
        }
    }

    CGContextClosePath( maContextHolder.get() );
    CGContextDrawPath( maContextHolder.get(), eMode );
    RefreshRect( nX, nY, nWidth, nHeight );
}

bool AquaSalGraphics::drawPolygonBezier( sal_uInt32, const Point*, const PolyFlags* )
{
    return false;
}

bool AquaSalGraphics::drawPolyPolygonBezier( sal_uInt32, const sal_uInt32*,
                                             const Point* const*, const PolyFlags* const* )
{
    return false;
}

void AquaSalGraphics::drawRect(
    tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight )
{
    if( !CheckContext() )
        return;

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
        CGContextFillRect( maContextHolder.get(), aRect );
    }
    if( IsPenVisible() )
    {
        CGContextStrokeRect( maContextHolder.get(), aRect );
    }
    RefreshRect( nX, nY, nWidth, nHeight );
}

void AquaSalGraphics::drawPolyLine( sal_uInt32 nPoints, const Point *pPtAry )
{
    if( nPoints < 1 )
        return;

    if( !CheckContext() )
        return;

    tools::Long nX = 0, nY = 0, nWidth = 0, nHeight = 0;
    getBoundRect( nPoints, pPtAry, nX, nY, nWidth, nHeight );

    float fX, fY;
    CGContextBeginPath( maContextHolder.get() );
    alignLinePoint( pPtAry, fX, fY );
    CGContextMoveToPoint( maContextHolder.get(), fX, fY );
    pPtAry++;

    for( sal_uInt32 nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++ )
    {
        alignLinePoint( pPtAry, fX, fY );
        CGContextAddLineToPoint( maContextHolder.get(), fX, fY );
    }
    CGContextStrokePath(maContextHolder.get());

    RefreshRect( nX, nY, nWidth, nHeight );
}

sal_uInt16 AquaSalGraphics::GetBitCount() const
{
    sal_uInt16 nBits = mnBitmapDepth ? mnBitmapDepth : 32;//24;
    return nBits;
}

std::shared_ptr<SalBitmap> AquaSalGraphics::getBitmap(
    tools::Long  nX, tools::Long  nY, tools::Long  nDX, tools::Long  nDY )
{
    SAL_WARN_IF(!maLayer.isSet(), "vcl.quartz", "AquaSalGraphics::getBitmap() with no layer this=" << this);

    ApplyXorContext();

    std::shared_ptr<QuartzSalBitmap> pBitmap = std::make_shared<QuartzSalBitmap>();
    if (!pBitmap->Create(maLayer, mnBitmapDepth, nX, nY, nDX, nDY, IsFlipped()))
    {
        pBitmap = nullptr;
    }
    return pBitmap;
}

SystemGraphicsData AquaSalGraphics::GetGraphicsData() const
{
    SystemGraphicsData aRes;
    aRes.nSize = sizeof(aRes);
    aRes.rCGContext = maContextHolder.get();
    return aRes;
}

tools::Long AquaSalGraphics::GetGraphicsWidth() const
{
    tools::Long w = 0;
    if( maContextHolder.isSet() && (
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

Color AquaSalGraphics::getPixel( tools::Long nX, tools::Long nY )
{
    // return default value on printers or when out of bounds
    if (!maLayer.isSet() || (nX < 0) || (nX >= mnWidth) ||
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
                               uint32_t(kCGImageAlphaNoneSkipFirst) | uint32_t(kCGBitmapByteOrder32Big) );

    // update this graphics layer
    ApplyXorContext();

    // copy the requested pixel into the bitmap context
    if( IsFlipped() )
    {
        nY = mnHeight - nY;
    }
    const CGPoint aCGPoint = CGPointMake(-nX, -nY);
    CGContextDrawLayerAtPoint(xOnePixelContext, aCGPoint, maLayer.get());

    CGContextRelease( xOnePixelContext );

    Color nColor( aPixel.r, aPixel.g, aPixel.b );
    return nColor;
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
    // This *must* be 96 or else the iOS app will behave very badly (tiles are scaled wrongly and
    // don't match each others at their boundaries, and other issues). But *why* it must be 96 I
    // have no idea. The commit that changed it to 96 from (the arbitrary) 200 did not say. If you
    // know where else 96 is explicitly or implicitly hard-coded, please modify this comment.

    // Follow-up: It might be this: in 'online', loleaflet/src/map/Map.js:
        // 15 = 1440 twips-per-inch / 96 dpi.
        // Chosen to match previous hardcoded value of 3840 for
        // the current tile pixel size of 256.
    rDPIX = rDPIY = 96;
#endif
}

void AquaSalGraphics::ImplDrawPixel( tools::Long nX, tools::Long nY, const RGBAColor& rColor )
{
    if( !CheckContext() )
    {
        return;
    }
    // overwrite the fill color
    CGContextSetFillColor( maContextHolder.get(), rColor.AsArray() );
    // draw 1x1 rect, there is no pixel drawing in Quartz
    const CGRect aDstRect = CGRectMake(nX, nY, 1, 1);
    CGContextFillRect( maContextHolder.get(), aDstRect );
    RefreshRect( aDstRect );
    // reset the fill color
    CGContextSetFillColor( maContextHolder.get(), maFillColor.AsArray() );
}

#ifndef IOS

void AquaSalGraphics::initResolution(NSWindow* nsWindow)
{
    if (!nsWindow)
    {
        if (Application::IsBitmapRendering())
            mnRealDPIX = mnRealDPIY = 96;
        return;
    }

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
                    const CGDirectDisplayID nDisplayID = static_cast<CGDirectDisplayID>([pVal longValue]);
                    const CGSize aSize = CGDisplayScreenSize( nDisplayID ); // => result is in millimeters
                    mnRealDPIX = static_cast<sal_Int32>((CGDisplayPixelsWide( nDisplayID ) * 25.4) / aSize.width);
                    mnRealDPIY = static_cast<sal_Int32>((CGDisplayPixelsHigh( nDisplayID ) * 25.4) / aSize.height);
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

void AquaSalGraphics::invert(
    tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, SalInvert nFlags )
{
    if ( CheckContext() )
    {
        CGRect aCGRect = CGRectMake( nX, nY, nWidth, nHeight);
        maContextHolder.saveState();
        if ( nFlags & SalInvert::TrackFrame )
        {
            const CGFloat dashLengths[2]  = { 4.0, 4.0 };     // for drawing dashed line
            CGContextSetBlendMode( maContextHolder.get(), kCGBlendModeDifference );
            CGContextSetRGBStrokeColor ( maContextHolder.get(), 1.0, 1.0, 1.0, 1.0 );
            CGContextSetLineDash ( maContextHolder.get(), 0, dashLengths, 2 );
            CGContextSetLineWidth( maContextHolder.get(), 2.0);
            CGContextStrokeRect ( maContextHolder.get(), aCGRect );
        }
        else if ( nFlags & SalInvert::N50 )
        {
            //CGContextSetAllowsAntialiasing( maContextHolder.get(), false );
            CGContextSetBlendMode(maContextHolder.get(), kCGBlendModeDifference);
            CGContextAddRect( maContextHolder.get(), aCGRect );
            Pattern50Fill();
        }
        else // just invert
        {
            CGContextSetBlendMode(maContextHolder.get(), kCGBlendModeDifference);
            CGContextSetRGBFillColor ( maContextHolder.get(),1.0, 1.0, 1.0 , 1.0 );
            CGContextFillRect ( maContextHolder.get(), aCGRect );
        }
        maContextHolder.restoreState();
        RefreshRect( aCGRect );
    }
}

namespace {

CGPoint* makeCGptArray(sal_uInt32 nPoints, const Point* pPtAry)
{
    CGPoint *CGpoints = new CGPoint[nPoints];
    for(sal_uLong i=0;i<nPoints;i++)
    {
        CGpoints[i].x = pPtAry[i].getX();
        CGpoints[i].y = pPtAry[i].getY();
    }
    return CGpoints;
}

}

void AquaSalGraphics::invert( sal_uInt32 nPoints, const Point*  pPtAry, SalInvert nSalFlags )
{
    if ( CheckContext() )
    {
        maContextHolder.saveState();
        CGPoint* CGpoints = makeCGptArray(nPoints,pPtAry);
        CGContextAddLines ( maContextHolder.get(), CGpoints, nPoints );
        if ( nSalFlags & SalInvert::TrackFrame )
        {
            const CGFloat dashLengths[2]  = { 4.0, 4.0 };     // for drawing dashed line
            CGContextSetBlendMode( maContextHolder.get(), kCGBlendModeDifference );
            CGContextSetRGBStrokeColor ( maContextHolder.get(), 1.0, 1.0, 1.0, 1.0 );
            CGContextSetLineDash ( maContextHolder.get(), 0, dashLengths, 2 );
            CGContextSetLineWidth( maContextHolder.get(), 2.0);
            CGContextStrokePath ( maContextHolder.get() );
        }
        else if ( nSalFlags & SalInvert::N50 )
        {
            CGContextSetBlendMode(maContextHolder.get(), kCGBlendModeDifference);
            Pattern50Fill();
        }
        else // just invert
        {
            CGContextSetBlendMode( maContextHolder.get(), kCGBlendModeDifference );
            CGContextSetRGBFillColor( maContextHolder.get(), 1.0, 1.0, 1.0, 1.0 );
            CGContextFillPath( maContextHolder.get() );
        }
        const CGRect aRefreshRect = CGContextGetClipBoundingBox(maContextHolder.get());
        maContextHolder.restoreState();
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
    SAL_WARN_IF( !maContextHolder.get(), "vcl.quartz", "maContextHolder.get() is NULL" );
    CGContextSetFillColorSpace( maContextHolder.get(), mxP50Space );
    CGContextSetFillPattern( maContextHolder.get(), mxP50Pattern, aFillCol );
    CGContextFillPath( maContextHolder.get() );
}

void AquaSalGraphics::ResetClipRegion()
{
    // release old path and indicate no clipping
    if( mxClipPath )
    {
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
    maContextHolder.restoreState();
    maContextHolder.saveState();

    // setup clipping
    if( mxClipPath )
    {
        CGContextBeginPath( maContextHolder.get() );            // discard any existing path
        CGContextAddPath( maContextHolder.get(), mxClipPath );  // set the current path to the clipping path
        CGContextClip( maContextHolder.get() );                 // use it for clipping
    }

    // set RGB colorspace and line and fill colors
    CGContextSetFillColor( maContextHolder.get(), maFillColor.AsArray() );

    CGContextSetStrokeColor( maContextHolder.get(), maLineColor.AsArray() );
    CGContextSetShouldAntialias( maContextHolder.get(), false );
    if( mnXorMode == 2 )
    {
        CGContextSetBlendMode( maContextHolder.get(), kCGBlendModeDifference );
    }
}

void AquaSalGraphics::SetLineColor()
{
    maLineColor.SetAlpha( 0.0 );   // transparent
    if( CheckContext() )
    {
        CGContextSetRGBStrokeColor( maContextHolder.get(), maLineColor.GetRed(), maLineColor.GetGreen(),
                                    maLineColor.GetBlue(), maLineColor.GetAlpha() );
    }
}

void AquaSalGraphics::SetLineColor( Color nColor )
{
    maLineColor = RGBAColor( nColor );
    if( CheckContext() )
    {
        CGContextSetRGBStrokeColor( maContextHolder.get(), maLineColor.GetRed(), maLineColor.GetGreen(),
                                   maLineColor.GetBlue(), maLineColor.GetAlpha() );
    }
}

void AquaSalGraphics::SetFillColor()
{
    maFillColor.SetAlpha( 0.0 );   // transparent
    if( CheckContext() )
    {
        CGContextSetRGBFillColor( maContextHolder.get(), maFillColor.GetRed(), maFillColor.GetGreen(),
                                  maFillColor.GetBlue(), maFillColor.GetAlpha() );
    }
}

void AquaSalGraphics::SetFillColor( Color nColor )
{
    maFillColor = RGBAColor( nColor );
    if( CheckContext() )
    {
        CGContextSetRGBFillColor( maContextHolder.get(), maFillColor.GetRed(), maFillColor.GetGreen(),
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
        CGPathRelease( mxClipPath );
        mxClipPath = nullptr;
    }
    mxClipPath = CGPathCreateMutable();

    // set current path, either as polypolgon or sequence of rectangles
    RectangleVector aRectangles;
    i_rClip.GetRegionRectangles(aRectangles);

    for(const auto& rRect : aRectangles)
    {
        const tools::Long nW(rRect.Right() - rRect.Left() + 1); // uses +1 logic in original

        if(nW)
        {
            const tools::Long nH(rRect.Bottom() - rRect.Top() + 1); // uses +1 logic in original

            if(nH)
            {
                const CGRect aRect = CGRectMake( rRect.Left(), rRect.Top(), nW, nH);
                CGPathAddRect( mxClipPath, nullptr, aRect );
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
        SetFillColor( ImplGetROPColor( nROPColor ) );
    }
}

void AquaSalGraphics::SetROPLineColor( SalROPColor nROPColor )
{
    if( ! mbPrinter )
    {
        SetLineColor( ImplGetROPColor( nROPColor ) );
    }
}

void AquaSalGraphics::SetXORMode( bool bSet, bool bInvertOnly )
{
    // return early if XOR mode remains unchanged
    if( mbPrinter )
    {
        return;
    }
    if( ! bSet && mnXorMode == 2 )
    {
        CGContextSetBlendMode( maContextHolder.get(), kCGBlendModeNormal );
        mnXorMode = 0;
        return;
    }
    else if( bSet && bInvertOnly && mnXorMode == 0)
    {
        CGContextSetBlendMode( maContextHolder.get(), kCGBlendModeDifference );
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
        mpXorEmulation->SetTarget(mnWidth, mnHeight, mnBitmapDepth, maContextHolder.get(), maLayer.get());
    }

    // change the XOR mode
    if( bSet )
    {
        mpXorEmulation->Enable();
        maContextHolder.set(mpXorEmulation->GetMaskContext());
        mnXorMode = 1;
    }
    else
    {
        mpXorEmulation->UpdateTarget();
        mpXorEmulation->Disable();
        maContextHolder.set(mpXorEmulation->GetTargetContext());
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
        CGContextRelease( m_xMaskContext );
        delete[] m_pMaskBuffer;
        m_xMaskContext = nullptr;
        m_pMaskBuffer = nullptr;

        // cleanup the temp context if needed
        if( m_xTempContext )
        {
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
    int nBytesPerRow = 4;
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
    }

    // initialize XOR mask context for drawing
    CGContextSetFillColorSpace( m_xMaskContext, aCGColorSpace );
    CGContextSetStrokeColorSpace( m_xMaskContext, aCGColorSpace );
    CGContextSetShouldAntialias( m_xMaskContext, false );

    // improve the XorMask's XOR emulation a little
    // NOTE: currently only enabled for monochrome contexts
    if( aCGColorSpace == GetSalData()->mxGraySpace )
    {
        CGContextSetBlendMode( m_xMaskContext, kCGBlendModeDifference );
    }
    // initialize the transformation matrix to the drawing target
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
        const int nWidth  = static_cast<int>(CGImageGetWidth( xXorImage ));
        const int nHeight = static_cast<int>(CGImageGetHeight( xXorImage ));
        // TODO: update minimal changerect
        const CGRect aFullRect = CGRectMake(0, 0, nWidth, nHeight);
        CGContextDrawImage( m_xTargetContext, aFullRect, xXorImage );
        CGImageRelease( xXorImage );
    }

    // reset the XorMask to black again
    // TODO: not needed for last update
    memset( m_pMaskBuffer, 0, m_nBufferLongs * sizeof(sal_uLong) );

    // TODO: return FALSE if target was not changed
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
