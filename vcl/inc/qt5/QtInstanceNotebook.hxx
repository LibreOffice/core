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
#include "QtInstanceWidget.hxx"

#include <QtWidgets/QTabWidget>

#include <map>

class QtInstanceNotebook : public QtInstanceWidget, public virtual weld::Notebook
{
    Q_OBJECT

    QTabWidget* m_pTabWidget;

    OUString m_sCurrentTabId;

    mutable std::map<QWidget*, std::unique_ptr<QtInstanceContainer>> m_aPageContainerInstances;

public:
    QtInstanceNotebook(QTabWidget* pTabWidget);

    virtual int get_current_page() const override;
    virtual int get_page_index(const OUString& rIdent) const override;
    virtual OUString get_page_ident(int nPage) const override;
    virtual OUString get_current_page_ident() const override;
    virtual void set_current_page(int nPage) override;
    virtual void set_current_page(const OUString& rIdent) override;
    virtual void remove_page(const OUString& rIdent) override;
    virtual void insert_page(const OUString& rIdent, const OUString& rLabel, int nPos,
                             const OUString* pIconName = nullptr) override;
    virtual void set_tab_label_text(const OUString& rIdent, const OUString& rLabel) override;
    virtual OUString get_tab_label_text(const OUString& rIdent) const override;
    virtual void set_show_tabs(bool bShow) override;
    virtual int get_n_pages() const override;
    virtual weld::Container* get_page(const OUString& rIdent) const override;

    static void setTabIdAndLabel(QTabWidget& rTabWidget, int nPage, const OUString& rIdent,
                                 const OUString& rLabel);

private Q_SLOTS:
    void currentTabChanged();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
