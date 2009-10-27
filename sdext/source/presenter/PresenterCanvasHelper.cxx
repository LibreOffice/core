/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PresenterCanvasHelper.cxx,v $
 *
 * $Revision: 1.5 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#include "PresenterCanvasHelper.hxx"

#include "PresenterController.hxx"
#include "PresenterGeometryHelper.hxx"
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/TextDirection.hpp>
#include <com/sun/star/rendering/TexturingMode.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sdext { namespace presenter {

PresenterCanvasHelper::PresenterCanvasHelper (void)
    : maDefaultViewState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL),
      maDefaultRenderState(
          geometry::AffineMatrix2D(1,0,0, 0,1,0),
          NULL,
          Sequence<double>(4),
          rendering::CompositeOperation::SOURCE)
{
}




PresenterCanvasHelper::~PresenterCanvasHelper (void)
{
}




void PresenterCanvasHelper::Paint (
    const SharedBitmapDescriptor& rpBitmap,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
    const css::awt::Rectangle& rRepaintBox,
    const css::awt::Rectangle& rOuterBoundingBox,
    const css::awt::Rectangle& rContentBoundingBox) const
{
    PaintRectangle(rpBitmap,rxCanvas,rRepaintBox,rOuterBoundingBox,rContentBoundingBox,
        maDefaultViewState, maDefaultRenderState);
}




void PresenterCanvasHelper::PaintRectangle (
    const SharedBitmapDescriptor& rpBitmap,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
    const css::awt::Rectangle& rRepaintBox,
    const css::awt::Rectangle& rOuterBoundingBox,
    const css::awt::Rectangle& rContentBoundingBox,
    const css::rendering::ViewState& rDefaultViewState,
    const css::rendering::RenderState& rDefaultRenderState)
{
    if (rpBitmap.get() == NULL)
        return;

    if ( ! rxCanvas.is() || ! rxCanvas->getDevice().is())
        return;

    // Create a clip polypolygon that has the content box as hole.
    ::std::vector<awt::Rectangle> aRectangles;
    aRectangles.reserve(2);
    aRectangles.push_back(
        PresenterGeometryHelper::Intersection(rRepaintBox, rOuterBoundingBox));
    if (rContentBoundingBox.Width > 0 && rContentBoundingBox.Height > 0)
        aRectangles.push_back(
            PresenterGeometryHelper::Intersection(rRepaintBox, rContentBoundingBox));
    Reference<rendering::XPolyPolygon2D> xPolyPolygon (
        PresenterGeometryHelper::CreatePolygon(
            aRectangles,
            rxCanvas->getDevice()));
    if ( ! xPolyPolygon.is())
        return;
    xPolyPolygon->setFillRule(rendering::FillRule_EVEN_ODD);

    if (rpBitmap->GetNormalBitmap().is())
    {
        if (rpBitmap->meHorizontalTexturingMode == PresenterBitmapDescriptor::Repeat
            || rpBitmap->meVerticalTexturingMode == PresenterBitmapDescriptor::Repeat)
        {
            PaintTiledBitmap(
                Reference<rendering::XBitmap>(rpBitmap->GetNormalBitmap(), UNO_QUERY),
                rxCanvas,
                rRepaintBox,
                xPolyPolygon,
                rContentBoundingBox,
                rDefaultViewState,
                rDefaultRenderState);
        }
        else
        {
            PaintBitmap(
                Reference<rendering::XBitmap>(rpBitmap->GetNormalBitmap(), UNO_QUERY),
                awt::Point(rOuterBoundingBox.X, rOuterBoundingBox.Y),
                rxCanvas,
                rRepaintBox,
                xPolyPolygon,
                rDefaultViewState,
                rDefaultRenderState);
        }
    }
    else
    {
        PaintColor(
            rpBitmap->maReplacementColor,
            rxCanvas,
            rRepaintBox,
            xPolyPolygon,
            rDefaultViewState,
            rDefaultRenderState);
    }
}




void PresenterCanvasHelper::PaintTiledBitmap (
    const css::uno::Reference<css::rendering::XBitmap>& rxTexture,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
    const css::awt::Rectangle& rRepaintBox,
    const css::uno::Reference<css::rendering::XPolyPolygon2D>& rxPolygon,
    const css::awt::Rectangle& rHole,
    const css::rendering::ViewState& rDefaultViewState,
    const css::rendering::RenderState& rDefaultRenderState)
{
    if ( ! rxCanvas.is() || ! rxCanvas->getDevice().is())
        return;

    if ( ! rxTexture.is())
        return;

    if ( ! rxPolygon.is())
        return;

    rendering::ViewState aViewState (rDefaultViewState);
    aViewState.Clip = rxPolygon;

    // Create a local render state at which the location of the bitmap is
    // set.
    rendering::RenderState aRenderState (rDefaultRenderState);


    // Tile the bitmap over the repaint box.
    const geometry::IntegerSize2D aBitmapSize (rxTexture->getSize());
    const sal_Int32 nLeft = (rRepaintBox.X / aBitmapSize.Width) * aBitmapSize.Width;
    const sal_Int32 nTop = (rRepaintBox.Y / aBitmapSize.Height) * aBitmapSize.Height;
    const sal_Int32 nRight = ((rRepaintBox.X + rRepaintBox.Width - 1 + aBitmapSize.Width - 1)
        / aBitmapSize.Width) * aBitmapSize.Width;
    const sal_Int32 nBottom = ((rRepaintBox.Y + rRepaintBox.Height - 1 + aBitmapSize.Height - 1)
        / aBitmapSize.Height) * aBitmapSize.Height;

    for (sal_Int32 nY=nTop; nY<=nBottom; nY+=aBitmapSize.Height)
        for (sal_Int32 nX=nLeft; nX<=nRight; nX+=aBitmapSize.Width)
        {
            if (PresenterGeometryHelper::IsInside(
                awt::Rectangle(nX,nY,aBitmapSize.Width,aBitmapSize.Height),
                rHole))
            {
                continue;
            }
            aRenderState.AffineTransform.m02 = nX;
            aRenderState.AffineTransform.m12 = nY;
            rxCanvas->drawBitmap(
                rxTexture,
                aViewState,
                aRenderState);
        }
}




void PresenterCanvasHelper::PaintBitmap (
    const css::uno::Reference<css::rendering::XBitmap>& rxBitmap,
    const awt::Point& rLocation,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
    const css::awt::Rectangle& rRepaintBox,
    const css::uno::Reference<css::rendering::XPolyPolygon2D>& rxPolygon,
    const css::rendering::ViewState& rDefaultViewState,
    const css::rendering::RenderState& rDefaultRenderState)
{
    if ( ! rxCanvas.is() || ! rxCanvas->getDevice().is())
        return;

    if ( ! rxBitmap.is())
        return;

    if ( ! rxPolygon.is())
        return;

    // Set the repaint box as clip rectangle at the view state.
    rendering::ViewState aViewState (rDefaultViewState);
    aViewState.Clip = PresenterGeometryHelper::CreatePolygon(rRepaintBox, rxCanvas->getDevice());


    // Setup the rendering state so that the bitmap is painted top left in
    // the polygon bounding box.
    rendering::RenderState aRenderState (rDefaultRenderState);
    aRenderState.AffineTransform = geometry::AffineMatrix2D(1,0, rLocation.X, 0,1,rLocation.Y);
    aRenderState.Clip = rxPolygon;

    rxCanvas->drawBitmap(
        rxBitmap,
        aViewState,
        aRenderState);
}




void PresenterCanvasHelper::PaintColor (
    const css::util::Color nColor,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
    const css::awt::Rectangle& rRepaintBox,
    const css::uno::Reference<css::rendering::XPolyPolygon2D>& rxPolygon,
    const css::rendering::ViewState& rDefaultViewState,
    const css::rendering::RenderState& rDefaultRenderState)
{
    if ( ! rxCanvas.is() || ! rxCanvas->getDevice().is())
        return;

    if ( ! rxPolygon.is())
        return;

    // Set the repaint box as clip rectangle at the view state.
    rendering::ViewState aViewState (rDefaultViewState);
    aViewState.Clip = PresenterGeometryHelper::CreatePolygon(rRepaintBox, rxCanvas->getDevice());


    // Setup the rendering state to use the given color.
    rendering::RenderState aRenderState (rDefaultRenderState);
    SetDeviceColor(aRenderState, nColor);

    rxCanvas->fillPolyPolygon(
        rxPolygon,
        aViewState,
        aRenderState);
}




void PresenterCanvasHelper::SetDeviceColor(
    rendering::RenderState& rRenderState,
    const util::Color aColor)
{
    // Other component counts then 4 (RGBA) are not accepted (anymore).

    OSL_ASSERT(rRenderState.DeviceColor.getLength() == 4);
    if (rRenderState.DeviceColor.getLength() == 4)
    {
        rRenderState.DeviceColor[0] = ((aColor >> 16) & 0x0ff) / 255.0;
        rRenderState.DeviceColor[1] = ((aColor >> 8) & 0x0ff) / 255.0;
        rRenderState.DeviceColor[2] = ((aColor >> 0) & 0x0ff) / 255.0;
        rRenderState.DeviceColor[3] = 1.0 - ((aColor >> 24) & 0x0ff) / 255.0;
    }
}




css::geometry::RealRectangle2D PresenterCanvasHelper::GetTextBoundingBox (
    const css::uno::Reference<css::rendering::XCanvasFont>& rxFont,
    const ::rtl::OUString& rsText,
    const sal_Int8 nTextDirection)
{
    if (rxFont.is() && rsText.getLength() > 0)
    {
        rendering::StringContext aContext (rsText, 0, rsText.getLength());
        Reference<rendering::XTextLayout> xLayout (
            rxFont->createTextLayout(aContext, nTextDirection, 0));
        return xLayout->queryTextBounds();
    }
    else
    {
        return geometry::RealRectangle2D(0,0,0,0);
    }
}




css::geometry::RealSize2D PresenterCanvasHelper::GetTextSize (
    const css::uno::Reference<css::rendering::XCanvasFont>& rxFont,
    const ::rtl::OUString& rsText,
    const sal_Int8 nTextDirection)
{
    const geometry::RealRectangle2D aTextBBox (GetTextBoundingBox(rxFont, rsText, nTextDirection));
    return css::geometry::RealSize2D(aTextBBox.X2 - aTextBBox.X1, aTextBBox.Y2 - aTextBBox.Y1);
}


} } // end of namespace sdext::presenter
