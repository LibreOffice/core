/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "openglx11cairotextrender.hxx"

#include "salbmp.hxx"
#include <vcl/salbtype.hxx>

#include <cairo-svg.h>

OpenGLX11CairoTextRender::OpenGLX11CairoTextRender(bool bPrinter, X11SalGraphics& rParent):
    X11CairoTextRender(bPrinter, rParent)
{
}

cairo_surface_t* OpenGLX11CairoTextRender::getCairoSurface()
{
    // static size_t id = 0;
    // OString aFileName = OString("/tmp/libo_logs/text_rendering") + OString::number(id++) + OString(".svg");
    // cairo_surface_t* surface = cairo_svg_surface_create(aFileName.getStr(), GetWidth(), GetHeight());
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, GetWidth(), GetHeight());
    return surface;
}

void OpenGLX11CairoTextRender::drawSurface(cairo_t* cr)
{
    cairo_surface_t* pSurface = cairo_get_target(cr);
    int nWidth = cairo_image_surface_get_width( pSurface );
    int nHeight = cairo_image_surface_get_height( pSurface );
    SalBitmap* pBitmap = ImplGetSVData()->mpDefInst->CreateSalBitmap();
    pBitmap->Create(Size(nWidth, nHeight), 32, BitmapPalette());

    cairo_surface_flush( pSurface );
    BitmapBuffer* pBuffer = pBitmap->AcquireBuffer(false);
    unsigned char *pSrc = cairo_image_surface_get_data( pSurface );
    unsigned int nSrcStride = cairo_image_surface_get_stride( pSurface );
    unsigned int nDestStride = pBuffer->mnScanlineSize;
    for( unsigned long y = 0; y < (unsigned long) nHeight; y++ )
    {
        // Cairo surface is y-inverse
        sal_uInt32 *pSrcPix = (sal_uInt32 *)(pSrc + nSrcStride * (nHeight - y - 1));
        sal_uInt32 *pDestPix = (sal_uInt32 *)(pBuffer->mpBits + nDestStride * y);
        for( unsigned long x = 0; x < (unsigned long) nWidth; x++ )
        {
            sal_uInt8 nAlpha = (*pSrcPix >> 24);
            sal_uInt8 nR = (*pSrcPix >> 16) & 0xff;
            sal_uInt8 nG = (*pSrcPix >> 8) & 0xff;
            sal_uInt8 nB = *pSrcPix & 0xff;
            if( nAlpha != 0 && nAlpha != 255 )
            {
                // Cairo uses pre-multiplied alpha - we do not => re-multiply
                nR = (sal_uInt8) MinMax( ((sal_uInt32)nR * 255) / nAlpha, 0, 255 );
                nG = (sal_uInt8) MinMax( ((sal_uInt32)nG * 255) / nAlpha, 0, 255 );
                nB = (sal_uInt8) MinMax( ((sal_uInt32)nB * 255) / nAlpha, 0, 255 );
            }

            // FIXME: lfrb: depends on endianness (use BitmapWriteAccess)
            *pDestPix = (nAlpha << 24) + (nB << 16) + (nG << 8) + nR;
            pSrcPix++;
            pDestPix++;
        }
    }
    pBitmap->ReleaseBuffer(pBuffer, false);

    SalTwoRect aRect;
    aRect.mnSrcX = 0;
    aRect.mnSrcY = 0;
    aRect.mnSrcWidth = nWidth;
    aRect.mnSrcHeight = nHeight;
    aRect.mnDestX = 0;
    aRect.mnDestY = 0;
    aRect.mnDestWidth = nWidth;
    aRect.mnDestHeight = nHeight;

    mrParent.drawAlphaBitmap(aRect, *pBitmap);
    delete pBitmap;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
