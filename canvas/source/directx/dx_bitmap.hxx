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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
