/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GTV_APPLICATION_WINDOW_H
#define GTV_APPLICATION_WINDOW_H

#include <gtk/gtk.h>

#include <LibreOfficeKit/LibreOfficeKitGtk.h>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <gtv-main-toolbar.hxx>

#include <string>

struct _GtvApplicationWindow
{
    GtkApplicationWindow parent_instance;

    GtkWidget* scrolledwindow;
    GtkWidget* lokdocview;
    LibreOfficeKitDocumentType doctype;

    GtkWidget* rowbar;
    GtkWidget* columnbar;
    GtkWidget* cornerarea;

    GtkWidget* commentssidebar;
    GtkWidget* statusbar;
    GtkWidget* zoomlabel;
    GtkWidget* redlinelabel;
    GtkWidget* findbarlabel;
    GtkWidget* findbarEntry;
    GtkWidget* findAll;

    GtkWidget* findtoolbar;
};

struct GtvRenderingArgs
{
    std::string m_aLoPath;
    std::string m_aUserProfile;
    bool m_bEnableTiledAnnotations;

    std::string m_aBackgroundColor;
    bool m_bHidePageShadow;
    bool m_bHideWhiteSpace;
};

G_BEGIN_DECLS

#define GTV_APPLICATION_WINDOW_TYPE (gtv_application_window_get_type())
G_DECLARE_FINAL_TYPE(GtvApplicationWindow, gtv_application_window, GTV, APPLICATION_WINDOW, GtkApplicationWindow);


GtvApplicationWindow* gtv_application_window_new(GtkApplication* application);

void gtv_application_window_load_document(GtvApplicationWindow* application,
                                          const GtvRenderingArgs* aArgs,
                                          const std::string& aDocPath);

void gtv_application_window_create_view_from_window(GtvApplicationWindow* window);

void gtv_application_window_get_visible_area(GtvApplicationWindow* pWindow, GdkRectangle* pArea);

void gtv_application_window_toggle_findbar(GtvApplicationWindow* window);

GtkToolItem* gtv_application_window_find_tool_by_unocommand(GtvApplicationWindow* window, const std::string& unoCmd);

GtvMainToolbar* gtv_application_window_get_main_toolbar(GtvApplicationWindow* window);

void gtv_application_window_set_toolbar_broadcast(GtvApplicationWindow* window, bool broadcast);

gboolean gtv_application_window_get_toolbar_broadcast(GtvApplicationWindow* window);

void gtv_application_window_set_part_broadcast(GtvApplicationWindow* window, bool broadcast);

gboolean gtv_application_window_get_part_broadcast(GtvApplicationWindow* window);

G_END_DECLS

#endif /* GTV_APPLICATION_WINDOW_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
