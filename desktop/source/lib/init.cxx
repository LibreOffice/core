/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <liblibreoffice_impl.hxx>

#include <stdio.h>

#include <tools/errinf.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/strbuf.hxx>
#include <rtl/bootstrap.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XUniversalContentBroker.hpp>

#include <vcl/svapp.hxx>
#include <tools/resmgr.hxx>
#include <vcl/graphicfilter.hxx>
#include <unotools/syslocaleoptions.hxx>

using namespace ::com::sun::star;

// Wonder global state ...
static uno::Reference<css::uno::XComponentContext> xContext;
static uno::Reference<css::lang::XMultiServiceFactory> xSFactory;
static uno::Reference<css::lang::XMultiComponentFactory> xFactory;

LODocument *
LibLibreOffice_Impl::documentLoad( const char *docUrl )
{
    OUString sUrl = OUString::createFromAscii (docUrl);
    OUString sAbsoluteDocUrl, sWorkingDir, sDocPathUrl;

    uno::Reference < css::frame::XDesktop2 > xComponentLoader =
            css::frame::Desktop::create(xContext);

    osl_getProcessWorkingDir(&sWorkingDir.pData);
    osl::FileBase::getFileURLFromSystemPath(sUrl, sDocPathUrl);
    osl::FileBase::getAbsoluteFileURL(sWorkingDir, sDocPathUrl, sAbsoluteDocUrl);

    uno::Reference < css::lang::XComponent > xComponent = xComponentLoader->loadComponentFromURL(
            sAbsoluteDocUrl, OUString("_blank"), 0,
            uno::Sequence < css::beans::PropertyValue >());
    return NULL;
}

bool
LibLibreOffice_Impl::documentSave( const char * )
{
    return 1;
}

static void
force_c_locale( void )
{
    // force locale (and resource files loaded) to en-US
    OUString aLangISO( "en-US" );
    LanguageTag aLocale( aLangISO );
    ResMgr::SetDefaultLocale( aLocale );
    SvtSysLocaleOptions aLocalOptions;
    aLocalOptions.SetLocaleConfigString( aLangISO );
    aLocalOptions.SetUILocaleConfigString( aLangISO );
}

static void
aBasicErrorFunc( const OUString &rErr, const OUString &rAction )
{
    OStringBuffer aErr( "Unexpected dialog: " );
    aErr.append( OUStringToOString( rAction, RTL_TEXTENCODING_ASCII_US ) );
    aErr.append( " Error: " );
    aErr.append( OUStringToOString( rErr, RTL_TEXTENCODING_ASCII_US ) );
    fprintf( stderr, "Unexpected basic error dialog '%s'\n", aErr.getStr() );
}

static void
initialize_uno( const rtl::OUString &aAppURL )
{
    rtl::Bootstrap::setIniFilename( aAppURL + "/fundamentalrc" );

    rtl::Bootstrap::set( "CONFIGURATION_LAYERS",
                         "xcsxcu:${BRAND_BASE_DIR}/share/registry "
                         "res:${BRAND_BASE_DIR}/share/registry "
//                       "bundledext:${${BRAND_BASE_DIR}/program/unorc:BUNDLED_EXTENSIONS_USER}/registry/com.sun.star.comp.deployment.configuration.PackageRegistryBackend/configmgr.ini " );
//                       "sharedext:${${BRAND_BASE_DIR}/program/unorc:SHARED_EXTENSIONS_USER}/registry/com.sun.star.comp.deployment.configuration.PackageRegistryBackend/configmgr.ini "
//                       "userext:${${BRAND_BASE_DIR}/program/unorc:UNO_USER_PACKAGES_CACHE}/registry/com.sun.star.comp.deployment.configuration.PackageRegistryBackend/configmgr.ini "
//                         "user:${$BRAND_BASE_DIR/program/bootstraprc:UserInstallation}/user/registrymodifications.xcu"
                         );

    xContext = cppu::defaultBootstrap_InitialComponentContext();
    fprintf( stderr, "Uno initialized %d\n", xContext.is() );
    xFactory = xContext->getServiceManager();
    xSFactory = uno::Reference<lang::XMultiServiceFactory>(xFactory, uno::UNO_QUERY_THROW);
    comphelper::setProcessServiceFactory(xSFactory);

    // set UserInstallation to user profile dir in test/user-template
//    rtl::Bootstrap aDefaultVars;
//    aDefaultVars.set(rtl::OUString("UserInstallation"), aAppURL + "../registry" );
    // configmgr setup ?
}

bool
LibLibreOffice_Impl::initialize( const char *app_path )
{
    static bool bInitialized = false;
    if( bInitialized )
        return true;

    if( !app_path )
        return false;

    OUString aAppPath( app_path, strlen( app_path ), RTL_TEXTENCODING_UTF8 );
    OUString aAppURL;
    if( osl::FileBase::getFileURLFromSystemPath( aAppPath, aAppURL ) !=
        osl::FileBase::E_None )
        return false;

    try {
        initialize_uno( aAppURL );
        force_c_locale();

        // Force headless
        rtl::Bootstrap::set( "SAL_USE_VCLPLUGIN", "svp" );
        InitVCL();
        Application::EnableHeadlessMode(true);

        ErrorHandler::RegisterDisplay( aBasicErrorFunc );

        fprintf( stderr, "initialized\n" );
        bInitialized = true;
    } catch (css::uno::Exception & e) {
        fprintf( stderr, "bootstrapping exception '%s'\n",
                 OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
    }
    return bInitialized;
}

extern "C" {
    SAL_DLLPUBLIC_EXPORT LibLibreOffice *liblibreoffice_hook(void);
}

LibLibreOffice *liblibreoffice_hook(void)
{
    fprintf( stderr, "create libreoffice object\n" );
    return new LibLibreOffice_Impl();
}

LibLibreOffice_Impl::~LibLibreOffice_Impl ()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
