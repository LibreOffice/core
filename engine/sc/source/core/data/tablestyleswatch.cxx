/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tablestyleswatch.hxx>
#include <tablestyle.hxx>

#include <o3tl/string_view.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/macros.h>

#include <algorithm>
#include <cmath>
#include <string_view>

#include "tablestylesvg.inc"

namespace
{
// "#RRGGBB" for an SVG fill/stroke attribute.
OUString hexColor(const Color& rColor)
{
    static constexpr char aDigits[] = "0123456789ABCDEF";
    OUStringBuffer aBuffer("#");
    auto append = [&aBuffer](sal_uInt8 nValue) {
        aBuffer.append(OUStringChar(aDigits[(nValue >> 4) & 0xF]));
        aBuffer.append(OUStringChar(aDigits[nValue & 0xF]));
    };
    append(rColor.GetRed());
    append(rColor.GetGreen());
    append(rColor.GetBlue());
    return aBuffer.makeStringAndClear();
}

// Scale each channel toward black; matches the former JS darkenColor.
Color darkenColor(const Color& rColor, double fFactor)
{
    auto scale = [fFactor](sal_uInt8 nValue) {
        return static_cast<sal_uInt8>(std::clamp(std::lround(nValue * (1.0 - fFactor)), 0L, 255L));
    };
    return Color(scale(rColor.GetRed()), scale(rColor.GetGreen()), scale(rColor.GetBlue()));
}

// Deepen a tint into the accent used for borders and content marks. The lightness is
// scaled in HSL, except for colours close to grey, which are darkened directly.
Color strengthenColor(const Color& rColor, double fFactor)
{
    const double r = rColor.GetRed() / 255.0;
    const double g = rColor.GetGreen() / 255.0;
    const double b = rColor.GetBlue() / 255.0;
    const double fMax = std::max({ r, g, b });
    const double fMin = std::min({ r, g, b });
    const double fDelta = fMax - fMin;

    double h = 0.0;
    double s = 0.0;
    double l = (fMax + fMin) / 2.0;
    if (fDelta != 0.0)
    {
        s = fDelta / (1.0 - std::abs(2.0 * l - 1.0));
        if (fMax == r)
            h = std::fmod((g - b) / fDelta, 6.0);
        else if (fMax == g)
            h = (b - r) / fDelta + 2.0;
        else
            h = (r - g) / fDelta + 4.0;
        h *= 60.0;
        if (h < 0.0)
            h += 360.0;
    }

    if (s < 0.15)
        return l > 0.5 ? darkenColor(rColor, 0.4) : darkenColor(rColor, 0.6);

    l *= fFactor;
    const double c = 1.0 - std::abs(2.0 * l - 1.0);
    const double x = c * (1.0 - std::abs(std::fmod(h / 60.0, 2.0) - 1.0));
    const double m = l - c / 2.0;

    double ro = 0.0;
    double go = 0.0;
    double bo = 0.0;
    if (h < 60.0)
    {
        ro = c;
        go = x;
    }
    else if (h < 120.0)
    {
        ro = x;
        go = c;
    }
    else if (h < 180.0)
    {
        go = c;
        bo = x;
    }
    else if (h < 240.0)
    {
        go = x;
        bo = c;
    }
    else if (h < 300.0)
    {
        ro = x;
        bo = c;
    }
    else
    {
        ro = c;
        bo = x;
    }

    auto chan = [](double v) {
        return static_cast<sal_uInt8>(std::clamp(std::lround(v * 255.0), 0L, 255L));
    };
    return Color(chan(ro + m), chan(go + m), chan(bo + m));
}

double colorBrightness(const Color& rColor)
{
    return (rColor.GetRed() * 299.0 + rColor.GetGreen() * 587.0 + rColor.GetBlue() * 114.0)
           / 1000.0;
}

OUString contrastStroke(const Color& rColor)
{
    return colorBrightness(rColor) < 128.0 ? u"white"_ustr : u"black"_ustr;
}

OUString bodyContrastStroke(const Color& rFirst, const Color& rSecond)
{
    return (colorBrightness(rFirst) + colorBrightness(rSecond)) / 2.0 < 128.0 ? u"white"_ustr
                                                                              : u"black"_ustr;
}

// The trailing number of a built-in style name (e.g. "TableStyleLight12" -> 12).
sal_Int32 styleNumber(const OUString& rName)
{
    sal_Int32 nStart = rName.getLength();
    while (nStart > 0 && rName[nStart - 1] >= '0' && rName[nStart - 1] <= '9')
        --nStart;
    return nStart < rName.getLength() ? o3tl::toInt32(rName.subView(nStart)) : 1;
}
}

ScTableStyleSwatch ScComputeTableStyleSwatch(const OUString& rName, bool bIsBuiltin,
                                             const Color& rHeader, const Color& rBody,
                                             const Color& rFirstStripe, const Color& rSecondStripe)
{
    const sal_Int32 nIndex = styleNumber(rName);
    sal_Int32 nTemplate = 8; // custom
    double fAccentFactor = 0.25;
    OUString aHeaderLine = u"black"_ustr;
    OUString aBodyLine = u"black"_ustr;
    OUString aSeparator = hexColor(rFirstStripe);

    switch (ScGetTableStyleFamily(rName, bIsBuiltin))
    {
        case ScTableStyleFamily::Light:
            nTemplate = nIndex <= 7 ? 0 : (nIndex <= 14 ? 1 : 2);
            fAccentFactor = 0.25;
            break;
        case ScTableStyleFamily::Medium:
            nTemplate = nIndex <= 7 ? 3 : (nIndex <= 15 ? 4 : (nIndex <= 21 ? 5 : 6));
            fAccentFactor = 0.45;
            break;
        case ScTableStyleFamily::Dark:
            nTemplate = 7;
            aHeaderLine = (nIndex >= 1 && nIndex <= 11) ? u"white"_ustr : u"black"_ustr;
            aBodyLine = (nIndex >= 1 && nIndex <= 7) ? u"white"_ustr : u"black"_ustr;
            aSeparator = (nIndex >= 1 && nIndex <= 7) ? u"#FFFFFF"_ustr : hexColor(rFirstStripe);
            break;
        case ScTableStyleFamily::Custom:
            break;
    }

    const Color aAccent = strengthenColor(rFirstStripe, fAccentFactor);

    ScTableStyleSwatch aSwatch;
    aSwatch.nTemplateIndex = nTemplate;
    aSwatch.aReplacements = {
        { u"${headerColor}"_ustr, hexColor(rHeader) },
        { u"${bodyColor}"_ustr, hexColor(rBody) },
        { u"${stripeColor}"_ustr, hexColor(rFirstStripe) },
        { u"${firstRowStripe}"_ustr, hexColor(rFirstStripe) },
        { u"${secondRowStripe}"_ustr, hexColor(rSecondStripe) },
        { u"${accentColor}"_ustr, hexColor(aAccent) },
        { u"${headerLineColor}"_ustr, aHeaderLine },
        { u"${bodyLineColor}"_ustr, aBodyLine },
        { u"${separatorColor}"_ustr, aSeparator },
        { u"${lineColor}"_ustr, bodyContrastStroke(rFirstStripe, rSecondStripe) },
        { u"${getContrastStroke(headerColor)}"_ustr, contrastStroke(rHeader) },
    };
    return aSwatch;
}

OUString ScApplyTableStyleSwatch(const ScTableStyleSwatch& rSwatch)
{
    if (rSwatch.nTemplateIndex < 0
        || rSwatch.nTemplateIndex >= sal_Int32(SAL_N_ELEMENTS(gaTableStyleSvgTemplates)))
        return OUString();

    OUString aSvg = OUString::fromUtf8(gaTableStyleSvgTemplates[rSwatch.nTemplateIndex]);
    for (const auto & [ rPlaceholder, rValue ] : rSwatch.aReplacements)
        aSvg = aSvg.replaceAll(rPlaceholder, rValue);
    return aSvg;
}

sal_Int32 ScTableStyleSvgTemplateCount()
{
    return sal_Int32(SAL_N_ELEMENTS(gaTableStyleSvgTemplates));
}

OUString ScTableStyleSvgTemplate(sal_Int32 nIndex)
{
    if (nIndex < 0 || nIndex >= ScTableStyleSvgTemplateCount())
        return OUString();
    return OUString::fromUtf8(gaTableStyleSvgTemplates[nIndex]);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
