/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtk/gtk.h>

#include <gtv-application-window.hxx>
#include <gtv-main-toolbar.hxx>

struct _GtvMainToolbar
{
    GtkBox parent;
};

struct GtvMainToolbarPrivate
{
    GtkWidget* toolbar1;
    GtkWidget* toolbar2;

    GtkWidget* btn_save;
    GtkWidget* btn_bold;
};

G_DEFINE_TYPE_WITH_PRIVATE(GtvMainToolbar, gtv_main_toolbar, GTK_TYPE_BOX);

static GtvMainToolbarPrivate*
getPrivate(GtvMainToolbar* toolbar)
{
    return static_cast<GtvMainToolbarPrivate*>(gtv_main_toolbar_get_instance_private(toolbar));
}

static void
btn_clicked(GtkWidget* pWidget, gpointer)
{
    GApplication* app = g_application_get_default();
    GtkWindow* window = gtk_application_get_active_window(GTK_APPLICATION(app));

    GtkToolButton* pItem = GTK_TOOL_BUTTON(pWidget);
    const gchar* unoCmd = gtk_tool_button_get_label(pItem);
    LOKDocView* lokdocview = gtv_application_window_get_lokdocview(GTV_APPLICATION_WINDOW(window));
    if (lokdocview)
    {
        lok_doc_view_post_command(lokdocview, unoCmd, nullptr, false);
    }
}

static void
gtv_main_toolbar_init(GtvMainToolbar* toolbar)
{
    GtkBuilder* builder = gtk_builder_new_from_file("gtv.ui");
    GtvMainToolbarPrivate* priv = getPrivate(toolbar);

    priv->toolbar1 = GTK_WIDGET(gtk_builder_get_object(builder, "toolbar1"));
    gtk_box_pack_start(GTK_BOX(toolbar), priv->toolbar1, false, false, false);
    priv->toolbar2 = GTK_WIDGET(gtk_builder_get_object(builder, "toolbar2"));
    gtk_box_pack_start(GTK_BOX(toolbar), priv->toolbar2, false, false, false);

    priv->btn_save = GTK_WIDGET(gtk_builder_get_object(builder, "btn_save"));
    priv->btn_bold = GTK_WIDGET(gtk_builder_get_object(builder, "btn_bold"));

    gtk_builder_add_callback_symbol(builder, "btn_clicked", G_CALLBACK(btn_clicked));
    gtk_builder_connect_signals(builder, nullptr);

    gtk_widget_show_all(GTK_WIDGET(toolbar));
}

static void
gtv_main_toolbar_class_init(GtvMainToolbarClass*)
{
    // TODO: Use templates to bind objects maybe ?
    // But that requires compiling the .ui file into C source requiring
    // glib-compile-resources (another dependency) as I can't find any gtk
    // method to set the template from the .ui file directly; can only be set
    // from gresource
}

GtkWidget*
gtv_main_toolbar_new()
{
    return GTK_WIDGET(g_object_new(GTV_MAIN_TOOLBAR_TYPE,
                                   "orientation", GTK_ORIENTATION_VERTICAL,
                                   nullptr));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
