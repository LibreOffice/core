/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceNotebook.hxx>
#include <QtInstanceNotebook.moc>

QtInstanceNotebook::QtInstanceNotebook(QTabWidget* pTabWidget)
    : QtInstanceWidget(pTabWidget)
    , m_pTabWidget(pTabWidget)
{
    assert(m_pTabWidget);
}

int QtInstanceNotebook::get_current_page() const
{
    assert(false && "Not implemented yet");
    return -1;
}

int QtInstanceNotebook::get_page_index(const OUString&) const
{
    assert(false && "Not implemented yet");
    return -1;
}

OUString QtInstanceNotebook::get_page_ident(int) const
{
    assert(false && "Not implemented yet");
    return OUString();
}

OUString QtInstanceNotebook::get_current_page_ident() const
{
    assert(false && "Not implemented yet");
    return OUString();
}

void QtInstanceNotebook::set_current_page(int) { assert(false && "Not implemented yet"); }

void QtInstanceNotebook::set_current_page(const OUString&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceNotebook::remove_page(const OUString&) { assert(false && "Not implemented yet"); }

void QtInstanceNotebook::insert_page(const OUString&, const OUString&, int)
{
    assert(false && "Not implemented yet");
}

void QtInstanceNotebook::set_tab_label_text(const OUString&, const OUString&)
{
    assert(false && "Not implemented yet");
}

OUString QtInstanceNotebook::get_tab_label_text(const OUString&) const
{
    assert(false && "Not implemented yet");
    return OUString();
}

void QtInstanceNotebook::set_show_tabs(bool) { assert(false && "Not implemented yet"); }

int QtInstanceNotebook::get_n_pages() const
{
    assert(false && "Not implemented yet");
    return 0;
}

weld::Container* QtInstanceNotebook::get_page(const OUString&) const
{
    assert(false && "Not implemented yet");
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
