/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "notifyinglayout.hxx"

struct _NotifyingLayout
{
    GtkLayoutManager parent_instance;

    GtkWidget* m_pWidget;
    GtkLayoutManager* m_pOrigManager;
    Link<void*, void> m_aLink;
};

G_DEFINE_TYPE(NotifyingLayout, notifying_layout, GTK_TYPE_LAYOUT_MANAGER)

static void notifying_layout_measure(GtkLayoutManager* pLayoutManager, GtkWidget* widget,
                                     GtkOrientation orientation, int for_size, int* minimum,
                                     int* natural, int* minimum_baseline, int* natural_baseline)
{
    NotifyingLayout* self = NOTIFYING_LAYOUT(pLayoutManager);
    GtkLayoutManagerClass* pKlass
        = GTK_LAYOUT_MANAGER_CLASS(G_OBJECT_GET_CLASS(self->m_pOrigManager));
    pKlass->measure(self->m_pOrigManager, widget, orientation, for_size, minimum, natural,
                    minimum_baseline, natural_baseline);
}

static void notifying_layout_allocate(GtkLayoutManager* pLayoutManager, GtkWidget* widget,
                                      int width, int height, int baseline)
{
    NotifyingLayout* self = NOTIFYING_LAYOUT(pLayoutManager);
    GtkLayoutManagerClass* pKlass
        = GTK_LAYOUT_MANAGER_CLASS(G_OBJECT_GET_CLASS(self->m_pOrigManager));
    pKlass->allocate(self->m_pOrigManager, widget, width, height, baseline);
    self->m_aLink.Call(nullptr);
}

static GtkSizeRequestMode notifying_layout_get_request_mode(GtkLayoutManager* pLayoutManager,
                                                            GtkWidget* widget)
{
    NotifyingLayout* self = NOTIFYING_LAYOUT(pLayoutManager);
    GtkLayoutManagerClass* pKlass
        = GTK_LAYOUT_MANAGER_CLASS(G_OBJECT_GET_CLASS(self->m_pOrigManager));
    return pKlass->get_request_mode(self->m_pOrigManager, widget);
}

static void notifying_layout_class_init(NotifyingLayoutClass* klass)
{
    GtkLayoutManagerClass* layout_class = GTK_LAYOUT_MANAGER_CLASS(klass);

    layout_class->get_request_mode = notifying_layout_get_request_mode;
    layout_class->measure = notifying_layout_measure;
    layout_class->allocate = notifying_layout_allocate;
}

static void notifying_layout_init(NotifyingLayout* self)
{
    self->m_pWidget = nullptr;
    self->m_pOrigManager = nullptr;

    // prevent loplugin:unreffun firing on macro generated function
    (void)notifying_layout_get_instance_private(self);
}

void notifying_layout_start_watch(NotifyingLayout* self, GtkWidget* pWidget,
                                  const Link<void*, void>& rLink)
{
    self->m_pWidget = pWidget;
    self->m_aLink = rLink;

    self->m_pOrigManager = gtk_widget_get_layout_manager(self->m_pWidget);
    g_object_ref(self->m_pOrigManager);

    gtk_widget_set_layout_manager(pWidget, GTK_LAYOUT_MANAGER(self));
}

void notifying_layout_stop_watch(NotifyingLayout* self)
{
    gtk_widget_set_layout_manager(self->m_pWidget, self->m_pOrigManager);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
