/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_UNX_GTK_INC_GTKPRINTWRAPPER_HXX
#define INCLUDED_VCL_UNX_GTK_INC_GTKPRINTWRAPPER_HXX

#include <gtk/gtk.h>

#include <gtk/gtkunixprint.h>

#include <osl/module.hxx>
#include <sal/types.h>

namespace vcl::unx
{

class GtkPrintWrapper
{
private:
    GtkPrintWrapper(const GtkPrintWrapper&) = delete;
    GtkPrintWrapper& operator=(const GtkPrintWrapper&) = delete;
public:
    GtkPrintWrapper();
    ~GtkPrintWrapper();

    bool supportsPrinting() const;
    bool supportsPrintSelection() const;

    // general printing support, since 2.10.0
    GtkPageSetup* page_setup_new() const;

    GtkPrintJob* print_job_new(const gchar* title, GtkPrinter* printer, GtkPrintSettings* settings, GtkPageSetup* page_setup) const;
    void print_job_send(GtkPrintJob* job, GtkPrintJobCompleteFunc callback, gpointer user_data, GDestroyNotify dnotify) const;
    gboolean print_job_set_source_file(GtkPrintJob* job, const gchar* filename, GError** error) const;

    const gchar* print_settings_get(GtkPrintSettings* settings, const gchar* key) const;
    gboolean print_settings_get_collate(GtkPrintSettings* settings) const;
    void print_settings_set_collate(GtkPrintSettings* settings, gboolean collate) const;
    gint print_settings_get_n_copies(GtkPrintSettings* settings) const;
    void print_settings_set_n_copies(GtkPrintSettings* settings, gint num_copies) const;
    GtkPageRange* print_settings_get_page_ranges(GtkPrintSettings* settings, gint* num_ranges) const;
    void print_settings_set_print_pages(GtkPrintSettings* settings, GtkPrintPages pages) const;

    GtkWidget* print_unix_dialog_new() const;
    void print_unix_dialog_add_custom_tab(GtkPrintUnixDialog* dialog, GtkWidget* child, GtkWidget* tab_label) const;
    GtkPrinter* print_unix_dialog_get_selected_printer(GtkPrintUnixDialog* dialog) const;
    void print_unix_dialog_set_manual_capabilities(GtkPrintUnixDialog* dialog, GtkPrintCapabilities capabilities) const;
    GtkPrintSettings* print_unix_dialog_get_settings(GtkPrintUnixDialog* dialog) const;
    void print_unix_dialog_set_settings(GtkPrintUnixDialog* dialog, GtkPrintSettings* settings) const;

    // print selection support, since 2.17.4
    void print_unix_dialog_set_support_selection(GtkPrintUnixDialog* dialog, gboolean support_selection) const;
    void print_unix_dialog_set_has_selection(GtkPrintUnixDialog* dialog, gboolean has_selection) const;
};

}

#endif // INCLUDED_VCL_UNX_GTK_INC_GTKPRINTWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
