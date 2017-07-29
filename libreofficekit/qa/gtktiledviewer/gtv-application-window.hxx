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

struct GtvRenderingArgs
{
    std::string m_aLoPath;
    std::string m_aUserProfile;
    bool m_bEnableTiledAnnotations;

    std::string m_aBackgroundColor;
    bool m_bHidePageShadow;
    bool m_bHideWhiteSpace;

    GtvRenderingArgs()
        : m_bEnableTiledAnnotations(false),
          m_bHidePageShadow(false),
          m_bHideWhiteSpace(false)
        { }
};

G_BEGIN_DECLS

#define GTV_TYPE_APPLICATION_WINDOW            (gtv_application_window_get_type())
#define GTV_APPLICATION_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GTV_TYPE_APPLICATION_WINDOW, GtvApplicationWindow))
#define GTV_IS_APPLICATION_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTV_TYPE_APPLICATION_WINDOW))
#define GTV_APPLICATION_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  GTV_TYPE_APPLICATION_WINDOW, GtvApplicationWindowClass))
#define GTV_IS_APPLICATION_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  GTV_TYPE_APPLICATION_WINDOW))
#define GTV_APPLICATION_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  GTV_TYPE_APPLICATION_WINDOW, GtvApplicationWindowClass))

struct GtvApplicationWindow
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

struct GtvApplicationWindowClass
{
    GtkApplicationWindowClass parentClass;
};

GType gtv_application_window_get_type               (void) G_GNUC_CONST;

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

void gtv_application_window_register_child_window(GtvApplicationWindow* window, GtkWindow* pChildWin);

void gtv_application_window_unregister_child_window(GtvApplicationWindow* window, GtkWindow* pChildWin);

GtkWindow* gtv_application_window_get_child_window_by_id(GtvApplicationWindow* window, const gchar* pWinId);

GList* gtv_application_window_get_all_child_windows(GtvApplicationWindow* window);

G_END_DECLS

#endif /* GTV_APPLICATION_WINDOW_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
