/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/attribute/fillgradientattribute.hxx>
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>

#include <ChartGradientPaletteHelper.hxx>
#include <ChartModel.hxx>
#include <sal/log.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>

#include <frozen/bits/defines.h>
#include <frozen/bits/elsa_std.h>
#include <frozen/unordered_map.h>

constexpr auto ChartGradientTypes
    = frozen::make_unordered_map<ChartGradientType, ChartGradientProperties>({
        { ChartGradientType::TopLeftToBottomRight,
          ChartGradientProperties{ css::awt::GradientStyle::GradientStyle_LINEAR, -1350, 0, 0 } },
        { ChartGradientType::LinearDown,
          ChartGradientProperties{ css::awt::GradientStyle::GradientStyle_LINEAR, 1800, 0, 0 } },
        { ChartGradientType::TopRightToBottomLeft,
          ChartGradientProperties{ css::awt::GradientStyle::GradientStyle_LINEAR, 1350, 100, 0 } },
        { ChartGradientType::FromBottomRightCorner,
          ChartGradientProperties{ css::awt::GradientStyle::GradientStyle_RADIAL, 0, 100, 100 } },
        { ChartGradientType::FromBottomLeftCorner,
          ChartGradientProperties{ css::awt::GradientStyle::GradientStyle_RADIAL, 0, 0, 100 } },
        { ChartGradientType::LinearRight,
          ChartGradientProperties{ css::awt::GradientStyle::GradientStyle_LINEAR, -900, 0, 0 } },
        { ChartGradientType::FromCenter,
          ChartGradientProperties{ css::awt::GradientStyle::GradientStyle_RADIAL, 0, 50, 50 } },
        { ChartGradientType::LinearLeft,
          ChartGradientProperties{ css::awt::GradientStyle::GradientStyle_LINEAR, 900, 0, 0 } },
        { ChartGradientType::FromTopRightCorner,
          ChartGradientProperties{ css::awt::GradientStyle::GradientStyle_RADIAL, 0, 100, 0 } },
        { ChartGradientType::FromTopLeftCorner,
          ChartGradientProperties{ css::awt::GradientStyle::GradientStyle_RADIAL, 0, 0, 0 } },
        { ChartGradientType::BottomLeftToTopRight,
          ChartGradientProperties{ css::awt::GradientStyle::GradientStyle_LINEAR, -450, 0, 100 } },
        { ChartGradientType::LinearUp,
          ChartGradientProperties{ css::awt::GradientStyle::GradientStyle_LINEAR, 0, 0, 1000 } },
        { ChartGradientType::BottomRightToTopLeft,
          ChartGradientProperties{ css::awt::GradientStyle::GradientStyle_LINEAR, 450, 100, 100 } },
    });

namespace chart
{
constexpr tools::Long BORDER = ChartGradientPaletteLayout::ItemBorder;
constexpr tools::Long SIZE = ChartGradientPaletteLayout::ItemSize;

namespace
{
// Clamp helpers
double clamp01(double v) { return std::max(0.0, std::min(1.0, v)); }
double srgbToLinear(double u)
{
    u = clamp01(u);
    if (u <= 0.04045)
        return u / 12.92;
    return std::pow((u + 0.055) / 1.055, 2.4);
}
double linearToSrgb(double u)
{
    if (u <= 0.0031308)
        return 12.92 * u;
    return 1.055 * std::pow(u, 1.0 / 2.4) - 0.055;
}

struct RGBd
{
    double r, g, b;
};

RGBd hslToRgb(double hDeg, double s, double l)
{
    s = clamp01(s);
    l = clamp01(l);
    const double c = (1.0 - std::fabs(2.0 * l - 1.0)) * s;
    double h = std::fmod(hDeg / 60.0, 6.0);
    if (h < 0)
        h += 6.0;
    const double x = c * (1.0 - std::fabs(std::fmod(h, 2.0) - 1.0));

    double r1 = 0, g1 = 0, b1 = 0;
    if (0 <= h && h < 1)
    {
        r1 = c;
        g1 = x;
        b1 = 0;
    }
    else if (1 <= h && h < 2)
    {
        r1 = x;
        g1 = c;
        b1 = 0;
    }
    else if (2 <= h && h < 3)
    {
        r1 = 0;
        g1 = c;
        b1 = x;
    }
    else if (3 <= h && h < 4)
    {
        r1 = 0;
        g1 = x;
        b1 = c;
    }
    else if (4 <= h && h < 5)
    {
        r1 = x;
        g1 = 0;
        b1 = c;
    }
    else
    {
        r1 = c;
        g1 = 0;
        b1 = x;
    }

    const double m = l - 0.5 * c;
    return { clamp01(r1 + m), clamp01(g1 + m), clamp01(b1 + m) };
}

void rgbToHsl(const RGBd& rgb, double& hDeg, double& s, double& l)
{
    const double r = clamp01(rgb.r), g = clamp01(rgb.g), b = clamp01(rgb.b);
    const double maxc = std::max(r, std::max(g, b));
    const double minc = std::min(r, std::min(g, b));
    const double d = maxc - minc;
    l = 0.5 * (maxc + minc);
    if (d == 0.0)
    {
        hDeg = 0.0;
        s = 0.0;
        return;
    }
    s = d / (1.0 - std::fabs(2.0 * l - 1.0));
    double h;
    if (maxc == r)
    {
        h = (g - b) / d + (g < b ? 6.0 : 0.0);
    }
    else if (maxc == g)
    {
        h = (b - r) / d + 2.0;
    }
    else
    {
        h = (r - g) / d + 4.0;
    }
    hDeg = std::fmod(h * 60.0, 360.0);
    if (hDeg < 0)
        hDeg += 360.0;
}

RGBd applyTintLinearRgb(const RGBd& rgb, double t)
{
    double rl = srgbToLinear(rgb.r);
    double gl = srgbToLinear(rgb.g);
    double bl = srgbToLinear(rgb.b);
    rl = rl + (1.0 - rl) * t;
    gl = gl + (1.0 - gl) * t;
    bl = bl + (1.0 - bl) * t;
    return { clamp01(linearToSrgb(rl)), clamp01(linearToSrgb(gl)), clamp01(linearToSrgb(bl)) };
}

RGBd applySatModHsl(const RGBd& rgb, double satMod)
{
    double h, s, l;
    rgbToHsl(rgb, h, s, l);
    s = clamp01(s * satMod);
    return hslToRgb(h, s, l);
}

// Convert tools::Color -> RGBd [0..1]
RGBd colorToRgbd(const Color& c)
{
    return { c.GetRed() / 255.0, c.GetGreen() / 255.0, c.GetBlue() / 255.0 };
}

basegfx::BColor toBColor(const RGBd& d) { return { clamp01(d.r), clamp01(d.g), clamp01(d.b) }; }

// Compute 3 light-variation stops as basegfx::BColorStops (offsets 0.0, 0.5, 1.0)
basegfx::BColorStops ComputeLightVariationStops(const Color& rBase)
{
    // Tuned constants (match Python you validated)
    static constexpr double TINTS[3] = { 0.235, 0.445, 0.660 }; // center→middle→edge
    static constexpr double SATMOD = 1.6; // a:satMod
    static constexpr double K_L = 0.16; // darkness-driven lift
    static constexpr double L0_REF = 0.55; // small nudge above 0.5
    static constexpr double K_S = 0.07; // high-sat-driven lift
    static constexpr double S0_REF = 0.80; // engage for high S
    static constexpr double STOP_M[3] = { 1.65, 1.30, 0.95 }; // per-stop scaling

    // Base color as sRGB [0..1]
    const RGBd baseRgb = colorToRgbd(rBase);

    // Base HSL (for L0/S0 gating)
    double H0, S0, L0;
    rgbToHsl(baseRgb, H0, S0, L0);

    basegfx::BColorStops stops;
    stops.reserve(3);

    for (int i = 0; i < 3; ++i)
    {
        const double t = TINTS[i];
        const double mt = STOP_M[i];

        // 1) Tint in linear-light sRGB toward white
        RGBd tinted = applyTintLinearRgb(baseRgb, t);

        // 2) Post-tint lightness lift in HSL (no headroom term)
        double h, s, l;
        rgbToHsl(tinted, h, s, l);
        const double darkness = std::max(0.0, L0_REF - L0);
        const double satLift = std::max(0.0, S0_REF - S0);
        if (darkness > 0.0 && t > 0.0)
        {
            l = clamp01(l + mt * (K_L * darkness) + K_S * satLift);
        }
        tinted = hslToRgb(h, s, l);

        // 3) Apply saturation modulation
        RGBd finalRgb = applySatModHsl(tinted, SATMOD);

        // 4) Add stop (offsets 0.0, 0.5, 1.0)
        const double offset = i == 0 ? 1.0 : (i == 1 ? 0.5 : 0.0);
        stops.emplace_back(offset, toBColor(finalRgb));
    }

    return stops;
}

// Shade toward black in linear-light sRGB
RGBd applyShadeLinearRgb(const RGBd& rgb, double shadeFactor)
{
    const double rl = srgbToLinear(rgb.r) * shadeFactor;
    const double gl = srgbToLinear(rgb.g) * shadeFactor;
    const double bl = srgbToLinear(rgb.b) * shadeFactor;
    return { clamp01(linearToSrgb(rl)), clamp01(linearToSrgb(gl)), clamp01(linearToSrgb(bl)) };
}

// Given a base tools::Color, return the 3 “Dark variation” stops as basegfx::BColorStops
// at offsets 0.0 (center), 0.5 (mid), 1.0 (edge).
basegfx::BColorStops ComputeDarkVariationStops(const Color& rBase)
{
    // DrawingML parameters from the provided gradFill
    static constexpr double kShade[3] = { 0.300, 0.675, 1.000 }; // center, middle, edge
    static constexpr double kSatMod = 1.15;

    const RGBd base = colorToRgbd(rBase);

    basegfx::BColorStops stops;
    stops.reserve(3);

    for (int i = 0; i < 3; ++i)
    {
        const double f = kShade[i];

        // 1) shade in linear-light sRGB
        RGBd shaded = applyShadeLinearRgb(base, f);

        // 2) saturation modulation in HSL
        RGBd out = applySatModHsl(shaded, kSatMod);

        // 3) append stop at 0.0, 0.5, 1.0
        const double offset = i == 0 ? 1.0 : (i == 1 ? 0.5 : 0.0);
        stops.emplace_back(offset, basegfx::BColor(out.r, out.g, out.b));
    }

    return stops;
}

void drawBGradient(OutputDevice& rOutDev, const tools::Rectangle& rRect,
                   const basegfx::BGradient& rGrad)
{
    // 1) Rectangle → B2DRange (cover bottom/right pixel)
    const double x0 = rRect.Left();
    const double y0 = rRect.Top();
    const double x1 = rRect.Right() + 1.0;
    const double y1 = rRect.Bottom() + 1.0;
    basegfx::B2DRange aRange(x0, y0, x1, y1);

    // 2) FillGradientAttribute expects:
    //    (css::awt::GradientStyle eStyle, double fBorder, double fOffsetX,
    //     double fOffsetY, double fAngleInRadians, const BColorStops&, sal_uInt16 nSteps)
    const css::awt::GradientStyle eStyle = rGrad.GetGradientStyle(); // already UNO enum
    const double fBorder = static_cast<double>(rGrad.GetBorder()) / 100.0; // normalize [0..1]
    const double fOffsetX = rGrad.GetXOffset() / 100.0;
    const double fOffsetY = rGrad.GetYOffset() / 100.0;

    // Convert Degree10 (tenths of degree) to radians
    const double angleDeg10 = static_cast<sal_Int16>(rGrad.GetAngle()); // e.g. 900 => 90.0°
    const double fAngleRad = (angleDeg10 / 10.0) * (M_PI / 180.0); // radians

    const basegfx::BColorStops& rStops = rGrad.GetColorStops();
    const sal_uInt16 nSteps = rGrad.GetSteps();

    const drawinglayer::attribute::FillGradientAttribute aFillAttr(
        eStyle, fBorder, fOffsetX, fOffsetY, fAngleRad, rStops, nSteps);

    // 3) Primitive container
    drawinglayer::primitive2d::Primitive2DContainer aSeq;
    aSeq.push_back(new drawinglayer::primitive2d::FillGradientPrimitive2D(aRange, aFillAttr));

    // 4) Minimal view info and processor
    const drawinglayer::geometry::ViewInformation2D aViewInfo;
    const std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor(
        drawinglayer::processor2d::createProcessor2DFromOutputDevice(rOutDev, aViewInfo));

    if (pProcessor)
        pProcessor->process(aSeq);
}
} // anonymous namespace end

ChartGradientPaletteHelper::ChartGradientPaletteHelper(const std::vector<Color>& aColorSet)
    : maColorSet(aColorSet)
{
}

const Color& ChartGradientPaletteHelper::getSampleColor() const
{
    for (const Color& rColor : maColorSet)
    {
        if (rColor != ChartGradientPresetInvalidColor)
            return rColor;
    }
    return ChartGradientPresetDefaultColor;
}

basegfx::BGradient ChartGradientPaletteHelper::createItem(const basegfx::BColorStops& rColorStops,
                                                          const ChartGradientType eType)
{
    const ChartGradientProperties& rType = ChartGradientTypes.at(eType);

    basegfx::BGradient aGradient(rColorStops, rType.style);
    aGradient.SetAngle(Degree10(rType.angle));
    aGradient.SetXOffset(rType.offsetX);
    aGradient.SetYOffset(rType.offsetY);
    return aGradient;
}

basegfx::BGradient
ChartGradientPaletteHelper::getGradientSample(const ChartGradientVariation eVariation,
                                              const ChartGradientType eType) const
{
    switch (eVariation)
    {
        case ChartGradientVariation::LightVariation:
        {
            const auto aStopColors = ComputeLightVariationStops(getSampleColor());
            return createItem(aStopColors, eType);
        }
        case ChartGradientVariation::DarkVariation:
        {
            const auto aStopColors = ComputeDarkVariationStops(getSampleColor());
            return createItem(aStopColors, eType);
        }
        default:
            SAL_WARN("chart2", "ChartGradientPaletteHelper::getGradientSample: unknown gradient "
                               "variation requested");
            return {};
    }
}

ChartGradientPalette
ChartGradientPaletteHelper::getGradientPalette(const ChartGradientVariation eVariation,
                                               const ChartGradientType eType) const
{
    ChartGradientPalette aPalette;
    switch (eVariation)
    {
        case ChartGradientVariation::LightVariation:
            for (const Color& rColor : maColorSet)
            {
                const basegfx::BColorStops aStopColors
                    = rColor == ChartGradientPresetInvalidColor
                          ? ComputeLightVariationStops(ChartGradientPresetDefaultColor)
                          : ComputeLightVariationStops(rColor);

                aPalette.push_back(createItem(aStopColors, eType));
            }
            break;
        case ChartGradientVariation::DarkVariation:
            for (const Color& rColor : maColorSet)
            {
                const basegfx::BColorStops aStopColors
                    = rColor == ChartGradientPresetInvalidColor
                          ? ComputeDarkVariationStops(ChartGradientPresetDefaultColor)
                          : ComputeDarkVariationStops(rColor);

                aPalette.push_back(createItem(aStopColors, eType));
            }
            break;
        default:
            SAL_WARN(
                "chart2",
                "ChartGradientPaletteHelper::getGradientPalette: unknown palette type requested");
            return {};
    }
    return aPalette;
}

void ChartGradientPaletteHelper::renderGradientItem(OutputDevice* pDev,
                                                    const tools::Rectangle& rDrawArea,
                                                    const basegfx::BGradient& rGradient,
                                                    const bool bDrawItemBorder)
{
    const Point aPosition = rDrawArea.GetPos();
    const Size aSize = rDrawArea.GetSize();

    static constexpr Size aMin(2 * BORDER + SIZE, 2 * BORDER + SIZE);

    const tools::Long startX = (aSize.Width() - aMin.Width()) / 2.0;
    const tools::Long startY = (aSize.Height() - aMin.Height()) / 2.0;
    static constexpr tools::Long x = BORDER;
    static constexpr tools::Long y = BORDER;

    if (bDrawItemBorder)
    {
        const tools::Rectangle aFrame(Point(aPosition.X() + startX, aPosition.Y() + startY), aMin);
        pDev->SetFillColor(COL_LIGHTGRAY);
        pDev->DrawRect(aFrame);
    }

    pDev->SetFillColor();

    const tools::Rectangle aRect(Point(aPosition.X() + startX + x, aPosition.Y() + startY + y),
                                 Size(SIZE, SIZE));

    drawBGradient(*pDev, aRect, rGradient);
}

void ChartGradientPaletteHelper::renderNoGradient(OutputDevice* pDev,
                                                  const tools::Rectangle& rDrawArea)
{
    pDev->SetLineColor(COL_BLACK);
    pDev->SetFillColor(Application::GetSettings().GetStyleSettings().GetFaceColor());
    pDev->DrawRect(rDrawArea);
}
} // end of namespace ::chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
