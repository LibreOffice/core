/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceToggleButton.hxx>
#include <QtInstanceToggleButton.moc>

QtInstanceToggleButton::QtInstanceToggleButton(QAbstractButton* pButton)
    : QtInstanceButton(pButton)
{
    assert(pButton);
    pButton->setCheckable(true);
}

void QtInstanceToggleButton::set_active(bool bActive)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { getButton().setChecked(bActive); });
}

bool QtInstanceToggleButton::get_active() const
{
    SolarMutexGuard g;

    bool bActive = false;
    GetQtInstance().RunInMainThread([&] { bActive = getButton().isChecked(); });

    return bActive;
};

void QtInstanceToggleButton::set_inconsistent(bool bInconsistent)
{
    assert(!bInconsistent && "Inconsistent state not supported (yet?)");
    (void)bInconsistent;
};

bool QtInstanceToggleButton::get_inconsistent() const
{
    assert(false && "Not implemented yet");
    return false;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
