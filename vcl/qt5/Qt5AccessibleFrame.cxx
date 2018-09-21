/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <Qt5AccessibleFrame.hxx>
#include <Qt5AccessibleFrame.moc>

#include <Qt5Frame.hxx>

Qt5AccessibleFrame::Qt5AccessibleFrame(Qt5Frame* pFrame)
    : m_pFrame(pFrame)
{
}

QWindow* Qt5AccessibleFrame::window() const { return nullptr; }
int Qt5AccessibleFrame::childCount() const { return 0; }
int Qt5AccessibleFrame::indexOfChild(const QAccessibleInterface* /* child */) const { return 0; }
QVector<QPair<QAccessibleInterface*, QAccessible::Relation>>
    Qt5AccessibleFrame::relations(QAccessible::Relation /* match */) const
{
    return QVector<QPair<QAccessibleInterface*, QAccessible::Relation>>();
}
QAccessibleInterface* Qt5AccessibleFrame::focusChild() const
{
    return QAccessible::queryAccessibleInterface(object());
}

QRect Qt5AccessibleFrame::rect() const { return QRect(); }

QAccessibleInterface* Qt5AccessibleFrame::parent() const
{
    return QAccessible::queryAccessibleInterface(nullptr);
}
QAccessibleInterface* Qt5AccessibleFrame::child(int /* index */) const { return 0; }

QString Qt5AccessibleFrame::text(QAccessible::Text /* t */) const { return QString(); }
QAccessible::Role Qt5AccessibleFrame::role() const { return QAccessible::CheckBox; }
QAccessible::State Qt5AccessibleFrame::state() const
{
    QAccessible::State state;
    return state;
}

QColor Qt5AccessibleFrame::foregroundColor() const { return QColor(); }
QColor Qt5AccessibleFrame::backgroundColor() const { return QColor(); }

void* Qt5AccessibleFrame::interface_cast(QAccessible::InterfaceType t)
{
    if (t == QAccessible::ActionInterface)
        return static_cast<QAccessibleActionInterface*>(this);
    return nullptr;
}

// QAccessibleActionInterface
QStringList Qt5AccessibleFrame::actionNames() const
{
    QStringList actionNames;
    return actionNames;
}
void Qt5AccessibleFrame::doAction(const QString& /* actionName */) {}
QStringList Qt5AccessibleFrame::keyBindingsForAction(const QString& /* actionName */) const
{
    return QStringList();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
