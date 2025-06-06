/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceLevelBar.hxx>
#include <QtInstanceLevelBar.moc>

QtInstanceLevelBar::QtInstanceLevelBar(QProgressBar* pProgressBar)
    : QtInstanceWidget(pProgressBar)
    , m_pProgressBar(pProgressBar)
{
    assert(pProgressBar);
}

void QtInstanceLevelBar::set_percentage(double fPercentage)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pProgressBar->setValue(std::round(fPercentage)); });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
