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

#include "Qt5Graphics.hxx"
#include "Qt5Frame.hxx"

#include <QtWidgets/QWidget>

#include <QtGui/QPainter>

#include <QtGui/QImage>

Qt5Graphics::Qt5Graphics( Qt5Frame *pFrame )
    : m_pFrame( pFrame )
    , m_pQImage( nullptr )
    , m_pFontCollection( nullptr )
{
}

Qt5Graphics::Qt5Graphics( QImage *pQImage )
    : m_pFrame( nullptr )
    , m_pQImage( pQImage )
    , m_pFontCollection( nullptr )
{
}

Qt5Graphics::~Qt5Graphics()
{
}

void Qt5Graphics::PreparePainter()
{
    if ( m_pPainter.get() )
        return;
    if ( m_pQImage )
        m_pPainter.reset( new QPainter( m_pQImage ) );
    else
    {
        assert( dynamic_cast< QPaintDevice* >( m_pFrame->GetQWidget() ) );
        m_pPainter.reset( new QPainter( m_pFrame->GetQWidget() ) );
    }
    if (!m_aClipRegion.isEmpty())
        m_pPainter->setClipRegion( m_aClipRegion );
}

void Qt5Graphics::ChangeQImage( QImage *pQImage )
{
    m_pPainter.reset();
    m_pQImage = pQImage;
}

SalGraphicsImpl* Qt5Graphics::GetImpl() const
{
    return nullptr;
}

SystemGraphicsData Qt5Graphics::GetGraphicsData() const
{
    return SystemGraphicsData();
}

bool Qt5Graphics::supportsOperation( OutDevSupportType ) const
{
    return false;
}

#if ENABLE_CAIRO_CANVAS

bool Qt5Graphics::SupportsCairo() const
{
    return false;
}

cairo::SurfaceSharedPtr Qt5Graphics::CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const
{
    return nullptr;
}

cairo::SurfaceSharedPtr Qt5Graphics::CreateSurface(const OutputDevice& rRefDevice, int x, int y, int width, int height) const
{
    return nullptr;
}

cairo::SurfaceSharedPtr Qt5Graphics::CreateBitmapSurface(const OutputDevice& rRefDevice, const BitmapSystemData& rData, const Size& rSize) const
{
    return nullptr;
}

css::uno::Any Qt5Graphics::GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& rSurface, const basegfx::B2ISize& rSize) const
{
    return css::uno::Any();
}

SystemFontData Qt5Graphics::GetSysFontData( int nFallbacklevel ) const
{
    return SystemFontData();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
