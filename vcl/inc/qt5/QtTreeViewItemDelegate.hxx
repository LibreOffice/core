/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <QtCore/QPointer>
#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QWidget>

/** Item delegate for QtInstanceTreeView. */
class QtTreeViewItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    std::function<bool(const QModelIndex& rIndex)> m_aStartEditingFunction;
    std::function<bool(const QModelIndex& rIndex, const QString& rNewText)>
        m_aFinishEditingFunction;

    mutable QPointer<QWidget> m_pEditor;

public:
    QtTreeViewItemDelegate(QObject* pParent,
                           std::function<bool(const QModelIndex& rIndex)> aStartEditingFunction,
                           std::function<bool(const QModelIndex& rIndex, const QString& rNewText)>
                               aEndEditingFunction);

    virtual QWidget* createEditor(QWidget* pParent, const QStyleOptionViewItem& rOption,
                                  const QModelIndex& rIndex) const override;
    virtual void setModelData(QWidget* pEditor, QAbstractItemModel* pModel,
                              const QModelIndex& rIndex) const override;

    void endEditing();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
