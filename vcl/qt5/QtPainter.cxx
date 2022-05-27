/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtPainter.hxx>

#include <QtGui/QColor>

QtPainter::QtPainter(QtGraphicsBackend& rGraphics, bool bPrepareBrush, sal_uInt8 nTransparency)
    : m_rGraphics(rGraphics)
{
    if (rGraphics.m_pQImage)
    {
        if (!begin(rGraphics.m_pQImage))
            std::abort();
    }
    else
    {
        assert(rGraphics.m_pFrame);
        if (!begin(rGraphics.m_pFrame->GetQWidget()))
            std::abort();
    }
    if (!rGraphics.m_aClipPath.isEmpty())
        setClipPath(rGraphics.m_aClipPath);
    else
        setClipRegion(rGraphics.m_aClipRegion);
    if (SALCOLOR_NONE != rGraphics.m_aLineColor)
    {
        QColor aColor = toQColor(rGraphics.m_aLineColor);
        aColor.setAlpha(nTransparency);
        setPen(aColor);
    }
    else
        setPen(Qt::NoPen);
    if (bPrepareBrush && SALCOLOR_NONE != rGraphics.m_aFillColor)
    {
        QColor aColor = toQColor(rGraphics.m_aFillColor);
        aColor.setAlpha(nTransparency);
        setBrush(aColor);
    }
    setCompositionMode(rGraphics.m_eCompositionMode);
    setRenderHint(QPainter::Antialiasing, m_rGraphics.getAntiAlias());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
