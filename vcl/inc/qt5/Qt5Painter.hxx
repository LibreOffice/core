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

#pragma once

#include <QtCore/QRectF>
#include <QtGui/QPainter>
#include <QtWidgets/QWidget>

#include "Qt5Frame.hxx"
#include "Qt5Graphics.hxx"

class Qt5Painter final : public QPainter
{
    Qt5GraphicsBackend& m_rGraphics;
    QRegion m_aRegion;

public:
    Qt5Painter(Qt5GraphicsBackend& rGraphics, bool bPrepareBrush = false,
               sal_uInt8 nTransparency = 255);
    ~Qt5Painter()
    {
        if (m_rGraphics.m_pFrame && !m_aRegion.isEmpty())
            m_rGraphics.m_pFrame->GetQWidget()->update(m_aRegion);
    }

    void update(int nx, int ny, int nw, int nh)
    {
        if (m_rGraphics.m_pFrame)
            m_aRegion += scaledQRect({ nx, ny, nw, nh }, 1 / m_rGraphics.devicePixelRatioF());
    }

    void update(const QRect& rRect)
    {
        if (m_rGraphics.m_pFrame)
            m_aRegion += scaledQRect(rRect, 1 / m_rGraphics.devicePixelRatioF());
    }

    void update(const QRectF& rRectF)
    {
        if (m_rGraphics.m_pFrame)
            update(scaledQRect(rRectF.toAlignedRect(), 1 / m_rGraphics.devicePixelRatioF()));
    }

    void update()
    {
        if (m_rGraphics.m_pFrame)
            m_aRegion += m_rGraphics.m_pFrame->GetQWidget()->rect();
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
