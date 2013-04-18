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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
