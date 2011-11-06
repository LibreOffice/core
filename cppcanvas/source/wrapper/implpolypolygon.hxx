/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
