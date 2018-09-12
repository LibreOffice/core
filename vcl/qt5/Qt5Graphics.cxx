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

#include <Qt5Font.hxx>
#include <Qt5Frame.hxx>
#include <Qt5Painter.hxx>

#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

#include <vcl/sysdata.hxx>

Qt5Graphics::Qt5Graphics( Qt5Frame *pFrame, QImage *pQImage )
    : m_pFrame( pFrame )
    , m_pQImage( pQImage )
    , m_aLineColor( 0x00, 0x00, 0x00 )
    , m_aFillColor( 0xFF, 0xFF, 0XFF )
    , m_eCompositionMode( QPainter::CompositionMode_SourceOver )
    , m_pFontCollection( nullptr )
    , m_pTextStyle{ nullptr, }
    , m_aTextColor( 0x00, 0x00, 0x00 )
{
    ResetClipRegion();
}

Qt5Graphics::~Qt5Graphics()
{
    // release the text styles
    for (int i = 0; i < MAX_FALLBACK; ++i)
    {
        if (!m_pTextStyle[i])
            break;
        m_pTextStyle[i].clear();
    }
}

void Qt5Graphics::ChangeQImage(QImage* pQImage)
{
    m_pQImage = pQImage;
    ResetClipRegion();
}

SalGraphicsImpl* Qt5Graphics::GetImpl() const { return nullptr; }

SystemGraphicsData Qt5Graphics::GetGraphicsData() const { return SystemGraphicsData(); }

bool Qt5Graphics::supportsOperation(OutDevSupportType eType) const
{
    switch (eType)
    {
        case OutDevSupportType::B2DDraw:
        case OutDevSupportType::TransparentRect:
            return true;
        default:
            return false;
    }
}

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

SystemFontData Qt5Graphics::GetSysFontData(int /*nFallbacklevel*/) const
{
    return SystemFontData();
}

#endif

bool Qt5Graphics::drawNativeControl(ControlType nType, ControlPart nPart,
                                    const tools::Rectangle& rControlRegion, ControlState nState,
                                    const ImplControlValue& aValue, const OUString& aCaption)
{
    bool bHandled
        = m_aControl.drawNativeControl(nType, nPart, rControlRegion, nState, aValue, aCaption);
    if (bHandled)
    {
        Qt5Painter aPainter(*this);
        aPainter.drawImage(QPoint(rControlRegion.getX(), rControlRegion.getY()),
                           m_aControl.getImage());
        aPainter.update(toQRect(rControlRegion));
    }
    return bHandled;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
