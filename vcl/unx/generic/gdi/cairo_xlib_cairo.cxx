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

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

#include "cairo_xlib_cairo.hxx"

#include <utility>
#include <vcl/sysdata.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/virdev.hxx>
#include <sal/log.hxx>

#include <cairo-xlib.h>
#include <cairo-xlib-xrender.h>

namespace
{
    Pixmap limitXCreatePixmap(Display *display, Drawable d, unsigned int width, unsigned int height, unsigned int depth)
    {
        // The X protocol request CreatePixmap puts an upper bound
        // of 16 bit to the size. And in practice some drivers
        // fall over with values close to the max.

        // see, e.g. moz#424333, fdo#48961, rhbz#1086714
        // we've a duplicate of this in vcl :-(
        if (width > SAL_MAX_INT16-10 || height > SAL_MAX_INT16-10)
        {
            SAL_WARN("canvas", "overlarge pixmap: " << width << " x " << height);
            return None;
        }
        return XCreatePixmap(display, d, width, height, depth);
    }
}

namespace cairo
{

    X11SysData::X11SysData() :
        pDisplay(nullptr),
        hDrawable(0),
        pVisual(nullptr),
        nScreen(0)
    {}

    X11SysData::X11SysData( const SystemGraphicsData& pSysDat ) :
        pDisplay(static_cast<_XDisplay*>(pSysDat.pDisplay)),
        hDrawable(pSysDat.hDrawable),
        pVisual(static_cast<Visual*>(pSysDat.pVisual)),
        nScreen(pSysDat.nScreen)
    {}

    X11SysData::X11SysData( const SystemEnvData& pSysDat, const SalFrame* pReference ) :
        pDisplay(static_cast<_XDisplay*>(pSysDat.pDisplay)),
        hDrawable(pSysDat.GetWindowHandle(pReference)),
        pVisual(static_cast<Visual*>(pSysDat.pVisual)),
        nScreen(pSysDat.nScreen)
    {}

    X11Pixmap::~X11Pixmap()
    {
        if( mpDisplay && mhDrawable )
            XFreePixmap( mpDisplay, mhDrawable );
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
                            X11PixmapSharedPtr     rPixmap,
                            CairoSurfaceSharedPtr  pSurface ) :
        maSysData(rSysData),
        mpPixmap(std::move(rPixmap)),
        mpSurface(std::move(pSurface))
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
    X11Surface::X11Surface( CairoSurfaceSharedPtr  pSurface ) :
        maSysData(),
        mpSurface(std::move(pSurface))
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
        mpSurface(
            cairo_xlib_surface_create( rSysData.pDisplay,
                                       rSysData.hDrawable,
                                       rSysData.pVisual,
                                       width + x, height + y ),
            &cairo_surface_destroy)
    {
        cairo_surface_set_device_offset(mpSurface.get(), x, y );
    }

    /**
     * Surface::Surface:   Create platform native Canvas surface from BitmapSystemData
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
        mpSurface(
            cairo_xlib_surface_create( rSysData.pDisplay,
                                       reinterpret_cast<Drawable>(rData.aPixmap),
                                       rSysData.pVisual,
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
     * @param cairo_content_type format of the new surface (cairo_content_t from cairo/src/cairo.h)
     * @param width width of the new surface
     * @param height height of the new surface
     *
     * Creates a new Canvas surface. This normally creates platform native surface, even though
     * generic function is used.
     *
     * Cairo surface from cairo_content_type (cairo_content_t)
     *
     * @return new surface or NULL
     **/
    SurfaceSharedPtr X11Surface::getSimilar(int cairo_content_type, int width, int height ) const
    {
        if( maSysData.pDisplay && maSysData.hDrawable )
        {
            XRenderPictFormat* pFormat;
            int nFormat;

            switch (cairo_content_type)
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

            pFormat = XRenderFindStandardFormat( maSysData.pDisplay, nFormat );
            Pixmap hPixmap = limitXCreatePixmap( maSysData.pDisplay, maSysData.hDrawable,
                                     width > 0 ? width : 1, height > 0 ? height : 1,
                                     pFormat->depth );

            return SurfaceSharedPtr(
                new X11Surface( maSysData,
                                std::make_shared<X11Pixmap>(hPixmap, maSysData.pDisplay),
                                CairoSurfaceSharedPtr(
                                    cairo_xlib_surface_create_with_xrender_format(
                                        maSysData.pDisplay,
                                        hPixmap,
                                        ScreenOfDisplay(maSysData.pDisplay, maSysData.nScreen),
                                        pFormat, width, height ),
                                    &cairo_surface_destroy) ));
        }
        else
            return SurfaceSharedPtr(
                new X11Surface( maSysData,
                                X11PixmapSharedPtr(),
                                CairoSurfaceSharedPtr(
                                    cairo_surface_create_similar( mpSurface.get(),
                                        static_cast<cairo_content_t>(cairo_content_type), width, height ),
                                    &cairo_surface_destroy )));
    }

    VclPtr<VirtualDevice> X11Surface::createVirtualDevice() const
    {
        SystemGraphicsData aSystemGraphicsData;

        cairo_surface_t* pSurface = mpSurface.get();

        aSystemGraphicsData.nSize = sizeof(SystemGraphicsData);
        aSystemGraphicsData.hDrawable = mpPixmap ? mpPixmap->mhDrawable : maSysData.hDrawable;
        aSystemGraphicsData.pSurface = pSurface;

        int width = cairo_xlib_surface_get_width(pSurface);
        int height = cairo_xlib_surface_get_height(pSurface);

        return VclPtr<VirtualDevice>::Create(aSystemGraphicsData,
                              Size(width, height),
                              DeviceFormat::WITHOUT_ALPHA);
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
    bool X11Surface::Resize(int width, int height)
    {
        cairo_xlib_surface_set_size(mpSurface.get(), width, height);
        return true;
    }

    void X11Surface::flush() const
    {
        XSync( maSysData.pDisplay, false );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
