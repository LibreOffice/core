/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Red Hat, Inc., David Tardon <dtardon@redhat.com>
 *  (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef UNX_GTK_INC_GTKPRINTWRAPPER_HXX_INCLUDED
#define UNX_GTK_INC_GTKPRINTWRAPPER_HXX_INCLUDED

#include <boost/noncopyable.hpp>

#include <gtk/gtk.h>

#if defined ENABLE_GTK_PRINT || GTK_CHECK_VERSION(3,0,0)
#if GTK_CHECK_VERSION(2,14,0)
#include <gtk/gtkunixprint.h>
#else
#include <gtk/gtkpagesetupunixdialog.h>
#endif

#if !GTK_CHECK_VERSION(3,0,0)
#include <osl/module.hxx>
#endif

#endif

namespace vcl
{
namespace unx
{

class GtkPrintWrapper : private boost::noncopyable
{
#if defined ENABLE_GTK_PRINT || GTK_CHECK_VERSION(3,0,0)
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

    GtkWidget* print_unix_dialog_new(const gchar* title, GtkWindow* parent) const;
    void print_unix_dialog_add_custom_tab(GtkPrintUnixDialog* dialog, GtkWidget* child, GtkWidget* tab_label) const;
    GtkPrinter* print_unix_dialog_get_selected_printer(GtkPrintUnixDialog* dialog) const;
    void print_unix_dialog_set_manual_capabilities(GtkPrintUnixDialog* dialog, GtkPrintCapabilities capabilities) const;
    GtkPrintSettings* print_unix_dialog_get_settings(GtkPrintUnixDialog* dialog) const;
    void print_unix_dialog_set_settings(GtkPrintUnixDialog* dialog, GtkPrintSettings* settings) const;

    // print selection support, since 2.17.4
    void print_unix_dialog_set_support_selection(GtkPrintUnixDialog* dialog, gboolean support_selection) const;
    void print_unix_dialog_set_has_selection(GtkPrintUnixDialog* dialog, gboolean has_selection) const;

#if !GTK_CHECK_VERSION(3,0,0)
private:
    void impl_load();

private:
    typedef GtkPageSetup* (* page_setup_new_t)();
    typedef GtkPrintJob* (* print_job_new_t)(const gchar*, GtkPrinter*, GtkPrintSettings*, GtkPageSetup*);
    typedef void (* print_job_send_t)(GtkPrintJob*, GtkPrintJobCompleteFunc, gpointer, GDestroyNotify);
    typedef gboolean (* print_job_set_source_file_t)(GtkPrintJob*, const gchar*, GError**);
    typedef const gchar* (* print_settings_get_t)(GtkPrintSettings*, const gchar*);
    typedef gboolean (* print_settings_get_collate_t)(GtkPrintSettings*);
    typedef void (* print_settings_set_collate_t)(GtkPrintSettings*, gboolean);
    typedef gint (* print_settings_get_n_copies_t)(GtkPrintSettings*);
    typedef void (* print_settings_set_n_copies_t)(GtkPrintSettings*, gint);
    typedef GtkPageRange* (* print_settings_get_page_ranges_t)(GtkPrintSettings*, gint*);
    typedef void (* print_settings_set_print_pages_t)(GtkPrintSettings*, GtkPrintPages);
    typedef GtkWidget* (* print_unix_dialog_new_t)(const gchar*, GtkWindow*);
    typedef void (* print_unix_dialog_add_custom_tab_t)(GtkPrintUnixDialog*, GtkWidget*, GtkWidget*);
    typedef GtkPrinter* (* print_unix_dialog_get_selected_printer_t)(GtkPrintUnixDialog*);
    typedef void (* print_unix_dialog_set_manual_capabilities_t)(GtkPrintUnixDialog*, GtkPrintCapabilities);
    typedef GtkPrintSettings* (* print_unix_dialog_get_settings_t)(GtkPrintUnixDialog*);
    typedef void (* print_unix_dialog_set_settings_t)(GtkPrintUnixDialog*, GtkPrintSettings*);
    typedef void (* print_unix_dialog_set_support_selection_t)(GtkPrintUnixDialog*, gboolean);
    typedef void (* print_unix_dialog_set_has_selection_t)(GtkPrintUnixDialog*, gboolean);

private:
    osl::Module m_aModule;

    // general printing support, since 2.10.0
    page_setup_new_t m_page_setup_new;
    print_job_new_t m_print_job_new;
    print_job_send_t m_print_job_send;
    print_job_set_source_file_t m_print_job_set_source_file;
    print_settings_get_t m_print_settings_get;
    print_settings_get_collate_t m_print_settings_get_collate;
    print_settings_set_collate_t m_print_settings_set_collate;
    print_settings_get_n_copies_t m_print_settings_get_n_copies;
    print_settings_set_n_copies_t m_print_settings_set_n_copies;
    print_settings_get_page_ranges_t m_print_settings_get_page_ranges;
    print_settings_set_print_pages_t m_print_settings_set_print_pages;
    print_unix_dialog_new_t m_print_unix_dialog_new;
    print_unix_dialog_add_custom_tab_t m_print_unix_dialog_add_custom_tab;
    print_unix_dialog_get_selected_printer_t m_print_unix_dialog_get_selected_printer;
    print_unix_dialog_set_manual_capabilities_t m_print_unix_dialog_set_manual_capabilities;
    print_unix_dialog_get_settings_t m_print_unix_dialog_get_settings;
    print_unix_dialog_set_settings_t m_print_unix_dialog_set_settings;

    // print selection support, since 2.17.4
    print_unix_dialog_set_support_selection_t m_print_unix_dialog_set_support_selection;
    print_unix_dialog_set_has_selection_t m_print_unix_dialog_set_has_selection;
#endif
#endif
};

}
}

#endif // UNX_GTK_INC_GTKPRINTWRAPPER_HXX_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
