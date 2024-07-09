/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <docmodel/uno/UnoGradientTools.hxx>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/awt/Gradient2.hpp>
#include <com/sun/star/rendering/RGBColor.hpp>
#include <basegfx/utils/bgradient.hxx>
#include <tools/color.hxx>

using namespace css;

namespace model::gradient
{
css::awt::ColorStopSequence createColorStopSequence(basegfx::BColorStops const& rColorStops)
{
    // Fill color stops
    css::awt::ColorStopSequence aSequence(rColorStops.size());
    css::awt::ColorStop* pSequence(aSequence.getArray());

    for (const auto& rStop : rColorStops)
    {
        const auto& rBColor = rStop.getStopColor();
        pSequence->StopOffset = rStop.getStopOffset();
        pSequence->StopColor
            = css::rendering::RGBColor(rBColor.getRed(), rBColor.getGreen(), rBColor.getBlue());
        pSequence++;
    }

    return aSequence;
}

css::awt::Gradient2 createUnoGradient2(basegfx::BGradient const& rGradient)
{
    css::awt::Gradient2 aGradient2;

    // standard values
    aGradient2.Style = rGradient.GetGradientStyle();
    aGradient2.Angle = static_cast<short>(rGradient.GetAngle());
    aGradient2.Border = rGradient.GetBorder();
    aGradient2.XOffset = rGradient.GetXOffset();
    aGradient2.YOffset = rGradient.GetYOffset();
    aGradient2.StartIntensity = rGradient.GetStartIntens();
    aGradient2.EndIntensity = rGradient.GetEndIntens();
    aGradient2.StepCount = rGradient.GetSteps();

    // for compatibility, still set StartColor/EndColor
    // NOTE: All code after adapting to multi color gradients works
    //       using the ColorSteps, so in principle Start/EndColor might
    //       be either
    //        (a) ignored consequently everywhere or
    //        (b) be set/added consequently everywhere
    //       since this is - in principle - redundant data.
    //       Be aware that e.g. cases like DrawingML::EqualGradients
    //       and others would have to be identified and adapted (!)
    //       Since awt::Gradient2 is UNO API data there might
    //       be cases where just awt::Gradient is transferred, so (b)
    //       is far better backwards compatible and thus more safe, so
    //       all changes will make use of additionally using/setting
    //       these additionally, but will only make use of the given
    //       ColorSteps if these are not empty, assuming that these
    //       already contain Start/EndColor.
    //       In principle that redundancy and that it is conflict-free
    //       could even be checked and asserted, but consequently using
    //       (b) methodically should be safe.
    const basegfx::BColorStops& rColorStops = rGradient.GetColorStops();
    aGradient2.StartColor = static_cast<sal_Int32>(::Color(rColorStops.front().getStopColor()));
    aGradient2.EndColor = static_cast<sal_Int32>(::Color(rColorStops.back().getStopColor()));

    aGradient2.ColorStops = createColorStopSequence(rColorStops);

    return aGradient2;
}

namespace
{
void fillFromColorStopSequence(basegfx::BColorStops& rColorStops,
                               css::awt::ColorStopSequence const& rUnoColorStopSequence)
{
    const auto nLength = rUnoColorStopSequence.getLength();
    if (0 != nLength)
    {
        rColorStops.clear();
        rColorStops.reserve(nLength);

        for (css::awt::ColorStop const& rSourceStop : rUnoColorStopSequence)
        {
            rColorStops.emplace_back(rSourceStop.StopOffset,
                                     basegfx::BColor(rSourceStop.StopColor.Red,
                                                     rSourceStop.StopColor.Green,
                                                     rSourceStop.StopColor.Blue));
        }
    }
}

void fillFromGradient2(basegfx::BGradient& rBGradient, css::awt::Gradient2 const& rGradient2)
{
    rBGradient.SetGradientStyle(rGradient2.Style);
    rBGradient.SetAngle(Degree10(rGradient2.Angle));
    rBGradient.SetBorder(rGradient2.Border);
    rBGradient.SetXOffset(rGradient2.XOffset);
    rBGradient.SetYOffset(rGradient2.YOffset);
    rBGradient.SetStartIntens(rGradient2.StartIntensity);
    rBGradient.SetEndIntens(rGradient2.EndIntensity);
    rBGradient.SetSteps(rGradient2.StepCount);

    // set ColorStops
    if (rGradient2.ColorStops.hasElements())
    {
        basegfx::BColorStops aColorStops;
        fillFromColorStopSequence(aColorStops, rGradient2.ColorStops);
        rBGradient.SetColorStops(aColorStops);
    }
    else
    {
        // if not, for compatibility, use StartColor/EndColor
        basegfx::BColorStops aColorStops{
            basegfx::BColorStop(0.0, ::Color(ColorTransparency, rGradient2.StartColor).getBColor()),
            basegfx::BColorStop(1.0, ::Color(ColorTransparency, rGradient2.EndColor).getBColor())
        };
        rBGradient.SetColorStops(aColorStops);
    }
}

} // end anonymous

basegfx::BColorStops getColorStopsFromUno(css::awt::ColorStopSequence const& rColorStopSequence)
{
    basegfx::BColorStops aColorStops;
    fillFromColorStopSequence(aColorStops, rColorStopSequence);
    return aColorStops;
}

basegfx::BColorStops getColorStopsFromAny(css::uno::Any const& rAny)
{
    basegfx::BColorStops aColorStops;
    if (!rAny.has<css::awt::ColorStopSequence>())
        return aColorStops;

    auto aSequence = rAny.get<css::awt::ColorStopSequence>();
    fillFromColorStopSequence(aColorStops, aSequence);
    return aColorStops;
}

basegfx::BGradient getFromUnoGradient2(css::awt::Gradient2 const& rGradient2)
{
    basegfx::BGradient aBGradient;
    fillFromGradient2(aBGradient, rGradient2);
    return aBGradient;
}

basegfx::BGradient getFromAny(css::uno::Any const& rAny)
{
    basegfx::BGradient aBGradient;

    if (rAny.has<css::awt::Gradient2>())
    {
        // we can use awt::Gradient2 directly
        css::awt::Gradient2 aGradient2;
        rAny >>= aGradient2;

        fillFromGradient2(aBGradient, aGradient2);
    }
    else if (rAny.has<css::awt::Gradient>())
    {
        // use awt::Gradient
        css::awt::Gradient aGradient;
        rAny >>= aGradient;

        // set values
        aBGradient.SetGradientStyle(aGradient.Style);
        aBGradient.SetAngle(Degree10(aGradient.Angle));
        aBGradient.SetBorder(aGradient.Border);
        aBGradient.SetXOffset(aGradient.XOffset);
        aBGradient.SetYOffset(aGradient.YOffset);
        aBGradient.SetStartIntens(aGradient.StartIntensity);
        aBGradient.SetEndIntens(aGradient.EndIntensity);
        aBGradient.SetSteps(aGradient.StepCount);

        basegfx::BColorStops aColorStops{
            basegfx::BColorStop(0.0, ::Color(ColorTransparency, aGradient.StartColor).getBColor()),
            basegfx::BColorStop(1.0, ::Color(ColorTransparency, aGradient.EndColor).getBColor())
        };

        aBGradient.SetColorStops(aColorStops);
    }

    return aBGradient;
}

} // end model::gradient

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
