/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: treeopt.cxx,v $
 *
 *  $Revision: 1.55 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 16:42:29 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XDICTIONARYLIST_HPP_
#include <com/sun/star/linguistic2/XDictionaryList.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include <com/sun/star/frame/XModuleManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMACROEXPANDER_HPP_
#include <com/sun/star/util/XMacroExpander.hpp>
#endif

#ifndef _OSL_MODULE_HXX_
#include <osl/module.hxx>
#endif

#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/frame/XLoadable.hpp>

#ifndef _TOOLS_RCID_H
#include <tools/rcid.h>
#endif

#include <tools/shl.hxx>

#include <comphelper/processfactory.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COMPHELPER_CONFIGURATIONHELPER_HXX_
#include <comphelper/configurationhelper.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_XDIALOGPROVIDER_HPP_
#include <com/sun/star/awt/XDialogProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XDIALOGPROVIDER2_HPP_
#include <com/sun/star/awt/XDialogProvider2.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTAINERWINDOWPROVIDER_HPP_
#include <com/sun/star/awt/XContainerWindowProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XDIALOGEVENTHANDLER_HPP_
#include <com/sun/star/awt/XDialogEventHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTAINERWINDOWEVENTHANDLER_HPP_
#include <com/sun/star/awt/XContainerWindowEventHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
#endif

#ifndef _SV_HELP_HXX
#include <vcl/help.hxx>
#endif
#ifndef _LINGUISTIC_MISC_HHX_
#include <linguistic/misc.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_HELPOPT_HXX
#include <svtools/helpopt.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#ifndef _SVTOOLS_LANGUAGEOPTIONS_HXX
#include <svtools/languageoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_OPTIONSDLG_HXX
#include <svtools/optionsdlg.hxx>
#endif
#ifndef _SFXMODULE_HXX //autogen
#include <sfx2/module.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SV_WAITOBJ_HXX //autogen
#include <vcl/waitobj.hxx>
#endif
#ifndef _SFXSLSTITM_HXX //autogen
#include <svtools/slstitm.hxx>
#endif
#include <sfx2/viewfrm.hxx>
#ifndef INCLUDED_SVTOOLS_VIEWOPTIONS_HXX
#include <svtools/viewoptions.hxx>
#endif
#ifndef _SFX_PRINTOPT_HXX
#include <sfx2/printopt.hxx>
#endif
#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _RTL_BOOTSTRAP_HXX_
#include <rtl/bootstrap.hxx>
#endif

#include <svtools/misccfg.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/tplpitem.hxx>
#include <svtools/apearcfg.hxx>
#include <svtools/linguprops.hxx>
#include <sfx2/app.hxx>

#ifndef _XDEF_HXX //autogen
#include <svx/xdef.hxx>
#endif
#ifndef _SVX_XFLCLIT_HXX //autogen
#include <svx/xflclit.hxx>
#endif
#ifndef _XPOOL_HXX //autogen
#include <svx/xpool.hxx>
#endif
#ifndef _SVX_TAB_AREA_HXX //autogen
#include "cuitabarea.hxx"
#endif
#ifndef _OFF_OFAITEM_HXX //autogen
#include "ofaitem.hxx"
#endif
#ifndef _OFA_OPTHTML_HXX //autogen
#include "opthtml.hxx"
#endif
#ifndef _SVX_OPTCOLOR_HXX
#include "optcolor.hxx"
#endif
#ifndef _SVX_OPTCTL_HXX
#include "optctl.hxx"
#endif
#ifndef _SVX_OPTJAVA_HXX
#include "optjava.hxx"
#endif
#ifndef _SVX_OPTSAVE_HXX //autogen
#include "optsave.hxx"
#endif
#ifndef _SVX_OPTPATH_HXX //autogen
#include "optpath.hxx"
#endif
#ifndef _SVX_CUIOPTGENRL_HXX //autogen
#include "cuioptgenrl.hxx"
#endif
#ifndef _SVX_OPTLINGU_HXX //autogen
#include <svx/optlingu.hxx>
#endif
#ifndef _SVX_TAB_AREA_HXX //autogen
#include "cuitabarea.hxx"
#endif
#ifndef _SVX_OPTINET_HXX //autogen
#include "optinet2.hxx"
#endif
#ifndef _SVX_OPTASIAN_HXX
#include "optasian.hxx"
#endif
#ifndef _SVX_OPTACCESSIBILITY_HXX
#include "optaccessibility.hxx"
#endif
#ifndef _SVX_OPTJSEARCH_HXX_
#include "optjsearch.hxx"
#endif
#ifndef _OFFAPP_CONNPOOLOPTIONS_HXX_
#include "connpooloptions.hxx"
#endif
#ifndef _SVX_OPTUPDT_HXX
#include "optupdt.hxx"
#endif
#ifndef _SVX_OPTCHART_HXX
#include "optchart.hxx"
#endif

#include "optgdlg.hxx"
#include "optmemory.hxx"
#include "optfltr.hxx"
#include <svx/dialogs.hrc>
#include "helpid.hrc"
#include <svx/dialmgr.hxx>
#include "treeopt.hxx"
#include "treeopt.hrc"
#include "fontsubs.hxx"
#include "unolingu.hxx"
#include <svx/xtable.hxx>
#include "connpoolconfig.hxx"
#include "dbregister.hxx"
#include "dbregisterednamesconfig.hxx"
#include "cfgchart.hxx"

#ifndef _SVX_LANGITEM_HXX
#define ITEMID_LANGUAGE SID_ATTR_CHAR_LANGUAGE
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_OPTITEMS_HXX


#include <optitems.hxx>
#endif

#ifndef _SVX_DRAWITEM_HXX
#include <drawitem.hxx>
#endif

#ifndef _RTL_URI_HXX_
#include <rtl/uri.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

#ifdef C2U
    #error  "Who define C2U before! I use it to create const ascii strings ..."
#else
    #define C2U(cChar)      rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( cChar ) )
#endif

#define HINT_TIMEOUT            200
#define SELECT_FIRST_TIMEOUT    0
#define SELECT_TIMEOUT          300
#define COLORPAGE_UNKNOWN       ((sal_uInt16)0xFFFF)
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
    USHORT      m_nNodeId;
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
    USHORT nIndex = 0;
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
    USHORT nIndex = 0;
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

        if ( rModule == C2U("Writer") )
            sGroupName = pPageRes->GetTextArray().GetString(0);
        else if ( rModule == C2U("WriterWeb") )
            sGroupName = pPageRes->GetHTMLArray().GetString(0);
        else if ( rModule == C2U("Calc") )
            sGroupName = pPageRes->GetCalcArray().GetString(0);
        else if ( rModule == C2U("Impress") )
            sGroupName = pPageRes->GetImpressArray().GetString(0);
        else if ( rModule == C2U("Draw") )
            sGroupName = pPageRes->GetDrawArray().GetString(0);
        else if ( rModule == C2U("Math") )
            sGroupName = pPageRes->GetStarMathArray().GetString(0);
        else if ( rModule == C2U("Base") )
            sGroupName = pPageRes->GetDatasourcesArray().GetString(0);
    }
    return sGroupName;
}

static void deleteGroupNames()
{
    USHORT nIndex = 0;
    while ( ModuleMap[ nIndex ].m_pModule )
        ModuleMap[ nIndex++ ].m_sGroupName = String::EmptyString();
}

static USHORT getGroupNodeId( const rtl::OUString& rModule )
{
    USHORT nNodeId = 0xFFFF, nIndex = 0;
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

/*-- 29.10.2004 13:57:25---------------------------------------------------

  -----------------------------------------------------------------------*/
class MailMergeCfg_Impl : public utl::ConfigItem
{
    friend class SvxEMailTabPage;
    // variables
    sal_Bool bIsEmailSupported;

public:
    MailMergeCfg_Impl();
    virtual ~MailMergeCfg_Impl();

    virtual void    Commit();

    sal_Bool IsEmailSupported() const {return bIsEmailSupported;}

};
/*-- 29.10.2004 13:57:25---------------------------------------------------

  -----------------------------------------------------------------------*/
MailMergeCfg_Impl::MailMergeCfg_Impl() :
    utl::ConfigItem(C2U("Office.Writer/MailMergeWizard")),
    bIsEmailSupported(sal_False)
{
    Sequence<rtl::OUString> aNames(1);
    aNames.getArray()[0] = C2U("EMailSupported");
    const Sequence< Any > aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    if(aValues.getLength() && pValues[0].hasValue())
        pValues[0] >>= bIsEmailSupported;
}
/*-- 29.10.2004 13:57:25---------------------------------------------------

  -----------------------------------------------------------------------*/
MailMergeCfg_Impl::~MailMergeCfg_Impl()
{
}
/* -------------------------------------------------------------------------*/
void MailMergeCfg_Impl::Commit()
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
        case RID_SVXPAGE_INET_SEARCH:               fnCreate = &SvxSearchTabPage::Create; break;
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
        case RID_SVXPAGE_SSO:                       fnCreate = GetSSOCreator(); break;
        case RID_SVXPAGE_OPTIONS_CTL:               fnCreate = &SvxCTLOptionsPage::Create ; break;
        //added by jmeng begin
        case RID_SVXPAGE_INET_MOZPLUGIN:            fnCreate = &MozPluginTabPage::Create; break;
        //added by jmeng end
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
    USHORT      m_nPageId;
};

static OptionsMapping_Impl __READONLY_DATA OptionsMap_Impl[] =
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
    { "ProductName",        "NetworkIdentity",      RID_SVXPAGE_SSO },
    { "ProductName",        "OnlineUpdate",         RID_SVXPAGE_ONLINEUPDATE },
    { "LanguageSettings",   NULL,                   SID_LANGUAGE_OPTIONS },
    { "LanguageSettings",   "Languages",            OFA_TP_LANGUAGES  },
    { "LanguageSettings",   "WritingAids",          RID_SFXPAGE_LINGU },
    { "LanguageSettings",   "SearchingInJapanese",  RID_SVXPAGE_JSEARCH_OPTIONS },
    { "LanguageSettings",   "AsianLayout",          RID_SVXPAGE_ASIAN_LAYOUT },
    { "LanguageSettings",   "ComplexTextLayout",    RID_SVXPAGE_OPTIONS_CTL },
    { "Internet",           NULL,                   SID_INET_DLG },
    { "Internet",           "Proxy",                RID_SVXPAGE_INET_PROXY },
    { "Internet",           "Search",               RID_SVXPAGE_INET_SEARCH },
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
    { "Calc",               "SortLists",            SID_SC_TP_USERLISTS },
    { "Calc",               "Changes",              SID_SC_TP_CHANGES },
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

static sal_Bool lcl_getStringFromID( USHORT _nPageId, String& _rGroupName, String& _rPageName )
{
    sal_Bool bRet = sal_False;

    USHORT nIdx = 0;
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

static sal_Bool lcl_isOptionHidden( USHORT _nPageId, const SvtOptionsDialogOptions& _rOptOptions )
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

/* -----------------11.02.99 09:56-------------------
 *
 * --------------------------------------------------*/
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

/* -----------------04.05.99 15:51-------------------
 *
 * --------------------------------------------------*/
sal_Bool OfaOptionsTreeListBox::Collapse( SvLBoxEntry* pParent )
{
    bInCollapse = sal_True;
    sal_Bool bRet = SvTreeListBox::Collapse(pParent);
    bInCollapse = sal_False;
    return bRet;
}

// -----------------------------------------------------------------------

#define INI_LIST() \
    aOkPB               ( this, SVX_RES( PB_OK ) ),\
    aCancelPB           ( this, SVX_RES( PB_CANCEL ) ),\
    aHelpPB             ( this, SVX_RES( PB_HELP ) ),\
    aBackPB             ( this, SVX_RES( PB_BACK ) ),\
    aHiddenGB           ( this, SVX_RES( FB_BORDER ) ),\
    aPageTitleFT        ( this, SVX_RES( FT_PAGE_TITLE ) ),\
    aLine1FL            ( this, SVX_RES( FL_LINE_1 ) ),\
    aHelpFT             ( this, SVX_RES( FT_HELPTEXT ) ),\
    aHelpImg            ( this, SVX_RES( IMG_HELP ) ),\
    aHelpTextsArr       (       SVX_RES( STR_HELPTEXTS ) ),\
    aTreeLB             ( this, SVX_RES( TLB_PAGES ) ),\
    sTitle              ( GetText() ),\
    sNotLoadedError     (       SVX_RES( ST_LOAD_ERROR ) ),\
    pCurrentPageEntry   ( NULL ),\
    pColorPageItemSet   ( NULL ),\
    pColorTab           ( NULL ),\
    nChangeType         ( CT_NONE ),\
    nUnknownType        ( COLORPAGE_UNKNOWN ),\
    nUnknownPos         ( COLORPAGE_UNKNOWN ),\
    bIsAreaTP           ( sal_False ),\
    bForgetSelection    ( sal_False ),\
    bImageResized       ( sal_False ),\
    bInSelectHdl_Impl   ( false ),\
    bIsFromExtensionManager( false ), \
    bIsForSetDocumentLanguage( false )

// Ctor() with Frame -----------------------------------------------------
using namespace ::com::sun::star;
OfaTreeOptionsDialog::OfaTreeOptionsDialog(
    Window* pParent,
    const Reference< XFrame >& _xFrame,
    bool bActivateLastSelection ) :

    SfxModalDialog( pParent, SVX_RES( RID_OFADLG_OPTIONS_TREE ) ),
    INI_LIST()
{
    FreeResource();

    InitTreeAndHandler();
    Initialize( _xFrame );
    LoadExtensionOptions( rtl::OUString() );
    ResizeTreeLB();
    if (bActivateLastSelection)
        ActivateLastSelection();
}

// Ctor() with ExtensionId -----------------------------------------------

OfaTreeOptionsDialog::OfaTreeOptionsDialog( Window* pParent, const rtl::OUString& rExtensionId ) :

    SfxModalDialog( pParent, SVX_RES( RID_OFADLG_OPTIONS_TREE ) ),

    INI_LIST()

{
    FreeResource();

    bIsFromExtensionManager = ( rExtensionId.getLength() > 0 );
    InitTreeAndHandler();
    LoadExtensionOptions( rExtensionId );
    ResizeTreeLB();
    ActivateLastSelection();
}

/* -----------------11.02.99 07:58-------------------
 *
 * --------------------------------------------------*/

OfaTreeOptionsDialog::~OfaTreeOptionsDialog()
{
    SvLBoxEntry* pEntry = aTreeLB.First();
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

/* -----------------11.02.99 08:21-------------------
 *
 * --------------------------------------------------*/
OptionsPageInfo* OfaTreeOptionsDialog::AddTabPage(
    sal_uInt16 nId, const String& rPageName, sal_uInt16 nGroup )
{
    OptionsPageInfo* pPageInfo = new OptionsPageInfo( nId );
    SvLBoxEntry* pParent = aTreeLB.GetEntry( 0, nGroup );
    DBG_ASSERT( pParent, "OfaTreeOptionsDialog::AddTabPage(): no group found" );
    SvLBoxEntry* pEntry = aTreeLB.InsertEntry( rPageName, pParent );
    pEntry->SetUserData( pPageInfo );
    return pPageInfo;
}

/* -----------------11.02.99 10:02-------------------
 *  der ItemSet* geht in den Besitz des Dialogs
 * --------------------------------------------------*/
sal_uInt16  OfaTreeOptionsDialog::AddGroup(const String& rGroupName,
                                        SfxShell* pCreateShell,
                                        SfxModule* pCreateModule,
                                        sal_uInt16 nDialogId )
{
    SvLBoxEntry* pEntry = aTreeLB.InsertEntry(rGroupName);
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

/* -----------------11.02.99 10:31-------------------
 *
 * --------------------------------------------------*/
IMPL_LINK(OfaTreeOptionsDialog, ShowPageHdl_Impl, SvTreeListBox*, EMPTYARG)
{
    if ( aSelectTimer.GetTimeout() == SELECT_FIRST_TIMEOUT )
    {
        aSelectTimer.SetTimeout( SELECT_TIMEOUT );
        SelectHdl_Impl( NULL );
    }
    else if ( aSelectTimer.GetTimeout() == SELECT_TIMEOUT )
        aSelectTimer.Start();

    return 0;
}
/* -----------------11.02.99 10:49-------------------
 *
 * --------------------------------------------------*/
IMPL_LINK( OfaTreeOptionsDialog, BackHdl_Impl, PushButton*, EMPTYARG )
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
/* -----------------11.02.99 16:45-------------------
 *
 * --------------------------------------------------*/
IMPL_LINK( OfaTreeOptionsDialog, OKHdl_Impl, Button *, EMPTYARG )
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
                    //die Seite darf nicht verlassen werden!
                    aTreeLB.Select(pCurrentPageEntry);
                    return 0;
                }
            }
            pPageInfo->m_pPage->Hide();
        }
    }

    SvLBoxEntry* pEntry = aTreeLB.First();
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

/* -----------------17.02.99 09:15-------------------
 * Eine aufgeklappte Gruppe soll vollstaendig sichtbar sein
 * --------------------------------------------------*/
IMPL_LINK(OfaTreeOptionsDialog, ExpandedHdl_Impl, SvTreeListBox*, pBox )
{
    pBox->Update();
    pBox->InitStartEntry();
    SvLBoxEntry* pEntry = pBox->GetHdlEntry();
    if(pEntry && pBox->IsExpanded(pEntry))
    {
        sal_uInt32 nChildCount = pBox->GetChildCount( pEntry );

        SvLBoxEntry* pNext = pEntry;
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

/* -----------------11.02.99 10:49-------------------
 *
 * --------------------------------------------------*/
void OfaTreeOptionsDialog::ApplyItemSets()
{
    SvLBoxEntry* pEntry = aTreeLB.First();
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

    String sResName = String::CreateFromAscii( "iso" );
    ResMgr* pIsoRes = ResMgr::CreateResMgr( ::rtl::OUStringToOString( sResName, RTL_TEXTENCODING_UTF8 ) );
    if ( !pIsoRes )
    {
        // Fallback: Use ooo resource file
        String sOOoName = String::CreateFromAscii( "ooo" );
        pIsoRes = ResMgr::CreateResMgr( ::rtl::OUStringToOString( sOOoName, RTL_TEXTENCODING_UTF8 ) );
    }

    //! ResMgr* pIsoRes = SFX_APP()->GetLabelResManager();
    ResId aImgLstRes( RID_IMGLIST_TREEOPT, *pIsoRes );
    aImgLstRes.SetRT( RSC_IMAGELIST );
    if ( pIsoRes->IsAvailable( aImgLstRes ) )
        aPageImages = ImageList( ResId( RID_IMGLIST_TREEOPT, *pIsoRes ) );
    ResId aImgLstHCRes( RID_IMGLIST_TREEOPT_HC, *pIsoRes );
    aImgLstHCRes.SetRT( RSC_IMAGELIST );
    if ( pIsoRes->IsAvailable( aImgLstHCRes ) )
        aPageImagesHC = ImageList( ResId( RID_IMGLIST_TREEOPT_HC, *pIsoRes ) );
    delete pIsoRes;

    aTreeLB.SetHelpId( HID_OFADLG_TREELISTBOX );
    aTreeLB.SetWindowBits( WB_HASBUTTONS | WB_HASBUTTONSATROOT |
                           WB_HASLINES | WB_HASLINESATROOT |
                           WB_CLIPCHILDREN | WB_HSCROLL | WB_FORCE_MAKEVISIBLE );
    aTreeLB.SetSpaceBetweenEntries( 0 );
    aTreeLB.SetSelectionMode( SINGLE_SELECTION );
    aTreeLB.SetSublistOpenWithLeftRight( TRUE );
    aTreeLB.SetExpandedHdl( LINK( this, OfaTreeOptionsDialog, ExpandedHdl_Impl ) );
    aTreeLB.SetSelectHdl( LINK( this, OfaTreeOptionsDialog, ShowPageHdl_Impl ) );
    aBackPB.SetClickHdl( LINK( this, OfaTreeOptionsDialog, BackHdl_Impl ) );
    aOkPB.SetClickHdl( LINK( this, OfaTreeOptionsDialog, OKHdl_Impl ) );

    aHiddenGB.Show();
    aSelectTimer.SetTimeout( SELECT_FIRST_TIMEOUT );
    aSelectTimer.SetTimeoutHdl( LINK( this, OfaTreeOptionsDialog, SelectHdl_Impl ) );
}

/* -----------------17.02.99 09:51-------------------
 *
 * --------------------------------------------------*/
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

/* -----------------16.02.99 13:17-------------------
 *
 * --------------------------------------------------*/
void OfaTreeOptionsDialog::ActivateLastSelection()
{
    SvLBoxEntry* pEntry = NULL;
    if ( pLastPageSaver )
    {
        String sExpand( RTL_CONSTASCII_STRINGPARAM( EXPAND_PROTOCOL ) );
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
            Reference< XComponentContext > xContext;
            Reference< XPropertySet > xProps( ::comphelper::getProcessServiceFactory(), UNO_QUERY );
            xProps->getPropertyValue(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ) ) ) >>= xContext;
            if ( xContext.is() )
                m_xMacroExpander = Reference< com::sun::star::util::XMacroExpander >(
                    xContext->getValueByName( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                        "/singletons/com.sun.star.util.theMacroExpander" ) ) ), UNO_QUERY );
        }

        SvLBoxEntry* pTemp = aTreeLB.First();
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

    SvLBoxEntry* pParent = aTreeLB.GetParent(pEntry);
    aTreeLB.Expand(pParent);
    aTreeLB.MakeVisible(pParent);
    aTreeLB.MakeVisible(pEntry);
    aTreeLB.Select(pEntry);
    aTreeLB.GrabFocus();
}

/* -----------------22.02.99 08:52-------------------
 *
 * --------------------------------------------------*/
long    OfaTreeOptionsDialog::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        const KeyCode aKeyCode = pKEvt->GetKeyCode();

        if( aKeyCode.GetCode() == KEY_PAGEUP ||
                aKeyCode.GetCode() == KEY_PAGEDOWN)
        {
            SvLBoxEntry* pCurEntry = aTreeLB.FirstSelected();
            SvLBoxEntry*  pTemp = 0;
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

void OfaTreeOptionsDialog::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxModalDialog::DataChanged( rDCEvt );

    SvLBoxEntry* pEntry = aTreeLB.GetCurEntry();
    if ( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) &&
        !aTreeLB.GetParent(pEntry))
    {
        OptionsGroupInfo* pInfo = static_cast<OptionsGroupInfo*>(pEntry->GetUserData());
        bool bHighContrast = GetDisplayBackground().GetColor().IsDark() != 0;
        ImageList* pImgLst = bHighContrast ? &aPageImagesHC : &aPageImages;
        for ( sal_uInt16 i = 0; i < aHelpTextsArr.Count(); ++i )
        {
            if ( aHelpTextsArr.GetValue(i) == pInfo->m_nDialogId )
            {
                aHelpImg.SetImage( pImgLst->GetImage( pInfo->m_nDialogId ) );
                break;
            }
        }
    }
}
class FlagSet_Impl
{
    bool & rFlag;
    public:
        FlagSet_Impl(bool& bFlag) : rFlag(bFlag){rFlag = true;}
        ~FlagSet_Impl(){rFlag = false;}
};

IMPL_LINK( OfaTreeOptionsDialog, SelectHdl_Impl, Timer*, EMPTYARG )
{
    SvTreeListBox* pBox = &aTreeLB;
    SvLBoxEntry* pEntry = pBox->GetCurEntry();
    SvLBoxEntry* pParent = pBox->GetParent(pEntry);
    pBox->EndSelection();

    DBG_ASSERT(!bInSelectHdl_Impl, "Timeout handler called twice")
    if(bInSelectHdl_Impl || pCurrentPageEntry == pEntry)
        return 0;
    //#111938# lock the SelectHdl_Impl to prevent multiple executes
    FlagSet_Impl aFlag(bInSelectHdl_Impl);
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
            return 0;
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
    if ( pParent )
    {
        aPageTitleFT.Hide();
        aLine1FL.Hide();
        aHelpFT.Hide();
        aHelpImg.Hide();
        OptionsPageInfo *pPageInfo = (OptionsPageInfo *)pEntry->GetUserData();
        OptionsGroupInfo* pGroupInfo = (OptionsGroupInfo *)pParent->GetUserData();
        if(!pPageInfo->m_pPage && pPageInfo->m_nPageId > 0)
        {
            if(pGroupInfo->m_bLoadError)
                return 0;
            if ( RID_SVXPAGE_COLOR == pPageInfo->m_nPageId )
            {
                if(!pColorPageItemSet)
                {
                    // Move usage of a static XOutdevItemPool instance here
                    if(!mpStaticXOutdevItemPool)
                    {
                        mpStaticXOutdevItemPool = new XOutdevItemPool();
                    }
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
                        return 0;
                    }
                    if(bIdentical)
                         pGroupInfo->m_pShell = pGroupInfo->m_pModule;
                    //jetzt noch testen, ob es auch in anderen Gruppen das gleiche Module gab (z.B. Text+HTML)
                    SvLBoxEntry* pTemp = aTreeLB.First();
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

//              if ( pPageInfo->nPageId != RID_OPTPAGE_CHART_DEFCOLORS )
                {
                    if(!pGroupInfo->m_pInItemSet)
                        pGroupInfo->m_pInItemSet = pGroupInfo->m_pShell
                            ? pGroupInfo->m_pShell->CreateItemSet( pGroupInfo->m_nDialogId )
                            : CreateItemSet( pGroupInfo->m_nDialogId );
                    if(!pGroupInfo->m_pOutItemSet)
                        pGroupInfo->m_pOutItemSet = new SfxItemSet(
                            *pGroupInfo->m_pInItemSet->GetPool(),
                            pGroupInfo->m_pInItemSet->GetRanges());
                }
            }

            if(pGroupInfo->m_pModule)
            {
                pPageInfo->m_pPage = pGroupInfo->m_pModule->CreateTabPage(
                    pPageInfo->m_nPageId, this, *pGroupInfo->m_pInItemSet );
            }
            else if(RID_SVXPAGE_COLOR != pPageInfo->m_nPageId)
                pPageInfo->m_pPage = ::CreateGeneralTabPage( pPageInfo->m_nPageId, this, *pGroupInfo->m_pInItemSet );
            else
            {
                pPageInfo->m_pPage = ::CreateGeneralTabPage(
                    pPageInfo->m_nPageId, this, *pColorPageItemSet );
                SvxColorTabPage& rColPage = *(SvxColorTabPage*)pPageInfo->m_pPage;
                const OfaPtrItem* pPtr = NULL;
                if ( SfxViewFrame::Current() && SfxViewFrame::Current()->GetDispatcher() )
                    pPtr = (const OfaPtrItem*)SfxViewFrame::Current()->
                        GetDispatcher()->Execute( SID_GET_COLORTABLE, SFX_CALLMODE_SYNCHRON );
                pColorTab = pPtr ? (XColorTable*)pPtr->GetValue() : XColorTable::GetStdColorTable();

                rColPage.SetColorTable( pColorTab );
                rColPage.SetPageType( &nUnknownType );
                rColPage.SetDlgType( &nUnknownType );
                rColPage.SetPos( &nUnknownPos );
                rColPage.SetAreaTP( &bIsAreaTP );
                rColPage.SetColorChgd( (ChangeType*)&nChangeType );
                rColPage.Construct();
            }

            DBG_ASSERT( pPageInfo->m_pPage, "tabpage could not created")
            if ( pPageInfo->m_pPage )
            {
                SvtViewOptions aTabPageOpt( E_TABPAGE, String::CreateFromInt32( pPageInfo->m_nPageId ) );
                pPageInfo->m_pPage->SetUserData( GetViewOptUserItem( aTabPageOpt ) );

                Point aTreePos(aTreeLB.GetPosPixel());
                Size aTreeSize(aTreeLB.GetSizePixel());
                Point aGBPos(aHiddenGB.GetPosPixel());
                Size aPageSize(pPageInfo->m_pPage->GetSizePixel());
                Size aGBSize(aHiddenGB.GetSizePixel());
                Point aPagePos( aGBPos.X() + ( aGBSize.Width() - aPageSize.Width() ) / 2,
                                aGBPos.Y() + ( aGBSize.Height() - aPageSize.Height() ) / 2 );
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
                        C2U("com.sun.star.awt.ContainerWindowProvider") ), UNO_QUERY );
                DBG_ASSERT( m_xContainerWinProvider.is(), "service com.sun.star.awt.ContainerWindowProvider could not be loaded" );
            }

            pPageInfo->m_pExtPage = new ExtensionsTabPage(
                this, 0, pPageInfo->m_sPageURL, pPageInfo->m_sEventHdl, m_xContainerWinProvider );
            Size aSize = aHiddenGB.GetSizePixel();
            aSize.Width() = aSize.Width() - 4;
            aSize.Height() = aSize.Height() - 4;
            Point aPos = aHiddenGB.GetPosPixel();
            aPos.X() = aPos.X() + 2;
            aPos.Y() = aPos.Y() + 2;
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

        String sTmpTitle = sTitle;
        sTmpTitle += String::CreateFromAscii(" - ");
        sTmpTitle += aTreeLB.GetEntryText(pParent);
        sTmpTitle += String::CreateFromAscii(" - ");
        sTmpTitle += aTreeLB.GetEntryText(pEntry);
        SetText(sTmpTitle);
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
    }
    else
    {
        OptionsGroupInfo* pTGInfo = (OptionsGroupInfo *)pEntry->GetUserData();
        if ( pTGInfo->m_sPageURL.getLength() == 0 )
        {
            bool bHighContrast = GetDisplayBackground().GetColor().IsDark() != 0;
            ImageList* pImgLst = bHighContrast ? &aPageImagesHC : &aPageImages;
            //hier den Hilfetext anzeigen
            for ( sal_uInt16 i = 0; i < aHelpTextsArr.Count(); ++i )
            {
                if ( aHelpTextsArr.GetValue(i) == pTGInfo->m_nDialogId )
                {
                    aHelpFT.SetText(aHelpTextsArr.GetString(i));
                    aHelpImg.SetImage(pImgLst->GetImage(pTGInfo->m_nDialogId));
                    break;
                }
            }

            aPageTitleFT.Show();
            aLine1FL.Show();
            aHelpFT.Show();
            aHelpImg.Show();

            //auf die Groesse der Bitmap anpassen
            if(!bImageResized)
            {
                const long nCtrlDist = 2;
                bImageResized = sal_True;
                Point aImgPos(aHelpImg.GetPosPixel());
                Size aImgSize(aHelpImg.GetSizePixel());
                Point aTitlePos(aPageTitleFT.GetPosPixel());
                Point aLinePos(aLine1FL.GetPosPixel());
                Point aHelpPos(aHelpFT.GetPosPixel());
                Size aHelpSize(aHelpFT.GetSizePixel());
                long nXDiff = 0;
                long nYDiff = 0;
                if(aTitlePos.X() <= (aImgPos.X() + aImgSize.Width() + nCtrlDist))
                {
                    nXDiff = aImgPos.X() + aImgSize.Width() + nCtrlDist - aTitlePos.X();
                }
                if(aLinePos.Y() <= (aImgPos.Y() + aImgSize.Height() + nCtrlDist))
                {
                    nYDiff = aImgPos.Y() + aImgSize.Height() + nCtrlDist - aLinePos.Y();
                }
                aLinePos.Y() += nYDiff;
                aLine1FL.SetPosPixel(aLinePos);

                aTitlePos.X() += nXDiff;
                aPageTitleFT.SetPosPixel(aTitlePos);

                aHelpPos.X() += nXDiff;
                aHelpPos.Y() += nYDiff;
                aHelpSize.Width() -= nXDiff;
                aHelpSize.Height() -= nYDiff;
                aHelpFT.SetPosSizePixel(aHelpPos, aHelpSize);

                Font aFont = aHelpFT.GetFont();
                Size aSz = aFont.GetSize();
                aSz.Height() = (aSz.Height() * 14 ) / 10;
                aFont.SetSize(aSz);
                aPageTitleFT.SetFont(aFont);
            }

            String sTmpTitle = sTitle;
            sTmpTitle += String::CreateFromAscii(" - ");
            aPageTitleFT.SetText(aTreeLB.GetEntryText(pEntry));
            sTmpTitle += aPageTitleFT.GetText();
            SetText(sTmpTitle);
            pCurrentPageEntry = NULL;
        }
        else
        {
            if ( !pTGInfo->m_pExtPage )
            {
                if ( !m_xContainerWinProvider.is() )
                {
                    Reference < XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
                    m_xContainerWinProvider = Reference < awt::XContainerWindowProvider >(
                        xFactory->createInstance(
                            C2U("com.sun.star.awt.ContainerWindowProvider") ), UNO_QUERY );
                    DBG_ASSERT( m_xContainerWinProvider.is(), "service com.sun.star.awt.ContainerWindowProvider could not be loaded" );
                }

                pTGInfo->m_pExtPage =
                    new ExtensionsTabPage( this, 0, pTGInfo->m_sPageURL, rtl::OUString(), m_xContainerWinProvider );
                Size aSize = aHiddenGB.GetSizePixel();
                aSize.Width() = aSize.Width() - 4;
                aSize.Height() = aSize.Height() - 4;
                Point aPos = aHiddenGB.GetPosPixel();
                aPos.X() = aPos.X() + 2;
                aPos.Y() = aPos.Y() + 2;
                pTGInfo->m_pExtPage->SetPosSizePixel( aPos, aSize );
            }

            if ( pTGInfo->m_pExtPage )
            {
                pTGInfo->m_pExtPage->Show();
                pTGInfo->m_pExtPage->ActivatePage();
            }

            pCurrentPageEntry = pEntry;
        }
    }

    // restore lost focus, if necessary
    Window* pFocusWin = Application::GetFocusWindow();
    // if the focused window is not the options treebox and the old page has the focus
    if ( pFocusWin && pFocusWin != pBox && pOldPage && pOldPage->HasChildPathFocus() )
        // then set the focus to the new page or if we are on a group set the focus to the options treebox
        pNewPage ? pNewPage->GrabFocus() : pBox->GrabFocus();

    return 0;
}

/* -----------------11.02.99 15:51-------------------
 *
 * --------------------------------------------------*/

OfaPageResource::OfaPageResource() :
    Resource(SVX_RES(RID_OFADLG_OPTIONS_TREE_PAGES)),
    aGeneralDlgAry(SVX_RES(SID_GENERAL_OPTIONS)),
    aInetDlgAry(SVX_RES(SID_INET_DLG)),
    aLangDlgAry(SVX_RES(SID_LANGUAGE_OPTIONS)),
    aTextDlgAry(SVX_RES(SID_SW_EDITOPTIONS)),
    aHTMLDlgAry(SVX_RES(SID_SW_ONLINEOPTIONS)),
    aCalcDlgAry(SVX_RES(SID_SC_EDITOPTIONS)),
    aStarMathDlgAry(SVX_RES(SID_SM_EDITOPTIONS)),
    aImpressDlgAry(SVX_RES(SID_SD_EDITOPTIONS)),
    aDrawDlgAry(SVX_RES(SID_SD_GRAPHIC_OPTIONS)),
    aChartDlgAry(SVX_RES(SID_SCH_EDITOPTIONS)),
    aFilterDlgAry(SVX_RES(SID_FILTER_DLG)),
    aDatasourcesDlgAry(SVX_RES(SID_SB_STARBASEOPTIONS))
{
    FreeResource();
}

BOOL EnableSSO( void )
{
    // SSO must be enabled if the configuration manager bootstrap settings
    // are configured as follows ...
    //  CFG_Offline=false
    //  CFG_ServerType=uno ( or unspecified )
    //  CFG_BackendService=
    //   com.sun.star.comp.configuration.backend.LdapSingleBackend

    rtl::OUString theIniFile;
    osl_getExecutableFile( &theIniFile.pData );
    theIniFile = theIniFile.copy( 0, theIniFile.lastIndexOf( '/' ) + 1 ) +
                 rtl::OUString::createFromAscii( SAL_CONFIGFILE( "configmgr" ) );
    ::rtl::Bootstrap theBootstrap( theIniFile );

    rtl::OUString theOfflineValue;
    rtl::OUString theDefaultOfflineValue = rtl::OUString::createFromAscii( "false" );
    theBootstrap.getFrom( rtl::OUString::createFromAscii( "CFG_Offline" ),
                          theOfflineValue,
                          theDefaultOfflineValue );

    rtl::OUString theServerTypeValue;
    theBootstrap.getFrom( rtl::OUString::createFromAscii( "CFG_ServerType" ),
                          theServerTypeValue );

    rtl::OUString theBackendServiceTypeValue;
    theBootstrap.getFrom( rtl::OUString::createFromAscii( "CFG_BackendService" ),
                          theBackendServiceTypeValue );

    BOOL bSSOEnabled =
        ( theOfflineValue == theDefaultOfflineValue                     &&
          ( theServerTypeValue.getLength() == 0 ||
          theServerTypeValue == rtl::OUString::createFromAscii( "uno" ) ) &&
          theBackendServiceTypeValue ==
            rtl::OUString::createFromAscii(
                "com.sun.star.comp.configuration.backend.LdapSingleBackend" ) );
    if ( bSSOEnabled && GetSSOCreator() == 0 )
    {
        bSSOEnabled = FALSE;
    }
    return bSSOEnabled;
}

extern "C" { static void SAL_CALL thisModule() {} }

CreateTabPage GetSSOCreator( void )
{
    static CreateTabPage theSymbol = 0;
    if ( theSymbol == 0 )
    {
        osl::Module aModule;
        rtl::OUString theModuleName( RTL_CONSTASCII_USTRINGPARAM( SVLIBRARY( "ssoopt" ) ) );
        if( aModule.loadRelative(
                &thisModule, theModuleName, SAL_LOADMODULE_DEFAULT ) )
        {
            rtl::OUString theSymbolName( rtl::OUString::createFromAscii( "CreateSSOTabPage" ) );
            theSymbol = reinterpret_cast<CreateTabPage>(aModule.getFunctionSymbol( theSymbolName ));
        }
    }

    return theSymbol;
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
                SID_AUTOSPELL_CHECK, SID_AUTOSPELL_MARKOFF,
                SID_ATTR_QUICKLAUNCHER, SID_ATTR_QUICKLAUNCHER,
                SID_ATTR_YEAR2000, SID_ATTR_YEAR2000,
                SID_HTML_MODE, SID_HTML_MODE,
                0 );

            SfxItemSet aOptSet( SFX_APP()->GetPool(), SID_ATTR_QUICKLAUNCHER, SID_ATTR_QUICKLAUNCHER );
            SFX_APP()->GetOptions(aOptSet);
            pRet->Put(aOptSet);

            SfxMiscCfg* pMisc = SFX_APP()->GetMiscConfig();
            const SfxPoolItem* pItem;
            SfxViewFrame* pViewFrame = SfxViewFrame::Current();
            if ( pViewFrame )
            {
                SfxDispatcher* pDispatch = pViewFrame->GetDispatcher();

                // Sonstiges - Year2000
                if( SFX_ITEM_AVAILABLE <= pDispatch->QueryState( SID_ATTR_YEAR2000, pItem ) )
                    pRet->Put( SfxUInt16Item( SID_ATTR_YEAR2000, ((const SfxUInt16Item*)pItem)->GetValue() ) );
                else
                    pRet->Put( SfxUInt16Item( SID_ATTR_YEAR2000, (USHORT)pMisc->GetYear2000() ) );
            }
            else
                pRet->Put( SfxUInt16Item( SID_ATTR_YEAR2000, (USHORT)pMisc->GetYear2000() ) );


            // Sonstiges - Tabulator
            pRet->Put(SfxBoolItem(SID_PRINTER_NOTFOUND_WARN, pMisc->IsNotFoundWarning()));

            sal_uInt16 nFlag = pMisc->IsPaperSizeWarning() ? SFX_PRINTER_CHG_SIZE : 0;
            nFlag  |= pMisc->IsPaperOrientationWarning()  ? SFX_PRINTER_CHG_ORIENTATION : 0;
            pRet->Put( SfxFlagItem( SID_PRINTER_CHANGESTODOC, nFlag ));

        }
        break;
        case SID_LANGUAGE_OPTIONS :
        {
            pRet = new SfxItemSet(SFX_APP()->GetPool(),
                    SID_ATTR_LANGUAGE, SID_AUTOSPELL_MARKOFF,
                    SID_ATTR_CHAR_CJK_LANGUAGE, SID_ATTR_CHAR_CTL_LANGUAGE,
                    SID_OPT_LOCALE_CHANGED, SID_OPT_LOCALE_CHANGED,
                    SID_SET_DOCUMENT_LANGUAGE, SID_SET_DOCUMENT_LANGUAGE,
                    0 );

            // fuer die Linguistik

            Reference< XSpellChecker1 >  xSpell = SvxGetSpellChecker();
            pRet->Put(SfxSpellCheckItem( xSpell, SID_ATTR_SPELL ));
            SfxHyphenRegionItem aHyphen( SID_ATTR_HYPHENREGION );

            sal_Int16   nMinLead  = 2,
                        nMinTrail = 2;
            if (xProp.is())
            {
                xProp->getPropertyValue( String::CreateFromAscii(
                        UPN_HYPH_MIN_LEADING) ) >>= nMinLead;
                xProp->getPropertyValue( String::CreateFromAscii(
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
                            xProp->getPropertyValue( String::CreateFromAscii( UPN_IS_SPELL_AUTO) ) >>= bVal;
                        }

                        pRet->Put(SfxBoolItem(SID_AUTOSPELL_CHECK, bVal));
                }

                if(SFX_ITEM_AVAILABLE <= pDispatch->QueryState(SID_AUTOSPELL_MARKOFF, pItem))
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
                        xProp->getPropertyValue( String::CreateFromAscii( UPN_IS_SPELL_HIDE) ) >>= bVal;
                    }
                    pRet->Put(SfxBoolItem(SID_AUTOSPELL_MARKOFF, bVal));
                }
            }
            pRet->Put( SfxBoolItem( SID_SET_DOCUMENT_LANGUAGE, bIsForSetDocumentLanguage ) );
        }
        break;
        case SID_INET_DLG :
                pRet = new SfxItemSet( SFX_APP()->GetPool(),
                                SID_BASIC_ENABLED, SID_BASIC_ENABLED,
                //SID_OPTIONS_START - ..END
                                SID_OPTIONS_START, SID_INET_PROXY_PORT,
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
            SfxMiscCfg* pMisc = SFX_APP()->GetMiscConfig();
            const SfxPoolItem* pItem;
            SfxItemSet aOptSet(SFX_APP()->GetPool(), SID_ATTR_QUICKLAUNCHER, SID_ATTR_QUICKLAUNCHER );
            aOptSet.Put(rSet);
            if(aOptSet.Count())
                SFX_APP()->SetOptions( aOptSet );
            // Dispatcher neu holen, weil SetOptions() ggf. den Dispatcher zerst"ort hat
            SfxViewFrame *pViewFrame = SfxViewFrame::Current();
// -------------------------------------------------------------------------
//          Year2000 auswerten
// -------------------------------------------------------------------------
            USHORT nY2K = USHRT_MAX;
            if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_YEAR2000, sal_False, &pItem ) )
                nY2K = ((const SfxUInt16Item*)pItem)->GetValue();
            if( USHRT_MAX != nY2K )
            {
                if ( pViewFrame )
                {
                    SfxDispatcher* pDispatch = pViewFrame->GetDispatcher();
                    pDispatch->Execute( SID_ATTR_YEAR2000, SFX_CALLMODE_ASYNCHRON, pItem, 0L);
                }
                pMisc->SetYear2000(nY2K);
            }

// -------------------------------------------------------------------------
//          Drucken auswerten
// -------------------------------------------------------------------------
            if(SFX_ITEM_SET == rSet.GetItemState(SID_PRINTER_NOTFOUND_WARN, sal_False, &pItem))
                pMisc->SetNotFoundWarning(((const SfxBoolItem*)pItem)->GetValue());

            if(SFX_ITEM_SET == rSet.GetItemState(SID_PRINTER_CHANGESTODOC, sal_False, &pItem))
            {
                const SfxFlagItem* pFlag = (const SfxFlagItem*)pItem;
                pMisc->SetPaperSizeWarning(0 != (pFlag->GetValue() &  SFX_PRINTER_CHG_SIZE ));
                pMisc->SetPaperOrientationWarning(0 !=  (pFlag->GetValue() & SFX_PRINTER_CHG_ORIENTATION ));
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
            DBG_ERROR( "Unhandled option in ApplyItemSet" );
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
            xMgr->createInstance( ::rtl::OUString::createFromAscii(
                    "com.sun.star.linguistic2.LinguProperties") ),
            UNO_QUERY );
    if ( SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_HYPHENREGION, sal_False, &pItem ) )
    {
        const SfxHyphenRegionItem* pHyphenItem = (const SfxHyphenRegionItem*)pItem;

        if (xProp.is())
        {
            xProp->setPropertyValue(
                    String::CreateFromAscii(UPN_HYPH_MIN_LEADING),
                    makeAny((sal_Int16) pHyphenItem->GetMinLead()) );
            xProp->setPropertyValue(
                    String::CreateFromAscii(UPN_HYPH_MIN_TRAILING),
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
                        String::CreateFromAscii(UPN_IS_SPELL_AUTO),
                        makeAny(bOnlineSpelling) );
            }
        }

        if( SFX_ITEM_SET == rSet.GetItemState(SID_AUTOSPELL_MARKOFF, sal_False, &pItem ))
        {
            sal_Bool bHideSpell = ((const SfxBoolItem*)pItem)->GetValue();
            pDispatch->Execute(SID_AUTOSPELL_MARKOFF, SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD, pItem, 0L);

            if (xProp.is())
            {
                xProp->setPropertyValue(
                        String::CreateFromAscii(UPN_IS_SPELL_HIDE),
                        makeAny(bHideSpell) );
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
            _pViewFrame = SfxViewFrame::GetNext( *pViewFrame );
        }
    }
}

rtl::OUString getCurrentFactory_Impl( const Reference< XFrame >& _xFrame )
{
    rtl::OUString sIdentifier;
    Reference < XFrame > xCurrentFrame( _xFrame );
    Reference < XModuleManager > xModuleManager( ::comphelper::getProcessServiceFactory()->createInstance(
        DEFINE_CONST_UNICODE("com.sun.star.frame.ModuleManager") ), UNO_QUERY );
    if ( !xCurrentFrame.is() )
    {
        Reference< XDesktop > xDesktop( ::comphelper::getProcessServiceFactory()->createInstance(
            DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );
        if ( xDesktop.is() )
            xCurrentFrame = xDesktop->getCurrentFrame();
    }

    if ( xCurrentFrame.is() && xModuleManager.is() )
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
            DBG_ERRORFILE( "getActiveModule_Impl(): exception of XModuleManager::identify()" );
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
    BOOL isSSOEnabled = EnableSSO();
    if ( !lcl_isOptionHidden( SID_GENERAL_OPTIONS, aOptionsDlgOpt ) )
    {
        ResStringArray& rGeneralArray = aDlgResource.GetGeneralArray();
        setGroupName( C2U("ProductName"), rGeneralArray.GetString(0) );
        nGroup = AddGroup( rGeneralArray.GetString(0), 0, 0, SID_GENERAL_OPTIONS );
        sal_uInt16 nEnd = static_cast< sal_uInt16 >( rGeneralArray.Count() );

        for ( i = 1; i < nEnd; ++i )
        {
            nPageId = (sal_uInt16)rGeneralArray.GetValue(i);
            if ( lcl_isOptionHidden( nPageId, aOptionsDlgOpt ) )
                continue;

            // Disable Online Update page if service not installed
            if( nPageId == RID_SVXPAGE_ONLINEUPDATE )
            {
                Reference < XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
                Reference < XInterface > xService( xFactory->createInstance(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.setup.UpdateCheck"))) );

                if( ! xService.is() )
                    continue;
            }

            if ( nPageId != RID_SVXPAGE_SSO || isSSOEnabled )
                AddTabPage( nPageId, rGeneralArray.GetString(i), nGroup );
        }
    }

    // Load and Save options
    if ( !lcl_isOptionHidden( SID_FILTER_DLG, aOptionsDlgOpt ) )
    {
        ResStringArray& rFilterArray = aDlgResource.GetFilterArray();
        setGroupName( C2U("LoadSave"), rFilterArray.GetString(0) );
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
        setGroupName( C2U("LanguageSettings"), rLangArray.GetString(0) );
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
    sal_Bool bHasAnyFilter = sal_False;
    SvtModuleOptions aModuleOpt;
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
    {
        // Textdokument
        bHasAnyFilter = sal_True;
        ResStringArray& rTextArray = aDlgResource.GetTextArray();
        if (   aFactory.equalsAscii( "com.sun.star.text.TextDocument" )
            || aFactory.equalsAscii( "com.sun.star.text.WebDocument" )
            || aFactory.equalsAscii( "com.sun.star.text.GlobalDocument" ) )
        {
            SfxModule* pSwMod = (*(SfxModule**) GetAppData(SHL_WRITER));
            if ( !lcl_isOptionHidden( SID_SW_EDITOPTIONS, aOptionsDlgOpt ) )
            {
                if ( aFactory.equalsAscii( "com.sun.star.text.WebDocument" ) )
                    setGroupName( C2U("WriterWeb"), rTextArray.GetString(0) );
                else
                    setGroupName( C2U("Writer"), rTextArray.GetString(0) );
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
#ifndef PRODUCT
                AddTabPage( RID_SW_TP_OPTTEST_PAGE, String::CreateFromAscii("Interner Test"), nGroup );
#endif
            }

            // HTML-Dokument
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
#ifndef PRODUCT
                AddTabPage( RID_SW_TP_OPTTEST_PAGE, String::CreateFromAscii("Interner Test"), nGroup );
#endif
            }
        }
    }

    // Calc options
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC ) )
    {
        bHasAnyFilter = sal_True;
        if ( aFactory.equalsAscii( "com.sun.star.sheet.SpreadsheetDocument" ))
        {
            if ( !lcl_isOptionHidden( SID_SC_EDITOPTIONS, aOptionsDlgOpt ) )
            {
                ResStringArray& rCalcArray = aDlgResource.GetCalcArray();
                SfxModule* pScMod = ( *( SfxModule** ) GetAppData( SHL_CALC ) );
                setGroupName( C2U("Calc"), rCalcArray.GetString(0) );
                nGroup = AddGroup( rCalcArray.GetString( 0 ), pScMod, pScMod, SID_SC_EDITOPTIONS );
                const USHORT nCount = static_cast< const USHORT >( rCalcArray.Count() );
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
        bHasAnyFilter = sal_True;
        if ( aFactory.equalsAscii( "com.sun.star.presentation.PresentationDocument" ))
        {
            if ( !lcl_isOptionHidden( SID_SD_EDITOPTIONS, aOptionsDlgOpt ) )
            {
                ResStringArray& rImpressArray = aDlgResource.GetImpressArray();
                setGroupName( C2U("Impress"), rImpressArray.GetString(0) );
                nGroup = AddGroup( rImpressArray.GetString( 0 ), pSdMod, pSdMod, SID_SD_EDITOPTIONS );
                const sal_Bool bCTL = aLanguageOptions.IsCTLFontEnabled();
                const USHORT nCount = static_cast< const USHORT >( rImpressArray.Count() );
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
        if ( aFactory.equalsAscii( "com.sun.star.drawing.DrawingDocument" ))
        {
            if ( !lcl_isOptionHidden( SID_SD_GRAPHIC_OPTIONS, aOptionsDlgOpt ) )
            {
                ResStringArray& rDrawArray = aDlgResource.GetDrawArray();
                setGroupName( C2U("Draw"), rDrawArray.GetString(0) );
                nGroup = AddGroup( rDrawArray.GetString( 0 ), pSdMod, pSdMod, SID_SD_GRAPHIC_OPTIONS );
                const sal_Bool bCTL = aLanguageOptions.IsCTLFontEnabled();
                const USHORT nCount = static_cast< const USHORT >( rDrawArray.Count() );
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
        if ( aFactory.equalsAscii( "com.sun.star.formula.FormulaProperties" ))
        {
            if ( !lcl_isOptionHidden( SID_SM_EDITOPTIONS, aOptionsDlgOpt ) )
            {
                ResStringArray& rStarMathArray = aDlgResource.GetStarMathArray();
                SfxModule* pSmMod = (*(SfxModule**) GetAppData(SHL_SM));
                setGroupName( C2U("Math"), rStarMathArray.GetString(0) );
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
        setGroupName( C2U("Base"), rDSArray.GetString(0) );
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
        setGroupName( C2U("Charts"), rChartArray.GetString(0) );
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
        setGroupName( C2U("Internet"), rInetArray.GetString(0) );
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
    const long  nMax = aHiddenGB.GetSizePixel().Width() * 42 / 100;
                                            // don't ask where 42 comes from... but it looks / feels ok ;-)
    long        nDelta = 50;                // min.
    USHORT      nDepth = 0;
    const long  nIndent0 = PixelToLogic( Size( 28, 0 ) ).Width();
    const long  nIndent1 = PixelToLogic( Size( 52, 0 ) ).Width();

    SvTreeList*         pTreeList = aTreeLB.GetModel();
    DBG_ASSERT( pTreeList, "-OfaTreeOptionsDialog::ResizeTreeLB(): no model, no cookies!" );

    SvListEntry*        pEntry = pTreeList->First();
    while( pEntry )
    {
        long n = aTreeLB.GetTextWidth( aTreeLB.GetEntryText( static_cast< SvLBoxEntry* >( pEntry ) ) );
        n += ( nDepth == 0 )? nIndent0 : nIndent1;

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
    MoveControl( aHiddenGB, nDelta );
    MoveControl( aPageTitleFT, nDelta );
    MoveControl( aLine1FL, nDelta );
    MoveControl( aHelpFT, nDelta );
    MoveControl( aHelpImg, nDelta );
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
    Reference< XMultiServiceFactory > xMSFac = comphelper::getProcessServiceFactory();
    // open optionsdialog.xcu
    Reference< XNameAccess > xRoot(
        ::comphelper::ConfigurationHelper::openConfig(
            xMSFac, C2U("org.openoffice.Office.OptionsDialog"),
            ::comphelper::ConfigurationHelper::E_READONLY ), UNO_QUERY );
    DBG_ASSERT( xRoot.is(), "OfaTreeOptionsDialog::LoadExtensionOptions(): no config" );
    // when called by Tools - Options then load nodes of active module
    if ( rExtensionId.getLength() == 0 )
        pModule = LoadModule( GetModuleIdentifier( xMSFac, Reference< XFrame >() ), xRoot );

    VectorOfNodes aNodeList;
    LoadNodes( xRoot, pModule, rExtensionId, aNodeList );
    InsertNodes( aNodeList );
}

rtl::OUString OfaTreeOptionsDialog::GetModuleIdentifier(
    const Reference< XMultiServiceFactory >& xMFac, const Reference< XFrame >& rFrame )
{
    rtl::OUString sModule;
    Reference < XFrame > xCurrentFrame( rFrame );
    Reference < XModuleManager > xModuleManager( xMFac->createInstance(
        C2U("com.sun.star.frame.ModuleManager") ), UNO_QUERY );

    if ( !xCurrentFrame.is() )
    {
        Reference < XDesktop > xDesktop( xMFac->createInstance(
            C2U("com.sun.star.frame.Desktop") ), UNO_QUERY );
        if ( xDesktop.is() )
            xCurrentFrame = xDesktop->getCurrentFrame();
    }

    if ( xCurrentFrame.is() && xModuleManager.is() )
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
            DBG_ERRORFILE( "OfaTreeOptionsDialog::GetModuleIdentifier(): exception of XModuleManager::identify()" );
        }
    }
    return sModule;
}

Module* OfaTreeOptionsDialog::LoadModule(
    const rtl::OUString& rModuleIdentifier, const Reference< XNameAccess >& xRoot )
{
    Module* pModule = NULL;
    Reference< XNameAccess > xSet;

    if ( xRoot->hasByName( C2U("Modules") ) )
    {
        xRoot->getByName( C2U("Modules") ) >>= xSet;
        if ( xSet.is() )
        {
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
                        xModAccess->getByName( C2U("Nodes") ) >>= xNodeAccess;
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
                                    xAccess->getByName( C2U("Index") ) >>= nIndex;
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
        }
    }
    return pModule;
}

void OfaTreeOptionsDialog::LoadNodes(
    const Reference< XNameAccess >& xRoot, Module* pModule,
    const rtl::OUString& rExtensionId, VectorOfNodes& rOutNodeList )
{
    Reference< XNameAccess > xSet;
    if ( xRoot->hasByName( C2U("Nodes") ) )
    {
        xRoot->getByName( C2U("Nodes") ) >>= xSet;
        if ( xSet.is() )
        {
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
                    xNodeAccess->getByName( C2U("Label") ) >>= sLabel;
                    xNodeAccess->getByName( C2U("OptionsPage") ) >>= sPageURL;
                    xNodeAccess->getByName( C2U("AllModules") ) >>= bAllModules;
                    xNodeAccess->getByName( C2U("GroupId") ) >>= sGroupId;
                    xNodeAccess->getByName( C2U("GroupIndex") ) >>= nGroupIndex;

                    if ( sLabel.getLength() == 0 )
                        sLabel = sGroupName;
                    String sTemp = getGroupName( sLabel, rExtensionId.getLength() > 0 );
                    if ( sTemp.Len() > 0 )
                        sLabel = sTemp;
                    OptionsNode* pNode =
                        new OptionsNode( sNodeId, sLabel, sPageURL, bAllModules, sGroupId, nGroupIndex );

                    if ( !rExtensionId.getLength() && !isNodeActive( pNode, pModule ) )
                    {
                        delete pNode;
                        continue;
                    }

                    Reference< XNameAccess > xLeavesSet;
                    xNodeAccess->getByName( C2U( "Leaves" ) ) >>= xLeavesSet;
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

                                xLeaveAccess->getByName( C2U("Id") ) >>= sId;
                                xLeaveAccess->getByName( C2U("Label") ) >>= sLeafLabel;
                                xLeaveAccess->getByName( C2U("OptionsPage") ) >>= sLeafURL;
                                xLeaveAccess->getByName( C2U("EventHandlerService") ) >>= sEventHdl;
                                xLeaveAccess->getByName( C2U("GroupId") ) >>= sLeafGrpId;
                                xLeaveAccess->getByName( C2U("GroupIndex") ) >>= nLeafGrpIdx;

                                if ( !rExtensionId.getLength() || sId == rExtensionId )
                                {
                                    OptionsLeaf* pLeaf = new OptionsLeaf(
                                        sId, sLeafLabel, sLeafURL, sEventHdl, sLeafGrpId, nLeafGrpIdx );

                                    if ( sLeafGrpId.getLength() > 0 )
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
                        pModule ? aNodeList.push_back( pNode ) : rOutNodeList.push_back( pNode );
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
                            rOutNodeList.push_back( pNode );
                            aNodeList.erase( aNodeList.begin() + j );
                            break;
                        }
                    }
                }

                for ( i = 0; i < aNodeList.size(); ++i )
                    rOutNodeList.push_back( aNodeList[i] );
            }
        }
    }
}

USHORT lcl_getGroupId( const rtl::OUString& rGroupName, const SvTreeListBox& rTreeLB )
{
    String sGroupName( rGroupName );
    USHORT nRet = 0;
    SvLBoxEntry* pEntry = rTreeLB.First();
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

void lcl_insertLeaf(
    OfaTreeOptionsDialog* pDlg, OptionsNode* pNode, OptionsLeaf* pLeaf, const SvTreeListBox& rTreeLB )
{
    USHORT nGrpId = lcl_getGroupId( pNode->m_sLabel, rTreeLB );
    if ( USHRT_MAX == nGrpId )
    {
        USHORT nNodeGrpId = getGroupNodeId( pNode->m_sId );
        nGrpId = pDlg->AddGroup( pNode->m_sLabel, NULL, NULL, nNodeGrpId );
        if ( pNode->m_sPageURL.getLength() > 0 )
        {
            SvLBoxEntry* pGrpEntry = rTreeLB.GetEntry( 0, nGrpId );
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
        if( GetColorTable() )
        {
            GetColorTable()->Save();

            // notify current viewframe it it uses the same color table
            if ( SfxViewFrame::Current() && SfxViewFrame::Current()->GetDispatcher() )
            {
                const OfaPtrItem* pPtr = (const OfaPtrItem*)SfxViewFrame::Current()->GetDispatcher()->Execute( SID_GET_COLORTABLE, SFX_CALLMODE_SYNCHRON );
                if( pPtr )
                {
                    XColorTable* _pColorTab = (XColorTable*)pPtr->GetValue();

                    if( _pColorTab &&
                        _pColorTab->GetPath() == GetColorTable()->GetPath() &&
                        _pColorTab->GetName() == GetColorTable()->GetName() )
                        SfxObjectShell::Current()->PutItem( SvxColorTableItem( GetColorTable(), SID_COLOR_TABLE ) );
                }
            }
        }

        utl::ConfigManager::GetConfigManager()->StoreConfigItems();
    }

    return nRet;
}

// class ExtensionsTabPage -----------------------------------------------

ExtensionsTabPage::ExtensionsTabPage(
    Window* pParent, const ResId& rResId, const rtl::OUString& rPageURL,
    const rtl::OUString& rEvtHdl, const Reference< awt::XContainerWindowProvider >& rProvider ) :

    TabPage( pParent, rResId ),

    m_sPageURL          ( rPageURL ),
    m_sEventHdl         ( rEvtHdl ),
    m_xWinProvider      ( rProvider ),
    m_bIsWindowHidden   ( false )

{
}

// -----------------------------------------------------------------------

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
        bool bWithHandler = ( m_sEventHdl.getLength() > 0 );
        if ( bWithHandler )
        {
            Reference < XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
            m_xEventHdl = Reference< awt::XContainerWindowEventHandler >(
                xFactory->createInstance( m_sEventHdl ), UNO_QUERY );
        }

        if ( !bWithHandler || m_xEventHdl.is() )
        {
            Reference< awt::XWindowPeer > xParent( VCLUnoHelper::GetInterface( this ), UNO_QUERY );
            m_xPage = Reference < awt::XWindow >(
                m_xWinProvider->createContainerWindow(
                    m_sPageURL, rtl::OUString(), xParent, m_xEventHdl ), UNO_QUERY );
        }
    }
    catch ( ::com::sun::star::lang::IllegalArgumentException& )
    {
        DBG_ERRORFILE( "ExtensionsTabPage::CreateDialogWithHandler(): illegal argument" );
    }
    catch ( Exception& )
    {
        DBG_ERRORFILE( "ExtensionsTabPage::CreateDialogWithHandler(): exception of XDialogProvider2::createDialogWithHandler()" );
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
            bRet = m_xEventHdl->callHandlerMethod( m_xPage, makeAny( rAction ), C2U("external_event") );
        }
        catch ( Exception& )
        {
            DBG_ERRORFILE( "ExtensionsTabPage::DispatchAction(): exception of XDialogEventHandler::callHandlerMethod()" );
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
            if ( m_sEventHdl.getLength() > 0 )
                DispatchAction( C2U("initialize") );
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
    DispatchAction( C2U("back") );
    ActivatePage();
}

// -----------------------------------------------------------------------

void ExtensionsTabPage::SavePage()
{
    DispatchAction( C2U("ok") );
}

// -----------------------------------------------------------------------

void ExtensionsTabPage::HideWindow()
{
    if ( !m_bIsWindowHidden && m_xPage.is() )
    {
        m_xPage->setVisible( sal_False );
        m_bIsWindowHidden = true;
    }
}

