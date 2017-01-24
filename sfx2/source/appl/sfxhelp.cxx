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

#include <sfx2/sfxhelp.hxx>

#include <set>
#include <algorithm>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XFrame2.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
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
#include <osl/process.h>
#include <osl/file.hxx>
#include <unotools/bootstrap.hxx>
#include <rtl/uri.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/layout.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/sfxecode.hxx>

#include "newhelp.hxx"
#include <sfx2/objsh.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/sfxresid.hxx>
#include "helper.hxx"
#include "app.hrc"
#include <sfx2/sfxuno.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/frame.hxx>
#include <rtl/string.hxx>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::system;

class NoHelpErrorBox : public MessageDialog
{
public:
    explicit NoHelpErrorBox( vcl::Window* _pParent );

    virtual void    RequestHelp( const HelpEvent& rHEvt ) override;
};

NoHelpErrorBox::NoHelpErrorBox( vcl::Window* _pParent )
    : MessageDialog(_pParent, SfxResId(RID_STR_HLPFILENOTEXIST))
{
    // Error message: "No help available"
}

void NoHelpErrorBox::RequestHelp( const HelpEvent& )
{
    // do nothing, because no help available
}

static bool impl_hasHelpInstalled( const OUString &rLang );

/// Return the locale we prefer for displaying help
static OUString const & HelpLocaleString()
{
    static OUString aLocaleStr;
    if (aLocaleStr.isEmpty())
    {
        const OUString aEnglish( "en"  );
        // detect installed locale
        aLocaleStr = utl::ConfigManager::getLocale();
        bool bOk = !aLocaleStr.isEmpty();
        if ( !bOk )
            aLocaleStr = aEnglish;
        else
        {
            OUString aBaseInstallPath;
            utl::Bootstrap::locateBaseInstallation(aBaseInstallPath);
            static const char szHelpPath[] = "/help/";

            OUString sHelpPath = aBaseInstallPath + szHelpPath + aLocaleStr;
            osl::DirectoryItem aDirItem;

            if (osl::DirectoryItem::get(sHelpPath, aDirItem) != osl::FileBase::E_None)
            {
                bOk = false;
                OUString sLang(aLocaleStr);
                sal_Int32 nSepPos = sLang.indexOf( '-' );
                if (nSepPos != -1)
                {
                    bOk = true;
                    sLang = sLang.copy( 0, nSepPos );
                    sHelpPath = aBaseInstallPath + szHelpPath + sLang;
                    if (osl::DirectoryItem::get(sHelpPath, aDirItem) != osl::FileBase::E_None)
                        bOk = false;
                }
            }
        }
        // if not OK, and not even English installed, we use online help, and
        // have to preserve the full locale name
        if ( !bOk && impl_hasHelpInstalled( aEnglish ) )
            aLocaleStr = aEnglish;
    }
    return aLocaleStr;
}

void AppendConfigToken( OUStringBuffer& rURL, bool bQuestionMark, const OUString &rLang )
{
    OUString aLocaleStr( rLang );
    if ( aLocaleStr.isEmpty() )
        aLocaleStr = HelpLocaleString();

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
        if ( ( aCnt.getPropertyValue("AnchorName") >>= sAnchor ) )
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
    {
        OUString sHelpDebug;
        OUString sEnvVarName( "HELP_DEBUG"  );
        osl_getEnvironment( sEnvVarName.pData, &sHelpDebug.pData );
        bIsDebug = !sHelpDebug.isEmpty();
    }
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

Reference< XFrame > getCurrentFrame()
{
    Reference < XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    Reference < XDesktop2 > xDesktop = Desktop::create(xContext);
    return xDesktop->getCurrentFrame();
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

OUString SfxHelp::GetHelpModuleName_Impl()
{
    OUString aFactoryShortName;
    OUString aModuleIdentifier = getCurrentModuleIdentifier_Impl();

    if ( !aModuleIdentifier.isEmpty() )
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

    OUString sDefaultModule = getDefaultModule_Impl();
    if ( !aFactoryShortName.isEmpty() )
    {
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
                || aFactoryShortName == "swreport"
                || aFactoryShortName == "swform" )
            aFactoryShortName = "sdatabase";
        else if ( aFactoryShortName == "sbibliography"
                || aFactoryShortName == "StartModule" )
            aFactoryShortName = sDefaultModule;
    }
    else
        aFactoryShortName = sDefaultModule;

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
    VclPtrInstance<SfxHelpWindow_Impl>  pHelpWindow( xHelpTask, pParentWindow, WB_DOCKBORDER );
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
                makeAny(SfxResId(STR_HELP_WINDOW_TITLE).toString()));

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
    OUString sModuleName = GetHelpModuleName_Impl();
    OUString sRealCommand = vcl::CommandInfoProvider::GetRealCommandForCommand( aCommandURL, getCurrentFrame() );
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

/// Check for built-in help
static bool impl_hasHelpInstalled( const OUString &rLang = OUString() )
{
    OUStringBuffer aHelpRootURL("vnd.sun.star.help://");
    AppendConfigToken(aHelpRootURL, true, rLang);
    std::vector< OUString > aFactories = SfxContentHelper::GetResultSet(aHelpRootURL.makeStringAndClear());

    return !aFactories.empty();
}

bool SfxHelp::SearchKeyword( const OUString& rKeyword )
{
    return Start_Impl( OUString(), nullptr, rKeyword );
}

bool SfxHelp::Start( const OUString& rURL, const vcl::Window* pWindow )
{
    return Start_Impl( rURL, pWindow, OUString() );
}

/// Redirect the vnd.sun.star.help:// urls to http://help.libreoffice.org
static bool impl_showOnlineHelp( const OUString& rURL )
{
    OUString aInternal( "vnd.sun.star.help://"  );
    if ( rURL.getLength() <= aInternal.getLength() || !rURL.startsWith(aInternal) )
        return false;

    OUString aHelpLink( "http://help.libreoffice.org/"  );
    aHelpLink += rURL.copy( aInternal.getLength() );
    aHelpLink = aHelpLink.replaceAll("%2F","/");
    try
    {
        Reference< XSystemShellExecute > xSystemShell(
                SystemShellExecute::create(::comphelper::getProcessComponentContext()) );

        xSystemShell->execute( aHelpLink, OUString(), SystemShellExecuteFlags::URIS_ONLY );
        return true;
    }
    catch (const Exception&)
    {
    }
    return false;
}

bool SfxHelp::Start_Impl(const OUString& rURL, const vcl::Window* pWindow, const OUString& rKeyword)
{
    OUStringBuffer aHelpRootURL("vnd.sun.star.help://");
    AppendConfigToken(aHelpRootURL, true);
    SfxContentHelper::GetResultSet(aHelpRootURL.makeStringAndClear());

    /* rURL may be
        - a "real" URL
        - a HelpID (formerly a long, now a string)
       If rURL is a URL, CreateHelpURL should be called for this URL
       If rURL is an arbitrary string, the same should happen, but the URL should be tried out
       if it delivers real help content. In case only the Help Error Document is returned, the
       parent of the window for that help was called, is asked for its HelpID.
       For compatibility reasons this upward search is not implemented for "real" URLs.
       Help keyword search now is implemented as own method; in former versions it
       was done via Help::Start, but this implementation conflicted with the upward search.
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
            OUString aHelpModuleName( GetHelpModuleName_Impl() );
            OUString aRealCommand;

            if ( nProtocol == INetProtocol::Uno )
                // Command can be just an alias to another command.
                aRealCommand = vcl::CommandInfoProvider::GetRealCommandForCommand( rURL, getCurrentFrame() );

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

    if ( !impl_hasHelpInstalled() )
    {
        ScopedVclPtrInstance< MessageDialog > aQueryBox(const_cast< vcl::Window* >( pWindow ),"onlinehelpmanual","sfx/ui/helpmanual.ui");
        short OnlineHelpBox = aQueryBox->Execute();

        if(OnlineHelpBox == RET_OK)
        {
            if ( impl_showOnlineHelp( aHelpURL ) )
                return true;
            else
            {
                ScopedVclPtrInstance< NoHelpErrorBox > aErrBox(const_cast< vcl::Window* >( pWindow ));
                aErrBox->Execute();
                return false;
            }
        }
        else
        {
            return false;
        }

    }

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
