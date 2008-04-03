/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterGeometryHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 15:58:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
