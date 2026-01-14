/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtTreeViewModel.hxx>
#include <QtTreeViewModel.moc>

#include <QtGui/QStandardItemModel>

// role used to indicate whether an item has on-demand children
constexpr int ROLE_CHILDREN_ON_DEMAND = Qt::UserRole + 2000;

QtTreeViewModel::QtTreeViewModel(QWidget* pParent)
    : QSortFilterProxyModel(pParent)
{
    setSourceModel(new QStandardItemModel(pParent));
}

Qt::ItemFlags QtTreeViewModel::flags(const QModelIndex& rIndex) const
{
    Qt::ItemFlags eFlags = QSortFilterProxyModel::flags(rIndex);

    if (!m_aEditableColumns.contains(rIndex.column()))
        eFlags &= ~Qt::ItemIsEditable;

    return eFlags;
}

bool QtTreeViewModel::hasChildren(const QModelIndex& rIndex) const
{
    if (getChildrenOnDemand(rIndex))
        return true;

    return QSortFilterProxyModel::hasChildren(rIndex);
}

void QtTreeViewModel::setEditableColumns(const std::unordered_set<int>& rEditableColumns)
{
    m_aEditableColumns = rEditableColumns;
}

bool QtTreeViewModel::getChildrenOnDemand(const QModelIndex& rIndex) const
{
    return data(rIndex, ROLE_CHILDREN_ON_DEMAND).toBool();
}

void QtTreeViewModel::setChildrenOnDemand(const QModelIndex& rIndex, bool bOnDemandChildren)
{
    setData(rIndex, QVariant(bOnDemandChildren), ROLE_CHILDREN_ON_DEMAND);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
