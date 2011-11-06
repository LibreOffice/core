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



#ifndef _CAIROCANVAS_SPRITEDEVICEHELPER_HXX
#define _CAIROCANVAS_SPRITEDEVICEHELPER_HXX

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XBufferController.hpp>

#include <boost/utility.hpp>

#include <vcl/window.hxx>
#include <vcl/bitmap.hxx>

#include "cairo_cairo.hxx"
#include "cairo_devicehelper.hxx"

/* Definition of DeviceHelper class */

namespace cairocanvas
{
    class SpriteCanvas;
    class SpriteCanvasHelper;

    class SpriteDeviceHelper : public DeviceHelper
    {
    public:
        SpriteDeviceHelper();

        void init( Window&                   rOutputWindow,
                   SpriteCanvas&             rSpriteCanvas,
                   const ::basegfx::B2ISize& rSize,
                   bool                      bFullscreen );

        /// Dispose all internal references
        void disposing();

        // XWindowGraphicDevice
        ::sal_Int32 createBuffers( ::sal_Int32 nBuffers );
        void        destroyBuffers(  );
        ::sal_Bool  showBuffer( bool, ::sal_Bool );
        ::sal_Bool  switchBuffer( bool, ::sal_Bool bUpdateAll );

        ::com::sun::star::uno::Any isAccelerated() const;
        ::com::sun::star::uno::Any getDeviceHandle() const;
        ::com::sun::star::uno::Any getSurfaceHandle() const;

        void notifySizeUpdate( const ::com::sun::star::awt::Rectangle& rBounds );
        void setSize( const ::basegfx::B2ISize& rSize );

        ::cairo::SurfaceSharedPtr getSurface();
        ::cairo::SurfaceSharedPtr getBufferSurface();
        ::cairo::SurfaceSharedPtr getWindowSurface();
        ::cairo::SurfaceSharedPtr createSurface( const ::basegfx::B2ISize& rSize, ::cairo::Content aContent = CAIRO_CONTENT_COLOR_ALPHA );
        ::cairo::SurfaceSharedPtr createSurface( BitmapSystemData& rData, const Size& rSize );
        const ::basegfx::B2ISize& getSizePixel();
        void flush();

    private:
        /// Pointer to sprite canvas (owner of this helper), needed to create bitmaps
        SpriteCanvas*             mpSpriteCanvas;

        ::cairo::SurfaceSharedPtr mpBufferSurface;

        ::basegfx::B2ISize        maSize;
        bool                      mbFullScreen;
    };
}

#endif
