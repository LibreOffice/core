/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <test/outputdevice.hxx>
#include <vcl/bitmapex.hxx>

namespace vcl::test
{
namespace
{
void drawPolyLineOffset(OutputDevice& rDevice, tools::Rectangle const& rRect, int nOffset)
{
    int nMidOffset = rRect.GetWidth() / 2;
    basegfx::B2DPolygon aPolygon{
        basegfx::B2DPoint(rRect.Left() + nOffset - (nOffset + 1) / 2, rRect.Top() + nOffset - 1),
        basegfx::B2DPoint(rRect.Right() - nMidOffset - nOffset / 3, rRect.Top() + nOffset - 1),
        basegfx::B2DPoint(rRect.Right() - nMidOffset - nOffset / 3, rRect.Bottom() - nOffset + 1),
        basegfx::B2DPoint(rRect.Left() + nOffset - (nOffset + 1) / 2, rRect.Bottom() - nOffset + 1),
    };
    aPolygon.setClosed(true);

    rDevice.DrawPolyLine(aPolygon, 0.0); // draw hairline
}

void addDiamondPoints(tools::Rectangle rRect, int nOffset, basegfx::B2DPolygon& rPolygon)
{
    double midPointX = rRect.Left() + (rRect.Right() - rRect.Left()) / 2.0;
    double midPointY = rRect.Top() + (rRect.Bottom() - rRect.Top()) / 2.0;

    rPolygon.append({ midPointX, midPointY - nOffset });
    rPolygon.append({ midPointX + nOffset, midPointY });
    rPolygon.append({ midPointX, midPointY + nOffset });
    rPolygon.append({ midPointX - nOffset, midPointY });
}

} // end anonymous namespace

Bitmap OutputDeviceTestPolyLineB2D::setupRectangle(bool bEnableAA)
{
    initialSetup(13, 13, constBackgroundColor, bEnableAA);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    drawPolyLineOffset(*mpVirtualDevice, maVDRectangle, 2);
    drawPolyLineOffset(*mpVirtualDevice, maVDRectangle, 5);

    return mpVirtualDevice->GetBitmapEx(maVDRectangle.TopLeft(), maVDRectangle.GetSize())
        .GetBitmap();
}

Bitmap OutputDeviceTestPolyLineB2D::setupDiamond()
{
    initialSetup(11, 11, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    basegfx::B2DPolygon aPolygon;
    addDiamondPoints(maVDRectangle, 4, aPolygon);
    aPolygon.setClosed(true);

    mpVirtualDevice->DrawPolyLine(aPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolyLineB2D::setupBezier()
{
    initialSetup(21, 21, constBackgroundColor, false);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    basegfx::B2DPolygon aPolygon;
    addDiamondPoints(maVDRectangle, 8, aPolygon);
    aPolygon.setClosed(true);

    double minX = maVDRectangle.Left() + 4;
    double maxX = maVDRectangle.Right() - 4;
    double minY = maVDRectangle.Top() + 4;
    double maxY = maVDRectangle.Bottom() - 4;

    aPolygon.setControlPoints(0, { minX, minY }, { maxX, minY });
    aPolygon.setControlPoints(1, { maxX, minY }, { maxX, maxY });
    aPolygon.setControlPoints(2, { maxX, maxY }, { minX, maxY });
    aPolygon.setControlPoints(3, { minX, maxY }, { minX, minY });

    mpVirtualDevice->DrawPolyLine(aPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolyLineB2D::setupAABezier()
{
    initialSetup(21, 21, constBackgroundColor, true);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    basegfx::B2DPolygon aPolygon;
    addDiamondPoints(maVDRectangle, 8, aPolygon);
    aPolygon.setClosed(true);

    double minX = maVDRectangle.Left() + 4;
    double maxX = maVDRectangle.Right() - 4;
    double minY = maVDRectangle.Top() + 4;
    double maxY = maVDRectangle.Bottom() - 4;

    aPolygon.setControlPoints(0, { minX, minY }, { maxX, minY });
    aPolygon.setControlPoints(1, { maxX, minY }, { maxX, maxY });
    aPolygon.setControlPoints(2, { maxX, maxY }, { minX, maxY });
    aPolygon.setControlPoints(3, { minX, maxY }, { minX, minY });

    mpVirtualDevice->DrawPolyLine(aPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolyLineB2D::setupHalfEllipse(bool aEnableAA)
{
    initialSetup(19, 21, constBackgroundColor, aEnableAA);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    mpVirtualDevice->DrawPolyLine(OutputDeviceTestCommon::createHalfEllipsePolygon());

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolyLineB2D::setupRectangleOnSize1028()
{
    initialSetup(1028, 1028, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    drawPolyLineOffset(*mpVirtualDevice, maVDRectangle, 2);
    drawPolyLineOffset(*mpVirtualDevice, maVDRectangle, 5);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolyLineB2D::setupRectangleOnSize4096()
{
    initialSetup(4096, 4096, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    drawPolyLineOffset(*mpVirtualDevice, maVDRectangle, 2);
    drawPolyLineOffset(*mpVirtualDevice, maVDRectangle, 5);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

} // end namespace vcl::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
