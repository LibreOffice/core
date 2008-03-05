/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optgdlg.hxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:53:32 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _OFA_OPTGDLG_HXX
#define _OFA_OPTGDLG_HXX
#ifndef _SV_LSTBOX_HXX //autogen wg. ListBox
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_GROUP_HXX //autogen wg. FixedLine
#include <vcl/group.hxx>
#endif
#ifndef _SV_FIELD_HXX //autogen wg. NumericField
#include <vcl/field.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen wg. FixedText
#include <vcl/fixed.hxx>
#endif
#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SVX_LANGBOX_HXX
#include <svx/langbox.hxx>
#endif
#ifndef _SVX_READONLYIMAGE_HXX
#include <readonlyimage.hxx>
#endif
#define FOLDERWEBVIEW_DEFAULTFILE   "folder.so"

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
    CheckBox        aMenuIconsCB;

    FixedLine       aFontListsFL;
    CheckBox        aFontShowCB;
    CheckBox        aFontHistoryCB;

    FixedLine       a3DGB;
    CheckBox        a3DOpenGLCB;
    CheckBox        a3DOpenGLFasterCB;
    CheckBox        a3DDitheringCB;
    CheckBox        a3DShowFullCB;

    FixedLine       aRenderingFL;
    CheckBox        aUseHardwareAccell;

    FixedLine       aMouseFL;
    FixedText       aMousePosFT;
    ListBox         aMousePosLB;
    FixedText       aMouseMiddleFT;
    ListBox         aMouseMiddleLB;

    UINT16          nSizeLB_InitialSelection;
    UINT16          nStyleLB_InitialSelection;
    BOOL            bSfxSymbolsAuto;

    SvtTabAppearanceCfg* pAppearanceCfg;

    DECL_LINK( OpenGLHdl, CheckBox* );
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


