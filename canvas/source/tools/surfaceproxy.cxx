/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"

#include <boost/bind.hpp>
#include <basegfx/polygon/b2dpolygoncutandtouch.hxx>
#include <basegfx/polygon/b2dpolygontriangulator.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
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
        const ::basegfx::B2DPolygon& rTriangulatedPolygon(
            ::basegfx::triangulator::triangulate(rClipPoly));

#if defined(VERBOSE) && OSL_DEBUG_LEVEL > 0
        // dump polygons
        OSL_TRACE( "Original clip polygon: %s\n"
                   "Triangulated polygon: %s\n",
                   rtl::OUStringToOString(
                       basegfx::tools::exportToSvgD( rClipPoly, true, true, false ),
                       RTL_TEXTENCODING_ASCII_US).getStr(),
                   rtl::OUStringToOString(
                       basegfx::tools::exportToSvgD(
                           basegfx::B2DPolyPolygon(rTriangulatedPolygon), true, true, false ),
                       RTL_TEXTENCODING_ASCII_US).getStr() );
#endif

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
