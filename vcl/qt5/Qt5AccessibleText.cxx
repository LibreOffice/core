/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <Qt5AccessibleText.hxx>

#include <QtGui/QAccessibleInterface>

#include <Qt5Frame.hxx>
#include <Qt5Tools.hxx>
#include <Qt5Widget.hxx>
#include <Qt5XAccessible.hxx>

#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleKeyBinding.hpp>
#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <comphelper/AccessibleImplementationHelper.hxx>
#include <sal/log.hxx>
#include <vcl/popupmenuwindow.hxx>

using namespace css;
using namespace css::accessibility;
using namespace css::uno;

Qt5AccessibleText::Qt5AccessibleText(const Reference<XAccessible> xAccessible)
    : m_xAccessible(xAccessible)
{
}

void Qt5AccessibleText::addSelection(int /* startOffset */, int /* endOffset */) {}
QString Qt5AccessibleText::attributes(int /* offset */, int* /* startOffset */,
                                      int* /* endOffset */) const
{
    return QString();
}
int Qt5AccessibleText::characterCount() const { return 0; }
QRect Qt5AccessibleText::characterRect(int /* offset */) const { return QRect(); }
int Qt5AccessibleText::cursorPosition() const { return 0; }
int Qt5AccessibleText::offsetAtPoint(const QPoint& /* point */) const { return 0; }
void Qt5AccessibleText::removeSelection(int /* selectionIndex */) {}
void Qt5AccessibleText::scrollToSubstring(int /* startIndex */, int /* endIndex */) {}
void Qt5AccessibleText::selection(int /* selectionIndex */, int* /* startOffset */,
                                  int* /* endOffset */) const
{
}
int Qt5AccessibleText::selectionCount() const { return 0; }
void Qt5AccessibleText::setCursorPosition(int /* position */) {}
void Qt5AccessibleText::setSelection(int /* selectionIndex */, int /* startOffset */,
                                     int /* endOffset */)
{
}
QString Qt5AccessibleText::text(int /* startOffset */, int /* endOffset */) const
{
    return QString();
}
QString Qt5AccessibleText::textAfterOffset(int /* offset */,
                                           QAccessible::TextBoundaryType /* boundaryType */,
                                           int* /* startOffset */, int* /* endOffset */) const
{
    return QString();
}
QString Qt5AccessibleText::textAtOffset(int /* offset */,
                                        QAccessible::TextBoundaryType /* boundaryType */,
                                        int* /* startOffset */, int* /* endOffset */) const
{
    return QString();
}
QString Qt5AccessibleText::textBeforeOffset(int /* offset */,
                                            QAccessible::TextBoundaryType /* boundaryType */,
                                            int* /* startOffset */, int* /* endOffset */) const
{
    return QString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
