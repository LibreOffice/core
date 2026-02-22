/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <QtWidgets/QAccessibleWidget>

/**
 * Accessibility implementation for Qt widget that is the (single) direct
 * accessible child of a VCL widget when a native Qt widget (sub)tree is used
 * inside vcl widgets.
 *
 * This is used to ensure that the vcl widget is reported as the Qt widget's
 * accessible parent.
 *
 * The counterpart to ensure that the Qt widget is reported as the vcl widget's
 * accessible child is QtAccessibleInterimParentWidget.
 *
 * See also QtInstance::CreateInterimBuilder.
 */
class QtAccessibleInterimChildWidget : public QAccessibleWidget
{
    QObject* const m_pParent;

public:
    static constexpr const char* PROPERTY_INTERIM_PARENT = "interim-parent";

    QtAccessibleInterimChildWidget(QWidget* w, QObject* pParent,
                                   QAccessible::Role eRole = QAccessible::Client,
                                   const QString& rName = QString());

    virtual QAccessibleInterface* parent() const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
