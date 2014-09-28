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

#ifndef INCLUDED_CPPCANVAS_VCLFACTORY_HXX
#define INCLUDED_CPPCANVAS_VCLFACTORY_HXX

#include <cppcanvas/canvas.hxx>
#include <cppcanvas/bitmapcanvas.hxx>
#include <cppcanvas/spritecanvas.hxx>
#include <cppcanvas/polypolygon.hxx>
#include <cppcanvas/bitmap.hxx>
#include <cppcanvas/renderer.hxx>
#include <cppcanvas/text.hxx>
#include <cppcanvas/sprite.hxx>

#include <cppcanvas/cppcanvasdllapi.h>

namespace vcl { class Window; }
class Bitmap;
class BitmapEx;
class Polygon;
namespace tools { class PolyPolygon; }
class Size;
class Graphic;
class GDIMetaFile;
class Animation;

namespace com { namespace sun { namespace star { namespace rendering
{
    class  XCanvas;
    class  XBitmapCanvas;
    class  XSpriteCanvas;
} } } }

/* Definition of VCLFactory class */

namespace cppcanvas
{
    /** The VCLFactory creates Canvas objects for various VCL
        OutputDevice primitives, such as windows, polygons, bitmaps
        and metafiles.

        Please note that the objects created for a specific Canvas can
        only be drawn on exactly that canvas. You have to regenerate
        them for different canvases.
     */
    class CPPCANVAS_DLLPUBLIC VCLFactory
    {
    public:
        static VCLFactory& getInstance();

        CanvasSharedPtr   createCanvas( const ::com::sun::star::uno::Reference<
                                                          ::com::sun::star::rendering::XCanvas >& xCanvas );
        BitmapCanvasSharedPtr   createBitmapCanvas( const ::com::sun::star::uno::Reference<
                                                          ::com::sun::star::rendering::XBitmapCanvas >& xCanvas );

        SpriteCanvasSharedPtr   createSpriteCanvas( const vcl::Window& rVCLWindow ) const;
        SpriteCanvasSharedPtr   createSpriteCanvas( const ::com::sun::star::uno::Reference<
                                                               ::com::sun::star::rendering::XSpriteCanvas >& xCanvas ) const;

        /** Create a bitmap from a VCL Bitmap
         */
        BitmapSharedPtr         createBitmap( const CanvasSharedPtr&, const ::BitmapEx& rBmpEx ) const;

        /** Create a renderer object from a Metafile

            The created renderer initially draws the metafile
            one-by-one units large, in user coordinate space
         */
        RendererSharedPtr       createRenderer( const CanvasSharedPtr&          rCanvas,
                                                const ::GDIMetaFile&            rMtf,
                                                const Renderer::Parameters&     rParms ) const;

    private:
        friend struct InitInstance;

        // singleton
        VCLFactory();

        // default: disabled copy/assignment
        VCLFactory(const VCLFactory&);
        VCLFactory& operator=( const VCLFactory& );
    };

}

#endif // INCLUDED_CPPCANVAS_VCLFACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
