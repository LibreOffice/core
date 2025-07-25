/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceColorChooserDialog.hxx>
#include <QtInstanceColorChooserDialog.moc>

QtInstanceColorChooserDialog::QtInstanceColorChooserDialog(QColorDialog* pColorDialog)
    : QtInstanceDialog(pColorDialog)
    , m_pColorDialog(pColorDialog)
{
    assert(m_pColorDialog);
}

QtInstanceColorChooserDialog::~QtInstanceColorChooserDialog() {}

void QtInstanceColorChooserDialog::set_color(const Color& rColor)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread(
        [this, &rColor] { m_pColorDialog->setCurrentColor(toQColor(rColor)); });
}

Color QtInstanceColorChooserDialog::get_color() const
{
    SolarMutexGuard g;

    Color aColor;
    GetQtInstance().RunInMainThread([&] { aColor = toColor(m_pColorDialog->currentColor()); });

    return aColor;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
