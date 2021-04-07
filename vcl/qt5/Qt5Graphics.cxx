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

#include <Qt5Graphics.hxx>

#include <Qt5Data.hxx>
#include <Qt5Font.hxx>
#include <Qt5Frame.hxx>
#include <Qt5Graphics_Controls.hxx>
#include <Qt5Painter.hxx>

#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

Qt5Graphics::Qt5Graphics( Qt5Frame *pFrame, QImage *pQImage )
    : m_pFrame( pFrame )
    , m_pTextStyle{ nullptr, }
    , m_aTextColor( 0x00, 0x00, 0x00 )
{
    m_pBackend = std::make_unique<Qt5GraphicsBackend>(m_pFrame, pQImage);

    if (!initWidgetDrawBackends(false))
    {
        if (!Qt5Data::noNativeControls())
            m_pWidgetDraw.reset(new Qt5Graphics_Controls(*this));
    }
    if (m_pFrame)
        setDevicePixelRatioF(m_pFrame->devicePixelRatioF());
}

Qt5Graphics::~Qt5Graphics() { ReleaseFonts(); }

void Qt5Graphics::ChangeQImage(QImage* pQImage)
{
    m_pBackend->setQImage(pQImage);
    m_pBackend->ResetClipRegion();
}

SalGraphicsImpl* Qt5Graphics::GetImpl() const { return m_pBackend.get(); }

SystemGraphicsData Qt5Graphics::GetGraphicsData() const { return SystemGraphicsData(); }

#if ENABLE_CAIRO_CANVAS

bool Qt5Graphics::SupportsCairo() const { return false; }

cairo::SurfaceSharedPtr
Qt5Graphics::CreateSurface(const cairo::CairoSurfaceSharedPtr& /*rSurface*/) const
{
    return nullptr;
}

cairo::SurfaceSharedPtr Qt5Graphics::CreateSurface(const OutputDevice& /*rRefDevice*/, int /*x*/,
                                                   int /*y*/, int /*width*/, int /*height*/) const
{
    return nullptr;
}

cairo::SurfaceSharedPtr Qt5Graphics::CreateBitmapSurface(const OutputDevice& /*rRefDevice*/,
                                                         const BitmapSystemData& /*rData*/,
                                                         const Size& /*rSize*/) const
{
    return nullptr;
}

css::uno::Any Qt5Graphics::GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& /*rSurface*/,
                                                  const basegfx::B2ISize& /*rSize*/) const
{
    return css::uno::Any();
}

#endif

void Qt5Graphics::handleDamage(const tools::Rectangle& rDamagedRegion)
{
    assert(m_pWidgetDraw);
    assert(dynamic_cast<Qt5Graphics_Controls*>(m_pWidgetDraw.get()));
    assert(!rDamagedRegion.IsEmpty());

    QImage* pImage = static_cast<Qt5Graphics_Controls*>(m_pWidgetDraw.get())->getImage();
    QImage blit(*pImage);
    blit.setDevicePixelRatio(1);
    Qt5Painter aPainter(*m_pBackend);
    aPainter.drawImage(QPoint(rDamagedRegion.getX(), rDamagedRegion.getY()), blit);
    aPainter.update(toQRect(rDamagedRegion));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
