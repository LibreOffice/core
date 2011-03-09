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

#include "svpgdi.hxx"
#include "svpbmp.hxx"

#include <vcl/sysdata.hxx>
#include <vcl/region.h>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2irange.hxx>
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

#include <svppspgraphics.hxx>

using namespace basegfx;
using namespace basebmp;

inline void dbgOut( const BitmapDeviceSharedPtr&
#if OSL_DEBUG_LEVEL > 2
rDevice
#endif
)
{
    #if OSL_DEBUG_LEVEL > 2
    static int dbgStreamNum = 0;
    rtl::OStringBuffer aBuf( 256 );
    aBuf.append( "debug" );
    mkdir( aBuf.getStr(), 0777 );
    aBuf.append( "/" );
    aBuf.append( sal_Int64(reinterpret_cast<sal_uInt32>(rDevice.get())), 16 );
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

bool SvpSalGraphics::drawAlphaRect( long /*nX*/, long /*nY*/, long /*nWidth*/, long /*nHeight*/, sal_uInt8 /*nTransparency*/ )
{
    // TODO(P3) implement alpha blending
    return false;
}

SvpSalGraphics::SvpSalGraphics() :
    m_bUseLineColor( true ),
    m_aLineColor( COL_BLACK ),
    m_bUseFillColor( false ),
    m_aFillColor( COL_WHITE ),
    m_aTextColor( COL_BLACK ),
    m_aDrawMode( DrawMode_PAINT ),
    m_eTextFmt( Format::EIGHT_BIT_GREY )
{
    for( int i = 0; i < MAX_FALLBACK; ++i )
        m_pServerFont[i] = NULL;
}

SvpSalGraphics::~SvpSalGraphics()
{
}

void SvpSalGraphics::setDevice( BitmapDeviceSharedPtr& rDevice )
{
    m_aDevice = rDevice;
    m_aOrigDevice = rDevice;
    m_aClipMap.reset();

    // determine matching bitmap format for masks
    sal_uInt32 nDeviceFmt = m_aDevice->getScanlineFormat();
    DBG_ASSERT( (nDeviceFmt <= (sal_uInt32)Format::MAX), "SVP::setDevice() with invalid bitmap format" );
    switch( nDeviceFmt )
    {
        case Format::EIGHT_BIT_GREY:
        case Format::SIXTEEN_BIT_LSB_TC_MASK:
        case Format::SIXTEEN_BIT_MSB_TC_MASK:
        case Format::TWENTYFOUR_BIT_TC_MASK:
        case Format::THIRTYTWO_BIT_TC_MASK:
            m_eTextFmt = Format::EIGHT_BIT_GREY;
            break;
        default:
            m_eTextFmt = Format::ONE_BIT_LSB_GREY;
            break;
    }
}

void SvpSalGraphics::GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY )
{
    rDPIX = rDPIY = 96;
}

sal_uInt16 SvpSalGraphics::GetBitCount()
{
    return SvpElement::getBitCountFromScanlineFormat( m_aDevice->getScanlineFormat() );
}

long SvpSalGraphics::GetGraphicsWidth() const
{
    if( m_aDevice.get() )
    {
        B2IVector aSize = m_aDevice->getSize();
        return aSize.getX();
    }
    return 0;
}

void SvpSalGraphics::ResetClipRegion()
{
    m_aDevice = m_aOrigDevice;
    m_aClipMap.reset();
}

bool SvpSalGraphics::setClipRegion( const Region& i_rClip )
{
    if( i_rClip.IsEmpty() )
        m_aClipMap.reset();
    else if( i_rClip.GetRectCount() == 1 )
    {
        m_aClipMap.reset();
        Rectangle aBoundRect( i_rClip.GetBoundRect() );
        m_aDevice = basebmp::subsetBitmapDevice( m_aOrigDevice,
                                                 basegfx::B2IRange(aBoundRect.Left(),aBoundRect.Top(),aBoundRect.Right(),aBoundRect.Bottom()) );
    }
    else
    {
        m_aDevice = m_aOrigDevice;
        B2IVector aSize = m_aDevice->getSize();
        m_aClipMap = createBitmapDevice( aSize, false, Format::ONE_BIT_MSB_GREY );
        m_aClipMap->clear( basebmp::Color(0xFFFFFFFF) );

        ImplRegionInfo aInfo;
        long nX, nY, nW, nH;
        bool bRegionRect = i_rClip.ImplGetFirstRect(aInfo, nX, nY, nW, nH );
        while( bRegionRect )
        {
            if ( nW && nH )
            {
                B2DPolyPolygon aFull;
                aFull.append( tools::createPolygonFromRect( B2DRectangle( nX, nY, nX+nW, nY+nH ) ) );
                m_aClipMap->fillPolyPolygon( aFull, basebmp::Color(0), DrawMode_PAINT );
            }
            bRegionRect = i_rClip.ImplGetNextRect( aInfo, nX, nY, nW, nH );
        }
    }
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
    m_aDrawMode = bSet ? DrawMode_XOR : DrawMode_PAINT;
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

void SvpSalGraphics::SetTextColor( SalColor nSalColor )
{
    m_aTextColor = basebmp::Color( nSalColor );
}

void SvpSalGraphics::drawPixel( long nX, long nY )
{
    if( m_bUseLineColor )
        m_aDevice->setPixel( B2IPoint( nX, nY ),
                             m_aLineColor,
                             m_aDrawMode,
                             m_aClipMap
                             );
    dbgOut( m_aDevice );
}

void SvpSalGraphics::drawPixel( long nX, long nY, SalColor nSalColor )
{
    basebmp::Color aColor( nSalColor );
    m_aDevice->setPixel( B2IPoint( nX, nY ),
                         aColor,
                         m_aDrawMode,
                         m_aClipMap
                         );
    dbgOut( m_aDevice );
}

void SvpSalGraphics::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
    if( m_bUseLineColor )
        m_aDevice->drawLine( B2IPoint( nX1, nY1 ),
                             B2IPoint( nX2, nY2 ),
                             m_aLineColor,
                             m_aDrawMode,
                             m_aClipMap );
    dbgOut( m_aDevice );
}

void SvpSalGraphics::drawRect( long nX, long nY, long nWidth, long nHeight )
{
    if( m_bUseLineColor || m_bUseFillColor )
    {
        B2DPolygon aRect = tools::createPolygonFromRect( B2DRectangle( nX, nY, nX+nWidth, nY+nHeight ) );
        if( m_bUseFillColor )
        {
            B2DPolyPolygon aPolyPoly( aRect );
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
        B2DPolygon aPoly;
        aPoly.append( B2DPoint( pPtAry->mnX, pPtAry->mnY ), nPoints );
        for( sal_uLong i = 1; i < nPoints; i++ )
            aPoly.setB2DPoint( i, B2DPoint( pPtAry[i].mnX, pPtAry[i].mnY ) );
        aPoly.setClosed( false );
        m_aDevice->drawPolygon( aPoly, m_aLineColor, m_aDrawMode, m_aClipMap );
    }
    dbgOut( m_aDevice );
}

void SvpSalGraphics::drawPolygon( sal_uLong nPoints, const SalPoint* pPtAry )
{
    if( ( m_bUseLineColor || m_bUseFillColor ) && nPoints )
    {
        B2DPolygon aPoly;
        aPoly.append( B2DPoint( pPtAry->mnX, pPtAry->mnY ), nPoints );
        for( sal_uLong i = 1; i < nPoints; i++ )
            aPoly.setB2DPoint( i, B2DPoint( pPtAry[i].mnX, pPtAry[i].mnY ) );
        if( m_bUseFillColor )
        {
            aPoly.setClosed( true );
            m_aDevice->fillPolyPolygon( B2DPolyPolygon(aPoly), m_aFillColor, m_aDrawMode, m_aClipMap );
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
        B2DPolyPolygon aPolyPoly;
        for( sal_uInt32 nPolygon = 0; nPolygon < nPoly; nPolygon++ )
        {
            sal_uInt32 nPoints = pPointCounts[nPolygon];
            if( nPoints )
            {
                PCONSTSALPOINT pPoints = pPtAry[nPolygon];
                B2DPolygon aPoly;
                aPoly.append( B2DPoint( pPoints->mnX, pPoints->mnY ), nPoints );
                for( sal_uInt32 i = 1; i < nPoints; i++ )
                    aPoly.setB2DPoint( i, B2DPoint( pPoints[i].mnX, pPoints[i].mnY ) );

                aPolyPoly.append( aPoly );
            }
        }
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

bool SvpSalGraphics::drawPolyLine( const ::basegfx::B2DPolygon&, double /*fTransparency*/, const ::basegfx::B2DVector& /*rLineWidths*/, basegfx::B2DLineJoin /*eJoin*/ )
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
    B2IRange aSrcRect( nSrcX, nSrcY, nSrcX+nSrcWidth, nSrcY+nSrcHeight );
    B2IRange aDestRect( nDestX, nDestY, nDestX+nSrcWidth, nDestY+nSrcHeight );
    m_aDevice->drawBitmap( m_aOrigDevice, aSrcRect, aDestRect, DrawMode_PAINT, m_aClipMap );
    dbgOut( m_aDevice );
}

void SvpSalGraphics::copyBits( const SalTwoRect* pPosAry,
                               SalGraphics*      pSrcGraphics )
{
    SvpSalGraphics* pSrc = pSrcGraphics ?
        static_cast<SvpSalGraphics*>(pSrcGraphics) : this;
    B2IRange aSrcRect( pPosAry->mnSrcX, pPosAry->mnSrcY,
                       pPosAry->mnSrcX+pPosAry->mnSrcWidth,
                       pPosAry->mnSrcY+pPosAry->mnSrcHeight );
    B2IRange aDestRect( pPosAry->mnDestX, pPosAry->mnDestY,
                        pPosAry->mnDestX+pPosAry->mnDestWidth,
                        pPosAry->mnDestY+pPosAry->mnDestHeight );
    m_aDevice->drawBitmap( pSrc->m_aOrigDevice, aSrcRect, aDestRect, DrawMode_PAINT, m_aClipMap );
    dbgOut( m_aDevice );
}

void SvpSalGraphics::drawBitmap( const SalTwoRect* pPosAry,
                                        const SalBitmap& rSalBitmap )
{
    const SvpSalBitmap& rSrc = static_cast<const SvpSalBitmap&>(rSalBitmap);
    B2IRange aSrcRect( pPosAry->mnSrcX, pPosAry->mnSrcY,
                       pPosAry->mnSrcX+pPosAry->mnSrcWidth,
                       pPosAry->mnSrcY+pPosAry->mnSrcHeight );
    B2IRange aDestRect( pPosAry->mnDestX, pPosAry->mnDestY,
                        pPosAry->mnDestX+pPosAry->mnDestWidth,
                        pPosAry->mnDestY+pPosAry->mnDestHeight );
    m_aDevice->drawBitmap( rSrc.getBitmap(), aSrcRect, aDestRect, DrawMode_PAINT, m_aClipMap );
    dbgOut( m_aDevice );
}

void SvpSalGraphics::drawBitmap( const SalTwoRect*,
                                 const SalBitmap&,
                                 SalColor )
{
    // SNI, as in X11 plugin
}

void SvpSalGraphics::drawBitmap( const SalTwoRect* pPosAry,
                                 const SalBitmap& rSalBitmap,
                                 const SalBitmap& rTransparentBitmap )
{
    const SvpSalBitmap& rSrc = static_cast<const SvpSalBitmap&>(rSalBitmap);
    const SvpSalBitmap& rSrcTrans = static_cast<const SvpSalBitmap&>(rTransparentBitmap);
    B2IRange aSrcRect( pPosAry->mnSrcX, pPosAry->mnSrcY,
                       pPosAry->mnSrcX+pPosAry->mnSrcWidth,
                       pPosAry->mnSrcY+pPosAry->mnSrcHeight );
    B2IRange aDestRect( pPosAry->mnDestX, pPosAry->mnDestY,
                        pPosAry->mnDestX+pPosAry->mnDestWidth,
                        pPosAry->mnDestY+pPosAry->mnDestHeight );
    m_aDevice->drawMaskedBitmap( rSrc.getBitmap(), rSrcTrans.getBitmap(), aSrcRect, aDestRect, DrawMode_PAINT, m_aClipMap );
    dbgOut( m_aDevice );
}

void SvpSalGraphics::drawMask( const SalTwoRect* pPosAry,
                               const SalBitmap& rSalBitmap,
                               SalColor nMaskColor )
{
    const SvpSalBitmap& rSrc = static_cast<const SvpSalBitmap&>(rSalBitmap);
    B2IRange aSrcRect( pPosAry->mnSrcX, pPosAry->mnSrcY,
                       pPosAry->mnSrcX+pPosAry->mnSrcWidth,
                       pPosAry->mnSrcY+pPosAry->mnSrcHeight );
    B2IPoint aDestPoint( pPosAry->mnDestX, pPosAry->mnDestY );

    // BitmapDevice::drawMaskedColor works with 0==transparent,
    // 255==opaque. drawMask() semantic is the other way
    // around. Therefore, invert mask.
    BitmapDeviceSharedPtr aCopy =
        cloneBitmapDevice( B2IVector( pPosAry->mnSrcWidth, pPosAry->mnSrcHeight ),
                           rSrc.getBitmap() );
    basebmp::Color aBgColor( COL_WHITE );
    aCopy->clear(aBgColor);
    basebmp::Color aFgColor( COL_BLACK );
    aCopy->drawMaskedColor( aFgColor, rSrc.getBitmap(), aSrcRect, B2IPoint() );

    basebmp::Color aColor( nMaskColor );
    B2IRange aSrcRect2( 0, 0, pPosAry->mnSrcWidth, pPosAry->mnSrcHeight );
    m_aDevice->drawMaskedColor( aColor, aCopy, aSrcRect, aDestPoint, m_aClipMap );
    dbgOut( m_aDevice );
}

SalBitmap* SvpSalGraphics::getBitmap( long nX, long nY, long nWidth, long nHeight )
{
    BitmapDeviceSharedPtr aCopy =
        cloneBitmapDevice( B2IVector( nWidth, nHeight ),
                           m_aDevice );
    B2IRange aSrcRect( nX, nY, nX+nWidth, nY+nHeight );
    B2IRange aDestRect( 0, 0, nWidth, nHeight );
    aCopy->drawBitmap( m_aOrigDevice, aSrcRect, aDestRect, DrawMode_PAINT );

    SvpSalBitmap* pBitmap = new SvpSalBitmap();
    pBitmap->setBitmap( aCopy );
    return pBitmap;
}

SalColor SvpSalGraphics::getPixel( long nX, long nY )
{
    basebmp::Color aColor( m_aDevice->getPixel( B2IPoint( nX, nY ) ) );
    return aColor.toInt32();
}

void SvpSalGraphics::invert( long nX, long nY, long nWidth, long nHeight, SalInvert /*nFlags*/ )
{
    // FIXME: handle SAL_INVERT_50 and SAL_INVERT_TRACKFRAME
    B2DPolygon aRect = tools::createPolygonFromRect( B2DRectangle( nX, nY, nX+nWidth, nY+nHeight ) );
    B2DPolyPolygon aPolyPoly( aRect );
    m_aDevice->fillPolyPolygon( aPolyPoly, basebmp::Color( 0xffffff ), DrawMode_XOR, m_aClipMap );
    dbgOut( m_aDevice );
}

void SvpSalGraphics::invert( sal_uLong nPoints, const SalPoint* pPtAry, SalInvert /*nFlags*/ )
{
    // FIXME: handle SAL_INVERT_50 and SAL_INVERT_TRACKFRAME
    B2DPolygon aPoly;
    aPoly.append( B2DPoint( pPtAry->mnX, pPtAry->mnY ), nPoints );
    for( sal_uLong i = 1; i < nPoints; i++ )
        aPoly.setB2DPoint( i, B2DPoint( pPtAry[i].mnX, pPtAry[i].mnY ) );
    aPoly.setClosed( true );
    m_aDevice->fillPolyPolygon( B2DPolyPolygon(aPoly), basebmp::Color( 0xffffff ), DrawMode_XOR, m_aClipMap );
    dbgOut( m_aDevice );
}

sal_Bool SvpSalGraphics::drawEPS( long, long, long, long, void*, sal_uLong )
{
    return sal_False;
}

SystemFontData SvpSalGraphics::GetSysFontData( int nFallbacklevel ) const
{
    SystemFontData aSysFontData;

    if (nFallbacklevel >= MAX_FALLBACK) nFallbacklevel = MAX_FALLBACK - 1;
    if (nFallbacklevel < 0 ) nFallbacklevel = 0;

    aSysFontData.nSize = sizeof( SystemFontData );
    aSysFontData.nFontId = 0;
    aSysFontData.nFontFlags = 0;
    aSysFontData.bFakeBold = false;
    aSysFontData.bFakeItalic = false;
    aSysFontData.bAntialias = true;
    return aSysFontData;
}

SystemGraphicsData SvpSalGraphics::GetGraphicsData() const
{
    SystemGraphicsData aRes;
    aRes.nSize = sizeof(aRes);
    aRes.hDrawable = 0;
    aRes.pRenderFormat = 0;
    return aRes;
}

bool SvpSalGraphics::supportsOperation( OutDevSupportType ) const
{
    return false;
}

