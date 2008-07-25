/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PresenterUIPainter.cxx,v $
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

#include "PresenterUIPainter.hxx"

#include "PresenterCanvasHelper.hxx"
#include "PresenterGeometryHelper.hxx"
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sdext { namespace presenter {


void PresenterUIPainter::PaintHorizontalBitmapComposite (
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
    const css::awt::Rectangle& rRepaintBox,
    const css::awt::Rectangle& rBoundingBox,
    const css::uno::Reference<css::rendering::XBitmap>& rxLeftBitmap,
    const css::uno::Reference<css::rendering::XBitmap>& rxRepeatableCenterBitmap,
    const css::uno::Reference<css::rendering::XBitmap>& rxRightBitmap)
{
    if (PresenterGeometryHelper::AreRectanglesDisjoint(rRepaintBox, rBoundingBox))
    {
        // The bounding box lies completly outside the repaint area.
        // Nothing has to be repainted.
        return;
    }

    // Get bitmap sizes.
    geometry::IntegerSize2D aLeftBitmapSize;
    if (rxLeftBitmap.is())
        aLeftBitmapSize = rxLeftBitmap->getSize();
    geometry::IntegerSize2D aCenterBitmapSize;
    if (rxRepeatableCenterBitmap.is())
        aCenterBitmapSize = rxRepeatableCenterBitmap->getSize();
    geometry::IntegerSize2D aRightBitmapSize;
    if (rxRightBitmap.is())
        aRightBitmapSize = rxRightBitmap->getSize();

    // Prepare painting.
    rendering::ViewState aViewState (
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL);

    rendering::RenderState aRenderState (
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL,
        Sequence<double>(4),
        rendering::CompositeOperation::SOURCE);

    // Paint the left bitmap once.
    if (rxLeftBitmap.is())
    {
        const awt::Rectangle aLeftBoundingBox (
            rBoundingBox.X,
            rBoundingBox.Y,
            ::std::min(aLeftBitmapSize.Width, rBoundingBox.Width),
            rBoundingBox.Height);
        aViewState.Clip = Reference<rendering::XPolyPolygon2D>(
            PresenterGeometryHelper::CreatePolygon(
                PresenterGeometryHelper::Intersection(rRepaintBox, aLeftBoundingBox),
                rxCanvas->getDevice()));
        aRenderState.AffineTransform.m02 = aLeftBoundingBox.X;
        aRenderState.AffineTransform.m12
            = aLeftBoundingBox.Y + (aLeftBoundingBox.Height - aLeftBitmapSize.Height) / 2;
        rxCanvas->drawBitmap(rxLeftBitmap, aViewState, aRenderState);
    }

    // Paint the right bitmap once.
    if (rxRightBitmap.is())
    {
        const awt::Rectangle aRightBoundingBox (
            rBoundingBox.X + rBoundingBox.Width - aRightBitmapSize.Width,
            rBoundingBox.Y,
            ::std::min(aRightBitmapSize.Width, rBoundingBox.Width),
            rBoundingBox.Height);
        aViewState.Clip = Reference<rendering::XPolyPolygon2D>(
            PresenterGeometryHelper::CreatePolygon(
                PresenterGeometryHelper::Intersection(rRepaintBox, aRightBoundingBox),
                rxCanvas->getDevice()));
        aRenderState.AffineTransform.m02
            = aRightBoundingBox.X + aRightBoundingBox.Width - aRightBitmapSize.Width;
        aRenderState.AffineTransform.m12
            = aRightBoundingBox.Y + (aRightBoundingBox.Height - aRightBitmapSize.Height) / 2;
        rxCanvas->drawBitmap(rxRightBitmap, aViewState, aRenderState);
    }

    // Paint the center bitmap to fill the remaining space.
    if (rxRepeatableCenterBitmap.is())
    {
        const awt::Rectangle aCenterBoundingBox (
            rBoundingBox.X + aLeftBitmapSize.Width,
            rBoundingBox.Y,
            rBoundingBox.Width - aLeftBitmapSize.Width - aRightBitmapSize.Width,
            rBoundingBox.Height);
        if (aCenterBoundingBox.Width > 0)
        {
            aViewState.Clip = Reference<rendering::XPolyPolygon2D>(
                PresenterGeometryHelper::CreatePolygon(
                    PresenterGeometryHelper::Intersection(rRepaintBox, aCenterBoundingBox),
                    rxCanvas->getDevice()));
            sal_Int32 nX (aCenterBoundingBox.X);
            const sal_Int32 nRight (aCenterBoundingBox.X + aCenterBoundingBox.Width - 1);
            aRenderState.AffineTransform.m12
                = aCenterBoundingBox.Y + (aCenterBoundingBox.Height-aCenterBitmapSize.Height) / 2;
            while(nX <= nRight)
            {
                aRenderState.AffineTransform.m02 = nX;
                rxCanvas->drawBitmap(rxRepeatableCenterBitmap, aViewState, aRenderState);
                nX += aCenterBitmapSize.Width;
            }
        }
    }
}




void PresenterUIPainter::PaintVerticalBitmapComposite (
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
    const css::awt::Rectangle& rRepaintBox,
    const css::awt::Rectangle& rBoundingBox,
    const css::uno::Reference<css::rendering::XBitmap>& rxTopBitmap,
    const css::uno::Reference<css::rendering::XBitmap>& rxRepeatableCenterBitmap,
    const css::uno::Reference<css::rendering::XBitmap>& rxBottomBitmap)
{
    if (PresenterGeometryHelper::AreRectanglesDisjoint(rRepaintBox, rBoundingBox))
    {
        // The bounding box lies completly outside the repaint area.
        // Nothing has to be repainted.
        return;
    }

    // Get bitmap sizes.
    geometry::IntegerSize2D aTopBitmapSize;
    if (rxTopBitmap.is())
        aTopBitmapSize = rxTopBitmap->getSize();
    geometry::IntegerSize2D aCenterBitmapSize;
    if (rxRepeatableCenterBitmap.is())
        aCenterBitmapSize = rxRepeatableCenterBitmap->getSize();
    geometry::IntegerSize2D aBottomBitmapSize;
    if (rxBottomBitmap.is())
        aBottomBitmapSize = rxBottomBitmap->getSize();

    // Prepare painting.
    rendering::ViewState aViewState (
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL);

    rendering::RenderState aRenderState (
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL,
        Sequence<double>(4),
        rendering::CompositeOperation::SOURCE);

    // Paint the top bitmap once.
    if (rxTopBitmap.is())
    {
        const awt::Rectangle aTopBoundingBox (
            rBoundingBox.X,
            rBoundingBox.Y,
            rBoundingBox.Width,
            ::std::min(aTopBitmapSize.Height, rBoundingBox.Height));
        aViewState.Clip = Reference<rendering::XPolyPolygon2D>(
            PresenterGeometryHelper::CreatePolygon(
                PresenterGeometryHelper::Intersection(rRepaintBox, aTopBoundingBox),
                rxCanvas->getDevice()));
        aRenderState.AffineTransform.m02
            = aTopBoundingBox.X + (aTopBoundingBox.Width - aTopBitmapSize.Width) / 2;
        aRenderState.AffineTransform.m12 = aTopBoundingBox.Y;
        rxCanvas->drawBitmap(rxTopBitmap, aViewState, aRenderState);
    }

    // Paint the bottom bitmap once.
    if (rxBottomBitmap.is())
    {
        const sal_Int32 nBBoxHeight (::std::min(aBottomBitmapSize.Height, rBoundingBox.Height));
        const awt::Rectangle aBottomBoundingBox (
            rBoundingBox.X,
            rBoundingBox.Y  + rBoundingBox.Height - nBBoxHeight,
            rBoundingBox.Width,
            nBBoxHeight);
        aViewState.Clip = Reference<rendering::XPolyPolygon2D>(
            PresenterGeometryHelper::CreatePolygon(
                PresenterGeometryHelper::Intersection(rRepaintBox, aBottomBoundingBox),
                rxCanvas->getDevice()));
        aRenderState.AffineTransform.m02
            = aBottomBoundingBox.X + (aBottomBoundingBox.Width - aBottomBitmapSize.Width) / 2;
        aRenderState.AffineTransform.m12
            = aBottomBoundingBox.Y + aBottomBoundingBox.Height - aBottomBitmapSize.Height;
        rxCanvas->drawBitmap(rxBottomBitmap, aViewState, aRenderState);
    }

    // Paint the center bitmap to fill the remaining space.
    if (rxRepeatableCenterBitmap.is())
    {
        const awt::Rectangle aCenterBoundingBox (
            rBoundingBox.X,
            rBoundingBox.Y + aTopBitmapSize.Height,
            rBoundingBox.Width,
            rBoundingBox.Height - aTopBitmapSize.Height - aBottomBitmapSize.Height);
        if (aCenterBoundingBox.Height > 0)
        {
            aViewState.Clip = Reference<rendering::XPolyPolygon2D>(
                PresenterGeometryHelper::CreatePolygon(
                    PresenterGeometryHelper::Intersection(rRepaintBox, aCenterBoundingBox),
                    rxCanvas->getDevice()));
            sal_Int32 nY (aCenterBoundingBox.Y);
            const sal_Int32 nBottom (aCenterBoundingBox.Y + aCenterBoundingBox.Height - 1);
            aRenderState.AffineTransform.m02
                = aCenterBoundingBox.X + (aCenterBoundingBox.Width-aCenterBitmapSize.Width) / 2;
            while(nY <= nBottom)
            {
                aRenderState.AffineTransform.m12 = nY;
                rxCanvas->drawBitmap(rxRepeatableCenterBitmap, aViewState, aRenderState);
                nY += aCenterBitmapSize.Height;
            }
        }
    }
}





} } // end of namespace sdext::presenter
