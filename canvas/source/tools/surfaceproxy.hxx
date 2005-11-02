/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: surfaceproxy.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:56:02 $
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

#ifndef INCLUDED_CANVAS_SURFACEPROXY_HXX
#define INCLUDED_CANVAS_SURFACEPROXY_HXX

#ifndef INCLUDED_CANVAS_ISURFACEPROXY_HXX
#include <canvas/rendering/isurfaceproxy.hxx>
#endif
#ifndef INCLUDED_CANVAS_ICOLORBUFFER_HXX
#include <canvas/rendering/icolorbuffer.hxx>
#endif

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
