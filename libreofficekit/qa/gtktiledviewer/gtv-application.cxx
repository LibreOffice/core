/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtk/gtk.h>

#include <gtv-application.hxx>
#include <gtv-application-window.hxx>

struct _GtvApplication
{
    GtkApplication parent;
};

G_DEFINE_TYPE(GtvApplication, gtv_application, GTK_TYPE_APPLICATION);

static void
gtv_application_activate(GApplication* application)
{
    GtvApplicationWindow* win = GTV_APPLICATION_WINDOW(gtv_application_window_new(GTK_APPLICATION(application)));
    gtk_window_present(GTK_WINDOW(win));
}

//gtv_application_open(GApplication* /*win*/, GFile** /*file*/, gint, const gchar*)

static void
gtv_application_init(GtvApplication*)
{
    // TODO
}

static void
gtv_application_class_init(GtvApplicationClass* klass)
{
    G_APPLICATION_CLASS(klass)->activate = gtv_application_activate;
}

GtvApplication* gtv_application_new()
{
    return GTV_APPLICATION(g_object_new(GTV_APPLICATION_TYPE,
                                        "application-id", "org.libreoffice.gtktiledviewer",
                                        nullptr));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
