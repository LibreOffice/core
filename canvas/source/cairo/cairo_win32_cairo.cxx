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

#define _WIN32_WINNT 0x0500

#ifdef WNT
/************************************************************************
 * Win32 surface backend for OpenOffice.org Cairo Canvas                *
 ************************************************************************/

#include <osl/diagnose.h>
#include <vcl/bitmap.hxx>
#include <vcl/virdev.hxx>
#include <vcl/sysdata.hxx>

#include "cairo_win32_cairo.hxx"

#ifdef CAIRO_HAS_WIN32_SURFACE

namespace cairo
{

#include <cairo-win32.h>

    bool IsCairoWorking( OutputDevice* )
    {
        // trivially true for Windows
        return true;
    }

    /**
     * Surface::Surface:     Create generic Canvas surface using given Cairo Surface
     *
     * @param pSurface Cairo Surface
     *
     * This constructor only stores data, it does no processing.
     * It is used with e.g. cairo_image_surface_create_for_data()
     * and Surface::getSimilar()
     *
     * Set the mpSurface to the new surface or NULL
     **/
    Win32Surface::Win32Surface( const CairoSurfaceSharedPtr& pSurface ) :
        mpSurface( pSurface )
    {}

    /**
     * Surface::Surface:   Create Canvas surface from Window reference.
     * @param pSysData Platform native system environment data (struct SystemEnvData in vcl/inc/sysdata.hxx)
     * @param x horizontal location of the new surface
     * @param y vertical location of the new surface
     *
     * pSysData contains the platform native Window reference.
     * pSysData is used to create a surface on the Window
     *
     * Set the mpSurface to the new surface or NULL
     **/
    Win32Surface::Win32Surface( HDC hDC, int x, int y) :
        mpSurface(
            cairo_win32_surface_create(hDC),
            &cairo_surface_destroy)
    {
        cairo_surface_set_device_offset( mpSurface.get(), x, y );
    }

    /**
     * Surface::Surface:   Create platfrom native Canvas surface from BitmapSystemData
     * @param pBmpData Platform native image data (struct BitmapSystemData in vcl/inc/bitmap.hxx)
     *
     * Create a surface based on image data on pBmpData
     *
     * Set the mpSurface to the new surface or NULL
     **/
    Win32Surface::Win32Surface( const BitmapSystemData& rBmpData ) :
        mpSurface()
    {
        OSL_ASSERT(rBmpData.pDIB == NULL);

        if(rBmpData.pDIB != NULL)
        {
            // So just leave mpSurface to NULL, little else we can do at
            // this stage. Hopefully the Win32 patch to
            // cairocanvas::DeviceHelper::getSurface(BitmapSystemData&,
            // const Size&) will catch the cases where this
            // constructor would be called with a DIB bitmap, and we
            // will never get here. At least it worked for Ballmer.ppt.
        }
        else
        {
            HDC hDC = CreateCompatibleDC(NULL);
            void* hOrigBitmap;
            OSL_TRACE ("Surface::Surface(): Selecting bitmap %p into DC %p", rBmpData.pDDB, hDC);
            hOrigBitmap = SelectObject( hDC, (HANDLE)rBmpData.pDDB );
            if(hOrigBitmap == NULL)
                OSL_TRACE ("SelectObject failed: %d", GetLastError ());
            mpSurface.reset(
                cairo_win32_surface_create(hDC),
                &cairo_surface_destroy);
        }
    }

    /**
     * Surface::getCairo:  Create Cairo (drawing object) for the Canvas surface
     *
     * @return new Cairo or NULL
     **/
    CairoSharedPtr Win32Surface::getCairo() const
    {
        return CairoSharedPtr( cairo_create(mpSurface.get()),
                               &cairo_destroy );
    }

    /**
     * Surface::getSimilar:  Create new similar Canvas surface
     * @param aContent format of the new surface (cairo_content_t from cairo/src/cairo.h)
     * @param width width of the new surface
     * @param height height of the new surface
     *
     * Creates a new Canvas surface. This normally creates platform native surface, even though
     * generic function is used.
     *
     * Cairo surface from aContent (cairo_content_t)
     *
     * @return new surface or NULL
     **/
    SurfaceSharedPtr Win32Surface::getSimilar( Content aContent, int width, int height ) const
    {
        return SurfaceSharedPtr(
            new Win32Surface(
                CairoSurfaceSharedPtr(
                    cairo_surface_create_similar( mpSurface.get(), aContent, width, height ),
                    &cairo_surface_destroy )));
    }

    /**
     * Surface::Resize:  Resizes the Canvas surface.
     * @param width new width of the surface
     * @param height new height of the surface
     *
     * Only used on X11.
     *
     * @return The new surface or NULL
     **/
    void Win32Surface::Resize( int /*width*/, int /*height*/ )
    {
        OSL_FAIL("not supposed to be called!");
    }

    void Win32Surface::flush() const
    {
        GdiFlush();
    }

    /**
     * Surface::getDepth:  Get the color depth of the Canvas surface.
     *
     * @return color depth
     **/
    int Win32Surface::getDepth() const
    {
        if (mpSurface)
        {
            switch (cairo_surface_get_content (mpSurface.get()))
            {
                case CAIRO_CONTENT_ALPHA:       return 8;  break;
                case CAIRO_CONTENT_COLOR:       return 24; break;
                case CAIRO_CONTENT_COLOR_ALPHA: return 32; break;
            }
        }
        OSL_TRACE("Canvas::cairo::Surface::getDepth(): ERROR - depth unspecified!");
        return -1;
    }


    /**
     * cairo::createVirtualDevice:  Create a VCL virtual device for the CGContext in the cairo Surface
     *
     * @return The new virtual device
     **/
    boost::shared_ptr<VirtualDevice> Win32Surface::createVirtualDevice() const
    {
        SystemGraphicsData aSystemGraphicsData;
        aSystemGraphicsData.nSize = sizeof(SystemGraphicsData);
        aSystemGraphicsData.hDC = cairo_win32_surface_get_dc( mpSurface.get() );

        return boost::shared_ptr<VirtualDevice>(
            new VirtualDevice( &aSystemGraphicsData, sal::static_int_cast<USHORT>(getDepth()) ));
    }


    /**
     * cairo::createSurface:     Create generic Canvas surface using given Cairo Surface
     *
     * @param rSurface Cairo Surface
     *
     * @return new Surface
     */
    SurfaceSharedPtr createSurface( const CairoSurfaceSharedPtr& rSurface )
    {
        return SurfaceSharedPtr(new Win32Surface(rSurface));
    }


    /**
     * cairo::createSurface:     Create Canvas surface using given VCL Window or Virtualdevice
     *
     * @param rSurface Cairo Surface
     *
     *  For VCL Window, use platform native system environment data (struct SystemEnvData in vcl/inc/sysdata.hxx)
     *  For VCL Virtualdevice, use platform native system graphics data (struct SystemGraphicsData in vcl/inc/sysdata.hxx)
     *
     * @return new Surface
     */
    SurfaceSharedPtr createSurface( const OutputDevice& rRefDevice,
                                    int x, int y, int /* width */, int /* height */)
    {
        SurfaceSharedPtr surf;

        if( rRefDevice.GetOutDevType() == OUTDEV_WINDOW )
        {
            const Window &rWindow = (const Window &) rRefDevice;
            const SystemEnvData* pSysData = GetSysData(&rWindow);
            if (pSysData && pSysData->hWnd)
                surf = SurfaceSharedPtr(new Win32Surface(GetDC((HWND) pSysData->hWnd), x, y));
        }
        else if( rRefDevice.GetOutDevType() == OUTDEV_VIRDEV )
        {
            SystemGraphicsData aSysData = ((const VirtualDevice&) rRefDevice).GetSystemGfxData();
            if (aSysData.hDC)
                surf = SurfaceSharedPtr(new Win32Surface((HDC) aSysData.hDC, x, y));
        }
        return surf;
    }


    /**
     * cairo::createBitmapSurface:   Create platfrom native Canvas surface from BitmapSystemData
     * @param OutputDevice (not used)
     * @param rData Platform native image data (struct BitmapSystemData in vcl/inc/bitmap.hxx)
     * @param rSize width and height of the new surface
     *
     * Create a surface based on image data on rData
     *
     * @return new surface or empty surface
     **/
    SurfaceSharedPtr createBitmapSurface( const OutputDevice&     /* rRefDevice */,
                                          const BitmapSystemData& rData,
                                          const Size&             rSize )
    {
        OSL_TRACE( "requested size: %d x %d available size: %d x %d",
                   rSize.Width(), rSize.Height(), rData.mnWidth, rData.mnHeight );

        if ( rData.mnWidth == rSize.Width() && rData.mnHeight == rSize.Height() )
            return SurfaceSharedPtr(new Win32Surface( rData ));
        else
            return SurfaceSharedPtr();
    }


    /**
     * cairo::ucs4toindex: Convert ucs4 char to glyph index
     * @param ucs4 an ucs4 char
     * @param hfont current font
     *
     * @return true if successful
     **/
    unsigned long ucs4toindex(unsigned int ucs4, HFONT hfont)
    {
        wchar_t unicode[2];
        WORD glyph_index;
        HDC hdc = NULL;

        hdc = CreateCompatibleDC (NULL);

        if (!hdc) return 0;
        if (!SetGraphicsMode (hdc, GM_ADVANCED))
        {
            DeleteDC (hdc);
            return 0;
        }

        SelectObject (hdc, hfont);
        SetMapMode (hdc, MM_TEXT);

        unicode[0] = ucs4;
        unicode[1] = 0;
        if (GetGlyphIndicesW (hdc, unicode, 1, &glyph_index, 0) == GDI_ERROR)
        {
            glyph_index = 0;
        }

        DeleteDC (hdc);
        return glyph_index;
    }


}  // namespace cairo

#endif   // CAIRO_HAS_WIN32_SURFACE

#endif   // WNT

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
