/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtInstanceWidget.hxx"

#include <QtWidgets/QToolBar>

class QtInstanceToolbar : public QtInstanceWidget, public virtual weld::Toolbar
{
    Q_OBJECT

    QToolBar* m_pToolBar;

public:
    QtInstanceToolbar(QToolBar* pToolBar);

    virtual void set_item_sensitive(const OUString& rIdent, bool bSensitive) override;
    virtual bool get_item_sensitive(const OUString& rIdent) const override;
    virtual void set_item_active(const OUString& rIdent, bool bActive) override;
    virtual bool get_item_active(const OUString& rIdent) const override;
    virtual void set_menu_item_active(const OUString& rIdent, bool bActive) override;
    virtual bool get_menu_item_active(const OUString& rIdent) const override;
    virtual void set_item_menu(const OUString& rIdent, weld::Menu* pMenu) override;
    virtual void set_item_popover(const OUString& rIdent, weld::Widget* pPopover) override;
    virtual void set_item_visible(const OUString& rIdent, bool bVisible) override;
    virtual void set_item_help_id(const OUString& rIdent, const OUString& rHelpId) override;
    virtual bool get_item_visible(const OUString& rIdent) const override;
    virtual void set_item_label(const OUString& rIdent, const OUString& rLabel) override;
    virtual OUString get_item_label(const OUString& rIdent) const override;
    virtual void set_item_tooltip_text(const OUString& rIdent, const OUString& rTip) override;
    virtual OUString get_item_tooltip_text(const OUString& rIdent) const override;
    virtual void set_item_icon_name(const OUString& rIdent, const OUString& rIconName) override;
    virtual void set_item_image_mirrored(const OUString& rIdent, bool bMirrored) override;
    virtual void set_item_image(const OUString& rIdent,
                                const css::uno::Reference<css::graphic::XGraphic>& rIcon) override;
    virtual void set_item_image(const OUString& rIdent, VirtualDevice* pDevice) override;

    virtual void insert_item(int pos, const OUString& rId) override;
    virtual void insert_separator(int pos, const OUString& rId) override;

    virtual int get_n_items() const override;
    virtual OUString get_item_ident(int nIndex) const override;
    virtual void set_item_ident(int nIndex, const OUString& rIdent) override;
    virtual void set_item_label(int nIndex, const OUString& rLabel) override;
    virtual void set_item_image(int nIndex,
                                const css::uno::Reference<css::graphic::XGraphic>& rIcon) override;
    virtual void set_item_tooltip_text(int nIndex, const OUString& rTip) override;
    virtual void set_item_accessible_name(int nIndex, const OUString& rName) override;
    virtual void set_item_accessible_name(const OUString& rIdent, const OUString& rName) override;

    virtual vcl::ImageType get_icon_size() const override;
    virtual void set_icon_size(vcl::ImageType eType) override;

    virtual sal_uInt16 get_modifier_state() const override;

    virtual int get_drop_index(const Point& rPoint) const override;

private Q_SLOTS:
    void toolButtonClicked(const QString& rId);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
