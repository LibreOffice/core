/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtAccessibleInterimParentWidget.hxx>

QtAccessibleInterimParentWidget::QtAccessibleInterimParentWidget(
    const css::uno::Reference<css::accessibility::XAccessible>& xAccessible, QObject& rObject,
    QWidget* pNativeChild)
    : QtAccessibleWidget(xAccessible, rObject)
    , m_pNativeChild(pNativeChild)
{
}

int QtAccessibleInterimParentWidget::childCount() const { return 1; }

QAccessibleInterface* QtAccessibleInterimParentWidget::child(int index) const
{
    if (index == 0)
        return QAccessible::queryAccessibleInterface(m_pNativeChild);

    return nullptr;
}

int QtAccessibleInterimParentWidget::indexOfChild(const QAccessibleInterface* pChild) const
{
    if (pChild && pChild->object() == m_pNativeChild)
        return 0;

    return -1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
