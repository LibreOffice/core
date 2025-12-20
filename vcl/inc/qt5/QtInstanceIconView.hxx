/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtInstanceItemView.hxx"

#include <vcl/weld/IconView.hxx>

#include <QtGui/QStandardItemModel>
#include <QtWidgets/QListView>

class QtInstanceIconView : public QtInstanceItemView, public virtual weld::IconView
{
    Q_OBJECT

    QListView* m_pListView;
    QStandardItemModel* m_pModel;
    QItemSelectionModel* m_pSelectionModel;

public:
    QtInstanceIconView(QListView* pListView);

    virtual int get_item_width() const override;
    virtual void set_item_width(int width) override;

    virtual void do_insert(int nPos, const OUString* pStr, const OUString* pId,
                           const OUString* pIconName, weld::TreeIter* pRet) override;

    virtual void do_insert(int nPos, const OUString* pStr, const OUString* pId, const Bitmap* pIcon,
                           weld::TreeIter* pRet) override;

    virtual void insert_separator(int pos, const OUString* pId) override;

    virtual OUString get_selected_id() const override;

    virtual int count_selected_items() const override;

    virtual OUString get_selected_text() const override;

    virtual void do_select(const weld::TreeIter& rIter) override;
    virtual void do_unselect(const weld::TreeIter& rIter) override;

    virtual void set_image(int nPos, VirtualDevice& rDevice) override;
    virtual void set_text(int nPos, const OUString& rText) override;
    virtual void set_item_accessible_name(int nPos, const OUString& rName) override;
    virtual void set_item_tooltip_text(int nPos, const OUString& rToolTip) override;
    virtual void do_remove(int pos) override;
    virtual tools::Rectangle get_rect(const weld::TreeIter& rIter) const override;

    virtual bool get_selected(weld::TreeIter* pIter) const override;
    virtual OUString get_text(const weld::TreeIter& rIter) const override;
    virtual void do_scroll_to_item(const weld::TreeIter& rIter) override;

    virtual void selected_foreach(const std::function<bool(weld::TreeIter&)>& func) override;

    virtual int n_children() const override;

protected:
    virtual bool handleToolTipEvent(const QHelpEvent& rHelpEvent) override;

private:
    void do_insert(int nPos, const OUString* pStr, const OUString* pId, const QPixmap* pIcon,
                   weld::TreeIter* pRet);

private Q_SLOTS:
    void handleActivated();
    void handleSelectionChanged();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
