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

#ifndef INCLUDED_CPPCANVAS_SOURCE_WRAPPER_IMPLPOLYPOLYGON_HXX
#define INCLUDED_CPPCANVAS_SOURCE_WRAPPER_IMPLPOLYPOLYGON_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/StrokeAttributes.hpp>

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
                             const css::uno::Reference<
                                 css::rendering::XPolyPolygon2D >& rPolyPoly );

            virtual ~ImplPolyPolygon();

            virtual void            setRGBAFillColor( Color::IntSRGBA ) override;
            virtual void            setRGBALineColor( Color::IntSRGBA ) override;
            virtual Color::IntSRGBA getRGBALineColor() const override;

            virtual void            setStrokeWidth( const double& rStrokeWidth ) override;
            virtual double          getStrokeWidth() const override;

            virtual bool            draw() const override;

            virtual css::uno::Reference<
                css::rendering::XPolyPolygon2D > getUNOPolyPolygon() const override;

        private:
            ImplPolyPolygon(const ImplPolyPolygon&) = delete;
            ImplPolyPolygon& operator= ( const ImplPolyPolygon& ) = delete;

            const css::uno::Reference< css::rendering::XPolyPolygon2D > mxPolyPoly;

            css::rendering::StrokeAttributes                            maStrokeAttributes;

            css::uno::Sequence< double >                                maFillColor;
            css::uno::Sequence< double >                                maStrokeColor;
            bool                                                        mbFillColorSet;
            bool                                                        mbStrokeColorSet;
        };

    }
}

#endif // INCLUDED_CPPCANVAS_SOURCE_WRAPPER_IMPLPOLYPOLYGON_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
