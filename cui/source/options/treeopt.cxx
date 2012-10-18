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

#include "cuires.hrc"
#include "helpid.hrc"
#include "treeopt.hrc"

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
#include "optpath.hxx"
#include "optsave.hxx"
#include "optupdt.hxx"
#include "treeopt.hxx"

#include <com/sun/star/awt/XContainerWindowEventHandler.hpp>
#include <com/sun/star/awt/XContainerWindowProvider.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/loader/CannotActivateFactoryException.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <comphelper/processfactory.hxx>
#include <editeng/optitems.hxx>
#include <editeng/unolingu.hxx>
#include <linguistic/misc.hxx>
#include <officecfg/Office/OptionsDialog.hxx>
#include <osl/module.hxx>
#include <osl/process.h>
#include <rtl/bootstrap.hxx>
#include <rtl/uri.hxx>
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
#include <vcl/msgbox.hxx>
#include <vcl/waitobj.hxx>

#ifndef _SVX_LANGITEM_HXX
#define ITEMID_LANGUAGE SID_ATTR_CHAR_LANGUAGE
#include <editeng/langitem.hxx>
#endif

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

#define EXPAND_PROTOCOL         "vnd.sun.star.expand:"

LastPageSaver* OfaTreeOptionsDialog::pLastPageSaver = NULL;

// some stuff for easier changes for SvtViewOptions
static const sal_Char*      pViewOptDataName = "page data";
#define VIEWOPT_DATANAME    rtl::OUString::createFromAscii( pViewOptDataName )

static XOutdevItemPool* mpStaticXOutdevItemPool = 0L;

static inline void SetViewOptUserItem( SvtViewOptions& rOpt, const String& rData )
{
    rOpt.SetUserItem( VIEWOPT_DATANAME, makeAny( rtl::OUString( rData ) ) );
}

static inline String GetViewOptUserItem( const SvtViewOptions& rOpt )
{
    Any aAny( rOpt.GetUserItem( VIEWOPT_DATANAME ) );
    rtl::OUString aUserData;
    aAny >>= aUserData;

    return String( aUserData );
}

struct ModuleToGroupNameMap_Impl
{
    const char* m_pModule;
    String      m_sGroupName;
    sal_uInt16      m_nNodeId;
};

static OfaPageResource* pPageRes = NULL;

static ModuleToGroupNameMap_Impl ModuleMap[] =
{
    { "ProductName", String::EmptyString(), SID_GENERAL_OPTIONS },
    { "LanguageSettings", String::EmptyString(), SID_LANGUAGE_OPTIONS },
    { "Internet", String::EmptyString(), SID_INET_DLG },
    { "LoadSave", String::EmptyString(), SID_FILTER_DLG },
    { "Writer", String::EmptyString(), SID_SW_EDITOPTIONS },
    { "WriterWeb", String::EmptyString(), SID_SW_ONLINEOPTIONS },
    { "Math", String::EmptyString(), SID_SM_EDITOPTIONS },
    { "Calc", String::EmptyString(), SID_SC_EDITOPTIONS },
    { "Impress", String::EmptyString(), SID_SD_EDITOPTIONS },
    { "Draw", String::EmptyString(), SID_SD_GRAPHIC_OPTIONS },
    { "Charts", String::EmptyString(), SID_SCH_EDITOPTIONS },
    { "Base", String::EmptyString(), SID_SB_STARBASEOPTIONS },

    { NULL, String::EmptyString(), 0xFFFF }
};

static void setGroupName( const rtl::OUString& rModule, const String& rGroupName )
{
    sal_uInt16 nIndex = 0;
    while ( ModuleMap[ nIndex ].m_pModule )
    {
        rtl::OUString sTemp =
            rtl::OUString::createFromAscii( ModuleMap[ nIndex ].m_pModule );
        if ( sTemp == rModule )
        {
            ModuleMap[ nIndex ].m_sGroupName = rGroupName;
            break;
        }
        ++nIndex;
    }
}

static String getGroupName( const rtl::OUString& rModule, bool bForced )
{
    String sGroupName;
    sal_uInt16 nIndex = 0;
    while ( ModuleMap[ nIndex ].m_pModule )
    {
        rtl::OUString sTemp =
            rtl::OUString::createFromAscii( ModuleMap[ nIndex ].m_pModule );
        if ( sTemp == rModule )
        {
            sGroupName = ModuleMap[ nIndex ].m_sGroupName;
            break;
        }
        ++nIndex;
    }

    if ( sGroupName.Len() == 0 && bForced )
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
        ModuleMap[ nIndex++ ].m_sGroupName = String::EmptyString();
}

static sal_uInt16 getGroupNodeId( const rtl::OUString& rModule )
{
    sal_uInt16 nNodeId = 0xFFFF, nIndex = 0;
    while ( ModuleMap[ nIndex ].m_pModule )
    {
        rtl::OUString sTemp =
            rtl::OUString::createFromAscii( ModuleMap[ nIndex ].m_pModule );
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
    friend class SvxEMailTabPage;
    // variables
    sal_Bool bIsEmailSupported;

public:
    MailMergeCfg_Impl();
    virtual ~MailMergeCfg_Impl();

    virtual void    Commit();
    virtual void Notify( const com::sun::star::uno::Sequence< rtl::OUString >& _rPropertyNames);

    sal_Bool IsEmailSupported() const {return bIsEmailSupported;}

};

MailMergeCfg_Impl::MailMergeCfg_Impl() :
    utl::ConfigItem("Office.Writer/MailMergeWizard"),
    bIsEmailSupported(sal_False)
{
    Sequence<rtl::OUString> aNames(1);
    aNames.getArray()[0] = "EMailSupported";
    const Sequence< Any > aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    if(aValues.getLength() && pValues[0].hasValue())
        pValues[0] >>= bIsEmailSupported;
}

MailMergeCfg_Impl::~MailMergeCfg_Impl()
{
}
/* -------------------------------------------------------------------------*/
void MailMergeCfg_Impl::Commit()
{
}

void MailMergeCfg_Impl::Notify( const com::sun::star::uno::Sequence< rtl::OUString >& )
{
}

//typedef SfxTabPage* (*FNCreateTabPage)( Window *pParent, const SfxItemSet &rAttrSet );
SfxTabPage* CreateGeneralTabPage( sal_uInt16 nId, Window* pParent, const SfxItemSet& rSet )
{
    CreateTabPage fnCreate = 0;
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
        case RID_SVXPAGE_COLORCONFIG:               fnCreate = &SvxColorOptionsTabPage::Create; break;
        case RID_OFAPAGE_HTMLOPT:                   fnCreate = &OfaHtmlTabPage::Create; break;
        case SID_OPTFILTER_MSOFFICE:                fnCreate = &OfaMSFilterTabPage::Create; break;
        case RID_OFAPAGE_MSFILTEROPT2:              fnCreate = &OfaMSFilterTabPage2::Create; break;
        case RID_SVXPAGE_JSEARCH_OPTIONS:           fnCreate = &SvxJSearchOptionsPage::Create ; break;
        case SID_SB_CONNECTIONPOOLING:              fnCreate = &::offapp::ConnectionPoolOptionsPage::Create; break;
        case SID_SB_DBREGISTEROPTIONS:              fnCreate = &::svx::DbRegistrationOptionsPage::Create; break;
        case RID_SVXPAGE_ACCESSIBILITYCONFIG:       fnCreate = &SvxAccessibilityOptionsTabPage::Create; break;
        case RID_SVXPAGE_OPTIONS_CTL:               fnCreate = &SvxCTLOptionsPage::Create ; break;
        case RID_SVXPAGE_INET_MOZPLUGIN:            fnCreate = &MozPluginTabPage::Create; break;
        case RID_SVXPAGE_OPTIONS_JAVA:              fnCreate = &SvxJavaOptionsPage::Create ; break;
        case RID_SVXPAGE_ONLINEUPDATE:              fnCreate = &SvxOnlineUpdateTabPage::Create; break;
        case RID_OPTPAGE_CHART_DEFCOLORS:           fnCreate = &SvxDefaultColorOptPage::Create; break;
    }

    SfxTabPage* pRet = fnCreate ? (*fnCreate)( pParent, rSet ) : NULL;
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
    { "ProductName",        NULL,                   SID_GENERAL_OPTIONS },
    { "ProductName",        "UserData",             RID_SFXPAGE_GENERAL },
    { "ProductName",        "General",              OFA_TP_MISC },
    { "ProductName",        "Memory",               OFA_TP_MEMORY },
    { "ProductName",        "View",                 OFA_TP_VIEW },
    { "ProductName",        "Print",                RID_SFXPAGE_PRINTOPTIONS },
    { "ProductName",        "Paths",                RID_SFXPAGE_PATH },
    { "ProductName",        "Colors",               RID_SVXPAGE_COLOR },
    { "ProductName",        "Fonts",                RID_SVX_FONT_SUBSTITUTION },
    { "ProductName",        "Security",             RID_SVXPAGE_INET_SECURITY },
    { "ProductName",        "Appearance",           RID_SVXPAGE_COLORCONFIG },
    { "ProductName",        "Accessibility",        RID_SVXPAGE_ACCESSIBILITYCONFIG },
    { "ProductName",        "Java",                 RID_SVXPAGE_OPTIONS_JAVA },
    { "ProductName",        "OnlineUpdate",         RID_SVXPAGE_ONLINEUPDATE },
    { "LanguageSettings",   NULL,                   SID_LANGUAGE_OPTIONS },
    { "LanguageSettings",   "Languages",            OFA_TP_LANGUAGES  },
    { "LanguageSettings",   "WritingAids",          RID_SFXPAGE_LINGU },
    { "LanguageSettings",   "SearchingInJapanese",  RID_SVXPAGE_JSEARCH_OPTIONS },
    { "LanguageSettings",   "AsianLayout",          RID_SVXPAGE_ASIAN_LAYOUT },
    { "LanguageSettings",   "ComplexTextLayout",    RID_SVXPAGE_OPTIONS_CTL },
    { "Internet",           NULL,                   SID_INET_DLG },
    { "Internet",           "Proxy",                RID_SVXPAGE_INET_PROXY },
    { "Internet",           "Email",                RID_SVXPAGE_INET_MAIL },
    { "Internet",           "MozillaPlugin",        RID_SVXPAGE_INET_MOZPLUGIN },
    { "LoadSave",           NULL,                   SID_FILTER_DLG },
    { "LoadSave",           "General",              RID_SFXPAGE_SAVE },
    { "LoadSave",           "VBAProperties",        SID_OPTFILTER_MSOFFICE },
    { "LoadSave",           "MicrosoftOffice",      RID_OFAPAGE_MSFILTEROPT2 },
    { "LoadSave",           "HTMLCompatibility",    RID_OFAPAGE_HTMLOPT },
    { "Writer",             NULL,                   SID_SW_EDITOPTIONS },
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
    { "WriterWeb",          NULL,                   SID_SW_ONLINEOPTIONS },
    { "WriterWeb",          "View",                 RID_SW_TP_HTML_CONTENT_OPT },
    { "WriterWeb",          "FormattingAids",       RID_SW_TP_HTML_OPTSHDWCRSR },
    { "WriterWeb",          "Grid",                 RID_SW_TP_HTML_OPTGRID_PAGE },
    { "WriterWeb",          "Print",                RID_SW_TP_HTML_OPTPRINT_PAGE },
    { "WriterWeb",          "Table",                RID_SW_TP_HTML_OPTTABLE_PAGE },
    { "WriterWeb",          "Background",           RID_SW_TP_BACKGROUND },
    { "Math",               NULL,                   SID_SM_EDITOPTIONS },
    { "Math",               "Settings",             SID_SM_TP_PRINTOPTIONS },
    { "Calc",               NULL,                   SID_SC_EDITOPTIONS },
    { "Calc",               "General",              SID_SC_TP_LAYOUT },
    { "Calc",               "View",                 SID_SC_TP_CONTENT },
    { "Calc",               "International",        RID_OFA_TP_INTERNATIONAL },
    { "Calc",               "Calculate",            SID_SC_TP_CALC },
    { "Calc",               "Formula",              SID_SC_TP_FORMULA },
    { "Calc",               "SortLists",            SID_SC_TP_USERLISTS },
    { "Calc",               "Changes",              SID_SC_TP_CHANGES },
    { "Calc",               "Compatibility",        SID_SC_TP_COMPATIBILITY },
    { "Calc",               "Grid",                 SID_SC_TP_GRID },
    { "Calc",               "Print",                RID_SC_TP_PRINT },
    { "Impress",            NULL,                   SID_SD_EDITOPTIONS },
    { "Impress",            "General",              SID_SI_TP_MISC },
    { "Impress",            "View",                 SID_SI_TP_CONTENTS },
    { "Impress",            "Grid",                 SID_SI_TP_SNAP },
    { "Impress",            "Print",                SID_SI_TP_PRINT },
    { "Draw",               NULL,                   SID_SD_GRAPHIC_OPTIONS },
    { "Draw",               "General",              SID_SD_TP_MISC },
    { "Draw",               "View",                 SID_SD_TP_CONTENTS },
    { "Draw",               "Grid",                 SID_SD_TP_SNAP },
    { "Draw",               "Print",                SID_SD_TP_PRINT },
    { "Charts",             NULL,                   SID_SCH_EDITOPTIONS },
    { "Charts",             "DefaultColors",        RID_OPTPAGE_CHART_DEFCOLORS },
    { "Base",               NULL,                   SID_SB_STARBASEOPTIONS },
    { "Base",               "Connections",          SID_SB_CONNECTIONPOOLING },
    { "Base",               "Databases",            SID_SB_DBREGISTEROPTIONS },
    { NULL,                 NULL,                   0 }
};

static sal_Bool lcl_getStringFromID( sal_uInt16 _nPageId, String& _rGroupName, String& _rPageName )
{
    sal_Bool bRet = sal_False;

    sal_uInt16 nIdx = 0;
    while ( OptionsMap_Impl[nIdx].m_pGroupName != NULL )
    {
        if ( _nPageId == OptionsMap_Impl[nIdx].m_nPageId )
        {
            bRet = sal_True;
            _rGroupName = String( OptionsMap_Impl[nIdx].m_pGroupName, RTL_TEXTENCODING_ASCII_US );
            if ( OptionsMap_Impl[nIdx].m_pPageName != NULL )
                _rPageName = String( OptionsMap_Impl[nIdx].m_pPageName, RTL_TEXTENCODING_ASCII_US );
            break;
        }
        ++nIdx;
    }

    return bRet;
}

static sal_Bool lcl_isOptionHidden( sal_uInt16 _nPageId, const SvtOptionsDialogOptions& _rOptOptions )
{
    sal_Bool bIsHidden = sal_False;
    String sGroupName, sPageName;
    if ( lcl_getStringFromID( _nPageId, sGroupName, sPageName ) )
    {
        if ( sPageName.Len() == 0 )
            bIsHidden =  _rOptOptions.IsGroupHidden( sGroupName );
        else
            bIsHidden =  _rOptOptions.IsPageHidden( sPageName, sGroupName );
    }
    return bIsHidden;
}

struct OptionsPageInfo
{
    SfxTabPage*         m_pPage;
    sal_uInt16          m_nPageId;
    rtl::OUString       m_sPageURL;
    rtl::OUString       m_sEventHdl;
    ExtensionsTabPage*  m_pExtPage;

    OptionsPageInfo( sal_uInt16 nId ) : m_pPage( NULL ), m_nPageId( nId ), m_pExtPage( NULL ) {}
};

struct OptionsGroupInfo
{
    SfxItemSet*         m_pInItemSet;
    SfxItemSet*         m_pOutItemSet;
    SfxShell*           m_pShell;       // used to create the page
    SfxModule*          m_pModule;      // used to create the ItemSet
    sal_uInt16          m_nDialogId;    // Id of the former dialog
    sal_Bool            m_bLoadError;   // load fails?
    rtl::OUString       m_sPageURL;
    ExtensionsTabPage*  m_pExtPage;

    OptionsGroupInfo( SfxShell* pSh, SfxModule* pMod, sal_uInt16 nId ) :
        m_pInItemSet( NULL ), m_pOutItemSet( NULL ), m_pShell( pSh ),
        m_pModule( pMod ), m_nDialogId( nId ), m_bLoadError( sal_False ),
        m_sPageURL( rtl::OUString() ), m_pExtPage( NULL ) {}
    ~OptionsGroupInfo() { delete m_pInItemSet; delete m_pOutItemSet; }
};

// -----------------------------------------------------------------------

#define INI_LIST() \
    aOkPB               ( this, CUI_RES( PB_OK ) ),\
    aCancelPB           ( this, CUI_RES( PB_CANCEL ) ),\
    aHelpPB             ( this, CUI_RES( PB_HELP ) ),\
    aBackPB             ( this, CUI_RES( PB_BACK ) ),\
    aSeparatorFL        ( this, CUI_RES( FL_SEPARATOR ) ),\
    aTreeLB             ( this, CUI_RES( TLB_PAGES ) ),\
    sTitle              ( GetText() ),\
    sNotLoadedError     (       CUI_RES( ST_LOAD_ERROR ) ),\
    pCurrentPageEntry   ( NULL ),\
    pColorPageItemSet   ( NULL ),\
    mpColorPage         ( NULL ),\
    bForgetSelection    ( sal_False ),\
    bIsFromExtensionManager( false ), \
    bIsForSetDocumentLanguage( false )

// Ctor() with Frame -----------------------------------------------------
using namespace ::com::sun::star;
OfaTreeOptionsDialog::OfaTreeOptionsDialog(
    Window* pParent,
    const Reference< XFrame >& _xFrame,
    bool bActivateLastSelection ) :

    SfxModalDialog( pParent, CUI_RES( RID_OFADLG_OPTIONS_TREE ) ),
    INI_LIST()
{
    FreeResource();

    InitTreeAndHandler();
    Initialize( _xFrame );
    LoadExtensionOptions( rtl::OUString() );
    ResizeTreeLB();
    if (bActivateLastSelection)
        ActivateLastSelection();

    aTreeLB.SetAccessibleName(GetDisplayText());
}

// Ctor() with ExtensionId -----------------------------------------------

OfaTreeOptionsDialog::OfaTreeOptionsDialog( Window* pParent, const rtl::OUString& rExtensionId ) :

    SfxModalDialog( pParent, CUI_RES( RID_OFADLG_OPTIONS_TREE ) ),
    INI_LIST()
{
    FreeResource();

    bIsFromExtensionManager = ( !rExtensionId.isEmpty() );
    InitTreeAndHandler();
    LoadExtensionOptions( rExtensionId );
    ResizeTreeLB();
    ActivateLastSelection();
}

OfaTreeOptionsDialog::~OfaTreeOptionsDialog()
{
    SvTreeListEntry* pEntry = aTreeLB.First();
    // first children
    while(pEntry)
    {
        // if Child (has parent), then OptionsPageInfo
        if(aTreeLB.GetParent(pEntry))
        {
            OptionsPageInfo *pPageInfo = (OptionsPageInfo *)pEntry->GetUserData();
            if(pPageInfo->m_pPage)
            {
                pPageInfo->m_pPage->FillUserData();
                String aPageData(pPageInfo->m_pPage->GetUserData());
                if ( aPageData.Len() )
                {
                    SvtViewOptions aTabPageOpt( E_TABPAGE, String::CreateFromInt32( pPageInfo->m_nPageId ) );
                    SetViewOptUserItem( aTabPageOpt, aPageData );
                }
                delete pPageInfo->m_pPage;
            }

            if (pPageInfo->m_nPageId == RID_SFXPAGE_LINGU)
            {
                // write personal dictionaries
                Reference< XDictionaryList >  xDicList( SvxGetDictionaryList() );
                if (xDicList.is())
                {
                    linguistic::SaveDictionaries( xDicList );
                }
            }

            if( pPageInfo->m_pExtPage )
                delete pPageInfo->m_pExtPage;

            delete pPageInfo;
        }
        pEntry = aTreeLB.Next(pEntry);
    }

    // and parents
    pEntry = aTreeLB.First();
    while(pEntry)
    {
        if(!aTreeLB.GetParent(pEntry))
        {
            OptionsGroupInfo* pGroupInfo = (OptionsGroupInfo*)pEntry->GetUserData();
            if ( pGroupInfo && pGroupInfo->m_pExtPage )
                delete pGroupInfo->m_pExtPage;
            delete pGroupInfo;
        }
        pEntry = aTreeLB.Next(pEntry);
    }
    delete pColorPageItemSet;
    deleteGroupNames();
}

OptionsPageInfo* OfaTreeOptionsDialog::AddTabPage(
    sal_uInt16 nId, const String& rPageName, sal_uInt16 nGroup )
{
    OptionsPageInfo* pPageInfo = new OptionsPageInfo( nId );
    SvTreeListEntry* pParent = aTreeLB.GetEntry( 0, nGroup );
    DBG_ASSERT( pParent, "OfaTreeOptionsDialog::AddTabPage(): no group found" );
    SvTreeListEntry* pEntry = aTreeLB.InsertEntry( rPageName, pParent );
    pEntry->SetUserData( pPageInfo );
    return pPageInfo;
}

// the ItemSet* is passed on to the dialog's ownership
sal_uInt16  OfaTreeOptionsDialog::AddGroup(const String& rGroupName,
                                        SfxShell* pCreateShell,
                                        SfxModule* pCreateModule,
                                        sal_uInt16 nDialogId )
{
    SvTreeListEntry* pEntry = aTreeLB.InsertEntry(rGroupName);
    OptionsGroupInfo* pInfo =
        new OptionsGroupInfo( pCreateShell, pCreateModule, nDialogId );
    pEntry->SetUserData(pInfo);
    sal_uInt16 nRet = 0;
    pEntry = aTreeLB.First();
    while(pEntry)
    {
        if(!aTreeLB.GetParent(pEntry))
            nRet++;
        pEntry = aTreeLB.Next(pEntry);
    }
    return nRet - 1;
}

IMPL_LINK_NOARG(OfaTreeOptionsDialog, ShowPageHdl_Impl)
{
    SelectHdl_Impl();
    return 0;
}

IMPL_LINK_NOARG(OfaTreeOptionsDialog, BackHdl_Impl)
{
    if ( pCurrentPageEntry && aTreeLB.GetParent( pCurrentPageEntry ) )
    {
        OptionsPageInfo* pPageInfo = (OptionsPageInfo*)pCurrentPageEntry->GetUserData();
        if ( pPageInfo->m_pPage )
        {
            OptionsGroupInfo* pGroupInfo =
                (OptionsGroupInfo*)aTreeLB.GetParent( pCurrentPageEntry )->GetUserData();
            if ( RID_SVXPAGE_COLOR == pPageInfo->m_nPageId )
                pPageInfo->m_pPage->Reset( *pColorPageItemSet );
            else
                pPageInfo->m_pPage->Reset( *pGroupInfo->m_pInItemSet );
        }
        else if ( pPageInfo->m_pExtPage )
            pPageInfo->m_pExtPage->ResetPage();
    }
    return 0;
}

IMPL_LINK_NOARG(OfaTreeOptionsDialog, OKHdl_Impl)
{
    aTreeLB.EndSelection();
    if ( pCurrentPageEntry && aTreeLB.GetParent( pCurrentPageEntry ) )
    {
        OptionsPageInfo* pPageInfo = (OptionsPageInfo *)pCurrentPageEntry->GetUserData();
        if ( pPageInfo->m_pPage )
        {
            OptionsGroupInfo* pGroupInfo =
                (OptionsGroupInfo *)aTreeLB.GetParent(pCurrentPageEntry)->GetUserData();
            if ( RID_SVXPAGE_COLOR != pPageInfo->m_nPageId
                && pPageInfo->m_pPage->HasExchangeSupport() )
            {
                int nLeave = pPageInfo->m_pPage->DeactivatePage(pGroupInfo->m_pOutItemSet);
                if ( nLeave == SfxTabPage::KEEP_PAGE )
                {
                    // the page mustn't be left
                    aTreeLB.Select(pCurrentPageEntry);
                    return 0;
                }
            }
            pPageInfo->m_pPage->Hide();
        }
    }

    SvTreeListEntry* pEntry = aTreeLB.First();
    while ( pEntry )
    {
        if ( aTreeLB.GetParent( pEntry ) )
        {
            OptionsPageInfo* pPageInfo = (OptionsPageInfo *)pEntry->GetUserData();
            if ( pPageInfo->m_pPage && !pPageInfo->m_pPage->HasExchangeSupport() )
            {
                OptionsGroupInfo* pGroupInfo =
                    (OptionsGroupInfo*)aTreeLB.GetParent(pEntry)->GetUserData();
                pPageInfo->m_pPage->FillItemSet(*pGroupInfo->m_pOutItemSet);
            }

            if ( pPageInfo->m_pExtPage )
            {
                pPageInfo->m_pExtPage->DeactivatePage();
                pPageInfo->m_pExtPage->SavePage();
            }
        }
        pEntry = aTreeLB.Next(pEntry);
    }
    EndDialog(RET_OK);
    return 0;
}

// an opened group shall be completely visible
IMPL_LINK(OfaTreeOptionsDialog, ExpandedHdl_Impl, SvTreeListBox*, pBox )
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
    return 0;
}

void OfaTreeOptionsDialog::ApplyItemSets()
{
    SvTreeListEntry* pEntry = aTreeLB.First();
    while(pEntry)
    {
        if(!aTreeLB.GetParent(pEntry))
        {
            OptionsGroupInfo* pGroupInfo = (OptionsGroupInfo *)pEntry->GetUserData();
            if(pGroupInfo->m_pOutItemSet)
            {
                if(pGroupInfo->m_pShell)
                    pGroupInfo->m_pShell->ApplyItemSet( pGroupInfo->m_nDialogId, *pGroupInfo->m_pOutItemSet);
                else
                    ApplyItemSet( pGroupInfo->m_nDialogId, *pGroupInfo->m_pOutItemSet);
            }
        }
        pEntry = aTreeLB.Next(pEntry);
    }
}

void OfaTreeOptionsDialog::InitTreeAndHandler()
{
    aTreeLB.SetNodeDefaultImages();

    aTreeLB.SetHelpId( HID_OFADLG_TREELISTBOX );
    aTreeLB.SetStyle( aTreeLB.GetStyle()|WB_HASBUTTONS | WB_HASBUTTONSATROOT |
                           WB_HASLINES | WB_HASLINESATROOT |
                           WB_CLIPCHILDREN | WB_HSCROLL | WB_FORCE_MAKEVISIBLE | WB_QUICK_SEARCH );
    aTreeLB.SetSpaceBetweenEntries( 0 );
    aTreeLB.SetSelectionMode( SINGLE_SELECTION );
    aTreeLB.SetSublistOpenWithLeftRight( sal_True );
    aTreeLB.SetExpandedHdl( LINK( this, OfaTreeOptionsDialog, ExpandedHdl_Impl ) );
    aTreeLB.SetSelectHdl( LINK( this, OfaTreeOptionsDialog, ShowPageHdl_Impl ) );
    aBackPB.SetClickHdl( LINK( this, OfaTreeOptionsDialog, BackHdl_Impl ) );
    aOkPB.SetClickHdl( LINK( this, OfaTreeOptionsDialog, OKHdl_Impl ) );
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
    bForgetSelection = sal_True;
    sal_uInt16 nTemp = pLastPageSaver->m_nLastPageId;
    pLastPageSaver->m_nLastPageId = nResId;
    ActivateLastSelection();
    pLastPageSaver->m_nLastPageId = nTemp;
}

void OfaTreeOptionsDialog::ActivatePage( const String& rPageURL )
{
    DBG_ASSERT( !bIsFromExtensionManager, "OfaTreeOptionsDialog::ActivatePage(): call from extension manager" );
    if ( !pLastPageSaver )
        pLastPageSaver = new LastPageSaver;
    bForgetSelection = sal_True;
    pLastPageSaver->m_nLastPageId = 0;
    pLastPageSaver->m_sLastPageURL_Tools = rPageURL;
    ActivateLastSelection();
}

void OfaTreeOptionsDialog::ActivateLastSelection()
{
    SvTreeListEntry* pEntry = NULL;
    if ( pLastPageSaver )
    {
        String sExpand( EXPAND_PROTOCOL  );
        String sLastURL = bIsFromExtensionManager ? pLastPageSaver->m_sLastPageURL_ExtMgr
                                                  : pLastPageSaver->m_sLastPageURL_Tools;
        if ( sLastURL.Len() == 0 )
        {
            sLastURL = !bIsFromExtensionManager ? pLastPageSaver->m_sLastPageURL_ExtMgr
                                                : pLastPageSaver->m_sLastPageURL_Tools;
        }

        // MacroExpander to convert "expand"-URL to "file"-URL
        Reference< XMacroExpander > m_xMacroExpander;
        bool bMustExpand = ( INetURLObject( sLastURL ).GetProtocol() == INET_PROT_FILE );

        if ( bMustExpand )
        {
            Reference< XComponentContext > xContext(
                comphelper::getProcessComponentContext() );
            m_xMacroExpander = Reference< com::sun::star::util::XMacroExpander >(
                xContext->getValueByName( ::rtl::OUString( "/singletons/com.sun.star.util.theMacroExpander"  ) ), UNO_QUERY );
        }

        SvTreeListEntry* pTemp = aTreeLB.First();
        while( !pEntry && pTemp )
        {
            // restore only selection of a leaf
            if ( aTreeLB.GetParent( pTemp ) && pTemp->GetUserData() )
            {
                OptionsPageInfo* pPageInfo = (OptionsPageInfo*)pTemp->GetUserData();
                String sPageURL = pPageInfo->m_sPageURL;
                if ( bMustExpand
                    && sPageURL.Len() > 0
                    && sExpand.Match( sPageURL ) == STRING_MATCH )
                {
                    // cut protocol
                    ::rtl::OUString sTemp( sPageURL.Copy( sizeof( EXPAND_PROTOCOL ) -1 ) );
                    // decode uri class chars
                    sTemp = ::rtl::Uri::decode(
                        sTemp, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
                    // expand string
                    sPageURL = m_xMacroExpander->expandMacros( sTemp );
                }

                if ( ( !bIsFromExtensionManager
                        && pPageInfo->m_nPageId && pPageInfo->m_nPageId == pLastPageSaver->m_nLastPageId )
                            || ( !pPageInfo->m_nPageId && sLastURL == sPageURL ) )
                    pEntry = pTemp;
            }
            pTemp = aTreeLB.Next(pTemp);
        }
    }

    if ( !pEntry )
    {
        pEntry = aTreeLB.First();
        pEntry = aTreeLB.Next(pEntry);
    }

    SvTreeListEntry* pParent = aTreeLB.GetParent(pEntry);
    aTreeLB.Expand(pParent);
    aTreeLB.MakeVisible(pParent);
    aTreeLB.MakeVisible(pEntry);
    aTreeLB.Select(pEntry);
    aTreeLB.GrabFocus();
}

long    OfaTreeOptionsDialog::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        const KeyCode aKeyCode = pKEvt->GetKeyCode();

        if( aKeyCode.GetCode() == KEY_PAGEUP ||
                aKeyCode.GetCode() == KEY_PAGEDOWN)
        {
            SvTreeListEntry* pCurEntry = aTreeLB.FirstSelected();
            SvTreeListEntry*  pTemp = 0;
            if(aKeyCode.GetCode() == KEY_PAGEDOWN)
            {
                pTemp =  aTreeLB.Next( pCurEntry ) ;
                if(pTemp && !aTreeLB.GetParent(pTemp))
                {
                    pTemp =  aTreeLB.Next( pTemp ) ;
                    aTreeLB.Select(pTemp);
                }
            }
            else
            {
                pTemp =  aTreeLB.Prev( pCurEntry ) ;
                if(pTemp && !aTreeLB.GetParent(pTemp))
                {
                    pTemp =  aTreeLB.Prev( pTemp ) ;
                }
            }
            if(pTemp)
            {
                if(!aTreeLB.IsExpanded(aTreeLB.GetParent(pTemp)))
                    aTreeLB.Expand(aTreeLB.GetParent(pTemp));
                aTreeLB.MakeVisible(pTemp);
                aTreeLB.Select(pTemp);
            }
        }
    }
    return SfxModalDialog::Notify(rNEvt);
}

// --------------------------------------------------------------------

void OfaTreeOptionsDialog::SelectHdl_Impl()
{
    SvTreeListBox* pBox = &aTreeLB;

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

    TabPage* pOldPage = NULL;
    TabPage* pNewPage = NULL;
    OptionsPageInfo* pOptPageInfo = ( pCurrentPageEntry && aTreeLB.GetParent( pCurrentPageEntry ) )
        ? (OptionsPageInfo*)pCurrentPageEntry->GetUserData() : NULL;

    if ( pOptPageInfo && pOptPageInfo->m_pPage && pOptPageInfo->m_pPage->IsVisible() )
    {
        pOldPage = pOptPageInfo->m_pPage;
        OptionsGroupInfo* pGroupInfo = (OptionsGroupInfo*)aTreeLB.GetParent(pCurrentPageEntry)->GetUserData();
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
    else if ( pCurrentPageEntry && !aTreeLB.GetParent( pCurrentPageEntry ) )
    {
        OptionsGroupInfo* pGroupInfo = (OptionsGroupInfo*)pCurrentPageEntry->GetUserData();
        if ( pGroupInfo && pGroupInfo->m_pExtPage )
        {
            pGroupInfo->m_pExtPage->Hide();
            pGroupInfo->m_pExtPage->DeactivatePage();
        }
    }

    OptionsPageInfo *pPageInfo = (OptionsPageInfo *)pEntry->GetUserData();
    OptionsGroupInfo* pGroupInfo = (OptionsGroupInfo *)pParent->GetUserData();
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
                sal_Bool bIdentical = pGroupInfo->m_pModule == pGroupInfo->m_pShell;

                WaitObject aWait(this);
                //pGroupInfo->pModule = pGroupInfo->pModule->Load();
                if(!pGroupInfo->m_pModule)
                {
                    pGroupInfo->m_bLoadError = sal_True;
                    InfoBox(pBox, sNotLoadedError).Execute();
                    return;
                }
                if(bIdentical)
                    pGroupInfo->m_pShell = pGroupInfo->m_pModule;
                // now test whether there was the same module in other groups, too (e. g. Text+HTML)
                SvTreeListEntry* pTemp = aTreeLB.First();
                while(pTemp)
                {
                    if(!aTreeLB.GetParent(pTemp) && pTemp != pEntry)
                    {
                        OptionsGroupInfo* pTGInfo = (OptionsGroupInfo *)pTemp->GetUserData();
                        if(pTGInfo->m_pModule == pOldModule)
                        {
                            pTGInfo->m_pModule = pGroupInfo->m_pModule;
                            if(bIdentical)
                                pTGInfo->m_pShell = pGroupInfo->m_pModule;
                        }
                    }
                    pTemp = aTreeLB.Next(pTemp);
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
            pPageInfo->m_pPage = ::CreateGeneralTabPage(
                pPageInfo->m_nPageId, this, *pColorPageItemSet );
            mpColorPage = (SvxColorTabPage*)pPageInfo->m_pPage;
            mpColorPage->SetupForViewFrame( SfxViewFrame::Current() );
        }
        else
        {
            pPageInfo->m_pPage = ::CreateGeneralTabPage(pPageInfo->m_nPageId, this, *pGroupInfo->m_pInItemSet );

            if(!pPageInfo->m_pPage && pGroupInfo->m_pModule)
                pPageInfo->m_pPage = pGroupInfo->m_pModule->CreateTabPage(pPageInfo->m_nPageId, this, *pGroupInfo->m_pInItemSet);
        }

        DBG_ASSERT( pPageInfo->m_pPage, "tabpage could not created");
        if ( pPageInfo->m_pPage )
        {
            SvtViewOptions aTabPageOpt( E_TABPAGE, String::CreateFromInt32( pPageInfo->m_nPageId ) );
            pPageInfo->m_pPage->SetUserData( GetViewOptUserItem( aTabPageOpt ) );

            Point aPagePos( aSeparatorFL.GetPosPixel().X(), aTreeLB.GetPosPixel().Y());
            pPageInfo->m_pPage->SetPosPixel( aPagePos );
            if ( RID_SVXPAGE_COLOR == pPageInfo->m_nPageId )
            {
                pPageInfo->m_pPage->Reset( *pColorPageItemSet );
                pPageInfo->m_pPage->ActivatePage( *pColorPageItemSet );
            }
            else
            {
                pPageInfo->m_pPage->Reset( *pGroupInfo->m_pInItemSet );
            }
        }
    }
    else if ( 0 == pPageInfo->m_nPageId && !pPageInfo->m_pExtPage )
    {
        if ( !m_xContainerWinProvider.is() )
        {
            Reference < XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
            m_xContainerWinProvider = Reference < awt::XContainerWindowProvider >(
                xFactory->createInstance(
                "com.sun.star.awt.ContainerWindowProvider" ), UNO_QUERY );
            DBG_ASSERT( m_xContainerWinProvider.is(), "service com.sun.star.awt.ContainerWindowProvider could not be loaded" );
        }

        pPageInfo->m_pExtPage = new ExtensionsTabPage(
            this, 0, pPageInfo->m_sPageURL, pPageInfo->m_sEventHdl, m_xContainerWinProvider );

        Point aPos(aSeparatorFL.GetPosPixel().X(), aTreeLB.GetPosPixel().Y());
        Size aSize(aSeparatorFL.GetSizePixel().Width(),
                   aSeparatorFL.GetPosPixel().Y() - aTreeLB.GetPosPixel().Y());
        pPageInfo->m_pExtPage->SetPosSizePixel( aPos, aSize );
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
        ::rtl::OUStringBuffer sTitleBuf(sTitle);
        sTitleBuf.appendAscii(RTL_CONSTASCII_STRINGPARAM(" - "));
        sTitleBuf.append(aTreeLB.GetEntryText(pParent));
        sTitleBuf.appendAscii(RTL_CONSTASCII_STRINGPARAM(" - "));
        sTitleBuf.append(aTreeLB.GetEntryText(pEntry));
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
    Window* pFocusWin = Application::GetFocusWindow();
    // if the focused window is not the options treebox and the old page has the focus
    if ( pFocusWin && pFocusWin != pBox && pOldPage && pOldPage->HasChildPathFocus() )
        // then set the focus to the new page or if we are on a group set the focus to the options treebox
        pNewPage ? pNewPage->GrabFocus() : pBox->GrabFocus();

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
    Reference< XPropertySet >  xProp( SvxGetLinguPropertySet() );
    SfxItemSet* pRet = 0;
    switch(nId)
    {
        case SID_GENERAL_OPTIONS:
        {
            pRet = new SfxItemSet(
                SFX_APP()->GetPool(),
                SID_ATTR_METRIC, SID_ATTR_SPELL,
                SID_AUTOSPELL_CHECK, SID_AUTOSPELL_CHECK,
                SID_ATTR_QUICKLAUNCHER, SID_ATTR_QUICKLAUNCHER,
                SID_ATTR_YEAR2000, SID_ATTR_YEAR2000,
                SID_HTML_MODE, SID_HTML_MODE,
                0 );

            SfxItemSet aOptSet( SFX_APP()->GetPool(), SID_ATTR_QUICKLAUNCHER, SID_ATTR_QUICKLAUNCHER );
            SFX_APP()->GetOptions(aOptSet);
            pRet->Put(aOptSet);

            utl::MiscCfg    aMisc;
            const SfxPoolItem* pItem;
            SfxViewFrame* pViewFrame = SfxViewFrame::Current();
            if ( pViewFrame )
            {
                SfxDispatcher* pDispatch = pViewFrame->GetDispatcher();

                // miscellaneous - Year2000
                if( SFX_ITEM_AVAILABLE <= pDispatch->QueryState( SID_ATTR_YEAR2000, pItem ) )
                    pRet->Put( SfxUInt16Item( SID_ATTR_YEAR2000, ((const SfxUInt16Item*)pItem)->GetValue() ) );
                else
                    pRet->Put( SfxUInt16Item( SID_ATTR_YEAR2000, (sal_uInt16)aMisc.GetYear2000() ) );
            }
            else
                pRet->Put( SfxUInt16Item( SID_ATTR_YEAR2000, (sal_uInt16)aMisc.GetYear2000() ) );


            // miscellaneous - Tabulator
            pRet->Put(SfxBoolItem(SID_PRINTER_NOTFOUND_WARN, aMisc.IsNotFoundWarning()));

            sal_uInt16 nFlag = aMisc.IsPaperSizeWarning() ? SFX_PRINTER_CHG_SIZE : 0;
            nFlag  |= aMisc.IsPaperOrientationWarning()  ? SFX_PRINTER_CHG_ORIENTATION : 0;
            pRet->Put( SfxFlagItem( SID_PRINTER_CHANGESTODOC, nFlag ));

        }
        break;
        case SID_LANGUAGE_OPTIONS :
        {
            pRet = new SfxItemSet(SFX_APP()->GetPool(),
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
                xProp->getPropertyValue( rtl::OUString(
                        UPN_HYPH_MIN_LEADING) ) >>= nMinLead;
                xProp->getPropertyValue( rtl::OUString(
                        UPN_HYPH_MIN_TRAILING) ) >>= nMinTrail;
            }
            aHyphen.GetMinLead()  = (sal_uInt8)nMinLead;
            aHyphen.GetMinTrail() = (sal_uInt8)nMinTrail;

            const SfxPoolItem* pItem;
            SfxPoolItem* pClone;
            SfxViewFrame* pViewFrame = SfxViewFrame::Current();
            if ( pViewFrame )
            {
                SfxDispatcher* pDispatch = pViewFrame->GetDispatcher();
                if(SFX_ITEM_AVAILABLE <= pDispatch->QueryState(SID_ATTR_LANGUAGE, pItem))
                    pRet->Put(SfxUInt16Item(SID_ATTR_LANGUAGE, ((const SvxLanguageItem*)pItem)->GetLanguage()));
                if(SFX_ITEM_AVAILABLE <= pDispatch->QueryState(SID_ATTR_CHAR_CJK_LANGUAGE, pItem))
                    pRet->Put(SfxUInt16Item(SID_ATTR_CHAR_CJK_LANGUAGE, ((const SvxLanguageItem*)pItem)->GetLanguage()));
                if(SFX_ITEM_AVAILABLE <= pDispatch->QueryState(SID_ATTR_CHAR_CTL_LANGUAGE, pItem))
                    pRet->Put(SfxUInt16Item(SID_ATTR_CHAR_CTL_LANGUAGE, ((const SvxLanguageItem*)pItem)->GetLanguage()));

                pRet->Put(aHyphen);
                if(SFX_ITEM_AVAILABLE <= pDispatch->QueryState(SID_AUTOSPELL_CHECK, pItem))
                {
                    pClone = pItem->Clone();
                    pRet->Put(*pClone);
                    delete pClone;
                }
                else
                {
                        sal_Bool bVal = sal_False;
                        if (xProp.is())
                        {
                            xProp->getPropertyValue( rtl::OUString( UPN_IS_SPELL_AUTO) ) >>= bVal;
                        }

                        pRet->Put(SfxBoolItem(SID_AUTOSPELL_CHECK, bVal));
                }
            }
            pRet->Put( SfxBoolItem( SID_SET_DOCUMENT_LANGUAGE, bIsForSetDocumentLanguage ) );
        }
        break;
        case SID_INET_DLG :
                pRet = new SfxItemSet( SFX_APP()->GetPool(),
                                SID_BASIC_ENABLED, SID_BASIC_ENABLED,
                //SID_OPTIONS_START - ..END
                                SID_INET_PROXY_PORT,
                                SID_SAVEREL_INET, SID_SAVEREL_FSYS,
                                SID_INET_SMTPSERVER, SID_INET_SMTPSERVER,
                                SID_INET_NOPROXY, SID_INET_SOCKS_PROXY_PORT,
                                SID_INET_DNS_AUTO, SID_INET_DNS_SERVER,
                                SID_SECURE_URL, SID_SECURE_URL,
                                0L );
                SFX_APP()->GetOptions(*pRet);
        break;
        case SID_FILTER_DLG:
            pRet = new SfxItemSet( SFX_APP()->GetPool(),
            SID_ATTR_DOCINFO, SID_ATTR_AUTOSAVEMINUTE,
            SID_SAVEREL_INET, SID_SAVEREL_FSYS,
            SID_ATTR_PRETTYPRINTING, SID_ATTR_PRETTYPRINTING,
            SID_ATTR_WARNALIENFORMAT, SID_ATTR_WARNALIENFORMAT,
            0 );
            SFX_APP()->GetOptions(*pRet);
            break;

        case SID_SB_STARBASEOPTIONS:
            pRet = new SfxItemSet( SFX_APP()->GetPool(),
            SID_SB_POOLING_ENABLED, SID_SB_DB_REGISTER,
            0 );
            ::offapp::ConnectionPoolConfig::GetOptions(*pRet);
            ::svx::DbRegisteredNamesConfig::GetOptions(*pRet);
            break;

        case SID_SCH_EDITOPTIONS:
        {
            SvxChartOptions aChartOpt;
            pRet = new SfxItemSet( SFX_APP()->GetPool(), SID_SCH_EDITOPTIONS, SID_SCH_EDITOPTIONS );
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
            const SfxPoolItem* pItem;
            SfxItemSet aOptSet(SFX_APP()->GetPool(), SID_ATTR_QUICKLAUNCHER, SID_ATTR_QUICKLAUNCHER );
            aOptSet.Put(rSet);
            if(aOptSet.Count())
                SFX_APP()->SetOptions( aOptSet );
            // get dispatcher anew, because SetOptions() might have destroyed the dispatcher
            SfxViewFrame *pViewFrame = SfxViewFrame::Current();
// -------------------------------------------------------------------------
//          evaluate Year2000
// -------------------------------------------------------------------------
            sal_uInt16 nY2K = USHRT_MAX;
            if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_YEAR2000, sal_False, &pItem ) )
                nY2K = ((const SfxUInt16Item*)pItem)->GetValue();
            if( USHRT_MAX != nY2K )
            {
                if ( pViewFrame )
                {
                    SfxDispatcher* pDispatch = pViewFrame->GetDispatcher();
                    pDispatch->Execute( SID_ATTR_YEAR2000, SFX_CALLMODE_ASYNCHRON, pItem, 0L);
                }
                aMisc.SetYear2000(nY2K);
            }

// -------------------------------------------------------------------------
//          evaluate print
// -------------------------------------------------------------------------
            if(SFX_ITEM_SET == rSet.GetItemState(SID_PRINTER_NOTFOUND_WARN, sal_False, &pItem))
                aMisc.SetNotFoundWarning(((const SfxBoolItem*)pItem)->GetValue());

            if(SFX_ITEM_SET == rSet.GetItemState(SID_PRINTER_CHANGESTODOC, sal_False, &pItem))
            {
                const SfxFlagItem* pFlag = (const SfxFlagItem*)pItem;
                aMisc.SetPaperSizeWarning(0 != (pFlag->GetValue() &  SFX_PRINTER_CHG_SIZE ));
                aMisc.SetPaperOrientationWarning(0 !=  (pFlag->GetValue() & SFX_PRINTER_CHG_ORIENTATION ));
            }
// -------------------------------------------------------------------------
//          evaluate help options
// -------------------------------------------------------------------------
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
            SFX_APP()->SetOptions( rSet );
        break;

        case SID_SB_STARBASEOPTIONS:
            ::offapp::ConnectionPoolConfig::SetOptions( rSet );
            ::svx::DbRegisteredNamesConfig::SetOptions(rSet);
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
    sal_Bool bSaveSpellCheck = sal_False;
    const SfxPoolItem* pItem;

    if ( SFX_ITEM_SET == rSet.GetItemState( SID_SPELL_MODIFIED, sal_False, &pItem ) )
    {
        bSaveSpellCheck = ( (const SfxBoolItem*)pItem )->GetValue();
    }
    Reference< XMultiServiceFactory >  xMgr( ::comphelper::getProcessServiceFactory() );
    Reference< XPropertySet >  xProp(
            xMgr->createInstance( ::rtl::OUString( "com.sun.star.linguistic2.LinguProperties" ) ),
            UNO_QUERY );
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_HYPHENREGION, sal_False, &pItem ) )
    {
        const SfxHyphenRegionItem* pHyphenItem = (const SfxHyphenRegionItem*)pItem;

        if (xProp.is())
        {
            xProp->setPropertyValue(
                    rtl::OUString(UPN_HYPH_MIN_LEADING),
                    makeAny((sal_Int16) pHyphenItem->GetMinLead()) );
            xProp->setPropertyValue(
                    rtl::OUString(UPN_HYPH_MIN_TRAILING),
                    makeAny((sal_Int16) pHyphenItem->GetMinTrail()) );
        }
        bSaveSpellCheck = sal_True;
    }

    SfxViewFrame *pViewFrame = SfxViewFrame::Current();
    if ( pViewFrame )
    {
        SfxDispatcher* pDispatch = pViewFrame->GetDispatcher();
        pItem = 0;
        if(SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_LANGUAGE, sal_False, &pItem ))
        {
            pDispatch->Execute(pItem->Which(),    SFX_CALLMODE_ASYNCHRON, pItem, 0L);
            bSaveSpellCheck = sal_True;
        }
        if(SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_CHAR_CTL_LANGUAGE, sal_False, &pItem ))
        {
            pDispatch->Execute(pItem->Which(),    SFX_CALLMODE_ASYNCHRON, pItem, 0L);
            bSaveSpellCheck = sal_True;
        }
        if(SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_CHAR_CJK_LANGUAGE, sal_False, &pItem ))
        {
            pDispatch->Execute(pItem->Which(),    SFX_CALLMODE_ASYNCHRON, pItem, 0L);
            bSaveSpellCheck = sal_True;
        }

        if( SFX_ITEM_SET == rSet.GetItemState(SID_AUTOSPELL_CHECK, sal_False, &pItem ))
        {
            sal_Bool bOnlineSpelling = ((const SfxBoolItem*)pItem)->GetValue();
            pDispatch->Execute(SID_AUTOSPELL_CHECK,
                SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD, pItem, 0L);

            if (xProp.is())
            {
                xProp->setPropertyValue(
                        rtl::OUString(UPN_IS_SPELL_AUTO),
                        makeAny(bOnlineSpelling) );
            }
        }

        if( bSaveSpellCheck )
        {
            //! the config item has changed since we modified the
            //! property set it uses
            pDispatch->Execute(SID_SPELLCHECKER_CHANGED, SFX_CALLMODE_ASYNCHRON);
        }
    }

    if( SFX_ITEM_SET == rSet.GetItemState(SID_OPT_LOCALE_CHANGED, sal_False, &pItem ))
    {
        SfxViewFrame* _pViewFrame = SfxViewFrame::GetFirst();
        while ( _pViewFrame )
        {
            _pViewFrame->GetDispatcher()->Execute(pItem->Which(),    SFX_CALLMODE_ASYNCHRON, pItem, 0L);
            _pViewFrame = SfxViewFrame::GetNext( *_pViewFrame );
        }
    }
}

rtl::OUString getCurrentFactory_Impl( const Reference< XFrame >& _xFrame )
{
    rtl::OUString sIdentifier;
    Reference < XFrame > xCurrentFrame( _xFrame );
    Reference < XModuleManager2 > xModuleManager( ModuleManager::create(::comphelper::getProcessComponentContext()) );
    if ( !xCurrentFrame.is() )
    {
        Reference< XDesktop > xDesktop( ::comphelper::getProcessServiceFactory()->createInstance(
            DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );
        if ( xDesktop.is() )
            xCurrentFrame = xDesktop->getCurrentFrame();
    }

    if ( xCurrentFrame.is() )
    {
        try
        {
            sIdentifier = xModuleManager->identify( xCurrentFrame );
        }
        catch ( ::com::sun::star::frame::UnknownModuleException& )
        {
            DBG_WARNING( "getActiveModule_Impl(): unknown module" );
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
        nGroup = AddGroup( rGeneralArray.GetString(0), 0, 0, SID_GENERAL_OPTIONS );
        sal_uInt16 nEnd = static_cast< sal_uInt16 >( rGeneralArray.Count() );
        String sPageTitle;

        for ( i = 1; i < nEnd; ++i )
        {
            String sNewTitle = rGeneralArray.GetString(i);
            nPageId = (sal_uInt16)rGeneralArray.GetValue(i);
            if ( lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                continue;

            // Disable Online Update page if service not installed
            if( RID_SVXPAGE_ONLINEUPDATE == nPageId )
            {
                const ::rtl::OUString sService = "com.sun.star.setup.UpdateCheck";

                try
                {
                    Reference < XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
                    Reference < XInterface > xService( xFactory->createInstance( sService ) );

                    if( ! xService.is() )
                        continue;
                }
                catch ( ::com::sun::star::loader::CannotActivateFactoryException& )
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
        nGroup = AddGroup( rFilterArray.GetString(0), 0, 0, SID_FILTER_DLG );
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
        nGroup = AddGroup( rLangArray.GetString(0), 0, 0, SID_LANGUAGE_OPTIONS );
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


    rtl::OUString aFactory = getCurrentFactory_Impl( _xFrame );
    rtl::OUString sTemp = GetModuleIdentifier( comphelper::getProcessServiceFactory(), _xFrame );
    DBG_ASSERT( sTemp == aFactory, "S H I T!!!" );

    // Writer and Writer/Web options
    SvtModuleOptions aModuleOpt;
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
    {
        // text document
        ResStringArray& rTextArray = aDlgResource.GetTextArray();
        if (   aFactory.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.text.TextDocument" ) )
            || aFactory.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.text.WebDocument" ) )
            || aFactory.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.text.GlobalDocument" ) ) )
        {
            SfxModule* pSwMod = (*(SfxModule**) GetAppData(SHL_WRITER));
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
                AddTabPage( RID_SW_TP_OPTTEST_PAGE, rtl::OUString("Interner Test"), nGroup );
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
                AddTabPage( RID_SW_TP_OPTTEST_PAGE, rtl::OUString("Interner Test"), nGroup );
#endif
            }
        }
    }

    // Calc options
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC ) )
    {
        if ( aFactory == "com.sun.star.sheet.SpreadsheetDocument" )
        {
            if ( !lcl_isOptionHidden( SID_SC_EDITOPTIONS, aOptionsDlgOpt ) )
            {
                ResStringArray& rCalcArray = aDlgResource.GetCalcArray();
                SfxModule* pScMod = ( *( SfxModule** ) GetAppData( SHL_CALC ) );
                setGroupName( "Calc", rCalcArray.GetString(0) );
                nGroup = AddGroup( rCalcArray.GetString( 0 ), pScMod, pScMod, SID_SC_EDITOPTIONS );
                const sal_uInt16 nCount = static_cast< const sal_uInt16 >( rCalcArray.Count() );
                for ( i = 1; i < nCount; ++i )
                {
                    nPageId = (sal_uInt16)rCalcArray.GetValue(i);
                    if ( lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                        continue;
//                  if( bCTL || nId != RID_OFA_TP_INTERNATIONAL )
//                  #103755# if an international tabpage is need one day, this should be used again... ;-)
                    if ( nPageId != RID_OFA_TP_INTERNATIONAL )
                        AddTabPage( nPageId, rCalcArray.GetString( i ), nGroup );
                }
            }
        }
    }

    // Impress options
    SfxModule* pSdMod = ( *( SfxModule** ) GetAppData( SHL_DRAW ) );
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SIMPRESS ) )
    {
        if ( aFactory == "com.sun.star.presentation.PresentationDocument" )
        {
            if ( !lcl_isOptionHidden( SID_SD_EDITOPTIONS, aOptionsDlgOpt ) )
            {
                ResStringArray& rImpressArray = aDlgResource.GetImpressArray();
                setGroupName( "Impress", rImpressArray.GetString(0) );
                nGroup = AddGroup( rImpressArray.GetString( 0 ), pSdMod, pSdMod, SID_SD_EDITOPTIONS );
                const sal_Bool bCTL = aLanguageOptions.IsCTLFontEnabled();
                const sal_uInt16 nCount = static_cast< const sal_uInt16 >( rImpressArray.Count() );
                for ( i = 1; i < nCount; ++i )
                {
                    nPageId = (sal_uInt16)rImpressArray.GetValue(i);
                    if ( lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                        continue;
                    if ( bCTL || nPageId != RID_OFA_TP_INTERNATIONAL_IMPR )
                        AddTabPage( nPageId, rImpressArray.GetString(i), nGroup );
                }
            }
        }
    }

    // Draw options
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SDRAW ) )
    {
        if ( aFactory == "com.sun.star.drawing.DrawingDocument" )
        {
            if ( !lcl_isOptionHidden( SID_SD_GRAPHIC_OPTIONS, aOptionsDlgOpt ) )
            {
                ResStringArray& rDrawArray = aDlgResource.GetDrawArray();
                setGroupName( "Draw", rDrawArray.GetString(0) );
                nGroup = AddGroup( rDrawArray.GetString( 0 ), pSdMod, pSdMod, SID_SD_GRAPHIC_OPTIONS );
                const sal_Bool bCTL = aLanguageOptions.IsCTLFontEnabled();
                const sal_uInt16 nCount = static_cast< const sal_uInt16 >( rDrawArray.Count() );
                for ( i = 1; i < nCount; ++i )
                {
                    nPageId = (sal_uInt16)rDrawArray.GetValue(i);
                    if ( lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                        continue;
                    if ( bCTL || nPageId != RID_OFA_TP_INTERNATIONAL_SD )
                        AddTabPage( nPageId, rDrawArray.GetString(i), nGroup );
                }
            }
        }
    }

    // Math options
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SMATH ) )
    {
        if ( aFactory == "com.sun.star.formula.FormulaProperties" )
        {
            if ( !lcl_isOptionHidden( SID_SM_EDITOPTIONS, aOptionsDlgOpt ) )
            {
                ResStringArray& rStarMathArray = aDlgResource.GetStarMathArray();
                SfxModule* pSmMod = (*(SfxModule**) GetAppData(SHL_SM));
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
        (   aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SDATABASE )
        ||  aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER )
        ||  aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC )
        ) )
    {
        ResStringArray& rDSArray = aDlgResource.GetDatasourcesArray();
        setGroupName( "Base", rDSArray.GetString(0) );
        nGroup = AddGroup( rDSArray.GetString(0), 0, NULL, SID_SB_STARBASEOPTIONS );
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
        nGroup = AddGroup( rChartArray.GetString(0), 0, 0, SID_SCH_EDITOPTIONS );
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
        nGroup = AddGroup(rInetArray.GetString(0), 0, 0, SID_INET_DLG );

        for ( i = 1; i < rInetArray.Count(); ++i )
        {
            nPageId = (sal_uInt16)rInetArray.GetValue(i);
            if ( lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                continue;
#if defined WNT
            // Disable E-mail tab-page on Windows
            if ( nPageId == RID_SVXPAGE_INET_MAIL )
                continue;
#endif
#if defined MACOSX
            // Disable Mozilla Plug-in tab-page on Mac
            if ( nPageId == RID_SVXPAGE_INET_MOZPLUGIN )
                continue;
#endif
#ifdef LINUX
            // Disable Mozilla Plug-in tab-page on Linux if we find a
            // globally installed plugin
            if ( nPageId == RID_SVXPAGE_INET_MOZPLUGIN ) {
                struct stat sb;
                char *p;
                bool bHaveSystemWidePlugin = false;
                char mozpaths[]="/usr/lib/mozilla/plugins/libnpsoplugin.so:/usr/lib/firefox/plugins/libnpsoplugin.so:/usr/lib/mozilla-firefox/plugins/libnpsoplugin.so:/usr/lib/iceweasel/plugins/libnpsoplugin.so:/usr/lib/iceape/plugins/libnpsoplugin.so:/usr/lib/browser-plugins/libnpsoplugin.so:/usr/lib64/browser-plugins/libnpsoplugin.so";

                p = strtok(mozpaths, ":");
                while (p != NULL) {
                    if (stat(p, &sb) != -1) {
                         bHaveSystemWidePlugin = true;
                         break;
                    }
                    p = strtok(NULL, ":");
                }

                if (bHaveSystemWidePlugin == true)
                    continue;
            }
#endif
            AddTabPage( nPageId, rInetArray.GetString(i), nGroup );
        }
    }

/*!!!
    ResizeTreeLB();
    ActivateLastSelection();
 */
}

namespace
{
    void MoveControl( Control& _rCtrl, long _nDeltaPixel )
    {
        Point   aPt( _rCtrl.GetPosPixel() );
        aPt.X() += _nDeltaPixel;
        _rCtrl.SetPosPixel( aPt );
    }
}

void OfaTreeOptionsDialog::ResizeTreeLB( void )
{
    const long  nMax = aSeparatorFL.GetSizePixel().Width() * 42 / 100;
                                            // don't ask where 42 comes from... but it looks / feels ok ;-)
    long        nDelta = 50;                // min.
    sal_uInt16      nDepth = 0;
    const long  nIndent0 = PixelToLogic( Size( 28, 0 ) ).Width();
    const long  nIndent1 = PixelToLogic( Size( 52, 0 ) ).Width();

    SvLBoxTreeList* pTreeList = aTreeLB.GetModel();
    DBG_ASSERT( pTreeList, "-OfaTreeOptionsDialog::ResizeTreeLB(): no model, no cookies!" );

    SvTreeListEntry* pEntry = pTreeList->First();
    while( pEntry )
    {
        long n = aTreeLB.GetTextWidth(aTreeLB.GetEntryText(pEntry));
        n += ((nDepth == 0) ? nIndent0 : nIndent1);

        if( n > nDelta )
            nDelta = n;

        pEntry = pTreeList->Next( pEntry, &nDepth );
    }

    nDelta = LogicToPixel( Size( nDelta + 3, 0 ) ).Width();         // + extra space [logic]
    nDelta += GetSettings().GetStyleSettings().GetScrollBarSize();  // + scroll bar, in case it's needed

    if( nDelta > nMax )
        nDelta = nMax;

    // starting resizing with this
    Size            aSize( GetSizePixel() );
    aSize.Width() += nDelta;
    SetSizePixel( aSize );

    // resize treelistbox
    aSize = aTreeLB.GetSizePixel();
    aSize.Width() += nDelta;
    aTreeLB.SetSizePixel( aSize );

    // ... and move depending controls
    MoveControl( aOkPB, nDelta );
    MoveControl( aCancelPB, nDelta );
    MoveControl( aHelpPB, nDelta );
    MoveControl( aBackPB, nDelta );
    MoveControl( aSeparatorFL, nDelta );
}

bool isNodeActive( OptionsNode* pNode, Module* pModule )
{
    if ( pNode )
    {
        // Node for all modules actine?
        if ( pNode->m_bAllModules )
            return true;

        // OOo-Nodes (Writer, Calc, Impress...) are active if node is already inserted
        if ( getGroupName( pNode->m_sId, false ).Len() > 0 )
            return true;

        // no module -> not active
        if ( !pModule )
            return false;

        // search node in active module
        if ( pModule->m_bActive )
        {
            for ( sal_uInt32 j = 0; j < pModule->m_aNodeList.size(); ++j )
                if ( pModule->m_aNodeList[j]->m_sId == pNode->m_sId )
                    return true;
        }
    }
    return false;
}

void OfaTreeOptionsDialog::LoadExtensionOptions( const rtl::OUString& rExtensionId )
{
    Module* pModule = NULL;

    // when called by Tools - Options then load nodes of active module
    if ( rExtensionId.isEmpty() )
    {
        Reference< XMultiServiceFactory > xMSFac = comphelper::getProcessServiceFactory();
        pModule = LoadModule( GetModuleIdentifier( xMSFac, Reference< XFrame >() ) );
    }

    VectorOfNodes aNodeList = LoadNodes( pModule, rExtensionId );
    InsertNodes( aNodeList );

    delete pModule;
}

rtl::OUString OfaTreeOptionsDialog::GetModuleIdentifier(
    const Reference< XMultiServiceFactory >& xMFac, const Reference< XFrame >& rFrame )
{
    rtl::OUString sModule;
    Reference < XFrame > xCurrentFrame( rFrame );
    Reference < XModuleManager2 > xModuleManager( ModuleManager::create(comphelper::getComponentContext(xMFac)) );

    if ( !xCurrentFrame.is() )
    {
        Reference < XDesktop > xDesktop( xMFac->createInstance(
            "com.sun.star.frame.Desktop" ), UNO_QUERY );
        if ( xDesktop.is() )
            xCurrentFrame = xDesktop->getCurrentFrame();
    }

    if ( xCurrentFrame.is() )
    {
        try
        {
            sModule = xModuleManager->identify( xCurrentFrame );
        }
        catch ( ::com::sun::star::frame::UnknownModuleException& )
        {
            DBG_WARNING( "OfaTreeOptionsDialog::GetModuleIdentifier(): unknown module" );
        }
        catch ( Exception& )
        {
            SAL_WARN( "cui.options", "OfaTreeOptionsDialog::GetModuleIdentifier(): exception of XModuleManager::identify()" );
        }
    }
    return sModule;
}

Module* OfaTreeOptionsDialog::LoadModule(
    const rtl::OUString& rModuleIdentifier )
{
    Module* pModule = NULL;
    Reference< XNameAccess > xSet(
        officecfg::Office::OptionsDialog::Modules::get());

    Sequence< rtl::OUString > seqNames = xSet->getElementNames();
    for ( int i = 0; i < seqNames.getLength(); ++i )
    {
        rtl::OUString sModule( seqNames[i] );
        if ( rModuleIdentifier == sModule )
        {
            // current active module found
            pModule = new Module( sModule );
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
                    Sequence< rtl::OUString > xTemp = xNodeAccess->getElementNames();
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
        }
    }
    return pModule;
}

VectorOfNodes OfaTreeOptionsDialog::LoadNodes(
    Module* pModule, const rtl::OUString& rExtensionId)
{
    VectorOfNodes aOutNodeList;

    Reference< XNameAccess > xSet(
        officecfg::Office::OptionsDialog::Nodes::get());
    VectorOfNodes aNodeList;
    Sequence< rtl::OUString > seqNames = xSet->getElementNames();

    for ( int i = 0; i < seqNames.getLength(); ++i )
    {
        String sGroupName( seqNames[i] );
        Reference< XNameAccess > xNodeAccess;
        xSet->getByName( seqNames[i] ) >>= xNodeAccess;

        if ( xNodeAccess.is() )
        {
            rtl::OUString sNodeId, sLabel, sPageURL, sGroupId;
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
            String sTemp = getGroupName( sLabel, !rExtensionId.isEmpty() );
            if ( sTemp.Len() > 0 )
                sLabel = sTemp;
            OptionsNode* pNode =
                new OptionsNode( sNodeId, sLabel, sPageURL, bAllModules, sGroupId, nGroupIndex );

            if ( rExtensionId.isEmpty() && !isNodeActive( pNode, pModule ) )
            {
                delete pNode;
                continue;
            }

            Reference< XNameAccess > xLeavesSet;
            xNodeAccess->getByName( "Leaves" ) >>= xLeavesSet;
            if ( xLeavesSet.is() )
            {
                Sequence< rtl::OUString > seqLeaves = xLeavesSet->getElementNames();
                for ( int j = 0; j < seqLeaves.getLength(); ++j )
                {
                    Reference< XNameAccess > xLeaveAccess;
                    xLeavesSet->getByName( seqLeaves[j] ) >>= xLeaveAccess;

                    if ( xLeaveAccess.is() )
                    {
                        rtl::OUString sId, sLeafLabel, sEventHdl, sLeafURL, sLeafGrpId;
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
                                sId, sLeafLabel, sLeafURL, sEventHdl, sLeafGrpId, nLeafGrpIdx );

                            if ( !sLeafGrpId.isEmpty() )
                            {
                                bool bAlreadyOpened = false;
                                if ( pNode->m_aGroupedLeaves.size() > 0 )
                                {
                                    for ( sal_uInt32 k = 0;
                                          k < pNode->m_aGroupedLeaves.size(); ++k )
                                    {
                                        if ( pNode->m_aGroupedLeaves[k].size() > 0 &&
                                             pNode->m_aGroupedLeaves[k][0]->m_sGroupId
                                             == sLeafGrpId )
                                        {
                                            sal_uInt32 l = 0;
                                            for ( ; l < pNode->m_aGroupedLeaves[k].size(); ++l )
                                            {
                                                if ( pNode->m_aGroupedLeaves[k][l]->
                                                     m_nGroupIndex >= nLeafGrpIdx )
                                                    break;
                                            }
                                            pNode->m_aGroupedLeaves[k].insert(
                                                pNode->m_aGroupedLeaves[k].begin() + l, pLeaf );
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
                                pNode->m_aLeaves.push_back(
                                    new OptionsLeaf(
                                        sId, sLeafLabel, sLeafURL,
                                        sEventHdl, sLeafGrpId, nLeafGrpIdx ) );
                        }
                    }
                }
            }

            // do not insert nodes without leaves
            if ( pNode->m_aLeaves.size() > 0 || pNode->m_aGroupedLeaves.size() > 0 )
            {
                pModule ? aNodeList.push_back( pNode ) : aOutNodeList.push_back( pNode );
            }
        }
    }

    if ( pModule && aNodeList.size() > 0 )
    {
        sal_uInt32 i = 0, j = 0;
        for ( ; i < pModule->m_aNodeList.size(); ++i )
        {
            rtl::OUString sNodeId = pModule->m_aNodeList[i]->m_sId;
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

static sal_uInt16 lcl_getGroupId( const rtl::OUString& rGroupName, const SvTreeListBox& rTreeLB )
{
    String sGroupName( rGroupName );
    sal_uInt16 nRet = 0;
    SvTreeListEntry* pEntry = rTreeLB.First();
    while( pEntry )
    {
        if ( !rTreeLB.GetParent( pEntry ) )
        {
            String sTemp( rTreeLB.GetEntryText( pEntry ) );
            if ( sTemp == sGroupName )
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
        nGrpId = pDlg->AddGroup( pNode->m_sLabel, NULL, NULL, nNodeGrpId );
        if ( !pNode->m_sPageURL.isEmpty() )
        {
            SvTreeListEntry* pGrpEntry = rTreeLB.GetEntry( 0, nGrpId );
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
    for ( sal_uInt32 i = 0; i < rNodeList.size(); ++i )
    {
        OptionsNode* pNode = rNodeList[i];

        if ( pNode->m_aLeaves.size() > 0 || pNode->m_aGroupedLeaves.size() > 0 )
        {
            sal_uInt32 j = 0;
            for ( ; j < pNode->m_aGroupedLeaves.size(); ++j )
            {
                for ( sal_uInt32 k = 0; k < pNode->m_aGroupedLeaves[j].size(); ++k )
                {
                    OptionsLeaf* pLeaf = pNode->m_aGroupedLeaves[j][k];
                    lcl_insertLeaf( this, pNode, pLeaf, aTreeLB );
                }
            }

            for ( j = 0; j < pNode->m_aLeaves.size(); ++j )
            {
                OptionsLeaf* pLeaf = pNode->m_aLeaves[j];
                lcl_insertLeaf( this, pNode, pLeaf, aTreeLB );
            }
        }
    }
}

short OfaTreeOptionsDialog::Execute()
{
    ::std::auto_ptr< SvxDicListChgClamp > pClamp;
    if ( !bIsFromExtensionManager )
    {
        // collect all DictionaryList Events while the dialog is executed
        Reference<com::sun::star::linguistic2::XDictionaryList> xDictionaryList(SvxGetDictionaryList());
        pClamp = ::std::auto_ptr< SvxDicListChgClamp >( new SvxDicListChgClamp( xDictionaryList ) );
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
    Window* pParent, WinBits nStyle, const rtl::OUString& rPageURL,
    const rtl::OUString& rEvtHdl, const Reference< awt::XContainerWindowProvider >& rProvider ) :

    TabPage( pParent, nStyle ),

    m_sPageURL          ( rPageURL ),
    m_sEventHdl         ( rEvtHdl ),
    m_xWinProvider      ( rProvider ),
    m_bIsWindowHidden   ( false )

{
}

// -----------------------------------------------------------------------

ExtensionsTabPage::~ExtensionsTabPage()
{
    Hide();
    DeactivatePage();
}

// -----------------------------------------------------------------------

void ExtensionsTabPage::CreateDialogWithHandler()
{
    try
    {
        bool bWithHandler = ( !m_sEventHdl.isEmpty() );
        if ( bWithHandler )
        {
            Reference < XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
            m_xEventHdl = Reference< awt::XContainerWindowEventHandler >(
                xFactory->createInstance( m_sEventHdl ), UNO_QUERY );
        }

        if ( !bWithHandler || m_xEventHdl.is() )
        {
            SetStyle( GetStyle() | WB_DIALOGCONTROL | WB_CHILDDLGCTRL );
            Reference< awt::XWindowPeer > xParent( VCLUnoHelper::GetInterface( this ), UNO_QUERY );
            m_xPage = Reference < awt::XWindow >(
                m_xWinProvider->createContainerWindow(
                    m_sPageURL, rtl::OUString(), xParent, m_xEventHdl ), UNO_QUERY );

            Reference< awt::XControl > xPageControl( m_xPage, UNO_QUERY );
            if ( xPageControl.is() )
            {
                Reference< awt::XWindowPeer > xWinPeer( xPageControl->getPeer() );
                if ( xWinPeer.is() )
                {
                    Window* pWindow = VCLUnoHelper::GetWindow( xWinPeer );
                    if ( pWindow )
                        pWindow->SetStyle( pWindow->GetStyle() | WB_DIALOGCONTROL | WB_CHILDDLGCTRL );
                }
            }
        }
    }
    catch ( ::com::sun::star::lang::IllegalArgumentException& )
    {
        SAL_WARN( "cui.options", "ExtensionsTabPage::CreateDialogWithHandler(): illegal argument" );
    }
    catch ( Exception& )
    {
        SAL_WARN( "cui.options", "ExtensionsTabPage::CreateDialogWithHandler(): exception of XDialogProvider2::createDialogWithHandler()" );
    }
}

// -----------------------------------------------------------------------

sal_Bool ExtensionsTabPage::DispatchAction( const rtl::OUString& rAction )
{
    sal_Bool bRet = sal_False;
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

// -----------------------------------------------------------------------

void ExtensionsTabPage::ActivatePage()
{
    TabPage::ActivatePage();

    if ( !m_xPage.is() )
    {
        CreateDialogWithHandler();

        if ( m_xPage.is() )
        {
            Point aPos = Point();
            Size aSize = GetSizePixel();
            m_xPage->setPosSize( aPos.X() + 1, aPos.Y() + 1,
                                 aSize.Width() - 2, aSize.Height() - 2, awt::PosSize::POSSIZE );
            if ( !m_sEventHdl.isEmpty() )
                DispatchAction( "initialize" );
        }
    }

    if ( m_xPage.is() )
    {
        m_xPage->setVisible( sal_True );
        m_bIsWindowHidden = false;
    }
}

// -----------------------------------------------------------------------

void ExtensionsTabPage::DeactivatePage()
{
    TabPage::DeactivatePage();

    if ( m_xPage.is() )
        m_xPage->setVisible( sal_False );
}

// -----------------------------------------------------------------------

void ExtensionsTabPage::ResetPage()
{
    DispatchAction( "back" );
    ActivatePage();
}

// -----------------------------------------------------------------------

void ExtensionsTabPage::SavePage()
{
    DispatchAction( "ok" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
