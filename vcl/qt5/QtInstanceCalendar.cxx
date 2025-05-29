/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceCalendar.hxx>
#include <QtInstanceCalendar.moc>

QtInstanceCalendar::QtInstanceCalendar(QCalendarWidget* pCalendarWidget)
    : QtInstanceWidget(pCalendarWidget)
    , m_pCalendarWidget(pCalendarWidget)
{
    assert(m_pCalendarWidget);

    connect(m_pCalendarWidget, &QCalendarWidget::activated, this,
            &QtInstanceCalendar::handleActivated);
    connect(m_pCalendarWidget, &QCalendarWidget::selectionChanged, this,
            &QtInstanceCalendar::handleSelectionChanged);
}

void QtInstanceCalendar::set_date(const Date& rDate)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { m_pCalendarWidget->setSelectedDate(toQDate(rDate)); });
}

Date QtInstanceCalendar::get_date() const
{
    SolarMutexGuard g;

    Date aDate(Date::DateInitEmpty::EMPTY);
    GetQtInstance().RunInMainThread([&] { aDate = toDate(m_pCalendarWidget->selectedDate()); });

    return aDate;
}

void QtInstanceCalendar::handleActivated()
{
    SolarMutexGuard g;
    signal_activated();
}

void QtInstanceCalendar::handleSelectionChanged()
{
    SolarMutexGuard g;
    signal_selected();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
