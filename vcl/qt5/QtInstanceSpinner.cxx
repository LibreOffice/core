/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceSpinner.hxx>
#include <QtInstanceSpinner.moc>

QtInstanceSpinner::QtInstanceSpinner(QProgressBar* pProgressBar)
    : QtInstanceWidget(pProgressBar)
{
    assert(pProgressBar);

    // This implementation relies on using a QProgressBar in undeterminate state,
    // i.e. with range [0, 0] and only shows/hides that progress bar
    // to make the animation (in)visible.
    assert(pProgressBar->minimum() == 0 && pProgressBar->maximum() == 0
           && "Progressbar is not in undeterminate state");
}

void QtInstanceSpinner::start() { show(); }

void QtInstanceSpinner::stop() { hide(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
