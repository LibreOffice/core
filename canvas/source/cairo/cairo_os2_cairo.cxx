/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"

#ifdef OS2
/************************************************************************
 * OS/2 surface backend for Apache OpenOffice Cairo Canvas              *
 ************************************************************************/

#define INCL_WIN
#include <os2.h>

#include <osl/diagnose.h>
#include <vcl/bitmap.hxx>
#include <vcl/virdev.hxx>
#include <vcl/sysdata.hxx>

#include "cairo_os2_cairo.hxx"

namespace cairo
{

#include <cairo/cairo-os2.h>

// workaround for svpm.h definitions
#define sal_Bool BOOL
#define PM_FIXED FIXED
#define LPGLYPHMETRICS void*
#define MAT2 void
#include <ft2lib.h>

    bool IsCairoWorking( OutputDevice* )
    {
        // trivially true for OS/2
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
    Os2Surface::Os2Surface( const CairoSurfaceSharedPtr& pSurface ) :
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
    Os2Surface::Os2Surface( HWND hwnd, int x, int y, int w, int h) :
        mpSurface(
            cairo_os2_surface_create_for_window( hwnd, w + x, h + y),
            &cairo_surface_destroy)
    {
        OSL_TRACE( "Os2Surface::Os2Surface hwnd:%x offset: %d,%d size %d x %d",
                   hwnd, x, y, w, h);
        cairo_surface_set_device_offset( mpSurface.get(), 0, 0);
    }

    /**
     * Surface::Surface:   Create platform native Canvas surface from BitmapSystemData
     * @param pBmpData Platform native image data (struct BitmapSystemData in vcl/inc/bitmap.hxx)
     *
     * Create a surface based on image data on pBmpData
     *
     * Set the mpSurface to the new surface or NULL
     **/
    Os2Surface::Os2Surface( const BitmapSystemData& rBmpData ) :
        mpSurface()
    {
        OSL_TRACE( "Os2Surface::Os2Surface bitmap");
#if 0
        OSL_ASSERT(rBmpData.pDIB == NULL);

        if(rBmpData.pDIB != NULL) {
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
#endif
    }

    /**
     * Surface::getCairo:  Create Cairo (drawing object) for the Canvas surface
     *
     * @return new Cairo or NULL
     **/
    CairoSharedPtr Os2Surface::getCairo() const
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
    SurfaceSharedPtr Os2Surface::getSimilar( Content aContent, int width, int height ) const
    {
        OSL_TRACE( "Os2Surface::getSimilar size: %d x %d", width, height);
        // cairo hits assertion in cairo-surface.c#535 if size is 0x0
        int w = (width == 0 ? 1 : width);
        int h = (height == 0 ? 1 : height);
        return SurfaceSharedPtr(
            new Os2Surface(
                CairoSurfaceSharedPtr(
                    cairo_surface_create_similar( mpSurface.get(), aContent, w, h),
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
    void Os2Surface::Resize( int width, int height)
    {
        cairo_os2_surface_set_size( mpSurface.get(), width, height, false);
    }

    void Os2Surface::flush() const
    {
        OSL_TRACE( "Os2Surface::flush");
        cairo_os2_surface_paint_window( mpSurface.get(), NULL, NULL, 0);
    }

    /**
     * Surface::getDepth:  Get the color depth of the Canvas surface.
     *
     * @return color depth
     **/
    int Os2Surface::getDepth() const
    {
        OSL_TRACE( "Os2Surface::getDepth");
        if (mpSurface) {
            switch (cairo_surface_get_content (mpSurface.get())) {
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
    boost::shared_ptr<VirtualDevice> Os2Surface::createVirtualDevice() const
    {
        SystemGraphicsData aSystemGraphicsData;
        aSystemGraphicsData.nSize = sizeof(SystemGraphicsData);
        //aSystemGraphicsData.hDC = cairo_win32_surface_get_dc( mpSurface.get() );
        OSL_TRACE( "Os2Surface::createVirtualDevice");

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
        OSL_TRACE( "Os2Surface createSurface from surface");
        return SurfaceSharedPtr(new Os2Surface(rSurface));
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
                                    int x, int y, int width, int height)
    {
        SurfaceSharedPtr surf;
        // cairo hits assertion in cairo-surface.c#535 if size is 0x0
        int w = (width == 0 ? 1 : width);
        int h = (height == 0 ? 1 : height);
        OSL_TRACE( "createSurface refDev:%x, offset: %d x %d", &rRefDevice, x, y);

        if( rRefDevice.GetOutDevType() == OUTDEV_WINDOW )
        {
            OSL_TRACE( "Os2Surface createSurface for WINDOW");
            const Window &rWindow = (const Window &) rRefDevice;
            const SystemEnvData* pSysData = GetSysData(&rWindow);
            if (pSysData && pSysData->hWnd)
                surf = SurfaceSharedPtr(new Os2Surface(
                                            pSysData->hWnd, x, y, w, h));
        }
        else if( rRefDevice.GetOutDevType() == OUTDEV_VIRDEV )
        {
            OSL_TRACE( "Os2Surface createSurface for VIRDEV");
            //SystemGraphicsData aSysData = ((const VirtualDevice&) rRefDevice).GetSystemGfxData();
            //if (aSysData.hDC)
            //    surf = SurfaceSharedPtr(new Os2Surface((HDC) aSysData.hDC, x, y));
        }
        return surf;
    }


    /**
     * cairo::createBitmapSurface:   Create platform native Canvas surface from BitmapSystemData
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
        OSL_TRACE( "createBitmapSurface requested size: %d x %d available size: %d x %d",
                   rSize.Width(), rSize.Height(), rData.mnWidth, rData.mnHeight );

        if ( rData.mnWidth == rSize.Width() && rData.mnHeight == rSize.Height() )
            return SurfaceSharedPtr(new Os2Surface( rData ));
        else
            return SurfaceSharedPtr();
    }

    typedef USHORT WCHAR;
    extern "C" ULONG APIENTRY Ft2GetGlyphIndices( HPS, WCHAR *, int, USHORT *, ULONG );

    /**
     * cairo::ucs4toindex: Convert ucs4 char to glyph index
     * @param ucs4 an ucs4 char
     * @param hfont current font
     *
     * @return true if successful
     **/
    unsigned long ucs4toindex(unsigned int ucs4, const char* font)
    {
        WCHAR unicode[2];
        USHORT glyph_index;
        HPS hps = NULL;
        FATTRS fontAttrs;
        APIRET rc;

        hps = WinGetPS( HWND_DESKTOP);
        if (!hps) return 0;

        memset( &fontAttrs, 0, sizeof( fontAttrs));
        fontAttrs.usRecordLength = sizeof( FATTRS);
        fontAttrs.usCodePage = 850;
        fontAttrs.fsType = FATTR_TYPE_MBCS;
        fontAttrs.fsFontUse = FATTR_FONTUSE_NOMIX;
        // map StarSymbol to OpenSymbol
        if (strcmp( font, "StarSymbol") == 0)
            strcpy( fontAttrs.szFacename, "OpenSymbol");
        else
            strcpy( fontAttrs.szFacename, font);
        rc = Ft2CreateLogFont( hps, NULL, 1L, &fontAttrs);
        rc = Ft2SetCharSet( hps, 1L);

        unicode[0] = ucs4;
        unicode[1] = 0;
        if (Ft2GetGlyphIndices( hps, unicode, 1, &glyph_index, 0) == -1) {
            glyph_index = 0;
        }

        WinReleasePS( hps);

        return glyph_index;
    }


}  // namespace cairo

#endif   // OS2
