/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "tp_ChartGradients.hxx"

#include <ChartGradientPaletteHelper.hxx>
#include <ChartModel.hxx>
#include <chartview/ChartSfxItemIds.hxx>
#include <o3tl/enumrange.hxx>
#include <svx/chrtitem.hxx>
#include <vcl/svapp.hxx>

namespace chart
{
ChartGradientsTabPage::ChartGradientsTabPage(weld::Container* pPage,
                                             weld::DialogController* pController,
                                             const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, "modules/schart/ui/tp_ChartGradients.ui", "tp_ChartGradients",
                 &rInAttrs)
    , mxLightPalettes(new ChartGradientPalettes(*m_xBuilder, "light_palettes", "lightwin"))
    , mxDarkPalettes(new ChartGradientPalettes(*m_xBuilder, "dark_palettes", "darkwin"))
{
    mxLightPalettes->SetSelectHdl(LINK(this, ChartGradientsTabPage, SelectLightPaletteHdl));

    mxDarkPalettes->SetSelectHdl(LINK(this, ChartGradientsTabPage, SelectDarkPaletteHdl));
}

ChartGradientsTabPage::~ChartGradientsTabPage()
{
    mxLightPalettes.reset();
    mxDarkPalettes.reset();
}

std::unique_ptr<SfxTabPage> ChartGradientsTabPage::Create(weld::Container* pPage,
                                                          weld::DialogController* pController,
                                                          const SfxItemSet* rInAttrs)
{
    return std::make_unique<ChartGradientsTabPage>(pPage, pController, *rInAttrs);
}

void ChartGradientsTabPage::init(const rtl::Reference<ChartModel>& xChartModel)
{
    assert(xChartModel);
    mxChartModel = xChartModel;
    meCurrentVariation = mxChartModel->getGradientPaletteVariation();
    meCurrentType = mxChartModel->getGradientPaletteType();
    mxHelper = std::make_unique<ChartGradientPaletteHelper>(
        mxChartModel->getDataSeriesColorsForGradient(false));

    initGradientPalettes();
    selectItem(meCurrentVariation, meCurrentType);
}

void ChartGradientsTabPage::initGradientPalettes() const
{
    if (!mxHelper)
        return;

    for (auto type : o3tl::enumrange<ChartGradientType>())
    {
        mxLightPalettes->insert(
            mxHelper->getGradientSample(ChartGradientVariation::LightVariation, type));
        mxDarkPalettes->insert(
            mxHelper->getGradientSample(ChartGradientVariation::DarkVariation, type));
    }
    mxLightPalettes->Fill();
    mxDarkPalettes->Fill();
}

bool ChartGradientsTabPage::isCurrentGradientPreset(ChartGradientVariation eVariation,
                                                    ChartGradientType eType) const
{
    return eVariation == meCurrentVariation && eType == meCurrentType;
}

void ChartGradientsTabPage::selectItem(const ChartGradientVariation eVariation,
                                       const ChartGradientType eType) const
{
    const sal_uInt16 nIndex = static_cast<sal_uInt16>(eType) + 1;

    switch (eVariation)
    {
        default:
        case ChartGradientVariation::Unknown:
            mxLightPalettes->SetNoSelection();
            mxDarkPalettes->SetNoSelection();
            break;
        case ChartGradientVariation::LightVariation:
            mxLightPalettes->SelectItem(nIndex);
            break;
        case ChartGradientVariation::DarkVariation:
            mxDarkPalettes->SelectItem(nIndex);
            break;
    }
}

bool ChartGradientsTabPage::FillItemSet(SfxItemSet* pOutAttrs)
{
    ChartGradientVariation eVariation = ChartGradientVariation::Unknown;
    ChartGradientType eType = ChartGradientType::Invalid;
    sal_uInt32 nIndex = 0;

    if (!mxLightPalettes->IsNoSelection())
    {
        eVariation = ChartGradientVariation::LightVariation;
        nIndex = mxLightPalettes->GetSelectedItemId() - 1;
    }
    else if (!mxDarkPalettes->IsNoSelection())
    {
        eVariation = ChartGradientVariation::DarkVariation;
        nIndex = mxDarkPalettes->GetSelectedItemId() - 1;
    }

    if (nIndex != static_cast<sal_uInt32>(-1))
        eType = static_cast<ChartGradientType>(nIndex);

    if (!isCurrentGradientPreset(eVariation, eType))
        pOutAttrs->Put(SvxChartGradientPresetItem(eVariation, eType, SCHATTR_GRADIENT_PRESET));

    return true;
}

void ChartGradientsTabPage::Reset(const SfxItemSet*)
{
    selectItem(mxChartModel->getGradientPaletteVariation(), mxChartModel->getGradientPaletteType());
}

DeactivateRC ChartGradientsTabPage::DeactivatePage(SfxItemSet* pItemSet)
{
    if (pItemSet)
        FillItemSet(pItemSet);

    return DeactivateRC::LeavePage;
}

IMPL_LINK_NOARG(ChartGradientsTabPage, SelectLightPaletteHdl, weld::IconView&, bool)
{
    sal_uInt32 nIndex = GetSelectedItem(mxLightPalettes);
    if (nIndex != static_cast<sal_uInt32>(-1))
    {
        mxDarkPalettes->SetNoSelection();
    }
    return true;
}

IMPL_LINK_NOARG(ChartGradientsTabPage, SelectDarkPaletteHdl, weld::IconView&, bool)
{
    sal_uInt32 nIndex = GetSelectedItem(mxDarkPalettes);
    if (nIndex != static_cast<sal_uInt32>(-1))
    {
        mxLightPalettes->SetNoSelection();
    }
    return true;
}

sal_uInt32
ChartGradientsTabPage::GetSelectedItem(const std::unique_ptr<ChartGradientPalettes>& xPalettes)
{
    const sal_uInt32 nItemId = xPalettes->GetSelectedItemId();
    if (!nItemId)
        return static_cast<sal_uInt32>(-1);

    const sal_uInt32 nIndex = nItemId - 1;

    if (xPalettes->getPalette(nIndex))
    {
        return nIndex;
    }
    return static_cast<sal_uInt32>(-1);
}
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
