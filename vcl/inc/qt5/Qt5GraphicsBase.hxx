/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <QtWidgets/QApplication>

class Qt5GraphicsBase
{
    qreal m_fDPR;

public:
    Qt5GraphicsBase()
        : m_fDPR(qApp ? qApp->devicePixelRatio() : 1.0)
    {
    }

    void setDevicePixelRatioF(qreal fDPR) { m_fDPR = fDPR; }

    qreal devicePixelRatioF() const { return m_fDPR; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
