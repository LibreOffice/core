/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: polypolygon.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _CPPCANVAS_POLYPOLYGON_HXX
#define _CPPCANVAS_POLYPOLYGON_HXX

#include <com/sun/star/uno/Reference.hxx>

#include <boost/shared_ptr.hpp>
#include <cppcanvas/canvasgraphic.hxx>

namespace basegfx
{
    class B2DPolygon;
    class B2DPolyPolygon;
}

namespace com { namespace sun { namespace star { namespace rendering
{
    class  XPolyPolygon2D;
} } } }


/* Definition of PolyPolygon interface */

namespace cppcanvas
{

    /** This interface defines a PolyPolygon canvas object

        Consider this object part of the view, and not of the model
        data. Although the given polygon is typically copied and held
        internally (to facilitate migration to incompatible canvases),
        ::basegfx::B2DPolygon et al. are ref-counted copy-on-write
        classes, i.e. memory shouldn't be wasted. On the other hand,
        the API polygon created internally _does_ necessarily
        duplicate the data held, but can be easily flushed away via
        flush().
     */
    class PolyPolygon : public virtual CanvasGraphic
    {
    public:
        virtual void addPolygon( const ::basegfx::B2DPolygon& rPoly ) = 0;
        virtual void addPolyPolygon( const ::basegfx::B2DPolyPolygon& rPoly ) = 0;

        /** Set polygon fill color
         */
        virtual void                        setRGBAFillColor( Color::IntSRGBA ) = 0;
        /** Set polygon line color
         */
        virtual void                        setRGBALineColor( Color::IntSRGBA ) = 0;
        /** Get polygon fill color
         */
        virtual Color::IntSRGBA             getRGBAFillColor() const = 0;
        /** Get polygon line color
         */
        virtual Color::IntSRGBA             getRGBALineColor() const = 0;

        virtual void                        setStrokeWidth( const double& rStrokeWidth ) = 0;
        virtual double                      getStrokeWidth() const = 0;

        virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::rendering::XPolyPolygon2D > getUNOPolyPolygon() const = 0;
    };

    typedef ::boost::shared_ptr< ::cppcanvas::PolyPolygon > PolyPolygonSharedPtr;
}

#endif /* _CPPCANVAS_POLYPOLYGON_HXX */
