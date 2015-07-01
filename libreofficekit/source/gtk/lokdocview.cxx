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
#include <vector>
#include <string>

#include <com/sun/star/awt/Key.hpp>
#define LOK_USE_UNSTABLE_API
#include <LibreOfficeKit/LibreOfficeKit.h>
#include <LibreOfficeKit/LibreOfficeKitInit.h>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <LibreOfficeKit/LibreOfficeKitGtk.h>
#include <rsc/rsc-vcl-shared-types.hxx>

#include "tilebuffer.hxx"

#if !GLIB_CHECK_VERSION(2,32,0)
#define G_SOURCE_REMOVE FALSE
#define G_SOURCE_CONTINUE TRUE
#endif
#if !GLIB_CHECK_VERSION(2,40,0)
#define g_info(...) g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, __VA_ARGS__)
#endif

// Cursor bitmaps from the Android app.
#define CURSOR_HANDLE_DIR "/android/source/res/drawable/"
// Number of handles around a graphic selection.
#define GRAPHIC_HANDLE_COUNT 8

struct _LOKDocViewPrivate
{
    gchar* m_aLOPath;
    gchar* m_aDocPath;
    guint m_nLoadProgress;
    gboolean m_bIsLoading;
    gboolean m_bCanZoomIn;
    gboolean m_bCanZoomOut;
    LibreOfficeKit* m_pOffice;
    LibreOfficeKitDocument* m_pDocument;

    TileBuffer m_aTileBuffer;

    gfloat m_fZoom;
    glong m_nDocumentWidthTwips;
    glong m_nDocumentHeightTwips;
    /// View or edit mode.
    gboolean m_bEdit;
    /// Position and size of the visible cursor.
    GdkRectangle m_aVisibleCursor;
    /// Cursor overlay is visible or hidden (for blinking).
    gboolean m_bCursorOverlayVisible;
    /// Cursor is visible or hidden (e.g. for graphic selection).
    gboolean m_bCursorVisible;
    /// Time of the last button press.
    guint32 m_nLastButtonPressTime;
    /// Time of the last button release.
    guint32 m_nLastButtonReleaseTime;
    /// Rectangles of the current text selection.
    std::vector<GdkRectangle> m_aTextSelectionRectangles;
    /// Position and size of the selection start (as if there would be a cursor caret there).
    GdkRectangle m_aTextSelectionStart;
    /// Position and size of the selection end.
    GdkRectangle m_aTextSelectionEnd;
    GdkRectangle m_aGraphicSelection;
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
    /// Bitmap of a graphic selection handle.
    cairo_surface_t* m_pGraphicHandle;
    /// Rectangle of a graphic selection handle, to know if the user clicked on it or not.
    GdkRectangle m_aGraphicHandleRects[8];
    /// If we are in the middle of a drag of a graphic selection handle.
    gboolean m_bInDragGraphicHandles[8];
    ///@}
};

enum
{
    LOAD_CHANGED,
    LOAD_FAILED,
    EDIT_CHANGED,
    COMMAND_CHANGED,
    SEARCH_NOT_FOUND,
    PART_CHANGED,
    HYPERLINK_CLICKED,

    LAST_SIGNAL
};

enum
{
    PROP_0,

    PROP_LO_PATH,
    PROP_DOC_PATH,
    PROP_EDITABLE,
    PROP_LOAD_PROGRESS,
    PROP_ZOOM,
    PROP_IS_LOADING,
    PROP_DOC_WIDTH,
    PROP_DOC_HEIGHT,
    PROP_CAN_ZOOM_IN,
    PROP_CAN_ZOOM_OUT
};

static guint doc_view_signals[LAST_SIGNAL] = { 0 };

static void lok_doc_view_initable_iface_init (GInitableIface *iface);

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
    case LOK_CALLBACK_GRAPHIC_SELECTION:
        return "LOK_CALLBACK_GRAPHIC_SELECTION";
    case LOK_CALLBACK_HYPERLINK_CLICKED:
        return "LOK_CALLBACK_HYPERLINK_CLICKED";
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
    }
    return 0;
}

static bool
isEmptyRectangle(const GdkRectangle& rRectangle)
{
    return rRectangle.x == 0 && rRectangle.y == 0 && rRectangle.width == 0 && rRectangle.height == 0;
}

static gboolean
signalKey (GtkWidget* pWidget, GdkEventKey* pEvent)
{
    LOKDocView* pDocView = LOK_DOC_VIEW(pWidget);
    LOKDocViewPrivate* priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));
    int nCharCode = 0;
    int nKeyCode = 0;

    if (!priv->m_bEdit)
    {
        g_info("signalKey: not in edit mode, ignore");
        return FALSE;
    }

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

    if (pEvent->type == GDK_KEY_RELEASE)
        priv->m_pDocument->pClass->postKeyEvent(priv->m_pDocument, LOK_KEYEVENT_KEYUP, nCharCode, nKeyCode);
    else
        priv->m_pDocument->pClass->postKeyEvent(priv->m_pDocument, LOK_KEYEVENT_KEYINPUT, nCharCode, nKeyCode);

    return FALSE;
}

static gboolean
handleTimeout (gpointer pData)
{
    LOKDocView* pDocView = LOK_DOC_VIEW (pData);
    LOKDocViewPrivate* priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));

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

static void
setPart(LOKDocView* pDocView, const std::string& rString)
{
    g_signal_emit(pDocView, doc_view_signals[PART_CHANGED], 0, std::stoi(rString));
}

static void
hyperlinkClicked(LOKDocView* pDocView, const std::string& rString)
{
    g_signal_emit(pDocView, doc_view_signals[HYPERLINK_CLICKED], 0, rString.c_str());
}

/// Implementation of the global callback handler, invoked by globalCallback();
static gboolean
globalCallback (gpointer pData)
{
    CallbackData* pCallback = static_cast<CallbackData*>(pData);

    switch (pCallback->m_nType)
    {
    case LOK_CALLBACK_STATUS_INDICATOR_START:
    {
    }
    break;
    case LOK_CALLBACK_STATUS_INDICATOR_SET_VALUE:
    {
    }
    break;
    case LOK_CALLBACK_STATUS_INDICATOR_FINISH:
    {
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
    LOKDocViewPrivate* priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));
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
    LOKDocViewPrivate* priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));
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
        for (int j = aStart.y; j < aEnd.y; j++)
            priv->m_aTileBuffer.setInvalid(i, j);
}

static gboolean
callback (gpointer pData)
{
    CallbackData* pCallback = static_cast<CallbackData*>(pData);
    LOKDocView* pDocView = LOK_DOC_VIEW (pCallback->m_pDocView);
    LOKDocViewPrivate* priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));

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
            priv->m_aTileBuffer.resetAllTiles();

        gtk_widget_queue_draw(GTK_WIDGET(pDocView));
    }
    break;
    case LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
    {
        priv->m_aVisibleCursor = payloadToRectangle(pDocView, pCallback->m_aPayload.c_str());
        priv->m_bCursorOverlayVisible = true;
        gtk_widget_queue_draw(GTK_WIDGET(pDocView));
    }
    break;
    case LOK_CALLBACK_TEXT_SELECTION:
    {
        priv->m_aTextSelectionRectangles = payloadToRectangles(pDocView, pCallback->m_aPayload.c_str());
        // In case the selection is empty, then we get no LOK_CALLBACK_TEXT_SELECTION_START/END events.
        if (priv->m_aTextSelectionRectangles.empty())
        {
            memset(&priv->m_aTextSelectionStart, 0, sizeof(priv->m_aTextSelectionStart));
            memset(&priv->m_aHandleStartRect, 0, sizeof(priv->m_aHandleStartRect));
            memset(&priv->m_aTextSelectionEnd, 0, sizeof(priv->m_aTextSelectionEnd));
            memset(&priv->m_aHandleEndRect, 0, sizeof(priv->m_aHandleEndRect));
        }
        else
            memset(&priv->m_aHandleMiddleRect, 0, sizeof(priv->m_aHandleMiddleRect));
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
    case LOK_CALLBACK_GRAPHIC_SELECTION:
    {
        if (pCallback->m_aPayload != "EMPTY")
            priv->m_aGraphicSelection = payloadToRectangle(pDocView, pCallback->m_aPayload.c_str());
        else
            memset(&priv->m_aGraphicSelection, 0, sizeof(priv->m_aGraphicSelection));
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
    }
    break;
    case LOK_CALLBACK_SET_PART:
    {
        setPart(pDocView, pCallback->m_aPayload);
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
callbackWorker (int nType, const char* pPayload, void* pData)
{
    LOKDocView* pDocView = LOK_DOC_VIEW (pData);

    CallbackData* pCallback = new CallbackData(nType, pPayload ? pPayload : "(nil)", pDocView);
    g_info("lok_doc_view_callbackWorker: %s, '%s'", callbackTypeToString(nType), pPayload);
    gdk_threads_add_idle(callback, pCallback);
}

static void
renderHandle(LOKDocView* pDocView,
             cairo_t* pCairo,
             const GdkRectangle& rCursor,
             cairo_surface_t* pHandle,
             GdkRectangle& rRectangle)
{
    LOKDocViewPrivate* priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));
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

/// Renders pHandle around an rSelection rectangle on pCairo.
static void
renderGraphicHandle(LOKDocView* pDocView,
                    cairo_t* pCairo,
                    const GdkRectangle& rSelection,
                    cairo_surface_t* pHandle)
{
    LOKDocViewPrivate* priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));
    int nHandleWidth, nHandleHeight;
    GdkRectangle aSelection;

    nHandleWidth = cairo_image_surface_get_width(pHandle);
    nHandleHeight = cairo_image_surface_get_height(pHandle);

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

        cairo_save (pCairo);
        cairo_translate(pCairo, x, y);
        cairo_set_source_surface(pCairo, pHandle, 0, 0);
        cairo_paint(pCairo);
        cairo_restore (pCairo);
    }
}


static gboolean
renderDocument(LOKDocView* pDocView, cairo_t* pCairo)
{
    LOKDocViewPrivate *priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));
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

            if (!gdk_rectangle_intersect(&aVisibleArea, &aTileRectangleTwips, 0))
                bPaint = false;

            if (bPaint)
            {
                Tile& currentTile = priv->m_aTileBuffer.getTile(nRow, nColumn, priv->m_fZoom);
                GdkPixbuf* pPixBuf = currentTile.getBuffer();
                gdk_cairo_set_source_pixbuf (pCairo, pPixBuf,
                                             twipToPixel(aTileRectangleTwips.x, priv->m_fZoom),
                                             twipToPixel(aTileRectangleTwips.y, priv->m_fZoom));
                cairo_paint(pCairo);
            }
        }
    }

    return FALSE;
}

static gboolean
renderOverlay(LOKDocView* pDocView, cairo_t* pCairo)
{
    LOKDocViewPrivate *priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));

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

    if (priv->m_bEdit && priv->m_bCursorVisible && !isEmptyRectangle(priv->m_aVisibleCursor) && priv->m_aTextSelectionRectangles.empty())
    {
        // Have a cursor, but no selection: we need the middle handle.
        gchar* handleMiddlePath = g_strconcat (priv->m_aLOPath, "/../..", CURSOR_HANDLE_DIR, "handle_middle.png", NULL);
        if (!priv->m_pHandleMiddle)
            priv->m_pHandleMiddle = cairo_image_surface_create_from_png(handleMiddlePath);
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
            gchar* handleStartPath = g_strconcat (priv->m_aLOPath, "/../..", CURSOR_HANDLE_DIR, "handle_start.png", NULL);
            if (!priv->m_pHandleStart)
                priv->m_pHandleStart = cairo_image_surface_create_from_png(handleStartPath);
            renderHandle(pDocView, pCairo, priv->m_aTextSelectionStart, priv->m_pHandleStart, priv->m_aHandleStartRect);
            g_free (handleStartPath);
        }
        if (!isEmptyRectangle(priv->m_aTextSelectionEnd))
        {
            // Have a start position: we need an end handle.
            gchar* handleEndPath = g_strconcat (priv->m_aLOPath, "/../..", CURSOR_HANDLE_DIR, "handle_end.png", NULL);
            if (!priv->m_pHandleEnd)
                priv->m_pHandleEnd = cairo_image_surface_create_from_png(handleEndPath);
            renderHandle(pDocView, pCairo, priv->m_aTextSelectionEnd, priv->m_pHandleEnd, priv->m_aHandleEndRect);
            g_free (handleEndPath);
        }
    }

    if (!isEmptyRectangle(priv->m_aGraphicSelection))
    {
        gchar* handleGraphicPath = g_strconcat (priv->m_aLOPath, "/../..", CURSOR_HANDLE_DIR, "handle_graphic.png", NULL);
        if (!priv->m_pGraphicHandle)
            priv->m_pGraphicHandle = cairo_image_surface_create_from_png(handleGraphicPath);
        renderGraphicHandle(pDocView, pCairo, priv->m_aGraphicSelection, priv->m_pGraphicHandle);
        g_free (handleGraphicPath);
    }

    return FALSE;
}

static gboolean
lok_doc_view_signal_button(GtkWidget* pWidget, GdkEventButton* pEvent)
{
    LOKDocView* pDocView = LOK_DOC_VIEW (pWidget);
    LOKDocViewPrivate *priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));

    g_info("LOKDocView_Impl::signalButton: %d, %d (in twips: %d, %d)",
           (int)pEvent->x, (int)pEvent->y,
           (int)pixelToTwip(pEvent->x, priv->m_fZoom),
           (int)pixelToTwip(pEvent->y, priv->m_fZoom));

    if (pEvent->type == GDK_BUTTON_RELEASE)
    {
        if (priv->m_bInDragStartHandle)
        {
            g_info("LOKDocView_Impl::signalButton: end of drag start handle");
            priv->m_bInDragStartHandle = false;
            return FALSE;
        }
        else if (priv->m_bInDragMiddleHandle)
        {
            g_info("LOKDocView_Impl::signalButton: end of drag middle handle");
            priv->m_bInDragMiddleHandle = false;
            return FALSE;
        }
        else if (priv->m_bInDragEndHandle)
        {
            g_info("LOKDocView_Impl::signalButton: end of drag end handle");
            priv->m_bInDragEndHandle = false;
            return FALSE;
        }

        for (int i = 0; i < GRAPHIC_HANDLE_COUNT; ++i)
        {
            if (priv->m_bInDragGraphicHandles[i])
            {
                g_info("LOKDocView_Impl::signalButton: end of drag graphic handle #%d", i);
                priv->m_bInDragGraphicHandles[i] = false;
                priv->m_pDocument->pClass->setGraphicSelection(priv->m_pDocument, LOK_SETGRAPHICSELECTION_END, pixelToTwip(pEvent->x, priv->m_fZoom), pixelToTwip(pEvent->y, priv->m_fZoom));
                return FALSE;
            }
        }

        if (priv->m_bInDragGraphicSelection)
        {
            g_info("LOKDocView_Impl::signalButton: end of drag graphic selection");
            priv->m_bInDragGraphicSelection = false;
            priv->m_pDocument->pClass->setGraphicSelection(priv->m_pDocument, LOK_SETGRAPHICSELECTION_END, pixelToTwip(pEvent->x, priv->m_fZoom), pixelToTwip(pEvent->y, priv->m_fZoom));
            return FALSE;
        }
    }

    if (priv->m_bEdit)
    {
        GdkRectangle aClick;
        aClick.x = pEvent->x;
        aClick.y = pEvent->y;
        aClick.width = 1;
        aClick.height = 1;
        if (pEvent->type == GDK_BUTTON_PRESS)
        {
            if (gdk_rectangle_intersect(&aClick, &priv->m_aHandleStartRect, NULL))
            {
                g_info("LOKDocView_Impl::signalButton: start of drag start handle");
                priv->m_bInDragStartHandle = true;
                return FALSE;
            }
            else if (gdk_rectangle_intersect(&aClick, &priv->m_aHandleMiddleRect, NULL))
            {
                g_info("LOKDocView_Impl::signalButton: start of drag middle handle");
                priv->m_bInDragMiddleHandle = true;
                return FALSE;
            }
            else if (gdk_rectangle_intersect(&aClick, &priv->m_aHandleEndRect, NULL))
            {
                g_info("LOKDocView_Impl::signalButton: start of drag end handle");
                priv->m_bInDragEndHandle = true;
                return FALSE;
            }

            for (int i = 0; i < GRAPHIC_HANDLE_COUNT; ++i)
            {
                if (gdk_rectangle_intersect(&aClick, &priv->m_aGraphicHandleRects[i], NULL))
                {
                    g_info("LOKDocView_Impl::signalButton: start of drag graphic handle #%d", i);
                    priv->m_bInDragGraphicHandles[i] = true;
                    priv->m_pDocument->pClass->setGraphicSelection(priv->m_pDocument,
                                                             LOK_SETGRAPHICSELECTION_START,
                                                             pixelToTwip(priv->m_aGraphicHandleRects[i].x + priv->m_aGraphicHandleRects[i].width / 2, priv->m_fZoom),
                                                             pixelToTwip(priv->m_aGraphicHandleRects[i].y + priv->m_aGraphicHandleRects[i].height / 2, priv->m_fZoom));
                    return FALSE;
                }
            }
        }
    }

    if (!priv->m_bEdit)
        lok_doc_view_set_edit(pDocView, TRUE);

    switch (pEvent->type)
    {
    case GDK_BUTTON_PRESS:
    {
        int nCount = 1;
        if ((pEvent->time - priv->m_nLastButtonPressTime) < 250)
            nCount++;
        priv->m_nLastButtonPressTime = pEvent->time;
        priv->m_pDocument->pClass->postMouseEvent(priv->m_pDocument, LOK_MOUSEEVENT_MOUSEBUTTONDOWN, pixelToTwip(pEvent->x, priv->m_fZoom), pixelToTwip(pEvent->y, priv->m_fZoom), nCount);
        break;
    }
    case GDK_BUTTON_RELEASE:
    {
        int nCount = 1;
        if ((pEvent->time - priv->m_nLastButtonReleaseTime) < 250)
            nCount++;
        priv->m_nLastButtonReleaseTime = pEvent->time;
        priv->m_pDocument->pClass->postMouseEvent(priv->m_pDocument, LOK_MOUSEEVENT_MOUSEBUTTONUP, pixelToTwip(pEvent->x, priv->m_fZoom), pixelToTwip(pEvent->y, priv->m_fZoom), nCount);
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
    LOKDocViewPrivate *priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));
    GdkPoint aPoint;

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
    if (gdk_rectangle_intersect(&aMotionInTwipsInTwips, &priv->m_aGraphicSelection, 0))
    {
        g_info("lcl_signalMotion: start of drag graphic selection");
        priv->m_bInDragGraphicSelection = true;
        priv->m_pDocument->pClass->setGraphicSelection(priv->m_pDocument, LOK_SETGRAPHICSELECTION_START, pixelToTwip(pEvent->x, priv->m_fZoom), pixelToTwip(pEvent->y, priv->m_fZoom));
        return FALSE;
    }

    // Otherwise a mouse move, as on the desktop.
    priv->m_pDocument->pClass->postMouseEvent(priv->m_pDocument, LOK_MOUSEEVENT_MOUSEMOVE, pixelToTwip(pEvent->x, priv->m_fZoom), pixelToTwip(pEvent->y, priv->m_fZoom), 1);

    return FALSE;
}

static void lok_doc_view_init (LOKDocView* pDocView)
{
    LOKDocViewPrivate *priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));
    priv->m_bCursorVisible = true;

    gtk_widget_add_events(GTK_WIDGET(pDocView),
                          GDK_BUTTON_PRESS_MASK
                          |GDK_BUTTON_RELEASE_MASK
                          |GDK_BUTTON_MOTION_MASK
                          |GDK_KEY_PRESS_MASK
                          |GDK_KEY_RELEASE_MASK);
}

static void lok_doc_view_set_property (GObject* object, guint propId, const GValue *value, GParamSpec *pspec)
{
    LOKDocView* pDocView = LOK_DOC_VIEW (object);
    LOKDocViewPrivate* priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));

    switch (propId)
    {
    case PROP_LO_PATH:
        priv->m_aLOPath = g_value_dup_string (value);
        break;
    case PROP_DOC_PATH:
        priv->m_aDocPath = g_value_dup_string (value);
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
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, propId, pspec);
    }
}

static void lok_doc_view_get_property (GObject* object, guint propId, GValue *value, GParamSpec *pspec)
{
    LOKDocView* pDocView = LOK_DOC_VIEW (object);
    LOKDocViewPrivate* priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));

    switch (propId)
    {
    case PROP_LO_PATH:
        g_value_set_string (value, priv->m_aLOPath);
        break;
    case PROP_DOC_PATH:
        g_value_set_string (value, priv->m_aDocPath);
        break;
    case PROP_EDITABLE:
        g_value_set_boolean (value, priv->m_bEdit);
        break;
    case PROP_LOAD_PROGRESS:
        g_value_set_uint (value, priv->m_nLoadProgress);
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
    LOKDocViewPrivate* priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));

    if (priv->m_pDocument)
        priv->m_pDocument->pClass->destroy (priv->m_pDocument);
    if (priv->m_pOffice)
        priv->m_pOffice->pClass->destroy (priv->m_pOffice);

    G_OBJECT_CLASS (lok_doc_view_parent_class)->finalize (object);
}

static gboolean lok_doc_view_initable_init (GInitable *initable, GCancellable* /*cancellable*/, GError **error)
{
    LOKDocView *pDocView = LOK_DOC_VIEW (initable);
    LOKDocViewPrivate* priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));

    if (priv->m_pOffice != NULL)
        return TRUE;

    priv->m_pOffice = lok_init (priv->m_aLOPath);

    if (priv->m_pOffice == NULL)
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
    g_object_class_install_property (pGObjectClass,
          PROP_LO_PATH,
          g_param_spec_string("lopath",
                              "LO Path",
                              "LibreOffice Install Path",
                              0,
                              static_cast<GParamFlags>(G_PARAM_READWRITE
                                                       | G_PARAM_CONSTRUCT_ONLY)));

    /**
     * LOKDocView:docpath:
     *
     * The path of the document that is currently being viewed.
     */
    g_object_class_install_property (pGObjectClass,
          PROP_DOC_PATH,
          g_param_spec_string("docpath",
                              "Document Path",
                              "The URI of the document to open",
                              0,
                              static_cast<GParamFlags>(G_PARAM_READWRITE)));

    /**
     * LOKDocView:editable:
     *
     * Whether the document loaded inside of #LOKDocView is editable or not.
     */
    g_object_class_install_property (pGObjectClass,
          PROP_EDITABLE,
          g_param_spec_boolean("editable",
                               "Editable",
                               "Whether the content is in edit mode or not",
                               FALSE,
                               static_cast<GParamFlags>(G_PARAM_READWRITE)));

    /**
     * LOKDocView:load-progress:
     *
     * The percent completion of the current loading operation of the
     * document. This can be used for progress bars. Note that this is not a
     * very accurate progress indicator, and its value might reset it couple of
     * times to 0 and start again. You should not rely on its numbers.
     */
    g_object_class_install_property (pGObjectClass,
          PROP_LOAD_PROGRESS,
          g_param_spec_int("load-progress",
                           "Estimated Load Progress",
                           "Whether the content is in edit mode or not",
                           0, 100, 0,
                           G_PARAM_READABLE));

    /**
     * LOKDocView:zoom-level:
     *
     * The current zoom level of the document loaded inside #LOKDocView. The
     * default value is 1.0.
     */
    g_object_class_install_property (pGObjectClass,
          PROP_ZOOM,
          g_param_spec_float("zoom-level",
                             "Zoom Level",
                             "The current zoom level of the content",
                             0, 5.0, 1.0,
                             static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT)));

    /**
     * LOKDocView:is-loading:
     *
     * Whether the requested document is being loaded or not. %TRUE if it is
     * being loaded, otherwise %FALSE.
     */
    g_object_class_install_property (pGObjectClass,
          PROP_IS_LOADING,
          g_param_spec_boolean("is-loading",
                               "Is Loading",
                               "Whether the view is loading a document",
                               FALSE,
                               static_cast<GParamFlags>(G_PARAM_READABLE)));

    /**
     * LOKDocView:doc-width:
     *
     * The width of the currently loaded document in #LOKDocView in twips.
     */
    g_object_class_install_property (pGObjectClass,
          PROP_DOC_WIDTH,
          g_param_spec_long("doc-width",
                            "Document Width",
                            "Width of the document in twips",
                            0, G_MAXLONG, 0,
                            static_cast<GParamFlags>(G_PARAM_READWRITE)));

    /**
     * LOKDocView:doc-height:
     *
     * The height of the currently loaded document in #LOKDocView in twips.
     */
    g_object_class_install_property (pGObjectClass,
          PROP_DOC_HEIGHT,
          g_param_spec_long("doc-height",
                            "Document Height",
                            "Height of the document in twips",
                            0, G_MAXLONG, 0,
                            static_cast<GParamFlags>(G_PARAM_READWRITE)));

    /**
     * LOKDocView:can-zoom-in:
     *
     * It tells whether the view can further be zoomed in or not.
     */
    g_object_class_install_property (pGObjectClass,
          PROP_CAN_ZOOM_IN,
          g_param_spec_boolean("can-zoom-in",
                               "Can Zoom In",
                               "Whether the view can be zoomed in further",
                               TRUE,
                               static_cast<GParamFlags>(G_PARAM_READABLE
                                                       | G_PARAM_STATIC_STRINGS)));

    /**
     * LOKDocView:can-zoom-out:
     *
     * It tells whether the view can further be zoomed out or not.
     */
    g_object_class_install_property (pGObjectClass,
          PROP_CAN_ZOOM_OUT,
          g_param_spec_boolean("can-zoom-out",
                               "Can Zoom Out",
                               "Whether the view can be zoomed out further",
                               TRUE,
                               static_cast<GParamFlags>(G_PARAM_READABLE
                                                       | G_PARAM_STATIC_STRINGS)));

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
                     NULL, NULL,
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
                     NULL, NULL,
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
                     NULL, NULL,
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
                     NULL, NULL,
                     g_cclosure_marshal_VOID__INT,
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
                     NULL, NULL,
                     g_cclosure_marshal_VOID__STRING,
                     G_TYPE_NONE, 1,
                     G_TYPE_STRING);
}

/**
 * lok_doc_view_new:
 * @pPath: LibreOffice install path.
 * @cancellable: The cancellable object that you can use to cancel this
 * operation.
 * @error: The error that will be set if the object fails to initialize.
 *
 * Returns: (transfer none): The #LOKDocView widget instance.
 */
SAL_DLLPUBLIC_EXPORT GtkWidget*
lok_doc_view_new (const gchar* pPath, GCancellable *cancellable, GError **error)
{
    return GTK_WIDGET (g_initable_new (LOK_TYPE_DOC_VIEW, cancellable, error, "lopath", pPath, NULL));
}

/**
 * lok_doc_view_open_document:
 * @pDocView: The #LOKDocView instance
 * @pPath: The path of the document that #LOKDocView widget should try to open
 *
 * Returns: %TRUE if the document is loaded succesfully, %FALSE otherwise
 */
SAL_DLLPUBLIC_EXPORT gboolean
lok_doc_view_open_document (LOKDocView* pDocView, const gchar* pPath)
{
    LOKDocViewPrivate *priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));

    if ( priv->m_pDocument )
    {
        priv->m_pDocument->pClass->destroy( priv->m_pDocument );
        priv->m_pDocument = 0;
    }

    priv->m_pOffice->pClass->registerCallback(priv->m_pOffice, globalCallbackWorker, pDocView);
    priv->m_pDocument = priv->m_pOffice->pClass->documentLoad( priv->m_pOffice, pPath );
    if ( !priv->m_pDocument )
    {
        // FIXME: should have a GError parameter and populate it.
        char *pError = priv->m_pOffice->pClass->getError( priv->m_pOffice );
        fprintf( stderr, "Error opening document '%s'\n", pError );
        return FALSE;
    }
    else
    {
        priv->m_pDocument->pClass->initializeForRendering(priv->m_pDocument);
        priv->m_pDocument->pClass->registerCallback(priv->m_pDocument, callbackWorker, pDocView);
        priv->m_pDocument->pClass->getDocumentSize(priv->m_pDocument, &priv->m_nDocumentWidthTwips, &priv->m_nDocumentHeightTwips);
        g_timeout_add(600, handleTimeout, pDocView);

        float zoom = priv->m_fZoom;
        long nDocumentWidthTwips = priv->m_nDocumentWidthTwips;
        long nDocumentHeightTwips = priv->m_nDocumentHeightTwips;
        long nDocumentWidthPixels = twipToPixel(nDocumentWidthTwips, zoom);
        long nDocumentHeightPixels = twipToPixel(nDocumentHeightTwips, zoom);
        // Total number of columns in this document.
        guint nColumns = ceil((double)nDocumentWidthPixels / nTileSizePixels);


        priv->m_aTileBuffer = TileBuffer(priv->m_pDocument,
                                         nColumns);
        gtk_widget_set_size_request(GTK_WIDGET(pDocView),
                                    nDocumentWidthPixels,
                                    nDocumentHeightPixels);
        gtk_widget_set_can_focus(GTK_WIDGET(pDocView), TRUE);
        gtk_widget_grab_focus(GTK_WIDGET(pDocView));
    }
    return TRUE;
}

/**
 * lok_doc_view_get_document:
 * @pDocView: The #LOKDocView instance
 *
 * Returns: The #LibreOfficeKitDocument instance the widget is currently showing
 */
SAL_DLLPUBLIC_EXPORT LibreOfficeKitDocument*
lok_doc_view_get_document (LOKDocView* pDocView)
{
    LOKDocViewPrivate *priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));
    return priv->m_pDocument;
}

/**
 * lok_doc_view_set_zoom:
 * @pDocView: The #LOKDocView instance
 * @fZoom: The new zoom level that pDocView must set it into.
 *
 * Sets the new zoom level for the widget.
 */
SAL_DLLPUBLIC_EXPORT void
lok_doc_view_set_zoom (LOKDocView* pDocView, float fZoom)
{
    LOKDocViewPrivate *priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));

    priv->m_fZoom = fZoom;
    long nDocumentWidthPixels = twipToPixel(priv->m_nDocumentWidthTwips, fZoom);
    long nDocumentHeightPixels = twipToPixel(priv->m_nDocumentHeightTwips, fZoom);
    // Total number of columns in this document.
    guint nColumns = ceil((double)nDocumentWidthPixels / nTileSizePixels);

    priv->m_aTileBuffer = TileBuffer(priv->m_pDocument,
                                     nColumns);
    gtk_widget_set_size_request(GTK_WIDGET(pDocView),
                                nDocumentWidthPixels,
                                nDocumentHeightPixels);
}

/**
 * lok_doc_view_get_zoom:
 * @pDocView: The #LOKDocView instance
 *
 * Returns: The current zoom factor value in float for pDocView
 */
SAL_DLLPUBLIC_EXPORT float
lok_doc_view_get_zoom (LOKDocView* pDocView)
{
    LOKDocViewPrivate *priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));
    return priv->m_fZoom;
}

SAL_DLLPUBLIC_EXPORT int
lok_doc_view_get_parts (LOKDocView* pDocView)
{
    LOKDocViewPrivate *priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));
    return priv->m_pDocument->pClass->getParts( priv->m_pDocument );
}

SAL_DLLPUBLIC_EXPORT int
lok_doc_view_get_part (LOKDocView* pDocView)
{
    LOKDocViewPrivate *priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));
    return priv->m_pDocument->pClass->getPart( priv->m_pDocument );
}

SAL_DLLPUBLIC_EXPORT void
lok_doc_view_set_part (LOKDocView* pDocView, int nPart)
{
    LOKDocViewPrivate *priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));
    priv->m_pDocument->pClass->setPart( priv->m_pDocument, nPart );
}

SAL_DLLPUBLIC_EXPORT char*
lok_doc_view_get_part_name (LOKDocView* pDocView, int nPart)
{
    LOKDocViewPrivate *priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));
    return priv->m_pDocument->pClass->getPartName( priv->m_pDocument, nPart );
}

SAL_DLLPUBLIC_EXPORT void
lok_doc_view_set_partmode(LOKDocView* pDocView,
                          int nPartMode)
{
    LOKDocViewPrivate *priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));
    priv->m_pDocument->pClass->setPartMode( priv->m_pDocument, nPartMode );
}

/**
 * lok_doc_view_set_edit:
 * @pDocView: The #LOKDocView instance
 * @bEdit: %TRUE if the pDocView should go in edit mode, %FALSE otherwise
 *
 * Sets the edit-mode for pDocView
 */
SAL_DLLPUBLIC_EXPORT void
lok_doc_view_set_edit(LOKDocView* pDocView,
                      gboolean bEdit)
{
    LOKDocViewPrivate *priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));
    gboolean bWasEdit = priv->m_bEdit;

    if (!priv->m_bEdit && bEdit)
        g_info("lok_doc_view_set_edit: entering edit mode");
    else if (priv->m_bEdit && !bEdit)
    {
        g_info("lok_doc_view_set_edit: leaving edit mode");
        priv->m_pDocument->pClass->resetSelection(priv->m_pDocument);
    }
    priv->m_bEdit = bEdit;
    g_signal_emit(pDocView, doc_view_signals[EDIT_CHANGED], 0, bWasEdit);
    gtk_widget_queue_draw(GTK_WIDGET(pDocView));
}

/**
 * lok_doc_view_get_edit:
 * @pDocView: The #LOKDocView instance
 *
 * Returns: %TRUE if the given pDocView is in edit mode.
 */
SAL_DLLPUBLIC_EXPORT gboolean
lok_doc_view_get_edit (LOKDocView* pDocView)
{
    LOKDocViewPrivate *priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));
    return priv->m_bEdit;
}

/**
 * lok_doc_view_post_command:
 * @pDocView: the #LOKDocView instance
 * @pCommand: the command to issue to LO core
 * @pArguments: the arguments to the given command
 *
 * This methods forwards your command to LO core.
*/
SAL_DLLPUBLIC_EXPORT void
lok_doc_view_post_command (LOKDocView* pDocView,
                           const char* pCommand,
                           const char* pArguments)
{
    LOKDocViewPrivate *priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));
    priv->m_pDocument->pClass->postUnoCommand(priv->m_pDocument, pCommand, pArguments);
}

/**
 * lok_doc_view_pixel_to_twip:
 * @pDocView: The #LOKDocView instance
 * @fInput: The value in pixels to convert to twips
 *
 * Converts the value in pixels to twips according to zoom level.
 *
 * Returns: The corresponding value in twips
 */
SAL_DLLPUBLIC_EXPORT float
lok_doc_view_pixel_to_twip (LOKDocView* pDocView, float fInput)
{
    LOKDocViewPrivate *priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));
    return pixelToTwip(fInput, priv->m_fZoom);
}

/**
 * lok_doc_view_twip_to_pixel:
 * @pDocView: The #LOKDocView instance
 * @fInput: The value in twips to convert to pixels
 *
 * Converts the value in twips to pixels according to zoom level.
 *
 * Returns: The corresponding value in pixels
 */
SAL_DLLPUBLIC_EXPORT float
lok_doc_view_twip_to_pixel (LOKDocView* pDocView, float fInput)
{
    LOKDocViewPrivate *priv = static_cast<LOKDocViewPrivate*>(lok_doc_view_get_instance_private (pDocView));
    return twipToPixel(fInput, priv->m_fZoom);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
