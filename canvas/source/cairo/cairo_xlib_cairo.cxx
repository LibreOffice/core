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

#include <utility>

#include <tools/prex.h>
#include <X11/extensions/Xrender.h>
#include <X11/Xlib.h>
#include <tools/postx.h>

#include "cairo_xlib_cairo.hxx"

#include <vcl/sysdata.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/virdev.hxx>
#include <basegfx/vector/b2isize.hxx>

namespace
{
    Pixmap limitXCreatePixmap(Display *display, Drawable d, unsigned int width, unsigned int height, unsigned int depth)
    {
        // The X protocol request CreatePixmap puts an upper bound
        // of 16 bit to the size.
        //
        // see, e.g. moz#424333, fdo#48961
        // we've a duplicate of this in vcl :-(
        if (width > SAL_MAX_INT16 || height > SAL_MAX_INT16)
        {
            SAL_WARN("canvas", "overlarge pixmap: " << width << " x " << height);
            return None;
        }
        return XCreatePixmap(display, d, width, height, depth);
    }
}

namespace cairo
{

#include <cairo-xlib.h>
#include <cairo-xlib-xrender.h>

    // TODO(F3): svp headless case!

    bool IsCairoWorking( OutputDevice* pOutDev )
    {
        if( !pOutDev )
            return false;

        Display* pDisplay = (Display*)pOutDev->GetSystemGfxData().pDisplay;
        if( !pDisplay )
            return false;

        int nDummy;
        return XQueryExtension( pDisplay, "RENDER", &nDummy, &nDummy, &nDummy );
    }

    X11SysData::X11SysData() :
        pDisplay(NULL),
        hDrawable(0),
        pVisual(NULL),
        nScreen(0),
        nDepth(-1),
        aColormap(-1),
        pRenderFormat(NULL)
    {}

    X11SysData::X11SysData( const SystemGraphicsData& pSysDat ) :
        pDisplay(pSysDat.pDisplay),
        hDrawable(pSysDat.hDrawable),
        pVisual(pSysDat.pVisual),
        nScreen(pSysDat.nScreen),
        nDepth(pSysDat.nDepth),
        aColormap(pSysDat.aColormap),
        pRenderFormat(pSysDat.pXRenderFormat)
    {}

    X11SysData::X11SysData( const SystemEnvData& pSysDat ) :
        pDisplay(pSysDat.pDisplay),
        hDrawable(pSysDat.aWindow),
        pVisual(pSysDat.pVisual),
        nScreen(pSysDat.nScreen),
        nDepth(pSysDat.nDepth),
        aColormap(pSysDat.aColormap),
        pRenderFormat(NULL)
    {}

    X11Pixmap::~X11Pixmap()
    {
        if( mpDisplay && mhDrawable )
            XFreePixmap( (Display*)mpDisplay, mhDrawable );
    }

    /**
     * Surface::Surface:   Create Canvas surface with existing data
     * @param pSysData Platform native system environment data (struct SystemEnvData in vcl/inc/sysdata.hxx)
     * @param pSurface Cairo surface
     *
     * pSysData contains the platform native Drawable reference
     * This constructor only stores data, it does no processing.
     * It is used by e.g. Surface::getSimilar()
     *
     * Set the mpSurface as pSurface
     **/
    X11Surface::X11Surface( const X11SysData&            rSysData,
                            const X11PixmapSharedPtr&    rPixmap,
                            const CairoSurfaceSharedPtr& pSurface ) :
        maSysData(rSysData),
        mpPixmap(rPixmap),
        mpSurface(pSurface)
    {}

    /**
     * Surface::Surface:     Create generic Canvas surface using given Cairo Surface
     *
     * @param pSurface Cairo Surface
     *
     * This constructor only stores data, it does no processing.
     * It is used with e.g. cairo_image_surface_create_for_data()
     * Unlike other constructors, mpSysData is set to NULL
     *
     * Set the mpSurface as pSurface
     **/
    X11Surface::X11Surface( const CairoSurfaceSharedPtr& pSurface ) :
        maSysData(),
        mpPixmap(),
        mpSurface(pSurface)
    {}

    /**
     * Surface::Surface:   Create Canvas surface from Window reference.
     * @param pSysData Platform native system environment data (struct SystemEnvData in vcl/inc/sysdata.hxx)
     * @param x horizontal location of the new surface
     * @param y vertical location of the new surface
     * @param width width of the new surface
     * @param height height of the new surface
     *
     * pSysData contains the platform native Window reference.
     *
     * pSysData is used to create a surface on the Window
     *
     * Set the mpSurface to the new surface or NULL
     **/
    X11Surface::X11Surface( const X11SysData& rSysData, int x, int y, int width, int height ) :
        maSysData(rSysData),
        mpPixmap(),
        mpSurface(
            cairo_xlib_surface_create( (Display*)rSysData.pDisplay,
                                       rSysData.hDrawable,
                                       (Visual*)rSysData.pVisual,
                                       width + x, height + y ),
            &cairo_surface_destroy)
    {
        cairo_surface_set_device_offset(mpSurface.get(), x, y );
    }

    /**
     * Surface::Surface:   Create platfrom native Canvas surface from BitmapSystemData
     * @param pSysData Platform native system environment data (struct SystemEnvData in vcl/inc/sysdata.hxx)
     * @param pBmpData Platform native image data (struct BitmapSystemData in vcl/inc/bitmap.hxx)
     * @param width width of the new surface
     * @param height height of the new surface
     *
     * The pBmpData provides the imagedata that the created surface should contain.
     *
     * Set the mpSurface to the new surface or NULL
     **/
    X11Surface::X11Surface( const X11SysData&       rSysData,
                            const BitmapSystemData& rData ) :
        maSysData( rSysData ),
        mpPixmap(),
        mpSurface(
            cairo_xlib_surface_create( (Display*)rSysData.pDisplay,
                                       (Drawable)rData.aPixmap,
                                       (Visual*) rSysData.pVisual,
                                       rData.mnWidth, rData.mnHeight ),
            &cairo_surface_destroy)
    {
    }

    /**
     * Surface::getCairo:  Create Cairo (drawing object) for the Canvas surface
     *
     * @return new Cairo or NULL
     **/
    CairoSharedPtr X11Surface::getCairo() const
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
    SurfaceSharedPtr X11Surface::getSimilar( Content aContent, int width, int height ) const
    {
        Pixmap hPixmap;

        if( maSysData.pDisplay && maSysData.hDrawable )
        {
            XRenderPictFormat* pFormat;
            int nFormat;

            switch (aContent)
            {
                case CAIRO_CONTENT_ALPHA:
                    nFormat = PictStandardA8;
                    break;
                case CAIRO_CONTENT_COLOR:
                    nFormat = PictStandardRGB24;
                    break;
                case CAIRO_CONTENT_COLOR_ALPHA:
                default:
                    nFormat = PictStandardARGB32;
                    break;
            }

            pFormat = XRenderFindStandardFormat( (Display*)maSysData.pDisplay, nFormat );
            hPixmap = limitXCreatePixmap( (Display*)maSysData.pDisplay, maSysData.hDrawable,
                                     width > 0 ? width : 1, height > 0 ? height : 1,
                                     pFormat->depth );

            X11SysData aSysData(maSysData);
            aSysData.pRenderFormat = pFormat;
            return SurfaceSharedPtr(
                new X11Surface( aSysData,
                                X11PixmapSharedPtr(
                                    new X11Pixmap(hPixmap, maSysData.pDisplay)),
                                CairoSurfaceSharedPtr(
                                    cairo_xlib_surface_create_with_xrender_format(
                                        (Display*)maSysData.pDisplay,
                                        hPixmap,
                                        ScreenOfDisplay((Display *)maSysData.pDisplay, maSysData.nScreen),
                                        pFormat, width, height ),
                                    &cairo_surface_destroy) ));
        }
        else
            return SurfaceSharedPtr(
                new X11Surface( maSysData,
                                X11PixmapSharedPtr(),
                                CairoSurfaceSharedPtr(
                                    cairo_surface_create_similar( mpSurface.get(), aContent, width, height ),
                                    &cairo_surface_destroy )));
    }

    boost::shared_ptr<VirtualDevice> X11Surface::createVirtualDevice() const
    {
        SystemGraphicsData aSystemGraphicsData;

        aSystemGraphicsData.nSize = sizeof(SystemGraphicsData);
        aSystemGraphicsData.hDrawable = getDrawable();
        aSystemGraphicsData.pXRenderFormat = getRenderFormat();

        return boost::shared_ptr<VirtualDevice>(
            new VirtualDevice( &aSystemGraphicsData, std::max( getDepth(), 0 ) ));
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
    void X11Surface::Resize( int width, int height )
    {
        cairo_xlib_surface_set_size( mpSurface.get(), width, height );
    }

    void X11Surface::flush() const
    {
        XSync( (Display*)maSysData.pDisplay, false );
    }

    /**
     * Surface::getDepth:  Get the color depth of the Canvas surface.
     *
     * @return color depth
     **/
    int X11Surface::getDepth() const
    {
        if( maSysData.pRenderFormat )
            return ((XRenderPictFormat*) maSysData.pRenderFormat)->depth;

        return -1;
    }

    SurfaceSharedPtr createSurface( const CairoSurfaceSharedPtr& rSurface )
    {
        return SurfaceSharedPtr(new X11Surface(rSurface));
    }

    static X11SysData getSysData( const Window& rWindow )
    {
        const SystemEnvData* pSysData = GetSysData(&rWindow);

        if( !pSysData )
            return X11SysData();
        else
            return X11SysData(*pSysData);
    }

    static X11SysData getSysData( const VirtualDevice& rVirDev )
    {
        return X11SysData( rVirDev.GetSystemGfxData() );
    }

    SurfaceSharedPtr createSurface( const OutputDevice& rRefDevice,
                                    int x, int y, int width, int height )
    {
        if( rRefDevice.GetOutDevType() == OUTDEV_WINDOW )
            return SurfaceSharedPtr(new X11Surface(getSysData((const Window&)rRefDevice),
                                                   x,y,width,height));
        else if( rRefDevice.GetOutDevType() == OUTDEV_VIRDEV )
            return SurfaceSharedPtr(new X11Surface(getSysData((const VirtualDevice&)rRefDevice),
                                                   x,y,width,height));
        else
            return SurfaceSharedPtr();
    }

    SurfaceSharedPtr createBitmapSurface( const OutputDevice&     rRefDevice,
                                          const BitmapSystemData& rData,
                                          const Size&             rSize )
    {
        OSL_TRACE( "requested size: %d x %d available size: %d x %d",
                   rSize.Width(), rSize.Height(), rData.mnWidth, rData.mnHeight );
        if ( rData.mnWidth == rSize.Width() && rData.mnHeight == rSize.Height() )
        {
            if( rRefDevice.GetOutDevType() == OUTDEV_WINDOW )
                return SurfaceSharedPtr(new X11Surface(getSysData((const Window&)rRefDevice), rData ));
            else if( rRefDevice.GetOutDevType() == OUTDEV_VIRDEV )
                return SurfaceSharedPtr(new X11Surface(getSysData((const VirtualDevice&)rRefDevice), rData ));
        }

        return SurfaceSharedPtr();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
