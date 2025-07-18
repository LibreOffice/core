/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceCheckButton.hxx>
#include <QtInstanceCheckButton.moc>

#include <vcl/qt/QtUtils.hxx>

QtInstanceCheckButton::QtInstanceCheckButton(QCheckBox* pCheckBox)
    : QtInstanceWidget(pCheckBox)
    , m_pCheckBox(pCheckBox)
{
    assert(m_pCheckBox);
    connect(m_pCheckBox, &QCheckBox::toggled, this, &QtInstanceCheckButton::handleToggled);
}

void QtInstanceCheckButton::set_state(TriState eState)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] {
        m_pCheckBox->setTristate(eState == TRISTATE_INDET);
        m_pCheckBox->setCheckState(toQtCheckState(eState));
    });
}

TriState QtInstanceCheckButton::get_state() const
{
    SolarMutexGuard g;
    TriState eState = TRISTATE_INDET;
    GetQtInstance().RunInMainThread([&] { eState = toVclTriState(m_pCheckBox->checkState()); });
    return eState;
}

void QtInstanceCheckButton::set_label(const OUString& rText)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread(
        [&] { m_pCheckBox->setText(vclToQtStringWithAccelerator(rText)); });
}
OUString QtInstanceCheckButton::get_label() const
{
    SolarMutexGuard g;
    OUString sLabel;
    GetQtInstance().RunInMainThread(
        [&] { sLabel = qtToVclStringWithAccelerator(m_pCheckBox->text()); });
    return sLabel;
}

void QtInstanceCheckButton::QtInstanceCheckButton::set_label_wrap(bool /*bWrap*/)
{
    assert(false && "Not implemented yet");
}

void QtInstanceCheckButton::handleToggled()
{
    SolarMutexGuard g;
    signal_toggled();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
