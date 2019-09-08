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

#include <vcl/hatch.hxx>
#include <vcl/metaact.hxx>
#include <vcl/outdev.hxx>
#include <vcl/drawables/HatchDrawable.hxx>

#include <salgdi.hxx>

void OutputDevice::DrawHatch(const tools::PolyPolygon& rPolyPoly, const Hatch& rHatch)
{
    Draw(vcl::HatchDrawable(rPolyPoly, rHatch));
}

void OutputDevice::AddHatchActions(const tools::PolyPolygon& rPolyPoly, const Hatch& rHatch,
                                   GDIMetaFile& rMtf)
{
    tools::PolyPolygon aPolyPoly(rPolyPoly);
    aPolyPoly.Optimize(PolyOptimizeFlags::NO_SAME | PolyOptimizeFlags::CLOSE);

    if (aPolyPoly.Count())
    {
        GDIMetaFile* pOldMtf = mpMetaFile;

        mpMetaFile = &rMtf;
        mpMetaFile->AddAction(new MetaPushAction(PushFlags::ALL));
        mpMetaFile->AddAction(new MetaLineColorAction(rHatch.GetColor(), true));
        DrawHatch(aPolyPoly, rHatch, true);
        mpMetaFile->AddAction(new MetaPopAction());
        mpMetaFile = pOldMtf;
    }
}

void OutputDevice::DrawHatch(const tools::PolyPolygon& rPolyPoly, const Hatch& rHatch, bool bMtf)
{
    Draw(vcl::HatchDrawable(rPolyPoly, rHatch, bMtf));
}

void OutputDevice::DrawHatchLine_DrawLine(const Point& rStartPoint, const Point& rEndPoint)
{
    Point aPt1{ ImplLogicToDevicePixel(rStartPoint) }, aPt2{ ImplLogicToDevicePixel(rEndPoint) };
    mpGraphics->DrawLine(aPt1.X(), aPt1.Y(), aPt2.X(), aPt2.Y(), this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
