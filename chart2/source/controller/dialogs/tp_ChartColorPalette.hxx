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

#include <sfx2/tabdlg.hxx>
#include <ChartColorPalettes.hxx>

namespace chart
{
class ChartModel;
class ChartColorPaletteHelper;

// This tab page is used to set up data series colors according to a given color palette
// It is present in the Chart Format Dialog for several chart elements.
class ChartColorPaletteTabPage final : public SfxTabPage
{
public:
    ChartColorPaletteTabPage(weld::Container* pPage, weld::DialogController* pController,
                             const SfxItemSet& rInAttrs);
    ~ChartColorPaletteTabPage() override;

    static std::unique_ptr<SfxTabPage>
    Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rInAttrs);

    bool FillItemSet(SfxItemSet* pOutAttrs) override;
    void Reset(const SfxItemSet* rInAttrs) override;
    DeactivateRC DeactivatePage(SfxItemSet* pItemSet) override;

    void init(const rtl::Reference<ChartModel>& xChartModel);

private:
    void selectItem(ChartColorPaletteType eType, sal_uInt32 nIndex) const;
    void initColorPalettes() const;

private:
    rtl::Reference<ChartModel> mxChartModel;
    std::unique_ptr<ChartColorPaletteHelper> mxHelper;
    std::unique_ptr<ChartColorPalettes> mxColorfulPalettes;
    std::unique_ptr<ChartColorPalettes> mxMonoPalettes;

    DECL_LINK(SelectColorfulPaletteHdl, weld::IconView&, bool);
    DECL_LINK(SelectMonoPaletteHdl, weld::IconView&, bool);
    static sal_uInt32 SelectPaletteHdl(const std::unique_ptr<ChartColorPalettes>& xValueSet);
};
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
