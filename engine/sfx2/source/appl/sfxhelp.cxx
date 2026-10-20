/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <string_view>
#include <algorithm>
#include <cstddef>

#include <sal/log.hxx>
#include <cpo/uno/Reference.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/UnknownModuleException.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <i18nlangtag/languagetag.hxx>
#include <rtl/ustrbuf.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/moduleoptions.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <rtl/ustring.hxx>
#include <o3tl/environment.hxx>
#include <o3tl/string_view.hxx>
#include <officecfg/Office/Common.hxx>
#include <rtl/uri.hxx>
#include <vcl/commandinfoprovider.hxx>

#include <comphelper/kit.hxx>
#include <COKit/COKit.hxx>
#include <sfx2/viewsh.hxx>

#include <helper.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/diagnose_ex.hxx>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::cpo::uno;
using namespace cpo::uno;

/// Return the locale we prefer for displaying help
static OUString const & HelpLocaleString()
{
    return comphelper::COKit::getLanguageTag().getBcp47();
}

static void AppendConfigToken( OUStringBuffer& rURL, bool bQuestionMark )
{
    const OUString& aLocaleStr = HelpLocaleString();

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
    rURL.append(officecfg::Office::Common::Help::System::get());
    rURL.append("&Version=");
    rURL.append(utl::ConfigManager::getProductVersion());
}

static bool GetHelpAnchor_Impl( std::u16string_view _rURL, OUString& _rAnchor )
{
    bool bRet = false;

    try
    {
        ::ucbhelper::Content aCnt( INetURLObject( _rURL ).GetMainURL( INetURLObject::DecodeMechanism::NONE ),
                             Reference< css::ucb::XCommandEnvironment >(),
                             comphelper::getProcessComponentContext() );
        OUString sAnchor;
        if ( aCnt.getPropertyValue(u"AnchorName"_ustr) >>= sAnchor )
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
    catch (const cpo::uno::Exception&)
    {
    }

    return bRet;
}

namespace {

class SfxHelp_Impl
{
public:
    static OUString GetHelpText( const OUString& aCommandURL, const OUString& rModule );
};

}

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

SfxHelp::SfxHelp()
    : bIsDebug(false)
    , bLaunchingHelp(false)
{
    // read the environment variable "HELP_DEBUG"
    // if it's set, you will see debug output on active help
    bIsDebug = !o3tl::getEnvironment(u"HELP_DEBUG"_ustr).isEmpty();
}

SfxHelp::~SfxHelp()
{
}

static OUString getDefaultModule_Impl()
{
    OUString sDefaultModule;
    SvtModuleOptions aModOpt;
    if (aModOpt.IsWriterInstalled())
        sDefaultModule = u"swriter"_ustr;
    else if (aModOpt.IsCalcInstalled())
        sDefaultModule = u"scalc"_ustr;
    else if (aModOpt.IsImpressInstalled())
        sDefaultModule = u"simpress"_ustr;
    else if (aModOpt.IsDrawInstalled())
        sDefaultModule = u"sdraw"_ustr;
    else if (aModOpt.IsMathInstalled())
        sDefaultModule = u"smath"_ustr;
    else if (aModOpt.IsChartInstalled())
        sDefaultModule = u"schart"_ustr;
    else if (SvtModuleOptions::IsBasicIDEInstalled())
        sDefaultModule = u"sbasic"_ustr;
    else if (aModOpt.IsDataBaseInstalled())
        sDefaultModule = u"sdatabase"_ustr;
    else
    {
        SAL_WARN( "sfx.appl", "getDefaultModule_Impl(): no module installed" );
    }
    return sDefaultModule;
}

static OUString getCurrentModuleIdentifier_Impl()
{
    OUString sIdentifier;
    const Reference < XComponentContext >& xContext = ::comphelper::getProcessComponentContext();
    Reference < XModuleManager2 > xModuleManager = ModuleManager::create(xContext);
    Reference < XDesktop > xDesktop = Desktop::create(xContext);
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
        catch (const cpo::uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION( "sfx.appl", "SfxHelp::getCurrentModuleIdentifier_Impl(): exception of XModuleManager::identify()" );
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
            aFactoryShortName = u"schart"_ustr ;
        else if ( aFactoryShortName == "BasicIDE" )
            aFactoryShortName = u"sbasic"_ustr;
        else if ( aFactoryShortName == "sweb"
                || aFactoryShortName == "sglobal"
                || aFactoryShortName == "swxform" )
            aFactoryShortName = u"swriter"_ustr ;
        else if ( aFactoryShortName == "dbquery"
                || aFactoryShortName == "dbbrowser"
                || aFactoryShortName == "dbrelation"
                || aFactoryShortName == "dbtable"
                || aFactoryShortName == "dbapp"
                || aFactoryShortName == "dbtdata" )
            aFactoryShortName = u"sdatabase"_ustr;
        else if ( aFactoryShortName == "sbibliography"
                || aFactoryShortName == "sabpilot"
                || aFactoryShortName == "scanner"
                || aFactoryShortName == "spropctrlr"
                || aFactoryShortName == "StartModule" )
            aFactoryShortName.clear();

        return aFactoryShortName;
    }
}

OUString SfxHelp::GetHelpModuleName_Impl(std::u16string_view rHelpID)
{
    OUString aFactoryShortName;

    //rhbz#1438876 detect preferred module for this help id, e.g. csv dialog
    //for calc import before any toplevel is created and so context is
    //otherwise unknown. Cosmetic, same help is shown in any case because its
    //in the shared section, but title bar would state "Writer" when context is
    //expected to be "Calc"
    std::u16string_view sRemainder;
    if (o3tl::starts_with(rHelpID, u"modules/", &sRemainder))
    {
        std::size_t nEndModule = sRemainder.find(u'/');
        aFactoryShortName = nEndModule != std::u16string_view::npos
            ? sRemainder.substr(0, nEndModule) : sRemainder;
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
                auto pProp = std::find_if(std::cbegin(lProps), std::cend(lProps),
                    [](const PropertyValue& rProp) { return rProp.Name == "ooSetupFactoryShortName"; });
                if (pProp != std::cend(lProps))
                    pProp->Value >>= aFactoryShortName;
            }
            catch (const cpo::uno::Exception&)
            {
                TOOLS_WARN_EXCEPTION( "sfx.appl", "SfxHelp::GetHelpModuleName_Impl()" );
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
        aHelpURL.append("/" +
            rtl::Uri::encode(aCommandURL,
                              rtl_UriCharClassRelSegment,
                              rtl_UriEncodeKeepEscapes,
                              RTL_TEXTENCODING_UTF8));

        OUStringBuffer aTempURL = aHelpURL;
        AppendConfigToken( aTempURL, true );
        bHasAnchor = GetHelpAnchor_Impl(aTempURL, aAnchor);
    }

    AppendConfigToken( aHelpURL, true );

    if ( bHasAnchor )
        aHelpURL.append("#" + aAnchor);

    return aHelpURL.makeStringAndClear();
}

OUString SfxHelp::GetHelpText(const OUString& aCommandURL)
{
    OUString sModuleName = GetHelpModuleName_Impl(aCommandURL);
    auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(aCommandURL, getCurrentModuleIdentifier_Impl());
    OUString sRealCommand = vcl::CommandInfoProvider::GetRealCommandForCommand(aProperties);
    OUString sHelpText = SfxHelp_Impl::GetHelpText( sRealCommand.isEmpty() ? aCommandURL : sRealCommand, sModuleName );

    // add some debug information?
    if ( bIsDebug )
    {
        sHelpText += "\n-------------\n" +
            sModuleName + ": " + aCommandURL;
    }

    return sHelpText;
}

void SfxHelp::SearchKeyword( const OUString& /*rKeyword*/ )
{
    Start_Impl(OUString());
}

bool SfxHelp::Start( const OUString& rURL, const vcl::Window* /*pWindow*/ )
{
    if (bLaunchingHelp)
        return true;
    bLaunchingHelp = true;
    bool bRet = Start_Impl(rURL);
    bLaunchingHelp = false;
    return bRet;
}

bool SfxHelp::Start(const OUString& rURL, weld::Widget* /*pWidget*/)
{
    if (bLaunchingHelp)
        return true;
    bLaunchingHelp = true;
    bool bRet = Start_Impl(rURL);
    bLaunchingHelp = false;
    return bRet;
}

/// Redirect the vnd.sun.star.help:// urls to http://help.libreoffice.org
static bool impl_showOnlineHelp(const OUString& rURL)
{
    static constexpr OUString aInternal(u"vnd.sun.star.help://"_ustr);
    if ( rURL.getLength() <= aInternal.getLength() || !rURL.startsWith(aInternal) )
        return false;

    OUString aHelpLink = officecfg::Office::Common::Help::HelpRootURL::get();
    OUString aTarget = OUString::Concat("Target=") + rURL.subView(aInternal.getLength());
    aTarget = aTarget.replaceAll("%2F", "/").replaceAll("?", "&");
    aHelpLink += aTarget;

    if(SfxViewShell* pViewShell = SfxViewShell::Current())
    {
        pViewShell->viewCallback(COKitCallbackType::HYPERLINK_CLICKED,
                                               aHelpLink.toUtf8());
        return true;
    }
    else if (GetpApp())
    {
        GetpApp()->viewCallback(COKitCallbackType::HYPERLINK_CLICKED,
                                               aHelpLink.toUtf8());
        return true;
    }

    return false;
}

bool SfxHelp::Start_Impl(const OUString& rURL)
{
    OUStringBuffer aHelpRootURL("vnd.sun.star.help://");
    AppendConfigToken(aHelpRootURL, true);
    SfxContentHelper::GetResultSet(aHelpRootURL.makeStringAndClear());

    // rURL is either a vnd.sun.star.help URL or a help ID, which becomes a help URL.
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
            {
                // Command can be just an alias to another command.
                auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(rURL, getCurrentModuleIdentifier_Impl());
                aRealCommand = vcl::CommandInfoProvider::GetRealCommandForCommand(aProperties);
            }

            // no URL, just a HelpID (maybe empty in case of keyword search)
            aHelpURL = CreateHelpURL_Impl( aRealCommand.isEmpty() ? rURL : aRealCommand, aHelpModuleName );
            break;
        }
    }

    impl_showOnlineHelp(aHelpURL);
    return true;
}

OUString SfxHelp::CreateHelpURL(const OUString& aCommandURL, const OUString& rModuleName)
{
    SfxHelp* pHelp = static_cast< SfxHelp* >(Application::GetHelp());
    return pHelp ? SfxHelp::CreateHelpURL_Impl( aCommandURL, rModuleName ) : OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
