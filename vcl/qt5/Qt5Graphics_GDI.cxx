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

static const basegfx::B2DPoint aHalfPointOfs ( 0.5, 0.5 );

static void AddPolygonToPath( QPainterPath& rPath,
                              const basegfx::B2DPolygon& rPolygon,
                              bool bClosePath, bool bPixelSnap, bool bLineDraw )
{
    // short circuit if there is nothing to do
    const int nPointCount = rPolygon.count();
    if( nPointCount <= 0 )
        return;

    const bool bHasCurves = rPolygon.areControlPointsUsed();
    for( int nPointIdx = 0, nPrevIdx = 0;; nPrevIdx = nPointIdx++ )
    {
        int nClosedIdx = nPointIdx;
        if( nPointIdx >= nPointCount )
        {
            // prepare to close last curve segment if needed
            if( bClosePath && (nPointIdx == nPointCount) )
                nClosedIdx = 0;
            else
                break;
        }

        basegfx::B2DPoint aPoint = rPolygon.getB2DPoint( nClosedIdx );

        if( bPixelSnap )
        {
            // snap device coordinates to full pixels
            aPoint.setX( basegfx::fround( aPoint.getX() ) );
            aPoint.setY( basegfx::fround( aPoint.getY() ) );
        }

        if( bLineDraw )
            aPoint += aHalfPointOfs;
        if( !nPointIdx )
        {
            // first point => just move there
            rPath.moveTo( aPoint.getX(), aPoint.getY() );
            continue;
        }

        bool bPendingCurve = false;
        if( bHasCurves )
        {
            bPendingCurve = rPolygon.isNextControlPointUsed( nPrevIdx );
            bPendingCurve |= rPolygon.isPrevControlPointUsed( nClosedIdx );
        }

        if( !bPendingCurve )    // line segment
            rPath.lineTo( aPoint.getX(), aPoint.getY() );
        else                    // cubic bezier segment
        {
            basegfx::B2DPoint aCP1 = rPolygon.getNextControlPoint( nPrevIdx );
            basegfx::B2DPoint aCP2 = rPolygon.getPrevControlPoint( nClosedIdx );
            if( bLineDraw )
            {
                aCP1 += aHalfPointOfs;
                aCP2 += aHalfPointOfs;
            }
            rPath.cubicTo( aCP1.getX(), aCP1.getY(),
                           aCP2.getX(), aCP2.getY(), aPoint.getX(), aPoint.getY() );
        }
    }

    if( bClosePath )
        rPath.closeSubpath();
}

static bool AddPolyPolygonToPath( QPainterPath& rPath,
                                  const basegfx::B2DPolyPolygon& rPolyPoly,
                                  bool bPixelSnap, bool bLineDraw )
{
    const int nPolyCount = rPolyPoly.count();
    if( nPolyCount <= 0 )
        return false;
    for( int nPolyIdx = 0; nPolyIdx < nPolyCount; ++nPolyIdx )
    {
        const basegfx::B2DPolygon rPolygon = rPolyPoly.getB2DPolygon( nPolyIdx );
        AddPolygonToPath( rPath, rPolygon, true, bPixelSnap, bLineDraw );
    }
    return true;
}

bool Qt5Graphics::setClipRegion( const vcl::Region& rRegion )
{
    if ( rRegion.IsRectangle() )
    {
        m_aClipRegion = toQRect( rRegion.GetBoundRect() );
        if ( !m_aClipPath.isEmpty() )
        {
            QPainterPath aPath;
            m_aClipPath.swap( aPath );
        }
    }
    else if( !rRegion.HasPolyPolygonOrB2DPolyPolygon() )
    {
        QRegion aQRegion;
        RectangleVector aRectangles;
        rRegion.GetRegionRectangles( aRectangles );
        for ( auto & rRect : aRectangles )
            aQRegion += toQRect( rRect );
        m_aClipRegion = aQRegion;
        if ( !m_aClipPath.isEmpty() )
        {
            QPainterPath aPath;
            m_aClipPath.swap( aPath );
        }
    }
    else
    {
        QPainterPath aPath;
        const basegfx::B2DPolyPolygon aPolyClip( rRegion.GetAsB2DPolyPolygon() );
        AddPolyPolygonToPath( aPath, aPolyClip, !getAntiAliasB2DDraw(), false );
        m_aClipPath.swap( aPath );
        if ( !m_aClipRegion.isEmpty() )
        {
            QRegion aRegion;
            m_aClipRegion.swap( aRegion );
        }
    }
    return true;
}

void Qt5Graphics::ResetClipRegion()
{
    m_aClipRegion = QRegion( m_pQImage->rect() );
    if ( !m_aClipPath.isEmpty() )
    {
        QPainterPath aPath;
        m_aClipPath.swap( aPath );
    }
}

void Qt5Graphics::drawPixel( long nX, long nY )
{
    PREPARE_PAINTER;
    aPainter.drawPoint( nX, nY );
}

void Qt5Graphics::drawPixel( long nX, long nY, SalColor nSalColor )
{
    PREPARE_PAINTER;
    aPainter.setPen( QColor( QRgb( nSalColor )  ) );
    aPainter.setPen( Qt::SolidLine );
    aPainter.drawPoint( nX, nY );
}

void Qt5Graphics::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
    PREPARE_PAINTER;
    aPainter.drawLine( nX1, nY1, nX2, nY2 );
}

void Qt5Graphics::drawRect( long nX, long nY, long nWidth, long nHeight )
{
    PREPARE_PAINTER;
    aPainter.drawRect( nX, nY, nWidth, nHeight );
}

void Qt5Graphics::drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry )
{
    PREPARE_PAINTER;
    QPoint *pPoints = new QPoint[ nPoints ];
    for ( sal_uInt32 i = 0; i < nPoints; ++i, ++pPtAry )
        pPoints[ i ] = QPoint( pPtAry->mnX, pPtAry->mnY );
    aPainter.drawPolyline( pPoints, nPoints );
    delete [] pPoints;
}

void Qt5Graphics::drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry )
{
    PREPARE_PAINTER;
    QPoint *pPoints = new QPoint[ nPoints ];
    for ( sal_uInt32 i = 0; i < nPoints; ++i, ++pPtAry )
        pPoints[ i ] = QPoint( pPtAry->mnX, pPtAry->mnY );
    aPainter.drawPolygon( pPoints, nPoints );
    delete [] pPoints;
}

void Qt5Graphics::drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints, PCONSTSALPOINT* pPtAry )
{
}

bool Qt5Graphics::drawPolyPolygon( const basegfx::B2DPolyPolygon& rPolyPoly,
                                   double fTransparency )
{
    // ignore invisible polygons
    if (SALCOLOR_NONE == m_aFillColor && SALCOLOR_NONE == m_aLineColor )
        return true;
    if( (fTransparency >= 1.0) || (fTransparency < 0) )
        return true;

    QPainterPath aPath;
    // ignore empty polygons
    if( !AddPolyPolygonToPath( aPath, rPolyPoly, !getAntiAliasB2DDraw(),
                               m_aLineColor != SALCOLOR_NONE ) )
        return true;

    PREPARE_PAINTER;

    QBrush aBrush = aPainter.brush();
    aBrush.setStyle( SALCOLOR_NONE == m_aFillColor ? Qt::NoBrush : Qt::SolidPattern );
    aPainter.setBrush( aBrush );

    aPainter.drawPath( aPath );

    return true;
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

bool Qt5Graphics::drawPolyLine( const basegfx::B2DPolygon& rPolyLine,
                                double fTransparency,
                                const basegfx::B2DVector& rLineWidths,
                                basegfx::B2DLineJoin eLineJoin,
                                css::drawing::LineCap eLineCap,
                                double fMiterMinimumAngle )
{
    if (SALCOLOR_NONE == m_aFillColor && SALCOLOR_NONE == m_aLineColor )
        return true;

    if( basegfx::B2DLineJoin::NONE == eLineJoin )
        return false;

    // short circuit if there is nothing to do
    const int nPointCount = rPolyLine.count();
    if( nPointCount <= 0 )
        return true;

    // setup poly-polygon path
    QPainterPath aPath;
    AddPolygonToPath( aPath, rPolyLine, rPolyLine.isClosed(),
                      !getAntiAliasB2DDraw(), true );

    QPainter aPainter;
    PreparePainter( aPainter, 255 * fTransparency );

    // setup line attributes
    QPen aPen = aPainter.pen();
    aPen.setWidth( rLineWidths.getX() );

    switch( eLineJoin )
    {
    case basegfx::B2DLineJoin::NONE: std::abort(); return false;
    case basegfx::B2DLineJoin::Bevel: aPen.setJoinStyle( Qt::BevelJoin ); break;
    case basegfx::B2DLineJoin::Round: aPen.setJoinStyle( Qt::RoundJoin ); break;
    case basegfx::B2DLineJoin::Miter:
        aPen.setMiterLimit( 1.0 / sin(fMiterMinimumAngle / 2.0) );
        aPen.setJoinStyle( Qt::MiterJoin );
        break;
    }

    switch(eLineCap)
    {
    default: // css::drawing::LineCap_BUTT:
        aPen.setCapStyle( Qt::FlatCap ); break;
    case css::drawing::LineCap_ROUND: aPen.setCapStyle( Qt::RoundCap ); break;
    case css::drawing::LineCap_SQUARE: aPen.setCapStyle( Qt::SquareCap ); break;
    }

    aPainter.setPen( aPen );
    aPainter.drawPath( aPath );
    return true;
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

    QImage aImage, *pImage = &aImage;
    if ( !pSrcGraphics || this == pSrcGraphics )
    {
        if ( rPosAry.mnDestX == rPosAry.mnSrcX
                && rPosAry.mnDestY == rPosAry.mnSrcY )
            return;
        aImage = pImage->copy( rPosAry.mnSrcX, rPosAry.mnSrcY,
                               rPosAry.mnSrcWidth, rPosAry.mnSrcHeight );
    }
    else
        pImage = static_cast< Qt5Graphics* >( pSrcGraphics )->m_pQImage;

    PREPARE_PAINTER;

    aPainter.drawImage( QPoint( rPosAry.mnDestX, rPosAry.mnDestY ),
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

    PREPARE_PAINTER;

    const QImage *pImage = static_cast< const Qt5Bitmap* >( &rSalBitmap )->GetQImage();
    assert( pImage );

    aPainter.drawImage( QPoint( rPosAry.mnDestX, rPosAry.mnDestY ),
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
    if( rPosAry.mnSrcWidth <= 0 || rPosAry.mnSrcHeight <= 0
            || rPosAry.mnDestWidth <= 0 || rPosAry.mnDestHeight <= 0 )
        return;

    assert( rPosAry.mnSrcWidth == rPosAry.mnDestWidth );
    assert( rPosAry.mnSrcHeight == rPosAry.mnDestHeight );
}

void Qt5Graphics::drawMask( const SalTwoRect& rPosAry,
                            const SalBitmap& rSalBitmap,
                            SalColor nMaskColor )
{
    if( rPosAry.mnSrcWidth <= 0 || rPosAry.mnSrcHeight <= 0
            || rPosAry.mnDestWidth <= 0 || rPosAry.mnDestHeight <= 0 )
        return;

    assert( rPosAry.mnSrcWidth == rPosAry.mnDestWidth );
    assert( rPosAry.mnSrcHeight == rPosAry.mnDestHeight );
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
    m_aLineColor = SALCOLOR_NONE;
}

void Qt5Graphics::SetLineColor( SalColor nSalColor )
{
    m_aLineColor = nSalColor;
}

void Qt5Graphics::SetFillColor()
{
    m_aFillColor = SALCOLOR_NONE;
}

void Qt5Graphics::SetFillColor( SalColor nSalColor )
{
    m_aFillColor = nSalColor;
}

void Qt5Graphics::SetXORMode( bool bSet )
{
    if ( bSet )
        m_eCompositionMode = QPainter::CompositionMode_Xor;
    else
        m_eCompositionMode = QPainter::CompositionMode_SourceOver;
}

void Qt5Graphics::SetROPLineColor( SalROPColor nROPColor )
{
}

void Qt5Graphics::SetROPFillColor( SalROPColor nROPColor )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
