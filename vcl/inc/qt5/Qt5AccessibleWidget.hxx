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

#include <com/sun/star/accessibility/XAccessible.hpp>

class Qt5Frame;
class Qt5Widget;

class VCLPLUG_QT5_PUBLIC Qt5AccessibleWidget : public QObject, public QAccessibleInterface
{
    Q_OBJECT

public:
    Qt5AccessibleWidget(const css::uno::Reference<css::accessibility::XAccessible> xAccessible);
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

    bool isValid() const override;
    QObject* object() const override;
    void setText(QAccessible::Text t, const QString& text) override;
    QAccessibleInterface* childAt(int x, int y) const override;

    void* interface_cast(QAccessible::InterfaceType t) override;

    // QAccessibleActionInterface
    /*  QStringList actionNames() const override;
    void doAction(const QString& actionName) override;
    QStringList keyBindingsForAction(const QString& actionName) const override; */

    // Factory
    static QAccessibleInterface* customFactory(const QString& classname, QObject* object);

private:
    css::uno::Reference<css::accessibility::XAccessible> m_xAccessible;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
