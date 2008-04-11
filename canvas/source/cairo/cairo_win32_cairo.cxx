/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cairo_win32_cairo.cxx,v $
 *
 * $Revision: 1.3 $
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


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"


#ifdef WNT
/************************************************************************
 * Win32 surface backend for OpenOffice.org Cairo Canvas                *
 ************************************************************************/

#include <tools/prewin.h>
#include <windows.h>
#include <tools/postwin.h>

#include <osl/diagnose.h>

#include "cairo_cairo.hxx"

#ifdef CAIRO_HAS_WIN32_SURFACE

#include <cairo-win32.h>

namespace cairo
{
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
    Surface::Surface( cairo_surface_t* pSurface )
            : mnRefCount( 1 ),
              mpSurface( pSurface )
    {
    }


  /**
   * Surface::Surface:   Create Canvas surface from Window reference.
   * @param pSysData Platform native system environment data (struct SystemEnvData in vcl/inc/sysdata.hxx)
   * @param x horizontal location of the new surface
   * @param y vertical location of the new surface
   * @param width width of the new surface
   * @param height height of the new surface
   *
   * pSysData contains the platform native Window reference.
   * pSysData is used to create a surface on the Window
   *
   * Set the mpSurface to the new surface or NULL
   **/
    Surface::Surface( const SystemEnvData* pSysData, int x, int y, int /* width */, int /* height */ )
        : mnRefCount( 1 ),
          mpSurface( NULL )
    {
        HDC hDC;

        hDC = GetDC( pSysData->hWnd );
        mpSurface = cairo_win32_surface_create( hDC );
        cairo_surface_set_device_offset( mpSurface, x, y );
    }


  /**
   * Surface::Surface:   Create platfrom native Canvas surface from BitmapSystemData
   * @param pSysData (not used)
   * @param pBmpData Platform native image data (struct BitmapSystemData in vcl/inc/bitmap.hxx)
   * @param width width of the new surface
   * @param height height of the new surface
   *
   * Create a surface based on image data on pBmpData
   *
   * Set the mpSurface to the new surface or NULL
   **/
    Surface::Surface( const SystemEnvData* /* pSysData */, const BitmapSystemData* pBmpData, int width, int height )
        : mnRefCount( 1 ),
          mpSurface( NULL )
    {
        OSL_ASSERT (pBmpData->mnWidth == width && pBmpData->mnHeight == height);

        // NOTE: width and height are not used in win32
        OSL_ASSERT (pBmpData->pDIB == NULL);

        // FIXME: could we use
        // cairo_win32_surface_create_with_ddb (HDC hdc, cairo_format_t format, int width, int height);
        // and
        // cairo_win32_surface_create_with_dib (cairo_format_t format, int width, int height);
        // instead?
        //

        if (pBmpData->pDIB != NULL) {
#if 0
            // This code will not work anyway, as most (?) DIBs that
            // come here will be in bottom-down order (upside-down)
            // and in any case have a different order of colour
            // channels compared to what cairo expects.
            PBITMAPINFOHEADER pBIH = (PBITMAPINFOHEADER) GlobalLock ((HANDLE) p_BmpData->pDIB);
            cairo_format_t fmt;
            OSL_ASSERT (pBIH->biBitCount == 24 && pBIH->biCompression == BI_RGB);
            mhDIB = p_BmpData->pDIB;
            if (pBIH->biBitCount == 24 && pBIH->biCompression == BI_RGB)
            {
                mpSurface = cairo_image_surface_create_for_data (((unsigned char *) pBIH) + pBIH->biSize,
                                                                 CAIRO_FORMAT_RGB24,
                                                                 pBIH->biWidth, pBIH->biHeight,
                                                                 4*((3*pBIH->biWidth-1)/4+1));
            }
#else
            // So just leave mpSurface to NULL, little else we can do at
            // this stage. Hopefully the Win32 patch to
            // cairocanvas::DeviceHelper::getSurface(BitmapSystemData&,
            // const Size&) will catch the cases where this
            // constructor would be called with a DIB bitmap, and we
            // will never get here. At least it worked for Ballmer.ppt.
#endif
        } else {
            HDC hDC;
            void* hOrigBitmap;
            hDC = CreateCompatibleDC( NULL );
            OSL_TRACE ("::cairo::cairo::Surface::Surface(): Selecting bitmap %p into DC %p", pBmpData->pDDB, hDC);
            hOrigBitmap = SelectObject( hDC, (HANDLE) pBmpData->pDDB );
            if (hOrigBitmap == NULL)
                OSL_TRACE ("SelectObject failed: %d", GetLastError ());
            mpSurface = cairo_win32_surface_create( hDC );
        }

    }


  /**
   * Surface::getDepth:  Get the color depth of the Canvas surface.
   *
   * @return color depth
   **/
    int
    Surface::getDepth()
    {
        if (mpSurface) {
            switch (cairo_surface_get_content (mpSurface)) {
                case CAIRO_CONTENT_ALPHA:       return 8;  break;
                case CAIRO_CONTENT_COLOR:       return 24; break;
                case CAIRO_CONTENT_COLOR_ALPHA: return 32; break;
            }
        }
        OSL_TRACE("Canvas::cairo::Surface::getDepth(): ERROR - depth unspecified!");
        return -1;
    }


  /**
   * Surface::fillSystemGraphicsData:   Fill SystemGraphicsData with native surface data
   * @param aSystemGraphicsData Platform native system graphics data (struct SystemGraphicsData in vcl/inc/sysdata.hxx)
   *
   */
    void Surface::fillSystemGraphicsData( SystemGraphicsData& aSystemGraphicsData)
    {
        aSystemGraphicsData.hDC = cairo_win32_surface_get_dc( mpSurface );
    }

}  // namespace cairo

#endif   // CAIRO_HAS_WIN32_SURFACE

#endif   // WNT
