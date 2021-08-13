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

namespace vcl::test
{
Bitmap OutputDeviceTestClip::setupClipRectangle()
{
    initialSetup(13, 13, constBackgroundColor);

    tools::Rectangle rectangle = maVDRectangle;
    rectangle.shrink(2);
    mpVirtualDevice->SetClipRegion(vcl::Region(rectangle));
    mpVirtualDevice->SetBackground(constFillColor);
    mpVirtualDevice->Erase(maVDRectangle);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestClip::setupClipPolygon()
{
    initialSetup(13, 13, constBackgroundColor);

    tools::Rectangle rectangle = maVDRectangle;
    rectangle.shrink(2);
    mpVirtualDevice->SetClipRegion(vcl::Region(tools::Polygon(rectangle)));
    mpVirtualDevice->SetBackground(constFillColor);
    mpVirtualDevice->Erase(maVDRectangle);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestClip::setupClipPolyPolygon()
{
    initialSetup(13, 13, constBackgroundColor);

    tools::Rectangle rectangle = maVDRectangle;
    rectangle.shrink(2);
    mpVirtualDevice->SetClipRegion(vcl::Region(tools::PolyPolygon(rectangle)));
    mpVirtualDevice->SetBackground(constFillColor);
    mpVirtualDevice->Erase(maVDRectangle);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestClip::setupClipB2DPolyPolygon()
{
    initialSetup(13, 13, constBackgroundColor);

    tools::Rectangle rectangle = maVDRectangle;
    rectangle.shrink(2);
    mpVirtualDevice->SetClipRegion(vcl::Region(basegfx::B2DPolyPolygon(basegfx::B2DPolygon{
        basegfx::B2DPoint(rectangle.Left(), rectangle.Top()),
        basegfx::B2DPoint(rectangle.Left(), rectangle.Bottom()),
        basegfx::B2DPoint(rectangle.Right(), rectangle.Bottom()),
        basegfx::B2DPoint(rectangle.Right(), rectangle.Top()),
    })));
    mpVirtualDevice->SetBackground(constFillColor);
    mpVirtualDevice->Erase(maVDRectangle);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

TestResult OutputDeviceTestClip::checkClip(Bitmap& aBitmap)
{
    std::vector<Color> aExpected{ constBackgroundColor, constBackgroundColor, constFillColor,
                                  constFillColor,       constFillColor,       constFillColor,
                                  constFillColor };
    return checkRectangles(aBitmap, aExpected);
}

} // end namespace vcl::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
