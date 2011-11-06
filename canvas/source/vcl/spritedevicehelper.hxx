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



#ifndef _VCLCANVAS_SPRITEDEVICEHELPER_HXX
#define _VCLCANVAS_SPRITEDEVICEHELPER_HXX

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XBufferController.hpp>

#include <vcl/outdev.hxx>
#include <vcl/window.hxx>

#include "backbuffer.hxx"
#include "devicehelper.hxx"

#include <boost/utility.hpp>


/* Definition of DeviceHelper class */

namespace vclcanvas
{
    class SpriteCanvas;
    class SpriteCanvasHelper;

    class SpriteDeviceHelper : public DeviceHelper
    {
    public:
        SpriteDeviceHelper();

        void init( const OutDevProviderSharedPtr& rOutDev );

        /// Dispose all internal references
        void disposing();

        ::sal_Int32 createBuffers( ::sal_Int32 nBuffers );
        void        destroyBuffers(  );
        ::sal_Bool  showBuffer( bool bWindowVisible, ::sal_Bool bUpdateAll );
        ::sal_Bool  switchBuffer( bool bWindowVisible, ::sal_Bool bUpdateAll );

        ::com::sun::star::uno::Any isAccelerated() const;
        ::com::sun::star::uno::Any getDeviceHandle() const;
        ::com::sun::star::uno::Any getSurfaceHandle() const;

        void dumpScreenContent() const;
        BackBufferSharedPtr getBackBuffer() const { return mpBackBuffer; }

        void notifySizeUpdate( const ::com::sun::star::awt::Rectangle& rBounds );

    private:
        /// This buffer holds the background content for all associated canvases
        BackBufferSharedPtr     mpBackBuffer;
    };
}

#endif /* _VCLCANVAS_SPRITEDEVICEHELPER_HXX */
