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

#include "Qt5Graphics.hxx"

#include "Qt5Bitmap.hxx"
#include "Qt5Frame.hxx"
#include "Qt5Tools.hxx"

#include <QtGui/QPainter>
#include <QtGui/QScreen>
#include <QtGui/QWindow>
#include <QtWidgets/QWidget>

bool Qt5Graphics::setClipRegion( const vcl::Region& rRegion )
{
    PreparePainter();
    if ( rRegion.IsRectangle() )
        m_aClipRegion = toQRect( rRegion.GetBoundRect() );
    else if( !rRegion.HasPolyPolygonOrB2DPolyPolygon() )
    {
        QRegion aQRegion;
        RectangleVector aRectangles;
        rRegion.GetRegionRectangles( aRectangles );
        for ( auto & rRect : aRectangles )
            aQRegion += toQRect( rRect );
        m_aClipRegion = aQRegion;
    }
    else
    {
        QPolygon aPolygon;
    }
    m_pPainter->setClipRegion( m_aClipRegion );
    return true;
}

void Qt5Graphics::ResetClipRegion()
{
    m_aClipRegion = QRegion( m_pQImage->rect() );
    PreparePainter();
}

void Qt5Graphics::drawPixel( long nX, long nY )
{
    PreparePainter();
    m_pPainter->drawPoint( nX, nY );
}

void Qt5Graphics::drawPixel( long nX, long nY, SalColor nSalColor )
{
    PreparePainter();
    m_pPainter->setPen( QColor( QRgb( nSalColor )  ) );
    m_pPainter->drawPoint( nX, nY );
}

void Qt5Graphics::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
    PreparePainter();
    m_pPainter->drawLine( nX1, nY1, nX2, nY2 );
}

void Qt5Graphics::drawRect( long nX, long nY, long nWidth, long nHeight )
{
    PreparePainter();
    m_pPainter->drawRect( nX, nY, nWidth, nHeight );
}

void Qt5Graphics::drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry )
{
    PreparePainter();
    QPoint *pPoints = new QPoint[ nPoints ];
    for ( sal_uInt32 i = 0; i < nPoints; ++i, ++pPtAry )
        pPoints[ i ] = QPoint( pPtAry->mnX, pPtAry->mnY );
    m_pPainter->drawPolyline( pPoints, nPoints );
    delete [] pPoints;
}

void Qt5Graphics::drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry )
{
    PreparePainter();
    QPoint *pPoints = new QPoint[ nPoints ];
    for ( sal_uInt32 i = 0; i < nPoints; ++i, ++pPtAry )
        pPoints[ i ] = QPoint( pPtAry->mnX, pPtAry->mnY );
    m_pPainter->drawPolygon( pPoints, nPoints );
    delete [] pPoints;
}

void Qt5Graphics::drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints, PCONSTSALPOINT* pPtAry )
{
    if( 0 == nPoly )
        return;
}

bool Qt5Graphics::drawPolyPolygon( const basegfx::B2DPolyPolygon&, double fTransparency )
{
    return false;
}

bool Qt5Graphics::drawPolyLineBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const PolyFlags* pFlgAry )
{
    return false;
}

bool Qt5Graphics::drawPolygonBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const PolyFlags* pFlgAry )
{
    return false;
}

bool Qt5Graphics::drawPolyPolygonBezier( sal_uInt32 nPoly, const sal_uInt32* pPoints,
                                         const SalPoint* const* pPtAry, const PolyFlags* const* pFlgAry )
{
    return false;
}

bool Qt5Graphics::drawPolyLine( const basegfx::B2DPolygon&,
                                double fTransparency,
                                const basegfx::B2DVector& rLineWidths,
                                basegfx::B2DLineJoin,
                                css::drawing::LineCap eLineCap,
                                double fMiterMinimumAngle )
{
    return false;
}

bool Qt5Graphics::drawGradient( const tools::PolyPolygon&, const Gradient& )
{
    return false;
}

void Qt5Graphics::copyArea( long nDestX, long nDestY, long nSrcX, long nSrcY, long nSrcWidth,
                            long nSrcHeight, bool bWindowInvalidate )
{
    if ( nDestX == nSrcX && nDestY == nSrcY )
        return;

    SalTwoRect aTR( nSrcX, nSrcY, nSrcWidth, nSrcHeight,
                    nDestX, nDestY, nSrcWidth, nSrcHeight );
    copyBits( aTR, this );
}

void Qt5Graphics::copyBits( const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics )
{
    if( rPosAry.mnSrcWidth <= 0 || rPosAry.mnSrcHeight <= 0
            || rPosAry.mnDestWidth <= 0 || rPosAry.mnDestHeight <= 0 )
        return;

    assert( rPosAry.mnSrcWidth == rPosAry.mnDestWidth );
    assert( rPosAry.mnSrcHeight == rPosAry.mnDestHeight );

    QImage *pImage;
    QImage aImage;
    if ( !pSrcGraphics || this == pSrcGraphics )
    {
        if ( rPosAry.mnDestX == rPosAry.mnSrcX
                && rPosAry.mnDestY == rPosAry.mnSrcY )
            return;
        aImage = pImage->copy( rPosAry.mnSrcX, rPosAry.mnSrcY,
                               rPosAry.mnSrcWidth, rPosAry.mnSrcHeight );
        pImage = &aImage;
    }
    else
        pImage = static_cast< Qt5Graphics* >( pSrcGraphics )->m_pQImage;

    PreparePainter();
    m_pPainter->drawImage( QPoint( rPosAry.mnDestX, rPosAry.mnDestY ),
        *pImage, QRect( rPosAry.mnSrcX, rPosAry.mnSrcY,
                        rPosAry.mnSrcWidth, rPosAry.mnSrcHeight) );
}

void Qt5Graphics::drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap )
{
    if( rPosAry.mnSrcWidth <= 0 || rPosAry.mnSrcHeight <= 0
            || rPosAry.mnDestWidth <= 0 || rPosAry.mnDestHeight <= 0 )
        return;

    assert( rPosAry.mnSrcWidth == rPosAry.mnDestWidth );
    assert( rPosAry.mnSrcHeight == rPosAry.mnDestHeight );

    PreparePainter();
    const QImage *pImage = static_cast< const Qt5Bitmap* >( &rSalBitmap )->GetQImage();

    m_pPainter->drawImage( QPoint( rPosAry.mnDestX, rPosAry.mnDestY ),
        *pImage, QRect( rPosAry.mnSrcX, rPosAry.mnSrcY,
                        rPosAry.mnSrcWidth, rPosAry.mnSrcHeight) );

    // Workaround to get updates
    if ( m_pFrame )
        m_pFrame->GetQWidget()->update();
}

void Qt5Graphics::drawBitmap( const SalTwoRect& rPosAry,
                              const SalBitmap& rSalBitmap,
                              const SalBitmap& rTransparentBitmap )
{
}

void Qt5Graphics::drawMask( const SalTwoRect& rPosAry,
                                      const SalBitmap& rSalBitmap,
                                      SalColor nMaskColor )
{
}

SalBitmap* Qt5Graphics::getBitmap( long nX, long nY, long nWidth, long nHeight )
{
    return new Qt5Bitmap( m_pQImage->copy( nX, nY, nWidth, nHeight ) );
}

SalColor Qt5Graphics::getPixel( long nX, long nY )
{
    return m_pQImage->pixel( nX, nY );
}

void Qt5Graphics::invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags)
{
}

void Qt5Graphics::invert( sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags )
{
}

bool Qt5Graphics::drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uLong nSize )
{
    return false;
}

bool Qt5Graphics::blendBitmap( const SalTwoRect&,
                                         const SalBitmap& rBitmap )
{
    return false;
}

bool Qt5Graphics::blendAlphaBitmap( const SalTwoRect&,
                                              const SalBitmap& rSrcBitmap,
                                              const SalBitmap& rMaskBitmap,
                                              const SalBitmap& rAlphaBitmap )
{
    return false;
}

bool Qt5Graphics::drawAlphaBitmap( const SalTwoRect&,
                                             const SalBitmap& rSourceBitmap,
                                             const SalBitmap& rAlphaBitmap )
{
    return false;
}

bool Qt5Graphics::drawTransformedBitmap(
                                            const basegfx::B2DPoint& rNull,
                                            const basegfx::B2DPoint& rX,
                                            const basegfx::B2DPoint& rY,
                                            const SalBitmap& rSourceBitmap,
                                            const SalBitmap* pAlphaBitmap)
{
    return false;
}

bool Qt5Graphics::drawAlphaRect( long nX, long nY, long nWidth,
                                           long nHeight, sal_uInt8 nTransparency )
{
    return false;
}

void Qt5Graphics::GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY )
{
    char* pForceDpi;
    if ((pForceDpi = getenv("SAL_FORCEDPI")))
    {
        OString sForceDPI(pForceDpi);
        rDPIX = rDPIY = sForceDPI.toInt32();
        return;
    }

    if ( !m_pFrame || !m_pFrame->GetQWidget()->window()->windowHandle() )
        return;

    QScreen *pScreen = m_pFrame->GetQWidget()->window()->windowHandle()->screen();
    rDPIX = pScreen->physicalDotsPerInchX();
    rDPIY = pScreen->physicalDotsPerInchY();
}

sal_uInt16 Qt5Graphics::GetBitCount() const
{
    return getFormatBits( m_pQImage->format() );
}

long Qt5Graphics::GetGraphicsWidth() const
{
    return m_pQImage->width();
}

void Qt5Graphics::SetLineColor()
{
}

void Qt5Graphics::SetLineColor( SalColor nSalColor )
{
}

void Qt5Graphics::SetFillColor()
{
}

void Qt5Graphics::SetFillColor( SalColor nSalColor )
{
}

void Qt5Graphics::SetXORMode( bool bSet )
{
}

void Qt5Graphics::SetROPLineColor( SalROPColor nROPColor )
{
}

void Qt5Graphics::SetROPFillColor( SalROPColor nROPColor )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
