/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vclpluginapi.h>

#include <QtCore/QObject>
#include <QtCore/QPair>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>
#include <QtGui/QAccessible>
#include <QtGui/QAccessibleActionInterface>
#include <QtGui/QAccessibleInterface>
#include <QtGui/QColor>
#include <QtGui/QWindow>

class Qt5Frame;

class VCLPLUG_QT5_PUBLIC Qt5AccessibleFrame : public QObject,
                                              public QAccessibleInterface,
                                              public QAccessibleActionInterface
{
    Q_OBJECT

public:
    Qt5AccessibleFrame(Qt5Frame* pFrame);
    QWindow* window() const override;
    int childCount() const override;
    int indexOfChild(const QAccessibleInterface* child) const override;
    QVector<QPair<QAccessibleInterface*, QAccessible::Relation>>
    relations(QAccessible::Relation match = QAccessible::AllRelations) const override;
    QAccessibleInterface* focusChild() const override;

    QRect rect() const override;

    QAccessibleInterface* parent() const override;
    QAccessibleInterface* child(int index) const override;

    QString text(QAccessible::Text t) const override;
    QAccessible::Role role() const override;
    QAccessible::State state() const override;

    QColor foregroundColor() const override;
    QColor backgroundColor() const override;

    void* interface_cast(QAccessible::InterfaceType t) override;

    // QAccessibleActionInterface
    QStringList actionNames() const override;
    void doAction(const QString& actionName) override;
    QStringList keyBindingsForAction(const QString& actionName) const override;

private:
    const Qt5Frame* m_pFrame;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
