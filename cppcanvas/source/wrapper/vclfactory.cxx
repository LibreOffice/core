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

#include <osl/diagnose.h>
#include <vcl/window.hxx>
#include <vcl/canvastools.hxx>

#include <cppcanvas/vclfactory.hxx>

#include "implbitmapcanvas.hxx"
#include "implspritecanvas.hxx"
#include "implbitmap.hxx"
#include <implrenderer.hxx>

using namespace ::com::sun::star;

namespace cppcanvas
{
    CanvasSharedPtr VCLFactory::createCanvas( const uno::Reference< rendering::XCanvas >& xCanvas )
    {
        return std::make_shared<internal::ImplCanvas>( xCanvas );
    }

    BitmapCanvasSharedPtr VCLFactory::createBitmapCanvas( const uno::Reference< rendering::XBitmapCanvas >& xCanvas )
    {
        return std::make_shared<internal::ImplBitmapCanvas>( xCanvas );
    }

    SpriteCanvasSharedPtr VCLFactory::createSpriteCanvas( const vcl::Window& rVCLWindow )
    {
        return std::make_shared<internal::ImplSpriteCanvas>(
                    rVCLWindow.GetOutDev()->GetSpriteCanvas() );
    }

    SpriteCanvasSharedPtr VCLFactory::createSpriteCanvas( const uno::Reference< rendering::XSpriteCanvas >& xCanvas )
    {
        return std::make_shared<internal::ImplSpriteCanvas>( xCanvas );
    }

    BitmapSharedPtr VCLFactory::createBitmap( const CanvasSharedPtr&    rCanvas,
                                              const ::Bitmap&         rBmp )
    {
        OSL_ENSURE( rCanvas && rCanvas->getUNOCanvas().is(),
                    "VCLFactory::createBitmap(): Invalid canvas" );

        if( !rCanvas )
            return BitmapSharedPtr();

        uno::Reference< rendering::XCanvas > xCanvas( rCanvas->getUNOCanvas() );
        if( !xCanvas.is() )
            return BitmapSharedPtr();

        return std::make_shared<internal::ImplBitmap>( rCanvas,
                                                          vcl::unotools::xBitmapFromBitmap(rBmp) );
    }

    RendererSharedPtr VCLFactory::createRenderer( const CanvasSharedPtr&        rCanvas,
                                                  const ::GDIMetaFile&          rMtf,
                                                  const Renderer::Parameters&   rParms )
    {
        return std::make_shared<internal::ImplRenderer>( rCanvas,
                                                              rMtf,
                                                              rParms );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
