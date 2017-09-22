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


#include <rtl/instance.hxx>
#include <osl/getglobalmutex.hxx>
#include <osl/diagnose.h>

#include <com/sun/star/rendering/InterpolationMode.hpp>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/utils/canvastools.hxx>

#include <cppcanvas/basegfxfactory.hxx>

#include <com/sun/star/rendering/RenderState.hpp>
#include <com/sun/star/rendering/StringContext.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XCanvasFont.hpp>

#include <canvasgraphichelper.hxx>

#include "implpolypolygon.hxx"
#include "implbitmap.hxx"

using namespace ::com::sun::star;

namespace cppcanvas
{
    PolyPolygonSharedPtr BaseGfxFactory::createPolyPolygon( const CanvasSharedPtr&          rCanvas,
                                                            const ::basegfx::B2DPolygon&    rPoly )
    {
        OSL_ENSURE( rCanvas.get() != nullptr &&
                    rCanvas->getUNOCanvas().is(),
                    "BaseGfxFactory::createPolyPolygon(): Invalid canvas" );

        if( rCanvas.get() == nullptr )
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

    BitmapSharedPtr BaseGfxFactory::createBitmap( const CanvasSharedPtr&    rCanvas,
                                                  const ::basegfx::B2ISize& rSize )
    {
        OSL_ENSURE( rCanvas.get() != nullptr &&
                    rCanvas->getUNOCanvas().is(),
                    "BaseGfxFactory::createBitmap(): Invalid canvas" );

        if( rCanvas.get() == nullptr )
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
                                                       const ::basegfx::B2ISize& rSize )
    {
        OSL_ENSURE( rCanvas.get() != nullptr &&
                    rCanvas->getUNOCanvas().is(),
                    "BaseGfxFactory::createBitmap(): Invalid canvas" );

        if( rCanvas.get() == nullptr )
            return BitmapSharedPtr();

        uno::Reference< rendering::XCanvas > xCanvas( rCanvas->getUNOCanvas() );
        if( !xCanvas.is() )
            return BitmapSharedPtr();

        return BitmapSharedPtr(
            new internal::ImplBitmap( rCanvas,
                                      xCanvas->getDevice()->createCompatibleAlphaBitmap(
                                          ::basegfx::unotools::integerSize2DFromB2ISize(rSize) ) ) );
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
