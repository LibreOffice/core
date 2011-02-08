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

#ifndef _CAIROCANVAS_WIN32_CAIRO_HXX
#define _CAIROCANVAS_WIN32_CAIRO_HXX

#ifdef WNT
# include <prewin.h>
# include <windows.h>
# include <postwin.h>
#endif

#include "cairo_cairo.hxx"

namespace cairo {

    class Win32Surface : public Surface
    {
        CairoSurfaceSharedPtr mpSurface;

    public:
        /// takes over ownership of passed cairo_surface
        explicit Win32Surface( const CairoSurfaceSharedPtr& pSurface );
        /// create surface on subarea of given drawable
        Win32Surface( HDC hDC, int x, int y );
        /// create surface for given bitmap data
        Win32Surface( const BitmapSystemData& rBmpData );

        // Surface interface
        virtual CairoSharedPtr getCairo() const;
        virtual CairoSurfaceSharedPtr getCairoSurface() const { return mpSurface; }
        virtual SurfaceSharedPtr getSimilar( Content aContent, int width, int height ) const;

        virtual boost::shared_ptr<VirtualDevice> createVirtualDevice() const;

        virtual void Resize( int width, int height );

        virtual void flush() const;

        int getDepth() const;
    };

    unsigned long ucs4toindex(unsigned int ucs4, HFONT hfont);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
