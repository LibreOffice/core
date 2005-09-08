/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basegfxfactory.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:24:34 $
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

#ifndef INCLUDED_RTL_INSTANCE_HXX
#include <rtl/instance.hxx>
#endif
#ifndef INCLUDED_OSL_GETGLOBALMUTEX_HXX
#include <osl/getglobalmutex.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _COM_SUN_STAR_RENDERING_INTERPOLATIONMODE_HPP_
#include <com/sun/star/rendering/InterpolationMode.hpp>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif
#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif

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
