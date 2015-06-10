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
#define CURSOR_HANDLE_DIR "android/source/res/drawable/"
// Number of handles around a graphic selection.
#define GRAPHIC_HANDLE_COUNT 8

/// Holds data used by LOKDocView only.
struct LOKDocView_Impl
{
    LOKDocView* m_pDocView;
    TileBuffer m_aTileBuffer;

    float m_fZoom;

    LibreOfficeKit* m_pOffice;
    LibreOfficeKitDocument* m_pDocument;
    long m_nDocumentWidthTwips;
    long m_nDocumentHeightTwips;
    /// View or edit mode.
    bool m_bEdit;
    /// Position and size of the visible cursor.
    GdkRectangle m_aVisibleCursor;
    /// Cursor overlay is visible or hidden (for blinking).
    bool m_bCursorOverlayVisible;
    /// Cursor is visible or hidden (e.g. for graphic selection).
    bool m_bCursorVisible;
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
    bool m_bInDragGraphicSelection;

    /// @name Start/middle/end handle.
    ///@{
    /// Bitmap of the text selection start handle.
    cairo_surface_t* m_pHandleStart;
    /// Rectangle of the text selection start handle, to know if the user clicked on it or not
    GdkRectangle m_aHandleStartRect;
    /// If we are in the middle of a drag of the text selection end handle.
    bool m_bInDragStartHandle;
    /// Bitmap of the text selection middle handle.
    cairo_surface_t* m_pHandleMiddle;
    /// Rectangle of the text selection middle handle, to know if the user clicked on it or not
    GdkRectangle m_aHandleMiddleRect;
    /// If we are in the middle of a drag of the text selection middle handle.
    bool m_bInDragMiddleHandle;
    /// Bitmap of the text selection end handle.
    cairo_surface_t* m_pHandleEnd;
    /// Rectangle of the text selection end handle, to know if the user clicked on it or not
    GdkRectangle m_aHandleEndRect;
    /// If we are in the middle of a drag of the text selection end handle.
    bool m_bInDragEndHandle;
    ///@}

    /// @name Graphic handles.
    ///@{
    /// Bitmap of a graphic selection handle.
    cairo_surface_t* m_pGraphicHandle;
    /// Rectangle of a graphic selection handle, to know if the user clicked on it or not.
    GdkRectangle m_aGraphicHandleRects[8];
    /// If we are in the middle of a drag of a graphic selection handle.
    bool m_bInDragGraphicHandles[8];
    ///@}

    /// Callback data, allocated in lok_doc_view_callback_worker(), released in lok_doc_view_callback().
    struct CallbackData
    {
        int m_nType;
        std::string m_aPayload;
        LOKDocView* m_pDocView;

        CallbackData(int nType, const std::string& rPayload, LOKDocView* pDocView);
    };


    LOKDocView_Impl(LOKDocView* pDocView);
    ~LOKDocView_Impl();
    /// Connected to the destroy signal of LOKDocView, deletes its LOKDocView_Impl.
    static void destroy(LOKDocView* pDocView, gpointer pData);
    /// Connected to the expose-event of the GtkDrawingArea
    static void onExposed(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
    /// Receives a key press or release event.
    void signalKey(GdkEventKey* pEvent);
    /*
     * The user drags the handle, which is below the cursor, but wants to move the
     * cursor accordingly.
     *
     * @param pHandle the rectangle of the handle
     * @param pEvent the motion event
     * @param pPoint the computed point (output parameter)
     */
    static void getDragPoint(GdkRectangle* pHandle, GdkEventButton* pEvent, GdkPoint* pPoint);
    /// Receives a button press event.
    static gboolean signalButton(GtkWidget* pEventBox, GdkEventButton* pEvent, LOKDocView* pDocView);
    /// Implementation of button press event handler, invoked by signalButton().
    gboolean signalButtonImpl(GdkEventButton* pEvent);
    /// Receives a motion event.
    static gboolean signalMotion(GtkWidget* pEventBox, GdkEventButton* pEvent, LOKDocView* pDocView);
    /// Implementation of motion event handler, invoked by signalMotion().
    gboolean signalMotionImpl(GdkEventButton* pEvent);
    /// Receives an expose event.
    static gboolean renderOverlay(GtkWidget* pWidget, GdkEventExpose* pEvent, LOKDocView* pDocView);
    /// Implementation of expose event handler (renders cursor and selection overlay), invoked by renderOverlay().
    gboolean renderOverlayImpl(GtkWidget* pEventBox);
    /// Is rRectangle empty?
    static bool isEmptyRectangle(const GdkRectangle& rRectangle);
    /*
     * Renders pHandle below an rCursor rectangle on pCairo.
     * @param rRectangle output parameter, the rectangle that contains the rendered handle.
     */
    void renderHandle(cairo_t* pCairo, const GdkRectangle& rCursor, cairo_surface_t* pHandle, GdkRectangle& rRectangle);
    /// Renders pHandle around an rSelection rectangle on pCairo.
    void renderGraphicHandle(cairo_t* pCairo, const GdkRectangle& rSelection, cairo_surface_t* pHandle);
    /// Takes care of the blinking cursor.
    static gboolean handleTimeout(gpointer pData);
    /// Implementation of the timeout handler, invoked by handleTimeout().
    gboolean handleTimeoutImpl();
    /// Implementation of expose event handler, invoked by onExposed().
    void onExposedImpl(GdkEventExpose* event);
    /// Returns the GdkRectangle of a x,y,width,height string.
    GdkRectangle payloadToRectangle(const char* pPayload);
    /// Returns the GdkRectangles of a x1,y1,w1,h1;x2,y2,w2,h2;... string.
    std::vector<GdkRectangle> payloadToRectangles(const char* pPayload);
    /// Returns the string representation of a LibreOfficeKitCallbackType enumeration element.
    static const char* callbackTypeToString(int nType);
    /// Invoked on the main thread if callbackWorker() requests so.
    static gboolean callback(gpointer pData);
    /// Invoked on the main thread if globalCallbackWorker() requests so.
    static gboolean globalCallback(gpointer pData);
    /// Implementation of the callback handler, invoked by callback();
    gboolean callbackImpl(CallbackData* pCallbackData);
    /// Our LOK callback, runs on the LO thread.
    static void callbackWorker(int nType, const char* pPayload, void* pData);
    /// Implementation of the callback worder handler, invoked by callbackWorker().
    void callbackWorkerImpl(int nType, const char* pPayload);
    /// Our global LOK callback, runs on the LO thread.
    static void globalCallbackWorker(int nType, const char* pPayload, void* pData);
    /// Implementation of the global callback worder handler, invoked by globalCallbackWorker().
    void globalCallbackWorkerImpl(int nType, const char* pPayload);
    /// Command state (various buttons like bold are toggled or not) is changed.
    void commandChanged(const std::string& rPayload);
    /// Search did not find any matches.
    void searchNotFound(const std::string& rPayload);
    /// LOK decided to change parts, need to update UI.
    void setPart(const std::string& rPayload);
    /// Sets the tiles enclosed by rRectangle as invalid in m_aTileBuffer
    void setTilesInvalid(const GdkRectangle& rRectangle);
};

enum
{
    EDIT_CHANGED,
    COMMAND_CHANGED,
    SEARCH_NOT_FOUND,
    PART_CHANGED,
    LAST_SIGNAL
};


static guint doc_view_signals[LAST_SIGNAL] = { 0 };

SAL_DLLPUBLIC_EXPORT GType lok_doc_view_get_type();
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
G_DEFINE_TYPE(LOKDocView, lok_doc_view, GTK_TYPE_DRAWING_AREA)
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

namespace {

/// Sets rWidth and rHeight from a "width, height" string.
void payloadToSize(const char* pPayload, long& rWidth, long& rHeight)
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

}



namespace {

/// Implementation of the global callback handler, invoked by globalCallback();
gboolean globalCallbackImpl(LOKDocView_Impl::CallbackData* pCallback)
{
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

}

LOKDocView_Impl::CallbackData::CallbackData(int nType, const std::string& rPayload, LOKDocView* pDocView)
    : m_nType(nType),
    m_aPayload(rPayload),
    m_pDocView(pDocView)
{
}

LOKDocView_Impl::LOKDocView_Impl(LOKDocView* pDocView)
    : m_pDocView(pDocView),
      m_aTileBuffer(TileBuffer(0,0)),
      m_fZoom(1),
      m_pOffice(0),
      m_pDocument(0),
      m_nDocumentWidthTwips(0),
      m_nDocumentHeightTwips(0),
      m_bEdit(false),
      m_aVisibleCursor({0, 0, 0, 0}),
      m_bCursorOverlayVisible(false),
      m_bCursorVisible(true),
      m_nLastButtonPressTime(0),
      m_nLastButtonReleaseTime(0),
      m_aTextSelectionStart({0, 0, 0, 0}),
      m_aTextSelectionEnd({0, 0, 0, 0}),
      m_aGraphicSelection({0, 0, 0, 0}),
      m_bInDragGraphicSelection(false),

      // Start/middle/end handle.
      m_pHandleStart(0),
      m_aHandleStartRect({0, 0, 0, 0}),
      m_bInDragStartHandle(false),
      m_pHandleMiddle(0),
      m_aHandleMiddleRect({0, 0, 0, 0}),
      m_bInDragMiddleHandle(false),
      m_pHandleEnd(0),
      m_aHandleEndRect({0, 0, 0, 0}),
      m_bInDragEndHandle(false),

      m_pGraphicHandle(0)
{
    memset(&m_aGraphicHandleRects, 0, sizeof(m_aGraphicHandleRects));
    memset(&m_bInDragGraphicHandles, 0, sizeof(m_bInDragGraphicHandles));
}

LOKDocView_Impl::~LOKDocView_Impl()
{
    if (m_pDocument)
        m_pDocument->pClass->destroy(m_pDocument);
    if (m_pOffice)
        m_pOffice->pClass->destroy(m_pOffice);
    m_pDocument = 0;
    m_pOffice = 0;
}

void LOKDocView_Impl::destroy(LOKDocView* pDocView, gpointer /*pData*/)
{
    // We specifically need to destroy the document when closing in order to ensure
    // that lock files etc. are cleaned up.
    delete pDocView->m_pImpl;
}

void LOKDocView_Impl::onExposed(GtkWidget* /*widget*/, GdkEventExpose* event, gpointer userdata)
{
    LOKDocView *pDocView = LOK_DOC_VIEW (userdata);
    pDocView->m_pImpl->onExposedImpl(event);
}

void LOKDocView_Impl::onExposedImpl(GdkEventExpose* event)
{
    long nDocumentWidthPixels = twipToPixel(m_nDocumentWidthTwips, m_fZoom);
    long nDocumentHeightPixels = twipToPixel(m_nDocumentHeightTwips, m_fZoom);
    // Total number of rows / columns in this document.
    guint nRows = ceil((double)nDocumentHeightPixels / nTileSizePixels);
    guint nColumns = ceil((double)nDocumentWidthPixels / nTileSizePixels);
    GdkRectangle aVisibleArea = event->area;
    cairo_t *pcairo = gdk_cairo_create(gtk_widget_get_window(GTK_WIDGET(m_pDocView)));

    aVisibleArea.x = pixelToTwip (aVisibleArea.x, m_fZoom);
    aVisibleArea.y = pixelToTwip (aVisibleArea.y, m_fZoom);
    aVisibleArea.width = pixelToTwip (aVisibleArea.width, m_fZoom);
    aVisibleArea.height = pixelToTwip (aVisibleArea.height, m_fZoom);

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
            aTileRectangleTwips.x = pixelToTwip(nTileSizePixels, m_fZoom) * nColumn;
            aTileRectangleTwips.y = pixelToTwip(nTileSizePixels, m_fZoom) * nRow;
            aTileRectangleTwips.width = pixelToTwip(aTileRectanglePixels.width, m_fZoom);
            aTileRectangleTwips.height = pixelToTwip(aTileRectanglePixels.height, m_fZoom);

            if (!gdk_rectangle_intersect(&aVisibleArea, &aTileRectangleTwips, 0))
                bPaint = false;

            if (bPaint)
            {
                Tile& currentTile = m_aTileBuffer.getTile(nRow, nColumn, m_fZoom);
                GdkPixbuf* pPixBuf = currentTile.getBuffer();

                gdk_cairo_set_source_pixbuf (pcairo, pPixBuf,
                                             twipToPixel(aTileRectangleTwips.x, m_fZoom),
                                             twipToPixel(aTileRectangleTwips.y, m_fZoom));
                cairo_paint(pcairo);
            }
        }
    }

    cairo_destroy(pcairo);
}

void LOKDocView_Impl::signalKey(GdkEventKey* pEvent)
{
    int nCharCode = 0;
    int nKeyCode = 0;

    if (!m_bEdit)
    {
        g_info("signalKey: not in edit mode, ignore");
        return;
    }

    switch (pEvent->keyval)
    {
    case GDK_KEY_BackSpace:
        nKeyCode = com::sun::star::awt::Key::BACKSPACE;
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
        m_pDocument->pClass->postKeyEvent(m_pDocument, LOK_KEYEVENT_KEYUP, nCharCode, nKeyCode);
    else
        m_pDocument->pClass->postKeyEvent(m_pDocument, LOK_KEYEVENT_KEYINPUT, nCharCode, nKeyCode);
}

gboolean LOKDocView_Impl::signalButton(GtkWidget* /*pEventBox*/, GdkEventButton* pEvent, LOKDocView* pDocView)
{
    return pDocView->m_pImpl->signalButtonImpl(pEvent);
}

/// Receives a button press event.
gboolean LOKDocView_Impl::signalButtonImpl(GdkEventButton* pEvent)
{
    g_info("LOKDocView_Impl::signalButton: %d, %d (in twips: %d, %d)", (int)pEvent->x, (int)pEvent->y, (int)pixelToTwip(pEvent->x, m_fZoom), (int)pixelToTwip(pEvent->y, m_fZoom));

    if (pEvent->type == GDK_BUTTON_RELEASE)
    {
        if (m_bInDragStartHandle)
        {
            g_info("LOKDocView_Impl::signalButton: end of drag start handle");
            m_bInDragStartHandle = false;
            return FALSE;
        }
        else if (m_bInDragMiddleHandle)
        {
            g_info("LOKDocView_Impl::signalButton: end of drag middle handle");
            m_bInDragMiddleHandle = false;
            return FALSE;
        }
        else if (m_bInDragEndHandle)
        {
            g_info("LOKDocView_Impl::signalButton: end of drag end handle");
            m_bInDragEndHandle = false;
            return FALSE;
        }

        for (int i = 0; i < GRAPHIC_HANDLE_COUNT; ++i)
        {
            if (m_bInDragGraphicHandles[i])
            {
                g_info("LOKDocView_Impl::signalButton: end of drag graphic handle #%d", i);
                m_bInDragGraphicHandles[i] = false;
                m_pDocument->pClass->setGraphicSelection(m_pDocument, LOK_SETGRAPHICSELECTION_END, pixelToTwip(pEvent->x, m_fZoom), pixelToTwip(pEvent->y, m_fZoom));
                return FALSE;
            }
        }

        if (m_bInDragGraphicSelection)
        {
            g_info("LOKDocView_Impl::signalButton: end of drag graphic selection");
            m_bInDragGraphicSelection = false;
            m_pDocument->pClass->setGraphicSelection(m_pDocument, LOK_SETGRAPHICSELECTION_END, pixelToTwip(pEvent->x, m_fZoom), pixelToTwip(pEvent->y, m_fZoom));
            return FALSE;
        }
    }

    if (m_bEdit)
    {
        GdkRectangle aClick;
        aClick.x = pEvent->x;
        aClick.y = pEvent->y;
        aClick.width = 1;
        aClick.height = 1;
        if (pEvent->type == GDK_BUTTON_PRESS)
        {
            if (gdk_rectangle_intersect(&aClick, &m_aHandleStartRect, NULL))
            {
                g_info("LOKDocView_Impl::signalButton: start of drag start handle");
                m_bInDragStartHandle = true;
                return FALSE;
            }
            else if (gdk_rectangle_intersect(&aClick, &m_aHandleMiddleRect, NULL))
            {
                g_info("LOKDocView_Impl::signalButton: start of drag middle handle");
                m_bInDragMiddleHandle = true;
                return FALSE;
            }
            else if (gdk_rectangle_intersect(&aClick, &m_aHandleEndRect, NULL))
            {
                g_info("LOKDocView_Impl::signalButton: start of drag end handle");
                m_bInDragEndHandle = true;
                return FALSE;
            }

            for (int i = 0; i < GRAPHIC_HANDLE_COUNT; ++i)
            {
                if (gdk_rectangle_intersect(&aClick, &m_aGraphicHandleRects[i], NULL))
                {
                    g_info("LOKDocView_Impl::signalButton: start of drag graphic handle #%d", i);
                    m_bInDragGraphicHandles[i] = true;
                    m_pDocument->pClass->setGraphicSelection(m_pDocument,
                                                             LOK_SETGRAPHICSELECTION_START,
                                                             pixelToTwip(m_aGraphicHandleRects[i].x + m_aGraphicHandleRects[i].width / 2, m_fZoom),
                                                             pixelToTwip(m_aGraphicHandleRects[i].y + m_aGraphicHandleRects[i].height / 2, m_fZoom));
                    return FALSE;
                }
            }
        }
    }

    if (!m_bEdit)
        lok_doc_view_set_edit(m_pDocView, TRUE);

    switch (pEvent->type)
    {
    case GDK_BUTTON_PRESS:
    {
        int nCount = 1;
        if ((pEvent->time - m_nLastButtonPressTime) < 250)
            nCount++;
        m_nLastButtonPressTime = pEvent->time;
        m_pDocument->pClass->postMouseEvent(m_pDocument, LOK_MOUSEEVENT_MOUSEBUTTONDOWN, pixelToTwip(pEvent->x, m_fZoom), pixelToTwip(pEvent->y, m_fZoom), nCount);
        break;
    }
    case GDK_BUTTON_RELEASE:
    {
        int nCount = 1;
        if ((pEvent->time - m_nLastButtonReleaseTime) < 250)
            nCount++;
        m_nLastButtonReleaseTime = pEvent->time;
        m_pDocument->pClass->postMouseEvent(m_pDocument, LOK_MOUSEEVENT_MOUSEBUTTONUP, pixelToTwip(pEvent->x, m_fZoom), pixelToTwip(pEvent->y, m_fZoom), nCount);
        break;
    }
    default:
        break;
    }
    return FALSE;
}

void LOKDocView_Impl::getDragPoint(GdkRectangle* pHandle, GdkEventButton* pEvent, GdkPoint* pPoint)
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

gboolean LOKDocView_Impl::signalMotion(GtkWidget* /*pEventBox*/, GdkEventButton* pEvent, LOKDocView* pDocView)
{
    return pDocView->m_pImpl->signalMotionImpl(pEvent);
}

gboolean LOKDocView_Impl::signalMotionImpl(GdkEventButton* pEvent)
{
    GdkPoint aPoint;

    if (m_bInDragMiddleHandle)
    {
        g_info("lcl_signalMotion: dragging the middle handle");
        LOKDocView_Impl::getDragPoint(&m_aHandleMiddleRect, pEvent, &aPoint);
        m_pDocument->pClass->setTextSelection(m_pDocument, LOK_SETTEXTSELECTION_RESET, pixelToTwip(aPoint.x, m_fZoom), pixelToTwip(aPoint.y, m_fZoom));
        return FALSE;
    }
    if (m_bInDragStartHandle)
    {
        g_info("lcl_signalMotion: dragging the start handle");
        LOKDocView_Impl::getDragPoint(&m_aHandleStartRect, pEvent, &aPoint);
        m_pDocument->pClass->setTextSelection(m_pDocument, LOK_SETTEXTSELECTION_START, pixelToTwip(aPoint.x, m_fZoom), pixelToTwip(aPoint.y, m_fZoom));
        return FALSE;
    }
    if (m_bInDragEndHandle)
    {
        g_info("lcl_signalMotion: dragging the end handle");
        LOKDocView_Impl::getDragPoint(&m_aHandleEndRect, pEvent, &aPoint);
        m_pDocument->pClass->setTextSelection(m_pDocument, LOK_SETTEXTSELECTION_END, pixelToTwip(aPoint.x, m_fZoom), pixelToTwip(aPoint.y, m_fZoom));
        return FALSE;
    }
    for (int i = 0; i < GRAPHIC_HANDLE_COUNT; ++i)
    {
        if (m_bInDragGraphicHandles[i])
        {
            g_info("lcl_signalMotion: dragging the graphic handle #%d", i);
            return FALSE;
        }
    }
    if (m_bInDragGraphicSelection)
    {
        g_info("lcl_signalMotion: dragging the graphic selection");
        return FALSE;
    }

    GdkRectangle aMotionInTwipsInTwips;
    aMotionInTwipsInTwips.x = pixelToTwip(pEvent->x, m_fZoom);
    aMotionInTwipsInTwips.y = pixelToTwip(pEvent->y, m_fZoom);
    aMotionInTwipsInTwips.width = 1;
    aMotionInTwipsInTwips.height = 1;
    if (gdk_rectangle_intersect(&aMotionInTwipsInTwips, &m_aGraphicSelection, 0))
    {
        g_info("lcl_signalMotion: start of drag graphic selection");
        m_bInDragGraphicSelection = true;
        m_pDocument->pClass->setGraphicSelection(m_pDocument, LOK_SETGRAPHICSELECTION_START, pixelToTwip(pEvent->x, m_fZoom), pixelToTwip(pEvent->y, m_fZoom));
        return FALSE;
    }

    // Otherwise a mouse move, as on the desktop.
    m_pDocument->pClass->postMouseEvent(m_pDocument, LOK_MOUSEEVENT_MOUSEMOVE, pixelToTwip(pEvent->x, m_fZoom), pixelToTwip(pEvent->y, m_fZoom), 1);

    return FALSE;
}

gboolean LOKDocView_Impl::renderOverlay(GtkWidget* pEventBox, GdkEventExpose* /*pEvent*/, LOKDocView* pDocView)
{
    return pDocView->m_pImpl->renderOverlayImpl(pEventBox);
}

gboolean LOKDocView_Impl::renderOverlayImpl(GtkWidget* pWidget)
{
#if GTK_CHECK_VERSION(2,14,0) // we need gtk_widget_get_window()
    cairo_t* pCairo = gdk_cairo_create(gtk_widget_get_window(pWidget));

    if (m_bEdit && m_bCursorVisible && m_bCursorOverlayVisible && !isEmptyRectangle(m_aVisibleCursor))
    {
        if (m_aVisibleCursor.width < 30)
            // Set a minimal width if it would be 0.
            m_aVisibleCursor.width = 30;

        cairo_set_source_rgb(pCairo, 0, 0, 0);
        cairo_rectangle(pCairo,
                        twipToPixel(m_aVisibleCursor.x, m_fZoom),
                        twipToPixel(m_aVisibleCursor.y, m_fZoom),
                        twipToPixel(m_aVisibleCursor.width, m_fZoom),
                        twipToPixel(m_aVisibleCursor.height, m_fZoom));
        cairo_fill(pCairo);
    }

    if (m_bEdit && m_bCursorVisible && !isEmptyRectangle(m_aVisibleCursor) && m_aTextSelectionRectangles.empty())
    {
        // Have a cursor, but no selection: we need the middle handle.
        if (!m_pHandleMiddle)
            m_pHandleMiddle = cairo_image_surface_create_from_png(CURSOR_HANDLE_DIR "handle_middle.png");
        renderHandle(pCairo, m_aVisibleCursor, m_pHandleMiddle, m_aHandleMiddleRect);
    }

    if (!m_aTextSelectionRectangles.empty())
    {
        for (GdkRectangle& rRectangle : m_aTextSelectionRectangles)
        {
            // Blue with 75% transparency.
            cairo_set_source_rgba(pCairo, ((double)0x43)/255, ((double)0xac)/255, ((double)0xe8)/255, 0.25);
            cairo_rectangle(pCairo,
                            twipToPixel(rRectangle.x, m_fZoom),
                            twipToPixel(rRectangle.y, m_fZoom),
                            twipToPixel(rRectangle.width, m_fZoom),
                            twipToPixel(rRectangle.height, m_fZoom));
            cairo_fill(pCairo);
        }

        // Handles
        if (!isEmptyRectangle(m_aTextSelectionStart))
        {
            // Have a start position: we need a start handle.
            if (!m_pHandleStart)
                m_pHandleStart = cairo_image_surface_create_from_png(CURSOR_HANDLE_DIR "handle_start.png");
            renderHandle(pCairo, m_aTextSelectionStart, m_pHandleStart, m_aHandleStartRect);
        }
        if (!isEmptyRectangle(m_aTextSelectionEnd))
        {
            // Have a start position: we need an end handle.
            if (!m_pHandleEnd)
                m_pHandleEnd = cairo_image_surface_create_from_png(CURSOR_HANDLE_DIR "handle_end.png");
            renderHandle(pCairo, m_aTextSelectionEnd, m_pHandleEnd, m_aHandleEndRect);
        }
    }

    if (!isEmptyRectangle(m_aGraphicSelection))
    {
        if (!m_pGraphicHandle)
            m_pGraphicHandle = cairo_image_surface_create_from_png(CURSOR_HANDLE_DIR "handle_graphic.png");
        renderGraphicHandle(pCairo, m_aGraphicSelection, m_pGraphicHandle);
    }

    cairo_destroy(pCairo);
#endif
    return FALSE;
}

bool LOKDocView_Impl::isEmptyRectangle(const GdkRectangle& rRectangle)
{
    return rRectangle.x == 0 && rRectangle.y == 0 && rRectangle.width == 0 && rRectangle.height == 0;
}

void LOKDocView_Impl::setTilesInvalid(const GdkRectangle& rRectangle)
{
    GdkRectangle aRectanglePixels;
    GdkPoint aStart, aEnd;

    aRectanglePixels.x = twipToPixel(rRectangle.x, m_fZoom);
    aRectanglePixels.y = twipToPixel(rRectangle.y, m_fZoom);
    aRectanglePixels.width = twipToPixel(rRectangle.width, m_fZoom);
    aRectanglePixels.height = twipToPixel(rRectangle.height, m_fZoom);

    aStart.x = aRectanglePixels.y / nTileSizePixels;
    aStart.y = aRectanglePixels.x / nTileSizePixels;
    aEnd.x = (aRectanglePixels.y + aRectanglePixels.height + nTileSizePixels) / nTileSizePixels;
    aEnd.y = (aRectanglePixels.x + aRectanglePixels.width + nTileSizePixels) / nTileSizePixels;

    for (int i = aStart.x; i < aEnd.x; i++)
        for (int j = aStart.y; j < aEnd.y; j++)
            m_aTileBuffer.setInvalid(i, j);
}

void LOKDocView_Impl::renderHandle(cairo_t* pCairo, const GdkRectangle& rCursor, cairo_surface_t* pHandle, GdkRectangle& rRectangle)
{
    GdkPoint aCursorBottom;
    int nHandleWidth, nHandleHeight;
    double fHandleScale;

    nHandleWidth = cairo_image_surface_get_width(pHandle);
    nHandleHeight = cairo_image_surface_get_height(pHandle);
    // We want to scale down the handle, so that its height is the same as the cursor caret.
    fHandleScale = twipToPixel(rCursor.height, m_fZoom) / nHandleHeight;
    // We want the top center of the handle bitmap to be at the bottom center of the cursor rectangle.
    aCursorBottom.x = twipToPixel(rCursor.x, m_fZoom) + twipToPixel(rCursor.width, m_fZoom) / 2 - (nHandleWidth * fHandleScale) / 2;
    aCursorBottom.y = twipToPixel(rCursor.y, m_fZoom) + twipToPixel(rCursor.height, m_fZoom);
    cairo_save(pCairo);
    cairo_translate(pCairo, aCursorBottom.x, aCursorBottom.y);
    cairo_scale(pCairo, fHandleScale, fHandleScale);
    cairo_set_source_surface(pCairo, pHandle, 0, 0);
    cairo_paint(pCairo);
    cairo_restore(pCairo);

    rRectangle.x = aCursorBottom.x;
    rRectangle.y = aCursorBottom.y;
    rRectangle.width = nHandleWidth * fHandleScale;
    rRectangle.height = nHandleHeight * fHandleScale;
}

/// Renders pHandle around an rSelection rectangle on pCairo.
void LOKDocView_Impl::renderGraphicHandle(cairo_t* pCairo, const GdkRectangle& rSelection, cairo_surface_t* pHandle)
{
    int nHandleWidth, nHandleHeight;
    GdkRectangle aSelection;

    nHandleWidth = cairo_image_surface_get_width(pHandle);
    nHandleHeight = cairo_image_surface_get_height(pHandle);

    aSelection.x = twipToPixel(rSelection.x, m_fZoom);
    aSelection.y = twipToPixel(rSelection.y, m_fZoom);
    aSelection.width = twipToPixel(rSelection.width, m_fZoom);
    aSelection.height = twipToPixel(rSelection.height, m_fZoom);

    for (int i = 0; i < GRAPHIC_HANDLE_COUNT; ++i)
    {
        int x = aSelection.x, y = aSelection.y;
        cairo_save(pCairo);

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

        m_aGraphicHandleRects[i].x = x;
        m_aGraphicHandleRects[i].y = y;
        m_aGraphicHandleRects[i].width = nHandleWidth;
        m_aGraphicHandleRects[i].height = nHandleHeight;

        cairo_translate(pCairo, x, y);
        cairo_set_source_surface(pCairo, pHandle, 0, 0);
        cairo_paint(pCairo);
        cairo_restore(pCairo);
    }
}

gboolean LOKDocView_Impl::handleTimeout(gpointer pData)
{
    LOKDocView* pDocView = static_cast<LOKDocView*>(pData);
    return pDocView->m_pImpl->handleTimeoutImpl();
}

gboolean LOKDocView_Impl::handleTimeoutImpl()
{
    if (m_bEdit)
    {
        if (m_bCursorOverlayVisible)
            m_bCursorOverlayVisible = false;
        else
            m_bCursorOverlayVisible = true;
        gtk_widget_queue_draw(GTK_WIDGET(m_pDocView));
    }

    return G_SOURCE_CONTINUE;
}

GdkRectangle LOKDocView_Impl::payloadToRectangle(const char* pPayload)
{
    GdkRectangle aRet;

    aRet.width = aRet.height = aRet.x = aRet.y = 0;
    gchar** ppCoordinates = g_strsplit(pPayload, ", ", 4);
    gchar** ppCoordinate = ppCoordinates;
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
    if (aRet.x + aRet.width > m_nDocumentWidthTwips)
        aRet.width = m_nDocumentWidthTwips - aRet.x;
    ++ppCoordinate;
    if (!*ppCoordinate)
        return aRet;
    aRet.height = atoi(*ppCoordinate);
    if (aRet.y + aRet.height > m_nDocumentHeightTwips)
        aRet.height = m_nDocumentHeightTwips - aRet.y;
    g_strfreev(ppCoordinates);
    return aRet;
}

std::vector<GdkRectangle> LOKDocView_Impl::payloadToRectangles(const char* pPayload)
{
    std::vector<GdkRectangle> aRet;

    gchar** ppRectangles = g_strsplit(pPayload, "; ", 0);
    for (gchar** ppRectangle = ppRectangles; *ppRectangle; ++ppRectangle)
        aRet.push_back(payloadToRectangle(*ppRectangle));
    g_strfreev(ppRectangles);

    return aRet;
}

/// Returns the string representation of a LibreOfficeKitCallbackType enumeration element.
const char* LOKDocView_Impl::callbackTypeToString(int nType)
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

gboolean LOKDocView_Impl::callback(gpointer pData)
{
    LOKDocView_Impl::CallbackData* pCallback = static_cast<LOKDocView_Impl::CallbackData*>(pData);
    return pCallback->m_pDocView->m_pImpl->callbackImpl(pCallback);
}

gboolean LOKDocView_Impl::globalCallback(gpointer pData)
{
    LOKDocView_Impl::CallbackData* pCallback = static_cast<LOKDocView_Impl::CallbackData*>(pData);
    return globalCallbackImpl(pCallback);
}

gboolean LOKDocView_Impl::callbackImpl(CallbackData* pCallback)
{
    switch (pCallback->m_nType)
    {
    case LOK_CALLBACK_INVALIDATE_TILES:
    {
        if (pCallback->m_aPayload != "EMPTY")
        {
            GdkRectangle aRectangle = LOKDocView_Impl::payloadToRectangle(pCallback->m_aPayload.c_str());
            setTilesInvalid(aRectangle);
        }
        else
            m_aTileBuffer.resetAllTiles();

        gtk_widget_queue_draw(GTK_WIDGET(m_pDocView));
    }
    break;
    case LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
    {
        m_aVisibleCursor = LOKDocView_Impl::payloadToRectangle(pCallback->m_aPayload.c_str());
        m_bCursorOverlayVisible = true;
        gtk_widget_queue_draw(GTK_WIDGET(m_pDocView));
    }
    break;
    case LOK_CALLBACK_TEXT_SELECTION:
    {
        m_aTextSelectionRectangles = LOKDocView_Impl::payloadToRectangles(pCallback->m_aPayload.c_str());

        // In case the selection is empty, then we get no LOK_CALLBACK_TEXT_SELECTION_START/END events.
        if (m_aTextSelectionRectangles.empty())
        {
            memset(&m_aTextSelectionStart, 0, sizeof(m_aTextSelectionStart));
            memset(&m_aHandleStartRect, 0, sizeof(m_aHandleStartRect));
            memset(&m_aTextSelectionEnd, 0, sizeof(m_aTextSelectionEnd));
            memset(&m_aHandleEndRect, 0, sizeof(m_aHandleEndRect));
        }
        else
            memset(&m_aHandleMiddleRect, 0, sizeof(m_aHandleMiddleRect));
    }
    break;
    case LOK_CALLBACK_TEXT_SELECTION_START:
    {
        m_aTextSelectionStart = LOKDocView_Impl::payloadToRectangle(pCallback->m_aPayload.c_str());
    }
    break;
    case LOK_CALLBACK_TEXT_SELECTION_END:
    {
        m_aTextSelectionEnd = LOKDocView_Impl::payloadToRectangle(pCallback->m_aPayload.c_str());
    }
    break;
    case LOK_CALLBACK_CURSOR_VISIBLE:
    {
        m_bCursorVisible = pCallback->m_aPayload == "true";
    }
    break;
    case LOK_CALLBACK_GRAPHIC_SELECTION:
    {
        if (pCallback->m_aPayload != "EMPTY")
            m_aGraphicSelection = LOKDocView_Impl::payloadToRectangle(pCallback->m_aPayload.c_str());
        else
            memset(&m_aGraphicSelection, 0, sizeof(m_aGraphicSelection));
        gtk_widget_queue_draw(GTK_WIDGET(m_pDocView));
    }
    break;
    case LOK_CALLBACK_HYPERLINK_CLICKED:
    {
        GError* pError = NULL;
#if GTK_CHECK_VERSION(2,14,0)
        gtk_show_uri(NULL, pCallback->m_aPayload.c_str(), GDK_CURRENT_TIME, &pError);
#endif
    }
    break;
    case LOK_CALLBACK_STATE_CHANGED:
    {
        commandChanged(pCallback->m_aPayload);
    }
    break;
    case LOK_CALLBACK_SEARCH_NOT_FOUND:
    {
        searchNotFound(pCallback->m_aPayload);
    }
    break;
    case LOK_CALLBACK_DOCUMENT_SIZE_CHANGED:
    {
        payloadToSize(pCallback->m_aPayload.c_str(), m_nDocumentWidthTwips, m_nDocumentHeightTwips);
        gtk_widget_set_size_request(GTK_WIDGET(m_pDocView),
                                    twipToPixel(m_nDocumentWidthTwips, m_fZoom),
                                    twipToPixel(m_nDocumentHeightTwips, m_fZoom));
    }
    break;
    case LOK_CALLBACK_SET_PART:
    {
        setPart(pCallback->m_aPayload);
    }
    break;
    default:
        g_assert(false);
        break;
    }
    delete pCallback;

    return G_SOURCE_REMOVE;
}

void LOKDocView_Impl::callbackWorker(int nType, const char* pPayload, void* pData)
{
    LOKDocView* pDocView = static_cast<LOKDocView*>(pData);
    pDocView->m_pImpl->callbackWorkerImpl(nType, pPayload);
}

void LOKDocView_Impl::globalCallbackWorker(int nType, const char* pPayload, void* pData)
{
    LOKDocView* pDocView = static_cast<LOKDocView*>(pData);
    pDocView->m_pImpl->globalCallbackWorkerImpl(nType, pPayload);
}

void LOKDocView_Impl::callbackWorkerImpl(int nType, const char* pPayload)
{
    LOKDocView_Impl::CallbackData* pCallback = new LOKDocView_Impl::CallbackData(nType, pPayload ? pPayload : "(nil)", m_pDocView);
    g_info("lok_doc_view_callback_worker: %s, '%s'", LOKDocView_Impl::callbackTypeToString(nType), pPayload);
#if GTK_CHECK_VERSION(2,12,0)
    gdk_threads_add_idle(LOKDocView_Impl::callback, pCallback);
#endif
}

void LOKDocView_Impl::globalCallbackWorkerImpl(int nType, const char* pPayload)
{
    LOKDocView_Impl::CallbackData* pCallback = new LOKDocView_Impl::CallbackData(nType, pPayload ? pPayload : "(nil)", m_pDocView);
    g_info("LOKDocView_Impl::globalCallbackWorkerImpl: %s, '%s'", LOKDocView_Impl::callbackTypeToString(nType), pPayload);
#if GTK_CHECK_VERSION(2,12,0)
    gdk_threads_add_idle(LOKDocView_Impl::globalCallback, pCallback);
#endif
}



void LOKDocView_Impl::commandChanged(const std::string& rString)
{
    g_signal_emit(m_pDocView, doc_view_signals[COMMAND_CHANGED], 0, rString.c_str());
}

void LOKDocView_Impl::searchNotFound(const std::string& rString)
{
    g_signal_emit(m_pDocView, doc_view_signals[SEARCH_NOT_FOUND], 0, rString.c_str());
}

void LOKDocView_Impl::setPart(const std::string& rString)
{
    g_signal_emit(m_pDocView, doc_view_signals[PART_CHANGED], 0, std::stoi(rString));
}

static void lok_doc_view_class_init (LOKDocViewClass* pClass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(pClass);
    pClass->edit_changed = NULL;
    doc_view_signals[EDIT_CHANGED] =
        g_signal_new("edit-changed",
                     G_TYPE_FROM_CLASS (gobject_class),
                     G_SIGNAL_RUN_FIRST,
                     G_STRUCT_OFFSET (LOKDocViewClass, edit_changed),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__BOOLEAN,
                     G_TYPE_NONE, 1,
                     G_TYPE_BOOLEAN);
    pClass->command_changed = NULL;
    doc_view_signals[COMMAND_CHANGED] =
        g_signal_new("command-changed",
                     G_TYPE_FROM_CLASS(gobject_class),
                     G_SIGNAL_RUN_FIRST,
                     G_STRUCT_OFFSET(LOKDocViewClass, command_changed),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__STRING,
                     G_TYPE_NONE, 1,
                     G_TYPE_STRING);
    pClass->search_not_found = 0;
    doc_view_signals[SEARCH_NOT_FOUND] =
        g_signal_new("search-not-found",
                     G_TYPE_FROM_CLASS(gobject_class),
                     G_SIGNAL_RUN_FIRST,
                     G_STRUCT_OFFSET(LOKDocViewClass, search_not_found),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__STRING,
                     G_TYPE_NONE, 1,
                     G_TYPE_STRING);
    pClass->part_changed = 0;
    doc_view_signals[PART_CHANGED] =
        g_signal_new("part-changed",
                     G_TYPE_FROM_CLASS(gobject_class),
                     G_SIGNAL_RUN_FIRST,
                     G_STRUCT_OFFSET(LOKDocViewClass, part_changed),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__INT,
                     G_TYPE_NONE, 1,
                     G_TYPE_INT);
}

static void lok_doc_view_init (LOKDocView* pDocView)
{
    pDocView->m_pImpl = new LOKDocView_Impl(pDocView);

    g_signal_connect(G_OBJECT(pDocView),
                     "expose-event",
                     G_CALLBACK(LOKDocView_Impl::onExposed), pDocView);
    g_signal_connect(G_OBJECT(pDocView),
                     "expose-event",
                     G_CALLBACK(LOKDocView_Impl::renderOverlay), pDocView);
    gtk_widget_add_events(GTK_WIDGET(pDocView),
                           GDK_BUTTON_PRESS_MASK
                          |GDK_BUTTON_RELEASE_MASK
                          |GDK_BUTTON_MOTION_MASK);

    g_signal_connect(G_OBJECT(pDocView),
                     "button-press-event",
                     G_CALLBACK(LOKDocView_Impl::signalButton), pDocView);
    g_signal_connect(G_OBJECT(pDocView),
                     "button-release-event",
                     G_CALLBACK(LOKDocView_Impl::signalButton), pDocView);
    g_signal_connect(G_OBJECT(pDocView),
                     "motion-notify-event",
                     G_CALLBACK(LOKDocView_Impl::signalMotion), pDocView);

    g_signal_connect(G_OBJECT(pDocView), "destroy", G_CALLBACK(LOKDocView_Impl::destroy), 0);
}

/**
 * lok_doc_view_new:
 * @pPath: LibreOffice install path.
 *
 * Returns: The #LOKDocView widget instance.
 */
SAL_DLLPUBLIC_EXPORT GtkWidget* lok_doc_view_new(const char* pPath)
{
    LOKDocView* pDocView = LOK_DOC_VIEW(g_object_new(LOK_TYPE_DOC_VIEW, NULL));
    pDocView->m_pImpl->m_pOffice = lok_init (pPath);
    if (pDocView->m_pImpl->m_pOffice == NULL)
        return NULL;
    return GTK_WIDGET( pDocView );
}

/**
 * lok_doc_view_open_document:
 * @pDocView: The #LOKDocView instance
 * @pPath: The path of the document that #LOKDocView widget should try to open
 *
 * Returns: %TRUE if the document is loaded succesfully, %FALSE otherwise
 */
SAL_DLLPUBLIC_EXPORT gboolean lok_doc_view_open_document( LOKDocView* pDocView, char* pPath )
{
    if ( pDocView->m_pImpl->m_pDocument )
    {
        pDocView->m_pImpl->m_pDocument->pClass->destroy( pDocView->m_pImpl->m_pDocument );
        pDocView->m_pImpl->m_pDocument = 0;
    }

    pDocView->m_pImpl->m_pOffice->pClass->registerCallback(pDocView->m_pImpl->m_pOffice, &LOKDocView_Impl::globalCallbackWorker, pDocView);
    pDocView->m_pImpl->m_pDocument = pDocView->m_pImpl->m_pOffice->pClass->documentLoad( pDocView->m_pImpl->m_pOffice,
                                                                   pPath );
    if ( !pDocView->m_pImpl->m_pDocument )
    {
        // FIXME: should have a GError parameter and populate it.
        char *pError = pDocView->m_pImpl->m_pOffice->pClass->getError( pDocView->m_pImpl->m_pOffice );
        fprintf( stderr, "Error opening document '%s'\n", pError );
        return FALSE;
    }
    else
    {
        pDocView->m_pImpl->m_pDocument->pClass->initializeForRendering(pDocView->m_pImpl->m_pDocument);
        pDocView->m_pImpl->m_pDocument->pClass->registerCallback(pDocView->m_pImpl->m_pDocument, &LOKDocView_Impl::callbackWorker, pDocView);
        pDocView->m_pImpl->m_pDocument->pClass->getDocumentSize(pDocView->m_pImpl->m_pDocument, &pDocView->m_pImpl->m_nDocumentWidthTwips, &pDocView->m_pImpl->m_nDocumentHeightTwips);
        g_timeout_add(600, &LOKDocView_Impl::handleTimeout, pDocView);

        float zoom = pDocView->m_pImpl->m_fZoom;
        long nDocumentWidthTwips = pDocView->m_pImpl->m_nDocumentWidthTwips;
        long nDocumentHeightTwips = pDocView->m_pImpl->m_nDocumentHeightTwips;
        long nDocumentWidthPixels = twipToPixel(nDocumentWidthTwips, zoom);
        long nDocumentHeightPixels = twipToPixel(nDocumentHeightTwips, zoom);
        // Total number of columns in this document.
        guint nColumns = ceil((double)nDocumentWidthPixels / nTileSizePixels);


        pDocView->m_pImpl->m_aTileBuffer = TileBuffer(pDocView->m_pImpl->m_pDocument,
                                                      nColumns);
        gtk_widget_set_size_request(GTK_WIDGET(pDocView),
                                    nDocumentWidthPixels,
                                    nDocumentHeightPixels);
    }

    return TRUE;
}

/**
 * lok_doc_view_get_document:
 * @pDocView: The #LOKDocView instance
 *
 * Returns: The #LibreOfficeKitDocument instance the widget is currently showing
 */
SAL_DLLPUBLIC_EXPORT LibreOfficeKitDocument* lok_doc_view_get_document(LOKDocView* pDocView)
{
    return pDocView->m_pImpl->m_pDocument;
}

/**
 * lok_doc_view_set_zoom:
 * @pDocView: The #LOKDocView instance
 * @fZoom: The new zoom level that pDocView must set it into.
 *
 * Sets the new zoom level for the widget.
 */
SAL_DLLPUBLIC_EXPORT void lok_doc_view_set_zoom ( LOKDocView* pDocView, float fZoom )
{
    pDocView->m_pImpl->m_fZoom = fZoom;
    long nDocumentWidthPixels = twipToPixel(pDocView->m_pImpl->m_nDocumentWidthTwips, fZoom);
    long nDocumentHeightPixels = twipToPixel(pDocView->m_pImpl->m_nDocumentHeightTwips, fZoom);
    // Total number of columns in this document.
    guint nColumns = ceil((double)nDocumentWidthPixels / nTileSizePixels);

    pDocView->m_pImpl->m_aTileBuffer = TileBuffer(pDocView->m_pImpl->m_pDocument,
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
SAL_DLLPUBLIC_EXPORT float lok_doc_view_get_zoom ( LOKDocView* pDocView )
{
    return pDocView->m_pImpl->m_fZoom;
}

SAL_DLLPUBLIC_EXPORT int lok_doc_view_get_parts( LOKDocView* pDocView )
{
    return pDocView->m_pImpl->m_pDocument->pClass->getParts( pDocView->m_pImpl->m_pDocument );
}

SAL_DLLPUBLIC_EXPORT int lok_doc_view_get_part( LOKDocView* pDocView )
{
    return pDocView->m_pImpl->m_pDocument->pClass->getPart( pDocView->m_pImpl->m_pDocument );
}

SAL_DLLPUBLIC_EXPORT void lok_doc_view_set_part( LOKDocView* pDocView, int nPart)
{
    pDocView->m_pImpl->m_pDocument->pClass->setPart( pDocView->m_pImpl->m_pDocument, nPart );
}

SAL_DLLPUBLIC_EXPORT char* lok_doc_view_get_part_name( LOKDocView* pDocView, int nPart )
{
    return pDocView->m_pImpl->m_pDocument->pClass->getPartName( pDocView->m_pImpl->m_pDocument, nPart );
}

SAL_DLLPUBLIC_EXPORT void lok_doc_view_set_partmode( LOKDocView* pDocView,
                                                    int nPartMode )
{
    pDocView->m_pImpl->m_pDocument->pClass->setPartMode( pDocView->m_pImpl->m_pDocument, nPartMode );
}

/**
 * lok_doc_view_set_edit:
 * @pDocView: The #LOKDocView instance
 * @bEdit: %TRUE if the pDocView should go in edit mode, %FALSE otherwise
 *
 * Sets the edit-mode for pDocView
 */
SAL_DLLPUBLIC_EXPORT void lok_doc_view_set_edit( LOKDocView* pDocView,
                                                gboolean bEdit )
{
    gboolean bWasEdit = pDocView->m_pImpl->m_bEdit;

    if (!pDocView->m_pImpl->m_bEdit && bEdit)
        g_info("lok_doc_view_set_edit: entering edit mode");
    else if (pDocView->m_pImpl->m_bEdit && !bEdit)
    {
        g_info("lok_doc_view_set_edit: leaving edit mode");
        pDocView->m_pImpl->m_pDocument->pClass->resetSelection(pDocView->m_pImpl->m_pDocument);
    }
    pDocView->m_pImpl->m_bEdit = bEdit;
    g_signal_emit(pDocView, doc_view_signals[EDIT_CHANGED], 0, bWasEdit);
    gtk_widget_queue_draw(GTK_WIDGET(pDocView));
}

/**
 * lok_doc_view_get_edit:
 * @pDocView: The #LOKDocView instance
 *
 * Returns: %TRUE if the given pDocView is in edit mode.
 */
SAL_DLLPUBLIC_EXPORT gboolean lok_doc_view_get_edit(LOKDocView* pDocView)
{
    return pDocView->m_pImpl->m_bEdit;
}

SAL_DLLPUBLIC_EXPORT void lok_doc_view_post_command(LOKDocView* pDocView, const char* pCommand, const char* pArguments)
{
    pDocView->m_pImpl->m_pDocument->pClass->postUnoCommand(pDocView->m_pImpl->m_pDocument, pCommand, pArguments);
}

SAL_DLLPUBLIC_EXPORT void lok_doc_view_post_key(GtkWidget* /*pWidget*/, GdkEventKey* pEvent, gpointer pData)
{
    LOKDocView* pDocView = static_cast<LOKDocView *>(pData);
    pDocView->m_pImpl->signalKey(pEvent);
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
SAL_DLLPUBLIC_EXPORT float lok_doc_view_pixel_to_twip(LOKDocView* pDocView, float fInput)
{
    return pixelToTwip(fInput, pDocView->m_pImpl->m_fZoom);
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
SAL_DLLPUBLIC_EXPORT float lok_doc_view_twip_to_pixel(LOKDocView* pDocView, float fInput)
{
    return twipToPixel(fInput, pDocView->m_pImpl->m_fZoom);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
