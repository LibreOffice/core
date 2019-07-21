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

#include <vcl/window.hxx>
#include <vcl/virdev.hxx>
#include <vcl/vclptr.hxx>

#include <WindowAnimationRenderer.hxx>
#include <window.h>

void WindowAnimationRenderer::DrawToIndex(sal_uLong nIndex)
{
    VclPtr<vcl::RenderContext> pRenderContext = mpRenderContext;

    std::unique_ptr<PaintBufferGuard> pGuard;

    vcl::Window* pWindow = static_cast<vcl::Window*>(mpRenderContext.get());
    pGuard.reset(new PaintBufferGuard(pWindow->ImplGetWindowImpl()->mpFrameData, pWindow));
    pRenderContext = pGuard->GetRenderContext();

    ScopedVclPtrInstance<VirtualDevice> aVDev;
    std::unique_ptr<vcl::Region> xOldClip(
        !maClip.IsNull() ? new vcl::Region(pRenderContext->GetClipRegion()) : nullptr);

    aVDev->SetOutputSizePixel(maSizePx, false);
    nIndex = std::min(nIndex, static_cast<sal_uLong>(mpParent->Count()) - 1);

    for (sal_uLong i = 0; i <= nIndex; i++)
        Draw(i, aVDev.get());

    if (xOldClip)
        pRenderContext->SetClipRegion(maClip);

    pRenderContext->DrawOutDev(maDispPt, maDispSz, Point(), maSizePx, *aVDev);
    if (pGuard)
        pGuard->SetPaintRect(tools::Rectangle(maDispPt, maDispSz));

    if (xOldClip)
        pRenderContext->SetClipRegion(*xOldClip);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
