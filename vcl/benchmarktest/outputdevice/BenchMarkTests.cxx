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

#include <ctime>

namespace vcl::test
{
Bitmap BenchMarkTests::setupMultiplePolygonsWithPolyPolygon(clock_t& aTimeTaken)
{
    initialSetup(24, 24, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor(constFillColor);

    tools::PolyPolygon aPolyPolygon(4);

    for (int nOffset = 1; nOffset <= 11; nOffset += 2)
    {
        tools::Polygon aPolygon1(4);
        aPolygon1.SetPoint(Point(maVDRectangle.Left() + nOffset, maVDRectangle.Top() + nOffset), 0);
        aPolygon1.SetPoint(Point(maVDRectangle.Right() - nOffset, maVDRectangle.Top() + nOffset),
                           1);
        aPolygon1.SetPoint(Point(maVDRectangle.Right() - nOffset, maVDRectangle.Bottom() - nOffset),
                           2);
        aPolygon1.SetPoint(Point(maVDRectangle.Left() + nOffset, maVDRectangle.Bottom() - nOffset),
                           3);
        aPolyPolygon.Insert(aPolygon1);
    }

    aTimeTaken = clock();
    mpVirtualDevice->DrawPolyPolygon(aPolyPolygon);
    aTimeTaken = clock() - aTimeTaken;

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

}
