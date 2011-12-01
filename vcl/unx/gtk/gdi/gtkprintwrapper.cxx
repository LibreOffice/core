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
 * Copyright (C) 2011 David Tardon, Red Hat Inc. <dtardon@redhat.com> (initial developer)
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

#include <cassert>

#include <rtl/ustring.hxx>

#include "gtkprintwrapper.hxx"

namespace vcl
{
namespace unx
{

GtkPrintWrapper::GtkPrintWrapper()
    : m_page_setup_new(0)
    , m_print_job_new(0)
    , m_print_job_send(0)
    , m_print_job_set_source_file(0)
    , m_print_settings_get(0)
    , m_print_settings_get_collate(0)
    , m_print_settings_set_collate(0)
    , m_print_settings_get_n_copies(0)
    , m_print_settings_set_n_copies(0)
    , m_print_settings_get_page_ranges(0)
    , m_print_settings_set_print_pages(0)
    , m_print_unix_dialog_new(0)
    , m_print_unix_dialog_add_custom_tab(0)
    , m_print_unix_dialog_get_selected_printer(0)
    , m_print_unix_dialog_set_manual_capabilities(0)
    , m_print_unix_dialog_get_settings(0)
    , m_print_unix_dialog_set_settings(0)
    , m_print_operation_set_support_selection(0)
    , m_print_operation_set_has_selection(0)
    , m_print_unix_dialog_set_support_selection(0)
    , m_print_unix_dialog_set_has_selection(0)
{
    impl_load();
}

GtkPrintWrapper::~GtkPrintWrapper()
{
}

void GtkPrintWrapper::impl_load()
{
    m_aModule.load(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("libgtk-x11-2.0.so.0")));
    if (!m_aModule.is())
        m_aModule.load(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("libgtk-x11-2.0.so")));
    if (!m_aModule.is())
        return;

    m_page_setup_new = reinterpret_cast<page_setup_new_t>(m_aModule.getFunctionSymbol("gtk_page_setup_new"));
    m_print_job_new = reinterpret_cast<print_job_new_t>(m_aModule.getFunctionSymbol("gtk_print_job_new"));
    m_print_job_send = reinterpret_cast<print_job_send_t>(m_aModule.getFunctionSymbol("gtk_print_job_send"));
    m_print_job_set_source_file = reinterpret_cast<print_job_set_source_file_t>(m_aModule.getFunctionSymbol("gtk_print_job_set_source_file"));
    m_print_settings_get = reinterpret_cast<print_settings_get_t>(m_aModule.getFunctionSymbol("gtk_print_settings_get"));
    m_print_settings_get_collate = reinterpret_cast<print_settings_get_collate_t>(m_aModule.getFunctionSymbol("gtk_print_settings_get_collate"));
    m_print_settings_set_collate = reinterpret_cast<print_settings_set_collate_t>(m_aModule.getFunctionSymbol("gtk_print_settings_set_collate"));
    m_print_settings_get_n_copies = reinterpret_cast<print_settings_get_n_copies_t>(m_aModule.getFunctionSymbol("gtk_print_settings_get_n_copies"));
    m_print_settings_set_n_copies = reinterpret_cast<print_settings_set_n_copies_t>(m_aModule.getFunctionSymbol("gtk_print_settings_set_n_copies"));
    m_print_settings_get_page_ranges = reinterpret_cast<print_settings_get_page_ranges_t>(m_aModule.getFunctionSymbol("gtk_print_settings_get_page_ranges"));
    m_print_settings_set_print_pages = reinterpret_cast<print_settings_set_print_pages_t>(m_aModule.getFunctionSymbol("gtk_print_settings_set_print_pages"));
    m_print_unix_dialog_new = reinterpret_cast<print_unix_dialog_new_t>(m_aModule.getFunctionSymbol("gtk_print_unix_dialog_new"));
    m_print_unix_dialog_add_custom_tab = reinterpret_cast<print_unix_dialog_add_custom_tab_t>(m_aModule.getFunctionSymbol("gtk_print_unix_dialog_add_custom_tab"));
    m_print_unix_dialog_get_selected_printer = reinterpret_cast<print_unix_dialog_get_selected_printer_t>(m_aModule.getFunctionSymbol("gtk_print_unix_dialog_get_selected_printer"));
    m_print_unix_dialog_set_manual_capabilities = reinterpret_cast<print_unix_dialog_set_manual_capabilities_t>(m_aModule.getFunctionSymbol("gtk_print_unix_dialog_set_manual_capabilities"));
    m_print_unix_dialog_get_settings = reinterpret_cast<print_unix_dialog_get_settings_t>(m_aModule.getFunctionSymbol("gtk_print_unix_dialog_get_settings"));
    m_print_unix_dialog_set_settings = reinterpret_cast<print_unix_dialog_set_settings_t>(m_aModule.getFunctionSymbol("gtk_print_unix_dialog_set_settings"));
    m_print_operation_set_support_selection = reinterpret_cast<print_operation_set_support_selection_t>(m_aModule.getFunctionSymbol("gtk_print_operation_set_support_selection"));
    m_print_operation_set_has_selection = reinterpret_cast<print_operation_set_has_selection_t>(m_aModule.getFunctionSymbol("gtk_print_operation_set_has_selection"));
    m_print_unix_dialog_set_support_selection = reinterpret_cast<print_unix_dialog_set_support_selection_t>(m_aModule.getFunctionSymbol("gtk_print_unix_dialog_set_support_selection"));
    m_print_unix_dialog_set_has_selection = reinterpret_cast<print_unix_dialog_set_has_selection_t>(m_aModule.getFunctionSymbol("gtk_print_unix_dialog_set_has_selection"));
}

bool GtkPrintWrapper::supportsPrinting() const
{
    return
        m_page_setup_new
        && m_print_job_new
        && m_print_job_send
        && m_print_job_set_source_file
        && m_print_settings_get
        && m_print_settings_get_collate
        && m_print_settings_set_collate
        && m_print_settings_get_n_copies
        && m_print_settings_set_n_copies
        && m_print_settings_get_page_ranges
        && m_print_settings_set_print_pages
        && m_print_unix_dialog_new
        && m_print_unix_dialog_add_custom_tab
        && m_print_unix_dialog_get_selected_printer
        && m_print_unix_dialog_set_manual_capabilities
        && m_print_unix_dialog_get_settings
        && m_print_unix_dialog_set_settings
        ;
}

bool GtkPrintWrapper::supportsPrintSelection() const
{
    return
        supportsPrinting()
        && m_print_operation_set_support_selection
        && m_print_operation_set_has_selection
        && m_print_unix_dialog_set_support_selection
        && m_print_unix_dialog_set_has_selection
        ;
}

GtkPageSetup* GtkPrintWrapper::page_setup_new() const
{
    assert(m_page_setup_new);
    return (*m_page_setup_new)();
}

GtkPrintJob* GtkPrintWrapper::print_job_new(const gchar* title, GtkPrinter* printer, GtkPrintSettings* settings, GtkPageSetup* page_setup) const
{
    assert(m_print_job_new);
    return (*m_print_job_new)(title, printer, settings, page_setup);
}

void GtkPrintWrapper::print_job_send(GtkPrintJob* job, GtkPrintJobCompleteFunc callback, gpointer user_data, GDestroyNotify dnotify) const
{
    assert(m_print_job_send);
    (*m_print_job_send)(job, callback, user_data, dnotify);
}

gboolean GtkPrintWrapper::print_job_set_source_file(GtkPrintJob* job, const gchar* filename, GError** error) const
{
    assert(m_print_job_set_source_file);
    return (*m_print_job_set_source_file)(job, filename, error);
}

const gchar* GtkPrintWrapper::print_settings_get(GtkPrintSettings* settings, const gchar* key) const
{
    assert(m_print_settings_get);
    return (*m_print_settings_get)(settings, key);
}

gboolean GtkPrintWrapper::print_settings_get_collate(GtkPrintSettings* settings) const
{
    assert(m_print_settings_get_collate);
    return (*m_print_settings_get_collate)(settings);
}

void GtkPrintWrapper::print_settings_set_collate(GtkPrintSettings* settings, gboolean collate) const
{
    assert(m_print_settings_set_collate);
    (*m_print_settings_set_collate)(settings, collate);
}

gint GtkPrintWrapper::print_settings_get_n_copies(GtkPrintSettings* settings) const
{
    assert(m_print_settings_get_n_copies);
    return (*m_print_settings_get_n_copies)(settings);
}

void GtkPrintWrapper::print_settings_set_n_copies(GtkPrintSettings* settings, gint num_copies) const
{
    assert(m_print_settings_set_n_copies);
    (*m_print_settings_set_n_copies)(settings, num_copies);
}

GtkPageRange* GtkPrintWrapper::print_settings_get_page_ranges(GtkPrintSettings* settings, gint* num_ranges) const
{
    assert(m_print_settings_get_page_ranges);
    return (*m_print_settings_get_page_ranges)(settings, num_ranges);
}

void GtkPrintWrapper::print_settings_set_print_pages(GtkPrintSettings* settings, GtkPrintPages pages) const
{
    assert(m_print_settings_set_print_pages);
    (*m_print_settings_set_print_pages)(settings, pages);
}

GtkWidget* GtkPrintWrapper::print_unix_dialog_new(const gchar* title, GtkWindow* parent) const
{
    assert(m_print_unix_dialog_new);
    return (*m_print_unix_dialog_new)(title, parent);
}

void GtkPrintWrapper::print_unix_dialog_add_custom_tab(GtkPrintUnixDialog* dialog, GtkWidget* child, GtkWidget* tab_label) const
{
    assert(m_print_unix_dialog_add_custom_tab);
    (*m_print_unix_dialog_add_custom_tab)(dialog, child, tab_label);
}

GtkPrinter* GtkPrintWrapper::print_unix_dialog_get_selected_printer(GtkPrintUnixDialog* dialog) const
{
    assert(m_print_unix_dialog_get_selected_printer);
    return (*m_print_unix_dialog_get_selected_printer)(dialog);
}

void GtkPrintWrapper::print_unix_dialog_set_manual_capabilities(GtkPrintUnixDialog* dialog, GtkPrintCapabilities capabilities) const
{
    assert(m_print_unix_dialog_set_manual_capabilities);
    (*m_print_unix_dialog_set_manual_capabilities)(dialog, capabilities);
}

GtkPrintSettings* GtkPrintWrapper::print_unix_dialog_get_settings(GtkPrintUnixDialog* dialog) const
{
    assert(m_print_unix_dialog_get_settings);
    return (*m_print_unix_dialog_get_settings)(dialog);
}

void GtkPrintWrapper::print_unix_dialog_set_settings(GtkPrintUnixDialog* dialog, GtkPrintSettings* settings) const
{
    assert(m_print_unix_dialog_set_settings);
    (*m_print_unix_dialog_set_settings)(dialog, settings);
}

void GtkPrintWrapper::print_operation_set_support_selection(GtkPrintOperation* op, gboolean support_selection) const
{
    assert(m_print_operation_set_support_selection);
    (*m_print_operation_set_support_selection)(op, support_selection);
}

void GtkPrintWrapper::print_operation_set_has_selection(GtkPrintOperation* op, gboolean has_selection) const
{
    assert(m_print_operation_set_has_selection);
    (*m_print_operation_set_has_selection)(op, has_selection);
}

void GtkPrintWrapper::print_unix_dialog_set_support_selection(GtkPrintUnixDialog* dialog, gboolean support_selection) const
{
    assert(m_print_unix_dialog_set_support_selection);
    (*m_print_unix_dialog_set_support_selection)(dialog, support_selection);
}

void GtkPrintWrapper::print_unix_dialog_set_has_selection(GtkPrintUnixDialog* dialog, gboolean has_selection) const
{
    assert(m_print_unix_dialog_set_has_selection);
    (*m_print_unix_dialog_set_has_selection)(dialog, has_selection);
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
