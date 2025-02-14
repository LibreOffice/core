/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtFrame.hxx"
#include "QtInstance.hxx"

#include <QtWidgets/QApplication>

#include <sal/types.h>

class QtGraphicsBase
{
    qreal m_fDPR;

public:
    QtGraphicsBase()
        : m_fDPR(qApp ? GetQtInstance().EmscriptenLightweightRunInMainThread(
                            [] { return qApp->devicePixelRatio(); })
                      : 1.0)
    {
    }

    void setDevicePixelRatioF(qreal fDPR) { m_fDPR = fDPR; }

    qreal devicePixelRatioF() const { return m_fDPR; }

protected:
    static void ImplGetResolution(QtFrame* pFrame, sal_Int32& rDPIX, sal_Int32& rDPIY);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
