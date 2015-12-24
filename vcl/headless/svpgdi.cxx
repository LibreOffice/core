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
#ifndef IOS
#include "headless/svpcairotextrender.hxx"
#endif
#include "saldatabasic.hxx"

#include <vcl/sysdata.hxx>
#include <config_cairo_canvas.h>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2ibox.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basebmp/scanlineformats.hxx>

#include <cairo.h>

#if OSL_DEBUG_LEVEL > 2
#include <basebmp/debug.hxx>
#include <fstream>
#include <rtl/strbuf.hxx>
#include <sys/stat.h>
#endif

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

#if CAIRO_VERSION < CAIRO_VERSION_ENCODE(1, 10, 0)

#define CAIRO_OPERATOR_DIFFERENCE (static_cast<cairo_operator_t>(23))

struct cairo_rectangle_int_t
{
    double x;
    double y;
    double width;
    double height;
};

#endif

namespace
{
    cairo_rectangle_int_t getFillDamage(cairo_t* cr)
    {
        cairo_rectangle_int_t extents;
        double x1, y1, x2, y2;

        cairo_clip_extents(cr, &x1, &y1, &x2, &y2);
        extents.x = x1, extents.y = y1, extents.width = x2-x1, extents.height = y2-y1;

#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 10, 0)
        cairo_region_t *region = cairo_region_create_rectangle(&extents);

        cairo_fill_extents(cr, &x1, &y1, &x2, &y2);
        extents.x = x1, extents.y = y1, extents.width = x2-x1, extents.height = y2-y1;
        cairo_region_intersect_rectangle(region, &extents);

        cairo_region_get_extents(region, &extents);
        cairo_region_destroy(region);
#endif

        return extents;
    }

    cairo_rectangle_int_t getStrokeDamage(cairo_t* cr)
    {
        cairo_rectangle_int_t extents;
        double x1, y1, x2, y2;

        cairo_clip_extents(cr, &x1, &y1, &x2, &y2);
        extents.x = x1, extents.y = y1, extents.width = x2-x1, extents.height = y2-y1;

#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 10, 0)
        cairo_region_t *region = cairo_region_create_rectangle(&extents);

        cairo_stroke_extents(cr, &x1, &y1, &x2, &y2);
        extents.x = x1, extents.y = y1, extents.width = x2-x1, extents.height = y2-y1;
        cairo_region_intersect_rectangle(region, &extents);

        cairo_region_get_extents(region, &extents);
        cairo_region_destroy(region);
#endif

        return extents;
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
    unsigned char reverseAndInvert(unsigned char b)
    {
        b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
        b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
        b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
        return ~b;
    }
}

bool SvpSalGraphics::drawAlphaBitmap( const SalTwoRect& rTR, const SalBitmap& rSourceBitmap, const SalBitmap& rAlphaBitmap )
{
    if (rAlphaBitmap.GetBitCount() != 8 && rAlphaBitmap.GetBitCount() != 1)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawAlphaBitmap alpha depth case: " << rAlphaBitmap.GetBitCount());
        return false;
    }

    cairo_surface_t* source = nullptr;

    const SvpSalBitmap& rSrc = static_cast<const SvpSalBitmap&>(rSourceBitmap);
    const basebmp::BitmapDeviceSharedPtr& rSrcBmp = rSrc.getBitmap();

    SvpSalBitmap aTmpBmp;
    if (rSourceBitmap.GetBitCount() != 32)
    {
        //big stupid copy here
        static bool bWarnedOnce;
        SAL_WARN_IF(!bWarnedOnce, "vcl.gdi", "non default depth bitmap, slow convert, upscale the input");
        bWarnedOnce = true;
        Size aSize = rSourceBitmap.GetSize();
        aTmpBmp.Create(aSize, 0, BitmapPalette());
        assert(aTmpBmp.GetBitCount() == 32);
        basegfx::B2IBox aRect(0, 0, aSize.Width(), aSize.Height());
        const basebmp::BitmapDeviceSharedPtr& rTmpSrc = aTmpBmp.getBitmap();
        rTmpSrc->drawBitmap(rSrcBmp, aRect, aRect, basebmp::DrawMode::Paint );
        source = createCairoSurface(rTmpSrc);
    }
    else
        source = createCairoSurface(rSrcBmp);

    if (!source)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawAlphaBitmap case");
        return false;
    }

    const SvpSalBitmap& rMask = static_cast<const SvpSalBitmap&>(rAlphaBitmap);
    const basebmp::BitmapDeviceSharedPtr& rMaskBmp = rMask.getBitmap();

    cairo_surface_t *mask = nullptr;

    unsigned char* pAlphaBits = nullptr;

    basegfx::B2IVector size = rMaskBmp->getSize();
    sal_Int32 nStride = rMaskBmp->getScanlineStride();
    basebmp::RawMemorySharedArray data = rMaskBmp->getBuffer();

    if (rAlphaBitmap.GetBitCount() == 8)
    {
        // the alpha values need to be inverted for Cairo
        // so big stupid copy and invert here
        const int nImageSize = size.getY() * nStride;
        const unsigned char* pSrcBits = data.get();
        pAlphaBits = new unsigned char[nImageSize];
        memcpy(pAlphaBits, pSrcBits, nImageSize);

        // TODO: make upper layers use standard alpha
        sal_uInt32* pLDst = reinterpret_cast<sal_uInt32*>(pAlphaBits);
        for( int i = nImageSize/sizeof(sal_uInt32); --i >= 0; ++pLDst )
            *pLDst = ~*pLDst;
        assert(reinterpret_cast<unsigned char*>(pLDst) == pAlphaBits+nImageSize);

        mask = cairo_image_surface_create_for_data(pAlphaBits,
                                        CAIRO_FORMAT_A8,
                                        size.getX(), size.getY(),
                                        nStride);
    }
    else
    {
        // the alpha values need to be inverted *and* reordered for Cairo
        // so big stupid copy and reverse + invert here
        const int nImageSize = size.getY() * nStride;
        const unsigned char* pSrcBits = data.get();
        pAlphaBits = new unsigned char[nImageSize];
        memcpy(pAlphaBits, pSrcBits, nImageSize);

        unsigned char* pDst = pAlphaBits;
        for (int i = nImageSize; --i >= 0; ++pDst)
            *pDst = reverseAndInvert(*pDst);

        mask = cairo_image_surface_create_for_data(pAlphaBits,
                                        CAIRO_FORMAT_A1,
                                        size.getX(), size.getY(),
                                        nStride);
    }

    if (!mask)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawAlphaBitmap case");
        cairo_surface_destroy(source);
        delete[] pAlphaBits;
        return false;
    }

    cairo_t* cr = getCairoContext();
    assert(cr && m_aDevice->isTopDown());

    clipRegion(cr);

    cairo_rectangle_int_t extents = {0, 0, 0, 0};
    basebmp::IBitmapDeviceDamageTrackerSharedPtr xDamageTracker(m_aDevice->getDamageTracker());

    cairo_rectangle(cr, rTR.mnDestX, rTR.mnDestY, rTR.mnDestWidth, rTR.mnDestHeight);

    if (xDamageTracker)
        extents = getFillDamage(cr);

    cairo_clip(cr);

    cairo_translate(cr, rTR.mnDestX, rTR.mnDestY);
    cairo_scale(cr, (double)(rTR.mnDestWidth)/rTR.mnSrcWidth, ((double)rTR.mnDestHeight)/rTR.mnSrcHeight);
    cairo_set_source_surface(cr, source, -rTR.mnSrcX, -rTR.mnSrcY);
    cairo_mask_surface(cr, mask, -rTR.mnSrcX, -rTR.mnSrcY);

    cairo_surface_flush(cairo_get_target(cr));
    cairo_surface_destroy(mask);
    cairo_surface_destroy(source);
    delete[] pAlphaBits;
    cairo_destroy(cr); // unref

    if (xDamageTracker)
    {
        xDamageTracker->damaged(basegfx::B2IBox(extents.x, extents.y, extents.x + extents.width,
                                                extents.y + extents.height));
    }
    return true;
}

bool SvpSalGraphics::drawTransformedBitmap(
    const basegfx::B2DPoint& rNull,
    const basegfx::B2DPoint& rX,
    const basegfx::B2DPoint& rY,
    const SalBitmap& rSourceBitmap,
    const SalBitmap* pAlphaBitmap)
{
    // here direct support for transformed bitmaps can be implemented
    (void)rNull; (void)rX; (void)rY; (void)rSourceBitmap; (void)pAlphaBitmap;
    SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawTransformedBitmap case");
    return false;
}

namespace
{
    bool isCairoCompatible(const basebmp::BitmapDeviceSharedPtr &rBuffer)
    {
        if (!rBuffer)
            return false;

        if (rBuffer->getScanlineFormat() != SVP_CAIRO_FORMAT &&
            rBuffer->getScanlineFormat() != basebmp::Format::OneBitMsbPal)
            return false;

        basegfx::B2IVector size = rBuffer->getSize();
        sal_Int32 nStride = rBuffer->getScanlineStride();
        cairo_format_t nFormat;
        if (rBuffer->getScanlineFormat() == SVP_CAIRO_FORMAT)
            nFormat = CAIRO_FORMAT_ARGB32;
        else
            nFormat = CAIRO_FORMAT_A1;
        return (cairo_format_stride_for_width(nFormat, size.getX()) == nStride);
    }
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
    cairo_t* cr = getCairoContext();
    assert(cr && m_aDevice->isTopDown());

    clipRegion(cr);

    const double fTransparency = (100 - nTransparency) * (1.0/100);

    cairo_rectangle_int_t extents = {0, 0, 0, 0};
    basebmp::IBitmapDeviceDamageTrackerSharedPtr xDamageTracker(m_aDevice->getDamageTracker());

    cairo_rectangle(cr, nX, nY, nWidth, nHeight);

    if (m_bUseFillColor)
    {
        cairo_set_source_rgba(cr, m_aFillColor.getRed()/255.0,
                                  m_aFillColor.getGreen()/255.0,
                                  m_aFillColor.getBlue()/255.0,
                                  fTransparency);

        if (xDamageTracker && !m_bUseLineColor)
            extents = getFillDamage(cr);

        cairo_fill_preserve(cr);
    }

    if (m_bUseLineColor)
    {
        cairo_set_source_rgba(cr, m_aLineColor.getRed()/255.0,
                                  m_aLineColor.getGreen()/255.0,
                                  m_aLineColor.getBlue()/255.0,
                                  fTransparency);

        if (xDamageTracker)
            extents = getStrokeDamage(cr);

        cairo_stroke_preserve(cr);
    }

    cairo_surface_flush(cairo_get_target(cr));
    cairo_destroy(cr); // unref

    if (xDamageTracker)
    {
        xDamageTracker->damaged(basegfx::B2IBox(extents.x, extents.y, extents.x + extents.width,
                                                extents.y + extents.height));
    }
    return true;
}

SvpSalGraphics::SvpSalGraphics() :
    m_bUseLineColor( true ),
    m_aLineColor( COL_BLACK ),
    m_bUseFillColor( false ),
    m_aFillColor( COL_WHITE ),
    m_aDrawMode( basebmp::DrawMode::Paint ),
    m_bClipSetup( false ),
    m_aTextRenderImpl(*this)
{
}

SvpSalGraphics::~SvpSalGraphics()
{
}

void SvpSalGraphics::setDevice( basebmp::BitmapDeviceSharedPtr& rDevice )
{
    m_aOrigDevice = rDevice;
    ResetClipRegion();
}

void SvpSalGraphics::GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY )
{
    rDPIX = rDPIY = 96;
}

#ifndef IOS

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
    m_aClipMap = basebmp::createClipDevice( aSize );

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
                m_aClipMap->fillPolyPolygon(aFull, basebmp::Color(0), basebmp::DrawMode::Paint);
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
        SAL_INFO("vcl.headless", "SvpSalGraphics::isClippedSetup: degenerate case detected ...");
        return true;
    }
    else if( nHit == 1 ) // common path: rendering against just one clipping region
    {
        if( aHitRect.IsInside( aRect ) )
        {
            //The region to be painted (aRect) is equal to or inside the
            //current clipping region
            SAL_INFO("vcl.headless", "SvpSalGraphics::isClippedSetup: is inside ! avoid deeper clip ...");
            return false;
        }
        SAL_INFO("vcl.headless", "SvpSalGraphics::isClippedSetup: operation only overlaps with a single clip zone");
        rUndo.m_aDevice = m_aDevice;
        m_aDevice = basebmp::subsetBitmapDevice( m_aOrigDevice,
                                                 basegfx::B2IBox (aHitRect.Left(),
                                                                  aHitRect.Top(),
                                                                  aHitRect.Right() + 1,
                                                                  aHitRect.Bottom() + 1) );
        return false;
    }
    SAL_INFO("vcl.headless", "SvpSalGraphics::isClippedSetup: URK: complex & slow clipping case\n");
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

// ensureClipFor() or ensureClip() need to be called before
// real rendering. FIXME: we should prolly push this down to
// bitmapdevice instead.
bool SvpSalGraphics::setClipRegion( const vcl::Region& i_rClip )
{
    m_aClipRegion = i_rClip;
    m_aClipMap.reset();
    if( i_rClip.IsEmpty() )
    {
        m_aDevice = m_aOrigDevice;
        m_bClipSetup = true;
        return true;
    }

    RectangleVector aRectangles;
    i_rClip.GetRegionRectangles(aRectangles);

    if (1 == aRectangles.size())
    {
        //simplest case, subset the device to clip bounds
        m_aClipMap.reset();

        const Rectangle& aBoundRect = aRectangles[0];
        m_aDevice = basebmp::subsetBitmapDevice(
            m_aOrigDevice,
            basegfx::B2IBox(aBoundRect.Left(),aBoundRect.Top(),aBoundRect.Right() + 1,aBoundRect.Bottom() + 1) );

        m_bClipSetup = true;
    }
    else
    {
        //more complex, either setup and tear down temporary
        //subsets of the original device around render calls
        //or generate m_aClipMap and pass that to basebmp
        //calls
        m_aDevice = m_aOrigDevice;
        m_bClipSetup = false;
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
    m_aDrawMode = bSet ? basebmp::DrawMode::XOR : basebmp::DrawMode::Paint;
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
    if ((m_bUseLineColor || m_bUseFillColor) && m_aDevice)
    {
        ensureClip(); // FIXME: for ...
        if( m_bUseFillColor )
        {
            basegfx::B2DPolygon aRect = basegfx::tools::createPolygonFromRect( basegfx::B2DRectangle( nX, nY, nX+nWidth, nY+nHeight ) );
            basegfx::B2DPolyPolygon aPolyPoly( aRect );
            m_aDevice->fillPolyPolygon( aPolyPoly, m_aFillColor, m_aDrawMode, m_aClipMap );
        }
        if( m_bUseLineColor )
        {
            // need same -1 hack as X11SalGraphicsImpl::drawRect
            basegfx::B2DPolygon aRect = basegfx::tools::createPolygonFromRect( basegfx::B2DRectangle( nX, nY, nX+nWidth-1, nY+nHeight-1 ) );
            m_aDevice->drawPolygon( aRect, m_aLineColor, m_aDrawMode, m_aClipMap );
        }
    }
    dbgOut( m_aDevice );
}

void SvpSalGraphics::drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry )
{
    if (m_bUseLineColor && nPoints && m_aDevice)
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

void SvpSalGraphics::drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry )
{
    if ((m_bUseLineColor || m_bUseFillColor) && nPoints && m_aDevice)
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

void SvpSalGraphics::drawPolyPolygon( sal_uInt32 nPoly,
                                      const sal_uInt32* pPointCounts,
                                      PCONSTSALPOINT*   pPtAry )
{
    if ((m_bUseLineColor || m_bUseFillColor) && nPoly && m_aDevice)
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

static void AddPolygonToPath(cairo_t* cr, const basegfx::B2DPolygon& rPolygon, bool bClosePath)
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

        if( !nPointIdx )
        {
            // first point => just move there
            cairo_move_to(cr, aPoint.getX(), aPoint.getY());
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
            cairo_curve_to(cr, aCP1.getX(), aCP1.getY(), aCP2.getX(), aCP2.getY(),
                               aPoint.getX(), aPoint.getY());
        }
    }

    if( bClosePath )
    {
        cairo_close_path(cr);
    }
}

bool SvpSalGraphics::drawPolyLine(
    const basegfx::B2DPolygon& rPolyLine,
    double fTransparency,
    const basegfx::B2DVector& rLineWidths,
    basegfx::B2DLineJoin eLineJoin,
    css::drawing::LineCap eLineCap)
{
    // short circuit if there is nothing to do
    const int nPointCount = rPolyLine.count();
    if (nPointCount <= 0)
    {
        return true;
    }

    // reject requests that cannot be handled yet
    if (rLineWidths.getX() != rLineWidths.getY())
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawPolyLine case");
        return false;
    }

    // #i101491# Cairo does not support B2DLineJoin::NONE; return false to use
    // the fallback (own geometry preparation)
    // #i104886# linejoin-mode and thus the above only applies to "fat" lines
    if (basegfx::B2DLineJoin::NONE == eLineJoin && rLineWidths.getX() > 1.3)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawPolyLine case");
        return false;
    }

    cairo_t* cr = getCairoContext();
    assert(cr && m_aDevice->isTopDown());

    clipRegion(cr);

    // setup line attributes
    cairo_line_join_t eCairoLineJoin = CAIRO_LINE_JOIN_MITER;
    switch (eLineJoin)
    {
        case basegfx::B2DLineJoin::NONE:
            eCairoLineJoin = /*TODO?*/CAIRO_LINE_JOIN_MITER;
            break;
        case basegfx::B2DLineJoin::Middle:
            eCairoLineJoin = /*TODO?*/CAIRO_LINE_JOIN_MITER;
            break;
        case basegfx::B2DLineJoin::Bevel:
            eCairoLineJoin = CAIRO_LINE_JOIN_BEVEL;
            break;
        case basegfx::B2DLineJoin::Miter:
            eCairoLineJoin = CAIRO_LINE_JOIN_MITER;
            break;
        case basegfx::B2DLineJoin::Round:
            eCairoLineJoin = CAIRO_LINE_JOIN_ROUND;
            break;
    }

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

    AddPolygonToPath(cr, rPolyLine, rPolyLine.isClosed());

    cairo_rectangle_int_t extents = {0, 0, 0, 0};
    basebmp::IBitmapDeviceDamageTrackerSharedPtr xDamageTracker(m_aDevice->getDamageTracker());

    cairo_set_source_rgba(cr, m_aLineColor.getRed()/255.0,
                              m_aLineColor.getGreen()/255.0,
                              m_aLineColor.getBlue()/255.0,
                              1.0-fTransparency);

    cairo_set_line_join(cr, eCairoLineJoin);
    cairo_set_line_cap(cr, eCairoLineCap);
    cairo_set_line_width(cr, rLineWidths.getX());

    if (xDamageTracker)
        extents = getStrokeDamage(cr);

    cairo_stroke(cr);

    cairo_surface_flush(cairo_get_target(cr));
    cairo_destroy(cr); // unref

    if (xDamageTracker)
    {
        xDamageTracker->damaged(basegfx::B2IBox(extents.x, extents.y, extents.x + extents.width,
                                                extents.y + extents.height));
    }

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

bool SvpSalGraphics::drawPolyPolygon(const basegfx::B2DPolyPolygon& rPolyPoly, double fTransparency)
{
    cairo_t* cr = getCairoContext();
    assert(cr && m_aDevice->isTopDown());

    clipRegion(cr);

    for (const basegfx::B2DPolygon* pPoly = rPolyPoly.begin(); pPoly != rPolyPoly.end(); ++pPoly)
        AddPolygonToPath(cr, *pPoly, true);

    cairo_rectangle_int_t extents = {0, 0, 0, 0};
    basebmp::IBitmapDeviceDamageTrackerSharedPtr xDamageTracker(m_aDevice->getDamageTracker());

    if (m_bUseFillColor)
    {
        cairo_set_source_rgba(cr, m_aFillColor.getRed()/255.0,
                                  m_aFillColor.getGreen()/255.0,
                                  m_aFillColor.getBlue()/255.0,
                                  1.0-fTransparency);

        if (xDamageTracker && !m_bUseLineColor)
            extents = getFillDamage(cr);

        cairo_fill_preserve(cr);
    }

    if (m_bUseLineColor)
    {
        cairo_set_source_rgba(cr, m_aLineColor.getRed()/255.0,
                                  m_aLineColor.getGreen()/255.0,
                                  m_aLineColor.getBlue()/255.0,
                                  1.0-fTransparency);

        if (xDamageTracker)
            extents = getStrokeDamage(cr);

        cairo_stroke_preserve(cr);
    }

    cairo_surface_flush(cairo_get_target(cr));
    cairo_destroy(cr); // unref

    if (xDamageTracker)
    {
        xDamageTracker->damaged(basegfx::B2IBox(extents.x, extents.y, extents.x + extents.width,
                                                extents.y + extents.height));
    }

    return true;
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
        m_aDevice->drawBitmap( m_aOrigDevice, aSrcRect, aDestRect, basebmp::DrawMode::Paint, m_aClipMap );
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
        m_aDevice->drawBitmap( pSrc->m_aOrigDevice, aSrcRect, aDestRect, basebmp::DrawMode::Paint, m_aClipMap );
    dbgOut( m_aDevice );
}

void SvpSalGraphics::drawBitmap( const SalTwoRect& rPosAry,
                                 const SalBitmap& rSalBitmap )
{
    if( !m_aDevice.get() )
        return;

    const SvpSalBitmap& rSrc = static_cast<const SvpSalBitmap&>(rSalBitmap);
    basegfx::B2IBox aSrcRect( rPosAry.mnSrcX, rPosAry.mnSrcY,
                     rPosAry.mnSrcX+rPosAry.mnSrcWidth,
                     rPosAry.mnSrcY+rPosAry.mnSrcHeight );
    basegfx::B2IBox aDestRect( rPosAry.mnDestX, rPosAry.mnDestY,
                      rPosAry.mnDestX+rPosAry.mnDestWidth,
                      rPosAry.mnDestY+rPosAry.mnDestHeight );

    SvpSalGraphics::ClipUndoHandle aUndo( this );
    if( !isClippedSetup( aDestRect, aUndo ) )
        m_aDevice->drawBitmap( rSrc.getBitmap(), aSrcRect, aDestRect, basebmp::DrawMode::Paint, m_aClipMap );
    dbgOut( m_aDevice );
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
    if (!isClippedSetup(aDestRect, aUndo) && m_aDevice)
        m_aDevice->drawMaskedBitmap( rSrc.getBitmap(), rSrcTrans.getBitmap(),
                                     aSrcRect, aDestRect, basebmp::DrawMode::Paint, m_aClipMap );
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
    SvpSalBitmap* pBitmap = new SvpSalBitmap();

    if (m_aOrigDevice)
    {
        basebmp::BitmapDeviceSharedPtr aCopy;
        aCopy = cloneBitmapDevice(basegfx::B2IVector(nWidth, nHeight),
                                   m_aOrigDevice);
        basegfx::B2IVector size = aCopy->getSize();
        basegfx::B2IBox aSrcRect( nX, nY, nX+nWidth, nY+nHeight );
        basegfx::B2IBox aDestRect( 0, 0, nWidth, nHeight );

        aCopy->drawBitmap( m_aOrigDevice, aSrcRect, aDestRect, basebmp::DrawMode::Paint );

        pBitmap->setBitmap( aCopy );
    }

    return pBitmap;
}

SalColor SvpSalGraphics::getPixel( long nX, long nY )
{
    basebmp::Color aColor( m_aOrigDevice->getPixel( basegfx::B2IPoint( nX, nY ) ) );
    return aColor.toInt32();
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

void SvpSalGraphics::invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags )
{
    if (m_aDrawMode != basebmp::DrawMode::XOR)
    {
        cairo_t* cr = getCairoContext();
        assert(cr && m_aDevice->isTopDown());

        clipRegion(cr);

        cairo_rectangle_int_t extents = {0, 0, 0, 0};
        basebmp::IBitmapDeviceDamageTrackerSharedPtr xDamageTracker(m_aDevice->getDamageTracker());

        cairo_rectangle(cr, nX, nY, nWidth, nHeight);

        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);

        if (cairo_version() >= CAIRO_VERSION_ENCODE(1, 10, 0))
        {
            cairo_set_operator(cr, CAIRO_OPERATOR_DIFFERENCE);
        }
        else
        {
            SAL_WARN("vcl.gdi", "SvpSalGraphics::invert, archaic cairo");
        }

        if (nFlags & SAL_INVERT_TRACKFRAME)
        {
            cairo_set_line_width(cr, 2.0);
            const double dashLengths[2] = { 4.0, 4.0 };
            cairo_set_dash(cr, dashLengths, 2, 0);

            if (xDamageTracker)
                extents = getStrokeDamage(cr);

            cairo_stroke(cr);
        }
        else
        {
            if (xDamageTracker)
                extents = getFillDamage(cr);

            cairo_clip(cr);

            if (nFlags & SAL_INVERT_50)
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

        cairo_surface_flush(cairo_get_target(cr));
        cairo_destroy(cr); // unref

        if (xDamageTracker)
        {
            xDamageTracker->damaged(basegfx::B2IBox(extents.x, extents.y, extents.x + extents.width,
                                                    extents.y + extents.height));
        }

        return;
    }

    SAL_WARN("vcl.gdi", "SvpSalGraphics::invert unhandled XOR (?)");

    basegfx::B2DPolygon aRect = basegfx::tools::createPolygonFromRect( basegfx::B2DRectangle( nX, nY, nX+nWidth, nY+nHeight ) );
    basegfx::B2DPolyPolygon aPolyPoly( aRect );
    basegfx::B2IBox aDestRange( nX, nY, nX + nWidth, nY + nHeight );

    SvpSalGraphics::ClipUndoHandle aUndo( this );
    if( !isClippedSetup( aDestRange, aUndo ) )
        m_aDevice->fillPolyPolygon( aPolyPoly, basebmp::Color( 0xffffff ), basebmp::DrawMode::XOR, m_aClipMap );
    dbgOut( m_aDevice );
}

void SvpSalGraphics::invert( sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert /*nFlags*/ )
{
    SAL_WARN("vcl.gdi", "SvpSalGraphics::invert, unhandled points case");

    // FIXME: handle SAL_INVERT_50 and SAL_INVERT_TRACKFRAME
    basegfx::B2DPolygon aPoly;
    aPoly.append( basegfx::B2DPoint( pPtAry->mnX, pPtAry->mnY ), nPoints );
    for( sal_uLong i = 1; i < nPoints; i++ )
        aPoly.setB2DPoint( i, basegfx::B2DPoint( pPtAry[i].mnX, pPtAry[i].mnY ) );
    aPoly.setClosed( true );
    ensureClip(); // FIXME for ...
    m_aDevice->fillPolyPolygon( basegfx::B2DPolyPolygon(aPoly), basebmp::Color( 0xffffff ), basebmp::DrawMode::XOR, m_aClipMap );
    dbgOut( m_aDevice );
}

#endif

bool SvpSalGraphics::drawEPS( long, long, long, long, void*, sal_uLong )
{
    return false;
}

cairo_surface_t* SvpSalGraphics::createCairoSurface(const basebmp::BitmapDeviceSharedPtr &rBuffer)
{
    if (!isCairoCompatible(rBuffer))
        return nullptr;

    basegfx::B2IVector size = rBuffer->getSize();
    sal_Int32 nStride = rBuffer->getScanlineStride();

    basebmp::RawMemorySharedArray data = rBuffer->getBuffer();
    cairo_format_t nFormat;
    if (rBuffer->getScanlineFormat() == SVP_CAIRO_FORMAT)
        nFormat = CAIRO_FORMAT_ARGB32;
    else
        nFormat = CAIRO_FORMAT_A1;
    cairo_surface_t *target =
        cairo_image_surface_create_for_data(data.get(),
                                        nFormat,
                                        size.getX(), size.getY(),
                                        nStride);
    return target;
}

cairo_t* SvpSalGraphics::createCairoContext(const basebmp::BitmapDeviceSharedPtr &rBuffer)
{
    cairo_surface_t *target = createCairoSurface(rBuffer);
    if (!target)
        return nullptr;
    cairo_t* cr = cairo_create(target);
    cairo_surface_destroy(target);
    return cr;
}

cairo_t* SvpSalGraphics::getCairoContext() const
{
    return SvpSalGraphics::createCairoContext(m_aOrigDevice);
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
    if (m_aDrawMode == basebmp::DrawMode::XOR)
        return false;
    if (!isCairoCompatible(m_aOrigDevice))
        return false;
    switch (eType)
    {
        case OutDevSupport_TransparentRect:
        case OutDevSupport_B2DDraw:
            return true;
        case OutDevSupport_B2DClip: //what's this one ?
            return false;
    }
    return false;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
