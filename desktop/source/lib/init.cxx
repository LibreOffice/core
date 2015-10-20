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

#include <memory>
#include <boost/property_tree/json_parser.hpp>

#define LOK_USE_UNSTABLE_API
#include <LibreOfficeKit/LibreOfficeKit.h>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <sal/log.hxx>
#include <tools/errinf.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <osl/thread.h>
#include <rtl/bootstrap.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/uri.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XUniversalContentBroker.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

#include <editeng/fontitem.hxx>
#include <editeng/flstitem.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/lokhelper.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <svx/svxids.hrc>
#include <vcl/svapp.hxx>
#include <vcl/svpforlokit.hxx>
#include <tools/resmgr.hxx>
#include <tools/fract.hxx>
#include <svtools/ctrltool.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/virdev.hxx>
#include <vcl/ITiledRenderable.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <unotools/mediadescriptor.hxx>
#include <osl/module.hxx>
#include <comphelper/sequence.hxx>

#include <app.hxx>

#include "../app/cmdlineargs.hxx"
// We also need to hackily be able to start the main libreoffice thread:
#include "../app/sofficemain.h"
#include "../app/officeipcthread.hxx"
#include "../../inc/lib/init.hxx"

#include "lokinteractionhandler.hxx"

using namespace css;
using namespace vcl;
using namespace desktop;
using namespace utl;

static LibLibreOffice_Impl *gImpl = NULL;
static std::weak_ptr< LibreOfficeKitClass > gOfficeClass;
static std::weak_ptr< LibreOfficeKitDocumentClass > gDocumentClass;

typedef struct
{
    const char *extn;
    const char *filterName;
} ExtensionMap;

// We need a shared_array for passing into the BitmapDevice (via
// VirtualDevice.SetOutputSizePixelScaleOffsetAndBuffer which goes via the
// SvpVirtualDevice, ending up in the basebmp BitmapDevice. However as we're
// given the array externally we can't delete it, and hence need to override
// shared_array's default of deleting its pointer.
template<typename T>
struct NoDelete
{
   void operator()(T* /* p */) {}
};

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
    { "png",   "writer_png_Export" },
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
    { "png",   "impress_png_Export"},
    { NULL, NULL }
};

static const ExtensionMap aDrawExtensionMap[] =
{
    { "fodg",  "draw_ODG_FlatXML" },
    { "html",  "draw_html_Export" },
    { "odg",   "draw8" },
    { "pdf",   "draw_pdf_Export" },
    { "svg",   "draw_svg_Export" },
    { "swf",   "draw_flash_Export" },
    { "xhtml", "XHTML Draw File" },
    { "png",   "draw_png_Export"},
    { NULL, NULL }
};

static OUString getUString(const char* pString)
{
    if (pString == NULL)
        return OUString();

    OString sString(pString, strlen(pString));
    return OStringToOUString(sString, RTL_TEXTENCODING_UTF8);
}

/// Try to convert a relative URL to an absolute one, unless it already looks like an URL.
static OUString getAbsoluteURL(const char* pURL)
{
    OUString aURL(getUString(pURL));
    if (aURL.isEmpty())
        return aURL;

    // convert relative paths to absolute ones
    OUString aWorkingDir;
    osl_getProcessWorkingDir(&aWorkingDir.pData);
    if (!aWorkingDir.endsWith("/"))
        aWorkingDir += "/";

    try {
        return rtl::Uri::convertRelToAbs(aWorkingDir, aURL);
    }
    catch (const rtl::MalformedUriException &)
    {
    }

    return OUString();
}

extern "C"
{

static void doc_destroy(LibreOfficeKitDocument* pThis);
static int  doc_saveAs(LibreOfficeKitDocument* pThis, const char* pUrl, const char* pFormat, const char* pFilterOptions);
static int doc_getDocumentType(LibreOfficeKitDocument* pThis);
static int doc_getParts(LibreOfficeKitDocument* pThis);
static char* doc_getPartPageRectangles(LibreOfficeKitDocument* pThis);
static int doc_getPart(LibreOfficeKitDocument* pThis);
static void doc_setPart(LibreOfficeKitDocument* pThis, int nPart);
static char* doc_getPartName(LibreOfficeKitDocument* pThis, int nPart);
static void doc_setPartMode(LibreOfficeKitDocument* pThis, int nPartMode);
void        doc_paintTile(LibreOfficeKitDocument* pThis,
                          unsigned char* pBuffer,
                          const int nCanvasWidth, const int nCanvasHeight,
                          const int nTilePosX, const int nTilePosY,
                          const int nTileWidth, const int nTileHeight);
static void doc_getDocumentSize(LibreOfficeKitDocument* pThis,
                                long* pWidth,
                                long* pHeight);
static void doc_initializeForRendering(LibreOfficeKitDocument* pThis);

static void doc_registerCallback(LibreOfficeKitDocument* pThis,
                                LibreOfficeKitCallback pCallback,
                                void* pData);
static void doc_postKeyEvent(LibreOfficeKitDocument* pThis,
                             int nType,
                             int nCharCode,
                             int nKeyCode);
static void doc_postMouseEvent (LibreOfficeKitDocument* pThis,
                                int nType,
                                int nX,
                                int nY,
                                int nCount,
                                int nButtons,
                                int nModifier);
static void doc_postUnoCommand(LibreOfficeKitDocument* pThis,
                               const char* pCommand,
                               const char* pArguments);
static void doc_setTextSelection (LibreOfficeKitDocument* pThis,
                                  int nType,
                                  int nX,
                                  int nY);
static char* doc_getTextSelection(LibreOfficeKitDocument* pThis,
                                  const char* pMimeType,
                                  char** pUsedMimeType);
static void doc_setGraphicSelection (LibreOfficeKitDocument* pThis,
                                  int nType,
                                  int nX,
                                  int nY);
static void doc_resetSelection (LibreOfficeKitDocument* pThis);
static char* doc_getCommandValues(LibreOfficeKitDocument* pThis, const char* pCommand);

static int doc_createView(LibreOfficeKitDocument* pThis);
static void doc_destroyView(LibreOfficeKitDocument* pThis, int nId);
static void doc_setView(LibreOfficeKitDocument* pThis, int nId);
static int doc_getView(LibreOfficeKitDocument* pThis);
static int doc_getViews(LibreOfficeKitDocument* pThis);

LibLODocument_Impl::LibLODocument_Impl(const uno::Reference <css::lang::XComponent> &xComponent) :
    mxComponent( xComponent )
{
    if (!(m_pDocumentClass = gDocumentClass.lock()))
    {
        m_pDocumentClass.reset(new LibreOfficeKitDocumentClass);

        m_pDocumentClass->nSize = sizeof(LibreOfficeKitDocument);

        m_pDocumentClass->destroy = doc_destroy;
        m_pDocumentClass->saveAs = doc_saveAs;
        m_pDocumentClass->getDocumentType = doc_getDocumentType;
        m_pDocumentClass->getParts = doc_getParts;
        m_pDocumentClass->getPartPageRectangles = doc_getPartPageRectangles;
        m_pDocumentClass->getPart = doc_getPart;
        m_pDocumentClass->setPart = doc_setPart;
        m_pDocumentClass->getPartName = doc_getPartName;
        m_pDocumentClass->setPartMode = doc_setPartMode;
        m_pDocumentClass->paintTile = doc_paintTile;
        m_pDocumentClass->getDocumentSize = doc_getDocumentSize;
        m_pDocumentClass->initializeForRendering = doc_initializeForRendering;
        m_pDocumentClass->registerCallback = doc_registerCallback;
        m_pDocumentClass->postKeyEvent = doc_postKeyEvent;
        m_pDocumentClass->postMouseEvent = doc_postMouseEvent;
        m_pDocumentClass->postUnoCommand = doc_postUnoCommand;
        m_pDocumentClass->setTextSelection = doc_setTextSelection;
        m_pDocumentClass->getTextSelection = doc_getTextSelection;
        m_pDocumentClass->setGraphicSelection = doc_setGraphicSelection;
        m_pDocumentClass->resetSelection = doc_resetSelection;
        m_pDocumentClass->getCommandValues = doc_getCommandValues;

        m_pDocumentClass->createView = doc_createView;
        m_pDocumentClass->destroyView = doc_destroyView;
        m_pDocumentClass->setView = doc_setView;
        m_pDocumentClass->getView = doc_getView;
        m_pDocumentClass->getViews = doc_getViews;

        gDocumentClass = m_pDocumentClass;
    }
    pClass = m_pDocumentClass.get();
}

LibLODocument_Impl::~LibLODocument_Impl()
{
    mxComponent->dispose();
}

static void doc_destroy(LibreOfficeKitDocument *pThis)
{
    LibLODocument_Impl *pDocument = static_cast<LibLODocument_Impl*>(pThis);
    delete pDocument;
}

static void                    lo_destroy       (LibreOfficeKit* pThis);
static int                     lo_initialize    (LibreOfficeKit* pThis, const char* pInstallPath, const char* pUserProfilePath);
static LibreOfficeKitDocument* lo_documentLoad  (LibreOfficeKit* pThis, const char* pURL);
static char *                  lo_getError      (LibreOfficeKit* pThis);
static LibreOfficeKitDocument* lo_documentLoadWithOptions  (LibreOfficeKit* pThis,
                                                           const char* pURL,
                                                           const char* pOptions);
static void                    lo_registerCallback (LibreOfficeKit* pThis,
                                                    LibreOfficeKitCallback pCallback,
                                                    void* pData);
static char* lo_getFilterTypes(LibreOfficeKit* pThis);

LibLibreOffice_Impl::LibLibreOffice_Impl()
    : maThread(0)
    , mpCallback(nullptr)
    , mpCallbackData(nullptr)
{
    if(!(m_pOfficeClass = gOfficeClass.lock())) {
        m_pOfficeClass.reset(new LibreOfficeKitClass);
        m_pOfficeClass->nSize = sizeof(LibreOfficeKitClass);

        m_pOfficeClass->destroy = lo_destroy;
        m_pOfficeClass->documentLoad = lo_documentLoad;
        m_pOfficeClass->getError = lo_getError;
        m_pOfficeClass->documentLoadWithOptions = lo_documentLoadWithOptions;
        m_pOfficeClass->registerCallback = lo_registerCallback;
        m_pOfficeClass->getFilterTypes = lo_getFilterTypes;

        gOfficeClass = m_pOfficeClass;
    }

    pClass = m_pOfficeClass.get();
}

namespace
{

ITiledRenderable* getTiledRenderable(LibreOfficeKitDocument* pThis)
{
    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);
    return dynamic_cast<ITiledRenderable*>(pDocument->mxComponent.get());
}

} // anonymous namespace

// Wonder global state ...
static uno::Reference<css::uno::XComponentContext> xContext;
static uno::Reference<css::lang::XMultiServiceFactory> xSFactory;
static uno::Reference<css::lang::XMultiComponentFactory> xFactory;

static LibreOfficeKitDocument* lo_documentLoad(LibreOfficeKit* pThis, const char* pURL)
{
    return lo_documentLoadWithOptions(pThis, pURL, NULL);
}

static LibreOfficeKitDocument* lo_documentLoadWithOptions(LibreOfficeKit* pThis, const char* pURL, const char* pOptions)
{
    LibLibreOffice_Impl* pLib = static_cast<LibLibreOffice_Impl*>(pThis);

    SolarMutexGuard aGuard;

    OUString aURL(getAbsoluteURL(pURL));
    if (aURL.isEmpty())
    {
        pLib->maLastExceptionMsg = "Filename to load was not provided.";
        SAL_INFO("lok", "URL for load is empty");
        return NULL;
    }

    pLib->maLastExceptionMsg.clear();

    if (!xContext.is())
    {
        pLib->maLastExceptionMsg = "ComponentContext is not available";
        SAL_INFO("lok", "ComponentContext is not available");
        return NULL;
    }

    uno::Reference<frame::XDesktop2> xComponentLoader = frame::Desktop::create(xContext);

    if (!xComponentLoader.is())
    {
        pLib->maLastExceptionMsg = "ComponentLoader is not available";
        SAL_INFO("lok", "ComponentLoader is not available");
        return NULL;
    }

    try
    {
        uno::Sequence<css::beans::PropertyValue> aFilterOptions(2);
        aFilterOptions[0] = css::beans::PropertyValue( OUString("FilterOptions"),
                                                       0,
                                                       uno::makeAny(OUString::createFromAscii(pOptions)),
                                                       beans::PropertyState_DIRECT_VALUE);

        uno::Reference<task::XInteractionHandler2> xInteraction(new LOKInteractionHandler(::comphelper::getProcessComponentContext()));
        aFilterOptions[1].Name = "InteractionHandler";
        aFilterOptions[1].Value <<= xInteraction;

        /* TODO
        sal_Int16 nMacroExecMode = document::MacroExecMode::USE_CONFIG;
        aFilterOptions[2].Name = "MacroExecutionMode";
        aFilterOptions[2].Value <<= nMacroExecMode;

        sal_Int16 nUpdateDoc = document::UpdateDocMode::ACCORDING_TO_CONFIG;
        aFilterOptions[3].Name = "UpdateDocMode";
        aFilterOptions[3].Value <<= nUpdateDoc;
        */

        uno::Reference<lang::XComponent> xComponent;
        xComponent = xComponentLoader->loadComponentFromURL(
                                            aURL, OUString("_blank"), 0,
                                            aFilterOptions);

        if (!xComponent.is())
        {
            pLib->maLastExceptionMsg = "loadComponentFromURL returned an empty reference";
            SAL_INFO("lok", "Document can't be loaded - " << pLib->maLastExceptionMsg);
            return NULL;
        }

        return new LibLODocument_Impl(xComponent);

    }
    catch (const uno::Exception& exception)
    {
        pLib->maLastExceptionMsg = exception.Message;
        SAL_INFO("lok", "Document can't be loaded - exception: " << exception.Message);
    }

    return NULL;
}

static void lo_registerCallback (LibreOfficeKit* pThis,
                                 LibreOfficeKitCallback pCallback,
                                 void* pData)
{
    LibLibreOffice_Impl* pLib = static_cast<LibLibreOffice_Impl*>(pThis);

    pLib->mpCallback = pCallback;
    pLib->mpCallbackData = pData;
}

static int doc_saveAs(LibreOfficeKitDocument* pThis, const char* sUrl, const char* pFormat, const char* pFilterOptions)
{
    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);

    OUString sFormat = getUString(pFormat);
    OUString aURL(getAbsoluteURL(sUrl));
    if (aURL.isEmpty())
    {
        gImpl->maLastExceptionMsg = "Filename to save to was not provided.";
        SAL_INFO("lok", "URL for save is empty");
        return false;
    }

    try
    {
        const ExtensionMap* pMap;

        switch (doc_getDocumentType(pThis))
        {
        case LOK_DOCTYPE_SPREADSHEET:
            pMap = aCalcExtensionMap;
            break;
        case LOK_DOCTYPE_PRESENTATION:
            pMap = aImpressExtensionMap;
            break;
        case LOK_DOCTYPE_DRAWING:
            pMap = aDrawExtensionMap;
            break;
        case LOK_DOCTYPE_TEXT:
            pMap = aWriterExtensionMap;
            break;
        case LOK_DOCTYPE_OTHER:
        default:
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

static void doc_iniUnoCommands ()
{
    OUString sUnoCommands[] =
    {
        OUString(".uno:Bold"),
        OUString(".uno:Italic"),
        OUString(".uno:Underline"),
        OUString(".uno:Strikeout"),
        OUString(".uno:DefaultBullet"),
        OUString(".uno:DefaultNumbering"),
        OUString(".uno:LeftPara"),
        OUString(".uno:CenterPara"),
        OUString(".uno:RightPara"),
        OUString(".uno:JustifyPara"),
        OUString(".uno:IncrementIndent"),
        OUString(".uno:DecrementIndent"),
        OUString(".uno:CharFontName"),
        OUString(".uno:FontHeight"),
        OUString(".uno:StyleApply")
    };

    util::URL aCommandURL;
    SfxViewShell* pViewShell = SfxViewShell::Current();
    SfxViewFrame* pViewFrame = pViewShell? pViewShell->GetViewFrame(): NULL;

    // check if Frame-Controller were created.
    if (!pViewShell && !pViewFrame)
    {
        SAL_WARN("lok", "iniUnoCommands: No Frame-Controller created.");
        return;
    }

    if (!xContext.is())
        xContext = comphelper::getProcessComponentContext();
    if (!xContext.is())
    {
        SAL_WARN("lok", "iniUnoCommands: Component context is not available");
        return;
    }

    SfxSlotPool& rSlotPool = SfxSlotPool::GetSlotPool(pViewFrame);
    uno::Reference<util::XURLTransformer> xParser(util::URLTransformer::create(xContext));

    for (sal_uInt32 nIterator = 0; nIterator < SAL_N_ELEMENTS(sUnoCommands); nIterator++)
    {
        const SfxSlot* pSlot = NULL;

        aCommandURL.Complete = sUnoCommands[nIterator];
        xParser->parseStrict(aCommandURL);
        pSlot = rSlotPool.GetUnoSlot(aCommandURL.Path);

        // when null, this command is not supported by the given component
        // (like eg. Calc does not have ".uno:DefaultBullet" etc.)
        if (pSlot)
        {
            // Initialize slot to dispatch .uno: Command.
            pViewFrame->GetBindings().GetDispatch(pSlot, aCommandURL, false);
        }
    }
}

static int doc_getDocumentType (LibreOfficeKitDocument* pThis)
{
    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);

    try
    {
        uno::Reference<lang::XServiceInfo> xDocument(pDocument->mxComponent, uno::UNO_QUERY_THROW);

        if (xDocument->supportsService("com.sun.star.sheet.SpreadsheetDocument"))
        {
            return LOK_DOCTYPE_SPREADSHEET;
        }
        else if (xDocument->supportsService("com.sun.star.presentation.PresentationDocument"))
        {
            return LOK_DOCTYPE_PRESENTATION;
        }
        else if (xDocument->supportsService("com.sun.star.drawing.DrawingDocument"))
        {
            return LOK_DOCTYPE_DRAWING;
        }
        else if (xDocument->supportsService("com.sun.star.text.TextDocument"))
        {
            return LOK_DOCTYPE_TEXT;
        }
        else
        {
            gImpl->maLastExceptionMsg = "unknown document type";
        }
    }
    catch (const uno::Exception& exception)
    {
        gImpl->maLastExceptionMsg = "exception: " + exception.Message;
    }
    return LOK_DOCTYPE_OTHER;
}

static int doc_getParts (LibreOfficeKitDocument* pThis)
{
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        gImpl->maLastExceptionMsg = "Document doesn't support tiled rendering";
        return 0;
    }

    return pDoc->getParts();
}

static int doc_getPart (LibreOfficeKitDocument* pThis)
{
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        gImpl->maLastExceptionMsg = "Document doesn't support tiled rendering";
        return 0;
    }

    return pDoc->getPart();
}

static void doc_setPart(LibreOfficeKitDocument* pThis, int nPart)
{
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        gImpl->maLastExceptionMsg = "Document doesn't support tiled rendering";
        return;
    }

    SolarMutexGuard aGuard;
    pDoc->setPart( nPart );
}

static char* doc_getPartPageRectangles(LibreOfficeKitDocument* pThis)
{
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        gImpl->maLastExceptionMsg = "Document doesn't support tiled rendering";
        return 0;
    }

    OUString sRectangles = pDoc->getPartPageRectangles();
    OString aString = OUStringToOString(sRectangles, RTL_TEXTENCODING_UTF8);
    char* pMemory = static_cast<char*>(malloc(aString.getLength() + 1));
    strcpy(pMemory, aString.getStr());
    return pMemory;

}

static char* doc_getPartName(LibreOfficeKitDocument* pThis, int nPart)
{
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        gImpl->maLastExceptionMsg = "Document doesn't support tiled rendering";
        return 0;
    }

    OUString sName = pDoc->getPartName( nPart );
    OString aString = OUStringToOString(sName, RTL_TEXTENCODING_UTF8);
    char* pMemory = static_cast<char*>(malloc(aString.getLength() + 1));
    strcpy(pMemory, aString.getStr());
    return pMemory;

}

static void doc_setPartMode(LibreOfficeKitDocument* pThis,
                            int nPartMode)
{
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        gImpl->maLastExceptionMsg = "Document doesn't support tiled rendering";
        return;
    }

    SolarMutexGuard aGuard;

    int nCurrentPart = pDoc->getPart();

    pDoc->setPartMode(nPartMode);

    // We need to make sure the internal state is updated, just changing the mode
    // might not update the relevant shells (i.e. impress will keep rendering the
    // previous mode unless we do this).
    // TODO: we might want to do this within the relevant components rather than
    // here, but that's also dependent on how we implement embedded object
    // rendering I guess?
    // TODO: we could be clever and e.g. set to 0 when we change to/from
    // embedded object mode, and not when changing between slide/notes/combined
    // modes?
    if ( nCurrentPart < pDoc->getParts() )
    {
        pDoc->setPart( nCurrentPart );
    }
    else
    {
        pDoc->setPart( 0 );
    }
}

void doc_paintTile (LibreOfficeKitDocument* pThis,
                    unsigned char* pBuffer,
                    const int nCanvasWidth, const int nCanvasHeight,
                    const int nTilePosX, const int nTilePosY,
                    const int nTileWidth, const int nTileHeight)
{
    SAL_INFO( "lok.tiledrendering", "paintTile: painting [" << nTileWidth << "x" << nTileHeight <<
              "]@(" << nTilePosX << ", " << nTilePosY << ") to [" <<
              nCanvasWidth << "x" << nCanvasHeight << "]px" );

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        gImpl->maLastExceptionMsg = "Document doesn't support tiled rendering";
        return;
    }

    SolarMutexGuard aGuard;

#if defined(UNX) && !defined(MACOSX) && !defined(ENABLE_HEADLESS)

#ifndef IOS
    InitSvpForLibreOfficeKit();

    ScopedVclPtrInstance< VirtualDevice > pDevice(nullptr, Size(1, 1), (sal_uInt16)32) ;

    // Set background to transparent by default.
    memset(pBuffer, 0, nCanvasWidth * nCanvasHeight * 4);
    pDevice->SetBackground(Wallpaper(Color(COL_TRANSPARENT)));

    boost::shared_array< sal_uInt8 > aBuffer( pBuffer, NoDelete< sal_uInt8 >() );

    // Allocate a separate buffer for the alpha device.
    std::vector<sal_uInt8> aAlpha(nCanvasWidth * nCanvasHeight);
    memset(aAlpha.data(), 0, nCanvasWidth * nCanvasHeight);
    boost::shared_array<sal_uInt8> aAlphaBuffer(aAlpha.data(), NoDelete<sal_uInt8>());

    pDevice->SetOutputSizePixelScaleOffsetAndBuffer(
                Size(nCanvasWidth, nCanvasHeight), Fraction(1.0), Point(),
                aBuffer, aAlphaBuffer, true );

    pDoc->paintTile(*pDevice.get(), nCanvasWidth, nCanvasHeight,
                    nTilePosX, nTilePosY, nTileWidth, nTileHeight);

    // Overwrite pBuffer's alpha channel with the separate alpha buffer.
    for (int nRow = 0; nRow < nCanvasHeight; ++nRow)
    {
        for (int nCol = 0; nCol < nCanvasWidth; ++nCol)
        {
            const int nOffset = (nCanvasWidth * nRow) + nCol;
            // VCL's transparent is 0, RGBA's transparent is 0xff.
            pBuffer[nOffset * 4 +3] = 0xff - aAlpha[nOffset];
        }
    }

#else
    SystemGraphicsData aData;
    aData.rCGContext = reinterpret_cast<CGContextRef>(pBuffer);
    // the Size argument is irrelevant, I hope
    ScopedVclPtrInstance<VirtualDevice> pDevice(&aData, Size(1, 1), (sal_uInt16)0);

    pDoc->paintTile(*pDevice.get(), nCanvasWidth, nCanvasHeight,
                    nTilePosX, nTilePosY, nTileWidth, nTileHeight);
#endif

    static bool bDebug = getenv("LOK_DEBUG") != 0;
    if (bDebug)
    {
        // Draw a small red rectangle in the top left corner so that it's easy to see where a new tile begins.
        Rectangle aRect(0, 0, 5, 5);
        aRect = pDevice->PixelToLogic(aRect);
        pDevice->Push(PushFlags::FILLCOLOR | PushFlags::LINECOLOR);
        pDevice->SetFillColor(COL_LIGHTRED);
        pDevice->SetLineColor();
        pDevice->DrawRect(aRect);
        pDevice->Pop();
    }

#else
    (void) pBuffer;
    (void) nCanvasWidth;
    (void) nCanvasHeight;
    (void) nTilePosX;
    (void) nTilePosY;
    (void) nTileWidth;
    (void) nTileHeight;
#endif
}

static void doc_getDocumentSize(LibreOfficeKitDocument* pThis,
                                long* pWidth,
                                long* pHeight)
{
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (pDoc)
    {
        Size aDocumentSize = pDoc->getDocumentSize();
        *pWidth = aDocumentSize.Width();
        *pHeight = aDocumentSize.Height();
    }
    else
    {
        gImpl->maLastExceptionMsg = "Document doesn't support tiled rendering";
    }
}

static void doc_initializeForRendering(LibreOfficeKitDocument* pThis)
{
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (pDoc)
    {
        doc_iniUnoCommands();
        pDoc->initializeForTiledRendering();
    }
}

static void doc_registerCallback(LibreOfficeKitDocument* pThis,
                                LibreOfficeKitCallback pCallback,
                                void* pData)
{
    if (comphelper::LibreOfficeKit::isViewCallback())
    {
        if (SfxViewShell* pViewShell = SfxViewFrame::Current()->GetViewShell())
            pViewShell->registerLibreOfficeKitViewCallback(pCallback, pData);
    }
    else
    {
        ITiledRenderable* pDoc = getTiledRenderable(pThis);
        if (!pDoc)
        {
            gImpl->maLastExceptionMsg = "Document doesn't support tiled rendering";
            return;
        }

        pDoc->registerCallback(pCallback, pData);
    }
}

static void doc_postKeyEvent(LibreOfficeKitDocument* pThis, int nType, int nCharCode, int nKeyCode)
{
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        gImpl->maLastExceptionMsg = "Document doesn't support tiled rendering";
        return;
    }

    pDoc->postKeyEvent(nType, nCharCode, nKeyCode);
}

static void jsonToPropertyValues(const char* pJSON, uno::Sequence<beans::PropertyValue>& rPropertyValues)
{
    std::vector<beans::PropertyValue> aArguments;
    if (pJSON)
    {
        boost::property_tree::ptree aTree;
        std::stringstream aStream(pJSON);
        boost::property_tree::read_json(aStream, aTree);

        for (const std::pair<std::string, boost::property_tree::ptree>& rPair : aTree)
        {
            const std::string& rType = rPair.second.get<std::string>("type");
            const std::string& rValue = rPair.second.get<std::string>("value");

            beans::PropertyValue aValue;
            aValue.Name = OUString::fromUtf8(rPair.first.c_str());
            if (rType == "string")
                aValue.Value <<= OUString::fromUtf8(rValue.c_str());
            else if (rType == "boolean")
                aValue.Value <<= OString(rValue.c_str()).toBoolean();
            else if (rType == "float")
                aValue.Value <<= OString(rValue.c_str()).toFloat();
            else if (rType == "long")
                aValue.Value <<= OString(rValue.c_str()).toInt32();
            else if (rType == "unsigned short")
                aValue.Value <<= static_cast<sal_uInt16>(OString(rValue.c_str()).toUInt32());
            else
                SAL_WARN("desktop.lib", "jsonToPropertyValues: unhandled type '"<<rType<<"'");
            aArguments.push_back(aValue);
        }
    }
    rPropertyValues = comphelper::containerToSequence(aArguments);
}

static void doc_postUnoCommand(LibreOfficeKitDocument* /*pThis*/, const char* pCommand, const char* pArguments)
{
    OUString aCommand(pCommand, strlen(pCommand), RTL_TEXTENCODING_UTF8);

    uno::Sequence<beans::PropertyValue> aPropertyValues;
    jsonToPropertyValues(pArguments, aPropertyValues);
    if (!comphelper::dispatchCommand(aCommand, aPropertyValues))
    {
        gImpl->maLastExceptionMsg = "Failed to dispatch the .uno: command";
    }
}

static void doc_postMouseEvent(LibreOfficeKitDocument* pThis, int nType, int nX, int nY, int nCount, int nButtons, int nModifier)
{
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        gImpl->maLastExceptionMsg = "Document doesn't support tiled rendering";
        return;
    }

    pDoc->postMouseEvent(nType, nX, nY, nCount, nButtons, nModifier);
}

static void doc_setTextSelection(LibreOfficeKitDocument* pThis, int nType, int nX, int nY)
{
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        gImpl->maLastExceptionMsg = "Document doesn't support tiled rendering";
        return;
    }

    pDoc->setTextSelection(nType, nX, nY);
}

static char* doc_getTextSelection(LibreOfficeKitDocument* pThis, const char* pMimeType, char** pUsedMimeType)
{
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        gImpl->maLastExceptionMsg = "Document doesn't support tiled rendering";
        return 0;
    }

    OString aUsedMimeType;
    OString aRet = pDoc->getTextSelection(pMimeType, aUsedMimeType);
    if (aUsedMimeType.isEmpty())
        aRet = pDoc->getTextSelection("text/plain;charset=utf-8", aUsedMimeType);

    char* pMemory = static_cast<char*>(malloc(aRet.getLength() + 1));
    strcpy(pMemory, aRet.getStr());

    if (pUsedMimeType)
    {
        *pUsedMimeType = static_cast<char*>(malloc(aUsedMimeType.getLength() + 1));
        strcpy(*pUsedMimeType, aUsedMimeType.getStr());
    }

    return pMemory;
}

static void doc_setGraphicSelection(LibreOfficeKitDocument* pThis, int nType, int nX, int nY)
{
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        gImpl->maLastExceptionMsg = "Document doesn't support tiled rendering";
        return;
    }

    pDoc->setGraphicSelection(nType, nX, nY);
}

static void doc_resetSelection(LibreOfficeKitDocument* pThis)
{
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        gImpl->maLastExceptionMsg = "Document doesn't support tiled rendering";
        return;
    }

    pDoc->resetSelection();
}
static char* getFonts (const char* pCommand)
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SvxFontListItem* pFonts = static_cast<const SvxFontListItem*>(
        pDocSh->GetItem(SID_ATTR_CHAR_FONTLIST));
    const FontList* pList = pFonts ? pFonts->GetFontList() : 0;

    boost::property_tree::ptree aTree;
    aTree.put("commandName", pCommand);
    boost::property_tree::ptree aValues;
    if ( pList )
    {
        sal_uInt16 nFontCount = pList->GetFontNameCount();
        for (sal_uInt16 i = 0; i < nFontCount; ++i)
        {
            boost::property_tree::ptree aChildren;
            const vcl::FontInfo& rInfo = pList->GetFontName(i);
            const sal_IntPtr* pAry = pList->GetSizeAry(rInfo);
            sal_uInt16 nSizeCount = 0;
            while (pAry[nSizeCount])
            {
                boost::property_tree::ptree aChild;
                aChild.put("", (float)pAry[nSizeCount] / 10);
                aChildren.push_back(std::make_pair("", aChild));
                nSizeCount++;
            }
            aValues.add_child(rInfo.GetName().toUtf8().getStr(), aChildren);
        }
    }
    aTree.add_child("commandValues", aValues);
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);
    char* pJson = static_cast<char*>(malloc(aStream.str().size() + 1));
    strcpy(pJson, aStream.str().c_str());
    pJson[aStream.str().size()] = '\0';
    return pJson;
}

static char* getStyles(LibreOfficeKitDocument* pThis, const char* pCommand)
{
    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);

    boost::property_tree::ptree aTree;
    aTree.put("commandName", pCommand);
    uno::Reference<css::style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(pDocument->mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies(xStyleFamiliesSupplier->getStyleFamilies(), uno::UNO_QUERY);
    uno::Sequence<OUString> aStyleFamilies = xStyleFamilies->getElementNames();

    boost::property_tree::ptree aValues;
    for (sal_Int32 nStyleFam = 0; nStyleFam < aStyleFamilies.getLength(); ++nStyleFam)
    {
        boost::property_tree::ptree aChildren;
        OUString sStyleFam = aStyleFamilies[nStyleFam];
        uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName(sStyleFam), uno::UNO_QUERY);
        uno::Sequence<OUString> aStyles = xStyleFamily->getElementNames();
        for (sal_Int32 nInd = 0; nInd < aStyles.getLength(); ++nInd)
        {
            boost::property_tree::ptree aChild;
            aChild.put("", aStyles[nInd]);
            aChildren.push_back(std::make_pair("", aChild));
        }
        aValues.add_child(sStyleFam.toUtf8().getStr(), aChildren);
    }
    aTree.add_child("commandValues", aValues);
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);
    char* pJson = static_cast<char*>(malloc(aStream.str().size() + 1));
    strcpy(pJson, aStream.str().c_str());
    pJson[aStream.str().size()] = '\0';
    return pJson;
}

static char* doc_getCommandValues(LibreOfficeKitDocument* pThis, const char* pCommand)
{
    if (!strcmp(pCommand, ".uno:CharFontName"))
    {
        return getFonts(pCommand);
    }
    else if (!strcmp(pCommand, ".uno:StyleApply"))
    {
        return getStyles(pThis, pCommand);
    }
    else {
        gImpl->maLastExceptionMsg = "Unknown command, no values returned";
        return NULL;
    }
}

static int doc_createView(LibreOfficeKitDocument* /*pThis*/)
{
    SolarMutexGuard aGuard;

    return SfxLokHelper::createView();
}

static void doc_destroyView(LibreOfficeKitDocument* /*pThis*/, int nId)
{
    SolarMutexGuard aGuard;

    SfxLokHelper::destroyView(nId);
}

static void doc_setView(LibreOfficeKitDocument* /*pThis*/, int nId)
{
    SolarMutexGuard aGuard;

    SfxLokHelper::setView(nId);
}

static int doc_getView(LibreOfficeKitDocument* /*pThis*/)
{
    SolarMutexGuard aGuard;

    return SfxLokHelper::getView();
}

static int doc_getViews(LibreOfficeKitDocument* /*pThis*/)
{
    SolarMutexGuard aGuard;

    return SfxLokHelper::getViews();
}

static char* lo_getError (LibreOfficeKit *pThis)
{
    LibLibreOffice_Impl* pLib = static_cast<LibLibreOffice_Impl*>(pThis);
    OString aString = OUStringToOString(pLib->maLastExceptionMsg, RTL_TEXTENCODING_UTF8);
    char* pMemory = static_cast<char*>(malloc(aString.getLength() + 1));
    strcpy(pMemory, aString.getStr());
    return pMemory;
}

static char* lo_getFilterTypes(LibreOfficeKit* pThis)
{
    LibLibreOffice_Impl* pImpl = static_cast<LibLibreOffice_Impl*>(pThis);

    if (!xSFactory.is())
        xSFactory = comphelper::getProcessServiceFactory();

    if (!xSFactory.is())
    {
        pImpl->maLastExceptionMsg = "Service factory is not available";
        return 0;
    }

    uno::Reference<container::XNameAccess> xTypeDetection(xSFactory->createInstance("com.sun.star.document.TypeDetection"), uno::UNO_QUERY);
    uno::Sequence<OUString> aTypes = xTypeDetection->getElementNames();
    boost::property_tree::ptree aTree;
    for (const OUString& rType : aTypes)
    {
        uno::Sequence<beans::PropertyValue> aValues;
        if (xTypeDetection->getByName(rType) >>= aValues)
        {
            auto it = std::find_if(aValues.begin(), aValues.end(), [](const beans::PropertyValue& rValue) { return rValue.Name == "MediaType"; });
            OUString aValue;
            if (it != aValues.end() && (it->Value >>= aValue) && !aValue.isEmpty())
            {
                boost::property_tree::ptree aChild;
                aChild.put("MediaType", aValue.toUtf8());
                aTree.add_child(rType.toUtf8().getStr(), aChild);
            }
        }
    }
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);
    return strdup(aStream.str().c_str());
}

static void force_c_locale()
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

static bool initialize_uno(const OUString& aAppProgramURL)
{
#ifdef IOS
    // For iOS we already hardcode the inifile as "rc" in the .app directory.
    (void) aAppProgramURL;
#elif defined MACOSX
    rtl::Bootstrap::setIniFilename(aAppProgramURL + "/../Resources/" SAL_CONFIGFILE("soffice"));
#else
    rtl::Bootstrap::setIniFilename(aAppProgramURL + "/" SAL_CONFIGFILE("soffice"));
#endif

    xContext = cppu::defaultBootstrap_InitialComponentContext();
    if (!xContext.is())
    {
        gImpl->maLastExceptionMsg = "XComponentContext could not be created";
        SAL_INFO("lok", "XComponentContext could not be created");
        return false;
    }

    xFactory = xContext->getServiceManager();
    if (!xFactory.is())
    {
        gImpl->maLastExceptionMsg = "XMultiComponentFactory could not be created";
        SAL_INFO("lok", "XMultiComponentFactory could not be created");
        return false;
    }

    xSFactory = uno::Reference<lang::XMultiServiceFactory>(xFactory, uno::UNO_QUERY_THROW);
    if (!xSFactory.is())
    {
        gImpl->maLastExceptionMsg = "XMultiServiceFactory could not be created";
        SAL_INFO("lok", "XMultiServiceFactory could not be created");
        return false;
    }
    comphelper::setProcessServiceFactory(xSFactory);

    SAL_INFO("lok", "Uno initialized  - " <<  xContext.is());

    // set UserInstallation to user profile dir in test/user-template
//    rtl::Bootstrap aDefaultVars;
//    aDefaultVars.set(OUString("UserInstallation"), aAppProgramURL + "../registry" );
    // configmgr setup ?

    return true;
}

static void lo_startmain(void*)
{
    osl_setThreadName("lo_startmain");

    soffice_main();
}

static bool bInitialized = false;

static void lo_status_indicator_callback(void *data, comphelper::LibreOfficeKit::statusIndicatorCallbackType type, int percent)
{
    LibLibreOffice_Impl* pLib = static_cast<LibLibreOffice_Impl*>(data);

    if (!pLib->mpCallback)
        return;

    switch (type)
    {
    case comphelper::LibreOfficeKit::statusIndicatorCallbackType::Start:
        pLib->mpCallback(LOK_CALLBACK_STATUS_INDICATOR_START, 0, pLib->mpCallbackData);
        break;
    case comphelper::LibreOfficeKit::statusIndicatorCallbackType::SetValue:
        pLib->mpCallback(LOK_CALLBACK_STATUS_INDICATOR_SET_VALUE, OUString::number(percent).toUtf8().getStr(), pLib->mpCallbackData);
        break;
    case comphelper::LibreOfficeKit::statusIndicatorCallbackType::Finish:
        pLib->mpCallback(LOK_CALLBACK_STATUS_INDICATOR_FINISH, 0, pLib->mpCallbackData);
        break;
    }
}

static int lo_initialize(LibreOfficeKit* pThis, const char* pAppPath, const char* pUserProfilePath)
{
    LibLibreOffice_Impl* pLib = static_cast<LibLibreOffice_Impl*>(pThis);

    if (bInitialized)
        return 1;

    comphelper::LibreOfficeKit::setActive();

    static bool bViewCallback = getenv("LOK_VIEW_CALLBACK");
    comphelper::LibreOfficeKit::setViewCallback(bViewCallback);

    comphelper::LibreOfficeKit::setStatusIndicatorCallback(lo_status_indicator_callback, pLib);

    if (pUserProfilePath)
        rtl::Bootstrap::set(OUString("UserInstallation"), OUString(pUserProfilePath, strlen(pUserProfilePath), RTL_TEXTENCODING_UTF8));

    OUString aAppPath;
    if (pAppPath)
    {
        aAppPath = OUString(pAppPath, strlen(pAppPath), RTL_TEXTENCODING_UTF8);
    }
    else
    {
        // Fun conversion dance back and forth between URLs and system paths...
        OUString aAppURL;
        ::osl::Module::getUrlFromAddress( reinterpret_cast< oslGenericFunction >(lo_initialize),
                                          aAppURL);
        osl::FileBase::getSystemPathFromFileURL( aAppURL, aAppPath );
    }

    OUString aAppURL;
    if (osl::FileBase::getFileURLFromSystemPath(aAppPath, aAppURL) != osl::FileBase::E_None)
        return 0;

    try
    {
        SAL_INFO("lok", "Attempting to initalize UNO");
        if (!initialize_uno(aAppURL))
        {
            return false;
        }
        force_c_locale();

        // Force headless -- this is only for bitmap rendering.
        rtl::Bootstrap::set("SAL_USE_VCLPLUGIN", "svp");

        // We specifically need to make sure we have the "headless"
        // command arg set (various code specifically checks via
        // CommandLineArgs):
        desktop::Desktop::GetCommandLineArgs().setHeadless();

        Application::EnableHeadlessMode(true);

        // This is horrible crack. I really would want to go back to simply just call
        // InitVCL() here. The OfficeIPCThread thing is just horrible.

        // We could use InitVCL() here -- and used to before using soffice_main,
        // however that now deals with the initialisation for us (and it's not
        // possible to try to set up VCL twice.

        // Instead VCL init is done for us by soffice_main in a separate thread,
        // however we specifically can't proceed until this setup is complete
        // (or you get segfaults trying to use VCL and/or deadlocks due to other
        //  setup within soffice_main). Specifically the various Application::
        // functions depend on VCL being ready -- the deadlocks would happen
        // if you try to use loadDocument too early.

        // The OfficeIPCThread is specifically set to be read when all the other
        // init in Desktop::Main (run from soffice_main) is done. We can "enable"
        // the Thread from wherever (it's done again in Desktop::Main), and can
        // then use it to wait until we're definitely ready to continue.

        SAL_INFO("lok", "Enabling OfficeIPCThread");
        OfficeIPCThread::EnableOfficeIPCThread();
        SAL_INFO("lok", "Starting soffice_main");
        pLib->maThread = osl_createThread(lo_startmain, NULL);
        SAL_INFO("lok", "Waiting for OfficeIPCThread");
        OfficeIPCThread::WaitForReady();
        SAL_INFO("lok", "OfficeIPCThread ready -- continuing");

        // If the Thread has been disabled again that indicates that a
        // restart is required (or in any case we don't have a useable
        // process around).
        if (!OfficeIPCThread::IsEnabled())
        {
            fprintf(stderr, "LOK init failed -- restart required\n");
            return false;
        }

        ErrorHandler::RegisterDisplay(aBasicErrorFunc);

        SAL_INFO("lok", "LOK Initialized");
        bInitialized = true;
    }
    catch (css::uno::Exception& exception)
    {
        fprintf(stderr, "Bootstrapping exception '%s'\n",
                 OUStringToOString(exception.Message, RTL_TEXTENCODING_UTF8).getStr());
    }
    return bInitialized;
}

// Undo our clever trick of having SAL_DLLPUBLIC_EXPORT actually not
// meaning what is says in for the DISABLE_DYNLOADING case. See
// <sal/types.h>. Normally, when building just one big dylib (Android)
// or executable (iOS), most of our "public" symbols don't need to be
// visible outside that resulting dylib/executable. But
// libreofficekit_hook must be exported for dlsym() to find it,
// though, at least on iOS.

#if defined(__GNUC__) && defined(HAVE_GCC_VISIBILITY_FEATURE) && defined(DISABLE_DYNLOADING)
__attribute__ ((visibility("default")))
#else
SAL_DLLPUBLIC_EXPORT
#endif
LibreOfficeKit *libreofficekit_hook_2(const char* install_path, const char* user_profile_path)
{
    if (!gImpl)
    {
        SAL_INFO("lok", "Create libreoffice object");

        gImpl = new LibLibreOffice_Impl();
        if (!lo_initialize(gImpl, install_path, user_profile_path))
        {
            lo_destroy(gImpl);
        }
    }
    return static_cast<LibreOfficeKit*>(gImpl);
}

#if defined(__GNUC__) && defined(HAVE_GCC_VISIBILITY_FEATURE) && defined(DISABLE_DYNLOADING)
__attribute__ ((visibility("default")))
#else
SAL_DLLPUBLIC_EXPORT
#endif
LibreOfficeKit *libreofficekit_hook(const char* install_path)
{
    return libreofficekit_hook_2(install_path, NULL);
}

static void lo_destroy(LibreOfficeKit* pThis)
{
    LibLibreOffice_Impl* pLib = static_cast<LibLibreOffice_Impl*>(pThis);
    gImpl = NULL;

    SAL_INFO("lok", "LO Destroy");

    comphelper::LibreOfficeKit::setStatusIndicatorCallback(0, 0);

    Application::Quit();
    osl_joinWithThread(pLib->maThread);
    osl_destroyThread(pLib->maThread);

    delete pLib;
    bInitialized = false;
    SAL_INFO("lok", "LO Destroy Done");
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
