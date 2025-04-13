/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <ChartColorPaletteHelper.hxx>
#include <ChartModel.hxx>
#include <sal/log.hxx>
#include <docmodel/uno/UnoTheme.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>

namespace chart
{
constexpr tools::Long BORDER = ChartColorPaletteLayout::ItemBorder;
constexpr tools::Long SIZE = ChartColorPaletteLayout::ItemSize;

namespace
{
void decreaseLuminance(Color& color)
{
    const sal_uInt8 nLumDiff = std::floor(color.GetLuminance() * 0.40);
    color.DecreaseLuminance(nLumDiff);
}
} // end of unnamed namespace

ChartColorPaletteHelper::ChartColorPaletteHelper(const std::shared_ptr<model::Theme>& pTheme)
{
    createBasePaletteFromTheme(pTheme);
}

void ChartColorPaletteHelper::createBasePaletteFromTheme(
    const std::shared_ptr<model::Theme>& pTheme)
{
    if (pTheme)
    {
        mBasePalette[0] = pTheme->GetColor(model::ThemeColorType::Accent1);
        mBasePalette[1] = pTheme->GetColor(model::ThemeColorType::Accent2);
        mBasePalette[2] = pTheme->GetColor(model::ThemeColorType::Accent3);
        mBasePalette[3] = pTheme->GetColor(model::ThemeColorType::Accent4);
        mBasePalette[4] = pTheme->GetColor(model::ThemeColorType::Accent5);
        mBasePalette[5] = pTheme->GetColor(model::ThemeColorType::Accent6);
    }
    else // default palette
    {
        SAL_WARN("chart2",
                 "ChartColorPaletteHelper::createBasePaletteFromTheme: no valid theme provided");
        mBasePalette[0] = Color::STRtoRGB(u"18A303");
        mBasePalette[1] = Color::STRtoRGB(u"0369A3");
        mBasePalette[2] = Color::STRtoRGB(u"A33E03");
        mBasePalette[3] = Color::STRtoRGB(u"8E03A3");
        mBasePalette[4] = Color::STRtoRGB(u"C99C00");
        mBasePalette[5] = Color::STRtoRGB(u"C9211E");
    }
}

ChartColorPalette ChartColorPaletteHelper::getColorPalette(const ChartColorPaletteType eType,
                                                           const sal_uInt32 nIndex) const
{
    switch (eType)
    {
        case ChartColorPaletteType::Colorful:
            return createColorfulPalette(nIndex);
        case ChartColorPaletteType::Monochromatic:
            return createMonotonicPalette(nIndex);
        default:
            SAL_WARN("chart2",
                     "ChartColorPaletteHelper::getColorPalette: unknown palette type requested");
            return mBasePalette;
    }
}

ChartColorPalette ChartColorPaletteHelper::createColorfulPalette(const sal_uInt32 nIndex) const
{
    if (nIndex >= ColorfulPaletteSize)
    {
        SAL_WARN("chart2",
                 "ChartColorPaletteHelper::createColorfulPalette: wrong index: " << nIndex);
        return mBasePalette;
    }

    switch (nIndex)
    {
        default:
        case 0:
            return mBasePalette;
        case 1:
            return createColorfulPaletteImpl(0, 2, 4);
        case 2:
            return createColorfulPaletteImpl(1, 3, 5);
        case 3:
            return createColorfulPaletteImpl(5, 4, 3);
    }
}

ChartColorPalette ChartColorPaletteHelper::createColorfulPaletteImpl(const size_t nIdx1,
                                                                     const size_t nIdx2,
                                                                     const size_t nIdx3) const
{
    ChartColorPalette colorPalette = {
        mBasePalette[nIdx1], mBasePalette[nIdx2], mBasePalette[nIdx3],
        mBasePalette[nIdx1], mBasePalette[nIdx2], mBasePalette[nIdx3],
    };
    decreaseLuminance(colorPalette[3]);
    decreaseLuminance(colorPalette[4]);
    decreaseLuminance(colorPalette[5]);
    return colorPalette;
}

ChartColorPalette ChartColorPaletteHelper::createMonotonicPalette(sal_uInt32 nIndex) const
{
    if (nIndex >= mBasePalette.size())
    {
        SAL_WARN("chart2",
                 "ChartColorPaletteHelper::createMonotonicPalette: wrong index: " << nIndex);
        nIndex = 0;
    }

    ChartColorPalette colorPalette = {
        mBasePalette[nIndex], mBasePalette[nIndex], mBasePalette[nIndex],
        mBasePalette[nIndex], mBasePalette[nIndex], mBasePalette[nIndex],
    };

    for (size_t i = 0; i < colorPalette.size(); ++i)
    {
        Color& color = colorPalette[i];
        sal_uInt16 nH, nS, nB;
        color.RGBtoHSB(nH, nS, nB);
        SAL_INFO("chart2", "createMonotonicPalette: i: " << i << ": before: nH: " << nH
                                                         << ", nS: " << nS << ", nB: " << nB);
        nB += (static_cast<int>(i) - 2) * std::round(nB * 0.13);
        nB = std::clamp(nB, sal_uInt16{ 0 }, sal_uInt16{ 100 });

        if (i < 2)
            nS += (2 - i) * std::round(nS * 0.013);
        else if (i > 2)
            nS = nS / (2 * (i - 2));
        nS = std::clamp(nS, sal_uInt16{ 0 }, sal_uInt16{ 100 });

        SAL_INFO("chart2", "createMonotonicPalette: i: " << i << ": after: nH: " << nH
                                                         << ", nS: " << nS << ", nB: " << nB);
        color = Color::HSBtoRGB(nH, nS, nB);
    }

    return colorPalette;
}

void ChartColorPaletteHelper::renderColorPalette(OutputDevice* pDev,
                                                 const tools::Rectangle& rDrawArea,
                                                 const ChartColorPalette& rColorSet,
                                                 const bool bDrawItemBorder)
{
    const Point aPosition = rDrawArea.GetPos();
    const Size aSize = rDrawArea.GetSize();

    static constexpr Size aMin(BORDER * 6 + SIZE * ChartColorPaletteSize / 2,
                               BORDER * 5 + SIZE * 2);

    const tools::Long startX = (aSize.Width() / 2.0) - (aMin.Width() / 2.0) + BORDER;
    const tools::Long startY = (aSize.Height() / 2.0) - (aMin.Height() / 2.0) + BORDER;
    tools::Long x = BORDER;
    static constexpr tools::Long y1 = BORDER;
    static constexpr tools::Long y2 = y1 + SIZE + BORDER;

    pDev->SetFillColor();

    if (bDrawItemBorder)
    {
        pDev->SetLineColor(COL_BLACK);
        pDev->DrawRect(tools::Rectangle(aPosition, aSize));
    }

    for (sal_uInt32 i = 0; i < ChartColorPaletteSize / 2; i += 1)
    {
        pDev->SetFillColor(rColorSet[i]);
        pDev->DrawRect(tools::Rectangle(
            Point(aPosition.X() + x + startX, aPosition.Y() + startY + y1), Size(SIZE, SIZE)));

        pDev->SetFillColor(rColorSet[i + ChartColorPaletteSize / 2]);
        pDev->DrawRect(tools::Rectangle(
            Point(aPosition.X() + x + startX, aPosition.Y() + startY + y2), Size(SIZE, SIZE)));

        x += SIZE + BORDER;
    }
}

void ChartColorPaletteHelper::renderNoPalette(OutputDevice* pDev, const tools::Rectangle& rDrawArea)
{
    pDev->SetLineColor(COL_BLACK);
    pDev->SetFillColor(Application::GetSettings().GetStyleSettings().GetFaceColor());
    pDev->DrawRect(rDrawArea);
}
} // end of namespace ::chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
