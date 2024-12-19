/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceScale.hxx>
#include <QtInstanceScale.moc>

QtInstanceScale::QtInstanceScale(QSlider* pSlider)
    : QtInstanceWidget(pSlider)
    , m_pSlider(pSlider)
{
    assert(m_pSlider);
    connect(m_pSlider, &QSlider::valueChanged, this, &QtInstanceScale::handleValueChanged);
}

void QtInstanceScale::set_value(int nValue)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { m_pSlider->setValue(nValue); });
}

int QtInstanceScale::get_value() const
{
    SolarMutexGuard g;

    int nValue;
    GetQtInstance().RunInMainThread([&] { nValue = m_pSlider->value(); });

    return nValue;
}

void QtInstanceScale::set_range(int nMin, int nMax)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        m_pSlider->setMinimum(nMin);
        m_pSlider->setMaximum(nMax);
    });
}

void QtInstanceScale::set_increments(int nStep, int nPage)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        m_pSlider->setSingleStep(nStep);
        m_pSlider->setPageStep(nPage);
    });
}
void QtInstanceScale::get_increments(int& rStep, int& rPage) const
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        rStep = m_pSlider->singleStep();
        rPage = m_pSlider->pageStep();
    });
}

void QtInstanceScale::handleValueChanged()
{
    SolarMutexGuard aGuard;
    signal_value_changed();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
