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

#include <QtSvpGraphics.hxx>
#include <QtSvpSalFrame.hxx>
#include <QtSvpSalFrame.moc>

void CairoDeleter::operator()(cairo_surface_t* pSurface) const { cairo_surface_destroy(pSurface); }

static void SvpDamageHandler(void* handle, sal_Int32 nExtentsX, sal_Int32 nExtentsY,
                             sal_Int32 nExtentsWidth, sal_Int32 nExtentsHeight)
{
    QtFrame* pThis = static_cast<QtFrame*>(handle);
    pThis->Damage(nExtentsX, nExtentsY, nExtentsWidth, nExtentsHeight);
}

QtSvpSalFrame::QtSvpSalFrame(QtFrame* pParent, SalFrameStyleFlags nSalFrameStyle)
    : QtFrame(pParent, nSalFrameStyle)
{
    m_aDamageHandler.handle = this;
    m_aDamageHandler.damaged = ::SvpDamageHandler;
}

QtSvpSalFrame::~QtSvpSalFrame() {}

SalGraphics* QtSvpSalFrame::GetGraphics() { return m_pSvpGraphics.get(); }

QImage QtSvpSalFrame::GetImage()
{
    cairo_surface_t* pSurface = m_pSurface.get();
    cairo_surface_flush(pSurface);

    return QImage(cairo_image_surface_get_data(pSurface), cairo_image_surface_get_width(pSurface),
                  cairo_image_surface_get_height(pSurface), Qt_DefaultFormat32);
}

SalGraphics* QtSvpSalFrame::DoAcquireGraphics(const QSize& rSize)
{
    if (!m_pSvpGraphics)
    {
        m_pSvpGraphics.reset(new QtSvpGraphics(this));
        m_pSurface.reset(
            cairo_image_surface_create(CAIRO_FORMAT_ARGB32, rSize.width(), rSize.height()));
        m_pSvpGraphics->setSurface(m_pSurface.get(),
                                   basegfx::B2IVector(rSize.width(), rSize.height()));
        cairo_surface_set_user_data(m_pSurface.get(), QtSvpGraphics::getDamageKey(),
                                    &m_aDamageHandler, nullptr);
    }
    return m_pSvpGraphics.get();
}

void QtSvpSalFrame::DoHandleResizeEvent(int nWidth, int nHeight)
{
    if (m_pSurface)
    {
        const int nOldWidth = cairo_image_surface_get_width(m_pSurface.get());
        const int nOldHeight = cairo_image_surface_get_height(m_pSurface.get());
        if (nOldWidth != nWidth || nOldHeight != nHeight)
        {
            cairo_surface_t* pSurface
                = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, nWidth, nHeight);
            cairo_surface_set_user_data(pSurface, SvpSalGraphics::getDamageKey(), &m_aDamageHandler,
                                        nullptr);
            m_pSvpGraphics->setSurface(pSurface, basegfx::B2IVector(nWidth, nHeight));
            UniqueCairoSurface old_surface(m_pSurface.release());
            m_pSurface.reset(pSurface);

            const int nMinWidth = qMin(nOldWidth, nWidth);
            const int nMinHeight = qMin(nOldHeight, nHeight);
            SalTwoRect rect(0, 0, nMinWidth, nMinHeight, 0, 0, nMinWidth, nMinHeight);
            m_pSvpGraphics->copySource(rect, old_surface.get());
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
