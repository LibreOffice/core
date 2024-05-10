/* -*- Mode: C++; tab-width: offset; indent-tabs-mode: nil; c-basic-offset: offset -*- */
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
Bitmap OutputDeviceTestText::setupTextBitmap()
{
    initialSetup(21, 21, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    tools::Long midX = (maVDRectangle.Right() - maVDRectangle.Left()) / 2.0;
    tools::Long midY = (maVDRectangle.Bottom() - maVDRectangle.Top()) / 2.0;

    vcl::Font Font(u"DejaVu Sans"_ustr, u"Book"_ustr, Size(0, 10));

    mpVirtualDevice->Erase();
    mpVirtualDevice->SetFont(Font);
    mpVirtualDevice->SetTextColor(COL_LIGHTRED);
    mpVirtualDevice->DrawText(Point(midX, midY - midY / 2), u"I"_ustr);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}
}