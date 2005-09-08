/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basegfxfactory.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:11:50 $
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

#ifndef _CPPCANVAS_BASEGFXFACTORY_HXX
#define _CPPCANVAS_BASEGFXFACTORY_HXX

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

#ifndef _BGFX_VECTOR_B2ISIZE_HXX
#include <basegfx/vector/b2isize.hxx>
#endif


namespace basegfx
{
    class B2DPolygon;
    class B2DPolyPolygon;
}

namespace rtl
{
    class OUString;
}

/* Definition of BaseGfxFactory class */

namespace cppcanvas
{
    /** The BaseGfxFactory creates Canvas objects for various basegfx
        primitives, such as polygons and bitmaps (not yet
        implemented).

        Please note that the objects created for a specific Canvas can
        only be drawn on exactly that canvas. You have to regenerate
        them for different canvases.
     */
    class BaseGfxFactory
    {
    public:
        static BaseGfxFactory& getInstance();

        /** Create a polygon from a ::basegfx::B2DPolygon

            The created polygon initially has the same size in user
            coordinate space as the source polygon
         */
        PolyPolygonSharedPtr    createPolyPolygon( const CanvasSharedPtr&, const ::basegfx::B2DPolygon& rPoly ) const;
        PolyPolygonSharedPtr    createPolyPolygon( const CanvasSharedPtr&, const ::basegfx::B2DPolyPolygon& rPoly ) const;

        /** Create an uninitialized bitmap with the given size
         */
        BitmapSharedPtr         createBitmap( const CanvasSharedPtr&, const ::basegfx::B2ISize& rSize ) const;

        /** Create an uninitialized alpha bitmap with the given size
         */
        BitmapSharedPtr         createAlphaBitmap( const CanvasSharedPtr&, const ::basegfx::B2ISize& rSize ) const;

        /** Create a text portion with the given content string
         */
        TextSharedPtr           createText( const CanvasSharedPtr&, const ::rtl::OUString& ) const;

    private:
        friend struct InitInstance2;

        // singleton
        BaseGfxFactory();
        ~BaseGfxFactory();

        // default: disabled copy/assignment
        BaseGfxFactory(const BaseGfxFactory&);
        BaseGfxFactory& operator=( const BaseGfxFactory& );
    };

}

#endif /* _CPPCANVAS_BASEGFXFACTORY_HXX */
