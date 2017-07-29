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

#include <LibreOfficeKit/LibreOfficeKitGtk.h>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <gtv-application-window.hxx>
#include <gtv-signal-handlers.hxx>
#include <gtv-helpers.hxx>
#include <gtv-lok-dialog.hxx>

#include <com/sun/star/awt/Key.hpp>
#include <vcl/event.hxx>

#include <map>
#include <boost/property_tree/json_parser.hpp>

struct GtvLokDialogPrivate
{
    LOKDocView* lokdocview;
    GtkWidget* pDialogDrawingArea;

    guint32 m_nLastButtonPressTime;
    guint32 m_nLastButtonReleaseTime;
    guint32 m_nKeyModifier;
    guint32 m_nLastButtonPressed;

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

static float
pixelToTwip(float fInput)
{
    return (fInput / 96 / 1.0 /* zoom */) * 1440.0f;
}

#if 0
static float
twipToPixel(float fInput)
{
    return fInput / 1440.0f * 96 * 1.0 /* zoom */;
}
#endif

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

static gboolean
gtv_lok_dialog_signal_button(GtkWidget* pDialogDrawingArea, GdkEventButton* pEvent)
{
    GtvLokDialog* pDialog = GTV_LOK_DIALOG(gtk_widget_get_toplevel(pDialogDrawingArea));
    GtvLokDialogPrivate* priv = getPrivate(pDialog);

    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_window_get_transient_for(GTK_WINDOW(pDialog)));
    LibreOfficeKitDocument* pDocument = lok_doc_view_get_document(LOK_DOC_VIEW(window->lokdocview));

    g_info("lok_dialog_signal_button: %d, %d (in twips: %d, %d)",
           (int)pEvent->x, (int)pEvent->y,
           (int)pixelToTwip(pEvent->x),
           (int)pixelToTwip(pEvent->y));
    gtk_widget_grab_focus(GTK_WIDGET(pDialog));

    switch (pEvent->type)
    {
    case GDK_BUTTON_PRESS:
    {
        int nCount = 1;
        if ((pEvent->time - priv->m_nLastButtonPressTime) < 250)
            nCount++;
        priv->m_nLastButtonPressTime = pEvent->time;
        int nEventButton = 0;
        switch (pEvent->button)
        {
        case 1:
            nEventButton = MOUSE_LEFT;
            break;
        case 2:
            nEventButton = MOUSE_MIDDLE;
            break;
        case 3:
            nEventButton = MOUSE_RIGHT;
            break;
        }
        priv->m_nLastButtonPressed = nEventButton;
        pDocument->pClass->postDialogMouseEvent(pDocument,
                                                priv->dialogid,
                                                LOK_MOUSEEVENT_MOUSEBUTTONDOWN,
                                                (pEvent->x),
                                                (pEvent->y),
                                                nCount,
                                                nEventButton,
                                                0/* Modifier */);

        break;
    }
    case GDK_BUTTON_RELEASE:
    {
        int nCount = 1;
        if ((pEvent->time - priv->m_nLastButtonReleaseTime) < 250)
            nCount++;
        priv->m_nLastButtonReleaseTime = pEvent->time;
        int nEventButton = 0;
        switch (pEvent->button)
        {
        case 1:
            nEventButton = MOUSE_LEFT;
            break;
        case 2:
            nEventButton = MOUSE_MIDDLE;
            break;
        case 3:
            nEventButton = MOUSE_RIGHT;
            break;
        }
        priv->m_nLastButtonPressed = nEventButton;
        pDocument->pClass->postDialogMouseEvent(pDocument,
                                                priv->dialogid,
                                                LOK_MOUSEEVENT_MOUSEBUTTONUP,
                                                (pEvent->x),
                                                (pEvent->y),
                                                nCount,
                                                nEventButton,
                                                0/* Modifier */);
        break;
    }
    default:
        break;
    }
    return FALSE;
}

static gboolean
gtv_lok_dialog_signal_motion(GtkWidget* pDialogDrawingArea, GdkEventButton* pEvent)
{
    GtvLokDialog* pDialog = GTV_LOK_DIALOG(gtk_widget_get_toplevel(pDialogDrawingArea));
    GtvLokDialogPrivate* priv = getPrivate(pDialog);

    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_window_get_transient_for(GTK_WINDOW(pDialog)));
    LibreOfficeKitDocument* pDocument = lok_doc_view_get_document(LOK_DOC_VIEW(window->lokdocview));

    g_info("lok_dialog_signal_button: %d, %d (in twips: %d, %d)",
           (int)pEvent->x, (int)pEvent->y,
           (int)pixelToTwip(pEvent->x),
           (int)pixelToTwip(pEvent->y));
    gtk_widget_grab_focus(GTK_WIDGET(pDialog));

    pDocument->pClass->postDialogMouseEvent(pDocument,
                                            priv->dialogid,
                                            LOK_MOUSEEVENT_MOUSEMOVE,
                                            (pEvent->x),
                                            (pEvent->y),
                                            1,
                                            priv->m_nLastButtonPressed,
                                            0/* Modifier */);

    return FALSE;
}

static void
gtv_lok_dialog_init(GtvLokDialog* dialog)
{
    GtvLokDialogPrivate* priv = getPrivate(dialog);

    GtkWidget* pContentArea = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    priv->pDialogDrawingArea = gtk_drawing_area_new();

    priv->m_nLastButtonPressTime = 0;
    priv->m_nLastButtonReleaseTime = 0;
    priv->m_nKeyModifier = 0;
    priv->m_nLastButtonPressed = 0;

    gtk_widget_add_events(GTK_WIDGET(priv->pDialogDrawingArea),
                          GDK_BUTTON_PRESS_MASK
                          |GDK_BUTTON_RELEASE_MASK
                          |GDK_BUTTON_MOTION_MASK);

    g_signal_connect(G_OBJECT(priv->pDialogDrawingArea), "draw", G_CALLBACK(gtv_lok_dialog_draw), nullptr);
    g_signal_connect(G_OBJECT(priv->pDialogDrawingArea), "button-press-event", G_CALLBACK(gtv_lok_dialog_signal_button), nullptr);
    g_signal_connect(G_OBJECT(priv->pDialogDrawingArea), "button-release-event", G_CALLBACK(gtv_lok_dialog_signal_button), nullptr);
    g_signal_connect(G_OBJECT(priv->pDialogDrawingArea), "motion-notify-event", G_CALLBACK(gtv_lok_dialog_signal_motion), nullptr);
    gtk_container_add(GTK_CONTAINER(pContentArea), priv->pDialogDrawingArea);
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

void
gtv_lok_dialog_invalidate(GtvLokDialog* dialog)
{
    // trigger a draw on the drawing area
    GtvLokDialogPrivate* priv = getPrivate(dialog);
    gtk_widget_queue_draw(priv->pDialogDrawingArea);
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
