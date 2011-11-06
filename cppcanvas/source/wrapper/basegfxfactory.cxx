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
#include "precompiled_cppcanvas.hxx"

#include <rtl/instance.hxx>
#include <osl/getglobalmutex.hxx>
#include <osl/diagnose.h>

#include <com/sun/star/rendering/InterpolationMode.hpp>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/tools/canvastools.hxx>

#include <cppcanvas/basegfxfactory.hxx>

#include "implpolypolygon.hxx"
#include "implbitmap.hxx"
#include "impltext.hxx"


using namespace ::com::sun::star;

namespace cppcanvas
{
    /* Singleton handling */
    struct InitInstance2
    {
        BaseGfxFactory* operator()()
        {
            return new BaseGfxFactory();
        }
    };

    BaseGfxFactory& BaseGfxFactory::getInstance()
    {
        return *rtl_Instance< BaseGfxFactory, InitInstance2, ::osl::MutexGuard,
            ::osl::GetGlobalMutex >::create(
                InitInstance2(), ::osl::GetGlobalMutex());
    }

    BaseGfxFactory::BaseGfxFactory()
    {
    }

    BaseGfxFactory::~BaseGfxFactory()
    {
    }

    PolyPolygonSharedPtr BaseGfxFactory::createPolyPolygon( const CanvasSharedPtr&          rCanvas,
                                                            const ::basegfx::B2DPolygon&    rPoly ) const
    {
        OSL_ENSURE( rCanvas.get() != NULL &&
                    rCanvas->getUNOCanvas().is(),
                    "BaseGfxFactory::createPolyPolygon(): Invalid canvas" );

        if( rCanvas.get() == NULL )
            return PolyPolygonSharedPtr();

        uno::Reference< rendering::XCanvas > xCanvas( rCanvas->getUNOCanvas() );
        if( !xCanvas.is() )
            return PolyPolygonSharedPtr();

        return PolyPolygonSharedPtr(
            new internal::ImplPolyPolygon( rCanvas,
                                           ::basegfx::unotools::xPolyPolygonFromB2DPolygon(
                                               xCanvas->getDevice(),
                                               rPoly) ) );
    }

    PolyPolygonSharedPtr BaseGfxFactory::createPolyPolygon( const CanvasSharedPtr&              rCanvas,
                                                            const ::basegfx::B2DPolyPolygon&    rPolyPoly ) const
    {
        OSL_ENSURE( rCanvas.get() != NULL &&
                    rCanvas->getUNOCanvas().is(),
                    "BaseGfxFactory::createPolyPolygon(): Invalid canvas" );

        if( rCanvas.get() == NULL )
            return PolyPolygonSharedPtr();

        uno::Reference< rendering::XCanvas > xCanvas( rCanvas->getUNOCanvas() );
        if( !xCanvas.is() )
            return PolyPolygonSharedPtr();

        return PolyPolygonSharedPtr(
            new internal::ImplPolyPolygon( rCanvas,
                                           ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                                               xCanvas->getDevice(),
                                               rPolyPoly) ) );
    }

    BitmapSharedPtr BaseGfxFactory::createBitmap( const CanvasSharedPtr&    rCanvas,
                                                  const ::basegfx::B2ISize& rSize ) const
    {
        OSL_ENSURE( rCanvas.get() != NULL &&
                    rCanvas->getUNOCanvas().is(),
                    "BaseGfxFactory::createBitmap(): Invalid canvas" );

        if( rCanvas.get() == NULL )
            return BitmapSharedPtr();

        uno::Reference< rendering::XCanvas > xCanvas( rCanvas->getUNOCanvas() );
        if( !xCanvas.is() )
            return BitmapSharedPtr();

        return BitmapSharedPtr(
            new internal::ImplBitmap( rCanvas,
                                      xCanvas->getDevice()->createCompatibleBitmap(
                                          ::basegfx::unotools::integerSize2DFromB2ISize(rSize) ) ) );
    }

    BitmapSharedPtr BaseGfxFactory::createAlphaBitmap( const CanvasSharedPtr&   rCanvas,
                                                       const ::basegfx::B2ISize& rSize ) const
    {
        OSL_ENSURE( rCanvas.get() != NULL &&
                    rCanvas->getUNOCanvas().is(),
                    "BaseGfxFactory::createBitmap(): Invalid canvas" );

        if( rCanvas.get() == NULL )
            return BitmapSharedPtr();

        uno::Reference< rendering::XCanvas > xCanvas( rCanvas->getUNOCanvas() );
        if( !xCanvas.is() )
            return BitmapSharedPtr();

        return BitmapSharedPtr(
            new internal::ImplBitmap( rCanvas,
                                      xCanvas->getDevice()->createCompatibleAlphaBitmap(
                                          ::basegfx::unotools::integerSize2DFromB2ISize(rSize) ) ) );
    }

    TextSharedPtr BaseGfxFactory::createText( const CanvasSharedPtr& rCanvas, const ::rtl::OUString& rText ) const
    {
        return TextSharedPtr( new internal::ImplText( rCanvas,
                                                      rText ) );
    }

}
