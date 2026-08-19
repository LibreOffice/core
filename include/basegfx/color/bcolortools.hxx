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

#pragma once

#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
class BColor;
}

namespace basegfx::utils
{
/// Transform from RGB to HSL
BASEGFX_DLLPUBLIC BColor rgb2hsl(const BColor& rRGBColor);
/// Transform from HSL to RGB
BASEGFX_DLLPUBLIC BColor hsl2rgb(const BColor& rHSLColor);

/// Transform from RGB to HSV
BASEGFX_DLLPUBLIC BColor rgb2hsv(const BColor& rRGBColor);
/// Transform from HSV to RGB
BASEGFX_DLLPUBLIC BColor hsv2rgb(const BColor& rHSVColor);

/** Determines an interpolated color using HSL color model

    @param rStartRGB color at position 0
    @param rEndRGB color at position 1
    The components are assumed to contain RGB values, such as Color.getBColor() produces.

    @param fFact position in interval [0..1] for which the interpolated color is requested.
    Values <0 are treated as 0, values >1 are treated as 1.

    @param bInc. If bInc is true, the arc of the color wheel where the angles increase from the start
    color to the end color is used. If bInc is false, the opposite arc is used. Example:
    For start color Magenta (Hue 300°) and end color Cyan (Hue 180°) and fFact=0.5, for bInc value
    true the color Yellow (Hue 60°) is used for the result, whereas for value false the color Blue
    (Hue 240°) is used.

    @returns the color at the position fFact as BColor to be interpreted as RGB color, with each
    component in range [0..1].
*/
BASEGFX_DLLPUBLIC BColor interpolateInHSL(const BColor& rStartRGB, const BColor& rEndRGB,
                                          const double fFact, const bool bInc);

} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
