/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <vcl/weld/TreeView.hxx>

#include <QtCore/QSortFilterProxyModel>
#include <QtWidgets/QWidget>

#include <unordered_set>
#include <vector>

/** Item model for QtInstanceTreeView. */
class QtTreeViewModel : public QSortFilterProxyModel
{
    Q_OBJECT

    std::unordered_set<int> m_aCenteredColumns;
    std::vector<int> m_aEditableColumns;

public:
    QtTreeViewModel(QWidget* pParent);

    virtual Qt::ItemFlags flags(const QModelIndex& rIndex) const override;

    virtual bool hasChildren(const QModelIndex& rIndex = QModelIndex()) const override;

    QVariant data(const QModelIndex& rIndex, int nRole = Qt::DisplayRole) const override;

    void setCenteredColumn(int nCol);

    const std::vector<int>& editableColumns() const;
    void setEditableColumns(const std::vector<int>& rEditableColumns);

    bool getChildrenOnDemand(const QModelIndex& rIndex) const;
    void setChildrenOnDemand(const QModelIndex& rIndex, bool bOnDemandChildren);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
