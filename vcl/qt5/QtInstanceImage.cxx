/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceImage.hxx>
#include <QtInstanceImage.moc>

#include <vcl/qt/QtUtils.hxx>

QtInstanceImage::QtInstanceImage(QLabel* pLabel)
    : QtInstanceWidget(pLabel)
    , m_pLabel(pLabel)
{
    assert(m_pLabel);
}

void QtInstanceImage::set_from_icon_name(const OUString& rIconName)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { m_pLabel->setPixmap(loadQPixmapIcon(rIconName)); });
}

void QtInstanceImage::set_image(VirtualDevice* pDevice)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        if (pDevice)
            m_pLabel->setPixmap(toQPixmap(*pDevice));
    });
}

void QtInstanceImage::set_image(const css::uno::Reference<css::graphic::XGraphic>& rGraphic)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { m_pLabel->setPixmap(toQPixmap(rGraphic)); });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
