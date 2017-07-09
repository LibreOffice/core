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

struct GtvMainToolbarPrivateImpl
{
    GtkWidget* toolbar1;
    GtkWidget* toolbar2;

    /// Sensitivity (enabled or disabled) for each tool item, ignoring edit state
    std::map<GtkToolItem*, bool> m_aToolItemSensitivities;

    GtvMainToolbarPrivateImpl() :
        toolbar1(nullptr),
        toolbar2(nullptr)
        { }

    ~GtvMainToolbarPrivateImpl()
        { }
};

struct GtvMainToolbarPrivate
{
    GtvMainToolbarPrivateImpl* m_pImpl;

    GtvMainToolbarPrivateImpl* operator->()
    {
        return m_pImpl;
    }
};

G_DEFINE_TYPE_WITH_PRIVATE(GtvMainToolbar, gtv_main_toolbar, GTK_TYPE_BOX);

static GtvMainToolbarPrivate&
getPrivate(GtvMainToolbar* toolbar)
{
    return *static_cast<GtvMainToolbarPrivate*>(gtv_main_toolbar_get_instance_private(toolbar));
}

static void
gtv_main_toolbar_init(GtvMainToolbar* toolbar)
{
    GtvMainToolbarPrivate& priv = getPrivate(toolbar);
    priv.m_pImpl = new GtvMainToolbarPrivateImpl();

    GtkBuilder* builder = gtk_builder_new_from_file("gtv.ui");

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
gtv_main_toolbar_finalize(GObject* object)
{
    GtvMainToolbarPrivate& priv = getPrivate(GTV_MAIN_TOOLBAR(object));

    delete priv.m_pImpl;
    priv.m_pImpl = nullptr;

    G_OBJECT_CLASS (gtv_main_toolbar_parent_class)->finalize (object);
}

static void
gtv_main_toolbar_class_init(GtvMainToolbarClass* klass)
{
    // TODO: Use templates to bind objects maybe ?
    // But that requires compiling the .ui file into C source requiring
    // glib-compile-resources (another dependency) as I can't find any gtk
    // method to set the template from the .ui file directly; can only be set
    // from gresource
    G_OBJECT_CLASS(klass)->finalize = gtv_main_toolbar_finalize;
}

GtkContainer*
gtv_main_toolbar_get_first_toolbar(GtvMainToolbar* toolbar)
{
    GtvMainToolbarPrivate& priv = getPrivate(toolbar);
    return GTK_CONTAINER(priv->toolbar1);
}

GtkContainer*
gtv_main_toolbar_get_second_toolbar(GtvMainToolbar* toolbar)
{
    GtvMainToolbarPrivate& priv = getPrivate(toolbar);
    return GTK_CONTAINER(priv->toolbar2);
}

void
gtv_main_toolbar_set_sensitive_internal(GtvMainToolbar* toolbar, GtkToolItem* pItem, bool isSensitive)
{
    GtvMainToolbarPrivate& priv = getPrivate(toolbar);
    priv->m_aToolItemSensitivities[pItem] = isSensitive;
}

void
gtv_main_toolbar_set_edit(GtvMainToolbar* toolbar, gboolean bEdit)
{
    GtvMainToolbarPrivate& priv = getPrivate(toolbar);
    GList* pList = gtk_container_get_children(GTK_CONTAINER(priv->toolbar1));
    for (GList* l = pList; l != nullptr; l = l->next)
    {
        if (GTK_IS_TOOL_BUTTON(l->data))
        {
            GtkToolButton* pButton = GTK_TOOL_BUTTON(l->data);
            const gchar* pIconName = gtk_tool_button_get_icon_name(pButton);
            if (g_strcmp0(pIconName, "zoom-in-symbolic") != 0 &&
                g_strcmp0(pIconName, "zoom-original-symbolic") != 0 &&
                g_strcmp0(pIconName, "zoom-out-symbolic") != 0 &&
                g_strcmp0(pIconName, "insert-text-symbolic") != 0 &&
                g_strcmp0(pIconName, "view-continuous-symbolic") != 0 &&
                g_strcmp0(pIconName, "document-properties") != 0 &&
                g_strcmp0(pIconName, "system-run") != 0)
            {
                gtk_widget_set_sensitive(GTK_WIDGET(pButton), bEdit);
            }
        }
    }
}

GtkWidget*
gtv_main_toolbar_new()
{
    return GTK_WIDGET(g_object_new(GTV_MAIN_TOOLBAR_TYPE,
                                   "orientation", GTK_ORIENTATION_VERTICAL,
                                   nullptr));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
