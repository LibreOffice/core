/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <basegfx/polygon/WaveLine.hxx>
#include <basegfx/point/b2dpoint.hxx>

namespace basegfx
{
BASEGFX_DLLPUBLIC B2DPolygon createWaveLinePolygon(basegfx::B2DRectangle const& rRectangle)
{
    basegfx::B2DPolygon aPolygon;

    double fWaveHeight = rRectangle.getHeight();
    // Wavelength depends on the wave height so it looks nice
    double fHalfWaveLength = fWaveHeight + 1.0;
    double fWaveAmplitude = fWaveHeight / 2.0;

    double fLastX = rRectangle.getMinX();
    double fBaseY = rRectangle.getMinY() + fWaveAmplitude;
    double fDirection = 1.0;

    // In quadratic bezier the curve is 1/2 of the control height
    // so we need to compensate for that.
    double fHeightCompensation = 2.0;

    aPolygon.append(basegfx::B2DPoint(fLastX, fBaseY));

    for (double fI = fHalfWaveLength; fI <= rRectangle.getWidth(); fI += fHalfWaveLength)
    {
        basegfx::B2DPoint aPoint(fLastX + fHalfWaveLength, fBaseY);
        basegfx::B2DPoint aControl(fLastX + (fHalfWaveLength / 2.0),
                                   fBaseY + fDirection * fWaveAmplitude * fHeightCompensation);

        aPolygon.appendQuadraticBezierSegment(aControl, aPoint);

        fLastX = aPoint.getX(); // next iteration
        fDirection *= -1.0; // fDirection iterates between 1 and -1
    }

    return aPolygon;
}

} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
