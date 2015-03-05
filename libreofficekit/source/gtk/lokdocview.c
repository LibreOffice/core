/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#define LOK_USE_UNSTABLE_API
#include <LibreOfficeKit/LibreOfficeKit.h>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <LibreOfficeKit/LibreOfficeKitGtk.h>

#if !GLIB_CHECK_VERSION(2,32,0)
#define G_SOURCE_REMOVE FALSE
#define G_SOURCE_CONTINUE TRUE
#endif
#ifndef g_info
#define g_info(...) g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, __VA_ARGS__)
#endif

// Cursor bitmaps from the Android app.
#define CURSOR_HANDLE_DIR "android/experimental/LOAndroid3/res/drawable/"

static void lok_docview_class_init( LOKDocViewClass* pClass );
static void lok_docview_init( LOKDocView* pDocView );
static float pixelToTwip(float nInput);
static gboolean renderOverlay(GtkWidget* pWidget, GdkEventExpose* pEvent, gpointer pData);

// We specifically need to destroy the document when closing in order to ensure
// that lock files etc. are cleaned up.
void lcl_onDestroy( LOKDocView* pDocView, gpointer pData )
{
    (void) pData;
    if ( pDocView->pDocument )
        pDocView->pDocument->pClass->destroy( pDocView->pDocument );
    pDocView->pDocument = NULL;
}

/**
 * The user drags the handle, which is below the cursor, but wants to move the
 * cursor accordingly.
 *
 * @param pHandle the rectangle of the handle
 * @param pEvent the motion event
 * @param pPoint the computed point (output parameter)
 */
void lcl_getDragPoint(GdkRectangle* pHandle, GdkEventButton* pEvent, GdkPoint* pPoint)
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

gboolean lcl_signalMotion(GtkWidget* pEventBox, GdkEventButton* pEvent, LOKDocView* pDocView)
{
    GdkPoint aPoint;

    (void)pEventBox;
    if (pDocView->m_bInDragMiddleHandle)
    {
        g_info("lcl_signalMotion: dragging the middle handle");
        lcl_getDragPoint(&pDocView->m_aHandleMiddleRect, pEvent, &aPoint);
        pDocView->pDocument->pClass->setTextSelection(
                pDocView->pDocument, LOK_SETTEXTSELECTION_RESET,
                pixelToTwip(aPoint.x) / pDocView->fZoom, pixelToTwip(aPoint.y) / pDocView->fZoom);
    }
    else if (pDocView->m_bInDragStartHandle)
    {
        g_info("lcl_signalMotion: dragging the start handle");
        lcl_getDragPoint(&pDocView->m_aHandleStartRect, pEvent, &aPoint);
        pDocView->pDocument->pClass->setTextSelection(
                pDocView->pDocument, LOK_SETTEXTSELECTION_START,
                pixelToTwip(aPoint.x) / pDocView->fZoom, pixelToTwip(aPoint.y) / pDocView->fZoom);
    }
    else if (pDocView->m_bInDragEndHandle)
    {
        g_info("lcl_signalMotion: dragging the end handle");
        lcl_getDragPoint(&pDocView->m_aHandleEndRect, pEvent, &aPoint);
        pDocView->pDocument->pClass->setTextSelection(
                pDocView->pDocument, LOK_SETTEXTSELECTION_END,
                pixelToTwip(aPoint.x) / pDocView->fZoom, pixelToTwip(aPoint.y) / pDocView->fZoom);
    }
    return FALSE;
}

/// Receives a button press event.
gboolean lcl_signalButton(GtkWidget* pEventBox, GdkEventButton* pEvent, LOKDocView* pDocView)
{
    g_info("lcl_signalButton: %d, %d (in twips: %d, %d)", (int)pEvent->x, (int)pEvent->y, (int)pixelToTwip(pEvent->x), (int)pixelToTwip(pEvent->y));
    (void) pEventBox;

    if (pEvent->type == GDK_BUTTON_RELEASE)
    {
        if (pDocView->m_bInDragStartHandle)
        {
            g_info("lcl_signalButton: end of drag start handle");
            pDocView->m_bInDragStartHandle = FALSE;
            return FALSE;
        }
        else if (pDocView->m_bInDragMiddleHandle)
        {
            g_info("lcl_signalButton: end of drag middle handle");
            pDocView->m_bInDragMiddleHandle = FALSE;
            return FALSE;
        }
        else if (pDocView->m_bInDragEndHandle)
        {
            g_info("lcl_signalButton: end of drag end handle");
            pDocView->m_bInDragEndHandle = FALSE;
            return FALSE;
        }
    }

    if (pDocView->m_bEdit)
    {
        GdkRectangle aClick;
        aClick.x = pEvent->x;
        aClick.y = pEvent->y;
        aClick.width = 1;
        aClick.height = 1;
        if (pEvent->type == GDK_BUTTON_PRESS)
        {
            if (gdk_rectangle_intersect(&aClick, &pDocView->m_aHandleStartRect, NULL))
            {
                g_info("lcl_signalButton: start of drag start handle");
                pDocView->m_bInDragStartHandle = TRUE;
                return FALSE;
            }
            else if (gdk_rectangle_intersect(&aClick, &pDocView->m_aHandleMiddleRect, NULL))
            {
                g_info("lcl_signalButton: start of drag middle handle");
                pDocView->m_bInDragMiddleHandle = TRUE;
                return FALSE;
            }
            else if (gdk_rectangle_intersect(&aClick, &pDocView->m_aHandleEndRect, NULL))
            {
                g_info("lcl_signalButton: start of drag end handle");
                pDocView->m_bInDragEndHandle = TRUE;
                return FALSE;
            }
        }
    }

    lok_docview_set_edit(pDocView, TRUE);

    switch (pEvent->type)
    {
    case GDK_BUTTON_PRESS:
    {
        int nCount = 1;
        if ((pEvent->time - pDocView->m_nLastButtonPressTime) < 250)
            nCount++;
        pDocView->m_nLastButtonPressTime = pEvent->time;
        pDocView->pDocument->pClass->postMouseEvent(
                pDocView->pDocument, LOK_MOUSEEVENT_MOUSEBUTTONDOWN,
                pixelToTwip(pEvent->x) / pDocView->fZoom,
                pixelToTwip(pEvent->y) / pDocView->fZoom, nCount);
        break;
    }
    case GDK_BUTTON_RELEASE:
    {
        int nCount = 1;
        if ((pEvent->time - pDocView->m_nLastButtonReleaseTime) < 250)
            nCount++;
        pDocView->m_nLastButtonReleaseTime = pEvent->time;
        pDocView->pDocument->pClass->postMouseEvent(
                pDocView->pDocument, LOK_MOUSEEVENT_MOUSEBUTTONUP,
                pixelToTwip(pEvent->x) / pDocView->fZoom,
                pixelToTwip(pEvent->y) / pDocView->fZoom, nCount);
        break;
    }
    default:
        break;
    }
    return FALSE;
}

SAL_DLLPUBLIC_EXPORT guint lok_docview_get_type()
{
    static guint lok_docview_type = 0;

    if (!lok_docview_type)
    {
        GtkTypeInfo lok_docview_info =
        {
            "LokDocView",
            sizeof( LOKDocView ),
            sizeof( LOKDocViewClass ),
            (GtkClassInitFunc) lok_docview_class_init,
            (GtkObjectInitFunc) lok_docview_init,
            NULL,
            NULL,
            (GtkClassInitFunc) NULL
        };

        lok_docview_type = gtk_type_unique( gtk_scrolled_window_get_type(), &lok_docview_info );
    }
    return lok_docview_type;
}

static void lok_docview_class_init( LOKDocViewClass* pClass )
{
    (void)pClass;
}

static void lok_docview_init( LOKDocView* pDocView )
{
    // Gtk ScrolledWindow is apparently not fully initialised yet, we specifically
    // have to set the [hv]adjustment to prevent GTK assertions from firing, see
    // https://bugzilla.gnome.org/show_bug.cgi?id=438114 for more info.
    gtk_scrolled_window_set_hadjustment( GTK_SCROLLED_WINDOW( pDocView ), NULL );
    gtk_scrolled_window_set_vadjustment( GTK_SCROLLED_WINDOW( pDocView ), NULL );

    pDocView->pEventBox = gtk_event_box_new();
    gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW(pDocView),
                                           pDocView->pEventBox );

    gtk_widget_set_events(pDocView->pEventBox, GDK_BUTTON_PRESS_MASK); // So that drag doesn't try to move the whole window.
    gtk_signal_connect(GTK_OBJECT(pDocView->pEventBox), "button-press-event", GTK_SIGNAL_FUNC(lcl_signalButton), pDocView);
    gtk_signal_connect(GTK_OBJECT(pDocView->pEventBox), "button-release-event", GTK_SIGNAL_FUNC(lcl_signalButton), pDocView);
    gtk_signal_connect(GTK_OBJECT(pDocView->pEventBox), "motion-notify-event", GTK_SIGNAL_FUNC(lcl_signalMotion), pDocView);

    gtk_widget_show( pDocView->pEventBox );

    pDocView->pTable = 0;
    pDocView->pCanvas = 0;

    // TODO: figure out a clever view of getting paths set up.
    pDocView->pOffice = 0;
    pDocView->pDocument = 0;

    pDocView->fZoom = 1;
    pDocView->m_bEdit = FALSE;
    memset(&pDocView->m_aVisibleCursor, 0, sizeof(pDocView->m_aVisibleCursor));
    pDocView->m_bCursorOverlayVisible = FALSE;
    pDocView->m_bCursorVisible = FALSE;
    pDocView->m_nLastButtonPressTime = 0;
    pDocView->m_nLastButtonReleaseTime = 0;
    pDocView->m_pTextSelectionRectangles = NULL;
    memset(&pDocView->m_aTextSelectionStart, 0, sizeof(pDocView->m_aTextSelectionStart));
    memset(&pDocView->m_aTextSelectionEnd, 0, sizeof(pDocView->m_aTextSelectionEnd));
    memset(&pDocView->m_aGraphicSelection, 0, sizeof(pDocView->m_aGraphicSelection));

    // Start/middle/end handle.
    pDocView->m_pHandleStart = NULL;
    memset(&pDocView->m_aHandleStartRect, 0, sizeof(pDocView->m_aHandleStartRect));
    pDocView->m_bInDragStartHandle = FALSE;
    pDocView->m_pHandleMiddle = NULL;
    memset(&pDocView->m_aHandleMiddleRect, 0, sizeof(pDocView->m_aHandleMiddleRect));
    pDocView->m_bInDragMiddleHandle = FALSE;
    pDocView->m_pHandleEnd = NULL;
    memset(&pDocView->m_aHandleEndRect, 0, sizeof(pDocView->m_aHandleEndRect));
    pDocView->m_bInDragEndHandle = FALSE;

    gtk_signal_connect( GTK_OBJECT(pDocView), "destroy",
                        GTK_SIGNAL_FUNC(lcl_onDestroy), NULL );
    g_signal_connect_after(pDocView->pEventBox, "expose-event",
                           G_CALLBACK(renderOverlay), pDocView);
}

SAL_DLLPUBLIC_EXPORT GtkWidget* lok_docview_new( LibreOfficeKit* pOffice )
{
    LOKDocView* pDocView = gtk_type_new( lok_docview_get_type() );
    pDocView->pOffice = pOffice;
    return GTK_WIDGET( pDocView );
}

// We know that VirtualDevices use a DPI of 96.
static const int g_nDPI = 96;

/// Converts from document coordinates to screen pixels.
static float twipToPixel(float nInput)
{
    return nInput / 1440.0f * g_nDPI;
}

/// Converts from screen pixels to document coordinates
static float pixelToTwip(float nInput)
{
    return (nInput / g_nDPI) * 1440.0f;
}

static gboolean lcl_isEmptyRectangle(GdkRectangle* pRectangle)
{
    return pRectangle->x == 0 && pRectangle->y == 0 && pRectangle->width == 0 && pRectangle->height == 0;
}

/// Takes care of the blinking cursor.
static gboolean lcl_handleTimeout(gpointer pData)
{
    LOKDocView* pDocView = pData;

    if (pDocView->m_bEdit)
    {
        if (pDocView->m_bCursorOverlayVisible)
            pDocView->m_bCursorOverlayVisible = FALSE;
        else
            pDocView->m_bCursorOverlayVisible = TRUE;
        gtk_widget_queue_draw(GTK_WIDGET(pDocView->pEventBox));
    }

    return G_SOURCE_CONTINUE;
}

/// Renders pHandle below a pCursor rectangle on pCairo.
static void lcl_renderHandle(cairo_t* pCairo, GdkRectangle* pCursor, cairo_surface_t* pHandle,
                             GdkRectangle* pRectangle, float fZoom)
{
    GdkPoint aCursorBottom;
    int nHandleWidth, nHandleHeight;
    double fHandleScale;

    nHandleWidth = cairo_image_surface_get_width(pHandle);
    nHandleHeight = cairo_image_surface_get_height(pHandle);
    // We want to scale down the handle, so that its height is the same as the cursor caret.
    fHandleScale = twipToPixel(pCursor->height) * fZoom / nHandleHeight;
    // We want the top center of the handle bitmap to be at the bottom center of the cursor rectangle.
    aCursorBottom.x = twipToPixel(pCursor->x) * fZoom + twipToPixel(pCursor->width) * fZoom / 2 - (nHandleWidth * fHandleScale) / 2;
    aCursorBottom.y = twipToPixel(pCursor->y) * fZoom + twipToPixel(pCursor->height) * fZoom;
    cairo_save(pCairo);
    cairo_translate(pCairo, aCursorBottom.x, aCursorBottom.y);
    cairo_scale(pCairo, fHandleScale, fHandleScale);
    cairo_set_source_surface(pCairo, pHandle, 0, 0);
    cairo_paint(pCairo);
    cairo_restore(pCairo);

    if (pRectangle)
    {
        // Return the rectangle that contains the rendered handle.
        pRectangle->x = aCursorBottom.x;
        pRectangle->y = aCursorBottom.y;
        pRectangle->width = nHandleWidth * fHandleScale;
        pRectangle->height = nHandleHeight * fHandleScale;
    }
}

static gboolean renderOverlay(GtkWidget* pWidget, GdkEventExpose* pEvent, gpointer pData)
{
#if GTK_CHECK_VERSION(2,14,0) // we need gtk_widget_get_window()
    LOKDocView* pDocView = pData;
    cairo_t* pCairo;

    (void)pEvent;
    pCairo = gdk_cairo_create(gtk_widget_get_window(pWidget));

    if (pDocView->m_bCursorVisible && pDocView->m_bCursorOverlayVisible && !lcl_isEmptyRectangle(&pDocView->m_aVisibleCursor))
    {
        if (pDocView->m_aVisibleCursor.width == 0)
            // Set a minimal width if it would be 0.
            pDocView->m_aVisibleCursor.width = 30;

        cairo_set_source_rgb(pCairo, 0, 0, 0);
        cairo_rectangle(pCairo,
                        twipToPixel(pDocView->m_aVisibleCursor.x) * pDocView->fZoom,
                        twipToPixel(pDocView->m_aVisibleCursor.y) * pDocView->fZoom,
                        twipToPixel(pDocView->m_aVisibleCursor.width) * pDocView->fZoom,
                        twipToPixel(pDocView->m_aVisibleCursor.height) * pDocView->fZoom);
        cairo_fill(pCairo);

    }

    if (pDocView->m_bCursorVisible && !lcl_isEmptyRectangle(&pDocView->m_aVisibleCursor) && !pDocView->m_pTextSelectionRectangles)
    {
        // Have a cursor, but no selection: we need the middle handle.
        if (!pDocView->m_pHandleMiddle)
            pDocView->m_pHandleMiddle = cairo_image_surface_create_from_png(CURSOR_HANDLE_DIR "handle_middle.png");
        lcl_renderHandle(pCairo, &pDocView->m_aVisibleCursor,
                         pDocView->m_pHandleMiddle, &pDocView->m_aHandleMiddleRect,
                         pDocView->fZoom);
    }

    if (pDocView->m_pTextSelectionRectangles)
    {
        GList* i;

        for (i = pDocView->m_pTextSelectionRectangles; i != NULL; i = i->next)
        {
            GdkRectangle* pRectangle = i->data;
            // Blue with 75% transparency.
            cairo_set_source_rgba(pCairo, ((double)0x43)/255, ((double)0xac)/255, ((double)0xe8)/255, 0.25);
            cairo_rectangle(pCairo,
                            twipToPixel(pRectangle->x) * pDocView->fZoom,
                            twipToPixel(pRectangle->y) * pDocView->fZoom,
                            twipToPixel(pRectangle->width) * pDocView->fZoom,
                            twipToPixel(pRectangle->height) * pDocView->fZoom);
            cairo_fill(pCairo);
        }

        // Handles
        if (!lcl_isEmptyRectangle(&pDocView->m_aTextSelectionStart))
        {
            // Have a start position: we need a start handle.
            if (!pDocView->m_pHandleStart)
                pDocView->m_pHandleStart = cairo_image_surface_create_from_png(CURSOR_HANDLE_DIR "handle_start.png");
            lcl_renderHandle(pCairo, &pDocView->m_aTextSelectionStart,
                             pDocView->m_pHandleStart, &pDocView->m_aHandleStartRect,
                             pDocView->fZoom);
        }
        if (!lcl_isEmptyRectangle(&pDocView->m_aTextSelectionEnd))
        {
            // Have a start position: we need an end handle.
            if (!pDocView->m_pHandleEnd)
                pDocView->m_pHandleEnd = cairo_image_surface_create_from_png(CURSOR_HANDLE_DIR "handle_end.png");
            lcl_renderHandle(pCairo, &pDocView->m_aTextSelectionEnd,
                             pDocView->m_pHandleEnd, &pDocView->m_aHandleEndRect,
                             pDocView->fZoom);
        }
    }

    if (!lcl_isEmptyRectangle(&pDocView->m_aGraphicSelection))
    {
    }

    cairo_destroy(pCairo);
#endif
    return FALSE;
}

void renderDocument(LOKDocView* pDocView, GdkRectangle* pPartial)
{
    long nDocumentWidthTwips, nDocumentHeightTwips, nDocumentWidthPixels, nDocumentHeightPixels;
    const int nTileSizePixels = 256;
    // Current row / column.
    guint nRow, nColumn;
    // Total number of rows / columns in this document.
    guint nRows, nColumns;

    // Get document size and find out how many rows / columns we need.
    pDocView->pDocument->pClass->getDocumentSize(pDocView->pDocument, &nDocumentWidthTwips, &nDocumentHeightTwips);
    nDocumentWidthPixels = twipToPixel(nDocumentWidthTwips) * pDocView->fZoom;
    nDocumentHeightPixels = twipToPixel(nDocumentHeightTwips) * pDocView->fZoom;
    nRows = ceil((double)nDocumentHeightPixels / nTileSizePixels);
    nColumns = ceil((double)nDocumentWidthPixels / nTileSizePixels);

    // Set up our table and the tile pointers.
    if (!pDocView->pTable)
        pPartial = NULL;
    if (pPartial)
    {
        // Same as nRows / nColumns, but from the previous renderDocument() call.
        guint nOldRows, nOldColumns;

#if GTK_CHECK_VERSION(2,22,0)
        gtk_table_get_size(GTK_TABLE(pDocView->pTable), &nOldRows, &nOldColumns);
        if (nOldRows != nRows || nOldColumns != nColumns)
            // Can't do partial rendering, document size changed.
            pPartial = NULL;
#else
        pPartial = NULL;
#endif
    }
    if (!pPartial)
    {
        if (pDocView->pTable)
            gtk_container_remove(GTK_CONTAINER(pDocView->pEventBox), pDocView->pTable);
        pDocView->pTable = gtk_table_new(nRows, nColumns, FALSE);
        gtk_container_add(GTK_CONTAINER(pDocView->pEventBox), pDocView->pTable);
        gtk_widget_show(pDocView->pTable);
        if (pDocView->pCanvas)
            g_free(pDocView->pCanvas);
        pDocView->pCanvas = g_malloc0(sizeof(GtkWidget*) * nRows * nColumns);
    }

    // Render the tiles.
    for (nRow = 0; nRow < nRows; ++nRow)
    {
        for (nColumn = 0; nColumn < nColumns; ++nColumn)
        {
            GdkRectangle aTileRectangleTwips, aTileRectanglePixels;
            gboolean bPaint = TRUE;

            // Determine size of the tile: the rightmost/bottommost tiles may be smaller and we need the size to decide if we need to repaint.
            if (nColumn == nColumns - 1)
                aTileRectanglePixels.width = nDocumentWidthPixels - nColumn * nTileSizePixels;
            else
                aTileRectanglePixels.width = nTileSizePixels;
            if (nRow == nRows - 1)
                aTileRectanglePixels.height = nDocumentHeightPixels - nRow * nTileSizePixels;
            else
                aTileRectanglePixels.height = nTileSizePixels;

            // Determine size and position of the tile in document coordinates, so we can decide if we can skip painting for partial rendering.
            aTileRectangleTwips.x = pixelToTwip(nTileSizePixels) / pDocView->fZoom * nColumn;
            aTileRectangleTwips.y = pixelToTwip(nTileSizePixels) / pDocView->fZoom * nRow;
            aTileRectangleTwips.width = pixelToTwip(aTileRectanglePixels.width) / pDocView->fZoom;
            aTileRectangleTwips.height = pixelToTwip(aTileRectanglePixels.height) / pDocView->fZoom;
            if (pPartial && !gdk_rectangle_intersect(pPartial, &aTileRectangleTwips, NULL))
                    bPaint = FALSE;

            if (bPaint)
            {
                // Index of the current tile.
                guint nTile = nRow * nColumns + nColumn;
                GdkPixbuf* pPixBuf;
                unsigned char* pBuffer;
                int nRowStride;

                pPixBuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, aTileRectanglePixels.width, aTileRectanglePixels.height);
                pBuffer = gdk_pixbuf_get_pixels(pPixBuf);
                g_info("renderDocument: paintTile(%d, %d)", nRow, nColumn);
                pDocView->pDocument->pClass->paintTile(pDocView->pDocument,
                                                       // Buffer and its size, depends on the position only.
                                                       pBuffer,
                                                       aTileRectanglePixels.width, aTileRectanglePixels.height,
                                                       &nRowStride,
                                                       // Position of the tile.
                                                       aTileRectangleTwips.x, aTileRectangleTwips.y,
                                                       // Size of the tile, depends on the zoom factor and the tile position only.
                                                       aTileRectangleTwips.width, aTileRectangleTwips.height);
                (void) nRowStride;

                if (pDocView->pCanvas[nTile])
                    gtk_widget_destroy(GTK_WIDGET(pDocView->pCanvas[nTile]));
                pDocView->pCanvas[nTile] = gtk_image_new();
                gtk_image_set_from_pixbuf(GTK_IMAGE(pDocView->pCanvas[nTile]), pPixBuf);
                g_object_unref(G_OBJECT(pPixBuf));
                gtk_widget_show(pDocView->pCanvas[nTile]);
                gtk_table_attach(GTK_TABLE(pDocView->pTable),
                                 pDocView->pCanvas[nTile],
                                 nColumn, nColumn + 1, nRow, nRow + 1,
                                 GTK_SHRINK, GTK_SHRINK, 0, 0);
            }
        }
    }
}

/// Callback data, allocated in lok_docview_callback_worker(), released in lok_docview_callback().
typedef struct
{
    int m_nType;
    char* m_pPayload;
    LOKDocView* m_pDocView;
}
LOKDocViewCallbackData;

/// Returns the GdkRectangle of a width,height,x,y string.
static GdkRectangle lcl_payloadToRectangle(const char* pPayload)
{
    GdkRectangle aRet;
    gchar** ppCoordinates;
    gchar** ppCoordinate;

    aRet.width = aRet.height = aRet.x = aRet.y = 0;
    ppCoordinates = g_strsplit(pPayload, ", ", 4);
    ppCoordinate = ppCoordinates;
    if (!*ppCoordinate)
        return aRet;
    aRet.width = atoi(*ppCoordinate);
    ++ppCoordinate;
    if (!*ppCoordinate)
        return aRet;
    aRet.height = atoi(*ppCoordinate);
    ++ppCoordinate;
    if (!*ppCoordinate)
        return aRet;
    aRet.x = atoi(*ppCoordinate);
    ++ppCoordinate;
    if (!*ppCoordinate)
        return aRet;
    aRet.y = atoi(*ppCoordinate);
    g_strfreev(ppCoordinates);
    return aRet;
}

/// Returns the GdkRectangle list of a w,h,x,y;w2,h2,x2,y2;... string.
static GList* lcl_payloadToRectangles(const char* pPayload)
{
    GList* pRet = NULL;
    gchar** ppRectangles;
    gchar** ppRectangle;

    ppRectangles = g_strsplit(pPayload, "; ", 0);
    for (ppRectangle = ppRectangles; *ppRectangle; ++ppRectangle)
    {
        GdkRectangle aRect = lcl_payloadToRectangle(*ppRectangle);
        GdkRectangle* pRect = g_new0(GdkRectangle, 1);
        *pRect = aRect;
        pRet = g_list_prepend(pRet, pRect);
    }
    g_strfreev(ppRectangles);
    return pRet;
}

static const gchar* lcl_LibreOfficeKitCallbackTypeToString(int nType)
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
    }
    return 0;
}

/// Invoked on the main thread if lok_docview_callback_worker() requests so.
static gboolean lok_docview_callback(gpointer pData)
{
#if GLIB_CHECK_VERSION(2,28,0) // we need g_list_free_full()
    LOKDocViewCallbackData* pCallback = pData;

    switch (pCallback->m_nType)
    {
    case LOK_CALLBACK_INVALIDATE_TILES:
    {
        if (strcmp(pCallback->m_pPayload, "EMPTY") != 0)
        {
            GdkRectangle aRectangle = lcl_payloadToRectangle(pCallback->m_pPayload);
            renderDocument(pCallback->m_pDocView, &aRectangle);
        }
        else
            renderDocument(pCallback->m_pDocView, NULL);
    }
    break;
    case LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
    {
        pCallback->m_pDocView->m_aVisibleCursor = lcl_payloadToRectangle(pCallback->m_pPayload);
        pCallback->m_pDocView->m_bCursorOverlayVisible = TRUE;
        gtk_widget_queue_draw(GTK_WIDGET(pCallback->m_pDocView->pEventBox));
    }
    break;
    case LOK_CALLBACK_TEXT_SELECTION:
    {
        LOKDocView* pDocView = pCallback->m_pDocView;
        GList* pRectangles = lcl_payloadToRectangles(pCallback->m_pPayload);
        if (pDocView->m_pTextSelectionRectangles)
            g_list_free_full(pDocView->m_pTextSelectionRectangles, g_free);
        pDocView->m_pTextSelectionRectangles = pRectangles;

        // In case the selection is empty, then we get no LOK_CALLBACK_TEXT_SELECTION_START/END events.
        if (pRectangles == NULL)
        {
            memset(&pDocView->m_aTextSelectionStart, 0, sizeof(pDocView->m_aTextSelectionStart));
            memset(&pDocView->m_aHandleStartRect, 0, sizeof(pDocView->m_aHandleStartRect));
            memset(&pDocView->m_aTextSelectionEnd, 0, sizeof(pDocView->m_aTextSelectionEnd));
            memset(&pDocView->m_aHandleEndRect, 0, sizeof(pDocView->m_aHandleEndRect));
        }
        else
            memset(&pDocView->m_aHandleMiddleRect, 0, sizeof(pDocView->m_aHandleMiddleRect));
        gtk_widget_queue_draw(GTK_WIDGET(pDocView->pEventBox));
    }
    break;
    case LOK_CALLBACK_TEXT_SELECTION_START:
    {
        pCallback->m_pDocView->m_aTextSelectionStart = lcl_payloadToRectangle(pCallback->m_pPayload);
    }
    break;
    case LOK_CALLBACK_TEXT_SELECTION_END:
    {
        pCallback->m_pDocView->m_aTextSelectionEnd = lcl_payloadToRectangle(pCallback->m_pPayload);
    }
    break;
    case LOK_CALLBACK_CURSOR_VISIBLE:
    {
        pCallback->m_pDocView->m_bCursorVisible = strcmp(pCallback->m_pPayload, "true") == 0;
    }
    break;
    case LOK_CALLBACK_GRAPHIC_SELECTION:
    {
        if (strcmp(pCallback->m_pPayload, "EMPTY") != 0)
            pCallback->m_pDocView->m_aGraphicSelection = lcl_payloadToRectangle(pCallback->m_pPayload);
        else
            memset(&pCallback->m_pDocView->m_aGraphicSelection, 0, sizeof(pCallback->m_pDocView->m_aGraphicSelection));
        gtk_widget_queue_draw(GTK_WIDGET(pCallback->m_pDocView->pEventBox));
    }
    break;
    default:
        break;
    }

    g_free(pCallback->m_pPayload);
    g_free(pCallback);
#endif
    return G_SOURCE_REMOVE;
}

/// Our LOK callback, runs on the LO thread.
static void lok_docview_callback_worker(int nType, const char* pPayload, void* pData)
{
    LOKDocView* pDocView = pData;

    LOKDocViewCallbackData* pCallback = g_new0(LOKDocViewCallbackData, 1);
    pCallback->m_nType = nType;
    pCallback->m_pPayload = g_strdup(pPayload);
    pCallback->m_pDocView = pDocView;
    g_info("lok_docview_callback_worker: %s, '%s'", lcl_LibreOfficeKitCallbackTypeToString(nType), pPayload);
#if GTK_CHECK_VERSION(2,12,0)
    gdk_threads_add_idle(lok_docview_callback, pCallback);
#else
    g_idle_add(lok_docview_callback, pDocView);
#endif
}

SAL_DLLPUBLIC_EXPORT gboolean lok_docview_open_document( LOKDocView* pDocView, char* pPath )
{
    if ( pDocView->pDocument )
    {
        pDocView->pDocument->pClass->destroy( pDocView->pDocument );
        pDocView->pDocument = NULL;
    }

    pDocView->pDocument = pDocView->pOffice->pClass->documentLoad( pDocView->pOffice,
                                                                   pPath );
    if ( !pDocView->pDocument )
    {
        // FIXME: should have a GError parameter and populate it.
        char *pError = pDocView->pOffice->pClass->getError( pDocView->pOffice );
        fprintf( stderr, "Error opening document '%s'\n", pError );
        return FALSE;
    }
    else
    {
        pDocView->pDocument->pClass->initializeForRendering(pDocView->pDocument);
        renderDocument(pDocView, NULL);
    }

    return TRUE;
}

SAL_DLLPUBLIC_EXPORT void lok_docview_set_zoom ( LOKDocView* pDocView, float fZoom )
{
    pDocView->fZoom = fZoom;

    if ( pDocView->pDocument )
    {
        renderDocument(pDocView, NULL);
    }
    // TODO: maybe remember and reset positiong?
}

SAL_DLLPUBLIC_EXPORT float lok_docview_get_zoom ( LOKDocView* pDocView )
{
    return pDocView->fZoom;
}

SAL_DLLPUBLIC_EXPORT int lok_docview_get_parts( LOKDocView* pDocView )
{
    return pDocView->pDocument->pClass->getParts( pDocView->pDocument );
}

SAL_DLLPUBLIC_EXPORT int lok_docview_get_part( LOKDocView* pDocView )
{
    return pDocView->pDocument->pClass->getPart( pDocView->pDocument );
}

SAL_DLLPUBLIC_EXPORT void lok_docview_set_part( LOKDocView* pDocView, int nPart)
{
    pDocView->pDocument->pClass->setPart( pDocView->pDocument, nPart );
    renderDocument(pDocView, NULL);
}

SAL_DLLPUBLIC_EXPORT char* lok_docview_get_part_name( LOKDocView* pDocView, int nPart )
{
    return pDocView->pDocument->pClass->getPartName( pDocView->pDocument, nPart );
}

SAL_DLLPUBLIC_EXPORT void lok_docview_set_partmode( LOKDocView* pDocView,
                                                    int nPartMode )
{
    pDocView->pDocument->pClass->setPartMode( pDocView->pDocument, nPartMode );
    renderDocument(pDocView, NULL);
}

SAL_DLLPUBLIC_EXPORT void lok_docview_set_edit( LOKDocView* pDocView,
                                                gboolean bEdit )
{
    if (!pDocView->m_bEdit && bEdit)
    {
        g_info("lok_docview_set_edit: entering edit mode, registering callback");
        pDocView->pDocument->pClass->registerCallback(pDocView->pDocument, &lok_docview_callback_worker, pDocView);
        g_timeout_add(600, &lcl_handleTimeout, pDocView);
    }
    pDocView->m_bEdit = bEdit;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
