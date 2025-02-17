/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtGraphicsBase.hxx>
#include <QtInstance.hxx>

#include <QtGui/QScreen>

#include <o3tl/string_view.hxx>

void QtGraphicsBase::ImplGetResolution(QtFrame* pFrame, sal_Int32& rDPIX, sal_Int32& rDPIY)
{
    char* pForceDpi;
    if ((pForceDpi = getenv("SAL_FORCEDPI")))
    {
        rDPIX = rDPIY = o3tl::toInt32(std::string_view(pForceDpi));
        return;
    }

    if (!pFrame)
        return;

    QScreen* pScreen = pFrame->GetQWidget()->screen();
    qreal devicePixelRatio = GetQtInstance().EmscriptenLightweightRunInMainThread(
        [pScreen] { return pScreen->devicePixelRatio(); });
    rDPIX = pScreen->logicalDotsPerInchX() * devicePixelRatio + 0.5;
    rDPIY = pScreen->logicalDotsPerInchY() * devicePixelRatio + 0.5;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
