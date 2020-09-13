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
#include <svtools/valueset.hxx>
#include <svx/xtable.hxx>
#include <svx/PaletteManager.hxx>
#include <vcl/customweld.hxx>

#include "cfgchart.hxx"

typedef std::vector<Color> ImpColorList;

class SvxDefaultColorOptPage : public SfxTabPage
{
private:
    std::unique_ptr<SvxChartOptions>        m_SvxChartOptionsUniquePtr;
    // no reason to use a cloned SfxItem here (SvxChartColorTableItem)
    // that just leads to non-const SfxItem and potential trouble
    std::unique_ptr<SvxChartColorTable>     m_SvxChartColorTableUniquePtr;

    ImpColorList            aColorList;
    PaletteManager          aPaletteManager;

    std::unique_ptr<weld::TreeView> m_xLbChartColors;
    std::unique_ptr<weld::ComboBox> m_xLbPaletteSelector;
    std::unique_ptr<weld::Button> m_xPBDefault;
    std::unique_ptr<weld::Button> m_xPBAdd;
    std::unique_ptr<weld::Button> m_xPBRemove;
    std::unique_ptr<SvxColorValueSet> m_xValSetColorBox;
    std::unique_ptr<weld::CustomWeld> m_xValSetColorBoxWin;

    DECL_LINK(ResetToDefaults, weld::Button&, void);
    DECL_LINK(AddChartColor, weld::Button&, void);
    DECL_LINK(RemoveChartColor, weld::Button&, void);
    DECL_LINK(BoxClickedHdl, ValueSet*, void);
    DECL_LINK(SelectPaletteLbHdl, weld::ComboBox&, void);

    void FillPaletteLB();

private:
    void InsertColorEntry(const XColorEntry& rEntry, sal_Int32 nPos = -1);
    void RemoveColorEntry(sal_Int32 nPos);
    void ModifyColorEntry(const XColorEntry& rEntry, sal_Int32 nPos);
    void ClearColorEntries();
    void FillBoxChartColorLB();

public:
    SvxDefaultColorOptPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs);
    virtual ~SvxDefaultColorOptPage() override;

    void    Construct();

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rInAttrs );
    virtual bool        FillItemSet( SfxItemSet* rOutAttrs ) override;
    virtual void        Reset( const SfxItemSet* rInAttrs ) override;

    void    SaveChartOptions();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
