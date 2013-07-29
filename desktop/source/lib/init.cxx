/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "liblibreoffice.hxx"

#include <tools/errinf.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/strbuf.hxx>
#include <rtl/bootstrap.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
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

class LibLODocument_Impl;
class LibLibreOffice_Impl;

static LibLibreOffice_Impl *gImpl = NULL;

typedef struct {
    const char *extn;
    const char *filterName;
} ExtensionMap;

static const ExtensionMap
aWriterExtensionMap[] = {
    { "doc",   "MS Word 97" },
    { "docx",  "MS Word 2007 XML" },
    { "fodt",  "OpenDocument Text Flat XML" },
    { "html",  "HTML (StarWriter)" },
    { "odt",   "writer8" },
    { "ott",   "writer8_template" },
    { "pdf",   "writer_pdf_Export" },
    { "txt",   "Text" },
    { "xhtml", "XHTML Writer File" },
    { NULL, NULL }
};

static const ExtensionMap
aCalcExtensionMap[] = {
    { "csv",   "Text - txt - csv (StarCalc)" },
    { "fods",  "OpenDocument Spreadsheet Flat XML" },
    { "html",  "HTML (StarCalc)" },
    { "ods",   "calc8" },
    { "ots",   "calc8_template" },
    { "pdf",   "calc_pdf_Export" },
    { "xhtml", "XHTML Calc File" },
    { "xls",   "MS Excel 97" },
    { "xlsx",  "Calc MS Excel 2007 XML" },
    { NULL, NULL }
};

static const ExtensionMap
aImpressExtensionMap[] = {
    { "fodp",  "OpenDocument Presentation Flat XML" },
    { "html",  "impress_html_Export" },
    { "odg",   "impress8_draw" },
    { "odp",   "impress8" },
    { "otp",   "impress8_template" },
    { "pdf",   "impress_pdf_Export" },
    { "potm",  "Impress MS PowerPoint 2007 XML Template" },
    { "pot",   "MS PowerPoint 97 Vorlage" },
    { "pptx",  "Impress MS PowerPoint 2007 XML" },
    { "pps",   "MS PowerPoint 97 Autoplay" },
    { "ppt",   "MS PowerPoint 97" },
    { "svg",   "impress_svg_Export" },
    { "swf",   "impress_flash_Export" },
    { "xhtml", "XHTML Impress File" },
    { NULL, NULL }
};


class LibLODocument_Impl : public LODocument
{
    uno::Reference < css::lang::XComponent > mxComponent;
public:
    LibLODocument_Impl( const uno::Reference < css::lang::XComponent > &xComponent )
            : mxComponent( xComponent )
        { }
    virtual bool saveAs (const char *url, const char *format);
};

class LibLibreOffice_Impl : public LibLibreOffice
{
public:
    rtl::OUString       maLastExceptionMsg;

    virtual bool        initialize (const char *installPath);

    virtual LODocument *documentLoad (const char *url);

    virtual char       *getError();

    virtual ~LibLibreOffice_Impl ();
};

// Wonder global state ...
static uno::Reference<css::uno::XComponentContext> xContext;
static uno::Reference<css::lang::XMultiServiceFactory> xSFactory;
static uno::Reference<css::lang::XMultiComponentFactory> xFactory;

static OUString getUString( const char *str )
{
    if( !str )
        return OUString( "" );
    return OStringToOUString( OString( str, strlen (str) ),
                              RTL_TEXTENCODING_UTF8 );
}

LODocument *
LibLibreOffice_Impl::documentLoad( const char *docUrl )
{
    OUString sUrl = getUString( docUrl );
    OUString sAbsoluteDocUrl, sWorkingDir, sDocPathUrl;

    uno::Reference < css::frame::XDesktop2 > xComponentLoader =
            css::frame::Desktop::create(xContext);

    osl_getProcessWorkingDir(&sWorkingDir.pData);
    osl::FileBase::getFileURLFromSystemPath( sUrl, sDocPathUrl );
    osl::FileBase::getAbsoluteFileURL(sWorkingDir, sDocPathUrl, sAbsoluteDocUrl);

    maLastExceptionMsg = "";
    try {
        uno::Reference < css::lang::XComponent > xComponent =
            xComponentLoader->loadComponentFromURL(
                sAbsoluteDocUrl, OUString("_blank"), 0,
                uno::Sequence < css::beans::PropertyValue >());
        if( xComponentLoader.is() )
            return new LibLODocument_Impl( xComponent );
        else
            maLastExceptionMsg = "unknown load failure";
    } catch (const uno::Exception &ex) {
        maLastExceptionMsg = ex.Message;
    }
    return NULL;
}

bool LibLODocument_Impl::saveAs (const char *url, const char *format)
{
    OUString sURL = getUString( url );
    OUString sFormat = getUString( format );

    try {
        uno::Reference< frame::XModel > xDocument( mxComponent, uno::UNO_QUERY_THROW );
        uno::Sequence< beans::PropertyValue > aSeq = xDocument->getArgs();

        OUString aFilterName, aDocumentService;
        for( sal_Int32 i = 0; i < aSeq.getLength(); ++i )
        {
            if( aSeq[i].Name == "FilterName" )
                aSeq[i].Value >>= aFilterName;
            else if( aSeq[i].Name == "DocumentService" )
                aSeq[i].Value >>= aDocumentService;
            OUString aValue;
            aSeq[i].Value >>= aValue;
        }

        if( aDocumentService == "")
        {
            gImpl->maLastExceptionMsg = "Unknown document type";
            return false;
        }
        const ExtensionMap *pMap;

        if( aDocumentService == "com.sun.star.sheet.SpreadsheetDocument" )
            pMap = (const ExtensionMap *)aCalcExtensionMap;
        else if( aDocumentService == "com.sun.star.presentation.PresentationDocument" )
            pMap = (const ExtensionMap *)aImpressExtensionMap;
        else // for the sake of argument only writer documents ...
            pMap = (const ExtensionMap *)aWriterExtensionMap;

        if( format )
        {
            for( sal_Int32 i = 0; pMap[i].extn; i++ )
            {
                if( sFormat.equalsIgnoreAsciiCaseAscii( pMap[i].extn ) )
                {
                    aFilterName = getUString( pMap[i].filterName );
                    break;
                }
            }
        }

        aSeq.realloc(2);
        aSeq[0].Name = "Overwrite";
        aSeq[0].Value <<= sal_True;
        aSeq[1].Name = "FilterName";
        aSeq[1].Value <<= aFilterName;

        uno::Reference< frame::XStorable > xStorable( mxComponent, uno::UNO_QUERY_THROW );
        xStorable->storeToURL( sURL, aSeq );

        return true;
    } catch (const uno::Exception &ex) {
        gImpl->maLastExceptionMsg = "exception " + ex.Message;
        return false;
    }
}

char *LibLibreOffice_Impl::getError()
{
    OString aStr = rtl::OUStringToOString( maLastExceptionMsg, RTL_TEXTENCODING_UTF8 );
    char *pMem = (char *) malloc (aStr.getLength() + 1);
    strcpy( pMem, aStr.getStr() );
    return pMem;
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
initialize_uno( const OUString &aAppURL )
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
//    aDefaultVars.set(OUString("UserInstallation"), aAppURL + "../registry" );
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
    if( !gImpl )
    {
        fprintf( stderr, "create libreoffice object\n" );
        gImpl = new LibLibreOffice_Impl();
    }
    return gImpl;
}

LibLibreOffice_Impl::~LibLibreOffice_Impl ()
{
    gImpl = NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
