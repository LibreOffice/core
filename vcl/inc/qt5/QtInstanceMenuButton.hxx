/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtInstanceToggleButton.hxx"

#include <QtWidgets/QToolButton>

class QtInstanceMenuButton : public QtInstanceToggleButton, public virtual weld::MenuButton
{
    Q_OBJECT

    QToolButton* m_pToolButton;
    QWidget* m_pPopover;

public:
    QtInstanceMenuButton(QToolButton* pButton);

    virtual void insert_item(int nPos, const OUString& rId, const OUString& rStr,
                             const OUString* pIconName, VirtualDevice* pImageSurface,
                             TriState eCheckRadioFalse) override;

    virtual void insert_separator(int nPos, const OUString& rId) override;
    virtual void remove_item(const OUString& rId) override;
    virtual void clear() override;
    virtual void set_item_sensitive(const OUString& rIdent, bool bSensitive) override;
    virtual void set_item_active(const OUString& rIdent, bool bActive) override;
    virtual void set_item_label(const OUString& rIdent, const OUString& rLabel) override;
    virtual OUString get_item_label(const OUString& rIdent) const override;
    virtual void set_item_visible(const OUString& rIdent, bool bVisible) override;

    virtual void set_popover(weld::Widget* pPopover) override;

private:
    QMenu& getMenu() const;
    QAction* getAction(const OUString& rIdent) const;
    void insertAction(QAction* pAction, int nPos);

private Q_SLOTS:
    void handleButtonClicked();
    void handleMenuItemTriggered(QAction* pAction);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
