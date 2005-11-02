/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: surfaceproxy.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:55:51 $
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

#include <boost/bind.hpp>
#include <basegfx/polygon/b2dpolygoncutandtouch.hxx>
#include <basegfx/polygon/b2dpolygontriangulator.hxx>
#include "surfaceproxy.hxx"

namespace canvas
{
    //////////////////////////////////////////////////////////////////////////////////
    // SurfaceProxy::SurfaceProxy
    //////////////////////////////////////////////////////////////////////////////////

    SurfaceProxy::SurfaceProxy( const canvas::IColorBufferSharedPtr& pBuffer,
                                const PageManagerSharedPtr&          pPageManager ) :
        mpPageManager( pPageManager ),
        maSurfaceList(),
        mpBuffer( pBuffer )
    {
        const ::basegfx::B2ISize aImageSize(mpBuffer->getWidth(),mpBuffer->getHeight());
        const ::basegfx::B2ISize aPageSize(mpPageManager->getPageSize());
        const sal_Int32 aPageSizeX(aPageSize.getX());
        const sal_Int32 aPageSizeY(aPageSize.getY());
        const sal_Int32 aImageSizeX(aImageSize.getX());
        const sal_Int32 aImageSizeY(aImageSize.getY());

        // see if the size of the colorbuffer is larger than the size
        // of a single page. if this is the case we divide the
        // colorbuffer into as many surfaces as we need to get the
        // whole area distributed.  otherwise (the colorbuffer is
        // smaller than the size of a single page) we search for free
        // pages or create a new one.
        // the incoming image is too large to fit into a single
        // page.  strategy: we split the image into rectangular
        // areas that are as large as the maximum page size
        // dictates and follow the strategy for fitting images.
        size_t dwNumSurfaces(0);
        for(sal_Int32 y=0; y<aImageSizeY; y+=aPageSizeY)
            for(sal_Int32 x=0; x<aImageSizeX; x+=aPageSizeX)
                ++dwNumSurfaces;
        maSurfaceList.reserve(dwNumSurfaces);

        for(sal_Int32 y=0; y<aImageSizeY; y+=aPageSizeY)
        {
            for(sal_Int32 x=0; x<aImageSizeX; x+=aPageSizeX)
            {
                // the current surface is located at the position [x,y]
                // and has the size [min(restx,pagesizex),min(resty,pagesizey)
                ::basegfx::B2IPoint aOffset(x,y);
                ::basegfx::B2ISize aSize( ::std::min( aImageSize.getX()-x,
                                                      aPageSize.getX() ),
                                          ::std::min( aImageSize.getY()-y,
                                                      aPageSize.getY() ) );

                maSurfaceList.push_back(
                    SurfaceSharedPtr(
                        new Surface(
                            mpPageManager,
                            mpBuffer,
                            aOffset,
                            aSize)));
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////////
    // SurfaceProxy::setColorBufferDirty
    //////////////////////////////////////////////////////////////////////////////////

    void SurfaceProxy::setColorBufferDirty()
    {
        ::std::for_each( maSurfaceList.begin(),
                         maSurfaceList.end(),
                         ::boost::mem_fn(&Surface::setColorBufferDirty));
    }

    //////////////////////////////////////////////////////////////////////////////////
    // SurfaceProxy::draw
    //////////////////////////////////////////////////////////////////////////////////

    bool SurfaceProxy::draw( double                         fAlpha,
                             const ::basegfx::B2DPoint&     rPos,
                             const ::basegfx::B2DHomMatrix& rTransform )
    {
        ::std::for_each( maSurfaceList.begin(),
                         maSurfaceList.end(),
                         ::boost::bind( &Surface::draw,
                                        _1,
                                        fAlpha,
                                        ::boost::cref(rPos),
                                        ::boost::cref(rTransform)));

        return true;
    }

    //////////////////////////////////////////////////////////////////////////////////
    // SurfaceProxy::draw
    //////////////////////////////////////////////////////////////////////////////////

    bool SurfaceProxy::draw( double                         fAlpha,
                             const ::basegfx::B2DPoint&     rPos,
                             const ::basegfx::B2DRange&     rArea,
                             const ::basegfx::B2DHomMatrix& rTransform )
    {
        ::std::for_each( maSurfaceList.begin(),
                         maSurfaceList.end(),
                         ::boost::bind(&Surface::drawRectangularArea,
                                       _1,
                                       fAlpha,
                                       ::boost::cref(rPos),
                                       ::boost::cref(rArea),
                                       ::boost::cref(rTransform)));

        return true;
    }

    //////////////////////////////////////////////////////////////////////////////////
    // SurfaceProxy::draw
    //////////////////////////////////////////////////////////////////////////////////

    bool SurfaceProxy::draw( double                           fAlpha,
                             const ::basegfx::B2DPoint&       rPos,
                             const ::basegfx::B2DPolyPolygon& rClipPoly,
                             const ::basegfx::B2DHomMatrix&   rTransform )
    {
        const ::basegfx::B2DPolyPolygon& rPreparedPolyPolygon(
            ::basegfx::tools::addPointsAtCutsAndTouches(rClipPoly) );
        const ::basegfx::B2DPolygon& rTriangulatedPolygon(
            ::basegfx::triangulator::triangulate(rPreparedPolyPolygon));

        ::std::for_each( maSurfaceList.begin(),
                         maSurfaceList.end(),
                         ::boost::bind(&Surface::drawWithClip,
                                       _1,
                                       fAlpha,
                                       ::boost::cref(rPos),
                                       ::boost::cref(rTriangulatedPolygon),
                                       ::boost::cref(rTransform)));

        return true;
    }
}
