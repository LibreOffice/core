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

namespace vcl {
namespace test {

Bitmap OutputDeviceTestGradient::setupLinearGradient()
{
    initialSetup(12, 12, constBackgroundColor);

    Gradient aGradient(GradientStyle::Linear, Color(0xFF, 0xFF, 0xFF), Color(0x00, 0x00, 0x00));
    aGradient.SetAngle(900);
    tools::Rectangle aDrawRect(maVDRectangle.Left() + 1,  maVDRectangle.Top() + 1,
                        maVDRectangle.Right() - 1, maVDRectangle.Bottom() - 1);
    mpVirtualDevice->DrawGradient(aDrawRect, aGradient);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestGradient::setupRadialGradient()
{
    initialSetup(12, 12, constBackgroundColor);

    Gradient aGradient(GradientStyle::Radial, Color(0xFF, 0xFF, 0xFF), Color(0x00, 0x00, 0x00));
    tools::Rectangle aDrawRect(maVDRectangle.Left() + 1,  maVDRectangle.Top() + 1,
                        maVDRectangle.Right() - 1, maVDRectangle.Bottom() - 1);
    mpVirtualDevice->DrawGradient(aDrawRect, aGradient);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

}} // end namespace vcl::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
