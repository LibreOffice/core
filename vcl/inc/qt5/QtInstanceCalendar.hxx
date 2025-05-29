/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtInstanceWidget.hxx"

#include <QtWidgets/QCalendarWidget>

class QtInstanceCalendar : public QtInstanceWidget, public virtual weld::Calendar
{
    Q_OBJECT

    QCalendarWidget* m_pCalendarWidget;

public:
    QtInstanceCalendar(QCalendarWidget* pCalendarWidget);

    virtual void set_date(const Date& rDate) override;
    virtual Date get_date() const override;

private Q_SLOTS:
    void handleActivated();
    void handleSelectionChanged();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
