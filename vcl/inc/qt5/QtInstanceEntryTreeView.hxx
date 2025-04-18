/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtInstanceContainer.hxx"

#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTreeView>

class QtInstanceBuilder;

class QtInstanceEntryTreeView final : public QtInstanceContainer, public virtual weld::EntryTreeView
{
    Q_OBJECT

    QLineEdit* m_pLineEdit;
    QTreeView* m_pTreeView;

public:
    QtInstanceEntryTreeView(QWidget* pContainer, QLineEdit* pLineEdit, QTreeView* pTreeView,
                            std::unique_ptr<weld::Entry> xEntry,
                            std::unique_ptr<weld::TreeView> xTreeView);

    virtual void grab_focus() override;
    virtual void connect_focus_in(const Link<Widget&, void>& rLink) override;
    virtual void connect_focus_out(const Link<Widget&, void>& rLink) override;

    virtual void make_sorted() override;
    virtual bool changed_by_direct_pick() const override;
    virtual void set_entry_completion(bool bEnable, bool bCaseSensitive = false) override;
    virtual VclPtr<VirtualDevice> create_render_virtual_device() const override;

    virtual bool eventFilter(QObject* pObject, QEvent* pEvent) override;

private Q_SLOTS:
    void editTextChanged(const QString& rText);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
