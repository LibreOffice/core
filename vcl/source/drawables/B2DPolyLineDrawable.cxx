/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with pRenderContext
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with pRenderContext work for additional information regarding copyright
 *   ownership. The ASF licenses pRenderContext file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use pRenderContext file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>

#include <vcl/gdimtf.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <vcl/drawables/B2DPolyLineDrawable.hxx>
#include <vcl/drawables/B2DPolyPolyLineDrawable.hxx>
#include <vcl/drawables/PolyHairlineDrawable.hxx>

#include <drawables/B2DPolyLineDrawableHelper.hxx>
#include <salgdi.hxx>
#include <outdata.hxx>

#include <cassert>
#include <numeric>

namespace vcl
{
bool B2DPolyLineDrawable::DrawCommand(OutputDevice* pRenderContext) const
{
    return Draw(pRenderContext, maPolyLine, maLineInfo, mfMiterMinimumAngle);
}

bool B2DPolyLineDrawable::Draw(OutputDevice* pRenderContext, basegfx::B2DPolygon const& rB2DPolygon,
                               LineInfo const& rLineInfo, double fMiterMinimumAngle) const
{
    if (!pRenderContext->Draw(vcl::PolyHairlineDrawable(basegfx::B2DHomMatrix(), rB2DPolygon,
                                                        rLineInfo, 0.0, fMiterMinimumAngle)))
    {
        if (!B2DPolyLineDrawableHelper::DrawB2DPolyLine(pRenderContext, rB2DPolygon, rLineInfo,
                                                        fMiterMinimumAngle))
        {
            if (B2DPolyLineDrawableHelper::DrawFallbackPolyLine(pRenderContext, rB2DPolygon,
                                                                rLineInfo))
                DrawAlphaVirtDev(pRenderContext);
            else
                return false;
        }
    }

    return true;
}

bool B2DPolyLineDrawable::CanDraw(OutputDevice* pRenderContext) const
{
    return (!(!pRenderContext->IsDeviceOutputNecessary()
              || (!pRenderContext->IsLineColor() && !pRenderContext->IsFillColor())
              || pRenderContext->ImplIsRecordLayout()));
}
} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
