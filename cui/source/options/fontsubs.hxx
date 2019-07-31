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
#ifndef INCLUDED_CUI_SOURCE_OPTIONS_FONTSUBS_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_FONTSUBS_HXX

#include <sfx2/tabdlg.hxx>
#include <svtools/ctrlbox.hxx>
#include <svtools/simptabl.hxx>
#include <vcl/treelistentry.hxx>
#include <vcl/layout.hxx>

// class SvxFontSubstTabPage ----------------------------------------------------
class SvtFontSubstConfig;
class SvxFontSubstTabPage : public SfxTabPage
{
    OUString                    m_sAutomatic;

    std::unique_ptr<SvtFontSubstConfig> m_xConfig;

    std::unique_ptr<weld::CheckButton> m_xUseTableCB;
    std::unique_ptr<weld::ComboBox> m_xFont1CB;
    std::unique_ptr<weld::ComboBox> m_xFont2CB;
    std::unique_ptr<weld::Button> m_xApply;
    std::unique_ptr<weld::Button> m_xDelete;
    std::unique_ptr<weld::TreeView> m_xCheckLB;
    std::unique_ptr<weld::ComboBox> m_xFontNameLB;
    std::unique_ptr<weld::CheckButton> m_xNonPropFontsOnlyCB;
    std::unique_ptr<weld::ComboBox> m_xFontHeightLB;

    DECL_LINK(SelectComboBoxHdl, weld::ComboBox&, void);
    DECL_LINK(ClickHdl, weld::Button&, void);
    DECL_LINK(TreeListBoxSelectHdl, weld::TreeView&, void);
    DECL_LINK(NonPropFontsHdl, weld::ToggleButton&, void);
    void SelectHdl(const weld::Widget* pWidget);

    void            CheckEnable();
    void            setColSizes();

    virtual ~SvxFontSubstTabPage() override;

public:
    SvxFontSubstTabPage(TabPageParent pParent, const SfxItemSet& rSet);
    static VclPtr<SfxTabPage> Create( TabPageParent pParent, const SfxItemSet* rAttrSet);
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

#endif // INCLUDED_CUI_SOURCE_OPTIONS_FONTSUBS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
