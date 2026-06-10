/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <QtInstanceToggleButton.hxx>
#include <QtInstanceToggleButton.moc>

QtInstanceToggleButton::QtInstanceToggleButton(QToolButton* pButton)
    : QtInstanceButton(pButton)
    , m_pToolButton(pButton)
{
    assert(m_pToolButton);

    connect(m_pToolButton, &QAbstractButton::toggled, this, &QtInstanceToggleButton::signalToggled);
}

void QtInstanceToggleButton::set_label(const OUString& rText)
{
    QtInstanceButton::set_label(rText);
    updateToolButtonStyle(getToolButton());
}

void QtInstanceToggleButton::do_set_active(bool bActive)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { m_pToolButton->setChecked(bActive); });
}

bool QtInstanceToggleButton::get_active() const
{
    SolarMutexGuard g;

    bool bActive = false;
    GetQtInstance().RunInMainThread([&] { bActive = m_pToolButton->isChecked(); });

    return bActive;
};

void QtInstanceToggleButton::updateToolButtonStyle(QToolButton& rToolButton)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        // set tool button style to ensure that text is shown when not empty,
        // and no unnecessary space is reserved for text if it is empty
        const Qt::ToolButtonStyle eStyle
            = rToolButton.text().isEmpty() ? Qt::ToolButtonIconOnly : Qt::ToolButtonTextBesideIcon;
        rToolButton.setToolButtonStyle(eStyle);
    });
}

QToolButton& QtInstanceToggleButton::getToolButton() const
{
    assert(m_pToolButton);
    return *m_pToolButton;
}

void QtInstanceToggleButton::signalToggled()
{
    SolarMutexGuard g;
    signal_toggled();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
