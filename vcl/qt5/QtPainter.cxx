/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
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

QtPainter::~QtPainter()
{
    if (!m_rGraphics.m_pFrame || m_aRegion.isEmpty())
        return;

    QWidget* pWidget = m_rGraphics.m_pFrame->GetQWidget();
    QRect aParentUpdateRect(
        scaledQRect(m_aRegion.boundingRect(), 1 / m_rGraphics.devicePixelRatioF()));
    if (!m_rGraphics.m_pFrame->GetTopLevelWindow())
        pWidget->update(m_aRegion);
    else
    {
        QRect aIntersectedRect(aParentUpdateRect.intersected(pWidget->geometry()));
        if (!aIntersectedRect.isEmpty())
            pWidget->update(aIntersectedRect.translated(-pWidget->geometry().topLeft()));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
