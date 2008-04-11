/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: canvas.hxx,v $
 * $Revision: 1.10 $
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

#ifndef _CPPCANVAS_CANVAS_HXX
#define _CPPCANVAS_CANVAS_HXX

#include <com/sun/star/uno/Reference.hxx>

#include <boost/shared_ptr.hpp>
#include <cppcanvas/font.hxx>
#include <cppcanvas/color.hxx>

namespace rtl
{
    class OUString;
}

namespace basegfx
{
    class B2DHomMatrix;
    class B2DPolyPolygon;
}

namespace com { namespace sun { namespace star { namespace rendering
{
    class  XCanvas;
    struct ViewState;
} } } }


/* Definition of BitmapCanvas */

namespace cppcanvas
{
    class PolyPolygon;
    class Canvas;

    // forward declaration, since PolyPolygon also references Canvas
    typedef ::boost::shared_ptr< PolyPolygon > PolyPolygonSharedPtr;

    // forward declaration, since cloneCanvas() also references Canvas
    typedef ::boost::shared_ptr< Canvas > CanvasSharedPtr;

    /** Canvas interface
     */
    class Canvas
    {
    public:
        enum
        {
            /** Extra pixel used when canvas anti-aliases.

                Enlarge the bounding box of drawing primitives by this
                amount in both dimensions, and on both sides of the
                bounds, to account for extra pixel touched outside the
                actual primitive bounding box, when the canvas
                performs anti-aliasing.
             */
            ANTIALIASING_EXTRA_SIZE=2
        };

        virtual ~Canvas() {}

        virtual void                             setTransformation( const ::basegfx::B2DHomMatrix& rMatrix ) = 0;
        virtual ::basegfx::B2DHomMatrix          getTransformation() const = 0;

        virtual void                             setClip( const ::basegfx::B2DPolyPolygon& rClipPoly ) = 0;
        virtual void                             setClip() = 0;

        /** Get current clip

            @return NULL, if no clip is set, otherwise the current clip poly-polygon
         */
        virtual ::basegfx::B2DPolyPolygon const* getClip() const = 0;

        virtual FontSharedPtr                    createFont( const ::rtl::OUString& rFontName, const double& rCellSize ) const = 0;

        virtual ColorSharedPtr                   createColor() const = 0;

        virtual CanvasSharedPtr                  clone() const = 0;
        virtual void                             clear() const = 0;

        // this should be considered private. if RTTI gets enabled
        // someday, remove that to a separate interface
        virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::rendering::XCanvas >          getUNOCanvas() const = 0;
        virtual ::com::sun::star::rendering::ViewState      getViewState() const = 0;
    };

}

#endif /* _CPPCANVAS_CANVAS_HXX */
