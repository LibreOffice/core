/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtInstance.hxx"
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QMessageBox>
#include "QtTools.hxx"
#include <vcl/weld.hxx>
#include <string_view>
#include <rtl/ustring.hxx>
#include <vcl/sysdata.hxx>

class QtInstanceWidget : public QObject, public virtual weld::Widget
{
    Q_OBJECT

    QWidget* m_pWidget;

    int m_nBusyCount = 0;

public:
    QtInstanceWidget(QWidget* pWidget);

    virtual QWidget* getQWidget() const { return m_pWidget; }

    virtual void set_sensitive(bool bSensitive) override;

    virtual bool get_sensitive() const override;

    virtual bool get_visible() const override;

    virtual bool is_visible() const override;

    virtual void set_can_focus(bool bCanFocus) override;

    virtual void grab_focus() override;

    virtual bool has_focus() const override;

    virtual bool is_active() const override;

    virtual bool has_child_focus() const override;

    virtual void show() override;

    virtual void hide() override;

    virtual void set_size_request(int nWidth, int nHeight) override;

    virtual Size get_size_request() const override;

    virtual Size get_preferred_size() const override;

    virtual float get_approximate_digit_width() const override;

    virtual int get_text_height() const override;

    virtual Size get_pixel_size(const OUString& rText) const override;

    virtual vcl::Font get_font() override;

    virtual OUString get_buildable_name() const override;

    virtual void set_buildable_name(const OUString&) override;

    virtual void set_help_id(const OUString& rHelpId) override;

    virtual OUString get_help_id() const override;

    virtual void set_hexpand(bool) override;

    virtual bool get_hexpand() const override;

    virtual void set_vexpand(bool) override;

    virtual bool get_vexpand() const override;

    virtual void set_margin_top(int nMargin) override;

    virtual void set_margin_bottom(int nMargin) override;

    virtual void set_margin_start(int nMargin) override;

    virtual void set_margin_end(int nMargin) override;

    virtual int get_margin_top() const override;

    virtual int get_margin_bottom() const override;

    virtual int get_margin_start() const override;

    virtual int get_margin_end() const override;

    virtual void set_accessible_name(const OUString& rName) override;

    virtual void set_accessible_description(const OUString& rDescription) override;

    virtual OUString get_accessible_name() const override;

    virtual OUString get_accessible_description() const override;

    virtual OUString get_accessible_id() const override;

    virtual void set_accessible_relation_labeled_by(weld::Widget*) override;

    virtual void set_tooltip_text(const OUString& rTip) override;

    virtual OUString get_tooltip_text() const override;

    virtual void set_cursor_data(void*) override;

    virtual void grab_add() override;

    virtual bool has_grab() const override;

    virtual void grab_remove() override;

    virtual bool get_extents_relative_to(const Widget& rRelative, int& rX, int& rY, int& rWidth,
                                         int& rHeight) const override;

    virtual bool get_direction() const override;

    virtual void set_direction(bool bRTL) override;

    virtual void freeze() override;

    virtual void thaw() override;

    virtual void set_busy_cursor(bool bBusy) override;

    virtual std::unique_ptr<weld::Container> weld_parent() const override;

    virtual void queue_resize() override;

    virtual void help_hierarchy_foreach(const std::function<bool(const OUString&)>&) override;

    virtual OUString strip_mnemonic(const OUString& rLabel) const override;

    virtual OUString escape_ui_str(const OUString& rLabel) const override;

    virtual VclPtr<VirtualDevice> create_virtual_device() const override;

    virtual css::uno::Reference<css::datatransfer::dnd::XDropTarget> get_drop_target() override;

    virtual css::uno::Reference<css::datatransfer::clipboard::XClipboard>
    get_clipboard() const override;

    virtual void connect_get_property_tree(const Link<tools::JsonWriter&, void>&) override;

    virtual void get_property_tree(tools::JsonWriter&) override;

    virtual void call_attention_to() override;

    virtual void set_stack_background() override;

    virtual void set_title_background() override;

    virtual void set_toolbar_background() override;

    virtual void set_highlight_background() override;

    void setFontColor(const Color& rFontColor);
    virtual void set_background(const Color& rBackColor) override;

    virtual void draw(OutputDevice&, const Point&, const Size&) override;

    static void setHelpId(QWidget& rWidget, const OUString& rHelpId);

private Q_SLOTS:
    void applicationFocusChanged(QWidget* pOldFocus, QWidget* pNewFocus);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
