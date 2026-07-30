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

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/geometry/Matrix2D.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/rendering/FontRequest.hpp>
#include <com/sun/star/rendering/RenderState.hpp>
#include <com/sun/star/rendering/StringContext.hpp>
#include <com/sun/star/rendering/StrokeAttributes.hpp>
#include <com/sun/star/rendering/Texture.hpp>
#include <com/sun/star/rendering/ViewState.hpp>

namespace com::sun::star::rendering
{
class XBitmap;
class XCanvasFont;
class XPolyPolygon2D;
class XTextLayout;
}
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Reference.h>
#include <cpo/uno/Sequence.h>
#include <cppu/macros.hxx>
#include <sal/types.h>

namespace cpo::uno
{
class Type;
}

namespace vclcanvas
{
class XCachedPrimitive;
class XGraphicDevice;
}

/** Central interface for rendering.<p>

    This is the central interface for graphical output production, and
    the place where all draw methods are located.<p>

    Some notes are in order to explain the concepts used here. The
    XCanvas interface is free of client-modifiable state,
    i.e. it can be used safely and without external synchronization in
    a multi-threaded environment. On the other hand, this implies that
    for nearly every canvas operation, external state is
    required. This is provided by ViewState and
    RenderState in a unified fashion, supplemented by a
    few extra state parameters for some methods (e.g. textured
    polygons or text rendering).<p>

    When used careless, this scheme can be inefficient to some extend,
    because internally, view, render and other states have to be
    combined before rendering. This is especially expensive for
    complex clip polygons, i.e. when both ViewState and
    RenderState have a complex clip polygon set, which
    have to be intersected before rendering. It is therefore
    recommended to combine ViewState and
    RenderState already at the client side, when objects
    are organized in a hierarchical way: the classic example are
    grouped draw shapes, whose parent group object imposes a
    common clipping and a common transformation on its siblings. The
    group object would therefore merge the ViewState and
    the RenderState it is called with into a new
    ViewState, and call its siblings with a
    RenderState containing only the local offset (and no
    extra clipping).<p>

    Further on, this stateless nature provides easy ways for
    caching. Every non-trivial operation on XCanvas can
    return a cache object, which, when called to redraw, renders the
    primitive usually much more quickly than the original method. Note
    that such caching is a lot more complicated, should the actual
    rendering a method yields depend on internal state (which is the
    case e.g. for the
    ::com::sun::star::awt::XGraphics
    interface).  Please note, though, that deciding whether to return
    an XCachedPrimitive is completely up to the
    implementation - don't rely on the methods returning something
    (this is because there might be cases when returning such a cache
    object will actually be a pessimization, since it involves memory
    allocation and comparisons).<p>

    Things that need more than a small, fixed amount of data are
    encapsulated in own interfaces, e.g. polygons and bitmaps. You
    can, in principle, roll your own implementations of these
    interfaces, wrap it around your internal representation of
    polygons and bitmaps, and render them. It might just not be overly
    fast, because the XCanvas would need to convert for
    each render call. It is therefore recommended to create such
    objects via the XGraphicDevice factory (to be
    retrieved from every canvas object via the
    getDevice() call) - they will then internally
    optimize to the underlying graphics subsystem.<p>
 */
namespace vclcanvas
{
class XCanvas : public ::css::uno::XInterface
{
public:
    XCanvas() = default;
    XCanvas(XCanvas const&) = default;
    XCanvas(XCanvas&&) = default;
    XCanvas& operator=(XCanvas const&) = default;
    XCanvas& operator=(XCanvas&&) = default;

    /** Clear the whole canvas area.<p>

        This method clears the whole canvas area to the device default
        color (e.g. white for a printer).
     */
    virtual void clear() = 0;

    /** Draw a point in device resolution on the device.

        @param aPoint
        The point to draw.

        @param aViewState
        The view state to be used when drawing this point.

        @param aRenderState
        The render state to be used when drawing this point.

        @throws com::sun::star::lang::IllegalArgumentException
        if one of the view and render state parameters are outside the
        specified range.
     */
    virtual void drawPoint(const ::css::geometry::RealPoint2D& aPoint,
                           const ::css::rendering::ViewState& aViewState,
                           const ::css::rendering::RenderState& aRenderState)
        = 0;

    /** Draw a line in device resolution width (i.e. one device pixel
        wide).

        @param aStartPoint
        The start point of the line to draw.

        @param aEndPoint
        The end point of the line to draw.

        @param aViewState
        The view state to be used when drawing this line.

        @param aRenderState
        The render state to be used when drawing this line.

        @throws com::sun::star::lang::IllegalArgumentException
        if one of the view and render state parameters are outside the
        specified range.
     */
    virtual void drawLine(const ::css::geometry::RealPoint2D& aStartPoint,
                          const ::css::geometry::RealPoint2D& aEndPoint,
                          const ::css::rendering::ViewState& aViewState,
                          const ::css::rendering::RenderState& aRenderState)
        = 0;

    /** Draw a poly-polygon in device resolution line width (i.e. the
        lines are one device pixel wide).

        @param xPolyPolygon
        The poly-polygon to draw.

        @param aViewState
        The view state to be used when drawing this polygon.

        @param aRenderState
        The render state to be used when drawing this polygon.

        @throws com::sun::star::lang::IllegalArgumentException
        if one of the view and render state parameters are outside the
        specified range.
     */
    virtual void
    drawPolyPolygon(const ::css::uno::Reference<::css::rendering::XPolyPolygon2D>& xPolyPolygon,
                    const ::css::rendering::ViewState& aViewState,
                    const ::css::rendering::RenderState& aRenderState)
        = 0;

    /** Stroke each polygon of the provided poly-polygon with the
        specified stroke attributes.<p>

        This method considers the stroking of all polygons as an
        atomic operation in relation to the RenderState's
        CompositeOperationy operation. That means,
        overlapping strokes from distinct polygons will look exactly
        as overlapping segments of the same polygon, even with
        transparency.<p>

        @param xPolyPolygon
        The poly-polygon to render.

        @param aViewState
        The view state to be used when stroking this polygon.

        @param aRenderState
        The render state to be used when stroking this polygon.

        @param aStrokeAttributes
        Further attributes used to parameterize the stroking.

        @throws com::sun::star::lang::IllegalArgumentException
        if one of the view and render state parameters are outside the
        specified range.
     */
    virtual void
    strokePolyPolygon(const ::css::uno::Reference<::css::rendering::XPolyPolygon2D>& xPolyPolygon,
                      const ::css::rendering::ViewState& aViewState,
                      const ::css::rendering::RenderState& aRenderState,
                      const ::css::rendering::StrokeAttributes& aStrokeAttributes)
        = 0;

    /** Fill the given poly-polygon.<p>

        This method fills the given poly-polygon according to the
        RenderState's color and the poly-polygon's fill
        rule.<p>

        @param xPolyPolygon
        The poly-polygon to render.

        @param aViewState
        The view state to be used when filling this polygon.

        @param aRenderState
        The render state to be used when filling this polygon.

        @return a handle to the cached rendering output.

        @throws com::sun::star::lang::IllegalArgumentException
        if one of the view and render state parameters are outside the
        specified range.
     */
    virtual ::css::uno::Reference<::vclcanvas::XCachedPrimitive>
    fillPolyPolygon(const ::css::uno::Reference<::css::rendering::XPolyPolygon2D>& xPolyPolygon,
                    const ::css::rendering::ViewState& aViewState,
                    const ::css::rendering::RenderState& aRenderState)
        = 0;

    /** Fill the given poly-polygon with a texture.<p>

        This method fills the given poly-polygon according to the
        RenderState's color, the given textures and
        poly-polygon's fill rule.<p>

        @param xPolyPolygon
        The poly-polygon to render.

        @param aViewState
        The view state to be used when filling this polygon.

        @param aRenderState
        The render state to be used when filling this polygon.

        @param xTextures
        A sequence of texture definitions, with which to fill the
        polygonal area.

        @return a handle to the cached rendering output.

        @throws com::sun::star::lang::IllegalArgumentException
        if one of the view and render state parameters are outside the
        specified range.
    */
    virtual ::css::uno::Reference<::vclcanvas::XCachedPrimitive> fillTexturedPolyPolygon(
        const ::css::uno::Reference<::css::rendering::XPolyPolygon2D>& xPolyPolygon,
        const ::css::rendering::ViewState& aViewState,
        const ::css::rendering::RenderState& aRenderState,
        const ::cpo::uno::Sequence<::css::rendering::Texture>& xTextures)
        = 0;

    /** Create a suitable font for the specified font description.

        @param aFontRequest

        @param aExtraFontProperties
        Additional font properties to be applied when selecting this
        font. Normally, you should not need this parameter. Currently,
        the following property is recognized:

        -   EmphasisMark: long integer that represents the emphasis mark.
            @see ::com::sun::star::awt::FontEmphasisMark

        @param aFontMatrix
        Font-specific transformation matrix, which affects both the
        glyphs as well as the advancement.

        @returns the requested font, or an invalid reference, if the
        request failed.

        @throws com::sun::star::lang::IllegalArgumentException
        if one of the parameters is not within the allowed range.
     */
    virtual ::css::uno::Reference<::css::rendering::XCanvasFont>
    createFont(const ::css::rendering::FontRequest& aFontRequest,
               const ::cpo::uno::Sequence<::css::beans::PropertyValue>& aExtraFontProperties,
               const ::css::geometry::Matrix2D& aFontMatrix)
        = 0;

    /** Draw the text given by the substring of the specified string
        with the given font.<p>

        The local origin of this output operation is either the left
        end of the text baseline, for textDirection equal
        LEFT_TO_RIGHT, or the right end of the baseline, for
        textDirection equal to RIGHT_TO_LEFT, respectively.<p>

        @param aText
        The text to output.

        @param xFont
        The font retrieved from this canvas to be used when drawing
        the text.

        @param aViewState
        The view state to be used when drawing this text.

        @param aRenderState
        The render state to be used when drawing this text.

        @param nTextDirection
        A value from the TextDirection collection,
        denoting the main writing direction for this string. The main
        writing direction determines the origin of the text output,
        i.e. the left edge for left-to-right and the right edge for
        right-to-left text.

        @throws com::sun::star::lang::IllegalArgumentException
        if one of the view and render state parameters are outside the
        specified range.
     */
    virtual void drawText(const ::css::rendering::StringContext& aText,
                          const ::css::uno::Reference<::css::rendering::XCanvasFont>& xFont,
                          const ::css::rendering::ViewState& aViewState,
                          const ::css::rendering::RenderState& aRenderState,
                          ::sal_Int8 nTextDirection)
        = 0;

    /** Draw the formatted text given by the text layout.<p>

        The glyphs as represented by the text layout are always output
        with the reference position being the leftmost edge of the
        layout object's baseline. If the layout contains more than one
        baseline, the baseline of the first strong character in
        logical order is used here (strong in this context means that
        the character can be unambiguously assigned to a Unicode
        script).<p>

        @param xLayoutetText
        An interface to the readily layouted text, obtained from a
        XCanvasFont created at this canvas. The text
        layout already carries intrinsic font information.

        @param aViewState
        The view state to be used when drawing this text.

        @param aRenderState
        The render state to be used when drawing this text.

        @throws com::sun::star::lang::IllegalArgumentException
        if one of the view and render state parameters are outside the
        specified range.
     */
    virtual void
    drawTextLayout(const ::css::uno::Reference<::css::rendering::XTextLayout>& xLayoutetText,
                   const ::css::rendering::ViewState& aViewState,
                   const ::css::rendering::RenderState& aRenderState)
        = 0;

    /** Render the given bitmap.<p>

        This method renders the bitmap, at a position and shape as
        specified by the combined view and render transformations. For
        fast render speed, the bitmap should be created by the
        corresponding XGraphicDevice's
        XGraphicDevice::createCompatibleBitmap()
        method.<p>

        @param xBitmap
        The bitmap to render.

        @param aViewState
        The view state to be used when drawing this text.

        @param aRenderState
        The render state to be used when drawing this text.

        @return a handle to the cached rendering output.

        @throws com::sun::star::lang::IllegalArgumentException
        if one of the view and render state parameters are outside the
        specified range.
     */
    virtual ::css::uno::Reference<::vclcanvas::XCachedPrimitive>
    drawBitmap(const ::css::uno::Reference<::css::rendering::XBitmap>& xBitmap,
               const ::css::rendering::ViewState& aViewState,
               const ::css::rendering::RenderState& aRenderState)
        = 0;

    /** Request the associated graphic device for this canvas.<p>

        A graphic device provides methods specific to the underlying
        output device capabilities, which are common for all canvases
        rendering to such a device. This includes device resolution,
        color space, or bitmap formats.

        @return the associated XGraphicDevice.
     */
    virtual ::css::uno::Reference<::vclcanvas::XGraphicDevice> getDevice() = 0;

    static inline ::cpo::uno::Type const& static_type(void* = nullptr);

protected:
    ~XCanvas() noexcept {} // avoid warnings about virtual members and non-virtual dtor
};

inline ::cpo::uno::Type const&
cppu_detail_getUnoType(SAL_UNUSED_PARAMETER ::vclcanvas::XCanvas const*)
{
    static typelib_TypeDescriptionReference* the_type = nullptr;
    if (!the_type)
    {
        typelib_static_type_init(&the_type, typelib_TypeClass_INTERFACE, "vclcanvas.XCanvas");
    }
    return *reinterpret_cast<::cpo::uno::Type*>(&the_type);
}
}

::cpo::uno::Type const& ::vclcanvas::XCanvas::static_type(SAL_UNUSED_PARAMETER void*)
{
    return ::cppu::UnoType<::vclcanvas::XCanvas>::get();
}

namespace cppu::detail
{
template <> struct IsUnoInterfaceType<::vclcanvas::XCanvas> : ::std::true_type
{
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
