/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>

#include <rtl/ustring.hxx>

#include <unx/gtk/gtkprintwrapper.hxx>

namespace vcl
{
namespace unx
{

GtkPrintWrapper::GtkPrintWrapper()
{
}

GtkPrintWrapper::~GtkPrintWrapper()
{
}

bool GtkPrintWrapper::supportsPrinting() const
{
    (void) this; // loplugin:staticmethods
    return true;
}

bool GtkPrintWrapper::supportsPrintSelection() const
{
    (void) this; // loplugin:staticmethods
    return true;
}

GtkPageSetup* GtkPrintWrapper::page_setup_new() const
{
    (void) this; // loplugin:staticmethods
    return gtk_page_setup_new();
}

GtkPrintJob* GtkPrintWrapper::print_job_new(const gchar* title, GtkPrinter* printer, GtkPrintSettings* settings, GtkPageSetup* page_setup) const
{
    (void) this; // loplugin:staticmethods
    return gtk_print_job_new(title, printer, settings, page_setup);
}

void GtkPrintWrapper::print_job_send(GtkPrintJob* job, GtkPrintJobCompleteFunc callback, gpointer user_data, GDestroyNotify dnotify) const
{
    (void) this; // loplugin:staticmethods
    gtk_print_job_send(job, callback, user_data, dnotify);
}

gboolean GtkPrintWrapper::print_job_set_source_file(GtkPrintJob* job, const gchar* filename, GError** error) const
{
    (void) this; // loplugin:staticmethods
    return gtk_print_job_set_source_file(job, filename, error);
}

const gchar* GtkPrintWrapper::print_settings_get(GtkPrintSettings* settings, const gchar* key) const
{
    (void) this; // loplugin:staticmethods
    return gtk_print_settings_get(settings, key);
}

gboolean GtkPrintWrapper::print_settings_get_collate(GtkPrintSettings* settings) const
{
    (void) this; // loplugin:staticmethods
    return gtk_print_settings_get_collate(settings);
}

void GtkPrintWrapper::print_settings_set_collate(GtkPrintSettings* settings, gboolean collate) const
{
    (void) this; // loplugin:staticmethods
    gtk_print_settings_set_collate(settings, collate);
}

gint GtkPrintWrapper::print_settings_get_n_copies(GtkPrintSettings* settings) const
{
    (void) this; // loplugin:staticmethods
    return gtk_print_settings_get_n_copies(settings);
}

void GtkPrintWrapper::print_settings_set_n_copies(GtkPrintSettings* settings, gint num_copies) const
{
    (void) this; // loplugin:staticmethods
    gtk_print_settings_set_n_copies(settings, num_copies);
}

GtkPageRange* GtkPrintWrapper::print_settings_get_page_ranges(GtkPrintSettings* settings, gint* num_ranges) const
{
    (void) this; // loplugin:staticmethods
    return gtk_print_settings_get_page_ranges(settings, num_ranges);
}

void GtkPrintWrapper::print_settings_set_print_pages(GtkPrintSettings* settings, GtkPrintPages pages) const
{
    (void) this; // loplugin:staticmethods
    gtk_print_settings_set_print_pages(settings, pages);
}

GtkWidget* GtkPrintWrapper::print_unix_dialog_new() const
{
    (void) this; // loplugin:staticmethods
    return gtk_print_unix_dialog_new(nullptr, nullptr);
}

void GtkPrintWrapper::print_unix_dialog_add_custom_tab(GtkPrintUnixDialog* dialog, GtkWidget* child, GtkWidget* tab_label) const
{
    (void) this; // loplugin:staticmethods
    gtk_print_unix_dialog_add_custom_tab(dialog, child, tab_label);
}

GtkPrinter* GtkPrintWrapper::print_unix_dialog_get_selected_printer(GtkPrintUnixDialog* dialog) const
{
    (void) this; // loplugin:staticmethods
    GtkPrinter* pRet = gtk_print_unix_dialog_get_selected_printer(dialog);
    g_object_ref(G_OBJECT(pRet));
    return pRet;
}

void GtkPrintWrapper::print_unix_dialog_set_manual_capabilities(GtkPrintUnixDialog* dialog, GtkPrintCapabilities capabilities) const
{
    (void) this; // loplugin:staticmethods
    gtk_print_unix_dialog_set_manual_capabilities(dialog, capabilities);
}

GtkPrintSettings* GtkPrintWrapper::print_unix_dialog_get_settings(GtkPrintUnixDialog* dialog) const
{
    (void) this; // loplugin:staticmethods
    return gtk_print_unix_dialog_get_settings(dialog);
}

void GtkPrintWrapper::print_unix_dialog_set_settings(GtkPrintUnixDialog* dialog, GtkPrintSettings* settings) const
{
    (void) this; // loplugin:staticmethods
    gtk_print_unix_dialog_set_settings(dialog, settings);
}

void GtkPrintWrapper::print_unix_dialog_set_support_selection(GtkPrintUnixDialog* dialog, gboolean support_selection) const
{
    (void) this; // loplugin:staticmethods
    gtk_print_unix_dialog_set_support_selection(dialog, support_selection);
}

void GtkPrintWrapper::print_unix_dialog_set_has_selection(GtkPrintUnixDialog* dialog, gboolean has_selection) const
{
    (void) this; // loplugin:staticmethods
    gtk_print_unix_dialog_set_has_selection(dialog, has_selection);
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
