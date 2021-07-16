/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <FontFeatures.hxx>

std::vector<vcl::font::Feature> getFontFeatureList(OUString const& rFontName, VirtualDevice& rVDev)
{
    rVDev.SetOutputSizePixel(Size(10, 10));

    vcl::Font aFont = rVDev.GetFont();
    aFont.SetFamilyName(rFontName);
    rVDev.SetFont(aFont);

    std::vector<vcl::font::Feature> vFontFeatures;

    if (!rVDev.GetFontFeatures(vFontFeatures))
    {
        vFontFeatures.clear();
    }

    return vFontFeatures;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */