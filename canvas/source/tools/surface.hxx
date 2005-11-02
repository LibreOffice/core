/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: surface.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:55:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_CANVAS_SURFACE_HXX
#define INCLUDED_CANVAS_SURFACE_HXX

#ifndef _BGFX_POINT_B2IPOINT_HXX
#include <basegfx/point/b2ipoint.hxx>
#endif
#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif
#ifndef _BGFX_RANGE_B2DRECTANGLE_HXX
#include <basegfx/range/b2drectangle.hxx>
#endif
#ifndef _BGFX_VECTOR_B2ISIZE_HXX
#include <basegfx/vector/b2isize.hxx>
#endif
#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif
#ifndef INCLUDED_CANVAS_IRENDERMODULE_HXX
#include <canvas/rendering/irendermodule.hxx>
#endif
#ifndef INCLUDED_CANVAS_ICOLORBUFFER_HXX
#include <canvas/rendering/icolorbuffer.hxx>
#endif
#ifndef INCLUDED_CANVAS_ISURFACE_HXX
#include <canvas/rendering/isurface.hxx>
#endif

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
