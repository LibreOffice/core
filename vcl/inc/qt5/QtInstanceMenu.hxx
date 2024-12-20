/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <QtWidgets/QMenu>

#include <vcl/weld.hxx>

class QtInstanceMenu : public QObject, public virtual weld::Menu
{
    Q_OBJECT

    QMenu* m_pMenu;

public:
    QtInstanceMenu(QMenu* pMenu);

    virtual OUString popup_at_rect(weld::Widget* pParent, const tools::Rectangle& rRect,
                                   weld::Placement ePlace = weld::Placement::Under) override;

    virtual void set_sensitive(const OUString& rIdent, bool bSensitive) override;
    virtual bool get_sensitive(const OUString& rIdent) const override;
    virtual void set_label(const OUString& rIdent, const OUString& rLabel) override;
    virtual OUString get_label(const OUString& rIdent) const override;
    virtual void set_active(const OUString& rIdent, bool bActive) override;
    virtual bool get_active(const OUString& rIdent) const override;
    virtual void set_visible(const OUString& rIdent, bool bVisible) override;

    virtual void insert(int pos, const OUString& rId, const OUString& rStr,
                        const OUString* pIconName, VirtualDevice* pImageSurface,
                        const css::uno::Reference<css::graphic::XGraphic>& rImage,
                        TriState eCheckRadioFalse) override;

    virtual void set_item_help_id(const OUString& rIdent, const OUString& rHelpId) override;
    virtual void remove(const OUString& rId) override;

    virtual void clear() override;

    virtual void insert_separator(int pos, const OUString& rId) override;

    virtual int n_children() const override;

    virtual OUString get_id(int nPos) const override;

    static void setActionName(QAction& rAction, const OUString& rActionName);

private:
    // get action with the given ID
    QAction* getAction(const OUString& rIdent) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
