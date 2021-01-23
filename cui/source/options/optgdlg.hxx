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
#ifndef INCLUDED_CUI_SOURCE_OPTIONS_OPTGDLG_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_OPTGDLG_HXX
#include <memory>
#include <sfx2/tabdlg.hxx>
#include <svx/langbox.hxx>

class CanvasSettings;
class SvtOptionsDrawinglayer;

namespace vcl {
    class IconThemeInfo;
}

namespace svt {
    class OpenGLCfg;
}

class OfaMiscTabPage : public SfxTabPage
{
private:
    OUString             m_aStrDateInfo;

    std::unique_ptr<weld::CheckButton> m_xExtHelpCB;
    std::unique_ptr<weld::CheckButton> m_xPopUpNoHelpCB;
    std::unique_ptr<weld::CheckButton> m_xShowTipOfTheDay;
    std::unique_ptr<weld::Widget> m_xFileDlgFrame;
    std::unique_ptr<weld::Widget> m_xPrintDlgFrame;
    std::unique_ptr<weld::Widget> m_xFileDlgROImage;
    std::unique_ptr<weld::CheckButton> m_xFileDlgCB;
    std::unique_ptr<weld::CheckButton> m_xPrintDlgCB;
    std::unique_ptr<weld::CheckButton> m_xDocStatusCB;
    std::unique_ptr<weld::Widget> m_xYearFrame;
    std::unique_ptr<weld::SpinButton> m_xYearValueField;
    std::unique_ptr<weld::Label> m_xToYearFT;
    std::unique_ptr<weld::Widget> m_xQuickStarterFrame;
    std::unique_ptr<weld::CheckButton> m_xQuickLaunchCB;

    DECL_LINK(TwoFigureHdl, weld::SpinButton&, void);
protected:
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

public:
    OfaMiscTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~OfaMiscTabPage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet );

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

class SvtTabAppearanceCfg;

class OfaViewTabPage : public SfxTabPage
{
private:
    sal_Int32      nSizeLB_InitialSelection;
    sal_Int32      nSidebarSizeLB_InitialSelection;
    sal_Int32      nNotebookbarSizeLB_InitialSelection;
    sal_Int32      nStyleLB_InitialSelection;

    std::unique_ptr<SvtTabAppearanceCfg>    pAppearanceCfg;
    std::unique_ptr<CanvasSettings>         pCanvasSettings;
    std::unique_ptr<SvtOptionsDrawinglayer> mpDrawinglayerOpt;
    std::unique_ptr<svt::OpenGLCfg> mpOpenGLConfig;

    std::vector<vcl::IconThemeInfo> mInstalledIconThemes;

    std::unique_ptr<weld::ComboBox> m_xIconSizeLB;
    std::unique_ptr<weld::ComboBox> m_xSidebarIconSizeLB;
    std::unique_ptr<weld::ComboBox> m_xNotebookbarIconSizeLB;
    std::unique_ptr<weld::ComboBox> m_xIconStyleLB;

    std::unique_ptr<weld::CheckButton> m_xFontAntiAliasing;
    std::unique_ptr<weld::Label> m_xAAPointLimitLabel;
    std::unique_ptr<weld::MetricSpinButton> m_xAAPointLimit;

    std::unique_ptr<weld::Widget> m_xMenuIconBox;
    std::unique_ptr<weld::ComboBox> m_xMenuIconsLB;

    std::unique_ptr<weld::ComboBox> m_xContextMenuShortcutsLB;

    std::unique_ptr<weld::CheckButton> m_xFontShowCB;

    std::unique_ptr<weld::CheckButton> m_xUseHardwareAccell;
    std::unique_ptr<weld::CheckButton> m_xUseAntiAliase;
    std::unique_ptr<weld::CheckButton> m_xUseOpenGL;
    std::unique_ptr<weld::CheckButton> m_xForceOpenGL;

    std::unique_ptr<weld::Label> m_xOpenGLStatusEnabled;
    std::unique_ptr<weld::Label> m_xOpenGLStatusDisabled;

    std::unique_ptr<weld::ComboBox> m_xMousePosLB;
    std::unique_ptr<weld::ComboBox> m_xMouseMiddleLB;

#if defined( UNX )
    DECL_LINK(OnAntialiasingToggled, weld::ToggleButton&, void);
#endif
    DECL_LINK(OnForceOpenGLToggled, weld::ToggleButton&, void);
    void UpdateOGLStatus();

public:
    OfaViewTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~OfaViewTabPage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet );

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

struct LanguageConfig_Impl;

class OfaLanguagesTabPage : public SfxTabPage
{
    bool        m_bOldAsian;
    bool        m_bOldCtl;
    std::unique_ptr<LanguageConfig_Impl> pLangConfig;

    OUString        m_sUserLocaleValue;
    OUString        m_sSystemDefaultString;
    OUString        m_sDecimalSeparatorLabel;

    bool            m_bDatePatternsValid;

    std::unique_ptr<weld::ComboBox> m_xUserInterfaceLB;
    std::unique_ptr<weld::Label> m_xLocaleSettingFT;
    std::unique_ptr<SvxLanguageBox> m_xLocaleSettingLB;
    std::unique_ptr<weld::CheckButton> m_xDecimalSeparatorCB;
    std::unique_ptr<weld::Label> m_xCurrencyFT;
    std::unique_ptr<weld::ComboBox> m_xCurrencyLB;
    std::unique_ptr<weld::Label> m_xDatePatternsFT;
    std::unique_ptr<weld::Entry> m_xDatePatternsED;

    std::unique_ptr<SvxLanguageBox> m_xWesternLanguageLB;
    std::unique_ptr<weld::Label> m_xWesternLanguageFT;
    std::unique_ptr<SvxLanguageBox> m_xAsianLanguageLB;
    std::unique_ptr<SvxLanguageBox> m_xComplexLanguageLB;
    std::unique_ptr<weld::CheckButton> m_xCurrentDocCB;
    std::unique_ptr<weld::CheckButton> m_xAsianSupportCB;
    std::unique_ptr<weld::CheckButton> m_xCTLSupportCB;
    std::unique_ptr<weld::CheckButton> m_xIgnoreLanguageChangeCB;

    DECL_LINK(SupportHdl, weld::ToggleButton&, void);
    DECL_LINK(LocaleSettingHdl, weld::ComboBox&, void);
    DECL_LINK(DatePatternsHdl, weld::Entry&, void);

public:
    OfaLanguagesTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~OfaLanguagesTabPage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet );

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

#endif // INCLUDED_CUI_SOURCE_OPTIONS_OPTGDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
