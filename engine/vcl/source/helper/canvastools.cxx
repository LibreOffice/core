/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/geometry/RealSize2D.hpp>
#include <com/sun/star/geometry/IntegerSize2D.hpp>
#include <com/sun/star/geometry/IntegerPoint2D.hpp>
#include <com/sun/star/geometry/IntegerRectangle2D.hpp>

#include <com/sun/star/rendering/VolatileContentDestroyedException.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>
#include <com/sun/star/rendering/ColorComponentTag.hpp>
#include <com/sun/star/rendering/ARGBColor.hpp>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/range/b2irectangle.hxx>

#include <sal/log.hxx>
#include <tools/helpers.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <vcl/bitmap.hxx>

#include <canvasbitmap.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/BitmapWriteAccess.hxx>

using namespace ::com::sun::star;

namespace vcl::unotools
{
        uno::Reference< rendering::XBitmap > xBitmapFromBitmap(const ::Bitmap& inputBitmap )
        {
            SAL_INFO( "vcl.helper", "vcl::unotools::xBitmapFromBitmapEx()" );

            return new vcl::unotools::VclCanvasBitmap( inputBitmap );
        }

        ::Bitmap bitmapFromXBitmap( const uno::Reference< rendering::XBitmap >& xInputBitmap )
        {
            SAL_INFO( "vcl.helper", "vcl::unotools::bitmapExFromXBitmap()" );

            if( !xInputBitmap.is() )
                return ::Bitmap();

            // tunnel directly for known implementation
            VclCanvasBitmap* pImplBitmap = dynamic_cast<VclCanvasBitmap*>(xInputBitmap.get());
            if( pImplBitmap )
                return pImplBitmap->getBitmap();

            // The only other possible implementation of XBitmap is
            // vclcanvas::CanvasBitmap in canvas/source/vcl/canvasbitmap.cxx, which has a XFastPropertySet fast-path
            uno::Reference<css::beans::XFastPropertySet> xFastProp(xInputBitmap, uno::UNO_QUERY_THROW);
            cpo::uno::Any aAny = xFastProp->getFastPropertyValue(0);
            sal_Int64 nBitmapPtr(0);
            aAny >>= nBitmapPtr;
            std::unique_ptr<Bitmap> pBitmap(reinterpret_cast<Bitmap*>(nBitmapPtr));
            return *pBitmap;
        }

        geometry::RealSize2D size2DFromSize( const Size& rSize )
        {
            return geometry::RealSize2D( rSize.Width(),
                                         rSize.Height() );
        }

        Size sizeFromRealSize2D( const geometry::RealSize2D& rSize )
        {
            return Size( static_cast<tools::Long>(rSize.Width + .5),
                         static_cast<tools::Long>(rSize.Height + .5) );
        }

        ::Size sizeFromB2DSize( const basegfx::B2DVector& rVec )
        {
            return ::Size(basegfx::fround<tools::Long>(rVec.getX()),
                          basegfx::fround<tools::Long>(rVec.getY()));
        }

        ::Point pointFromB2DPoint( const basegfx::B2DPoint& rPoint )
        {
            return pointFromB2IPoint(basegfx::fround(rPoint));
        }

        ::tools::Rectangle rectangleFromB2DRectangle( const basegfx::B2DRange& rRect )
        {
            return rectangleFromB2IRectangle(basegfx::fround(rRect));
        }

        Point pointFromB2IPoint( const basegfx::B2IPoint& rPoint )
        {
            return ::Point( rPoint.getX(),
                            rPoint.getY() );
        }

        basegfx::B2IPoint b2IPointFromPoint(Point const& rPoint)
        {
            return basegfx::B2IPoint(rPoint.X(), rPoint.Y());
        }

        tools::Rectangle rectangleFromB2IRectangle( const basegfx::B2IRange& rRect )
        {
            return ::tools::Rectangle( rRect.getMinX(),
                                rRect.getMinY(),
                                rRect.getMaxX(),
                                rRect.getMaxY() );
        }

        basegfx::B2IRectangle b2IRectangleFromRectangle(tools::Rectangle const& rRect)
        {
            // although B2IRange internally has separate height/width emptiness, it doesn't
            // expose any API to let us set them separately, so just do the best we can.
            if (rRect.IsWidthEmpty() && rRect.IsHeightEmpty())
                return basegfx::B2IRange( basegfx::B2ITuple( rRect.Left(), rRect.Top() ) );
            return basegfx::B2IRange( rRect.Left(),
                                  rRect.Top(),
                                  rRect.IsWidthEmpty() ? rRect.Left() : rRect.Right(),
                                  rRect.IsHeightEmpty() ? rRect.Top() : rRect.Bottom() );
        }

        basegfx::B2DSize b2DSizeFromSize(const Size& rSize)
        {
            return basegfx::B2DSize(rSize.Width(), rSize.Height());
        }

        basegfx::B2DVector b2DVectorFromSize(const Size& rSize)
        {
            return basegfx::B2DVector(rSize.Width(), rSize.Height());
        }

        basegfx::B2DPoint b2DPointFromPoint( const ::Point& rPoint )
        {
            return basegfx::B2DPoint( rPoint.X(),
                                        rPoint.Y() );
        }

        basegfx::B2DRange b2DRectangleFromRectangle( const ::tools::Rectangle& rRect )
        {
            // although B2DRange internally has separate height/width emptiness, it doesn't
            // expose any API to let us set them separately, so just do the best we can.
            if (rRect.IsWidthEmpty() && rRect.IsHeightEmpty())
                return basegfx::B2DRange( basegfx::B2DTuple( rRect.Left(), rRect.Top() ) );
            return basegfx::B2DRectangle( rRect.Left(),
                                  rRect.Top(),
                                  rRect.IsWidthEmpty() ? rRect.Left() : rRect.Right(),
                                  rRect.IsHeightEmpty() ? rRect.Top() : rRect.Bottom() );
        }

        geometry::IntegerSize2D integerSize2DFromSize( const Size& rSize )
        {
            return geometry::IntegerSize2D( rSize.Width(),
                                            rSize.Height() );
        }

        Size sizeFromIntegerSize2D( const geometry::IntegerSize2D& rSize )
        {
            return Size( rSize.Width,
                         rSize.Height );
        }

        Point pointFromIntegerPoint2D( const geometry::IntegerPoint2D& rPoint )
        {
            return Point( rPoint.X,
                          rPoint.Y );
        }

        tools::Rectangle rectangleFromIntegerRectangle2D( const geometry::IntegerRectangle2D& rRectangle )
        {
            return tools::Rectangle( rRectangle.X1, rRectangle.Y1,
                              rRectangle.X2, rRectangle.Y2 );
        }

} // namespace canvas

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
