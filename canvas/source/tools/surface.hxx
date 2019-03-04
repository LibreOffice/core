/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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

#ifndef INCLUDED_CANVAS_SOURCE_TOOLS_SURFACE_HXX
#define INCLUDED_CANVAS_SOURCE_TOOLS_SURFACE_HXX

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
    /** surfaces denote occupied areas within pages.

        pages encapsulate the hardware buffers that
        contain image data which can be used for texturing.
        surfaces are areas within those pages.
    */
    class Surface
    {
    public:

        Surface( const PageManagerSharedPtr&  rPageManager,
                 const std::shared_ptr<IColorBuffer>& rColorBuffer,
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
            const ::basegfx::B2DRectangle& rArea,
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

    private:
        std::shared_ptr<IColorBuffer> mpColorBuffer;

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

    private:
        void prepareRendering();

        basegfx::B2DRectangle getUVCoords() const;
        basegfx::B2DRectangle getUVCoords( const ::basegfx::B2IPoint& rPos,
                                           const ::basegfx::B2ISize&  rSize ) const;
    };

    typedef std::shared_ptr< Surface > SurfaceSharedPtr;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
