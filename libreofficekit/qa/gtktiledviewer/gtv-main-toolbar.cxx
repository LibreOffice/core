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
#include <gtv-signal-handlers.hxx>
#include <gtv-helpers.hxx>

#include <map>
#include <boost/property_tree/json_parser.hpp>
#include <boost/optional.hpp>

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
gtv_main_toolbar_init(GtvMainToolbar* toolbar)
{
    GtkBuilder* builder = gtk_builder_new_from_file("gtv.ui");
    GtvMainToolbarPrivate* priv = getPrivate(toolbar);

    priv->toolbar1 = GTK_WIDGET(gtk_builder_get_object(builder, "toolbar1"));
    gtk_box_pack_start(GTK_BOX(toolbar), priv->toolbar1, false, false, false);
    priv->toolbar2 = GTK_WIDGET(gtk_builder_get_object(builder, "toolbar2"));
    gtk_box_pack_start(GTK_BOX(toolbar), priv->toolbar2, false, false, false);

    gtk_builder_add_callback_symbol(builder, "btn_clicked", G_CALLBACK(btn_clicked));
    gtk_builder_add_callback_symbol(builder, "doCopy", G_CALLBACK(doCopy));
    gtk_builder_add_callback_symbol(builder, "doPaste", G_CALLBACK(doPaste));
    gtk_builder_add_callback_symbol(builder, "createView", G_CALLBACK(createView));
    gtk_builder_add_callback_symbol(builder, "unoCommandDebugger", G_CALLBACK(unoCommandDebugger));
    gtk_builder_add_callback_symbol(builder, "toggleEditing", G_CALLBACK(toggleEditing));
//    gtk_builder_add_callback_symbol(builder, "changePartMode", G_CALLBACK(changePartMode));
//    gtk_builder_add_callback_symbol(builder, "changePart", G_CALLBACK(changePart));
    gtk_builder_add_callback_symbol(builder, "changeZoom", G_CALLBACK(changeZoom));
    gtk_builder_add_callback_symbol(builder, "toggleFindbar", G_CALLBACK(toggleFindbar));
    gtk_builder_add_callback_symbol(builder, "documentRedline", G_CALLBACK(documentRedline));
    gtk_builder_add_callback_symbol(builder, "documentRepair", G_CALLBACK(documentRepair));

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
