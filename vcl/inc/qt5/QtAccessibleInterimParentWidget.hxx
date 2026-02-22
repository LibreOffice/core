/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtAccessibleWidget.hxx"

#include <QtWidgets/QWidget>

/**
 * Accessibility implementation for the vcl widget that is the (accessible)
 * parent of a Qt widget when a native Qt widget (sub)tree is used
 * inside vcl widgets.
 *
 * This is used to ensure that the Qt widget is reported as the vcl widget's
 * accessible child.
 *
 * The counterpart to ensure that the vcl widget is reported as the Qt widget's
 * accessible parent is QtAccessibleInterimChildWidget.
 *
 * See also QtInstance::CreateInterimBuilder.
 */
class QtAccessibleInterimParentWidget : public QtAccessibleWidget
{
    QWidget* m_pNativeChild = nullptr;

public:
    QtAccessibleInterimParentWidget(
        const css::uno::Reference<css::accessibility::XAccessible>& xAccessible, QObject& rObject,
        QWidget* pNativeChild);

    int childCount() const override;
    QAccessibleInterface* child(int index) const override;
    int indexOfChild(const QAccessibleInterface* pChild) const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
