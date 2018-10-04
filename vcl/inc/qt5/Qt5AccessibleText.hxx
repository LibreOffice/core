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

#include "Qt5AccessibleWidget.hxx"

#include <QtCore/QObject>
#include <QtCore/QPair>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>
#include <QtGui/QAccessible>
#include <QtGui/QAccessibleActionInterface>
#include <QtGui/QAccessibleInterface>
#include <QtGui/QAccessibleValueInterface>
#include <QtGui/QColor>
#include <QtGui/QWindow>

#include <com/sun/star/accessibility/XAccessible.hpp>

class Qt5Frame;
class Qt5Widget;

class VCLPLUG_QT5_PUBLIC Qt5AccessibleText : public QAccessibleTextInterface
{
public:
    Qt5AccessibleText(const css::uno::Reference<css::accessibility::XAccessible> xAccessible);

    void addSelection(int startOffset, int endOffset) override;
    QString attributes(int offset, int* startOffset, int* endOffset) const override;
    int characterCount() const override;
    QRect characterRect(int offset) const override;
    int cursorPosition() const override;
    int offsetAtPoint(const QPoint& point) const override;
    void removeSelection(int selectionIndex) override;
    void scrollToSubstring(int startIndex, int endIndex) override;
    void selection(int selectionIndex, int* startOffset, int* endOffset) const override;
    int selectionCount() const override;
    void setCursorPosition(int position) override;
    void setSelection(int selectionIndex, int startOffset, int endOffset) override;
    QString text(int startOffset, int endOffset) const override;
    QString textAfterOffset(int offset, QAccessible::TextBoundaryType boundaryType,
                            int* startOffset, int* endOffset) const override;
    QString textAtOffset(int offset, QAccessible::TextBoundaryType boundaryType, int* startOffset,
                         int* endOffset) const override;
    QString textBeforeOffset(int offset, QAccessible::TextBoundaryType boundaryType,
                             int* startOffset, int* endOffset) const override;

private:
    css::uno::Reference<css::accessibility::XAccessible> m_xAccessible;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
