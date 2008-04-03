/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterGeometryHelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 15:58:34 $
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

#include "PresenterGeometryHelper.hxx"

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace {

sal_Int32 Right (const awt::Rectangle& rBox)
{
    return rBox.X + rBox.Width - 1;
}

sal_Int32 Bottom (const awt::Rectangle& rBox)
{
    return rBox.Y + rBox.Height - 1;
}

sal_Int32 Width (const sal_Int32 nLeft, const sal_Int32 nRight)
{
    return nRight - nLeft + 1;
}

sal_Int32 Height (const sal_Int32 nTop, const sal_Int32 nBottom)
{
    return nBottom - nTop + 1;
}

} // end of anonymous namespace



namespace sdext { namespace presenter {

awt::Rectangle PresenterGeometryHelper::TranslateRectangle (
    const css::awt::Rectangle& rBox,
    const sal_Int32 nXOffset,
    const sal_Int32 nYOffset)
{
    return awt::Rectangle(rBox.X + nXOffset, rBox.Y + nYOffset, rBox.Width, rBox.Height);
}




awt::Rectangle PresenterGeometryHelper::Intersection (
    const css::awt::Rectangle& rBox1,
    const css::awt::Rectangle& rBox2)
{
    const sal_Int32 nLeft (::std::max(rBox1.X, rBox2.X));
    const sal_Int32 nTop (::std::max(rBox1.Y, rBox2.Y));
    const sal_Int32 nRight (::std::min(Right(rBox1), Right(rBox2)));
    const sal_Int32 nBottom (::std::min(Bottom(rBox1), Bottom(rBox2)));
    if (nLeft >= nRight || nTop >= nBottom)
        return awt::Rectangle();
    else
        return awt::Rectangle(nLeft,nTop, Width(nLeft,nRight), Height(nTop,nBottom));
}




bool PresenterGeometryHelper::IsInside (
    const css::awt::Rectangle& rBox,
    const css::awt::Point& rPoint)
{
    return rBox.X <= rPoint.X
        && rBox.Y <= rPoint.Y
        && rBox.X+rBox.Width > rPoint.X
        && rBox.Y+rBox.Height > rPoint.Y;
}




bool PresenterGeometryHelper::IsInside (
    const css::geometry::RealRectangle2D& rBox,
    const css::geometry::RealPoint2D& rPoint)
{
    return rBox.X1 <= rPoint.X
        && rBox.Y1 <= rPoint.Y
        && rBox.X2 >= rPoint.X
        && rBox.Y2 >= rPoint.Y;
}




awt::Rectangle PresenterGeometryHelper::Union (
    const css::awt::Rectangle& rBox1,
    const css::awt::Rectangle& rBox2)
{
    if (rBox1.Width<=0 || rBox1.Height<=0)
        return rBox2;
    else if (rBox2.Width<=0 || rBox2.Height<=0)
        return rBox1;

    const sal_Int32 nLeft (::std::min(rBox1.X, rBox2.X));
    const sal_Int32 nTop (::std::min(rBox1.Y, rBox2.Y));
    const sal_Int32 nRight (::std::max(Right(rBox1), Right(rBox2)));
    const sal_Int32 nBottom (::std::max(Bottom(rBox1), Bottom(rBox2)));
    if (nLeft >= nRight || nTop >= nBottom)
        return awt::Rectangle();
    else
        return awt::Rectangle(nLeft,nTop, Width(nLeft,nRight), Height(nTop,nBottom));
}




bool PresenterGeometryHelper::AreRectanglesDisjoint (
    const css::awt::Rectangle& rBox1,
    const css::awt::Rectangle& rBox2)
{
    return rBox1.X+rBox1.Width <= rBox2.X
        || rBox1.Y+rBox1.Height <= rBox2.Y
        || rBox1.X >= rBox2.X+rBox2.Width
        || rBox1.Y >= rBox2.Y+rBox2.Height;
}




Reference<rendering::XPolyPolygon2D> PresenterGeometryHelper::CreatePolygon(
    const awt::Rectangle& rBox,
    const Reference<rendering::XGraphicDevice>& rxDevice)
{
    if ( ! rxDevice.is())
        return NULL;

    Sequence<Sequence<geometry::RealPoint2D> > aPoints(1);
    aPoints[0] = Sequence<geometry::RealPoint2D>(4);
    aPoints[0][0] = geometry::RealPoint2D(rBox.X, rBox.Y);
    aPoints[0][1] = geometry::RealPoint2D(rBox.X, rBox.Y+rBox.Height);
    aPoints[0][2] = geometry::RealPoint2D(rBox.X+rBox.Width, rBox.Y+rBox.Height);
    aPoints[0][3] = geometry::RealPoint2D(rBox.X+rBox.Width, rBox.Y);
    Reference<rendering::XPolyPolygon2D> xRectangle (
        rxDevice->createCompatibleLinePolyPolygon(aPoints),
        UNO_QUERY);
    if (xRectangle.is())
        xRectangle->setClosed(0, sal_True);

    return xRectangle;
}




Reference<rendering::XPolyPolygon2D> PresenterGeometryHelper::CreatePolygon(
    const geometry::RealRectangle2D& rBox,
    const Reference<rendering::XGraphicDevice>& rxDevice)
{
    if ( ! rxDevice.is())
        return NULL;

    Sequence<Sequence<geometry::RealPoint2D> > aPoints(1);
    aPoints[0] = Sequence<geometry::RealPoint2D>(4);
    aPoints[0][0] = geometry::RealPoint2D(rBox.X1, rBox.Y1);
    aPoints[0][1] = geometry::RealPoint2D(rBox.X1, rBox.Y2);
    aPoints[0][2] = geometry::RealPoint2D(rBox.X2, rBox.Y2);
    aPoints[0][3] = geometry::RealPoint2D(rBox.X2, rBox.Y1);
    Reference<rendering::XPolyPolygon2D> xRectangle (
        rxDevice->createCompatibleLinePolyPolygon(aPoints),
        UNO_QUERY);
    if (xRectangle.is())
        xRectangle->setClosed(0, sal_True);

    return xRectangle;
}




Reference<rendering::XPolyPolygon2D> PresenterGeometryHelper::CreatePolygon(
    const ::std::vector<css::awt::Rectangle>& rBoxes,
    const Reference<rendering::XGraphicDevice>& rxDevice)
{
    if ( ! rxDevice.is())
        return NULL;

    const sal_Int32 nCount (rBoxes.size());
    Sequence<Sequence<geometry::RealPoint2D> > aPoints(nCount);
    for (sal_Int32 nIndex=0; nIndex<nCount; ++nIndex)
    {
        const awt::Rectangle& rBox (rBoxes[nIndex]);
        aPoints[nIndex] = Sequence<geometry::RealPoint2D>(4);
        aPoints[nIndex][0] = geometry::RealPoint2D(rBox.X, rBox.Y);
        aPoints[nIndex][1] = geometry::RealPoint2D(rBox.X, rBox.Y+rBox.Height);
        aPoints[nIndex][2] = geometry::RealPoint2D(rBox.X+rBox.Width, rBox.Y+rBox.Height);
        aPoints[nIndex][3] = geometry::RealPoint2D(rBox.X+rBox.Width, rBox.Y);
    }

    Reference<rendering::XPolyPolygon2D> xRectangle (
        rxDevice->createCompatibleLinePolyPolygon(aPoints),
        UNO_QUERY);
    if (xRectangle.is())
        for (sal_Int32 nIndex=0; nIndex<nCount; ++nIndex)
            xRectangle->setClosed(nIndex, sal_True);

    return xRectangle;
}


} }
