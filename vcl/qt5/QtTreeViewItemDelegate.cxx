/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtTreeViewItemDelegate.hxx>
#include <QtTreeViewItemDelegate.moc>

#include <QtWidgets/QLineEdit>

QtTreeViewItemDelegate::QtTreeViewItemDelegate(
    QObject* pParent, std::function<bool(const QModelIndex& rIndex)> aStartEditingFunction,
    std::function<bool(const QModelIndex& rIndex, const QString& rNewText)> aFinishEditingFunction)
    : QStyledItemDelegate(pParent)
    , m_aStartEditingFunction(aStartEditingFunction)
    , m_aFinishEditingFunction(aFinishEditingFunction)
{
}

QWidget* QtTreeViewItemDelegate::createEditor(QWidget* pParent, const QStyleOptionViewItem& rOption,
                                              const QModelIndex& rIndex) const
{
    if (!m_aStartEditingFunction(rIndex))
        return nullptr;

    m_pEditor = QStyledItemDelegate::createEditor(pParent, rOption, rIndex);
    return m_pEditor;
}

void QtTreeViewItemDelegate::setModelData(QWidget* pEditor, QAbstractItemModel* pModel,
                                          const QModelIndex& rIndex) const
{
    // check whether new string is accepted
    if (QLineEdit* pLineEdit = qobject_cast<QLineEdit*>(pEditor))
    {
        if (!m_aFinishEditingFunction(rIndex, pLineEdit->text()))
            return;
    }

    QStyledItemDelegate::setModelData(pEditor, pModel, rIndex);
}

void QtTreeViewItemDelegate::endEditing() { closeEditor(m_pEditor); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
