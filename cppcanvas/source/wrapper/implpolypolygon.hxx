/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: implpolypolygon.hxx,v $
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

#ifndef _CANVAS_IMPLPOLYPOLYGON_HXX
#define _CANVAS_IMPLPOLYPOLYGON_HXX

#include <com/sun/star/uno/Reference.hxx>
#ifndef _COM_SUN_STAR_RENDERING_XPOLYPOLYGON2D_HPP__
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XGRAPHICDEVICE_HPP__
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_STROKEATTRIBUTES_HPP__
#include <com/sun/star/rendering/StrokeAttributes.hpp>
#endif

#include <cppcanvas/polypolygon.hxx>
#include <canvasgraphichelper.hxx>

namespace com { namespace sun { namespace star { namespace rendering
{
    struct RealPoint2D;
} } } }


namespace cppcanvas
{
    namespace internal
    {

        class ImplPolyPolygon : public virtual ::cppcanvas::PolyPolygon, protected CanvasGraphicHelper
        {
        public:
            ImplPolyPolygon( const CanvasSharedPtr& rParentCanvas,
                             const ::com::sun::star::uno::Reference<
                                 ::com::sun::star::rendering::XPolyPolygon2D >& rPolyPoly );

            virtual ~ImplPolyPolygon();

            virtual void            addPolygon( const ::basegfx::B2DPolygon& rPoly );
            virtual void            addPolyPolygon( const ::basegfx::B2DPolyPolygon& rPoly );

            virtual void            setRGBAFillColor( Color::IntSRGBA );
            virtual void            setRGBALineColor( Color::IntSRGBA );
            virtual Color::IntSRGBA getRGBAFillColor() const;
            virtual Color::IntSRGBA getRGBALineColor() const;

            virtual void            setStrokeWidth( const double& rStrokeWidth );
            virtual double          getStrokeWidth() const;

            virtual bool            draw() const;

            virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::rendering::XPolyPolygon2D > getUNOPolyPolygon() const;

        private:
            // default: disabled copy/assignment
            ImplPolyPolygon(const ImplPolyPolygon&);
            ImplPolyPolygon& operator= ( const ImplPolyPolygon& );

            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >   mxPolyPoly;

            ::com::sun::star::rendering::StrokeAttributes                                           maStrokeAttributes;

            ::com::sun::star::uno::Sequence< double >                                                       maFillColor;
            ::com::sun::star::uno::Sequence< double >                                                       maStrokeColor;
            bool                                                                                            mbFillColorSet;
            bool                                                                                            mbStrokeColorSet;
        };

    }
}

#endif /* _CANVAS_IMPLPOLYPOLYGON_HXX */
