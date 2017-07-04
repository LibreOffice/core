/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtk/gtk.h>

#include <LibreOfficeKit/LibreOfficeKitGtk.h>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <gtv-application-window.hxx>
#include <gtv-main-toolbar.hxx>

struct _GtvApplicationWindow
{
    GtkApplicationWindow parent_instance;
};

struct GtvApplicationWindowPrivate
{
    GtkWidget* container;
    GtkWidget* toolbarcontainer;

    GtkWidget* scrolledwindow;
    GtkWidget* lokdocview;
    GtkWidget* statusbar;
};

G_DEFINE_TYPE_WITH_PRIVATE(GtvApplicationWindow, gtv_application_window, GTK_TYPE_APPLICATION_WINDOW);

static GtvApplicationWindowPrivate*
getPrivate(GtvApplicationWindow* win)
{
    return static_cast<GtvApplicationWindowPrivate*>(gtv_application_window_get_instance_private(win));
}

static void
gtv_application_window_init(GtvApplicationWindow* win)
{
    GtkBuilder* builder = gtk_builder_new_from_file("gtv.ui");
    GtvApplicationWindowPrivate* priv = getPrivate(win);

    // This is the parent GtkBox holding everything
    priv->container = GTK_WIDGET(gtk_builder_get_object(builder, "container"));
    // Toolbar container
    priv->toolbarcontainer = gtv_main_toolbar_new();

    // Attach to the toolbar to main window
    gtk_box_pack_start(GTK_BOX(priv->container), priv->toolbarcontainer, false, false, false);
    gtk_box_reorder_child(GTK_BOX(priv->container), priv->toolbarcontainer, 0);

    // scrolled window containing the main drawing area
    priv->scrolledwindow = GTK_WIDGET(gtk_builder_get_object(builder, "scrolledwindow"));
    // give life to lokdocview
    priv->lokdocview = lok_doc_view_new_from_user_profile(pLOPath, pUserProfile, nullptr, nullptr);

    // statusbar
    priv->statusbar = GTK_WIDGET(gtk_builder_get_object(builder, "statusbar"));

    gtk_container_add(GTK_CONTAINER(win), priv->container);

    g_object_unref(builder);
}

static void
gtv_application_window_class_init(GtvApplicationWindowClass*)
{
    // TODO: Use templates to bind objects maybe ?
    // But that requires compiling the .ui file into C source requiring
    // glib-compile-resources (another dependency) as I can't find any gtk
    // method to set the template from the .ui file directly; can only be set
    // from gresource
}

GtvApplicationWindow*
gtv_application_window_new(GtkApplication* app)
{
    g_return_val_if_fail(GTK_IS_APPLICATION(app), nullptr);

    return GTV_APPLICATION_WINDOW(g_object_new(GTV_APPLICATION_WINDOW_TYPE,
                                               "application", app,
                                               "width-request", 1024,
                                               "height-request", 768,
                                               "title", "LibreOffice GtkTiledViewer",
                                               "window-position", GTK_WIN_POS_CENTER,
                                               "show-menubar", false,
                                               nullptr));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
