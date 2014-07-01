/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_folders.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <LibreOfficeKit/LibreOfficeKit.h>

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
#include <unotools/mediadescriptor.hxx>
#include <osl/module.hxx>

using namespace css;
using namespace utl;

using namespace boost;

struct LibLODocument_Impl;
struct LibLibreOffice_Impl;

static LibLibreOffice_Impl *gImpl = NULL;
static weak_ptr< LibreOfficeKitClass > gOfficeClass;
static weak_ptr< LibreOfficeKitDocumentClass > gDocumentClass;

typedef struct
{
    const char *extn;
    const char *filterName;
} ExtensionMap;

static const ExtensionMap aWriterExtensionMap[] =
{
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

static const ExtensionMap aCalcExtensionMap[] =
{
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

static const ExtensionMap aImpressExtensionMap[] =
{
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

static const ExtensionMap aDrawExtensionMap[] =
{
    { "odg",   "draw8" },
    { "fodg",  "draw_ODG_FlatXML" },
    { "html",  "draw_html_Export" },
    { "svg",   "draw_svg_Export" },
    { "swf",   "draw_flash_Export" },
    { "xhtml", "XHTML Draw File" },
    { "vdx",   "draw_Visio_Document" },
    { "vsd",   "draw_Visio_Document" },
    { "vsdm",  "draw_Visio_Document" },
    { "vsdx",  "draw_Visio_Document" },
    { "pub",   "draw_Publisher_Document" },
    { "cdr",   "draw_CorelDraw_Document" },
    { "wpg",   "draw_WordPerfect_Graphics" },
    { NULL, NULL }
};

static OUString getUString(const char* pString)
{
    if (pString == NULL)
        return OUString();

    OString sString(pString, strlen(pString));
    return OStringToOUString(sString, RTL_TEXTENCODING_UTF8);
}

// Try to convert a relative URL to an absolute one
static OUString getAbsoluteURL(const char* pURL)
{
    OUString aURL( getUString( pURL ) );
    OUString sAbsoluteDocUrl, sWorkingDir, sDocPathUrl;

    // FIXME: this would appear to kill non-file URLs.
    osl_getProcessWorkingDir(&sWorkingDir.pData);
    osl::FileBase::getFileURLFromSystemPath( aURL, sDocPathUrl );
    osl::FileBase::getAbsoluteFileURL(sWorkingDir, sDocPathUrl, sAbsoluteDocUrl);

    return sAbsoluteDocUrl;
}

extern "C"
{

static void doc_destroy(LibreOfficeKitDocument* pThis);
static int  doc_saveAs(LibreOfficeKitDocument* pThis, const char* pUrl, const char* pFormat, const char* pFilterOptions);

struct LibLODocument_Impl : public _LibreOfficeKitDocument
{
    uno::Reference<css::lang::XComponent> mxComponent;
    shared_ptr< LibreOfficeKitDocumentClass > m_pDocumentClass;

    LibLODocument_Impl(const uno::Reference <css::lang::XComponent> &xComponent) :
        mxComponent( xComponent )
    {
        if (!(m_pDocumentClass = gDocumentClass.lock()))
        {
            m_pDocumentClass.reset(new LibreOfficeKitDocumentClass);

            m_pDocumentClass->nSize = sizeof(LibreOfficeKitDocument);

            m_pDocumentClass->destroy = doc_destroy;
            m_pDocumentClass->saveAs = doc_saveAs;

            gDocumentClass = m_pDocumentClass;
        }
        pClass = m_pDocumentClass.get();
    }

    ~LibLODocument_Impl()
    {
        mxComponent->dispose();
    }
};

static void doc_destroy(LibreOfficeKitDocument *pThis)
{
    LibLODocument_Impl *pDocument = static_cast<LibLODocument_Impl*>(pThis);
    delete pDocument;
}

static void                    lo_destroy       (LibreOfficeKit* pThis);
static int                     lo_initialize    (LibreOfficeKit* pThis, const char* pInstallPath);
static LibreOfficeKitDocument* lo_documentLoad  (LibreOfficeKit* pThis, const char* pURL);
static char *                  lo_getError      (LibreOfficeKit* pThis);

struct LibLibreOffice_Impl : public _LibreOfficeKit
{
    OUString maLastExceptionMsg;
    shared_ptr< LibreOfficeKitClass > m_pOfficeClass;

    LibLibreOffice_Impl()
    {
        if(!(m_pOfficeClass = gOfficeClass.lock())) {
            m_pOfficeClass.reset(new LibreOfficeKitClass);
            m_pOfficeClass->nSize = sizeof(LibreOfficeKitClass);

            m_pOfficeClass->destroy = lo_destroy;
            m_pOfficeClass->documentLoad = lo_documentLoad;
            m_pOfficeClass->getError = lo_getError;

            gOfficeClass = m_pOfficeClass;
        }

        pClass = m_pOfficeClass.get();
    }
};

// Wonder global state ...
static uno::Reference<css::uno::XComponentContext> xContext;
static uno::Reference<css::lang::XMultiServiceFactory> xSFactory;
static uno::Reference<css::lang::XMultiComponentFactory> xFactory;

static LibreOfficeKitDocument* lo_documentLoad(LibreOfficeKit* pThis, const char* pURL)
{
    LibLibreOffice_Impl* pLib = static_cast<LibLibreOffice_Impl*>(pThis);

    OUString aURL = getAbsoluteURL(pURL);

    uno::Reference<frame::XDesktop2> xComponentLoader = frame::Desktop::create(xContext);

    pLib->maLastExceptionMsg = "";

    try
    {
        uno::Reference<lang::XComponent> xComponent;
        xComponent = xComponentLoader->loadComponentFromURL(
                                            aURL, OUString("_blank"), 0,
                                            uno::Sequence<css::beans::PropertyValue>());

        if (xComponent.is())
            return new LibLODocument_Impl(xComponent);
        else
            pLib->maLastExceptionMsg = "unknown load failure";

    }
    catch (const uno::Exception& exception)
    {
        pLib->maLastExceptionMsg = exception.Message;
    }

    return NULL;
}

static int doc_saveAs(LibreOfficeKitDocument* pThis, const char* sUrl, const char* pFormat, const char* pFilterOptions)
{
    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);

    OUString sFormat = getUString(pFormat);
    OUString aURL = getAbsoluteURL(sUrl);

    try
    {
        uno::Reference<frame::XModel> xDocument(pDocument->mxComponent, uno::UNO_QUERY_THROW);
        uno::Sequence<beans::PropertyValue> aSequence = xDocument->getArgs();

        MediaDescriptor aMediaDescriptor(aSequence);
        OUString sPropertyName = MediaDescriptor::PROP_DOCUMENTSERVICE();
        OUString aDocumentService = aMediaDescriptor.getUnpackedValueOrDefault(sPropertyName, OUString());

        if (aDocumentService.isEmpty())
        {
            gImpl->maLastExceptionMsg = "unknown document type";
            return false;
        }

        const ExtensionMap* pMap;
        if (aDocumentService == "com.sun.star.sheet.SpreadsheetDocument")
            pMap = (const ExtensionMap*) aCalcExtensionMap;
        else if (aDocumentService == "com.sun.star.presentation.PresentationDocument")
            pMap = (const ExtensionMap*) aImpressExtensionMap;
        else if (aDocumentService == "com.sun.star.drawing.DrawingDocument")
            pMap = (const ExtensionMap*) aDrawExtensionMap;
        else if (aDocumentService == "com.sun.star.text.TextDocument")
            pMap = (const ExtensionMap*) aWriterExtensionMap;
        else
        {
            gImpl->maLastExceptionMsg = "unknown document mapping";
            return false;
        }

        if (pFormat == NULL)
        {
            // sniff from the extension
            sal_Int32 idx = aURL.lastIndexOf(".");
            if( idx > 0 )
            {
                sFormat = aURL.copy( idx + 1 );
            }
            else
            {
                gImpl->maLastExceptionMsg = "input filename without a suffix";
                return false;
            }
        }

        OUString aFilterName;
        for (sal_Int32 i = 0; pMap[i].extn; ++i)
        {
            if (sFormat.equalsIgnoreAsciiCaseAscii(pMap[i].extn))
            {
                aFilterName = getUString(pMap[i].filterName);
                break;
            }
        }
        if (aFilterName.isEmpty())
        {
            gImpl->maLastExceptionMsg = "no output filter found for provided suffix";
            return false;
        }

        OUString aFilterOptions = getUString(pFilterOptions);

        MediaDescriptor aSaveMediaDescriptor;
        aSaveMediaDescriptor["Overwrite"] <<= sal_True;
        aSaveMediaDescriptor["FilterName"] <<= aFilterName;
        aSaveMediaDescriptor[MediaDescriptor::PROP_FILTEROPTIONS()] <<= aFilterOptions;

        uno::Reference<frame::XStorable> xStorable(pDocument->mxComponent, uno::UNO_QUERY_THROW);
        xStorable->storeToURL(aURL, aSaveMediaDescriptor.getAsConstPropertyValueList());

        return true;
    }
    catch (const uno::Exception& exception)
    {
        gImpl->maLastExceptionMsg = "exception: " + exception.Message;
    }
    return false;
}

static char* lo_getError (LibreOfficeKit *pThis)
{
    LibLibreOffice_Impl* pLib = static_cast<LibLibreOffice_Impl*>(pThis);
    OString aString = OUStringToOString(pLib->maLastExceptionMsg, RTL_TEXTENCODING_UTF8);
    char* pMemory = (char*) malloc(aString.getLength() + 1);
    strcpy(pMemory, aString.getStr());
    return pMemory;
}

static void force_c_locale(void)
{
    // force locale (and resource files loaded) to en-US
    OUString aLangISO("en-US");
    LanguageTag aLocale(aLangISO);
    ResMgr::SetDefaultLocale(aLocale);
    SvtSysLocaleOptions aLocalOptions;
    aLocalOptions.SetLocaleConfigString(aLangISO);
    aLocalOptions.SetUILocaleConfigString(aLangISO);
}

static void aBasicErrorFunc(const OUString& rError, const OUString& rAction)
{
    OStringBuffer aBuffer("Unexpected dialog: ");
    aBuffer.append(OUStringToOString(rAction, RTL_TEXTENCODING_ASCII_US));
    aBuffer.append(" Error: ");
    aBuffer.append(OUStringToOString(rError, RTL_TEXTENCODING_ASCII_US));

    fprintf(stderr, "Unexpected basic error dialog '%s'\n", aBuffer.getStr());
}

static void initialize_uno(const OUString &aAppProgramURL)
{
    rtl::Bootstrap::setIniFilename(aAppProgramURL + "/" + SAL_CONFIGFILE("soffice"));

    xContext = cppu::defaultBootstrap_InitialComponentContext();
    fprintf(stderr, "Uno initialized %d\n", xContext.is());
    xFactory = xContext->getServiceManager();
    xSFactory = uno::Reference<lang::XMultiServiceFactory>(xFactory, uno::UNO_QUERY_THROW);
    comphelper::setProcessServiceFactory(xSFactory);

    // set UserInstallation to user profile dir in test/user-template
//    rtl::Bootstrap aDefaultVars;
//    aDefaultVars.set(OUString("UserInstallation"), aAppProgramURL + "../registry" );
    // configmgr setup ?
}

static int lo_initialize(LibreOfficeKit* pThis, const char* pAppPath)
{
    (void) pThis;

    static bool bInitialized = false;
    if (bInitialized)
        return 1;

    if (!pAppPath)
        return 0;

    OUString aAppPath;
    if (pAppPath)
    {
        aAppPath = OUString(pAppPath, strlen(pAppPath), RTL_TEXTENCODING_UTF8);
    }
    else
    {
        ::osl::Module::getUrlFromAddress( reinterpret_cast< oslGenericFunction >(lo_initialize),
                                          aAppPath);
    }

    OUString aAppURL;
    if (osl::FileBase::getFileURLFromSystemPath(aAppPath, aAppURL) != osl::FileBase::E_None)
        return 0;

    try
    {
        osl_setCommandArgs(0, NULL);
        initialize_uno(aAppURL);
        force_c_locale();

        // Force headless
        rtl::Bootstrap::set("SAL_USE_VCLPLUGIN", "svp");
        InitVCL();
        Application::EnableHeadlessMode(true);

        ErrorHandler::RegisterDisplay(aBasicErrorFunc);

        fprintf(stderr, "initialized\n");
        bInitialized = true;
    }
    catch (css::uno::Exception& exception)
    {
        fprintf(stderr, "bootstrapping exception '%s'\n",
                 OUStringToOString(exception.Message, RTL_TEXTENCODING_UTF8).getStr());
    }
    return bInitialized;
}

SAL_DLLPUBLIC_EXPORT LibreOfficeKit *libreofficekit_hook(const char* install_path)
{
    if (!gImpl)
    {
        fprintf(stderr, "create libreoffice object\n");
        gImpl = new LibLibreOffice_Impl();
        if (!lo_initialize(gImpl, install_path))
        {
            lo_destroy(gImpl);
        }
    }
    return static_cast<LibreOfficeKit*>(gImpl);
}

static void lo_destroy(LibreOfficeKit *pThis)
{
    LibLibreOffice_Impl* pLib = static_cast<LibLibreOffice_Impl*>(pThis);
    delete pLib;
    gImpl = NULL;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
