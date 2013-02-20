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
#ifndef _OFA_OPTGDLG_HXX
#define _OFA_OPTGDLG_HXX
#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/langbox.hxx>
#include <readonlyimage.hxx>

// predeclarations
class CanvasSettings;
class SvtOptionsDrawinglayer;

// class OfaMiscTabPage --------------------------------------------------

class OfaMiscTabPage : public SfxTabPage
{
    using TabPage::DeactivatePage;
private:
    CheckBox*     m_pToolTipsCB;
    CheckBox*     m_pExtHelpCB;
    CheckBox*     m_pHelpAgentCB;
    PushButton*   m_pHelpAgentResetBtn;

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
    DECL_LINK(HelpAgentResetHdl_Impl, void *);
#ifdef WNT
    DECL_LINK( OnFileDlgToggled, void* );
#endif
protected:
    virtual int         DeactivatePage( SfxItemSet* pSet = NULL );

public:
    OfaMiscTabPage( Window* pParent, const SfxItemSet& rSet );
    ~OfaMiscTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
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
    NumericField*   m_pAAPointLimit;
    FixedText*      m_pAAPointLimitUnits;

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

    // item ID for the given icon theme
    // might be zero when the theme is not installed and the item is removed
    sal_uLong           aIconStyleItemId[STYLE_SYMBOLS_THEMES_MAX];
    SvtTabAppearanceCfg*    pAppearanceCfg;
    CanvasSettings*         pCanvasSettings;
    SvtOptionsDrawinglayer* mpDrawinglayerOpt;

#if defined( UNX )
    DECL_LINK( OnAntialiasingToggled, void* );
#endif
    // #i97672#
    DECL_LINK( OnSelectionToggled, void* );

public:
    OfaViewTabPage( Window* pParent, const SfxItemSet& rSet );
    ~OfaViewTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

struct LanguageConfig_Impl;
class OfaLanguagesTabPage : public SfxTabPage
{
    FixedLine       aUILanguageGB;
    ReadOnlyImage   aLocaleSettingFI;
    FixedText       aUserInterfaceFT;
    ListBox         aUserInterfaceLB;
    FixedText       aLocaleSettingFT;
    SvxLanguageBox  aLocaleSettingLB;
    ReadOnlyImage   aCurrencyFI;
    FixedText       aDecimalSeparatorFT;
    CheckBox        aDecimalSeparatorCB;
    FixedText       aCurrencyFT;
    ListBox         aCurrencyLB;
    ReadOnlyImage   aDatePatternsFI;
    FixedText       aDatePatternsFT;
    Edit            aDatePatternsED;

    FixedLine       aLinguLanguageGB;
    ReadOnlyImage   aWesternLanguageFI;
    FixedText       aWesternLanguageFT;
    SvxLanguageBox  aWesternLanguageLB;
    ReadOnlyImage   aAsianLanguageFI;
    FixedText       aAsianLanguageFT;
    SvxLanguageBox  aAsianLanguageLB;
    ReadOnlyImage   aComplexLanguageFI;
    FixedText       aComplexLanguageFT;
    SvxLanguageBox  aComplexLanguageLB;
    CheckBox        aCurrentDocCB;
    FixedLine       aEnhancedFL;
    ReadOnlyImage   aAsianSupportFI;
    CheckBox        aAsianSupportCB;
    ReadOnlyImage   aCTLSupportFI;
    CheckBox        aCTLSupportCB;
    CheckBox        aIgnoreLanguageChangeCB;

    const String    sDecimalSeparatorLabel;

    sal_Bool        m_bOldAsian;
    sal_Bool        m_bOldCtl;
    LanguageConfig_Impl*    pLangConfig;

    rtl::OUString m_sUserLocaleValue;

    DECL_LINK(  SupportHdl, CheckBox* ) ;
    DECL_LINK(  LocaleSettingHdl, SvxLanguageBox* ) ;
    DECL_LINK(  DatePatternsHdl, Edit* ) ;

public:
    OfaLanguagesTabPage( Window* pParent, const SfxItemSet& rSet );
    ~OfaLanguagesTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};
#endif // #ifndef _OFA_OPTGDLG_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
