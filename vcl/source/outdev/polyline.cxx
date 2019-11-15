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

#include <sal/types.h>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>

#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/drawables/B2DPolyLineDrawable.hxx>
#include <vcl/drawables/B2DPolyPolyLineDrawable.hxx>
#include <vcl/drawables/PolyLineDrawable.hxx>
#include <vcl/drawables/PolyHairlineDrawable.hxx>

#include <salgdi.hxx>

void OutputDevice::DrawPolyLine(const tools::Polygon& rPoly, const LineInfo& rLineInfo)
{
    Draw(vcl::PolyLineDrawable(rPoly, rLineInfo));
}

void OutputDevice::DrawPolyLine(const basegfx::B2DPolygon& rB2DPolygon, double fLineWidth,
                                basegfx::B2DLineJoin eLineJoin, css::drawing::LineCap eLineCap,
                                double fMiterMinimumAngle, bool bUseScaffolding)
{
    LineInfo aLineInfo;
    aLineInfo.SetWidth(fLineWidth);
    aLineInfo.SetLineJoin(eLineJoin);
    aLineInfo.SetLineCap(eLineCap);

    Draw(vcl::B2DPolyLineDrawable(rB2DPolygon, aLineInfo, fMiterMinimumAngle, bUseScaffolding));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
