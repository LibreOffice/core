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
#ifndef INCLUDED_CUI_SOURCE_OPTIONS_OPTCOLOR_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_OPTCOLOR_HXX

#include <sfx2/tabdlg.hxx>

namespace svtools {class EditableColorConfig;class EditableExtendedColorConfig;}
class ColorConfigCtrl_Impl;
class AbstractSvxNameDialog;
class SvxColorOptionsTabPage : public SfxTabPage
{
    using SfxTabPage::DeactivatePage;

    bool bFillItemSetCalled;

    std::unique_ptr<weld::ComboBox> m_xColorSchemeLB;
    std::unique_ptr<weld::Button> m_xSaveSchemePB;
    std::unique_ptr<weld::Button> m_xDeleteSchemePB;
    std::unique_ptr<ColorConfigCtrl_Impl> m_xColorConfigCT;
    std::unique_ptr<weld::Widget> m_xTable;
    std::unique_ptr<weld::Label> m_xOnFT;
    std::unique_ptr<weld::Label> m_xElementFT;
    std::unique_ptr<weld::Label> m_xColorFT;
    weld::Widget& m_rWidget1;
    weld::Widget& m_rWidget2;

    std::unique_ptr<svtools::EditableColorConfig> pColorConfig;
    std::unique_ptr<svtools::EditableExtendedColorConfig> pExtColorConfig;

    DECL_LINK(SchemeChangedHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(SaveDeleteHdl_Impl, weld::Button&, void);
    DECL_LINK(CheckNameHdl_Impl, AbstractSvxNameDialog&, bool);
    DECL_LINK(AdjustHeaderBar, const Size&, void);
    void UpdateColorConfig();

public:
    SvxColorOptionsTabPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual void dispose() override;
    virtual ~SvxColorOptionsTabPage() override;

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent, const SfxItemSet* rAttrSet );

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;
    virtual void        FillUserData() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
