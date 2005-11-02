/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: canvasgraphic.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:38:39 $
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

#ifndef _CPPCANVAS_CANVASGRAPHIC_HXX
#define _CPPCANVAS_CANVASGRAPHIC_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#ifndef _CPPCANVAS_COLOR_HXX
#include <cppcanvas/color.hxx>
#endif
#ifndef _CPPCANVAS_CANVAS_HXX
#include <cppcanvas/canvas.hxx>
#endif

namespace basegfx
{
    class B2DHomMatrix;
    class B2DPolyPolygon;
}


/* Definition of CanvasGraphic interface */

namespace cppcanvas
{
    // forward declaration, since PolyPolygon also derives from CanvasGraphic
    typedef ::boost::shared_ptr< class PolyPolygon > PolyPolygonSharedPtr;


    /** This interface defines basic properties of
        objects that can be painted on a Canvas
     */
    class CanvasGraphic
    {
    public:

        /** These enums determine how the primitive color is combined
            with the background. When performing this calculations, it
            is assumed that all color values are premultiplied with
            the corresponding alpha values (if no alpha is specified,
            1.0 is assumed). Then, the following general compositing
            operation is performed:

            C = Ca * Fa + Cb * Fb

            where C is the result color, Ca and Cb are the input
            colors, premultiplied with alpha, and Fa and Fb are
            described for the different composite modes (wherein Aa
            and Ab denote source and destination alpha, respectively).
        */
        enum CompositeOp
        {
            /// Clear destination. Fa = Fb = 0.
            CLEAR,

            /// Copy source as-is to destination. Fa = 1, Fb = 0.
            SOURCE,

            /// Leave destination as-is.  Fa = 0, Fb = 1.
            DESTINATION,

            /// Copy source over destination. Fa = 1, Fb = 1-Aa.
            OVER,

            /// Copy source under destination. Fa = 1-Ab, Fb = 1.
            UNDER,

            /// Copy source to destination, but limited to where the destination is. Fa = Ab, Fb = 0.
            INSIDE,

            /// Leave destination as is, but only where source was. Fa = 0, Fb = Aa.
            INSIDE_REVERSE,

            /// Copy source to destination, but limited to where destination is not. Fa = 1-Ab, Fb = 0.
            OUTSIDE,

            /// Leave destination as is, but only where source has not been. Fa = 0, Fb = 1-Aa.
            OUTSIDE_REVERSE,

            /// Copy source over destination, but only where destination is. Keep destination. Fa = Ab, Fb = 1-Aa.
            ATOP,

            /// Copy destination over source, but only where source is. Keep source. Fa = 1-Ab, Fb = Aa.
            ATOP_REVERSE,

            /// Take only parts where either source or destination, but not both are. Fa = 1-Ab, Fb = 1-Aa.
            XOR,

            /** simply add contributions of both source and destination. The
                resulting color values are limited to the permissible color
                range, and clipped to the maximal value, if exceeded. Fa = 1, Fb = 1.
            */
            ADD,

            /// Fa = min(1,(1-Ab)/Aa), Fb = 1
            SATURATE
        };

        virtual ~CanvasGraphic() {}

        /** Set object transformation matrix
         */
        virtual void                        setTransformation( const ::basegfx::B2DHomMatrix& rMatrix ) = 0;
        /** Get object transformation matrix
         */
        virtual ::basegfx::B2DHomMatrix     getTransformation() const = 0;

        /** Set object clipping polygon
         */
        virtual void                        setClip( const ::basegfx::B2DPolyPolygon& rClipPoly ) = 0;
        /** Get object clipping polygon
         */
        virtual ::basegfx::B2DPolyPolygon   getClip() const = 0;

        /** Set object color
         */
        virtual void                        setRGBAColor( Color::IntSRGBA ) = 0;
        /** Get object color
         */
        virtual Color::IntSRGBA             getRGBAColor() const = 0;

        /** Set object composite mode
         */
        virtual void                        setCompositeOp( CompositeOp aOp ) = 0;
        /** Get object composite mode
         */
        virtual CompositeOp                 getCompositeOp() const = 0;

        /** Render to parent canvas

            This method renders the content to the parent canvas,
            i.e. the canvas this object was constructed for.

            @return whether the rendering finished successfully.
         */
        virtual bool draw() const = 0;

    };

    typedef ::boost::shared_ptr< ::cppcanvas::CanvasGraphic > CanvasGraphicSharedPtr;
}

#endif /* _CPPCANVAS_CANVASGRAPHIC_HXX */
