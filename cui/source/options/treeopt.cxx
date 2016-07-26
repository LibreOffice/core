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

#include <config_features.h>

#include "cuires.hrc"
#include "helpid.hrc"

#include <svx/dialogs.hrc>

#include "cfgchart.hxx"
#include "connpoolconfig.hxx"
#include "connpooloptions.hxx"
#include "cuioptgenrl.hxx"
#include "cuitabarea.hxx"
#include "dbregister.hxx"
#include "dbregisterednamesconfig.hxx"
#include "dialmgr.hxx"
#include "fontsubs.hxx"
#include "optaboutconfig.hxx"
#include "optaccessibility.hxx"
#include "optasian.hxx"
#include "optchart.hxx"
#include "optcolor.hxx"
#include "optctl.hxx"
#include "optfltr.hxx"
#include "optgdlg.hxx"
#include "opthtml.hxx"
#include "optinet2.hxx"
#include "optjava.hxx"
#include "optjsearch.hxx"
#include "optlingu.hxx"
#include "optmemory.hxx"
#if HAVE_FEATURE_OPENCL
#include "optopencl.hxx"
#endif
#include "optpath.hxx"
#include "optsave.hxx"
#include "optupdt.hxx"
#include "personalization.hxx"
#include "treeopt.hxx"
#include "optbasic.hxx"

#include <com/sun/star/awt/XContainerWindowEventHandler.hpp>
#include <com/sun/star/awt/ContainerWindowProvider.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
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
#include <svx/drawitem.hxx>
#include <svx/xtable.hxx>
#include <svx/xpool.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/shl.hxx>
#include <tools/urlobj.hxx>
#include <unotools/linguprops.hxx>
#include <unotools/misccfg.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/optionsdlg.hxx>
#include <unotools/viewoptions.hxx>
#include <vcl/help.hxx>
#include <vcl/layout.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/waitobj.hxx>
#include <vcl/settings.hxx>
#include "svtools/treelistentry.hxx"

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
static const sal_Char*      pViewOptDataName = "page data";
#define VIEWOPT_DATANAME    OUString::createFromAscii( pViewOptDataName )

static XOutdevItemPool* mpStaticXOutdevItemPool = nullptr;

static inline void SetViewOptUserItem( SvtViewOptions& rOpt, const OUString& rData )
{
    rOpt.SetUserItem( VIEWOPT_DATANAME, makeAny( OUString( rData ) ) );
}

static inline OUString GetViewOptUserItem( const SvtViewOptions& rOpt )
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

static OfaPageResource* pPageRes = nullptr;

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
        if ( !pPageRes )
            pPageRes = new OfaPageResource;

        if ( rModule == "Writer" )
            sGroupName = pPageRes->GetTextArray().GetString(0);
        else if ( rModule == "WriterWeb" )
            sGroupName = pPageRes->GetHTMLArray().GetString(0);
        else if ( rModule == "Calc" )
            sGroupName = pPageRes->GetCalcArray().GetString(0);
        else if ( rModule == "Impress" )
            sGroupName = pPageRes->GetImpressArray().GetString(0);
        else if ( rModule == "Draw" )
            sGroupName = pPageRes->GetDrawArray().GetString(0);
        else if ( rModule == "Math" )
            sGroupName = pPageRes->GetStarMathArray().GetString(0);
        else if ( rModule == "Base" )
            sGroupName = pPageRes->GetDatasourcesArray().GetString(0);
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
    virtual ~MailMergeCfg_Impl();

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

MailMergeCfg_Impl::~MailMergeCfg_Impl()
{
}
/* -------------------------------------------------------------------------*/
void MailMergeCfg_Impl::ImplCommit()
{
}

void MailMergeCfg_Impl::Notify( const css::uno::Sequence< OUString >& )
{
}

//typedef SfxTabPage* (*FNCreateTabPage)( vcl::Window *pParent, const SfxItemSet &rAttrSet );
VclPtr<SfxTabPage> CreateGeneralTabPage( sal_uInt16 nId, vcl::Window* pParent, const SfxItemSet& rSet )
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
        case RID_SVXPAGE_COLOR:                     fnCreate = &SvxColorTabPage::Create; break;
        case OFA_TP_VIEW:                           fnCreate = &OfaViewTabPage::Create; break;
        case OFA_TP_MISC:                           fnCreate = &OfaMiscTabPage::Create; break;
        case OFA_TP_MEMORY:                         fnCreate = &OfaMemoryOptionsPage::Create; break;
        case RID_SVXPAGE_ASIAN_LAYOUT:              fnCreate = &SvxAsianLayoutPage::Create; break;
        case RID_SVX_FONT_SUBSTITUTION:             fnCreate = &SvxFontSubstTabPage::Create; break;
        case RID_SVXPAGE_INET_PROXY:                fnCreate = &SvxProxyTabPage::Create; break;
        case RID_SVXPAGE_INET_SECURITY:             fnCreate = &SvxSecurityTabPage::Create; break;
        case RID_SVXPAGE_INET_MAIL:                 fnCreate = &SvxEMailTabPage::Create; break;
        case RID_SVXPAGE_PERSONALIZATION:           fnCreate = &SvxPersonalizationTabPage::Create; break;
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
    { "ProductName",        nullptr,                   SID_GENERAL_OPTIONS },
    { "ProductName",        "UserData",             RID_SFXPAGE_GENERAL },
    { "ProductName",        "General",              OFA_TP_MISC },
    { "ProductName",        "Memory",               OFA_TP_MEMORY },
    { "ProductName",        "View",                 OFA_TP_VIEW },
    { "ProductName",        "Print",                RID_SFXPAGE_PRINTOPTIONS },
    { "ProductName",        "Paths",                RID_SFXPAGE_PATH },
    { "ProductName",        "Colors",               RID_SVXPAGE_COLOR },
    { "ProductName",        "Fonts",                RID_SVX_FONT_SUBSTITUTION },
    { "ProductName",        "Security",             RID_SVXPAGE_INET_SECURITY },
    { "ProductName",        "Personalization",      RID_SVXPAGE_PERSONALIZATION },
    { "ProductName",        "Appearance",           RID_SVXPAGE_COLORCONFIG },
    { "ProductName",        "Accessibility",        RID_SVXPAGE_ACCESSIBILITYCONFIG },
    { "ProductName",        "Java",                 RID_SVXPAGE_OPTIONS_JAVA },
    { "ProductName",        "BasicIDEOptions",      RID_SVXPAGE_BASICIDE_OPTIONS },
    { "ProductName",        "OnlineUpdate",         RID_SVXPAGE_ONLINEUPDATE },
    { "LanguageSettings",   nullptr,                   SID_LANGUAGE_OPTIONS },
    { "LanguageSettings",   "Languages",            OFA_TP_LANGUAGES  },
    { "LanguageSettings",   "WritingAids",          RID_SFXPAGE_LINGU },
    { "LanguageSettings",   "SearchingInJapanese",  RID_SVXPAGE_JSEARCH_OPTIONS },
    { "LanguageSettings",   "AsianLayout",          RID_SVXPAGE_ASIAN_LAYOUT },
    { "LanguageSettings",   "ComplexTextLayout",    RID_SVXPAGE_OPTIONS_CTL },
    { "Internet",           nullptr,                   SID_INET_DLG },
    { "Internet",           "Proxy",                RID_SVXPAGE_INET_PROXY },
    { "Internet",           "Email",                RID_SVXPAGE_INET_MAIL },
    { "LoadSave",           nullptr,                   SID_FILTER_DLG },
    { "LoadSave",           "General",              RID_SFXPAGE_SAVE },
    { "LoadSave",           "VBAProperties",        SID_OPTFILTER_MSOFFICE },
    { "LoadSave",           "MicrosoftOffice",      RID_OFAPAGE_MSFILTEROPT2 },
    { "LoadSave",           "HTMLCompatibility",    RID_OFAPAGE_HTMLOPT },
    { "Writer",             nullptr,                   SID_SW_EDITOPTIONS },
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
    { "WriterWeb",          nullptr,                   SID_SW_ONLINEOPTIONS },
    { "WriterWeb",          "View",                 RID_SW_TP_HTML_CONTENT_OPT },
    { "WriterWeb",          "FormattingAids",       RID_SW_TP_HTML_OPTSHDWCRSR },
    { "WriterWeb",          "Grid",                 RID_SW_TP_HTML_OPTGRID_PAGE },
    { "WriterWeb",          "Print",                RID_SW_TP_HTML_OPTPRINT_PAGE },
    { "WriterWeb",          "Table",                RID_SW_TP_HTML_OPTTABLE_PAGE },
    { "WriterWeb",          "Background",           RID_SW_TP_BACKGROUND },
    { "Math",               nullptr,                   SID_SM_EDITOPTIONS },
    { "Math",               "Settings",             SID_SM_TP_PRINTOPTIONS },
    { "Calc",               nullptr,                   SID_SC_EDITOPTIONS },
    { "Calc",               "General",              SID_SC_TP_LAYOUT },
    { "Calc",               "View",                 SID_SC_TP_CONTENT },
    { "Calc",               "Calculate",            SID_SC_TP_CALC },
    { "Calc",               "Formula",              SID_SC_TP_FORMULA },
    { "Calc",               "SortLists",            SID_SC_TP_USERLISTS },
    { "Calc",               "Changes",              SID_SC_TP_CHANGES },
    { "Calc",               "Compatibility",        SID_SC_TP_COMPATIBILITY },
    { "Calc",               "Grid",                 SID_SC_TP_GRID },
    { "Calc",               "Print",                RID_SC_TP_PRINT },
    { "Impress",            nullptr,                   SID_SD_EDITOPTIONS },
    { "Impress",            "General",              SID_SI_TP_MISC },
    { "Impress",            "View",                 SID_SI_TP_CONTENTS },
    { "Impress",            "Grid",                 SID_SI_TP_SNAP },
    { "Impress",            "Print",                SID_SI_TP_PRINT },
    { "Draw",               nullptr,                   SID_SD_GRAPHIC_OPTIONS },
    { "Draw",               "General",              SID_SD_TP_MISC },
    { "Draw",               "View",                 SID_SD_TP_CONTENTS },
    { "Draw",               "Grid",                 SID_SD_TP_SNAP },
    { "Draw",               "Print",                SID_SD_TP_PRINT },
    { "Charts",             nullptr,                   SID_SCH_EDITOPTIONS },
    { "Charts",             "DefaultColors",        RID_OPTPAGE_CHART_DEFCOLORS },
    { "Base",               nullptr,                   SID_SB_STARBASEOPTIONS },
    { "Base",               "Connections",          SID_SB_CONNECTIONPOOLING },
    { "Base",               "Databases",            SID_SB_DBREGISTEROPTIONS },
    { nullptr,                 nullptr,                   0 }
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
    SfxItemSet*         m_pInItemSet;
    SfxItemSet*         m_pOutItemSet;
    SfxShell*           m_pShell;       // used to create the page
    SfxModule*          m_pModule;      // used to create the ItemSet
    sal_uInt16          m_nDialogId;    // Id of the former dialog
    bool            m_bLoadError;   // load fails?
    OUString       m_sPageURL;
    VclPtr<ExtensionsTabPage>  m_pExtPage;

    OptionsGroupInfo( SfxShell* pSh, SfxModule* pMod, sal_uInt16 nId ) :
        m_pInItemSet( nullptr ), m_pOutItemSet( nullptr ), m_pShell( pSh ),
        m_pModule( pMod ), m_nDialogId( nId ), m_bLoadError( false ),
        m_sPageURL( OUString() ), m_pExtPage( nullptr ) {}
    ~OptionsGroupInfo() { delete m_pInItemSet; delete m_pOutItemSet; }
};

#define INI_LIST() \
    pCurrentPageEntry   ( nullptr ),\
    sTitle              ( GetText() ),\
    sNotLoadedError     (       CUI_RES( RID_SVXSTR_LOAD_ERROR ) ),\
    pColorPageItemSet   ( nullptr ),\
    mpColorPage         ( nullptr ),\
    bForgetSelection    ( false ),\
    bIsFromExtensionManager( false ), \
    bIsForSetDocumentLanguage( false )

void OfaTreeOptionsDialog::InitWidgets()
{
    get(pOkPB, "ok");
    get(pBackPB, "revert");
    get(pTreeLB, "pages");
    get(pTabBox, "box");
    Size aSize(pTabBox->LogicToPixel(Size(278, 259), MAP_APPFONT));
    pTabBox->set_width_request(aSize.Width());
    pTabBox->set_height_request(aSize.Height() - get_action_area()->get_preferred_size().Height());
    pTreeLB->set_width_request(pTreeLB->approximate_char_width() * 25);
    pTreeLB->set_height_request(pTabBox->get_height_request());
}

// Ctor() with Frame -----------------------------------------------------
using namespace ::com::sun::star;
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
                    SvtViewOptions aTabPageOpt( E_TABPAGE, OUString::number( pPageInfo->m_nPageId) );
                    SetViewOptUserItem( aTabPageOpt, aPageData );
                }
                pPageInfo->m_pPage.disposeAndClear();
            }

            if (pPageInfo->m_nPageId == RID_SFXPAGE_LINGU)
            {
                // write personal dictionaries
                Reference< XSearchableDictionaryList >  xDicList( SvxGetDictionaryList() );
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
            if ( pGroupInfo )
                pGroupInfo->m_pExtPage.disposeAndClear();
            delete pGroupInfo;
        }
        pEntry = pTreeLB->Next(pEntry);
    }
    delete pColorPageItemSet;
    pColorPageItemSet = nullptr;
    deleteGroupNames();
    pOkPB.clear();
    pBackPB.clear();
    pTreeLB.clear();
    pTabBox.clear();
    mpColorPage.clear();
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

IMPL_LINK_NOARG_TYPED(OfaTreeOptionsDialog, ShowPageHdl_Impl, SvTreeListBox*, void)
{
    SelectHdl_Impl();
}

IMPL_LINK_NOARG_TYPED(OfaTreeOptionsDialog, BackHdl_Impl, Button*, void)
{
    if ( pCurrentPageEntry && pTreeLB->GetParent( pCurrentPageEntry ) )
    {
        OptionsPageInfo* pPageInfo = static_cast<OptionsPageInfo*>(pCurrentPageEntry->GetUserData());
        if ( pPageInfo->m_pPage )
        {
            OptionsGroupInfo* pGroupInfo =
                static_cast<OptionsGroupInfo*>(pTreeLB->GetParent( pCurrentPageEntry )->GetUserData());
            if ( RID_SVXPAGE_COLOR == pPageInfo->m_nPageId )
                pPageInfo->m_pPage->Reset( pColorPageItemSet );
            else
                pPageInfo->m_pPage->Reset( pGroupInfo->m_pInItemSet );
        }
        else if ( pPageInfo->m_pExtPage )
            pPageInfo->m_pExtPage->ResetPage();
    }
}

IMPL_LINK_NOARG_TYPED(OfaTreeOptionsDialog, OKHdl_Impl, Button*, void)
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
                int nLeave = pPageInfo->m_pPage->DeactivatePage(pGroupInfo->m_pOutItemSet);
                if ( nLeave == SfxTabPage::KEEP_PAGE )
                {
                    // the page mustn't be left
                    pTreeLB->Select(pCurrentPageEntry);
                    return;
                }
            }
            pPageInfo->m_pPage->Hide();
        }
    }

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
                pPageInfo->m_pPage->FillItemSet(pGroupInfo->m_pOutItemSet);
            }

            if ( pPageInfo->m_pExtPage )
            {
                pPageInfo->m_pExtPage->DeactivatePage();
                pPageInfo->m_pExtPage->SavePage();
            }
        }
        pEntry = pTreeLB->Next(pEntry);
    }
    EndDialog(RET_OK);
}

// an opened group shall be completely visible
IMPL_STATIC_LINK_TYPED(
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
                pBox->ScrollOutputArea( -(short)(nChildCount - i + 1) );
                break;
            }
            else
            {
                Size aSz(pBox->GetOutputSizePixel());
                int nHeight = pBox->GetEntryHeight();
                Point aPos(pBox->GetEntryPosition(pNext));
                if(aPos.Y()+nHeight > aSz.Height())
                {
                    pBox->ScrollOutputArea( -(short)(nChildCount - i + 1) );
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
                           WB_CLIPCHILDREN | WB_HSCROLL | WB_FORCE_MAKEVISIBLE | WB_QUICK_SEARCH );
    pTreeLB->SetSpaceBetweenEntries( 0 );
    pTreeLB->SetSelectionMode( SINGLE_SELECTION );
    pTreeLB->SetSublistOpenWithLeftRight();
    pTreeLB->SetExpandedHdl( LINK( this, OfaTreeOptionsDialog, ExpandedHdl_Impl ) );
    pTreeLB->SetSelectHdl( LINK( this, OfaTreeOptionsDialog, ShowPageHdl_Impl ) );
    pBackPB->SetClickHdl( LINK( this, OfaTreeOptionsDialog, BackHdl_Impl ) );
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

bool OfaTreeOptionsDialog::Notify( NotifyEvent& rNEvt )
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
    return SfxModalDialog::Notify(rNEvt);
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
        int nLeave = SfxTabPage::LEAVE_PAGE;
        if ( RID_SVXPAGE_COLOR != pOptPageInfo->m_nPageId && pOptPageInfo->m_pPage->HasExchangeSupport() )
           nLeave = pOptPageInfo->m_pPage->DeactivatePage( pGroupInfo->m_pOutItemSet );

        if ( nLeave == SfxTabPage::KEEP_PAGE )
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
    else if ( pCurrentPageEntry && !pTreeLB->GetParent( pCurrentPageEntry ) )
    {
        OptionsGroupInfo* pGroupInfo = static_cast<OptionsGroupInfo*>(pCurrentPageEntry->GetUserData());
        if ( pGroupInfo && pGroupInfo->m_pExtPage )
        {
            pGroupInfo->m_pExtPage->Hide();
            pGroupInfo->m_pExtPage->DeactivatePage();
        }
    }

    OptionsPageInfo *pPageInfo = static_cast<OptionsPageInfo *>(pEntry->GetUserData());
    OptionsGroupInfo* pGroupInfo = static_cast<OptionsGroupInfo *>(pParent->GetUserData());
    if(!pPageInfo->m_pPage && pPageInfo->m_nPageId > 0)
    {
        if(pGroupInfo->m_bLoadError)
            return;
        if ( RID_SVXPAGE_COLOR == pPageInfo->m_nPageId )
        {
            if(!pColorPageItemSet)
            {
                // Move usage of a static XOutdevItemPool instance here
                if(!mpStaticXOutdevItemPool)
                    mpStaticXOutdevItemPool = new XOutdevItemPool();

                pColorPageItemSet = new SfxItemSet( *mpStaticXOutdevItemPool, XATTR_FILLSTYLE, XATTR_FILLCOLOR);
                pColorPageItemSet->Put( XFillColorItem() );
            }
        }
        else
        {
            if(pGroupInfo->m_pModule /*&& !pGroupInfo->pModule->IsLoaded()*/)
            {
                SfxModule* pOldModule = pGroupInfo->m_pModule;
                bool bIdentical = pGroupInfo->m_pModule == pGroupInfo->m_pShell;

                WaitObject aWait(this);
                //pGroupInfo->pModule = pGroupInfo->pModule->Load();
                if(!pGroupInfo->m_pModule)
                {
                    pGroupInfo->m_bLoadError = true;
                    ScopedVclPtrInstance<InfoBox>(pBox, sNotLoadedError)->Execute();
                    return;
                }
                if(bIdentical)
                    pGroupInfo->m_pShell = pGroupInfo->m_pModule;
                // now test whether there was the same module in other groups, too (e. g. Text+HTML)
                SvTreeListEntry* pTemp = pTreeLB->First();
                while(pTemp)
                {
                    if(!pTreeLB->GetParent(pTemp) && pTemp != pEntry)
                    {
                        OptionsGroupInfo* pTGInfo = static_cast<OptionsGroupInfo *>(pTemp->GetUserData());
                        if(pTGInfo->m_pModule == pOldModule)
                        {
                            pTGInfo->m_pModule = pGroupInfo->m_pModule;
                            if(bIdentical)
                                pTGInfo->m_pShell = pGroupInfo->m_pModule;
                        }
                    }
                    pTemp = pTreeLB->Next(pTemp);
                }
            }

            if(!pGroupInfo->m_pInItemSet)
                pGroupInfo->m_pInItemSet = pGroupInfo->m_pShell
                    ? pGroupInfo->m_pShell->CreateItemSet( pGroupInfo->m_nDialogId )
                    : CreateItemSet( pGroupInfo->m_nDialogId );
            if(!pGroupInfo->m_pOutItemSet)
                pGroupInfo->m_pOutItemSet = new SfxItemSet(
                    *pGroupInfo->m_pInItemSet->GetPool(),
                    pGroupInfo->m_pInItemSet->GetRanges());
        }

        if(pPageInfo->m_nPageId == RID_SVXPAGE_COLOR)
        {
            pPageInfo->m_pPage.disposeAndReset( ::CreateGeneralTabPage(
                pPageInfo->m_nPageId, pTabBox, *pColorPageItemSet ) );
            mpColorPage = static_cast<SvxColorTabPage*>(pPageInfo->m_pPage.get());
            mpColorPage->SetupForViewFrame( SfxViewFrame::Current() );
        }
        else
        {
            pPageInfo->m_pPage.disposeAndReset( ::CreateGeneralTabPage(pPageInfo->m_nPageId, pTabBox, *pGroupInfo->m_pInItemSet ) );

            if(!pPageInfo->m_pPage && pGroupInfo->m_pModule)
                pPageInfo->m_pPage.disposeAndReset( pGroupInfo->m_pModule->CreateTabPage(pPageInfo->m_nPageId, pTabBox, *pGroupInfo->m_pInItemSet) );

        }

        DBG_ASSERT( pPageInfo->m_pPage, "tabpage could not created");
        if ( pPageInfo->m_pPage )
        {
            SvtViewOptions aTabPageOpt( E_TABPAGE, OUString::number( pPageInfo->m_nPageId) );
            pPageInfo->m_pPage->SetUserData( GetViewOptUserItem( aTabPageOpt ) );

            if ( RID_SVXPAGE_COLOR == pPageInfo->m_nPageId )
            {
                pPageInfo->m_pPage->Reset( pColorPageItemSet );
                pPageInfo->m_pPage->ActivatePage( *pColorPageItemSet );
            }
            else
            {
                pPageInfo->m_pPage->Reset( pGroupInfo->m_pInItemSet );
            }
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
        OUStringBuffer sTitleBuf(sTitle);
        sTitleBuf.append(" - ");
        sTitleBuf.append(pTreeLB->GetEntryText(pParent));
        sTitleBuf.append(" - ");
        sTitleBuf.append(pTreeLB->GetEntryText(pEntry));
        SetText(sTitleBuf.makeStringAndClear());
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

    return;
}

OfaPageResource::OfaPageResource() :
    Resource(CUI_RES(RID_OFADLG_OPTIONS_TREE_PAGES)),
    aGeneralDlgAry(CUI_RES(SID_GENERAL_OPTIONS)),
    aInetDlgAry(CUI_RES(SID_INET_DLG)),
    aLangDlgAry(CUI_RES(SID_LANGUAGE_OPTIONS)),
    aTextDlgAry(CUI_RES(SID_SW_EDITOPTIONS)),
    aHTMLDlgAry(CUI_RES(SID_SW_ONLINEOPTIONS)),
    aCalcDlgAry(CUI_RES(SID_SC_EDITOPTIONS)),
    aStarMathDlgAry(CUI_RES(SID_SM_EDITOPTIONS)),
    aImpressDlgAry(CUI_RES(SID_SD_EDITOPTIONS)),
    aDrawDlgAry(CUI_RES(SID_SD_GRAPHIC_OPTIONS)),
    aChartDlgAry(CUI_RES(SID_SCH_EDITOPTIONS)),
    aFilterDlgAry(CUI_RES(SID_FILTER_DLG)),
    aDatasourcesDlgAry(CUI_RES(SID_SB_STARBASEOPTIONS))
{
    FreeResource();
}

SfxItemSet* OfaTreeOptionsDialog::CreateItemSet( sal_uInt16 nId )
{
    Reference< XLinguProperties >  xProp( SvxGetLinguPropertySet() );
    SfxItemSet* pRet = nullptr;
    switch(nId)
    {
        case SID_GENERAL_OPTIONS:
        {
            pRet = new SfxItemSet(
                SfxGetpApp()->GetPool(),
                SID_ATTR_METRIC, SID_ATTR_SPELL,
                SID_AUTOSPELL_CHECK, SID_AUTOSPELL_CHECK,
                SID_ATTR_QUICKLAUNCHER, SID_ATTR_QUICKLAUNCHER,
                SID_ATTR_YEAR2000, SID_ATTR_YEAR2000,
                SID_HTML_MODE, SID_HTML_MODE,
                0 );

            SfxItemSet aOptSet( SfxGetpApp()->GetPool(), SID_ATTR_QUICKLAUNCHER, SID_ATTR_QUICKLAUNCHER );
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
                    pRet->Put( SfxUInt16Item( SID_ATTR_YEAR2000, (sal_uInt16)aMisc.GetYear2000() ) );
            }
            else
                pRet->Put( SfxUInt16Item( SID_ATTR_YEAR2000, (sal_uInt16)aMisc.GetYear2000() ) );


            // miscellaneous - Tabulator
            pRet->Put(SfxBoolItem(SID_PRINTER_NOTFOUND_WARN, aMisc.IsNotFoundWarning()));

            SfxPrinterChangeFlags nFlag = aMisc.IsPaperSizeWarning() ? SfxPrinterChangeFlags::CHG_SIZE : SfxPrinterChangeFlags::NONE;
            nFlag  |= aMisc.IsPaperOrientationWarning()  ? SfxPrinterChangeFlags::CHG_ORIENTATION : SfxPrinterChangeFlags::NONE;
            pRet->Put( SfxFlagItem( SID_PRINTER_CHANGESTODOC, static_cast<int>(nFlag) ));

        }
        break;
        case SID_LANGUAGE_OPTIONS :
        {
            pRet = new SfxItemSet(SfxGetpApp()->GetPool(),
                    SID_ATTR_LANGUAGE, SID_AUTOSPELL_CHECK,
                    SID_ATTR_CHAR_CJK_LANGUAGE, SID_ATTR_CHAR_CTL_LANGUAGE,
                    SID_OPT_LOCALE_CHANGED, SID_OPT_LOCALE_CHANGED,
                    SID_SET_DOCUMENT_LANGUAGE, SID_SET_DOCUMENT_LANGUAGE,
                    0 );

            // for linguistic

            Reference< XSpellChecker1 >  xSpell = SvxGetSpellChecker();
            pRet->Put(SfxSpellCheckItem( xSpell, SID_ATTR_SPELL ));
            SfxHyphenRegionItem aHyphen( SID_ATTR_HYPHENREGION );

            sal_Int16   nMinLead  = 2,
                        nMinTrail = 2;
            if (xProp.is())
            {
                nMinLead = xProp->getHyphMinLeading();
                nMinTrail = xProp->getHyphMinTrailing();
            }
            aHyphen.GetMinLead()  = (sal_uInt8)nMinLead;
            aHyphen.GetMinTrail() = (sal_uInt8)nMinTrail;

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
                    SfxPoolItem* pClone = pItem->Clone();
                    pRet->Put(*pClone);
                    delete pClone;
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
                pRet = new SfxItemSet( SfxGetpApp()->GetPool(),
                                SID_BASIC_ENABLED, SID_BASIC_ENABLED,
                //SID_OPTIONS_START - ..END
                                SID_INET_PROXY_TYPE, SID_INET_PROXY_PORT,
                                SID_SAVEREL_INET, SID_SAVEREL_FSYS,
                                SID_INET_SMTPSERVER, SID_INET_SMTPSERVER,
                                SID_INET_NOPROXY, SID_INET_SOCKS_PROXY_PORT,
                                SID_INET_DNS_AUTO, SID_INET_DNS_SERVER,
                                SID_SECURE_URL, SID_SECURE_URL,
                                0L );
                SfxGetpApp()->GetOptions(*pRet);
        break;
        case SID_FILTER_DLG:
            pRet = new SfxItemSet( SfxGetpApp()->GetPool(),
            SID_ATTR_DOCINFO, SID_ATTR_AUTOSAVEMINUTE,
            SID_SAVEREL_INET, SID_SAVEREL_FSYS,
            SID_ATTR_PRETTYPRINTING, SID_ATTR_PRETTYPRINTING,
            SID_ATTR_WARNALIENFORMAT, SID_ATTR_WARNALIENFORMAT,
            0 );
            SfxGetpApp()->GetOptions(*pRet);
            break;

        case SID_SB_STARBASEOPTIONS:
            pRet = new SfxItemSet( SfxGetpApp()->GetPool(),
            SID_SB_POOLING_ENABLED, SID_SB_DB_REGISTER,
            0 );
            ::offapp::ConnectionPoolConfig::GetOptions(*pRet);
            svx::DbRegisteredNamesConfig::GetOptions(*pRet);
            break;

        case SID_SCH_EDITOPTIONS:
        {
            SvxChartOptions aChartOpt;
            pRet = new SfxItemSet( SfxGetpApp()->GetPool(), SID_SCH_EDITOPTIONS, SID_SCH_EDITOPTIONS );
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
            SfxItemSet aOptSet(SfxGetpApp()->GetPool(), SID_ATTR_QUICKLAUNCHER, SID_ATTR_QUICKLAUNCHER );
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

    if ( SfxItemState::SET == rSet.GetItemState( SID_SPELL_MODIFIED, false, &pItem ) )
    {
        bSaveSpellCheck = static_cast<const SfxBoolItem*>(pItem)->GetValue();
    }
    Reference< XComponentContext >  xContext( ::comphelper::getProcessComponentContext() );
    Reference< XLinguProperties >  xProp = LinguProperties::create( xContext );
    if ( SfxItemState::SET == rSet.GetItemState(SID_ATTR_HYPHENREGION, false, &pItem ) )
    {
        const SfxHyphenRegionItem* pHyphenItem = static_cast<const SfxHyphenRegionItem*>(pItem);

        xProp->setHyphMinLeading( (sal_Int16) pHyphenItem->GetMinLead() );
        xProp->setHyphMinTrailing( (sal_Int16) pHyphenItem->GetMinTrail() );
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

OUString getCurrentFactory_Impl( const Reference< XFrame >& _xFrame )
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
        catch ( Exception& )
        {
            SAL_WARN( "cui.options", "getActiveModule_Impl(): exception of XModuleManager::identify()" );
        }
    }

    return sIdentifier;
}

void OfaTreeOptionsDialog::Initialize( const Reference< XFrame >& _xFrame )
{
    OfaPageResource aDlgResource;
    sal_uInt16 nGroup = 0;

    SvtOptionsDialogOptions aOptionsDlgOpt;
    sal_uInt16 i, nPageId;

    // %PRODUCTNAME options
    if ( !lcl_isOptionHidden( SID_GENERAL_OPTIONS, aOptionsDlgOpt ) )
    {
        ResStringArray& rGeneralArray = aDlgResource.GetGeneralArray();
        setGroupName( "ProductName", rGeneralArray.GetString(0) );
        nGroup = AddGroup( rGeneralArray.GetString(0), nullptr, nullptr, SID_GENERAL_OPTIONS );
        sal_uInt16 nEnd = static_cast< sal_uInt16 >( rGeneralArray.Count() );

        for ( i = 1; i < nEnd; ++i )
        {
            OUString sNewTitle = rGeneralArray.GetString(i);
            nPageId = (sal_uInt16)rGeneralArray.GetValue(i);
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
            AddTabPage( nPageId, sNewTitle, nGroup );
        }
    }

    // Load and Save options
    if ( !lcl_isOptionHidden( SID_FILTER_DLG, aOptionsDlgOpt ) )
    {
        ResStringArray& rFilterArray = aDlgResource.GetFilterArray();
        setGroupName( "LoadSave", rFilterArray.GetString(0) );
        nGroup = AddGroup( rFilterArray.GetString(0), nullptr, nullptr, SID_FILTER_DLG );
        for ( i = 1; i < rFilterArray.Count(); ++i )
        {
            nPageId = (sal_uInt16)rFilterArray.GetValue(i);
            if ( !lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                AddTabPage( nPageId, rFilterArray.GetString(i), nGroup );
        }
    }

    // Language options
    SvtLanguageOptions aLanguageOptions;
    if ( !lcl_isOptionHidden( SID_LANGUAGE_OPTIONS, aOptionsDlgOpt ) )
    {
        ResStringArray& rLangArray = aDlgResource.GetLangArray();
        setGroupName( "LanguageSettings", rLangArray.GetString(0) );
        nGroup = AddGroup( rLangArray.GetString(0), nullptr, nullptr, SID_LANGUAGE_OPTIONS );
        for ( i = 1; i < rLangArray.Count(); ++i )
        {
            nPageId = (sal_uInt16)rLangArray.GetValue(i);
            if ( lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                continue;
            if ( ( RID_SVXPAGE_JSEARCH_OPTIONS != nPageId || aLanguageOptions.IsJapaneseFindEnabled() ) &&
                 ( RID_SVXPAGE_ASIAN_LAYOUT != nPageId    || aLanguageOptions.IsAsianTypographyEnabled() ) &&
                 ( RID_SVXPAGE_OPTIONS_CTL != nPageId     || aLanguageOptions.IsCTLFontEnabled() ) )
                AddTabPage( nPageId, rLangArray.GetString(i), nGroup );
        }
    }


    OUString aFactory = getCurrentFactory_Impl( _xFrame );
    DBG_ASSERT( GetModuleIdentifier( _xFrame ) == aFactory, "S H I T!!!" );

    // Writer and Writer/Web options
    SvtModuleOptions aModuleOpt;
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::WRITER ) )
    {
        // text document
        ResStringArray& rTextArray = aDlgResource.GetTextArray();
        if (   aFactory == "com.sun.star.text.TextDocument"
            || aFactory == "com.sun.star.text.WebDocument"
            || aFactory == "com.sun.star.text.GlobalDocument" )
        {
            SfxModule* pSwMod = *reinterpret_cast<SfxModule**>(GetAppData(SHL_WRITER));
            if ( !lcl_isOptionHidden( SID_SW_EDITOPTIONS, aOptionsDlgOpt ) )
            {
                if ( aFactory == "com.sun.star.text.WebDocument" )
                    setGroupName( "WriterWeb", rTextArray.GetString(0) );
                else
                    setGroupName( "Writer", rTextArray.GetString(0) );
                nGroup = AddGroup(rTextArray.GetString(0), pSwMod, pSwMod, SID_SW_EDITOPTIONS );
                for ( i = 1; i < rTextArray.Count(); ++i )
                {
                    nPageId = (sal_uInt16)rTextArray.GetValue(i);
                    if ( lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                        continue;
                    if ( ( RID_SW_TP_STD_FONT_CJK != nPageId || aLanguageOptions.IsCJKFontEnabled() ) &&
                         ( RID_SW_TP_STD_FONT_CTL != nPageId || aLanguageOptions.IsCTLFontEnabled() ) &&
                         ( RID_SW_TP_MAILCONFIG != nPageId || MailMergeCfg_Impl().IsEmailSupported() ) )
                        AddTabPage( nPageId, rTextArray.GetString(i), nGroup );
                }
#ifdef DBG_UTIL
                AddTabPage( RID_SW_TP_OPTTEST_PAGE, "Internal Test", nGroup );
#endif
            }

            // HTML documents
            if ( !lcl_isOptionHidden( SID_SW_ONLINEOPTIONS, aOptionsDlgOpt ) )
            {
                ResStringArray& rHTMLArray = aDlgResource.GetHTMLArray();
                nGroup = AddGroup(rHTMLArray.GetString(0), pSwMod, pSwMod, SID_SW_ONLINEOPTIONS );
                for( i = 1; i < rHTMLArray.Count(); ++i )
                {
                    nPageId = (sal_uInt16)rHTMLArray.GetValue(i);
                    if ( !lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                        AddTabPage( nPageId, rHTMLArray.GetString(i), nGroup );
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
                ResStringArray& rCalcArray = aDlgResource.GetCalcArray();
                SfxModule* pScMod = *reinterpret_cast<SfxModule**>(GetAppData( SHL_CALC ));
                setGroupName( "Calc", rCalcArray.GetString(0) );
                nGroup = AddGroup( rCalcArray.GetString( 0 ), pScMod, pScMod, SID_SC_EDITOPTIONS );
                const sal_uInt16 nCount = static_cast< const sal_uInt16 >( rCalcArray.Count() );
                for ( i = 1; i < nCount; ++i )
                {
                    nPageId = (sal_uInt16)rCalcArray.GetValue(i);
                    if ( lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                        continue;

                    AddTabPage( nPageId, rCalcArray.GetString( i ), nGroup );
                }
            }
        }
    }

    // Impress options
    SfxModule* pSdMod = *reinterpret_cast<SfxModule**>(GetAppData( SHL_DRAW ));
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::EModule::IMPRESS ) )
    {
        if ( aFactory == "com.sun.star.presentation.PresentationDocument" )
        {
            if ( !lcl_isOptionHidden( SID_SD_EDITOPTIONS, aOptionsDlgOpt ) )
            {
                ResStringArray& rImpressArray = aDlgResource.GetImpressArray();
                setGroupName( "Impress", rImpressArray.GetString(0) );
                nGroup = AddGroup( rImpressArray.GetString( 0 ), pSdMod, pSdMod, SID_SD_EDITOPTIONS );
                const sal_uInt16 nCount = static_cast< const sal_uInt16 >( rImpressArray.Count() );
                for ( i = 1; i < nCount; ++i )
                {
                    nPageId = (sal_uInt16)rImpressArray.GetValue(i);
                    if ( lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                        continue;

                    AddTabPage( nPageId, rImpressArray.GetString(i), nGroup );
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
                ResStringArray& rDrawArray = aDlgResource.GetDrawArray();
                setGroupName( "Draw", rDrawArray.GetString(0) );
                nGroup = AddGroup( rDrawArray.GetString( 0 ), pSdMod, pSdMod, SID_SD_GRAPHIC_OPTIONS );
                const sal_uInt16 nCount = static_cast< const sal_uInt16 >( rDrawArray.Count() );
                for ( i = 1; i < nCount; ++i )
                {
                    nPageId = (sal_uInt16)rDrawArray.GetValue(i);
                    if ( lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                        continue;

                    AddTabPage( nPageId, rDrawArray.GetString(i), nGroup );
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
                ResStringArray& rStarMathArray = aDlgResource.GetStarMathArray();
                SfxModule* pSmMod = *reinterpret_cast<SfxModule**>(GetAppData(SHL_SM));
                setGroupName( "Math", rStarMathArray.GetString(0) );
                nGroup = AddGroup(rStarMathArray.GetString(0), pSmMod, pSmMod, SID_SM_EDITOPTIONS );
                for ( i = 1; i < rStarMathArray.Count(); ++i )
                {
                    nPageId = (sal_uInt16)rStarMathArray.GetValue(i);
                    if ( !lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                        AddTabPage( nPageId, rStarMathArray.GetString(i), nGroup );
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
        ResStringArray& rDSArray = aDlgResource.GetDatasourcesArray();
        setGroupName( "Base", rDSArray.GetString(0) );
        nGroup = AddGroup( rDSArray.GetString(0), nullptr, nullptr, SID_SB_STARBASEOPTIONS );
        for ( i = 1; i < rDSArray.Count(); ++i )
        {
            nPageId = (sal_uInt16)rDSArray.GetValue(i);
            if ( !lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                AddTabPage( nPageId, rDSArray.GetString(i), nGroup );
        }
    }

    // Chart options (always installed and active)
    if ( !lcl_isOptionHidden( SID_SCH_EDITOPTIONS, aOptionsDlgOpt ) )
    {
        ResStringArray& rChartArray = aDlgResource.GetChartArray();
        setGroupName( "Charts", rChartArray.GetString(0) );
        nGroup = AddGroup( rChartArray.GetString(0), nullptr, nullptr, SID_SCH_EDITOPTIONS );
        for ( i = 1; i < rChartArray.Count(); ++i )
        {
            nPageId = (sal_uInt16)rChartArray.GetValue(i);
            if ( !lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
               AddTabPage( nPageId, rChartArray.GetString(i), nGroup );
        }
    }

    // Internet options
    if ( !lcl_isOptionHidden( SID_INET_DLG, aOptionsDlgOpt ) )
    {
        ResStringArray& rInetArray = aDlgResource.GetInetArray();
        setGroupName( "Internet", rInetArray.GetString(0) );
        nGroup = AddGroup(rInetArray.GetString(0), nullptr, nullptr, SID_INET_DLG );

        for ( i = 1; i < rInetArray.Count(); ++i )
        {
            nPageId = (sal_uInt16)rInetArray.GetValue(i);
            if ( lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                continue;
#if defined(_WIN32)
            // Disable E-mail tab-page on Windows
            if ( nPageId == RID_SVXPAGE_INET_MAIL )
                continue;
#endif
            AddTabPage( nPageId, rInetArray.GetString(i), nGroup );
        }
    }

/*!!!
    ActivateLastSelection();
 */
}

bool isNodeActive( OptionsNode* pNode, Module* pModule )
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
            for (OrderedEntry* j : pModule->m_aNodeList)
                if ( j->m_sId == pNode->m_sId )
                    return true;
        }
    }
    return false;
}

void OfaTreeOptionsDialog::LoadExtensionOptions( const OUString& rExtensionId )
{
    Module* pModule = nullptr;

    // when called by Tools - Options then load nodes of active module
    if ( rExtensionId.isEmpty() )
    {
        pModule = LoadModule( GetModuleIdentifier( Reference< XFrame >() ) );
    }

    VectorOfNodes aNodeList = LoadNodes( pModule, rExtensionId );
    InsertNodes( aNodeList );

    delete pModule;
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
        catch ( Exception& )
        {
            SAL_WARN( "cui.options", "OfaTreeOptionsDialog::GetModuleIdentifier(): exception of XModuleManager::identify()" );
        }
    }
    return sModule;
}

Module* OfaTreeOptionsDialog::LoadModule(
    const OUString& rModuleIdentifier )
{
    Module* pModule = nullptr;
    Reference< XNameAccess > xSet(
        officecfg::Office::OptionsDialog::Modules::get());

    Sequence< OUString > seqNames = xSet->getElementNames();
    for ( int i = 0; i < seqNames.getLength(); ++i )
    {
        OUString sModule( seqNames[i] );
        if ( rModuleIdentifier == sModule )
        {
            // current active module found
            pModule = new Module;
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
                                    new OrderedEntry( nIndex, xTemp[x] ) );
                            else
                            {
                                // search position of the node
                                sal_uInt32 y = 0;
                                for ( ; y < pModule->m_aNodeList.size(); ++y )
                                {
                                    sal_Int32 nNodeIdx = pModule->m_aNodeList[y]->m_nIndex;
                                    if ( nNodeIdx < 0 || nNodeIdx > nIndex )
                                        break;
                                }
                                // and insert the node on this position
                                pModule->m_aNodeList.insert(
                                    pModule->m_aNodeList.begin() + y,
                                    new OrderedEntry( nIndex, xTemp[x] ) );
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
            OUString sNodeId, sLabel, sPageURL, sGroupId;
            bool bAllModules = false;
            sal_Int32 nGroupIndex = 0;

            sNodeId = seqNames[i];
            xNodeAccess->getByName( "Label" ) >>= sLabel;
            xNodeAccess->getByName( "OptionsPage" ) >>= sPageURL;
            xNodeAccess->getByName( "AllModules" ) >>= bAllModules;
            xNodeAccess->getByName( "GroupId" ) >>= sGroupId;
            xNodeAccess->getByName( "GroupIndex" ) >>= nGroupIndex;

            if ( sLabel.isEmpty() )
                sLabel = sGroupName;
            OUString sTemp = getGroupName( sLabel, !rExtensionId.isEmpty() );
            if ( !sTemp.isEmpty() )
                sLabel = sTemp;
            OptionsNode* pNode =
                new OptionsNode( sNodeId, sLabel, sPageURL, bAllModules );

            if ( rExtensionId.isEmpty() && !isNodeActive( pNode, pModule ) )
            {
                delete pNode;
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
                            OptionsLeaf* pLeaf = new OptionsLeaf(
                                sLeafLabel, sLeafURL, sEventHdl, sLeafGrpId, nLeafGrpIdx );

                            if ( !sLeafGrpId.isEmpty() )
                            {
                                bool bAlreadyOpened = false;
                                if ( pNode->m_aGroupedLeaves.size() > 0 )
                                {
                                    for (std::vector<OptionsLeaf*> & rGroup : pNode->m_aGroupedLeaves)
                                    {
                                        if ( rGroup.size() > 0 &&
                                             rGroup[0]->m_sGroupId == sLeafGrpId )
                                        {
                                            sal_uInt32 l = 0;
                                            for ( ; l < rGroup.size(); ++l )
                                            {
                                                if ( rGroup[l]->m_nGroupIndex >= nLeafGrpIdx )
                                                    break;
                                            }
                                            rGroup.insert( rGroup.begin() + l, pLeaf );
                                            bAlreadyOpened = true;
                                            break;
                                        }
                                    }
                                }
                                if ( !bAlreadyOpened )
                                {
                                    VectorOfLeaves aGroupedLeaves;
                                    aGroupedLeaves.push_back( pLeaf );
                                    pNode->m_aGroupedLeaves.push_back( aGroupedLeaves );
                                }
                            }
                            else
                                pNode->m_aLeaves.push_back( pLeaf );
                        }
                    }
                }
            }

            // do not insert nodes without leaves
            if ( pNode->m_aLeaves.size() > 0 || pNode->m_aGroupedLeaves.size() > 0 )
            {
                pModule ? aNodeList.push_back( pNode ) : aOutNodeList.push_back( pNode );
            }
            else
                delete pNode;
        }
    }

    if ( pModule && aNodeList.size() > 0 )
    {
        sal_uInt32 i = 0, j = 0;
        for ( ; i < pModule->m_aNodeList.size(); ++i )
        {
            OUString sNodeId = pModule->m_aNodeList[i]->m_sId;
            for ( j = 0; j < aNodeList.size(); ++j )
            {
                OptionsNode* pNode = aNodeList[j];
                if ( pNode->m_sId == sNodeId )
                {
                    aOutNodeList.push_back( pNode );
                    aNodeList.erase( aNodeList.begin() + j );
                    break;
                }
            }
        }

        for ( i = 0; i < aNodeList.size(); ++i )
            aOutNodeList.push_back( aNodeList[i] );
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
    OfaTreeOptionsDialog* pDlg, OptionsNode* pNode, OptionsLeaf* pLeaf, const SvTreeListBox& rTreeLB )
{
    sal_uInt16 nGrpId = lcl_getGroupId( pNode->m_sLabel, rTreeLB );
    if ( USHRT_MAX == nGrpId )
    {
        sal_uInt16 nNodeGrpId = getGroupNodeId( pNode->m_sId );
        nGrpId = pDlg->AddGroup( pNode->m_sLabel, nullptr, nullptr, nNodeGrpId );
        if ( !pNode->m_sPageURL.isEmpty() )
        {
            SvTreeListEntry* pGrpEntry = rTreeLB.GetEntry( nullptr, nGrpId );
            DBG_ASSERT( pGrpEntry, "OfaTreeOptionsDialog::InsertNodes(): no group" );
            if ( pGrpEntry )
            {
                OptionsGroupInfo* pGrpInfo =
                    static_cast<OptionsGroupInfo*>(pGrpEntry->GetUserData());
                pGrpInfo->m_sPageURL = pNode->m_sPageURL;
            }
        }
    }
    OptionsPageInfo* pInfo = pDlg->AddTabPage( 0, pLeaf->m_sLabel, nGrpId );
    pInfo->m_sPageURL = pLeaf->m_sPageURL;
    pInfo->m_sEventHdl = pLeaf->m_sEventHdl;
}

void  OfaTreeOptionsDialog::InsertNodes( const VectorOfNodes& rNodeList )
{
    for (OptionsNode* pNode : rNodeList)
    {
        if ( pNode->m_aLeaves.size() > 0 || pNode->m_aGroupedLeaves.size() > 0 )
        {
            sal_uInt32 j = 0;
            for ( ; j < pNode->m_aGroupedLeaves.size(); ++j )
            {
                for ( size_t k = 0; k < pNode->m_aGroupedLeaves[j].size(); ++k )
                {
                    OptionsLeaf* pLeaf = pNode->m_aGroupedLeaves[j][k];
                    lcl_insertLeaf( this, pNode, pLeaf, *pTreeLB );
                }
            }

            for ( j = 0; j < pNode->m_aLeaves.size(); ++j )
            {
                OptionsLeaf* pLeaf = pNode->m_aLeaves[j];
                lcl_insertLeaf( this, pNode, pLeaf, *pTreeLB );
            }
        }
    }
}

short OfaTreeOptionsDialog::Execute()
{
    std::unique_ptr< SvxDicListChgClamp > pClamp;
    if ( !bIsFromExtensionManager )
    {
        // collect all DictionaryList Events while the dialog is executed
        Reference<css::linguistic2::XSearchableDictionaryList> xDictionaryList(SvxGetDictionaryList());
        pClamp.reset( new SvxDicListChgClamp( xDictionaryList ) );
    }
    short nRet = SfxModalDialog::Execute();

    if( RET_OK == nRet )
    {
        ApplyItemSets();
        if( mpColorPage )
            mpColorPage->SaveToViewFrame( SfxViewFrame::Current() );
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
    m_xWinProvider      ( rProvider ),
    m_bIsWindowHidden   ( false )

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
        bool bWithHandler = ( !m_sEventHdl.isEmpty() );
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
                    vcl::Window* pWindow = VCLUnoHelper::GetWindow( xWinPeer );
                    if ( pWindow )
                        pWindow->SetStyle( pWindow->GetStyle() | WB_DIALOGCONTROL | WB_CHILDDLGCTRL );
                }
            }
        }
    }
    catch (const css::lang::IllegalArgumentException& e)
    {
        SAL_WARN("cui.options", "ExtensionsTabPage::CreateDialogWithHandler(): illegal argument:" << e.Message);
    }
    catch (const Exception& e)
    {
        SAL_WARN( "cui.options", "ExtensionsTabPage::CreateDialogWithHandler(): exception of XDialogProvider2::createDialogWithHandler(): " << e.Message);
    }
}


bool ExtensionsTabPage::DispatchAction( const OUString& rAction )
{
    bool bRet = false;
    if ( m_xEventHdl.is() )
    {
        try
        {
            bRet = m_xEventHdl->callHandlerMethod( m_xPage, makeAny( rAction ), "external_event" );
        }
        catch ( Exception& )
        {
            SAL_WARN( "cui.options", "ExtensionsTabPage::DispatchAction(): exception of XDialogEventHandler::callHandlerMethod()" );
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
        m_bIsWindowHidden = false;
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
