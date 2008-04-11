/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: basegfxfactory.cxx,v $
 * $Revision: 1.8 $
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

#include <implpolypolygon.hxx>
#include <implbitmap.hxx>
#include <impltext.hxx>


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
