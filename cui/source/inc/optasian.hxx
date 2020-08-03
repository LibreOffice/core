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

#include <memory>
#include <sfx2/tabdlg.hxx>
#include <svx/langbox.hxx>

struct SvxAsianLayoutPage_Impl;
class SvxAsianLayoutPage : public SfxTabPage
{
    std::unique_ptr<SvxAsianLayoutPage_Impl> pImpl;

    std::unique_ptr<weld::RadioButton> m_xCharKerningRB;
    std::unique_ptr<weld::RadioButton> m_xCharPunctKerningRB;
    std::unique_ptr<weld::RadioButton> m_xNoCompressionRB;
    std::unique_ptr<weld::RadioButton> m_xPunctCompressionRB;
    std::unique_ptr<weld::RadioButton> m_xPunctKanaCompressionRB;
    std::unique_ptr<weld::Label> m_xLanguageFT;
    std::unique_ptr<SvxLanguageBox> m_xLanguageLB;
    std::unique_ptr<weld::CheckButton> m_xStandardCB;
    std::unique_ptr<weld::Label> m_xStartFT;
    std::unique_ptr<weld::Entry> m_xStartED;
    std::unique_ptr<weld::Label> m_xEndFT;
    std::unique_ptr<weld::Entry> m_xEndED;
    std::unique_ptr<weld::Label> m_xHintFT;

    DECL_LINK(LanguageHdl, weld::ComboBox&, void);
    DECL_LINK(ChangeStandardHdl, weld::ToggleButton&, void);
    DECL_LINK(ModifyHdl, weld::Entry&, void);

public:
    SvxAsianLayoutPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet );
    virtual ~SvxAsianLayoutPage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet );
    static const sal_uInt16*  GetRanges();
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
