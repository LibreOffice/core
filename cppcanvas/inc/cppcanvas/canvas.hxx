/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: canvas.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:38:25 $
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

#ifndef _CPPCANVAS_CANVAS_HXX
#define _CPPCANVAS_CANVAS_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#ifndef _CPPCANVAS_FONT_HXX
#include <cppcanvas/font.hxx>
#endif
#ifndef _CPPCANVAS_COLOR_HXX
#include <cppcanvas/color.hxx>
#endif

namespace rtl
{
    class OUString;
}

namespace basegfx
{
    class B2DHomMatrix;
    class B2DPolyPolygon;
}

namespace com { namespace sun { namespace star { namespace rendering
{
    class  XCanvas;
    struct ViewState;
} } } }


/* Definition of BitmapCanvas */

namespace cppcanvas
{
    class PolyPolygon;
    class Canvas;

    // forward declaration, since PolyPolygon also references Canvas
    typedef ::boost::shared_ptr< PolyPolygon > PolyPolygonSharedPtr;

    // forward declaration, since cloneCanvas() also references Canvas
    typedef ::boost::shared_ptr< Canvas > CanvasSharedPtr;

    /** Canvas interface
     */
    class Canvas
    {
    public:
        enum
        {
            /** Extra pixel used when canvas anti-aliases.

                Enlarge the bounding box of drawing primitives by this
                amount in both dimensions, and on both sides of the
                bounds, to account for extra pixel touched outside the
                actual primitive bounding box, when the canvas
                performs anti-aliasing.
             */
            ANTIALIASING_EXTRA_SIZE=2
        };

        virtual ~Canvas() {}

        virtual void                        setTransformation( const ::basegfx::B2DHomMatrix& rMatrix ) = 0;
        virtual ::basegfx::B2DHomMatrix     getTransformation() const = 0;

        virtual void                        setClip( const ::basegfx::B2DPolyPolygon& rClipPoly ) = 0;
        virtual ::basegfx::B2DPolyPolygon   getClip() const = 0;

        virtual FontSharedPtr               createFont( const ::rtl::OUString& rFontName, const double& rCellSize ) const = 0;

        virtual ColorSharedPtr              createColor() const = 0;

        virtual CanvasSharedPtr             clone() const = 0;

        // this should be considered private. if RTTI gets enabled
        // someday, remove that to a separate interface
        virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::rendering::XCanvas >          getUNOCanvas() const = 0;
        virtual ::com::sun::star::rendering::ViewState      getViewState() const = 0;
    };

}

#endif /* _CPPCANVAS_CANVAS_HXX */
