/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <com/sun/star/rendering/XCanvas.hpp>
#include <osl/diagnose.h>
#include <vcl/window.hxx>
#include <vcl/canvastools.hxx>

#include "vclfactory.hxx"

#include "canvas.hxx"
#include "renderer.hxx"
#include <canvas/cppcanvastest.hxx>
#include <canvas.hxx>

using namespace ::com::sun::star;

namespace cppcanvas
{
    CanvasSharedPtr VCLFactory::createCanvas( const uno::Reference< rendering::XCanvas >& xCanvas )
    {
        return std::make_shared<Canvas>( xCanvas );
    }

    RendererSharedPtr VCLFactory::createRenderer( const CanvasSharedPtr&        rCanvas,
                                                  const ::GDIMetaFile&          rMtf )
    {
        return std::make_shared<Renderer>( rCanvas, rMtf );
    }

    // only here so we can do a unit test from drawinglayer/qa/unit/vclmetafileprocessor2d.cxx
    bool testCanvasDraw(OutputDevice* pOutDev,
            const basegfx::B2DHomMatrix& rTransform1,
            GDIMetaFile& rMetaFile,
            const basegfx::B2DHomMatrix& rTransform2)
    {
        css::uno::Reference<css::rendering::XCanvas> rCanvas = new vclcanvas::Canvas(pOutDev);
        cppcanvas::CanvasSharedPtr cppCanvas = cppcanvas::VCLFactory::createCanvas(rCanvas);
        // I got these matrices from a breakpoint in drawing the polyline, and walking up
        // the stack to the canvas code.
        cppCanvas->setTransformation(rTransform1);
        cppcanvas::RendererSharedPtr renderer
            = cppcanvas::VCLFactory::createRenderer(cppCanvas, rMetaFile);
        renderer->setTransformation(rTransform2);
        return renderer->draw();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
