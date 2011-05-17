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

#ifndef _CAIROCANVAS_SURFACEPROVIDER_HXX
#define _CAIROCANVAS_SURFACEPROVIDER_HXX

#include <rtl/ref.hxx>
#include <com/sun/star/uno/XInterface.hpp>

#include "cairo_cairo.hxx"

using namespace ::cairo;

class OutputDevice;
class Bitmap;

namespace cairocanvas
{
    class Bitmap;

    /* Definition of RepaintTarget interface */

    /** Target interface for XCachedPrimitive implementations

        This interface must be implemented on all canvas
        implementations that hand out XCachedPrimitives
     */
    class SurfaceProvider : public ::com::sun::star::uno::XInterface
    {
    public:
        virtual ~SurfaceProvider() {}

        /** Query surface from this provider

            This should return the default surface to render on.
         */
        virtual SurfaceSharedPtr getSurface() = 0;

        /// create new surface in given size
        virtual SurfaceSharedPtr createSurface( const ::basegfx::B2ISize& rSize,
                                                Content aContent = CAIRO_CONTENT_COLOR_ALPHA ) = 0;
        /// create new surface from given bitmap
        virtual SurfaceSharedPtr createSurface( ::Bitmap& rBitmap ) = 0;

        /** convert surface between alpha and non-alpha
            channel. returns new surface on success, NULL otherwise
        */
        virtual SurfaceSharedPtr changeSurface( bool bHasAlpha, bool bCopyContent ) = 0;

        /** Provides the underlying vcl outputdevice this surface renders on
         */
        virtual OutputDevice* getOutputDevice() = 0;
    };

    typedef ::rtl::Reference< SurfaceProvider > SurfaceProviderRef;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
