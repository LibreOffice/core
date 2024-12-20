/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceMenu.hxx>
#include <QtInstanceMenu.moc>

#include <QtInstance.hxx>

#include <vcl/svapp.hxx>

QtInstanceMenu::QtInstanceMenu(QMenu* pMenu)
    : m_pMenu(pMenu)
{
    assert(m_pMenu);
}

OUString QtInstanceMenu::popup_at_rect(weld::Widget*, const tools::Rectangle&, weld::Placement)
{
    assert(false && "Not implemented yet");
    return OUString();
}

void QtInstanceMenu::set_sensitive(const OUString&, bool)
{
    assert(false && "Not implemented yet");
}

bool QtInstanceMenu::get_sensitive(const OUString&) const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceMenu::set_label(const OUString&, const OUString&)
{
    assert(false && "Not implemented yet");
}

OUString QtInstanceMenu::get_label(const OUString&) const
{
    assert(false && "Not implemented yet");
    return OUString();
}

void QtInstanceMenu::set_active(const OUString&, bool) { assert(false && "Not implemented yet"); }

bool QtInstanceMenu::get_active(const OUString&) const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceMenu::set_visible(const OUString&, bool) { assert(false && "Not implemented yet"); }

void QtInstanceMenu::insert(int, const OUString&, const OUString&, const OUString*, VirtualDevice*,
                            const css::uno::Reference<css::graphic::XGraphic>&, TriState)
{
    assert(false && "Not implemented yet");
}

void QtInstanceMenu::set_item_help_id(const OUString&, const OUString&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceMenu::remove(const OUString&) { assert(false && "Not implemented yet"); }

void QtInstanceMenu::clear()
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pMenu->clear(); });
}

void QtInstanceMenu::insert_separator(int, const OUString&)
{
    assert(false && "Not implemented yet");
}

int QtInstanceMenu::n_children() const
{
    assert(false && "Not implemented yet");
    return 0;
}

OUString QtInstanceMenu::get_id(int) const
{
    assert(false && "Not implemented yet");
    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
