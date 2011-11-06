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



#ifndef _CPPCANVAS_IMPLBITMAPCANVAS_HXX
#define _CPPCANVAS_IMPLBITMAPCANVAS_HXX

#ifndef _COM_SUN_STAR_RENDERING_XBITMAPCANVAS_HPP__
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XBITMAP_HPP__
#include <com/sun/star/rendering/XBitmap.hpp>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif
#include <basegfx/vector/b2dsize.hxx>
#include <cppcanvas/bitmapcanvas.hxx>

#include <implcanvas.hxx>


namespace cppcanvas
{
    namespace internal
    {
        // share Canvas implementation from ImplCanvas
        class ImplBitmapCanvas : public virtual BitmapCanvas, protected virtual ImplCanvas
        {
        public:
            ImplBitmapCanvas( const ::com::sun::star::uno::Reference<
                                  ::com::sun::star::rendering::XBitmapCanvas >& rCanvas );
            virtual ~ImplBitmapCanvas();

            virtual ::basegfx::B2ISize      getSize() const;

            virtual CanvasSharedPtr         clone() const;

            // take compiler-provided default copy constructor
            //ImplBitmapCanvas(const ImplBitmapCanvas&);

        private:
            // default: disabled assignment
            ImplBitmapCanvas& operator=( const ImplBitmapCanvas& );

            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapCanvas >    mxBitmapCanvas;
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap >          mxBitmap;
        };
    }
}

#endif /* _CPPCANVAS_IMPLBITMAPCANVAS_HXX */
