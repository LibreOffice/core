/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceImage.hxx>

#include <vcl/qt/QtUtils.hxx>

QtInstanceImage::QtInstanceImage(QLabel* pLabel)
    : QtInstanceWidget(pLabel)
    , m_pLabel(pLabel)
{
    assert(m_pLabel);
}

void QtInstanceImage::set_from_icon_name(const OUString&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceImage::set_image(VirtualDevice*) { assert(false && "Not implemented yet"); }

void QtInstanceImage::set_image(const css::uno::Reference<css::graphic::XGraphic>& rGraphic)
{
    m_pLabel->setPixmap(toQPixmap(rGraphic));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
