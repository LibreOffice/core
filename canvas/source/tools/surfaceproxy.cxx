/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <boost/bind.hpp>
#include <basegfx/polygon/b2dpolygoncutandtouch.hxx>
#include <basegfx/polygon/b2dpolygontriangulator.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include "surfaceproxy.hxx"

namespace canvas
{
    
    
    

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

        
        
        
        
        
        
        
        
        
        
        size_t dwNumSurfaces(0);
        for(sal_Int32 y=0; y<aImageSizeY; y+=aPageSizeY)
            for(sal_Int32 x=0; x<aImageSizeX; x+=aPageSizeX)
                ++dwNumSurfaces;
        maSurfaceList.reserve(dwNumSurfaces);

        for(sal_Int32 y=0; y<aImageSizeY; y+=aPageSizeY)
        {
            for(sal_Int32 x=0; x<aImageSizeX; x+=aPageSizeX)
            {
                
                
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

    
    
    

    void SurfaceProxy::setColorBufferDirty()
    {
        ::std::for_each( maSurfaceList.begin(),
                         maSurfaceList.end(),
                         ::boost::mem_fn(&Surface::setColorBufferDirty));
    }

    
    
    

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

    
    
    

    bool SurfaceProxy::draw( double                           fAlpha,
                             const ::basegfx::B2DPoint&       rPos,
                             const ::basegfx::B2DPolyPolygon& rClipPoly,
                             const ::basegfx::B2DHomMatrix&   rTransform )
    {
        const ::basegfx::B2DPolygon& rTriangulatedPolygon(
            ::basegfx::triangulator::triangulate(rClipPoly));

#if OSL_DEBUG_LEVEL > 2
        
        OSL_TRACE( "Original clip polygon: %s\n"
                   "Triangulated polygon: %s\n",
                   OUStringToOString(
                       basegfx::tools::exportToSvgD( rClipPoly, true, true, false ),
                       RTL_TEXTENCODING_ASCII_US).getStr(),
                   OUStringToOString(
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
