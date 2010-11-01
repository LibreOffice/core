/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _CAIROCANVAS_XLIB_CAIRO_HXX
#define _CAIROCANVAS_XLIB_CAIRO_HXX

#include "cairo_cairo.hxx"

struct SystemEnvData;
struct SystemGraphicsData;

namespace cairo {

    /// Holds all X11-output relevant data
    struct X11SysData
    {
        X11SysData();
        explicit X11SysData( const SystemGraphicsData& );
        explicit X11SysData( const SystemEnvData& );

        void*   pDisplay;       // the relevant display connection
        long    hDrawable;      // a drawable
        void*   pVisual;        // the visual in use
        int nScreen;        // the current screen of the drawable
        int     nDepth;         // depth of said visual
        long    aColormap;      // the colormap being used
        void*   pRenderFormat;  // render format for drawable
    };

    /// RAII wrapper for a pixmap
    struct X11Pixmap
    {
        void* mpDisplay;  // the relevant display connection
        long  mhDrawable; // a drawable

        X11Pixmap( long hDrawable, void* pDisplay ) :
            mpDisplay(pDisplay),
            mhDrawable(hDrawable)
        {}

        ~X11Pixmap();

        void clear() { mpDisplay=NULL; mhDrawable=0; }
    };

    typedef boost::shared_ptr<X11Pixmap>       X11PixmapSharedPtr;

    class X11Surface : public Surface
    {
        const X11SysData      maSysData;
        X11PixmapSharedPtr    mpPixmap;
        CairoSurfaceSharedPtr mpSurface;

        X11Surface( const X11SysData& rSysData, const X11PixmapSharedPtr& rPixmap, const CairoSurfaceSharedPtr& pSurface );

    public:
        /// takes over ownership of passed cairo_surface
        explicit X11Surface( const CairoSurfaceSharedPtr& pSurface );
        /// create surface on subarea of given drawable
        X11Surface( const X11SysData& rSysData, int x, int y, int width, int height );
        /// create surface for given bitmap data
        X11Surface( const X11SysData& rSysData, const BitmapSystemData& rBmpData );

        // Surface interface
        virtual CairoSharedPtr getCairo() const;
        virtual CairoSurfaceSharedPtr getCairoSurface() const { return mpSurface; }
        virtual SurfaceSharedPtr getSimilar( Content aContent, int width, int height ) const;

        virtual boost::shared_ptr<VirtualDevice> createVirtualDevice() const;

        virtual void Resize( int width, int height );

        virtual void flush() const;

        int getDepth() const;
        X11PixmapSharedPtr getPixmap() const { return mpPixmap; }
        void* getRenderFormat() const { return maSysData.pRenderFormat; }
        long getDrawable() const { return mpPixmap ? mpPixmap->mhDrawable : maSysData.hDrawable; }
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
