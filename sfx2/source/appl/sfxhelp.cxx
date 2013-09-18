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

#include "sfx2/sfxhelp.hxx"

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
#include <vcl/msgbox.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/sfxecode.hxx>

#include "newhelp.hxx"
#include <sfx2/objsh.hxx>
#include <sfx2/docfac.hxx>
#include "sfx2/sfxresid.hxx"
#include "helper.hxx"
#include "app.hrc"
#include <sfx2/sfxuno.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/frame.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::system;

class NoHelpErrorBox : public ErrorBox
{
public:
    NoHelpErrorBox( Window* _pParent );

    virtual void    RequestHelp( const HelpEvent& rHEvt );
};

NoHelpErrorBox::NoHelpErrorBox( Window* _pParent ) :

    ErrorBox( _pParent, WB_OK, SfxResId( RID_STR_HLPFILENOTEXIST ).toString() )
{
    // Error message: "No help available"
}

void NoHelpErrorBox::RequestHelp( const HelpEvent& )
{
    // do nothing, because no help available
}

static bool impl_hasHelpInstalled( const OUString &rLang );

/// Return the locale we prefer for displaying help
static OUString HelpLocaleString()
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
            static const char *szHelpPath = "/help/";

            OUString sHelpPath = aBaseInstallPath +
                OUString::createFromAscii(szHelpPath) + aLocaleStr;
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
                    sHelpPath = aBaseInstallPath +
                        OUString::createFromAscii(szHelpPath) + sLang;
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

void AppendConfigToken( OUStringBuffer& rURL, sal_Bool bQuestionMark, const OUString &rLang )
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

sal_Bool GetHelpAnchor_Impl( const OUString& _rURL, OUString& _rAnchor )
{
    sal_Bool bRet = sal_False;
    OUString sAnchor;

    try
    {
        ::ucbhelper::Content aCnt( INetURLObject( _rURL ).GetMainURL( INetURLObject::NO_DECODE ),
                             Reference< ::com::sun::star::ucb::XCommandEnvironment >(),
                             comphelper::getProcessComponentContext() );
        if ( ( aCnt.getPropertyValue("AnchorName") >>= sAnchor ) )
        {

            if ( !sAnchor.isEmpty() )
            {
                _rAnchor = sAnchor;
                bRet = sal_True;
            }
        }
        else
        {
            SAL_WARN( "sfx.appl", "Property 'AnchorName' is missing" );
        }
    }
    catch (const ::com::sun::star::uno::Exception&)
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
    bIsDebug( sal_False ),
    pImp    ( NULL )
{
    // read the environment variable "HELP_DEBUG"
    // if it's set, you will see debug output on active help
    {
        OUString sHelpDebug;
        OUString sEnvVarName( "HELP_DEBUG"  );
        osl_getEnvironment( sEnvVarName.pData, &sHelpDebug.pData );
        bIsDebug = !sHelpDebug.isEmpty();
    }

    pImp = new SfxHelp_Impl();
}

SfxHelp::~SfxHelp()
{
    delete pImp;
}

OUString getDefaultModule_Impl()
{
    OUString sDefaultModule;
    SvtModuleOptions aModOpt;
    if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
        sDefaultModule = "swriter";
    else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC ) )
        sDefaultModule = "scalc";
    else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SIMPRESS ) )
        sDefaultModule = "simpress";
    else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SDRAW ) )
        sDefaultModule = "sdraw";
    else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SMATH ) )
        sDefaultModule = "smath";
    else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SCHART ) )
        sDefaultModule = "schart";
    else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SBASIC ) )
        sDefaultModule = "sbasic";
    else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SDATABASE ) )
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
        catch (const ::com::sun::star::frame::UnknownModuleException&)
        {
            DBG_WARNING( "SfxHelp::getCurrentModuleIdentifier_Impl(): unknown module (help in help?)" );
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
            aFactoryShortName = OUString( "schart"  );
        else if ( aFactoryShortName == "BasicIDE" )
            aFactoryShortName = OUString( "sbasic"  );
        else if ( aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("sweb"))
                || aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("sglobal"))
                || aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("swxform")) )
            aFactoryShortName = OUString( "swriter"  );
        else if ( aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("dbquery"))
                || aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("dbbrowser"))
                || aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("dbrelation"))
                || aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("dbtable"))
                || aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("dbapp"))
                || aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("dbreport"))
                || aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("swreport"))
                || aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("dbbrowser"))
                || aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("swform")) )
            aFactoryShortName = OUString( "sdatabase"  );
        else if ( aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("sbibliography"))
                || aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("StartModule")) )
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
    sal_Bool bHasAnchor = sal_False;
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
        AppendConfigToken( aTempURL, sal_True );
        bHasAnchor = GetHelpAnchor_Impl(aTempURL.makeStringAndClear(), aAnchor);
    }

    AppendConfigToken( aHelpURL, sal_True );

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
        return 0;

    // create all internal windows and sub frames ...
    Reference< ::com::sun::star::awt::XWindow > xParentWindow = xHelpTask->getContainerWindow();
    Window*                                     pParentWindow = VCLUnoHelper::GetWindow( xParentWindow );
    SfxHelpWindow_Impl*                         pHelpWindow   = new SfxHelpWindow_Impl( xHelpTask, pParentWindow, WB_DOCKBORDER );
    Reference< ::com::sun::star::awt::XWindow > xHelpWindow   = VCLUnoHelper::GetInterface( pHelpWindow );

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
        xParentWindow->setVisible(sal_True);
        xHelpWindow->setVisible(sal_True);

        // This sub frame is created internaly (if we called new SfxHelpWindow_Impl() ...)
        // It should exist :-)
        xHelpContent = xHelpTask->findFrame(OUString("OFFICE_HELP"), FrameSearchFlag::CHILDREN);
    }

    if (!xHelpContent.is())
    {
        delete pHelpWindow;
        return NULL;
    }

    xHelpContent->setName("OFFICE_HELP");

    rHelpTask    = xHelpTask;
    rHelpContent = xHelpContent;
    return pHelpWindow;
}

OUString SfxHelp::GetHelpText( const OUString& aCommandURL, const Window* pWindow )
{
    OUString sModuleName = GetHelpModuleName_Impl();
    OUString sHelpText = SfxHelp_Impl::GetHelpText( aCommandURL, sModuleName );

    OString aNewHelpId;

    if (pWindow && sHelpText.isEmpty())
    {
        // no help text found -> try with parent help id.
        Window* pParent = pWindow->GetParent();
        while ( pParent )
        {
            aNewHelpId = pParent->GetHelpId();
            sHelpText = SfxHelp_Impl::GetHelpText( OStringToOUString(aNewHelpId, RTL_TEXTENCODING_UTF8), sModuleName );
            if (!sHelpText.isEmpty())
                pParent = NULL;
            else
                pParent = pParent->GetParent();
        }

        if (bIsDebug && sHelpText.isEmpty())
            aNewHelpId = OString();
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
    AppendConfigToken(aHelpRootURL, sal_True, rLang);
    std::vector< OUString > aFactories = SfxContentHelper::GetResultSet(aHelpRootURL.makeStringAndClear());

    return !aFactories.empty();
}

sal_Bool SfxHelp::SearchKeyword( const OUString& rKeyword )
{
    return Start_Impl( OUString(), NULL, rKeyword );
}

sal_Bool SfxHelp::Start( const OUString& rURL, const Window* pWindow )
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

sal_Bool SfxHelp::Start_Impl(const OUString& rURL, const Window* pWindow, const OUString& rKeyword)
{
    OUStringBuffer aHelpRootURL("vnd.sun.star.help://");
    AppendConfigToken(aHelpRootURL, sal_True);
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
        case INET_PROT_VND_SUN_STAR_HELP:
            // already a vnd.sun.star.help URL -> nothing to do
            aHelpURL = rURL;
            break;
        default:
        {
            OUString aHelpModuleName( GetHelpModuleName_Impl() );
            // no URL, just a HelpID (maybe empty in case of keyword search)
            aHelpURL = CreateHelpURL_Impl( rURL, aHelpModuleName );

            if ( impl_hasHelpInstalled() && pWindow && SfxContentHelper::IsHelpErrorDocument( aHelpURL ) )
            {
                // no help found -> try with parent help id.
                Window* pParent = pWindow->GetParent();
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
                            Dialog *pDialog = ((Dialog*)pParent);
                            TabControl *pCtrl = pDialog->hasBuilder() ? pDialog->get<TabControl>("tabcontrol") : NULL;
                            TabPage* pTabPage = pCtrl ? pCtrl->GetTabPage(pCtrl->GetCurPageId()) : NULL;
                            Window *pTabChild = pTabPage ? pTabPage->GetWindow(WINDOW_FIRSTCHILD) : NULL;
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
        if ( impl_showOnlineHelp( aHelpURL ) )
            return sal_True;
        else
        {
            NoHelpErrorBox aErrBox( const_cast< Window* >( pWindow ) );
            aErrBox.Execute();
            return sal_False;
        }
    }

    Reference < XDesktop2 > xDesktop = Desktop::create( ::comphelper::getProcessComponentContext() );

    // check if help window is still open
    // If not, create a new one and return access directly to the internal sub frame showing the help content
    // search must be done here; search one desktop level could return an arbitraty frame
    Reference< XFrame2 > xHelp(
        xDesktop->findFrame( "OFFICE_HELP_TASK", FrameSearchFlag::CHILDREN),
        UNO_QUERY);
    Reference< XFrame > xHelpContent = xDesktop->findFrame(
        OUString("OFFICE_HELP"),
        FrameSearchFlag::CHILDREN);

    SfxHelpWindow_Impl* pHelpWindow = 0;
    if (!xHelp.is())
        pHelpWindow = impl_createHelp(xHelp, xHelpContent);
    else
        pHelpWindow = (SfxHelpWindow_Impl*)VCLUnoHelper::GetWindow(xHelp->getComponentWindow());
    if (!xHelp.is() || !xHelpContent.is() || !pHelpWindow)
        return sal_False;

#ifdef DBG_UTIL
    OStringBuffer aTmp(RTL_CONSTASCII_STRINGPARAM("SfxHelp: HelpId = "));
    aTmp.append(OUStringToOString(aHelpURL, RTL_TEXTENCODING_UTF8));
    OSL_TRACE( aTmp.getStr() );
#endif

    pHelpWindow->SetHelpURL( aHelpURL );
    pHelpWindow->loadHelpContent(aHelpURL);
    if (!rKeyword.isEmpty())
        pHelpWindow->OpenKeyword( rKeyword );

    Reference < ::com::sun::star::awt::XTopWindow > xTopWindow( xHelp->getContainerWindow(), UNO_QUERY );
    if ( xTopWindow.is() )
        xTopWindow->toFront();

    return sal_True;
}

OUString SfxHelp::CreateHelpURL(const OUString& aCommandURL, const OUString& rModuleName)
{
    SfxHelp* pHelp = static_cast< SfxHelp* >(Application::GetHelp());
    return pHelp ? pHelp->CreateHelpURL_Impl( aCommandURL, rModuleName ) : OUString();
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
