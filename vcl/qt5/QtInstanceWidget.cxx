/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceWidget.hxx>

#include <vcl/transfer.hxx>

QtInstanceWidget::QtInstanceWidget(QWidget* pWidget)
    : m_pWidget(pWidget)
{
    assert(pWidget);
}

void QtInstanceWidget::set_sensitive(bool bSensitive)
{
    assert(m_pWidget);
    m_pWidget->setEnabled(bSensitive);
}

bool QtInstanceWidget::get_sensitive() const
{
    assert(m_pWidget);
    return m_pWidget->isEnabled();
}

bool QtInstanceWidget::get_visible() const
{
    assert(m_pWidget);
    return m_pWidget->isVisible();
}

bool QtInstanceWidget::is_visible() const
{
    assert(m_pWidget);

    QWidget* pTopLevel = m_pWidget->topLevelWidget();
    assert(pTopLevel);
    return m_pWidget->isVisibleTo(pTopLevel) && pTopLevel->isVisible();
}

void QtInstanceWidget::set_can_focus(bool bCanFocus)
{
    assert(m_pWidget);
    if (bCanFocus)
        m_pWidget->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    else
        m_pWidget->setFocusPolicy(Qt::FocusPolicy::NoFocus);
}

void QtInstanceWidget::grab_focus()
{
    assert(m_pWidget);
    m_pWidget->setFocus();
}

bool QtInstanceWidget::has_focus() const
{
    assert(m_pWidget);
    return m_pWidget->hasFocus();
}

bool QtInstanceWidget::is_active() const { return has_focus(); }

bool QtInstanceWidget::has_child_focus() const
{
    QWidget* pFocusWidget = QApplication::focusWidget();
    if (!pFocusWidget)
        return false;

    QWidget* pParent = pFocusWidget->parentWidget();
    while (pParent)
    {
        if (pParent == m_pWidget)
            return true;
    }
    return false;
}

void QtInstanceWidget::show()
{
    assert(m_pWidget);
    m_pWidget->show();
}

void QtInstanceWidget::hide()
{
    assert(m_pWidget);
    m_pWidget->hide();
}

void QtInstanceWidget::set_size_request(int, int) {}

Size QtInstanceWidget::get_size_request() const { return Size(); }

Size QtInstanceWidget::get_preferred_size() const { return Size(); }

float QtInstanceWidget::get_approximate_digit_width() const { return 1.0; }

int QtInstanceWidget::get_text_height() const { return 1; }

Size QtInstanceWidget::get_pixel_size(const OUString&) const { return Size(); }

vcl::Font QtInstanceWidget::get_font() { return vcl::Font(); }

OUString QtInstanceWidget::get_buildable_name() const { return OUString(); }

void QtInstanceWidget::set_buildable_name(const OUString&) {}

void QtInstanceWidget::set_help_id(const OUString&) {}

OUString QtInstanceWidget::get_help_id() const { return OUString(); }

void QtInstanceWidget::set_grid_left_attach(int) {}

int QtInstanceWidget::get_grid_left_attach() const { return 1; }

void QtInstanceWidget::set_grid_width(int) {}

void QtInstanceWidget::set_grid_top_attach(int) {}

int QtInstanceWidget::get_grid_top_attach() const { return 1; }

void QtInstanceWidget::set_hexpand(bool) {}

bool QtInstanceWidget::get_hexpand() const { return true; }

void QtInstanceWidget::set_vexpand(bool) {}

bool QtInstanceWidget::get_vexpand() const { return true; }

void QtInstanceWidget::set_margin_top(int) {}

void QtInstanceWidget::set_margin_bottom(int) {}

void QtInstanceWidget::set_margin_start(int) {}

void QtInstanceWidget::set_margin_end(int) {}

int QtInstanceWidget::get_margin_top() const { return 1; }

int QtInstanceWidget::get_margin_bottom() const { return 1; }

int QtInstanceWidget::get_margin_start() const { return 1; }

int QtInstanceWidget::get_margin_end() const { return 1; }

void QtInstanceWidget::set_accessible_name(const OUString& rName)
{
    assert(m_pWidget);
    m_pWidget->setAccessibleName(toQString(rName));
}

void QtInstanceWidget::set_accessible_description(const OUString& rDescription)
{
    assert(m_pWidget);
    m_pWidget->setAccessibleDescription(toQString(rDescription));
}

OUString QtInstanceWidget::get_accessible_name() const
{
    assert(m_pWidget);
    return toOUString(m_pWidget->accessibleName());
}

OUString QtInstanceWidget::get_accessible_description() const
{
    assert(m_pWidget);
    return toOUString(m_pWidget->accessibleDescription());
}

OUString QtInstanceWidget::get_accessible_id() const
{
    assert(m_pWidget);
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    return toOUString(m_pWidget->accessibleIdentifier());
#else
    return OUString();
#endif
}

void QtInstanceWidget::set_accessible_relation_labeled_by(weld::Widget*) {}

void QtInstanceWidget::set_tooltip_text(const OUString& rTip)
{
    assert(m_pWidget);
    m_pWidget->setToolTip(toQString(rTip));
}

OUString QtInstanceWidget::get_tooltip_text() const
{
    assert(m_pWidget);
    return toOUString(m_pWidget->toolTip());
}

void QtInstanceWidget::set_cursor_data(void*) {}

void QtInstanceWidget::grab_add() {}

bool QtInstanceWidget::has_grab() const { return true; }

void QtInstanceWidget::grab_remove() {}

bool QtInstanceWidget::get_extents_relative_to(const Widget&, int&, int&, int&, int&) const
{
    return true;
}

bool QtInstanceWidget::get_direction() const { return true; }

void QtInstanceWidget::set_direction(bool) {}

void QtInstanceWidget::freeze() {}

void QtInstanceWidget::thaw() {}

void QtInstanceWidget::set_busy_cursor(bool) {}

std::unique_ptr<weld::Container> QtInstanceWidget::weld_parent() const { return nullptr; }

void QtInstanceWidget::queue_resize() {}

void QtInstanceWidget::help_hierarchy_foreach(const std::function<bool(const OUString&)>&) {}

OUString QtInstanceWidget::strip_mnemonic(const OUString&) const { return OUString(); }

OUString QtInstanceWidget::escape_ui_str(const OUString&) const { return OUString(); }

VclPtr<VirtualDevice> QtInstanceWidget::create_virtual_device() const { return nullptr; }

css::uno::Reference<css::datatransfer::dnd::XDropTarget> QtInstanceWidget::get_drop_target()
{
    return css::uno::Reference<css::datatransfer::dnd::XDropTarget>();
}

css::uno::Reference<css::datatransfer::clipboard::XClipboard>
QtInstanceWidget::get_clipboard() const
{
    return GetSystemClipboard();
}

void QtInstanceWidget::connect_get_property_tree(const Link<tools::JsonWriter&, void>&) {}

void QtInstanceWidget::get_property_tree(tools::JsonWriter&) {}

void QtInstanceWidget::call_attention_to() {}

void QtInstanceWidget::set_stack_background() {}

void QtInstanceWidget::set_title_background() {}

void QtInstanceWidget::set_toolbar_background() {}

void QtInstanceWidget::set_highlight_background() {}

void QtInstanceWidget::set_background(const Color&) {}

void QtInstanceWidget::draw(OutputDevice&, const Point&, const Size&) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
