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



#ifndef _CPPCANVAS_IMPLBITMAP_HXX
#define _CPPCANVAS_IMPLBITMAP_HXX

#include <com/sun/star/uno/Reference.hxx>
#ifndef _COM_SUN_STAR_RENDERING_XBITMAP_HPP__
#include <com/sun/star/rendering/XBitmap.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XGRAPHICDEVICE_HPP__
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#endif

#include <cppcanvas/bitmap.hxx>
#include <canvasgraphichelper.hxx>


/*Definition of ImplBitmap */

namespace cppcanvas
{

    namespace internal
    {
        // share partial CanvasGraphic implementation from CanvasGraphicHelper
        class ImplBitmap : public virtual ::cppcanvas::Bitmap, protected CanvasGraphicHelper
        {
        public:

            ImplBitmap( const CanvasSharedPtr& rParentCanvas,
                        const ::com::sun::star::uno::Reference<
                            ::com::sun::star::rendering::XBitmap >& rBitmap );

            virtual ~ImplBitmap();

            // CanvasGraphic implementation (that was not already implemented by CanvasGraphicHelper)
            virtual bool draw() const;
            virtual bool drawAlphaModulated( double nAlphaModulation ) const;

            virtual BitmapCanvasSharedPtr                       getBitmapCanvas() const;

            // Bitmap implementation
            virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::rendering::XBitmap >  getUNOBitmap() const;

        private:
            // default: disabled copy/assignment
            ImplBitmap(const ImplBitmap&);
            ImplBitmap& operator=( const ImplBitmap& );

            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap >  mxBitmap;
            BitmapCanvasSharedPtr                                                                   mpBitmapCanvas;
         };
    }
}

#endif /* _CPPCANVAS_IMPLBITMAP_HXX */
