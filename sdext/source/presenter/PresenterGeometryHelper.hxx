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

#ifndef SDEXT_PRESENTER_PRESENTER_GEOMETRY_HELPER_HXX
#define SDEXT_PRESENTER_PRESENTER_GEOMETRY_HELPER_HXX

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#include <com/sun/star/geometry/RealRectangle2D.hpp>
#include <vector>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

/** Collection of geometry related convenience functions.
*/
class PresenterGeometryHelper
{
public:
    static sal_Int32 Round (const double nValue);
    static sal_Int32 Floor (const double nValue);
    static sal_Int32 Ceil (const double nValue);

    /** Return the bounding box with integer coordinates of the given
        rectangle.  Note that due to different rounding of the left/top and
        the right/bottom border the width of the resulting rectangle may
        differ for different positions but constant width and height.
    */
    static css::awt::Rectangle ConvertRectangle (
        const css::geometry::RealRectangle2D& rBox);

    /** Convert the given rectangle to integer coordinates so that width and
        height remain constant when only the position changes.
    */
    static css::awt::Rectangle ConvertRectangleWithConstantSize (
        const css::geometry::RealRectangle2D& rBox);

    static css::geometry::RealRectangle2D ConvertRectangle (
        const css::awt::Rectangle& rBox);

    //    static css::awt::Size ConvertSize (
    //        const css::geometry::RealSize2D& rSize);

    static css::awt::Rectangle TranslateRectangle (
        const css::awt::Rectangle& rBox,
        const sal_Int32 nXOffset,
        const sal_Int32 nYOffset);

    static css::awt::Rectangle Intersection (
        const css::awt::Rectangle& rBox1,
        const css::awt::Rectangle& rBox2);

    static css::geometry::RealRectangle2D Intersection (
        const css::geometry::RealRectangle2D& rBox1,
        const css::geometry::RealRectangle2D& rBox2);

    static bool IsInside (
        const css::geometry::RealRectangle2D& rBox,
        const css::geometry::RealPoint2D& rPoint);

    /** Return whether rBox1 is completly inside rBox2.
    */
    static bool IsInside (
        const css::awt::Rectangle& rBox1,
        const css::awt::Rectangle& rBox2);

    static css::geometry::RealRectangle2D Union (
        const css::geometry::RealRectangle2D& rBox1,
        const css::geometry::RealRectangle2D& rBox2);

    static bool AreRectanglesDisjoint (
        const css::awt::Rectangle& rBox1,
        const css::awt::Rectangle& rBox2);

    static css::uno::Reference<css::rendering::XPolyPolygon2D> CreatePolygon(
        const css::awt::Rectangle& rBox,
        const css::uno::Reference<css::rendering::XGraphicDevice>& rxDevice);

    static css::uno::Reference<css::rendering::XPolyPolygon2D> CreatePolygon(
        const css::geometry::RealRectangle2D& rBox,
        const css::uno::Reference<css::rendering::XGraphicDevice>& rxDevice);

    static css::uno::Reference<css::rendering::XPolyPolygon2D> CreatePolygon(
        const ::std::vector<css::awt::Rectangle>& rBoxes,
        const css::uno::Reference<css::rendering::XGraphicDevice>& rxDevice);

    /** Create a polygon for a rounded rectangle.
    */
    /*    static css::uno::Reference<css::rendering::XPolyPolygon2D> CreatePolygon(
        const css::awt::Rectangle& rBox,
        const double nRadius,
        const css::uno::Reference<css::rendering::XGraphicDevice>&
        rxDevice);
    */
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
