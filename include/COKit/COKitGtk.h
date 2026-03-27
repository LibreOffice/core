/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COKIT_COKIT_GTK_H
#define INCLUDED_COKIT_COKIT_GTK_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "COKit.h"

 // Avoid "error C2375: 'foo': redefinition; different linkage" on MSVC
#if defined LOK_DOC_VIEW_IMPLEMENTATION
#include <sal/types.h>
#define LOK_DOC_VIEW_DLLPUBLIC SAL_DLLPUBLIC_EXPORT
#else
#define LOK_DOC_VIEW_DLLPUBLIC
#endif

G_BEGIN_DECLS

#define LOK_TYPE_DOC_VIEW            (lok_doc_view_get_type())
#define LOK_DOC_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), LOK_TYPE_DOC_VIEW, KitDocumentView))
#define LOK_IS_DOC_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), LOK_TYPE_DOC_VIEW))
#define LOK_DOC_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  LOK_TYPE_DOC_VIEW, KitDocumentViewClass))
#define LOK_IS_DOC_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  LOK_TYPE_DOC_VIEW))
#define LOK_DOC_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  LOK_TYPE_DOC_VIEW, KitDocumentViewClass))

typedef struct _KitDocumentView        KitDocumentView;
typedef struct _KitDocumentViewClass   KitDocumentViewClass;
typedef struct _KitDocumentViewPrivate KitDocumentViewPrivate;

struct _KitDocumentView
{
    GtkDrawingArea aDrawingArea;
};

struct _KitDocumentViewClass
{
    GtkDrawingAreaClass parent_class;
};

LOK_DOC_VIEW_DLLPUBLIC GType   lok_doc_view_get_type               (void) G_GNUC_CONST;

/**
 * lok_doc_view_new:
 * @pPath: (nullable) (allow-none): LibreOffice install path. Pass null to set it to default
 * path which in most cases would be $libdir/libreoffice/program
 * @cancellable: The cancellable object that you can use to cancel this
 * operation.
 * @error: The error that will be set if the object fails to initialize.
 *
 * Returns: (transfer none): The #KitDocumentView widget instance.
 */
LOK_DOC_VIEW_DLLPUBLIC GtkWidget* lok_doc_view_new                 (const gchar* pPath,
                                                                    GCancellable *cancellable,
                                                                    GError **error);

/**
 * lok_doc_view_new_from_user_profile:
 * @pPath: (nullable) (allow-none): LibreOffice install path. Pass null to set it to default
 * path which in most cases would be $libdir/libreoffice/program
 * @pUserProfile: (nullable) (allow-none): User profile URL. Must be either a file URL or a
 * special vnd.sun.star.pathname URL. Pass non-null to be able to use this
 * widget and LibreOffice itself in parallel.
 * @cancellable: The cancellable object that you can use to cancel this
 * operation.
 * @error: The error that will be set if the object fails to initialize.
 *
 * Returns: (transfer none): The #KitDocumentView widget instance.
 */
LOK_DOC_VIEW_DLLPUBLIC GtkWidget* lok_doc_view_new_from_user_profile (const gchar* pPath,
                                                                    const gchar* pUserProfile,
                                                                    GCancellable *cancellable,
                                                                    GError **error);

/**
 * lok_doc_view_new_from_widget:
 * @pDocView: The #KitDocumentView instance
 * @pRenderingArguments: (nullable) (allow-none): kit::Document::initializeForRendering() arguments.
 *
 * Returns: (transfer none): The #KitDocumentView widget instance.
 */
LOK_DOC_VIEW_DLLPUBLIC GtkWidget* lok_doc_view_new_from_widget     (KitDocumentView* pDocView,
                                                                    const gchar* pRenderingArguments);

/**
 * lok_doc_view_open_document:
 * @pDocView: The #KitDocumentView instance
 * @pPath: (transfer full): The path of the document that #KitDocumentView widget should try to open
 * @pRenderingArguments: (nullable) (allow-none): kit::Document::initializeForRendering() arguments.
 * @cancellable:
 * @callback:
 * @userdata:
 */
LOK_DOC_VIEW_DLLPUBLIC void    lok_doc_view_open_document          (KitDocumentView* pDocView,
                                                                    const gchar* pPath,
                                                                    const gchar* pRenderingArguments,
                                                                    GCancellable* cancellable,
                                                                    GAsyncReadyCallback callback,
                                                                    gpointer userdata);

/**
 * lok_doc_view_open_document_finish:
 * @pDocView: The #KitDocumentView instance
 * @res:
 * @error:
 *
 * Returns: %TRUE if the document is loaded successfully, %FALSE otherwise
 */
LOK_DOC_VIEW_DLLPUBLIC gboolean lok_doc_view_open_document_finish  (KitDocumentView* pDocView,
                                                                    GAsyncResult* res,
                                                                    GError** error);

/**
 * lok_doc_view_get_document:
 * @pDocView: The #KitDocumentView instance
 *
 * Gets the document the viewer displays.
 *
 * Returns: The #COKitDocument instance the widget is currently showing
 */
LOK_DOC_VIEW_DLLPUBLIC COKitDocument* lok_doc_view_get_document (KitDocumentView* pDocView);

/**
 * lok_doc_view_set_zoom:
 * @pDocView: The #KitDocumentView instance
 * @fZoom: The new zoom level that pDocView must set it into.
 *
 * Sets the new zoom level for the widget. Does nothing if fZoom is equal to
 * existing zoom level. Values outside the range [0.25, 5.0] are clamped into
 * the nearest allowed value in the interval.
 */
LOK_DOC_VIEW_DLLPUBLIC void    lok_doc_view_set_zoom               (KitDocumentView* pDocView,
                                                                    float fZoom);
/**
 * lok_doc_view_set_visible_area:
 * @pDocView: The #KitDocumentView instance
 * @pVisibleArea: The new visible area of pDocView in twips.
 *
 * Sets the new visible area of the widget. This helps e.g. the page down key
 * to jump the correct length, which depends on the amount of visible height of
 * the document.
 */
LOK_DOC_VIEW_DLLPUBLIC void    lok_doc_view_set_visible_area       (KitDocumentView* pDocView,
                                                                    GdkRectangle* pVisibleArea);

/**
 * lok_doc_view_get_zoom:
 * @pDocView: The #KitDocumentView instance
 *
 * Returns: The current zoom factor value in float for pDocView
 */
LOK_DOC_VIEW_DLLPUBLIC gfloat  lok_doc_view_get_zoom               (KitDocumentView* pDocView);

/**
 * lok_doc_view_get_parts:
 * @pDocView: The #KitDocumentView instance
 *
 * Returns: Part refers to either individual sheets in a Calc, or slides in Impress,
 * and has no relevance for Writer. Returns -1 if no document is set currently.
 */
LOK_DOC_VIEW_DLLPUBLIC gint    lok_doc_view_get_parts              (KitDocumentView* pDocView);

/**
 * lok_doc_view_get_part:
 * @pDocView: The #KitDocumentView instance
 *
 * Returns: Current part number of the document. Returns -1 if no document is set currently.
 */
LOK_DOC_VIEW_DLLPUBLIC gint    lok_doc_view_get_part               (KitDocumentView* pDocView);

/**
 * lok_doc_view_set_part:
 * @pDocView: The #KitDocumentView instance
 * @nPart:
 */
LOK_DOC_VIEW_DLLPUBLIC void    lok_doc_view_set_part               (KitDocumentView* pDocView,
                                                                    int nPart);

/**
 * lok_doc_view_get_part_name:
 * @pDocView: The #KitDocumentView instance
 * @nPart:
 *
 * Returns: Get current part name of loaded document. Returns null if no
 * document is set, or document has been destroyed using lok_doc_view_destroy_document.
 */
LOK_DOC_VIEW_DLLPUBLIC gchar*  lok_doc_view_get_part_name          (KitDocumentView* pDocView,
                                                                    int nPart);

/**
 * lok_doc_view_set_partmode:
 * @pDocView: The #KitDocumentView instance
 * @nPartMode:
 */
LOK_DOC_VIEW_DLLPUBLIC void    lok_doc_view_set_partmode           (KitDocumentView* pDocView,
                                                                    int nPartMode);

/**
 * lok_doc_view_reset_view:
 * @pDocView: The #KitDocumentView instance
 */
LOK_DOC_VIEW_DLLPUBLIC void    lok_doc_view_reset_view             (KitDocumentView* pDocView);

/**
 * lok_doc_view_set_edit:
 * @pDocView: The #KitDocumentView instance
 * @bEdit: %TRUE if the pDocView should go in edit mode, %FALSE otherwise
 *
 * Sets if the viewer is actually an editor or not.
 */
LOK_DOC_VIEW_DLLPUBLIC void    lok_doc_view_set_edit               (KitDocumentView* pDocView,
                                                                    gboolean bEdit);

/**
 * lok_doc_view_get_edit:
 * @pDocView: The #KitDocumentView instance
 *
 * Gets if the viewer is actually an editor or not.
 *
 * Returns: %TRUE if the given pDocView is in edit mode.
 */
LOK_DOC_VIEW_DLLPUBLIC gboolean lok_doc_view_get_edit              (KitDocumentView* pDocView);

/**
 * lok_doc_view_post_command:
 * @pDocView: the #KitDocumentView instance
 * @pCommand: the command to issue to LO core
 * @pArguments: the arguments to the given command
 * @bNotifyWhenFinished: normally false, but it may be useful for eg. .uno:Save
 *
 * Posts the .uno: command to the COKit.
 */
LOK_DOC_VIEW_DLLPUBLIC void    lok_doc_view_post_command           (KitDocumentView* pDocView,
                                                                    const gchar* pCommand,
                                                                    const gchar* pArguments,
                                                                    gboolean bNotifyWhenFinished);

/**
 * lok_doc_view_get_command_values:
 * @pDocView: the #KitDocumentView instance
 * @pCommand: the command to issue to LO core
 *
 * Get a json mapping of the possible values for the given command.
 * In the form of: {commandName: unoCmd, commandValues: {possible_values}}
 *
 * Returns: A json mapping of the possible values for the given command
 */
LOK_DOC_VIEW_DLLPUBLIC gchar * lok_doc_view_get_command_values     (KitDocumentView* pDocView,
                                                                    const gchar* pCommand);

/**
 * lok_doc_view_find_next:
 * @pDocView: The #KitDocumentView instance
 * @pText: text to search for
 * @bHighlightAll: Whether all the matches should be highlighted or not
 *
 * Highlights the next matching text in the view. `search-not-found` signal will
 * be emitted when no search is found
 */
LOK_DOC_VIEW_DLLPUBLIC void    lok_doc_view_find_next              (KitDocumentView* pDocView,
                                                                    const gchar* pText,
                                                                    gboolean bHighlightAll);

/**
 * lok_doc_view_find_prev:
 * @pDocView: The #KitDocumentView instance
 * @pText: text to search for
 * @bHighlightAll: Whether all the matches should be highlighted or not
 *
 * Highlights the previous matching text in the view. `search-not-found` signal
 * will be emitted when no search is found
 */
LOK_DOC_VIEW_DLLPUBLIC void    lok_doc_view_find_prev              (KitDocumentView* pDocView,
                                                                    const gchar* pText,
                                                                    gboolean bHighlightAll);

/**
 * lok_doc_view_highlight_all:
 * @pDocView: The #KitDocumentView instance
 * @pText: text to search for
 *
 * Highlights all matching texts in the view. `search-not-found` signal
 * will be emitted when no search is found
 */
LOK_DOC_VIEW_DLLPUBLIC void    lok_doc_view_highlight_all          (KitDocumentView* pDocView,
                                                                    const gchar* pText);

/**
 * lok_doc_view_copy_selection:
 * @pDocView: The #KitDocumentView instance
 * @pMimeType: suggests the return format, for example text/plain;charset=utf-8
 * @pUsedMimeType: (out): output parameter to inform about the determined format
 * (suggested or plain text).
 *
 * Returns: Selected text. The caller must free the returned buffer after
 * use. Returns null if no document is set.
 */
LOK_DOC_VIEW_DLLPUBLIC gchar*   lok_doc_view_copy_selection        (KitDocumentView* pDocView,
                                                                    const gchar* pMimeType,
                                                                    gchar** pUsedMimeType);

/**
 * lok_doc_view_paste:
 * @pDocView: The #KitDocumentView instance
 * @pMimeType: format of pData, for example text/plain;charset=utf-8
 * @pData: the data to be pasted
 * @nSize: length of data to be pasted
 *
 * Pastes the content at the current cursor position
 *
 * Returns: if pData was pasted successfully.
 */
LOK_DOC_VIEW_DLLPUBLIC gboolean lok_doc_view_paste                 (KitDocumentView* pDocView,
                                                                    const gchar* pMimeType,
                                                                    const gchar* pData,
                                                                    gsize nSize);

/**
 * lok_doc_view_set_document_password:
 * @pDocView: The #KitDocumentView instance
 * @pURL: the URL of the document to set password for, as sent with signal `password-required`
 * @pPassword: (nullable) (allow-none): the password, NULL for no password
 *
 * Set the password for password protected documents
 */
LOK_DOC_VIEW_DLLPUBLIC void     lok_doc_view_set_document_password (KitDocumentView* pDocView,
                                                                    const gchar* pURL,
                                                                    const gchar* pPassword);

/**
 * lok_doc_view_get_version_info:
 * @pDocView: The #KitDocumentView instance
 *
 * Get version information of the LOKit process
 *
 * Returns: JSON string containing version information in format:
 * {ProductName: <>, ProductVersion: <>, ProductExtension: <>, BuildId: <>}
 *
 * Eg: {"ProductName": "LibreOffice",
 * "ProductVersion": "5.3",
 * "ProductExtension": ".0.0.alpha0",
 * "BuildId": "<full 40 char git hash>"}
 */
LOK_DOC_VIEW_DLLPUBLIC gchar*   lok_doc_view_get_version_info       (KitDocumentView* pDocView);

/**
 * lok_doc_view_pixel_to_twip:
 * @pDocView: The #KitDocumentView instance
 * @fInput: The value in pixels to convert to twips
 *
 * Converts the value in pixels to twips according to zoom level.
 *
 * Returns: The corresponding value in twips
 */
LOK_DOC_VIEW_DLLPUBLIC gfloat  lok_doc_view_pixel_to_twip          (KitDocumentView* pDocView,
                                                                    float fInput);

/**
 * lok_doc_view_twip_to_pixel:
 * @pDocView: The #KitDocumentView instance
 * @fInput: The value in twips to convert to pixels
 *
 * Converts the value in twips to pixels according to zoom level.
 *
 * Returns: The corresponding value in pixels
 */
LOK_DOC_VIEW_DLLPUBLIC gfloat  lok_doc_view_twip_to_pixel          (KitDocumentView* pDocView,
                                                                    float fInput);

/**
 * lok_doc_view_send_content_control_event:
 * @pDocView: The #KitDocumentView instance
 * @pArguments: (nullable) (allow-none): see kit::Document::sendContentControlEvent() for the details.
 */
LOK_DOC_VIEW_DLLPUBLIC void lok_doc_view_send_content_control_event(KitDocumentView* pDocView, const gchar* pArguments);

G_END_DECLS

#endif // INCLUDED_COKIT_COKIT_GTK_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
