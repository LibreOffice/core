/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

    static css::awt::Rectangle Union (
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
