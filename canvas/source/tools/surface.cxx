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


#include "surface.hxx"
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <comphelper/scopeguard.hxx>
#include <boost/bind.hpp>

namespace canvas
{

    
    
    

    Surface::Surface( const PageManagerSharedPtr&  rPageManager,
                      const IColorBufferSharedPtr& rColorBuffer,
                      const ::basegfx::B2IPoint&   rPos,
                      const ::basegfx::B2ISize&    rSize ) :
        mpColorBuffer(rColorBuffer),
        mpPageManager(rPageManager),
        mpFragment(),
        maSourceOffset(rPos),
        maSize(rSize),
        mbIsDirty(true)
    {
    }

    
    
    

    Surface::~Surface()
    {
        if(mpFragment)
            mpPageManager->free(mpFragment);
    }

    
    
    

    void Surface::setColorBufferDirty()
    {
        mbIsDirty=true;
    }

    
    
    

    basegfx::B2DRectangle Surface::getUVCoords() const
    {
        ::basegfx::B2ISize aPageSize(mpPageManager->getPageSize());
        ::basegfx::B2IPoint aDestOffset;
        if( mpFragment )
            aDestOffset = mpFragment->getPos();

        const double pw( aPageSize.getX() );
        const double ph( aPageSize.getY() );
        const double ox( aDestOffset.getX() );
        const double oy( aDestOffset.getY() );
        const double sx( maSize.getX() );
        const double sy( maSize.getY() );

        return ::basegfx::B2DRectangle( ox/pw,
                                        oy/ph,
                                        (ox+sx)/pw,
                                        (oy+sy)/ph );
    }

    
    
    

    basegfx::B2DRectangle Surface::getUVCoords( const ::basegfx::B2IPoint& rPos,
                                                const ::basegfx::B2ISize&  rSize ) const
    {
        ::basegfx::B2ISize aPageSize(mpPageManager->getPageSize());

        const double pw( aPageSize.getX() );
        const double ph( aPageSize.getY() );
        const double ox( rPos.getX() );
        const double oy( rPos.getY() );
        const double sx( rSize.getX() );
        const double sy( rSize.getY() );

        return ::basegfx::B2DRectangle( ox/pw,
                                        oy/ph,
                                        (ox+sx)/pw,
                                        (oy+sy)/ph );
    }

    
    
    

    bool Surface::draw( double                          fAlpha,
                        const ::basegfx::B2DPoint&      rPos,
                        const ::basegfx::B2DHomMatrix&  rTransform )
    {
        IRenderModuleSharedPtr pRenderModule(mpPageManager->getRenderModule());

        RenderModuleGuard aGuard( pRenderModule );

        prepareRendering();

        
        const ::basegfx::B2DRectangle& rUV( getUVCoords() );

        const double u1(rUV.getMinX());
        const double v1(rUV.getMinY());
        const double u2(rUV.getMaxX());
        const double v2(rUV.getMaxY());

        
        
        
        
        
        
        
        basegfx::B2DHomMatrix aTransform(basegfx::tools::createTranslateB2DHomMatrix(
            maSourceOffset.getX(), maSourceOffset.getY()));
        aTransform = aTransform * rTransform;
        aTransform.translate(::basegfx::fround(rPos.getX()),
                             ::basegfx::fround(rPos.getY()));

        /*
            ######################################
            ######################################
            ######################################

                           Y
                           ^+1
                           |
                   2       |       3
                     x------------x
                     |     |      |
                      |    |      |
               ------|-----O------|------>X
               -1    |     |      |     +1
                     |     |      |
                     x------------x
                    1      |       0
                            |
                           |-1

            ######################################
            ######################################
            ######################################
        */

        const ::basegfx::B2DPoint& p0(aTransform * ::basegfx::B2DPoint(maSize.getX(),maSize.getY()));
        const ::basegfx::B2DPoint& p1(aTransform * ::basegfx::B2DPoint(0.0,maSize.getY()));
        const ::basegfx::B2DPoint& p2(aTransform * ::basegfx::B2DPoint(0.0,0.0));
        const ::basegfx::B2DPoint& p3(aTransform * ::basegfx::B2DPoint(maSize.getX(),0.0));

        canvas::Vertex vertex;
        vertex.r = 1.0f;
        vertex.g = 1.0f;
        vertex.b = 1.0f;
        vertex.a = static_cast<float>(fAlpha);
        vertex.z = 0.0f;

        {
            pRenderModule->beginPrimitive( canvas::IRenderModule::PRIMITIVE_TYPE_QUAD );

            
            const ::comphelper::ScopeGuard aScopeGuard(
                boost::bind( &::canvas::IRenderModule::endPrimitive,
                             ::boost::ref(pRenderModule) ) );

            vertex.u=static_cast<float>(u2); vertex.v=static_cast<float>(v2);
            vertex.x=static_cast<float>(p0.getX()); vertex.y=static_cast<float>(p0.getY());
            pRenderModule->pushVertex(vertex);

            vertex.u=static_cast<float>(u1); vertex.v=static_cast<float>(v2);
            vertex.x=static_cast<float>(p1.getX()); vertex.y=static_cast<float>(p1.getY());
            pRenderModule->pushVertex(vertex);

            vertex.u=static_cast<float>(u1); vertex.v=static_cast<float>(v1);
            vertex.x=static_cast<float>(p2.getX()); vertex.y=static_cast<float>(p2.getY());
            pRenderModule->pushVertex(vertex);

            vertex.u=static_cast<float>(u2); vertex.v=static_cast<float>(v1);
            vertex.x=static_cast<float>(p3.getX()); vertex.y=static_cast<float>(p3.getY());
            pRenderModule->pushVertex(vertex);
        }

        return !(pRenderModule->isError());
    }

    
    
    

    bool Surface::drawRectangularArea(
                        double                         fAlpha,
                        const ::basegfx::B2DPoint&     rPos,
                        const ::basegfx::B2DRectangle& rArea,
                        const ::basegfx::B2DHomMatrix& rTransform )
    {
        if( rArea.isEmpty() )
            return true; 

        IRenderModuleSharedPtr pRenderModule(mpPageManager->getRenderModule());

        RenderModuleGuard aGuard( pRenderModule );

        prepareRendering();

        
        ::basegfx::B2IPoint aPos1(
            ::basegfx::fround(rArea.getMinimum().getX()),
            ::basegfx::fround(rArea.getMinimum().getY()));
        ::basegfx::B2IPoint aPos2(
            ::basegfx::fround(rArea.getMaximum().getX()),
            ::basegfx::fround(rArea.getMaximum().getY()) );

        
        aPos1.setX(::std::max(aPos1.getX(),maSourceOffset.getX()));
        aPos1.setY(::std::max(aPos1.getY(),maSourceOffset.getY()));
        aPos2.setX(::std::min(aPos2.getX(),maSourceOffset.getX()+maSize.getX()));
        aPos2.setY(::std::min(aPos2.getY(),maSourceOffset.getY()+maSize.getY()));

        
        ::basegfx::B2IVector aSize(aPos2 - aPos1);
        if(aSize.getX() <= 0 || aSize.getY() <= 0)
            return true;

        ::basegfx::B2IPoint aDestOffset;
        if( mpFragment )
            aDestOffset = mpFragment->getPos();

        
        const ::basegfx::B2DRectangle& rUV(
            getUVCoords(aPos1 - maSourceOffset + aDestOffset,
                        aSize) );
        const double u1(rUV.getMinX());
        const double v1(rUV.getMinY());
        const double u2(rUV.getMaxX());
        const double v2(rUV.getMaxY());

        
        
        
        
        basegfx::B2DHomMatrix aTransform(basegfx::tools::createTranslateB2DHomMatrix(aPos1.getX(), aPos1.getY()));
        aTransform = aTransform * rTransform;
        aTransform.translate(::basegfx::fround(rPos.getX()),
                             ::basegfx::fround(rPos.getY()));


        /*
            ######################################
            ######################################
            ######################################

                           Y
                           ^+1
                           |
                   2       |       3
                     x------------x
                     |     |      |
                      |    |      |
               ------|-----O------|------>X
               -1    |     |      |     +1
                     |     |      |
                     x------------x
                    1      |       0
                            |
                           |-1

            ######################################
            ######################################
            ######################################
        */

        const ::basegfx::B2DPoint& p0(aTransform * ::basegfx::B2DPoint(aSize.getX(),aSize.getY()));
        const ::basegfx::B2DPoint& p1(aTransform * ::basegfx::B2DPoint(0.0,         aSize.getY()));
        const ::basegfx::B2DPoint& p2(aTransform * ::basegfx::B2DPoint(0.0,         0.0));
        const ::basegfx::B2DPoint& p3(aTransform * ::basegfx::B2DPoint(aSize.getX(),0.0));

        canvas::Vertex vertex;
        vertex.r = 1.0f;
        vertex.g = 1.0f;
        vertex.b = 1.0f;
        vertex.a = static_cast<float>(fAlpha);
        vertex.z = 0.0f;

        {
            pRenderModule->beginPrimitive( canvas::IRenderModule::PRIMITIVE_TYPE_QUAD );

            
            const ::comphelper::ScopeGuard aScopeGuard(
                boost::bind( &::canvas::IRenderModule::endPrimitive,
                             ::boost::ref(pRenderModule) ) );

            vertex.u=static_cast<float>(u2); vertex.v=static_cast<float>(v2);
            vertex.x=static_cast<float>(p0.getX()); vertex.y=static_cast<float>(p0.getY());
            pRenderModule->pushVertex(vertex);

            vertex.u=static_cast<float>(u1); vertex.v=static_cast<float>(v2);
            vertex.x=static_cast<float>(p1.getX()); vertex.y=static_cast<float>(p1.getY());
            pRenderModule->pushVertex(vertex);

            vertex.u=static_cast<float>(u1); vertex.v=static_cast<float>(v1);
            vertex.x=static_cast<float>(p2.getX()); vertex.y=static_cast<float>(p2.getY());
            pRenderModule->pushVertex(vertex);

            vertex.u=static_cast<float>(u2); vertex.v=static_cast<float>(v1);
            vertex.x=static_cast<float>(p3.getX()); vertex.y=static_cast<float>(p3.getY());
            pRenderModule->pushVertex(vertex);
        }

        return !(pRenderModule->isError());
    }

    
    
    

    bool Surface::drawWithClip( double                          fAlpha,
                                const ::basegfx::B2DPoint&      rPos,
                                const ::basegfx::B2DPolygon&    rClipPoly,
                                const ::basegfx::B2DHomMatrix&  rTransform )
    {
        IRenderModuleSharedPtr pRenderModule(mpPageManager->getRenderModule());

        RenderModuleGuard aGuard( pRenderModule );

        prepareRendering();

        
        
        
        const double x1(maSourceOffset.getX());
        const double y1(maSourceOffset.getY());
        const double w(maSize.getX());
        const double h(maSize.getY());
        const double x2(x1+w);
        const double y2(y1+h);
        const ::basegfx::B2DRectangle aSurfaceClipRect(x1,y1,x2,y2);

        
        
        
        
        
        basegfx::B2DHomMatrix aTransform;
        aTransform = aTransform * rTransform;
        aTransform.translate(::basegfx::fround(rPos.getX()),
                             ::basegfx::fround(rPos.getY()));

        /*
            ######################################
            ######################################
            ######################################

                           Y
                           ^+1
                           |
                   2       |       3
                     x------------x
                     |     |      |
                      |    |      |
               ------|-----O------|------>X
               -1    |     |      |     +1
                     |     |      |
                     x------------x
                    1      |       0
                            |
                           |-1

            ######################################
            ######################################
            ######################################
        */

        
        
        const ::basegfx::B2DRectangle& rUV( getUVCoords() );

        basegfx::B2DPolygon rTriangleList(basegfx::tools::clipTriangleListOnRange(rClipPoly,
                                                                                  aSurfaceClipRect));

        
        if(const sal_uInt32 nVertexCount = rTriangleList.count())
        {
            canvas::Vertex vertex;
            vertex.r = 1.0f;
            vertex.g = 1.0f;
            vertex.b = 1.0f;
            vertex.a = static_cast<float>(fAlpha);
            vertex.z = 0.0f;

#if defined(TRIANGLE_LOG) && defined(DBG_UTIL)
            OSL_TRACE( "Surface::draw(): numvertices %d numtriangles %d\n",
                        nVertexCount,
                        nVertexCount/3 );
#endif

            pRenderModule->beginPrimitive( canvas::IRenderModule::PRIMITIVE_TYPE_TRIANGLE );

            
            const ::comphelper::ScopeGuard aScopeGuard(
                boost::bind( &::canvas::IRenderModule::endPrimitive,
                                ::boost::ref(pRenderModule) ) );

            for(sal_uInt32 nIndex=0; nIndex<nVertexCount; ++nIndex)
            {
                const basegfx::B2DPoint &aPoint = rTriangleList.getB2DPoint(nIndex);
                basegfx::B2DPoint aTransformedPoint(aTransform * aPoint);
                const double tu(((aPoint.getX()-aSurfaceClipRect.getMinX())*rUV.getWidth()/w)+rUV.getMinX());
                const double tv(((aPoint.getY()-aSurfaceClipRect.getMinY())*rUV.getHeight()/h)+rUV.getMinY());
                vertex.u=static_cast<float>(tu);
                vertex.v=static_cast<float>(tv);
                vertex.x=static_cast<float>(aTransformedPoint.getX());
                vertex.y=static_cast<float>(aTransformedPoint.getY());
                pRenderModule->pushVertex(vertex);
            }
        }

        return !(pRenderModule->isError());
    }

    
    
    

    void Surface::prepareRendering()
    {
        mpPageManager->validatePages();

        
        
        
        
        if(!(mpFragment))
        {
            mpFragment = mpPageManager->allocateSpace(maSize);
            if( mpFragment )
            {
                mpFragment->setColorBuffer(mpColorBuffer);
                mpFragment->setSourceOffset(maSourceOffset);
            }
        }

        if( mpFragment )
        {
            
            
            
            
            
            if(!(mpFragment->select(mbIsDirty)))
                mpPageManager->nakedFragment(mpFragment);

        }
        mbIsDirty=false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
