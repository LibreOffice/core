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

#include <QtGui/QStandardItemModel>
#include <QtWidgets/QListView>

class QtInstanceIconView : public QtInstanceWidget, public virtual weld::IconView
{
    Q_OBJECT

    QListView* m_pListView;
    QStandardItemModel* m_pModel;
    QItemSelectionModel* m_pSelectionModel;

public:
    QtInstanceIconView(QListView* pListView);

    virtual int get_item_width() const override;
    virtual void set_item_width(int width) override;

    virtual void insert(int nPos, const OUString* pStr, const OUString* pId,
                        const OUString* pIconName, weld::TreeIter* pRet) override;

    virtual void insert(int nPos, const OUString* pStr, const OUString* pId,
                        const VirtualDevice* pIcon, weld::TreeIter* pRet) override;

    virtual void insert_separator(int pos, const OUString* pId) override;

    virtual OUString get_selected_id() const override;

    virtual void clear() override;

    virtual int count_selected_items() const override;

    virtual OUString get_selected_text() const override;

    virtual OUString get_id(int nPos) const override;
    virtual void select(int nPos) override;
    virtual void unselect(int pos) override;
    virtual void set_image(int nPos, VirtualDevice& rDevice) override;
    virtual void set_text(int nPos, const OUString& rText) override;
    virtual void set_id(int nPos, const OUString& rId) override;
    virtual void set_item_accessible_name(int nPos, const OUString& rName) override;
    virtual void remove(int pos) override;
    virtual tools::Rectangle get_rect(int pos) const override;

    virtual std::unique_ptr<weld::TreeIter> make_iterator(const weld::TreeIter* pOrig
                                                          = nullptr) const override;
    virtual bool get_selected(weld::TreeIter* pIter) const override;
    virtual bool get_cursor(weld::TreeIter* pIter) const override;
    virtual void set_cursor(const weld::TreeIter& rIter) override;
    virtual bool get_iter_first(weld::TreeIter& rIter) const override;
    virtual OUString get_id(const weld::TreeIter& rIter) const override;
    virtual OUString get_text(const weld::TreeIter& rIter) const override;
    virtual bool iter_next_sibling(weld::TreeIter& rIter) const override;
    virtual void scroll_to_item(const weld::TreeIter& rIter) override;

    virtual void selected_foreach(const std::function<bool(weld::TreeIter&)>& func) override;

    virtual int n_children() const override;

    virtual bool eventFilter(QObject* pObject, QEvent* pEvent) override;

private:
    QModelIndex modelIndex(int nPos) const;
    QModelIndex modelIndex(const weld::TreeIter& rIter) const;
    static int position(const weld::TreeIter& rIter);

    bool handleToolTipEvent(QHelpEvent* pEvent);

private Q_SLOTS:
    void handleActivated();
    void handleSelectionChanged();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
