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

#include <unx/x11/x11cairotextrender.hxx>

#include <unx/glyphcache.hxx>
#include <X11/Xregion.h>
#include <cairo.h>
#include <salframe.hxx>
#include <salvd.hxx>

X11CairoTextRender::X11CairoTextRender(X11SalGraphics& rParent)
    : mrParent(rParent)
{
}

cairo_t* X11CairoTextRender::getCairoContext()
{
    return mrParent.getCairoContext();
}

void X11CairoTextRender::getSurfaceOffset( double& nDX, double& nDY )
{
    nDX = 0;
    nDY = 0;
}

void X11CairoTextRender::clipRegion(cairo_t* cr)
{
    Region pClipRegion = mrParent.mpClipRegion;
    if( pClipRegion && !XEmptyRegion( pClipRegion ) )
    {
        for (tools::Long i = 0; i < pClipRegion->numRects; ++i)
        {
            cairo_rectangle(cr,
                pClipRegion->rects[i].x1,
                pClipRegion->rects[i].y1,
                pClipRegion->rects[i].x2 - pClipRegion->rects[i].x1,
                pClipRegion->rects[i].y2 - pClipRegion->rects[i].y1);
        }
        cairo_clip(cr);
    }
}

size_t X11CairoTextRender::GetWidth() const
{
    SalGeometryProvider *pProvider = mrParent.m_pFrame;
    if( !pProvider )
        pProvider = mrParent.m_pVDev;

    if( pProvider )
        return pProvider->GetWidth();
    else
        return 1;
}

size_t X11CairoTextRender::GetHeight() const
{
    SalGeometryProvider *pProvider = mrParent.m_pFrame;
    if( !pProvider )
        pProvider = mrParent.m_pVDev;

    if( pProvider )
        return pProvider->GetHeight();
    else
        return 1;
}

void X11CairoTextRender::releaseCairoContext(cairo_t* cr)
{
    X11SalGraphics::releaseCairoContext(cr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
