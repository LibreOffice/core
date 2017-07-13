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
#include <gtv-comments-sidebar.hxx>

#include <map>
#include <boost/property_tree/json_parser.hpp>
#include <boost/optional.hpp>

G_DEFINE_TYPE(GtvCommentsSidebar, gtv_comments_sidebar, GTK_TYPE_BOX);

static GtkWidget*
gtv_comments_sidebar_create_comment_box(const boost::property_tree::ptree& aComment)
{
    GtkWidget* pCommentVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
    gchar *id = g_strndup(aComment.get<std::string>("id").c_str(), 20);
    g_object_set_data_full(G_OBJECT(pCommentVBox), "id", id, g_free);

    // Set background if its a reply comment
    if (aComment.get("parent", -1) > 0)
    {
        GtkStyleContext* pStyleContext = gtk_widget_get_style_context(pCommentVBox);
        GtkCssProvider* pCssProvider = gtk_css_provider_get_default();
        gtk_style_context_add_class(pStyleContext, "commentbox");
        gtk_style_context_add_provider(pStyleContext, GTK_STYLE_PROVIDER(pCssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        gtk_css_provider_load_from_data(pCssProvider, ".commentbox {background-color: lightgreen;}", -1, nullptr);
    }

    GtkWidget* pCommentText = gtk_label_new(aComment.get<std::string>("text").c_str());
    GtkWidget* pCommentAuthor = gtk_label_new(aComment.get<std::string>("author").c_str());
    GtkWidget* pCommentDate = gtk_label_new(aComment.get<std::string>("dateTime").c_str());
    GtkWidget* pControlsHBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget* pEditButton = gtk_button_new_with_label("Edit");
    GtkWidget* pReplyButton = gtk_button_new_with_label("Reply");
    GtkWidget* pDeleteButton = gtk_button_new_with_label("Delete");
    g_signal_connect(G_OBJECT(pEditButton), "clicked", G_CALLBACK(editButtonClicked), pCommentVBox);
    g_signal_connect(G_OBJECT(pReplyButton), "clicked", G_CALLBACK(replyButtonClicked), pCommentVBox);
    g_signal_connect(G_OBJECT(pDeleteButton), "clicked", G_CALLBACK(deleteCommentButtonClicked), pCommentVBox);

    gtk_container_add(GTK_CONTAINER(pControlsHBox), pEditButton);
    gtk_container_add(GTK_CONTAINER(pControlsHBox), pReplyButton);
    gtk_container_add(GTK_CONTAINER(pControlsHBox), pDeleteButton);
    GtkWidget* pCommentSeparator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);

    gtk_container_add(GTK_CONTAINER(pCommentVBox), pCommentText);
    gtk_container_add(GTK_CONTAINER(pCommentVBox), pCommentAuthor);
    gtk_container_add(GTK_CONTAINER(pCommentVBox), pCommentDate);
    gtk_container_add(GTK_CONTAINER(pCommentVBox), pControlsHBox);
    gtk_container_add(GTK_CONTAINER(pCommentVBox), pCommentSeparator);

    gtk_label_set_line_wrap(GTK_LABEL(pCommentText), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(pCommentText), 35);

    return pCommentVBox;
}

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
    GList* children = gtk_container_get_children(GTK_CONTAINER(sidebar->commentsgrid));
    GList* iter;
    for (iter = children; iter != nullptr; iter = g_list_next(iter))
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    g_list_free(children);

    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    try
    {
        for (boost::property_tree::ptree::value_type& rValue : aTree.get_child("comments"))
        {
            GtkWidget* pCommentBox = gtv_comments_sidebar_create_comment_box(rValue.second);
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
#if GTK_CHECK_VERSION(3,12,0)
    // Hack to make sidebar grid wide enough to not need any horizontal scrollbar
    gtk_widget_set_margin_start(sidebar->viewannotationsButton, 20);
    gtk_widget_set_margin_end(sidebar->viewannotationsButton, 20);
#endif
    gtk_container_add(GTK_CONTAINER(sidebar), sidebar->viewannotationsButton);
    g_signal_connect_swapped(sidebar->viewannotationsButton, "clicked", G_CALLBACK(gtv_comments_sidebar_view_annotations_cb), sidebar);

    gtk_container_add(GTK_CONTAINER(sidebar), sidebar->scrolledwindow);
    gtk_container_add(GTK_CONTAINER(sidebar->scrolledwindow), sidebar->commentsgrid);

    gtk_widget_show_all(GTK_WIDGET(sidebar));
}

static void
gtv_comments_sidebar_class_init(GtvCommentsSidebarClass* klass)
{

}

GtkWidget*
gtv_comments_sidebar_new()
{
    return GTK_WIDGET(g_object_new(GTV_COMMENTS_SIDEBAR_TYPE,
                                   "orientation", GTK_ORIENTATION_VERTICAL,
                                   nullptr));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
