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

#ifndef INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERPANEBORDERPAINTER_HXX
#define INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERPANEBORDERPAINTER_HXX

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <memory>

namespace sdext::presenter {

class PresenterTheme;

/** See PresenterPaneBorderPainter and its addBorder() and removeBorder() methods
    for an explanation of the border type and its values.
*/
enum class BorderType
{
    INNER,
    OUTER,
    TOTAL
};

typedef ::cppu::WeakComponentImplHelper<> PresenterPaneBorderPainterInterfaceBase;

/** Paint the border around a rectangular region, typically a pane.

    <p>Calling objects have to be able to derive inner bounding boxes of the
    border from the outer ones and inner ones from outer ones.  This
    conversion and the painting of the border involves three rectangles.
    The inner and outer bounding box of the border.  This is a logical
    bounding box which the paint methods may paint over.  The center box is
    the third rectangle.  This is the actual border between outer and inner
    background color or bitmap and it is used for placing the bitmaps that are used
    paint the border.  The inner sides and corners are places relative to
    this center box, i.e. when not further offsets are given then the upper
    left corner bitmap is painted with its lower right at the upper left of
    the center box.</p>
*/
class PresenterPaneBorderPainter
    : protected ::cppu::BaseMutex,
      public PresenterPaneBorderPainterInterfaceBase
{
public:
    explicit PresenterPaneBorderPainter (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~PresenterPaneBorderPainter() override;
    PresenterPaneBorderPainter(const PresenterPaneBorderPainter&) = delete;
    PresenterPaneBorderPainter& operator=(const PresenterPaneBorderPainter&) = delete;

    /** Enlarge the given rectangle by the size of the specified part of the
        border.  This method can be used to convert an inner bounding box
        into the center box or the outer bounding box.
        @param sPaneBorderStyleName
            The pane style defines the sizes of the border.
        @param aRectangle
            This rectangle will be converted into a larger one.  This should
            be the center box or the inner bounding box of the border.
        @param eBorderType
            The part of the border to add to the given rectangle.
            Use INNER_BORDER to convert an inner bounding box into the
            center box or TOTAL_BORDER to convert it into the outer bounding
            box.  OUTER_BORDER can be used to convert the center box into
            the outer bounding box.
    */
    css::awt::Rectangle AddBorder (
        const OUString& rsPaneURL,
        const css::awt::Rectangle& rInnerBox,
        const BorderType eBorderType) const;

    /** Shrink the given rectangle by the size of the specified part of the
        border.  This method can be used to convert an outer bounding box
        into the center box or the inner bounding box.
        @param sPaneBorderStyleName
            The pane style defines the sizes of the border.
        @param aRectangle
            This rectangle will be converted into a smaller one that lies
            inside it.  It should be the center box or the outer bounding
            box of the border.
        @param eBorderType
            The part of the border to remove from the given rectangle.
            Use OUTER_BORDER to convert an outer bounding box into the
            center box or TOTAL_BORDER to convert it into the inner bounding
            box.  INNER_BORDER can be used to convert the center box into
            the inner bounding box.
    */
    css::awt::Rectangle RemoveBorder (
        const OUString& rsPaneURL,
        const css::awt::Rectangle& rOuterBox,
        const BorderType eBorderType) const;

    void SetTheme (const std::shared_ptr<PresenterTheme>& rpTheme);

    class Renderer;

    // XPaneBorderPainter

    css::awt::Rectangle addBorder (
        const OUString& rsPaneBorderStyleName,
        const css::awt::Rectangle& rRectangle,
        BorderType eBorderType);

    css::awt::Rectangle removeBorder (
        const OUString& rsPaneBorderStyleName,
        const css::awt::Rectangle& rRectangle,
        BorderType eBorderType);

    /** Paint the border around a pane.
        @param sPaneBorderStyleName
            The pane style to use for painting the border.
        @param xCanvas
            The canvas onto which the border is painted.
        @param aOuterBorderRectangle
            The outer bounding box of the border.  Use addBorder to convert
            the bounding box of a pane (the inner bounding box of the
            border) into this outer bounding box of the border.
        @param aRepaintArea
            The area in which the border has to be repainted.  The clip
            rectangle.
        @param sTitle
            The pane title.  Supply an empty string for panes without
            title.  It is the responsibility of the caller to supply a title
            only for pane border styles that support a title.
    */
    void paintBorder (
        const OUString& rsPaneBorderStyleName,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const css::awt::Rectangle& rOuterBorderRectangle,
        const css::awt::Rectangle& rRepaintArea,
        const OUString& rsTitle);

    /** Paint the border around a pane where the border includes a call out
        that is anchored at the given point.  Most arguments have the same
        meaning as in the paintBorder().

        @see paintBorder

        @param sPaneBorderStyleName
            See description in #paintBorder.
        @param xCanvas
            See description in #paintBorder.
        @param aOuterBorderRectangle
            See description in #paintBorder.
        @param aRepaintArea
            See description in #paintBorder.
        @param sTitle
            See description in #paintBorder.
        @param aCalloutAnchor
            The anchor point of the call out.  It is usually located outside
            the border.
    */
    void paintBorderWithCallout (
        const OUString& rsPaneBorderStyleName,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const css::awt::Rectangle& rOuterBorderRectangle,
        const css::awt::Rectangle& rRepaintArea,
        const OUString& rsTitle,
        const css::awt::Point& rCalloutAnchor);

    /** Return the offset of a call out anchor with respect to the outer
        border.  This value is used when the call out is realized by a fixed
        bitmap in order to determine the size and/or location of the outer
        border for a given call out.
    */
    css::awt::Point getCalloutOffset (
        const OUString& rsPaneBorderStyleName);

private:
    css::uno::Reference<css::uno::XComponentContext> mxContext;
    std::shared_ptr<PresenterTheme> mpTheme;
    std::unique_ptr<Renderer> mpRenderer;

    /** When the theme for the border has not yet been loaded then try again
        when this method is called.
        @return
            Returns <TRUE/> only one time when the theme is loaded and/or the
            renderer is initialized.
    */
    bool ProvideTheme (
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas);
    void ProvideTheme();

    /// @throws css::lang::DisposedException
    void ThrowIfDisposed() const;
};

} // end of namespace ::sd::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
