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

#include <config_folders.h>
#include <sfx2/sfxhelp.hxx>

#include <set>
#include <algorithm>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/UnknownModuleException.hpp>
#include <com/sun/star/frame/XFrame2.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <svtools/helpopt.hxx>
#include <unotools/moduleoptions.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/pathoptions.hxx>
#include <rtl/ustring.hxx>
#include <officecfg/Office/Common.hxx>
#include <osl/process.h>
#include <osl/file.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/tempfile.hxx>
#include <rtl/uri.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/layout.hxx>
#include <vcl/weld.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/sfxecode.hxx>
#include <openuriexternally.hxx>

#include <comphelper/lok.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <sfx2/viewsh.hxx>

#include "newhelp.hxx"
#include <sfx2/objsh.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/sfxresid.hxx>
#include <helper.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/sfxuno.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/frame.hxx>
#include <rtl/string.hxx>
#include <svtools/langtab.hxx>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::system;

class NoHelpErrorBox
{
private:
    std::unique_ptr<weld::MessageDialog> m_xErrBox;
public:
    DECL_STATIC_LINK(NoHelpErrorBox, HelpRequestHdl, weld::Widget&, bool);
public:
    explicit NoHelpErrorBox(weld::Widget* pParent)
        : m_xErrBox(Application::CreateMessageDialog(pParent, VclMessageType::Error, VclButtonsType::Ok,
                                                     SfxResId(RID_STR_HLPFILENOTEXIST)))
    {
        // Error message: "No help available"
        m_xErrBox->connect_help(LINK(nullptr, NoHelpErrorBox, HelpRequestHdl));
    }
    void run()
    {
        m_xErrBox->run();
    }
};

IMPL_STATIC_LINK_NOARG(NoHelpErrorBox, HelpRequestHdl, weld::Widget&, bool)
{
    // do nothing, because no help available
    return false;
}

static OUString HelpLocaleString();

namespace {

/// Root path of the help.
OUString const & getHelpRootURL()
{
    static OUString s_instURL;
    if (!s_instURL.isEmpty())
        return s_instURL;

    s_instURL = officecfg::Office::Common::Path::Current::Help::get(comphelper::getProcessComponentContext());
    if (s_instURL.isEmpty())
    {
        // try to determine path from default
        s_instURL = "$(instpath)/" LIBO_SHARE_HELP_FOLDER;
    }

    // replace anything like $(instpath);
    SvtPathOptions aOptions;
    s_instURL = aOptions.SubstituteVariable(s_instURL);

    OUString url;
    if (osl::FileBase::getFileURLFromSystemPath(s_instURL, url) == osl::FileBase::E_None)
        s_instURL = url;

    return s_instURL;
}

bool impl_checkHelpLocalePath(OUString const & rpPath)
{
    osl::DirectoryItem directoryItem;
    bool bOK = false;

    osl::FileStatus fileStatus(osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileURL | osl_FileStatus_Mask_FileName);
    if (osl::DirectoryItem::get(rpPath, directoryItem) == osl::FileBase::E_None &&
        directoryItem.getFileStatus(fileStatus) == osl::FileBase::E_None &&
        fileStatus.isDirectory())
    {
        bOK = true;
    }
    return bOK;
}

/// Check for built-in help
/// Check if help/<lang>/err.html file exist
bool impl_hasHelpInstalled()
{
    if (comphelper::LibreOfficeKit::isActive())
        return false;

    static OUString aLocaleStr;

    if (aLocaleStr.isEmpty())
    {
        // detect installed locale
        aLocaleStr = HelpLocaleString();
    }

    OUString helpRootURL = getHelpRootURL() + "/" + aLocaleStr + "/err.html";
    bool bOK = false;
    osl::DirectoryItem directoryItem;
    if(osl::DirectoryItem::get(helpRootURL, directoryItem) == osl::FileBase::E_None){
        bOK=true;
    }

    SAL_INFO( "sfx.appl", "Checking old help installed " << bOK);
    return bOK;
}

/// Check for html built-in help
/// Check if help/lang/text folder exist. Only html has it.
bool impl_hasHTMLHelpInstalled()
{
    if (comphelper::LibreOfficeKit::isActive())
        return false;

    static OUString aLocaleStr;

    if (aLocaleStr.isEmpty())
    {
        // detect installed locale
        aLocaleStr = HelpLocaleString();
    }

    OUString helpRootURL = getHelpRootURL() + "/" + aLocaleStr + "/text";
    bool bOK = impl_checkHelpLocalePath( helpRootURL );
    SAL_INFO( "sfx.appl", "Checking new help (html) installed " << bOK);
    return bOK;
}

} // namespace

/// Return the locale we prefer for displaying help
// static OUString const & HelpLocaleString()
static OUString  HelpLocaleString()
{
    if (comphelper::LibreOfficeKit::isActive())
        return comphelper::LibreOfficeKit::getLanguageTag().getBcp47();

    static OUString aLocaleStr;
    if (aLocaleStr.isEmpty())
    {
        const OUString aEnglish("en-US");
        // detect installed locale
        aLocaleStr = utl::ConfigManager::getLocale();

        if ( aLocaleStr.isEmpty() )
            aLocaleStr = aEnglish;
        else
        {
            // get fall-back language (country)
            OUString sLang = aLocaleStr ;
            sal_Int32 nSepPos = sLang.indexOf( '-' );
            if (nSepPos != -1)
            {
                sLang = sLang.copy( 0, nSepPos );
            }
            OUString sHelpPath("");
            sHelpPath = getHelpRootURL() + "/" + utl::ConfigManager::getProductVersion() + "/" + aLocaleStr;
            if (impl_checkHelpLocalePath(sHelpPath))
            {
                return aLocaleStr;
            }
            sHelpPath = getHelpRootURL() + "/" + utl::ConfigManager::getProductVersion() + "/" + sLang;
            if (impl_checkHelpLocalePath(sHelpPath))
            {
                aLocaleStr = sLang;
                return aLocaleStr;
            }
            sHelpPath = getHelpRootURL() + "/" + aLocaleStr;
            if (impl_checkHelpLocalePath(sHelpPath))
            {
                return aLocaleStr;
            }
            sHelpPath = getHelpRootURL() + "/" + sLang;
            if (impl_checkHelpLocalePath(sHelpPath))
            {
                aLocaleStr = sLang;
                return aLocaleStr;
            }

            sHelpPath = getHelpRootURL() + "/" + utl::ConfigManager::getProductVersion() + "/" + aEnglish;
            if (impl_checkHelpLocalePath(sHelpPath))
            {
                return aEnglish;
            }
            sHelpPath = getHelpRootURL() + "/" + aEnglish;
            if (impl_checkHelpLocalePath(sHelpPath))
            {
                aLocaleStr = sLang;
                return aEnglish;
            }
            aLocaleStr = utl::ConfigManager::getLocale();
            return aLocaleStr;
        }
    }

    return aLocaleStr;
}



void AppendConfigToken( OUStringBuffer& rURL, bool bQuestionMark )
{
    OUString aLocaleStr = HelpLocaleString();

    // query part exists?
    if ( bQuestionMark )
        // no, so start with '?'
        rURL.append('?');
    else
        // yes, so only append with '&'
        rURL.append('&');

    // set parameters
    rURL.append("Language=");
    rURL.append(aLocaleStr);
    rURL.append("&System=");
    rURL.append(SvtHelpOptions().GetSystem());
    rURL.append("&Version=");
    rURL.append(utl::ConfigManager::getProductVersion());
}

bool GetHelpAnchor_Impl( const OUString& _rURL, OUString& _rAnchor )
{
    bool bRet = false;
    OUString sAnchor;

    try
    {
        ::ucbhelper::Content aCnt( INetURLObject( _rURL ).GetMainURL( INetURLObject::DecodeMechanism::NONE ),
                             Reference< css::ucb::XCommandEnvironment >(),
                             comphelper::getProcessComponentContext() );
        if ( aCnt.getPropertyValue("AnchorName") >>= sAnchor )
        {

            if ( !sAnchor.isEmpty() )
            {
                _rAnchor = sAnchor;
                bRet = true;
            }
        }
        else
        {
            SAL_WARN( "sfx.appl", "Property 'AnchorName' is missing" );
        }
    }
    catch (const css::uno::Exception&)
    {
    }

    return bRet;
}

class SfxHelp_Impl
{
public:
    static OUString GetHelpText( const OUString& aCommandURL, const OUString& rModule );
};

OUString SfxHelp_Impl::GetHelpText( const OUString& aCommandURL, const OUString& rModule )
{
    // create help url
    OUStringBuffer aHelpURL( SfxHelp::CreateHelpURL( aCommandURL, rModule ) );
    // added 'active' parameter
    sal_Int32 nIndex = aHelpURL.lastIndexOf( '#' );
    if ( nIndex < 0 )
        nIndex = aHelpURL.getLength();
    aHelpURL.insert( nIndex, "&Active=true" );
    // load help string
    return SfxContentHelper::GetActiveHelpString( aHelpURL.makeStringAndClear() );
}

SfxHelp::SfxHelp() :
    bIsDebug( false )
{
    // read the environment variable "HELP_DEBUG"
    // if it's set, you will see debug output on active help
    OUString sHelpDebug;
    OUString sEnvVarName( "HELP_DEBUG"  );
    osl_getEnvironment( sEnvVarName.pData, &sHelpDebug.pData );
    bIsDebug = !sHelpDebug.isEmpty();
}

SfxHelp::~SfxHelp()
{
}

OUString getDefaultModule_Impl()
{
    OUString sDefaultModule;
    SvtModuleOptions aModOpt;
    if ( aModOpt.IsModuleInstalled( SvtModuleOptions::EModule::WRITER ) )
        sDefaultModule = "swriter";
    else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::EModule::CALC ) )
        sDefaultModule = "scalc";
    else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::EModule::IMPRESS ) )
        sDefaultModule = "simpress";
    else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::EModule::DRAW ) )
        sDefaultModule = "sdraw";
    else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::EModule::MATH ) )
        sDefaultModule = "smath";
    else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::EModule::CHART ) )
        sDefaultModule = "schart";
    else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::EModule::BASIC ) )
        sDefaultModule = "sbasic";
    else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::EModule::DATABASE ) )
        sDefaultModule = "sdatabase";
    else
    {
        SAL_WARN( "sfx.appl", "getDefaultModule_Impl(): no module installed" );
    }
    return sDefaultModule;
}

OUString getCurrentModuleIdentifier_Impl()
{
    OUString sIdentifier;
    Reference < XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    Reference < XModuleManager2 > xModuleManager = ModuleManager::create(xContext);
    Reference < XDesktop2 > xDesktop = Desktop::create(xContext);
    Reference < XFrame > xCurrentFrame = xDesktop->getCurrentFrame();

    if ( xCurrentFrame.is() )
    {
        try
        {
            sIdentifier = xModuleManager->identify( xCurrentFrame );
        }
        catch (const css::frame::UnknownModuleException&)
        {
            SAL_INFO( "sfx.appl", "SfxHelp::getCurrentModuleIdentifier_Impl(): unknown module (help in help?)" );
        }
        catch (const Exception&)
        {
            SAL_WARN( "sfx.appl", "SfxHelp::getCurrentModuleIdentifier_Impl(): exception of XModuleManager::identify()" );
        }
    }

    return sIdentifier;
}

namespace
{
    OUString MapModuleIdentifier(const OUString &rFactoryShortName)
    {
        OUString aFactoryShortName(rFactoryShortName);

        // Map some module identifiers to their "real" help module string.
        if ( aFactoryShortName == "chart2" )
            aFactoryShortName = "schart" ;
        else if ( aFactoryShortName == "BasicIDE" )
            aFactoryShortName = "sbasic";
        else if ( aFactoryShortName == "sweb"
                || aFactoryShortName == "sglobal"
                || aFactoryShortName == "swxform" )
            aFactoryShortName = "swriter" ;
        else if ( aFactoryShortName == "dbquery"
                || aFactoryShortName == "dbbrowser"
                || aFactoryShortName == "dbrelation"
                || aFactoryShortName == "dbtable"
                || aFactoryShortName == "dbapp"
                || aFactoryShortName == "dbreport"
                || aFactoryShortName == "dbtdata"
                || aFactoryShortName == "swreport"
                || aFactoryShortName == "swform" )
            aFactoryShortName = "sdatabase";
        else if ( aFactoryShortName == "sbibliography"
                || aFactoryShortName == "sabpilot"
                || aFactoryShortName == "scanner"
                || aFactoryShortName == "spropctrlr"
                || aFactoryShortName == "StartModule" )
            aFactoryShortName.clear();

        return aFactoryShortName;
    }
}

OUString SfxHelp::GetHelpModuleName_Impl(const OUString& rHelpID)
{
    OUString aFactoryShortName;

    //rhbz#1438876 detect preferred module for this help id, e.g. csv dialog
    //for calc import before any toplevel is created and so context is
    //otherwise unknown. Cosmetic, same help is shown in any case because its
    //in the shared section, but title bar would state "Writer" when context is
    //expected to be "Calc"
    OUString sRemainder;
    if (rHelpID.startsWith("modules/", &sRemainder))
    {
        sal_Int32 nEndModule = sRemainder.indexOf('/');
        aFactoryShortName = nEndModule != -1 ? sRemainder.copy(0, nEndModule) : sRemainder;
    }

    if (aFactoryShortName.isEmpty())
    {
        OUString aModuleIdentifier = getCurrentModuleIdentifier_Impl();
        if (!aModuleIdentifier.isEmpty())
        {
            try
            {
                Reference < XModuleManager2 > xModuleManager(
                    ModuleManager::create(::comphelper::getProcessComponentContext()) );
                Sequence< PropertyValue > lProps;
                xModuleManager->getByName( aModuleIdentifier ) >>= lProps;
                for ( sal_Int32 i = 0; i < lProps.getLength(); ++i )
                {
                    if ( lProps[i].Name == "ooSetupFactoryShortName" )
                    {
                        lProps[i].Value >>= aFactoryShortName;
                        break;
                    }
                }
            }
            catch (const Exception&)
            {
                SAL_WARN( "sfx.appl", "SfxHelp::GetHelpModuleName_Impl(): exception of XNameAccess::getByName()" );
            }
        }
    }

    if (!aFactoryShortName.isEmpty())
        aFactoryShortName = MapModuleIdentifier(aFactoryShortName);
    if (aFactoryShortName.isEmpty())
        aFactoryShortName = getDefaultModule_Impl();

    return aFactoryShortName;
}

OUString SfxHelp::CreateHelpURL_Impl( const OUString& aCommandURL, const OUString& rModuleName )
{
    // build up the help URL
    OUStringBuffer aHelpURL("vnd.sun.star.help://");
    bool bHasAnchor = false;
    OUString aAnchor;

    OUString aModuleName( rModuleName );
    if (aModuleName.isEmpty())
        aModuleName = getDefaultModule_Impl();

    aHelpURL.append(aModuleName);

    if ( aCommandURL.isEmpty() )
        aHelpURL.append("/start");
    else
    {
        aHelpURL.append('/');
        aHelpURL.append(rtl::Uri::encode(aCommandURL,
                                              rtl_UriCharClassRelSegment,
                                              rtl_UriEncodeKeepEscapes,
                                              RTL_TEXTENCODING_UTF8));

        OUStringBuffer aTempURL = aHelpURL;
        AppendConfigToken( aTempURL, true );
        bHasAnchor = GetHelpAnchor_Impl(aTempURL.makeStringAndClear(), aAnchor);
    }

    AppendConfigToken( aHelpURL, true );

    if ( bHasAnchor )
    {
        aHelpURL.append('#');
        aHelpURL.append(aAnchor);
    }

    return aHelpURL.makeStringAndClear();
}

SfxHelpWindow_Impl* impl_createHelp(Reference< XFrame2 >& rHelpTask   ,
                                    Reference< XFrame >& rHelpContent)
{
    Reference < XDesktop2 > xDesktop = Desktop::create( ::comphelper::getProcessComponentContext() );

    // otherwise - create new help task
    Reference< XFrame2 > xHelpTask(
        xDesktop->findFrame(  "OFFICE_HELP_TASK", FrameSearchFlag::TASKS | FrameSearchFlag::CREATE),
        UNO_QUERY);
    if (!xHelpTask.is())
        return nullptr;

    // create all internal windows and sub frames ...
    Reference< css::awt::XWindow >      xParentWindow = xHelpTask->getContainerWindow();
    VclPtr<vcl::Window>                 pParentWindow = VCLUnoHelper::GetWindow( xParentWindow );
    VclPtrInstance<SfxHelpWindow_Impl>  pHelpWindow( xHelpTask, pParentWindow );
    Reference< css::awt::XWindow >      xHelpWindow   = VCLUnoHelper::GetInterface( pHelpWindow );

    Reference< XFrame > xHelpContent;
    if (xHelpTask->setComponent( xHelpWindow, Reference< XController >() ))
    {
        // Customize UI ...
        xHelpTask->setName("OFFICE_HELP_TASK");

        Reference< XPropertySet > xProps(xHelpTask, UNO_QUERY);
        if (xProps.is())
            xProps->setPropertyValue(
                "Title",
                makeAny(SfxResId(STR_HELP_WINDOW_TITLE)));

        pHelpWindow->setContainerWindow( xParentWindow );
        xParentWindow->setVisible(true);
        xHelpWindow->setVisible(true);

        // This sub frame is created internally (if we called new SfxHelpWindow_Impl() ...)
        // It should exist :-)
        xHelpContent = xHelpTask->findFrame("OFFICE_HELP", FrameSearchFlag::CHILDREN);
    }

    if (!xHelpContent.is())
    {
        pHelpWindow.disposeAndClear();
        return nullptr;
    }

    xHelpContent->setName("OFFICE_HELP");

    rHelpTask    = xHelpTask;
    rHelpContent = xHelpContent;
    return pHelpWindow;
}

OUString SfxHelp::GetHelpText( const OUString& aCommandURL, const vcl::Window* pWindow )
{
    OUString sModuleName = GetHelpModuleName_Impl(aCommandURL);
    OUString sRealCommand = vcl::CommandInfoProvider::GetRealCommandForCommand( aCommandURL, getCurrentModuleIdentifier_Impl() );
    OUString sHelpText = SfxHelp_Impl::GetHelpText( sRealCommand.isEmpty() ? aCommandURL : sRealCommand, sModuleName );

    OString aNewHelpId;

    if (pWindow && sHelpText.isEmpty())
    {
        // no help text found -> try with parent help id.
        vcl::Window* pParent = pWindow->GetParent();
        while ( pParent )
        {
            aNewHelpId = pParent->GetHelpId();
            sHelpText = SfxHelp_Impl::GetHelpText( OStringToOUString(aNewHelpId, RTL_TEXTENCODING_UTF8), sModuleName );
            if (!sHelpText.isEmpty())
                pParent = nullptr;
            else
                pParent = pParent->GetParent();
        }

        if (bIsDebug && sHelpText.isEmpty())
            aNewHelpId.clear();
    }

    // add some debug information?
    if ( bIsDebug )
    {
        sHelpText += "\n-------------\n";
        sHelpText += sModuleName;
        sHelpText += ": ";
        sHelpText += aCommandURL;
        if ( !aNewHelpId.isEmpty() )
        {
            sHelpText += " - ";
            sHelpText += OStringToOUString(aNewHelpId, RTL_TEXTENCODING_UTF8);
        }
    }

    return sHelpText;
}

void SfxHelp::SearchKeyword( const OUString& rKeyword )
{
    Start_Impl(OUString(), static_cast<vcl::Window*>(nullptr), rKeyword);
}

bool SfxHelp::Start( const OUString& rURL, const vcl::Window* pWindow )
{
    return Start_Impl( rURL, pWindow, OUString() );
}

bool SfxHelp::Start(const OUString& rURL, weld::Widget* pWidget)
{
    return Start_Impl(rURL, pWidget, OUString());
}

/// Redirect the vnd.sun.star.help:// urls to http://help.libreoffice.org
static bool impl_showOnlineHelp( const OUString& rURL )
{
    OUString aInternal( "vnd.sun.star.help://"  );
    if ( rURL.getLength() <= aInternal.getLength() || !rURL.startsWith(aInternal) )
        return false;

    OUString aHelpLink( "https://help.libreoffice.org/help.html?"  );

    aHelpLink += rURL.copy( aInternal.getLength() );
    aHelpLink = aHelpLink.replaceAll("%2F","/");


    if (comphelper::LibreOfficeKit::isActive())
    {
        if(SfxViewShell* pViewShell = SfxViewShell::Current())
        {
            pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_HYPERLINK_CLICKED,
                                                   aHelpLink.toUtf8().getStr());
            return true;
        }
        return false;
    }

    try
    {
        sfx2::openUriExternally(aHelpLink, false);
        return true;
    }
    catch (const Exception&)
    {
    }
    return false;
}

#define SHTML1 "<!DOCTYPE HTML><html lang=\"en-US\"><head><meta charset=\"UTF-8\">"
#define SHTML2 "<meta http-equiv=\"refresh\" content=\"1\" url=\""
#define SHTML3 "\"><script type=\"text/javascript\"> window.location.href = \""
#define SHTML4 "\";</script><title>Help Page Redirection</title></head><body></body></html>"

static bool impl_showOfflineHelp( const OUString& rURL )
{
    OUString aBaseInstallPath = getHelpRootURL();
    OUString const aInternal( "vnd.sun.star.help://"  );

    OUString aHelpLink( aBaseInstallPath + "/index.html?" );
    aHelpLink += rURL.copy( aInternal.getLength() );
    aHelpLink = aHelpLink.replaceAll("%2F","/").replaceAll("%3A",":");

    // get a html tempfile
    OUString const aExtension(".html");
    ::utl::TempFile aTempFile("NewHelp", true, &aExtension, nullptr, false );

    SvStream* pStream = aTempFile.GetStream(StreamMode::WRITE);
    pStream->SetStreamCharSet(RTL_TEXTENCODING_UTF8);

    OUString aTempStr(SHTML1 SHTML2);
    aTempStr += aHelpLink + SHTML3;
    aTempStr += aHelpLink + SHTML4;

    pStream->WriteUnicodeOrByteText(aTempStr);

    aTempFile.CloseStream();

    try
    {
        sfx2::openUriExternally(aTempFile.GetURL(), false);
        return true;
    }
    catch (const Exception&)
    {
    }
    aTempFile.EnableKillingFile();
    return false;
}

bool SfxHelp::Start_Impl(const OUString& rURL, const vcl::Window* pWindow, const OUString& rKeyword)
{
    OUStringBuffer aHelpRootURL("vnd.sun.star.help://");
    AppendConfigToken(aHelpRootURL, true);
    SfxContentHelper::GetResultSet(aHelpRootURL.makeStringAndClear());

    /* rURL may be
     *       - a "real" URL
     *       - a HelpID (formerly a long, now a string)
     *      If rURL is a URL, CreateHelpURL should be called for this URL
     *      If rURL is an arbitrary string, the same should happen, but the URL should be tried out
     *      if it delivers real help content. In case only the Help Error Document is returned, the
     *      parent of the window for that help was called, is asked for its HelpID.
     *      For compatibility reasons this upward search is not implemented for "real" URLs.
     *      Help keyword search now is implemented as own method; in former versions it
     *      was done via Help::Start, but this implementation conflicted with the upward search.
     */
    OUString aHelpURL;
    INetURLObject aParser( rURL );
    INetProtocol nProtocol = aParser.GetProtocol();

    switch ( nProtocol )
    {
        case INetProtocol::VndSunStarHelp:
            // already a vnd.sun.star.help URL -> nothing to do
            aHelpURL = rURL;
            break;
        default:
        {
            OUString aHelpModuleName(GetHelpModuleName_Impl(rURL));
            OUString aRealCommand;

            if ( nProtocol == INetProtocol::Uno )
                // Command can be just an alias to another command.
                aRealCommand = vcl::CommandInfoProvider::GetRealCommandForCommand( rURL, getCurrentModuleIdentifier_Impl() );

            // no URL, just a HelpID (maybe empty in case of keyword search)
            aHelpURL = CreateHelpURL_Impl( aRealCommand.isEmpty() ? rURL : aRealCommand, aHelpModuleName );

            if ( impl_hasHelpInstalled() && pWindow && SfxContentHelper::IsHelpErrorDocument( aHelpURL ) )
            {
                // no help found -> try with parent help id.
                vcl::Window* pParent = pWindow->GetParent();
                bool bTriedTabPage = false;
                while ( pParent )
                {
                    OString aHelpId = pParent->GetHelpId();
                    aHelpURL = CreateHelpURL( OStringToOUString(aHelpId, RTL_TEXTENCODING_UTF8), aHelpModuleName );

                    if ( !SfxContentHelper::IsHelpErrorDocument( aHelpURL ) )
                    {
                        break;
                    }
                    else
                    {
                        pParent = pParent->GetParent();
                        if (!pParent)
                        {
                            // create help url of start page ( helpid == 0 -> start page)
                            aHelpURL = CreateHelpURL( OUString(), aHelpModuleName );

                        }
                        else if (pParent->IsDialog() && !bTriedTabPage)
                        {
                            //During help fallback, before we ask a dialog for its help
                            //see if it has a TabControl and ask the active tab of
                            //that for help
                            bTriedTabPage = true;
                            Dialog *pDialog = static_cast<Dialog*>(pParent);
                            TabControl *pCtrl = pDialog->hasBuilder() ? pDialog->get<TabControl>("tabcontrol") : nullptr;
                            TabPage* pTabPage = pCtrl ? pCtrl->GetTabPage(pCtrl->GetCurPageId()) : nullptr;
                            vcl::Window *pTabChild = pTabPage ? pTabPage->GetWindow(GetWindowType::FirstChild) : nullptr;
                            if (pTabChild)
                                pParent = pTabChild;
                        }
                    }
                }
            }
            break;
        }
    }

    if ( comphelper::LibreOfficeKit::isActive() )
    {
        impl_showOnlineHelp( aHelpURL );
        return true;
    }

    // If the HTML or no help is installed, but aHelpURL nevertheless references valid help content,
    // that implies that this help content belongs to an extension (and thus would not be available
    // in neither the offline nor online HTML help); in that case, fall through to the "old-help to
    // display" code below:
    if (SfxContentHelper::IsHelpErrorDocument(aHelpURL))
    {
        if ( impl_hasHTMLHelpInstalled() )
        {
            impl_showOfflineHelp(aHelpURL);
            return true;
        }

        if ( !impl_hasHelpInstalled() )
        {
            SvtHelpOptions aHelpOptions;
            bool bShowOfflineHelpPopUp = aHelpOptions.IsOfflineHelpPopUp();

            if(bShowOfflineHelpPopUp)
            {
                std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(pWindow ? pWindow->GetFrameWeld() : nullptr, "sfx/ui/helpmanual.ui"));
                std::unique_ptr<weld::MessageDialog> xQueryBox(xBuilder->weld_message_dialog("onlinehelpmanual"));
                std::unique_ptr<weld::CheckButton> m_xHideOfflineHelpCB(xBuilder->weld_check_button("hidedialog"));
                LanguageTag aLangTag = Application::GetSettings().GetUILanguageTag();
                OUString sLocaleString = SvtLanguageTable::GetLanguageString( aLangTag.getLanguageType() );
                OUString sPrimText = xQueryBox->get_primary_text();
                xQueryBox->set_primary_text(sPrimText.replaceAll("$UILOCALE", sLocaleString));
                short OnlineHelpBox = xQueryBox->run();
                bShowOfflineHelpPopUp = OnlineHelpBox != RET_OK;
                aHelpOptions.SetOfflineHelpPopUp(!m_xHideOfflineHelpCB->get_state());
            }
            if(!bShowOfflineHelpPopUp)
            {
                if ( impl_showOnlineHelp( aHelpURL ) )
                    return true;
                else
                {
                    NoHelpErrorBox aErrBox(pWindow ? pWindow->GetFrameWeld() : nullptr);
                    aErrBox.run();
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
    }

    // old-help to display
    Reference < XDesktop2 > xDesktop = Desktop::create( ::comphelper::getProcessComponentContext() );

    // check if help window is still open
    // If not, create a new one and return access directly to the internal sub frame showing the help content
    // search must be done here; search one desktop level could return an arbitrary frame
    Reference< XFrame2 > xHelp(
        xDesktop->findFrame( "OFFICE_HELP_TASK", FrameSearchFlag::CHILDREN),
                               UNO_QUERY);
    Reference< XFrame > xHelpContent = xDesktop->findFrame(
        "OFFICE_HELP",
        FrameSearchFlag::CHILDREN);

    SfxHelpWindow_Impl* pHelpWindow = nullptr;
    if (!xHelp.is())
        pHelpWindow = impl_createHelp(xHelp, xHelpContent);
    else
        pHelpWindow = static_cast<SfxHelpWindow_Impl*>(VCLUnoHelper::GetWindow(xHelp->getComponentWindow()).get());
    if (!xHelp.is() || !xHelpContent.is() || !pHelpWindow)
        return false;

    SAL_INFO("sfx.appl", "HelpId = " << aHelpURL);

    pHelpWindow->SetHelpURL( aHelpURL );
    pHelpWindow->loadHelpContent(aHelpURL);
    if (!rKeyword.isEmpty())
        pHelpWindow->OpenKeyword( rKeyword );

    Reference < css::awt::XTopWindow > xTopWindow( xHelp->getContainerWindow(), UNO_QUERY );
    if ( xTopWindow.is() )
        xTopWindow->toFront();

    return true;
}

bool SfxHelp::Start_Impl(const OUString& rURL, weld::Widget* pWidget, const OUString& rKeyword)
{
    OUStringBuffer aHelpRootURL("vnd.sun.star.help://");
    AppendConfigToken(aHelpRootURL, true);
    SfxContentHelper::GetResultSet(aHelpRootURL.makeStringAndClear());

    /* rURL may be
     *       - a "real" URL
     *       - a HelpID (formerly a long, now a string)
     *      If rURL is a URL, CreateHelpURL should be called for this URL
     *      If rURL is an arbitrary string, the same should happen, but the URL should be tried out
     *      if it delivers real help content. In case only the Help Error Document is returned, the
     *      parent of the window for that help was called, is asked for its HelpID.
     *      For compatibility reasons this upward search is not implemented for "real" URLs.
     *      Help keyword search now is implemented as own method; in former versions it
     *      was done via Help::Start, but this implementation conflicted with the upward search.
     */
    OUString aHelpURL;
    INetURLObject aParser( rURL );
    INetProtocol nProtocol = aParser.GetProtocol();

    switch ( nProtocol )
    {
        case INetProtocol::VndSunStarHelp:
            // already a vnd.sun.star.help URL -> nothing to do
            aHelpURL = rURL;
            break;
        default:
        {
            OUString aHelpModuleName(GetHelpModuleName_Impl(rURL));
            OUString aRealCommand;

            if ( nProtocol == INetProtocol::Uno )
                // Command can be just an alias to another command.
                aRealCommand = vcl::CommandInfoProvider::GetRealCommandForCommand( rURL, getCurrentModuleIdentifier_Impl() );

            // no URL, just a HelpID (maybe empty in case of keyword search)
            aHelpURL = CreateHelpURL_Impl( aRealCommand.isEmpty() ? rURL : aRealCommand, aHelpModuleName );

            if ( impl_hasHelpInstalled() && pWidget && SfxContentHelper::IsHelpErrorDocument( aHelpURL ) )
            {
                // no help found -> try with parent help id.
                std::unique_ptr<weld::Widget> xParent(pWidget->weld_parent());
                while (xParent)
                {
                    OString aHelpId = xParent->get_help_id();
                    aHelpURL = CreateHelpURL( OStringToOUString(aHelpId, RTL_TEXTENCODING_UTF8), aHelpModuleName );

                    if ( !SfxContentHelper::IsHelpErrorDocument( aHelpURL ) )
                    {
                        break;
                    }
                    else
                    {
                        xParent.reset(xParent->weld_parent());
                        if (!xParent)
                        {
                            // create help url of start page ( helpid == 0 -> start page)
                            aHelpURL = CreateHelpURL( OUString(), aHelpModuleName );
                        }
                    }
                }
            }
            break;
        }
    }

    if ( comphelper::LibreOfficeKit::isActive() )
    {
        impl_showOnlineHelp( aHelpURL );
        return true;
    }

    // If the HTML or no help is installed, but aHelpURL nevertheless references valid help content,
    // that implies that that help content belongs to an extension (and thus would not be available
    // in neither the offline nor online HTML help); in that case, fall through to the "old-help to
    // display" code below:
    if (SfxContentHelper::IsHelpErrorDocument(aHelpURL))
    {
        if ( impl_hasHTMLHelpInstalled() )
        {
            impl_showOfflineHelp(aHelpURL);
            return true;
        }

        if ( !impl_hasHelpInstalled() )
        {
            SvtHelpOptions aHelpOptions;
            bool bShowOfflineHelpPopUp = aHelpOptions.IsOfflineHelpPopUp();

            if(bShowOfflineHelpPopUp)
            {
                std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(pWidget, "sfx/ui/helpmanual.ui"));
                std::unique_ptr<weld::MessageDialog> xQueryBox(xBuilder->weld_message_dialog("onlinehelpmanual"));
                std::unique_ptr<weld::CheckButton> m_xHideOfflineHelpCB(xBuilder->weld_check_button("hidedialog"));
                LanguageTag aLangTag = Application::GetSettings().GetUILanguageTag();
                OUString sLocaleString = SvtLanguageTable::GetLanguageString( aLangTag.getLanguageType() );
                OUString sPrimText = xQueryBox->get_primary_text();
                xQueryBox->set_primary_text(sPrimText.replaceAll("$UILOCALE", sLocaleString));
                short OnlineHelpBox = xQueryBox->run();
                bShowOfflineHelpPopUp = OnlineHelpBox != RET_OK;
                aHelpOptions.SetOfflineHelpPopUp(!m_xHideOfflineHelpCB->get_state());
            }
            if(!bShowOfflineHelpPopUp)
            {
                if ( impl_showOnlineHelp( aHelpURL ) )
                    return true;
                else
                {
                    NoHelpErrorBox aErrBox(pWidget);
                    aErrBox.run();
                    return false;
                }
            }
            else
            {
                return false;
            }

        }
    }

    // old-help to display
    Reference < XDesktop2 > xDesktop = Desktop::create( ::comphelper::getProcessComponentContext() );

    // check if help window is still open
    // If not, create a new one and return access directly to the internal sub frame showing the help content
    // search must be done here; search one desktop level could return an arbitrary frame
    Reference< XFrame2 > xHelp(
        xDesktop->findFrame( "OFFICE_HELP_TASK", FrameSearchFlag::CHILDREN),
                               UNO_QUERY);
    Reference< XFrame > xHelpContent = xDesktop->findFrame(
        "OFFICE_HELP",
        FrameSearchFlag::CHILDREN);

    SfxHelpWindow_Impl* pHelpWindow = nullptr;
    if (!xHelp.is())
        pHelpWindow = impl_createHelp(xHelp, xHelpContent);
    else
        pHelpWindow = static_cast<SfxHelpWindow_Impl*>(VCLUnoHelper::GetWindow(xHelp->getComponentWindow()).get());
    if (!xHelp.is() || !xHelpContent.is() || !pHelpWindow)
        return false;

    SAL_INFO("sfx.appl", "HelpId = " << aHelpURL);

    pHelpWindow->SetHelpURL( aHelpURL );
    pHelpWindow->loadHelpContent(aHelpURL);
    if (!rKeyword.isEmpty())
        pHelpWindow->OpenKeyword( rKeyword );

    Reference < css::awt::XTopWindow > xTopWindow( xHelp->getContainerWindow(), UNO_QUERY );
    if ( xTopWindow.is() )
        xTopWindow->toFront();

    return true;
}

OUString SfxHelp::CreateHelpURL(const OUString& aCommandURL, const OUString& rModuleName)
{
    SfxHelp* pHelp = static_cast< SfxHelp* >(Application::GetHelp());
    return pHelp ? SfxHelp::CreateHelpURL_Impl( aCommandURL, rModuleName ) : OUString();
}

OUString SfxHelp::GetDefaultHelpModule()
{
    return getDefaultModule_Impl();
}

OUString SfxHelp::GetCurrentModuleIdentifier()
{
    return getCurrentModuleIdentifier_Impl();
}

bool SfxHelp::IsHelpInstalled()
{
    return impl_hasHelpInstalled();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
