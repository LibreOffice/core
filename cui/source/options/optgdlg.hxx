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
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
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
    using TabPage::DeactivatePage;
private:
    OUString             m_aStrDateInfo;

    std::unique_ptr<weld::CheckButton> m_xExtHelpCB;
    std::unique_ptr<weld::CheckButton> m_xPopUpNoHelpCB;
    std::unique_ptr<weld::Widget> m_xFileDlgFrame;
    std::unique_ptr<weld::Widget> m_xPrintDlgFrame;
    std::unique_ptr<weld::Widget> m_xFileDlgROImage;
    std::unique_ptr<weld::CheckButton> m_xFileDlgCB;
    std::unique_ptr<weld::CheckButton> m_xPrintDlgCB;
    std::unique_ptr<weld::CheckButton> m_xDocStatusCB;
    std::unique_ptr<weld::Widget> m_xYearFrame;
    std::unique_ptr<weld::SpinButton> m_xYearValueField;
    std::unique_ptr<weld::Label> m_xToYearFT;
    std::unique_ptr<weld::CheckButton> m_xCollectUsageInfo;
    std::unique_ptr<weld::Widget> m_xQuickStarterFrame;
    std::unique_ptr<weld::CheckButton> m_xQuickLaunchCB;

    DECL_LINK(TwoFigureHdl, weld::SpinButton&, void);
protected:
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

public:
    OfaMiscTabPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual ~OfaMiscTabPage() override;

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent, const SfxItemSet* rAttrSet );

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

class SvtTabAppearanceCfg;

class OfaViewTabPage : public SfxTabPage
{
private:
    VclPtr<ListBox>        m_pIconSizeLB;
    VclPtr<ListBox>        m_pSidebarIconSizeLB;
    VclPtr<ListBox>        m_pNotebookbarIconSizeLB;
    VclPtr<ListBox>        m_pIconStyleLB;

    VclPtr<CheckBox>       m_pFontAntiAliasing;
    VclPtr<FixedText>      m_pAAPointLimitLabel;
    VclPtr<MetricField>    m_pAAPointLimit;

    VclPtr<ListBox>        m_pMenuIconsLB;
    VclPtr<ListBox>        m_pContextMenuShortcutsLB;

    VclPtr<CheckBox>       m_pFontShowCB;

    VclPtr<CheckBox>       m_pUseHardwareAccell;
    VclPtr<CheckBox>       m_pUseAntiAliase;
    VclPtr<CheckBox>       m_pUseOpenGL;
    VclPtr<CheckBox>       m_pForceOpenGL;

    VclPtr<FixedText>      m_pOpenGLStatusEnabled;
    VclPtr<FixedText>      m_pOpenGLStatusDisabled;

    VclPtr<ListBox>        m_pMousePosLB;
    VclPtr<ListBox>        m_pMouseMiddleLB;

    sal_Int32      nSizeLB_InitialSelection;
    sal_Int32      nSidebarSizeLB_InitialSelection;
    sal_Int32      nNotebookbarSizeLB_InitialSelection;
    sal_Int32      nStyleLB_InitialSelection;

    std::unique_ptr<SvtTabAppearanceCfg>    pAppearanceCfg;
    std::unique_ptr<CanvasSettings>         pCanvasSettings;
    std::unique_ptr<SvtOptionsDrawinglayer> mpDrawinglayerOpt;
    std::unique_ptr<svt::OpenGLCfg> mpOpenGLConfig;

    std::vector<vcl::IconThemeInfo> mInstalledIconThemes;

#if defined( UNX )
    DECL_LINK( OnAntialiasingToggled, CheckBox&, void );
#endif
    DECL_LINK(OnForceOpenGLToggled, CheckBox&, void);
    void UpdateOGLStatus();

public:
    OfaViewTabPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~OfaViewTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent, const SfxItemSet* rAttrSet );

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

struct LanguageConfig_Impl;

class OfaLanguagesTabPage : public SfxTabPage
{
    VclPtr<ListBox>        m_pUserInterfaceLB;
    VclPtr<FixedText>      m_pLocaleSettingFT;
    VclPtr<SvxLanguageBox> m_pLocaleSettingLB;
    VclPtr<CheckBox>       m_pDecimalSeparatorCB;
    VclPtr<FixedText>      m_pCurrencyFT;
    VclPtr<ListBox>        m_pCurrencyLB;
    VclPtr<FixedText>      m_pDatePatternsFT;
    VclPtr<Edit>           m_pDatePatternsED;

    VclPtr<SvxLanguageBox> m_pWesternLanguageLB;
    VclPtr<FixedText>      m_pWesternLanguageFT;
    VclPtr<SvxLanguageBox> m_pAsianLanguageLB;
    VclPtr<SvxLanguageBox> m_pComplexLanguageLB;
    VclPtr<CheckBox>       m_pCurrentDocCB;
    VclPtr<CheckBox>       m_pAsianSupportCB;
    VclPtr<CheckBox>       m_pCTLSupportCB;
    VclPtr<CheckBox>       m_pIgnoreLanguageChangeCB;

    bool        m_bOldAsian;
    bool        m_bOldCtl;
    std::unique_ptr<LanguageConfig_Impl> pLangConfig;

    OUString        m_sUserLocaleValue;
    OUString        m_sSystemDefaultString;

    bool            m_bDatePatternsValid;

    DECL_LINK(  SupportHdl, Button*, void ) ;
    DECL_LINK(  LocaleSettingHdl, ListBox&, void ) ;
    DECL_LINK(  DatePatternsHdl, Edit&, void ) ;

public:
    OfaLanguagesTabPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~OfaLanguagesTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent, const SfxItemSet* rAttrSet );

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

#endif // INCLUDED_CUI_SOURCE_OPTIONS_OPTGDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
