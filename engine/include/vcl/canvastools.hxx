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

#ifndef INCLUDED_VCL_CANVASTOOLS_HXX
#define INCLUDED_VCL_CANVASTOOLS_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <cpo/uno/Sequence.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/range/b2irectangle.hxx>

#include <vcl/dllapi.h>

class Point;
class Size;
namespace tools { class Rectangle; }
class Bitmap;
class Color;

namespace basegfx
{
    class B2DVector;
    class B2DPoint;
    class B2DRange;
    class B2IPoint;
    class B2IRange;
    class B2DSize;
}

namespace com::sun::star::geometry
{
    struct RealSize2D;
    struct IntegerPoint2D;
    struct IntegerSize2D;
    struct IntegerRectangle2D;
}

namespace com::sun::star::rendering
{
    class  XBitmap;
}

namespace vcl::unotools
{
        // Bitmap conversions


        /** Create an XBitmap from VCL Bitmap(
         */
        css::uno::Reference< css::rendering::XBitmap >
            VCL_DLLPUBLIC xBitmapFromBitmap( const ::Bitmap& inputBitmap );

        /** Create a Bitmap from an XBitmap
         */
        ::Bitmap VCL_DLLPUBLIC bitmapFromXBitmap( const css::uno::Reference< css::rendering::XBitmap >& xInputBitmap );

        // Geometry conversions (vcl/tools <-> x)


        // geometry::Real
        css::geometry::RealSize2D          VCL_DLLPUBLIC size2DFromSize( const Size& );

        Size                               VCL_DLLPUBLIC sizeFromRealSize2D( const css::geometry::RealSize2D& );

        // geometry::Integer
        css::geometry::IntegerSize2D       VCL_DLLPUBLIC integerSize2DFromSize( const Size& );

        Size                               VCL_DLLPUBLIC sizeFromIntegerSize2D( const css::geometry::IntegerSize2D& );
        Point                              VCL_DLLPUBLIC pointFromIntegerPoint2D( const css::geometry::IntegerPoint2D& );
        tools::Rectangle                          VCL_DLLPUBLIC rectangleFromIntegerRectangle2D( const css::geometry::IntegerRectangle2D& );

        // basegfx::B2D
        Size                        VCL_DLLPUBLIC sizeFromB2DSize( const basegfx::B2DVector& );
        Point                       VCL_DLLPUBLIC pointFromB2DPoint( const basegfx::B2DPoint& );
        tools::Rectangle                   VCL_DLLPUBLIC rectangleFromB2DRectangle( const basegfx::B2DRange& );

        VCL_DLLPUBLIC basegfx::B2DSize b2DSizeFromSize(const Size& rSize);
        VCL_DLLPUBLIC basegfx::B2DVector b2DVectorFromSize(const Size& rSize);
        basegfx::B2DPoint           VCL_DLLPUBLIC b2DPointFromPoint( const Point& );
        basegfx::B2DRange           VCL_DLLPUBLIC b2DRectangleFromRectangle( const tools::Rectangle& );

        // basegfx::B2I
        Point                       VCL_DLLPUBLIC pointFromB2IPoint( const basegfx::B2IPoint& );
        tools::Rectangle                   VCL_DLLPUBLIC rectangleFromB2IRectangle( const basegfx::B2IRange& );
        basegfx::B2IPoint           VCL_DLLPUBLIC b2IPointFromPoint(const Point&);
        basegfx::B2IRectangle       VCL_DLLPUBLIC b2IRectangleFromRectangle(const tools::Rectangle&);

}

#endif // INCLUDED_VCL_CANVASTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
