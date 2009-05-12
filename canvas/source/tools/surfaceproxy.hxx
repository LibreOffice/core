/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: surfaceproxy.hxx,v $
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

#ifndef INCLUDED_CANVAS_SURFACEPROXY_HXX
#define INCLUDED_CANVAS_SURFACEPROXY_HXX

#include <canvas/rendering/isurfaceproxy.hxx>
#include <canvas/rendering/icolorbuffer.hxx>

#include "pagemanager.hxx"
#include "surface.hxx"

namespace canvas
{

    //////////////////////////////////////////////////////////////////////////////////
    // SurfaceProxy
    //////////////////////////////////////////////////////////////////////////////////

    /** Definition of the surface proxy class.

        Surface proxies are the connection between *one* source image
        and *one or more* hardware surfaces (or textures).  in a
        logical structure surface proxies represent soley this
        dependeny plus some simple cache management.
    */
    class SurfaceProxy : public ISurfaceProxy
    {
    public:

        SurfaceProxy( const canvas::IColorBufferSharedPtr& pBuffer,
                      const PageManagerSharedPtr &pPageManager );

        // ISurfaceProxy interface
        virtual void setColorBufferDirty();

        /** Render the surface content to screen.

            @param fAlpha
            Overall alpha for content

            @param rPos
            Output position

            @param rTransform
            Output transformation (does not affect output position)
        */
        virtual bool draw( double                         fAlpha,
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
        virtual bool draw( double                         fAlpha,
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
        virtual bool draw( double                           fAlpha,
                           const ::basegfx::B2DPoint&       rPos,
                           const ::basegfx::B2DPolyPolygon& rClipPoly,
                           const ::basegfx::B2DHomMatrix&   rTransform );

    private:
        PageManagerSharedPtr          mpPageManager;

        // the pagemanager will distribute the image
        // to one or more surfaces, this is why we
        // need a list here.
        std::vector<SurfaceSharedPtr> maSurfaceList;

        // pointer to the source of image data
        // which always is stored in system memory,
        // 32bit rgba and can have any size.
        canvas::IColorBufferSharedPtr mpBuffer;
    };

    typedef ::boost::shared_ptr< SurfaceProxy > SurfaceProxySharedPtr;
}

#endif
