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
#include <vcl/gradient.hxx>

namespace vcl::test
{
Bitmap OutputDeviceTestGradient::setupLinearGradient()
{
    initialSetup(12, 12, constBackgroundColor);

    Gradient aGradient(GradientStyle::Linear, Color(0xFF, 0xFF, 0xFF), Color(0x00, 0x00, 0x00));
    aGradient.SetAngle(900_deg10);
    tools::Rectangle aDrawRect(maVDRectangle.Left() + 1, maVDRectangle.Top() + 1,
                               maVDRectangle.Right() - 1, maVDRectangle.Bottom() - 1);
    mpVirtualDevice->DrawGradient(aDrawRect, aGradient);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestGradient::setupLinearGradientAngled()
{
    initialSetup(12, 12, constBackgroundColor);

    Gradient aGradient(GradientStyle::Linear, Color(0xFF, 0xFF, 0xFF), Color(0x00, 0x00, 0x00));
    aGradient.SetAngle(450_deg10);
    tools::Rectangle aDrawRect(maVDRectangle.Left() + 1, maVDRectangle.Top() + 1,
                               maVDRectangle.Right() - 1, maVDRectangle.Bottom() - 1);
    mpVirtualDevice->DrawGradient(aDrawRect, aGradient);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestGradient::setupLinearGradientBorder()
{
    initialSetup(12, 12, constBackgroundColor);

    Gradient aGradient(GradientStyle::Linear, Color(0xFF, 0xFF, 0xFF), Color(0x00, 0x00, 0x00));
    aGradient.SetBorder(50);
    tools::Rectangle aDrawRect(maVDRectangle.Left() + 1, maVDRectangle.Top() + 1,
                               maVDRectangle.Right() - 1, maVDRectangle.Bottom() - 1);
    mpVirtualDevice->DrawGradient(aDrawRect, aGradient);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestGradient::setupLinearGradientIntensity()
{
    initialSetup(12, 12, constBackgroundColor);

    Gradient aGradient(GradientStyle::Linear, Color(0xFF, 0xFF, 0xFF), Color(0x00, 0x00, 0x00));
    aGradient.SetStartIntensity(50);
    tools::Rectangle aDrawRect(maVDRectangle.Left() + 1, maVDRectangle.Top() + 1,
                               maVDRectangle.Right() - 1, maVDRectangle.Bottom() - 1);
    mpVirtualDevice->DrawGradient(aDrawRect, aGradient);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestGradient::setupLinearGradientSteps()
{
    initialSetup(12, 12, constBackgroundColor);

    Gradient aGradient(GradientStyle::Linear, Color(0xFF, 0xFF, 0xFF), Color(0x00, 0x00, 0x00));
    aGradient.SetAngle(900_deg10);
    aGradient.SetSteps(4);
    tools::Rectangle aDrawRect(maVDRectangle.Left() + 1, maVDRectangle.Top() + 1,
                               maVDRectangle.Right() - 1, maVDRectangle.Bottom() - 1);
    mpVirtualDevice->DrawGradient(aDrawRect, aGradient);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestGradient::setupAxialGradient()
{
    initialSetup(13, 13, constBackgroundColor);

    Gradient aGradient(GradientStyle::Axial, Color(0xFF, 0xFF, 0xFF), Color(0x00, 0x00, 0x00));
    aGradient.SetAngle(900_deg10);
    tools::Rectangle aDrawRect(maVDRectangle.Left() + 1, maVDRectangle.Top() + 1,
                               maVDRectangle.Right() - 1, maVDRectangle.Bottom() - 1);
    mpVirtualDevice->DrawGradient(aDrawRect, aGradient);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestGradient::setupRadialGradient()
{
    initialSetup(12, 12, constBackgroundColor);

    Gradient aGradient(GradientStyle::Radial, Color(0xFF, 0xFF, 0xFF), Color(0x00, 0x00, 0x00));
    tools::Rectangle aDrawRect(maVDRectangle.Left() + 1, maVDRectangle.Top() + 1,
                               maVDRectangle.Right() - 1, maVDRectangle.Bottom() - 1);
    mpVirtualDevice->DrawGradient(aDrawRect, aGradient);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestGradient::setupRadialGradientOfs()
{
    initialSetup(12, 12, constBackgroundColor);

    Gradient aGradient(GradientStyle::Radial, Color(0xFF, 0xFF, 0xFF), Color(0x00, 0x00, 0x00));
    aGradient.SetOfsX(100); // Move center to the bottom-right corner.
    aGradient.SetOfsY(100);
    tools::Rectangle aDrawRect(maVDRectangle.Left() + 1, maVDRectangle.Top() + 1,
                               maVDRectangle.Right() - 1, maVDRectangle.Bottom() - 1);
    mpVirtualDevice->DrawGradient(aDrawRect, aGradient);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

} // end namespace vcl::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
