/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <sfx2/tabdlg.hxx>
#include <ChartGradientPalettes.hxx>

namespace chart
{
class ChartModel;
class ChartGradientPaletteHelper;

// This tab page is used to apply gradients to data series according to a given selected gradient.
// It is present in the Chart Format Dialog for several chart elements.
class ChartGradientsTabPage final : public SfxTabPage
{
public:
    ChartGradientsTabPage(weld::Container* pPage, weld::DialogController* pController,
                          const SfxItemSet& rInAttrs);
    ~ChartGradientsTabPage() override;

    static std::unique_ptr<SfxTabPage>
    Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rInAttrs);

    bool FillItemSet(SfxItemSet* pOutAttrs) override;
    void Reset(const SfxItemSet* rInAttrs) override;
    DeactivateRC DeactivatePage(SfxItemSet* pItemSet) override;

    void init(const rtl::Reference<ChartModel>& xChartModel);

private:
    bool isCurrentGradientPreset(ChartGradientVariation eVariation, ChartGradientType eType) const;
    void selectItem(ChartGradientVariation eVariation, ChartGradientType eType) const;
    void initGradientPalettes() const;

private:
    rtl::Reference<ChartModel> mxChartModel;
    std::unique_ptr<ChartGradientPaletteHelper> mxHelper;
    std::unique_ptr<ChartGradientPalettes> mxLightPalettes;
    std::unique_ptr<ChartGradientPalettes> mxDarkPalettes;
    ChartGradientVariation meCurrentVariation;
    ChartGradientType meCurrentType;

    DECL_LINK(SelectLightPaletteHdl, weld::IconView&, bool);
    DECL_LINK(SelectDarkPaletteHdl, weld::IconView&, bool);
    static sal_uInt32 GetSelectedItem(const std::unique_ptr<ChartGradientPalettes>& xPalettes);
};
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
