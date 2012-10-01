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

#include "PresenterGeometryHelper.hxx"

#include <math.h>
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

sal_Int32 PresenterGeometryHelper::Floor (const double nValue)
{
    return sal::static_int_cast<sal_Int32>(floor(nValue));
}

sal_Int32 PresenterGeometryHelper::Ceil (const double nValue)
{
    return sal::static_int_cast<sal_Int32>(ceil(nValue));
}

sal_Int32 PresenterGeometryHelper::Round (const double nValue)
{
    return sal::static_int_cast<sal_Int32>(floor(0.5 + nValue));
}

awt::Rectangle PresenterGeometryHelper::ConvertRectangle (
    const geometry::RealRectangle2D& rBox)
{
    const sal_Int32 nLeft (Floor(rBox.X1));
    const sal_Int32 nTop (Floor(rBox.Y1));
    const sal_Int32 nRight (Ceil(rBox.X2));
    const sal_Int32 nBottom (Ceil(rBox.Y2));
    return awt::Rectangle (nLeft,nTop,nRight-nLeft,nBottom-nTop);
}

awt::Rectangle PresenterGeometryHelper::ConvertRectangleWithConstantSize (
    const geometry::RealRectangle2D& rBox)
{
    return awt::Rectangle (
        Round(rBox.X1),
        Round(rBox.Y1),
        Round(rBox.X2 - rBox.X1),
        Round(rBox.Y2 - rBox.Y1));
}

geometry::RealRectangle2D PresenterGeometryHelper::ConvertRectangle (
    const css::awt::Rectangle& rBox)
{
    return geometry::RealRectangle2D(
        rBox.X,
        rBox.Y,
        rBox.X + rBox.Width,
        rBox.Y + rBox.Height);
}

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

geometry::RealRectangle2D PresenterGeometryHelper::Intersection (
    const geometry::RealRectangle2D& rBox1,
    const geometry::RealRectangle2D& rBox2)
{
    const double nLeft (::std::max(rBox1.X1, rBox2.X1));
    const double nTop (::std::max(rBox1.Y1, rBox2.Y1));
    const double nRight (::std::min(rBox1.X2, rBox2.X2));
    const double nBottom (::std::min(rBox1.Y2, rBox2.Y2));
    if (nLeft >= nRight || nTop >= nBottom)
        return geometry::RealRectangle2D(0,0,0,0);
    else
        return geometry::RealRectangle2D(nLeft,nTop, nRight, nBottom);
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

bool PresenterGeometryHelper::IsInside (
    const css::awt::Rectangle& rBox1,
    const css::awt::Rectangle& rBox2)
{
    return rBox1.X >= rBox2.X
        && rBox1.Y >= rBox2.Y
        && rBox1.X+rBox1.Width <= rBox2.X+rBox2.Width
        && rBox1.Y+rBox1.Height <= rBox2.Y+rBox2.Height;
}

geometry::RealRectangle2D PresenterGeometryHelper::Union (
    const geometry::RealRectangle2D& rBox1,
    const geometry::RealRectangle2D& rBox2)
{
    const double nLeft (::std::min(rBox1.X1, rBox2.X1));
    const double nTop (::std::min(rBox1.Y1, rBox2.Y1));
    const double nRight (::std::max(rBox1.X2, rBox2.X2));
    const double nBottom (::std::max(rBox1.Y2, rBox2.Y2));
    if (nLeft >= nRight || nTop >= nBottom)
        return geometry::RealRectangle2D(0,0,0,0);
    else
        return geometry::RealRectangle2D(nLeft,nTop, nRight, nBottom);
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
    Reference<rendering::XLinePolyPolygon2D> xPolygon (
        rxDevice->createCompatibleLinePolyPolygon(aPoints));
    Reference<rendering::XPolyPolygon2D> xRectangle (xPolygon, UNO_QUERY);
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
    Reference<rendering::XLinePolyPolygon2D> xPolygon (
        rxDevice->createCompatibleLinePolyPolygon(aPoints));
    Reference<rendering::XPolyPolygon2D> xRectangle (xPolygon, UNO_QUERY);
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

    Reference<rendering::XLinePolyPolygon2D> xPolygon (
        rxDevice->createCompatibleLinePolyPolygon(aPoints));
    Reference<rendering::XPolyPolygon2D> xRectangle (xPolygon, UNO_QUERY);
    if (xRectangle.is())
        for (sal_Int32 nIndex=0; nIndex<nCount; ++nIndex)
            xRectangle->setClosed(nIndex, sal_True);

    return xRectangle;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
