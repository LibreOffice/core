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

#pragma once

#include <rtl/ref.hxx>
#include <com/sun/star/uno/XInterface.hpp>

#include <basegfx/vector/b2isize.hxx>
#include <vcl/cairo.hxx>

class OutputDevice;
class Bitmap;

namespace cairocanvas
{
    /* Definition of RepaintTarget interface */

    /** Target interface for XCachedPrimitive implementations

        This interface must be implemented on all canvas
        implementations that hand out XCachedPrimitives
     */
    class SurfaceProvider : public css::uno::XInterface
    {
    public:
        virtual ~SurfaceProvider() {}

        /** Query surface from this provider

            This should return the default surface to render on.
         */
        virtual ::cairo::SurfaceSharedPtr getSurface() = 0;

        /// create new surface in given size
        virtual ::cairo::SurfaceSharedPtr createSurface( const ::basegfx::B2ISize& rSize,
                                                         int aContent ) = 0;
        /// create new surface from given bitmap
        virtual ::cairo::SurfaceSharedPtr createSurface( ::Bitmap& rBitmap ) = 0;

        /** convert surface from alpha to non-alpha, does not copy content
            channel. returns new surface on success, NULL otherwise
        */
        virtual ::cairo::SurfaceSharedPtr changeSurface() = 0;

        /** Provides the underlying vcl outputdevice this surface renders on
         */
        virtual OutputDevice* getOutputDevice() = 0;
    };

    typedef ::rtl::Reference< SurfaceProvider > SurfaceProviderRef;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
