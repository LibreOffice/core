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



#ifndef _DXCANVAS_DXBITMAP_HXX
#define _DXCANVAS_DXBITMAP_HXX

#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <boost/shared_ptr.hpp>
#include <basegfx/vector/b2ivector.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drange.hxx>
#include "dx_winstuff.hxx"
#include "dx_ibitmap.hxx"
#include "dx_graphicsprovider.hxx"
#include "dx_gdiplususer.hxx"

namespace dxcanvas
{
    class DXBitmap : public IBitmap
    {
    public:
        DXBitmap( const BitmapSharedPtr& rBitmap,
                  bool                   bWithAlpha );
        DXBitmap( const ::basegfx::B2IVector& rSize,
                  bool                        bWithAlpha );

        virtual GraphicsSharedPtr         getGraphics();

        virtual BitmapSharedPtr           getBitmap() const;
        virtual ::basegfx::B2IVector      getSize() const;
        virtual bool                      hasAlpha() const;

        ::com::sun::star::uno::Sequence< sal_Int8 > getData(
            ::com::sun::star::rendering::IntegerBitmapLayout&       bitmapLayout,
            const ::com::sun::star::geometry::IntegerRectangle2D&   rect );

        void setData(
            const ::com::sun::star::uno::Sequence< sal_Int8 >&      data,
            const ::com::sun::star::rendering::IntegerBitmapLayout& bitmapLayout,
            const ::com::sun::star::geometry::IntegerRectangle2D&   rect );

        void setPixel(
            const ::com::sun::star::uno::Sequence< sal_Int8 >&      color,
            const ::com::sun::star::rendering::IntegerBitmapLayout& bitmapLayout,
            const ::com::sun::star::geometry::IntegerPoint2D&       pos );

        ::com::sun::star::uno::Sequence< sal_Int8 > getPixel(
            ::com::sun::star::rendering::IntegerBitmapLayout&       bitmapLayout,
            const ::com::sun::star::geometry::IntegerPoint2D&       pos );

    private:
        // Refcounted global GDI+ state container
        GDIPlusUserSharedPtr mpGdiPlusUser;

        // size of this image in pixels [integral unit]
        ::basegfx::B2IVector maSize;

        BitmapSharedPtr      mpBitmap;
        GraphicsSharedPtr    mpGraphics;

        // true if the bitmap contains an alpha channel
        bool                 mbAlpha;
    };

    typedef ::boost::shared_ptr< DXBitmap > DXBitmapSharedPtr;
}

#endif
