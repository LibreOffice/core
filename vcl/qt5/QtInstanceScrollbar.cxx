/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceScrollbar.hxx>
#include <QtInstanceScrollbar.moc>

QtInstanceScrollbar::QtInstanceScrollbar(QScrollBar* pScrollBar)
    : QtInstanceWidget(pScrollBar)
    , m_pScrollBar(pScrollBar)
{
    assert(m_pScrollBar);

    connect(m_pScrollBar, &QScrollBar::valueChanged, this,
            &QtInstanceScrollbar::signalValueChanged);
}

void QtInstanceScrollbar::adjustment_configure(int, int, int, int, int, int)
{
    assert(false && "not implemented yet");
}

int QtInstanceScrollbar::adjustment_get_value() const
{
    SolarMutexGuard g;

    int nValue = 0;
    GetQtInstance().RunInMainThread([&] { nValue = m_pScrollBar->value(); });

    return nValue;
}

void QtInstanceScrollbar::adjustment_set_value(int nValue)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { m_pScrollBar->setValue(nValue); });
}

int QtInstanceScrollbar::adjustment_get_upper() const
{
    SolarMutexGuard g;

    int nMax = 0;
    GetQtInstance().RunInMainThread([&] { nMax = m_pScrollBar->maximum(); });

    return nMax;
}

void QtInstanceScrollbar::adjustment_set_upper(int nUpper)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { m_pScrollBar->setMaximum(nUpper); });
}

int QtInstanceScrollbar::adjustment_get_page_size() const
{
    assert(false && "Not implemented yet");
    return 0;
}

void QtInstanceScrollbar::adjustment_set_page_size(int) { assert(false && "not implemented yet"); }

int QtInstanceScrollbar::adjustment_get_page_increment() const
{
    SolarMutexGuard g;

    int nPageStep = 0;
    GetQtInstance().RunInMainThread([&] { nPageStep = m_pScrollBar->pageStep(); });

    return nPageStep;
}

void QtInstanceScrollbar::adjustment_set_page_increment(int nSize)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { m_pScrollBar->setPageStep(nSize); });
}

int QtInstanceScrollbar::adjustment_get_step_increment() const
{
    SolarMutexGuard g;

    int nSingleStep = 0;
    GetQtInstance().RunInMainThread([&] { nSingleStep = m_pScrollBar->singleStep(); });

    return nSingleStep;
}

void QtInstanceScrollbar::adjustment_set_step_increment(int nSize)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { m_pScrollBar->setSingleStep(nSize); });
}

int QtInstanceScrollbar::adjustment_get_lower() const
{
    SolarMutexGuard g;

    int nMin = 0;
    GetQtInstance().RunInMainThread([&] { nMin = m_pScrollBar->minimum(); });

    return nMin;
}

void QtInstanceScrollbar::adjustment_set_lower(int nLower)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { m_pScrollBar->setMinimum(nLower); });
}

int QtInstanceScrollbar::get_scroll_thickness() const
{
    SolarMutexGuard g;

    int nThickness = 0;
    GetQtInstance().RunInMainThread([&] {
        if (m_pScrollBar->orientation() == Qt::Horizontal)
            nThickness = m_pScrollBar->height();
        else
            nThickness = m_pScrollBar->width();
    });

    return nThickness;
}

void QtInstanceScrollbar::set_scroll_thickness(int nThickness)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        if (m_pScrollBar->orientation() == Qt::Horizontal)
            m_pScrollBar->resize(m_pScrollBar->width(), nThickness);
        else
            m_pScrollBar->resize(nThickness, m_pScrollBar->height());
    });
}

void QtInstanceScrollbar::set_scroll_swap_arrows(bool) { assert(false && "not implemented yet"); }

ScrollType QtInstanceScrollbar::get_scroll_type() const
{
    assert(false && "not implemented yet");
    return ScrollType::DontKnow;
}

void QtInstanceScrollbar::signalValueChanged()
{
    SolarMutexGuard g;
    signal_adjustment_value_changed();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
