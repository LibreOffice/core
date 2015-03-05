/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKITGTK_H
#define INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKITGTK_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#define LOK_USE_UNSTABLE_API
#include <LibreOfficeKit/LibreOfficeKit.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define LOK_DOCVIEW(obj)          GTK_CHECK_CAST (obj, lok_docview_get_type(), LOKDocView)
#define LOK_DOCVIEW_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, lok_docview_get_type(), LOKDocViewClass)
#define IS_LOK_DOCVIEW(obj)       GTK_CHECK_TYPE (obj, lok_docview_get_type())

typedef struct _LOKDocView       LOKDocView;
typedef struct _LOKDocViewClass  LOKDocViewClass;

struct _LOKDocView
{
    GtkScrolledWindow scrollWindow;

    GtkWidget* pEventBox;
    GtkWidget* pTable;
    GtkWidget** pCanvas;

    float fZoom;

    LibreOfficeKit* pOffice;
    LibreOfficeKitDocument* pDocument;
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
    GList* m_pTextSelectionRectangles;
    /// Position and size of the selection start (as if there would be a cursor caret there).
    GdkRectangle m_aTextSelectionStart;
    /// Position and size of the selection end.
    GdkRectangle m_aTextSelectionEnd;
    GdkRectangle m_aGraphicSelection;

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

    cairo_surface_t* m_pGraphicHandle;
};

struct _LOKDocViewClass
{
  GtkScrolledWindowClass parent_class;
};

guint           lok_docview_get_type        (void);
GtkWidget*      lok_docview_new             ( LibreOfficeKit* pOffice );
gboolean        lok_docview_open_document   (LOKDocView* pDocView,
                                             char* pPath);
void            lok_docview_set_zoom        (LOKDocView* pDocView,
                                             float fZoom);
float           lok_docview_get_zoom        (LOKDocView* pDocView);

int             lok_docview_get_parts       (LOKDocView* pDocView);
int             lok_docview_get_part        (LOKDocView* pDocView);
void            lok_docview_set_part        (LOKDocView* pDocView,
                                             int nPart);
char*           lok_docview_get_part_name   (LOKDocView* pDocView,
                                             int nPart);
void            lok_docview_set_partmode    (LOKDocView* pDocView,
                                             int nPartMode);
/// Sets if the viewer is actually an editor or not.
void            lok_docview_set_edit        (LOKDocView* pDocView,
                                             gboolean bEdit);
#ifdef __cplusplus
}
#endif

#endif // INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKITGTK_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
