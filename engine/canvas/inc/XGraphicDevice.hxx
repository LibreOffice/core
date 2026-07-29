/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
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
#pragma once

#include <sal/config.h>

#include <com/sun/star/geometry/IntegerSize2D.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/geometry/IntegerSize2D.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>
#include <com/sun/star/rendering/XLinePolyPolygon2D.hpp>
#include <com/sun/star/rendering/XParametricPolyPolygon2D.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <cpo/uno/Sequence.hxx>
#include <cpo/uno/Type.hxx>
#include <cppu/unotype.hxx>
#include <cppu/macros.hxx>
#include <rtl/ustring.hxx>

#include <type_traits>

namespace cpo::uno
{
class Type;
}

namespace vclcanvas
{
/* TODO: There's obviously a concept called window missing here, where
   methods such as bufferController, fullscreen mode etc . belong
   to. But see below
 */

/** This interface provides access to a graphic device, such as a
    printer, or a screen device. Every canvas (@see XCanvas) has
    exactly one associated graphic device, into which its output is
    rendered.

    For a typical windowing system, the graphic device is equivalent
    to a distinct OS window, with its own clipped output area,
    fullscreen and double-buffering attributes. That is, even if one
    can have multiple canvases per system window, they all share the
    same graphic device and thus e.g. fullscreen state. If the OS
    restrictions are in such a way that fullscreen or double-buffering
    is screen-exclusive, i.e. that per screen, only one object can
    have this state, it might even be that all windows on the screen
    share a common graphic device.
 */
class XGraphicDevice : public ::css::uno::XInterface
{
public:
    XGraphicDevice() = default;
    XGraphicDevice(XGraphicDevice const&) = default;
    XGraphicDevice(XGraphicDevice&&) = default;
    XGraphicDevice& operator=(XGraphicDevice const&) = default;
    XGraphicDevice& operator=(XGraphicDevice&&) = default;

    /** Create a line poly-polygon which can internally use
        device-optimized representations already.

        @param points
        The points of the poly-polygon, in a separate array for every polygon.
     */
    virtual ::css::uno::Reference<::css::rendering::XLinePolyPolygon2D>
    createCompatibleLinePolyPolygon(
        const ::cpo::uno::Sequence<::cpo::uno::Sequence<::css::geometry::RealPoint2D>>& points)
        = 0;

    /** Create a bitmap with alpha channel whose memory layout and
        sample model is compatible to the graphic device.

        @param size
        Size of the requested bitmap in pixel. Both components of the
        size must be greater than 0
     */
    virtual ::css::uno::Reference<::css::rendering::XBitmap>
    createCompatibleAlphaBitmap(const ::css::geometry::IntegerSize2D& size) = 0;

    /** Create a parametric polygon.

        @return a parametric polygon.
        Although it is possible to use parametric polygons on
        all canvases, regardless of the associated graphic device,
        this is not advisable: each canvas implementation is free to
        internally generate optimized parametric polygons, which can
        be used more directly for e.g. texturing operations.

        Available services (all canvas implementations should provide
        this minimal set, though are free to add more; just check the
        getAvailableServiceNames() on the returned interface):

        - Gradients - all gradients need to support two construction
          parameters, "Colors" being a `sequence&lt; Color &gt;`
          and "Stops" being a `sequence&lt; double &gt;`. Both must
          have the same length, and at least two elements. See
          http://www.w3.org/TR/SVG11/pservers.html#GradientStops for
          the semantics of gradient stops and colors.
          Required gradient services:

          - "LinearGradient" - the gradient varies linearly between
            the given colors. without coordinate system
            transformation, the color interpolation happens in
            increasing x direction, and is constant in y
            direction. Equivalent to svg linear gradient
            http://www.w3.org/TR/SVG11/pservers.html#LinearGradients

          - "EllipticalGradient" - this gradient has zeroth color
            index in the middle, and varies linearly between center
            and final color. The services takes an additional
            parameter named "AspectRatio" of double
            (width divided by height), if this aspect ratio is 1, the
            gradient is circular. If it's not 1, the gradient is
            elliptical, with the special twist that the aspect ratio
            is maintained also for the center color: the gradient will
            not collapse into a single point, but become a line of
            center color. If "AspectRatio" is missing, or equal to 1,
            this gradient yields similar results as the svg radial
            gradient
            http://www.w3.org/TR/SVG11/pservers.html#RadialGradients

          - "RectangularGradient" - this gradient has zeroth color
            index in the middle, and varies linearly between center
            and final color via rectangular boxes
            around the center point. The services takes an additional
            parameter named "AspectRatio" of double
            (width divided by height), if this aspect ratio is 1, the
            gradient is quadratic. If it's not 1, the gradient is
            rectangular, with the special twist that the aspect ratio
            is maintained also for the center color: the gradient will
            not collapse into a single point, but become a line of
            center color.
     */
    virtual ::css::uno::Reference<::css::rendering::XParametricPolyPolygon2D>
    createParametricPolyPolygon(const ::rtl::OUString& GradientService,
                                const ::cpo::uno::Sequence<::cpo::uno::Sequence<double>>& colors,
                                const ::cpo::uno::Sequence<double>& stops, double aspectRatio)
        = 0;

    static inline ::cpo::uno::Type const& static_type(void* = nullptr);

protected:
    ~XGraphicDevice() noexcept {} // avoid warnings about virtual members and non-virtual dtor
};

inline ::cpo::uno::Type const&
cppu_detail_getUnoType(SAL_UNUSED_PARAMETER ::vclcanvas::XGraphicDevice const*)
{
    static typelib_TypeDescriptionReference* the_type = nullptr;
    if (!the_type)
    {
        typelib_static_type_init(&the_type, typelib_TypeClass_INTERFACE,
                                 "vclcanvas.XGraphicDevice");
    }
    return *reinterpret_cast<::cpo::uno::Type*>(&the_type);
}
}

::cpo::uno::Type const& ::vclcanvas::XGraphicDevice::static_type(SAL_UNUSED_PARAMETER void*)
{
    return ::cppu::UnoType<::vclcanvas::XGraphicDevice>::get();
}

namespace cppu::detail
{
template <> struct IsUnoInterfaceType<::vclcanvas::XGraphicDevice> : ::std::true_type
{
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
