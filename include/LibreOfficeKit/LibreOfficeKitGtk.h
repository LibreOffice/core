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
    GtkScrolledWindow aScrollWindow;
    struct LOKDocView_Impl* m_pImpl;
};

struct _LOKDocViewClass
{
  GtkScrolledWindowClass parent_class;
  void (* edit_changed)  (LOKDocView* pView, gboolean was_edit);
  void (* command_changed) (LOKDocView* pView, char* new_state);
  void (* search_not_found) (LOKDocView* pView, char* new_state);
};

guint           lok_docview_get_type        (void);
GtkWidget*      lok_docview_new             ( LibreOfficeKit* pOffice );
gboolean        lok_docview_open_document   (LOKDocView* pDocView,
                                             char* pPath);

/// Gets the document the viewer displays.
LibreOfficeKitDocument* lok_docview_get_document(LOKDocView* pDocView);

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
/// Gets if the viewer is actually an editor or not.
gboolean        lok_docview_get_edit        (LOKDocView* pDocView);

/// Posts the .uno: command to the LibreOfficeKit.
void            lok_docview_post_command    (LOKDocView* pDocView, const char* pCommand, const char* pArguments);

/// Posts a keyboard event to LibreOfficeKit.
void            lok_docview_post_key    (GtkWidget* pWidget, GdkEventKey* pEvent, gpointer pData);
#ifdef __cplusplus
}
#endif

#endif // INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKITGTK_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
