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

void QtTreeViewModel::setEditableColumns(const std::unordered_set<int>& rEditableColumns)
{
    m_aEditableColumns = rEditableColumns;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
