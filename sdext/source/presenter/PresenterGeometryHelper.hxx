/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PresenterGeometryHelper.hxx,v $
 *
 * $Revision: 1.3 $
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
#include <vector>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

/** Collection of geometry related convenience functions.
*/
class PresenterGeometryHelper
{
public:
    static css::awt::Rectangle TranslateRectangle (
        const css::awt::Rectangle& rBox,
        const sal_Int32 nXOffset,
        const sal_Int32 nYOffset);

    static css::awt::Rectangle Intersection (
        const css::awt::Rectangle& rBox1,
        const css::awt::Rectangle& rBox2);

    static bool IsInside (
        const css::awt::Rectangle& rBox,
        const css::awt::Point& rPoint);

    static bool IsInside (
        const css::geometry::RealRectangle2D& rBox,
        const css::geometry::RealPoint2D& rPoint);

    static css::awt::Rectangle Union (
        const css::awt::Rectangle& rBox1,
        const css::awt::Rectangle& rBox2);

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
};

} }

#endif
