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

#if defined ENABLE_GTK_PRINT || GTK_CHECK_VERSION(3,0,0)
#if GTK_CHECK_VERSION(2,14,0)
#include <gtk/gtkunixprint.h>
#else
#include <gtk/gtkprintjob.h>
#include <gtk/gtkpagesetupunixdialog.h>
#include <gtk/gtkprintunixdialog.h>
#endif

#include <osl/module.hxx>
#include <sal/types.h>

#endif

namespace vcl
{
namespace unx
{

class GtkPrintWrapper
{
private:
    GtkPrintWrapper(const GtkPrintWrapper&) = delete;
    GtkPrintWrapper& operator=(const GtkPrintWrapper&) = delete;
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

#endif // INCLUDED_VCL_UNX_GTK_INC_GTKPRINTWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
