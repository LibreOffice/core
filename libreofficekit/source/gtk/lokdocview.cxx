/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
#include <math.h>
#include <string.h>
#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <mutex>
#include <boost/property_tree/json_parser.hpp>

#include <com/sun/star/awt/Key.hpp>
#include <LibreOfficeKit/LibreOfficeKit.h>
#include <LibreOfficeKit/LibreOfficeKitInit.h>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <LibreOfficeKit/LibreOfficeKitGtk.h>
#include <rsc/rsc-vcl-shared-types.hxx>
#include <vcl/event.hxx>

#include "tilebuffer.hxx"

#if !GLIB_CHECK_VERSION(2,32,0)
#define G_SOURCE_REMOVE FALSE
#define G_SOURCE_CONTINUE TRUE
#endif
#if !GLIB_CHECK_VERSION(2,40,0)
#define g_info(...) g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, __VA_ARGS__)
#endif

// Cursor bitmaps from the installation set.
#define CURSOR_HANDLE_DIR "/../share/libreofficekit/"
// Number of handles around a graphic selection.
#define GRAPHIC_HANDLE_COUNT 8
// Maximum Zoom allowed
#define MAX_ZOOM 5.0f
// Minimum Zoom allowed
#define MIN_ZOOM 0.25f

/// This is expected to be locked during setView(), doSomethingElse() LOK calls.
std::mutex g_aLOKMutex;

/// Same as a GdkRectangle, but also tracks in which part the rectangle is.
struct ViewRectangle
{
    int m_nPart;
    GdkRectangle m_aRectangle;

    ViewRectangle(int nPart = 0, const GdkRectangle& rRectangle = GdkRectangle())
        : m_nPart(nPart),
        m_aRectangle(rRectangle)
    {
    }
};

/// Same as a list of GdkRectangles, but also tracks in which part the rectangle is.
struct ViewRectangles
{
    int m_nPart;
    std::vector<GdkRectangle> m_aRectangles;

    ViewRectangles(int nPart = 0, const std::vector<GdkRectangle>& rRectangles = std::vector<GdkRectangle>())
        : m_nPart(nPart),
        m_aRectangles(rRectangles)
    {
    }
};

/// Private struct used by this GObject type
struct LOKDocViewPrivateImpl
{
    const gchar* m_aLOPath;
    const gchar* m_pUserProfileURL;
    const gchar* m_aDocPath;
    std::string m_aRenderingArguments;
    gdouble m_nLoadProgress;
    gboolean m_bIsLoading;
    gboolean m_bCanZoomIn;
    gboolean m_bCanZoomOut;
    LibreOfficeKit* m_pOffice;
    LibreOfficeKitDocument* m_pDocument;

    std::unique_ptr<TileBuffer> m_pTileBuffer;
    GThreadPool* lokThreadPool;

    gfloat m_fZoom;
    glong m_nDocumentWidthTwips;
    glong m_nDocumentHeightTwips;
    /// View or edit mode.
    gboolean m_bEdit;
    /// LOK Features
    guint64 m_nLOKFeatures;
    /// Number of parts in currently loaded document
    gint m_nParts;
    /// Position and size of the visible cursor.
    GdkRectangle m_aVisibleCursor;
    /// Position and size of the view cursors. The current view can only see
    /// them, can't modify them. Key is the view id.
    std::map<int, ViewRectangle> m_aViewCursors;
    /// Cursor overlay is visible or hidden (for blinking).
    gboolean m_bCursorOverlayVisible;
    /// Cursor is visible or hidden (e.g. for graphic selection).
    gboolean m_bCursorVisible;
    /// Visibility of view selections. The current view can only see / them,
    /// can't modify them. Key is the view id.
    std::map<int, bool> m_aViewCursorVisibilities;
    /// Time of the last button press.
    guint32 m_nLastButtonPressTime;
    /// Time of the last button release.
    guint32 m_nLastButtonReleaseTime;
    /// Last pressed button (left, right, middle)
    guint32 m_nLastButtonPressed;
    /// Key modifier (ctrl, atl, shift)
    guint32 m_nKeyModifier;
    /// Rectangles of the current text selection.
    std::vector<GdkRectangle> m_aTextSelectionRectangles;
    /// Rectangles of view selections. The current view can only see
    /// them, can't modify them. Key is the view id.
    std::map<int, ViewRectangles> m_aTextViewSelectionRectangles;
    /// Position and size of the selection start (as if there would be a cursor caret there).
    GdkRectangle m_aTextSelectionStart;
    /// Position and size of the selection end.
    GdkRectangle m_aTextSelectionEnd;
    GdkRectangle m_aGraphicSelection;
    /// Position and size of the graphic view selections. The current view can only
    /// see them, can't modify them. Key is the view id.
    std::map<int, ViewRectangle> m_aGraphicViewSelections;
    GdkRectangle m_aCellCursor;
    /// Position and size of the cell view cursors. The current view can only
    /// see them, can't modify them. Key is the view id.
    std::map<int, ViewRectangle> m_aCellViewCursors;
    gboolean m_bInDragGraphicSelection;

    /// @name Start/middle/end handle.
    ///@{
    /// Bitmap of the text selection start handle.
    cairo_surface_t* m_pHandleStart;
    /// Rectangle of the text selection start handle, to know if the user clicked on it or not
    GdkRectangle m_aHandleStartRect;
    /// If we are in the middle of a drag of the text selection end handle.
    gboolean m_bInDragStartHandle;
    /// Bitmap of the text selection middle handle.
    cairo_surface_t* m_pHandleMiddle;
    /// Rectangle of the text selection middle handle, to know if the user clicked on it or not
    GdkRectangle m_aHandleMiddleRect;
    /// If we are in the middle of a drag of the text selection middle handle.
    gboolean m_bInDragMiddleHandle;
    /// Bitmap of the text selection end handle.
    cairo_surface_t* m_pHandleEnd;
    /// Rectangle of the text selection end handle, to know if the user clicked on it or not
    GdkRectangle m_aHandleEndRect;
    /// If we are in the middle of a drag of the text selection end handle.
    gboolean m_bInDragEndHandle;
    ///@}

    /// @name Graphic handles.
    ///@{
    /// Rectangle of a graphic selection handle, to know if the user clicked on it or not.
    GdkRectangle m_aGraphicHandleRects[8];
    /// If we are in the middle of a drag of a graphic selection handle.
    gboolean m_bInDragGraphicHandles[8];
    ///@}

    /// View ID, returned by createView() or 0 by default.
    int m_nViewId;

    /// Cached part ID, returned by getPart().
    int m_nPartId;

    /// Cached document type, returned by getDocumentType().
    LibreOfficeKitDocumentType m_eDocumentType;

    /**
     * Contains a freshly set zoom level: logic size of a tile.
     * It gets reset back to 0 when LOK was informed about this zoom change.
    */
    int m_nTileSizeTwips;

    GdkRectangle m_aVisibleArea;
    bool m_bVisibleAreaSet;

    /// Event source ID for handleTimeout() of this widget.
    guint m_nTimeoutId;

    /// Rectangles of view locks. The current view can only see
    /// them, can't modify them. Key is the view id.
    std::map<int, ViewRectangle> m_aViewLockRectangles;

    LOKDocViewPrivateImpl()
        : m_aLOPath(nullptr),
        m_pUserProfileURL(nullptr),
        m_aDocPath(nullptr),
        m_nLoadProgress(0),
        m_bIsLoading(false),
        m_bCanZoomIn(true),
        m_bCanZoomOut(true),
        m_pOffice(nullptr),
        m_pDocument(nullptr),
        lokThreadPool(nullptr),
        m_fZoom(0),
        m_nDocumentWidthTwips(0),
        m_nDocumentHeightTwips(0),
        m_bEdit(FALSE),
        m_nLOKFeatures(0),
        m_nParts(0),
        m_aVisibleCursor({0, 0, 0, 0}),
        m_bCursorOverlayVisible(false),
        m_bCursorVisible(true),
        m_nLastButtonPressTime(0),
        m_nLastButtonReleaseTime(0),
        m_nLastButtonPressed(0),
        m_nKeyModifier(0),
        m_aTextSelectionStart({0, 0, 0, 0}),
        m_aTextSelectionEnd({0, 0, 0, 0}),
        m_aGraphicSelection({0, 0, 0, 0}),
        m_aCellCursor({0, 0, 0, 0}),
        m_bInDragGraphicSelection(false),
        m_pHandleStart(nullptr),
        m_aHandleStartRect({0, 0, 0, 0}),
        m_bInDragStartHandle(0),
        m_pHandleMiddle(nullptr),
        m_aHandleMiddleRect({0, 0, 0, 0}),
        m_bInDragMiddleHandle(false),
        m_pHandleEnd(nullptr),
        m_aHandleEndRect({0, 0, 0, 0}),
        m_bInDragEndHandle(false),
        m_nViewId(0),
        m_nPartId(0),
        m_nTileSizeTwips(0),
        m_aVisibleArea({0, 0, 0, 0}),
        m_bVisibleAreaSet(false),
        m_nTimeoutId(0)
    {
        memset(&m_aGraphicHandleRects, 0, sizeof(m_aGraphicHandleRects));
        memset(&m_bInDragGraphicHandles, 0, sizeof(m_bInDragGraphicHandles));
    }

    ~LOKDocViewPrivateImpl()
    {
        g_source_remove(m_nTimeoutId);
    }
};

/// Wrapper around LOKDocViewPrivateImpl, managed by malloc/memset/free.
struct _LOKDocViewPrivate
{
    LOKDocViewPrivateImpl* m_pImpl;

    LOKDocViewPrivateImpl* operator->()
    {
        return m_pImpl;
    }
};

enum
{
    LOAD_CHANGED,
    EDIT_CHANGED,
    COMMAND_CHANGED,
    SEARCH_NOT_FOUND,
    PART_CHANGED,
    SIZE_CHANGED,
    HYPERLINK_CLICKED,
    CURSOR_CHANGED,
    SEARCH_RESULT_COUNT,
    COMMAND_RESULT,
    FORMULA_CHANGED,
    TEXT_SELECTION,
    PASSWORD_REQUIRED,

    LAST_SIGNAL
};

enum
{
    PROP_0,

    PROP_LO_PATH,
    PROP_LO_POINTER,
    PROP_USER_PROFILE_URL,
    PROP_DOC_PATH,
    PROP_DOC_POINTER,
    PROP_EDITABLE,
    PROP_LOAD_PROGRESS,
    PROP_ZOOM,
    PROP_IS_LOADING,
    PROP_DOC_WIDTH,
    PROP_DOC_HEIGHT,
    PROP_CAN_ZOOM_IN,
    PROP_CAN_ZOOM_OUT,
    PROP_DOC_PASSWORD,
    PROP_DOC_PASSWORD_TO_MODIFY,

    PROP_LAST
};

static guint doc_view_signals[LAST_SIGNAL] = { 0 };
static GParamSpec *properties[PROP_LAST] = { nullptr };

static void lok_doc_view_initable_iface_init (GInitableIface *iface);
static void callbackWorker (int nType, const char* pPayload, void* pData);

SAL_DLLPUBLIC_EXPORT GType lok_doc_view_get_type();
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
G_DEFINE_TYPE_WITH_CODE (LOKDocView, lok_doc_view, GTK_TYPE_DRAWING_AREA,
                         G_ADD_PRIVATE (LOKDocView)
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE, lok_doc_view_initable_iface_init));
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

static LOKDocViewPrivate& getPrivate(LOKDocView* pDocView)
{
    LOKDocViewPrivate* priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private(pDocView));
    return *priv;
}

/// Helper struct used to pass the data from soffice thread -> main thread.
struct CallbackData
{
    int m_nType;
    std::string m_aPayload;
    LOKDocView* m_pDocView;

    CallbackData(int nType, const std::string& rPayload, LOKDocView* pDocView)
        : m_nType(nType),
          m_aPayload(rPayload),
          m_pDocView(pDocView) {}
};

static void
payloadToSize(const char* pPayload, long& rWidth, long& rHeight)
{
    rWidth = rHeight = 0;
    gchar** ppCoordinates = g_strsplit(pPayload, ", ", 2);
    gchar** ppCoordinate = ppCoordinates;
    if (!*ppCoordinate)
        return;
    rWidth = atoi(*ppCoordinate);
    ++ppCoordinate;
    if (!*ppCoordinate)
        return;
    rHeight = atoi(*ppCoordinate);
    g_strfreev(ppCoordinates);
}

/// Returns the string representation of a LibreOfficeKitCallbackType enumeration element.
static const char*
callbackTypeToString (int nType)
{
    switch (nType)
    {
    case LOK_CALLBACK_INVALIDATE_TILES:
        return "LOK_CALLBACK_INVALIDATE_TILES";
    case LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
        return "LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR";
    case LOK_CALLBACK_TEXT_SELECTION:
        return "LOK_CALLBACK_TEXT_SELECTION";
    case LOK_CALLBACK_TEXT_SELECTION_START:
        return "LOK_CALLBACK_TEXT_SELECTION_START";
    case LOK_CALLBACK_TEXT_SELECTION_END:
        return "LOK_CALLBACK_TEXT_SELECTION_END";
    case LOK_CALLBACK_CURSOR_VISIBLE:
        return "LOK_CALLBACK_CURSOR_VISIBLE";
    case LOK_CALLBACK_VIEW_CURSOR_VISIBLE:
        return "LOK_CALLBACK_VIEW_CURSOR_VISIBLE";
    case LOK_CALLBACK_GRAPHIC_SELECTION:
        return "LOK_CALLBACK_GRAPHIC_SELECTION";
    case LOK_CALLBACK_GRAPHIC_VIEW_SELECTION:
        return "LOK_CALLBACK_GRAPHIC_VIEW_SELECTION";
    case LOK_CALLBACK_CELL_CURSOR:
        return "LOK_CALLBACK_CELL_CURSOR";
    case LOK_CALLBACK_HYPERLINK_CLICKED:
        return "LOK_CALLBACK_HYPERLINK_CLICKED";
    case LOK_CALLBACK_MOUSE_POINTER:
        return "LOK_CALLBACK_MOUSE_POINTER";
    case LOK_CALLBACK_STATE_CHANGED:
        return "LOK_CALLBACK_STATE_CHANGED";
    case LOK_CALLBACK_STATUS_INDICATOR_START:
        return "LOK_CALLBACK_STATUS_INDICATOR_START";
    case LOK_CALLBACK_STATUS_INDICATOR_SET_VALUE:
        return "LOK_CALLBACK_STATUS_INDICATOR_SET_VALUE";
    case LOK_CALLBACK_STATUS_INDICATOR_FINISH:
        return "LOK_CALLBACK_STATUS_INDICATOR_FINISH";
    case LOK_CALLBACK_SEARCH_NOT_FOUND:
        return "LOK_CALLBACK_SEARCH_NOT_FOUND";
    case LOK_CALLBACK_DOCUMENT_SIZE_CHANGED:
        return "LOK_CALLBACK_DOCUMENT_SIZE_CHANGED";
    case LOK_CALLBACK_SET_PART:
        return "LOK_CALLBACK_SET_PART";
    case LOK_CALLBACK_SEARCH_RESULT_SELECTION:
        return "LOK_CALLBACK_SEARCH_RESULT_SELECTION";
    case LOK_CALLBACK_DOCUMENT_PASSWORD:
        return "LOK_CALLBACK_DOCUMENT_PASSWORD";
    case LOK_CALLBACK_DOCUMENT_PASSWORD_TO_MODIFY:
        return "LOK_CALLBACK_DOCUMENT_PASSWORD_TO_MODIFY";
    case LOK_CALLBACK_CONTEXT_MENU:
        return "LOK_CALLBACK_CONTEXT_MENU";
    case LOK_CALLBACK_INVALIDATE_VIEW_CURSOR:
        return "LOK_CALLBACK_INVALIDATE_VIEW_CURSOR";
    case LOK_CALLBACK_TEXT_VIEW_SELECTION:
        return "LOK_CALLBACK_TEXT_VIEW_SELECTION";
    case LOK_CALLBACK_CELL_VIEW_CURSOR:
        return "LOK_CALLBACK_CELL_VIEW_CURSOR";
    case LOK_CALLBACK_CELL_FORMULA:
        return "LOK_CALLBACK_CELL_FORMULA";
    case LOK_CALLBACK_UNO_COMMAND_RESULT:
        return "LOK_CALLBACK_UNO_COMMAND_RESULT";
    case LOK_CALLBACK_ERROR:
        return "LOK_CALLBACK_ERROR";
    case LOK_CALLBACK_VIEW_LOCK:
        return "LOK_CALLBACK_VIEW_LOCK";
    }
    g_assert(false);
    return nullptr;
}

static void
LOKPostCommand (LOKDocView* pDocView,
                const gchar* pCommand,
                const gchar* pArguments,
                gboolean bNotifyWhenFinished)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    GTask* task = g_task_new(pDocView, nullptr, nullptr, nullptr);
    LOEvent* pLOEvent = new LOEvent(LOK_POST_COMMAND);
    GError* error = nullptr;
    pLOEvent->m_pCommand = pCommand;
    pLOEvent->m_pArguments  = g_strdup(pArguments);
    pLOEvent->m_bNotifyWhenFinished = bNotifyWhenFinished;

    g_task_set_task_data(task, pLOEvent, LOEvent::destroy);
    g_thread_pool_push(priv->lokThreadPool, g_object_ref(task), &error);
    if (error != nullptr)
    {
        g_warning("Unable to call LOK_POST_COMMAND: %s", error->message);
        g_clear_error(&error);
    }
    g_object_unref(task);
}

static void
doSearch(LOKDocView* pDocView, const char* pText, bool bBackwards, bool highlightAll)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    if (!priv->m_pDocument)
        return;

    boost::property_tree::ptree aTree;
    GtkWidget* drawingWidget = GTK_WIDGET(pDocView);
    GdkWindow* drawingWindow = gtk_widget_get_window(drawingWidget);
    if (!drawingWindow)
        return;
    std::shared_ptr<cairo_region_t> cairoVisRegion( gdk_window_get_visible_region(drawingWindow),
                                                    cairo_region_destroy);
    cairo_rectangle_int_t cairoVisRect;
    cairo_region_get_rectangle(cairoVisRegion.get(), 0, &cairoVisRect);
    int x = pixelToTwip (cairoVisRect.x, priv->m_fZoom);
    int y = pixelToTwip (cairoVisRect.y, priv->m_fZoom);

    aTree.put(boost::property_tree::ptree::path_type("SearchItem.SearchString/type", '/'), "string");
    aTree.put(boost::property_tree::ptree::path_type("SearchItem.SearchString/value", '/'), pText);
    aTree.put(boost::property_tree::ptree::path_type("SearchItem.Backward/type", '/'), "boolean");
    aTree.put(boost::property_tree::ptree::path_type("SearchItem.Backward/value", '/'), bBackwards);
    if (highlightAll)
    {
        aTree.put(boost::property_tree::ptree::path_type("SearchItem.Command/type", '/'), "unsigned short");
        // SvxSearchCmd::FIND_ALL
        aTree.put(boost::property_tree::ptree::path_type("SearchItem.Command/value", '/'), "1");
    }

    aTree.put(boost::property_tree::ptree::path_type("SearchItem.SearchStartPointX/type", '/'), "long");
    aTree.put(boost::property_tree::ptree::path_type("SearchItem.SearchStartPointX/value", '/'), x);
    aTree.put(boost::property_tree::ptree::path_type("SearchItem.SearchStartPointY/type", '/'), "long");
    aTree.put(boost::property_tree::ptree::path_type("SearchItem.SearchStartPointY/value", '/'), y);

    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);

    LOKPostCommand (pDocView, ".uno:ExecuteSearch", aStream.str().c_str(), false);
}

static bool
isEmptyRectangle(const GdkRectangle& rRectangle)
{
    return rRectangle.x == 0 && rRectangle.y == 0 && rRectangle.width == 0 && rRectangle.height == 0;
}

/// if handled, returns TRUE else FALSE
static bool
handleTextSelectionOnButtonPress(GdkRectangle& aClick, LOKDocView* pDocView) {
    LOKDocViewPrivate& priv = getPrivate(pDocView);

    if (gdk_rectangle_intersect(&aClick, &priv->m_aHandleStartRect, nullptr))
    {
        g_info("LOKDocView_Impl::signalButton: start of drag start handle");
        priv->m_bInDragStartHandle = true;
        return TRUE;
    }
    else if (gdk_rectangle_intersect(&aClick, &priv->m_aHandleMiddleRect, nullptr))
    {
        g_info("LOKDocView_Impl::signalButton: start of drag middle handle");
        priv->m_bInDragMiddleHandle = true;
        return TRUE;
    }
    else if (gdk_rectangle_intersect(&aClick, &priv->m_aHandleEndRect, nullptr))
    {
        g_info("LOKDocView_Impl::signalButton: start of drag end handle");
        priv->m_bInDragEndHandle = true;
        return TRUE;
    }

    return FALSE;
}

/// if handled, returns TRUE else FALSE
static bool
handleGraphicSelectionOnButtonPress(GdkRectangle& aClick, LOKDocView* pDocView) {
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    GError* error = nullptr;

    for (int i = 0; i < GRAPHIC_HANDLE_COUNT; ++i)
    {
        if (gdk_rectangle_intersect(&aClick, &priv->m_aGraphicHandleRects[i], nullptr))
        {
            g_info("LOKDocView_Impl::signalButton: start of drag graphic handle #%d", i);
            priv->m_bInDragGraphicHandles[i] = true;

            GTask* task = g_task_new(pDocView, nullptr, nullptr, nullptr);
            LOEvent* pLOEvent = new LOEvent(LOK_SET_GRAPHIC_SELECTION);
            pLOEvent->m_nSetGraphicSelectionType = LOK_SETGRAPHICSELECTION_START;
            pLOEvent->m_nSetGraphicSelectionX = pixelToTwip(priv->m_aGraphicHandleRects[i].x + priv->m_aGraphicHandleRects[i].width / 2, priv->m_fZoom);
            pLOEvent->m_nSetGraphicSelectionY = pixelToTwip(priv->m_aGraphicHandleRects[i].y + priv->m_aGraphicHandleRects[i].height / 2, priv->m_fZoom);
            g_task_set_task_data(task, pLOEvent, LOEvent::destroy);

            g_thread_pool_push(priv->lokThreadPool, g_object_ref(task), &error);
            if (error != nullptr)
            {
                g_warning("Unable to call LOK_SET_GRAPHIC_SELECTION: %s", error->message);
                g_clear_error(&error);
            }
            g_object_unref(task);

            return TRUE;
        }
    }

    return FALSE;
}

/// if handled, returns TRUE else FALSE
static bool
handleTextSelectionOnButtonRelease(LOKDocView* pDocView) {
    LOKDocViewPrivate& priv = getPrivate(pDocView);

    if (priv->m_bInDragStartHandle)
    {
        g_info("LOKDocView_Impl::signalButton: end of drag start handle");
        priv->m_bInDragStartHandle = false;
        return TRUE;
    }
    else if (priv->m_bInDragMiddleHandle)
    {
        g_info("LOKDocView_Impl::signalButton: end of drag middle handle");
        priv->m_bInDragMiddleHandle = false;
        return TRUE;
    }
    else if (priv->m_bInDragEndHandle)
    {
        g_info("LOKDocView_Impl::signalButton: end of drag end handle");
        priv->m_bInDragEndHandle = false;
        return TRUE;
    }

    return FALSE;
}

/// if handled, returns TRUE else FALSE
static bool
handleGraphicSelectionOnButtonRelease(LOKDocView* pDocView, GdkEventButton* pEvent) {
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    GError* error = nullptr;

    for (int i = 0; i < GRAPHIC_HANDLE_COUNT; ++i)
    {
        if (priv->m_bInDragGraphicHandles[i])
        {
            g_info("LOKDocView_Impl::signalButton: end of drag graphic handle #%d", i);
            priv->m_bInDragGraphicHandles[i] = false;

            GTask* task = g_task_new(pDocView, nullptr, nullptr, nullptr);
            LOEvent* pLOEvent = new LOEvent(LOK_SET_GRAPHIC_SELECTION);
            pLOEvent->m_nSetGraphicSelectionType = LOK_SETGRAPHICSELECTION_END;
            pLOEvent->m_nSetGraphicSelectionX = pixelToTwip(pEvent->x, priv->m_fZoom);
            pLOEvent->m_nSetGraphicSelectionY = pixelToTwip(pEvent->y, priv->m_fZoom);
            g_task_set_task_data(task, pLOEvent, LOEvent::destroy);

            g_thread_pool_push(priv->lokThreadPool, g_object_ref(task), &error);
            if (error != nullptr)
            {
                g_warning("Unable to call LOK_SET_GRAPHIC_SELECTION: %s", error->message);
                g_clear_error(&error);
            }
            g_object_unref(task);

            return TRUE;
        }
    }

    if (priv->m_bInDragGraphicSelection)
    {
        g_info("LOKDocView_Impl::signalButton: end of drag graphic selection");
        priv->m_bInDragGraphicSelection = false;

        GTask* task = g_task_new(pDocView, nullptr, nullptr, nullptr);
        LOEvent* pLOEvent = new LOEvent(LOK_SET_GRAPHIC_SELECTION);
        pLOEvent->m_nSetGraphicSelectionType = LOK_SETGRAPHICSELECTION_END;
        pLOEvent->m_nSetGraphicSelectionX = pixelToTwip(pEvent->x, priv->m_fZoom);
        pLOEvent->m_nSetGraphicSelectionY = pixelToTwip(pEvent->y, priv->m_fZoom);
        g_task_set_task_data(task, pLOEvent, LOEvent::destroy);

        g_thread_pool_push(priv->lokThreadPool, g_object_ref(task), &error);
        if (error != nullptr)
        {
            g_warning("Unable to call LOK_SET_GRAPHIC_SELECTION: %s", error->message);
            g_clear_error(&error);
        }
        g_object_unref(task);

        return TRUE;
    }

    return FALSE;
}

static void
postKeyEventInThread(gpointer data)
{
    GTask* task = G_TASK(data);
    LOKDocView* pDocView = LOK_DOC_VIEW(g_task_get_source_object(task));
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    LOEvent* pLOEvent = static_cast<LOEvent*>(g_task_get_task_data(task));

    std::unique_lock<std::mutex> aGuard(g_aLOKMutex);
    std::stringstream ss;
    ss << "lok::Document::setView(" << priv->m_nViewId << ")";
    g_info("%s", ss.str().c_str());
    priv->m_pDocument->pClass->setView(priv->m_pDocument, priv->m_nViewId);

    if (priv->m_nTileSizeTwips)
    {
        ss.str(std::string());
        ss << "lok::Document::setClientZoom(" << nTileSizePixels << ", " << nTileSizePixels << ", " << priv->m_nTileSizeTwips << ", " << priv->m_nTileSizeTwips << ")";
        g_info("%s", ss.str().c_str());
        priv->m_pDocument->pClass->setClientZoom(priv->m_pDocument,
                                                 nTileSizePixels,
                                                 nTileSizePixels,
                                                 priv->m_nTileSizeTwips,
                                                 priv->m_nTileSizeTwips);
        priv->m_nTileSizeTwips = 0;
    }
    if (priv->m_bVisibleAreaSet)
    {
        ss.str(std::string());
        ss << "lok::Document::setClientVisibleArea(" << priv->m_aVisibleArea.x << ", " << priv->m_aVisibleArea.y << ", ";
        ss << priv->m_aVisibleArea.width << ", " << priv->m_aVisibleArea.height << ")";
        g_info("%s", ss.str().c_str());
        priv->m_pDocument->pClass->setClientVisibleArea(priv->m_pDocument,
                                                        priv->m_aVisibleArea.x,
                                                        priv->m_aVisibleArea.y,
                                                        priv->m_aVisibleArea.width,
                                                        priv->m_aVisibleArea.height);
        priv->m_bVisibleAreaSet = false;
    }

    ss.str(std::string());
    ss << "lok::Document::postKeyEvent(" << pLOEvent->m_nKeyEvent << ", " << pLOEvent->m_nCharCode << ", " << pLOEvent->m_nKeyCode << ")";
    g_info("%s", ss.str().c_str());
    priv->m_pDocument->pClass->postKeyEvent(priv->m_pDocument,
                                            pLOEvent->m_nKeyEvent,
                                            pLOEvent->m_nCharCode,
                                            pLOEvent->m_nKeyCode);
}

static gboolean
signalKey (GtkWidget* pWidget, GdkEventKey* pEvent)
{
    LOKDocView* pDocView = LOK_DOC_VIEW(pWidget);
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    int nCharCode = 0;
    int nKeyCode = 0;
    GError* error = nullptr;

    if (!priv->m_bEdit)
    {
        g_info("signalKey: not in edit mode, ignore");
        return FALSE;
    }

    priv->m_nKeyModifier &= KEY_MOD2;
    switch (pEvent->keyval)
    {
    case GDK_KEY_BackSpace:
        nKeyCode = com::sun::star::awt::Key::BACKSPACE;
        break;
    case GDK_KEY_Delete:
        nKeyCode = com::sun::star::awt::Key::DELETE;
        break;
    case GDK_KEY_Return:
        nKeyCode = com::sun::star::awt::Key::RETURN;
        break;
    case GDK_KEY_Escape:
        nKeyCode = com::sun::star::awt::Key::ESCAPE;
        break;
    case GDK_KEY_Tab:
        nKeyCode = com::sun::star::awt::Key::TAB;
        break;
    case GDK_KEY_Down:
        nKeyCode = com::sun::star::awt::Key::DOWN;
        break;
    case GDK_KEY_Up:
        nKeyCode = com::sun::star::awt::Key::UP;
        break;
    case GDK_KEY_Left:
        nKeyCode = com::sun::star::awt::Key::LEFT;
        break;
    case GDK_KEY_Right:
        nKeyCode = com::sun::star::awt::Key::RIGHT;
        break;
    case GDK_KEY_Page_Down:
        nKeyCode = com::sun::star::awt::Key::PAGEDOWN;
        break;
    case GDK_KEY_Page_Up:
        nKeyCode = com::sun::star::awt::Key::PAGEUP;
        break;
    case GDK_KEY_Shift_L:
    case GDK_KEY_Shift_R:
        if (pEvent->type == GDK_KEY_PRESS)
            priv->m_nKeyModifier |= KEY_SHIFT;
        break;
    case GDK_KEY_Control_L:
    case GDK_KEY_Control_R:
        if (pEvent->type == GDK_KEY_PRESS)
            priv->m_nKeyModifier |= KEY_MOD1;
        break;
    case GDK_KEY_Alt_L:
    case GDK_KEY_Alt_R:
        if (pEvent->type == GDK_KEY_PRESS)
            priv->m_nKeyModifier |= KEY_MOD2;
        else
            priv->m_nKeyModifier &= ~KEY_MOD2;
        break;
    default:
        if (pEvent->keyval >= GDK_KEY_F1 && pEvent->keyval <= GDK_KEY_F26)
            nKeyCode = com::sun::star::awt::Key::F1 + (pEvent->keyval - GDK_KEY_F1);
        else
            nCharCode = gdk_keyval_to_unicode(pEvent->keyval);
    }

    // rsc is not public API, but should be good enough for debugging purposes.
    // If this is needed for real, then probably a new param of type
    // css::awt::KeyModifier is needed in postKeyEvent().
    if (pEvent->state & GDK_SHIFT_MASK)
        nKeyCode |= KEY_SHIFT;

    if (pEvent->state & GDK_CONTROL_MASK)
        nKeyCode |= KEY_MOD1;

    if (priv->m_nKeyModifier & KEY_MOD2)
        nKeyCode |= KEY_MOD2;

    if (nKeyCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2)) {
        if (pEvent->keyval >= GDK_KEY_a && pEvent->keyval <= GDK_KEY_z)
        {
            nKeyCode |= 512 + (pEvent->keyval - GDK_KEY_a);
        }
        else if (pEvent->keyval >= GDK_KEY_A && pEvent->keyval <= GDK_KEY_Z) {
                nKeyCode |= 512 + (pEvent->keyval - GDK_KEY_A);
        }
        else if (pEvent->keyval >= GDK_KEY_0 && pEvent->keyval <= GDK_KEY_9) {
                nKeyCode |= 256 + (pEvent->keyval - GDK_KEY_0);
        }
    }

    if (pEvent->type == GDK_KEY_RELEASE)
    {
        GTask* task = g_task_new(pDocView, nullptr, nullptr, nullptr);
        LOEvent* pLOEvent = new LOEvent(LOK_POST_KEY);
        pLOEvent->m_nKeyEvent = LOK_KEYEVENT_KEYUP;
        pLOEvent->m_nCharCode = nCharCode;
        pLOEvent->m_nKeyCode  = nKeyCode;
        g_task_set_task_data(task, pLOEvent, LOEvent::destroy);
        g_thread_pool_push(priv->lokThreadPool, g_object_ref(task), &error);
        if (error != nullptr)
        {
            g_warning("Unable to call LOK_POST_KEY: %s", error->message);
            g_clear_error(&error);
        }
        g_object_unref(task);
    }
    else
    {
        GTask* task = g_task_new(pDocView, nullptr, nullptr, nullptr);
        LOEvent* pLOEvent = new LOEvent(LOK_POST_KEY);
        pLOEvent->m_nKeyEvent = LOK_KEYEVENT_KEYINPUT;
        pLOEvent->m_nCharCode = nCharCode;
        pLOEvent->m_nKeyCode  = nKeyCode;
        g_task_set_task_data(task, pLOEvent, LOEvent::destroy);
        g_thread_pool_push(priv->lokThreadPool, g_object_ref(task), &error);
        if (error != nullptr)
        {
            g_warning("Unable to call LOK_POST_KEY: %s", error->message);
            g_clear_error(&error);
        }
        g_object_unref(task);
    }

    return FALSE;
}

static gboolean
handleTimeout (gpointer pData)
{
    LOKDocView* pDocView = LOK_DOC_VIEW (pData);
    LOKDocViewPrivate& priv = getPrivate(pDocView);

    if (priv->m_bEdit)
    {
        if (priv->m_bCursorOverlayVisible)
            priv->m_bCursorOverlayVisible = false;
        else
            priv->m_bCursorOverlayVisible = true;
        gtk_widget_queue_draw(GTK_WIDGET(pDocView));
    }

    return G_SOURCE_CONTINUE;
}

static void
commandChanged(LOKDocView* pDocView, const std::string& rString)
{
    g_signal_emit(pDocView, doc_view_signals[COMMAND_CHANGED], 0, rString.c_str());
}

static void
searchNotFound(LOKDocView* pDocView, const std::string& rString)
{
    g_signal_emit(pDocView, doc_view_signals[SEARCH_NOT_FOUND], 0, rString.c_str());
}

static void searchResultCount(LOKDocView* pDocView, const std::string& rString)
{
    g_signal_emit(pDocView, doc_view_signals[SEARCH_RESULT_COUNT], 0, rString.c_str());
}

static void commandResult(LOKDocView* pDocView, const std::string& rString)
{
    g_signal_emit(pDocView, doc_view_signals[COMMAND_RESULT], 0, rString.c_str());
}

static void formulaChanged(LOKDocView* pDocView, const std::string& rString)
{
    g_signal_emit(pDocView, doc_view_signals[FORMULA_CHANGED], 0, rString.c_str());
}

static void reportError(LOKDocView* /*pDocView*/, const std::string& rString)
{
    GtkWidget *dialog = gtk_message_dialog_new(nullptr,
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_CLOSE,
            "%s",
            rString.c_str());
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void
setPart(LOKDocView* pDocView, const std::string& rString)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    priv->m_nPartId = std::stoi(rString);
    g_signal_emit(pDocView, doc_view_signals[PART_CHANGED], 0, priv->m_nPartId);
}

static void
hyperlinkClicked(LOKDocView* pDocView, const std::string& rString)
{
    g_signal_emit(pDocView, doc_view_signals[HYPERLINK_CLICKED], 0, rString.c_str());
}

/// Trigger a redraw, invoked on the main thread by other functions running in a thread.
static gboolean queueDraw(gpointer pData)
{
    GtkWidget* pWidget = static_cast<GtkWidget*>(pData);

    gtk_widget_queue_draw(pWidget);

    return G_SOURCE_REMOVE;
}

/// Set up LOKDocView after the document is loaded, invoked on the main thread by openDocumentInThread() running in a thread.
static gboolean postDocumentLoad(gpointer pData)
{
    LOKDocView* pLOKDocView = static_cast<LOKDocView*>(pData);
    LOKDocViewPrivate& priv = getPrivate(pLOKDocView);

    std::unique_lock<std::mutex> aGuard(g_aLOKMutex);
    priv->m_pDocument->pClass->initializeForRendering(priv->m_pDocument, priv->m_aRenderingArguments.c_str());
    priv->m_nViewId = priv->m_pDocument->pClass->getView(priv->m_pDocument);
    priv->m_pDocument->pClass->registerCallback(priv->m_pDocument, callbackWorker, pLOKDocView);
    priv->m_pDocument->pClass->getDocumentSize(priv->m_pDocument, &priv->m_nDocumentWidthTwips, &priv->m_nDocumentHeightTwips);
    priv->m_nParts = priv->m_pDocument->pClass->getParts(priv->m_pDocument);
    aGuard.unlock();
    priv->m_nTimeoutId = g_timeout_add(600, handleTimeout, pLOKDocView);

    float zoom = priv->m_fZoom;
    long nDocumentWidthTwips = priv->m_nDocumentWidthTwips;
    long nDocumentHeightTwips = priv->m_nDocumentHeightTwips;
    long nDocumentWidthPixels = twipToPixel(nDocumentWidthTwips, zoom);
    long nDocumentHeightPixels = twipToPixel(nDocumentHeightTwips, zoom);
    // Total number of columns in this document.
    guint nColumns = ceil((double)nDocumentWidthPixels / nTileSizePixels);

    priv->m_pTileBuffer = std::unique_ptr<TileBuffer>(new TileBuffer(priv->m_pDocument,
                                                                     nColumns));
    gtk_widget_set_size_request(GTK_WIDGET(pLOKDocView),
                                nDocumentWidthPixels,
                                nDocumentHeightPixels);
    gtk_widget_set_can_focus(GTK_WIDGET(pLOKDocView), TRUE);
    gtk_widget_grab_focus(GTK_WIDGET(pLOKDocView));
    lok_doc_view_set_zoom(pLOKDocView, 1.0);

    return G_SOURCE_REMOVE;
}

/// Implementation of the global callback handler, invoked by globalCallback();
static gboolean
globalCallback (gpointer pData)
{
    CallbackData* pCallback = static_cast<CallbackData*>(pData);
    LOKDocViewPrivate& priv = getPrivate(pCallback->m_pDocView);
    gboolean bModify = false;

    switch (pCallback->m_nType)
    {
    case LOK_CALLBACK_STATUS_INDICATOR_START:
    {
        priv->m_nLoadProgress = 0.0;
        g_signal_emit (pCallback->m_pDocView, doc_view_signals[LOAD_CHANGED], 0, 0.0);
    }
    break;
    case LOK_CALLBACK_STATUS_INDICATOR_SET_VALUE:
    {
        priv->m_nLoadProgress = static_cast<gdouble>(std::stoi(pCallback->m_aPayload)/100.0);
        g_signal_emit (pCallback->m_pDocView, doc_view_signals[LOAD_CHANGED], 0, priv->m_nLoadProgress);
    }
    break;
    case LOK_CALLBACK_STATUS_INDICATOR_FINISH:
    {
        priv->m_nLoadProgress = 1.0;
        g_signal_emit (pCallback->m_pDocView, doc_view_signals[LOAD_CHANGED], 0, 1.0);
    }
    break;
    case LOK_CALLBACK_DOCUMENT_PASSWORD_TO_MODIFY:
        bModify = true;
        SAL_FALLTHROUGH;
    case LOK_CALLBACK_DOCUMENT_PASSWORD:
    {
        char const*const pURL(pCallback->m_aPayload.c_str());
        g_signal_emit (pCallback->m_pDocView, doc_view_signals[PASSWORD_REQUIRED], 0, pURL, bModify);
    }
    break;
    case LOK_CALLBACK_ERROR:
    {
        reportError(pCallback->m_pDocView, pCallback->m_aPayload);
    }
    break;
    default:
        g_assert(false);
        break;
    }
    delete pCallback;

    return G_SOURCE_REMOVE;
}

static void
globalCallbackWorker(int nType, const char* pPayload, void* pData)
{
    LOKDocView* pDocView = LOK_DOC_VIEW (pData);

    CallbackData* pCallback = new CallbackData(nType, pPayload ? pPayload : "(nil)", pDocView);
    g_info("LOKDocView_Impl::globalCallbackWorkerImpl: %s, '%s'", callbackTypeToString(nType), pPayload);
    gdk_threads_add_idle(globalCallback, pCallback);
}

static GdkRectangle
payloadToRectangle (LOKDocView* pDocView, const char* pPayload)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    GdkRectangle aRet;
    gchar** ppCoordinates = g_strsplit(pPayload, ", ", 4);
    gchar** ppCoordinate = ppCoordinates;

    aRet.width = aRet.height = aRet.x = aRet.y = 0;

    if (!*ppCoordinate)
        return aRet;
    aRet.x = atoi(*ppCoordinate);
    if (aRet.x < 0)
        aRet.x = 0;
    ++ppCoordinate;
    if (!*ppCoordinate)
        return aRet;
    aRet.y = atoi(*ppCoordinate);
    if (aRet.y < 0)
        aRet.y = 0;
    ++ppCoordinate;
    if (!*ppCoordinate)
        return aRet;
    aRet.width = atoi(*ppCoordinate);
    if (aRet.x + aRet.width > priv->m_nDocumentWidthTwips)
        aRet.width = priv->m_nDocumentWidthTwips - aRet.x;
    ++ppCoordinate;
    if (!*ppCoordinate)
        return aRet;
    aRet.height = atoi(*ppCoordinate);
    if (aRet.y + aRet.height > priv->m_nDocumentHeightTwips)
        aRet.height = priv->m_nDocumentHeightTwips - aRet.y;
    g_strfreev(ppCoordinates);

    return aRet;
}

static const std::vector<GdkRectangle>
payloadToRectangles(LOKDocView* pDocView, const char* pPayload)
{
    std::vector<GdkRectangle> aRet;

    gchar** ppRectangles = g_strsplit(pPayload, "; ", 0);
    for (gchar** ppRectangle = ppRectangles; *ppRectangle; ++ppRectangle)
        aRet.push_back(payloadToRectangle(pDocView, *ppRectangle));
    g_strfreev(ppRectangles);

    return aRet;
}


static void
setTilesInvalid (LOKDocView* pDocView, const GdkRectangle& rRectangle)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    GdkRectangle aRectanglePixels;
    GdkPoint aStart, aEnd;

    aRectanglePixels.x = twipToPixel(rRectangle.x, priv->m_fZoom);
    aRectanglePixels.y = twipToPixel(rRectangle.y, priv->m_fZoom);
    aRectanglePixels.width = twipToPixel(rRectangle.width, priv->m_fZoom);
    aRectanglePixels.height = twipToPixel(rRectangle.height, priv->m_fZoom);

    aStart.x = aRectanglePixels.y / nTileSizePixels;
    aStart.y = aRectanglePixels.x / nTileSizePixels;
    aEnd.x = (aRectanglePixels.y + aRectanglePixels.height + nTileSizePixels) / nTileSizePixels;
    aEnd.y = (aRectanglePixels.x + aRectanglePixels.width + nTileSizePixels) / nTileSizePixels;
    for (int i = aStart.x; i < aEnd.x; i++)
    {
        for (int j = aStart.y; j < aEnd.y; j++)
        {
            GTask* task = g_task_new(pDocView, nullptr, nullptr, nullptr);
            priv->m_pTileBuffer->setInvalid(i, j, priv->m_fZoom, task, priv->lokThreadPool);
            g_object_unref(task);
        }
    }
}

static gboolean
callback (gpointer pData)
{
    CallbackData* pCallback = static_cast<CallbackData*>(pData);
    LOKDocView* pDocView = LOK_DOC_VIEW (pCallback->m_pDocView);
    LOKDocViewPrivate& priv = getPrivate(pDocView);

    switch (pCallback->m_nType)
    {
    case LOK_CALLBACK_INVALIDATE_TILES:
    {
        if (pCallback->m_aPayload != "EMPTY")
        {
            GdkRectangle aRectangle = payloadToRectangle(pDocView, pCallback->m_aPayload.c_str());
            setTilesInvalid(pDocView, aRectangle);
        }
        else
            priv->m_pTileBuffer->resetAllTiles();

        gtk_widget_queue_draw(GTK_WIDGET(pDocView));
    }
    break;
    case LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
    {
        priv->m_aVisibleCursor = payloadToRectangle(pDocView, pCallback->m_aPayload.c_str());
        priv->m_bCursorOverlayVisible = true;
        g_signal_emit(pDocView, doc_view_signals[CURSOR_CHANGED], 0,
                      priv->m_aVisibleCursor.x,
                      priv->m_aVisibleCursor.y,
                      priv->m_aVisibleCursor.width,
                      priv->m_aVisibleCursor.height);
        gtk_widget_queue_draw(GTK_WIDGET(pDocView));
    }
    break;
    case LOK_CALLBACK_TEXT_SELECTION:
    {
        priv->m_aTextSelectionRectangles = payloadToRectangles(pDocView, pCallback->m_aPayload.c_str());
        gboolean bIsTextSelected = !priv->m_aTextSelectionRectangles.empty();
        // In case the selection is empty, then we get no LOK_CALLBACK_TEXT_SELECTION_START/END events.
        if (!bIsTextSelected)
        {
            memset(&priv->m_aTextSelectionStart, 0, sizeof(priv->m_aTextSelectionStart));
            memset(&priv->m_aHandleStartRect, 0, sizeof(priv->m_aHandleStartRect));
            memset(&priv->m_aTextSelectionEnd, 0, sizeof(priv->m_aTextSelectionEnd));
            memset(&priv->m_aHandleEndRect, 0, sizeof(priv->m_aHandleEndRect));
        }
        else
            memset(&priv->m_aHandleMiddleRect, 0, sizeof(priv->m_aHandleMiddleRect));

        g_signal_emit(pDocView, doc_view_signals[TEXT_SELECTION], 0, bIsTextSelected);
        gtk_widget_queue_draw(GTK_WIDGET(pDocView));
    }
    break;
    case LOK_CALLBACK_TEXT_SELECTION_START:
    {
        priv->m_aTextSelectionStart = payloadToRectangle(pDocView, pCallback->m_aPayload.c_str());
    }
    break;
    case LOK_CALLBACK_TEXT_SELECTION_END:
    {
        priv->m_aTextSelectionEnd = payloadToRectangle(pDocView, pCallback->m_aPayload.c_str());
    }
    break;
    case LOK_CALLBACK_CURSOR_VISIBLE:
    {
        priv->m_bCursorVisible = pCallback->m_aPayload == "true";
    }
    break;
    case LOK_CALLBACK_MOUSE_POINTER:
    {
        // We do not want the cursor to get changed in view-only mode
        if (priv->m_bEdit)
        {
            // The gtk docs claim that most css cursors should be supported, however
            // on my system at least this is not true and many cursors are unsupported.
            // In this case pCursor = null, which results in the default cursor
            // being set.
            GdkCursor* pCursor = gdk_cursor_new_from_name(gtk_widget_get_display(GTK_WIDGET(pDocView)),
                                                          pCallback->m_aPayload.c_str());
            gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(pDocView)), pCursor);
        }
    }
    break;
    case LOK_CALLBACK_GRAPHIC_SELECTION:
    {
        if (pCallback->m_aPayload != "EMPTY")
            priv->m_aGraphicSelection = payloadToRectangle(pDocView, pCallback->m_aPayload.c_str());
        else
            memset(&priv->m_aGraphicSelection, 0, sizeof(priv->m_aGraphicSelection));
        gtk_widget_queue_draw(GTK_WIDGET(pDocView));
    }
    break;
    case LOK_CALLBACK_GRAPHIC_VIEW_SELECTION:
    {
        std::stringstream aStream(pCallback->m_aPayload);
        boost::property_tree::ptree aTree;
        boost::property_tree::read_json(aStream, aTree);
        int nViewId = aTree.get<int>("viewId");
        int nPart = aTree.get<int>("part");
        const std::string& rRectangle = aTree.get<std::string>("selection");
        if (rRectangle != "EMPTY")
            priv->m_aGraphicViewSelections[nViewId] = ViewRectangle(nPart, payloadToRectangle(pDocView, rRectangle.c_str()));
        else
        {
            auto it = priv->m_aGraphicViewSelections.find(nViewId);
            if (it != priv->m_aGraphicViewSelections.end())
                priv->m_aGraphicViewSelections.erase(it);
        }
        gtk_widget_queue_draw(GTK_WIDGET(pDocView));
        break;
    }
    break;
    case LOK_CALLBACK_CELL_CURSOR:
    {
        if (pCallback->m_aPayload != "EMPTY")
            priv->m_aCellCursor = payloadToRectangle(pDocView, pCallback->m_aPayload.c_str());
        else
            memset(&priv->m_aCellCursor, 0, sizeof(priv->m_aCellCursor));
        gtk_widget_queue_draw(GTK_WIDGET(pDocView));
    }
    break;
    case LOK_CALLBACK_HYPERLINK_CLICKED:
    {
        hyperlinkClicked(pDocView, pCallback->m_aPayload);
    }
    break;
    case LOK_CALLBACK_STATE_CHANGED:
    {
        commandChanged(pDocView, pCallback->m_aPayload);
    }
    break;
    case LOK_CALLBACK_SEARCH_NOT_FOUND:
    {
        searchNotFound(pDocView, pCallback->m_aPayload);
    }
    break;
    case LOK_CALLBACK_DOCUMENT_SIZE_CHANGED:
    {
        payloadToSize(pCallback->m_aPayload.c_str(), priv->m_nDocumentWidthTwips, priv->m_nDocumentHeightTwips);
        gtk_widget_set_size_request(GTK_WIDGET(pDocView),
                                    twipToPixel(priv->m_nDocumentWidthTwips, priv->m_fZoom),
                                    twipToPixel(priv->m_nDocumentHeightTwips, priv->m_fZoom));

        g_signal_emit(pDocView, doc_view_signals[SIZE_CHANGED], 0, nullptr);
    }
    break;
    case LOK_CALLBACK_SET_PART:
    {
        setPart(pDocView, pCallback->m_aPayload);
    }
    break;
    case LOK_CALLBACK_SEARCH_RESULT_SELECTION:
    {
        boost::property_tree::ptree aTree;
        std::stringstream aStream(pCallback->m_aPayload);
        boost::property_tree::read_json(aStream, aTree);
        int nCount = aTree.get_child("searchResultSelection").size();
        searchResultCount(pDocView, std::to_string(nCount));
    }
    break;
    case LOK_CALLBACK_UNO_COMMAND_RESULT:
    {
        commandResult(pDocView, pCallback->m_aPayload);
    }
    break;
    case LOK_CALLBACK_CELL_FORMULA:
    {
        formulaChanged(pDocView, pCallback->m_aPayload);
    }
    break;
    case LOK_CALLBACK_ERROR:
    {
        reportError(pDocView, pCallback->m_aPayload);
    }
    break;
    case LOK_CALLBACK_CONTEXT_MENU:
    {
        // TODO: Implement me
        break;
    }
    case LOK_CALLBACK_INVALIDATE_VIEW_CURSOR:
    {
        std::stringstream aStream(pCallback->m_aPayload);
        boost::property_tree::ptree aTree;
        boost::property_tree::read_json(aStream, aTree);
        int nViewId = aTree.get<int>("viewId");
        int nPart = aTree.get<int>("part");
        const std::string& rRectangle = aTree.get<std::string>("rectangle");
        priv->m_aViewCursors[nViewId] = ViewRectangle(nPart, payloadToRectangle(pDocView, rRectangle.c_str()));
        gtk_widget_queue_draw(GTK_WIDGET(pDocView));
        break;
    }
    case LOK_CALLBACK_TEXT_VIEW_SELECTION:
    {
        std::stringstream aStream(pCallback->m_aPayload);
        boost::property_tree::ptree aTree;
        boost::property_tree::read_json(aStream, aTree);
        int nViewId = aTree.get<int>("viewId");
        int nPart = aTree.get<int>("part");
        const std::string& rSelection = aTree.get<std::string>("selection");
        priv->m_aTextViewSelectionRectangles[nViewId] = ViewRectangles(nPart, payloadToRectangles(pDocView, rSelection.c_str()));
        gtk_widget_queue_draw(GTK_WIDGET(pDocView));
        break;
    }
    case LOK_CALLBACK_VIEW_CURSOR_VISIBLE:
    {
        std::stringstream aStream(pCallback->m_aPayload);
        boost::property_tree::ptree aTree;
        boost::property_tree::read_json(aStream, aTree);
        int nViewId = aTree.get<int>("viewId");
        const std::string& rVisible = aTree.get<std::string>("visible");
        priv->m_aViewCursorVisibilities[nViewId] = rVisible == "true";
        gtk_widget_queue_draw(GTK_WIDGET(pDocView));
        break;
    }
    break;
    case LOK_CALLBACK_CELL_VIEW_CURSOR:
    {
        std::stringstream aStream(pCallback->m_aPayload);
        boost::property_tree::ptree aTree;
        boost::property_tree::read_json(aStream, aTree);
        int nViewId = aTree.get<int>("viewId");
        int nPart = aTree.get<int>("part");
        const std::string& rRectangle = aTree.get<std::string>("rectangle");
        if (rRectangle != "EMPTY")
            priv->m_aCellViewCursors[nViewId] = ViewRectangle(nPart, payloadToRectangle(pDocView, rRectangle.c_str()));
        else
        {
            auto it = priv->m_aCellViewCursors.find(nViewId);
            if (it != priv->m_aCellViewCursors.end())
                priv->m_aCellViewCursors.erase(it);
        }
        gtk_widget_queue_draw(GTK_WIDGET(pDocView));
        break;
    }
    case LOK_CALLBACK_VIEW_LOCK:
    {
        std::stringstream aStream(pCallback->m_aPayload);
        boost::property_tree::ptree aTree;
        boost::property_tree::read_json(aStream, aTree);
        int nViewId = aTree.get<int>("viewId");
        int nPart = aTree.get<int>("part");
        const std::string& rRectangle = aTree.get<std::string>("rectangle");
        if (rRectangle != "EMPTY")
            priv->m_aViewLockRectangles[nViewId] = ViewRectangle(nPart, payloadToRectangle(pDocView, rRectangle.c_str()));
        else
        {
            auto it = priv->m_aViewLockRectangles.find(nViewId);
            if (it != priv->m_aViewLockRectangles.end())
                priv->m_aViewLockRectangles.erase(it);
        }
        gtk_widget_queue_draw(GTK_WIDGET(pDocView));
        break;
    }
    default:
        g_assert(false);
        break;
    }
    delete pCallback;

    return G_SOURCE_REMOVE;
}

static void callbackWorker (int nType, const char* pPayload, void* pData)
{
    LOKDocView* pDocView = LOK_DOC_VIEW (pData);

    CallbackData* pCallback = new CallbackData(nType, pPayload ? pPayload : "(nil)", pDocView);
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    std::stringstream ss;
    ss << "callbackWorker, view #" << priv->m_nViewId << ": " << callbackTypeToString(nType) << ", '" << pPayload << "'";
    g_info("%s", ss.str().c_str());
    gdk_threads_add_idle(callback, pCallback);
}

static void
renderHandle(LOKDocView* pDocView,
             cairo_t* pCairo,
             const GdkRectangle& rCursor,
             cairo_surface_t* pHandle,
             GdkRectangle& rRectangle)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    GdkPoint aCursorBottom;
    int nHandleWidth, nHandleHeight;
    double fHandleScale;

    nHandleWidth = cairo_image_surface_get_width(pHandle);
    nHandleHeight = cairo_image_surface_get_height(pHandle);
    // We want to scale down the handle, so that its height is the same as the cursor caret.
    fHandleScale = twipToPixel(rCursor.height, priv->m_fZoom) / nHandleHeight;
    // We want the top center of the handle bitmap to be at the bottom center of the cursor rectangle.
    aCursorBottom.x = twipToPixel(rCursor.x, priv->m_fZoom) + twipToPixel(rCursor.width, priv->m_fZoom) / 2 - (nHandleWidth * fHandleScale) / 2;
    aCursorBottom.y = twipToPixel(rCursor.y, priv->m_fZoom) + twipToPixel(rCursor.height, priv->m_fZoom);

    cairo_save (pCairo);
    cairo_translate(pCairo, aCursorBottom.x, aCursorBottom.y);
    cairo_scale(pCairo, fHandleScale, fHandleScale);
    cairo_set_source_surface(pCairo, pHandle, 0, 0);
    cairo_paint(pCairo);
    cairo_restore (pCairo);

    rRectangle.x = aCursorBottom.x;
    rRectangle.y = aCursorBottom.y;
    rRectangle.width = nHandleWidth * fHandleScale;
    rRectangle.height = nHandleHeight * fHandleScale;
}

/// Renders handles around an rSelection rectangle on pCairo.
static void
renderGraphicHandle(LOKDocView* pDocView,
                    cairo_t* pCairo,
                    const GdkRectangle& rSelection,
                    const GdkRGBA& rColor)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    int nHandleWidth = 9, nHandleHeight = 9;
    GdkRectangle aSelection;

    aSelection.x = twipToPixel(rSelection.x, priv->m_fZoom);
    aSelection.y = twipToPixel(rSelection.y, priv->m_fZoom);
    aSelection.width = twipToPixel(rSelection.width, priv->m_fZoom);
    aSelection.height = twipToPixel(rSelection.height, priv->m_fZoom);

    for (int i = 0; i < GRAPHIC_HANDLE_COUNT; ++i)
    {
        int x = aSelection.x, y = aSelection.y;

        switch (i)
        {
        case 0: // top-left
            break;
        case 1: // top-middle
            x += aSelection.width / 2;
            break;
        case 2: // top-right
            x += aSelection.width;
            break;
        case 3: // middle-left
            y += aSelection.height / 2;
            break;
        case 4: // middle-right
            x += aSelection.width;
            y += aSelection.height / 2;
            break;
        case 5: // bottom-left
            y += aSelection.height;
            break;
        case 6: // bottom-middle
            x += aSelection.width / 2;
            y += aSelection.height;
            break;
        case 7: // bottom-right
            x += aSelection.width;
            y += aSelection.height;
            break;
        }

        // Center the handle.
        x -= nHandleWidth / 2;
        y -= nHandleHeight / 2;

        priv->m_aGraphicHandleRects[i].x = x;
        priv->m_aGraphicHandleRects[i].y = y;
        priv->m_aGraphicHandleRects[i].width = nHandleWidth;
        priv->m_aGraphicHandleRects[i].height = nHandleHeight;

        cairo_set_source_rgb(pCairo, rColor.red, rColor.green, rColor.blue);
        cairo_rectangle(pCairo, x, y, nHandleWidth, nHandleHeight);
        cairo_fill(pCairo);
    }
}

/// Finishes the paint tile operation and returns the result, if any
static gpointer
paintTileFinish(LOKDocView* pDocView, GAsyncResult* res, GError **error)
{
    GTask* task = G_TASK(res);

    g_return_val_if_fail(LOK_IS_DOC_VIEW(pDocView), nullptr);
    g_return_val_if_fail(g_task_is_valid(res, pDocView), nullptr);
    g_return_val_if_fail(error == nullptr || *error == nullptr, nullptr);

    return g_task_propagate_pointer(task, error);
}

/// Callback called in the main UI thread when paintTileInThread in LOK thread has finished
static void
paintTileCallback(GObject* sourceObject, GAsyncResult* res, gpointer userData)
{
    LOKDocView* pDocView = LOK_DOC_VIEW(sourceObject);
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    LOEvent* pLOEvent = static_cast<LOEvent*>(userData);
    std::unique_ptr<TileBuffer>& buffer = priv->m_pTileBuffer;
    int index = pLOEvent->m_nPaintTileX * buffer->m_nWidth + pLOEvent->m_nPaintTileY;
    GError* error;

    error = nullptr;
    cairo_surface_t* pSurface = static_cast<cairo_surface_t*>(paintTileFinish(pDocView, res, &error));
    if (error != nullptr)
    {
        if (error->domain == LOK_TILEBUFFER_ERROR &&
            error->code == LOK_TILEBUFFER_CHANGED)
            g_info("Skipping paint tile request because corresponding"
                   "tile buffer has been destroyed");
        else
            g_warning("Unable to get painted GdkPixbuf: %s", error->message);
        g_error_free(error);
        return;
    }

    buffer->m_mTiles[index].setSurface(pSurface);
    buffer->m_mTiles[index].valid = true;
    gdk_threads_add_idle(queueDraw, GTK_WIDGET(pDocView));

    cairo_surface_destroy(pSurface);
}


static gboolean
renderDocument(LOKDocView* pDocView, cairo_t* pCairo)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    GdkRectangle aVisibleArea;
    long nDocumentWidthPixels = twipToPixel(priv->m_nDocumentWidthTwips, priv->m_fZoom);
    long nDocumentHeightPixels = twipToPixel(priv->m_nDocumentHeightTwips, priv->m_fZoom);
    // Total number of rows / columns in this document.
    guint nRows = ceil((double)nDocumentHeightPixels / nTileSizePixels);
    guint nColumns = ceil((double)nDocumentWidthPixels / nTileSizePixels);

    gdk_cairo_get_clip_rectangle (pCairo, &aVisibleArea);
    aVisibleArea.x = pixelToTwip (aVisibleArea.x, priv->m_fZoom);
    aVisibleArea.y = pixelToTwip (aVisibleArea.y, priv->m_fZoom);
    aVisibleArea.width = pixelToTwip (aVisibleArea.width, priv->m_fZoom);
    aVisibleArea.height = pixelToTwip (aVisibleArea.height, priv->m_fZoom);

    // Render the tiles.
    for (guint nRow = 0; nRow < nRows; ++nRow)
    {
        for (guint nColumn = 0; nColumn < nColumns; ++nColumn)
        {
            GdkRectangle aTileRectangleTwips, aTileRectanglePixels;
            bool bPaint = true;

            // Determine size of the tile: the rightmost/bottommost tiles may
            // be smaller, and we need the size to decide if we need to repaint.
            if (nColumn == nColumns - 1)
                aTileRectanglePixels.width = nDocumentWidthPixels - nColumn * nTileSizePixels;
            else
                aTileRectanglePixels.width = nTileSizePixels;
            if (nRow == nRows - 1)
                aTileRectanglePixels.height = nDocumentHeightPixels - nRow * nTileSizePixels;
            else
                aTileRectanglePixels.height = nTileSizePixels;

            // Determine size and position of the tile in document coordinates,
            // so we can decide if we can skip painting for partial rendering.
            aTileRectangleTwips.x = pixelToTwip(nTileSizePixels, priv->m_fZoom) * nColumn;
            aTileRectangleTwips.y = pixelToTwip(nTileSizePixels, priv->m_fZoom) * nRow;
            aTileRectangleTwips.width = pixelToTwip(aTileRectanglePixels.width, priv->m_fZoom);
            aTileRectangleTwips.height = pixelToTwip(aTileRectanglePixels.height, priv->m_fZoom);

            if (!gdk_rectangle_intersect(&aVisibleArea, &aTileRectangleTwips, nullptr))
                bPaint = false;

            if (bPaint)
            {
                LOEvent* pLOEvent = new LOEvent(LOK_PAINT_TILE);
                pLOEvent->m_nPaintTileX = nRow;
                pLOEvent->m_nPaintTileY = nColumn;
                pLOEvent->m_fPaintTileZoom = priv->m_fZoom;
                pLOEvent->m_pTileBuffer = &*priv->m_pTileBuffer;
                GTask* task = g_task_new(pDocView, nullptr, paintTileCallback, pLOEvent);
                g_task_set_task_data(task, pLOEvent, LOEvent::destroy);

                Tile& currentTile = priv->m_pTileBuffer->getTile(nRow, nColumn, task, priv->lokThreadPool);
                cairo_surface_t* pSurface = currentTile.getBuffer();
                cairo_set_source_surface(pCairo, pSurface,
                                             twipToPixel(aTileRectangleTwips.x, priv->m_fZoom),
                                             twipToPixel(aTileRectangleTwips.y, priv->m_fZoom));
                cairo_paint(pCairo);
                g_object_unref(task);
            }
        }
    }

    return FALSE;
}

static const GdkRGBA& getDarkColor(int nViewId)
{
    static std::map<int, GdkRGBA> aColorMap;
    auto it = aColorMap.find(nViewId);
    if (it != aColorMap.end())
        return it->second;

    // Based on tools/colordata.hxx, COL_AUTHOR1_DARK..COL_AUTHOR9_DARK.
    static std::vector<GdkRGBA> aColors =
    {
        {((double)198)/255, ((double)146)/255, ((double)0)/255, 0},
        {((double)6)/255, ((double)70)/255, ((double)162)/255, 0},
        {((double)87)/255, ((double)157)/255, ((double)28)/255, 0},
        {((double)105)/255, ((double)43)/255, ((double)157)/255, 0},
        {((double)197)/255, ((double)0)/255, ((double)11)/255, 0},
        {((double)0)/255, ((double)128)/255, ((double)128)/255, 0},
        {((double)140)/255, ((double)132)/255, ((double)0)/255, 0},
        {((double)43)/255, ((double)85)/255, ((double)107)/255, 0},
        {((double)209)/255, ((double)118)/255, ((double)0)/255, 0},
    };
    static int nColorCounter = 0;
    GdkRGBA aColor = aColors[nColorCounter++ % aColors.size()];
    aColorMap[nViewId] = aColor;
    return aColorMap[nViewId];
}

static gboolean
renderOverlay(LOKDocView* pDocView, cairo_t* pCairo)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);

    if (priv->m_bEdit && priv->m_bCursorVisible && priv->m_bCursorOverlayVisible && !isEmptyRectangle(priv->m_aVisibleCursor))
    {
        if (priv->m_aVisibleCursor.width < 30)
            // Set a minimal width if it would be 0.
            priv->m_aVisibleCursor.width = 30;

        cairo_set_source_rgb(pCairo, 0, 0, 0);
        cairo_rectangle(pCairo,
                        twipToPixel(priv->m_aVisibleCursor.x, priv->m_fZoom),
                        twipToPixel(priv->m_aVisibleCursor.y, priv->m_fZoom),
                        twipToPixel(priv->m_aVisibleCursor.width, priv->m_fZoom),
                        twipToPixel(priv->m_aVisibleCursor.height, priv->m_fZoom));
        cairo_fill(pCairo);
    }

    // View cursors: they do not blink and are colored.
    if (priv->m_bEdit && !priv->m_aViewCursors.empty())
    {
        for (auto& rPair : priv->m_aViewCursors)
        {
            auto itVisibility = priv->m_aViewCursorVisibilities.find(rPair.first);
            if (itVisibility != priv->m_aViewCursorVisibilities.end() && !itVisibility->second)
                continue;

            // Show view cursors when in Writer or when the part matches.
            if (rPair.second.m_nPart != priv->m_nPartId && priv->m_eDocumentType != LOK_DOCTYPE_TEXT)
                continue;

            GdkRectangle& rCursor = rPair.second.m_aRectangle;
            if (rCursor.width < 30)
                // Set a minimal width if it would be 0.
                rCursor.width = 30;

            const GdkRGBA& rDark = getDarkColor(rPair.first);
            cairo_set_source_rgb(pCairo, rDark.red, rDark.green, rDark.blue);
            cairo_rectangle(pCairo,
                            twipToPixel(rCursor.x, priv->m_fZoom),
                            twipToPixel(rCursor.y, priv->m_fZoom),
                            twipToPixel(rCursor.width, priv->m_fZoom),
                            twipToPixel(rCursor.height, priv->m_fZoom));
            cairo_fill(pCairo);
        }
    }

    if (priv->m_bEdit && priv->m_bCursorVisible && !isEmptyRectangle(priv->m_aVisibleCursor) && priv->m_aTextSelectionRectangles.empty())
    {
        // Have a cursor, but no selection: we need the middle handle.
        gchar* handleMiddlePath = g_strconcat (priv->m_aLOPath, CURSOR_HANDLE_DIR, "handle_image_middle.png", nullptr);
        if (!priv->m_pHandleMiddle)
        {
            priv->m_pHandleMiddle = cairo_image_surface_create_from_png(handleMiddlePath);
            assert(cairo_surface_status(priv->m_pHandleMiddle) == CAIRO_STATUS_SUCCESS);
        }
        g_free (handleMiddlePath);
        renderHandle(pDocView, pCairo, priv->m_aVisibleCursor, priv->m_pHandleMiddle, priv->m_aHandleMiddleRect);
    }

    if (!priv->m_aTextSelectionRectangles.empty())
    {
        for (GdkRectangle& rRectangle : priv->m_aTextSelectionRectangles)
        {
            // Blue with 75% transparency.
            cairo_set_source_rgba(pCairo, ((double)0x43)/255, ((double)0xac)/255, ((double)0xe8)/255, 0.25);
            cairo_rectangle(pCairo,
                            twipToPixel(rRectangle.x, priv->m_fZoom),
                            twipToPixel(rRectangle.y, priv->m_fZoom),
                            twipToPixel(rRectangle.width, priv->m_fZoom),
                            twipToPixel(rRectangle.height, priv->m_fZoom));
            cairo_fill(pCairo);
        }

        // Handles
        if (!isEmptyRectangle(priv->m_aTextSelectionStart))
        {
            // Have a start position: we need a start handle.
            gchar* handleStartPath = g_strconcat (priv->m_aLOPath, CURSOR_HANDLE_DIR, "handle_image_start.png", nullptr);
            if (!priv->m_pHandleStart)
            {
                priv->m_pHandleStart = cairo_image_surface_create_from_png(handleStartPath);
                assert(cairo_surface_status(priv->m_pHandleStart) == CAIRO_STATUS_SUCCESS);
            }
            renderHandle(pDocView, pCairo, priv->m_aTextSelectionStart, priv->m_pHandleStart, priv->m_aHandleStartRect);
            g_free (handleStartPath);
        }
        if (!isEmptyRectangle(priv->m_aTextSelectionEnd))
        {
            // Have a start position: we need an end handle.
            gchar* handleEndPath = g_strconcat (priv->m_aLOPath, CURSOR_HANDLE_DIR, "handle_image_end.png", nullptr);
            if (!priv->m_pHandleEnd)
            {
                priv->m_pHandleEnd = cairo_image_surface_create_from_png(handleEndPath);
                assert(cairo_surface_status(priv->m_pHandleEnd) == CAIRO_STATUS_SUCCESS);
            }
            renderHandle(pDocView, pCairo, priv->m_aTextSelectionEnd, priv->m_pHandleEnd, priv->m_aHandleEndRect);
            g_free (handleEndPath);
        }
    }

    // Selections of other views.
    for (auto& rPair : priv->m_aTextViewSelectionRectangles)
    {
        if (rPair.second.m_nPart != priv->m_nPartId && priv->m_eDocumentType != LOK_DOCTYPE_TEXT)
            continue;

        for (GdkRectangle& rRectangle : rPair.second.m_aRectangles)
        {
            const GdkRGBA& rDark = getDarkColor(rPair.first);
            // 75% transparency.
            cairo_set_source_rgba(pCairo, rDark.red, rDark.green, rDark.blue, 0.25);
            cairo_rectangle(pCairo,
                            twipToPixel(rRectangle.x, priv->m_fZoom),
                            twipToPixel(rRectangle.y, priv->m_fZoom),
                            twipToPixel(rRectangle.width, priv->m_fZoom),
                            twipToPixel(rRectangle.height, priv->m_fZoom));
            cairo_fill(pCairo);
        }
    }

    if (!isEmptyRectangle(priv->m_aGraphicSelection))
    {
        GdkRGBA aBlack{0, 0, 0, 0};
        renderGraphicHandle(pDocView, pCairo, priv->m_aGraphicSelection, aBlack);
    }

    // Graphic selections of other views.
    for (auto& rPair : priv->m_aGraphicViewSelections)
    {
        const ViewRectangle& rRectangle = rPair.second;
        if (rRectangle.m_nPart != priv->m_nPartId && priv->m_eDocumentType != LOK_DOCTYPE_TEXT)
            continue;

        const GdkRGBA& rDark = getDarkColor(rPair.first);
        renderGraphicHandle(pDocView, pCairo, rRectangle.m_aRectangle, rDark);
    }

    // Draw the cell cursor.
    if (!isEmptyRectangle(priv->m_aCellCursor))
    {
        cairo_set_source_rgb(pCairo, 0, 0, 0);
        cairo_rectangle(pCairo,
                        twipToPixel(priv->m_aCellCursor.x, priv->m_fZoom),
                        twipToPixel(priv->m_aCellCursor.y, priv->m_fZoom),
                        twipToPixel(priv->m_aCellCursor.width, priv->m_fZoom),
                        twipToPixel(priv->m_aCellCursor.height, priv->m_fZoom));
        cairo_set_line_width(pCairo, 2.0);
        cairo_stroke(pCairo);
    }

    // Cell view cursors: they are colored.
    for (auto& rPair : priv->m_aCellViewCursors)
    {
        const ViewRectangle& rCursor = rPair.second;
        if (rCursor.m_nPart != priv->m_nPartId)
            continue;

        const GdkRGBA& rDark = getDarkColor(rPair.first);
        cairo_set_source_rgb(pCairo, rDark.red, rDark.green, rDark.blue);
        cairo_rectangle(pCairo,
                        twipToPixel(rCursor.m_aRectangle.x, priv->m_fZoom),
                        twipToPixel(rCursor.m_aRectangle.y, priv->m_fZoom),
                        twipToPixel(rCursor.m_aRectangle.width, priv->m_fZoom),
                        twipToPixel(rCursor.m_aRectangle.height, priv->m_fZoom));
        cairo_set_line_width(pCairo, 2.0);
        cairo_stroke(pCairo);
    }

    // View locks: they are colored.
    for (auto& rPair : priv->m_aViewLockRectangles)
    {
        const ViewRectangle& rRectangle = rPair.second;
        if (rRectangle.m_nPart != priv->m_nPartId)
            continue;

        // Draw a rectangle.
        const GdkRGBA& rDark = getDarkColor(rPair.first);
        cairo_set_source_rgb(pCairo, rDark.red, rDark.green, rDark.blue);
        cairo_rectangle(pCairo,
                        twipToPixel(rRectangle.m_aRectangle.x, priv->m_fZoom),
                        twipToPixel(rRectangle.m_aRectangle.y, priv->m_fZoom),
                        twipToPixel(rRectangle.m_aRectangle.width, priv->m_fZoom),
                        twipToPixel(rRectangle.m_aRectangle.height, priv->m_fZoom));
        cairo_set_line_width(pCairo, 2.0);
        cairo_stroke(pCairo);

        // And a lock.
        cairo_rectangle(pCairo,
                        twipToPixel(rRectangle.m_aRectangle.x + rRectangle.m_aRectangle.width, priv->m_fZoom) - 25,
                        twipToPixel(rRectangle.m_aRectangle.y + rRectangle.m_aRectangle.height, priv->m_fZoom) - 15,
                        20,
                        10);
        cairo_fill(pCairo);
        cairo_arc(pCairo,
                  twipToPixel(rRectangle.m_aRectangle.x + rRectangle.m_aRectangle.width, priv->m_fZoom) - 15,
                  twipToPixel(rRectangle.m_aRectangle.y + rRectangle.m_aRectangle.height, priv->m_fZoom) - 15,
                  5,
                  180.0 * (M_PI/180.0),
                  360.0 * (M_PI/180.0));
        cairo_stroke(pCairo);
    }

    return FALSE;
}

static gboolean
lok_doc_view_signal_button(GtkWidget* pWidget, GdkEventButton* pEvent)
{
    LOKDocView* pDocView = LOK_DOC_VIEW (pWidget);
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    GError* error = nullptr;

    g_info("LOKDocView_Impl::signalButton: %d, %d (in twips: %d, %d)",
           (int)pEvent->x, (int)pEvent->y,
           (int)pixelToTwip(pEvent->x, priv->m_fZoom),
           (int)pixelToTwip(pEvent->y, priv->m_fZoom));
    gtk_widget_grab_focus(GTK_WIDGET(pDocView));

    switch (pEvent->type)
    {
    case GDK_BUTTON_PRESS:
    {
        GdkRectangle aClick;
        aClick.x = pEvent->x;
        aClick.y = pEvent->y;
        aClick.width = 1;
        aClick.height = 1;

        if (handleTextSelectionOnButtonPress(aClick, pDocView))
            return FALSE;
        if (handleGraphicSelectionOnButtonPress(aClick, pDocView))
            return FALSE;

        int nCount = 1;
        if ((pEvent->time - priv->m_nLastButtonPressTime) < 250)
            nCount++;
        priv->m_nLastButtonPressTime = pEvent->time;
        GTask* task = g_task_new(pDocView, nullptr, nullptr, nullptr);
        LOEvent* pLOEvent = new LOEvent(LOK_POST_MOUSE_EVENT);
        pLOEvent->m_nPostMouseEventType = LOK_MOUSEEVENT_MOUSEBUTTONDOWN;
        pLOEvent->m_nPostMouseEventX = pixelToTwip(pEvent->x, priv->m_fZoom);
        pLOEvent->m_nPostMouseEventY = pixelToTwip(pEvent->y, priv->m_fZoom);
        pLOEvent->m_nPostMouseEventCount = nCount;
        switch (pEvent->button)
        {
        case 1:
            pLOEvent->m_nPostMouseEventButton = MOUSE_LEFT;
            break;
        case 2:
            pLOEvent->m_nPostMouseEventButton = MOUSE_MIDDLE;
            break;
        case 3:
            pLOEvent->m_nPostMouseEventButton = MOUSE_RIGHT;
            break;
        }
        pLOEvent->m_nPostMouseEventModifier = priv->m_nKeyModifier;
        priv->m_nLastButtonPressed = pLOEvent->m_nPostMouseEventButton;
        g_task_set_task_data(task, pLOEvent, LOEvent::destroy);

        g_thread_pool_push(priv->lokThreadPool, g_object_ref(task), &error);
        if (error != nullptr)
        {
            g_warning("Unable to call LOK_POST_MOUSE_EVENT: %s", error->message);
            g_clear_error(&error);
        }
        g_object_unref(task);
        break;
    }
    case GDK_BUTTON_RELEASE:
    {
        if (handleTextSelectionOnButtonRelease(pDocView))
            return FALSE;
        if (handleGraphicSelectionOnButtonRelease(pDocView, pEvent))
            return FALSE;

        int nCount = 1;
        if ((pEvent->time - priv->m_nLastButtonReleaseTime) < 250)
            nCount++;
        priv->m_nLastButtonReleaseTime = pEvent->time;
        GTask* task = g_task_new(pDocView, nullptr, nullptr, nullptr);
        LOEvent* pLOEvent = new LOEvent(LOK_POST_MOUSE_EVENT);
        pLOEvent->m_nPostMouseEventType = LOK_MOUSEEVENT_MOUSEBUTTONUP;
        pLOEvent->m_nPostMouseEventX = pixelToTwip(pEvent->x, priv->m_fZoom);
        pLOEvent->m_nPostMouseEventY = pixelToTwip(pEvent->y, priv->m_fZoom);
        pLOEvent->m_nPostMouseEventCount = nCount;
        switch (pEvent->button)
        {
        case 1:
            pLOEvent->m_nPostMouseEventButton = MOUSE_LEFT;
            break;
        case 2:
            pLOEvent->m_nPostMouseEventButton = MOUSE_MIDDLE;
            break;
        case 3:
            pLOEvent->m_nPostMouseEventButton = MOUSE_RIGHT;
            break;
        }
        pLOEvent->m_nPostMouseEventModifier = priv->m_nKeyModifier;
        priv->m_nLastButtonPressed = pLOEvent->m_nPostMouseEventButton;
        g_task_set_task_data(task, pLOEvent, LOEvent::destroy);

        g_thread_pool_push(priv->lokThreadPool, g_object_ref(task), &error);
        if (error != nullptr)
        {
            g_warning("Unable to call LOK_POST_MOUSE_EVENT: %s", error->message);
            g_clear_error(&error);
        }
        g_object_unref(task);
        break;
    }
    default:
        break;
    }
    return FALSE;
}

static void
getDragPoint(GdkRectangle* pHandle,
             GdkEventMotion* pEvent,
             GdkPoint* pPoint)
{
    GdkPoint aCursor, aHandle;

    // Center of the cursor rectangle: we know that it's above the handle.
    aCursor.x = pHandle->x + pHandle->width / 2;
    aCursor.y = pHandle->y - pHandle->height / 2;
    // Center of the handle rectangle.
    aHandle.x = pHandle->x + pHandle->width / 2;
    aHandle.y = pHandle->y + pHandle->height / 2;
    // Our target is the original cursor position + the dragged offset.
    pPoint->x = aCursor.x + (pEvent->x - aHandle.x);
    pPoint->y = aCursor.y + (pEvent->y - aHandle.y);
}

static gboolean
lok_doc_view_signal_motion (GtkWidget* pWidget, GdkEventMotion* pEvent)
{
    LOKDocView* pDocView = LOK_DOC_VIEW (pWidget);
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    GdkPoint aPoint;
    GError* error = nullptr;

    std::unique_lock<std::mutex> aGuard(g_aLOKMutex);
    std::stringstream ss;
    ss << "lok::Document::setView(" << priv->m_nViewId << ")";
    g_info("%s", ss.str().c_str());
    priv->m_pDocument->pClass->setView(priv->m_pDocument, priv->m_nViewId);
    if (priv->m_bInDragMiddleHandle)
    {
        g_info("lcl_signalMotion: dragging the middle handle");
        getDragPoint(&priv->m_aHandleMiddleRect, pEvent, &aPoint);
        priv->m_pDocument->pClass->setTextSelection(priv->m_pDocument, LOK_SETTEXTSELECTION_RESET, pixelToTwip(aPoint.x, priv->m_fZoom), pixelToTwip(aPoint.y, priv->m_fZoom));
        return FALSE;
    }
    if (priv->m_bInDragStartHandle)
    {
        g_info("lcl_signalMotion: dragging the start handle");
        getDragPoint(&priv->m_aHandleStartRect, pEvent, &aPoint);
        priv->m_pDocument->pClass->setTextSelection(priv->m_pDocument, LOK_SETTEXTSELECTION_START, pixelToTwip(aPoint.x, priv->m_fZoom), pixelToTwip(aPoint.y, priv->m_fZoom));
        return FALSE;
    }
    if (priv->m_bInDragEndHandle)
    {
        g_info("lcl_signalMotion: dragging the end handle");
        getDragPoint(&priv->m_aHandleEndRect, pEvent, &aPoint);
        priv->m_pDocument->pClass->setTextSelection(priv->m_pDocument, LOK_SETTEXTSELECTION_END, pixelToTwip(aPoint.x, priv->m_fZoom), pixelToTwip(aPoint.y, priv->m_fZoom));
        return FALSE;
    }
    aGuard.unlock();
    for (int i = 0; i < GRAPHIC_HANDLE_COUNT; ++i)
    {
        if (priv->m_bInDragGraphicHandles[i])
        {
            g_info("lcl_signalMotion: dragging the graphic handle #%d", i);
            return FALSE;
        }
    }
    if (priv->m_bInDragGraphicSelection)
    {
        g_info("lcl_signalMotion: dragging the graphic selection");
        return FALSE;
    }

    GdkRectangle aMotionInTwipsInTwips;
    aMotionInTwipsInTwips.x = pixelToTwip(pEvent->x, priv->m_fZoom);
    aMotionInTwipsInTwips.y = pixelToTwip(pEvent->y, priv->m_fZoom);
    aMotionInTwipsInTwips.width = 1;
    aMotionInTwipsInTwips.height = 1;
    if (gdk_rectangle_intersect(&aMotionInTwipsInTwips, &priv->m_aGraphicSelection, nullptr))
    {
        g_info("lcl_signalMotion: start of drag graphic selection");
        priv->m_bInDragGraphicSelection = true;

        GTask* task = g_task_new(pDocView, nullptr, nullptr, nullptr);
        LOEvent* pLOEvent = new LOEvent(LOK_SET_GRAPHIC_SELECTION);
        pLOEvent->m_nSetGraphicSelectionType = LOK_SETGRAPHICSELECTION_START;
        pLOEvent->m_nSetGraphicSelectionX = pixelToTwip(pEvent->x, priv->m_fZoom);
        pLOEvent->m_nSetGraphicSelectionY = pixelToTwip(pEvent->y, priv->m_fZoom);
        g_task_set_task_data(task, pLOEvent, LOEvent::destroy);

        g_thread_pool_push(priv->lokThreadPool, g_object_ref(task), &error);
        if (error != nullptr)
        {
            g_warning("Unable to call LOK_SET_GRAPHIC_SELECTION: %s", error->message);
            g_clear_error(&error);
        }
        g_object_unref(task);

        return FALSE;
    }

    // Otherwise a mouse move, as on the desktop.

    GTask* task = g_task_new(pDocView, nullptr, nullptr, nullptr);
    LOEvent* pLOEvent = new LOEvent(LOK_POST_MOUSE_EVENT);
    pLOEvent->m_nPostMouseEventType = LOK_MOUSEEVENT_MOUSEMOVE;
    pLOEvent->m_nPostMouseEventX = pixelToTwip(pEvent->x, priv->m_fZoom);
    pLOEvent->m_nPostMouseEventY = pixelToTwip(pEvent->y, priv->m_fZoom);
    pLOEvent->m_nPostMouseEventCount = 1;
    pLOEvent->m_nPostMouseEventButton = priv->m_nLastButtonPressed;
    pLOEvent->m_nPostMouseEventModifier = priv->m_nKeyModifier;

    g_task_set_task_data(task, pLOEvent, LOEvent::destroy);

    g_thread_pool_push(priv->lokThreadPool, g_object_ref(task), &error);
    if (error != nullptr)
    {
        g_warning("Unable to call LOK_MOUSEEVENT_MOUSEMOVE: %s", error->message);
        g_clear_error(&error);
    }
    g_object_unref(task);

    return FALSE;
}

static void
setGraphicSelectionInThread(gpointer data)
{
    GTask* task = G_TASK(data);
    LOKDocView* pDocView = LOK_DOC_VIEW(g_task_get_source_object(task));
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    LOEvent* pLOEvent = static_cast<LOEvent*>(g_task_get_task_data(task));

    std::lock_guard<std::mutex> aGuard(g_aLOKMutex);
    std::stringstream ss;
    ss << "lok::Document::setView(" << priv->m_nViewId << ")";
    g_info("%s", ss.str().c_str());
    priv->m_pDocument->pClass->setView(priv->m_pDocument, priv->m_nViewId);
    ss.str(std::string());
    ss << "lok::Document::setGraphicSelection(" << pLOEvent->m_nSetGraphicSelectionType;
    ss << ", " << pLOEvent->m_nSetGraphicSelectionX;
    ss << ", " << pLOEvent->m_nSetGraphicSelectionY << ")";
    g_info("%s", ss.str().c_str());
    priv->m_pDocument->pClass->setGraphicSelection(priv->m_pDocument,
                                                   pLOEvent->m_nSetGraphicSelectionType,
                                                   pLOEvent->m_nSetGraphicSelectionX,
                                                   pLOEvent->m_nSetGraphicSelectionY);
}

static void
setClientZoomInThread(gpointer data)
{
    GTask* task = G_TASK(data);
    LOKDocView* pDocView = LOK_DOC_VIEW(g_task_get_source_object(task));
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    LOEvent* pLOEvent = static_cast<LOEvent*>(g_task_get_task_data(task));

    std::lock_guard<std::mutex> aGuard(g_aLOKMutex);
    priv->m_pDocument->pClass->setClientZoom(priv->m_pDocument,
                                             pLOEvent->m_nTilePixelWidth,
                                             pLOEvent->m_nTilePixelHeight,
                                             pLOEvent->m_nTileTwipWidth,
                                             pLOEvent->m_nTileTwipHeight);
}

static void
postMouseEventInThread(gpointer data)
{
    GTask* task = G_TASK(data);
    LOKDocView* pDocView = LOK_DOC_VIEW(g_task_get_source_object(task));
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    LOEvent* pLOEvent = static_cast<LOEvent*>(g_task_get_task_data(task));

    std::lock_guard<std::mutex> aGuard(g_aLOKMutex);
    std::stringstream ss;
    ss << "lok::Document::setView(" << priv->m_nViewId << ")";
    g_info("%s", ss.str().c_str());
    priv->m_pDocument->pClass->setView(priv->m_pDocument, priv->m_nViewId);
    ss.str(std::string());
    ss << "lok::Document::postMouseEvent(" << pLOEvent->m_nPostMouseEventType;
    ss << ", " << pLOEvent->m_nPostMouseEventX;
    ss << ", " << pLOEvent->m_nPostMouseEventY;
    ss << ", " << pLOEvent->m_nPostMouseEventCount;
    ss << ", " << pLOEvent->m_nPostMouseEventButton;
    ss << ", " << pLOEvent->m_nPostMouseEventModifier << ")";
    g_info("%s", ss.str().c_str());
    priv->m_pDocument->pClass->postMouseEvent(priv->m_pDocument,
                                              pLOEvent->m_nPostMouseEventType,
                                              pLOEvent->m_nPostMouseEventX,
                                              pLOEvent->m_nPostMouseEventY,
                                              pLOEvent->m_nPostMouseEventCount,
                                              pLOEvent->m_nPostMouseEventButton,
                                              pLOEvent->m_nPostMouseEventModifier);
}

static void
openDocumentInThread (gpointer data)
{
    GTask* task = G_TASK(data);
    LOKDocView* pDocView = LOK_DOC_VIEW(g_task_get_source_object(task));
    LOKDocViewPrivate& priv = getPrivate(pDocView);

    std::lock_guard<std::mutex> aGuard(g_aLOKMutex);
    if ( priv->m_pDocument )
    {
        priv->m_pDocument->pClass->destroy( priv->m_pDocument );
        priv->m_pDocument = nullptr;
    }

    priv->m_pOffice->pClass->registerCallback(priv->m_pOffice, globalCallbackWorker, pDocView);
    priv->m_pDocument = priv->m_pOffice->pClass->documentLoad( priv->m_pOffice, priv->m_aDocPath );
    priv->m_eDocumentType = static_cast<LibreOfficeKitDocumentType>(priv->m_pDocument->pClass->getDocumentType(priv->m_pDocument));
    if ( !priv->m_pDocument )
    {
        char *pError = priv->m_pOffice->pClass->getError( priv->m_pOffice );
        g_task_return_new_error(task, g_quark_from_static_string ("LOK error"), 0, "%s", pError);
    }
    else
    {
        gdk_threads_add_idle(postDocumentLoad, pDocView);
        g_task_return_boolean (task, true);
    }
}

static void
setPartInThread(gpointer data)
{
    GTask* task = G_TASK(data);
    LOKDocView* pDocView = LOK_DOC_VIEW(g_task_get_source_object(task));
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    LOEvent* pLOEvent = static_cast<LOEvent*>(g_task_get_task_data(task));
    int nPart = pLOEvent->m_nPart;

    std::unique_lock<std::mutex> aGuard(g_aLOKMutex);
    std::stringstream ss;
    ss << "lok::Document::setView(" << priv->m_nViewId << ")";
    g_info("%s", ss.str().c_str());
    priv->m_pDocument->pClass->setView(priv->m_pDocument, priv->m_nViewId);
    priv->m_pDocument->pClass->setPart( priv->m_pDocument, nPart );
    aGuard.unlock();

    lok_doc_view_reset_view(pDocView);
}

static void
setPartmodeInThread(gpointer data)
{
    GTask* task = G_TASK(data);
    LOKDocView* pDocView = LOK_DOC_VIEW(g_task_get_source_object(task));
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    LOEvent* pLOEvent = static_cast<LOEvent*>(g_task_get_task_data(task));
    int nPartMode = pLOEvent->m_nPartMode;

    std::lock_guard<std::mutex> aGuard(g_aLOKMutex);
    std::stringstream ss;
    ss << "lok::Document::setView(" << priv->m_nViewId << ")";
    g_info("%s", ss.str().c_str());
    priv->m_pDocument->pClass->setView(priv->m_pDocument, priv->m_nViewId);
    priv->m_pDocument->pClass->setPartMode( priv->m_pDocument, nPartMode );
}

static void
setEditInThread(gpointer data)
{
    GTask* task = G_TASK(data);
    LOKDocView* pDocView = LOK_DOC_VIEW(g_task_get_source_object(task));
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    LOEvent* pLOEvent = static_cast<LOEvent*>(g_task_get_task_data(task));
    gboolean bWasEdit = priv->m_bEdit;
    gboolean bEdit = pLOEvent->m_bEdit;

    if (!priv->m_bEdit && bEdit)
        g_info("lok_doc_view_set_edit: entering edit mode");
    else if (priv->m_bEdit && !bEdit)
    {
        g_info("lok_doc_view_set_edit: leaving edit mode");
        std::lock_guard<std::mutex> aGuard(g_aLOKMutex);
        std::stringstream ss;
        ss << "lok::Document::setView(" << priv->m_nViewId << ")";
        g_info("%s", ss.str().c_str());
        priv->m_pDocument->pClass->setView(priv->m_pDocument, priv->m_nViewId);
        priv->m_pDocument->pClass->resetSelection(priv->m_pDocument);
    }
    priv->m_bEdit = bEdit;
    g_signal_emit(pDocView, doc_view_signals[EDIT_CHANGED], 0, bWasEdit);
    gdk_threads_add_idle(queueDraw, GTK_WIDGET(pDocView));
}

static void
postCommandInThread (gpointer data)
{
    GTask* task = G_TASK(data);
    LOKDocView* pDocView = LOK_DOC_VIEW(g_task_get_source_object(task));
    LOEvent* pLOEvent = static_cast<LOEvent*>(g_task_get_task_data(task));
    LOKDocViewPrivate& priv = getPrivate(pDocView);

    std::lock_guard<std::mutex> aGuard(g_aLOKMutex);
    std::stringstream ss;
    ss << "lok::Document::setView(" << priv->m_nViewId << ")";
    g_info("%s", ss.str().c_str());
    priv->m_pDocument->pClass->setView(priv->m_pDocument, priv->m_nViewId);
    ss.str(std::string());
    ss << "lok::Document::postUnoCommand(" << pLOEvent->m_pCommand << ", " << pLOEvent->m_pArguments << ")";
    g_info("%s", ss.str().c_str());
    priv->m_pDocument->pClass->postUnoCommand(priv->m_pDocument, pLOEvent->m_pCommand, pLOEvent->m_pArguments, pLOEvent->m_bNotifyWhenFinished);
}

static void
paintTileInThread (gpointer data)
{
    GTask* task = G_TASK(data);
    LOKDocView* pDocView = LOK_DOC_VIEW(g_task_get_source_object(task));
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    LOEvent* pLOEvent = static_cast<LOEvent*>(g_task_get_task_data(task));

    // check if "source" tile buffer is different from "current" tile buffer
    if (pLOEvent->m_pTileBuffer != &*priv->m_pTileBuffer)
    {
        pLOEvent->m_pTileBuffer = nullptr;
        g_task_return_new_error(task,
                                LOK_TILEBUFFER_ERROR,
                                LOK_TILEBUFFER_CHANGED,
                                "TileBuffer has changed");
        return;
    }
    std::unique_ptr<TileBuffer>& buffer = priv->m_pTileBuffer;
    int index = pLOEvent->m_nPaintTileX * buffer->m_nWidth + pLOEvent->m_nPaintTileY;
    if (buffer->m_mTiles.find(index) != buffer->m_mTiles.end() &&
        buffer->m_mTiles[index].valid)
        return;

    cairo_surface_t *pSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, nTileSizePixels, nTileSizePixels);
    if (cairo_surface_status(pSurface) != CAIRO_STATUS_SUCCESS)
    {
        cairo_surface_destroy(pSurface);
        g_task_return_new_error(task,
                                LOK_TILEBUFFER_ERROR,
                                LOK_TILEBUFFER_MEMORY,
                                "Error allocating Surface");
        return;
    }

    unsigned char* pBuffer = cairo_image_surface_get_data(pSurface);
    GdkRectangle aTileRectangle;
    aTileRectangle.x = pixelToTwip(nTileSizePixels, pLOEvent->m_fPaintTileZoom) * pLOEvent->m_nPaintTileY;
    aTileRectangle.y = pixelToTwip(nTileSizePixels, pLOEvent->m_fPaintTileZoom) * pLOEvent->m_nPaintTileX;

    std::unique_lock<std::mutex> aGuard(g_aLOKMutex);
    std::stringstream ss;
    ss << "lok::Document::setView(" << priv->m_nViewId << ")";
    g_info("%s", ss.str().c_str());
    priv->m_pDocument->pClass->setView(priv->m_pDocument, priv->m_nViewId);
    ss.str(std::string());
    GTimer* aTimer = g_timer_new();
    gulong nElapsedMs;
    ss << "lok::Document::paintTile(" << static_cast<void*>(pBuffer) << ", "
        << nTileSizePixels << ", " << nTileSizePixels << ", "
        << aTileRectangle.x << ", " << aTileRectangle.y << ", "
        << pixelToTwip(nTileSizePixels, pLOEvent->m_fPaintTileZoom) << ", "
        << pixelToTwip(nTileSizePixels, pLOEvent->m_fPaintTileZoom) << ")";

    priv->m_pDocument->pClass->paintTile(priv->m_pDocument,
                                         pBuffer,
                                         nTileSizePixels, nTileSizePixels,
                                         aTileRectangle.x, aTileRectangle.y,
                                         pixelToTwip(nTileSizePixels, pLOEvent->m_fPaintTileZoom),
                                         pixelToTwip(nTileSizePixels, pLOEvent->m_fPaintTileZoom));
    aGuard.unlock();

    g_timer_elapsed(aTimer, &nElapsedMs);
    ss << " rendered in " << (nElapsedMs / 1000.) << " milliseconds";
    g_info("%s", ss.str().c_str());
    g_timer_destroy(aTimer);

    cairo_surface_mark_dirty(pSurface);

    // Its likely that while the tilebuffer has changed, one of the paint tile
    // requests has passed the previous check at start of this function, and has
    // rendered the tile already. We want to stop such rendered tiles from being
    // stored in new tile buffer.
    if (pLOEvent->m_pTileBuffer != &*priv->m_pTileBuffer)
    {
        pLOEvent->m_pTileBuffer = nullptr;
        g_task_return_new_error(task,
                                LOK_TILEBUFFER_ERROR,
                                LOK_TILEBUFFER_CHANGED,
                                "TileBuffer has changed");
        return;
    }

    g_task_return_pointer(task, pSurface, reinterpret_cast<GDestroyNotify>(cairo_surface_destroy));
}


static void
lokThreadFunc(gpointer data, gpointer /*user_data*/)
{
    GTask* task = G_TASK(data);
    LOEvent* pLOEvent = static_cast<LOEvent*>(g_task_get_task_data(task));
    LOKDocView* pDocView = LOK_DOC_VIEW(g_task_get_source_object(task));
    LOKDocViewPrivate& priv = getPrivate(pDocView);

    switch (pLOEvent->m_nType)
    {
    case LOK_LOAD_DOC:
        openDocumentInThread(task);
        break;
    case LOK_POST_COMMAND:
        postCommandInThread(task);
        break;
    case LOK_SET_EDIT:
        setEditInThread(task);
        break;
    case LOK_SET_PART:
        setPartInThread(task);
        break;
    case LOK_SET_PARTMODE:
        setPartmodeInThread(task);
        break;
    case LOK_POST_KEY:
        // view-only/editable mode already checked during signal key signal emission
        postKeyEventInThread(task);
        break;
    case LOK_PAINT_TILE:
        paintTileInThread(task);
        break;
    case LOK_POST_MOUSE_EVENT:
        postMouseEventInThread(task);
        break;
    case LOK_SET_GRAPHIC_SELECTION:
        if (priv->m_bEdit)
            setGraphicSelectionInThread(task);
        else
            g_info ("LOK_SET_GRAPHIC_SELECTION: skipping graphical operation in view-only mode");
        break;
    case LOK_SET_CLIENT_ZOOM:
        setClientZoomInThread(task);
        break;
    }

    g_object_unref(task);
}

static void lok_doc_view_init (LOKDocView* pDocView)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    priv.m_pImpl = new LOKDocViewPrivateImpl();

    gtk_widget_add_events(GTK_WIDGET(pDocView),
                          GDK_BUTTON_PRESS_MASK
                          |GDK_BUTTON_RELEASE_MASK
                          |GDK_BUTTON_MOTION_MASK
                          |GDK_KEY_PRESS_MASK
                          |GDK_KEY_RELEASE_MASK);

    priv->lokThreadPool = g_thread_pool_new(lokThreadFunc,
                                            nullptr,
                                            1,
                                            FALSE,
                                            nullptr);
}

static void lok_doc_view_set_property (GObject* object, guint propId, const GValue *value, GParamSpec *pspec)
{
    LOKDocView* pDocView = LOK_DOC_VIEW (object);
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    gboolean bDocPasswordEnabled = priv->m_nLOKFeatures & LOK_FEATURE_DOCUMENT_PASSWORD;
    gboolean bDocPasswordToModifyEnabled = priv->m_nLOKFeatures & LOK_FEATURE_DOCUMENT_PASSWORD_TO_MODIFY;

    switch (propId)
    {
    case PROP_LO_PATH:
        priv->m_aLOPath = g_value_dup_string (value);
        break;
    case PROP_LO_POINTER:
        priv->m_pOffice = static_cast<LibreOfficeKit*>(g_value_get_pointer(value));
        break;
    case PROP_USER_PROFILE_URL:
        priv->m_pUserProfileURL = g_value_dup_string(value);
        break;
    case PROP_DOC_PATH:
        priv->m_aDocPath = g_value_dup_string (value);
        break;
    case PROP_DOC_POINTER:
        priv->m_pDocument = static_cast<LibreOfficeKitDocument*>(g_value_get_pointer(value));
        priv->m_eDocumentType = static_cast<LibreOfficeKitDocumentType>(priv->m_pDocument->pClass->getDocumentType(priv->m_pDocument));
        break;
    case PROP_EDITABLE:
        lok_doc_view_set_edit (pDocView, g_value_get_boolean (value));
        break;
    case PROP_ZOOM:
        lok_doc_view_set_zoom (pDocView, g_value_get_float (value));
        break;
    case PROP_DOC_WIDTH:
        priv->m_nDocumentWidthTwips = g_value_get_long (value);
        break;
    case PROP_DOC_HEIGHT:
        priv->m_nDocumentHeightTwips = g_value_get_long (value);
        break;
    case PROP_DOC_PASSWORD:
        if (g_value_get_boolean (value) != bDocPasswordEnabled)
        {
            priv->m_nLOKFeatures = priv->m_nLOKFeatures ^ LOK_FEATURE_DOCUMENT_PASSWORD;
            priv->m_pOffice->pClass->setOptionalFeatures(priv->m_pOffice, priv->m_nLOKFeatures);
        }
        break;
    case PROP_DOC_PASSWORD_TO_MODIFY:
        if ( g_value_get_boolean (value) != bDocPasswordToModifyEnabled)
        {
            priv->m_nLOKFeatures = priv->m_nLOKFeatures ^ LOK_FEATURE_DOCUMENT_PASSWORD_TO_MODIFY;
            priv->m_pOffice->pClass->setOptionalFeatures(priv->m_pOffice, priv->m_nLOKFeatures);
        }
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, propId, pspec);
    }
}

static void lok_doc_view_get_property (GObject* object, guint propId, GValue *value, GParamSpec *pspec)
{
    LOKDocView* pDocView = LOK_DOC_VIEW (object);
    LOKDocViewPrivate& priv = getPrivate(pDocView);

    switch (propId)
    {
    case PROP_LO_PATH:
        g_value_set_string (value, priv->m_aLOPath);
        break;
    case PROP_LO_POINTER:
        g_value_set_pointer(value, priv->m_pOffice);
        break;
    case PROP_USER_PROFILE_URL:
        g_value_set_string(value, priv->m_pUserProfileURL);
        break;
    case PROP_DOC_PATH:
        g_value_set_string (value, priv->m_aDocPath);
        break;
    case PROP_DOC_POINTER:
        g_value_set_pointer(value, priv->m_pDocument);
        break;
    case PROP_EDITABLE:
        g_value_set_boolean (value, priv->m_bEdit);
        break;
    case PROP_LOAD_PROGRESS:
        g_value_set_double (value, priv->m_nLoadProgress);
        break;
    case PROP_ZOOM:
        g_value_set_float (value, priv->m_fZoom);
        break;
    case PROP_IS_LOADING:
        g_value_set_boolean (value, priv->m_bIsLoading);
        break;
    case PROP_DOC_WIDTH:
        g_value_set_long (value, priv->m_nDocumentWidthTwips);
        break;
    case PROP_DOC_HEIGHT:
        g_value_set_long (value, priv->m_nDocumentHeightTwips);
        break;
    case PROP_CAN_ZOOM_IN:
        g_value_set_boolean (value, priv->m_bCanZoomIn);
        break;
    case PROP_CAN_ZOOM_OUT:
        g_value_set_boolean (value, priv->m_bCanZoomOut);
        break;
    case PROP_DOC_PASSWORD:
        g_value_set_boolean (value, priv->m_nLOKFeatures & LOK_FEATURE_DOCUMENT_PASSWORD);
        break;
    case PROP_DOC_PASSWORD_TO_MODIFY:
        g_value_set_boolean (value, priv->m_nLOKFeatures & LOK_FEATURE_DOCUMENT_PASSWORD_TO_MODIFY);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, propId, pspec);
    }
}

static gboolean lok_doc_view_draw (GtkWidget* pWidget, cairo_t* pCairo)
{
    LOKDocView *pDocView = LOK_DOC_VIEW (pWidget);

    renderDocument (pDocView, pCairo);
    renderOverlay (pDocView, pCairo);

    return FALSE;
}

static void lok_doc_view_finalize (GObject* object)
{
    LOKDocView* pDocView = LOK_DOC_VIEW (object);
    LOKDocViewPrivate& priv = getPrivate(pDocView);

    // Ignore notifications sent to this view on shutdown.
    std::unique_lock<std::mutex> aGuard(g_aLOKMutex);
    std::stringstream ss;
    ss << "lok::Document::setView(" << priv->m_nViewId << ")";
    g_info("%s", ss.str().c_str());
    priv->m_pDocument->pClass->setView(priv->m_pDocument, priv->m_nViewId);
    priv->m_pDocument->pClass->registerCallback(priv->m_pDocument, nullptr, nullptr);
    aGuard.unlock();

    if (priv->m_pDocument && priv->m_pDocument->pClass->getViews(priv->m_pDocument) > 1)
    {
        priv->m_pDocument->pClass->destroyView(priv->m_pDocument, priv->m_nViewId);
    }
    else
    {
        if (priv->m_pDocument)
            priv->m_pDocument->pClass->destroy (priv->m_pDocument);
        if (priv->m_pOffice)
            priv->m_pOffice->pClass->destroy (priv->m_pOffice);
    }
    delete priv.m_pImpl;
    priv.m_pImpl = nullptr;

    G_OBJECT_CLASS (lok_doc_view_parent_class)->finalize (object);
}

static gboolean lok_doc_view_initable_init (GInitable *initable, GCancellable* /*cancellable*/, GError **error)
{
    LOKDocView *pDocView = LOK_DOC_VIEW (initable);
    LOKDocViewPrivate& priv = getPrivate(pDocView);

    if (priv->m_pOffice != nullptr)
        return TRUE;

    priv->m_pOffice = lok_init_2(priv->m_aLOPath, priv->m_pUserProfileURL);

    if (priv->m_pOffice == nullptr)
    {
        g_set_error (error,
                     g_quark_from_static_string ("LOK initialization error"), 0,
                     "Failed to get LibreOfficeKit context. Make sure path (%s) is correct",
                     priv->m_aLOPath);
        return FALSE;
    }

    return TRUE;
}

static void lok_doc_view_initable_iface_init (GInitableIface *iface)
{
    iface->init = lok_doc_view_initable_init;
}

static void lok_doc_view_class_init (LOKDocViewClass* pClass)
{
    GObjectClass *pGObjectClass = G_OBJECT_CLASS(pClass);
    GtkWidgetClass *pWidgetClass = GTK_WIDGET_CLASS(pClass);

    pGObjectClass->get_property = lok_doc_view_get_property;
    pGObjectClass->set_property = lok_doc_view_set_property;
    pGObjectClass->finalize = lok_doc_view_finalize;

    pWidgetClass->draw = lok_doc_view_draw;
    pWidgetClass->button_press_event = lok_doc_view_signal_button;
    pWidgetClass->button_release_event = lok_doc_view_signal_button;
    pWidgetClass->key_press_event = signalKey;
    pWidgetClass->key_release_event = signalKey;
    pWidgetClass->motion_notify_event = lok_doc_view_signal_motion;

    /**
     * LOKDocView:lopath:
     *
     * The absolute path of the LibreOffice install.
     */
    properties[PROP_LO_PATH] =
        g_param_spec_string("lopath",
                            "LO Path",
                            "LibreOffice Install Path",
                            nullptr,
                            static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                     G_PARAM_CONSTRUCT_ONLY |
                                                     G_PARAM_STATIC_STRINGS));

    /**
     * LOKDocView:lopointer:
     *
     * A LibreOfficeKit* in case lok_init() is already called
     * previously.
     */
    properties[PROP_LO_POINTER] =
        g_param_spec_pointer("lopointer",
                             "LO Pointer",
                             "A LibreOfficeKit* from lok_init()",
                             static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT_ONLY |
                                                      G_PARAM_STATIC_STRINGS));

    /**
     * LOKDocView:userprofileurl:
     *
     * The absolute path of the LibreOffice user profile.
     */
    properties[PROP_USER_PROFILE_URL] =
        g_param_spec_string("userprofileurl",
                            "User profile path",
                            "LibreOffice user profile path",
                            nullptr,
                            static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                     G_PARAM_CONSTRUCT_ONLY |
                                                     G_PARAM_STATIC_STRINGS));

    /**
     * LOKDocView:docpath:
     *
     * The path of the document that is currently being viewed.
     */
    properties[PROP_DOC_PATH] =
        g_param_spec_string("docpath",
                            "Document Path",
                            "The URI of the document to open",
                            nullptr,
                            static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                     G_PARAM_STATIC_STRINGS));

    /**
     * LOKDocView:docpointer:
     *
     * A LibreOfficeKitDocument* in case documentLoad() is already called
     * previously.
     */
    properties[PROP_DOC_POINTER] =
        g_param_spec_pointer("docpointer",
                             "Document Pointer",
                             "A LibreOfficeKitDocument* from documentLoad()",
                             static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                      G_PARAM_STATIC_STRINGS));

    /**
     * LOKDocView:editable:
     *
     * Whether the document loaded inside of #LOKDocView is editable or not.
     */
    properties[PROP_EDITABLE] =
        g_param_spec_boolean("editable",
                             "Editable",
                             "Whether the content is in edit mode or not",
                             FALSE,
                             static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                      G_PARAM_STATIC_STRINGS));

    /**
     * LOKDocView:load-progress:
     *
     * The percent completion of the current loading operation of the
     * document. This can be used for progress bars. Note that this is not a
     * very accurate progress indicator, and its value might reset it couple of
     * times to 0 and start again. You should not rely on its numbers.
     */
    properties[PROP_LOAD_PROGRESS] =
        g_param_spec_double("load-progress",
                            "Estimated Load Progress",
                            "Shows the progress of the document load operation",
                            0.0, 1.0, 0.0,
                            static_cast<GParamFlags>(G_PARAM_READABLE |
                                                     G_PARAM_STATIC_STRINGS));

    /**
     * LOKDocView:zoom-level:
     *
     * The current zoom level of the document loaded inside #LOKDocView. The
     * default value is 1.0.
     */
    properties[PROP_ZOOM] =
        g_param_spec_float("zoom-level",
                           "Zoom Level",
                           "The current zoom level of the content",
                           0, 5.0, 1.0,
                           static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                    G_PARAM_STATIC_STRINGS));

    /**
     * LOKDocView:is-loading:
     *
     * Whether the requested document is being loaded or not. %TRUE if it is
     * being loaded, otherwise %FALSE.
     */
    properties[PROP_IS_LOADING] =
        g_param_spec_boolean("is-loading",
                             "Is Loading",
                             "Whether the view is loading a document",
                             FALSE,
                             static_cast<GParamFlags>(G_PARAM_READABLE |
                                                      G_PARAM_STATIC_STRINGS));

    /**
     * LOKDocView:doc-width:
     *
     * The width of the currently loaded document in #LOKDocView in twips.
     */
    properties[PROP_DOC_WIDTH] =
        g_param_spec_long("doc-width",
                          "Document Width",
                          "Width of the document in twips",
                          0, G_MAXLONG, 0,
                          static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                   G_PARAM_STATIC_STRINGS));

    /**
     * LOKDocView:doc-height:
     *
     * The height of the currently loaded document in #LOKDocView in twips.
     */
    properties[PROP_DOC_HEIGHT] =
        g_param_spec_long("doc-height",
                          "Document Height",
                          "Height of the document in twips",
                          0, G_MAXLONG, 0,
                          static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                   G_PARAM_STATIC_STRINGS));

    /**
     * LOKDocView:can-zoom-in:
     *
     * It tells whether the view can further be zoomed in or not.
     */
    properties[PROP_CAN_ZOOM_IN] =
        g_param_spec_boolean("can-zoom-in",
                             "Can Zoom In",
                             "Whether the view can be zoomed in further",
                             TRUE,
                             static_cast<GParamFlags>(G_PARAM_READABLE
                                                      | G_PARAM_STATIC_STRINGS));

    /**
     * LOKDocView:can-zoom-out:
     *
     * It tells whether the view can further be zoomed out or not.
     */
    properties[PROP_CAN_ZOOM_OUT] =
        g_param_spec_boolean("can-zoom-out",
                             "Can Zoom Out",
                             "Whether the view can be zoomed out further",
                             TRUE,
                             static_cast<GParamFlags>(G_PARAM_READABLE
                                                      | G_PARAM_STATIC_STRINGS));

    /**
     * LOKDocView:doc-password:
     *
     * Set it to true if client supports providing password for viewing
     * password protected documents
     */
    properties[PROP_DOC_PASSWORD] =
        g_param_spec_boolean("doc-password",
                             "Document password capability",
                             "Whether client supports providing document passwords",
                             FALSE,
                             static_cast<GParamFlags>(G_PARAM_READWRITE
                                                      | G_PARAM_STATIC_STRINGS));

    /**
     * LOKDocView:doc-password-to-modify:
     *
     * Set it to true if client supports providing password for edit-protected documents
     */
    properties[PROP_DOC_PASSWORD_TO_MODIFY] =
        g_param_spec_boolean("doc-password-to-modify",
                             "Edit document password capability",
                             "Whether the client supports providing passwords to edit documents",
                             FALSE,
                             static_cast<GParamFlags>(G_PARAM_READWRITE
                                                      | G_PARAM_STATIC_STRINGS));

    g_object_class_install_properties(pGObjectClass, PROP_LAST, properties);

    /**
     * LOKDocView::load-changed:
     * @pDocView: the #LOKDocView on which the signal is emitted
     * @fLoadProgress: the new progress value
     */
    doc_view_signals[LOAD_CHANGED] =
        g_signal_new("load-changed",
                     G_TYPE_FROM_CLASS (pGObjectClass),
                     G_SIGNAL_RUN_FIRST,
                     0,
                     nullptr, nullptr,
                     g_cclosure_marshal_VOID__DOUBLE,
                     G_TYPE_NONE, 1,
                     G_TYPE_DOUBLE);

    /**
     * LOKDocView::edit-changed:
     * @pDocView: the #LOKDocView on which the signal is emitted
     * @bEdit: the new edit value of the view
     */
    doc_view_signals[EDIT_CHANGED] =
        g_signal_new("edit-changed",
                     G_TYPE_FROM_CLASS (pGObjectClass),
                     G_SIGNAL_RUN_FIRST,
                     0,
                     nullptr, nullptr,
                     g_cclosure_marshal_VOID__BOOLEAN,
                     G_TYPE_NONE, 1,
                     G_TYPE_BOOLEAN);

    /**
     * LOKDocView::command-changed:
     * @pDocView: the #LOKDocView on which the signal is emitted
     * @aCommand: the command that was changed
     */
    doc_view_signals[COMMAND_CHANGED] =
        g_signal_new("command-changed",
                     G_TYPE_FROM_CLASS(pGObjectClass),
                     G_SIGNAL_RUN_FIRST,
                     0,
                     nullptr, nullptr,
                     g_cclosure_marshal_VOID__STRING,
                     G_TYPE_NONE, 1,
                     G_TYPE_STRING);

    /**
     * LOKDocView::search-not-found:
     * @pDocView: the #LOKDocView on which the signal is emitted
     * @aCommand: the string for which the search was not found.
     */
    doc_view_signals[SEARCH_NOT_FOUND] =
        g_signal_new("search-not-found",
                     G_TYPE_FROM_CLASS(pGObjectClass),
                     G_SIGNAL_RUN_FIRST,
                     0,
                     nullptr, nullptr,
                     g_cclosure_marshal_VOID__STRING,
                     G_TYPE_NONE, 1,
                     G_TYPE_STRING);

    /**
     * LOKDocView::part-changed:
     * @pDocView: the #LOKDocView on which the signal is emitted
     * @aCommand: the part number which the view changed to
     */
    doc_view_signals[PART_CHANGED] =
        g_signal_new("part-changed",
                     G_TYPE_FROM_CLASS(pGObjectClass),
                     G_SIGNAL_RUN_FIRST,
                     0,
                     nullptr, nullptr,
                     g_cclosure_marshal_VOID__INT,
                     G_TYPE_NONE, 1,
                     G_TYPE_INT);

    /**
     * LOKDocView::size-changed:
     * @pDocView: the #LOKDocView on which the signal is emitted
     * @aCommand: NULL, we just notify that want to notify the UI elements that are interested.
     */
    doc_view_signals[SIZE_CHANGED] =
        g_signal_new("size-changed",
                     G_TYPE_FROM_CLASS(pGObjectClass),
                     G_SIGNAL_RUN_FIRST,
                     0,
                     nullptr, nullptr,
                     g_cclosure_marshal_VOID__VOID,
                     G_TYPE_NONE, 1,
                     G_TYPE_INT);

    /**
     * LOKDocView::hyperlinked-clicked:
     * @pDocView: the #LOKDocView on which the signal is emitted
     * @aHyperlink: the URI which the application should handle
     */
    doc_view_signals[HYPERLINK_CLICKED] =
        g_signal_new("hyperlink-clicked",
                     G_TYPE_FROM_CLASS(pGObjectClass),
                     G_SIGNAL_RUN_FIRST,
                     0,
                     nullptr, nullptr,
                     g_cclosure_marshal_VOID__STRING,
                     G_TYPE_NONE, 1,
                     G_TYPE_STRING);

    /**
     * LOKDocView::cursor-changed:
     * @pDocView: the #LOKDocView on which the signal is emitted
     * @nX: The new cursor position (X coordinate) in pixels
     * @nY: The new cursor position (Y coordinate) in pixels
     * @nWidth: The width of new cursor
     * @nHeight: The height of new cursor
     */
    doc_view_signals[CURSOR_CHANGED] =
        g_signal_new("cursor-changed",
                     G_TYPE_FROM_CLASS(pGObjectClass),
                     G_SIGNAL_RUN_FIRST,
                     0,
                     nullptr, nullptr,
                     g_cclosure_marshal_generic,
                     G_TYPE_NONE, 4,
                     G_TYPE_INT, G_TYPE_INT,
                     G_TYPE_INT, G_TYPE_INT);

    /**
     * LOKDocView::search-result-count:
     * @pDocView: the #LOKDocView on which the signal is emitted
     * @aCommand: number of matches.
     */
    doc_view_signals[SEARCH_RESULT_COUNT] =
        g_signal_new("search-result-count",
                     G_TYPE_FROM_CLASS(pGObjectClass),
                     G_SIGNAL_RUN_FIRST,
                     0,
                     nullptr, nullptr,
                     g_cclosure_marshal_VOID__STRING,
                     G_TYPE_NONE, 1,
                     G_TYPE_STRING);

    /**
     * LOKDocView::command-result:
     * @pDocView: the #LOKDocView on which the signal is emitted
     * @aCommand: JSON containing the info about the command that finished,
     * and its success status.
     */
    doc_view_signals[COMMAND_RESULT] =
        g_signal_new("command-result",
                     G_TYPE_FROM_CLASS(pGObjectClass),
                     G_SIGNAL_RUN_FIRST,
                     0,
                     nullptr, nullptr,
                     g_cclosure_marshal_VOID__STRING,
                     G_TYPE_NONE, 1,
                     G_TYPE_STRING);

    /**
     * LOKDocView::formula-changed:
     * @pDocView: the #LOKDocView on which the signal is emitted
     * @aCommand: formula text content
     */
    doc_view_signals[FORMULA_CHANGED] =
        g_signal_new("formula-changed",
                     G_TYPE_FROM_CLASS(pGObjectClass),
                     G_SIGNAL_RUN_FIRST,
                     0,
                     nullptr, nullptr,
                     g_cclosure_marshal_VOID__STRING,
                     G_TYPE_NONE, 1,
                     G_TYPE_STRING);

    /**
     * LOKDocView::text-selection:
     * @pDocView: the #LOKDocView on which the signal is emitted
     * @bIsTextSelected: whether text selected is non-null
     */
    doc_view_signals[TEXT_SELECTION] =
        g_signal_new("text-selection",
                     G_TYPE_FROM_CLASS(pGObjectClass),
                     G_SIGNAL_RUN_FIRST,
                     0,
                     nullptr, nullptr,
                     g_cclosure_marshal_VOID__BOOLEAN,
                     G_TYPE_NONE, 1,
                     G_TYPE_BOOLEAN);

    /**
     * LOKDocView::password-required:
     * @pDocView: the #LOKDocView on which the signal is emitted
     * @pUrl: URL of the document for which password is required
     * @bModify: whether password id required to modify the document
     * This is true when password is required to edit the document,
     * while it can still be viewed without password. In such cases, provide a NULL
     * password for read-only access to the document.
     * If false, password is required for opening the document, and document
     * cannot be opened without providing a valid password.
     *
     * Password must be provided by calling lok_doc_view_set_document_password
     * function with pUrl as provided by the callback.
     *
     * Upon entering a invalid password, another `password-required` signal is
     * emitted.
     * Upon entering a valid password, document starts to load.
     * Upon entering a NULL password: if bModify is %TRUE, document starts to
     * open in view-only mode, else loading of document is aborted.
     */
    doc_view_signals[PASSWORD_REQUIRED] =
        g_signal_new("password-required",
                     G_TYPE_FROM_CLASS(pGObjectClass),
                     G_SIGNAL_RUN_FIRST,
                     0,
                     nullptr, nullptr,
                     g_cclosure_marshal_generic,
                     G_TYPE_NONE, 2,
                     G_TYPE_STRING,
                     G_TYPE_BOOLEAN);
}

SAL_DLLPUBLIC_EXPORT GtkWidget*
lok_doc_view_new (const gchar* pPath, GCancellable *cancellable, GError **error)
{
    return GTK_WIDGET (g_initable_new (LOK_TYPE_DOC_VIEW, cancellable, error,
                                       "lopath", pPath == nullptr ? LOK_PATH : pPath,
                                       "halign", GTK_ALIGN_CENTER,
                                       "valign", GTK_ALIGN_CENTER,
                                       nullptr));
}

SAL_DLLPUBLIC_EXPORT GtkWidget*
lok_doc_view_new_from_user_profile (const gchar* pPath, const gchar* pUserProfile, GCancellable *cancellable, GError **error)
{
    return GTK_WIDGET(g_initable_new(LOK_TYPE_DOC_VIEW, cancellable, error,
                                     "lopath", pPath == nullptr ? LOK_PATH : pPath,
                                     "userprofileurl", pUserProfile,
                                     "halign", GTK_ALIGN_CENTER,
                                     "valign", GTK_ALIGN_CENTER,
                                     nullptr));
}

SAL_DLLPUBLIC_EXPORT GtkWidget* lok_doc_view_new_from_widget(LOKDocView* pOldLOKDocView)
{
    LOKDocViewPrivate& pOldPriv = getPrivate(pOldLOKDocView);
    GtkWidget* pNewDocView = GTK_WIDGET(g_initable_new(LOK_TYPE_DOC_VIEW, /*cancellable=*/nullptr, /*error=*/nullptr,
                                                       "lopath", pOldPriv->m_aLOPath,
                                                       "userprofileurl", pOldPriv->m_pUserProfileURL,
                                                       "lopointer", pOldPriv->m_pOffice,
                                                       "docpointer", pOldPriv->m_pDocument,
                                                       "halign", GTK_ALIGN_CENTER,
                                                       "valign", GTK_ALIGN_CENTER,
                                                       nullptr));

    // No documentLoad(), just a createView().
    LibreOfficeKitDocument* pDocument = lok_doc_view_get_document(LOK_DOC_VIEW(pNewDocView));
    LOKDocViewPrivate& pNewPriv = getPrivate(LOK_DOC_VIEW(pNewDocView));
    // Store the view id only later in postDocumentLoad(), as
    // initializeForRendering() changes the id in Impress.
    pDocument->pClass->createView(pDocument);
    pNewPriv->m_aRenderingArguments = pOldPriv->m_aRenderingArguments;

    postDocumentLoad(pNewDocView);
    return pNewDocView;
}

SAL_DLLPUBLIC_EXPORT gboolean
lok_doc_view_open_document_finish (LOKDocView* pDocView, GAsyncResult* res, GError** error)
{
    GTask* task = G_TASK(res);

    g_return_val_if_fail(g_task_is_valid(res, pDocView), false);
    g_return_val_if_fail(g_task_get_source_tag(task) == lok_doc_view_open_document, false);
    g_return_val_if_fail(error == nullptr || *error == nullptr, false);

    return g_task_propagate_boolean(task, error);
}

SAL_DLLPUBLIC_EXPORT void
lok_doc_view_open_document (LOKDocView* pDocView,
                            const gchar* pPath,
                            const gchar* pRenderingArguments,
                            GCancellable* cancellable,
                            GAsyncReadyCallback callback,
                            gpointer userdata)
{
    GTask* task = g_task_new(pDocView, cancellable, callback, userdata);
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    GError* error = nullptr;

    LOEvent* pLOEvent = new LOEvent(LOK_LOAD_DOC);
    pLOEvent->m_pPath = pPath;

    priv->m_aDocPath = pPath;
    if (pRenderingArguments)
        priv->m_aRenderingArguments = pRenderingArguments;
    g_task_set_task_data(task, pLOEvent, LOEvent::destroy);
    g_task_set_source_tag(task, reinterpret_cast<gpointer>(lok_doc_view_open_document));

    g_thread_pool_push(priv->lokThreadPool, g_object_ref(task), &error);
    if (error != nullptr)
    {
        g_warning("Unable to call LOK_LOAD_DOC: %s", error->message);
        g_clear_error(&error);
    }
    g_object_unref(task);
}

SAL_DLLPUBLIC_EXPORT LibreOfficeKitDocument*
lok_doc_view_get_document (LOKDocView* pDocView)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    return priv->m_pDocument;
}

SAL_DLLPUBLIC_EXPORT void
lok_doc_view_set_visible_area (LOKDocView* pDocView, GdkRectangle* pVisibleArea)
{
    if (!pVisibleArea)
        return;

    LOKDocViewPrivate& priv = getPrivate(pDocView);
    priv->m_aVisibleArea = *pVisibleArea;
    priv->m_bVisibleAreaSet = true;
}

SAL_DLLPUBLIC_EXPORT void
lok_doc_view_set_zoom (LOKDocView* pDocView, float fZoom)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    GError* error = nullptr;

    if (!priv->m_pDocument)
        return;

    // Clamp the input value in [MIN_ZOOM, MAX_ZOOM]
    fZoom = fZoom < MIN_ZOOM ? MIN_ZOOM : fZoom;
    fZoom = fZoom > MAX_ZOOM ? MAX_ZOOM : fZoom;

    if (rtl::math::approxEqual(fZoom, priv->m_fZoom))
        return;

    priv->m_fZoom = fZoom;
    long nDocumentWidthPixels = twipToPixel(priv->m_nDocumentWidthTwips, fZoom);
    long nDocumentHeightPixels = twipToPixel(priv->m_nDocumentHeightTwips, fZoom);
    // Total number of columns in this document.
    guint nColumns = ceil((double)nDocumentWidthPixels / nTileSizePixels);

    priv->m_pTileBuffer = std::unique_ptr<TileBuffer>(new TileBuffer(priv->m_pDocument,
                                                                     nColumns));
    gtk_widget_set_size_request(GTK_WIDGET(pDocView),
                                nDocumentWidthPixels,
                                nDocumentHeightPixels);

    g_object_notify_by_pspec(G_OBJECT(pDocView), properties[PROP_ZOOM]);

    // set properties to indicate if view can be further zoomed in/out
    bool bCanZoomIn  = priv->m_fZoom < MAX_ZOOM;
    bool bCanZoomOut = priv->m_fZoom > MIN_ZOOM;
    if (bCanZoomIn != bool(priv->m_bCanZoomIn))
    {
        priv->m_bCanZoomIn = bCanZoomIn;
        g_object_notify_by_pspec(G_OBJECT(pDocView), properties[PROP_CAN_ZOOM_IN]);
    }
    if (bCanZoomOut != bool(priv->m_bCanZoomOut))
    {
        priv->m_bCanZoomOut = bCanZoomOut;
        g_object_notify_by_pspec(G_OBJECT(pDocView), properties[PROP_CAN_ZOOM_OUT]);
    }

    // Update the client's view size
    GTask* task = g_task_new(pDocView, nullptr, nullptr, nullptr);
    LOEvent* pLOEvent = new LOEvent(LOK_SET_CLIENT_ZOOM);
    pLOEvent->m_nTilePixelWidth = nTileSizePixels;
    pLOEvent->m_nTilePixelHeight = nTileSizePixels;
    pLOEvent->m_nTileTwipWidth = pixelToTwip(nTileSizePixels, fZoom);
    pLOEvent->m_nTileTwipHeight = pixelToTwip(nTileSizePixels, fZoom);
    g_task_set_task_data(task, pLOEvent, LOEvent::destroy);

    g_thread_pool_push(priv->lokThreadPool, g_object_ref(task), &error);
    if (error != nullptr)
    {
        g_warning("Unable to call LOK_SET_CLIENT_ZOOM: %s", error->message);
        g_clear_error(&error);
    }
    g_object_unref(task);

    priv->m_nTileSizeTwips = pixelToTwip(nTileSizePixels, priv->m_fZoom);
}

SAL_DLLPUBLIC_EXPORT gfloat
lok_doc_view_get_zoom (LOKDocView* pDocView)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    return priv->m_fZoom;
}

SAL_DLLPUBLIC_EXPORT gint
lok_doc_view_get_parts (LOKDocView* pDocView)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    if (!priv->m_pDocument)
        return -1;

    std::unique_lock<std::mutex> aGuard(g_aLOKMutex);
    std::stringstream ss;
    ss << "lok::Document::setView(" << priv->m_nViewId << ")";
    g_info("%s", ss.str().c_str());
    priv->m_pDocument->pClass->setView(priv->m_pDocument, priv->m_nViewId);
    return priv->m_pDocument->pClass->getParts( priv->m_pDocument );
}

SAL_DLLPUBLIC_EXPORT gint
lok_doc_view_get_part (LOKDocView* pDocView)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    if (!priv->m_pDocument)
        return -1;

    std::unique_lock<std::mutex> aGuard(g_aLOKMutex);
    std::stringstream ss;
    ss << "lok::Document::setView(" << priv->m_nViewId << ")";
    g_info("%s", ss.str().c_str());
    priv->m_pDocument->pClass->setView(priv->m_pDocument, priv->m_nViewId);
    return priv->m_pDocument->pClass->getPart( priv->m_pDocument );
}

SAL_DLLPUBLIC_EXPORT void
lok_doc_view_set_part (LOKDocView* pDocView, int nPart)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    if (!priv->m_pDocument)
        return;

    if (nPart < 0 || nPart >= priv->m_nParts)
    {
        g_warning("Invalid part request : %d", nPart);
        return;
    }

    GTask* task = g_task_new(pDocView, nullptr, nullptr, nullptr);
    LOEvent* pLOEvent = new LOEvent(LOK_SET_PART);
    GError* error = nullptr;

    pLOEvent->m_nPart = nPart;
    g_task_set_task_data(task, pLOEvent, LOEvent::destroy);

    g_thread_pool_push(priv->lokThreadPool, g_object_ref(task), &error);
    if (error != nullptr)
    {
        g_warning("Unable to call LOK_SET_PART: %s", error->message);
        g_clear_error(&error);
    }
    g_object_unref(task);
    priv->m_nPartId = nPart;
}

SAL_DLLPUBLIC_EXPORT gchar*
lok_doc_view_get_part_name (LOKDocView* pDocView, int nPart)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    if (!priv->m_pDocument)
        return nullptr;

    std::unique_lock<std::mutex> aGuard(g_aLOKMutex);
    std::stringstream ss;
    ss << "lok::Document::setView(" << priv->m_nViewId << ")";
    g_info("%s", ss.str().c_str());
    priv->m_pDocument->pClass->setView(priv->m_pDocument, priv->m_nViewId);
    return priv->m_pDocument->pClass->getPartName( priv->m_pDocument, nPart );
}

SAL_DLLPUBLIC_EXPORT void
lok_doc_view_set_partmode(LOKDocView* pDocView,
                          int nPartMode)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    if (!priv->m_pDocument)
        return;

    GTask* task = g_task_new(pDocView, nullptr, nullptr, nullptr);
    LOEvent* pLOEvent = new LOEvent(LOK_SET_PARTMODE);
    GError* error = nullptr;

    pLOEvent->m_nPartMode = nPartMode;
    g_task_set_task_data(task, pLOEvent, LOEvent::destroy);

    g_thread_pool_push(priv->lokThreadPool, g_object_ref(task), &error);
    if (error != nullptr)
    {
        g_warning("Unable to call LOK_SET_PARTMODE: %s", error->message);
        g_clear_error(&error);
    }
    g_object_unref(task);
}

SAL_DLLPUBLIC_EXPORT void
lok_doc_view_reset_view(LOKDocView* pDocView)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);

    if (priv->m_pTileBuffer != nullptr)
        priv->m_pTileBuffer->resetAllTiles();
    priv->m_nLoadProgress = 0.0;

    memset(&priv->m_aVisibleCursor, 0, sizeof(priv->m_aVisibleCursor));
    priv->m_bCursorOverlayVisible = false;
    priv->m_bCursorVisible = false;

    priv->m_nLastButtonPressTime = 0;
    priv->m_nLastButtonReleaseTime = 0;
    priv->m_aTextSelectionRectangles.clear();

    memset(&priv->m_aTextSelectionStart, 0, sizeof(priv->m_aTextSelectionStart));
    memset(&priv->m_aTextSelectionEnd, 0, sizeof(priv->m_aTextSelectionEnd));
    memset(&priv->m_aGraphicSelection, 0, sizeof(priv->m_aGraphicSelection));
    priv->m_bInDragGraphicSelection = false;
    memset(&priv->m_aCellCursor, 0, sizeof(priv->m_aCellCursor));

    cairo_surface_destroy(priv->m_pHandleStart);
    priv->m_pHandleStart = nullptr;
    memset(&priv->m_aHandleStartRect, 0, sizeof(priv->m_aHandleStartRect));
    priv->m_bInDragStartHandle = false;

    cairo_surface_destroy(priv->m_pHandleMiddle);
    priv->m_pHandleMiddle = nullptr;
    memset(&priv->m_aHandleMiddleRect, 0, sizeof(priv->m_aHandleMiddleRect));
    priv->m_bInDragMiddleHandle = false;

    cairo_surface_destroy(priv->m_pHandleEnd);
    priv->m_pHandleEnd = nullptr;
    memset(&priv->m_aHandleEndRect, 0, sizeof(priv->m_aHandleEndRect));
    priv->m_bInDragEndHandle = false;

    memset(&priv->m_aGraphicHandleRects, 0, sizeof(priv->m_aGraphicHandleRects));
    memset(&priv->m_bInDragGraphicHandles, 0, sizeof(priv->m_bInDragGraphicHandles));

    gtk_widget_queue_draw(GTK_WIDGET(pDocView));
}

SAL_DLLPUBLIC_EXPORT void
lok_doc_view_set_edit(LOKDocView* pDocView,
                      gboolean bEdit)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    if (!priv->m_pDocument)
        return;

    GTask* task = g_task_new(pDocView, nullptr, nullptr, nullptr);
    LOEvent* pLOEvent = new LOEvent(LOK_SET_EDIT);
    GError* error = nullptr;

    pLOEvent->m_bEdit = bEdit;
    g_task_set_task_data(task, pLOEvent, LOEvent::destroy);

    g_thread_pool_push(priv->lokThreadPool, g_object_ref(task), &error);
    if (error != nullptr)
    {
        g_warning("Unable to call LOK_SET_EDIT: %s", error->message);
        g_clear_error(&error);
    }
    g_object_unref(task);
}

SAL_DLLPUBLIC_EXPORT gboolean
lok_doc_view_get_edit (LOKDocView* pDocView)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    return priv->m_bEdit;
}

SAL_DLLPUBLIC_EXPORT void
lok_doc_view_post_command (LOKDocView* pDocView,
                           const gchar* pCommand,
                           const gchar* pArguments,
                           gboolean bNotifyWhenFinished)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    if (!priv->m_pDocument)
        return;

    if (priv->m_bEdit)
        LOKPostCommand(pDocView, pCommand, pArguments, bNotifyWhenFinished);
    else
        g_info ("LOK_POST_COMMAND: ignoring commands in view-only mode");
}

SAL_DLLPUBLIC_EXPORT void
lok_doc_view_find_prev (LOKDocView* pDocView,
                        const gchar* pText,
                        gboolean bHighlightAll)
{
    doSearch(pDocView, pText, true, bHighlightAll);
}

SAL_DLLPUBLIC_EXPORT void
lok_doc_view_find_next (LOKDocView* pDocView,
                        const gchar* pText,
                        gboolean bHighlightAll)
{
    doSearch(pDocView, pText, false, bHighlightAll);
}

SAL_DLLPUBLIC_EXPORT void
lok_doc_view_highlight_all (LOKDocView* pDocView,
                            const gchar* pText)
{
    doSearch(pDocView, pText, false, true);
}

SAL_DLLPUBLIC_EXPORT gchar*
lok_doc_view_copy_selection (LOKDocView* pDocView,
                             const gchar* pMimeType,
                             gchar** pUsedMimeType)
{
    LibreOfficeKitDocument* pDocument = lok_doc_view_get_document(pDocView);
    if (!pDocument)
        return nullptr;

    std::stringstream ss;
    ss << "lok::Document::getTextSelection('" << pMimeType << "')";
    g_info("%s", ss.str().c_str());
    return pDocument->pClass->getTextSelection(pDocument, pMimeType, pUsedMimeType);
}

SAL_DLLPUBLIC_EXPORT gboolean
lok_doc_view_paste (LOKDocView* pDocView,
                    const gchar* pMimeType,
                    const gchar* pData,
                    gsize nSize)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    LibreOfficeKitDocument* pDocument = priv->m_pDocument;
    gboolean ret = 0;

    if (!pDocument)
        return false;

    if (!priv->m_bEdit)
    {
        g_info ("ignoring paste in view-only mode");
        return ret;
    }

    if (pData)
    {
        std::stringstream ss;
        ss << "lok::Document::paste('" << pMimeType << "', '" << std::string(pData, nSize) << ", "<<nSize<<"')";
        g_info("%s", ss.str().c_str());
        ret = pDocument->pClass->paste(pDocument, pMimeType, pData, nSize);
    }

    return ret;
}

SAL_DLLPUBLIC_EXPORT void
lok_doc_view_set_document_password (LOKDocView* pDocView,
                                    const gchar* pURL,
                                    const gchar* pPassword)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);

    priv->m_pOffice->pClass->setDocumentPassword(priv->m_pOffice, pURL, pPassword);
}

SAL_DLLPUBLIC_EXPORT gchar*
lok_doc_view_get_version_info (LOKDocView* pDocView)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);

    return priv->m_pOffice->pClass->getVersionInfo(priv->m_pOffice);
}


SAL_DLLPUBLIC_EXPORT gfloat
lok_doc_view_pixel_to_twip (LOKDocView* pDocView, float fInput)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    return pixelToTwip(fInput, priv->m_fZoom);
}

SAL_DLLPUBLIC_EXPORT gfloat
lok_doc_view_twip_to_pixel (LOKDocView* pDocView, float fInput)
{
    LOKDocViewPrivate& priv = getPrivate(pDocView);
    return twipToPixel(fInput, priv->m_fZoom);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
