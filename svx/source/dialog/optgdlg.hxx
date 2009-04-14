/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: optgdlg.hxx,v $
 * $Revision: 1.23 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _OFA_OPTGDLG_HXX
#define _OFA_OPTGDLG_HXX
#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/langbox.hxx>
#include <readonlyimage.hxx>
#define FOLDERWEBVIEW_DEFAULTFILE   "folder.so"

// predeclarations
class CanvasSettings;
class SvtOptionsDrawinglayer;

// class OfaMiscTabPage --------------------------------------------------

class OfaMiscTabPage : public SfxTabPage
{
    using TabPage::DeactivatePage;
private:
    FixedLine           aHelpFL;
    CheckBox            aToolTipsCB;
    CheckBox            aExtHelpCB;
    CheckBox            aHelpAgentCB;
    PushButton          aHelpAgentResetBtn;
    FixedText           aHelpFormatFT;
    ListBox             aHelpFormatLB;

    FixedLine           aFileDlgFL;
    ReadOnlyImage       aFileDlgROImage;
    CheckBox            aFileDlgCB;

    FixedLine           aPrintDlgFL;
    CheckBox            aPrintDlgCB;

    FixedLine           aDocStatusFL;
    CheckBox            aDocStatusCB;

    FixedLine           aTwoFigureFL;
    FixedText           aInterpretFT;
    NumericField        aYearValueField;
    FixedText           aToYearFT;

    String              aStrDateInfo;

    DECL_LINK( TwoFigureHdl, NumericField* );
    DECL_LINK( TwoFigureConfigHdl, NumericField* );
    DECL_LINK( HelpCheckHdl_Impl, CheckBox* );
    DECL_LINK( HelpAgentResetHdl_Impl, PushButton* );
protected:
    virtual int         DeactivatePage( SfxItemSet* pSet = NULL );

public:
    OfaMiscTabPage( Window* pParent, const SfxItemSet& rSet );
    ~OfaMiscTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

// class OfaViewTabPage --------------------------------------------------
class SvtTabAppearanceCfg;

class OfaViewTabPage : public SfxTabPage
{
private:
    FixedLine       aUserInterfaceFL;
    FixedText       aWindowSizeFT;
    MetricField     aWindowSizeMF;
    FixedText       aIconSizeStyleFT;
    ListBox         aIconSizeLB;
    ListBox         aIconStyleLB;
    CheckBox        m_aSystemFont;

#if defined( UNX )
    CheckBox        aFontAntiAliasing;
    FixedText       aAAPointLimitLabel;
    NumericField    aAAPointLimit;
    FixedText       aAAPointLimitUnits;
#endif

    FixedLine       aMenuFL;
    FixedText       aMenuIconsFT;
    ListBox         aMenuIconsLB;

    FixedLine       aFontListsFL;
    CheckBox        aFontShowCB;
    CheckBox        aFontHistoryCB;

    FixedLine       aRenderingFL;
    CheckBox        aUseHardwareAccell;
    CheckBox        aUseAntiAliase;

    FixedLine       aMouseFL;
    FixedText       aMousePosFT;
    ListBox         aMousePosLB;
    FixedText       aMouseMiddleFT;
    ListBox         aMouseMiddleLB;

    UINT16          nSizeLB_InitialSelection;
    UINT16          nStyleLB_InitialSelection;
    BOOL            bSfxSymbolsAuto;

    SvtTabAppearanceCfg*    pAppearanceCfg;
    CanvasSettings*         pCanvasSettings;
    SvtOptionsDrawinglayer* mpDrawinglayerOpt;

#if defined( UNX )
    DECL_LINK( OnAntialiasingToggled, void* );
#endif
public:
    OfaViewTabPage( Window* pParent, const SfxItemSet& rSet );
    ~OfaViewTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};
/* -----------------------------23.11.00 13:04--------------------------------

 ---------------------------------------------------------------------------*/
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

    const String    sDecimalSeparatorLabel;

    sal_Bool        m_bOldAsian;
    sal_Bool        m_bOldCtl;
    LanguageConfig_Impl*    pLangConfig;

    rtl::OUString m_sUserLocaleValue;

    DECL_LINK(  SupportHdl, CheckBox* ) ;
    DECL_LINK(  LocaleSettingHdl, SvxLanguageBox* ) ;

public:
    OfaLanguagesTabPage( Window* pParent, const SfxItemSet& rSet );
    ~OfaLanguagesTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};
#endif // #ifndef _OFA_OPTGDLG_HXX


