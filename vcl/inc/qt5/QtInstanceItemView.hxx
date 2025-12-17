/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtInstanceTreeIter.hxx"
#include "QtInstanceWidget.hxx"

#include <vcl/weld/ItemView.hxx>

#include <QtCore/QAbstractItemModel>
#include <QtWidgets/QAbstractItemView>

class QtInstanceItemView : public QtInstanceWidget, public virtual weld::ItemView
{
    Q_OBJECT

    QAbstractItemModel& m_rModel;

protected:
    void do_clear() override;

public:
    QtInstanceItemView(QAbstractItemView* pItemView, QAbstractItemModel& rModel);

    virtual std::unique_ptr<weld::TreeIter> make_iterator(const weld::TreeIter* pOrig
                                                          = nullptr) const override;

    virtual std::unique_ptr<weld::TreeIter> get_iterator(int nPos) const override;

    virtual void select_all() override;
    virtual void unselect_all() override;

protected:
    QModelIndex modelIndex(int nRow, int nCol = 0,
                           const QModelIndex& rParentIndex = QModelIndex()) const;
    QModelIndex modelIndex(const weld::TreeIter& rIter, int nCol = 0) const;
    QtInstanceTreeIter treeIter(int nRow, const QModelIndex& rParentIndex = QModelIndex()) const;

private:
    QAbstractItemView& getItemView();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
