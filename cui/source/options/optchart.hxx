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

#ifndef INCLUDED_CUI_SOURCE_OPTIONS_OPTCHART_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_OPTCHART_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <svtools/valueset.hxx>
#include <svx/dlgctrl.hxx>
#include <vcl/button.hxx>
#include <svx/xtable.hxx>
#include <svx/PaletteManager.hxx>

#include "cfgchart.hxx"

typedef std::vector<Color> ImpColorList;

class SvxDefaultColorOptPage : public SfxTabPage
{
private:
    VclPtr<ListBox>                m_pLbChartColors;
    VclPtr<ListBox>                m_pLbPaletteSelector;
    VclPtr<SvxColorValueSet>       m_pValSetColorBox;
    VclPtr<PushButton>             m_pPBDefault;
    VclPtr<PushButton>             m_pPBAdd;
    VclPtr<PushButton>             m_pPBRemove;

    std::unique_ptr<SvxChartOptions>        pChartOptions;
    std::unique_ptr<SvxChartColorTableItem> pColorConfig;
    ImpColorList            aColorList;
    PaletteManager          aPaletteManager;

    DECL_LINK( ResetToDefaults, Button *, void );
    DECL_LINK( AddChartColor, Button *, void );
    DECL_LINK( RemoveChartColor, Button *, void );
    DECL_LINK(BoxClickedHdl, ValueSet*, void);
    DECL_LINK( SelectPaletteLbHdl, ListBox&, void );

    void FillPaletteLB();

private:
    void InsertColorEntry(const XColorEntry& rEntry, sal_Int32 nPos = LISTBOX_APPEND);
    void RemoveColorEntry(sal_Int32 nPos);
    void ModifyColorEntry(const XColorEntry& rEntry, sal_Int32 nPos);
    void ClearColorEntries();
    void FillBoxChartColorLB();

public:
    SvxDefaultColorOptPage( vcl::Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~SvxDefaultColorOptPage() override;
    virtual void dispose() override;

    void    Construct();

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent, const SfxItemSet* rInAttrs );
    virtual bool        FillItemSet( SfxItemSet* rOutAttrs ) override;
    virtual void        Reset( const SfxItemSet* rInAttrs ) override;

    void    SaveChartOptions();
};

#endif // INCLUDED_CUI_SOURCE_OPTIONS_OPTCHART_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
