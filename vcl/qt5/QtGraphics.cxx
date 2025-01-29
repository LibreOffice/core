/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <QtGraphics.hxx>

#include <QtData.hxx>
#include <QtFont.hxx>
#include <QtFrame.hxx>
#include <QtGraphics_Controls.hxx>
#include <QtPainter.hxx>

#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QtGraphics::QtGraphics( QtFrame *pFrame, QImage *pQImage )
    : m_pFrame( pFrame )
    , m_pTextStyle{ nullptr, }
    , m_aTextColor( 0x00, 0x00, 0x00 )
{
    m_pBackend = std::make_unique<QtGraphicsBackend>(m_pFrame, pQImage);

    if (!initWidgetDrawBackends(false))
    {
        if (!QtData::noNativeControls())
            m_pWidgetDraw.reset(new QtGraphics_Controls(*this));
    }
    if (m_pFrame)
        setDevicePixelRatioF(m_pFrame->devicePixelRatioF());
}

QtGraphics::~QtGraphics() { ReleaseFonts(); }

void QtGraphics::ChangeQImage(QImage* pQImage)
{
    m_pBackend->setQImage(pQImage);
    m_pBackend->ResetClipRegion();
}

SalGraphicsImpl* QtGraphics::GetImpl() const { return m_pBackend.get(); }

SystemGraphicsData QtGraphics::GetGraphicsData() const { return SystemGraphicsData(); }

void QtGraphics::handleDamage(const tools::Rectangle& rDamagedRegion)
{
    assert(m_pWidgetDraw);
    assert(dynamic_cast<QtGraphics_Controls*>(m_pWidgetDraw.get()));
    assert(!rDamagedRegion.IsEmpty());

    QImage* pImage = static_cast<QtGraphics_Controls*>(m_pWidgetDraw.get())->getImage();
    QImage blit(*pImage);
    blit.setDevicePixelRatio(1);
    QtPainter aPainter(*m_pBackend);
    aPainter.drawImage(QPoint(rDamagedRegion.Left(), rDamagedRegion.Top()), blit);
    aPainter.update(toQRect(rDamagedRegion));
}

void QtGraphics::GetResolution(sal_Int32& rDPIX, sal_Int32& rDPIY)
{
    QtGraphicsBase::ImplGetResolution(m_pFrame, rDPIX, rDPIY);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
