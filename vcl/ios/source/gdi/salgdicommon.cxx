/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <sal/types.h>
#include <osl/file.hxx>

#include "basegfx/polygon/b2dpolygon.hxx"

#include "ios/salbmp.h"
#include "ios/salgdi.h"

#include "fontsubset.hxx"
#include "region.h"
#include "sft.hxx"

using namespace vcl;

//typedef unsigned char Boolean; // copied from MacTypes.h, should be properly included
typedef std::vector<unsigned char> ByteVector;

static const basegfx::B2DPoint aHalfPointOfs ( 0.5, 0.5 );

static void AddPolygonToPath( CGMutablePathRef xPath,
                              const ::basegfx::B2DPolygon& rPolygon,
                              bool bClosePath, bool bPixelSnap, bool bLineDraw )
{
    // short circuit if there is nothing to do
    const int nPointCount = rPolygon.count();
    if( nPointCount <= 0 )
    {
        return;
    }
    (void)bPixelSnap; // TODO
    const CGAffineTransform* pTransform = NULL;

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

        ::basegfx::B2DPoint aPoint = rPolygon.getB2DPoint( nClosedIdx );

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
            CGPathMoveToPoint( xPath, pTransform, aPoint.getX(), aPoint.getY() );
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
            CGPathAddLineToPoint( xPath, pTransform, aPoint.getX(), aPoint.getY() );
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
            CGPathAddCurveToPoint( xPath, pTransform, aCP1.getX(), aCP1.getY(),
                                   aCP2.getX(), aCP2.getY(), aPoint.getX(), aPoint.getY() );
        }
    }

    if( bClosePath )
    {
        CGPathCloseSubpath( xPath );
    }
}

static void AddPolyPolygonToPath( CGMutablePathRef xPath,
                                  const ::basegfx::B2DPolyPolygon& rPolyPoly,
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
        const ::basegfx::B2DPolygon rPolygon = rPolyPoly.getB2DPolygon( nPolyIdx );
        AddPolygonToPath( xPath, rPolygon, true, bPixelSnap, bLineDraw );
    }
}

sal_Bool IosSalGraphics::CreateFontSubset( const rtl::OUString& rToFile,
                                            const PhysicalFontFace* pFontData,
                                            long* pGlyphIDs, sal_uInt8* pEncoding,
                                            sal_Int32* pGlyphWidths, int nGlyphCount,
                                            FontSubsetInfo& rInfo )
{
    // TODO: move more of the functionality here into the generic subsetter code

    // prepare the requested file name for writing the font-subset file
    rtl::OUString aSysPath;
    if( osl_File_E_None != osl_getSystemPathFromFileURL( rToFile.pData, &aSysPath.pData ) )
        return sal_False;
    const rtl_TextEncoding aThreadEncoding = osl_getThreadTextEncoding();
    const rtl::OString aToFile( rtl::OUStringToOString( aSysPath, aThreadEncoding ) );

    // get the raw-bytes from the font to be subset
    ByteVector aBuffer;
    bool bCffOnly = false;
    if( !GetRawFontData( pFontData, aBuffer, &bCffOnly ) )
        return sal_False;

    // handle CFF-subsetting
    if( bCffOnly )
    {
        // provide the raw-CFF data to the subsetter
        ByteCount nCffLen = aBuffer.size();
        rInfo.LoadFont( FontSubsetInfo::CFF_FONT, &aBuffer[0], nCffLen );

        // NOTE: assuming that all glyphids requested on Ios are fully translated

        // make the subsetter provide the requested subset
        FILE* pOutFile = fopen( aToFile.getStr(), "wb" );
        bool bRC = rInfo.CreateFontSubset( FontSubsetInfo::TYPE1_PFB, pOutFile, NULL,
            pGlyphIDs, pEncoding, nGlyphCount, pGlyphWidths );
        fclose( pOutFile );
        return bRC;
    }

    // TODO: modernize psprint's horrible fontsubset C-API
    // this probably only makes sense after the switch to another SCM
    // that can preserve change history after file renames

    // prepare data for psprint's font subsetter
    TrueTypeFont* pSftFont = NULL;
    int nRC = ::OpenTTFontBuffer( (void*)&aBuffer[0], aBuffer.size(), 0, &pSftFont);
    if( nRC != SF_OK )
        return sal_False;

    // get details about the subsetted font
    TTGlobalFontInfo aTTInfo;
    ::GetTTGlobalFontInfo( pSftFont, &aTTInfo );
    rInfo.m_nFontType   = FontSubsetInfo::SFNT_TTF;
    rInfo.m_aPSName     = String( aTTInfo.psname, RTL_TEXTENCODING_UTF8 );
    rInfo.m_aFontBBox   = Rectangle( Point( aTTInfo.xMin, aTTInfo.yMin ),
                                    Point( aTTInfo.xMax, aTTInfo.yMax ) );
    rInfo.m_nCapHeight  = aTTInfo.yMax; // Well ...
    rInfo.m_nAscent     = aTTInfo.winAscent;
    rInfo.m_nDescent    = aTTInfo.winDescent;
    // mac fonts usually do not have an OS2-table
    // => get valid ascent/descent values from other tables
    if( !rInfo.m_nAscent )
        rInfo.m_nAscent = +aTTInfo.typoAscender;
    if( !rInfo.m_nAscent )
        rInfo.m_nAscent = +aTTInfo.ascender;
    if( !rInfo.m_nDescent )
        rInfo.m_nDescent = +aTTInfo.typoDescender;
    if( !rInfo.m_nDescent )
        rInfo.m_nDescent = -aTTInfo.descender;

    // subset glyphs and get their properties
    // take care that subset fonts require the NotDef glyph in pos 0
    int nOrigCount = nGlyphCount;
    sal_uInt16    aShortIDs[ 256 ];
    sal_uInt8 aTempEncs[ 256 ];

    int nNotDef = -1;
    for( int i = 0; i < nGlyphCount; ++i )
    {
        aTempEncs[i] = pEncoding[i];
        sal_uInt32 nGlyphIdx = pGlyphIDs[i] & GF_IDXMASK;
        if( pGlyphIDs[i] & GF_ISCHAR )
        {
            bool bVertical = (pGlyphIDs[i] & GF_ROTMASK) != 0;
            nGlyphIdx = ::MapChar( pSftFont, static_cast<sal_uInt16>(nGlyphIdx), bVertical );
            if( nGlyphIdx == 0 && pFontData->IsSymbolFont() )
            {
                // #i12824# emulate symbol aliasing U+FXXX <-> U+0XXX
                nGlyphIdx = pGlyphIDs[i] & GF_IDXMASK;
                nGlyphIdx = (nGlyphIdx & 0xF000) ? (nGlyphIdx & 0x00FF) : (nGlyphIdx | 0xF000 );
                nGlyphIdx = ::MapChar( pSftFont, static_cast<sal_uInt16>(nGlyphIdx), bVertical );
            }
        }
        aShortIDs[i] = static_cast<sal_uInt16>( nGlyphIdx );
        if( !nGlyphIdx )
            if( nNotDef < 0 )
                nNotDef = i; // first NotDef glyph found
    }

    if( nNotDef != 0 )
    {
        // add fake NotDef glyph if needed
        if( nNotDef < 0 )
            nNotDef = nGlyphCount++;

        // NotDef glyph must be in pos 0 => swap glyphids
        aShortIDs[ nNotDef ] = aShortIDs[0];
        aTempEncs[ nNotDef ] = aTempEncs[0];
        aShortIDs[0] = 0;
        aTempEncs[0] = 0;
    }
    DBG_ASSERT( nGlyphCount < 257, "too many glyphs for subsetting" );

    // TODO: where to get bVertical?
    const bool bVertical = false;

    // fill the pGlyphWidths array
    // while making sure that the NotDef glyph is at index==0
    TTSimpleGlyphMetrics* pGlyphMetrics =
        ::GetTTSimpleGlyphMetrics( pSftFont, aShortIDs, nGlyphCount, bVertical );
    if( !pGlyphMetrics )
        return sal_False;
    sal_uInt16 nNotDefAdv       = pGlyphMetrics[0].adv;
    pGlyphMetrics[0].adv        = pGlyphMetrics[nNotDef].adv;
    pGlyphMetrics[nNotDef].adv  = nNotDefAdv;
    for( int i = 0; i < nOrigCount; ++i )
        pGlyphWidths[i] = pGlyphMetrics[i].adv;
    free( pGlyphMetrics );

    // write subset into destination file
    nRC = ::CreateTTFromTTGlyphs( pSftFont, aToFile.getStr(), aShortIDs,
            aTempEncs, nGlyphCount, 0, NULL, 0 );
    ::CloseTTFont(pSftFont);
    return (nRC == SF_OK);
}

static inline void alignLinePoint( const SalPoint* i_pIn, float& o_fX, float& o_fY )
{
    o_fX = static_cast<float>(i_pIn->mnX ) + 0.5;
    o_fY = static_cast<float>(i_pIn->mnY ) + 0.5;
}

void IosSalGraphics::copyBits( const SalTwoRect *pPosAry, SalGraphics *pSrcGraphics )
{
    if( !pSrcGraphics )
    {
        pSrcGraphics = this;
    }
    //from unix salgdi2.cxx
    //[FIXME] find a better way to prevent calc from crashing when width and height are negative
    if( pPosAry->mnSrcWidth <= 0
        || pPosAry->mnSrcHeight <= 0
        || pPosAry->mnDestWidth <= 0
        || pPosAry->mnDestHeight <= 0 )
    {
        return;
    }

    // accelerate trivial operations
    /*const*/ IosSalGraphics* pSrc = static_cast<IosSalGraphics*>(pSrcGraphics);
    const bool bSameGraphics = (this == pSrc) ||
        (mbWindow && mpFrame && pSrc->mbWindow && (mpFrame == pSrc->mpFrame));
    if( bSameGraphics &&
        (pPosAry->mnSrcWidth == pPosAry->mnDestWidth) &&
        (pPosAry->mnSrcHeight == pPosAry->mnDestHeight))
    {
        // short circuit if there is nothing to do
        if( (pPosAry->mnSrcX == pPosAry->mnDestX) &&
            (pPosAry->mnSrcY == pPosAry->mnDestY))
            return;
        // use copyArea() if source and destination context are identical
        copyArea( pPosAry->mnDestX, pPosAry->mnDestY, pPosAry->mnSrcX, pPosAry->mnSrcY,
            pPosAry->mnSrcWidth, pPosAry->mnSrcHeight, 0 );
        return;
    }

    ApplyXorContext();
    pSrc->ApplyXorContext();

    DBG_ASSERT( pSrc->mxLayer!=NULL, "IosSalGraphics::copyBits() from non-layered graphics" );

    const CGPoint aDstPoint = { +pPosAry->mnDestX - pPosAry->mnSrcX, pPosAry->mnDestY - pPosAry->mnSrcY };
    if( (pPosAry->mnSrcWidth == pPosAry->mnDestWidth &&
         pPosAry->mnSrcHeight == pPosAry->mnDestHeight) &&
        (!mnBitmapDepth || (aDstPoint.x + pSrc->mnWidth) <= mnWidth) ) // workaround a Quartz crasher
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
        CGContextSaveGState( xCopyContext );
        const CGRect aDstRect = { {pPosAry->mnDestX, pPosAry->mnDestY}, {pPosAry->mnDestWidth, pPosAry->mnDestHeight} };
        CGContextClipToRect( xCopyContext, aDstRect );

        // draw at new destination
        // NOTE: flipped drawing gets disabled for this, else the subimage would be drawn upside down
        if( pSrc->IsFlipped() )
        {
            CGContextTranslateCTM( xCopyContext, 0, +mnHeight ); CGContextScaleCTM( xCopyContext, +1, -1 );
        }
        // TODO: pSrc->size() != this->size()
        ::CGContextDrawLayerAtPoint( xCopyContext, aDstPoint, pSrc->mxLayer );
        CGContextRestoreGState( xCopyContext );
        // mark the destination rectangle as updated
        RefreshRect( aDstRect );
    }
    else
    {
        SalBitmap* pBitmap = pSrc->getBitmap( pPosAry->mnSrcX, pPosAry->mnSrcY,
                                              pPosAry->mnSrcWidth, pPosAry->mnSrcHeight );

        if( pBitmap )
        {
            SalTwoRect aPosAry( *pPosAry );
            aPosAry.mnSrcX = 0;
            aPosAry.mnSrcY = 0;
            drawBitmap( &aPosAry, *pBitmap );
            delete pBitmap;
        }
    }
}

static void DrawPattern50( void*, CGContextRef rContext )
{
    static const CGRect aRects[2] = { { {0,0}, { 2, 2 } }, { { 2, 2 }, { 2, 2 } } };
    CGContextAddRects( rContext, aRects, 2 );
    CGContextFillPath( rContext );
}

static void getBoundRect( sal_uLong nPoints, const SalPoint *pPtAry, long &rX, long& rY, long& rWidth, long& rHeight )
{
    long nX1 = pPtAry->mnX;
    long nX2 = nX1;
    long nY1 = pPtAry->mnY;
    long nY2 = nY1;
    for( sal_uLong n = 1; n < nPoints; n++ )
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
    if ( nROPColor == SAL_ROP_0 )
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
void IosSalGraphics::ApplyXorContext()
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

void IosSalGraphics::copyArea( long nDstX, long nDstY,long nSrcX, long nSrcY,
                                long nSrcWidth, long nSrcHeight, sal_uInt16 /*nFlags*/ )
{
    ApplyXorContext();

    DBG_ASSERT( mxLayer!=NULL, "IosSalGraphics::copyArea() for non-layered graphics" );

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
        const CGSize aSrcSize = { nSrcWidth, nSrcHeight };
        xSrcLayer = ::CGLayerCreateWithContext( xCopyContext, aSrcSize, NULL );
        const CGContextRef xSrcContext = CGLayerGetContext( xSrcLayer );
        CGPoint aSrcPoint = { -nSrcX, -nSrcY };
        if( IsFlipped() )
        {
            ::CGContextTranslateCTM( xSrcContext, 0, +nSrcHeight );
            ::CGContextScaleCTM( xSrcContext, +1, -1 );
            aSrcPoint.y = (nSrcY + nSrcHeight) - mnHeight;
        }
        ::CGContextDrawLayerAtPoint( xSrcContext, aSrcPoint, mxLayer );
    }

    // draw at new destination
    const CGPoint aDstPoint = { +nDstX, +nDstY };
    ::CGContextDrawLayerAtPoint( xCopyContext, aDstPoint, xSrcLayer );

    // cleanup
    if( xSrcLayer != mxLayer )
    {
        CGLayerRelease( xSrcLayer );
    }
    // mark the destination rectangle as updated
    RefreshRect( nDstX, nDstY, nSrcWidth, nSrcHeight );

}

void IosSalGraphics::copyResolution( IosSalGraphics& rGraphics )
{
    if( !rGraphics.mnRealDPIY && rGraphics.mbWindow && rGraphics.mpFrame )
    {
        rGraphics.initResolution( rGraphics.mpFrame->mpWindow );
    }
    mnRealDPIX = rGraphics.mnRealDPIX;
    mnRealDPIY = rGraphics.mnRealDPIY;
    mfFakeDPIScale = rGraphics.mfFakeDPIScale;
}

bool IosSalGraphics::drawAlphaBitmap( const SalTwoRect& rTR,
                                       const SalBitmap& rSrcBitmap,
                                       const SalBitmap& rAlphaBmp )
{
    // An image mask can't have a depth > 8 bits (should be 1 to 8 bits)
    if( rAlphaBmp.GetBitCount() > 8 )
    {
        return false;
    }
    // are these two tests really necessary? (see vcl/unx/source/gdi/salgdi2.cxx)
    // horizontal/vertical mirroring not implemented yet
    if( rTR.mnDestWidth < 0 || rTR.mnDestHeight < 0 )
    {
        return false;
    }

    const IosSalBitmap& rSrcSalBmp = static_cast<const IosSalBitmap&>(rSrcBitmap);
    const IosSalBitmap& rMaskSalBmp = static_cast<const IosSalBitmap&>(rAlphaBmp);
    CGImageRef xMaskedImage = rSrcSalBmp.CreateWithMask( rMaskSalBmp, rTR.mnSrcX,
                                                         rTR.mnSrcY, rTR.mnSrcWidth,
                                                         rTR.mnSrcHeight );
    if( !xMaskedImage )
    {
        return false;
    }
    if ( CheckContext() )
    {
        const CGRect aDstRect = {{rTR.mnDestX, rTR.mnDestY}, {rTR.mnDestWidth, rTR.mnDestHeight}};
        CGContextDrawImage( mrContext, aDstRect, xMaskedImage );
        RefreshRect( aDstRect );
    }

    CGImageRelease(xMaskedImage);
    return true;
}

bool IosSalGraphics::drawAlphaRect( long nX, long nY, long nWidth,
                                     long nHeight, sal_uInt8 nTransparency )
{
    if( !CheckContext() )
    {
        return true;
    }
    // save the current state
    CGContextSaveGState( mrContext );
    CGContextSetAlpha( mrContext, (100-nTransparency) * (1.0/100) );

    CGRect aRect = {{nX,nY},{nWidth-1,nHeight-1}};
    if( IsPenVisible() )
    {
        aRect.origin.x += 0.5;
        aRect.origin.y += 0.5;
    }

    CGContextBeginPath( mrContext );
    CGContextAddRect( mrContext, aRect );
    CGContextDrawPath( mrContext, kCGPathFill );

    // restore state
    CGContextRestoreGState(mrContext);
    RefreshRect( aRect );
    return true;
}

void IosSalGraphics::drawBitmap( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap )
{
    if( !CheckContext() )
    {
        return;
    }
    const IosSalBitmap& rBitmap = static_cast<const IosSalBitmap&>(rSalBitmap);
    CGImageRef xImage = rBitmap.CreateCroppedImage( (int)pPosAry->mnSrcX, (int)pPosAry->mnSrcY,
                                                    (int)pPosAry->mnSrcWidth, (int)pPosAry->mnSrcHeight );
    if( !xImage )
    {
        return;
    }
    const CGRect aDstRect = {{pPosAry->mnDestX, pPosAry->mnDestY},
                             {pPosAry->mnDestWidth, pPosAry->mnDestHeight}};
    CGContextDrawImage( mrContext, aDstRect, xImage );
    CGImageRelease( xImage );
    RefreshRect( aDstRect );
}

void IosSalGraphics::drawBitmap( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap,SalColor )
{
    OSL_FAIL("not implemented for color masking!");
    drawBitmap( pPosAry, rSalBitmap );
}

void IosSalGraphics::drawBitmap( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap,
                                  const SalBitmap& rTransparentBitmap )
{
    if( !CheckContext() )
    {
        return;
    }
    const IosSalBitmap& rBitmap = static_cast<const IosSalBitmap&>(rSalBitmap);
    const IosSalBitmap& rMask = static_cast<const IosSalBitmap&>(rTransparentBitmap);
    CGImageRef xMaskedImage( rBitmap.CreateWithMask( rMask, pPosAry->mnSrcX, pPosAry->mnSrcY,
                                                     pPosAry->mnSrcWidth, pPosAry->mnSrcHeight ) );
    if( !xMaskedImage )
    {
        return;
    }
    const CGRect aDstRect = {{pPosAry->mnDestX, pPosAry->mnDestY},
                             {pPosAry->mnDestWidth, pPosAry->mnDestHeight}};
    CGContextDrawImage( mrContext, aDstRect, xMaskedImage );
    CGImageRelease( xMaskedImage );
    RefreshRect( aDstRect );
}

sal_Bool IosSalGraphics::drawEPS( long /*nX*/, long /*nY*/, long /*nWidth*/, long /*nHeight*/,
    void* /*pEpsData*/, sal_uLong /*nByteCount*/ )
{
  return sal_False;
}

void IosSalGraphics::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
    if( nX1 == nX2 && nY1 == nY2 )
    {
        // #i109453# platform independent code expects at least one pixel to be drawn
        drawPixel( nX1, nY1 );
        return;
    }

    if( !CheckContext() )
    {
        return;
    }
    CGContextBeginPath( mrContext );
    CGContextMoveToPoint( mrContext, static_cast<float>(nX1)+0.5, static_cast<float>(nY1)+0.5 );
    CGContextAddLineToPoint( mrContext, static_cast<float>(nX2)+0.5, static_cast<float>(nY2)+0.5 );
    CGContextDrawPath( mrContext, kCGPathStroke );

    Rectangle aRefreshRect( nX1, nY1, nX2, nY2 );
}

void IosSalGraphics::drawMask( const SalTwoRect* pPosAry,
                                const SalBitmap& rSalBitmap,
                                SalColor nMaskColor )
{
    if( !CheckContext() )
    {
        return;
    }
    const IosSalBitmap& rBitmap = static_cast<const IosSalBitmap&>(rSalBitmap);
    CGImageRef xImage = rBitmap.CreateColorMask( pPosAry->mnSrcX, pPosAry->mnSrcY,
                                                 pPosAry->mnSrcWidth, pPosAry->mnSrcHeight,
                                                 nMaskColor );
    if( !xImage )
    {
        return;
    }
    const CGRect aDstRect = {{pPosAry->mnDestX, pPosAry->mnDestY},
                             {pPosAry->mnDestWidth, pPosAry->mnDestHeight}};
    CGContextDrawImage( mrContext, aDstRect, xImage );
    CGImageRelease( xImage );
    RefreshRect( aDstRect );
}

void IosSalGraphics::drawPixel( long nX, long nY )
{
    // draw pixel with current line color
    ImplDrawPixel( nX, nY, maLineColor );
}

void IosSalGraphics::drawPixel( long nX, long nY, SalColor nSalColor )
{
    const RGBAColor aPixelColor( nSalColor );
    ImplDrawPixel( nX, nY, aPixelColor );
}

bool IosSalGraphics::drawPolyLine( const ::basegfx::B2DPolygon& rPolyLine,
                                    double fTransparency,
                                    const ::basegfx::B2DVector& rLineWidths,
                                    basegfx::B2DLineJoin eLineJoin )
{
    // short circuit if there is nothing to do
    const int nPointCount = rPolyLine.count();
    if( nPointCount <= 0 )
    {
        return true;
    }
    // reject requests that cannot be handled yet
    if( rLineWidths.getX() != rLineWidths.getY() )
    {
        return false;
    }
    // #i101491# Ios does not support B2DLINEJOIN_NONE; return false to use
    // the fallback (own geometry preparation)
    // #i104886# linejoin-mode and thus the above only applies to "fat" lines
    if( (basegfx::B2DLINEJOIN_NONE == eLineJoin) &&
        (rLineWidths.getX() > 1.3) )
    {
        return false;
    }
    // setup line attributes
    CGLineJoin aCGLineJoin = kCGLineJoinMiter;
    switch( eLineJoin )
    {
    case ::basegfx::B2DLINEJOIN_NONE: aCGLineJoin = /*TODO?*/kCGLineJoinMiter; break;
    case ::basegfx::B2DLINEJOIN_MIDDLE: aCGLineJoin = /*TODO?*/kCGLineJoinMiter; break;
    case ::basegfx::B2DLINEJOIN_BEVEL: aCGLineJoin = kCGLineJoinBevel; break;
    case ::basegfx::B2DLINEJOIN_MITER: aCGLineJoin = kCGLineJoinMiter; break;
    case ::basegfx::B2DLINEJOIN_ROUND: aCGLineJoin = kCGLineJoinRound; break;
    }

    // setup poly-polygon path
    CGMutablePathRef xPath = CGPathCreateMutable();
    AddPolygonToPath( xPath, rPolyLine, rPolyLine.isClosed(), !getAntiAliasB2DDraw(), true );

    const CGRect aRefreshRect = CGPathGetBoundingBox( xPath );
#ifndef NO_I97317_WORKAROUND
    // #i97317# workaround for Quartz having problems with drawing small polygons
    if( ! ((aRefreshRect.size.width <= 0.125) && (aRefreshRect.size.height <= 0.125)) )
#endif
    {
        // use the path to prepare the graphics context
        CGContextSaveGState( mrContext );
        CGContextAddPath( mrContext, xPath );
        // draw path with antialiased line
        CGContextSetShouldAntialias( mrContext, true );
        CGContextSetAlpha( mrContext, 1.0 - fTransparency );
        CGContextSetLineJoin( mrContext, aCGLineJoin );
        CGContextSetLineWidth( mrContext, rLineWidths.getX() );
        CGContextDrawPath( mrContext, kCGPathStroke );
        CGContextRestoreGState( mrContext );

        // mark modified rectangle as updated
        RefreshRect( aRefreshRect );
    }

    CGPathRelease( xPath );

    return true;
}

sal_Bool IosSalGraphics::drawPolyLineBezier( sal_uLong, const SalPoint*, const sal_uInt8* )
{
    return sal_False;
}

bool IosSalGraphics::drawPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly,
    double fTransparency )
{
    // short circuit if there is nothing to do
    const int nPolyCount = rPolyPoly.count();
    if( nPolyCount <= 0 )
    {
        return true;
    }
    // ignore invisible polygons
    if( (fTransparency >= 1.0) || (fTransparency < 0) )
    {
        return true;
    }
    // setup poly-polygon path
    CGMutablePathRef xPath = CGPathCreateMutable();
    for( int nPolyIdx = 0; nPolyIdx < nPolyCount; ++nPolyIdx )
    {
        const ::basegfx::B2DPolygon rPolygon = rPolyPoly.getB2DPolygon( nPolyIdx );
        AddPolygonToPath( xPath, rPolygon, true, !getAntiAliasB2DDraw(), IsPenVisible() );
    }

    const CGRect aRefreshRect = CGPathGetBoundingBox( xPath );
#ifndef NO_I97317_WORKAROUND
    // #i97317# workaround for Quartz having problems with drawing small polygons
    if( ! ((aRefreshRect.size.width <= 0.125) && (aRefreshRect.size.height <= 0.125)) )
#endif
    {
        // use the path to prepare the graphics context
        CGContextSaveGState( mrContext );
        CGContextBeginPath( mrContext );
        CGContextAddPath( mrContext, xPath );

        // draw path with antialiased polygon
        CGContextSetShouldAntialias( mrContext, true );
        CGContextSetAlpha( mrContext, 1.0 - fTransparency );
        CGContextDrawPath( mrContext, kCGPathEOFillStroke );
        CGContextRestoreGState( mrContext );

        // mark modified rectangle as updated
        RefreshRect( aRefreshRect );
    }

    CGPathRelease( xPath );

    return true;
}

void IosSalGraphics::drawPolyPolygon( sal_uLong nPolyCount, const sal_uLong *pPoints, PCONSTSALPOINT  *ppPtAry )
{
    if( nPolyCount <= 0 )
        return;
    if( !CheckContext() )
        return;

    // find bound rect
    long leftX = 0, topY = 0, maxWidth = 0, maxHeight = 0;
    getBoundRect( pPoints[0], ppPtAry[0], leftX, topY, maxWidth, maxHeight );
    for( sal_uLong n = 1; n < nPolyCount; n++ )
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
        return;
    }
    // convert to CGPath
    CGContextBeginPath( mrContext );
    if( IsPenVisible() )
    {
        for( sal_uLong nPoly = 0; nPoly < nPolyCount; nPoly++ )
        {
            const sal_uLong nPoints = pPoints[nPoly];
            if( nPoints > 1 )
            {
                const SalPoint *pPtAry = ppPtAry[nPoly];
                float fX, fY;
                alignLinePoint( pPtAry, fX, fY );
                CGContextMoveToPoint( mrContext, fX, fY );
                pPtAry++;
                for( sal_uLong nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++ )
                {
                    alignLinePoint( pPtAry, fX, fY );
                    CGContextAddLineToPoint( mrContext, fX, fY );
                }
                CGContextClosePath(mrContext);
            }
        }
    }
    else
    {
        for( sal_uLong nPoly = 0; nPoly < nPolyCount; nPoly++ )
        {
            const sal_uLong nPoints = pPoints[nPoly];
            if( nPoints > 1 )
            {
                const SalPoint *pPtAry = ppPtAry[nPoly];
                CGContextMoveToPoint( mrContext, pPtAry->mnX, pPtAry->mnY );
                pPtAry++;
                for( sal_uLong nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++ )
                {
                    CGContextAddLineToPoint( mrContext, pPtAry->mnX, pPtAry->mnY );
                }
                CGContextClosePath(mrContext);
            }
        }
    }

    CGContextDrawPath( mrContext, eMode );

    RefreshRect( leftX, topY, maxWidth, maxHeight );
}

void IosSalGraphics::drawPolygon( sal_uLong nPoints, const SalPoint *pPtAry )
{
    if( nPoints <= 1 )
        return;
    if( !CheckContext() )
        return;

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
        return;
    }
    CGContextBeginPath( mrContext );

    if( IsPenVisible() )
    {
        float fX, fY;
        alignLinePoint( pPtAry, fX, fY );
        CGContextMoveToPoint( mrContext, fX, fY );
        pPtAry++;
        for( sal_uLong nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++ )
        {
            alignLinePoint( pPtAry, fX, fY );
            CGContextAddLineToPoint( mrContext, fX, fY );
        }
    }
    else
    {
        CGContextMoveToPoint( mrContext, pPtAry->mnX, pPtAry->mnY );
        pPtAry++;
        for( sal_uLong nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++ )
        {
            CGContextAddLineToPoint( mrContext, pPtAry->mnX, pPtAry->mnY );
        }
    }

    CGContextDrawPath( mrContext, eMode );
    RefreshRect( nX, nY, nWidth, nHeight );
}

sal_Bool IosSalGraphics::drawPolygonBezier( sal_uLong, const SalPoint*, const sal_uInt8* )
{
    return sal_False;
}

sal_Bool IosSalGraphics::drawPolyPolygonBezier( sal_uLong, const sal_uLong*,
                                                 const SalPoint* const*, const sal_uInt8* const* )
{
    return sal_False;
}

void IosSalGraphics::drawRect( long nX, long nY, long nWidth, long nHeight )
{
    if( !CheckContext() )
    {
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
        CGContextFillRect( mrContext, aRect );
    }
    if( IsPenVisible() )
    {
        CGContextStrokeRect( mrContext, aRect );
    }
    RefreshRect( nX, nY, nWidth, nHeight );
}


void IosSalGraphics::drawPolyLine( sal_uLong nPoints, const SalPoint *pPtAry )
{
    if( nPoints < 1 )
    {
        return;
    }
    if( !CheckContext() )
    {
        return;
    }

    long nX = 0, nY = 0, nWidth = 0, nHeight = 0;
    getBoundRect( nPoints, pPtAry, nX, nY, nWidth, nHeight );

    float fX, fY;
    CGContextBeginPath( mrContext );
    alignLinePoint( pPtAry, fX, fY );
    CGContextMoveToPoint( mrContext, fX, fY );
    pPtAry++;
    for( sal_uLong nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++ )
    {
        alignLinePoint( pPtAry, fX, fY );
        CGContextAddLineToPoint( mrContext, fX, fY );
    }
    CGContextDrawPath( mrContext, kCGPathStroke );

    RefreshRect( nX, nY, nWidth, nHeight );
}

sal_uInt16 IosSalGraphics::GetBitCount() const
{
    sal_uInt16 nBits = mnBitmapDepth ? mnBitmapDepth : 32;//24;
    return nBits;
}

SalBitmap* IosSalGraphics::getBitmap( long  nX, long  nY, long  nDX, long  nDY )
{
    DBG_ASSERT( mxLayer, "IosSalGraphics::getBitmap() with no layer" );

    ApplyXorContext();

    IosSalBitmap* pBitmap = new IosSalBitmap;
    if( !pBitmap->Create( mxLayer, mnBitmapDepth, nX, nY, nDX, nDY, !mbWindow ) )
    {
        delete pBitmap;
        pBitmap = NULL;
    }

    return pBitmap;
}

SystemGraphicsData IosSalGraphics::GetGraphicsData() const
{
    SystemGraphicsData aRes;
    aRes.nSize = sizeof(aRes);
    aRes.rCGContext = mrContext;
    return aRes;
}

long IosSalGraphics::GetGraphicsWidth() const
{
    long w = 0;
    if( mrContext && (mbWindow || mbVirDev) )
    {
        w = mnWidth;
    }

    if( w == 0 )
    {
        if( mbWindow && mpFrame )
        {
            w = mpFrame->maGeometry.nWidth;
        }
    }
    return w;
}

SalColor IosSalGraphics::getPixel( long nX, long nY )
{
    // return default value on printers or when out of bounds
    if( !mxLayer || (nX < 0) || (nX >= mnWidth) ||
        (nY < 0) || (nY >= mnHeight))
    {
        return COL_BLACK;
    }
    // prepare creation of matching a CGBitmapContext
    CGColorSpaceRef aCGColorSpace = GetSalData()->mxRGBSpace;
    CGBitmapInfo aCGBmpInfo = kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Big;
#if __BIG_ENDIAN__
    struct{ unsigned char b, g, r, a; } aPixel;
#else
    struct{ unsigned char a, r, g, b; } aPixel;
#endif

    // create a one-pixel bitmap context
    // TODO: is it worth to cache it?
    CGContextRef xOnePixelContext =
        ::CGBitmapContextCreate( &aPixel, 1, 1, 8, sizeof(aPixel),
                                 aCGColorSpace, aCGBmpInfo );

    // update this graphics layer
    ApplyXorContext();

    // copy the requested pixel into the bitmap context
    if( IsFlipped() )
    {
        nY = mnHeight - nY;
    }
    const CGPoint aCGPoint = {-nX, -nY};
    CGContextDrawLayerAtPoint( xOnePixelContext, aCGPoint, mxLayer );
    CGContextRelease( xOnePixelContext );

    SalColor nSalColor = MAKE_SALCOLOR( aPixel.r, aPixel.g, aPixel.b );
    return nSalColor;
}

void IosSalGraphics::GetResolution( long& rDPIX, long& rDPIY )
{
    if( !mnRealDPIY )
    {
        initResolution( (mbWindow && mpFrame) ? mpFrame->mpWindow : nil );
    }

    rDPIX = static_cast<long>(mfFakeDPIScale * mnRealDPIX);
    rDPIY = static_cast<long>(mfFakeDPIScale * mnRealDPIY);
}

void IosSalGraphics::ImplDrawPixel( long nX, long nY, const RGBAColor& rColor )
{
    if( !CheckContext() )
    {
        return;
    }
    // overwrite the fill color
    CGContextSetFillColor( mrContext, rColor.AsArray() );
    // draw 1x1 rect, there is no pixel drawing in Quartz
    CGRect aDstRect = {{nX,nY,},{1,1}};
    CGContextFillRect( mrContext, aDstRect );
    RefreshRect( aDstRect );
    // reset the fill color
    CGContextSetFillColor( mrContext, maFillColor.AsArray() );
}

void IosSalGraphics::initResolution( UIWindow* )
{
    // #i100617# read DPI only once; there is some kind of weird caching going on
    // if the main screen changes
    // FIXME: this is really unfortunate and needs to be investigated

    SalData* pSalData = GetSalData();
    if( pSalData->mnDPIX == 0 || pSalData->mnDPIY == 0 )
    {
        UIScreen* pScreen = [UIScreen mainScreen];

        mnRealDPIX = mnRealDPIY = 160;
        if( pScreen )
        {
            mnRealDPIX *= [pScreen scale];
            mnRealDPIY *= [pScreen scale];
        }
        else
        {
            OSL_FAIL( "no screen found" );
        }

        pSalData->mnDPIX = mnRealDPIX;
        pSalData->mnDPIY = mnRealDPIY;
    }
    else
    {
        mnRealDPIX = pSalData->mnDPIX;
        mnRealDPIY = pSalData->mnDPIY;
    }

    mfFakeDPIScale = 1.0;
}

void IosSalGraphics::invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags )
{
    if ( CheckContext() )
    {
        CGRect aCGRect = CGRectMake( nX, nY, nWidth, nHeight);
        CGContextSaveGState(mrContext);

        if ( nFlags & SAL_INVERT_TRACKFRAME )
        {
            const float dashLengths[2]  = { 4.0, 4.0 };     // for drawing dashed line
            CGContextSetBlendMode( mrContext, kCGBlendModeDifference );
            CGContextSetRGBStrokeColor ( mrContext, 1.0, 1.0, 1.0, 1.0 );
            CGContextSetLineDash ( mrContext, 0, dashLengths, 2 );
            CGContextSetLineWidth( mrContext, 2.0);
            CGContextStrokeRect ( mrContext, aCGRect );
        }
        else if ( nFlags & SAL_INVERT_50 )
        {
            //CGContextSetAllowsAntialiasing( mrContext, false );
            CGContextSetBlendMode(mrContext, kCGBlendModeDifference);
            CGContextAddRect( mrContext, aCGRect );
            Pattern50Fill();
        }
        else // just invert
        {
            CGContextSetBlendMode(mrContext, kCGBlendModeDifference);
            CGContextSetRGBFillColor ( mrContext,1.0, 1.0, 1.0 , 1.0 );
            CGContextFillRect ( mrContext, aCGRect );
        }
        CGContextRestoreGState( mrContext);
        RefreshRect( aCGRect );
    }
}

void IosSalGraphics::invert( sal_uLong nPoints, const SalPoint*  pPtAry, SalInvert nSalFlags )
{
    CGPoint* CGpoints ;
    if ( CheckContext() )
    {
        CGContextSaveGState(mrContext);
        CGpoints = makeCGptArray(nPoints,pPtAry);
        CGContextAddLines ( mrContext, CGpoints, nPoints );
        if ( nSalFlags & SAL_INVERT_TRACKFRAME )
        {
            const float dashLengths[2]  = { 4.0, 4.0 };     // for drawing dashed line
            CGContextSetBlendMode( mrContext, kCGBlendModeDifference );
            CGContextSetRGBStrokeColor ( mrContext, 1.0, 1.0, 1.0, 1.0 );
            CGContextSetLineDash ( mrContext, 0, dashLengths, 2 );
            CGContextSetLineWidth( mrContext, 2.0);
            CGContextStrokePath ( mrContext );
        }
        else if ( nSalFlags & SAL_INVERT_50 )
        {
            CGContextSetBlendMode(mrContext, kCGBlendModeDifference);
            Pattern50Fill();
        }
        else // just invert
        {
            CGContextSetBlendMode( mrContext, kCGBlendModeDifference );
            CGContextSetRGBFillColor( mrContext, 1.0, 1.0, 1.0, 1.0 );
            CGContextFillPath( mrContext );
        }
        const CGRect aRefreshRect = CGContextGetClipBoundingBox(mrContext);
        CGContextRestoreGState( mrContext);
        delete []  CGpoints;
        RefreshRect( aRefreshRect );
    }
}

void IosSalGraphics::Pattern50Fill()
{
    static const float aFillCol[4] = { 1,1,1,1 };
    static const CGPatternCallbacks aCallback = { 0, &DrawPattern50, NULL };
    if( ! GetSalData()->mxP50Space )
    {
        GetSalData()->mxP50Space = CGColorSpaceCreatePattern( GetSalData()->mxRGBSpace );
    }
    if( ! GetSalData()->mxP50Pattern )
    {
        GetSalData()->mxP50Pattern = CGPatternCreate( NULL, CGRectMake( 0, 0, 4, 4 ),
                                                      CGAffineTransformIdentity, 4, 4,
                                                      kCGPatternTilingConstantSpacing,
                                                      false, &aCallback );
    }
    CGContextSetFillColorSpace( mrContext, GetSalData()->mxP50Space );
    CGContextSetFillPattern( mrContext, GetSalData()->mxP50Pattern, aFillCol );
    CGContextFillPath( mrContext );
}


void IosSalGraphics::ResetClipRegion()
{
    // release old path and indicate no clipping
    if( mxClipPath )
    {
        CGPathRelease( mxClipPath );
        mxClipPath = NULL;
    }
    if( CheckContext() )
    {
        SetState();
    }
}

void IosSalGraphics::SetLineColor()
{
    maLineColor.SetAlpha( 0.0 );   // transparent
    if( CheckContext() )
    {
        CGContextSetStrokeColor( mrContext, maLineColor.AsArray() );
    }
}

void IosSalGraphics::SetLineColor( SalColor nSalColor )
{
    maLineColor = RGBAColor( nSalColor );
    if( CheckContext() )
    {
        CGContextSetStrokeColor( mrContext, maLineColor.AsArray() );
    }
}

void IosSalGraphics::SetFillColor()
{
    maFillColor.SetAlpha( 0.0 );   // transparent
    if( CheckContext() )
    {
        CGContextSetFillColor( mrContext, maFillColor.AsArray() );
    }
}

void IosSalGraphics::SetFillColor( SalColor nSalColor )
{
    maFillColor = RGBAColor( nSalColor );
    if( CheckContext() )
    {
        CGContextSetFillColor( mrContext, maFillColor.AsArray() );
    }
}

bool IosSalGraphics::supportsOperation( OutDevSupportType eType ) const
{
    bool bRet = false;
    switch( eType )
    {
    case OutDevSupport_TransparentRect:
    case OutDevSupport_B2DClip:
    case OutDevSupport_B2DDraw:
        bRet = true;
        break;
    default: break;
    }
    return bRet;
}

bool IosSalGraphics::setClipRegion( const Region& i_rClip )
{
    // release old clip path
    if( mxClipPath )
    {
        CGPathRelease( mxClipPath );
        mxClipPath = NULL;
    }
    mxClipPath = CGPathCreateMutable();

    // set current path, either as polypolgon or sequence of rectangles
    if( i_rClip.HasPolyPolygon() )
    {
        basegfx::B2DPolyPolygon aClip( const_cast<Region&>(i_rClip).ConvertToB2DPolyPolygon() );
        AddPolyPolygonToPath( mxClipPath, aClip, !getAntiAliasB2DDraw(), false );
    }
    else
    {
        long nX, nY, nW, nH;
        ImplRegionInfo aInfo;
        bool bRegionRect = i_rClip.ImplGetFirstRect(aInfo, nX, nY, nW, nH );
        while( bRegionRect )
        {
            if( nW && nH )
            {
                CGRect aRect = {{nX,nY}, {nW,nH}};
                CGPathAddRect( mxClipPath, NULL, aRect );
            }
            bRegionRect = i_rClip.ImplGetNextRect( aInfo, nX, nY, nW, nH );
        }
    }
    // set the current path as clip region
    if( CheckContext() )
    {
        SetState();
    }
    return true;
}

void IosSalGraphics::SetROPFillColor( SalROPColor nROPColor )
{
    if( ! mbPrinter )
        SetFillColor( ImplGetROPSalColor( nROPColor ) );
}

void IosSalGraphics::SetROPLineColor( SalROPColor nROPColor )
{
    if( ! mbPrinter )
        SetLineColor( ImplGetROPSalColor( nROPColor ) );
}

void IosSalGraphics::SetXORMode( bool bSet, bool bInvertOnly )
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
    else if( bSet && bInvertOnly && mnXorMode == 0)
    {
        CGContextSetBlendMode( mrContext, kCGBlendModeDifference );
        mnXorMode = 2;
        return;
    }

    if( (mpXorEmulation == NULL) && !bSet )
    {
        return;
    }
    if( (mpXorEmulation != NULL) && (bSet == mpXorEmulation->IsEnabled()) )
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

void IosSalGraphics::updateResolution()
{
    DBG_ASSERT( mbWindow, "updateResolution on inappropriate graphics" );

    initResolution( (mbWindow && mpFrame) ?  mpFrame->mpWindow : nil );
}


// -----------------------------------------------------------

XorEmulation::XorEmulation()
:   m_xTargetLayer( NULL )
,   m_xTargetContext( NULL )
,   m_xMaskContext( NULL )
,   m_xTempContext( NULL )
,   m_pMaskBuffer( NULL )
,   m_pTempBuffer( NULL )
,   m_nBufferLongs( 0 )
,   m_bIsEnabled( false )
{}

XorEmulation::~XorEmulation()
{
    Disable();
    SetTarget( 0, 0, 0, NULL, NULL );
}

void XorEmulation::SetTarget( int nWidth, int nHeight, int nTargetDepth,
                              CGContextRef xTargetContext, CGLayerRef xTargetLayer )
{
    // prepare to replace old mask+temp context
    if( m_xMaskContext )
    {
        // cleanup the mask context
        CGContextRelease( m_xMaskContext );
        delete[] m_pMaskBuffer;
        m_xMaskContext = NULL;
        m_pMaskBuffer = NULL;

        // cleanup the temp context if needed
        if( m_xTempContext )
        {
            CGContextRelease( m_xTempContext );
            delete[] m_pTempBuffer;
            m_xTempContext = NULL;
            m_pTempBuffer = NULL;
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
    m_xMaskContext = ::CGBitmapContextCreate( m_pMaskBuffer,
                                              nWidth, nHeight,
                                              nBitsPerComponent, nBytesPerRow,
                                              aCGColorSpace, aCGBmpInfo );
    // reset the XOR mask to black
    memset( m_pMaskBuffer, 0, m_nBufferLongs * sizeof(sal_uLong) );

    // a bitmap context will be needed for manual XORing
    // create one unless the target context is a bitmap context
    if( nTargetDepth )
        m_pTempBuffer = (sal_uLong*)CGBitmapContextGetData( m_xTargetContext );
    if( !m_pTempBuffer )
    {
        // create a bitmap context matching to the target context
        m_pTempBuffer = new sal_uLong[ m_nBufferLongs ];
        m_xTempContext = ::CGBitmapContextCreate( m_pTempBuffer,
                                                  nWidth, nHeight,
                                                  nBitsPerComponent, nBytesPerRow,
                                                  aCGColorSpace, aCGBmpInfo );
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
    if( !IsEnabled() )
    {
        return false;
    }
    // update the temp bitmap buffer if needed
    if( m_xTempContext )
    {
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
        const int nWidth  = (int)CGImageGetWidth( xXorImage );
        const int nHeight = (int)CGImageGetHeight( xXorImage );
        // TODO: update minimal changerect
        const CGRect aFullRect = {{0,0},{nWidth,nHeight}};
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
