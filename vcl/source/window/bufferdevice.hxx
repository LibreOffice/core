/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/virdev.hxx>
#include <vcl/window.hxx>

namespace vcl
{
/// Buffers drawing on a vcl::RenderContext using a VirtualDevice.
class VCL_DLLPUBLIC BufferDevice
{
    ScopedVclPtr<VirtualDevice> m_pBuffer;
    VclPtr<vcl::Window> m_pWindow;
    vcl::RenderContext& m_rRenderContext;
    bool m_bDisposed = false;

public:
    BufferDevice(const VclPtr<vcl::Window>& pWindow, vcl::RenderContext& rRenderContext);
    ~BufferDevice();
    void Dispose();

    vcl::RenderContext* operator->();
    vcl::RenderContext& operator*();
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
