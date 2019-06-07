/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "bufferdevice.hxx"

namespace vcl
{
BufferDevice::BufferDevice(const VclPtr<vcl::Window>& pWindow, vcl::RenderContext& rRenderContext)
    : m_pBuffer(VclPtr<VirtualDevice>::Create(rRenderContext))
    , m_pWindow(pWindow)
    , m_rRenderContext(rRenderContext)
{
    m_pBuffer->SetOutputSizePixel(pWindow->GetOutputSizePixel(), false);
    m_pBuffer->SetTextColor(rRenderContext.GetTextColor());
    m_pBuffer->DrawOutDev(Point(0, 0), pWindow->GetOutputSizePixel(), Point(0, 0),
                          pWindow->GetOutputSizePixel(), rRenderContext);
}

BufferDevice::~BufferDevice()
{
    m_rRenderContext.DrawOutDev(Point(0, 0), m_pWindow->GetOutputSizePixel(), Point(0, 0),
                                m_pWindow->GetOutputSizePixel(), *m_pBuffer);
}

vcl::RenderContext* BufferDevice::operator->() { return m_pBuffer.get(); }

vcl::RenderContext& BufferDevice::operator*() { return *m_pBuffer; }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
