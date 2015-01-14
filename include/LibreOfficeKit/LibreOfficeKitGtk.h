/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DESKTOP_INC_LIBREOFFICEKITGTK_H
#define INCLUDED_DESKTOP_INC_LIBREOFFICEKITGTK_H

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
                                             LibreOfficeKitPartMode ePartMode);
/// Sets if the viewer is actually an editor or not.
void            lok_docview_set_edit        (LOKDocView* pDocView,
                                             gboolean bEdit);
#ifdef __cplusplus
}
#endif

#endif
