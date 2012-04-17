/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <rtl/instance.hxx>
#include <osl/getglobalmutex.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/rendering/InterpolationMode.hpp>
#include <vcl/window.hxx>
#include <vcl/graph.hxx>
#include <vcl/canvastools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

#include <cppcanvas/vclfactory.hxx>

#include <implbitmapcanvas.hxx>
#include <implspritecanvas.hxx>
#include <implpolypolygon.hxx>
#include <implbitmap.hxx>
#include <implrenderer.hxx>
#include <implsprite.hxx>

using namespace ::com::sun::star;

namespace cppcanvas
{
    /* Singleton handling */
    struct InitInstance
    {
        VCLFactory* operator()()
        {
            return new VCLFactory();
        }
    };

    VCLFactory& VCLFactory::getInstance()
    {
        return *rtl_Instance< VCLFactory, InitInstance, ::osl::MutexGuard,
            ::osl::GetGlobalMutex >::create(
                InitInstance(), ::osl::GetGlobalMutex());
    }

    VCLFactory::VCLFactory()
    {
    }

    BitmapCanvasSharedPtr VCLFactory::createCanvas( const uno::Reference< rendering::XBitmapCanvas >& xCanvas )
    {
        return BitmapCanvasSharedPtr(
            new internal::ImplBitmapCanvas( xCanvas ) );
    }

    SpriteCanvasSharedPtr VCLFactory::createSpriteCanvas( const ::Window& rVCLWindow ) const
    {
        return SpriteCanvasSharedPtr(
            new internal::ImplSpriteCanvas(
                uno::Reference< rendering::XSpriteCanvas >(
                    rVCLWindow.GetSpriteCanvas(),
                    uno::UNO_QUERY) ) );
    }

    SpriteCanvasSharedPtr VCLFactory::createSpriteCanvas( const uno::Reference< rendering::XSpriteCanvas >& xCanvas ) const
    {
        return SpriteCanvasSharedPtr(
            new internal::ImplSpriteCanvas( xCanvas ) );
    }

    BitmapSharedPtr VCLFactory::createBitmap( const CanvasSharedPtr&    rCanvas,
                                              const ::BitmapEx&         rBmpEx ) const
    {
        OSL_ENSURE( rCanvas.get() != NULL &&
                    rCanvas->getUNOCanvas().is(),
                    "VCLFactory::createBitmap(): Invalid canvas" );

        if( rCanvas.get() == NULL )
            return BitmapSharedPtr();

        uno::Reference< rendering::XCanvas > xCanvas( rCanvas->getUNOCanvas() );
        if( !xCanvas.is() )
            return BitmapSharedPtr();

        return BitmapSharedPtr( new internal::ImplBitmap( rCanvas,
                                                          ::vcl::unotools::xBitmapFromBitmapEx(
                                                              xCanvas->getDevice(),
                                                              rBmpEx) ) );
    }

    RendererSharedPtr VCLFactory::createRenderer( const CanvasSharedPtr&        rCanvas,
                                                  const ::GDIMetaFile&          rMtf,
                                                  const Renderer::Parameters&   rParms ) const
    {
        return RendererSharedPtr( new internal::ImplRenderer( rCanvas,
                                                              rMtf,
                                                              rParms ) );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
