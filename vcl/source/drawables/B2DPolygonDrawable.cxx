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

#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/drawables/B2DPolygonDrawable.hxx>

#include <salgdi.hxx>

#include <cassert>

namespace vcl
{
bool B2DPolygonDrawable::DrawCommand(OutputDevice* pRenderContext) const
{
    return Draw(pRenderContext, maB2DPolygon);
}

bool B2DPolygonDrawable::Draw(OutputDevice* pRenderContext, basegfx::B2DPolygon const& rB2DPolygon)
{
    // Do NOT paint empty polygons
    if (rB2DPolygon.count())
    {
        basegfx::B2DPolyPolygon aPP(rB2DPolygon);
        pRenderContext->DrawPolyPolygon(aPP);
    }

    return true;
}
} // namespace vcl
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
