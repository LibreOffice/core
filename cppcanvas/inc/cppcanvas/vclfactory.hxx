/*************************************************************************
 *
 *  $RCSfile: vclfactory.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 20:52:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CPPCANVAS_VCLFACTORY_HXX
#define _CPPCANVAS_VCLFACTORY_HXX

#ifndef _CPPCANVAS_CANVAS_HXX
#include <cppcanvas/canvas.hxx>
#endif
#ifndef _CPPCANVAS_BITMAPCANVAS_HXX
#include <cppcanvas/bitmapcanvas.hxx>
#endif
#ifndef _CPPCANVAS_SPRITECANVAS_HXX
#include <cppcanvas/spritecanvas.hxx>
#endif
#ifndef _CPPCANVAS_POLYPOLYGON_HXX
#include <cppcanvas/polypolygon.hxx>
#endif
#ifndef _CPPCANVAS_BITMAP_HXX
#include <cppcanvas/bitmap.hxx>
#endif
#ifndef _CPPCANVAS_RENDERER_HXX
#include <cppcanvas/renderer.hxx>
#endif
#ifndef _CPPCANVAS_TEXT_HXX
#include <cppcanvas/text.hxx>
#endif
#ifndef _CPPCANVAS_SPRITE_HXX
#include <cppcanvas/sprite.hxx>
#endif


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
namespace drafts { namespace com { namespace sun { namespace star { namespace rendering
{
    class  XBitmapCanvas;
    class  XSpriteCanvas;
} } } } }

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
    class VCLFactory
    {
    public:
        static VCLFactory& getInstance();

        BitmapCanvasSharedPtr   createCanvas( const ::Window& rVCLWindow );
        BitmapCanvasSharedPtr   createCanvas( const ::com::sun::star::uno::Reference<
                                                          ::drafts::com::sun::star::rendering::XBitmapCanvas >& xCanvas );

        SpriteCanvasSharedPtr   createSpriteCanvas( const ::Window& rVCLWindow ) const;
        SpriteCanvasSharedPtr   createSpriteCanvas( const ::com::sun::star::uno::Reference<
                                                               ::drafts::com::sun::star::rendering::XSpriteCanvas >& xCanvas ) const;
        SpriteCanvasSharedPtr   createFullscreenSpriteCanvas( const ::Window& rVCLWindow, const Size& rFullscreenSize ) const;

        /** Create a polygon from a tools::Polygon

            The created polygon initially has the same size in user
            coordinate space as the source polygon
         */
        PolyPolygonSharedPtr    createPolyPolygon( const CanvasSharedPtr&, const ::Polygon& rPoly ) const;
        PolyPolygonSharedPtr    createPolyPolygon( const CanvasSharedPtr&, const ::PolyPolygon& rPoly ) const;

        /** Create an uninitialized bitmap with the given size
         */
        BitmapSharedPtr         createBitmap( const CanvasSharedPtr&, const ::Size& rSize ) const;

        /** Create an uninitialized alpha bitmap with the given size
         */
        BitmapSharedPtr         createAlphaBitmap( const CanvasSharedPtr&, const ::Size& rSize ) const;

        /** Create a bitmap from a VCL Bitmap
         */
        BitmapSharedPtr         createBitmap( const CanvasSharedPtr&, const ::Bitmap& rBitmap ) const;
        BitmapSharedPtr         createBitmap( const CanvasSharedPtr&, const ::BitmapEx& rBmpEx ) const;

        /** Create a renderer object from a Graphic

            The created renderer initially draws the graphic
            one-by-one units large, in user coordinate space
         */
        RendererSharedPtr       createRenderer( const CanvasSharedPtr&          rCanvas,
                                                const ::Graphic&                rGraphic,
                                                const Renderer::Parameters&     rParms ) const;
        /** Create a renderer object from a Metafile

            The created renderer initially draws the metafile
            one-by-one units large, in user coordinate space
         */
        RendererSharedPtr       createRenderer( const CanvasSharedPtr&          rCanvas,
                                                const ::GDIMetaFile&            rMtf,
                                                const Renderer::Parameters&     rParms ) const;

        /** Create an animated sprite from a VCL animation
         */
        SpriteSharedPtr         createAnimatedSprite( const SpriteCanvasSharedPtr&, const ::Animation& rAnim ) const;

        /** Create a text portion with the given content string
         */
        TextSharedPtr           createText( const CanvasSharedPtr&, const ::rtl::OUString& ) const;

    private:
        friend struct InitInstance;

        // singleton
        VCLFactory();
        ~VCLFactory();

        // default: disabled copy/assignment
        VCLFactory(const VCLFactory&);
        VCLFactory& operator=( const VCLFactory& );
    };

}

#endif /* _CPPCANVAS_VCLFACTORY_HXX */
