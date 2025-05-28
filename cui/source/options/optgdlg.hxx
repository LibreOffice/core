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
#include <config_features.h>
#include <sfx2/tabdlg.hxx>
#include <svx/langbox.hxx>

class CanvasSettings;

namespace vcl {
    class IconThemeInfo;
}

class OfaMiscTabPage : public SfxTabPage
{
private:
    OUString             m_aStrDateInfo;

    std::unique_ptr<weld::CheckButton> m_xExtHelpCB;
    std::unique_ptr<weld::Widget> m_xExtHelpImg;
    std::unique_ptr<weld::CheckButton> m_xPopUpNoHelpCB;
    std::unique_ptr<weld::Widget> m_xPopUpNoHelpImg;
    std::unique_ptr<weld::CheckButton> m_xShowTipOfTheDay;
    std::unique_ptr<weld::Widget> m_xShowTipOfTheDayImg;
    std::unique_ptr<weld::Widget> m_xFileDlgFrame;
    std::unique_ptr<weld::Widget> m_xFileDlgROImage;
    std::unique_ptr<weld::CheckButton> m_xFileDlgCB;
    std::unique_ptr<weld::CheckButton> m_xDocStatusCB;
    std::unique_ptr<weld::Widget> m_xDocStatusImg;
    std::unique_ptr<weld::Widget> m_xYearFrame;
    std::unique_ptr<weld::Label> m_xYearLabel;
    std::unique_ptr<weld::SpinButton> m_xYearValueField;
    std::unique_ptr<weld::Label> m_xToYearFT;
    std::unique_ptr<weld::Widget> m_xYearFrameImg;
#if HAVE_FEATURE_BREAKPAD
    std::unique_ptr<weld::Widget> m_xPrivacyFrame;
    std::unique_ptr<weld::CheckButton> m_xCrashReport;
    std::unique_ptr<weld::Widget> m_xCrashReportImg;
#endif
#if defined(_WIN32)
    std::unique_ptr<weld::Widget> m_xQuickStarterFrame;
    std::unique_ptr<weld::CheckButton> m_xQuickLaunchCB;
    std::unique_ptr<weld::Widget> m_xQuickLaunchImg;
    std::unique_ptr<weld::Widget> m_xFileAssocFrame;
    std::unique_ptr<weld::Button> m_xFileAssocBtn;
    std::unique_ptr<weld::CheckButton> m_xPerformFileExtCheck;
    std::unique_ptr<weld::Widget> m_xPerformFileExtImg;
#endif

    DECL_LINK(TwoFigureHdl, weld::SpinButton&, void);
#if defined(_WIN32)
    DECL_DLLPRIVATE_STATIC_LINK(OfaMiscTabPage, FileAssocClick, weld::Button&, void);
#endif
protected:
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

public:
    OfaMiscTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~OfaMiscTabPage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet );

    virtual OUString GetAllStrings() override;

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

class OfaViewTabPage : public SfxTabPage
{
private:
    std::unique_ptr<CanvasSettings>         pCanvasSettings;

    std::unique_ptr<weld::CheckButton> m_xFontAntiAliasing;
    std::unique_ptr<weld::Widget> m_xFontAntiAliasingImg;
    std::unique_ptr<weld::Label> m_xAAPointLimitLabel;
    std::unique_ptr<weld::Widget> m_xAAPointLimitLabelImg;
    std::unique_ptr<weld::MetricSpinButton> m_xAAPointLimit;

    std::unique_ptr<weld::CheckButton> m_xFontShowCB;
    std::unique_ptr<weld::Widget> m_xFontShowImg;

    std::unique_ptr<weld::CheckButton> m_xUseHardwareAccell;
    std::unique_ptr<weld::Widget> m_xUseHardwareAccellImg;
    std::unique_ptr<weld::CheckButton> m_xUseAntiAliase;
    std::unique_ptr<weld::Widget> m_xUseAntiAliaseImg;
    std::unique_ptr<weld::CheckButton> m_xUseSkia;
    std::unique_ptr<weld::Widget> m_xUseSkiaImg;
    std::unique_ptr<weld::CheckButton> m_xForceSkiaRaster;
    std::unique_ptr<weld::Widget> m_xForceSkiaRasterImg;

    std::unique_ptr<weld::Label> m_xSkiaStatusEnabled;
    std::unique_ptr<weld::Label> m_xSkiaStatusDisabled;
    std::unique_ptr<weld::Button> m_xSkiaLog;

    std::unique_ptr<weld::Label> m_xMouseMiddleLabel;
    std::unique_ptr<weld::ComboBox> m_xMouseMiddleLB;
    std::unique_ptr<weld::Widget> m_xMouseMiddleImg;
    std::unique_ptr<weld::Button> m_xRunGPTests;

    DECL_LINK(OnAntialiasingToggled, weld::Toggleable&, void);
    DECL_LINK(OnUseSkiaToggled, weld::Toggleable&, void);
    DECL_LINK(OnCopySkiaLog, weld::Button&, void);
    DECL_LINK(OnRunGPTestClick, weld::Button&, void);
    void UpdateSkiaStatus();
    void HideSkiaWidgets();
    void UpdateHardwareAccelStatus();
    void UpdateIconThemes();

public:
    OfaViewTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~OfaViewTabPage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet );

    virtual OUString GetAllStrings() override;

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
    std::unique_ptr<weld::Widget> m_xLocaleSettingImg;
    std::unique_ptr<weld::Label> m_xDecimalSeparatorFT;
    std::unique_ptr<weld::CheckButton> m_xDecimalSeparatorCB;
    std::unique_ptr<weld::Widget> m_xDecimalSeparatorImg;
    std::unique_ptr<weld::Label> m_xCurrencyFT;
    std::unique_ptr<weld::ComboBox> m_xCurrencyLB;
    std::unique_ptr<weld::Widget> m_xCurrencyImg;
    std::unique_ptr<weld::Label> m_xDatePatternsFT;
    std::unique_ptr<weld::Entry> m_xDatePatternsED;
    std::unique_ptr<weld::Widget> m_xDatePatternsImg;

    std::unique_ptr<SvxLanguageBox> m_xWesternLanguageLB;
    std::unique_ptr<weld::Label> m_xWesternLanguageFT;
    std::unique_ptr<weld::Widget> m_xWesternLanguageImg;
    std::unique_ptr<SvxLanguageBox> m_xAsianLanguageLB;
    std::unique_ptr<SvxLanguageBox> m_xComplexLanguageLB;
    std::unique_ptr<weld::CheckButton> m_xCurrentDocCB;
    std::unique_ptr<weld::CheckButton> m_xAsianSupportCB;
    std::unique_ptr<weld::Widget> m_xAsianSupportImg;
    std::unique_ptr<weld::CheckButton> m_xCTLSupportCB;
    std::unique_ptr<weld::Widget> m_xCTLSupportImg;
    std::unique_ptr<weld::CheckButton> m_xIgnoreLanguageChangeCB;
    std::unique_ptr<weld::Widget> m_xIgnoreLanguageChangeImg;

    DECL_LINK(SupportHdl, weld::Toggleable&, void);
    DECL_LINK(LocaleSettingHdl, weld::ComboBox&, void);
    DECL_LINK(DatePatternsHdl, weld::Entry&, void);

    bool validateDatePatterns( bool& rbModified, OUString& rPatterns );

public:
    OfaLanguagesTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~OfaLanguagesTabPage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet );

    virtual OUString GetAllStrings() override;

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
