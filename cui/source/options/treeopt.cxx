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
#include <config_features.h>
#include <config_gpgme.h>

#include <svx/dialogs.hrc>

#include <strings.hrc>
#include <treeopt.hrc>
#include <helpids.h>

#include "cfgchart.hxx"
#include "connpoolconfig.hxx"
#include "connpooloptions.hxx"
#include <cuioptgenrl.hxx>
#include <cuitabarea.hxx>
#include <dbregister.hxx>
#include "dbregisterednamesconfig.hxx"
#include <dialmgr.hxx>
#include "fontsubs.hxx"
#include "optaboutconfig.hxx"
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

#include <com/sun/star/awt/XContainerWindowEventHandler.hpp>
#include <com/sun/star/awt/ContainerWindowProvider.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/UnknownModuleException.hpp>
#include <com/sun/star/loader/CannotActivateFactoryException.hpp>
#include <com/sun/star/linguistic2/LinguProperties.hpp>
#include <com/sun/star/setup/UpdateCheck.hpp>
#include <comphelper/getexpandeduri.hxx>
#include <comphelper/processfactory.hxx>
#include <editeng/langitem.hxx>
#include <editeng/optitems.hxx>
#include <editeng/unolingu.hxx>
#include <linguistic/misc.hxx>
#include <officecfg/Office/OptionsDialog.hxx>
#include <osl/module.hxx>
#include <osl/process.h>
#include <rtl/bootstrap.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/module.hxx>
#include <sfx2/printopt.hxx>
#include <sfx2/shell.hxx>
#include <sfx2/tplpitem.hxx>
#include <sfx2/viewsh.hxx>
#include <svl/languageoptions.hxx>
#include <svtools/helpopt.hxx>
#include <svtools/miscopt.hxx>
#include <svx/drawitem.hxx>
#include <svx/xtable.hxx>
#include <svx/xpool.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/configmgr.hxx>
#include <unotools/linguprops.hxx>
#include <unotools/misccfg.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/optionsdlg.hxx>
#include <unotools/viewoptions.hxx>
#include <vcl/help.hxx>
#include <vcl/layout.hxx>
#include <vcl/weld.hxx>
#include <vcl/waitobj.hxx>
#include <vcl/settings.hxx>
#include <vcl/treelistentry.hxx>
#include <sal/log.hxx>

#ifdef LINUX
#include <sys/stat.h>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

LastPageSaver* OfaTreeOptionsDialog::pLastPageSaver = nullptr;

// some stuff for easier changes for SvtViewOptions
static char const VIEWOPT_DATANAME[] = "page data";

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

struct ModuleToGroupNameMap_Impl
{
    const char* m_pModule;
    OUString    m_sGroupName;
    sal_uInt16  m_nNodeId;
};

static ModuleToGroupNameMap_Impl ModuleMap[] =
{
    { "ProductName", OUString(), SID_GENERAL_OPTIONS },
    { "LanguageSettings", OUString(), SID_LANGUAGE_OPTIONS },
    { "Internet", OUString(), SID_INET_DLG },
    { "LoadSave", OUString(), SID_FILTER_DLG },
    { "Writer", OUString(), SID_SW_EDITOPTIONS },
    { "WriterWeb", OUString(), SID_SW_ONLINEOPTIONS },
    { "Math", OUString(), SID_SM_EDITOPTIONS },
    { "Calc", OUString(), SID_SC_EDITOPTIONS },
    { "Impress", OUString(), SID_SD_EDITOPTIONS },
    { "Draw", OUString(), SID_SD_GRAPHIC_OPTIONS },
    { "Charts", OUString(), SID_SCH_EDITOPTIONS },
    { "Base", OUString(), SID_SB_STARBASEOPTIONS },

    { nullptr, OUString(), 0xFFFF }
};

static void setGroupName( const OUString& rModule, const OUString& rGroupName )
{
    sal_uInt16 nIndex = 0;
    while ( ModuleMap[ nIndex ].m_pModule )
    {
        OUString sTemp =
            OUString::createFromAscii( ModuleMap[ nIndex ].m_pModule );
        if ( sTemp == rModule )
        {
            ModuleMap[ nIndex ].m_sGroupName = rGroupName;
            break;
        }
        ++nIndex;
    }
}

static OUString getGroupName( const OUString& rModule, bool bForced )
{
    OUString sGroupName;
    sal_uInt16 nIndex = 0;
    while ( ModuleMap[ nIndex ].m_pModule )
    {
        OUString sTemp =
            OUString::createFromAscii( ModuleMap[ nIndex ].m_pModule );
        if ( sTemp == rModule )
        {
            sGroupName = ModuleMap[ nIndex ].m_sGroupName;
            break;
        }
        ++nIndex;
    }

    if ( sGroupName.isEmpty() && bForced )
    {
        if ( rModule == "Writer" )
            sGroupName = CuiResId(SID_SW_EDITOPTIONS_RES[0].first);
        else if ( rModule == "WriterWeb" )
            sGroupName = CuiResId(SID_SW_ONLINEOPTIONS_RES[0].first);
        else if ( rModule == "Calc" )
            sGroupName = CuiResId(SID_SC_EDITOPTIONS_RES[0].first);
        else if ( rModule == "Impress" )
            sGroupName = CuiResId(SID_SD_EDITOPTIONS_RES[0].first);
        else if ( rModule == "Draw" )
            sGroupName = CuiResId(SID_SD_GRAPHIC_OPTIONS_RES[0].first);
        else if ( rModule == "Math" )
            sGroupName = CuiResId(SID_SM_EDITOPTIONS_RES[0].first);
        else if ( rModule == "Base" )
            sGroupName = CuiResId(SID_SB_STARBASEOPTIONS_RES[0].first);
    }
    return sGroupName;
}

static void deleteGroupNames()
{
    sal_uInt16 nIndex = 0;
    while ( ModuleMap[ nIndex ].m_pModule )
        ModuleMap[ nIndex++ ].m_sGroupName.clear();
}

static sal_uInt16 getGroupNodeId( const OUString& rModule )
{
    sal_uInt16 nNodeId = 0xFFFF, nIndex = 0;
    while ( ModuleMap[ nIndex ].m_pModule )
    {
        OUString sTemp =
            OUString::createFromAscii( ModuleMap[ nIndex ].m_pModule );
        if ( sTemp == rModule )
        {
            nNodeId = ModuleMap[ nIndex ].m_nNodeId;
            break;
        }
        ++nIndex;
    }

    return nNodeId;
}

class MailMergeCfg_Impl : public utl::ConfigItem
{
private:
    friend class SvxEMailTabPage;
    // variables
    bool bIsEmailSupported;

    virtual void    ImplCommit() override;

public:
    MailMergeCfg_Impl();

    virtual void Notify( const css::uno::Sequence< OUString >& _rPropertyNames) override;

    bool IsEmailSupported() const {return bIsEmailSupported;}

};

MailMergeCfg_Impl::MailMergeCfg_Impl() :
    utl::ConfigItem("Office.Writer/MailMergeWizard"),
    bIsEmailSupported(false)
{
    Sequence<OUString> aNames { "EMailSupported" };
    const Sequence< Any > aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    if(aValues.getLength() && pValues[0].hasValue())
        pValues[0] >>= bIsEmailSupported;
}

void MailMergeCfg_Impl::ImplCommit()
{
}

void MailMergeCfg_Impl::Notify( const css::uno::Sequence< OUString >& )
{
}

//typedef SfxTabPage* (*FNCreateTabPage)(TabPageParent pParent, const SfxItemSet &rAttrSet);
static VclPtr<SfxTabPage> CreateGeneralTabPage(sal_uInt16 nId, TabPageParent pParent, const SfxItemSet& rSet)
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

    VclPtr<SfxTabPage> pRet = fnCreate ? (*fnCreate)( pParent, &rSet ) : nullptr;
    return pRet;
}

struct OptionsMapping_Impl
{
    const char* m_pGroupName;
    const char* m_pPageName;
    sal_uInt16      m_nPageId;
};

static OptionsMapping_Impl const OptionsMap_Impl[] =
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
    ScopedVclPtr<SfxTabPage> m_pPage;
    sal_uInt16          m_nPageId;
    OUString       m_sPageURL;
    OUString       m_sEventHdl;
    VclPtr<ExtensionsTabPage>  m_pExtPage;

    explicit OptionsPageInfo( sal_uInt16 nId ) : m_pPage( nullptr ), m_nPageId( nId ), m_pExtPage( nullptr ) {}
};

struct OptionsGroupInfo
{
    std::unique_ptr<SfxItemSet> m_pInItemSet;
    std::unique_ptr<SfxItemSet> m_pOutItemSet;
    SfxShell*           m_pShell;       // used to create the page
    SfxModule*          m_pModule;      // used to create the ItemSet
    sal_uInt16          m_nDialogId;    // Id of the former dialog

    OptionsGroupInfo( SfxShell* pSh, SfxModule* pMod, sal_uInt16 nId ) :
        m_pShell( pSh ),
        m_pModule( pMod ), m_nDialogId( nId ) {}
};

#define INI_LIST() \
    m_pParent           ( pParent ),\
    pCurrentPageEntry   ( nullptr ),\
    sTitle              ( GetText() ),\
    bForgetSelection    ( false ),\
    bIsFromExtensionManager( false ), \
    bIsForSetDocumentLanguage( false ), \
    bNeedsRestart ( false ), \
    eRestartReason( svtools::RESTART_REASON_NONE )


void OfaTreeOptionsDialog::InitWidgets()
{
    get(pOkPB, "ok");
    get(pApplyPB, "apply");
    get(pBackPB, "revert");
    get(pTreeLB, "pages");
    get(pTabBox, "box");
    Size aSize(pTabBox->LogicToPixel(Size(278, 259), MapMode(MapUnit::MapAppFont)));
    pTabBox->set_width_request(aSize.Width());
#if HAVE_FEATURE_GPGME
    // tdf#115015: make enough space for crypto settings (approx. 14 text edits + padding)
    pTabBox->set_height_request((Edit::GetMinimumEditSize().Height() + 6) * 14);
#else
    pTabBox->set_height_request(aSize.Height() - get_action_area()->get_preferred_size().Height());
#endif
    pTreeLB->set_width_request(pTreeLB->approximate_char_width() * 25);
    pTreeLB->set_height_request(pTabBox->get_height_request());

}

// Ctor() with Frame -----------------------------------------------------
OfaTreeOptionsDialog::OfaTreeOptionsDialog(
    vcl::Window* pParent,
    const Reference< XFrame >& _xFrame,
    bool bActivateLastSelection ) :

    SfxModalDialog( pParent, "OptionsDialog", "cui/ui/optionsdialog.ui" ),
    INI_LIST()
{
    InitWidgets();

    InitTreeAndHandler();
    Initialize( _xFrame );
    LoadExtensionOptions( OUString() );
    if (bActivateLastSelection)
        ActivateLastSelection();

    pTreeLB->SetAccessibleName(GetDisplayText());
}

// Ctor() with ExtensionId -----------------------------------------------
OfaTreeOptionsDialog::OfaTreeOptionsDialog( vcl::Window* pParent, const OUString& rExtensionId ) :

    SfxModalDialog( pParent, "OptionsDialog", "cui/ui/optionsdialog.ui" ),
    INI_LIST()
{
    InitWidgets();

    bIsFromExtensionManager = ( !rExtensionId.isEmpty() );
    InitTreeAndHandler();
    LoadExtensionOptions( rExtensionId );
    ActivateLastSelection();
}

OfaTreeOptionsDialog::~OfaTreeOptionsDialog()
{
    disposeOnce();
}

void OfaTreeOptionsDialog::dispose()
{
    pCurrentPageEntry = nullptr;
    SvTreeListEntry* pEntry = pTreeLB ? pTreeLB->First() : nullptr;
    // first children
    while(pEntry)
    {
        // if Child (has parent), then OptionsPageInfo
        if(pTreeLB->GetParent(pEntry))
        {
            OptionsPageInfo *pPageInfo = static_cast<OptionsPageInfo *>(pEntry->GetUserData());
            if(pPageInfo->m_pPage)
            {
                pPageInfo->m_pPage->FillUserData();
                OUString aPageData(pPageInfo->m_pPage->GetUserData());
                if ( !aPageData.isEmpty() )
                {
                    SvtViewOptions aTabPageOpt( EViewType::TabPage, OUString::number( pPageInfo->m_nPageId) );
                    SetViewOptUserItem( aTabPageOpt, aPageData );
                }
                pPageInfo->m_pPage.disposeAndClear();
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

            pPageInfo->m_pExtPage.disposeAndClear();

            delete pPageInfo;
        }
        pEntry = pTreeLB->Next(pEntry);
    }

    // and parents
    pEntry = pTreeLB ? pTreeLB->First() : nullptr;
    while(pEntry)
    {
        if(!pTreeLB->GetParent(pEntry))
        {
            OptionsGroupInfo* pGroupInfo = static_cast<OptionsGroupInfo*>(pEntry->GetUserData());
            delete pGroupInfo;
        }
        pEntry = pTreeLB->Next(pEntry);
    }
    deleteGroupNames();
    m_pParent.clear();
    pOkPB.clear();
    pApplyPB.clear();
    pBackPB.clear();
    pTreeLB.clear();
    pTabBox.clear();
    SfxModalDialog::dispose();
}

OptionsPageInfo* OfaTreeOptionsDialog::AddTabPage(
    sal_uInt16 nId, const OUString& rPageName, sal_uInt16 nGroup )
{
    OptionsPageInfo* pPageInfo = new OptionsPageInfo( nId );
    SvTreeListEntry* pParent = pTreeLB->GetEntry( nullptr, nGroup );
    DBG_ASSERT( pParent, "OfaTreeOptionsDialog::AddTabPage(): no group found" );
    SvTreeListEntry* pEntry = pTreeLB->InsertEntry( rPageName, pParent );
    pEntry->SetUserData( pPageInfo );
    return pPageInfo;
}

// the ItemSet* is passed on to the dialog's ownership
sal_uInt16  OfaTreeOptionsDialog::AddGroup(const OUString& rGroupName,
                                        SfxShell* pCreateShell,
                                        SfxModule* pCreateModule,
                                        sal_uInt16 nDialogId )
{
    SvTreeListEntry* pEntry = pTreeLB->InsertEntry(rGroupName);
    OptionsGroupInfo* pInfo =
        new OptionsGroupInfo( pCreateShell, pCreateModule, nDialogId );
    pEntry->SetUserData(pInfo);
    sal_uInt16 nRet = 0;
    pEntry = pTreeLB->First();
    while(pEntry)
    {
        if(!pTreeLB->GetParent(pEntry))
            nRet++;
        pEntry = pTreeLB->Next(pEntry);
    }
    return nRet - 1;
}

IMPL_LINK_NOARG(OfaTreeOptionsDialog, ShowPageHdl_Impl, SvTreeListBox*, void)
{
    SelectHdl_Impl();
}

IMPL_LINK_NOARG(OfaTreeOptionsDialog, BackHdl_Impl, Button*, void)
{
    if ( pCurrentPageEntry && pTreeLB->GetParent( pCurrentPageEntry ) )
    {
        OptionsPageInfo* pPageInfo = static_cast<OptionsPageInfo*>(pCurrentPageEntry->GetUserData());
        if ( pPageInfo->m_pPage )
        {
            OptionsGroupInfo* pGroupInfo =
                static_cast<OptionsGroupInfo*>(pTreeLB->GetParent( pCurrentPageEntry )->GetUserData());
            pPageInfo->m_pPage->Reset( pGroupInfo->m_pInItemSet.get() );
        }
        else if ( pPageInfo->m_pExtPage )
            pPageInfo->m_pExtPage->ResetPage();
    }
}

void OfaTreeOptionsDialog::ApplyOptions(bool deactivate)
{
    SvTreeListEntry* pEntry = pTreeLB->First();
    while ( pEntry )
    {
        if ( pTreeLB->GetParent( pEntry ) )
        {
            OptionsPageInfo* pPageInfo = static_cast<OptionsPageInfo *>(pEntry->GetUserData());
            if ( pPageInfo->m_pPage && !pPageInfo->m_pPage->HasExchangeSupport() )
            {
                OptionsGroupInfo* pGroupInfo =
                    static_cast<OptionsGroupInfo*>(pTreeLB->GetParent(pEntry)->GetUserData());
                pPageInfo->m_pPage->FillItemSet(pGroupInfo->m_pOutItemSet.get());
            }

            if ( pPageInfo->m_pExtPage )
            {
                if ( deactivate )
                {
                    pPageInfo->m_pExtPage->DeactivatePage();
                }
                pPageInfo->m_pExtPage->SavePage();
            }
            if ( pPageInfo->m_pPage && RID_OPTPAGE_CHART_DEFCOLORS == pPageInfo->m_nPageId )
            {
                SvxDefaultColorOptPage* pPage = static_cast<SvxDefaultColorOptPage *>(pPageInfo->m_pPage.get());
                pPage->SaveChartOptions();
            }
        }
        pEntry = pTreeLB->Next(pEntry);
    }
}

IMPL_LINK_NOARG(OfaTreeOptionsDialog, ApplyHdl_Impl, Button*, void)
{
    ApplyOptions(/*deactivate =*/false);

    if ( bNeedsRestart )
    {
        SolarMutexGuard aGuard;
        if (svtools::executeRestartDialog(comphelper::getProcessComponentContext(),
                                        GetFrameWeld(), eRestartReason))
            EndDialog(RET_OK);
    }
}

IMPL_LINK_NOARG(OfaTreeOptionsDialog, OKHdl_Impl, Button*, void)
{
    pTreeLB->EndSelection();
    if ( pCurrentPageEntry && pTreeLB->GetParent( pCurrentPageEntry ) )
    {
        OptionsPageInfo* pPageInfo = static_cast<OptionsPageInfo *>(pCurrentPageEntry->GetUserData());
        if ( pPageInfo->m_pPage )
        {
            OptionsGroupInfo* pGroupInfo =
                static_cast<OptionsGroupInfo *>(pTreeLB->GetParent(pCurrentPageEntry)->GetUserData());
            if ( RID_SVXPAGE_COLOR != pPageInfo->m_nPageId
                && pPageInfo->m_pPage->HasExchangeSupport() )
            {
                DeactivateRC nLeave = pPageInfo->m_pPage->DeactivatePage(pGroupInfo->m_pOutItemSet.get());
                if ( nLeave == DeactivateRC::KeepPage )
                {
                    // the page mustn't be left
                    pTreeLB->Select(pCurrentPageEntry);
                    return;
                }
            }
            pPageInfo->m_pPage->Hide();
        }
    }

    ApplyOptions(/*deactivate =*/ true);
    EndDialog(RET_OK);

    if ( bNeedsRestart )
    {
        SolarMutexGuard aGuard;
        ::svtools::executeRestartDialog(comphelper::getProcessComponentContext(),
                                        m_pParent->GetFrameWeld(), eRestartReason);
    }
}

// an opened group shall be completely visible
IMPL_STATIC_LINK(
    OfaTreeOptionsDialog, ExpandedHdl_Impl, SvTreeListBox*, pBox, void )
{
    pBox->Update();
    pBox->InitStartEntry();
    SvTreeListEntry* pEntry = pBox->GetHdlEntry();
    if(pEntry && pBox->IsExpanded(pEntry))
    {
        sal_uInt32 nChildCount = pBox->GetChildCount( pEntry );

        SvTreeListEntry* pNext = pEntry;
        for(sal_uInt32 i = 0; i < nChildCount;i++)
        {
            pNext = pBox->GetNextEntryInView(pNext);
            if(!pNext)
            {
                pBox->ScrollOutputArea( -static_cast<short>(nChildCount - i + 1) );
                break;
            }
            else
            {
                Size aSz(pBox->GetOutputSizePixel());
                int nHeight = pBox->GetEntryHeight();
                Point aPos(pBox->GetEntryPosition(pNext));
                if(aPos.Y()+nHeight > aSz.Height())
                {
                    pBox->ScrollOutputArea( -static_cast<short>(nChildCount - i + 1) );
                    break;
                }
            }
        }
    }
}

void OfaTreeOptionsDialog::ApplyItemSets()
{
    SvTreeListEntry* pEntry = pTreeLB->First();
    while(pEntry)
    {
        if(!pTreeLB->GetParent(pEntry))
        {
            OptionsGroupInfo* pGroupInfo = static_cast<OptionsGroupInfo *>(pEntry->GetUserData());
            if(pGroupInfo->m_pOutItemSet)
            {
                if(pGroupInfo->m_pShell)
                    pGroupInfo->m_pShell->ApplyItemSet( pGroupInfo->m_nDialogId, *pGroupInfo->m_pOutItemSet);
                else
                    ApplyItemSet( pGroupInfo->m_nDialogId, *pGroupInfo->m_pOutItemSet);
            }
        }
        pEntry = pTreeLB->Next(pEntry);
    }
}

void OfaTreeOptionsDialog::InitTreeAndHandler()
{
    pTreeLB->SetNodeDefaultImages();

    pTreeLB->SetHelpId( HID_OFADLG_TREELISTBOX );
    pTreeLB->SetStyle( pTreeLB->GetStyle()|WB_HASBUTTONS | WB_HASBUTTONSATROOT |
                           WB_HASLINES | WB_HASLINESATROOT |
                           WB_CLIPCHILDREN | WB_HSCROLL );
    pTreeLB->SetForceMakeVisible(true);
    pTreeLB->SetQuickSearch(true);
    pTreeLB->SetSpaceBetweenEntries( 0 );
    pTreeLB->SetSelectionMode( SelectionMode::Single );
    pTreeLB->SetSublistOpenWithLeftRight();
    pTreeLB->SetExpandedHdl( LINK( this, OfaTreeOptionsDialog, ExpandedHdl_Impl ) );
    pTreeLB->SetSelectHdl( LINK( this, OfaTreeOptionsDialog, ShowPageHdl_Impl ) );
    pBackPB->SetClickHdl( LINK( this, OfaTreeOptionsDialog, BackHdl_Impl ) );
    pApplyPB->SetClickHdl( LINK( this, OfaTreeOptionsDialog, ApplyHdl_Impl ) );
    pOkPB->SetClickHdl( LINK( this, OfaTreeOptionsDialog, OKHdl_Impl ) );
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
    SvTreeListEntry* pEntry = nullptr;
    if ( pLastPageSaver )
    {
        OUString sLastURL = bIsFromExtensionManager ? pLastPageSaver->m_sLastPageURL_ExtMgr
                                                  : pLastPageSaver->m_sLastPageURL_Tools;
        if ( sLastURL.isEmpty() )
        {
            sLastURL = !bIsFromExtensionManager ? pLastPageSaver->m_sLastPageURL_ExtMgr
                                                : pLastPageSaver->m_sLastPageURL_Tools;
        }

        bool bMustExpand = ( INetURLObject( sLastURL ).GetProtocol() == INetProtocol::File );

        SvTreeListEntry* pTemp = pTreeLB->First();
        while( !pEntry && pTemp )
        {
            // restore only selection of a leaf
            if ( pTreeLB->GetParent( pTemp ) && pTemp->GetUserData() )
            {
                OptionsPageInfo* pPageInfo = static_cast<OptionsPageInfo*>(pTemp->GetUserData());
                OUString sPageURL = pPageInfo->m_sPageURL;
                if ( bMustExpand )
                {
                    sPageURL = comphelper::getExpandedUri(
                        comphelper::getProcessComponentContext(), sPageURL);
                }

                if ( ( !bIsFromExtensionManager
                        && pPageInfo->m_nPageId && pPageInfo->m_nPageId == pLastPageSaver->m_nLastPageId )
                            || ( !pPageInfo->m_nPageId && sLastURL == sPageURL ) )
                    pEntry = pTemp;
            }
            pTemp = pTreeLB->Next(pTemp);
        }
    }

    if ( !pEntry )
    {
        pEntry = pTreeLB->First();
        pEntry = pTreeLB->Next(pEntry);
    }

    if ( !pEntry )
        return;

    SvTreeListEntry* pParent = pTreeLB->GetParent(pEntry);
    pTreeLB->Expand(pParent);
    pTreeLB->MakeVisible(pParent);
    pTreeLB->MakeVisible(pEntry);
    pTreeLB->Select(pEntry);
    pTreeLB->GrabFocus();
}

bool OfaTreeOptionsDialog::EventNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        const vcl::KeyCode aKeyCode = pKEvt->GetKeyCode();

        if( aKeyCode.GetCode() == KEY_PAGEUP ||
                aKeyCode.GetCode() == KEY_PAGEDOWN)
        {
            SvTreeListEntry* pCurEntry = pTreeLB->FirstSelected();
            SvTreeListEntry*  pTemp = nullptr;
            if(aKeyCode.GetCode() == KEY_PAGEDOWN)
            {
                pTemp =  pTreeLB->Next( pCurEntry ) ;
                if(pTemp && !pTreeLB->GetParent(pTemp))
                {
                    pTemp =  pTreeLB->Next( pTemp ) ;
                    pTreeLB->Select(pTemp);
                }
            }
            else
            {
                pTemp =  pTreeLB->Prev( pCurEntry ) ;
                if(pTemp && !pTreeLB->GetParent(pTemp))
                {
                    pTemp =  pTreeLB->Prev( pTemp ) ;
                }
            }
            if(pTemp)
            {
                if(!pTreeLB->IsExpanded(pTreeLB->GetParent(pTemp)))
                    pTreeLB->Expand(pTreeLB->GetParent(pTemp));
                pTreeLB->MakeVisible(pTemp);
                pTreeLB->Select(pTemp);
            }
        }
    }
    return SfxModalDialog::EventNotify(rNEvt);
}

void OfaTreeOptionsDialog::SelectHdl_Impl()
{
    SvTreeListBox* pBox = pTreeLB;

    if(pCurrentPageEntry == pBox->GetCurEntry())
    {
        pBox->EndSelection();
        return;
    }

    SvTreeListEntry* pEntry = pBox->GetCurEntry();
    SvTreeListEntry* pParent = pBox->GetParent(pEntry);

    // If the user has selected a category, automatically switch to a suitable
    // default sub-page instead.
    if (!pParent)
    {
        pBox->EndSelection();
        return;
    }

    pBox->EndSelection();

    TabPage* pOldPage = nullptr;
    TabPage* pNewPage = nullptr;
    OptionsPageInfo* pOptPageInfo = ( pCurrentPageEntry && pTreeLB->GetParent( pCurrentPageEntry ) )
        ? static_cast<OptionsPageInfo*>(pCurrentPageEntry->GetUserData()) : nullptr;

    if ( pOptPageInfo && pOptPageInfo->m_pPage && pOptPageInfo->m_pPage->IsVisible() )
    {
        pOldPage = pOptPageInfo->m_pPage;
        OptionsGroupInfo* pGroupInfo = static_cast<OptionsGroupInfo*>(pTreeLB->GetParent(pCurrentPageEntry)->GetUserData());
        DeactivateRC nLeave = DeactivateRC::LeavePage;
        if ( RID_SVXPAGE_COLOR != pOptPageInfo->m_nPageId && pOptPageInfo->m_pPage->HasExchangeSupport() )
           nLeave = pOptPageInfo->m_pPage->DeactivatePage( pGroupInfo->m_pOutItemSet.get() );

        if ( nLeave == DeactivateRC::KeepPage )
        {
            // we cannot leave this page
            pBox->Select( pCurrentPageEntry );
            return;
        }
        else
            pOptPageInfo->m_pPage->Hide();
    }
    else if ( pOptPageInfo && pOptPageInfo->m_pExtPage )
    {
        pOptPageInfo->m_pExtPage->Hide();
        pOptPageInfo->m_pExtPage->DeactivatePage();
    }

    OptionsPageInfo *pPageInfo = static_cast<OptionsPageInfo *>(pEntry->GetUserData());
    OptionsGroupInfo* pGroupInfo = static_cast<OptionsGroupInfo *>(pParent->GetUserData());
    if(!pPageInfo->m_pPage && pPageInfo->m_nPageId > 0)
    {
        if(!pGroupInfo->m_pInItemSet)
            pGroupInfo->m_pInItemSet = pGroupInfo->m_pShell
                ? pGroupInfo->m_pShell->CreateItemSet( pGroupInfo->m_nDialogId )
                : CreateItemSet( pGroupInfo->m_nDialogId );
        if(!pGroupInfo->m_pOutItemSet)
            pGroupInfo->m_pOutItemSet = std::make_unique<SfxItemSet>(
                *pGroupInfo->m_pInItemSet->GetPool(),
                pGroupInfo->m_pInItemSet->GetRanges());

        TabPageParent pPageParent(pTabBox);

        pPageInfo->m_pPage.disposeAndReset( ::CreateGeneralTabPage(pPageInfo->m_nPageId, pPageParent, *pGroupInfo->m_pInItemSet ) );

        if(!pPageInfo->m_pPage && pGroupInfo->m_pModule)
            pPageInfo->m_pPage.disposeAndReset(pGroupInfo->m_pModule->CreateTabPage(pPageInfo->m_nPageId, pPageParent, *pGroupInfo->m_pInItemSet));

        DBG_ASSERT( pPageInfo->m_pPage, "tabpage could not created");
        if ( pPageInfo->m_pPage )
        {
            SvtViewOptions aTabPageOpt( EViewType::TabPage, OUString::number( pPageInfo->m_nPageId) );
            pPageInfo->m_pPage->SetUserData( GetViewOptUserItem( aTabPageOpt ) );
            pPageInfo->m_pPage->Reset( pGroupInfo->m_pInItemSet.get() );
        }
    }
    else if ( 0 == pPageInfo->m_nPageId && !pPageInfo->m_pExtPage )
    {
        if ( !m_xContainerWinProvider.is() )
        {
            m_xContainerWinProvider = awt::ContainerWindowProvider::create( ::comphelper::getProcessComponentContext() );
        }

        pPageInfo->m_pExtPage = VclPtr<ExtensionsTabPage>::Create(

            pTabBox, 0, pPageInfo->m_sPageURL, pPageInfo->m_sEventHdl, m_xContainerWinProvider );
    }

    if ( pPageInfo->m_pPage )
    {
        if ( RID_SVXPAGE_COLOR != pPageInfo->m_nPageId &&
             pPageInfo->m_pPage->HasExchangeSupport())
        {
            pPageInfo->m_pPage->ActivatePage(*pGroupInfo->m_pOutItemSet);
        }
        pPageInfo->m_pPage->Show();
    }
    else if ( pPageInfo->m_pExtPage )
    {
        pPageInfo->m_pExtPage->Show();
        pPageInfo->m_pExtPage->ActivatePage();
    }

    {
        OUString sTitleText = sTitle
                            + " - " + pTreeLB->GetEntryText(pParent)
                            + " - " + pTreeLB->GetEntryText(pEntry);
        SetText(sTitleText);
    }

    pCurrentPageEntry = pEntry;
    if ( !bForgetSelection )
    {
        if ( !pLastPageSaver )
            pLastPageSaver = new LastPageSaver;
        if ( !bIsFromExtensionManager )
            pLastPageSaver->m_nLastPageId = pPageInfo->m_nPageId;
        if ( pPageInfo->m_pExtPage )
        {
            if ( bIsFromExtensionManager )
                pLastPageSaver->m_sLastPageURL_ExtMgr = pPageInfo->m_sPageURL;
            else
                pLastPageSaver->m_sLastPageURL_Tools = pPageInfo->m_sPageURL;
        }
    }
    pNewPage = pPageInfo->m_pPage;

    // restore lost focus, if necessary
    vcl::Window* pFocusWin = Application::GetFocusWindow();
    // if the focused window is not the options treebox and the old page has the focus
    if ( pFocusWin && pFocusWin != pBox && pOldPage && pOldPage->HasChildPathFocus() )
        // then set the focus to the new page or if we are on a group set the focus to the options treebox
        pNewPage ? pNewPage->GrabFocus() : pBox->GrabFocus();

    //fdo#58170 use current page's layout child HelpId, unless there isn't a
    //current page
    OString sHelpId(HID_OFADLG_TREELISTBOX);
    if (::isLayoutEnabled(pNewPage))
    {
        vcl::Window *pFirstChild = pNewPage->GetWindow(GetWindowType::FirstChild);
        assert(pFirstChild);
        sHelpId = pFirstChild->GetHelpId();
    }
    pBox->SetHelpId(sHelpId);
}

std::unique_ptr<SfxItemSet> OfaTreeOptionsDialog::CreateItemSet( sal_uInt16 nId )
{
    Reference< XLinguProperties >  xProp( LinguMgr::GetLinguPropertySet() );
    std::unique_ptr<SfxItemSet> pRet;
    switch(nId)
    {
        case SID_GENERAL_OPTIONS:
        {
            pRet = std::make_unique<SfxItemSet>(
                SfxGetpApp()->GetPool(),
                svl::Items<
                    SID_HTML_MODE, SID_HTML_MODE,
                    SID_ATTR_METRIC, SID_ATTR_METRIC,
                    SID_AUTOSPELL_CHECK, SID_AUTOSPELL_CHECK,
                    SID_ATTR_QUICKLAUNCHER, SID_ATTR_QUICKLAUNCHER,
                    SID_ATTR_YEAR2000, SID_ATTR_YEAR2000>{} );

            SfxItemSet aOptSet( SfxGetpApp()->GetPool(), svl::Items<SID_ATTR_QUICKLAUNCHER, SID_ATTR_QUICKLAUNCHER>{} );
            SfxGetpApp()->GetOptions(aOptSet);
            pRet->Put(aOptSet);

            utl::MiscCfg    aMisc;
            SfxViewFrame* pViewFrame = SfxViewFrame::Current();
            if ( pViewFrame )
            {
                const SfxPoolItem* pItem = nullptr;
                SfxDispatcher* pDispatch = pViewFrame->GetDispatcher();

                // miscellaneous - Year2000
                if( SfxItemState::DEFAULT <= pDispatch->QueryState( SID_ATTR_YEAR2000, pItem ) )
                    pRet->Put( SfxUInt16Item( SID_ATTR_YEAR2000, static_cast<const SfxUInt16Item*>(pItem)->GetValue() ) );
                else
                    pRet->Put( SfxUInt16Item( SID_ATTR_YEAR2000, static_cast<sal_uInt16>(aMisc.GetYear2000()) ) );
            }
            else
                pRet->Put( SfxUInt16Item( SID_ATTR_YEAR2000, static_cast<sal_uInt16>(aMisc.GetYear2000()) ) );


            // miscellaneous - Tabulator
            pRet->Put(SfxBoolItem(SID_PRINTER_NOTFOUND_WARN, aMisc.IsNotFoundWarning()));

            SfxPrinterChangeFlags nFlag = aMisc.IsPaperSizeWarning() ? SfxPrinterChangeFlags::CHG_SIZE : SfxPrinterChangeFlags::NONE;
            nFlag  |= aMisc.IsPaperOrientationWarning()  ? SfxPrinterChangeFlags::CHG_ORIENTATION : SfxPrinterChangeFlags::NONE;
            pRet->Put( SfxFlagItem( SID_PRINTER_CHANGESTODOC, static_cast<int>(nFlag) ));

        }
        break;
        case SID_LANGUAGE_OPTIONS :
        {
            pRet = std::make_unique<SfxItemSet>(
                SfxGetpApp()->GetPool(),
                svl::Items<
                    SID_ATTR_CHAR_CJK_LANGUAGE, SID_ATTR_CHAR_CJK_LANGUAGE,
                    SID_ATTR_CHAR_CTL_LANGUAGE, SID_ATTR_CHAR_CTL_LANGUAGE,
                    SID_SET_DOCUMENT_LANGUAGE, SID_SET_DOCUMENT_LANGUAGE,
                    SID_ATTR_LANGUAGE, SID_ATTR_LANGUAGE,
                    SID_AUTOSPELL_CHECK, SID_AUTOSPELL_CHECK,
                    SID_OPT_LOCALE_CHANGED, SID_OPT_LOCALE_CHANGED>{});

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
                const SfxPoolItem* pItem = nullptr;
                SfxDispatcher* pDispatch = pViewFrame->GetDispatcher();
                if(SfxItemState::DEFAULT <= pDispatch->QueryState(SID_ATTR_LANGUAGE, pItem))
                    pRet->Put(
                        SvxLanguageItem(
                            (static_cast<const SvxLanguageItem*>(pItem)
                             ->GetLanguage()),
                            SID_ATTR_LANGUAGE));
                if(SfxItemState::DEFAULT <= pDispatch->QueryState(SID_ATTR_CHAR_CJK_LANGUAGE, pItem))
                    pRet->Put(
                        SvxLanguageItem(
                            (static_cast<const SvxLanguageItem*>(pItem)
                             ->GetLanguage()),
                            SID_ATTR_CHAR_CJK_LANGUAGE));
                if(SfxItemState::DEFAULT <= pDispatch->QueryState(SID_ATTR_CHAR_CTL_LANGUAGE, pItem))
                    pRet->Put(
                        SvxLanguageItem(
                            (static_cast<const SvxLanguageItem*>(pItem)
                             ->GetLanguage()),
                            SID_ATTR_CHAR_CTL_LANGUAGE));

                pRet->Put(aHyphen);
                if(SfxItemState::DEFAULT <= pDispatch->QueryState(SID_AUTOSPELL_CHECK, pItem))
                {
                    std::unique_ptr<SfxPoolItem> pClone(pItem->Clone());
                    pRet->Put(*pClone);
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
                pRet = std::make_unique<SfxItemSet>( SfxGetpApp()->GetPool(),
                                svl::Items<SID_BASIC_ENABLED, SID_BASIC_ENABLED,
                //SID_OPTIONS_START - ..END
                                SID_SAVEREL_INET, SID_SAVEREL_FSYS,
                                SID_INET_NOPROXY, SID_INET_FTP_PROXY_PORT,
                                SID_SECURE_URL, SID_SECURE_URL>{} );
                SfxGetpApp()->GetOptions(*pRet);
        break;
        case SID_FILTER_DLG:
            pRet = std::make_unique<SfxItemSet>(
                SfxGetpApp()->GetPool(),
                svl::Items<
                    SID_ATTR_WARNALIENFORMAT, SID_ATTR_WARNALIENFORMAT,
                    SID_ATTR_DOCINFO, SID_ATTR_AUTOSAVEMINUTE,
                    SID_SAVEREL_INET, SID_SAVEREL_FSYS,
                    SID_ATTR_PRETTYPRINTING, SID_ATTR_PRETTYPRINTING>{} );
            SfxGetpApp()->GetOptions(*pRet);
            break;

        case SID_SB_STARBASEOPTIONS:
            pRet = std::make_unique<SfxItemSet>( SfxGetpApp()->GetPool(),
            svl::Items<SID_SB_POOLING_ENABLED, SID_SB_DB_REGISTER>{} );
            ::offapp::ConnectionPoolConfig::GetOptions(*pRet);
            svx::DbRegisteredNamesConfig::GetOptions(*pRet);
            break;

        case SID_SCH_EDITOPTIONS:
        {
            SvxChartOptions aChartOpt;
            pRet = std::make_unique<SfxItemSet>( SfxGetpApp()->GetPool(), svl::Items<SID_SCH_EDITOPTIONS, SID_SCH_EDITOPTIONS>{} );
            pRet->Put( SvxChartColorTableItem( SID_SCH_EDITOPTIONS, aChartOpt.GetDefaultColors() ) );
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
            utl::MiscCfg    aMisc;
            const SfxPoolItem* pItem = nullptr;
            SfxItemSet aOptSet(SfxGetpApp()->GetPool(), svl::Items<SID_ATTR_QUICKLAUNCHER, SID_ATTR_QUICKLAUNCHER>{} );
            aOptSet.Put(rSet);
            if(aOptSet.Count())
                SfxGetpApp()->SetOptions( aOptSet );
            // get dispatcher anew, because SetOptions() might have destroyed the dispatcher
            SfxViewFrame *pViewFrame = SfxViewFrame::Current();

//          evaluate Year2000

            sal_uInt16 nY2K = USHRT_MAX;
            if( SfxItemState::SET == rSet.GetItemState( SID_ATTR_YEAR2000, false, &pItem ) )
                nY2K = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
            if( USHRT_MAX != nY2K )
            {
                if ( pViewFrame )
                {
                    SfxDispatcher* pDispatch = pViewFrame->GetDispatcher();
                    pDispatch->ExecuteList(SID_ATTR_YEAR2000,
                            SfxCallMode::ASYNCHRON, { pItem });
                }
                aMisc.SetYear2000(nY2K);
            }


//          evaluate print

            if(SfxItemState::SET == rSet.GetItemState(SID_PRINTER_NOTFOUND_WARN, false, &pItem))
                aMisc.SetNotFoundWarning(static_cast<const SfxBoolItem*>(pItem)->GetValue());

            if(SfxItemState::SET == rSet.GetItemState(SID_PRINTER_CHANGESTODOC, false, &pItem))
            {
                const SfxFlagItem* pFlag = static_cast<const SfxFlagItem*>(pItem);
                aMisc.SetPaperSizeWarning(bool(static_cast<SfxPrinterChangeFlags>(pFlag->GetValue()) &  SfxPrinterChangeFlags::CHG_SIZE ));
                aMisc.SetPaperOrientationWarning(bool(static_cast<SfxPrinterChangeFlags>(pFlag->GetValue()) & SfxPrinterChangeFlags::CHG_ORIENTATION ));
            }

//          evaluate help options

            if ( SvtHelpOptions().IsHelpTips() != Help::IsQuickHelpEnabled() )
                SvtHelpOptions().IsHelpTips() ? Help::EnableQuickHelp() : Help::DisableQuickHelp();
            if ( SvtHelpOptions().IsExtendedHelp() != Help::IsBalloonHelpEnabled() )
                SvtHelpOptions().IsExtendedHelp() ? Help::EnableBalloonHelp() : Help::DisableBalloonHelp();
        }
        break;
        case SID_LANGUAGE_OPTIONS :
        {
            OfaTreeOptionsDialog::ApplyLanguageOptions(rSet);
        }
        break;
        case SID_INET_DLG :
        case SID_FILTER_DLG:
            SfxGetpApp()->SetOptions( rSet );
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
            OSL_FAIL( "Unhandled option in ApplyItemSet" );
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
    if ( SfxItemState::SET == rSet.GetItemState(SID_ATTR_HYPHENREGION, false, &pItem ) )
    {
        const SfxHyphenRegionItem* pHyphenItem = static_cast<const SfxHyphenRegionItem*>(pItem);

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
            pDispatch->ExecuteList(pItem->Which(), SfxCallMode::ASYNCHRON, { pItem });
            bSaveSpellCheck = true;
        }
        if(SfxItemState::SET == rSet.GetItemState( SID_ATTR_CHAR_CTL_LANGUAGE, false, &pItem ))
        {
            pDispatch->ExecuteList(pItem->Which(), SfxCallMode::ASYNCHRON, { pItem });
            bSaveSpellCheck = true;
        }
        if(SfxItemState::SET == rSet.GetItemState( SID_ATTR_CHAR_CJK_LANGUAGE, false, &pItem ))
        {
            pDispatch->ExecuteList(pItem->Which(), SfxCallMode::ASYNCHRON, { pItem });
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

static OUString getCurrentFactory_Impl( const Reference< XFrame >& _xFrame )
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
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN( "cui.options", "getActiveModule_Impl(): exception of XModuleManager::identify() " << exceptionToString(ex) );
        }
    }

    return sIdentifier;
}

void OfaTreeOptionsDialog::Initialize( const Reference< XFrame >& _xFrame )
{
    sal_uInt16 nGroup = 0;

    SvtOptionsDialogOptions aOptionsDlgOpt;
    sal_uInt16 nPageId;

    // %PRODUCTNAME options
    if ( !lcl_isOptionHidden( SID_GENERAL_OPTIONS, aOptionsDlgOpt ) )
    {
        setGroupName("ProductName", CuiResId(SID_GENERAL_OPTIONS_RES[0].first));
        nGroup = AddGroup(CuiResId(SID_GENERAL_OPTIONS_RES[0].first), nullptr, nullptr, SID_GENERAL_OPTIONS );
        const sal_uInt16 nEnd = static_cast<sal_uInt16>(SAL_N_ELEMENTS(SID_GENERAL_OPTIONS_RES));

        for (sal_uInt16 i = 1; i < nEnd; ++i)
        {
            OUString sNewTitle = CuiResId(SID_GENERAL_OPTIONS_RES[i].first);
            nPageId = SID_GENERAL_OPTIONS_RES[i].second;
            if ( lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                continue;

            // Disable Online Update page if service not installed
            if( RID_SVXPAGE_ONLINEUPDATE == nPageId )
            {
                try
                {
                    Reference < XInterface > xService( setup::UpdateCheck::create( ::comphelper::getProcessComponentContext() ) );
                    if( ! xService.is() )
                        continue;
                }
                catch ( css::uno::DeploymentException& )
                {
                    continue;
                }
            }

            // Disable Basic IDE options, if experimental features are not enabled
            if( RID_SVXPAGE_BASICIDE_OPTIONS == nPageId )
            {
                    SvtMiscOptions aMiscOpt;
                    if( ! aMiscOpt.IsExperimentalMode() )
                        continue;
            }

            AddTabPage( nPageId, sNewTitle, nGroup );
        }
    }

    // Load and Save options
    if ( !lcl_isOptionHidden( SID_FILTER_DLG, aOptionsDlgOpt ) )
    {
        setGroupName( "LoadSave", CuiResId(SID_FILTER_DLG_RES[0].first) );
        nGroup = AddGroup( CuiResId(SID_FILTER_DLG_RES[0].first), nullptr, nullptr, SID_FILTER_DLG );
        for ( size_t i = 1; i < SAL_N_ELEMENTS(SID_FILTER_DLG_RES); ++i )
        {
            nPageId = static_cast<sal_uInt16>(SID_FILTER_DLG_RES[i].second);
            if ( !lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                AddTabPage( nPageId, CuiResId(SID_FILTER_DLG_RES[i].first), nGroup );
        }
    }

    // Language options
    SvtLanguageOptions aLanguageOptions;
    if ( !lcl_isOptionHidden( SID_LANGUAGE_OPTIONS, aOptionsDlgOpt ) )
    {
        setGroupName("LanguageSettings", CuiResId(SID_LANGUAGE_OPTIONS_RES[0].first));
        nGroup = AddGroup(CuiResId(SID_LANGUAGE_OPTIONS_RES[0].first), nullptr, nullptr, SID_LANGUAGE_OPTIONS );
        for (size_t i = 1; i < SAL_N_ELEMENTS(SID_LANGUAGE_OPTIONS_RES); ++i)
        {
            nPageId = static_cast<sal_uInt16>(SID_LANGUAGE_OPTIONS_RES[i].second);
            if ( lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                continue;
            if ( ( RID_SVXPAGE_JSEARCH_OPTIONS != nPageId || aLanguageOptions.IsJapaneseFindEnabled() ) &&
                 ( RID_SVXPAGE_ASIAN_LAYOUT != nPageId    || aLanguageOptions.IsAsianTypographyEnabled() ) &&
                 ( RID_SVXPAGE_OPTIONS_CTL != nPageId     || aLanguageOptions.IsCTLFontEnabled() ) )
                AddTabPage(nPageId, CuiResId(SID_LANGUAGE_OPTIONS_RES[i].first), nGroup);
        }
    }

    OUString aFactory = getCurrentFactory_Impl( _xFrame );
    DBG_ASSERT( GetModuleIdentifier( _xFrame ) == aFactory, "S H I T!!!" );

    // Writer and Writer/Web options
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
                    setGroupName( "WriterWeb", CuiResId(SID_SW_EDITOPTIONS_RES[0].first) );
                else
                    setGroupName( "Writer", CuiResId(SID_SW_EDITOPTIONS_RES[0].first) );
                nGroup = AddGroup(CuiResId(SID_SW_EDITOPTIONS_RES[0].first), pSwMod, pSwMod, SID_SW_EDITOPTIONS );
                for ( size_t i = 1; i < SAL_N_ELEMENTS(SID_SW_EDITOPTIONS_RES); ++i )
                {
                    nPageId = static_cast<sal_uInt16>(SID_SW_EDITOPTIONS_RES[i].second);
                    if ( lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                        continue;
                    if ( ( RID_SW_TP_STD_FONT_CJK != nPageId || aLanguageOptions.IsCJKFontEnabled() ) &&
                         ( RID_SW_TP_STD_FONT_CTL != nPageId || aLanguageOptions.IsCTLFontEnabled() ) &&
                         ( RID_SW_TP_MAILCONFIG != nPageId || MailMergeCfg_Impl().IsEmailSupported() ) )
                        AddTabPage( nPageId, CuiResId(SID_SW_EDITOPTIONS_RES[i].first), nGroup );
                }
#ifdef DBG_UTIL
                AddTabPage( RID_SW_TP_OPTTEST_PAGE, "Internal Test", nGroup );
#endif
            }

            // HTML documents
            if ( !lcl_isOptionHidden( SID_SW_ONLINEOPTIONS, aOptionsDlgOpt ) )
            {
                nGroup = AddGroup(CuiResId(SID_SW_ONLINEOPTIONS_RES[0].first), pSwMod, pSwMod, SID_SW_ONLINEOPTIONS );
                for( size_t i = 1; i < SAL_N_ELEMENTS(SID_SW_ONLINEOPTIONS_RES); ++i )
                {
                    nPageId = static_cast<sal_uInt16>(SID_SW_ONLINEOPTIONS_RES[i].second);
                    if ( !lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                        AddTabPage(nPageId, CuiResId(SID_SW_ONLINEOPTIONS_RES[i].first), nGroup);
                }
#ifdef DBG_UTIL
                AddTabPage( RID_SW_TP_OPTTEST_PAGE, "Internal Test", nGroup );
#endif
            }
        }
    }

    // Calc options
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::CALC ) )
    {
        if ( aFactory == "com.sun.star.sheet.SpreadsheetDocument" )
        {
            if ( !lcl_isOptionHidden( SID_SC_EDITOPTIONS, aOptionsDlgOpt ) )
            {
                SfxModule* pScMod = SfxApplication::GetModule( SfxToolsModule::Calc );
                setGroupName( "Calc", CuiResId(SID_SC_EDITOPTIONS_RES[0].first) );
                nGroup = AddGroup( CuiResId(SID_SC_EDITOPTIONS_RES[0].first), pScMod, pScMod, SID_SC_EDITOPTIONS );
                const sal_uInt16 nCount = static_cast<sal_uInt16>(SAL_N_ELEMENTS(SID_SC_EDITOPTIONS_RES));
                for ( sal_uInt16 i = 1; i < nCount; ++i )
                {
                    nPageId = static_cast<sal_uInt16>(SID_SC_EDITOPTIONS_RES[i].second);
                    if ( lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                        continue;

                    AddTabPage( nPageId, CuiResId(SID_SC_EDITOPTIONS_RES[i].first), nGroup );
                }
            }
        }
    }

    // Impress options
    SfxModule* pSdMod = SfxApplication::GetModule( SfxToolsModule::Draw );
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::IMPRESS ) )
    {
        if ( aFactory == "com.sun.star.presentation.PresentationDocument" )
        {
            if ( !lcl_isOptionHidden( SID_SD_EDITOPTIONS, aOptionsDlgOpt ) )
            {
                setGroupName( "Impress", CuiResId(SID_SD_EDITOPTIONS_RES[0].first) );
                nGroup = AddGroup( CuiResId(SID_SD_EDITOPTIONS_RES[0].first), pSdMod, pSdMod, SID_SD_EDITOPTIONS );
                const sal_uInt16 nCount = static_cast<sal_uInt16>(SAL_N_ELEMENTS(SID_SD_EDITOPTIONS_RES));
                for ( sal_uInt16 i = 1; i < nCount; ++i )
                {
                    nPageId = static_cast<sal_uInt16>(SID_SD_EDITOPTIONS_RES[i].second);
                    if ( lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                        continue;

                    AddTabPage( nPageId, CuiResId(SID_SD_EDITOPTIONS_RES[i].first), nGroup );
                }
            }
        }
    }

    // Draw options
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::DRAW ) )
    {
        if ( aFactory == "com.sun.star.drawing.DrawingDocument" )
        {
            if ( !lcl_isOptionHidden( SID_SD_GRAPHIC_OPTIONS, aOptionsDlgOpt ) )
            {
                setGroupName( "Draw", CuiResId(SID_SD_GRAPHIC_OPTIONS_RES[0].first) );
                nGroup = AddGroup( CuiResId(SID_SD_GRAPHIC_OPTIONS_RES[0].first), pSdMod, pSdMod, SID_SD_GRAPHIC_OPTIONS );
                const sal_uInt16 nCount = static_cast<sal_uInt16>(SAL_N_ELEMENTS(SID_SD_GRAPHIC_OPTIONS_RES));
                for ( sal_uInt16 i = 1; i < nCount; ++i )
                {
                    nPageId = static_cast<sal_uInt16>(SID_SD_GRAPHIC_OPTIONS_RES[i].second);
                    if ( lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                        continue;

                    AddTabPage( nPageId, CuiResId(SID_SD_GRAPHIC_OPTIONS_RES[i].first), nGroup );
                }
            }
        }
    }

    // Math options
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::MATH ) )
    {
        if ( aFactory == "com.sun.star.formula.FormulaProperties" )
        {
            if ( !lcl_isOptionHidden( SID_SM_EDITOPTIONS, aOptionsDlgOpt ) )
            {
                SfxModule* pSmMod = SfxApplication::GetModule(SfxToolsModule::Math);
                setGroupName( "Math", CuiResId(SID_SM_EDITOPTIONS_RES[0].first) );
                nGroup = AddGroup(CuiResId(SID_SM_EDITOPTIONS_RES[0].first), pSmMod, pSmMod, SID_SM_EDITOPTIONS );
                for ( size_t i = 1; i < SAL_N_ELEMENTS(SID_SM_EDITOPTIONS_RES); ++i )
                {
                    nPageId = static_cast<sal_uInt16>(SID_SM_EDITOPTIONS_RES[i].second);
                    if ( !lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                        AddTabPage( nPageId, CuiResId(SID_SM_EDITOPTIONS_RES[i].first), nGroup );
                }
            }
        }
    }

    // Database - needed only if there is an application which integrates with databases
    if ( !lcl_isOptionHidden( SID_SB_STARBASEOPTIONS, aOptionsDlgOpt ) &&
        (   aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::DATABASE )
        ||  aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::WRITER )
        ||  aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::CALC )
        ) )
    {
        setGroupName( "Base", CuiResId(SID_SB_STARBASEOPTIONS_RES[0].first) );
        nGroup = AddGroup( CuiResId(SID_SB_STARBASEOPTIONS_RES[0].first), nullptr, nullptr, SID_SB_STARBASEOPTIONS );
        for ( size_t i = 1; i < SAL_N_ELEMENTS(SID_SB_STARBASEOPTIONS_RES); ++i )
        {
            nPageId = static_cast<sal_uInt16>(SID_SB_STARBASEOPTIONS_RES[i].second);
            if ( !lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                AddTabPage( nPageId, CuiResId(SID_SB_STARBASEOPTIONS_RES[i].first), nGroup );
        }
    }

    // Chart options (always installed and active)
    if ( !lcl_isOptionHidden( SID_SCH_EDITOPTIONS, aOptionsDlgOpt ) )
    {
        setGroupName( "Charts", CuiResId(SID_SCH_EDITOPTIONS_RES[0].first) );
        nGroup = AddGroup( CuiResId(SID_SCH_EDITOPTIONS_RES[0].first), nullptr, nullptr, SID_SCH_EDITOPTIONS );
        for ( size_t i = 1; i < SAL_N_ELEMENTS(SID_SCH_EDITOPTIONS_RES); ++i )
        {
            nPageId = static_cast<sal_uInt16>(SID_SCH_EDITOPTIONS_RES[i].second);
            if ( !lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
               AddTabPage( nPageId, CuiResId(SID_SCH_EDITOPTIONS_RES[i].first), nGroup );
        }
    }

    // Internet options
    if ( !lcl_isOptionHidden( SID_INET_DLG, aOptionsDlgOpt ) )
    {
        setGroupName("Internet", CuiResId(SID_INET_DLG_RES[0].first));
        nGroup = AddGroup(CuiResId(SID_INET_DLG_RES[0].first), nullptr, nullptr, SID_INET_DLG );

        for ( size_t i = 1; i < SAL_N_ELEMENTS(SID_INET_DLG_RES); ++i )
        {
            nPageId = static_cast<sal_uInt16>(SID_INET_DLG_RES[i].second);
            if ( lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                continue;
#if defined(_WIN32)
            // Disable E-mail tab-page on Windows
            if ( nPageId == RID_SVXPAGE_INET_MAIL )
                continue;
#endif
            AddTabPage( nPageId, CuiResId(SID_INET_DLG_RES[i].first), nGroup );
        }
    }
}

static bool isNodeActive( OptionsNode const * pNode, Module* pModule )
{
    if ( pNode )
    {
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
    }
    return false;
}

void OfaTreeOptionsDialog::LoadExtensionOptions( const OUString& rExtensionId )
{
    std::unique_ptr<Module> pModule;

    // when called by Tools - Options then load nodes of active module
    if ( rExtensionId.isEmpty() )
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
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN( "cui.options", "OfaTreeOptionsDialog::GetModuleIdentifier(): exception of XModuleManager::identify() " << exceptionToString(ex));
        }
    }
    return sModule;
}

std::unique_ptr<Module> OfaTreeOptionsDialog::LoadModule(
    const OUString& rModuleIdentifier )
{
    std::unique_ptr<Module> pModule;
    Reference< XNameAccess > xSet(
        officecfg::Office::OptionsDialog::Modules::get());

    Sequence< OUString > seqNames = xSet->getElementNames();
    for ( int i = 0; i < seqNames.getLength(); ++i )
    {
        OUString sModule( seqNames[i] );
        if ( rModuleIdentifier == sModule )
        {
            // current active module found
            pModule.reset(new Module);
            pModule->m_bActive = true;

            Reference< XNameAccess > xModAccess;
            xSet->getByName( seqNames[i] ) >>= xModAccess;
            if ( xModAccess.is() )
            {
                // load the nodes of this module
                Reference< XNameAccess > xNodeAccess;
                xModAccess->getByName( "Nodes" ) >>= xNodeAccess;
                if ( xNodeAccess.is() )
                {
                    Sequence< OUString > xTemp = xNodeAccess->getElementNames();
                    Reference< XNameAccess > xAccess;
                    sal_Int32 nIndex = -1;
                    for ( int x = 0; x < xTemp.getLength(); ++x )
                    {
                        xNodeAccess->getByName( xTemp[x] ) >>= xAccess;
                        if ( xAccess.is() )
                        {
                            xAccess->getByName( "Index" ) >>= nIndex;
                            if ( nIndex < 0 )
                                // append nodes with index < 0
                                pModule->m_aNodeList.push_back(
                                 std::unique_ptr<OrderedEntry>(new OrderedEntry(nIndex, xTemp[x])));
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
                                    std::unique_ptr<OrderedEntry>(new OrderedEntry( nIndex, xTemp[x] )) );
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
    Module* pModule, const OUString& rExtensionId)
{
    VectorOfNodes aOutNodeList;

    Reference< XNameAccess > xSet(
        officecfg::Office::OptionsDialog::Nodes::get());
    VectorOfNodes aNodeList;
    Sequence< OUString > seqNames = xSet->getElementNames();

    for ( int i = 0; i < seqNames.getLength(); ++i )
    {
        OUString sGroupName( seqNames[i] );
        Reference< XNameAccess > xNodeAccess;
        xSet->getByName( seqNames[i] ) >>= xNodeAccess;

        if ( xNodeAccess.is() )
        {
            OUString sNodeId, sLabel, sPageURL;
            bool bAllModules = false;

            sNodeId = seqNames[i];
            xNodeAccess->getByName( "Label" ) >>= sLabel;
            xNodeAccess->getByName( "OptionsPage" ) >>= sPageURL;
            xNodeAccess->getByName( "AllModules" ) >>= bAllModules;

            if ( sLabel.isEmpty() )
                sLabel = sGroupName;
            OUString sTemp = getGroupName( sLabel, !rExtensionId.isEmpty() );
            if ( !sTemp.isEmpty() )
                sLabel = sTemp;
            std::unique_ptr<OptionsNode> pNode(new OptionsNode(sNodeId, sLabel, bAllModules));

            if ( rExtensionId.isEmpty() && !isNodeActive( pNode.get(), pModule ) )
            {
                continue;
            }

            Reference< XNameAccess > xLeavesSet;
            xNodeAccess->getByName( "Leaves" ) >>= xLeavesSet;
            if ( xLeavesSet.is() )
            {
                Sequence< OUString > seqLeaves = xLeavesSet->getElementNames();
                for ( int j = 0; j < seqLeaves.getLength(); ++j )
                {
                    Reference< XNameAccess > xLeaveAccess;
                    xLeavesSet->getByName( seqLeaves[j] ) >>= xLeaveAccess;

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

                        if ( rExtensionId.isEmpty() || sId == rExtensionId )
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

static sal_uInt16 lcl_getGroupId( const OUString& rGroupName, const SvTreeListBox& rTreeLB )
{
    sal_uInt16 nRet = 0;
    SvTreeListEntry* pEntry = rTreeLB.First();
    while( pEntry )
    {
        if ( !rTreeLB.GetParent( pEntry ) )
        {
            OUString sTemp( rTreeLB.GetEntryText( pEntry ) );
            if ( sTemp == rGroupName )
                return nRet;
            nRet++;
        }
        pEntry = rTreeLB.Next( pEntry );
    }

    return USHRT_MAX;
}

static void lcl_insertLeaf(
    OfaTreeOptionsDialog* pDlg, OptionsNode const * pNode, OptionsLeaf const * pLeaf, const SvTreeListBox& rTreeLB )
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
                    lcl_insertLeaf( this, node.get(), j[k].get(), *pTreeLB );
                }
            }

            for ( auto const & j: node->m_aLeaves )
            {
                lcl_insertLeaf( this, node.get(), j.get(), *pTreeLB );
            }
        }
    }
}

void OfaTreeOptionsDialog::SetNeedsRestart( svtools::RestartReason eReason)
{
    bNeedsRestart = true;
    eRestartReason = eReason;
}

short OfaTreeOptionsDialog::Execute()
{
    std::unique_ptr< SvxDicListChgClamp > pClamp;
    if ( !bIsFromExtensionManager )
    {
        // collect all DictionaryList Events while the dialog is executed
        Reference<css::linguistic2::XSearchableDictionaryList> xDictionaryList(LinguMgr::GetDictionaryList());
        pClamp.reset( new SvxDicListChgClamp( xDictionaryList ) );
    }
    short nRet = SfxModalDialog::Execute();

    if( RET_OK == nRet )
    {
        ApplyItemSets();
        utl::ConfigManager::storeConfigItems();
    }

    return nRet;
}

// class ExtensionsTabPage -----------------------------------------------
ExtensionsTabPage::ExtensionsTabPage(
    vcl::Window* pParent, WinBits nStyle, const OUString& rPageURL,
    const OUString& rEvtHdl, const Reference< awt::XContainerWindowProvider >& rProvider ) :

    TabPage( pParent, nStyle ),

    m_sPageURL          ( rPageURL ),
    m_sEventHdl         ( rEvtHdl ),
    m_xWinProvider      ( rProvider )
{
}

ExtensionsTabPage::~ExtensionsTabPage()
{
    disposeOnce();
}

void ExtensionsTabPage::dispose()
{
    Hide();
    DeactivatePage();

    if ( m_xPage.is() )
    {
        Reference< XComponent > xComponent( m_xPage, UNO_QUERY );
        if ( xComponent.is() )
        {
            try
            {
                xComponent->dispose();
            }
            catch ( const Exception & )
            {
            }
        }
        m_xPage.clear();
    }
    TabPage::dispose();
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
            SetStyle( GetStyle() | WB_DIALOGCONTROL | WB_CHILDDLGCTRL );
            Reference< awt::XWindowPeer > xParent( VCLUnoHelper::GetInterface( this ), UNO_QUERY );
            m_xPage.set(
                m_xWinProvider->createContainerWindow(
                    m_sPageURL, OUString(), xParent, m_xEventHdl ), UNO_QUERY );

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
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN( "cui.options", "ExtensionsTabPage::CreateDialogWithHandler(): exception of XDialogProvider2::createDialogWithHandler(): " << exceptionToString(ex));
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
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN( "cui.options", "ExtensionsTabPage::DispatchAction(): exception of XDialogEventHandler::callHandlerMethod() " << exceptionToString(ex) );
        }
    }
    return bRet;
}

void ExtensionsTabPage::ActivatePage()
{
    TabPage::ActivatePage();

    if ( !m_xPage.is() )
    {
        CreateDialogWithHandler();

        if ( m_xPage.is() )
        {
            Point aPos = Point();
            Size aSize = GetParent()->get_preferred_size();
            m_xPage->setPosSize( aPos.X() + 1, aPos.Y() + 1,
                                 aSize.Width() - 2, aSize.Height() - 2, awt::PosSize::POSSIZE );
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
    TabPage::DeactivatePage();

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
