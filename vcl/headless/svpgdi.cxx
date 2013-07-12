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

#include "headless/svpgdi.hxx"
#include "headless/svpbmp.hxx"

#include <vcl/sysdata.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2ibox.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basebmp/scanlineformats.hxx>

#include <tools/debug.hxx>

#if OSL_DEBUG_LEVEL > 2
#include <basebmp/debug.hxx>
#include <fstream>
#include <rtl/strbuf.hxx>
#include <sys/stat.h>
#endif

#include <stdio.h>

inline void dbgOut( const basebmp::BitmapDeviceSharedPtr&
#if OSL_DEBUG_LEVEL > 2
rDevice
#endif
)
{
    #if OSL_DEBUG_LEVEL > 2
    static int dbgStreamNum = 0;
    OStringBuffer aBuf( 256 );
    aBuf.append( "debug" );
    mkdir( aBuf.getStr(), 0777 );
    aBuf.append( "/" );
    aBuf.append( sal_Int64(reinterpret_cast<sal_IntPtr>(rDevice.get())), 16 );
    mkdir( aBuf.getStr(), 0777 );
    aBuf.append( "/bmp" );
    aBuf.append( sal_Int32(dbgStreamNum++) );
    std::fstream bmpstream( aBuf.getStr(), std::ios::out );
    debugDump( rDevice, bmpstream );
    #endif
}

// ===========================================================================

bool SvpSalGraphics::drawAlphaBitmap( const SalTwoRect&, const SalBitmap& /*rSourceBitmap*/, const SalBitmap& /*rAlphaBitmap*/ )
{
    // TODO(P3) implement alpha blending
    return false;
}

bool SvpSalGraphics::drawTransformedBitmap(
    const basegfx::B2DPoint& rNull,
    const basegfx::B2DPoint& rX,
    const basegfx::B2DPoint& rY,
    const SalBitmap& rSourceBitmap,
    const SalBitmap* pAlphaBitmap)
{
    // here direct support for transformed bitmaps can be impemented
    (void)rNull; (void)rX; (void)rY; (void)rSourceBitmap; (void)pAlphaBitmap;
    return false;
}

bool SvpSalGraphics::drawAlphaRect( long /*nX*/, long /*nY*/, long /*nWidth*/, long /*nHeight*/, sal_uInt8 /*nTransparency*/ )
{
    // TODO(P3) implement alpha blending
    return false;
}

#ifndef IOS

SvpSalGraphics::SvpSalGraphics() :
    m_bUseLineColor( true ),
    m_aLineColor( COL_BLACK ),
    m_bUseFillColor( false ),
    m_aFillColor( COL_WHITE ),
    m_aDrawMode( basebmp::DrawMode_PAINT ),
    m_aTextColor( COL_BLACK ),
    m_eTextFmt( basebmp::FORMAT_EIGHT_BIT_GREY ),
    m_bClipSetup( false )
{
    for( int i = 0; i < MAX_FALLBACK; ++i )
        m_pServerFont[i] = NULL;
}

SvpSalGraphics::~SvpSalGraphics()
{
}

#endif

void SvpSalGraphics::setDevice( basebmp::BitmapDeviceSharedPtr& rDevice )
{
    m_aOrigDevice = rDevice;
    ResetClipRegion();

#ifndef IOS
    // determine matching bitmap format for masks
    basebmp::Format nDeviceFmt = m_aDevice->getScanlineFormat();
    DBG_ASSERT( (nDeviceFmt <= (sal_uInt32)basebmp::FORMAT_MAX), "SVP::setDevice() with invalid bitmap format" );
    switch( nDeviceFmt )
    {
        case basebmp::FORMAT_EIGHT_BIT_GREY:
        case basebmp::FORMAT_SIXTEEN_BIT_LSB_TC_MASK:
        case basebmp::FORMAT_SIXTEEN_BIT_MSB_TC_MASK:
        case basebmp::FORMAT_TWENTYFOUR_BIT_TC_MASK:
        case basebmp::FORMAT_THIRTYTWO_BIT_TC_MASK_BGRA:
        case basebmp::FORMAT_THIRTYTWO_BIT_TC_MASK_ARGB:
        case basebmp::FORMAT_THIRTYTWO_BIT_TC_MASK_ABGR:
        case basebmp::FORMAT_THIRTYTWO_BIT_TC_MASK_RGBA:
            m_eTextFmt = basebmp::FORMAT_EIGHT_BIT_GREY;
            break;
        default:
            m_eTextFmt = basebmp::FORMAT_ONE_BIT_LSB_GREY;
            break;
    }
#endif
}

void SvpSalGraphics::GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY )
{
    rDPIX = rDPIY = 96;
}

sal_uInt16 SvpSalGraphics::GetBitCount() const
{
    return SvpSalBitmap::getBitCountFromScanlineFormat( m_aDevice->getScanlineFormat() );
}

long SvpSalGraphics::GetGraphicsWidth() const
{
    if( m_aDevice.get() )
    {
        basegfx::B2IVector aSize = m_aOrigDevice->getSize();
        return aSize.getX();
    }
    return 0;
}

void SvpSalGraphics::ResetClipRegion()
{
    m_aDevice = m_aOrigDevice;
    m_aClipMap.reset();
    m_bClipSetup = true;
    m_aClipRegion.SetNull();
}


// verify clip for the whole area is setup
void SvpSalGraphics::ensureClip()
{
    if (m_bClipSetup)
        return;

    m_aDevice = m_aOrigDevice;
    basegfx::B2IVector aSize = m_aDevice->getSize();
    m_aClipMap = basebmp::createBitmapDevice( aSize, false, basebmp::FORMAT_ONE_BIT_MSB_GREY );
    m_aClipMap->clear( basebmp::Color(0xFFFFFFFF) );

    RectangleVector aRectangles;
    m_aClipRegion.GetRegionRectangles(aRectangles);

    for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
    {
        const long nW(aRectIter->GetWidth());
        if(nW)
        {
            const long nH(aRectIter->GetHeight());

            if(nH)
            {
                basegfx::B2DPolyPolygon aFull;

                aFull.append(
                    basegfx::tools::createPolygonFromRect(
                        basegfx::B2DRectangle(
                            aRectIter->Left(),
                            aRectIter->Top(),
                            aRectIter->Left() + nW,
                            aRectIter->Top() + nH)));
                m_aClipMap->fillPolyPolygon(aFull, basebmp::Color(0), basebmp::DrawMode_PAINT);
            }
        }
    }
    m_bClipSetup = true;
}

SvpSalGraphics::ClipUndoHandle::~ClipUndoHandle()
{
    if( m_aDevice.get() )
        m_rGfx.m_aDevice = m_aDevice;
}

// setup a clip rectangle -only- iff we have to; if aRange
// is entirely contained inside an existing clip frame, we
// will avoid setting up the clip bitmap. Similarly if the
// range doesn't appear at all we return true to avoid
// rendering
bool SvpSalGraphics::isClippedSetup( const basegfx::B2IBox &aRange, SvpSalGraphics::ClipUndoHandle &rUndo )
{
    if( m_bClipSetup )
        return false;

    if( m_aClipRegion.IsEmpty() ) // no clipping
        return false;

    // fprintf( stderr, "ensureClipFor: %d, %d %dx%d\n",
    //         aRange.getMinX(), aRange.getMinY(),
    //         (int)aRange.getWidth(), (int)aRange.getHeight() );

    // first see if aRange is purely internal to one of the clip regions
    Rectangle aRect( Point( aRange.getMinX(), aRange.getMinY() ),
                     Size( aRange.getWidth(), aRange.getHeight() ) );

    // then see if we are overlapping with just one
    int nHit = 0;
    Rectangle aHitRect;
    RectangleVector aRectangles;
    m_aClipRegion.GetRegionRectangles(aRectangles);
    for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
    {
        if( aRectIter->IsOver( aRect ) )
        {
            aHitRect = *aRectIter;
            nHit++;
        }
    }

    if( nHit == 0 ) // rendering outside any clipping region
    {
//        fprintf (stderr, "denegerate case detected ...\n");
        return true;
    }
    else if( nHit == 1 ) // common path: rendering against just one clipping region
    {
        if( aHitRect.IsInside( aRect ) )
        {
//        fprintf (stderr, " is inside ! avoid deeper clip ...\n");
            return false;
        }
//    fprintf (stderr, " operation only overlaps with a single clip zone\n" );
        rUndo.m_aDevice = m_aDevice;
        m_aDevice = basebmp::subsetBitmapDevice( m_aOrigDevice,
                                                 basegfx::B2IBox (aHitRect.Left(),
                                                                  aHitRect.Top(),
                                                                  aHitRect.Right(),
                                                                  aHitRect.Bottom()) );
        return false;
    }
//    fprintf (stderr, "URK: complex & slow clipping case\n" );
    // horribly slow & complicated case ...

    ensureClip();
    return false;
}


// Clipping by creating unconditional mask bitmaps is horribly
// slow so defer it, as much as possible. It is common to get
// 3 rectangles pushed, and have to create a vast off-screen
// mask only to destroy it shortly afterwards. That is
// particularly galling if we render only to a small,
// well defined rectangular area inside one of these clip
// rectangles.
//
// ensureClipFor() or ensureClip() need to be called before
// real rendering. FIXME: we should prolly push this down to
// bitmapdevice instead.
bool SvpSalGraphics::setClipRegion( const Region& i_rClip )
{
    m_aClipRegion = i_rClip;
    m_aClipMap.reset();
    if( i_rClip.IsEmpty() )
    {
        m_bClipSetup = true;
        return true;
    }

    RectangleVector aRectangles;
    i_rClip.GetRegionRectangles(aRectangles);

    if(1 == aRectangles.size())
    {
        m_aClipMap.reset();

        const Rectangle& aBoundRect = aRectangles[0];
        m_aDevice = basebmp::subsetBitmapDevice(
            m_aOrigDevice,
            basegfx::B2IBox(aBoundRect.Left(),aBoundRect.Top(),aBoundRect.Right(),aBoundRect.Bottom()) );

        m_bClipSetup = true;
    }
    else
        m_bClipSetup = false;

    return true;
}

void SvpSalGraphics::SetLineColor()
{
    m_bUseLineColor = false;
}

void SvpSalGraphics::SetLineColor( SalColor nSalColor )
{
    m_bUseLineColor = true;
    m_aLineColor = basebmp::Color( nSalColor );
}

void SvpSalGraphics::SetFillColor()
{
    m_bUseFillColor = false;
}

void SvpSalGraphics::SetFillColor( SalColor nSalColor )
{
    m_bUseFillColor = true;
    m_aFillColor = basebmp::Color( nSalColor );
}

void SvpSalGraphics::SetXORMode( bool bSet, bool )
{
    m_aDrawMode = bSet ? basebmp::DrawMode_XOR : basebmp::DrawMode_PAINT;
}

void SvpSalGraphics::SetROPLineColor( SalROPColor nROPColor )
{
    m_bUseLineColor = true;
    switch( nROPColor )
    {
        case SAL_ROP_0:
            m_aLineColor = basebmp::Color( 0 );
            break;
        case SAL_ROP_1:
            m_aLineColor = basebmp::Color( 0xffffff );
            break;
        case SAL_ROP_INVERT:
            m_aLineColor = basebmp::Color( 0xffffff );
            break;
    }
}

void SvpSalGraphics::SetROPFillColor( SalROPColor nROPColor )
{
    m_bUseFillColor = true;
    switch( nROPColor )
    {
        case SAL_ROP_0:
            m_aFillColor = basebmp::Color( 0 );
            break;
        case SAL_ROP_1:
            m_aFillColor = basebmp::Color( 0xffffff );
            break;
        case SAL_ROP_INVERT:
            m_aFillColor = basebmp::Color( 0xffffff );
            break;
    }
}

void SvpSalGraphics::drawPixel( long nX, long nY )
{
    if( m_bUseLineColor )
    {
        ensureClip();
        m_aDevice->setPixel( basegfx::B2IPoint( nX, nY ),
                             m_aLineColor,
                             m_aDrawMode,
                             m_aClipMap
                             );
    }
    dbgOut( m_aDevice );
}

void SvpSalGraphics::drawPixel( long nX, long nY, SalColor nSalColor )
{
    basebmp::Color aColor( nSalColor );
    ensureClip();
    m_aDevice->setPixel( basegfx::B2IPoint( nX, nY ),
                         aColor,
                         m_aDrawMode,
                         m_aClipMap
                         );
    dbgOut( m_aDevice );
}

void SvpSalGraphics::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
    if( m_bUseLineColor )
    {
        ensureClip(); // FIXME: for ...
        m_aDevice->drawLine( basegfx::B2IPoint( nX1, nY1 ),
                             basegfx::B2IPoint( nX2, nY2 ),
                             m_aLineColor,
                             m_aDrawMode,
                             m_aClipMap );
    }
    dbgOut( m_aDevice );
}

void SvpSalGraphics::drawRect( long nX, long nY, long nWidth, long nHeight )
{
    if( m_bUseLineColor || m_bUseFillColor )
    {
        basegfx::B2DPolygon aRect = basegfx::tools::createPolygonFromRect( basegfx::B2DRectangle( nX, nY, nX+nWidth, nY+nHeight ) );
        ensureClip(); // FIXME: for ...
        if( m_bUseFillColor )
        {
            basegfx::B2DPolyPolygon aPolyPoly( aRect );
            m_aDevice->fillPolyPolygon( aPolyPoly, m_aFillColor, m_aDrawMode, m_aClipMap );
        }
        if( m_bUseLineColor )
            m_aDevice->drawPolygon( aRect, m_aLineColor, m_aDrawMode, m_aClipMap );
    }
    dbgOut( m_aDevice );
}

void SvpSalGraphics::drawPolyLine( sal_uLong nPoints, const SalPoint* pPtAry )
{
    if( m_bUseLineColor && nPoints )
    {
        basegfx::B2DPolygon aPoly;
        aPoly.append( basegfx::B2DPoint( pPtAry->mnX, pPtAry->mnY ), nPoints );
        for( sal_uLong i = 1; i < nPoints; i++ )
            aPoly.setB2DPoint( i, basegfx::B2DPoint( pPtAry[i].mnX, pPtAry[i].mnY ) );
        aPoly.setClosed( false );
        ensureClip(); // FIXME: for ...
        m_aDevice->drawPolygon( aPoly, m_aLineColor, m_aDrawMode, m_aClipMap );
    }
    dbgOut( m_aDevice );
}

void SvpSalGraphics::drawPolygon( sal_uLong nPoints, const SalPoint* pPtAry )
{
    if( ( m_bUseLineColor || m_bUseFillColor ) && nPoints )
    {
        basegfx::B2DPolygon aPoly;
        aPoly.append( basegfx::B2DPoint( pPtAry->mnX, pPtAry->mnY ), nPoints );
        for( sal_uLong i = 1; i < nPoints; i++ )
            aPoly.setB2DPoint( i, basegfx::B2DPoint( pPtAry[i].mnX, pPtAry[i].mnY ) );
        ensureClip(); // FIXME: for ...
        if( m_bUseFillColor )
        {
            aPoly.setClosed( true );
            m_aDevice->fillPolyPolygon( basegfx::B2DPolyPolygon(aPoly), m_aFillColor, m_aDrawMode, m_aClipMap );
        }
        if( m_bUseLineColor )
        {
            aPoly.setClosed( false );
            m_aDevice->drawPolygon( aPoly, m_aLineColor, m_aDrawMode, m_aClipMap );
        }
    }
    dbgOut( m_aDevice );
}

void SvpSalGraphics::drawPolyPolygon( sal_uInt32        nPoly,
                                      const sal_uInt32* pPointCounts,
                                      PCONSTSALPOINT*   pPtAry )
{
    if( ( m_bUseLineColor || m_bUseFillColor ) && nPoly )
    {
        basegfx::B2DPolyPolygon aPolyPoly;
        for( sal_uInt32 nPolygon = 0; nPolygon < nPoly; nPolygon++ )
        {
            sal_uInt32 nPoints = pPointCounts[nPolygon];
            if( nPoints )
            {
                PCONSTSALPOINT pPoints = pPtAry[nPolygon];
                basegfx::B2DPolygon aPoly;
                aPoly.append( basegfx::B2DPoint( pPoints->mnX, pPoints->mnY ), nPoints );
                for( sal_uInt32 i = 1; i < nPoints; i++ )
                    aPoly.setB2DPoint( i, basegfx::B2DPoint( pPoints[i].mnX, pPoints[i].mnY ) );

                aPolyPoly.append( aPoly );
            }
        }
        ensureClip(); // FIXME: for ...
        if( m_bUseFillColor )
        {
            aPolyPoly.setClosed( true );
            m_aDevice->fillPolyPolygon( aPolyPoly, m_aFillColor, m_aDrawMode, m_aClipMap );
        }
        if( m_bUseLineColor )
        {
            aPolyPoly.setClosed( false );
            nPoly = aPolyPoly.count();
            for( sal_uInt32 i = 0; i < nPoly; i++ )
                m_aDevice->drawPolygon( aPolyPoly.getB2DPolygon(i), m_aLineColor, m_aDrawMode, m_aClipMap );
        }
    }
    dbgOut( m_aDevice );
}

bool SvpSalGraphics::drawPolyLine(
    const ::basegfx::B2DPolygon&,
    double /*fTransparency*/,
    const ::basegfx::B2DVector& /*rLineWidths*/,
    basegfx::B2DLineJoin /*eJoin*/,
    com::sun::star::drawing::LineCap /*eLineCap*/)
{
        // TODO: implement and advertise OutDevSupport_B2DDraw support
        return false;
}

sal_Bool SvpSalGraphics::drawPolyLineBezier( sal_uLong,
                                             const SalPoint*,
                                             const sal_uInt8* )
{
    return sal_False;
}

sal_Bool SvpSalGraphics::drawPolygonBezier( sal_uLong,
                                            const SalPoint*,
                                            const sal_uInt8* )
{
    return sal_False;
}

sal_Bool SvpSalGraphics::drawPolyPolygonBezier( sal_uInt32,
                                                const sal_uInt32*,
                                                const SalPoint* const*,
                                                const sal_uInt8* const* )
{
    return sal_False;
}

bool SvpSalGraphics::drawPolyPolygon( const basegfx::B2DPolyPolygon&, double /*fTransparency*/ )
{
    // TODO: maybe BaseBmp can draw B2DPolyPolygons directly
    return false;
}

void SvpSalGraphics::copyArea( long nDestX,
                                      long nDestY,
                                      long nSrcX,
                                      long nSrcY,
                                      long nSrcWidth,
                                      long nSrcHeight,
                                      sal_uInt16 /*nFlags*/ )
{
    basegfx::B2IBox aSrcRect( nSrcX, nSrcY, nSrcX+nSrcWidth, nSrcY+nSrcHeight );
    basegfx::B2IBox aDestRect( nDestX, nDestY, nDestX+nSrcWidth, nDestY+nSrcHeight );
    // fprintf( stderr, "copyArea %ld pixels - clip region %d\n",
    //         (long)(nSrcWidth * nSrcHeight), m_aClipMap.get() != NULL );
    SvpSalGraphics::ClipUndoHandle aUndo( this );
    if( !isClippedSetup( aDestRect, aUndo ) )
        m_aDevice->drawBitmap( m_aOrigDevice, aSrcRect, aDestRect, basebmp::DrawMode_PAINT, m_aClipMap );
    dbgOut( m_aDevice );
}

void SvpSalGraphics::copyBits( const SalTwoRect& rPosAry,
                               SalGraphics*      pSrcGraphics )
{
    if( !m_aDevice.get() )
        return;

    SvpSalGraphics* pSrc = pSrcGraphics ?
        static_cast<SvpSalGraphics*>(pSrcGraphics) : this;
    basegfx::B2IBox aSrcRect( rPosAry.mnSrcX, rPosAry.mnSrcY,
                     rPosAry.mnSrcX+rPosAry.mnSrcWidth,
                     rPosAry.mnSrcY+rPosAry.mnSrcHeight );
    basegfx::B2IBox aDestRect( rPosAry.mnDestX, rPosAry.mnDestY,
                      rPosAry.mnDestX+rPosAry.mnDestWidth,
                      rPosAry.mnDestY+rPosAry.mnDestHeight );

    SvpSalGraphics::ClipUndoHandle aUndo( this );
    if( !isClippedSetup( aDestRect, aUndo ) )
        m_aDevice->drawBitmap( pSrc->m_aOrigDevice, aSrcRect, aDestRect, basebmp::DrawMode_PAINT, m_aClipMap );
    dbgOut( m_aDevice );
}

void SvpSalGraphics::drawBitmap( const SalTwoRect& rPosAry,
                                 const SalBitmap& rSalBitmap )
{
    const SvpSalBitmap& rSrc = static_cast<const SvpSalBitmap&>(rSalBitmap);
    basegfx::B2IBox aSrcRect( rPosAry.mnSrcX, rPosAry.mnSrcY,
                     rPosAry.mnSrcX+rPosAry.mnSrcWidth,
                     rPosAry.mnSrcY+rPosAry.mnSrcHeight );
    basegfx::B2IBox aDestRect( rPosAry.mnDestX, rPosAry.mnDestY,
                      rPosAry.mnDestX+rPosAry.mnDestWidth,
                      rPosAry.mnDestY+rPosAry.mnDestHeight );

    SvpSalGraphics::ClipUndoHandle aUndo( this );
    if( !isClippedSetup( aDestRect, aUndo ) )
        m_aDevice->drawBitmap( rSrc.getBitmap(), aSrcRect, aDestRect, basebmp::DrawMode_PAINT, m_aClipMap );
    dbgOut( m_aDevice );
}

void SvpSalGraphics::drawBitmap( const SalTwoRect&,
                                 const SalBitmap&,
                                 SalColor )
{
    // SNI, as in X11 plugin
}

void SvpSalGraphics::drawBitmap( const SalTwoRect& rPosAry,
                                 const SalBitmap& rSalBitmap,
                                 const SalBitmap& rTransparentBitmap )
{
    const SvpSalBitmap& rSrc = static_cast<const SvpSalBitmap&>(rSalBitmap);
    const SvpSalBitmap& rSrcTrans = static_cast<const SvpSalBitmap&>(rTransparentBitmap);
    basegfx::B2IBox aSrcRect( rPosAry.mnSrcX, rPosAry.mnSrcY,
                     rPosAry.mnSrcX+rPosAry.mnSrcWidth,
                     rPosAry.mnSrcY+rPosAry.mnSrcHeight );
    basegfx::B2IBox aDestRect( rPosAry.mnDestX, rPosAry.mnDestY,
                      rPosAry.mnDestX+rPosAry.mnDestWidth,
                      rPosAry.mnDestY+rPosAry.mnDestHeight );
    SvpSalGraphics::ClipUndoHandle aUndo( this );
    if( !isClippedSetup( aDestRect, aUndo ) )
        m_aDevice->drawMaskedBitmap( rSrc.getBitmap(), rSrcTrans.getBitmap(),
                                     aSrcRect, aDestRect, basebmp::DrawMode_PAINT, m_aClipMap );
    dbgOut( m_aDevice );
}

void SvpSalGraphics::drawMask( const SalTwoRect& rPosAry,
                               const SalBitmap& rSalBitmap,
                               SalColor nMaskColor )
{
    const SvpSalBitmap& rSrc = static_cast<const SvpSalBitmap&>(rSalBitmap);
    basegfx::B2IBox aSrcRect( rPosAry.mnSrcX, rPosAry.mnSrcY,
                     rPosAry.mnSrcX+rPosAry.mnSrcWidth,
                     rPosAry.mnSrcY+rPosAry.mnSrcHeight );
    basegfx::B2IPoint aDestPoint( rPosAry.mnDestX, rPosAry.mnDestY );

    // BitmapDevice::drawMaskedColor works with 0==transparent,
    // 255==opaque. drawMask() semantic is the other way
    // around. Therefore, invert mask.
    basebmp::BitmapDeviceSharedPtr aCopy =
        cloneBitmapDevice( basegfx::B2IVector( rPosAry.mnSrcWidth, rPosAry.mnSrcHeight ),
                           rSrc.getBitmap() );
    basebmp::Color aBgColor( COL_WHITE );
    aCopy->clear(aBgColor);
    basebmp::Color aFgColor( COL_BLACK );
    aCopy->drawMaskedColor( aFgColor, rSrc.getBitmap(), aSrcRect, basegfx::B2IPoint() );

    basebmp::Color aColor( nMaskColor );
    basegfx::B2IBox aSrcRect2( 0, 0, rPosAry.mnSrcWidth, rPosAry.mnSrcHeight );
    const basegfx::B2IBox aClipRect( aDestPoint, basegfx::B2ITuple( aSrcRect.getWidth(), aSrcRect.getHeight() ) );

    SvpSalGraphics::ClipUndoHandle aUndo( this );
    if( !isClippedSetup( aClipRect, aUndo ) )
        m_aDevice->drawMaskedColor( aColor, aCopy, aSrcRect, aDestPoint, m_aClipMap );
    dbgOut( m_aDevice );
}

SalBitmap* SvpSalGraphics::getBitmap( long nX, long nY, long nWidth, long nHeight )
{
    basebmp::BitmapDeviceSharedPtr aCopy =
        cloneBitmapDevice( basegfx::B2IVector( nWidth, nHeight ),
                           m_aDevice );
    basegfx::B2IBox aSrcRect( nX, nY, nX+nWidth, nY+nHeight );
    basegfx::B2IBox aDestRect( 0, 0, nWidth, nHeight );

    SvpSalGraphics::ClipUndoHandle aUndo( this );
    if( !isClippedSetup( aDestRect, aUndo ) )
        aCopy->drawBitmap( m_aOrigDevice, aSrcRect, aDestRect, basebmp::DrawMode_PAINT );

    SvpSalBitmap* pBitmap = new SvpSalBitmap();
    pBitmap->setBitmap( aCopy );
    return pBitmap;
}

SalColor SvpSalGraphics::getPixel( long nX, long nY )
{
    basebmp::Color aColor( m_aOrigDevice->getPixel( basegfx::B2IPoint( nX, nY ) ) );
    return aColor.toInt32();
}

void SvpSalGraphics::invert( long nX, long nY, long nWidth, long nHeight, SalInvert /*nFlags*/ )
{
    // FIXME: handle SAL_INVERT_50 and SAL_INVERT_TRACKFRAME
    basegfx::B2DPolygon aRect = basegfx::tools::createPolygonFromRect( basegfx::B2DRectangle( nX, nY, nX+nWidth, nY+nHeight ) );
    basegfx::B2DPolyPolygon aPolyPoly( aRect );
    basegfx::B2IBox aDestRange( nX, nY, nX + nWidth, nY + nHeight );

    SvpSalGraphics::ClipUndoHandle aUndo( this );
    if( !isClippedSetup( aDestRange, aUndo ) )
        m_aDevice->fillPolyPolygon( aPolyPoly, basebmp::Color( 0xffffff ), basebmp::DrawMode_XOR, m_aClipMap );
    dbgOut( m_aDevice );
}

void SvpSalGraphics::invert( sal_uLong nPoints, const SalPoint* pPtAry, SalInvert /*nFlags*/ )
{
    // FIXME: handle SAL_INVERT_50 and SAL_INVERT_TRACKFRAME
    basegfx::B2DPolygon aPoly;
    aPoly.append( basegfx::B2DPoint( pPtAry->mnX, pPtAry->mnY ), nPoints );
    for( sal_uLong i = 1; i < nPoints; i++ )
        aPoly.setB2DPoint( i, basegfx::B2DPoint( pPtAry[i].mnX, pPtAry[i].mnY ) );
    aPoly.setClosed( true );
    ensureClip(); // FIXME for ...
    m_aDevice->fillPolyPolygon( basegfx::B2DPolyPolygon(aPoly), basebmp::Color( 0xffffff ), basebmp::DrawMode_XOR, m_aClipMap );
    dbgOut( m_aDevice );
}

sal_Bool SvpSalGraphics::drawEPS( long, long, long, long, void*, sal_uLong )
{
    return sal_False;
}

SystemGraphicsData SvpSalGraphics::GetGraphicsData() const
{
    return SystemGraphicsData();
}

bool SvpSalGraphics::supportsOperation( OutDevSupportType ) const
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
