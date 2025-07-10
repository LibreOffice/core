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

#include <sal/config.h>
#include <sal/log.hxx>

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygontriangulator.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <utility>

#include "surfaceproxy.hxx"

namespace canvas
{
    SurfaceProxy::SurfaceProxy( std::shared_ptr<canvas::IColorBuffer> xBuffer,
                                PageManagerSharedPtr           xPageManager ) :
        mpPageManager(std::move( xPageManager )),
        mpBuffer(std::move( xBuffer ))
    {
        const ::basegfx::B2ISize aImageSize(mpBuffer->getWidth(),mpBuffer->getHeight());
        const ::basegfx::B2ISize aPageSize(mpPageManager->getPageSize());
        const sal_Int32 aPageSizeX(aPageSize.getWidth());
        const sal_Int32 aPageSizeY(aPageSize.getHeight());
        const sal_Int32 aImageSizeX(aImageSize.getWidth());
        const sal_Int32 aImageSizeY(aImageSize.getHeight());

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
                ::basegfx::B2ISize aSize( std::min( aImageSize.getWidth()-x,
                                                      aPageSize.getWidth() ),
                                          std::min( aImageSize.getHeight()-y,
                                                      aPageSize.getHeight() ) );

                maSurfaceList.push_back(
                    std::make_shared<Surface>(
                            mpPageManager,
                            mpBuffer,
                            aOffset,
                            aSize));
            }
        }
    }

    void SurfaceProxy::setColorBufferDirty()
    {
        for( const auto& rSurfacePtr : maSurfaceList )
            rSurfacePtr->setColorBufferDirty();
    }

    bool SurfaceProxy::draw( double                         fAlpha,
                             const ::basegfx::B2DPoint&     rPos,
                             const ::basegfx::B2DHomMatrix& rTransform )
    {
        for( const auto& rSurfacePtr : maSurfaceList )
            rSurfacePtr->draw( fAlpha, rPos, rTransform );

        return true;
    }

    bool SurfaceProxy::draw( double                         fAlpha,
                             const ::basegfx::B2DPoint&     rPos,
                             const ::basegfx::B2DRange&     rArea,
                             const ::basegfx::B2DHomMatrix& rTransform )
    {
        for( const auto& rSurfacePtr : maSurfaceList )
            rSurfacePtr->drawRectangularArea( fAlpha, rPos, rArea, rTransform );

        return true;
    }

    bool SurfaceProxy::draw( double                           fAlpha,
                             const ::basegfx::B2DPoint&       rPos,
                             const ::basegfx::B2DPolyPolygon& rClipPoly,
                             const ::basegfx::B2DHomMatrix&   rTransform )
    {
        const ::basegfx::triangulator::B2DTriangleVector aTriangulatedVector(
            ::basegfx::triangulator::triangulate(rClipPoly));

        // we have now an explicit ::B2DTriangle and ::B2DTriangleVector,
        // but I do not know enough about 'drawWithClip' or 'clipTriangleListOnRange'
        // to adapt to that. Convert back to old three-point-in-polygon convention
        ::basegfx::B2DPolygon aTriangulatedPolygon;
        aTriangulatedPolygon.reserve(aTriangulatedVector.size() * 3);

        for(const auto& rCandidate : aTriangulatedVector)
        {
            aTriangulatedPolygon.append(rCandidate.getA());
            aTriangulatedPolygon.append(rCandidate.getB());
            aTriangulatedPolygon.append(rCandidate.getC());
        }

        // dump polygons
        SAL_INFO("canvas", "Original clip polygon: " <<  basegfx::utils::exportToSvgD( rClipPoly, true, true, false ));
        SAL_INFO("canvas", "Triangulated polygon: " <<  basegfx::utils::exportToSvgD(basegfx::B2DPolyPolygon(aTriangulatedPolygon), true, true, false ));

        for( const auto& rSurfacePtr : maSurfaceList )
            rSurfacePtr->drawWithClip( fAlpha, rPos, aTriangulatedPolygon, rTransform );

        return true;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
