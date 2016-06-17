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
#include <comphelper/string.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/scopeguard.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/DispatchResultEvent.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XUniversalContentBroker.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>

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
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <svx/svxids.hrc>
#include <vcl/svapp.hxx>
#include <vcl/svpforlokit.hxx>
#include <tools/resmgr.hxx>
#include <tools/fract.hxx>
#include <svtools/ctrltool.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/ptrstyle.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/virdev.hxx>
#include <vcl/ITiledRenderable.hxx>
#include <unotools/configmgr.hxx>
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
#include <lokclipboard.hxx>

using namespace css;
using namespace vcl;
using namespace desktop;
using namespace utl;

static LibLibreOffice_Impl *gImpl = nullptr;
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
    { nullptr, nullptr }
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
    { "png",   "calc_png_Export" },
    { nullptr, nullptr }
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
    { nullptr, nullptr }
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
    { nullptr, nullptr }
};

/*
 * Map directly to css cursor styles to avoid further mapping in the client.
 * Gtk (via gdk_cursor_new_from_name) also supports the same css cursor styles.
 *
 * This was created partially with help of the mappings in gtkdata.cxx.
 * The list is incomplete as some cursor style simply aren't supported
 * by css, it might turn out to be worth mapping some of these missing cursors
 * to available cursors?
 */
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning( disable : 4592)
#endif
static const std::map <PointerStyle, OString> aPointerMap {
    { PointerStyle::Arrow, "default" },
    // PointerStyle::Null ?
    { PointerStyle::Wait, "wait" },
    { PointerStyle::Text, "text" },
    { PointerStyle::Help, "help" },
    { PointerStyle::Cross, "crosshair" },
    { PointerStyle::Fill, "fill" },
    { PointerStyle::Move, "move" },
    { PointerStyle::NSize, "n-resize" },
    { PointerStyle::SSize, "s-resize" },
    { PointerStyle::WSize, "w-resize" },
    { PointerStyle::ESize, "e-resize" },
    { PointerStyle::NWSize, "ne-resize" },
    { PointerStyle::NESize, "ne-resize" },
    { PointerStyle::SWSize, "sw-resize" },
    { PointerStyle::SESize, "se-resize" },
    // WindowNSize through WindowSESize
    { PointerStyle::HSplit, "col-resize" },
    { PointerStyle::VSplit, "row-resize" },
    { PointerStyle::HSizeBar, "col-resize" },
    { PointerStyle::VSizeBar, "row-resize" },
    { PointerStyle::Hand, "grab" },
    { PointerStyle::RefHand, "grabbing" },
    // Pen, Magnify, Fill, Rotate
    // HShear, VShear
    // Mirror, Crook, Crop, MovePoint, MoveBezierWeight
    // MoveData
    { PointerStyle::CopyData, "copy" },
    { PointerStyle::LinkData, "alias" },
    // MoveDataLink, CopyDataLink
    //MoveFile, CopyFile, LinkFile
    // MoveFileLink, CopyFileLink, MoveFiless, CopyFiles
    { PointerStyle::NotAllowed, "not-allowed" },
    // DrawLine through DrawCaption
    // Chart, Detective, PivotCol, PivotRow, PivotField, Chain, ChainNotAllowed
    // TimeEventMove, TimeEventSize
    // AutoScrollN through AutoScrollNSWE
    // Airbrush
    { PointerStyle::TextVertical, "vertical-text" }
    // Pivot Delete, TabSelectS through TabSelectSW
    // PaintBrush, HideWhiteSpace, ShowWhiteSpace
};
#ifdef _MSC_VER
#pragma warning(pop)
#endif

static OUString getUString(const char* pString)
{
    if (pString == nullptr)
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

static std::vector<beans::PropertyValue> jsonToPropertyValuesVector(const char* pJSON)
{
    std::vector<beans::PropertyValue> aArguments;
    if (pJSON && pJSON[0] != '\0')
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
                SAL_WARN("desktop.lib", "jsonToPropertyValuesVector: unhandled type '"<<rType<<"'");
            aArguments.push_back(aValue);
        }
    }
    return aArguments;
}

static boost::property_tree::ptree unoAnyToPropertyTree(const uno::Any& anyItem)
{
    boost::property_tree::ptree aTree;
    OUString aType = anyItem.getValueTypeName();
    aTree.put("type", aType.toUtf8().getStr());

    if (aType == "string")
        aTree.put("value", anyItem.get<OUString>().toUtf8().getStr());
    // TODO: Add more as required

    return aTree;
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
void        doc_paintPartTile(LibreOfficeKitDocument* pThis,
                              unsigned char* pBuffer,
                              const int nPart,
                              const int nCanvasWidth, const int nCanvasHeight,
                              const int nTilePosX, const int nTilePosY,
                              const int nTileWidth, const int nTileHeight);
static int doc_getTileMode(LibreOfficeKitDocument* pThis);
static void doc_getDocumentSize(LibreOfficeKitDocument* pThis,
                                long* pWidth,
                                long* pHeight);
static void doc_initializeForRendering(LibreOfficeKitDocument* pThis,
                                       const char* pArguments);

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
                               const char* pArguments,
                               bool bNotifyWhenFinished);
static void doc_setTextSelection (LibreOfficeKitDocument* pThis,
                                  int nType,
                                  int nX,
                                  int nY);
static char* doc_getTextSelection(LibreOfficeKitDocument* pThis,
                                  const char* pMimeType,
                                  char** pUsedMimeType);
static bool doc_paste(LibreOfficeKitDocument* pThis,
                      const char* pMimeType,
                      const char* pData,
                      size_t nSize);
static void doc_setGraphicSelection (LibreOfficeKitDocument* pThis,
                                  int nType,
                                  int nX,
                                  int nY);
static void doc_resetSelection (LibreOfficeKitDocument* pThis);
static char* doc_getCommandValues(LibreOfficeKitDocument* pThis, const char* pCommand);
static void doc_setClientZoom(LibreOfficeKitDocument* pThis,
                                    int nTilePixelWidth,
                                    int nTilePixelHeight,
                                    int nTileTwipWidth,
                                    int nTileTwipHeight);
static void doc_setClientVisibleArea(LibreOfficeKitDocument* pThis, int nX, int nY, int nWidth, int nHeight);
static int doc_createView(LibreOfficeKitDocument* pThis);
static void doc_destroyView(LibreOfficeKitDocument* pThis, int nId);
static void doc_setView(LibreOfficeKitDocument* pThis, int nId);
static int doc_getView(LibreOfficeKitDocument* pThis);
static int doc_getViews(LibreOfficeKitDocument* pThis);
static unsigned char* doc_renderFont(LibreOfficeKitDocument* pThis,
                          const char *pFontName,
                          int* pFontWidth,
                          int* pFontHeight);
static char* doc_getPartHash(LibreOfficeKitDocument* pThis, int nPart);

LibLODocument_Impl::LibLODocument_Impl(const uno::Reference <css::lang::XComponent> &xComponent)
    : mxComponent(xComponent)
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
        m_pDocumentClass->paintPartTile = doc_paintPartTile;
        m_pDocumentClass->getTileMode = doc_getTileMode;
        m_pDocumentClass->getDocumentSize = doc_getDocumentSize;
        m_pDocumentClass->initializeForRendering = doc_initializeForRendering;
        m_pDocumentClass->registerCallback = doc_registerCallback;
        m_pDocumentClass->postKeyEvent = doc_postKeyEvent;
        m_pDocumentClass->postMouseEvent = doc_postMouseEvent;
        m_pDocumentClass->postUnoCommand = doc_postUnoCommand;
        m_pDocumentClass->setTextSelection = doc_setTextSelection;
        m_pDocumentClass->getTextSelection = doc_getTextSelection;
        m_pDocumentClass->paste = doc_paste;
        m_pDocumentClass->setGraphicSelection = doc_setGraphicSelection;
        m_pDocumentClass->resetSelection = doc_resetSelection;
        m_pDocumentClass->getCommandValues = doc_getCommandValues;
        m_pDocumentClass->setClientZoom = doc_setClientZoom;
        m_pDocumentClass->setClientVisibleArea = doc_setClientVisibleArea;

        m_pDocumentClass->createView = doc_createView;
        m_pDocumentClass->destroyView = doc_destroyView;
        m_pDocumentClass->setView = doc_setView;
        m_pDocumentClass->getView = doc_getView;
        m_pDocumentClass->getViews = doc_getViews;

        m_pDocumentClass->renderFont = doc_renderFont;
        m_pDocumentClass->getPartHash = doc_getPartHash;

        gDocumentClass = m_pDocumentClass;
    }
    pClass = m_pDocumentClass.get();
}

LibLODocument_Impl::~LibLODocument_Impl()
{
    mxComponent->dispose();
}

CallbackFlushHandler::CallbackFlushHandler(LibreOfficeKitDocument* pDocument, LibreOfficeKitCallback pCallback, void* pData)
    : Idle( "lokit timer callback" ),
      m_pDocument(pDocument),
      m_pCallback(pCallback),
      m_pData(pData),
      m_bPartTilePainting(false)
{
    SetPriority(SchedulerPriority::POST_PAINT);

    // Add the states that are safe to skip duplicates on,
    // even when not consequent.
    m_states.emplace(LOK_CALLBACK_TEXT_SELECTION_START, "NIL");
    m_states.emplace(LOK_CALLBACK_TEXT_SELECTION_END, "NIL");
    m_states.emplace(LOK_CALLBACK_TEXT_SELECTION, "NIL");
    m_states.emplace(LOK_CALLBACK_GRAPHIC_SELECTION, "NIL");
    m_states.emplace(LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR, "NIL");
    m_states.emplace(LOK_CALLBACK_STATE_CHANGED, "NIL");
    m_states.emplace(LOK_CALLBACK_MOUSE_POINTER, "NIL");
    m_states.emplace(LOK_CALLBACK_CELL_CURSOR, "NIL");
    m_states.emplace(LOK_CALLBACK_CELL_FORMULA, "NIL");
    m_states.emplace(LOK_CALLBACK_CURSOR_VISIBLE, "NIL");
    m_states.emplace(LOK_CALLBACK_SET_PART, "NIL");

    Start();
}

CallbackFlushHandler::~CallbackFlushHandler()
{
    Stop();

    // We might have important notification (.uno:save?).
    flush();
}

void CallbackFlushHandler::Invoke()
{
    flush();
}

void CallbackFlushHandler::callback(const int type, const char* payload, void* data)
{
    CallbackFlushHandler* self = static_cast<CallbackFlushHandler*>(data);
    if (self)
    {
        self->queue(type, payload);
    }
}

void CallbackFlushHandler::queue(const int type, const char* data)
{
    const std::string payload(data ? data : "(nil)");
    if (m_bPartTilePainting)
    {
        // We drop notifications when this is set, except for important ones.
        // When we issue a complex command (such as .uno:InsertAnnotation)
        // there will be multiple notifications. On the first invalidation
        // we will start painting, but other events will get fired
        // while the complex command in question executes.
        // We don't want to supress everything here on the wrong assumption
        // that no new events are fired during painting.
        if (type != LOK_CALLBACK_STATE_CHANGED &&
            type != LOK_CALLBACK_INVALIDATE_TILES &&
            type != LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR &&
            type != LOK_CALLBACK_CURSOR_VISIBLE &&
            type != LOK_CALLBACK_TEXT_SELECTION)
        {
            //SAL_WARN("lokevt", "Skipping while painting [" + std::to_string(type) + "]: [" + payload + "].");
            return;
        }

        // In Writer we drop all notifications during painting.
        if (doc_getDocumentType(m_pDocument) == LOK_DOCTYPE_TEXT)
            return;
    }

    // Supress invalid payloads.
    if (type == LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR &&
        payload.find(", 0, 0, ") != std::string::npos)
    {
        // The cursor position is often the relative coordinates of the widget
        // issueing it, instead of the absolute one that we expect.
        // This is temporary however, and, once the control is created and initialized
        // correctly, it eventually emits the correct absolute coordinates.
        //SAL_WARN("lokevt", "Skipping invalid event [" + std::to_string(type) + "]: [" + payload + "].");
        return;
    }

    std::unique_lock<std::mutex> lock(m_mutex);

    const auto stateIt = m_states.find(type);
    if (stateIt != m_states.end())
    {
        // If the state didn't change, it's safe to ignore.
        if (stateIt->second == payload)
        {
            //SAL_WARN("lokevt", "Skipping duplicate [" + std::to_string(type) + "]: [" + payload + "].");
            return;
        }

        stateIt->second = payload;
    }

    if (type == LOK_CALLBACK_TEXT_SELECTION && payload.empty())
    {
        // Removing text selection invalidates the start and end as well.
        m_states[LOK_CALLBACK_TEXT_SELECTION_START] = "";
        m_states[LOK_CALLBACK_TEXT_SELECTION_END] = "";
    }

    m_queue.emplace_back(type, payload);

    // These are safe to use the latest state and ignore previous
    // ones (if any) since the last overrides previous ones.
    switch (type)
    {
        case LOK_CALLBACK_TEXT_SELECTION_START:
        case LOK_CALLBACK_TEXT_SELECTION_END:
        case LOK_CALLBACK_TEXT_SELECTION:
        case LOK_CALLBACK_GRAPHIC_SELECTION:
        case LOK_CALLBACK_MOUSE_POINTER:
        case LOK_CALLBACK_CELL_CURSOR:
        case LOK_CALLBACK_CELL_FORMULA:
        case LOK_CALLBACK_CURSOR_VISIBLE:
        case LOK_CALLBACK_SET_PART:
        case LOK_CALLBACK_STATUS_INDICATOR_SET_VALUE:
            removeAllButLast(type, false);
        break;

        // These come with rects, so drop earlier
        // only when the latter includes former ones.
        case LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
        case LOK_CALLBACK_INVALIDATE_TILES:
            if (payload.empty())
            {
                // Invalidating everything means previously
                // invalidated tiles can be dropped.
                removeAllButLast(type, false);
            }
            else
            {
                removeAllButLast(type, true);
            }

        break;
    }

    lock.unlock();
    if (!IsActive())
    {
        Start();
    }
}

void CallbackFlushHandler::setPartTilePainting(const bool bPartPainting)
{
    m_bPartTilePainting = bPartPainting;
}

bool CallbackFlushHandler::isPartTilePainting() const
{
    return m_bPartTilePainting;
}

void CallbackFlushHandler::flush()
{
    if (m_pCallback)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        for (auto& pair : m_queue)
        {
            m_pCallback(pair.first, pair.second.c_str(), m_pData);
        }

        m_queue.clear();
    }
}

void CallbackFlushHandler::removeAllButLast(const int type, const bool identical)
{
    int i = m_queue.size() - 1;
    std::string payload;
    for (; i >= 0; --i)
    {
        if (m_queue[i].first == type)
        {
            payload = m_queue[i].second;
            //SAL_WARN("lokevt", "Found [" + std::to_string(type) + "] at " + std::to_string(i) + ": [" + payload + "].");
            break;
        }
    }

    for (--i; i >= 0; --i)
    {
        if (m_queue[i].first == type &&
            (!identical || m_queue[i].second == payload))
        {
            //SAL_WARN("lokevt", "Removing [" + std::to_string(type) + "] at " + std::to_string(i) + ": " + m_queue[i].second + "].");
            m_queue.erase(m_queue.begin() + i);
        }
    }
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
static void                    lo_freeError     (char* pFree);
static LibreOfficeKitDocument* lo_documentLoadWithOptions  (LibreOfficeKit* pThis,
                                                           const char* pURL,
                                                           const char* pOptions);
static void                    lo_registerCallback (LibreOfficeKit* pThis,
                                                    LibreOfficeKitCallback pCallback,
                                                    void* pData);
static char* lo_getFilterTypes(LibreOfficeKit* pThis);
static void lo_setOptionalFeatures(LibreOfficeKit* pThis, uint64_t features);
static void                    lo_setDocumentPassword(LibreOfficeKit* pThis,
                                                       const char* pURL,
                                                       const char* pPassword);
static char*                   lo_getVersionInfo(LibreOfficeKit* pThis);

LibLibreOffice_Impl::LibLibreOffice_Impl()
    : m_pOfficeClass( gOfficeClass.lock() )
    , maThread(nullptr)
    , mpCallback(nullptr)
    , mpCallbackData(nullptr)
    , mOptionalFeatures(0)
{
    if(!m_pOfficeClass) {
        m_pOfficeClass.reset(new LibreOfficeKitClass);
        m_pOfficeClass->nSize = sizeof(LibreOfficeKitClass);

        m_pOfficeClass->destroy = lo_destroy;
        m_pOfficeClass->documentLoad = lo_documentLoad;
        m_pOfficeClass->getError = lo_getError;
        m_pOfficeClass->freeError = lo_freeError;
        m_pOfficeClass->documentLoadWithOptions = lo_documentLoadWithOptions;
        m_pOfficeClass->registerCallback = lo_registerCallback;
        m_pOfficeClass->getFilterTypes = lo_getFilterTypes;
        m_pOfficeClass->setOptionalFeatures = lo_setOptionalFeatures;
        m_pOfficeClass->setDocumentPassword = lo_setDocumentPassword;
        m_pOfficeClass->getVersionInfo = lo_getVersionInfo;

        gOfficeClass = m_pOfficeClass;
    }

    pClass = m_pOfficeClass.get();
}

LibLibreOffice_Impl::~LibLibreOffice_Impl()
{
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
    return lo_documentLoadWithOptions(pThis, pURL, nullptr);
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
        return nullptr;
    }

    pLib->maLastExceptionMsg.clear();

    if (!xContext.is())
    {
        pLib->maLastExceptionMsg = "ComponentContext is not available";
        SAL_INFO("lok", "ComponentContext is not available");
        return nullptr;
    }

    uno::Reference<frame::XDesktop2> xComponentLoader = frame::Desktop::create(xContext);

    if (!xComponentLoader.is())
    {
        pLib->maLastExceptionMsg = "ComponentLoader is not available";
        SAL_INFO("lok", "ComponentLoader is not available");
        return nullptr;
    }

    try
    {
        uno::Sequence<css::beans::PropertyValue> aFilterOptions(2);
        aFilterOptions[0] = css::beans::PropertyValue( OUString("FilterOptions"),
                                                       0,
                                                       uno::makeAny(OUString::createFromAscii(pOptions)),
                                                       beans::PropertyState_DIRECT_VALUE);

        rtl::Reference<LOKInteractionHandler> const pInteraction(
            new LOKInteractionHandler(::comphelper::getProcessComponentContext(), "load", pLib));
        auto const pair(pLib->mInteractionMap.insert(std::make_pair(aURL.toUtf8(), pInteraction)));
        comphelper::ScopeGuard const g([&] () {
                if (pair.second)
                {
                    pLib->mInteractionMap.erase(aURL.toUtf8());
                }
            });
        uno::Reference<task::XInteractionHandler2> const xInteraction(pInteraction.get());
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
                                            aURL, "_blank", 0,
                                            aFilterOptions);

        assert(!xComponent.is() || pair.second); // concurrent loading of same URL ought to fail

        if (!xComponent.is())
        {
            pLib->maLastExceptionMsg = "loadComponentFromURL returned an empty reference";
            SAL_INFO("lok", "Document can't be loaded - " << pLib->maLastExceptionMsg);
            return nullptr;
        }

        return new LibLODocument_Impl(xComponent);

    }
    catch (const uno::Exception& exception)
    {
        pLib->maLastExceptionMsg = exception.Message;
        SAL_INFO("lok", "Document can't be loaded - exception: " << exception.Message);
    }

    return nullptr;
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
            SAL_INFO("lok", "Can't save document - unsopported document type.");
            return false;
        }

        if (pFormat == nullptr)
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

        // 'TakeOwnership' == this is a 'real' SaveAs (that is, the document
        // gets a new name).  When this is not provided, the meaning of
        // saveAs() is more like save-a-copy, which allows saving to any
        // random format like PDF or PNG.
        // It is not a real filter option, so we have to filter it out.
        bool bTakeOwnership = false;
        int nIndex = -1;
        if (aFilterOptions == "TakeOwnership")
        {
            bTakeOwnership = true;
            aFilterOptions.clear();
        }
        else if ((nIndex = aFilterOptions.indexOf(",TakeOwnership")) >= 0 || (nIndex = aFilterOptions.indexOf("TakeOwnership,")) >= 0)
        {
            OUString aFiltered;
            if (nIndex > 0)
                aFiltered = aFilterOptions.copy(0, nIndex);
            if (nIndex + 14 < aFilterOptions.getLength())
                aFiltered = aFiltered + aFilterOptions.copy(nIndex + 14);

            bTakeOwnership = true;
            aFilterOptions = aFiltered;
        }

        MediaDescriptor aSaveMediaDescriptor;
        aSaveMediaDescriptor["Overwrite"] <<= true;
        aSaveMediaDescriptor["FilterName"] <<= aFilterName;
        aSaveMediaDescriptor[MediaDescriptor::PROP_FILTEROPTIONS()] <<= aFilterOptions;

        // add interaction handler too
        if (gImpl)
        {
            // gImpl does not have to exist when running from a unit test
            rtl::Reference<LOKInteractionHandler> const pInteraction(
                    new LOKInteractionHandler(::comphelper::getProcessComponentContext(), "saveas", gImpl, pDocument));
            uno::Reference<task::XInteractionHandler2> const xInteraction(pInteraction.get());

            aSaveMediaDescriptor[MediaDescriptor::PROP_INTERACTIONHANDLER()] <<= xInteraction;
        }

        uno::Reference<frame::XStorable> xStorable(pDocument->mxComponent, uno::UNO_QUERY_THROW);

        if (bTakeOwnership)
            xStorable->storeAsURL(aURL, aSaveMediaDescriptor.getAsConstPropertyValueList());
        else
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
        OUString(".uno:BackColor"),
        OUString(".uno:BackgroundColor"),
        OUString(".uno:Bold"),
        OUString(".uno:CenterPara"),
        OUString(".uno:CharBackColor"),
        OUString(".uno:CharBackgroundExt"),
        OUString(".uno:CharFontName"),
        OUString(".uno:Color"),
        OUString(".uno:DecrementIndent"),
        OUString(".uno:DefaultBullet"),
        OUString(".uno:DefaultNumbering"),
        OUString(".uno:FontColor"),
        OUString(".uno:FontHeight"),
        OUString(".uno:IncrementIndent"),
        OUString(".uno:Italic"),
        OUString(".uno:JustifyPara"),
        OUString(".uno:OutlineFont"),
        OUString(".uno:LeftPara"),
        OUString(".uno:RightPara"),
        OUString(".uno:Shadowed"),
        OUString(".uno:SubScript"),
        OUString(".uno:SuperScript"),
        OUString(".uno:Strikeout"),
        OUString(".uno:StyleApply"),
        OUString(".uno:Underline"),
        OUString(".uno:ModifiedStatus"),
        OUString(".uno:Undo"),
        OUString(".uno:Redo"),
        OUString(".uno:InsertPage"),
        OUString(".uno:DeletePage"),
        OUString(".uno:DuplicatePage"),
        OUString(".uno:Cut"),
        OUString(".uno:Copy"),
        OUString(".uno:Paste"),
        OUString(".uno:SelectAll"),
        OUString(".uno:InsertAnnotation"),
        OUString(".uno:InsertRowsBefore"),
        OUString(".uno:InsertRowsAfter"),
        OUString(".uno:InsertColumnsBefore"),
        OUString(".uno:InsertColumnsAfter"),
        OUString(".uno:DeleteRows"),
        OUString(".uno:DeleteColumns"),
        OUString(".uno:DeleteTable"),
        OUString(".uno:SelectTable"),
        OUString(".uno:EntireRow"),
        OUString(".uno:EntireColumn"),
        OUString(".uno:EntireCell"),
        OUString(".uno:MergeCells"),
        OUString(".uno:AssignLayout"),
        OUString(".uno:StatusDocPos"),
        OUString(".uno:RowColSelCount"),
        OUString(".uno:StatusPageStyle"),
        OUString(".uno:InsertMode"),
        OUString(".uno:StatusSelectionMode"),
        OUString(".uno:StateTableCell"),
        OUString(".uno:StatusBarFunc"),
        OUString(".uno:StatePageNumber"),
        OUString(".uno:StateWordCount"),
        OUString(".uno:SelectionMode"),
        OUString(".uno:PageStatus"),
        OUString(".uno:LayoutStatus"),
        OUString(".uno:Context"),
        OUString(".uno:WrapText"),
        OUString(".uno:ToggleMergeCells"),
        OUString(".uno:NumberFormatCurrency"),
        OUString(".uno:NumberFormatPercent"),
        OUString(".uno:NumberFormatDate"),
        OUString(".uno:SortAscending"),
        OUString(".uno:SortDescending")
    };

    util::URL aCommandURL;
    SfxViewShell* pViewShell = SfxViewShell::Current();
    SfxViewFrame* pViewFrame = pViewShell? pViewShell->GetViewFrame(): nullptr;

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

    for (const auto & sUnoCommand : sUnoCommands)
    {
        const SfxSlot* pSlot = nullptr;

        aCommandURL.Complete = sUnoCommand;
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
        else if (xDocument->supportsService("com.sun.star.text.TextDocument") || xDocument->supportsService("com.sun.star.text.WebDocument"))
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
        return nullptr;
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
        return nullptr;
    }

    OUString sName = pDoc->getPartName( nPart );
    OString aString = OUStringToOString(sName, RTL_TEXTENCODING_UTF8);
    char* pMemory = static_cast<char*>(malloc(aString.getLength() + 1));
    strcpy(pMemory, aString.getStr());
    return pMemory;

}

static char* doc_getPartHash(LibreOfficeKitDocument* pThis, int nPart)
{
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        gImpl->maLastExceptionMsg = "Document doesn't support tiled rendering";
        return nullptr;
    }

    OUString sHash = pDoc->getPartHash(nPart);
    OString aString = OUStringToOString(sHash, RTL_TEXTENCODING_UTF8);
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

void doc_paintTile(LibreOfficeKitDocument* pThis,
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

#if defined(IOS)
    SystemGraphicsData aData;
    aData.rCGContext = reinterpret_cast<CGContextRef>(pBuffer);
    // the Size argument is irrelevant, I hope
    ScopedVclPtrInstance<VirtualDevice> pDevice(&aData, Size(1, 1), DeviceFormat::DEFAULT);

    pDoc->paintTile(*pDevice.get(), nCanvasWidth, nCanvasHeight,
                    nTilePosX, nTilePosY, nTileWidth, nTileHeight);
#else
    ScopedVclPtrInstance< VirtualDevice > pDevice(nullptr, Size(1, 1), DeviceFormat::DEFAULT) ;

#if !defined(ANDROID)
    // Set background to transparent by default.
    pDevice->SetBackground(Wallpaper(Color(COL_TRANSPARENT)));
#endif

    boost::shared_array< sal_uInt8 > aBuffer( pBuffer, NoDelete< sal_uInt8 >() );

    pDevice->SetOutputSizePixelScaleOffsetAndBuffer(
                Size(nCanvasWidth, nCanvasHeight), Fraction(1.0), Point(),
                aBuffer);

    pDoc->paintTile(*pDevice.get(), nCanvasWidth, nCanvasHeight,
                    nTilePosX, nTilePosY, nTileWidth, nTileHeight);

    static bool bDebug = getenv("LOK_DEBUG") != nullptr;
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
#endif

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


void doc_paintPartTile(LibreOfficeKitDocument* pThis,
                       unsigned char* pBuffer,
                       const int nPart,
                       const int nCanvasWidth, const int nCanvasHeight,
                       const int nTilePosX, const int nTilePosY,
                       const int nTileWidth, const int nTileHeight)
{
    SAL_INFO( "lok.tiledrendering", "paintPartTile: painting @ " << nPart << " ["
               << nTileWidth << "x" << nTileHeight << "]@("
               << nTilePosX << ", " << nTilePosY << ") to ["
               << nCanvasWidth << "x" << nCanvasHeight << "]px" );

    // Disable callbacks while we are painting.
    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);
    std::size_t nView = SfxLokHelper::getView();
    pDocument->mpCallbackFlushHandlers[nView]->setPartTilePainting(true);
    try
    {
        // Text documents have a single coordinate system; don't change part.
        int nOrigPart = 0;
        const bool isText = (doc_getDocumentType(pThis) == LOK_DOCTYPE_TEXT);
        if (!isText)
        {
            nOrigPart = doc_getPart(pThis);
            if (nPart != nOrigPart)
            {
                doc_setPart(pThis, nPart);
            }
        }

        doc_paintTile(pThis, pBuffer, nCanvasWidth, nCanvasHeight, nTilePosX, nTilePosY, nTileWidth, nTileHeight);

        if (!isText && nPart != nOrigPart)
        {
            doc_setPart(pThis, nOrigPart);
        }
    }
    catch (const std::exception&)
    {
        // Nothing to do but restore the PartTilePainting flag.
    }

    pDocument->mpCallbackFlushHandlers[nView]->setPartTilePainting(false);
}

static int doc_getTileMode(LibreOfficeKitDocument* /*pThis*/)
{
    return LOK_TILEMODE_BGRA;
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

static void doc_initializeForRendering(LibreOfficeKitDocument* pThis,
                                       const char* pArguments)
{
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (pDoc)
    {
        doc_iniUnoCommands();

        pDoc->initializeForTiledRendering(
                comphelper::containerToSequence(jsonToPropertyValuesVector(pArguments)));
    }
}

static void doc_registerCallback(LibreOfficeKitDocument* pThis,
                                 LibreOfficeKitCallback pCallback,
                                 void* pData)
{
    SolarMutexGuard aGuard;
    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);

    std::size_t nView = SfxLokHelper::getView();
    pDocument->mpCallbackFlushHandlers[nView].reset(new CallbackFlushHandler(pThis, pCallback, pData));

    if (SfxViewShell* pViewShell = SfxViewFrame::Current()->GetViewShell())
        pViewShell->registerLibreOfficeKitViewCallback(CallbackFlushHandler::callback, pDocument->mpCallbackFlushHandlers[nView].get());
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

/** Class to react on finishing of a dispatched command.

    This will call a LOK_COMMAND_FINISHED callback when postUnoCommand was
    called with the parameter requesting the notification.

    @see LibreOfficeKitCallbackType::LOK_CALLBACK_UNO_COMMAND_RESULT.
*/
class DispatchResultListener : public cppu::WeakImplHelper<css::frame::XDispatchResultListener>
{
    OString maCommand;                 ///< Command for which this is the result.
    std::shared_ptr<CallbackFlushHandler> mpCallback; ///< Callback to call.

public:
    DispatchResultListener(const char* pCommand, std::shared_ptr<CallbackFlushHandler>& pCallback)
        : maCommand(pCommand)
        , mpCallback(pCallback)
    {
        assert(mpCallback);
    }

    virtual void SAL_CALL dispatchFinished(const css::frame::DispatchResultEvent& rEvent) throw(css::uno::RuntimeException, std::exception) override
    {
        boost::property_tree::ptree aTree;
        aTree.put("commandName", maCommand.getStr());

        if (rEvent.State != frame::DispatchResultState::DONTKNOW)
        {
            bool bSuccess = (rEvent.State == frame::DispatchResultState::SUCCESS);
            aTree.put("success", bSuccess);
        }

        // TODO UNO Any rEvent.Result -> JSON
        // aTree.put("result": "...");

        std::stringstream aStream;
        boost::property_tree::write_json(aStream, aTree);
        OString aPayload = aStream.str().c_str();
        mpCallback->queue(LOK_CALLBACK_UNO_COMMAND_RESULT, aPayload.getStr());
    }

    virtual void SAL_CALL disposing(const css::lang::EventObject&) throw (css::uno::RuntimeException, std::exception) override {}
};

static void doc_postUnoCommand(LibreOfficeKitDocument* pThis, const char* pCommand, const char* pArguments, bool bNotifyWhenFinished)
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    OUString aCommand(pCommand, strlen(pCommand), RTL_TEXTENCODING_UTF8);
    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);

    std::vector<beans::PropertyValue> aPropertyValuesVector(jsonToPropertyValuesVector(pArguments));
    std::size_t nView = comphelper::LibreOfficeKit::isViewCallback() ? SfxLokHelper::getView() : 0;

    // handle potential interaction
    if (gImpl && aCommand == ".uno:Save")
    {
        rtl::Reference<LOKInteractionHandler> const pInteraction(
            new LOKInteractionHandler(::comphelper::getProcessComponentContext(), "save", gImpl, pDocument));
        uno::Reference<task::XInteractionHandler2> const xInteraction(pInteraction.get());

        beans::PropertyValue aValue;
        aValue.Name = "InteractionHandler";
        aValue.Value <<= xInteraction;

        aPropertyValuesVector.push_back(aValue);

        // Check if DontSaveIfUnmodified is specified
        bool bDontSaveIfUnmodified = false;
        auto it = aPropertyValuesVector.begin();
        while (it != aPropertyValuesVector.end())
        {
            if (it->Name == "DontSaveIfUnmodified")
            {
                bDontSaveIfUnmodified = it->Value.get<bool>();

                // Also remove this param before handling to core
                it = aPropertyValuesVector.erase(it);
            }
            else
                it++;
        }

        // skip saving and tell the result via UNO_COMMAND_RESULT
        if (bDontSaveIfUnmodified && !pDocSh->IsModified())
        {
            boost::property_tree::ptree aTree;
            aTree.put("commandName", pCommand);
            aTree.put("success", false);

            // Add the reason for not saving
            const uno::Any aResultValue = uno::makeAny(OUString("unmodified"));
            aTree.add_child("result", unoAnyToPropertyTree(aResultValue));

            std::stringstream aStream;
            boost::property_tree::write_json(aStream, aTree);
            pDocument->mpCallbackFlushHandlers[nView]->queue(LOK_CALLBACK_UNO_COMMAND_RESULT, strdup(aStream.str().c_str()));
            return;
        }
    }

    bool bResult = false;

    std::size_t nView = SfxLokHelper::getView();
    if (bNotifyWhenFinished && pDocument->mpCallbackFlushHandlers[nView])
    {
        bResult = comphelper::dispatchCommand(aCommand, comphelper::containerToSequence(aPropertyValuesVector),
                new DispatchResultListener(pCommand, pDocument->mpCallbackFlushHandlers[nView]));
    }
    else
        bResult = comphelper::dispatchCommand(aCommand, comphelper::containerToSequence(aPropertyValuesVector));

    if (!bResult)
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

    Pointer aPointer = pDoc->getPointer();
    // We don't map all possible pointers hence we need a default
    OString aPointerString = "default";
    auto aIt = aPointerMap.find(aPointer.GetStyle());
    if (aIt != aPointerMap.end())
    {
        aPointerString = aIt->second;
    }

    LibLODocument_Impl* pLib = static_cast<LibLODocument_Impl*>(pThis);
    std::size_t nView = SfxLokHelper::getView();
    if (pLib->mpCallbackFlushHandlers[nView])
    {
        pLib->mpCallbackFlushHandlers[nView]->queue(LOK_CALLBACK_MOUSE_POINTER, aPointerString.getStr());
    }
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
        return nullptr;
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

static bool doc_paste(LibreOfficeKitDocument* pThis, const char* pMimeType, const char* pData, size_t nSize)
{
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        gImpl->maLastExceptionMsg = "Document doesn't support tiled rendering";
        return false;
    }

    uno::Reference<datatransfer::XTransferable> xTransferable(new LOKTransferable(pMimeType, pData, nSize));
    uno::Reference<datatransfer::clipboard::XClipboard> xClipboard(new LOKClipboard());
    xClipboard->setContents(xTransferable, uno::Reference<datatransfer::clipboard::XClipboardOwner>());
    pDoc->setClipboard(xClipboard);
    if (!pDoc->isMimeTypeSupported())
    {
        if (gImpl)
            gImpl->maLastExceptionMsg = "Document doesn't support this mime type";
        return false;
    }

    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"AnchorType", uno::makeAny(static_cast<sal_uInt16>(text::TextContentAnchorType_AS_CHARACTER))},
    }));
    if (!comphelper::dispatchCommand(".uno:Paste", aPropertyValues))
    {
        gImpl->maLastExceptionMsg = "Failed to dispatch the .uno: command";
        return false;
    }

    return true;
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
    const FontList* pList = pFonts ? pFonts->GetFontList() : nullptr;

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
            aValues.add_child(rInfo.GetFamilyName().toUtf8().getStr(), aChildren);
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

    static const std::vector<OUString> aWriterStyles =
    {
        "Text body",
        "Quotations",
        "Title",
        "Subtitle",
        "Heading 1",
        "Heading 2",
        "Heading 3"
    };

    // We need to keep a list of the default style names
    // in order to filter these out later when processing
    // the full list of styles.
    std::set<OUString> aDefaultStyleNames;

    boost::property_tree::ptree aValues;
    for (sal_Int32 nStyleFam = 0; nStyleFam < aStyleFamilies.getLength(); ++nStyleFam)
    {
        boost::property_tree::ptree aChildren;
        OUString sStyleFam = aStyleFamilies[nStyleFam];
        uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName(sStyleFam), uno::UNO_QUERY);

        // Writer provides a huge number of styles, we have a list of 7 "default" styles which
        // should be shown in the normal dropdown, which we should add to the start of the list
        // to simplify their selection.
        if (sStyleFam == "ParagraphStyles"
            && doc_getDocumentType(pThis) == LOK_DOCTYPE_TEXT)
        {
            for (const OUString& rStyle: aWriterStyles)
            {
                aDefaultStyleNames.insert( rStyle );

                boost::property_tree::ptree aChild;
                aChild.put("", rStyle.toUtf8());
                aChildren.push_back(std::make_pair("", aChild));
            }
        }

        uno::Sequence<OUString> aStyles = xStyleFamily->getElementNames();
        for (const OUString& rStyle: aStyles )
        {
            // Filter out the default styles - they are already at the top
            // of the list
            if (aDefaultStyleNames.find(rStyle) == aDefaultStyleNames.end() ||
                (sStyleFam != "ParagraphStyles" || doc_getDocumentType(pThis) != LOK_DOCTYPE_TEXT) )
            {
                boost::property_tree::ptree aChild;
                aChild.put("", rStyle.toUtf8());
                aChildren.push_back(std::make_pair("", aChild));
            }
        }
        aValues.add_child(sStyleFam.toUtf8().getStr(), aChildren);
    }

    {
        boost::property_tree::ptree aCommandList;

        {
            boost::property_tree::ptree aChild;

            OUString sClearFormat = SVX_RESSTR( RID_SVXSTR_CLEARFORM );

            boost::property_tree::ptree aName;
            aName.put("", sClearFormat.toUtf8());
            aChild.push_back(std::make_pair("text", aName));

            boost::property_tree::ptree aCommand;
            aCommand.put("", ".uno:ResetAttributes");
            aChild.push_back(std::make_pair("id", aCommand));

            aCommandList.push_back(std::make_pair("", aChild));
        }

        aValues.add_child("Commands", aCommandList);
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
    OString aCommand(pCommand);
    static const OString aViewRowColumnHeaders(".uno:ViewRowColumnHeaders");
    static const OString aCellCursor(".uno:CellCursor");

    if (!strcmp(pCommand, ".uno:CharFontName"))
    {
        return getFonts(pCommand);
    }
    else if (!strcmp(pCommand, ".uno:StyleApply"))
    {
        return getStyles(pThis, pCommand);
    }
    else if (aCommand.startsWith(aViewRowColumnHeaders))
    {
        ITiledRenderable* pDoc = getTiledRenderable(pThis);
        if (!pDoc)
        {
            gImpl->maLastExceptionMsg = "Document doesn't support tiled rendering";
            return nullptr;
        }

        Rectangle aRectangle;
        if (aCommand.getLength() > aViewRowColumnHeaders.getLength())
        {
            // Command has parameters.
            int nX = 0;
            int nY = 0;
            int nWidth = 0;
            int nHeight = 0;
            OString aArguments = aCommand.copy(aViewRowColumnHeaders.getLength() + 1);
            sal_Int32 nParamIndex = 0;
            do
            {
                OString aParamToken = aArguments.getToken(0, '&', nParamIndex);
                sal_Int32 nIndex = 0;
                OString aKey;
                OString aValue;
                do
                {
                    OString aToken = aParamToken.getToken(0, '=', nIndex);
                    if (!aKey.getLength())
                        aKey = aToken;
                    else
                        aValue = aToken;
                }
                while (nIndex >= 0);
                if (aKey == "x")
                    nX = aValue.toInt32();
                else if (aKey == "y")
                    nY = aValue.toInt32();
                else if (aKey == "width")
                    nWidth = aValue.toInt32();
                else if (aKey == "height")
                    nHeight = aValue.toInt32();
            }
            while (nParamIndex >= 0);
            aRectangle = Rectangle(nX, nY, nX + nWidth, nY + nHeight);
        }

        OUString aHeaders = pDoc->getRowColumnHeaders(aRectangle);
        OString aString = OUStringToOString(aHeaders, RTL_TEXTENCODING_UTF8);

        char* pMemory = static_cast<char*>(malloc(aString.getLength() + 1));
        strcpy(pMemory, aString.getStr());
        return pMemory;
    }
    else if (aCommand.startsWith(aCellCursor))
    {
        ITiledRenderable* pDoc = getTiledRenderable(pThis);
        if (!pDoc)
        {
            gImpl->maLastExceptionMsg = "Document doesn't support tiled rendering";
            return nullptr;
        }

        // Command has parameters.
        int nOutputWidth = 0;
        int nOutputHeight = 0;
        long nTileWidth = 0;
        long nTileHeight = 0;
        if (aCommand.getLength() > aCellCursor.getLength())
        {
            OString aArguments = aCommand.copy(aCellCursor.getLength() + 1);
            sal_Int32 nParamIndex = 0;
            do
            {
                OString aParamToken = aArguments.getToken(0, '&', nParamIndex);
                sal_Int32 nIndex = 0;
                OString aKey;
                OString aValue;
                do
                {
                    OString aToken = aParamToken.getToken(0, '=', nIndex);
                    if (!aKey.getLength())
                        aKey = aToken;
                    else
                        aValue = aToken;
                }
                while (nIndex >= 0);
                if (aKey == "outputWidth")
                    nOutputWidth = aValue.toInt32();
                else if (aKey == "outputHeight")
                    nOutputHeight = aValue.toInt32();
                else if (aKey == "tileWidth")
                    nTileWidth = aValue.toInt64();
                else if (aKey == "tileHeight")
                    nTileHeight = aValue.toInt64();
            }
            while (nParamIndex >= 0);
        }

        OString aString = pDoc->getCellCursor(nOutputWidth, nOutputHeight, nTileWidth, nTileHeight);

        char* pMemory = static_cast<char*>(malloc(aString.getLength() + 1));
        strcpy(pMemory, aString.getStr());
        return pMemory;
    }
    else
    {
        gImpl->maLastExceptionMsg = "Unknown command, no values returned";
        return nullptr;
    }
}

static void doc_setClientZoom(LibreOfficeKitDocument* pThis, int nTilePixelWidth, int nTilePixelHeight,
        int nTileTwipWidth, int nTileTwipHeight)
{
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        gImpl->maLastExceptionMsg = "Document doesn't support tiled rendering";
        return;
    }

    pDoc->setClientZoom(nTilePixelWidth, nTilePixelHeight, nTileTwipWidth, nTileTwipHeight);
}

static void doc_setClientVisibleArea(LibreOfficeKitDocument* pThis, int nX, int nY, int nWidth, int nHeight)
{
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        gImpl->maLastExceptionMsg = "Document doesn't support tiled rendering";
        return;
    }

    Rectangle aRectangle(Point(nX, nY), Size(nWidth, nHeight));
    pDoc->setClientVisibleArea(aRectangle);
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

unsigned char* doc_renderFont(LibreOfficeKitDocument* /*pThis*/,
                    const char* pFontName,
                    int* pFontWidth,
                    int* pFontHeight)
{
    OString aSearchedFontName(pFontName);
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SvxFontListItem* pFonts = static_cast<const SvxFontListItem*>(
        pDocSh->GetItem(SID_ATTR_CHAR_FONTLIST));
    const FontList* pList = pFonts ? pFonts->GetFontList() : nullptr;

    if ( pList )
    {
        sal_uInt16 nFontCount = pList->GetFontNameCount();
        for (sal_uInt16 i = 0; i < nFontCount; ++i)
        {
            const vcl::FontInfo& rInfo = pList->GetFontName(i);
            OUString aFontName = rInfo.GetFamilyName();
            if (!aSearchedFontName.equals(aFontName.toUtf8().getStr()))
                continue;

            auto aDevice(
                VclPtr<VirtualDevice>::Create(
                    nullptr, Size(1, 1), DeviceFormat::DEFAULT));
            ::Rectangle aRect;
            vcl::Font aFont(rInfo);
            aFont.SetSize(Size(0, 25));
            aDevice->SetFont(aFont);
            aDevice->GetTextBoundRect(aRect, aFontName);
            int nFontWidth = aRect.BottomRight().X() + 1;
            *pFontWidth = nFontWidth;
            int nFontHeight = aRect.BottomRight().Y() + 1;
            *pFontHeight = nFontHeight;

            unsigned char* pBuffer = static_cast<unsigned char*>(malloc(4 * nFontWidth * nFontHeight));
            memset(pBuffer, 0, nFontWidth * nFontHeight * 4);
            boost::shared_array<sal_uInt8> aBuffer(pBuffer, NoDelete< sal_uInt8 >());

            aDevice->SetBackground(Wallpaper(COL_TRANSPARENT));
            aDevice->SetOutputSizePixelScaleOffsetAndBuffer(
                        Size(nFontWidth, nFontHeight), Fraction(1.0), Point(),
                        aBuffer);
            aDevice->DrawText(Point(0,0), aFontName);

            return pBuffer;
        }
    }
    return nullptr;
}

static char* lo_getError (LibreOfficeKit *pThis)
{
    LibLibreOffice_Impl* pLib = static_cast<LibLibreOffice_Impl*>(pThis);
    OString aString = OUStringToOString(pLib->maLastExceptionMsg, RTL_TEXTENCODING_UTF8);
    char* pMemory = static_cast<char*>(malloc(aString.getLength() + 1));
    strcpy(pMemory, aString.getStr());
    return pMemory;
}

static void lo_freeError(char* pFree)
{
    free(pFree);
}

static char* lo_getFilterTypes(LibreOfficeKit* pThis)
{
    LibLibreOffice_Impl* pImpl = static_cast<LibLibreOffice_Impl*>(pThis);

    if (!xSFactory.is())
        xSFactory = comphelper::getProcessServiceFactory();

    if (!xSFactory.is())
    {
        pImpl->maLastExceptionMsg = "Service factory is not available";
        return nullptr;
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

static void lo_setOptionalFeatures(LibreOfficeKit* pThis, uint64_t const features)
{
    LibLibreOffice_Impl *const pLib = static_cast<LibLibreOffice_Impl*>(pThis);
    pLib->mOptionalFeatures = features;
}

static void lo_setDocumentPassword(LibreOfficeKit* pThis,
        const char* pURL, const char* pPassword)
{
    assert(pThis);
    assert(pURL);
    LibLibreOffice_Impl *const pLib = static_cast<LibLibreOffice_Impl*>(pThis);
    assert(pLib->mInteractionMap.find(OString(pURL)) != pLib->mInteractionMap.end());
    pLib->mInteractionMap.find(OString(pURL))->second->SetPassword(pPassword);
}

static char* lo_getVersionInfo(LibreOfficeKit* /*pThis*/)
{
    const OUString sVersionStrTemplate(
        "{ "
        "\"ProductName\": \"%PRODUCTNAME\", "
        "\"ProductVersion\": \"%PRODUCTVERSION\", "
        "\"ProductExtension\": \"%PRODUCTEXTENSION\", "
        "\"BuildId\": \"%BUILDID\" "
        "}"
    );
    const OString sVersionStr = OUStringToOString(ReplaceStringHookProc(sVersionStrTemplate), RTL_TEXTENCODING_UTF8);

    char* pVersion = static_cast<char*>(malloc(sVersionStr.getLength() + 1));
    strcpy(pVersion, sVersionStr.getStr());
    return pVersion;
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

    xSFactory.set(xFactory, uno::UNO_QUERY_THROW);
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

    if (GetpApp())
        Application::GetSolarMutex().tryToAcquire();

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
        pLib->mpCallback(LOK_CALLBACK_STATUS_INDICATOR_START, nullptr, pLib->mpCallbackData);
        break;
    case comphelper::LibreOfficeKit::statusIndicatorCallbackType::SetValue:
        pLib->mpCallback(LOK_CALLBACK_STATUS_INDICATOR_SET_VALUE, OUString::number(percent).toUtf8().getStr(), pLib->mpCallbackData);
        break;
    case comphelper::LibreOfficeKit::statusIndicatorCallbackType::Finish:
        pLib->mpCallback(LOK_CALLBACK_STATUS_INDICATOR_FINISH, nullptr, pLib->mpCallbackData);
        break;
    }
}

static int lo_initialize(LibreOfficeKit* pThis, const char* pAppPath, const char* pUserProfileUrl)
{
    enum {
        PRE_INIT,     // setup shared data in master process
        SECOND_INIT,  // complete init. after fork
        FULL_INIT     // do a standard complete init.
    } eStage;

    // Did we do a pre-initialize
    static bool bPreInited = false;

    // What stage are we at ?
    if (pThis == nullptr)
        eStage = PRE_INIT;
    else if (bPreInited)
        eStage = SECOND_INIT;
    else
        eStage = FULL_INIT;

    LibLibreOffice_Impl* pLib = static_cast<LibLibreOffice_Impl*>(pThis);

    if (bInitialized)
        return 1;

    if (eStage != SECOND_INIT)
        comphelper::LibreOfficeKit::setActive();

    if (eStage != PRE_INIT)
        comphelper::LibreOfficeKit::setStatusIndicatorCallback(lo_status_indicator_callback, pLib);

    if (eStage != SECOND_INIT && pUserProfileUrl)
    {
        OUString url(
            pUserProfileUrl, strlen(pUserProfileUrl), RTL_TEXTENCODING_UTF8);
        OUString path;
        if (url.startsWithIgnoreAsciiCase("vnd.sun.star.pathname:", &path))
        {
            OUString url2;
            osl::FileBase::RC e = osl::FileBase::getFileURLFromSystemPath(
                path, url2);
            if (e == osl::FileBase::E_None)
                url = url2;
            else
                SAL_WARN("lok", "resolving <" << url << "> failed with " << +e);
        }
        rtl::Bootstrap::set("UserInstallation", url);
    }

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
        if (eStage != SECOND_INIT)
        {
            SAL_INFO("lok", "Attempting to initalize UNO");

            if (!initialize_uno(aAppURL))
                return false;

            // Force headless -- this is only for bitmap rendering.
            rtl::Bootstrap::set("SAL_USE_VCLPLUGIN", "svp");

            // We specifically need to make sure we have the "headless"
            // command arg set (various code specifically checks via
            // CommandLineArgs):
            desktop::Desktop::GetCommandLineArgs().setHeadless();

            Application::EnableHeadlessMode(true);

            if (eStage == PRE_INIT)
            {
                InitVCL();
                // pre-load all component libraries.
                cppu::preInitBootstrap(xContext);
                // Release Solar Mutex, lo_startmain thread should acquire it.
                Application::ReleaseSolarMutex();
            }

            force_c_locale();
        }

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

        // The OfficeIPCThread is specifically set to be ready when all the other
        // init in Desktop::Main (run from soffice_main) is done. We can "enable"
        // the Thread from wherever (it's done again in Desktop::Main), and can
        // then use it to wait until we're definitely ready to continue.

        if (eStage != PRE_INIT)
        {
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
        }

        if (eStage != SECOND_INIT)
            ErrorHandler::RegisterDisplay(aBasicErrorFunc);

        SAL_INFO("lok", "LOK Initialized");
        if (eStage == PRE_INIT)
            bPreInited = true;
        else
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
    return libreofficekit_hook_2(install_path, nullptr);
}

SAL_JNI_EXPORT
int lok_preinit(const char* install_path, const char* user_profile_path)
{
    return lo_initialize(nullptr, install_path, user_profile_path);
}

static void lo_destroy(LibreOfficeKit* pThis)
{
    bool bSuccess = false;
    LibLibreOffice_Impl* pLib = static_cast<LibLibreOffice_Impl*>(pThis);
    gImpl = nullptr;

    SAL_INFO("lok", "LO Destroy");

    comphelper::LibreOfficeKit::setStatusIndicatorCallback(nullptr, nullptr);
    uno::Reference <frame::XDesktop2> xDesktop = frame::Desktop::create ( ::comphelper::getProcessComponentContext() );
    bSuccess = xDesktop.is() && xDesktop->terminate();

    if (!bSuccess)
    {
        bSuccess = GetpApp() && GetpApp()->QueryExit();
    }

    if (!bSuccess)
    {
        Application::Quit();
    }

    osl_joinWithThread(pLib->maThread);
    osl_destroyThread(pLib->maThread);

    delete pLib;
    bInitialized = false;
    SAL_INFO("lok", "LO Destroy Done");
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
