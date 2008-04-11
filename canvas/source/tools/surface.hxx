/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: surface.hxx,v $
 * $Revision: 1.3 $
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

#ifndef INCLUDED_CANVAS_SURFACE_HXX
#define INCLUDED_CANVAS_SURFACE_HXX

#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/vector/b2isize.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <canvas/rendering/irendermodule.hxx>
#include <canvas/rendering/icolorbuffer.hxx>
#include <canvas/rendering/isurface.hxx>

#include "surfacerect.hxx"
#include "pagemanager.hxx"

namespace canvas
{
    //////////////////////////////////////////////////////////////////////////////////
    // Surface
    //////////////////////////////////////////////////////////////////////////////////

    /** surfaces denote occupied areas withing pages.

        pages encapsulate the hardware buffers that
        contain image data which can be used for texturing.
        surfaces are areas within those pages.
    */
    class Surface
    {
    public:

        Surface( const PageManagerSharedPtr&  rPageManager,
                 const IColorBufferSharedPtr& rColorBuffer,
                 const ::basegfx::B2IPoint&   rPos,
                 const ::basegfx::B2ISize&    rSize );
        ~Surface();

        void setColorBufferDirty();

        /** Render the surface content to screen.

            @param fAlpha
            Overall alpha for content

            @param rPos
            Output position

            @param rTransform
            Output transformation (does not affect output position)
        */
        bool draw( double                         fAlpha,
                   const ::basegfx::B2DPoint&     rPos,
                   const ::basegfx::B2DHomMatrix& rTransform );

        /** Render the surface content to screen.

            @param fAlpha
            Overall alpha for content

            @param rPos
            Output position

            @param rArea
            Subset of the surface to render. Coordinate system are
            surface area pixel, given area will be clipped to the
            surface bounds.

            @param rTransform
            Output transformation (does not affect output position)
        */
        bool drawRectangularArea(
            double                         fAlpha,
            const ::basegfx::B2DPoint&     rPos,
            const ::basegfx::B2DRange&     rArea,
            const ::basegfx::B2DHomMatrix& rTransform );

        /** Render the surface content to screen.

            @param fAlpha
            Overall alpha for content

            @param rPos
            Output position

            @param rClipPoly
            Clip polygon for the surface. The clip polygon is also
            subject to the output transformation.

            @param rTransform
            Output transformation (does not affect output position)
        */
        bool drawWithClip( double                           fAlpha,
                           const ::basegfx::B2DPoint&       rPos,
                           const ::basegfx::B2DPolygon&     rClipPoly,
                           const ::basegfx::B2DHomMatrix&   rTransform );

        // private attributes
    private:
        IColorBufferSharedPtr mpColorBuffer;

        // invoking any of the above defined 'draw' methods
        // will forward primitive commands to the rendermodule.
        PageManagerSharedPtr  mpPageManager;

        FragmentSharedPtr     mpFragment;

        // the offset of this surface with regard to the source
        // image. if the source image had to be tiled into multiple
        // surfaces, this offset denotes the relative pixel distance
        // from the source image's upper, left corner
        ::basegfx::B2IPoint   maSourceOffset;

        // the size in pixels of this surface. please note that
        // this size is likely to be smaller than the size of
        // the colorbuffer we're associated with since we
        // maybe represent only a part of it.
        ::basegfx::B2ISize    maSize;

        bool                  mbIsDirty;

        // private methods
    private:
        bool refresh( canvas::IColorBuffer& rBuffer ) const;
        void prepareRendering();

        basegfx::B2DRectangle getUVCoords() const;
        basegfx::B2DRectangle getUVCoords( const ::basegfx::B2IPoint& rPos,
                                           const ::basegfx::B2ISize&  rSize ) const;
    };

    typedef ::boost::shared_ptr< Surface > SurfaceSharedPtr;
}

#endif
