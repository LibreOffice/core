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

// class SvxFontSubstTabPage ----------------------------------------------------
class SvxFontSubstTabPage : public SfxTabPage
{
    OUString                    m_sAutomatic;

    std::unique_ptr<weld::CheckButton> m_xUseTableCB;
    std::unique_ptr<weld::Widget> m_xUseTableImg;
    std::unique_ptr<weld::ComboBox> m_xFont1CB;
    std::unique_ptr<weld::ComboBox> m_xFont2CB;
    std::unique_ptr<weld::Button> m_xApply;
    std::unique_ptr<weld::Button> m_xDelete;
    std::unique_ptr<weld::TreeView> m_xCheckLB;
    std::unique_ptr<weld::ComboBox> m_xFontNameLB;
    std::unique_ptr<weld::Label> m_xFontNameLabel;
    std::unique_ptr<weld::Widget> m_xFontNameImg;
    std::unique_ptr<weld::CheckButton> m_xNonPropFontsOnlyCB;
    std::unique_ptr<weld::Widget> m_xNonPropFontsOnlyImg;
    std::unique_ptr<weld::ComboBox> m_xFontHeightLB;
    std::unique_ptr<weld::Label> m_xFontHeightLabel;
    std::unique_ptr<weld::Widget> m_xFontHeightImg;

    DECL_LINK(SelectComboBoxHdl, weld::ComboBox&, void);
    DECL_LINK(ToggleHdl, weld::Toggleable&, void);
    DECL_LINK(ClickHdl, weld::Button&, void);
    DECL_LINK(TreeListBoxSelectHdl, weld::TreeView&, void);
    DECL_LINK(NonPropFontsHdl, weld::Toggleable&, void);
    DECL_LINK(HeaderBarClick, int, void);
    DECL_LINK(ResizeHdl, const Size&, void);

    void SelectHdl(const weld::Widget* pWidget);

    void            CheckEnable();
    void            setColSizes(const Size& rSize);

public:
    SvxFontSubstTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet);
    virtual ~SvxFontSubstTabPage() override;

    virtual OUString GetAllStrings() override;

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
