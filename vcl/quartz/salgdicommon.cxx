/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "sal/config.h"

#include <cstring>

#include <sal/types.h>
#include <osl/endian.h>
#include <osl/file.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>

#include "quartz/salbmp.h"
#include "quartz/salgdi.h"

#include "fontsubset.hxx"
#include "sft.hxx"

#ifdef IOS
#include "saldatabasic.hxx"
#include "headless/svpbmp.hxx"
#include <basegfx/range/b2ibox.hxx>
#endif

using namespace vcl;

typedef std::vector<unsigned char> ByteVector;

static const basegfx::B2DPoint aHalfPointOfs ( 0.5, 0.5 );

static void AddPolygonToPath( CGMutablePathRef xPath,
                              const ::basegfx::B2DPolygon& rPolygon,
                              bool bClosePath, bool bPixelSnap, bool bLineDraw )
{
    
    const int nPointCount = rPolygon.count();
    if( nPointCount <= 0 )
    {
        return;
    }
    (void)bPixelSnap; 
    const CGAffineTransform* pTransform = NULL;

    const bool bHasCurves = rPolygon.areControlPointsUsed();
    for( int nPointIdx = 0, nPrevIdx = 0;; nPrevIdx = nPointIdx++ )
    {
        int nClosedIdx = nPointIdx;
        if( nPointIdx >= nPointCount )
        {
            
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
            
            aPoint.setX( basegfx::fround( aPoint.getX() ) );
            aPoint.setY( basegfx::fround( aPoint.getY() ) );
        }

        if( bLineDraw )
        {
            aPoint += aHalfPointOfs;
        }
        if( !nPointIdx )
        {
            
            CGPathMoveToPoint( xPath, pTransform, aPoint.getX(), aPoint.getY() );
            continue;
        }

        bool bPendingCurve = false;
        if( bHasCurves )
        {
            bPendingCurve = rPolygon.isNextControlPointUsed( nPrevIdx );
            bPendingCurve |= rPolygon.isPrevControlPointUsed( nClosedIdx );
        }

        if( !bPendingCurve )    
        {
            CGPathAddLineToPoint( xPath, pTransform, aPoint.getX(), aPoint.getY() );
        }
        else                        
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

bool AquaSalGraphics::CreateFontSubset( const OUString& rToFile,
                                            const PhysicalFontFace* pFontData,
                                            sal_GlyphId* pGlyphIds, sal_uInt8* pEncoding,
                                            sal_Int32* pGlyphWidths, int nGlyphCount,
                                            FontSubsetInfo& rInfo )
{
    

    
    OUString aSysPath;
    if( osl_File_E_None != osl_getSystemPathFromFileURL( rToFile.pData, &aSysPath.pData ) )
        return false;
    const rtl_TextEncoding aThreadEncoding = osl_getThreadTextEncoding();
    const OString aToFile( OUStringToOString( aSysPath, aThreadEncoding ) );

    
    ByteVector aBuffer;
    bool bCffOnly = false;
    if( !GetRawFontData( pFontData, aBuffer, &bCffOnly ) )
        return false;

    
    if( bCffOnly )
    {
        
        ByteCount nCffLen = aBuffer.size();
        rInfo.LoadFont( FontSubsetInfo::CFF_FONT, &aBuffer[0], nCffLen );

        

        
        FILE* pOutFile = fopen( aToFile.getStr(), "wb" );
        bool bRC = rInfo.CreateFontSubset( FontSubsetInfo::TYPE1_PFB, pOutFile, NULL,
            pGlyphIds, pEncoding, nGlyphCount, pGlyphWidths );
        fclose( pOutFile );
        return bRC;
    }

    
    
    

    
    TrueTypeFont* pSftFont = NULL;
    int nRC = ::OpenTTFontBuffer( (void*)&aBuffer[0], aBuffer.size(), 0, &pSftFont);
    if( nRC != SF_OK )
        return false;

    
    TTGlobalFontInfo aTTInfo;
    ::GetTTGlobalFontInfo( pSftFont, &aTTInfo );
    rInfo.m_nFontType   = FontSubsetInfo::SFNT_TTF;
    rInfo.m_aPSName     = OUString(
        aTTInfo.psname, std::strlen(aTTInfo.psname), RTL_TEXTENCODING_UTF8 );
    rInfo.m_aFontBBox   = Rectangle( Point( aTTInfo.xMin, aTTInfo.yMin ),
                                    Point( aTTInfo.xMax, aTTInfo.yMax ) );
    rInfo.m_nCapHeight  = aTTInfo.yMax; 
    rInfo.m_nAscent     = aTTInfo.winAscent;
    rInfo.m_nDescent    = aTTInfo.winDescent;
    
    
    if( !rInfo.m_nAscent )
        rInfo.m_nAscent = +aTTInfo.typoAscender;
    if( !rInfo.m_nAscent )
        rInfo.m_nAscent = +aTTInfo.ascender;
    if( !rInfo.m_nDescent )
        rInfo.m_nDescent = +aTTInfo.typoDescender;
    if( !rInfo.m_nDescent )
        rInfo.m_nDescent = -aTTInfo.descender;

    
    
    int nOrigCount = nGlyphCount;
    sal_uInt16    aShortIDs[ 256 ];
    sal_uInt8 aTempEncs[ 256 ];

    int nNotDef = -1;
    for( int i = 0; i < nGlyphCount; ++i )
    {
        aTempEncs[i] = pEncoding[i];
        sal_GlyphId aGlyphId(pGlyphIds[i] & GF_IDXMASK);
        if( pGlyphIds[i] & GF_ISCHAR )
        {
            bool bVertical = (pGlyphIds[i] & GF_ROTMASK) != 0;
            aGlyphId = ::MapChar( pSftFont, static_cast<sal_uInt16>(aGlyphId), bVertical );
            if( aGlyphId == 0 && pFontData->IsSymbolFont() )
            {
                
                aGlyphId = pGlyphIds[i] & GF_IDXMASK;
                aGlyphId = (aGlyphId & 0xF000) ? (aGlyphId & 0x00FF) : (aGlyphId | 0xF000 );
                aGlyphId = ::MapChar( pSftFont, static_cast<sal_uInt16>(aGlyphId), bVertical );
            }
        }
        aShortIDs[i] = static_cast<sal_uInt16>( aGlyphId );
        if( !aGlyphId )
            if( nNotDef < 0 )
                nNotDef = i; 
    }

    if( nNotDef != 0 )
    {
        
        if( nNotDef < 0 )
            nNotDef = nGlyphCount++;

        
        aShortIDs[ nNotDef ] = aShortIDs[0];
        aTempEncs[ nNotDef ] = aTempEncs[0];
        aShortIDs[0] = 0;
        aTempEncs[0] = 0;
    }
    DBG_ASSERT( nGlyphCount < 257, "too many glyphs for subsetting" );

    
    const bool bVertical = false;

    
    
    TTSimpleGlyphMetrics* pGlyphMetrics =
        ::GetTTSimpleGlyphMetrics( pSftFont, aShortIDs, nGlyphCount, bVertical );
    if( !pGlyphMetrics )
        return false;
    sal_uInt16 nNotDefAdv       = pGlyphMetrics[0].adv;
    pGlyphMetrics[0].adv        = pGlyphMetrics[nNotDef].adv;
    pGlyphMetrics[nNotDef].adv  = nNotDefAdv;
    for( int i = 0; i < nOrigCount; ++i )
        pGlyphWidths[i] = pGlyphMetrics[i].adv;
    free( pGlyphMetrics );

    
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

void AquaSalGraphics::copyBits( const SalTwoRect& rPosAry, SalGraphics *pSrcGraphics )
{

    if( !pSrcGraphics )
    {
        pSrcGraphics = this;
    }
    
    
    if( rPosAry.mnSrcWidth <= 0
        || rPosAry.mnSrcHeight <= 0
        || rPosAry.mnDestWidth <= 0
        || rPosAry.mnDestHeight <= 0 )
    {
        return;
    }

    
    /*const*/ AquaSalGraphics* pSrc = static_cast<AquaSalGraphics*>(pSrcGraphics);
    const bool bSameGraphics = (this == pSrc)
#ifdef MACOSX
        || (mbWindow && mpFrame && pSrc->mbWindow && (mpFrame == pSrc->mpFrame))
#endif
        ;
    if( bSameGraphics
    &&  (rPosAry.mnSrcWidth == rPosAry.mnDestWidth)
    &&  (rPosAry.mnSrcHeight == rPosAry.mnDestHeight))
    {
        
        if( (rPosAry.mnSrcX == rPosAry.mnDestX)
        &&  (rPosAry.mnSrcY == rPosAry.mnDestY))
            return;
        
        copyArea( rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnSrcX, rPosAry.mnSrcY,
            rPosAry.mnSrcWidth, rPosAry.mnSrcHeight, 0 );
        return;
    }

    ApplyXorContext();
    pSrc->ApplyXorContext();

    DBG_ASSERT( pSrc->mxLayer!=NULL, "AquaSalGraphics::copyBits() from non-layered graphics" );

    const CGPoint aDstPoint = CGPointMake(+rPosAry.mnDestX - rPosAry.mnSrcX, rPosAry.mnDestY - rPosAry.mnSrcY);
    if( (rPosAry.mnSrcWidth == rPosAry.mnDestWidth &&
         rPosAry.mnSrcHeight == rPosAry.mnDestHeight) &&
        (!mnBitmapDepth || (aDstPoint.x + pSrc->mnWidth) <= mnWidth)
        && pSrc->mxLayer ) 
    {
        
        
        CGContextRef xCopyContext = mrContext;
        if( mpXorEmulation && mpXorEmulation->IsEnabled() )
        {
            if( pSrcGraphics == this )
            {
                xCopyContext = mpXorEmulation->GetTargetContext();
            }
        }
        CGContextSaveGState( xCopyContext );
        const CGRect aDstRect = CGRectMake(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth, rPosAry.mnDestHeight);
        CGContextClipToRect( xCopyContext, aDstRect );

        
        
        if( pSrc->IsFlipped() )
        {
            CGContextTranslateCTM( xCopyContext, 0, +mnHeight ); CGContextScaleCTM( xCopyContext, +1, -1 );
        }
        
        CGContextDrawLayerAtPoint( xCopyContext, aDstPoint, pSrc->mxLayer );
        CGContextRestoreGState( xCopyContext );
        
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
    CGContextAddRects( rContext, aRects, 2 );
    CGContextFillPath( rContext );
}

static void getBoundRect( sal_uInt32 nPoints, const SalPoint *pPtAry, long &rX, long& rY, long& rWidth, long& rHeight )
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


void AquaSalGraphics::ApplyXorContext()
{
    if( !mpXorEmulation )
    {
        return;
    }
    if( mpXorEmulation->UpdateTarget() )
    {
        RefreshRect( 0, 0, mnWidth, mnHeight ); 
    }
}

void AquaSalGraphics::copyArea( long nDstX, long nDstY,long nSrcX, long nSrcY,
                                long nSrcWidth, long nSrcHeight, sal_uInt16 /*nFlags*/ )
{
#ifdef IOS
    if( !mxLayer )
        return;
#endif

    ApplyXorContext();

    DBG_ASSERT( mxLayer!=NULL, "AquaSalGraphics::copyArea() for non-layered graphics" );

    
    
    CGContextRef xCopyContext = mrContext;
    if( mpXorEmulation && mpXorEmulation->IsEnabled() )
    {
        xCopyContext = mpXorEmulation->GetTargetContext();
    }
    
    
    
    
    CGLayerRef xSrcLayer = mxLayer;
    
    {
        const CGSize aSrcSize = CGSizeMake(nSrcWidth, nSrcHeight);
        xSrcLayer = CGLayerCreateWithContext( xCopyContext, aSrcSize, NULL );
        const CGContextRef xSrcContext = CGLayerGetContext( xSrcLayer );
        CGPoint aSrcPoint = CGPointMake(-nSrcX, -nSrcY);
        if( IsFlipped() )
        {
            CGContextTranslateCTM( xSrcContext, 0, +nSrcHeight );
            CGContextScaleCTM( xSrcContext, +1, -1 );
            aSrcPoint.y = (nSrcY + nSrcHeight) - mnHeight;
        }
        CGContextDrawLayerAtPoint( xSrcContext, aSrcPoint, mxLayer );
    }

    
    const CGPoint aDstPoint = CGPointMake(+nDstX, +nDstY);
    CGContextDrawLayerAtPoint( xCopyContext, aDstPoint, xSrcLayer );

    
    if( xSrcLayer != mxLayer )
    {
        CGLayerRelease( xSrcLayer );
    }
    
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

bool AquaSalGraphics::drawAlphaBitmap( const SalTwoRect& rTR,
                                       const SalBitmap& rSrcBitmap,
                                       const SalBitmap& rAlphaBmp )
{
    
    if( rAlphaBmp.GetBitCount() > 8 )
    {
        return false;
    }
    
    
    if( rTR.mnDestWidth < 0 || rTR.mnDestHeight < 0 )
    {
        return false;
    }

    const QuartzSalBitmap& rSrcSalBmp = static_cast<const QuartzSalBitmap&>(rSrcBitmap);
    const QuartzSalBitmap& rMaskSalBmp = static_cast<const QuartzSalBitmap&>(rAlphaBmp);
    CGImageRef xMaskedImage = rSrcSalBmp.CreateWithMask( rMaskSalBmp, rTR.mnSrcX,
                                                         rTR.mnSrcY, rTR.mnSrcWidth,
                                                         rTR.mnSrcHeight );
    if( !xMaskedImage )
    {
        return false;
    }
    if ( CheckContext() )
    {
        const CGRect aDstRect = CGRectMake( rTR.mnDestX, rTR.mnDestY, rTR.mnDestWidth, rTR.mnDestHeight);
        CGContextDrawImage( mrContext, aDstRect, xMaskedImage );
        RefreshRect( aDstRect );
    }

    CGImageRelease(xMaskedImage);
    return true;
}

bool AquaSalGraphics::drawTransformedBitmap(
    const basegfx::B2DPoint& rNull, const basegfx::B2DPoint& rX, const basegfx::B2DPoint& rY,
    const SalBitmap& rSrcBitmap, const SalBitmap* pAlphaBmp )
{
    if( !CheckContext() )
        return true;

    
    CGImageRef xImage = NULL;
    const Size aSize = rSrcBitmap.GetSize();
    const QuartzSalBitmap& rSrcSalBmp = static_cast<const QuartzSalBitmap&>(rSrcBitmap);
    const QuartzSalBitmap* pMaskSalBmp = static_cast<const QuartzSalBitmap*>(pAlphaBmp);
    if( !pMaskSalBmp)
        xImage = rSrcSalBmp.CreateCroppedImage( 0, 0, (int)aSize.Width(), (int)aSize.Height() );
    else
        xImage = rSrcSalBmp.CreateWithMask( *pMaskSalBmp, 0, 0, (int)aSize.Width(), (int)aSize.Height() );
    if( !xImage )
        return false;

    
    
    CGContextSaveGState( mrContext );
    const basegfx::B2DVector aXRel = rX - rNull;
    const basegfx::B2DVector aYRel = rY - rNull;
    const CGAffineTransform aCGMat = CGAffineTransformMake(
        aXRel.getX()/aSize.Width(), aXRel.getY()/aSize.Width(),
        aYRel.getX()/aSize.Height(), aYRel.getY()/aSize.Height(),
        rNull.getX(), rNull.getY());
    CGContextConcatCTM( mrContext, aCGMat );

    
    const CGRect aSrcRect = CGRectMake(0, 0, aSize.Width(), aSize.Height());
    CGContextDrawImage( mrContext, aSrcRect, xImage );
    CGImageRelease( xImage );
    
    CGContextRestoreGState(mrContext);

    
    const CGRect aDstRect = CGRectApplyAffineTransform( aSrcRect, aCGMat );
    RefreshRect( aDstRect );
    return true;
}


bool AquaSalGraphics::drawAlphaRect( long nX, long nY, long nWidth,
                                     long nHeight, sal_uInt8 nTransparency )
{
    if( !CheckContext() )
    {
        return true;
    }
    
    CGContextSaveGState( mrContext );
    CGContextSetAlpha( mrContext, (100-nTransparency) * (1.0/100) );

    CGRect aRect = CGRectMake(nX, nY, nWidth-1, nHeight-1);
    if( IsPenVisible() )
    {
        aRect.origin.x += 0.5;
        aRect.origin.y += 0.5;
    }

    CGContextBeginPath( mrContext );
    CGContextAddRect( mrContext, aRect );
    CGContextDrawPath( mrContext, kCGPathFill );

    
    CGContextRestoreGState(mrContext);
    RefreshRect( aRect );
    return true;
}

void AquaSalGraphics::drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap )
{
    if( !CheckContext() )
    {
        return;
    }
    const QuartzSalBitmap& rBitmap = static_cast<const QuartzSalBitmap&>(rSalBitmap);
    CGImageRef xImage = rBitmap.CreateCroppedImage( (int)rPosAry.mnSrcX, (int)rPosAry.mnSrcY,
                                                    (int)rPosAry.mnSrcWidth, (int)rPosAry.mnSrcHeight );
    if( !xImage )
    {
        return;
    }

    const CGRect aDstRect = CGRectMake(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth, rPosAry.mnDestHeight);
    CGContextDrawImage( mrContext, aDstRect, xImage );
    CGImageRelease( xImage );
    RefreshRect( aDstRect );
}

void AquaSalGraphics::drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap,SalColor )
{
    OSL_FAIL("not implemented for color masking!");
    drawBitmap( rPosAry, rSalBitmap );
}

void AquaSalGraphics::drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap,
                                  const SalBitmap& rTransparentBitmap )
{
    if( !CheckContext() )
    {
        return;
    }
    const QuartzSalBitmap& rBitmap = static_cast<const QuartzSalBitmap&>(rSalBitmap);
    const QuartzSalBitmap& rMask = static_cast<const QuartzSalBitmap&>(rTransparentBitmap);
    CGImageRef xMaskedImage( rBitmap.CreateWithMask( rMask, rPosAry.mnSrcX, rPosAry.mnSrcY,
                                                     rPosAry.mnSrcWidth, rPosAry.mnSrcHeight ) );
    if( !xMaskedImage )
    {
        return;
    }

    const CGRect aDstRect = CGRectMake(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth, rPosAry.mnDestHeight);
    CGContextDrawImage( mrContext, aDstRect, xMaskedImage );
    CGImageRelease( xMaskedImage );
    RefreshRect( aDstRect );
}

#ifndef IOS

bool AquaSalGraphics::drawEPS( long nX, long nY, long nWidth, long nHeight,
                                   void* pEpsData, sal_uLong nByteCount )
{
    
    NSData* xNSData = [NSData dataWithBytes:(void*)pEpsData length:(int)nByteCount];
    NSImageRep* xEpsImage = [NSEPSImageRep imageRepWithData: xNSData];
    if( !xEpsImage )
    {
        return false;
    }
    
    if( !CheckContext() )
    {
        return false;
    }
    
    CGContextSaveGState( mrContext );

    CGContextScaleCTM( mrContext, +1, -1 );
    nY = /*mnHeight*/ - (nY + nHeight);

    
    NSGraphicsContext* pOrigNSCtx = [NSGraphicsContext currentContext];
    [pOrigNSCtx retain];

    
    NSGraphicsContext* pDrawNSCtx = [NSGraphicsContext graphicsContextWithGraphicsPort: mrContext flipped: IsFlipped()];
    
    [NSGraphicsContext setCurrentContext: pDrawNSCtx];

    
    const NSRect aDstRect = NSMakeRect( nX, nY, nWidth, nHeight);
    const BOOL bOK = [xEpsImage drawInRect: aDstRect];

    
    [NSGraphicsContext setCurrentContext: pOrigNSCtx];
    [pOrigNSCtx release]; 

    CGContextRestoreGState( mrContext );
    
    RefreshRect( aDstRect );

    return bOK;
}

#endif

void AquaSalGraphics::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
    if( nX1 == nX2 && nY1 == nY2 )
    {
        
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
    (void) aRefreshRect;
    
}

void AquaSalGraphics::drawMask( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap, SalColor nMaskColor )
{
    if( !CheckContext() )
    {
        return;
    }
    const QuartzSalBitmap& rBitmap = static_cast<const QuartzSalBitmap&>(rSalBitmap);
    CGImageRef xImage = rBitmap.CreateColorMask( rPosAry.mnSrcX, rPosAry.mnSrcY,
                                                 rPosAry.mnSrcWidth, rPosAry.mnSrcHeight,
                                                 nMaskColor );
    if( !xImage )
    {
        return;
    }

    const CGRect aDstRect = CGRectMake(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth, rPosAry.mnDestHeight);
    CGContextDrawImage( mrContext, aDstRect, xImage );
    CGImageRelease( xImage );
    RefreshRect( aDstRect );
}

void AquaSalGraphics::drawPixel( long nX, long nY )
{
    
    ImplDrawPixel( nX, nY, maLineColor );
}

void AquaSalGraphics::drawPixel( long nX, long nY, SalColor nSalColor )
{
    const RGBAColor aPixelColor( nSalColor );
    ImplDrawPixel( nX, nY, aPixelColor );
}

bool AquaSalGraphics::drawPolyLine(
    const ::basegfx::B2DPolygon& rPolyLine,
    double fTransparency,
    const ::basegfx::B2DVector& rLineWidths,
    basegfx::B2DLineJoin eLineJoin,
    com::sun::star::drawing::LineCap eLineCap)
{
    
    const int nPointCount = rPolyLine.count();
    if( nPointCount <= 0 )
    {
        return true;
    }
    
    if( rLineWidths.getX() != rLineWidths.getY() )
    {
        return false;
    }
    
    
    
    if( (basegfx::B2DLINEJOIN_NONE == eLineJoin) &&
        (rLineWidths.getX() > 1.3) )
    {
        return false;
    }
    
    CGLineJoin aCGLineJoin = kCGLineJoinMiter;
    switch( eLineJoin )
    {
    case ::basegfx::B2DLINEJOIN_NONE: aCGLineJoin = /*TODO?*/kCGLineJoinMiter; break;
    case ::basegfx::B2DLINEJOIN_MIDDLE: aCGLineJoin = /*TODO?*/kCGLineJoinMiter; break;
    case ::basegfx::B2DLINEJOIN_BEVEL: aCGLineJoin = kCGLineJoinBevel; break;
    case ::basegfx::B2DLINEJOIN_MITER: aCGLineJoin = kCGLineJoinMiter; break;
    case ::basegfx::B2DLINEJOIN_ROUND: aCGLineJoin = kCGLineJoinRound; break;
    }

    
    CGLineCap aCGLineCap(kCGLineCapButt);

    switch(eLineCap)
    {
        default: 
        {
            aCGLineCap = kCGLineCapButt;
            break;
        }
        case com::sun::star::drawing::LineCap_ROUND:
        {
            aCGLineCap = kCGLineCapRound;
            break;
        }
        case com::sun::star::drawing::LineCap_SQUARE:
        {
            aCGLineCap = kCGLineCapSquare;
            break;
        }
    }

    
    CGMutablePathRef xPath = CGPathCreateMutable();
    AddPolygonToPath( xPath, rPolyLine, rPolyLine.isClosed(), !getAntiAliasB2DDraw(), true );

    const CGRect aRefreshRect = CGPathGetBoundingBox( xPath );
    
    if( ! ((aRefreshRect.size.width <= 0.125) && (aRefreshRect.size.height <= 0.125)) )
    {
        
        CGContextSaveGState( mrContext );
        CGContextAddPath( mrContext, xPath );
        
        CGContextSetShouldAntialias( mrContext, true );
        CGContextSetAlpha( mrContext, 1.0 - fTransparency );
        CGContextSetLineJoin( mrContext, aCGLineJoin );
        CGContextSetLineCap( mrContext, aCGLineCap );
        CGContextSetLineWidth( mrContext, rLineWidths.getX() );
        CGContextDrawPath( mrContext, kCGPathStroke );
        CGContextRestoreGState( mrContext );

        
        RefreshRect( aRefreshRect );
    }

    CGPathRelease( xPath );

    return true;
}

bool AquaSalGraphics::drawPolyLineBezier( sal_uInt32, const SalPoint*, const sal_uInt8* )
{
    return false;
}

bool AquaSalGraphics::drawPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly,
    double fTransparency )
{
    
    const int nPolyCount = rPolyPoly.count();
    if( nPolyCount <= 0 )
    {
        return true;
    }
    
    if( (fTransparency >= 1.0) || (fTransparency < 0) )
    {
        return true;
    }
    
    CGMutablePathRef xPath = CGPathCreateMutable();
    for( int nPolyIdx = 0; nPolyIdx < nPolyCount; ++nPolyIdx )
    {
        const ::basegfx::B2DPolygon rPolygon = rPolyPoly.getB2DPolygon( nPolyIdx );
        AddPolygonToPath( xPath, rPolygon, true, !getAntiAliasB2DDraw(), IsPenVisible() );
    }

    const CGRect aRefreshRect = CGPathGetBoundingBox( xPath );
    
    if( ! ((aRefreshRect.size.width <= 0.125) && (aRefreshRect.size.height <= 0.125)) )
    {
        
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
            return true;
        }

        
        CGContextSaveGState( mrContext );
        CGContextBeginPath( mrContext );
        CGContextAddPath( mrContext, xPath );

        
        CGContextSetShouldAntialias( mrContext, true );
        CGContextSetAlpha( mrContext, 1.0 - fTransparency );
        CGContextDrawPath( mrContext, eMode );
        CGContextRestoreGState( mrContext );

        
        RefreshRect( aRefreshRect );
    }

    CGPathRelease( xPath );

    return true;
}

void AquaSalGraphics::drawPolyPolygon( sal_uInt32 nPolyCount, const sal_uInt32 *pPoints, PCONSTSALPOINT  *ppPtAry )
{
    if( nPolyCount <= 0 )
        return;
    if( !CheckContext() )
        return;

    
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
        for( sal_uInt32 nPoly = 0; nPoly < nPolyCount; nPoly++ )
        {
            const sal_uInt32 nPoints = pPoints[nPoly];
            if( nPoints > 1 )
            {
                const SalPoint *pPtAry = ppPtAry[nPoly];
                float fX, fY;
                alignLinePoint( pPtAry, fX, fY );
                CGContextMoveToPoint( mrContext, fX, fY );
                pPtAry++;
                for( sal_uInt32 nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++ )
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
        for( sal_uInt32 nPoly = 0; nPoly < nPolyCount; nPoly++ )
        {
            const sal_uInt32 nPoints = pPoints[nPoly];
            if( nPoints > 1 )
            {
                const SalPoint *pPtAry = ppPtAry[nPoly];
                CGContextMoveToPoint( mrContext, pPtAry->mnX, pPtAry->mnY );
                pPtAry++;
                for( sal_uInt32 nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++ )
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

void AquaSalGraphics::drawPolygon( sal_uInt32 nPoints, const SalPoint *pPtAry )
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
        for( sal_uInt32 nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++ )
        {
            alignLinePoint( pPtAry, fX, fY );
            CGContextAddLineToPoint( mrContext, fX, fY );
        }
    }
    else
    {
        CGContextMoveToPoint( mrContext, pPtAry->mnX, pPtAry->mnY );
        pPtAry++;
        for( sal_uInt32 nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++ )
        {
            CGContextAddLineToPoint( mrContext, pPtAry->mnX, pPtAry->mnY );
        }
    }

    CGContextClosePath( mrContext );
    CGContextDrawPath( mrContext, eMode );
    RefreshRect( nX, nY, nWidth, nHeight );
}

bool AquaSalGraphics::drawPolygonBezier( sal_uInt32, const SalPoint*, const sal_uInt8* )
{
    return false;
}

bool AquaSalGraphics::drawPolyPolygonBezier( sal_uInt32, const sal_uInt32*,
                                                 const SalPoint* const*, const sal_uInt8* const* )
{
    return false;
}

void AquaSalGraphics::drawRect( long nX, long nY, long nWidth, long nHeight )
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


void AquaSalGraphics::drawPolyLine( sal_uInt32 nPoints, const SalPoint *pPtAry )
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
    for( sal_uInt32 nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++ )
    {
        alignLinePoint( pPtAry, fX, fY );
        CGContextAddLineToPoint( mrContext, fX, fY );
    }
    CGContextDrawPath( mrContext, kCGPathStroke );

    RefreshRect( nX, nY, nWidth, nHeight );
}

sal_uInt16 AquaSalGraphics::GetBitCount() const
{
    sal_uInt16 nBits = mnBitmapDepth ? mnBitmapDepth : 32;
    return nBits;
}

SalBitmap* AquaSalGraphics::getBitmap( long  nX, long  nY, long  nDX, long  nDY )
{
#ifdef IOS
    if (!mbForeignContext && m_aDevice != NULL)
    {
        
        basegfx::B2IBox aRect( nX, nY, nX+nDX, nY+nDY );
        basebmp::BitmapDeviceSharedPtr aSubSet = basebmp::subsetBitmapDevice(m_aDevice , aRect );

        SvpSalBitmap* pSalBitmap = new SvpSalBitmap;
        pSalBitmap->setBitmap(aSubSet);
        BitmapBuffer* pBuffer = pSalBitmap->AcquireBuffer(true);
        QuartzSalBitmap* pBitmap = new QuartzSalBitmap;
        if( !pBitmap->Create(*pBuffer))
        {
            delete pBitmap;
            pBitmap = NULL;
        }
        pSalBitmap->ReleaseBuffer(pBuffer, true);
        delete pSalBitmap;
        return pBitmap;
    }
    else if (mbForeignContext)
    {
        
        CGImageRef backImage = CGBitmapContextCreateImage(mrContext);
        if (backImage)
        {
            QuartzSalBitmap* pBitmap = new QuartzSalBitmap;
            if( !pBitmap->Create(backImage, mnBitmapDepth, nX, nY, nDX, nDY))
            {
                delete pBitmap;
                pBitmap = NULL;
            }
            CGImageRelease(backImage);
            return pBitmap;
        }
        return NULL;
    }
#endif

    DBG_ASSERT( mxLayer, "AquaSalGraphics::getBitmap() with no layer" );

    ApplyXorContext();

    QuartzSalBitmap* pBitmap = new QuartzSalBitmap;
    if( !pBitmap->Create( mxLayer, mnBitmapDepth, nX, nY, nDX, nDY) )
    {
        delete pBitmap;
        pBitmap = NULL;
    }
    return pBitmap;
}

#ifndef IOS

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

SalColor AquaSalGraphics::getPixel( long nX, long nY )
{
    
    if( !mxLayer || (nX < 0) || (nX >= mnWidth) ||
        (nY < 0) || (nY >= mnHeight))
    {
        return COL_BLACK;
    }
    
    CGColorSpaceRef aCGColorSpace = GetSalData()->mxRGBSpace;
    CGBitmapInfo aCGBmpInfo = kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Big;
#if defined OSL_BIGENDIAN
    struct{ unsigned char b, g, r, a; } aPixel;
#else
    struct{ unsigned char a, r, g, b; } aPixel;
#endif

    
    
    CGContextRef xOnePixelContext =
        CGBitmapContextCreate( &aPixel, 1, 1, 8, sizeof(aPixel),
                               aCGColorSpace, aCGBmpInfo );

    
    ApplyXorContext();

    
    if( IsFlipped() )
    {
        nY = mnHeight - nY;
    }
    const CGPoint aCGPoint = CGPointMake(-nX, -nY);
    CGContextDrawLayerAtPoint( xOnePixelContext, aCGPoint, mxLayer );
    CGContextRelease( xOnePixelContext );

    SalColor nSalColor = MAKE_SALCOLOR( aPixel.r, aPixel.g, aPixel.b );
    return nSalColor;
}

void AquaSalGraphics::GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY )
{
    if( !mnRealDPIY )
    {
        initResolution( (mbWindow && mpFrame) ? mpFrame->getNSWindow() : nil );
    }

    rDPIX = mnRealDPIX;
    rDPIY = mnRealDPIY;
}

#endif

void AquaSalGraphics::ImplDrawPixel( long nX, long nY, const RGBAColor& rColor )
{
    if( !CheckContext() )
    {
        return;
    }
    
    CGContextSetFillColor( mrContext, rColor.AsArray() );
    
    const CGRect aDstRect = CGRectMake(nX, nY, 1, 1);
    CGContextFillRect( mrContext, aDstRect );
    RefreshRect( aDstRect );
    
    CGContextSetFillColor( mrContext, maFillColor.AsArray() );
}

#ifndef IOS

void AquaSalGraphics::initResolution( NSWindow* )
{
    
    
    

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
            if( pScreens )
                pScreen = [pScreens objectAtIndex: 0];
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
                    
                    
                    const CGDirectDisplayID nDisplayID = (CGDirectDisplayID)[pVal longValue];
                    const CGSize aSize = CGDisplayScreenSize( nDisplayID ); 
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

        
        
        
        static const int nMinDPI = 72;
        if( (mnRealDPIX < nMinDPI) || (mnRealDPIY < nMinDPI) )
        {
            mnRealDPIX = mnRealDPIY = nMinDPI;
        }
        static const int nMaxDPI = 200;
        if( (mnRealDPIX > nMaxDPI) || (mnRealDPIY > nMaxDPI) )
        {
            mnRealDPIX = mnRealDPIY = nMaxDPI;
        }
        
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
        CGContextSaveGState(mrContext);

        if ( nFlags & SAL_INVERT_TRACKFRAME )
        {
            const CGFloat dashLengths[2]  = { 4.0, 4.0 };     
            CGContextSetBlendMode( mrContext, kCGBlendModeDifference );
            CGContextSetRGBStrokeColor ( mrContext, 1.0, 1.0, 1.0, 1.0 );
            CGContextSetLineDash ( mrContext, 0, dashLengths, 2 );
            CGContextSetLineWidth( mrContext, 2.0);
            CGContextStrokeRect ( mrContext, aCGRect );
        }
        else if ( nFlags & SAL_INVERT_50 )
        {
            
            CGContextSetBlendMode(mrContext, kCGBlendModeDifference);
            CGContextAddRect( mrContext, aCGRect );
            Pattern50Fill();
        }
        else 
        {
            CGContextSetBlendMode(mrContext, kCGBlendModeDifference);
            CGContextSetRGBFillColor ( mrContext,1.0, 1.0, 1.0 , 1.0 );
            CGContextFillRect ( mrContext, aCGRect );
        }
        CGContextRestoreGState( mrContext);
        RefreshRect( aCGRect );
    }
}

void AquaSalGraphics::invert( sal_uInt32 nPoints, const SalPoint*  pPtAry, SalInvert nSalFlags )
{
    CGPoint* CGpoints ;
    if ( CheckContext() )
    {
        CGContextSaveGState(mrContext);
        CGpoints = makeCGptArray(nPoints,pPtAry);
        CGContextAddLines ( mrContext, CGpoints, nPoints );
        if ( nSalFlags & SAL_INVERT_TRACKFRAME )
        {
            const CGFloat dashLengths[2]  = { 4.0, 4.0 };     
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
        else 
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

void AquaSalGraphics::Pattern50Fill()
{
    static const CGFloat aFillCol[4] = { 1,1,1,1 };
    static const CGPatternCallbacks aCallback = { 0, &DrawPattern50, NULL };
    static const CGColorSpaceRef mxP50Space = CGColorSpaceCreatePattern( GetSalData()->mxRGBSpace );
    static const CGPatternRef mxP50Pattern = CGPatternCreate( NULL, CGRectMake( 0, 0, 4, 4 ),
                                                              CGAffineTransformIdentity, 4, 4,
                                                              kCGPatternTilingConstantSpacing,
                                                              false, &aCallback );
    CGContextSetFillColorSpace( mrContext, mxP50Space );
    CGContextSetFillPattern( mrContext, mxP50Pattern, aFillCol );
    CGContextFillPath( mrContext );
}


void AquaSalGraphics::ResetClipRegion()
{
    
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

void AquaSalGraphics::SetLineColor()
{
    maLineColor.SetAlpha( 0.0 );   
    if( CheckContext() )
    {
        CGContextSetRGBStrokeColor( mrContext, maLineColor.GetRed(), maLineColor.GetGreen(),
                                    maLineColor.GetBlue(), maLineColor.GetAlpha() );
    }
}

void AquaSalGraphics::SetLineColor( SalColor nSalColor )
{
    maLineColor = RGBAColor( nSalColor );
    if( CheckContext() )
    {
        CGContextSetRGBStrokeColor( mrContext, maLineColor.GetRed(), maLineColor.GetGreen(),
                                   maLineColor.GetBlue(), maLineColor.GetAlpha() );
    }
}

void AquaSalGraphics::SetFillColor()
{
    maFillColor.SetAlpha( 0.0 );   
    if( CheckContext() )
    {
        CGContextSetRGBFillColor( mrContext, maFillColor.GetRed(), maFillColor.GetGreen(),
                                  maFillColor.GetBlue(), maFillColor.GetAlpha() );
    }
}

void AquaSalGraphics::SetFillColor( SalColor nSalColor )
{
    maFillColor = RGBAColor( nSalColor );
    if( CheckContext() )
    {
        CGContextSetRGBFillColor( mrContext, maFillColor.GetRed(), maFillColor.GetGreen(),
                                 maFillColor.GetBlue(), maFillColor.GetAlpha() );
    }
}

bool AquaSalGraphics::supportsOperation( OutDevSupportType eType ) const
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

bool AquaSalGraphics::setClipRegion( const Region& i_rClip )
{
    
    if( mxClipPath )
    {
        CGPathRelease( mxClipPath );
        mxClipPath = NULL;
    }
    mxClipPath = CGPathCreateMutable();

    
    if(i_rClip.HasPolyPolygonOrB2DPolyPolygon())
    {
        const basegfx::B2DPolyPolygon aClip(i_rClip.GetAsB2DPolyPolygon());

        AddPolyPolygonToPath( mxClipPath, aClip, !getAntiAliasB2DDraw(), false );
    }
    else
    {
        RectangleVector aRectangles;
        i_rClip.GetRegionRectangles(aRectangles);

        for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
        {
            const long nW(aRectIter->Right() - aRectIter->Left() + 1); 

            if(nW)
            {
                const long nH(aRectIter->Bottom() - aRectIter->Top() + 1); 

                if(nH)
                {
                    const CGRect aRect = CGRectMake( aRectIter->Left(), aRectIter->Top(), nW, nH);
                    CGPathAddRect( mxClipPath, NULL, aRect );
                }
            }
        }
    }
    
    if( CheckContext() )
    {
        SetState();
    }
    return true;
}

void AquaSalGraphics::SetROPFillColor( SalROPColor nROPColor )
{
    if( ! mbPrinter )
        SetFillColor( ImplGetROPSalColor( nROPColor ) );
}

void AquaSalGraphics::SetROPLineColor( SalROPColor nROPColor )
{
    if( ! mbPrinter )
        SetLineColor( ImplGetROPSalColor( nROPColor ) );
}

void AquaSalGraphics::SetXORMode( bool bSet, bool bInvertOnly )
{
    
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
    
    if( !mpXorEmulation )
    {
        mpXorEmulation = new XorEmulation();
        mpXorEmulation->SetTarget( mnWidth, mnHeight, mnBitmapDepth, mrContext, mxLayer );
    }

    
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
    DBG_ASSERT( mbWindow, "updateResolution on inappropriate graphics" );

    initResolution( (mbWindow && mpFrame) ? mpFrame->getNSWindow() : nil );
}

#endif



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
    
    if( m_xMaskContext )
    {
        
        CGContextRelease( m_xMaskContext );
        delete[] m_pMaskBuffer;
        m_xMaskContext = NULL;
        m_pMaskBuffer = NULL;

        
        if( m_xTempContext )
        {
            CGContextRelease( m_xTempContext );
            delete[] m_pTempBuffer;
            m_xTempContext = NULL;
            m_pTempBuffer = NULL;
        }
    }

    
    if( !xTargetContext )
    {
        return;
    }
    
    m_xTargetLayer = xTargetLayer;
    m_xTargetContext = xTargetContext;

    
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

    
    m_pMaskBuffer = new sal_uLong[ m_nBufferLongs ];
    m_xMaskContext = CGBitmapContextCreate( m_pMaskBuffer,
                                            nWidth, nHeight,
                                            nBitsPerComponent, nBytesPerRow,
                                            aCGColorSpace, aCGBmpInfo );
    
    memset( m_pMaskBuffer, 0, m_nBufferLongs * sizeof(sal_uLong) );

    
    
    if( nTargetDepth )
        m_pTempBuffer = (sal_uLong*)CGBitmapContextGetData( m_xTargetContext );
    if( !m_pTempBuffer )
    {
        
        m_pTempBuffer = new sal_uLong[ m_nBufferLongs ];
        m_xTempContext = CGBitmapContextCreate( m_pTempBuffer,
                                                nWidth, nHeight,
                                                nBitsPerComponent, nBytesPerRow,
                                                aCGColorSpace, aCGBmpInfo );
    }

    
    CGContextSetFillColorSpace( m_xMaskContext, aCGColorSpace );
    CGContextSetStrokeColorSpace( m_xMaskContext, aCGColorSpace );
    CGContextSetShouldAntialias( m_xMaskContext, false );

    
    
    if( aCGColorSpace == GetSalData()->mxGraySpace )
    {
        CGContextSetBlendMode( m_xMaskContext, kCGBlendModeDifference );
    }
    
    const CGAffineTransform aCTM = CGContextGetCTM( xTargetContext );
    CGContextConcatCTM( m_xMaskContext, aCTM );
    if( m_xTempContext )
    {
        CGContextConcatCTM( m_xTempContext, aCTM );
    }
    
    CGContextSaveGState( m_xMaskContext );
}

bool XorEmulation::UpdateTarget()
{
    if( !IsEnabled() )
    {
        return false;
    }
    
    if( m_xTempContext )
    {
        CGContextDrawLayerAtPoint( m_xTempContext, CGPointZero, m_xTargetLayer );
    }
    
    
    
    const sal_uLong* pSrc = m_pMaskBuffer;
    sal_uLong* pDst = m_pTempBuffer;
    for( int i = m_nBufferLongs; --i >= 0;)
    {
        *(pDst++) ^= *(pSrc++);
    }
    
    if( m_xTempContext )
    {
        CGImageRef xXorImage = CGBitmapContextCreateImage( m_xTempContext );
        const int nWidth  = (int)CGImageGetWidth( xXorImage );
        const int nHeight = (int)CGImageGetHeight( xXorImage );
        
        const CGRect aFullRect = CGRectMake(0, 0, nWidth, nHeight);
        CGContextDrawImage( m_xTargetContext, aFullRect, xXorImage );
        CGImageRelease( xXorImage );
    }

    
    
    memset( m_pMaskBuffer, 0, m_nBufferLongs * sizeof(sal_uLong) );

    
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
