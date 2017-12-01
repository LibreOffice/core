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

#include "gtv-application-window.hxx"
#include "gtv-signal-handlers.hxx"
#include "gtv-helpers.hxx"
#include "gtv-comments-sidebar.hxx"

#include <map>
#include <boost/property_tree/json_parser.hpp>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
G_DEFINE_TYPE(GtvCommentsSidebar, gtv_comments_sidebar, GTK_TYPE_BOX);
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

void
gtv_comments_sidebar_view_annotations(GtvCommentsSidebar* sidebar)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(sidebar)));

    LibreOfficeKitDocument* pDocument = lok_doc_view_get_document(LOK_DOC_VIEW(window->lokdocview));
    char* pValues = pDocument->pClass->getCommandValues(pDocument, ".uno:ViewAnnotations");
    g_info("lok::Document::getCommandValues(%s) : %s", ".uno:ViewAnnotations", pValues);
    std::stringstream aStream(pValues);
    free(pValues);

    // empty the comments grid
    GtvGtkWrapper<GList> children(gtk_container_get_children(GTK_CONTAINER(sidebar->commentsgrid)),
                                  [](GList* l)
                                  {
                                      g_list_free(l);
                                  });
    GList* iter;
    for (iter = children.get(); iter != nullptr; iter = g_list_next(iter))
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    try
    {
        for (boost::property_tree::ptree::value_type& rValue : aTree.get_child("comments"))
        {
            GtkWidget* pCommentBox = GtvHelpers::createCommentBox(rValue.second);
            gtk_container_add(GTK_CONTAINER(sidebar->commentsgrid), pCommentBox);
        }
        gtk_widget_show_all(sidebar->scrolledwindow);
    }
    catch(boost::property_tree::ptree_bad_path& rException)
    {
        std::cerr << "CommentsSidebar::unoViewAnnotations: failed to get comments" << rException.what() << std::endl;
    }
}

static void
gtv_comments_sidebar_view_annotations_cb(GtkWidget* pWidget, gpointer)
{
    GtvCommentsSidebar* sidebar = GTV_COMMENTS_SIDEBAR(pWidget);
    gtv_comments_sidebar_view_annotations(sidebar);
}

static void
gtv_comments_sidebar_init(GtvCommentsSidebar* sidebar)
{
    sidebar->scrolledwindow = gtk_scrolled_window_new(nullptr, nullptr);
    gtk_widget_set_vexpand(sidebar->scrolledwindow, TRUE);
    sidebar->commentsgrid = gtk_grid_new();
    g_object_set(sidebar->commentsgrid, "orientation", GTK_ORIENTATION_VERTICAL, nullptr);

    sidebar->viewannotationsButton = gtk_button_new_with_label(".uno:ViewAnnotations");
    // Hack to make sidebar grid wide enough to not need any horizontal scrollbar
    gtk_widget_set_margin_start(sidebar->viewannotationsButton, 20);
    gtk_widget_set_margin_end(sidebar->viewannotationsButton, 20);
    gtk_container_add(GTK_CONTAINER(sidebar), sidebar->viewannotationsButton);
    g_signal_connect_swapped(sidebar->viewannotationsButton, "clicked", G_CALLBACK(gtv_comments_sidebar_view_annotations_cb), sidebar);

    gtk_container_add(GTK_CONTAINER(sidebar), sidebar->scrolledwindow);
    gtk_container_add(GTK_CONTAINER(sidebar->scrolledwindow), sidebar->commentsgrid);

    gtk_widget_show_all(GTK_WIDGET(sidebar));
}

static void
gtv_comments_sidebar_class_init(GtvCommentsSidebarClass* /*klass*/)
{
}

GtkWidget*
gtv_comments_sidebar_new()
{
    return GTK_WIDGET(g_object_new(GTV_TYPE_COMMENTS_SIDEBAR,
                                   "orientation", GTK_ORIENTATION_VERTICAL,
                                   nullptr));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
