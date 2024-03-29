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

#include <memory>
#include <string_view>

#include <config_features.h>
#include <config_feature_opencl.h>
#include <config_feature_desktop.h>
#include <config_gpgme.h>

#include <officecfg/Office/Common.hxx>
#include <officecfg/Office/Writer.hxx>

#include <svx/dialogs.hrc>
#include <svx/svxids.hrc>

#include <treeopt.hrc>
#include <helpids.h>

#include "cfgchart.hxx"
#include "connpoolconfig.hxx"
#include "connpooloptions.hxx"
#include <cuioptgenrl.hxx>
#include <dbregister.hxx>
#include "dbregisterednamesconfig.hxx"
#include <dialmgr.hxx>
#include "fontsubs.hxx"
#include "optaccessibility.hxx"
#include <optasian.hxx>
#include "optchart.hxx"
#include "optcolor.hxx"
#include "optctl.hxx"
#include "optfltr.hxx"
#include "optgdlg.hxx"
#include "opthtml.hxx"
#include "optinet2.hxx"
#include "optjava.hxx"
#include "optjsearch.hxx"
#include <optlingu.hxx>
#if HAVE_FEATURE_OPENCL
#include "optopencl.hxx"
#endif
#include <optpath.hxx>
#include "optsave.hxx"
#include "optupdt.hxx"
#include "personalization.hxx"
#include <treeopt.hxx>
#include "optbasic.hxx"
#include "optlanguagetool.hxx"
#include "optdeepl.hxx"

// include search util
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <unotools/textsearch.hxx>

#include <com/sun/star/awt/XContainerWindowEventHandler.hpp>
#include <com/sun/star/awt/ContainerWindowProvider.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/UnknownModuleException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/linguistic2/LinguProperties.hpp>
#include <comphelper/getexpandeduri.hxx>
#include <comphelper/processfactory.hxx>
#include <editeng/langitem.hxx>
#include <editeng/optitems.hxx>
#include <editeng/unolingu.hxx>
#include <linguistic/misc.hxx>
#include <o3tl/deleter.hxx>
#include <officecfg/Office/OptionsDialog.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/module.hxx>
#include <sfx2/printopt.hxx>
#include <sfx2/shell.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/flagitem.hxx>
#include <svl/intitem.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/ctloptions.hxx>
#include <svx/databaseregistrationui.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/urlobj.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/optionsdlg.hxx>
#include <unotools/viewoptions.hxx>
#include <utility>
#include <vcl/help.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weldutils.hxx>
#include <vcl/window.hxx>
#include <sal/log.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

LastPageSaver* OfaTreeOptionsDialog::pLastPageSaver = nullptr;

// some stuff for easier changes for SvtViewOptions
constexpr OUString VIEWOPT_DATANAME = u"page data"_ustr;

static void SetViewOptUserItem( SvtViewOptions& rOpt, const OUString& rData )
{
    rOpt.SetUserItem( VIEWOPT_DATANAME, Any( rData ) );
}

static OUString GetViewOptUserItem( const SvtViewOptions& rOpt )
{
    Any aAny( rOpt.GetUserItem( VIEWOPT_DATANAME ) );
    OUString aUserData;
    aAny >>= aUserData;

    return aUserData;
}

namespace {

struct ModuleToGroupNameMap_Impl
{
    std::u16string_view m_pModule;
    OUString    m_sGroupName;
    sal_uInt16  m_nNodeId;
};
}

static ModuleToGroupNameMap_Impl ModuleMap[] =
{
    { u"ProductName", OUString(), SID_GENERAL_OPTIONS },
    { u"LanguageSettings", OUString(), SID_LANGUAGE_OPTIONS },
    { u"Internet", OUString(), SID_INET_DLG },
    { u"LoadSave", OUString(), SID_FILTER_DLG },
    { u"Writer", OUString(), SID_SW_EDITOPTIONS },
    { u"WriterWeb", OUString(), SID_SW_ONLINEOPTIONS },
    { u"Math", OUString(), SID_SM_EDITOPTIONS },
    { u"Calc", OUString(), SID_SC_EDITOPTIONS },
    { u"Impress", OUString(), SID_SD_EDITOPTIONS },
    { u"Draw", OUString(), SID_SD_GRAPHIC_OPTIONS },
    { u"Charts", OUString(), SID_SCH_EDITOPTIONS },
    { u"Base", OUString(), SID_SB_STARBASEOPTIONS },
};

static void setGroupName( std::u16string_view rModule, const OUString& rGroupName )
{
    for (ModuleToGroupNameMap_Impl& rEntry : ModuleMap)
    {
        if ( rEntry.m_pModule == rModule )
        {
            rEntry.m_sGroupName = rGroupName;
            break;
        }
    }
}

static OUString getGroupName( std::u16string_view rModule, bool bForced )
{
    OUString sGroupName;
    for (const ModuleToGroupNameMap_Impl& rEntry : ModuleMap)
    {
        if ( rEntry.m_pModule == rModule )
        {
            sGroupName = rEntry.m_sGroupName;
            break;
        }
    }

    if ( sGroupName.isEmpty() && bForced )
    {
        if ( rModule == u"Writer" )
            sGroupName = CuiResId(SID_SW_EDITOPTIONS_RES[0].first);
        else if ( rModule == u"WriterWeb" )
            sGroupName = CuiResId(SID_SW_ONLINEOPTIONS_RES[0].first);
        else if ( rModule == u"Calc" )
            sGroupName = CuiResId(SID_SC_EDITOPTIONS_RES[0].first);
        else if ( rModule == u"Impress" )
            sGroupName = CuiResId(SID_SD_EDITOPTIONS_RES[0].first);
        else if ( rModule == u"Draw" )
            sGroupName = CuiResId(SID_SD_GRAPHIC_OPTIONS_RES[0].first);
        else if ( rModule == u"Math" )
            sGroupName = CuiResId(SID_SM_EDITOPTIONS_RES[0].first);
        else if ( rModule == u"Base" )
            sGroupName = CuiResId(SID_SB_STARBASEOPTIONS_RES[0].first);
    }
    return sGroupName;
}

static void deleteGroupNames()
{
    for (ModuleToGroupNameMap_Impl& rEntry : ModuleMap)
        rEntry.m_sGroupName.clear();
}

static sal_uInt16 getGroupNodeId( std::u16string_view rModule )
{
    sal_uInt16 nNodeId = 0xFFFF;
    for (const ModuleToGroupNameMap_Impl& rEntry : ModuleMap)
    {
        if ( rEntry.m_pModule == rModule )
        {
            nNodeId = rEntry.m_nNodeId;
            break;
        }
    }

    return nNodeId;
}

namespace {

bool MailMergeCfgIsEmailSupported()
{
    std::optional<bool> b = officecfg::Office::Writer::MailMergeWizard::EMailSupported::get();
    return b.has_value() && *b;
}

}

//typedef SfxTabPage* (*FNCreateTabPage)(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet &rAttrSet);
static std::unique_ptr<SfxTabPage> CreateGeneralTabPage(sal_uInt16 nId, weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
{
    CreateTabPage fnCreate = nullptr;
    switch(nId)
    {
        case RID_SFXPAGE_SAVE:                      fnCreate = &SvxSaveTabPage::Create; break;
        case RID_SFXPAGE_PATH:                      fnCreate = &SvxPathTabPage::Create; break;
        case RID_SFXPAGE_GENERAL:                   fnCreate = &SvxGeneralTabPage::Create; break;
        case RID_SFXPAGE_PRINTOPTIONS:              fnCreate = &SfxCommonPrintOptionsTabPage::Create; break;
        case OFA_TP_LANGUAGES:                      fnCreate = &OfaLanguagesTabPage::Create; break;
        case RID_SFXPAGE_LINGU:                     fnCreate = &SvxLinguTabPage::Create; break;
        case OFA_TP_VIEW:                           fnCreate = &OfaViewTabPage::Create; break;
        case OFA_TP_MISC:                           fnCreate = &OfaMiscTabPage::Create; break;
        case RID_SVXPAGE_ASIAN_LAYOUT:              fnCreate = &SvxAsianLayoutPage::Create; break;
        case RID_SVX_FONT_SUBSTITUTION:             fnCreate = &SvxFontSubstTabPage::Create; break;
        case RID_SVXPAGE_INET_PROXY:                fnCreate = &SvxProxyTabPage::Create; break;
        case RID_SVXPAGE_INET_SECURITY:             fnCreate = &SvxSecurityTabPage::Create; break;
        case RID_SVXPAGE_INET_MAIL:                 fnCreate = &SvxEMailTabPage::Create; break;
#if HAVE_FEATURE_DESKTOP
        case RID_SVXPAGE_PERSONALIZATION:           fnCreate = &SvxPersonalizationTabPage::Create; break;
#endif
        case RID_SVXPAGE_COLORCONFIG:               fnCreate = &SvxColorOptionsTabPage::Create; break;
        case RID_OFAPAGE_HTMLOPT:                   fnCreate = &OfaHtmlTabPage::Create; break;
        case SID_OPTFILTER_MSOFFICE:                fnCreate = &OfaMSFilterTabPage::Create; break;
        case RID_OFAPAGE_MSFILTEROPT2:              fnCreate = &OfaMSFilterTabPage2::Create; break;
        case RID_SVXPAGE_JSEARCH_OPTIONS:           fnCreate = &SvxJSearchOptionsPage::Create ; break;
        case SID_SB_CONNECTIONPOOLING:              fnCreate = &::offapp::ConnectionPoolOptionsPage::Create; break;
        case SID_SB_DBREGISTEROPTIONS:              fnCreate = &svx::DbRegistrationOptionsPage::Create; break;
        case RID_SVXPAGE_ACCESSIBILITYCONFIG:       fnCreate = &SvxAccessibilityOptionsTabPage::Create; break;
        case RID_SVXPAGE_OPTIONS_CTL:               fnCreate = &SvxCTLOptionsPage::Create ; break;
        case RID_SVXPAGE_LANGTOOL_OPTIONS:          fnCreate = &OptLanguageToolTabPage::Create ; break;
        case RID_SVXPAGE_DEEPL_OPTIONS:             fnCreate = &OptDeeplTabPage::Create ; break;
        case RID_SVXPAGE_OPTIONS_JAVA:              fnCreate = &SvxJavaOptionsPage::Create ; break;
#if HAVE_FEATURE_OPENCL
        case RID_SVXPAGE_OPENCL:                    fnCreate = &SvxOpenCLTabPage::Create ; break;
#endif
        case RID_SVXPAGE_ONLINEUPDATE:              fnCreate = &SvxOnlineUpdateTabPage::Create; break;
        case RID_OPTPAGE_CHART_DEFCOLORS:           fnCreate = &SvxDefaultColorOptPage::Create; break;
#if HAVE_FEATURE_SCRIPTING
        case RID_SVXPAGE_BASICIDE_OPTIONS:          fnCreate = &SvxBasicIDEOptionsPage::Create; break;
#endif
    }

    return fnCreate ? (*fnCreate)( pPage, pController, &rSet ) : nullptr;
}

namespace {

struct OptionsMapping_Impl
{
    const char* m_pGroupName;
    const char* m_pPageName;
    sal_uInt16      m_nPageId;
};

}

OptionsMapping_Impl const OptionsMap_Impl[] =
{
//    GROUP                 PAGE                    PAGE-ID
    { "ProductName",        nullptr,                SID_GENERAL_OPTIONS },
    { "ProductName",        "UserData",             RID_SFXPAGE_GENERAL },
    { "ProductName",        "General",              OFA_TP_MISC },
    { "ProductName",        "View",                 OFA_TP_VIEW },
    { "ProductName",        "Print",                RID_SFXPAGE_PRINTOPTIONS },
    { "ProductName",        "Paths",                RID_SFXPAGE_PATH },
    { "ProductName",        "Fonts",                RID_SVX_FONT_SUBSTITUTION },
    { "ProductName",        "Security",             RID_SVXPAGE_INET_SECURITY },
    { "ProductName",        "Personalization",      RID_SVXPAGE_PERSONALIZATION },
    { "ProductName",        "Appearance",           RID_SVXPAGE_COLORCONFIG },
    { "ProductName",        "Accessibility",        RID_SVXPAGE_ACCESSIBILITYCONFIG },
    { "ProductName",        "Java",                 RID_SVXPAGE_OPTIONS_JAVA },
    { "ProductName",        "BasicIDEOptions",      RID_SVXPAGE_BASICIDE_OPTIONS },
    { "ProductName",        "OnlineUpdate",         RID_SVXPAGE_ONLINEUPDATE },
    { "LanguageSettings",   nullptr,                SID_LANGUAGE_OPTIONS },
    { "LanguageSettings",   "Languages",            OFA_TP_LANGUAGES  },
    { "LanguageSettings",   "WritingAids",          RID_SFXPAGE_LINGU },
    { "LanguageSettings",   "SearchingInJapanese",  RID_SVXPAGE_JSEARCH_OPTIONS },
    { "LanguageSettings",   "AsianLayout",          RID_SVXPAGE_ASIAN_LAYOUT },
    { "LanguageSettings",   "ComplexTextLayout",    RID_SVXPAGE_OPTIONS_CTL },
    { "Internet",           nullptr,                SID_INET_DLG },
    { "Internet",           "Proxy",                RID_SVXPAGE_INET_PROXY },
    { "Internet",           "Email",                RID_SVXPAGE_INET_MAIL },
    { "LoadSave",           nullptr,                SID_FILTER_DLG },
    { "LoadSave",           "General",              RID_SFXPAGE_SAVE },
    { "LoadSave",           "VBAProperties",        SID_OPTFILTER_MSOFFICE },
    { "LoadSave",           "MicrosoftOffice",      RID_OFAPAGE_MSFILTEROPT2 },
    { "LoadSave",           "HTMLCompatibility",    RID_OFAPAGE_HTMLOPT },
    { "Writer",             nullptr,                SID_SW_EDITOPTIONS },
    { "Writer",             "General",              RID_SW_TP_OPTLOAD_PAGE },
    { "Writer",             "View",                 RID_SW_TP_CONTENT_OPT },
    { "Writer",             "FormattingAids",       RID_SW_TP_OPTSHDWCRSR },
    { "Writer",             "Grid",                 RID_SVXPAGE_GRID },
    { "Writer",             "BasicFontsWestern",    RID_SW_TP_STD_FONT },
    { "Writer",             "BasicFontsAsian",      RID_SW_TP_STD_FONT_CJK },
    { "Writer",             "BasicFontsCTL",        RID_SW_TP_STD_FONT_CTL },
    { "Writer",             "Print",                RID_SW_TP_OPTPRINT_PAGE },
    { "Writer",             "Table",                RID_SW_TP_OPTTABLE_PAGE },
    { "Writer",             "Changes",              RID_SW_TP_REDLINE_OPT },
    { "Writer",             "Comparison",           RID_SW_TP_COMPARISON_OPT },
    { "Writer",             "Compatibility",        RID_SW_TP_OPTCOMPATIBILITY_PAGE },
    { "Writer",             "AutoCaption",          RID_SW_TP_OPTCAPTION_PAGE },
    { "Writer",             "MailMerge",            RID_SW_TP_MAILCONFIG },
    { "WriterWeb",          nullptr,                SID_SW_ONLINEOPTIONS },
    { "WriterWeb",          "View",                 RID_SW_TP_HTML_CONTENT_OPT },
    { "WriterWeb",          "FormattingAids",       RID_SW_TP_HTML_OPTSHDWCRSR },
    { "WriterWeb",          "Grid",                 RID_SW_TP_HTML_OPTGRID_PAGE },
    { "WriterWeb",          "Print",                RID_SW_TP_HTML_OPTPRINT_PAGE },
    { "WriterWeb",          "Table",                RID_SW_TP_HTML_OPTTABLE_PAGE },
    { "WriterWeb",          "Background",           RID_SW_TP_BACKGROUND },
    { "Math",               nullptr,                SID_SM_EDITOPTIONS },
    { "Math",               "Settings",             SID_SM_TP_PRINTOPTIONS },
    { "Calc",               nullptr,                SID_SC_EDITOPTIONS },
    { "Calc",               "General",              SID_SC_TP_LAYOUT },
    { "Calc",               "View",                 SID_SC_TP_CONTENT },
    { "Calc",               "Calculate",            SID_SC_TP_CALC },
    { "Calc",               "Formula",              SID_SC_TP_FORMULA },
    { "Calc",               "SortLists",            SID_SC_TP_USERLISTS },
    { "Calc",               "Changes",              SID_SC_TP_CHANGES },
    { "Calc",               "Compatibility",        SID_SC_TP_COMPATIBILITY },
    { "Calc",               "Grid",                 SID_SC_TP_GRID },
    { "Calc",               "Print",                RID_SC_TP_PRINT },
    { "Impress",            nullptr,                SID_SD_EDITOPTIONS },
    { "Impress",            "General",              SID_SI_TP_MISC },
    { "Impress",            "View",                 SID_SI_TP_CONTENTS },
    { "Impress",            "Grid",                 SID_SI_TP_SNAP },
    { "Impress",            "Print",                SID_SI_TP_PRINT },
    { "Draw",               nullptr,                SID_SD_GRAPHIC_OPTIONS },
    { "Draw",               "General",              SID_SD_TP_MISC },
    { "Draw",               "View",                 SID_SD_TP_CONTENTS },
    { "Draw",               "Grid",                 SID_SD_TP_SNAP },
    { "Draw",               "Print",                SID_SD_TP_PRINT },
    { "Charts",             nullptr,                SID_SCH_EDITOPTIONS },
    { "Charts",             "DefaultColors",        RID_OPTPAGE_CHART_DEFCOLORS },
    { "Base",               nullptr,                SID_SB_STARBASEOPTIONS },
    { "Base",               "Connections",          SID_SB_CONNECTIONPOOLING },
    { "Base",               "Databases",            SID_SB_DBREGISTEROPTIONS },
    { nullptr,                 nullptr,             0 }
};

static bool lcl_getStringFromID( sal_uInt16 _nPageId, OUString& _rGroupName, OUString& _rPageName )
{
    bool bRet = false;

    sal_uInt16 nIdx = 0;
    while ( OptionsMap_Impl[nIdx].m_pGroupName != nullptr )
    {
        if ( _nPageId == OptionsMap_Impl[nIdx].m_nPageId )
        {
            bRet = true;
            _rGroupName = OUString::createFromAscii( OptionsMap_Impl[nIdx].m_pGroupName );
            if ( OptionsMap_Impl[nIdx].m_pPageName != nullptr )
                _rPageName = OUString::createFromAscii( OptionsMap_Impl[nIdx].m_pPageName );
            break;
        }
        ++nIdx;
    }

    return bRet;
}

static bool lcl_isOptionHidden( sal_uInt16 _nPageId, const SvtOptionsDialogOptions& _rOptOptions )
{
    bool bIsHidden = false;
    OUString sGroupName, sPageName;
    if ( lcl_getStringFromID( _nPageId, sGroupName, sPageName ) )
    {
        if ( sPageName.isEmpty() )
            bIsHidden =  _rOptOptions.IsGroupHidden( sGroupName );
        else
            bIsHidden =  _rOptOptions.IsPageHidden( sPageName, sGroupName );
    }
    return bIsHidden;
}

struct OptionsPageInfo
{
    std::unique_ptr<SfxTabPage> m_xPage;
    sal_uInt16          m_nPageId;
    OUString       m_sPageURL;
    OUString       m_sEventHdl;
    std::unique_ptr<ExtensionsTabPage>  m_xExtPage;

    explicit OptionsPageInfo( sal_uInt16 nId ) : m_nPageId( nId ) {}
};

struct OptionsGroupInfo
{
    std::optional<SfxItemSet> m_pInItemSet;
    std::unique_ptr<SfxItemSet> m_pOutItemSet;
    SfxShell*           m_pShell;       // used to create the page
    SfxModule*          m_pModule;      // used to create the ItemSet
    sal_uInt16          m_nDialogId;    // Id of the former dialog

    OptionsGroupInfo( SfxShell* pSh, SfxModule* pMod, sal_uInt16 nId ) :
        m_pShell( pSh ),
        m_pModule( pMod ), m_nDialogId( nId ) {}
};

struct OptionsPageIdInfo
{
    OUString m_sParentName;
    OUString m_sPageName;
    sal_uInt16 m_nParentId;
    sal_uInt16 m_nPageId;

    OptionsPageIdInfo(OUString sParentName, OUString sPageName, sal_uInt16 nParentId,
                      sal_uInt16 nPageId)
        : m_sParentName(std::move(sParentName))
        , m_sPageName(std::move(sPageName))
        , m_nParentId(nParentId)
        , m_nPageId(nPageId)
    {
    }
};

// Basic ctor with common initialization
OfaTreeOptionsDialog::OfaTreeOptionsDialog(weld::Window* pParent, bool fromExtensionManager)
    : SfxOkDialogController(pParent, "cui/ui/optionsdialog.ui", "OptionsDialog")
    , xOkPB(m_xBuilder->weld_button("ok"))
    , xApplyPB(m_xBuilder->weld_button("apply"))
    , xBackPB(m_xBuilder->weld_button("revert"))
    , xTreeLB(m_xBuilder->weld_tree_view("pages"))
    , xTabBox(m_xBuilder->weld_container("box"))
    , m_xSearchEdit(m_xBuilder->weld_entry("searchEntry"))
    , m_pParent(pParent)
    , m_aUpdateDataTimer("OfaTreeOptionsDialog UpdateDataTimer")
    , bIsFirtsInitialize(true)
    , sTitle(m_xDialog->get_title())
    , bForgetSelection(false)
    , bIsFromExtensionManager(fromExtensionManager)
    , bIsForSetDocumentLanguage(false)
    , bNeedsRestart(false)
    , eRestartReason(svtools::RESTART_REASON_NONE)
{
    Size aSize(xTreeLB->get_approximate_digit_width() * 82, xTreeLB->get_height_rows(30));
#if HAVE_FEATURE_GPGME
    // tdf#115015: make enough space for crypto settings (approx. 14 text edits + padding)
    aSize.setHeight((weld::GetMinimumEditHeight() + 6) * 14);
#endif
    xTabBox->set_size_request(aSize.Width(), aSize.Height());
    xTreeLB->set_size_request(xTreeLB->get_approximate_digit_width() * 35, aSize.Height());

    // Init tree and handler
    xTreeLB->set_help_id(HID_OFADLG_TREELISTBOX);
    xTreeLB->connect_changed(LINK(this, OfaTreeOptionsDialog, ShowPageHdl_Impl));
    xBackPB->connect_clicked(LINK(this, OfaTreeOptionsDialog, BackHdl_Impl));
    xApplyPB->connect_clicked(LINK(this, OfaTreeOptionsDialog, ApplyHdl_Impl));
    xOkPB->connect_clicked(LINK(this, OfaTreeOptionsDialog, ApplyHdl_Impl));
    m_xSearchEdit->connect_changed(LINK(this, OfaTreeOptionsDialog, SearchUpdateHdl));
    m_xSearchEdit->connect_focus_out(LINK(this, OfaTreeOptionsDialog, FocusOut_Impl));
    m_xDialog->connect_help(LINK(this, OfaTreeOptionsDialog, HelpHdl_Impl));

    m_aUpdateDataTimer.SetInvokeHandler(LINK(this, OfaTreeOptionsDialog, ImplUpdateDataHdl));
    m_aUpdateDataTimer.SetTimeout(EDIT_UPDATEDATA_TIMEOUT);

    // Initialize search util
    m_options.AlgorithmType2 = util::SearchAlgorithms2::ABSOLUTE;
    m_options.transliterateFlags |= TransliterationFlags::IGNORE_CASE;
    m_options.searchFlag
        |= (util::SearchFlags::REG_NOT_BEGINOFLINE | util::SearchFlags::REG_NOT_ENDOFLINE);

    xTreeLB->set_accessible_name(sTitle);
}

// Ctor() with Frame -----------------------------------------------------
OfaTreeOptionsDialog::OfaTreeOptionsDialog(weld::Window* pParent, const Reference< XFrame >& _xFrame, bool bActivateLastSelection)
    : OfaTreeOptionsDialog(pParent, false)
{
    Initialize( _xFrame );
    LoadExtensionOptions( u"" );
    if (bActivateLastSelection)
        ActivateLastSelection();
}

// Ctor() with ExtensionId -----------------------------------------------
OfaTreeOptionsDialog::OfaTreeOptionsDialog(weld::Window* pParent, std::u16string_view rExtensionId)
    : OfaTreeOptionsDialog(pParent, !rExtensionId.empty())
{
    LoadExtensionOptions( rExtensionId );
    ActivateLastSelection();
}

void OfaTreeOptionsDialog::ImplDestroy()
{
    xCurrentPageEntry.reset();

    std::unique_ptr<weld::TreeIter> xEntry = xTreeLB->make_iterator();
    bool bEntry = xTreeLB->get_iter_first(*xEntry);
    // first children
    while (bEntry)
    {
        // if Child (has parent), then OptionsPageInfo
        if (xTreeLB->get_iter_depth(*xEntry))
        {
            OptionsPageInfo *pPageInfo = weld::fromId<OptionsPageInfo*>(xTreeLB->get_id(*xEntry));
            if(pPageInfo->m_xPage)
            {
                pPageInfo->m_xPage->FillUserData();
                OUString aPageData(pPageInfo->m_xPage->GetUserData());
                if ( !aPageData.isEmpty() )
                {
                    SvtViewOptions aTabPageOpt( EViewType::TabPage, OUString::number( pPageInfo->m_nPageId) );
                    SetViewOptUserItem( aTabPageOpt, aPageData );
                }
                pPageInfo->m_xPage.reset();
            }

            if (pPageInfo->m_nPageId == RID_SFXPAGE_LINGU)
            {
                // write personal dictionaries
                Reference< XSearchableDictionaryList >  xDicList( LinguMgr::GetDictionaryList() );
                if (xDicList.is())
                {
                    linguistic::SaveDictionaries( xDicList );
                }
            }

            pPageInfo->m_xExtPage.reset();

            delete pPageInfo;
        }
        bEntry = xTreeLB->iter_next(*xEntry);
    }

    // and parents
    bEntry = xTreeLB->get_iter_first(*xEntry);
    while (bEntry)
    {
        if (!xTreeLB->get_iter_depth(*xEntry))
        {
            OptionsGroupInfo* pGroupInfo = weld::fromId<OptionsGroupInfo*>(xTreeLB->get_id(*xEntry));
            delete pGroupInfo;
        }
        bEntry = xTreeLB->iter_next(*xEntry);
    }
    deleteGroupNames();
}

OfaTreeOptionsDialog::~OfaTreeOptionsDialog()
{
    suppress_fun_call_w_exception(ImplDestroy());
}

OptionsPageInfo* OfaTreeOptionsDialog::AddTabPage(
    sal_uInt16 nId, const OUString& rPageName, sal_uInt16 nGroup )
{
    std::unique_ptr<weld::TreeIter> xParent = xTreeLB->make_iterator();
    if (!xTreeLB->get_iter_first(*xParent))
        return nullptr;
    xTreeLB->iter_nth_sibling(*xParent, nGroup);

    OptionsPageInfo* pPageInfo = new OptionsPageInfo( nId );
    OUString sId(weld::toId(pPageInfo));
    xTreeLB->insert(xParent.get(), -1, &rPageName, &sId, nullptr, nullptr, false, nullptr);
    return pPageInfo;
}

// the ItemSet* is passed on to the dialog's ownership
sal_uInt16  OfaTreeOptionsDialog::AddGroup(const OUString& rGroupName,
                                        SfxShell* pCreateShell,
                                        SfxModule* pCreateModule,
                                        sal_uInt16 nDialogId )
{
    OptionsGroupInfo* pInfo =
        new OptionsGroupInfo( pCreateShell, pCreateModule, nDialogId );
    OUString sId(weld::toId(pInfo));
    xTreeLB->append(sId, rGroupName);

    sal_uInt16 nRet = 0;
    std::unique_ptr<weld::TreeIter> xEntry = xTreeLB->make_iterator();
    bool bEntry = xTreeLB->get_iter_first(*xEntry);
    while (bEntry)
    {
        if (!xTreeLB->get_iter_depth(*xEntry))
            nRet++;
        bEntry = xTreeLB->iter_next(*xEntry);
    }
    return nRet - 1;
}

IMPL_LINK_NOARG(OfaTreeOptionsDialog, ShowPageHdl_Impl, weld::TreeView&, void)
{
    SelectHdl_Impl();
}

void OfaTreeOptionsDialog::ResetCurrentPageFromConfig()
{
    if (!(xCurrentPageEntry && xTreeLB->get_iter_depth(*xCurrentPageEntry)))
        return;

    OptionsPageInfo* pPageInfo = weld::fromId<OptionsPageInfo*>(xTreeLB->get_id(*xCurrentPageEntry));
    if (pPageInfo->m_xPage)
    {
        std::unique_ptr<weld::TreeIter> xParent = xTreeLB->make_iterator(xCurrentPageEntry.get());
        xTreeLB->iter_parent(*xParent);
        OptionsGroupInfo* pGroupInfo =
            weld::fromId<OptionsGroupInfo*>(xTreeLB->get_id(*xParent));
        pPageInfo->m_xPage->Reset( &*pGroupInfo->m_pInItemSet );
    }
    else if ( pPageInfo->m_xExtPage )
        pPageInfo->m_xExtPage->ResetPage();
}

IMPL_LINK_NOARG(OfaTreeOptionsDialog, BackHdl_Impl, weld::Button&, void)
{
    ResetCurrentPageFromConfig();
}

void OfaTreeOptionsDialog::ApplyOptions()
{
    std::unique_ptr<weld::TreeIter> xEntry = xTreeLB->make_iterator();
    bool bEntry = xTreeLB->get_iter_first(*xEntry);
    while (bEntry)
    {
        if (xTreeLB->get_iter_depth(*xEntry))
        {
            OptionsPageInfo* pPageInfo = weld::fromId<OptionsPageInfo*>(xTreeLB->get_id(*xEntry));
            if ( pPageInfo->m_xPage && !pPageInfo->m_xPage->HasExchangeSupport() )
            {
                std::unique_ptr<weld::TreeIter> xParent = xTreeLB->make_iterator(xEntry.get());
                xTreeLB->iter_parent(*xParent);
                OptionsGroupInfo* pGroupInfo =
                    weld::fromId<OptionsGroupInfo*>(xTreeLB->get_id(*xParent));
                pPageInfo->m_xPage->FillItemSet(pGroupInfo->m_pOutItemSet.get());
            }

            if ( pPageInfo->m_xExtPage )
            {
                pPageInfo->m_xExtPage->DeactivatePage();
                pPageInfo->m_xExtPage->SavePage();
            }
            if ( pPageInfo->m_xPage && RID_OPTPAGE_CHART_DEFCOLORS == pPageInfo->m_nPageId )
            {
                SvxDefaultColorOptPage* pPage = static_cast<SvxDefaultColorOptPage *>(pPageInfo->m_xPage.get());
                pPage->SaveChartOptions();
            }
        }
        bEntry = xTreeLB->iter_next(*xEntry);
    }
}

IMPL_LINK_NOARG(OfaTreeOptionsDialog, HelpHdl_Impl, weld::Widget&, bool)
{
    Help* pHelp = Application::GetHelp();
    if (pHelp && xCurrentPageEntry && xTreeLB->get_iter_depth(*xCurrentPageEntry))
    {
        OptionsPageInfo* pPageInfo = weld::fromId<OptionsPageInfo*>(xTreeLB->get_id(*xCurrentPageEntry));
        if (pPageInfo->m_xPage)
        {
            OUString sHelpId(pPageInfo->m_xPage->GetHelpId());
            pHelp->Start(sHelpId, m_xDialog.get());
            return false;
        }
    }
    return true;
}

IMPL_LINK(OfaTreeOptionsDialog, ApplyHdl_Impl, weld::Button&, rButton, void)
{
    bool bOkPressed = &rButton == xOkPB.get();

    OptionsGroupInfo* pGroupInfo = nullptr;

    if (xCurrentPageEntry && xTreeLB->get_iter_depth(*xCurrentPageEntry))
    {
        OptionsPageInfo* pPageInfo = weld::fromId<OptionsPageInfo*>(xTreeLB->get_id(*xCurrentPageEntry));
        if ( pPageInfo->m_xPage )
        {
            std::unique_ptr<weld::TreeIter> xParent = xTreeLB->make_iterator(xCurrentPageEntry.get());
            xTreeLB->iter_parent(*xParent);

            pGroupInfo = weld::fromId<OptionsGroupInfo*>(xTreeLB->get_id(*xParent));
            if ( RID_SVXPAGE_COLOR != pPageInfo->m_nPageId
                && pPageInfo->m_xPage->HasExchangeSupport() )
            {
                DeactivateRC nLeave = pPageInfo->m_xPage->DeactivatePage(pGroupInfo->m_pOutItemSet.get());
                if ( nLeave == DeactivateRC::KeepPage )
                {
                    // the page mustn't be left, so return early
                    assert(xTreeLB->is_selected(*xCurrentPageEntry)); // presumably this must be true here
                    if (bOkPressed)
                        return;
                }
            }
        }
    }

    ApplyOptions();
    ApplyItemSets();
    utl::ConfigManager::storeConfigItems();

    if (bOkPressed)
        m_xDialog->response(RET_OK);
    else
    {
        // tdf#137930 rebuild the in and out itemsets to reflect the current
        // post-apply state
        if (pGroupInfo && pGroupInfo->m_pInItemSet)
        {
            // tdf#138596 seeing as the SfxTabPages keep pointers to the m_pInItemSet
            // we update the contents of the existing SfxItemSets to match
            // the current settings, rather than create new ones
            auto xInItemSet = pGroupInfo->m_pShell
                ? pGroupInfo->m_pShell->CreateItemSet( pGroupInfo->m_nDialogId )
                : CreateItemSet( pGroupInfo->m_nDialogId );
            pGroupInfo->m_pInItemSet->Set(*xInItemSet, false);
            pGroupInfo->m_pOutItemSet->ClearItem();
        }

        // for the Apply case, now that the settings are saved to config,
        // reload the current page so it knows what the config now states
        ResetCurrentPageFromConfig();
        // reselect it to undo possible DeactivatePage above
        xCurrentPageEntry.reset();
        SelectHdl_Impl();
    }

    if (!bNeedsRestart)
        return;

    SolarMutexGuard aGuard;
    weld::Window* pParent;
    if (!bOkPressed)
        pParent = m_xDialog.get();
    else
    {
        m_xDialog->hide();
        pParent = m_pParent;
    }
    bool bRestart = ::svtools::executeRestartDialog(comphelper::getProcessComponentContext(),
                                                    pParent, eRestartReason);
    if (bRestart && !bOkPressed)
        m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(OfaTreeOptionsDialog, FocusOut_Impl, weld::Widget&, void)
{
    if (m_aUpdateDataTimer.IsActive())
    {
        m_aUpdateDataTimer.Stop();
        m_aUpdateDataTimer.Invoke();
    }
}

IMPL_LINK_NOARG(OfaTreeOptionsDialog, SearchUpdateHdl, weld::Entry&, void)
{
    m_aUpdateDataTimer.Start();
}

IMPL_LINK_NOARG(OfaTreeOptionsDialog, ImplUpdateDataHdl, Timer*, void)
{
    // initializeFirstNDialog() can take a long time, show wait cursor and disable input
    std::unique_ptr<weld::WaitObject> xWait(m_pParent ? new weld::WaitObject(m_pParent) : nullptr);

    // Pause redraw
    xTreeLB->freeze();

    if (bIsFirtsInitialize)
    {
        m_xSearchEdit->freeze();
        xTreeLB->hide();

        initializeFirstNDialog();

        m_xSearchEdit->thaw();
        xTreeLB->show();
        bIsFirtsInitialize = false;
    }

    // Apply the search filter
    OUString aSearchTerm(m_xSearchEdit->get_text());
    int nMatchFound = applySearchFilter(aSearchTerm);

    // Resume redraw
    xTreeLB->thaw();

    // select first child of first node after the search done
    if (nMatchFound != -1)
        selectFirstEntry();
}

void OfaTreeOptionsDialog::selectFirstEntry()
{
    std::unique_ptr<weld::TreeIter> xTemp = xTreeLB->make_iterator();
    bool bTemp = xTreeLB->get_iter_first(*xTemp);

    while (bTemp)
    {
        // select only the first child
        if (xTreeLB->get_iter_depth(*xTemp) && xTreeLB->get_id(*xTemp).toInt64())
        {
            std::unique_ptr<weld::TreeIter> xEntry(xTreeLB->make_iterator(xTemp.get()));

            std::unique_ptr<weld::TreeIter> xParent(xTreeLB->make_iterator(xEntry.get()));
            xTreeLB->iter_parent(*xParent);
            xTreeLB->expand_row(*xParent);
            xTreeLB->scroll_to_row(*xParent);
            xTreeLB->scroll_to_row(*xEntry);
            xTreeLB->set_cursor(*xEntry);
            xTreeLB->select(*xEntry);
            SelectHdl_Impl();

            break;
        }
        bTemp = xTreeLB->iter_next(*xTemp);
    }
}

void OfaTreeOptionsDialog::initializeFirstNDialog(sal_Int16 nNumberOfNode)
{
    std::unique_ptr<weld::TreeIter> xEntry;
    sal_Int16 nCount = 0;

    std::unique_ptr<weld::TreeIter> xTemp = xTreeLB->make_iterator();
    bool bTemp = xTreeLB->get_iter_first(*xTemp);

    while (bTemp)
    {
        if (xTreeLB->get_iter_depth(*xTemp) && xTreeLB->get_id(*xTemp).toInt64())
        {
            if (!(xEntry = xTreeLB->make_iterator(xTemp.get())))
            {
                xEntry = xTreeLB->make_iterator();
                if (!xTreeLB->get_iter_first(*xEntry) || !xTreeLB->iter_next(*xEntry))
                    xEntry.reset();
            }

            if (!xEntry)
                return;

            OptionsPageInfo* pPageInfo = weld::fromId<OptionsPageInfo*>(xTreeLB->get_id(*xEntry));
            if (TreeOptHelper::getStringsFromDialog(pPageInfo->m_nPageId).isEmpty())
            {
                SolarMutexGuard aGuard;
                initializeCurrentDialog(pPageInfo, xEntry);
            }
        }

        /* if nNumberOfNode is -1 (which is the default value if no parameter provided),
           this function will initialize all dialogs since nCount always greater than -1 */
        if (nCount == nNumberOfNode)
            break;

        ++nCount;
        bTemp = xTreeLB->iter_next(*xTemp);
    }
}

void OfaTreeOptionsDialog::clearOptionsDialog()
{
    // clear and reset
    std::unique_ptr<weld::TreeIter> xEntry = xTreeLB->make_iterator();
    bool bEntry = xTreeLB->get_iter_first(*xEntry);

    while (bEntry)
    {
        if (xTreeLB->get_iter_depth(*xEntry))
        {
            OptionsPageInfo* pPageInfo = weld::fromId<OptionsPageInfo*>(xTreeLB->get_id(*xEntry));
            if (pPageInfo->m_xPage)
                pPageInfo->m_xPage.reset();
        }
        bEntry = xTreeLB->iter_next(*xEntry);
    }
    xCurrentPageEntry.reset();
}

// store Options tree with their page names
void OfaTreeOptionsDialog::storeOptionsTree()
{
    m_aTreePageIds.clear();

    sal_uInt16 aParentId = 0;
    OUString sParentName;

    std::unique_ptr<weld::TreeIter> xEntry = xTreeLB->make_iterator();
    bool bEntry = xTreeLB->get_iter_first(*xEntry);

    while (bEntry)
    {
        OptionsGroupInfo* pGroupInfo = weld::fromId<OptionsGroupInfo*>(xTreeLB->get_id(*xEntry));
        bool bIsParentNode = !xTreeLB->get_iter_depth(*xEntry);

        if (bIsParentNode)
        {
            // Parent node
            sParentName = xTreeLB->get_text(*xEntry);
            aParentId = pGroupInfo->m_nDialogId;
            m_aTreePageIds.push_back(new OptionsPageIdInfo(sParentName, sParentName, aParentId, 0));
        }
        else
        {
            // Child node
            OUString sChildName = xTreeLB->get_text(*xEntry);
            OptionsPageInfo* pPageInfo = weld::fromId<OptionsPageInfo*>(xTreeLB->get_id(*xEntry));
            m_aTreePageIds.push_back(
                new OptionsPageIdInfo(sParentName, sChildName, aParentId, pPageInfo->m_nPageId));
        }
        bEntry = xTreeLB->iter_next(*xEntry);
    }
}

int OfaTreeOptionsDialog::applySearchFilter(const OUString& rSearchTerm)
{
    if (rSearchTerm.isEmpty())
    {
        clearOptionsDialog();
        xTreeLB->clear();
        Initialize(m_xFrame);

        return 0;
    }

    m_options.searchString = rSearchTerm;
    utl::TextSearch textSearch(m_options);

    clearOptionsDialog();

    if (xTreeLB->n_children() > 0)
        xTreeLB->clear();

    std::vector<std::pair<sal_uInt16, std::vector<sal_uInt16>>> aFoundIdsVector;

    for (std::size_t i = 0; i < m_aTreePageIds.size(); ++i)
    {
        const OUString sParentName = m_aTreePageIds[i]->m_sParentName;
        const OUString sPageName = m_aTreePageIds[i]->m_sPageName;
        const sal_uInt16 nParentId = m_aTreePageIds[i]->m_nParentId;
        const sal_uInt16 nPageId = m_aTreePageIds[i]->m_nPageId;

        const OUString sPageStrings = TreeOptHelper::getStringsFromDialog(nPageId);
        const OUString sPageNameAndStrings = sParentName + " " + sPageName + " " + sPageStrings;

        sal_Int32 aStartPos = 0;
        sal_Int32 aEndPos = sPageNameAndStrings.getLength();

        // check if rSearchTerm matches with sPageNameAndStrings
        if (textSearch.SearchForward(sPageNameAndStrings, &aStartPos, &aEndPos))
        {
            bool isFound = false;
            for (auto& aEntryId : aFoundIdsVector)
            {
                if (aEntryId.first == nParentId)
                {
                    isFound = true;
                    aEntryId.second.push_back(nPageId);
                }
            }

            if (!isFound)
                aFoundIdsVector.push_back({ nParentId, { nPageId } });
        }
    }

    showDialog(aFoundIdsVector);

    // if treeview is empty, return -1
    return xTreeLB->n_children() ? 0 : -1;
}

void OfaTreeOptionsDialog::showDialog(VectorOfMatchedIds& pSearchIds)
{
    //
    //     void generalOptions();      // SID_GENERAL_OPTIONS
    //     void loadAndSaveOptions();  // SID_FILTER_DLG
    //     void languageOptions();     // SID_LANGUAGE_OPTIONS
    //     void writerOptions();       // SID_SW_EDITOPTIONS
    //     void writerWebOptions();    // SID_SW_ONLINEOPTIONS
    //     void calcOptions();         // SID_SC_EDITOPTIONS
    //     void impressOptions();      // SID_SD_EDITOPTIONS
    //     void drawOptions();         // SID_SD_GRAPHIC_OPTIONS
    //     void mathOptions();         // SID_SM_EDITOPTIONS
    //     void databaseOptions();     // SID_SB_STARBASEOPTIONS
    //     void chartOptions();        // SID_SCH_EDITOPTIONS
    //     void internetOptions();     // SID_INET_DLG
    //

    for (auto& aEntryId : pSearchIds)
    {
        switch (aEntryId.first)
        {
            case SID_GENERAL_OPTIONS:
                generalOptions(aEntryId.second);
                break;

            case SID_FILTER_DLG:
                loadAndSaveOptions(aEntryId.second);
                break;

            case SID_LANGUAGE_OPTIONS:
                languageOptions(aEntryId.second);
                break;

            case SID_SW_EDITOPTIONS:
                writerOptions(aEntryId.second);
                break;

            case SID_SW_ONLINEOPTIONS:
                writerWebOptions(aEntryId.second);
                break;

            case SID_SC_EDITOPTIONS:
                calcOptions(aEntryId.second);
                break;

            case SID_SD_EDITOPTIONS:
                impressOptions(aEntryId.second);
                break;

            case SID_SD_GRAPHIC_OPTIONS:
                drawOptions(aEntryId.second);
                break;

            case SID_SM_EDITOPTIONS:
                mathOptions(aEntryId.second);
                break;

            case SID_SB_STARBASEOPTIONS:
                databaseOptions(aEntryId.second);
                break;

            case SID_SCH_EDITOPTIONS:
                chartOptions(aEntryId.second);
                break;

            case SID_INET_DLG:
                internetOptions(aEntryId.second);
                break;

            default:
                break;
        }
    }
}

void OfaTreeOptionsDialog::ApplyItemSets()
{
    std::unique_ptr<weld::TreeIter> xEntry = xTreeLB->make_iterator();
    bool bEntry = xTreeLB->get_iter_first(*xEntry);
    while (bEntry)
    {
        if (!xTreeLB->get_iter_depth(*xEntry))
        {
            OptionsGroupInfo* pGroupInfo = weld::fromId<OptionsGroupInfo*>(xTreeLB->get_id(*xEntry));
            if(pGroupInfo->m_pOutItemSet)
            {
                if(pGroupInfo->m_pShell)
                    pGroupInfo->m_pShell->ApplyItemSet( pGroupInfo->m_nDialogId, *pGroupInfo->m_pOutItemSet);
                else
                    ApplyItemSet( pGroupInfo->m_nDialogId, *pGroupInfo->m_pOutItemSet);
            }
        }
        bEntry = xTreeLB->iter_next(*xEntry);
    }
}

void OfaTreeOptionsDialog::ActivatePage( sal_uInt16 nResId )
{
    bIsForSetDocumentLanguage = false;
    if ( nResId == OFA_TP_LANGUAGES_FOR_SET_DOCUMENT_LANGUAGE )
    {
        bIsForSetDocumentLanguage = true;
        nResId = OFA_TP_LANGUAGES;
    }

    DBG_ASSERT( !bIsFromExtensionManager, "OfaTreeOptionsDialog::ActivatePage(): call from extension manager" );
    if ( !pLastPageSaver )
        pLastPageSaver = new LastPageSaver;
    bForgetSelection = true;
    sal_uInt16 nTemp = pLastPageSaver->m_nLastPageId;
    pLastPageSaver->m_nLastPageId = nResId;
    ActivateLastSelection();
    pLastPageSaver->m_nLastPageId = nTemp;
}

void OfaTreeOptionsDialog::ActivatePage( const OUString& rPageURL )
{
    DBG_ASSERT( !bIsFromExtensionManager, "OfaTreeOptionsDialog::ActivatePage(): call from extension manager" );
    if ( !pLastPageSaver )
        pLastPageSaver = new LastPageSaver;
    bForgetSelection = true;
    pLastPageSaver->m_nLastPageId = 0;
    pLastPageSaver->m_sLastPageURL_Tools = rPageURL;
    ActivateLastSelection();
}

void OfaTreeOptionsDialog::ActivateLastSelection()
{
    std::unique_ptr<weld::TreeIter> xEntry;

    if (pLastPageSaver)
    {
        OUString sLastURL = bIsFromExtensionManager ? pLastPageSaver->m_sLastPageURL_ExtMgr
                                                  : pLastPageSaver->m_sLastPageURL_Tools;
        if ( sLastURL.isEmpty() )
        {
            sLastURL = !bIsFromExtensionManager ? pLastPageSaver->m_sLastPageURL_ExtMgr
                                                : pLastPageSaver->m_sLastPageURL_Tools;
        }

        bool bMustExpand = ( INetURLObject( sLastURL ).GetProtocol() == INetProtocol::File );

        std::unique_ptr<weld::TreeIter> xTemp = xTreeLB->make_iterator();
        bool bTemp = xTreeLB->get_iter_first(*xTemp);
        while (bTemp)
        {
            // restore only selection of a leaf
            if (xTreeLB->get_iter_depth(*xTemp) && xTreeLB->get_id(*xTemp).toInt64())
            {
                OptionsPageInfo* pPageInfo = weld::fromId<OptionsPageInfo*>(xTreeLB->get_id(*xTemp));
                OUString sPageURL = pPageInfo->m_sPageURL;
                if ( bMustExpand )
                {
                    sPageURL = comphelper::getExpandedUri(
                        comphelper::getProcessComponentContext(), sPageURL);
                }

                if ( ( !bIsFromExtensionManager
                        && pPageInfo->m_nPageId && pPageInfo->m_nPageId == pLastPageSaver->m_nLastPageId )
                            || ( !pPageInfo->m_nPageId && sLastURL == sPageURL ) )
                {
                    xEntry = xTreeLB->make_iterator(xTemp.get());
                    break;
                }
            }
            bTemp = xTreeLB->iter_next(*xTemp);
        }
    }

    if (!xEntry)
    {
        xEntry = xTreeLB->make_iterator();
        if (!xTreeLB->get_iter_first(*xEntry) || !xTreeLB->iter_next(*xEntry))
            xEntry.reset();
    }

    if (!xEntry)
        return;

    std::unique_ptr<weld::TreeIter> xParent(xTreeLB->make_iterator(xEntry.get()));
    xTreeLB->iter_parent(*xParent);
    xTreeLB->expand_row(*xParent);
    xTreeLB->scroll_to_row(*xParent);
    xTreeLB->scroll_to_row(*xEntry);
    xTreeLB->set_cursor(*xEntry);
    xTreeLB->select(*xEntry);
    m_xSearchEdit->grab_focus();
    SelectHdl_Impl();

    // initializeFirstNDialog() can take a long time, show wait cursor
    std::unique_ptr<weld::WaitObject> xWait(m_pParent ? new weld::WaitObject(m_pParent) : nullptr);

    /* initialize first 25 dialogs which are almost half of the dialogs
    in a row while Options dialog opens. then clear&reselect to avoid UI test failures. */
    initializeFirstNDialog(25);
    clearOptionsDialog();
    SelectHdl_Impl();
}

void OfaTreeOptionsDialog::InitItemSets(OptionsGroupInfo& rGroupInfo)
{
    if (!rGroupInfo.m_pInItemSet)
        rGroupInfo.m_pInItemSet.emplace( rGroupInfo.m_pShell
            ? *rGroupInfo.m_pShell->CreateItemSet( rGroupInfo.m_nDialogId )
            : *CreateItemSet( rGroupInfo.m_nDialogId ) );
    if (!rGroupInfo.m_pOutItemSet)
        rGroupInfo.m_pOutItemSet = std::make_unique<SfxItemSet>(
            *rGroupInfo.m_pInItemSet->GetPool(),
            rGroupInfo.m_pInItemSet->GetRanges());
}

void OfaTreeOptionsDialog::initializeCurrentDialog(OptionsPageInfo*& pPageInfo,
                                                   std::unique_ptr<weld::TreeIter>& xEntry)
{
    std::unique_ptr<weld::TreeIter> xParent(xTreeLB->make_iterator(xEntry.get()));
    bool bParent = xTreeLB->iter_parent(*xParent);

    if (!bParent)
        return;

    if (pPageInfo->m_xPage)
    {
        TreeOptHelper::storeStringsOfDialog(pPageInfo->m_nPageId,
                                            pPageInfo->m_xPage->GetAllStrings());
        return;
    }

    OptionsGroupInfo* pGroupInfo = weld::fromId<OptionsGroupInfo*>(xTreeLB->get_id(*xParent));

    if (!pPageInfo->m_xPage && pPageInfo->m_nPageId > 0)
    {
        InitItemSets(*pGroupInfo);

        pPageInfo->m_xPage = ::CreateGeneralTabPage(pPageInfo->m_nPageId, xTabBox.get(), this,
                                                    *pGroupInfo->m_pInItemSet);

        if (!pPageInfo->m_xPage && pGroupInfo->m_pModule)
            pPageInfo->m_xPage = pGroupInfo->m_pModule->CreateTabPage(
                pPageInfo->m_nPageId, xTabBox.get(), this, *pGroupInfo->m_pInItemSet);

        DBG_ASSERT(pPageInfo->m_xPage, "tabpage could not created");
        if (pPageInfo->m_xPage)
        {
            SvtViewOptions aTabPageOpt(EViewType::TabPage, OUString::number(pPageInfo->m_nPageId));
            pPageInfo->m_xPage->SetUserData(GetViewOptUserItem(aTabPageOpt));
            pPageInfo->m_xPage->SetFrame(m_xFrame);
            pPageInfo->m_xPage->Reset(&*pGroupInfo->m_pInItemSet);
        }

        TreeOptHelper::storeStringsOfDialog(pPageInfo->m_nPageId,
                                            pPageInfo->m_xPage->GetAllStrings());

        pPageInfo->m_xPage->set_visible(false);
    }
    else if (0 == pPageInfo->m_nPageId && !pPageInfo->m_xExtPage)
    {
        if (!m_xContainerWinProvider.is())
            m_xContainerWinProvider
                = awt::ContainerWindowProvider::create(::comphelper::getProcessComponentContext());

        pPageInfo->m_xExtPage = std::make_unique<ExtensionsTabPage>(
            xTabBox.get(), pPageInfo->m_sPageURL, pPageInfo->m_sEventHdl, m_xContainerWinProvider);

        pPageInfo->m_xExtPage->Hide();
    }

    BuilderPage* pNewPage = pPageInfo->m_xPage.get();

    // fdo#58170 use current page's layout child HelpId, unless there isn't a current page
    OUString sHelpId(pNewPage ? pNewPage->GetHelpId() : OUString());
    if (sHelpId.isEmpty())
        sHelpId = HID_OFADLG_TREELISTBOX;
    xTreeLB->set_help_id(sHelpId);
}

void OfaTreeOptionsDialog::SelectHdl_Impl()
{
    std::unique_ptr<weld::TreeIter> xEntry(xTreeLB->make_iterator());

    if (!xTreeLB->get_cursor(xEntry.get()))
        return;

    if (xCurrentPageEntry && xCurrentPageEntry->equal(*xEntry))
        return;

    std::unique_ptr<weld::TreeIter> xParent(xTreeLB->make_iterator(xEntry.get()));
    bool bParent = xTreeLB->iter_parent(*xParent);

    // If the user has selected a category, automatically switch to a suitable
    // default sub-page instead.
    if (!bParent)
        return;

    BuilderPage* pNewPage = nullptr;
    OptionsPageInfo* pOptPageInfo = (xCurrentPageEntry && xTreeLB->get_iter_depth(*xCurrentPageEntry))
        ? weld::fromId<OptionsPageInfo*>(xTreeLB->get_id(*xCurrentPageEntry)) : nullptr;

    if (pOptPageInfo && pOptPageInfo->m_xPage && pOptPageInfo->m_xPage->IsVisible())
    {
        std::unique_ptr<weld::TreeIter> xCurParent(xTreeLB->make_iterator(xCurrentPageEntry.get()));
        xTreeLB->iter_parent(*xCurParent);

        OptionsGroupInfo* pGroupInfo = weld::fromId<OptionsGroupInfo*>(xTreeLB->get_id(*xCurParent));
        DeactivateRC nLeave = DeactivateRC::LeavePage;
        if ( RID_SVXPAGE_COLOR != pOptPageInfo->m_nPageId && pOptPageInfo->m_xPage->HasExchangeSupport() )
           nLeave = pOptPageInfo->m_xPage->DeactivatePage( pGroupInfo->m_pOutItemSet.get() );

        if ( nLeave == DeactivateRC::KeepPage )
        {
            // we cannot leave this page, this is may be from a user clicking a different entry
            // in the tree so reselect the current page
            xTreeLB->select(*xCurrentPageEntry);
            return;
        }
        else
            pOptPageInfo->m_xPage->set_visible(false);
    }
    else if ( pOptPageInfo && pOptPageInfo->m_xExtPage )
    {
        pOptPageInfo->m_xExtPage->Hide();
        pOptPageInfo->m_xExtPage->DeactivatePage();
    }

    OptionsPageInfo *pPageInfo = weld::fromId<OptionsPageInfo*>(xTreeLB->get_id(*xEntry));
    OptionsGroupInfo* pGroupInfo = weld::fromId<OptionsGroupInfo*>(xTreeLB->get_id(*xParent));
    if(!pPageInfo->m_xPage && pPageInfo->m_nPageId > 0)
    {
        InitItemSets(*pGroupInfo);

        pPageInfo->m_xPage = ::CreateGeneralTabPage(pPageInfo->m_nPageId, xTabBox.get(), this, *pGroupInfo->m_pInItemSet);

        if(!pPageInfo->m_xPage && pGroupInfo->m_pModule)
            pPageInfo->m_xPage = pGroupInfo->m_pModule->CreateTabPage(pPageInfo->m_nPageId, xTabBox.get(), this, *pGroupInfo->m_pInItemSet);

        DBG_ASSERT( pPageInfo->m_xPage, "tabpage could not created");
        if ( pPageInfo->m_xPage )
        {
            SvtViewOptions aTabPageOpt( EViewType::TabPage, OUString::number( pPageInfo->m_nPageId) );
            pPageInfo->m_xPage->SetUserData( GetViewOptUserItem( aTabPageOpt ) );
            pPageInfo->m_xPage->SetFrame( m_xFrame );
            pPageInfo->m_xPage->Reset( &*pGroupInfo->m_pInItemSet );
        }
    }
    else if ( 0 == pPageInfo->m_nPageId && !pPageInfo->m_xExtPage )
    {
        if ( !m_xContainerWinProvider.is() )
        {
            m_xContainerWinProvider = awt::ContainerWindowProvider::create( ::comphelper::getProcessComponentContext() );
        }

        pPageInfo->m_xExtPage = std::make_unique<ExtensionsTabPage>(
            xTabBox.get(), pPageInfo->m_sPageURL, pPageInfo->m_sEventHdl, m_xContainerWinProvider);
    }

    if ( pPageInfo->m_xPage )
    {
        if ( RID_SVXPAGE_COLOR != pPageInfo->m_nPageId &&
             pPageInfo->m_xPage->HasExchangeSupport())
        {
            pPageInfo->m_xPage->ActivatePage(*pGroupInfo->m_pOutItemSet);
        }
        pPageInfo->m_xPage->set_visible(true);
    }
    else if ( pPageInfo->m_xExtPage )
    {
        pPageInfo->m_xExtPage->Show();
        pPageInfo->m_xExtPage->ActivatePage();
    }

    {
        OUString sTitleText = sTitle
                            + " - " + xTreeLB->get_text(*xParent)
                            + " - " + xTreeLB->get_text(*xEntry);
        m_xDialog->set_title(sTitleText);
    }

    xCurrentPageEntry = std::move(xEntry);

    if ( !bForgetSelection )
    {
        if ( !pLastPageSaver )
            pLastPageSaver = new LastPageSaver;
        if ( !bIsFromExtensionManager )
            pLastPageSaver->m_nLastPageId = pPageInfo->m_nPageId;
        if ( pPageInfo->m_xExtPage )
        {
            if ( bIsFromExtensionManager )
                pLastPageSaver->m_sLastPageURL_ExtMgr = pPageInfo->m_sPageURL;
            else
                pLastPageSaver->m_sLastPageURL_Tools = pPageInfo->m_sPageURL;
        }
    }
    pNewPage = pPageInfo->m_xPage.get();

    // fdo#58170 use current page's layout child HelpId, unless there isn't a current page
    OUString sHelpId(pNewPage ? pNewPage->GetHelpId() : OUString());
    if (sHelpId.isEmpty())
        sHelpId = HID_OFADLG_TREELISTBOX;
    xTreeLB->set_help_id(sHelpId);
}

std::optional<SfxItemSet> OfaTreeOptionsDialog::CreateItemSet( sal_uInt16 nId )
{
    Reference< XLinguProperties >  xProp( LinguMgr::GetLinguPropertySet() );
    std::optional<SfxItemSet> pRet;
    switch(nId)
    {
        case SID_GENERAL_OPTIONS:
        {
            pRet.emplace(
                SfxGetpApp()->GetPool(),
                svl::Items<
                    SID_HTML_MODE, SID_HTML_MODE,
                    SID_ATTR_METRIC, SID_ATTR_METRIC,
                    SID_AUTOSPELL_CHECK, SID_AUTOSPELL_CHECK,
                    SID_ATTR_QUICKLAUNCHER, SID_ATTR_QUICKLAUNCHER,
                    SID_ATTR_YEAR2000, SID_ATTR_YEAR2000> );

            SfxItemSetFixed<SID_ATTR_QUICKLAUNCHER, SID_ATTR_QUICKLAUNCHER> aOptSet( SfxGetpApp()->GetPool() );
            SfxApplication::GetOptions(aOptSet);
            pRet->Put(aOptSet);

            SfxViewFrame* pViewFrame = SfxViewFrame::Current();
            if ( pViewFrame )
            {
                SfxPoolItemHolder aResult;
                const SfxItemState aState(pViewFrame->GetDispatcher()->QueryState(SID_ATTR_YEAR2000, aResult));
                const SfxUInt16Item* pItem(static_cast<const SfxUInt16Item*>(aResult.getItem()));

                // miscellaneous - Year2000
                if(SfxItemState::DEFAULT <= aState && nullptr != pItem)
                    pRet->Put( SfxUInt16Item( SID_ATTR_YEAR2000, pItem->GetValue() ) );
                else
                    pRet->Put( SfxUInt16Item( SID_ATTR_YEAR2000, officecfg::Office::Common::DateFormat::TwoDigitYear::get() ) );
            }
            else
                pRet->Put( SfxUInt16Item( SID_ATTR_YEAR2000, officecfg::Office::Common::DateFormat::TwoDigitYear::get() ) );


            // miscellaneous - Tabulator
            pRet->Put(SfxBoolItem(SID_PRINTER_NOTFOUND_WARN, officecfg::Office::Common::Print::Warning::NotFound::get()));

            SfxPrinterChangeFlags nFlag = officecfg::Office::Common::Print::Warning::PaperSize::get() ? SfxPrinterChangeFlags::CHG_SIZE : SfxPrinterChangeFlags::NONE;
            nFlag  |= officecfg::Office::Common::Print::Warning::PaperOrientation::get() ? SfxPrinterChangeFlags::CHG_ORIENTATION : SfxPrinterChangeFlags::NONE;
            pRet->Put( SfxFlagItem( SID_PRINTER_CHANGESTODOC, static_cast<int>(nFlag) ));

        }
        break;
        case SID_LANGUAGE_OPTIONS :
        {
            pRet.emplace(
                SfxGetpApp()->GetPool(),
                svl::Items<
                    SID_ATTR_CHAR_CJK_LANGUAGE, SID_ATTR_CHAR_CJK_LANGUAGE,
                    SID_ATTR_CHAR_CTL_LANGUAGE, SID_ATTR_CHAR_CTL_LANGUAGE,
                    SID_SET_DOCUMENT_LANGUAGE, SID_SET_DOCUMENT_LANGUAGE,
                    SID_ATTR_LANGUAGE, SID_ATTR_LANGUAGE,
                    SID_AUTOSPELL_CHECK, SID_AUTOSPELL_CHECK,
                    SID_OPT_LOCALE_CHANGED, SID_OPT_LOCALE_CHANGED>);

            // for linguistic
            SfxHyphenRegionItem aHyphen( SID_ATTR_HYPHENREGION );

            sal_Int16   nMinLead  = 2,
                        nMinTrail = 2;
            if (xProp.is())
            {
                nMinLead = xProp->getHyphMinLeading();
                nMinTrail = xProp->getHyphMinTrailing();
            }
            aHyphen.GetMinLead()  = static_cast<sal_uInt8>(nMinLead);
            aHyphen.GetMinTrail() = static_cast<sal_uInt8>(nMinTrail);

            SfxViewFrame* pViewFrame = SfxViewFrame::Current();
            if ( pViewFrame )
            {
                SfxPoolItemHolder aResult;
                SfxDispatcher* pDispatch(pViewFrame->GetDispatcher());

                SfxItemState aState(pDispatch->QueryState(SID_ATTR_LANGUAGE, aResult));
                if(SfxItemState::DEFAULT <= aState)
                    pRet->Put(*aResult.getItem());

                aState = pDispatch->QueryState(SID_ATTR_CHAR_CJK_LANGUAGE, aResult);
                if(SfxItemState::DEFAULT <= aState)
                    pRet->Put(*aResult.getItem());

                aState = pDispatch->QueryState(SID_ATTR_CHAR_CTL_LANGUAGE, aResult);
                if(SfxItemState::DEFAULT <= aState)
                    pRet->Put(*aResult.getItem());

                pRet->Put(aHyphen);

                aState = pDispatch->QueryState(SID_AUTOSPELL_CHECK, aResult);
                if(SfxItemState::DEFAULT <= aState)
                {
                    pRet->Put(*aResult.getItem()); // allow share/refcounting
                    // pRet->Put(std::unique_ptr<SfxPoolItem>(aResult.getItem()->Clone()));
                }
                else
                {
                        bool bVal = false;
                        if (xProp.is())
                        {
                            bVal = xProp->getIsSpellAuto();
                        }

                        pRet->Put(SfxBoolItem(SID_AUTOSPELL_CHECK, bVal));
                }
            }
            pRet->Put( SfxBoolItem( SID_SET_DOCUMENT_LANGUAGE, bIsForSetDocumentLanguage ) );
        }
        break;
        case SID_INET_DLG :
                pRet.emplace( SfxGetpApp()->GetPool(),
                                svl::Items<
                //SID_OPTIONS_START - ..END
                                SID_SAVEREL_INET, SID_SAVEREL_FSYS,
                                SID_INET_NOPROXY, SID_INET_HTTP_PROXY_PORT,
                                SID_SECURE_URL, SID_SECURE_URL> );
                SfxApplication::GetOptions(*pRet);
        break;
        case SID_FILTER_DLG:
            pRet.emplace(
                SfxGetpApp()->GetPool(),
                svl::Items<
                    SID_ATTR_WARNALIENFORMAT, SID_ATTR_WARNALIENFORMAT,
                    SID_ATTR_DOCINFO, SID_ATTR_AUTOSAVEMINUTE,
                    SID_SAVEREL_INET, SID_SAVEREL_FSYS,
                    SID_ATTR_PRETTYPRINTING, SID_ATTR_PRETTYPRINTING> );
            SfxApplication::GetOptions(*pRet);
            break;

        case SID_SB_STARBASEOPTIONS:
            pRet.emplace( SfxGetpApp()->GetPool(),
                    svl::Items<SID_SB_POOLING_ENABLED, SID_SB_DB_REGISTER> );
            ::offapp::ConnectionPoolConfig::GetOptions(*pRet);
            svx::DbRegisteredNamesConfig::GetOptions(*pRet);
            break;

        case SID_SCH_EDITOPTIONS:
        {
            pRet.emplace( SfxGetpApp()->GetPool(), svl::Items<SID_SCH_EDITOPTIONS, SID_SCH_EDITOPTIONS> );
            pRet->Put( SvxChartColorTableItem( SID_SCH_EDITOPTIONS, SvxChartOptions::GetDefaultColors() ) );
            break;
        }
    }
    return pRet;
}

void OfaTreeOptionsDialog::ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet )
{
    switch(nId)
    {
        case SID_GENERAL_OPTIONS:
        {
            std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());

            SfxItemSetFixed<SID_ATTR_QUICKLAUNCHER, SID_ATTR_QUICKLAUNCHER> aOptSet(SfxGetpApp()->GetPool());
            aOptSet.Put(rSet);
            if(aOptSet.Count())
                SfxApplication::SetOptions( aOptSet );
            // get dispatcher anew, because SetOptions() might have destroyed the dispatcher
            SfxViewFrame *pViewFrame = SfxViewFrame::Current();

//          evaluate Year2000
            sal_uInt16 nY2K = USHRT_MAX;
            const SfxUInt16Item* pYearItem = rSet.GetItemIfSet( SID_ATTR_YEAR2000, false );
            if( pYearItem )
                nY2K = pYearItem->GetValue();
            if( USHRT_MAX != nY2K )
            {
                if ( pViewFrame )
                {
                    SfxDispatcher* pDispatch = pViewFrame->GetDispatcher();
                    pDispatch->ExecuteList(SID_ATTR_YEAR2000,
                            SfxCallMode::ASYNCHRON, { pYearItem });
                }
                officecfg::Office::Common::DateFormat::TwoDigitYear::set(nY2K, batch);
            }

//          evaluate print
            if(const SfxBoolItem* pWarnItem = rSet.GetItemIfSet(SID_PRINTER_NOTFOUND_WARN, false))
                officecfg::Office::Common::Print::Warning::NotFound::set(pWarnItem->GetValue(), batch);

            if(const SfxFlagItem* pFlag = rSet.GetItemIfSet(SID_PRINTER_CHANGESTODOC, false))
            {
                bool bPaperSizeWarning = bool(static_cast<SfxPrinterChangeFlags>(pFlag->GetValue()) &  SfxPrinterChangeFlags::CHG_SIZE);
                officecfg::Office::Common::Print::Warning::PaperSize::set(bPaperSizeWarning, batch);
                bool bPaperOrientationWarning = bool(static_cast<SfxPrinterChangeFlags>(pFlag->GetValue()) & SfxPrinterChangeFlags::CHG_ORIENTATION);
                officecfg::Office::Common::Print::Warning::PaperOrientation::set(bPaperOrientationWarning, batch);
            }

//          evaluate help options
            bool bHelpTips = officecfg::Office::Common::Help::Tip::get();
            if ( bHelpTips != Help::IsQuickHelpEnabled() )
                bHelpTips ? Help::EnableQuickHelp() : Help::DisableQuickHelp();
            bool bExtendedHelp = officecfg::Office::Common::Help::ExtendedTip::get();
            if ( bExtendedHelp != Help::IsBalloonHelpEnabled() )
                bExtendedHelp ? Help::EnableBalloonHelp() : Help::DisableBalloonHelp();

            batch->commit();
        }
        break;
        case SID_LANGUAGE_OPTIONS :
        {
            OfaTreeOptionsDialog::ApplyLanguageOptions(rSet);
        }
        break;
        case SID_INET_DLG :
        case SID_FILTER_DLG:
            SfxApplication::SetOptions( rSet );
        break;

        case SID_SB_STARBASEOPTIONS:
            ::offapp::ConnectionPoolConfig::SetOptions( rSet );
            svx::DbRegisteredNamesConfig::SetOptions(rSet);
            break;

        case SID_SCH_EDITOPTIONS:
            // nothing to do. Chart options only apply to newly created charts
            break;

        default:
        {
            SAL_WARN("cui.options", "Unhandled option in ApplyItemSet");
        }
        break;
    }

}
void OfaTreeOptionsDialog::ApplyLanguageOptions(const SfxItemSet& rSet)
{
    bool bSaveSpellCheck = false;
    const SfxPoolItem* pItem = nullptr;

    Reference< XComponentContext >  xContext( ::comphelper::getProcessComponentContext() );
    Reference< XLinguProperties >  xProp = LinguProperties::create( xContext );
    if ( const SfxHyphenRegionItem* pHyphenItem = rSet.GetItemIfSet(SID_ATTR_HYPHENREGION, false ) )
    {
        xProp->setHyphMinLeading( static_cast<sal_Int16>(pHyphenItem->GetMinLead()) );
        xProp->setHyphMinTrailing( static_cast<sal_Int16>(pHyphenItem->GetMinTrail()) );
        bSaveSpellCheck = true;
    }

    SfxViewFrame *pViewFrame = SfxViewFrame::Current();
    if ( pViewFrame )
    {
        SfxDispatcher* pDispatch = pViewFrame->GetDispatcher();
        pItem = nullptr;
        if(SfxItemState::SET == rSet.GetItemState( SID_ATTR_LANGUAGE, false, &pItem ))
        {
            pDispatch->ExecuteList(pItem->Which(), SfxCallMode::SYNCHRON, { pItem });
            bSaveSpellCheck = true;
        }
        if(SfxItemState::SET == rSet.GetItemState( SID_ATTR_CHAR_CTL_LANGUAGE, false, &pItem ))
        {
            pDispatch->ExecuteList(pItem->Which(), SfxCallMode::SYNCHRON, { pItem });
            bSaveSpellCheck = true;
        }
        if(SfxItemState::SET == rSet.GetItemState( SID_ATTR_CHAR_CJK_LANGUAGE, false, &pItem ))
        {
            pDispatch->ExecuteList(pItem->Which(), SfxCallMode::SYNCHRON, { pItem });
            bSaveSpellCheck = true;
        }

        if( SfxItemState::SET == rSet.GetItemState(SID_AUTOSPELL_CHECK, false, &pItem ))
        {
            bool bOnlineSpelling = static_cast<const SfxBoolItem*>(pItem)->GetValue();
            pDispatch->ExecuteList(SID_AUTOSPELL_CHECK,
                SfxCallMode::ASYNCHRON|SfxCallMode::RECORD, { pItem });

            xProp->setIsSpellAuto( bOnlineSpelling );
        }

        if( bSaveSpellCheck )
        {
            //! the config item has changed since we modified the
            //! property set it uses
            pDispatch->Execute(SID_SPELLCHECKER_CHANGED, SfxCallMode::ASYNCHRON);
        }
    }

    if( SfxItemState::SET == rSet.GetItemState(SID_OPT_LOCALE_CHANGED, false, &pItem ))
    {
        SfxViewFrame* _pViewFrame = SfxViewFrame::GetFirst();
        while ( _pViewFrame )
        {
            _pViewFrame->GetDispatcher()->ExecuteList(pItem->Which(),
                    SfxCallMode::ASYNCHRON, { pItem });
            _pViewFrame = SfxViewFrame::GetNext( *_pViewFrame );
        }
    }
}

OUString OfaTreeOptionsDialog::getCurrentFactory_Impl( const Reference< XFrame >& _xFrame )
{
    OUString sIdentifier;
    Reference < XFrame > xCurrentFrame( _xFrame );
    Reference < XModuleManager2 > xModuleManager = ModuleManager::create(::comphelper::getProcessComponentContext());
    if ( !xCurrentFrame.is() )
    {
        Reference< XDesktop2 > xDesktop = Desktop::create( ::comphelper::getProcessComponentContext() );
        xCurrentFrame = xDesktop->getCurrentFrame();
    }

    if ( xCurrentFrame.is() )
    {
        try
        {
            sIdentifier = xModuleManager->identify( xCurrentFrame );
        }
        catch ( css::frame::UnknownModuleException& )
        {
            SAL_INFO( "cui.options", "unknown module" );
        }
        catch ( Exception const & )
        {
            TOOLS_WARN_EXCEPTION( "cui.options", "getActiveModule_Impl(): exception of XModuleManager::identify()" );
        }
    }

    return sIdentifier;
}

void OfaTreeOptionsDialog::generalOptions(const std::vector<sal_uInt16>& vPageId)
{
    sal_uInt16 nGroup = 0;
    SvtOptionsDialogOptions aOptionsDlgOpt;
    sal_uInt16 nPageId = 0;

    if ( !lcl_isOptionHidden( SID_GENERAL_OPTIONS, aOptionsDlgOpt ) )
    {
        setGroupName(u"ProductName", CuiResId(SID_GENERAL_OPTIONS_RES[0].first));
        nGroup = AddGroup(CuiResId(SID_GENERAL_OPTIONS_RES[0].first), nullptr, nullptr, SID_GENERAL_OPTIONS );
        const sal_uInt16 nEnd = static_cast<sal_uInt16>(std::size(SID_GENERAL_OPTIONS_RES));

        for (sal_uInt16 i = 1; i < nEnd; ++i)
        {
            OUString sNewTitle = CuiResId(SID_GENERAL_OPTIONS_RES[i].first);
            nPageId = SID_GENERAL_OPTIONS_RES[i].second;
            if ( lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                continue;

            // Disable Online Update page if neither mode is available
            if( RID_SVXPAGE_ONLINEUPDATE == nPageId
                && !(SvxOnlineUpdateTabPage::isTraditionalOnlineUpdateAvailable()
                     || SvxOnlineUpdateTabPage::isMarOnlineUpdateAvailable()) )
            {
                continue;
            }

            if (!vPageId.empty())
            {
                if (std::find(vPageId.begin(), vPageId.end(), nPageId) != vPageId.end())
                    AddTabPage(nPageId, sNewTitle, nGroup);
            }
            else
                AddTabPage(nPageId, sNewTitle, nGroup);
        }
    }
}

void OfaTreeOptionsDialog::loadAndSaveOptions(const std::vector<sal_uInt16>& vPageId)
{
    sal_uInt16 nGroup = 0;
    SvtOptionsDialogOptions aOptionsDlgOpt;
    sal_uInt16 nPageId = 0;

    if ( !lcl_isOptionHidden( SID_FILTER_DLG, aOptionsDlgOpt ) )
    {
        setGroupName( u"LoadSave", CuiResId(SID_FILTER_DLG_RES[0].first) );
        nGroup = AddGroup( CuiResId(SID_FILTER_DLG_RES[0].first), nullptr, nullptr, SID_FILTER_DLG );
        for ( size_t i = 1; i < std::size(SID_FILTER_DLG_RES); ++i )
        {
            nPageId = static_cast<sal_uInt16>(SID_FILTER_DLG_RES[i].second);
            if ( !lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
            {
                if (!vPageId.empty())
                {
                    if (std::find(vPageId.begin(), vPageId.end(), nPageId) != vPageId.end())
                        AddTabPage(nPageId, CuiResId(SID_FILTER_DLG_RES[i].first), nGroup);
                }
                else
                    AddTabPage(nPageId, CuiResId(SID_FILTER_DLG_RES[i].first), nGroup);
            }
        }
    }
}

void OfaTreeOptionsDialog::languageOptions(const std::vector<sal_uInt16>& vPageId)
{
    sal_uInt16 nGroup = 0;
    SvtOptionsDialogOptions aOptionsDlgOpt;
    sal_uInt16 nPageId = 0;

    SvtCTLOptions aCTLLanguageOptions;
    if ( !lcl_isOptionHidden( SID_LANGUAGE_OPTIONS, aOptionsDlgOpt ) )
    {
        setGroupName(u"LanguageSettings", CuiResId(SID_LANGUAGE_OPTIONS_RES[0].first));
        nGroup = AddGroup(CuiResId(SID_LANGUAGE_OPTIONS_RES[0].first), nullptr, nullptr, SID_LANGUAGE_OPTIONS );
        for (size_t i = 1; i < std::size(SID_LANGUAGE_OPTIONS_RES); ++i)
        {
            nPageId = static_cast<sal_uInt16>(SID_LANGUAGE_OPTIONS_RES[i].second);
            if ( lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                continue;

            // Disable DeepL translation settings, if experimental mode is not enabled
            if( RID_SVXPAGE_DEEPL_OPTIONS == nPageId )
            {
                if( ! officecfg::Office::Common::Misc::ExperimentalMode::get() )
                    continue;
            }

            if ( ( RID_SVXPAGE_JSEARCH_OPTIONS != nPageId || SvtCJKOptions::IsJapaneseFindEnabled() ) &&
                 ( RID_SVXPAGE_ASIAN_LAYOUT != nPageId    || SvtCJKOptions::IsAsianTypographyEnabled() ) &&
                 ( RID_SVXPAGE_OPTIONS_CTL != nPageId     || SvtCTLOptions::IsCTLFontEnabled() ) )
            {
                if (!vPageId.empty())
                {
                    if (std::find(vPageId.begin(), vPageId.end(), nPageId) != vPageId.end())
                        AddTabPage(nPageId, CuiResId(SID_LANGUAGE_OPTIONS_RES[i].first), nGroup);
                }
                else
                    AddTabPage(nPageId, CuiResId(SID_LANGUAGE_OPTIONS_RES[i].first), nGroup);
            }
        }
    }
}

void OfaTreeOptionsDialog::writerOptions(const std::vector<sal_uInt16>& vPageId)
{
    sal_uInt16 nGroup = 0;
    SvtOptionsDialogOptions aOptionsDlgOpt;
    sal_uInt16 nPageId = 0;

    OUString aFactory = getCurrentFactory_Impl( m_xFrame );
    DBG_ASSERT( GetModuleIdentifier( m_xFrame ) == aFactory, "This should not happen" );

    SvtModuleOptions aModuleOpt;
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::WRITER ) )
    {
        // text document
        if (   aFactory == "com.sun.star.text.TextDocument"
            || aFactory == "com.sun.star.text.WebDocument"
            || aFactory == "com.sun.star.text.GlobalDocument" )
        {
            SfxModule* pSwMod = SfxApplication::GetModule(SfxToolsModule::Writer);
            if ( !lcl_isOptionHidden( SID_SW_EDITOPTIONS, aOptionsDlgOpt ) )
            {
                if ( aFactory == "com.sun.star.text.WebDocument" )
                    setGroupName( u"WriterWeb", CuiResId(SID_SW_EDITOPTIONS_RES[0].first) );
                else
                    setGroupName( u"Writer", CuiResId(SID_SW_EDITOPTIONS_RES[0].first) );
                nGroup = AddGroup(CuiResId(SID_SW_EDITOPTIONS_RES[0].first), pSwMod, pSwMod, SID_SW_EDITOPTIONS );
                SvtCTLOptions aCTLLanguageOptions;
                for ( size_t i = 1; i < std::size(SID_SW_EDITOPTIONS_RES); ++i )
                {
                    nPageId = static_cast<sal_uInt16>(SID_SW_EDITOPTIONS_RES[i].second);
                    if ( lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                        continue;
                    if ( ( RID_SW_TP_STD_FONT_CJK != nPageId || SvtCJKOptions::IsCJKFontEnabled() ) &&
                         ( RID_SW_TP_STD_FONT_CTL != nPageId || SvtCTLOptions::IsCTLFontEnabled() ) &&
                         ( RID_SW_TP_MAILCONFIG != nPageId || MailMergeCfgIsEmailSupported() ) )
                    {
                        if (!vPageId.empty())
                        {
                            if (std::find(vPageId.begin(), vPageId.end(), nPageId) != vPageId.end())
                                AddTabPage(nPageId, CuiResId(SID_SW_EDITOPTIONS_RES[i].first),
                                           nGroup);
                        }
                        else
                            AddTabPage(nPageId, CuiResId(SID_SW_EDITOPTIONS_RES[i].first), nGroup);
                    }
                }
#ifdef DBG_UTIL
                if (!vPageId.empty())
                {
                    if (std::find(vPageId.begin(), vPageId.end(), RID_SW_TP_OPTTEST_PAGE)
                        != vPageId.end())
                        AddTabPage(RID_SW_TP_OPTTEST_PAGE, "Internal Test", nGroup);
                }
                else
                    AddTabPage(RID_SW_TP_OPTTEST_PAGE, "Internal Test", nGroup);
#endif
            }
        }
    }
}

void OfaTreeOptionsDialog::writerWebOptions(const std::vector<sal_uInt16>& vPageId)
{
    sal_uInt16 nGroup = 0;
    SvtOptionsDialogOptions aOptionsDlgOpt;
    sal_uInt16 nPageId = 0;

    OUString aFactory = getCurrentFactory_Impl( m_xFrame );
    DBG_ASSERT( GetModuleIdentifier( m_xFrame ) == aFactory, "This should not happen" );

    SvtModuleOptions aModuleOpt;
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::WRITER ) )
    {
        // text document
        if (   aFactory == "com.sun.star.text.TextDocument"
            || aFactory == "com.sun.star.text.WebDocument"
            || aFactory == "com.sun.star.text.GlobalDocument" )
        {
            SfxModule* pSwMod = SfxApplication::GetModule(SfxToolsModule::Writer);

            // HTML documents
            if ( !lcl_isOptionHidden( SID_SW_ONLINEOPTIONS, aOptionsDlgOpt ) )
            {
                nGroup = AddGroup(CuiResId(SID_SW_ONLINEOPTIONS_RES[0].first), pSwMod, pSwMod, SID_SW_ONLINEOPTIONS );
                for( size_t i = 1; i < std::size(SID_SW_ONLINEOPTIONS_RES); ++i )
                {
                    nPageId = static_cast<sal_uInt16>(SID_SW_ONLINEOPTIONS_RES[i].second);
                    if ( !lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                    {
                        if (!vPageId.empty())
                        {
                            if (std::find(vPageId.begin(), vPageId.end(), nPageId) != vPageId.end())
                                AddTabPage(nPageId, CuiResId(SID_SW_ONLINEOPTIONS_RES[i].first),
                                           nGroup);
                        }
                        else
                            AddTabPage(nPageId, CuiResId(SID_SW_ONLINEOPTIONS_RES[i].first),
                                       nGroup);
                    }
                }
#ifdef DBG_UTIL
                if (!vPageId.empty())
                {
                    if (std::find(vPageId.begin(), vPageId.end(), RID_SW_TP_OPTTEST_PAGE)
                        != vPageId.end())
                        AddTabPage(RID_SW_TP_OPTTEST_PAGE, "Internal Test", nGroup);
                }
                else
                    AddTabPage(RID_SW_TP_OPTTEST_PAGE, "Internal Test", nGroup);
#endif
            }
        }
    }
}

void OfaTreeOptionsDialog::calcOptions(const std::vector<sal_uInt16>& vPageId)
{
    sal_uInt16 nGroup = 0;
    SvtOptionsDialogOptions aOptionsDlgOpt;
    sal_uInt16 nPageId = 0;

    OUString aFactory = getCurrentFactory_Impl( m_xFrame );
    DBG_ASSERT( GetModuleIdentifier( m_xFrame ) == aFactory, "This should not happen" );

    SvtModuleOptions aModuleOpt;
    // Calc options
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::CALC ) )
    {
        if ( aFactory == "com.sun.star.sheet.SpreadsheetDocument" )
        {
            if ( !lcl_isOptionHidden( SID_SC_EDITOPTIONS, aOptionsDlgOpt ) )
            {
                SfxModule* pScMod = SfxApplication::GetModule( SfxToolsModule::Calc );
                setGroupName( u"Calc", CuiResId(SID_SC_EDITOPTIONS_RES[0].first) );
                nGroup = AddGroup( CuiResId(SID_SC_EDITOPTIONS_RES[0].first), pScMod, pScMod, SID_SC_EDITOPTIONS );
                const sal_uInt16 nCount = static_cast<sal_uInt16>(std::size(SID_SC_EDITOPTIONS_RES));
                for ( sal_uInt16 i = 1; i < nCount; ++i )
                {
                    nPageId = static_cast<sal_uInt16>(SID_SC_EDITOPTIONS_RES[i].second);
                    if ( lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                        continue;

                    if (!vPageId.empty())
                    {
                        if (std::find(vPageId.begin(), vPageId.end(), nPageId) != vPageId.end())
                            AddTabPage(nPageId, CuiResId(SID_SC_EDITOPTIONS_RES[i].first), nGroup);
                    }
                    else
                        AddTabPage(nPageId, CuiResId(SID_SC_EDITOPTIONS_RES[i].first), nGroup);
                }
            }
        }
    }
}

void OfaTreeOptionsDialog::impressOptions(const std::vector<sal_uInt16>& vPageId)
{
    sal_uInt16 nGroup = 0;
    SvtOptionsDialogOptions aOptionsDlgOpt;
    sal_uInt16 nPageId = 0;

    OUString aFactory = getCurrentFactory_Impl( m_xFrame );
    DBG_ASSERT( GetModuleIdentifier( m_xFrame ) == aFactory, "This should not happen" );
    SvtModuleOptions aModuleOpt;

    // Impress options
    SfxModule* pSdMod = SfxApplication::GetModule( SfxToolsModule::Draw );
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::IMPRESS ) )
    {
        if ( aFactory == "com.sun.star.presentation.PresentationDocument" )
        {
            if ( !lcl_isOptionHidden( SID_SD_EDITOPTIONS, aOptionsDlgOpt ) )
            {
                setGroupName( u"Impress", CuiResId(SID_SD_EDITOPTIONS_RES[0].first) );
                nGroup = AddGroup( CuiResId(SID_SD_EDITOPTIONS_RES[0].first), pSdMod, pSdMod, SID_SD_EDITOPTIONS );
                const sal_uInt16 nCount = static_cast<sal_uInt16>(std::size(SID_SD_EDITOPTIONS_RES));
                for ( sal_uInt16 i = 1; i < nCount; ++i )
                {
                    nPageId = static_cast<sal_uInt16>(SID_SD_EDITOPTIONS_RES[i].second);
                    if ( lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                        continue;

                    if (!vPageId.empty())
                    {
                        if (std::find(vPageId.begin(), vPageId.end(), nPageId) != vPageId.end())
                            AddTabPage(nPageId, CuiResId(SID_SD_EDITOPTIONS_RES[i].first), nGroup);
                    }
                    else
                        AddTabPage(nPageId, CuiResId(SID_SD_EDITOPTIONS_RES[i].first), nGroup);
                }
            }
        }
    }
}

void OfaTreeOptionsDialog::drawOptions(const std::vector<sal_uInt16>& vPageId)
{
    sal_uInt16 nGroup = 0;
    SvtOptionsDialogOptions aOptionsDlgOpt;
    sal_uInt16 nPageId = 0;

    OUString aFactory = getCurrentFactory_Impl( m_xFrame );
    DBG_ASSERT( GetModuleIdentifier( m_xFrame ) == aFactory, "This should not happen" );
    SvtModuleOptions aModuleOpt;

    // Draw options
    SfxModule* pSdMod = SfxApplication::GetModule(SfxToolsModule::Draw);
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::DRAW ) )
    {
        if ( aFactory == "com.sun.star.drawing.DrawingDocument" )
        {
            if ( !lcl_isOptionHidden( SID_SD_GRAPHIC_OPTIONS, aOptionsDlgOpt ) )
            {
                setGroupName( u"Draw", CuiResId(SID_SD_GRAPHIC_OPTIONS_RES[0].first) );
                nGroup = AddGroup( CuiResId(SID_SD_GRAPHIC_OPTIONS_RES[0].first), pSdMod, pSdMod, SID_SD_GRAPHIC_OPTIONS );
                const sal_uInt16 nCount = static_cast<sal_uInt16>(std::size(SID_SD_GRAPHIC_OPTIONS_RES));
                for ( sal_uInt16 i = 1; i < nCount; ++i )
                {
                    nPageId = static_cast<sal_uInt16>(SID_SD_GRAPHIC_OPTIONS_RES[i].second);
                    if ( lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                        continue;

                    if (!vPageId.empty())
                    {
                        if (std::find(vPageId.begin(), vPageId.end(), nPageId) != vPageId.end())
                            AddTabPage(nPageId, CuiResId(SID_SD_GRAPHIC_OPTIONS_RES[i].first),
                                       nGroup);
                    }
                    else
                        AddTabPage(nPageId, CuiResId(SID_SD_GRAPHIC_OPTIONS_RES[i].first), nGroup);
                }
            }
        }
    }
}

void OfaTreeOptionsDialog::mathOptions(const std::vector<sal_uInt16>& vPageId)
{
    sal_uInt16 nGroup = 0;
    SvtOptionsDialogOptions aOptionsDlgOpt;
    sal_uInt16 nPageId = 0;

    OUString aFactory = getCurrentFactory_Impl( m_xFrame );
    DBG_ASSERT( GetModuleIdentifier( m_xFrame ) == aFactory, "This should not happen" );
    SvtModuleOptions aModuleOpt;

    // Math options
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::MATH ) )
    {
        if ( aFactory == "com.sun.star.formula.FormulaProperties" )
        {
            if ( !lcl_isOptionHidden( SID_SM_EDITOPTIONS, aOptionsDlgOpt ) )
            {
                SfxModule* pSmMod = SfxApplication::GetModule(SfxToolsModule::Math);
                setGroupName( u"Math", CuiResId(SID_SM_EDITOPTIONS_RES[0].first) );
                nGroup = AddGroup(CuiResId(SID_SM_EDITOPTIONS_RES[0].first), pSmMod, pSmMod, SID_SM_EDITOPTIONS );
                for ( size_t i = 1; i < std::size(SID_SM_EDITOPTIONS_RES); ++i )
                {
                    nPageId = static_cast<sal_uInt16>(SID_SM_EDITOPTIONS_RES[i].second);
                    if ( !lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                    {
                        if (!vPageId.empty())
                        {
                            if (std::find(vPageId.begin(), vPageId.end(), nPageId) != vPageId.end())
                                AddTabPage(nPageId, CuiResId(SID_SM_EDITOPTIONS_RES[i].first),
                                           nGroup);
                        }
                        else
                            AddTabPage(nPageId, CuiResId(SID_SM_EDITOPTIONS_RES[i].first), nGroup);
                    }
                }
            }
        }
    }
}

void OfaTreeOptionsDialog::databaseOptions(const std::vector<sal_uInt16>& vPageId)
{
    sal_uInt16 nGroup = 0;
    SvtOptionsDialogOptions aOptionsDlgOpt;
    sal_uInt16 nPageId = 0;

    SvtModuleOptions aModuleOpt;

    // Database - needed only if there is an application which integrates with databases
    if ( !lcl_isOptionHidden( SID_SB_STARBASEOPTIONS, aOptionsDlgOpt ) &&
        (   aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::DATABASE )
        ||  aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::WRITER )
        ||  aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::CALC )
        ) )
    {
        setGroupName( u"Base", CuiResId(SID_SB_STARBASEOPTIONS_RES[0].first) );
        nGroup = AddGroup( CuiResId(SID_SB_STARBASEOPTIONS_RES[0].first), nullptr, nullptr, SID_SB_STARBASEOPTIONS );
        for ( size_t i = 1; i < std::size(SID_SB_STARBASEOPTIONS_RES); ++i )
        {
            nPageId = static_cast<sal_uInt16>(SID_SB_STARBASEOPTIONS_RES[i].second);
            if ( !lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
            {
                if (!vPageId.empty())
                {
                    if (std::find(vPageId.begin(), vPageId.end(), nPageId) != vPageId.end())
                        AddTabPage(nPageId, CuiResId(SID_SB_STARBASEOPTIONS_RES[i].first), nGroup);
                }
                else
                    AddTabPage(nPageId, CuiResId(SID_SB_STARBASEOPTIONS_RES[i].first), nGroup);
            }
        }
    }
}

void OfaTreeOptionsDialog::chartOptions(const std::vector<sal_uInt16>& vPageId)
{
    sal_uInt16 nGroup = 0;
    SvtOptionsDialogOptions aOptionsDlgOpt;
    sal_uInt16 nPageId = 0;

    if ( !lcl_isOptionHidden( SID_SCH_EDITOPTIONS, aOptionsDlgOpt ) )
    {
        setGroupName( u"Charts", CuiResId(SID_SCH_EDITOPTIONS_RES[0].first) );
        nGroup = AddGroup( CuiResId(SID_SCH_EDITOPTIONS_RES[0].first), nullptr, nullptr, SID_SCH_EDITOPTIONS );
        for ( size_t i = 1; i < std::size(SID_SCH_EDITOPTIONS_RES); ++i )
        {
            nPageId = static_cast<sal_uInt16>(SID_SCH_EDITOPTIONS_RES[i].second);
            if ( !lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
            {
                if (!vPageId.empty())
                {
                    if (std::find(vPageId.begin(), vPageId.end(), nPageId) != vPageId.end())
                        AddTabPage(nPageId, CuiResId(SID_SCH_EDITOPTIONS_RES[i].first), nGroup);
                }
                else
                    AddTabPage(nPageId, CuiResId(SID_SCH_EDITOPTIONS_RES[i].first), nGroup);
            }
        }
    }
}

void OfaTreeOptionsDialog::internetOptions(const std::vector<sal_uInt16>& vPageId)
{
    sal_uInt16 nGroup = 0;
    SvtOptionsDialogOptions aOptionsDlgOpt;
    sal_uInt16 nPageId = 0;

    if ( lcl_isOptionHidden( SID_INET_DLG, aOptionsDlgOpt ) )
        return;

    setGroupName(u"Internet", CuiResId(SID_INET_DLG_RES[0].first));
    nGroup = AddGroup(CuiResId(SID_INET_DLG_RES[0].first), nullptr, nullptr, SID_INET_DLG );

    for ( size_t i = 1; i < std::size(SID_INET_DLG_RES); ++i )
    {
        nPageId = static_cast<sal_uInt16>(SID_INET_DLG_RES[i].second);
        if ( lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
            continue;
#if defined(_WIN32)
        // Disable E-mail tab-page on Windows
        if ( nPageId == RID_SVXPAGE_INET_MAIL )
            continue;
#endif
        if (!vPageId.empty())
        {
            if (std::find(vPageId.begin(), vPageId.end(), nPageId) != vPageId.end())
                AddTabPage(nPageId, CuiResId(SID_INET_DLG_RES[i].first), nGroup);
        }
        else
            AddTabPage(nPageId, CuiResId(SID_INET_DLG_RES[i].first), nGroup);
    }
}

void OfaTreeOptionsDialog::Initialize(const Reference<XFrame>& _xFrame)
{
    m_xFrame = _xFrame;

    // %PRODUCTNAME options
    generalOptions();

    // Load and Save options
    loadAndSaveOptions();

    // Language options
    languageOptions();

    // Writer and Writer/Web options
    writerOptions();
    writerWebOptions();

    // Calc options
    calcOptions();

    // Impress options
    impressOptions();

    // Draw options
    drawOptions();

    // Math options
    mathOptions();

    // Database - needed only if there is an application which integrates with databases
    databaseOptions();

    // Chart options (always installed and active)
    chartOptions();

    // Internet options
    internetOptions();

    // store Options Dialogs with their page names and strings
    // in order to use them when searching
    storeOptionsTree();
}

static bool isNodeActive( OptionsNode const * pNode, Module* pModule )
{
    if ( !pNode )
        return false;

    // Node for all modules active?
    if ( pNode->m_bAllModules )
        return true;

    // OOo-Nodes (Writer, Calc, Impress...) are active if node is already inserted
    if ( !getGroupName( pNode->m_sId, false ).isEmpty() )
        return true;

    // no module -> not active
    if ( !pModule )
        return false;

    // search node in active module
    if ( pModule->m_bActive )
    {
        for (auto const& j : pModule->m_aNodeList)
            if ( j->m_sId == pNode->m_sId )
                return true;
    }
    return false;
}

void OfaTreeOptionsDialog::LoadExtensionOptions( std::u16string_view rExtensionId )
{
    std::unique_ptr<Module> pModule;

    // when called by Tools - Options then load nodes of active module
    if ( rExtensionId.empty() )
    {
        pModule = LoadModule( GetModuleIdentifier( Reference< XFrame >() ) );
    }

    VectorOfNodes aNodeList = LoadNodes( pModule.get(), rExtensionId );
    InsertNodes( aNodeList );
}

OUString OfaTreeOptionsDialog::GetModuleIdentifier( const Reference< XFrame >& rFrame )
{
    OUString sModule;
    Reference < XFrame > xCurrentFrame( rFrame );
    Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();
    Reference < XModuleManager2 > xModuleManager = ModuleManager::create(xContext);

    if ( !xCurrentFrame.is() )
    {
        Reference < XDesktop2 > xDesktop = Desktop::create( xContext );
        xCurrentFrame = xDesktop->getCurrentFrame();
    }

    if ( xCurrentFrame.is() )
    {
        try
        {
            sModule = xModuleManager->identify( xCurrentFrame );
        }
        catch ( css::frame::UnknownModuleException& )
        {
            SAL_INFO( "cui.options", "unknown module" );
        }
        catch ( Exception const & )
        {
            TOOLS_WARN_EXCEPTION( "cui.options", "OfaTreeOptionsDialog::GetModuleIdentifier(): exception of XModuleManager::identify()");
        }
    }
    return sModule;
}

std::unique_ptr<Module> OfaTreeOptionsDialog::LoadModule(
    std::u16string_view rModuleIdentifier )
{
    std::unique_ptr<Module> pModule;
    Reference< XNameAccess > xSet(
        officecfg::Office::OptionsDialog::Modules::get());

    const Sequence< OUString > seqNames = xSet->getElementNames();
    for ( const OUString& rModule : seqNames )
    {
        if ( rModuleIdentifier == rModule )
        {
            // current active module found
            pModule.reset(new Module);
            pModule->m_bActive = true;

            Reference< XNameAccess > xModAccess;
            xSet->getByName( rModule ) >>= xModAccess;
            if ( xModAccess.is() )
            {
                // load the nodes of this module
                Reference< XNameAccess > xNodeAccess;
                xModAccess->getByName( "Nodes" ) >>= xNodeAccess;
                if ( xNodeAccess.is() )
                {
                    const Sequence< OUString > xTemp = xNodeAccess->getElementNames();
                    Reference< XNameAccess > xAccess;
                    sal_Int32 nIndex = -1;
                    for ( const OUString& rNode : xTemp)
                    {
                        xNodeAccess->getByName( rNode ) >>= xAccess;
                        if ( xAccess.is() )
                        {
                            xAccess->getByName( "Index" ) >>= nIndex;
                            if ( nIndex < 0 )
                                // append nodes with index < 0
                                pModule->m_aNodeList.push_back(
                                 std::unique_ptr<OrderedEntry>(new OrderedEntry(nIndex, rNode)));
                            else
                            {
                                // search position of the node
                                std::vector<OrderedEntry *>::size_type y = 0;
                                for ( ; y < pModule->m_aNodeList.size(); ++y )
                                {
                                    sal_Int32 nNodeIdx = pModule->m_aNodeList[y]->m_nIndex;
                                    if ( nNodeIdx < 0 || nNodeIdx > nIndex )
                                        break;
                                }
                                // and insert the node on this position
                                pModule->m_aNodeList.insert(
                                    pModule->m_aNodeList.begin() + y,
                                    std::unique_ptr<OrderedEntry>(new OrderedEntry( nIndex, rNode )) );
                            }
                        }
                    }
                }
            }
            break;
        }
    }
    return pModule;
}

VectorOfNodes OfaTreeOptionsDialog::LoadNodes(
    Module* pModule, std::u16string_view rExtensionId)
{
    VectorOfNodes aOutNodeList;

    Reference< XNameAccess > xSet(
        officecfg::Office::OptionsDialog::Nodes::get());
    VectorOfNodes aNodeList;
    const Sequence< OUString > seqNames = xSet->getElementNames();

    for ( OUString const & sGroupName : seqNames )
    {
        Reference< XNameAccess > xNodeAccess;
        xSet->getByName( sGroupName ) >>= xNodeAccess;

        if ( xNodeAccess.is() )
        {
            OUString sNodeId, sLabel, sPageURL;
            bool bAllModules = false;

            sNodeId = sGroupName;
            xNodeAccess->getByName( "Label" ) >>= sLabel;
            xNodeAccess->getByName( "OptionsPage" ) >>= sPageURL;
            xNodeAccess->getByName( "AllModules" ) >>= bAllModules;

            if ( sLabel.isEmpty() )
                sLabel = sGroupName;
            OUString sTemp = getGroupName( sLabel, !rExtensionId.empty() );
            if ( !sTemp.isEmpty() )
                sLabel = sTemp;
            std::unique_ptr<OptionsNode> pNode(new OptionsNode(sNodeId, sLabel, bAllModules));

            if ( rExtensionId.empty() && !isNodeActive( pNode.get(), pModule ) )
            {
                continue;
            }

            Reference< XNameAccess > xLeavesSet;
            xNodeAccess->getByName( "Leaves" ) >>= xLeavesSet;
            if ( xLeavesSet.is() )
            {
                const Sequence< OUString > seqLeaves = xLeavesSet->getElementNames();
                for ( OUString const & leafName : seqLeaves )
                {
                    Reference< XNameAccess > xLeaveAccess;
                    xLeavesSet->getByName( leafName ) >>= xLeaveAccess;

                    if ( xLeaveAccess.is() )
                    {
                        OUString sId, sLeafLabel, sEventHdl, sLeafURL, sLeafGrpId;
                        sal_Int32 nLeafGrpIdx = 0;

                        xLeaveAccess->getByName( "Id" ) >>= sId;
                        xLeaveAccess->getByName( "Label" ) >>= sLeafLabel;
                        xLeaveAccess->getByName( "OptionsPage" ) >>= sLeafURL;
                        xLeaveAccess->getByName( "EventHandlerService" ) >>= sEventHdl;
                        xLeaveAccess->getByName( "GroupId" ) >>= sLeafGrpId;
                        xLeaveAccess->getByName( "GroupIndex" ) >>= nLeafGrpIdx;

                        if ( rExtensionId.empty() || sId == rExtensionId )
                        {
                            std::unique_ptr<OptionsLeaf> pLeaf(new OptionsLeaf(
                                sLeafLabel, sLeafURL, sEventHdl, sLeafGrpId, nLeafGrpIdx ));

                            if ( !sLeafGrpId.isEmpty() )
                            {
                                bool bAlreadyOpened = false;
                                if ( !pNode->m_aGroupedLeaves.empty() )
                                {
                                    for (auto & rGroup : pNode->m_aGroupedLeaves)
                                    {
                                        if ( !rGroup.empty() &&
                                             rGroup[0]->m_sGroupId == sLeafGrpId )
                                        {
                                            std::vector<std::unique_ptr<OptionsLeaf>>::size_type l = 0;
                                            for ( ; l < rGroup.size(); ++l )
                                            {
                                                if ( rGroup[l]->m_nGroupIndex >= nLeafGrpIdx )
                                                    break;
                                            }
                                            rGroup.insert( rGroup.begin() + l, std::move(pLeaf) );
                                            bAlreadyOpened = true;
                                            break;
                                        }
                                    }
                                }
                                if ( !bAlreadyOpened )
                                {
                                    std::vector< std::unique_ptr<OptionsLeaf> > aGroupedLeaves;
                                    aGroupedLeaves.push_back( std::move(pLeaf) );
                                    pNode->m_aGroupedLeaves.push_back( std::move(aGroupedLeaves) );
                                }
                            }
                            else
                                pNode->m_aLeaves.push_back( std::move(pLeaf) );
                        }
                    }
                }
            }

            // do not insert nodes without leaves
            if ( !pNode->m_aLeaves.empty() || !pNode->m_aGroupedLeaves.empty() )
            {
                pModule ? aNodeList.push_back( std::move(pNode) ) : aOutNodeList.push_back( std::move(pNode) );
            }
        }
    }

    if ( pModule && !aNodeList.empty() )
    {
        for ( auto const & i: pModule->m_aNodeList )
        {
            OUString sNodeId = i->m_sId;
            for ( auto j = aNodeList.begin(); j != aNodeList.end(); ++j )
            {
                if ( (*j)->m_sId == sNodeId )
                {
                    aOutNodeList.push_back( std::move(*j) );
                    aNodeList.erase( j );
                    break;
                }
            }
        }

        for ( auto & i: aNodeList )
            aOutNodeList.push_back( std::move(i) );
    }
    return aOutNodeList;
}

static sal_uInt16 lcl_getGroupId( std::u16string_view rGroupName, const weld::TreeView& rTreeLB )
{
    sal_uInt16 nRet = 0;

    std::unique_ptr<weld::TreeIter> xEntry = rTreeLB.make_iterator();
    bool bEntry = rTreeLB.get_iter_first(*xEntry);
    while (bEntry)
    {
        if (!rTreeLB.get_iter_depth(*xEntry))
        {
            OUString sTemp(rTreeLB.get_text(*xEntry));
            if (sTemp == rGroupName)
                return nRet;
            nRet++;
        }
        bEntry = rTreeLB.iter_next(*xEntry);
    }

    return USHRT_MAX;
}

static void lcl_insertLeaf(
    OfaTreeOptionsDialog* pDlg, OptionsNode const * pNode, OptionsLeaf const * pLeaf, const weld::TreeView& rTreeLB )
{
    sal_uInt16 nGrpId = lcl_getGroupId( pNode->m_sLabel, rTreeLB );
    if ( USHRT_MAX == nGrpId )
    {
        sal_uInt16 nNodeGrpId = getGroupNodeId( pNode->m_sId );
        nGrpId = pDlg->AddGroup( pNode->m_sLabel, nullptr, nullptr, nNodeGrpId );
    }
    OptionsPageInfo* pInfo = pDlg->AddTabPage( 0, pLeaf->m_sLabel, nGrpId );
    pInfo->m_sPageURL = pLeaf->m_sPageURL;
    pInfo->m_sEventHdl = pLeaf->m_sEventHdl;
}

void  OfaTreeOptionsDialog::InsertNodes( const VectorOfNodes& rNodeList )
{
    for (auto const& node : rNodeList)
    {
        if ( !node->m_aLeaves.empty() || !node->m_aGroupedLeaves.empty() )
        {
            for ( auto const & j: node->m_aGroupedLeaves )
            {
                for ( size_t k = 0; k < j.size(); ++k )
                {
                    lcl_insertLeaf( this, node.get(), j[k].get(), *xTreeLB );
                }
            }

            for ( auto const & j: node->m_aLeaves )
            {
                lcl_insertLeaf( this, node.get(), j.get(), *xTreeLB );
            }
        }
    }
}

void OfaTreeOptionsDialog::SetNeedsRestart( svtools::RestartReason eReason)
{
    bNeedsRestart = true;
    eRestartReason = eReason;
}

short OfaTreeOptionsDialog::run()
{
    std::unique_ptr< SvxDicListChgClamp > pClamp;
    if ( !bIsFromExtensionManager )
    {
        // collect all DictionaryList Events while the dialog is executed
        pClamp.reset(new SvxDicListChgClamp(LinguMgr::GetDictionaryList()));
    }

    return SfxOkDialogController::run();
}

// class ExtensionsTabPage -----------------------------------------------
ExtensionsTabPage::ExtensionsTabPage(
    weld::Container* pParent, OUString aPageURL,
    OUString aEvtHdl, const Reference< awt::XContainerWindowProvider >& rProvider )
    : m_pContainer(pParent)
    , m_sPageURL(std::move(aPageURL))
    , m_sEventHdl(std::move(aEvtHdl))
    , m_xWinProvider(rProvider)
{
}

ExtensionsTabPage::~ExtensionsTabPage()
{
    Hide();
    DeactivatePage();

    if ( m_xPage.is() )
    {
        try
        {
            m_xPage->dispose();
        }
        catch (const Exception&)
        {
        }
        m_xPage.clear();
    }

    if ( m_xPageParent.is() )
    {
        try
        {
            m_xPageParent->dispose();
        }
        catch (const Exception&)
        {
        }
        m_xPageParent.clear();
    }
}

void ExtensionsTabPage::CreateDialogWithHandler()
{
    try
    {
        bool bWithHandler = !m_sEventHdl.isEmpty();
        if ( bWithHandler )
        {
            Reference < XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
            m_xEventHdl.set( xFactory->createInstance( m_sEventHdl ), UNO_QUERY );
        }

        if ( !bWithHandler || m_xEventHdl.is() )
        {
            m_xPageParent = m_pContainer->CreateChildFrame();
            Reference<awt::XWindowPeer> xParent(m_xPageParent, UNO_QUERY);
            m_xPage =
                m_xWinProvider->createContainerWindow(
                    m_sPageURL, OUString(), xParent, m_xEventHdl );

            Reference< awt::XControl > xPageControl( m_xPage, UNO_QUERY );
            if ( xPageControl.is() )
            {
                Reference< awt::XWindowPeer > xWinPeer( xPageControl->getPeer() );
                if ( xWinPeer.is() )
                {
                    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xWinPeer );
                    if ( pWindow )
                        pWindow->SetStyle( pWindow->GetStyle() | WB_DIALOGCONTROL | WB_CHILDDLGCTRL );
                }
            }
        }
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "cui.options", "ExtensionsTabPage::CreateDialogWithHandler(): exception of XDialogProvider2::createDialogWithHandler()");
    }
}

bool ExtensionsTabPage::DispatchAction( const OUString& rAction )
{
    bool bRet = false;
    if ( m_xEventHdl.is() )
    {
        try
        {
            bRet = m_xEventHdl->callHandlerMethod( m_xPage, Any( rAction ), "external_event" );
        }
        catch ( Exception const & )
        {
            TOOLS_WARN_EXCEPTION( "cui.options", "ExtensionsTabPage::DispatchAction(): exception of XDialogEventHandler::callHandlerMethod()" );
        }
    }
    return bRet;
}

void ExtensionsTabPage::Show()
{
    if (!m_xPageParent.is())
        return;

    VclPtr<vcl::Window> xPageParent = VCLUnoHelper::GetWindow(m_xPageParent);
    if (xPageParent)
    {
        // NoActivate otherwise setVisible will call Window::Show which will grab
        // focus to the page by default
        xPageParent->Show(true, ShowFlags::NoActivate);
    }

    m_xPageParent->setVisible(true);
}

void ExtensionsTabPage::Hide()
{
    if (!m_xPageParent.is())
        return;
    m_xPageParent->setVisible(false);
}

void ExtensionsTabPage::ActivatePage()
{
    if ( !m_xPage.is() )
    {
        CreateDialogWithHandler();

        if ( m_xPage.is() )
        {
            auto aWindowRect = m_xPageParent->getPosSize();
            m_xPage->setPosSize(0, 0, aWindowRect.Width, aWindowRect.Height, awt::PosSize::POSSIZE);
            if ( !m_sEventHdl.isEmpty() )
                DispatchAction( "initialize" );
        }
    }

    if ( m_xPage.is() )
    {
        m_xPage->setVisible( true );
    }
}

void ExtensionsTabPage::DeactivatePage()
{
    if ( m_xPage.is() )
        m_xPage->setVisible( false );
}

void ExtensionsTabPage::ResetPage()
{
    DispatchAction( "back" );
    ActivatePage();
}

void ExtensionsTabPage::SavePage()
{
    DispatchAction( "ok" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
