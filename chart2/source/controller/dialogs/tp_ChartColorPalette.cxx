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

#include "tp_ChartColorPalette.hxx"

#include <ChartColorPaletteHelper.hxx>
#include <ChartModel.hxx>
#include <chartview/ChartSfxItemIds.hxx>
#include <svx/chrtitem.hxx>
#include <vcl/svapp.hxx>

namespace chart
{
ChartColorPaletteTabPage::ChartColorPaletteTabPage(weld::Container* pPage,
                                                   weld::DialogController* pController,
                                                   const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, "modules/schart/ui/tp_ChartColorPalette.ui",
                 "tp_ChartColorPalette", &rInAttrs)
    , mxColorfulValueSet(new ChartColorPalettes)
    , mxColorfulValueSetWin(
          new weld::CustomWeld(*m_xBuilder, "colorful_palettes", *mxColorfulValueSet))
    , mxMonoValueSet(new ChartColorPalettes)
    , mxMonoValueSetWin(
          new weld::CustomWeld(*m_xBuilder, "monochromatic_palettes", *mxMonoValueSet))
{
    mxColorfulValueSet->SetColCount(2);
    mxColorfulValueSet->SetLineCount(2);
    mxColorfulValueSet->SetColor(Application::GetSettings().GetStyleSettings().GetFaceColor());

    mxMonoValueSet->SetColCount(2);
    mxMonoValueSet->SetLineCount(3);
    mxMonoValueSet->SetColor(Application::GetSettings().GetStyleSettings().GetFaceColor());

    mxColorfulValueSet->SetOptimalSize();
    mxColorfulValueSet->SetSelectHdl(
        LINK(this, ChartColorPaletteTabPage, SelectColorfulValueSetHdl));

    mxMonoValueSet->SetOptimalSize();
    mxMonoValueSet->SetSelectHdl(LINK(this, ChartColorPaletteTabPage, SelectMonoValueSetHdl));
}

ChartColorPaletteTabPage::~ChartColorPaletteTabPage()
{
    mxColorfulValueSetWin.reset();
    mxColorfulValueSet.reset();
    mxMonoValueSetWin.reset();
    mxMonoValueSet.reset();
}

std::unique_ptr<SfxTabPage> ChartColorPaletteTabPage::Create(weld::Container* pPage,
                                                             weld::DialogController* pController,
                                                             const SfxItemSet* rInAttrs)
{
    return std::make_unique<ChartColorPaletteTabPage>(pPage, pController, *rInAttrs);
}

void ChartColorPaletteTabPage::init(const rtl::Reference<ChartModel>& xChartModel)
{
    assert(xChartModel);
    mxChartModel = xChartModel;

    const std::shared_ptr<model::Theme> pTheme = mxChartModel->getDocumentTheme();
    mxHelper = std::make_unique<ChartColorPaletteHelper>(pTheme);

    selectItem(mxChartModel->getColorPaletteType(), mxChartModel->getColorPaletteIndex() + 1);
    initColorPalettes();
}

void ChartColorPaletteTabPage::initColorPalettes() const
{
    if (!mxHelper)
        return;
    // colorful palettes
    for (size_t i = 0; i < ChartColorPaletteHelper::ColorfulPaletteSize; ++i)
        mxColorfulValueSet->insert(mxHelper->getColorPalette(ChartColorPaletteType::Colorful, i));
    // monotonic palettes
    for (size_t i = 0; i < ChartColorPaletteHelper::MonotonicPaletteSize; ++i)
        mxMonoValueSet->insert(mxHelper->getColorPalette(ChartColorPaletteType::Monochromatic, i));
}

void ChartColorPaletteTabPage::selectItem(const ChartColorPaletteType eType,
                                          const sal_uInt32 nIndex) const
{
    switch (eType)
    {
        default:
        case ChartColorPaletteType::Unknown:
            mxColorfulValueSet->SetNoSelection();
            mxMonoValueSet->SetNoSelection();
            break;
        case ChartColorPaletteType::Colorful:
            mxMonoValueSet->SetNoSelection();
            mxColorfulValueSet->SelectItem(nIndex);
            break;
        case ChartColorPaletteType::Monochromatic:
            mxColorfulValueSet->SetNoSelection();
            mxMonoValueSet->SelectItem(nIndex);
            break;
    }
}

bool ChartColorPaletteTabPage::FillItemSet(SfxItemSet* pOutAttrs)
{
    ChartColorPaletteType eType = ChartColorPaletteType::Unknown;
    sal_uInt32 nIndex = 0;

    if (!mxColorfulValueSet->IsNoSelection())
    {
        eType = ChartColorPaletteType::Colorful;
        nIndex = mxColorfulValueSet->GetSelectedItemId() - 1;
    }
    else if (!mxMonoValueSet->IsNoSelection())
    {
        eType = ChartColorPaletteType::Monochromatic;
        nIndex = mxMonoValueSet->GetSelectedItemId() - 1;
    }

    pOutAttrs->Put(SvxChartColorPaletteItem(eType, nIndex, SCHATTR_COLOR_PALETTE));

    return true;
}

void ChartColorPaletteTabPage::Reset(const SfxItemSet*)
{
    selectItem(mxChartModel->getColorPaletteType(), mxChartModel->getColorPaletteIndex() + 1);
}

DeactivateRC ChartColorPaletteTabPage::DeactivatePage(SfxItemSet* pItemSet)
{
    if (pItemSet)
        FillItemSet(pItemSet);

    return DeactivateRC::LeavePage;
}

IMPL_LINK_NOARG(ChartColorPaletteTabPage, SelectColorfulValueSetHdl, ValueSet*, void)
{
    sal_uInt32 nIndex = SelectValueSetHdl(mxColorfulValueSet);
    if (nIndex != static_cast<sal_uInt32>(-1))
    {
        mxMonoValueSet->SetNoSelection();
    }
}

IMPL_LINK_NOARG(ChartColorPaletteTabPage, SelectMonoValueSetHdl, ValueSet*, void)
{
    sal_uInt32 nIndex = SelectValueSetHdl(mxMonoValueSet);
    if (nIndex != static_cast<sal_uInt32>(-1))
    {
        mxColorfulValueSet->SetNoSelection();
    }
}

sal_uInt32
ChartColorPaletteTabPage::SelectValueSetHdl(const std::unique_ptr<ChartColorPalettes>& xValueSet)
{
    const sal_uInt32 nItemId = xValueSet->GetSelectedItemId();

    if (!nItemId)
        return -1;

    const sal_uInt32 nIndex = nItemId - 1;

    if (xValueSet->getPalette(nIndex))
    {
        return nIndex;
    }
    return -1;
}
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
