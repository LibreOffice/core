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

#ifndef _CPPCANVAS_VCLFACTORY_HXX
#define _CPPCANVAS_VCLFACTORY_HXX

#include <cppcanvas/canvas.hxx>
#include <cppcanvas/bitmapcanvas.hxx>
#include <cppcanvas/spritecanvas.hxx>
#include <cppcanvas/polypolygon.hxx>
#include <cppcanvas/bitmap.hxx>
#include <cppcanvas/renderer.hxx>
#include <cppcanvas/text.hxx>
#include <cppcanvas/sprite.hxx>

#include <cppcanvas/cppcanvasdllapi.h>

class Window;
class Bitmap;
class BitmapEx;
class Polygon;
class PolyPolygon;
class Size;
class Graphic;
class GDIMetaFile;
class Animation;

namespace rtl
{
    class OUString;
}
namespace com { namespace sun { namespace star { namespace rendering
{
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

        BitmapCanvasSharedPtr   createCanvas( const ::com::sun::star::uno::Reference<
                                                          ::com::sun::star::rendering::XBitmapCanvas >& xCanvas );

        SpriteCanvasSharedPtr   createSpriteCanvas( const ::Window& rVCLWindow ) const;
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

#endif /* _CPPCANVAS_VCLFACTORY_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
