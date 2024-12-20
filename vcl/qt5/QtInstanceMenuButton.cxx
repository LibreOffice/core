/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceMenuButton.hxx>
#include <QtInstanceMenuButton.moc>

QtInstanceMenuButton::QtInstanceMenuButton(QAbstractButton* pButton)
    : QtInstanceToggleButton(pButton)
{
    assert(pButton);
}

void QtInstanceMenuButton::insert_item(int, const OUString&, const OUString&, const OUString*,
                                       VirtualDevice*, TriState)
{
    assert(false && "Not implemented yet");
}

void QtInstanceMenuButton::insert_separator(int, const OUString&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceMenuButton::remove_item(const OUString&) { assert(false && "Not implemented yet"); }

void QtInstanceMenuButton::clear() { assert(false && "Not implemented yet"); }

void QtInstanceMenuButton::set_item_sensitive(const OUString&, bool)
{
    assert(false && "Not implemented yet");
}

void QtInstanceMenuButton::set_item_active(const OUString&, bool)
{
    assert(false && "Not implemented yet");
}

void QtInstanceMenuButton::set_item_label(const OUString&, const OUString&)
{
    assert(false && "Not implemented yet");
}

OUString QtInstanceMenuButton::get_item_label(const OUString&) const
{
    assert(false && "Not implemented yet");
    return OUString();
}

void QtInstanceMenuButton::set_item_visible(const OUString&, bool)
{
    assert(false && "Not implemented yet");
}

void QtInstanceMenuButton::set_popover(weld::Widget*) { assert(false && "Not implemented yet"); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
