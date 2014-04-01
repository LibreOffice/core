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
#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/langbox.hxx>

// predeclarations
class CanvasSettings;
class SvtOptionsDrawinglayer;
namespace vcl {
class IconThemeInfo;
}

// class OfaMiscTabPage --------------------------------------------------

class OfaMiscTabPage : public SfxTabPage
{
    using TabPage::DeactivatePage;
private:
    CheckBox*     m_pToolTipsCB;
    CheckBox*     m_pExtHelpCB;

    FixedImage*   m_pFileDlgROImage;
    CheckBox*     m_pFileDlgCB;

    CheckBox*     m_pPrintDlgCB;

    CheckBox*     m_pDocStatusCB;
    CheckBox*     m_pSaveAlwaysCB;

    VclContainer* m_pYearFrame;
    NumericField* m_pYearValueField;
    FixedText*    m_pToYearFT;

    OUString      m_aStrDateInfo;

    DECL_LINK( TwoFigureHdl, NumericField* );
    DECL_LINK( TwoFigureConfigHdl, NumericField* );
    DECL_LINK(HelpCheckHdl_Impl, void *);
#ifdef WNT
    DECL_LINK( OnFileDlgToggled, void* );
#endif
protected:
    virtual int         DeactivatePage( SfxItemSet* pSet = NULL ) SAL_OVERRIDE;

public:
    OfaMiscTabPage( Window* pParent, const SfxItemSet& rSet );
    virtual ~OfaMiscTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual bool        FillItemSet( SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet& rSet ) SAL_OVERRIDE;
};

// class OfaViewTabPage --------------------------------------------------
class SvtTabAppearanceCfg;

class OfaViewTabPage : public SfxTabPage
{
private:
    MetricField*    m_pWindowSizeMF;
    ListBox*        m_pIconSizeLB;
    ListBox*        m_pIconStyleLB;
    CheckBox*       m_pSystemFont;

    CheckBox*       m_pFontAntiAliasing;
    FixedText*      m_pAAPointLimitLabel;
    MetricField*    m_pAAPointLimit;

    ListBox*        m_pMenuIconsLB;

    CheckBox*       m_pFontShowCB;
    CheckBox*       m_pFontHistoryCB;

    CheckBox*       m_pUseHardwareAccell;
    CheckBox*       m_pUseAntiAliase;

    ListBox*        m_pMousePosLB;
    ListBox*        m_pMouseMiddleLB;

    // #i97672#
    CheckBox*       m_pSelectionCB;
    MetricField*    m_pSelectionMF;

    sal_uInt16      nSizeLB_InitialSelection;
    sal_uInt16      nStyleLB_InitialSelection;

    SvtTabAppearanceCfg*    pAppearanceCfg;
    CanvasSettings*         pCanvasSettings;
    SvtOptionsDrawinglayer* mpDrawinglayerOpt;

    std::vector<vcl::IconThemeInfo> mInstalledIconThemes;

#if defined( UNX )
    DECL_LINK( OnAntialiasingToggled, void* );
#endif
    // #i97672#
    DECL_LINK( OnSelectionToggled, void* );

public:
    OfaViewTabPage( Window* pParent, const SfxItemSet& rSet );
    virtual ~OfaViewTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual bool        FillItemSet( SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet& rSet ) SAL_OVERRIDE;
};

struct LanguageConfig_Impl;
class OfaLanguagesTabPage : public SfxTabPage
{
    ListBox*        m_pUserInterfaceLB;
    FixedText*      m_pLocaleSettingFT;
    SvxLanguageBox* m_pLocaleSettingLB;
    CheckBox*       m_pDecimalSeparatorCB;
    FixedText*      m_pCurrencyFT;
    ListBox*        m_pCurrencyLB;
    FixedText*      m_pDatePatternsFT;
    Edit*           m_pDatePatternsED;

    SvxLanguageBox* m_pWesternLanguageLB;
    FixedText*      m_pWesternLanguageFT;
    SvxLanguageBox* m_pAsianLanguageLB;
    SvxLanguageBox* m_pComplexLanguageLB;
    CheckBox*       m_pCurrentDocCB;
    CheckBox*       m_pAsianSupportCB;
    CheckBox*       m_pCTLSupportCB;
    CheckBox*       m_pIgnoreLanguageChangeCB;

    sal_Bool        m_bOldAsian;
    sal_Bool        m_bOldCtl;
    LanguageConfig_Impl*    pLangConfig;

    OUString        m_sUserLocaleValue;
    OUString        m_sSystemDefaultString;

    DECL_LINK(  SupportHdl, CheckBox* ) ;
    DECL_LINK(  LocaleSettingHdl, SvxLanguageBox* ) ;
    DECL_LINK(  DatePatternsHdl, Edit* ) ;

public:
    OfaLanguagesTabPage( Window* pParent, const SfxItemSet& rSet );
    virtual ~OfaLanguagesTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual bool        FillItemSet( SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet& rSet ) SAL_OVERRIDE;
};
#endif // INCLUDED_CUI_SOURCE_OPTIONS_OPTGDLG_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
