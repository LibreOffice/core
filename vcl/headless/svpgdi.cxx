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
#include "headless/svpframe.hxx"
#ifndef IOS
#include "headless/svpcairotextrender.hxx"
#endif
#include "saldatabasic.hxx"

#include <vcl/sysdata.hxx>
#include <config_cairo_canvas.h>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2ibox.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include <cairo.h>

#if ENABLE_CAIRO_CANVAS
#if defined CAIRO_VERSION && CAIRO_VERSION < CAIRO_VERSION_ENCODE(1, 10, 0)
#   define CAIRO_OPERATOR_DIFFERENCE (static_cast<cairo_operator_t>(23))
#endif
#endif

namespace
{
    basegfx::B2DRange getClipBox(cairo_t* cr)
    {
        double x1, y1, x2, y2;

        cairo_clip_extents(cr, &x1, &y1, &x2, &y2);

        return basegfx::B2DRange(x1, y1, x2, y2);
    }

    basegfx::B2DRange getFillDamage(cairo_t* cr)
    {
        double x1, y1, x2, y2;

        cairo_fill_extents(cr, &x1, &y1, &x2, &y2);

        return basegfx::B2DRange(x1, y1, x2, y2);
    }

    basegfx::B2DRange getClippedFillDamage(cairo_t* cr)
    {
        basegfx::B2DRange aDamageRect(getFillDamage(cr));
        aDamageRect.intersect(getClipBox(cr));
        return aDamageRect;
    }

    basegfx::B2DRange getStrokeDamage(cairo_t* cr)
    {
        double x1, y1, x2, y2;

        cairo_stroke_extents(cr, &x1, &y1, &x2, &y2);

        return basegfx::B2DRange(x1, y1, x2, y2);
    }

    basegfx::B2DRange getClippedStrokeDamage(cairo_t* cr)
    {
        basegfx::B2DRange aDamageRect(getStrokeDamage(cr));
        aDamageRect.intersect(getClipBox(cr));
        return aDamageRect;
    }
}

#ifndef IOS

bool SvpSalGraphics::blendBitmap( const SalTwoRect&, const SalBitmap& /*rBitmap*/ )
{
    SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::blendBitmap case");
    return false;
}

bool SvpSalGraphics::blendAlphaBitmap( const SalTwoRect&, const SalBitmap&, const SalBitmap&, const SalBitmap& )
{
    SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::blendAlphaBitmap case");
    return false;
}

namespace
{
    class SourceHelper
    {
    public:
        explicit SourceHelper(const SalBitmap& rSourceBitmap)
        {
            const SvpSalBitmap& rSrcBmp = static_cast<const SvpSalBitmap&>(rSourceBitmap);

            if (rSrcBmp.GetBitCount() != 32)
            {
                //big stupid copy here
                static bool bWarnedOnce;
                SAL_WARN_IF(!bWarnedOnce, "vcl.gdi", "non default depth bitmap, slow convert, upscale the input");
                bWarnedOnce = true;

                const BitmapBuffer* pSrc = rSrcBmp.GetBuffer();
                const SalTwoRect aTwoRect = { 0, 0, pSrc->mnWidth, pSrc->mnHeight,
                                              0, 0, pSrc->mnWidth, pSrc->mnHeight };
                aTmpBmp.Create(StretchAndConvert(*pSrc, aTwoRect, SVP_CAIRO_FORMAT));


                assert(aTmpBmp.GetBitCount() == 32);
                source = SvpSalGraphics::createCairoSurface(aTmpBmp.GetBuffer());
            }
            else
                source = SvpSalGraphics::createCairoSurface(rSrcBmp.GetBuffer());
        }
        ~SourceHelper()
        {
            cairo_surface_destroy(source);
        }
        cairo_surface_t* getSurface()
        {
            return source;
        }
    private:
        SvpSalBitmap aTmpBmp;
        cairo_surface_t* source;
    };

    class MaskHelper
    {
    public:
        explicit MaskHelper(const SalBitmap& rAlphaBitmap)
        {
            const SvpSalBitmap& rMask = static_cast<const SvpSalBitmap&>(rAlphaBitmap);
            const BitmapBuffer* pMaskBuf = rMask.GetBuffer();

            if (rAlphaBitmap.GetBitCount() == 8)
            {
                // the alpha values need to be inverted for Cairo
                // so big stupid copy and invert here
                const int nImageSize = pMaskBuf->mnHeight * pMaskBuf->mnScanlineSize;
                pAlphaBits = new unsigned char[nImageSize];
                memcpy(pAlphaBits, pMaskBuf->mpBits, nImageSize);

                // TODO: make upper layers use standard alpha
                sal_uInt32* pLDst = reinterpret_cast<sal_uInt32*>(pAlphaBits);
                for( int i = nImageSize/sizeof(sal_uInt32); --i >= 0; ++pLDst )
                    *pLDst = ~*pLDst;
                assert(reinterpret_cast<unsigned char*>(pLDst) == pAlphaBits+nImageSize);

                mask = cairo_image_surface_create_for_data(pAlphaBits,
                                                CAIRO_FORMAT_A8,
                                                pMaskBuf->mnWidth, pMaskBuf->mnHeight,
                                                pMaskBuf->mnScanlineSize);
            }
            else
            {
                // the alpha values need to be inverted for Cairo
                // so big stupid copy and invert here
                const int nImageSize = pMaskBuf->mnHeight * pMaskBuf->mnScanlineSize;
                pAlphaBits = new unsigned char[nImageSize];
                memcpy(pAlphaBits, pMaskBuf->mpBits, nImageSize);

                // TODO: make upper layers use standard alpha
                unsigned char* pDst = pAlphaBits;
                for (int i = nImageSize; --i >= 0; ++pDst)
                    *pDst = ~*pDst;

                mask = cairo_image_surface_create_for_data(pAlphaBits,
                                                CAIRO_FORMAT_A1,
                                                pMaskBuf->mnWidth, pMaskBuf->mnHeight,
                                                pMaskBuf->mnScanlineSize);
            }
        }
        ~MaskHelper()
        {
            cairo_surface_destroy(mask);
            delete[] pAlphaBits;
        }
        cairo_surface_t* getMask()
        {
            return mask;
        }
    private:
        cairo_surface_t *mask;
        unsigned char* pAlphaBits;
    };
}

bool SvpSalGraphics::drawAlphaBitmap( const SalTwoRect& rTR, const SalBitmap& rSourceBitmap, const SalBitmap& rAlphaBitmap )
{
    if (rAlphaBitmap.GetBitCount() != 8 && rAlphaBitmap.GetBitCount() != 1)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawAlphaBitmap alpha depth case: " << rAlphaBitmap.GetBitCount());
        return false;
    }

    SourceHelper aSurface(rSourceBitmap);
    cairo_surface_t* source = aSurface.getSurface();
    if (!source)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawAlphaBitmap case");
        return false;
    }

    MaskHelper aMask(rAlphaBitmap);
    cairo_surface_t *mask = aMask.getMask();
    if (!mask)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawAlphaBitmap case");
        return false;
    }

    cairo_t* cr = getCairoContext(false);
    clipRegion(cr);

    cairo_rectangle(cr, rTR.mnDestX, rTR.mnDestY, rTR.mnDestWidth, rTR.mnDestHeight);

    basegfx::B2DRange extents = getClippedFillDamage(cr);

    cairo_clip(cr);

    cairo_translate(cr, rTR.mnDestX, rTR.mnDestY);
    cairo_scale(cr, (double)(rTR.mnDestWidth)/rTR.mnSrcWidth, ((double)rTR.mnDestHeight)/rTR.mnSrcHeight);
    cairo_set_source_surface(cr, source, -rTR.mnSrcX, -rTR.mnSrcY);
    cairo_mask_surface(cr, mask, -rTR.mnSrcX, -rTR.mnSrcY);

    releaseCairoContext(cr, false, extents);

    return true;
}

bool SvpSalGraphics::drawTransformedBitmap(
    const basegfx::B2DPoint& rNull,
    const basegfx::B2DPoint& rX,
    const basegfx::B2DPoint& rY,
    const SalBitmap& rSourceBitmap,
    const SalBitmap* pAlphaBitmap)
{
    if (pAlphaBitmap && pAlphaBitmap->GetBitCount() != 8 && pAlphaBitmap->GetBitCount() != 1)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawTransformedBitmap alpha depth case: " << pAlphaBitmap->GetBitCount());
        return false;
    }

    SourceHelper aSurface(rSourceBitmap);
    cairo_surface_t* source = aSurface.getSurface();
    if (!source)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawTransformedBitmap case");
        return false;
    }

    std::unique_ptr<MaskHelper> xMask;
    cairo_surface_t *mask = nullptr;
    if (pAlphaBitmap)
    {
        xMask.reset(new MaskHelper(*pAlphaBitmap));
        mask = xMask->getMask();
        if (!mask)
        {
            SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawTransformedBitmap case");
            return false;
        }
    }

    const Size aSize = rSourceBitmap.GetSize();

    cairo_t* cr = getCairoContext(false);
    clipRegion(cr);

    // setup the image transformation
    // using the rNull,rX,rY points as destinations for the (0,0),(0,Width),(Height,0) source points
    const basegfx::B2DVector aXRel = rX - rNull;
    const basegfx::B2DVector aYRel = rY - rNull;
    cairo_matrix_t matrix;
    cairo_matrix_init(&matrix,
                      aXRel.getX()/aSize.Width(), aXRel.getY()/aSize.Width(),
                      aYRel.getX()/aSize.Height(), aYRel.getY()/aSize.Height(),
                      rNull.getX(), rNull.getY());

    cairo_transform(cr, &matrix);

    cairo_rectangle(cr, 0, 0, aSize.Width(), aSize.Height());
    basegfx::B2DRange extents = getClippedFillDamage(cr);
    cairo_clip(cr);

    cairo_set_source_surface(cr, source, 0, 0);
    if (mask)
        cairo_mask_surface(cr, mask, 0, 0);
    else
        cairo_paint(cr);

    releaseCairoContext(cr, false, extents);

    return true;
}

#endif

void SvpSalGraphics::clipRegion(cairo_t* cr)
{
    RectangleVector aRectangles;
    if (!m_aClipRegion.IsEmpty())
    {
        m_aClipRegion.GetRegionRectangles(aRectangles);
    }
    if (!aRectangles.empty())
    {
        for (RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
        {
            cairo_rectangle(cr, aRectIter->Left(), aRectIter->Top(), aRectIter->GetWidth(), aRectIter->GetHeight());
        }
        cairo_clip(cr);
    }
}

bool SvpSalGraphics::drawAlphaRect(long nX, long nY, long nWidth, long nHeight, sal_uInt8 nTransparency)
{
    cairo_t* cr = getCairoContext(false);
    clipRegion(cr);

    const double fTransparency = (100 - nTransparency) * (1.0/100);

    basegfx::B2DRange extents(0, 0, 0, 0);

    cairo_rectangle(cr, nX, nY, nWidth, nHeight);

    if (m_aFillColor != SALCOLOR_NONE)
    {
        cairo_set_source_rgba(cr, SALCOLOR_RED(m_aFillColor)/255.0,
                                  SALCOLOR_GREEN(m_aFillColor)/255.0,
                                  SALCOLOR_BLUE(m_aFillColor)/255.0,
                                  fTransparency);

        if (m_aLineColor == SALCOLOR_NONE)
            extents = getClippedFillDamage(cr);

        cairo_fill_preserve(cr);
    }

    if (m_aLineColor != SALCOLOR_NONE)
    {
        cairo_set_source_rgba(cr, SALCOLOR_RED(m_aLineColor)/255.0,
                                  SALCOLOR_GREEN(m_aLineColor)/255.0,
                                  SALCOLOR_BLUE(m_aLineColor)/255.0,
                                  fTransparency);

        extents = getClippedStrokeDamage(cr);

        cairo_stroke_preserve(cr);
    }

    releaseCairoContext(cr, false, extents);

    return true;
}

SvpSalGraphics::SvpSalGraphics()
    : m_pSurface(nullptr)
    , m_aLineColor(MAKE_SALCOLOR(0x00, 0x00, 0x00))
    , m_aFillColor(MAKE_SALCOLOR(0xFF, 0xFF, 0XFF))
    , m_ePaintMode(OVERPAINT)
    , m_aTextRenderImpl(*this)
{
}

SvpSalGraphics::~SvpSalGraphics()
{
}

void SvpSalGraphics::setSurface(cairo_surface_t* pSurface)
{
    m_pSurface = pSurface;
    ResetClipRegion();
}

void SvpSalGraphics::GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY )
{
    rDPIX = rDPIY = 96;
}

#ifndef IOS

sal_uInt16 SvpSalGraphics::GetBitCount() const
{
    if (CAIRO_FORMAT_A1 == cairo_image_surface_get_format(m_pSurface))
        return 1;
    return 32;
}

long SvpSalGraphics::GetGraphicsWidth() const
{
    return m_pSurface ? cairo_image_surface_get_width(m_pSurface) : 0;
}

void SvpSalGraphics::ResetClipRegion()
{
    m_aClipRegion.SetNull();
}

bool SvpSalGraphics::setClipRegion( const vcl::Region& i_rClip )
{
    m_aClipRegion = i_rClip;
    return true;
}

void SvpSalGraphics::SetLineColor()
{
    m_aLineColor = SALCOLOR_NONE;
}

void SvpSalGraphics::SetLineColor( SalColor nSalColor )
{
    m_aLineColor = nSalColor;
}

void SvpSalGraphics::SetFillColor()
{
    m_aFillColor = SALCOLOR_NONE;
}

void SvpSalGraphics::SetFillColor( SalColor nSalColor )
{
    m_aFillColor = nSalColor;
}

void SvpSalGraphics::SetXORMode(bool bSet )
{
    m_ePaintMode = bSet ? XOR : OVERPAINT;
}

void SvpSalGraphics::SetROPLineColor( SalROPColor nROPColor )
{
    switch( nROPColor )
    {
        case SalROPColor::N0:
            m_aLineColor = MAKE_SALCOLOR(0, 0, 0);
            break;
        case SalROPColor::N1:
            m_aLineColor = MAKE_SALCOLOR(0xff, 0xff, 0xff);
            break;
        case SalROPColor::Invert:
            m_aLineColor = MAKE_SALCOLOR(0xff, 0xff, 0xff);
            break;
    }
}

void SvpSalGraphics::SetROPFillColor( SalROPColor nROPColor )
{
    switch( nROPColor )
    {
        case SalROPColor::N0:
            m_aFillColor = MAKE_SALCOLOR(0, 0, 0);
            break;
        case SalROPColor::N1:
            m_aFillColor = MAKE_SALCOLOR(0xff, 0xff, 0xff);
            break;
        case SalROPColor::Invert:
            m_aFillColor = MAKE_SALCOLOR(0xff, 0xff, 0xff);
            break;
    }
}

void SvpSalGraphics::drawPixel( long nX, long nY )
{
    if (m_aLineColor != SALCOLOR_NONE)
    {
        drawPixel(nX, nY, m_aLineColor);
    }
}

void SvpSalGraphics::drawPixel( long nX, long nY, SalColor nSalColor )
{
    SalColor aOrigFillColor = m_aFillColor;
    SalColor aOrigLineColor = m_aLineColor;

    basegfx::B2DPolygon aRect = basegfx::tools::createPolygonFromRect(basegfx::B2DRectangle(nX, nY, nX+1, nY+1));
    m_aLineColor = SALCOLOR_NONE;
    m_aFillColor = nSalColor;

    drawPolyPolygon(basegfx::B2DPolyPolygon(aRect));

    m_aFillColor = aOrigFillColor;
    m_aLineColor = aOrigLineColor;
}

void SvpSalGraphics::drawRect( long nX, long nY, long nWidth, long nHeight )
{
    // because of the -1 hack we have to do fill and draw separately
    SalColor aOrigFillColor = m_aFillColor;
    SalColor aOrigLineColor = m_aLineColor;
    m_aFillColor = SALCOLOR_NONE;
    m_aLineColor = SALCOLOR_NONE;

    if (aOrigFillColor != SALCOLOR_NONE)
    {
        basegfx::B2DPolygon aRect = basegfx::tools::createPolygonFromRect(basegfx::B2DRectangle(nX, nY, nX+nWidth, nY+nHeight));
        m_aFillColor = aOrigFillColor;
        drawPolyPolygon(basegfx::B2DPolyPolygon(aRect));
        m_aFillColor = SALCOLOR_NONE;
    }

    if (aOrigLineColor != SALCOLOR_NONE)
    {
        // need same -1 hack as X11SalGraphicsImpl::drawRect
        basegfx::B2DPolygon aRect = basegfx::tools::createPolygonFromRect(basegfx::B2DRectangle( nX, nY, nX+nWidth-1, nY+nHeight-1));
        m_aLineColor = aOrigLineColor;
        drawPolyPolygon(basegfx::B2DPolyPolygon(aRect));
        m_aLineColor = SALCOLOR_NONE;
    }

    m_aFillColor = aOrigFillColor;
    m_aLineColor = aOrigLineColor;
}

void SvpSalGraphics::drawPolyLine(sal_uInt32 nPoints, const SalPoint* pPtAry)
{
    basegfx::B2DPolygon aPoly;
    aPoly.append(basegfx::B2DPoint(pPtAry->mnX, pPtAry->mnY), nPoints);
    for (sal_uInt32 i = 1; i < nPoints; ++i)
        aPoly.setB2DPoint(i, basegfx::B2DPoint(pPtAry[i].mnX, pPtAry[i].mnY));
    aPoly.setClosed(false);

    drawPolyLine(aPoly, 0.0, basegfx::B2DVector(1.0, 1.0), basegfx::B2DLineJoin::Miter,
                 css::drawing::LineCap_BUTT, 15.0 * F_PI180 /*default*/);
}

void SvpSalGraphics::drawPolygon(sal_uInt32 nPoints, const SalPoint* pPtAry)
{
    basegfx::B2DPolygon aPoly;
    aPoly.append(basegfx::B2DPoint(pPtAry->mnX, pPtAry->mnY), nPoints);
    for (sal_uInt32 i = 1; i < nPoints; ++i)
        aPoly.setB2DPoint(i, basegfx::B2DPoint(pPtAry[i].mnX, pPtAry[i].mnY));

    drawPolyPolygon(basegfx::B2DPolyPolygon(aPoly));
}

void SvpSalGraphics::drawPolyPolygon(sal_uInt32 nPoly,
                                     const sal_uInt32* pPointCounts,
                                     PCONSTSALPOINT*   pPtAry)
{
    basegfx::B2DPolyPolygon aPolyPoly;
    for(sal_uInt32 nPolygon = 0; nPolygon < nPoly; ++nPolygon)
    {
        sal_uInt32 nPoints = pPointCounts[nPolygon];
        if (nPoints)
        {
            PCONSTSALPOINT pPoints = pPtAry[nPolygon];
            basegfx::B2DPolygon aPoly;
            aPoly.append( basegfx::B2DPoint(pPoints->mnX, pPoints->mnY), nPoints);
            for (sal_uInt32 i = 1; i < nPoints; ++i)
                aPoly.setB2DPoint(i, basegfx::B2DPoint( pPoints[i].mnX, pPoints[i].mnY));

            aPolyPoly.append(aPoly);
        }
    }

    drawPolyPolygon(aPolyPoly);
}

static const basegfx::B2DPoint aHalfPointOfs(0.5, 0.5);

static void AddPolygonToPath(cairo_t* cr, const basegfx::B2DPolygon& rPolygon, bool bClosePath,
                             bool bPixelSnap, bool bLineDraw)
{
    // short circuit if there is nothing to do
    const int nPointCount = rPolygon.count();
    if( nPointCount <= 0 )
    {
        return;
    }

    const bool bHasCurves = rPolygon.areControlPointsUsed();
    basegfx::B2DPoint aLast;

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
            cairo_move_to(cr, aPoint.getX(), aPoint.getY());
            aLast = aPoint;
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
            cairo_line_to(cr, aPoint.getX(), aPoint.getY());
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

            // tdf#99165 if the control points are 'empty', create the mathematical
            // correct replacement ones to avoid problems with the graphical sub-system
            // tdf#101026 The 1st attempt to create a mathematically correct replacement control
            // vector was wrong. Best alternative is one as close as possible which means short.
            if (aCP1.equal(aLast))
            {
                aCP1 = aLast + ((aCP2 - aLast) * 0.0005);
            }

            if(aCP2.equal(aPoint))
            {
                aCP2 = aPoint + ((aCP1 - aPoint) * 0.0005);
            }

            cairo_curve_to(cr, aCP1.getX(), aCP1.getY(), aCP2.getX(), aCP2.getY(),
                               aPoint.getX(), aPoint.getY());
        }

        aLast = aPoint;
    }

    if( bClosePath )
    {
        cairo_close_path(cr);
    }
}

void SvpSalGraphics::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
    basegfx::B2DPolygon aPoly;
    aPoly.append(basegfx::B2DPoint(nX1, nY1), 2);
    aPoly.setB2DPoint(1, basegfx::B2DPoint(nX2, nY2));
    aPoly.setClosed(false);

    cairo_t* cr = getCairoContext(false);
    clipRegion(cr);

    bool bAA = getAntiAliasB2DDraw();

    cairo_set_antialias(cr, bAA ? CAIRO_ANTIALIAS_DEFAULT : CAIRO_ANTIALIAS_NONE);
    AddPolygonToPath(cr, aPoly, aPoly.isClosed(), !bAA, true);

    applyColor(cr, m_aLineColor);

    basegfx::B2DRange extents = getClippedStrokeDamage(cr);

    cairo_stroke(cr);

    releaseCairoContext(cr, false, extents);
}

bool SvpSalGraphics::drawPolyLine(
    const basegfx::B2DPolygon& rPolyLine,
    double fTransparency,
    const basegfx::B2DVector& rLineWidths,
    basegfx::B2DLineJoin eLineJoin,
    css::drawing::LineCap eLineCap,
    double fMiterMinimumAngle)
{
    // short circuit if there is nothing to do
    const int nPointCount = rPolyLine.count();
    if (nPointCount <= 0)
    {
        return true;
    }

    const bool bNoJoin = (basegfx::B2DLineJoin::NONE == eLineJoin && basegfx::fTools::more(rLineWidths.getX(), 0.0));

    cairo_t* cr = getCairoContext(false);
    clipRegion(cr);

    // setup line attributes
    cairo_line_join_t eCairoLineJoin = CAIRO_LINE_JOIN_MITER;
    switch (eLineJoin)
    {
        case basegfx::B2DLineJoin::Bevel:
            eCairoLineJoin = CAIRO_LINE_JOIN_BEVEL;
            break;
        case basegfx::B2DLineJoin::Round:
            eCairoLineJoin = CAIRO_LINE_JOIN_ROUND;
            break;
        case basegfx::B2DLineJoin::NONE:
        case basegfx::B2DLineJoin::Miter:
            eCairoLineJoin = CAIRO_LINE_JOIN_MITER;
            break;
    }

    // convert miter minimum angle to miter limit
    double fMiterLimit = 1.0 / sin( fMiterMinimumAngle / 2.0);

    // setup cap attribute
    cairo_line_cap_t eCairoLineCap(CAIRO_LINE_CAP_BUTT);

    switch (eLineCap)
    {
        default: // css::drawing::LineCap_BUTT:
        {
            eCairoLineCap = CAIRO_LINE_CAP_BUTT;
            break;
        }
        case css::drawing::LineCap_ROUND:
        {
            eCairoLineCap = CAIRO_LINE_CAP_ROUND;
            break;
        }
        case css::drawing::LineCap_SQUARE:
        {
            eCairoLineCap = CAIRO_LINE_CAP_SQUARE;
            break;
        }
    }

    cairo_set_source_rgba(cr, SALCOLOR_RED(m_aLineColor)/255.0,
                              SALCOLOR_GREEN(m_aLineColor)/255.0,
                              SALCOLOR_BLUE(m_aLineColor)/255.0,
                              1.0-fTransparency);

    cairo_set_line_join(cr, eCairoLineJoin);
    cairo_set_line_cap(cr, eCairoLineCap);
    cairo_set_line_width(cr, rLineWidths.getX());
    cairo_set_miter_limit(cr, fMiterLimit);

    cairo_set_antialias(cr, getAntiAliasB2DDraw() ? CAIRO_ANTIALIAS_DEFAULT : CAIRO_ANTIALIAS_NONE);

    basegfx::B2DRange extents(0, 0, 0, 0);

    if (!bNoJoin)
    {
        AddPolygonToPath(cr, rPolyLine, rPolyLine.isClosed(), !getAntiAliasB2DDraw(), true);
        extents = getClippedStrokeDamage(cr);
        cairo_stroke(cr);
    }
    else
    {
        // emulate rendering::PathJoinType::NONE by painting single edges
        const sal_uInt32 nEdgeCount(rPolyLine.isClosed() ? nPointCount : nPointCount - 1);
        basegfx::B2DPolygon aEdge;
        aEdge.append(rPolyLine.getB2DPoint(0));
        aEdge.append(basegfx::B2DPoint(0.0, 0.0));

        for (sal_uInt32 i = 0; i < nEdgeCount; ++i)
        {
            const sal_uInt32 nNextIndex((i + 1) % nPointCount);
            aEdge.setB2DPoint(1, rPolyLine.getB2DPoint(nNextIndex));
            aEdge.setNextControlPoint(0, rPolyLine.getNextControlPoint(i % nPointCount));
            aEdge.setPrevControlPoint(1, rPolyLine.getPrevControlPoint(nNextIndex));

            AddPolygonToPath(cr, aEdge, false, !getAntiAliasB2DDraw(), true);

            extents.expand(getStrokeDamage(cr));

            cairo_stroke(cr);

            // prepare next step
            aEdge.setB2DPoint(0, aEdge.getB2DPoint(1));
        }

        extents.intersect(getClipBox(cr));
    }

    releaseCairoContext(cr, false, extents);

    return true;
}

bool SvpSalGraphics::drawPolyLineBezier( sal_uInt32,
                                         const SalPoint*,
                                         const sal_uInt8* )
{
    SAL_INFO("vcl.gdi", "unsupported SvpSalGraphics::drawPolyLineBezier case");
    return false;
}

bool SvpSalGraphics::drawPolygonBezier( sal_uInt32,
                                        const SalPoint*,
                                        const sal_uInt8* )
{
    SAL_INFO("vcl.gdi", "unsupported SvpSalGraphics::drawPolygonBezier case");
    return false;
}

bool SvpSalGraphics::drawPolyPolygonBezier( sal_uInt32,
                                            const sal_uInt32*,
                                            const SalPoint* const*,
                                            const sal_uInt8* const* )
{
    SAL_INFO("vcl.gdi", "unsupported SvpSalGraphics::drawPolyPolygonBezier case");
    return false;
}

void SvpSalGraphics::setupPolyPolygon(cairo_t* cr, const basegfx::B2DPolyPolygon& rPolyPoly)
{
    clipRegion(cr);

    for (const auto & rPoly : rPolyPoly)
        AddPolygonToPath(cr, rPoly, true, !getAntiAliasB2DDraw(), m_aLineColor != SALCOLOR_NONE);
}

bool SvpSalGraphics::drawPolyPolygon(const basegfx::B2DPolyPolygon& rPolyPoly, double fTransparency)
{
    cairo_t* cr = getCairoContext(true);

    setupPolyPolygon(cr, rPolyPoly);

    basegfx::B2DRange extents(0, 0, 0, 0);

    if (m_aFillColor != SALCOLOR_NONE)
    {
        cairo_set_source_rgba(cr, SALCOLOR_RED(m_aFillColor)/255.0,
                                  SALCOLOR_GREEN(m_aFillColor)/255.0,
                                  SALCOLOR_BLUE(m_aFillColor)/255.0,
                                  1.0-fTransparency);

        if (m_aLineColor == SALCOLOR_NONE)
            extents = getClippedFillDamage(cr);

        cairo_fill_preserve(cr);
    }

    if (m_aLineColor != SALCOLOR_NONE)
    {
        cairo_set_source_rgba(cr, SALCOLOR_RED(m_aLineColor)/255.0,
                                  SALCOLOR_GREEN(m_aLineColor)/255.0,
                                  SALCOLOR_BLUE(m_aLineColor)/255.0,
                                  1.0-fTransparency);

        extents = getClippedStrokeDamage(cr);

        cairo_stroke_preserve(cr);
    }

    releaseCairoContext(cr, true, extents);

    return true;
}

void SvpSalGraphics::applyColor(cairo_t *cr, SalColor aColor)
{
    if (CAIRO_FORMAT_ARGB32 == cairo_image_surface_get_format(m_pSurface))
    {
        cairo_set_source_rgba(cr, SALCOLOR_RED(aColor)/255.0,
                                  SALCOLOR_GREEN(aColor)/255.0,
                                  SALCOLOR_BLUE(aColor)/255.0,
                                  1.0);
    }
    else
    {
        double fSet = aColor == COL_BLACK ? 0.0 : 1.0;
        cairo_set_source_rgba(cr, 1, 1, 1, fSet);
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    }
}

void SvpSalGraphics::drawPolyPolygon(const basegfx::B2DPolyPolygon& rPolyPoly)
{
    cairo_t* cr = getCairoContext(true);

    setupPolyPolygon(cr, rPolyPoly);

    basegfx::B2DRange extents(0, 0, 0, 0);

    if (m_aFillColor != SALCOLOR_NONE)
    {
        applyColor(cr, m_aFillColor);
        if (m_aLineColor == SALCOLOR_NONE)
            extents = getClippedFillDamage(cr);
        cairo_fill_preserve(cr);
    }

    if (m_aLineColor != SALCOLOR_NONE)
    {
        applyColor(cr, m_aLineColor);
        extents = getClippedStrokeDamage(cr);
        cairo_stroke_preserve(cr);
    }

    releaseCairoContext(cr, true, extents);
}

void SvpSalGraphics::copyArea( long nDestX,
                               long nDestY,
                               long nSrcX,
                               long nSrcY,
                               long nSrcWidth,
                               long nSrcHeight,
                               bool /*bWindowInvalidate*/ )
{
    SalTwoRect aTR(nSrcX, nSrcY, nSrcWidth, nSrcHeight, nDestX, nDestY, nSrcWidth, nSrcHeight);
    copyBits(aTR, this);
}

static basegfx::B2DRange renderSource(cairo_t* cr, const SalTwoRect& rTR,
                                          cairo_surface_t* source)
{
    cairo_rectangle(cr, rTR.mnDestX, rTR.mnDestY, rTR.mnDestWidth, rTR.mnDestHeight);

    basegfx::B2DRange extents = getClippedFillDamage(cr);

    cairo_clip(cr);

    cairo_translate(cr, rTR.mnDestX, rTR.mnDestY);
    if (rTR.mnSrcWidth != 0 && rTR.mnSrcHeight != 0) {
        cairo_scale(cr, (double)(rTR.mnDestWidth)/rTR.mnSrcWidth, ((double)rTR.mnDestHeight)/rTR.mnSrcHeight);
    }
    cairo_set_source_surface(cr, source, -rTR.mnSrcX, -rTR.mnSrcY);
    cairo_paint(cr);

    return extents;
}

void SvpSalGraphics::copySource( const SalTwoRect& rTR,
                                 cairo_surface_t* source )
{
    cairo_t* cr = getCairoContext(false);
    clipRegion(cr);

    basegfx::B2DRange extents = renderSource(cr, rTR, source);

    releaseCairoContext(cr, false, extents);
}

void SvpSalGraphics::copyBits( const SalTwoRect& rTR,
                               SalGraphics*      pSrcGraphics )
{
    SalTwoRect aTR(rTR);

    SvpSalGraphics* pSrc = pSrcGraphics ?
        static_cast<SvpSalGraphics*>(pSrcGraphics) : this;

    cairo_surface_t* source = pSrc->m_pSurface;

    cairo_surface_t *pCopy = nullptr;
    if (pSrc == this)
    {
        //self copy is a problem, so dup source in that case
#if CAIRO_VERSION < CAIRO_VERSION_ENCODE(1, 12, 0)
        pCopy = cairo_surface_create_similar(source,
                                            cairo_surface_get_content(m_pSurface),
                                            aTR.mnSrcWidth,
                                            aTR.mnSrcHeight);
#else
        pCopy = cairo_surface_create_similar_image(source,
                                            cairo_image_surface_get_format(m_pSurface),
                                            aTR.mnSrcWidth,
                                            aTR.mnSrcHeight);
#endif

        cairo_t* cr = cairo_create(pCopy);
        cairo_set_source_surface(cr, source, -aTR.mnSrcX, -aTR.mnSrcY);
        cairo_rectangle(cr, 0, 0, aTR.mnSrcWidth, aTR.mnSrcHeight);
        cairo_fill(cr);
        cairo_destroy(cr);

        source = pCopy;

        aTR.mnSrcX = 0;
        aTR.mnSrcY = 0;
    }

    copySource(aTR, source);

    if (pCopy)
        cairo_surface_destroy(pCopy);
}

void SvpSalGraphics::drawBitmap(const SalTwoRect& rTR, const SalBitmap& rSourceBitmap)
{
    SourceHelper aSurface(rSourceBitmap);
    cairo_surface_t* source = aSurface.getSurface();
    if (!source)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawBitmap case");
    }
    copySource(rTR, source);
}

void SvpSalGraphics::drawBitmap( const SalTwoRect& rTR,
                                 const SalBitmap& rSourceBitmap,
                                 const SalBitmap& rTransparentBitmap )
{
    drawAlphaBitmap(rTR, rSourceBitmap, rTransparentBitmap);
}

static sal_uInt8 unpremultiply(sal_uInt8 c, sal_uInt8 a)
{
    return (a > 0) ? (c * 255 + a / 2) / a : 0;
}

static sal_uInt8 premultiply(sal_uInt8 c, sal_uInt8 a)
{
    return (c * a + 127) / 255;
}

void SvpSalGraphics::drawMask( const SalTwoRect& rTR,
                               const SalBitmap& rSalBitmap,
                               SalColor nMaskColor )
{
    /** creates an image from the given rectangle, replacing all black pixels
     *  with nMaskColor and make all other full transparent */
    SourceHelper aSurface(rSalBitmap);
    cairo_surface_t* mask = aSurface.getSurface();

    cairo_surface_flush(mask);

    unsigned char *mask_data = cairo_image_surface_get_data(mask);

    cairo_format_t nFormat = cairo_image_surface_get_format(mask);
    assert(nFormat == CAIRO_FORMAT_ARGB32 && "need to implement CAIRO_FORMAT_A1 after all here");
    sal_Int32 nStride = cairo_format_stride_for_width(nFormat,
                                                      cairo_image_surface_get_width(mask));
    for (sal_Int32 y = rTR.mnSrcY ; y < rTR.mnSrcY + rTR.mnSrcHeight; ++y)
    {
        unsigned char *row = mask_data + (nStride*y);
        unsigned char *data = row + (rTR.mnSrcX * 4);
        for (sal_Int32 x = rTR.mnSrcX; x < rTR.mnSrcX + rTR.mnSrcWidth; ++x)
        {
            sal_uInt8 b = unpremultiply(data[SVP_CAIRO_BLUE], data[SVP_CAIRO_ALPHA]);
            sal_uInt8 g = unpremultiply(data[SVP_CAIRO_GREEN], data[SVP_CAIRO_ALPHA]);
            sal_uInt8 r = unpremultiply(data[SVP_CAIRO_RED], data[SVP_CAIRO_ALPHA]);
            if (r == 0 && g == 0 && b == 0)
            {
                data[0] = SALCOLOR_BLUE(nMaskColor);
                data[1] = SALCOLOR_GREEN(nMaskColor);
                data[2] = SALCOLOR_RED(nMaskColor);
                data[3] = 0xff;
            }
            else
            {
                data[0] = 0;
                data[1] = 0;
                data[2] = 0;
                data[3] = 0;
            }
            data+=4;
        }
    }
    cairo_surface_mark_dirty(mask);

    cairo_t* cr = getCairoContext(false);
    clipRegion(cr);

    cairo_rectangle(cr, rTR.mnDestX, rTR.mnDestY, rTR.mnDestWidth, rTR.mnDestHeight);

    basegfx::B2DRange extents = getClippedFillDamage(cr);

    cairo_clip(cr);

    cairo_translate(cr, rTR.mnDestX, rTR.mnDestY);
    cairo_scale(cr, (double)(rTR.mnDestWidth)/rTR.mnSrcWidth, ((double)rTR.mnDestHeight)/rTR.mnSrcHeight);
    cairo_set_source_surface(cr, mask, -rTR.mnSrcX, -rTR.mnSrcY);
    cairo_paint(cr);

    releaseCairoContext(cr, false, extents);
}

SalBitmap* SvpSalGraphics::getBitmap( long nX, long nY, long nWidth, long nHeight )
{
    SvpSalBitmap* pBitmap = new SvpSalBitmap();
    pBitmap->Create(Size(nWidth, nHeight), 32, BitmapPalette());

    cairo_surface_t* target = SvpSalGraphics::createCairoSurface(pBitmap->GetBuffer());
    cairo_t* cr = cairo_create(target);

    SalTwoRect aTR(nX, nY, nWidth, nHeight, 0, 0, nWidth, nHeight);
    renderSource(cr, aTR, m_pSurface);

    cairo_destroy(cr);
    cairo_surface_destroy(target);

    return pBitmap;
}

SalColor SvpSalGraphics::getPixel( long nX, long nY )
{
    cairo_surface_flush(m_pSurface);
    cairo_format_t nFormat = cairo_image_surface_get_format(m_pSurface);
    assert(nFormat == CAIRO_FORMAT_ARGB32 && "need to implement CAIRO_FORMAT_A1 after all here");
    sal_Int32 nStride = cairo_format_stride_for_width(nFormat,
                                                      cairo_image_surface_get_width(m_pSurface));
    unsigned char *surface_data = cairo_image_surface_get_data(m_pSurface);
    unsigned char *row = surface_data + (nStride*nY);
    unsigned char *data = row + (nX * 4);
    sal_uInt8 b = unpremultiply(data[SVP_CAIRO_BLUE], data[SVP_CAIRO_ALPHA]);
    sal_uInt8 g = unpremultiply(data[SVP_CAIRO_GREEN], data[SVP_CAIRO_ALPHA]);
    sal_uInt8 r = unpremultiply(data[SVP_CAIRO_RED], data[SVP_CAIRO_ALPHA]);
    return MAKE_SALCOLOR(r, g, b);
}

namespace
{
    cairo_pattern_t * create_stipple()
    {
        static unsigned char data[16] = { 0xFF, 0xFF, 0x00, 0x00,
                                          0xFF, 0xFF, 0x00, 0x00,
                                          0x00, 0x00, 0xFF, 0xFF,
                                          0x00, 0x00, 0xFF, 0xFF };
        cairo_surface_t* surface = cairo_image_surface_create_for_data(data, CAIRO_FORMAT_A8, 4, 4, 4);
        cairo_pattern_t* pattern = cairo_pattern_create_for_surface(surface);
        cairo_surface_destroy(surface);
        cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
        cairo_pattern_set_filter(pattern, CAIRO_FILTER_NEAREST);
        return pattern;
    }
}

void SvpSalGraphics::invert(const basegfx::B2DPolygon &rPoly, SalInvert nFlags)
{
    cairo_t* cr = getCairoContext(false);
    clipRegion(cr);

    basegfx::B2DRange extents(0, 0, 0, 0);

    AddPolygonToPath(cr, rPoly, true, !getAntiAliasB2DDraw(), false);

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);

    if (cairo_version() >= CAIRO_VERSION_ENCODE(1, 10, 0))
    {
        cairo_set_operator(cr, CAIRO_OPERATOR_DIFFERENCE);
    }
    else
    {
        SAL_WARN("vcl.gdi", "SvpSalGraphics::invert, archaic cairo");
    }

    if (nFlags & SalInvert::TrackFrame)
    {
        cairo_set_line_width(cr, 2.0);
        const double dashLengths[2] = { 4.0, 4.0 };
        cairo_set_dash(cr, dashLengths, 2, 0);

        extents = getClippedStrokeDamage(cr);

        cairo_stroke(cr);
    }
    else
    {
        extents = getClippedFillDamage(cr);

        cairo_clip(cr);

        if (nFlags & SalInvert::N50)
        {
            cairo_pattern_t *pattern = create_stipple();
            cairo_mask(cr, pattern);
            cairo_pattern_destroy(pattern);
        }
        else
        {
            cairo_paint(cr);
        }
    }

    releaseCairoContext(cr, false, extents);
}

void SvpSalGraphics::invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags )
{
    basegfx::B2DPolygon aRect = basegfx::tools::createPolygonFromRect(basegfx::B2DRectangle(nX, nY, nX+nWidth, nY+nHeight));

    invert(aRect, nFlags);
}

void SvpSalGraphics::invert(sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags)
{
    basegfx::B2DPolygon aPoly;
    aPoly.append(basegfx::B2DPoint(pPtAry->mnX, pPtAry->mnY), nPoints);
    for (sal_uInt32 i = 1; i < nPoints; ++i)
        aPoly.setB2DPoint(i, basegfx::B2DPoint(pPtAry[i].mnX, pPtAry[i].mnY));
    aPoly.setClosed(true);

    invert(aPoly, nFlags);
}

#endif

bool SvpSalGraphics::drawEPS( long, long, long, long, void*, sal_uLong )
{
    return false;
}

namespace
{
    cairo_format_t getCairoFormat(const BitmapBuffer& rBuffer)
    {
        cairo_format_t nFormat;
        assert(rBuffer.mnBitCount == 32 || rBuffer.mnBitCount == 1);
        if (rBuffer.mnBitCount == 32)
            nFormat = CAIRO_FORMAT_ARGB32;
        else
            nFormat = CAIRO_FORMAT_A1;
        return nFormat;
    }

    bool isCairoCompatible(const BitmapBuffer* pBuffer)
    {
        if (!pBuffer)
            return false;

        if (pBuffer->mnBitCount != 32 && pBuffer->mnBitCount != 1)
            return false;

        cairo_format_t nFormat = getCairoFormat(*pBuffer);
        return (cairo_format_stride_for_width(nFormat, pBuffer->mnWidth) == pBuffer->mnScanlineSize);
    }
}

cairo_surface_t* SvpSalGraphics::createCairoSurface(const BitmapBuffer *pBuffer)
{
    if (!isCairoCompatible(pBuffer))
        return nullptr;

    cairo_format_t nFormat = getCairoFormat(*pBuffer);
    cairo_surface_t *target =
        cairo_image_surface_create_for_data(pBuffer->mpBits,
                                        nFormat,
                                        pBuffer->mnWidth, pBuffer->mnHeight,
                                        pBuffer->mnScanlineSize);
    return target;
}

static cairo_t* createTmpCompatibleCairoContext(cairo_surface_t* pSurface)
{
    cairo_surface_t *target = cairo_image_surface_create(
                                cairo_image_surface_get_format(pSurface),
                                cairo_image_surface_get_width(pSurface),
                                cairo_image_surface_get_height(pSurface));
    return cairo_create(target);
}

cairo_t* SvpSalGraphics::getCairoContext(bool bXorModeAllowed) const
{
    cairo_t* cr;
    if (m_ePaintMode == XOR && bXorModeAllowed)
        cr = createTmpCompatibleCairoContext(m_pSurface);
    else
        cr = cairo_create(m_pSurface);
    cairo_set_line_width(cr, 1);
    cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
    if (m_ePaintMode == INVERT)
        cairo_set_operator(cr, CAIRO_OPERATOR_DIFFERENCE);
    else
        cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    return cr;
}

cairo_user_data_key_t* SvpSalGraphics::getDamageKey()
{
    static cairo_user_data_key_t aDamageKey;
    return &aDamageKey;
}

void SvpSalGraphics::releaseCairoContext(cairo_t* cr, bool bXorModeAllowed, const basegfx::B2DRange& rExtents) const
{
    if (rExtents.isEmpty())
    {
        //nothing changed, return early
        cairo_destroy(cr);
        return;
    }

    sal_Int32 nExtentsLeft(rExtents.getMinX()), nExtentsTop(rExtents.getMinY());
    sal_Int32 nExtentsRight(rExtents.getMaxX()), nExtentsBottom(rExtents.getMaxY());
    sal_Int32 nWidth = cairo_image_surface_get_width(m_pSurface);
    sal_Int32 nHeight = cairo_image_surface_get_height(m_pSurface);
    nExtentsLeft = std::max<sal_Int32>(nExtentsLeft, 0);
    nExtentsTop = std::max<sal_Int32>(nExtentsTop, 0);
    nExtentsRight = std::min<sal_Int32>(nExtentsRight, nWidth);
    nExtentsBottom = std::min<sal_Int32>(nExtentsBottom, nHeight);

    cairo_surface_t* surface = cairo_get_target(cr);
    cairo_surface_flush(surface);

    //For the most part we avoid the use of XOR these days, but there
    //are some edge cases where legacy stuff still supports it, so
    //emulate it (slowly) here.
    if (m_ePaintMode == XOR && bXorModeAllowed)
    {
        cairo_surface_t* true_surface = m_pSurface;
        cairo_surface_flush(true_surface);
        unsigned char *true_surface_data = cairo_image_surface_get_data(true_surface);
        unsigned char *xor_surface_data = cairo_image_surface_get_data(surface);

        cairo_format_t nFormat = cairo_image_surface_get_format(m_pSurface);
        assert(nFormat == CAIRO_FORMAT_ARGB32 && "need to implement CAIRO_FORMAT_A1 after all here");
        sal_Int32 nStride = cairo_format_stride_for_width(nFormat, nWidth);
        for (sal_Int32 y = nExtentsTop; y < nExtentsBottom; ++y)
        {
            unsigned char *true_row = true_surface_data + (nStride*y);
            unsigned char *xor_row = xor_surface_data + (nStride*y);
            unsigned char *true_data = true_row + (nExtentsLeft * 4);
            unsigned char *xor_data = xor_row + (nExtentsLeft * 4);
            for (sal_Int32 x = nExtentsLeft; x < nExtentsRight; ++x)
            {
                sal_uInt8 b = unpremultiply(true_data[SVP_CAIRO_BLUE], true_data[SVP_CAIRO_ALPHA]) ^
                              unpremultiply(xor_data[SVP_CAIRO_BLUE], xor_data[SVP_CAIRO_ALPHA]);
                sal_uInt8 g = unpremultiply(true_data[SVP_CAIRO_GREEN], true_data[SVP_CAIRO_ALPHA]) ^
                              unpremultiply(xor_data[SVP_CAIRO_GREEN], xor_data[SVP_CAIRO_ALPHA]);
                sal_uInt8 r = unpremultiply(true_data[SVP_CAIRO_RED], true_data[SVP_CAIRO_ALPHA]) ^
                              unpremultiply(xor_data[SVP_CAIRO_RED], xor_data[SVP_CAIRO_ALPHA]);
                true_data[0] = premultiply(b, true_data[SVP_CAIRO_ALPHA]);
                true_data[1] = premultiply(g, true_data[SVP_CAIRO_ALPHA]);
                true_data[2] = premultiply(r, true_data[SVP_CAIRO_ALPHA]);
                true_data+=4;
                xor_data+=4;
            }
        }
        cairo_surface_mark_dirty(true_surface);
        cairo_surface_destroy(surface);
    }

    cairo_destroy(cr); // unref

    DamageHandler* pDamage = static_cast<DamageHandler*>(cairo_surface_get_user_data(m_pSurface, getDamageKey()));

    if (pDamage)
    {
        pDamage->damaged(pDamage->handle, nExtentsLeft, nExtentsTop, nExtentsRight, nExtentsBottom);
    }
}

#if ENABLE_CAIRO_CANVAS
bool SvpSalGraphics::SupportsCairo() const
{
    return false;
}

cairo::SurfaceSharedPtr SvpSalGraphics::CreateSurface(const cairo::CairoSurfaceSharedPtr& /*rSurface*/) const
{
    return cairo::SurfaceSharedPtr();
}

cairo::SurfaceSharedPtr SvpSalGraphics::CreateSurface(const OutputDevice& /*rRefDevice*/, int /*x*/, int /*y*/, int /*width*/, int /*height*/) const
{
    return cairo::SurfaceSharedPtr();
}

cairo::SurfaceSharedPtr SvpSalGraphics::CreateBitmapSurface(const OutputDevice& /*rRefDevice*/, const BitmapSystemData& /*rData*/, const Size& /*rSize*/) const
{
    return cairo::SurfaceSharedPtr();
}

css::uno::Any SvpSalGraphics::GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& /*rSurface*/, const basegfx::B2ISize& /*rSize*/) const
{
    return css::uno::Any();
}

#endif // ENABLE_CAIRO_CANVAS

#ifndef IOS

SystemGraphicsData SvpSalGraphics::GetGraphicsData() const
{
    return SystemGraphicsData();
}

bool SvpSalGraphics::supportsOperation(OutDevSupportType eType) const
{
    switch (eType)
    {
        case OutDevSupport_TransparentRect:
        case OutDevSupport_B2DDraw:
            return true;
    }
    return false;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
