/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceRadioButton.hxx>
#include <QtInstanceRadioButton.moc>

#include <vcl/qt/QtUtils.hxx>

QtInstanceRadioButton::QtInstanceRadioButton(QRadioButton* pRadioButton)
    : QtInstanceWidget(pRadioButton)
    , m_pRadioButton(pRadioButton)
{
    assert(m_pRadioButton);
    connect(m_pRadioButton, &QRadioButton::toggled, this, [&] { signal_toggled(); });
}

void QtInstanceRadioButton::set_active(bool bActive)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pRadioButton->setChecked(bActive); });
}

bool QtInstanceRadioButton::get_active() const
{
    SolarMutexGuard g;
    bool bActive;
    GetQtInstance().RunInMainThread([&] { bActive = m_pRadioButton->isChecked(); });
    return bActive;
}

void QtInstanceRadioButton::set_inconsistent(bool) { assert(false && "Not implemented yet"); }

bool QtInstanceRadioButton::get_inconsistent() const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceRadioButton::set_label(const OUString& rText)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pRadioButton->setText(toQString(rText)); });
}
OUString QtInstanceRadioButton::get_label() const
{
    SolarMutexGuard g;
    OUString sLabel;
    GetQtInstance().RunInMainThread([&] { sLabel = toOUString(m_pRadioButton->text()); });
    return sLabel;
}

void QtInstanceRadioButton::QtInstanceRadioButton::set_label_wrap(bool /*bWrap*/)
{
    assert(false && "Not implemented yet");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
