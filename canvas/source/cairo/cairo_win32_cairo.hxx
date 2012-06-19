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

#ifndef _CAIROCANVAS_WIN32_CAIRO_HXX
#define _CAIROCANVAS_WIN32_CAIRO_HXX

#include "cairo_cairo.hxx"
#include <prewin.h>
#include <postwin.h>

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
