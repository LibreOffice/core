/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_LOK_QA_INC_LIBREOFFICEKITGTK_H
#define INCLUDED_LOK_QA_INC_LIBREOFFICEKITGTK_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#define LOK_USE_UNSTABLE_API
#include <LibreOfficeKit/LibreOfficeKit.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define LOK_DOCVIEW_QUAD(obj)          GTK_CHECK_CAST (obj, lok_docview_quad_get_type(), LOKDocViewQuad)
#define LOK_DOCVIEW_QUAD_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, lok_docview_quad_get_type(), LOKDocViewQuadClass)
#define IS_LOK_DOCVIEW_QUAD(obj)       GTK_CHECK_TYPE (obj, lok_docview_quad_get_type())


typedef struct _LOKDocViewQuad       LOKDocViewQuad;
typedef struct _LOKDocViewQuadClass  LOKDocViewQuadClass;

struct _LOKDocViewQuad
{
    GtkScrolledWindow scrollWindow;

    GtkWidget* pEventBox;
    GtkWidget* pGrid;
    GtkWidget* pCanvas[2][2];
    GdkPixbuf* pPixBuf[2][2];

    float fZoom;

    LibreOfficeKit* pOffice;
    LibreOfficeKitDocument* pDocument;
};

struct _LOKDocViewQuadClass
{
  GtkScrolledWindowClass parent_class;

  void (*lok_docview_quad) (LOKDocViewQuad* pDocView);
};

guint           lok_docview_quad_get_type        (void);
GtkWidget*      lok_docview_quad_new             ( LibreOfficeKit* pOffice );
gboolean        lok_docview_quad_open_document   (LOKDocViewQuad* pDocView,
                                             char* pPath);
void            lok_docview_quad_set_zoom        (LOKDocViewQuad* pDocView,
                                             float fZoom);
float           lok_docview_quad_get_zoom        (LOKDocViewQuad* pDocView);

#ifdef __cplusplus
}
#endif

#endif
