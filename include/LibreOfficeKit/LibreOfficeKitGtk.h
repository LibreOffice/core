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

/**
 * lok_doc_view_new:
 * @pPath: (nullable): LibreOffice install path. Pass null to set it to default
 * path which in most cases would be $libdir/libreoffice/program
 * @cancellable: The cancellable object that you can use to cancel this
 * operation.
 * @error: The error that will be set if the object fails to initialize.
 *
 * Returns: (transfer none): The #LOKDocView widget instance.
 */
GtkWidget*                     lok_doc_view_new                    (const gchar* pPath,
                                                                    GCancellable *cancellable,
                                                                    GError **error);

/**
 * lok_doc_view_new_from_widget:
 * @pDocView: The #LOKDocView instance
 *
 * Returns: (transfer none): The #LOKDocView widget instance.
 */
GtkWidget*                     lok_doc_view_new_from_widget        (LOKDocView* pDocView);

/**
 * lok_doc_view_open_document:
 * @pDocView: The #LOKDocView instance
 * @pPath: (transfer full): The path of the document that #LOKDocView widget should try to open
 * @pRenderingArguments: (nullable): lok::Document::initializeForRendering() arguments.
 * @cancellable:
 * @callback:
 * @userdata:
 *
 * Returns: %TRUE if the document is loaded succesfully, %FALSE otherwise
 */
void                           lok_doc_view_open_document          (LOKDocView* pDocView,
                                                                    const gchar* pPath,
                                                                    const gchar* pRenderingArguments,
                                                                    GCancellable* cancellable,
                                                                    GAsyncReadyCallback callback,
                                                                    gpointer userdata);

/**
 * lok_doc_view_open_document_finish:
 * @pDocView: The #LOKDocView instance
 * @res:
 * @error:
 *
 * Returns: %TRUE if the document is loaded succesfully, %FALSE otherwise
 */
gboolean                       lok_doc_view_open_document_finish   (LOKDocView* pDocView,
                                                                    GAsyncResult* res,
                                                                    GError** error);

/**
 * lok_doc_view_get_document:
 * @pDocView: The #LOKDocView instance
 *
 * Gets the document the viewer displays.
 *
 * Returns: The #LibreOfficeKitDocument instance the widget is currently showing
 */
LibreOfficeKitDocument*        lok_doc_view_get_document           (LOKDocView* pDocView);

/**
 * lok_doc_view_set_zoom:
 * @pDocView: The #LOKDocView instance
 * @fZoom: The new zoom level that pDocView must set it into.
 *
 * Sets the new zoom level for the widget.
 */
void                           lok_doc_view_set_zoom               (LOKDocView* pDocView,
                                                                    float fZoom);

/**
 * lok_doc_view_get_zoom:
 * @pDocView: The #LOKDocView instance
 *
 * Returns: The current zoom factor value in float for pDocView
 */
gfloat                         lok_doc_view_get_zoom               (LOKDocView* pDocView);

/**
 * lok_doc_view_get_parts:
 * @pDocView: The #LOKDocView instance
 *
 * Returns: Part refers to either individual sheets in a Calc, or slides in Impress,
 * and has no relevance for Writer.
 */
gint                           lok_doc_view_get_parts              (LOKDocView* pDocView);

/**
 * lok_doc_view_get_part:
 * @pDocView: The #LOKDocView instance
 *
 * Returns: Current part number of the document
 */
gint                           lok_doc_view_get_part               (LOKDocView* pDocView);

/**
 * lok_doc_view_set_part:
 * @pDocView: The #LOKDocView instance
 * @nPart:
 */
void                           lok_doc_view_set_part               (LOKDocView* pDocView,
                                                                    int nPart);

/**
 * lok_doc_view_get_part_name:
 * @pDocView: The #LOKDocView instance
 * @nPart:
 *
 * Returns: Get current part name of loaded document
 */
gchar*                         lok_doc_view_get_part_name          (LOKDocView* pDocView,
                                                                    int nPart);

/**
 * lok_doc_view_set_partmode:
 * @pDocView: The #LOKDocView instance
 * @nPartMode:
 */
void                           lok_doc_view_set_partmode           (LOKDocView* pDocView,
                                                                    int nPartMode);

/**
 * lok_doc_view_reset_view:
 * @pDocView: The #LOKDocView instance
 */
void                           lok_doc_view_reset_view             (LOKDocView* pDocView);

/**
 * lok_doc_view_set_edit:
 * @pDocView: The #LOKDocView instance
 * @bEdit: %TRUE if the pDocView should go in edit mode, %FALSE otherwise
 *
 * Sets if the viewer is actually an editor or not.
 */
void                           lok_doc_view_set_edit               (LOKDocView* pDocView,
                                                                    gboolean bEdit);

/**
 * lok_doc_view_get_edit:
 * @pDocView: The #LOKDocView instance
 *
 * Gets if the viewer is actually an editor or not.
 *
 * Returns: %TRUE if the given pDocView is in edit mode.
 */
gboolean                       lok_doc_view_get_edit               (LOKDocView* pDocView);

/**
 * lok_doc_view_post_command:
 * @pDocView: the #LOKDocView instance
 * @pCommand: the command to issue to LO core
 * @pArguments: the arguments to the given command
 * @bNotifyWhenFinished: normally false, but it may be useful for eg. .uno:Save
 *
 * Posts the .uno: command to the LibreOfficeKit.
 */
void                           lok_doc_view_post_command           (LOKDocView* pDocView,
                                                                    const gchar* pCommand,
                                                                    const gchar* pArguments,
                                                                    gboolean bNotifyWhenFinished);


/**
 * lok_doc_view_find_next:
 * @pDocView: The #LOKDocView instance
 * @pText: text to search for
 * @bHighlightAll: Whether all the matches should be highlighted or not
 *
 * Highlights the next matching text in the view. `search-not-found` signal will
 * be emitted when no search is found
 */
void                           lok_doc_view_find_next              (LOKDocView* pDocView,
                                                                    const gchar* pText,
                                                                    gboolean bHighlightAll);

/**
 * lok_doc_view_find_prev:
 * @pDocView: The #LOKDocView instance
 * @pText: text to search for
 * @bHighlightAll: Whether all the matches should be highlighted or not
 *
 * Highlights the previous matching text in the view. `search-not-found` signal
 * will be emitted when no search is found
 */
void                           lok_doc_view_find_prev              (LOKDocView* pDocView,
                                                                    const gchar* pText,
                                                                    gboolean bHighlightAll);

/**
 * lok_doc_view_highlight_all:
 * @pDocView: The #LOKDocView instance
 * @pText: text to search for
 *
 * Highlights all matching texts in the view. `search-not-found` signal
 * will be emitted when no search is found
 */
void                           lok_doc_view_highlight_all          (LOKDocView* pDocView,
                                                                    const gchar* pText);

/**
 * lok_doc_view_copy_selection:
 * @pDocView: The #LOKDocView instance
 * @pMimeType: suggests the return format, for example text/plain;charset=utf-8
 * @pUsedMimeType: (out): output parameter to inform about the determined format
 * (suggested or plain text).
 *
 * Returns: Selected text. The caller must free the returned buffer after use.
 */
gchar*                          lok_doc_view_copy_selection        (LOKDocView* pDocView,
                                                                    const gchar* pMimeType,
                                                                    gchar** pUsedMimeType);

/**
 * lok_doc_view_paste:
 * @pDocView: The #LOKDocView instance
 * @pMimeType: format of pData, for example text/plain;charset=utf-8
 * @pData: the data to be pasted
 * @nSize: length of data to be pasted
 *
 * Pastes the content at the current cursor position
 *
 * Returns: if pData was pasted successfully.
 */
gboolean                        lok_doc_view_paste                 (LOKDocView* pDocView,
                                                                    const gchar* pMimeType,
                                                                    const gchar* pData,
                                                                    gsize nSize);

/**
 * lok_doc_view_pixel_to_twip:
 * @pDocView: The #LOKDocView instance
 * @fInput: The value in pixels to convert to twips
 *
 * Converts the value in pixels to twips according to zoom level.
 *
 * Returns: The corresponding value in twips
 */
gfloat                         lok_doc_view_pixel_to_twip          (LOKDocView* pDocView,
                                                                    float fInput);

/**
 * lok_doc_view_twip_to_pixel:
 * @pDocView: The #LOKDocView instance
 * @fInput: The value in twips to convert to pixels
 *
 * Converts the value in twips to pixels according to zoom level.
 *
 * Returns: The corresponding value in pixels
 */
gfloat                         lok_doc_view_twip_to_pixel          (LOKDocView* pDocView,
                                                                    float fInput);

G_END_DECLS

#endif // INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKITGTK_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
