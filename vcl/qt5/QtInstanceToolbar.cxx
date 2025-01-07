/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceToolbar.hxx>
#include <QtInstanceToolbar.moc>

QtInstanceToolbar::QtInstanceToolbar(QToolBar* pToolBar)
    : QtInstanceWidget(pToolBar)
    , m_pToolBar(pToolBar)
{
    assert(m_pToolBar);
}

void QtInstanceToolbar::set_item_sensitive(const OUString&, bool)
{
    assert(false && "Not implemented yet");
}

bool QtInstanceToolbar::get_item_sensitive(const OUString&) const
{
    assert(false && "Not implemented yet");
    return false;
}
void QtInstanceToolbar::set_item_active(const OUString&, bool)
{
    assert(false && "Not implemented yet");
}

bool QtInstanceToolbar::get_item_active(const OUString&) const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceToolbar::set_menu_item_active(const OUString&, bool)
{
    assert(false && "Not implemented yet");
}

bool QtInstanceToolbar::get_menu_item_active(const OUString&) const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceToolbar::set_item_menu(const OUString&, weld::Menu*)
{
    assert(false && "Not implemented yet");
}
void QtInstanceToolbar::set_item_popover(const OUString&, weld::Widget*)
{
    assert(false && "Not implemented yet");
}

void QtInstanceToolbar::set_item_visible(const OUString&, bool)
{
    assert(false && "Not implemented yet");
}

void QtInstanceToolbar::set_item_help_id(const OUString&, const OUString&)
{
    assert(false && "Not implemented yet");
}

bool QtInstanceToolbar::get_item_visible(const OUString&) const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceToolbar::set_item_label(const OUString&, const OUString&)
{
    assert(false && "Not implemented yet");
}

OUString QtInstanceToolbar::get_item_label(const OUString&) const
{
    assert(false && "Not implemented yet");
    return OUString();
}

void QtInstanceToolbar::set_item_tooltip_text(const OUString&, const OUString&)
{
    assert(false && "Not implemented yet");
}

OUString QtInstanceToolbar::get_item_tooltip_text(const OUString&) const
{
    assert(false && "Not implemented yet");
    return OUString();
}

void QtInstanceToolbar::set_item_icon_name(const OUString&, const OUString&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceToolbar::set_item_image_mirrored(const OUString&, bool)
{
    assert(false && "Not implemented yet");
}

void QtInstanceToolbar::set_item_image(const OUString&,
                                       const css::uno::Reference<css::graphic::XGraphic>&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceToolbar::set_item_image(const OUString&, VirtualDevice*)
{
    assert(false && "Not implemented yet");
}

void QtInstanceToolbar::insert_item(int, const OUString&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceToolbar::insert_separator(int, const OUString&)
{
    assert(false && "Not implemented yet");
}

int QtInstanceToolbar::get_n_items() const
{
    assert(false && "Not implemented yet");
    return 0;
}

OUString QtInstanceToolbar::get_item_ident(int) const
{
    assert(false && "Not implemented yet");
    return OUString();
}

void QtInstanceToolbar::set_item_ident(int, const OUString&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceToolbar::set_item_label(int, const OUString&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceToolbar::set_item_image(int, const css::uno::Reference<css::graphic::XGraphic>&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceToolbar::set_item_tooltip_text(int, const OUString&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceToolbar::set_item_accessible_name(int, const OUString&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceToolbar::set_item_accessible_name(const OUString&, const OUString&)
{
    assert(false && "Not implemented yet");
}

vcl::ImageType QtInstanceToolbar::get_icon_size() const
{
    assert(false && "Not implemented yet");
    return vcl::ImageType::Size32;
}

void QtInstanceToolbar::set_icon_size(vcl::ImageType) { assert(false && "Not implemented yet"); }

sal_uInt16 QtInstanceToolbar::get_modifier_state() const
{
    assert(false && "Not implemented yet");
    return 0;
}

int QtInstanceToolbar::get_drop_index(const Point&) const
{
    assert(false && "Not implemented yet");
    return -1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
