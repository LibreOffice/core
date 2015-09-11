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

#include <LibreOfficeKit/LibreOfficeKit.h>

G_BEGIN_DECLS

#define LOK_TYPE_DOC_VIEW            (lok_doc_view_get_type())
#define LOK_DOC_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), LOK_TYPE_DOC_VIEW, LOKDocView))
#define LOK_IS_DOC_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), LOK_TYPE_DOC_VIEW))
#define LOK_DOC_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  LOK_TYPE_DOC_VIEW, LOKDocViewClass))
#define LOK_IS_DOC_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  LOK_TYPE_DOC_VIEW))
#define LOK_DOC_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  LOK_TYPE_DOC_VIEW, LOKDocViewClass))

typedef struct _LOKDocView        LOKDocView;
typedef struct _LOKDocViewClass   LOKDocViewClass;
typedef struct _LOKDocViewPrivate LOKDocViewPrivate;

struct _LOKDocView
{
    GtkDrawingArea aDrawingArea;
};

struct _LOKDocViewClass
{
    GtkDrawingAreaClass parent_class;
};

GType                          lok_doc_view_get_type               (void) G_GNUC_CONST;

GtkWidget*                     lok_doc_view_new                    (const gchar* pPath,
                                                                    GCancellable *cancellable,
                                                                    GError **error);

void                           lok_doc_view_open_document          (LOKDocView* pDocView,
                                                                    const gchar* pPath,
                                                                    GCancellable* cancellable,
                                                                    GAsyncReadyCallback callback,
                                                                    gpointer userdata);

gboolean                       lok_doc_view_open_document_finish   (LOKDocView* pDocView,
                                                                    GAsyncResult* res,
                                                                    GError** error);

/// Gets the document the viewer displays.
LibreOfficeKitDocument*        lok_doc_view_get_document           (LOKDocView* pDocView);

void                           lok_doc_view_set_zoom               (LOKDocView* pDocView,
                                                                    float fZoom);
float                          lok_doc_view_get_zoom               (LOKDocView* pDocView);

int                            lok_doc_view_get_parts              (LOKDocView* pDocView);
int                            lok_doc_view_get_part               (LOKDocView* pDocView);
void                           lok_doc_view_set_part               (LOKDocView* pDocView,
                                                                    int nPart);
char*                          lok_doc_view_get_part_name          (LOKDocView* pDocView,
                                                                    int nPart);
void                           lok_doc_view_set_partmode           (LOKDocView* pDocView,
                                                                    int nPartMode);

void                           lok_doc_view_reset_view             (LOKDocView* pDocView);

/// Sets if the viewer is actually an editor or not.
void                           lok_doc_view_set_edit               (LOKDocView* pDocView,
                                                                    gboolean bEdit);
/// Gets if the viewer is actually an editor or not.
gboolean                       lok_doc_view_get_edit               (LOKDocView* pDocView);

/// Posts the .uno: command to the LibreOfficeKit.
void                           lok_doc_view_post_command           (LOKDocView* pDocView,
                                                                    const gchar* pCommand,
                                                                    const gchar* pArguments);

float                          lok_doc_view_pixel_to_twip          (LOKDocView* pDocView,
                                                                    float fInput);

float                          lok_doc_view_twip_to_pixel          (LOKDocView* pDocView,
                                                                    float fInput);

G_END_DECLS

#endif // INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKITGTK_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
