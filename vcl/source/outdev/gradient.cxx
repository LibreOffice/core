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

#include <vcl/outdev.hxx>
#include <vcl/drawables/GradientDrawable.hxx>

void OutputDevice::DrawGradient(const tools::Rectangle& rRect, const Gradient& rGradient)
{
    Draw(vcl::GradientDrawable(rRect, rGradient));
}

void OutputDevice::DrawGradient(const tools::PolyPolygon& rPolyPoly, const Gradient& rGradient)
{
    Draw(vcl::GradientDrawable(rPolyPoly, rGradient));
}

long OutputDevice::GetGradientStepCount(long nMinRect)
{
    return (nMinRect < 50) ? 2 : 4;
}

void OutputDevice::ClipAndDrawGradientMetafile ( const Gradient &rGradient, const tools::PolyPolygon &rPolyPoly )
{
    const tools::Rectangle aBoundRect( rPolyPoly.GetBoundRect() );
    const bool  bOldOutput = IsOutputEnabled();

    EnableOutput( false );
    Push( PushFlags::RASTEROP );
    SetRasterOp( RasterOp::Xor );
    DrawGradient( aBoundRect, rGradient );
    SetFillColor( COL_BLACK );
    SetRasterOp( RasterOp::N0 );
    DrawPolyPolygon( rPolyPoly );
    SetRasterOp( RasterOp::Xor );
    DrawGradient( aBoundRect, rGradient );
    Pop();
    EnableOutput( bOldOutput );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
