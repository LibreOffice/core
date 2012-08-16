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
#include <com/sun/star/frame/XFrame.hpp>
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
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/system/XSystemShellExecute.hpp>
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
using namespace ::com::sun::star::frame;
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

#define STARTERLIST 0

static bool impl_hasHelpInstalled( const rtl::OUString &rLang );

/// Return the locale we prefer for displaying help
static rtl::OUString HelpLocaleString()
{
    static rtl::OUString aLocaleStr;
    if (aLocaleStr.isEmpty())
    {
        const rtl::OUString aEnglish( "en"  );
        // detect installed locale
        aLocaleStr = utl::ConfigManager::getLocale();
        bool bOk = !aLocaleStr.isEmpty();
        if ( !bOk )
            aLocaleStr = aEnglish;
        else
        {
            rtl::OUString aBaseInstallPath;
            utl::Bootstrap::locateBaseInstallation(aBaseInstallPath);
            static const char *szHelpPath = "/help/";

            rtl::OUString sHelpPath = aBaseInstallPath +
                rtl::OUString::createFromAscii(szHelpPath) + aLocaleStr;
            osl::DirectoryItem aDirItem;

            if (!osl::DirectoryItem::get(sHelpPath, aDirItem) == osl::FileBase::E_None)
            {
                bOk = false;
                String sLang(aLocaleStr);
                xub_StrLen nSepPos = sLang.Search( '-' );
                if (nSepPos != STRING_NOTFOUND)
                {
                    bOk = true;
                    sLang = sLang.Copy( 0, nSepPos );
                    sHelpPath = aBaseInstallPath +
                        rtl::OUString::createFromAscii(szHelpPath) + sLang;
                    if (!osl::DirectoryItem::get(sHelpPath, aDirItem) == osl::FileBase::E_None)
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

void AppendConfigToken( String& rURL, sal_Bool bQuestionMark, const rtl::OUString &rLang )
{
    ::rtl::OUString aLocaleStr( rLang );
    if ( aLocaleStr.isEmpty() )
        aLocaleStr = HelpLocaleString();

    // query part exists?
    if ( bQuestionMark )
        // no, so start with '?'
        rURL += '?';
    else
        // yes, so only append with '&'
        rURL += '&';

    // set parameters
    rURL += DEFINE_CONST_UNICODE("Language=");
    rURL += String( aLocaleStr );
    rURL += DEFINE_CONST_UNICODE("&System=");
    rURL += SvtHelpOptions().GetSystem();
    rURL += DEFINE_CONST_UNICODE("&Version=");
    rURL += utl::ConfigManager::getProductVersion();
}

sal_Bool GetHelpAnchor_Impl( const String& _rURL, String& _rAnchor )
{
    sal_Bool bRet = sal_False;
    ::rtl::OUString sAnchor;

    try
    {
        ::ucbhelper::Content aCnt( INetURLObject( _rURL ).GetMainURL( INetURLObject::NO_DECODE ),
                             Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        if ( ( aCnt.getPropertyValue( ::rtl::OUString("AnchorName") ) >>= sAnchor ) )
        {

            if ( !sAnchor.isEmpty() )
            {
                _rAnchor = String( sAnchor );
                bRet = sal_True;
            }
        }
        else
        {
            SAL_WARN( "sfx2.appl", "Property 'AnchorName' is missing" );
        }
    }
    catch (const ::com::sun::star::uno::Exception&)
    {
    }

    return bRet;
}

class SfxHelpOptions_Impl : public utl::ConfigItem
{
private:
    std::set < rtl::OString > m_aIds;

public:
                    SfxHelpOptions_Impl();
                    ~SfxHelpOptions_Impl();

    bool            HasId( const rtl::OString& rId ) { return m_aIds.size() ? m_aIds.find( rId ) != m_aIds.end() : false; }
    virtual void            Notify( const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
    virtual void            Commit();
};

static Sequence< ::rtl::OUString > GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "HelpAgentStarterList",
    };

    const int nCount = sizeof( aPropNames ) / sizeof( const char* );
    Sequence< ::rtl::OUString > aNames( nCount );
    ::rtl::OUString* pNames = aNames.getArray();
    ::rtl::OUString* pEnd   = pNames + aNames.getLength();
    int i = 0;
    for ( ; pNames != pEnd; ++pNames )
        *pNames = ::rtl::OUString::createFromAscii( aPropNames[i++] );

    return aNames;
}

SfxHelpOptions_Impl::SfxHelpOptions_Impl()
    : ConfigItem( ::rtl::OUString("Office.SFX/Help") )
{
    Sequence< ::rtl::OUString > aNames = GetPropertyNames();
    Sequence< Any > aValues = GetProperties( aNames );
    EnableNotification( aNames );
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT( aValues.getLength() == aNames.getLength(), "GetProperties failed" );
    if ( aValues.getLength() == aNames.getLength() )
    {
        for ( int nProp = 0; nProp < aNames.getLength(); nProp++ )
        {
            DBG_ASSERT( pValues[nProp].hasValue(), "property value missing" );
            if ( pValues[nProp].hasValue() )
            {
                switch ( nProp )
                {
                    case STARTERLIST :
                    {
                        ::rtl::OUString aCodedList;
                        if ( pValues[nProp] >>= aCodedList )
                        {
                            rtl::OString aTmp(
                                rtl::OUStringToOString(
                                    aCodedList, RTL_TEXTENCODING_UTF8 ) );
                            sal_Int32 nIndex = 0;
                            do
                            {
                                rtl::OString aToken = aTmp.getToken( 0, ',', nIndex );
                                if ( !aToken.isEmpty() )
                                    m_aIds.insert( aToken );
                            }
                            while ( nIndex >= 0 );
                        }
                        else {
                            SAL_WARN( "sfx2.appl", "Wrong property type!" );
                        }

                        break;
                    }

                    default:
                        SAL_WARN( "sfx2.appl", "Wrong property!" );
                        break;
                }
            }
        }
    }
}

SfxHelpOptions_Impl::~SfxHelpOptions_Impl()
{
}


void SfxHelpOptions_Impl::Notify( const com::sun::star::uno::Sequence< rtl::OUString >& )
{
}

void SfxHelpOptions_Impl::Commit()
{
}

class SfxHelp_Impl
{
private:
    SfxHelpOptions_Impl*                m_pOpt;         // the options
    ::std::vector< ::rtl::OUString >    m_aModulesList; // list of all installed modules

public:
    SfxHelp_Impl();
    ~SfxHelp_Impl();

    SfxHelpOptions_Impl*    GetOptions();
    static String           GetHelpText( const rtl::OUString& aCommandURL, const String& rModule );
};

SfxHelp_Impl::SfxHelp_Impl() :

    m_pOpt          ( NULL )

{
}

SfxHelp_Impl::~SfxHelp_Impl()
{
    delete m_pOpt;
}

String SfxHelp_Impl::GetHelpText( const rtl::OUString& aCommandURL, const String& rModule )
{
    // create help url
    String aHelpURL = SfxHelp::CreateHelpURL( aCommandURL, rModule );
    // added 'active' parameter
    aHelpURL.Insert( String( DEFINE_CONST_UNICODE("&Active=true") ), aHelpURL.SearchBackward( '#' ) );
    // load help string
    return SfxContentHelper::GetActiveHelpString( aHelpURL );
}

SfxHelpOptions_Impl* SfxHelp_Impl::GetOptions()
{
    // create if not exists
    if ( !m_pOpt )
        m_pOpt = new SfxHelpOptions_Impl;
    return m_pOpt;
}

SfxHelp::SfxHelp() :

    bIsDebug( sal_False ),
    pImp    ( NULL )

{
    // read the environment variable "HELP_DEBUG"
    // if it's set, you will see debug output on active help
    {
        ::rtl::OUString sHelpDebug;
        ::rtl::OUString sEnvVarName( "HELP_DEBUG"  );
        osl_getEnvironment( sEnvVarName.pData, &sHelpDebug.pData );
        bIsDebug = !sHelpDebug.isEmpty();
    }

    pImp = new SfxHelp_Impl();

    ::rtl::OUString aLocaleStr = HelpLocaleString();

    sal_Int32 nSepPos = aLocaleStr.indexOf( '_' );
    if ( nSepPos != -1 )
    {
        aLanguageStr = aLocaleStr.copy( 0, nSepPos );
        aCountryStr = aLocaleStr.copy( nSepPos+1 );
    }
    else
    {
        nSepPos = aLocaleStr.indexOf( '-' );
        if ( nSepPos != -1 )
        {
            aLanguageStr = aLocaleStr.copy( 0, nSepPos );
            aCountryStr = aLocaleStr.copy( nSepPos+1 );
        }
        else
        {
            aLanguageStr = aLocaleStr;
        }
    }
}

SfxHelp::~SfxHelp()
{
    delete pImp;
}

::rtl::OUString getDefaultModule_Impl()
{
    rtl::OUString sDefaultModule;
    SvtModuleOptions aModOpt;
    if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
        sDefaultModule = DEFINE_CONST_UNICODE("swriter");
    else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC ) )
        sDefaultModule = DEFINE_CONST_UNICODE("scalc");
    else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SIMPRESS ) )
        sDefaultModule = DEFINE_CONST_UNICODE("simpress");
    else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SDRAW ) )
        sDefaultModule = DEFINE_CONST_UNICODE("sdraw");
    else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SMATH ) )
        sDefaultModule = DEFINE_CONST_UNICODE("smath");
    else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SCHART ) )
        sDefaultModule = DEFINE_CONST_UNICODE("schart");
    else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SBASIC ) )
        sDefaultModule = DEFINE_CONST_UNICODE("sbasic");
    else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SDATABASE ) )
        sDefaultModule = DEFINE_CONST_UNICODE("sdatabase");
    else
    {
        SAL_WARN( "sfx2.appl", "getDefaultModule_Impl(): no module installed" );
    }
    return sDefaultModule;
}

::rtl::OUString getCurrentModuleIdentifier_Impl()
{
    ::rtl::OUString sIdentifier;
    Reference < XFrame > xCurrentFrame;
    Reference < XModuleManager > xModuleManager( ::comphelper::getProcessServiceFactory()->createInstance(
        DEFINE_CONST_UNICODE("com.sun.star.frame.ModuleManager") ), UNO_QUERY );
    Reference < XDesktop > xDesktop( ::comphelper::getProcessServiceFactory()->createInstance(
        DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );
    if ( xDesktop.is() )
        xCurrentFrame = xDesktop->getCurrentFrame();

    if ( xCurrentFrame.is() && xModuleManager.is() )
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
            SAL_WARN( "sfx2.appl", "SfxHelp::getCurrentModuleIdentifier_Impl(): exception of XModuleManager::identify()" );
        }
    }

    return sIdentifier;
}

String SfxHelp::GetHelpModuleName_Impl()
{
    String sModuleName;
    rtl::OUString aFactoryShortName;
    rtl::OUString aModuleIdentifier = getCurrentModuleIdentifier_Impl();

    if ( !aModuleIdentifier.isEmpty() )
    {
        try
        {
            Reference < XModuleManager > xModuleManager(
                ::comphelper::getProcessServiceFactory()->createInstance(
                    DEFINE_CONST_UNICODE("com.sun.star.frame.ModuleManager") ), UNO_QUERY );
            Sequence< PropertyValue > lProps;
            Reference< ::com::sun::star::container::XNameAccess > xCont( xModuleManager, UNO_QUERY);
            if ( xCont.is() )
                xCont->getByName( aModuleIdentifier ) >>= lProps;
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
            SAL_WARN( "sfx2.appl", "SfxHelp::GetHelpModuleName_Impl(): exception of XNameAccess::getByName()" );
        }
    }

    rtl::OUString sDefaultModule = getDefaultModule_Impl();
    if ( !aFactoryShortName.isEmpty() )
    {
        // Map some module identifiers to their "real" help module string.
        if ( aFactoryShortName == "chart2" )
            aFactoryShortName = rtl::OUString( "schart"  );
        else if ( aFactoryShortName == "BasicIDE" )
            aFactoryShortName = rtl::OUString( "sbasic"  );
        else if ( aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("sweb"))
                || aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("sglobal"))
                || aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("swxform")) )
            aFactoryShortName = rtl::OUString( "swriter"  );
        else if ( aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("dbquery"))
                || aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("dbbrowser"))
                || aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("dbrelation"))
                || aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("dbtable"))
                || aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("dbapp"))
                || aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("dbreport"))
                || aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("swreport"))
                || aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("dbbrowser"))
                || aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("swform")) )
            aFactoryShortName = rtl::OUString( "sdatabase"  );
        else if ( aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("sbibliography"))
                || aFactoryShortName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("StartModule")) )
            aFactoryShortName = sDefaultModule;
    }
    else
        aFactoryShortName = sDefaultModule;

    sModuleName = String( aFactoryShortName );
    return sModuleName;
}

String  SfxHelp::CreateHelpURL_Impl( const String& aCommandURL, const String& rModuleName )
{
    // build up the help URL
    String aHelpURL;
    sal_Bool bHasAnchor = sal_False;
    String aAnchor;

    String aModuleName( rModuleName );
    if ( aModuleName.Len() == 0 )
        aModuleName = getDefaultModule_Impl();

    aHelpURL = rtl::OUString("vnd.sun.star.help://");
    aHelpURL += aModuleName;

    if ( !aCommandURL.Len() )
        aHelpURL += rtl::OUString("/start");
    else
    {
        aHelpURL += '/';
        aHelpURL += String( rtl::Uri::encode( aCommandURL,
                                              rtl_UriCharClassRelSegment,
                                              rtl_UriEncodeKeepEscapes,
                                              RTL_TEXTENCODING_UTF8 ));

        String aTempURL = aHelpURL;
        AppendConfigToken( aTempURL, sal_True );
        bHasAnchor = GetHelpAnchor_Impl( aTempURL, aAnchor );
    }

    AppendConfigToken( aHelpURL, sal_True );

    if ( bHasAnchor )
    {
        aHelpURL += '#';
        aHelpURL += aAnchor;
    }

    return aHelpURL;
}

SfxHelpWindow_Impl* impl_createHelp(Reference< XFrame >& rHelpTask   ,
                                    Reference< XFrame >& rHelpContent)
{
    Reference < XFrame > xDesktop( ::comphelper::getProcessServiceFactory()->createInstance(
        DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );

    // otherwhise - create new help task
    Reference< XFrame > xHelpTask = xDesktop->findFrame(
        ::rtl::OUString(DEFINE_CONST_UNICODE("OFFICE_HELP_TASK")),
        FrameSearchFlag::TASKS | FrameSearchFlag::CREATE);
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
        xHelpTask->setName( ::rtl::OUString(DEFINE_CONST_UNICODE("OFFICE_HELP_TASK")) );

        Reference< XPropertySet > xProps(xHelpTask, UNO_QUERY);
        if (xProps.is())
            xProps->setPropertyValue(
                DEFINE_CONST_UNICODE("Title"),
                makeAny(SfxResId(STR_HELP_WINDOW_TITLE).toString()));

        pHelpWindow->setContainerWindow( xParentWindow );
        xParentWindow->setVisible(sal_True);
        xHelpWindow->setVisible(sal_True);

        // This sub frame is created internaly (if we called new SfxHelpWindow_Impl() ...)
        // It should exist :-)
        xHelpContent = xHelpTask->findFrame(::rtl::OUString(DEFINE_CONST_UNICODE("OFFICE_HELP")), FrameSearchFlag::CHILDREN);
    }

    if (!xHelpContent.is())
        delete pHelpWindow;

    xHelpContent->setName(::rtl::OUString(DEFINE_CONST_UNICODE("OFFICE_HELP")));

    rHelpTask    = xHelpTask;
    rHelpContent = xHelpContent;
    return pHelpWindow;
}

XubString SfxHelp::GetHelpText( const String& aCommandURL, const Window* pWindow )
{
    String sModuleName = GetHelpModuleName_Impl();
    String sHelpText = pImp->GetHelpText( aCommandURL, sModuleName );

    rtl::OString aNewHelpId;

    if ( pWindow && !sHelpText.Len() )
    {
        // no help text found -> try with parent help id.
        Window* pParent = pWindow->GetParent();
        while ( pParent )
        {
            aNewHelpId = pParent->GetHelpId();
            sHelpText = pImp->GetHelpText( rtl::OStringToOUString(aNewHelpId, RTL_TEXTENCODING_UTF8), sModuleName );
            if ( sHelpText.Len() > 0 )
                pParent = NULL;
            else
                pParent = pParent->GetParent();
        }

        if ( bIsDebug && !sHelpText.Len() )
            aNewHelpId = rtl::OString();
    }

    // add some debug information?
    if ( bIsDebug )
    {
        sHelpText += DEFINE_CONST_UNICODE("\n-------------\n");
        sHelpText += String( sModuleName );
        sHelpText += DEFINE_CONST_UNICODE(": ");
        sHelpText += aCommandURL;
        if ( !aNewHelpId.isEmpty() )
        {
            sHelpText += DEFINE_CONST_UNICODE(" - ");
            sHelpText += String(rtl::OStringToOUString(aNewHelpId, RTL_TEXTENCODING_UTF8));
        }
    }

    return sHelpText;
}

/// Check for built-in help
static bool impl_hasHelpInstalled( const rtl::OUString &rLang = rtl::OUString() )
{
    String aHelpRootURL( DEFINE_CONST_OUSTRING("vnd.sun.star.help://") );
    AppendConfigToken( aHelpRootURL, sal_True, rLang );
    Sequence< ::rtl::OUString > aFactories = SfxContentHelper::GetResultSet( aHelpRootURL );

    return ( aFactories.getLength() != 0   );
}

sal_Bool SfxHelp::SearchKeyword( const XubString& rKeyword )
{
    return Start_Impl( String(), NULL, rKeyword );
}

sal_Bool SfxHelp::Start( const String& rURL, const Window* pWindow )
{
    return Start_Impl( rURL, pWindow, String() );
}

/// Redirect the vnd.sun.star.help:// urls to http://help.libreoffice.org
static bool impl_showOnlineHelp( const String& rURL )
{
    String aInternal( "vnd.sun.star.help://"  );
    if ( rURL.Len() <= aInternal.Len() || rURL.Copy( 0, aInternal.Len() ) != aInternal )
        return false;

    rtl::OUString aHelpLink( "http://help.libreoffice.org/"  );
    aHelpLink += rURL.Copy( aInternal.Len() );
    try
    {
        Reference< XSystemShellExecute > xSystemShell(
                ::comphelper::getProcessServiceFactory()->createInstance(
                    rtl::OUString( "com.sun.star.system.SystemShellExecute"  ) ),
                UNO_QUERY );

        if ( xSystemShell.is() )
        {
            xSystemShell->execute( aHelpLink, rtl::OUString(), SystemShellExecuteFlags::URIS_ONLY );
            return true;
        }
    }
    catch (const Exception&)
    {
    }
    return false;
}

sal_Bool SfxHelp::Start_Impl( const String& rURL, const Window* pWindow, const String& rKeyword )
{
    String aHelpRootURL( DEFINE_CONST_OUSTRING("vnd.sun.star.help://") );
    AppendConfigToken( aHelpRootURL, sal_True);
    Sequence< ::rtl::OUString > aFactories = SfxContentHelper::GetResultSet( aHelpRootURL );

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
    String aHelpURL;
    INetURLObject aParser( rURL );
    INetProtocol nProtocol = aParser.GetProtocol();
    String aHelpModuleName( GetHelpModuleName_Impl() );

    switch ( nProtocol )
    {
        case INET_PROT_VND_SUN_STAR_HELP:
            // already a vnd.sun.star.help URL -> nothing to do
            aHelpURL = rURL;
            break;
        default:
        {
            // no URL, just a HelpID (maybe empty in case of keyword search)
            aHelpURL = CreateHelpURL_Impl( rURL, aHelpModuleName );

            if ( impl_hasHelpInstalled() && pWindow && SfxContentHelper::IsHelpErrorDocument( aHelpURL ) )
            {
                // no help found -> try with parent help id.
                Window* pParent = pWindow->GetParent();
                while ( pParent )
                {
                    rtl::OString aHelpId = pParent->GetHelpId();
                    aHelpURL = CreateHelpURL( rtl::OStringToOUString(aHelpId, RTL_TEXTENCODING_UTF8), aHelpModuleName );
                    if ( !SfxContentHelper::IsHelpErrorDocument( aHelpURL ) )
                        break;
                    else
                    {
                        pParent = pParent->GetParent();
                        if ( !pParent )
                            // create help url of start page ( helpid == 0 -> start page)
                            aHelpURL = CreateHelpURL( String(), aHelpModuleName );
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

    Reference < XFrame > xDesktop( ::comphelper::getProcessServiceFactory()->createInstance(
        DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );

    // check if help window is still open
    // If not, create a new one and return access directly to the internal sub frame showing the help content
    // search must be done here; search one desktop level could return an arbitraty frame
    Reference< XFrame > xHelp = xDesktop->findFrame(
        ::rtl::OUString(DEFINE_CONST_UNICODE("OFFICE_HELP_TASK")),
        FrameSearchFlag::CHILDREN);
    Reference< XFrame > xHelpContent = xDesktop->findFrame(
        ::rtl::OUString(DEFINE_CONST_UNICODE("OFFICE_HELP")),
        FrameSearchFlag::CHILDREN);

    SfxHelpWindow_Impl* pHelpWindow = 0;
    if (!xHelp.is())
        pHelpWindow = impl_createHelp(xHelp, xHelpContent);
    else
        pHelpWindow = (SfxHelpWindow_Impl*)VCLUnoHelper::GetWindow(xHelp->getComponentWindow());
    if (!xHelp.is() || !xHelpContent.is() || !pHelpWindow)
        return sal_False;

#ifdef DBG_UTIL
    rtl::OStringBuffer aTmp(RTL_CONSTASCII_STRINGPARAM("SfxHelp: HelpId = "));
    aTmp.append(rtl::OUStringToOString(aHelpURL, RTL_TEXTENCODING_UTF8));
    OSL_TRACE( aTmp.getStr() );
#endif

    pHelpWindow->SetHelpURL( aHelpURL );
    pHelpWindow->loadHelpContent(aHelpURL);
    if ( rKeyword.Len() )
        pHelpWindow->OpenKeyword( rKeyword );

    Reference < ::com::sun::star::awt::XTopWindow > xTopWindow( xHelp->getContainerWindow(), UNO_QUERY );
    if ( xTopWindow.is() )
        xTopWindow->toFront();

    return sal_True;
}

String SfxHelp::CreateHelpURL( const String& aCommandURL, const String& rModuleName )
{
    String aURL;
    SfxHelp* pHelp = (static_cast< SfxHelp* >(Application::GetHelp()) );
    if ( pHelp )
        aURL = pHelp->CreateHelpURL_Impl( aCommandURL, rModuleName );
    return aURL;
}

void SfxHelp::OpenHelpAgent( SfxFrame*, const rtl::OString& sHelpId )
{
    SfxHelp* pHelp = (static_cast< SfxHelp* >(Application::GetHelp()) );
    if ( pHelp )
        pHelp->OpenHelpAgent( sHelpId );
}

void SfxHelp::OpenHelpAgent( const rtl::OString& sHelpId )
{
    if ( SvtHelpOptions().IsHelpAgentAutoStartMode() )
    {
            SfxHelpOptions_Impl *pOpt = pImp->GetOptions();
            if ( !pOpt->HasId( sHelpId ) )
                return;

            try
            {
                URL aURL;
                aURL.Complete = CreateHelpURL_Impl( rtl::OStringToOUString(sHelpId, RTL_TEXTENCODING_UTF8), GetHelpModuleName_Impl() );
                Reference< XURLTransformer > xTrans( URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
                xTrans->parseStrict(aURL);

                Reference < XFrame > xCurrentFrame;
                Reference < XDesktop > xDesktop( ::comphelper::getProcessServiceFactory()->createInstance(
                    DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );
                if ( xDesktop.is() )
                    xCurrentFrame = xDesktop->getCurrentFrame();

                Reference< XDispatchProvider > xDispProv( xCurrentFrame, UNO_QUERY );
                Reference< XDispatch > xHelpDispatch;
                if ( xDispProv.is() )
                    xHelpDispatch = xDispProv->queryDispatch(
                        aURL, ::rtl::OUString("_helpagent"),
                        FrameSearchFlag::PARENT | FrameSearchFlag::SELF );

                DBG_ASSERT( xHelpDispatch.is(), "OpenHelpAgent: could not get a dispatcher!" );
                if ( xHelpDispatch.is() )
                    xHelpDispatch->dispatch( aURL, Sequence< PropertyValue >() );
            }
            catch (const Exception&)
            {
                SAL_WARN( "sfx2.appl", "OpenHelpAgent: caught an exception while executing the dispatch!" );
            }
    }
}

String SfxHelp::GetDefaultHelpModule()
{
    return getDefaultModule_Impl();
}

::rtl::OUString SfxHelp::GetCurrentModuleIdentifier()
{
    return getCurrentModuleIdentifier_Impl();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
