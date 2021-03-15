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

#ifndef INCLUDED_CANVAS_SOURCE_CAIRO_CAIRO_XLIB_CAIRO_HXX
#define INCLUDED_CANVAS_SOURCE_CAIRO_CAIRO_XLIB_CAIRO_HXX

#include <sal/config.h>
#include <vcl/cairo.hxx>
#include <vcl/salgtype.hxx>

struct BitmapSystemData;
class SalFrame;
struct SystemEnvData;
struct SystemGraphicsData;

namespace cairo {

    /// Holds all X11-output relevant data
    struct X11SysData
    {
        X11SysData();
        explicit X11SysData( const SystemGraphicsData& );
        explicit X11SysData( const SystemEnvData&, const SalFrame* pReference );

        void*   pDisplay;       // the relevant display connection
        Drawable hDrawable;     // a drawable
        void*   pVisual;        // the visual in use
        int nScreen;        // the current screen of the drawable
        void*   pRenderFormat;  // render format for drawable
    };

    /// RAII wrapper for a pixmap
    struct X11Pixmap
    {
        void* mpDisplay;  // the relevant display connection
        Pixmap  mhDrawable; // a drawable

        X11Pixmap( Pixmap hDrawable, void* pDisplay ) :
            mpDisplay(pDisplay),
            mhDrawable(hDrawable)
        {}

        ~X11Pixmap();
    };

    typedef std::shared_ptr<X11Pixmap>       X11PixmapSharedPtr;

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
        virtual CairoSharedPtr getCairo() const override;
        virtual CairoSurfaceSharedPtr getCairoSurface() const override { return mpSurface; }
        virtual SurfaceSharedPtr getSimilar(int cairo_content_type, int width, int height) const override;

        virtual VclPtr<VirtualDevice> createVirtualDevice() const override;

        virtual bool Resize( int width, int height ) override;

        virtual void flush() const override;

        int getDepth() const;
        DeviceFormat getFormat() const;
        const X11PixmapSharedPtr& getPixmap() const { return mpPixmap; }
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
