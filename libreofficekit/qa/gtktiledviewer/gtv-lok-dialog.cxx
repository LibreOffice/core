/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtk/gtk.h>

#include <cmath>
#include <iostream>

#include <gtv-application-window.hxx>
#include <gtv-signal-handlers.hxx>
#include <gtv-helpers.hxx>
#include <gtv-lok-dialog.hxx>

#include <map>
#include <boost/property_tree/json_parser.hpp>

struct GtvLokDialogPrivate
{
    LOKDocView* lokdocview;
    gchar* dialogid;
};

G_DEFINE_TYPE_WITH_PRIVATE(GtvLokDialog, gtv_lok_dialog, GTK_TYPE_DIALOG);

enum
{
    PROP_0,
    PROP_LOKDOCVIEW_CONTEXT,
    PROP_DIALOG_ID,
    PROP_LAST
};

static GParamSpec* properties[PROP_LAST];

static GtvLokDialogPrivate*
getPrivate(GtvLokDialog* dialog)
{
    return static_cast<GtvLokDialogPrivate*>(gtv_lok_dialog_get_instance_private(dialog));
}

static void
gtv_lok_dialog_draw(GtkWidget* pDialogDrawingArea, cairo_t* pCairo, gpointer)
{
    GtvLokDialog* pDialog = GTV_LOK_DIALOG(gtk_widget_get_toplevel(pDialogDrawingArea));
    GtvLokDialogPrivate* priv = getPrivate(pDialog);

    int nWidth = 1024;
    int nHeight = 768;
    cairo_surface_t* pSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, nWidth, nHeight);
    unsigned char* pBuffer = cairo_image_surface_get_data(pSurface);
    LibreOfficeKitDocument* pDocument = lok_doc_view_get_document(LOK_DOC_VIEW(priv->lokdocview));
    pDocument->pClass->paintDialog(pDocument, priv->dialogid, pBuffer, &nWidth, &nHeight);
    gtk_widget_set_size_request(GTK_WIDGET(pDialogDrawingArea), nWidth, nHeight);

    cairo_surface_flush(pSurface);
    cairo_surface_mark_dirty(pSurface);

    cairo_set_source_surface(pCairo, pSurface, 0, 0);
    cairo_paint(pCairo);
}

static void
gtv_lok_dialog_init(GtvLokDialog* dialog)
{
    GtkWidget* pContentArea = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget* pDialogDrawingArea = gtk_drawing_area_new();

    g_signal_connect(G_OBJECT(pDialogDrawingArea), "draw", G_CALLBACK(gtv_lok_dialog_draw), nullptr);
    gtk_container_add(GTK_CONTAINER(pContentArea), pDialogDrawingArea);
}

static void
gtv_lok_dialog_set_property(GObject* object, guint propId, const GValue* value, GParamSpec* pspec)
{
    GtvLokDialog* self = GTV_LOK_DIALOG(object);
    GtvLokDialogPrivate* priv = getPrivate(self);

    switch(propId)
    {
    case PROP_LOKDOCVIEW_CONTEXT:
        priv->lokdocview = LOK_DOC_VIEW(g_value_get_object(value));
        break;
    case PROP_DIALOG_ID:
        priv->dialogid = g_value_dup_string(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, propId, pspec);
    }
}

static void
gtv_lok_dialog_get_property(GObject* object, guint propId, GValue* value, GParamSpec* pspec)
{
    GtvLokDialog* self = GTV_LOK_DIALOG(object);
    GtvLokDialogPrivate* priv = getPrivate(self);

    switch(propId)
    {
    case PROP_LOKDOCVIEW_CONTEXT:
        g_value_set_object(value, priv->lokdocview);
        break;
    case PROP_DIALOG_ID:
        g_value_set_string(value, priv->dialogid);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, propId, pspec);
    }
}

static void
gtv_lok_dialog_finalize(GObject* object)
{
    GtvLokDialog* self = GTV_LOK_DIALOG(object);
    GtvLokDialogPrivate* priv = getPrivate(self);

    g_free(priv->dialogid);

    G_OBJECT_CLASS(gtv_lok_dialog_parent_class)->finalize(object);
}

static void
gtv_lok_dialog_class_init(GtvLokDialogClass* klass)
{
    G_OBJECT_CLASS(klass)->get_property = gtv_lok_dialog_get_property;
    G_OBJECT_CLASS(klass)->set_property = gtv_lok_dialog_set_property;
    G_OBJECT_CLASS(klass)->finalize = gtv_lok_dialog_finalize;

    properties[PROP_LOKDOCVIEW_CONTEXT] = g_param_spec_object("lokdocview",
                                                              "LOKDocView Context",
                                                              "The LOKDocView context object to be used for dialog rendering",
                                                              LOK_TYPE_DOC_VIEW,
                                                              static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                                                       G_PARAM_CONSTRUCT_ONLY |
                                                                                       G_PARAM_STATIC_STRINGS));

    properties[PROP_DIALOG_ID] = g_param_spec_string("dialogid",
                                                     "Dialog identifier",
                                                     "Unique dialog identifier; UNO command for now",
                                                     nullptr,
                                                     static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                                              G_PARAM_CONSTRUCT_ONLY |
                                                                              G_PARAM_STATIC_STRINGS));

    g_object_class_install_properties (G_OBJECT_CLASS(klass), PROP_LAST, properties);
}

GtkWidget*
gtv_lok_dialog_new(LOKDocView* pDocView, const gchar* dialogId)
{
    GtkWindow* pWindow = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(pDocView)));
    return GTK_WIDGET(g_object_new(GTV_TYPE_LOK_DIALOG,
                                   "lokdocview", pDocView,
                                   "dialogid", dialogId,
                                   "title", "LOK Dialog",
                                   "modal", false,
                                   "transient-for", pWindow,
                                   nullptr));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
