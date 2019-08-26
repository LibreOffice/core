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
#include <tools/poly.hxx>

#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/drawables/PolygonDrawable.hxx>
#include <vcl/drawables/PolyPolygonDrawable.hxx>
#include <vcl/drawables/B2DPolygonDrawable.hxx>
#include <vcl/drawables/B2DPolyPolyLineDrawable.hxx>

#include <salgdi.hxx>

#include <memory>
#include <cassert>

#define OUTDEV_POLYPOLY_STACKBUF        32

void OutputDevice::DrawPolygon(const tools::Polygon& rPoly)
{
    Draw(vcl::PolygonDrawable(rPoly));
}

void OutputDevice::DrawPolygon( const basegfx::B2DPolygon& rB2DPolygon)
{
    Draw(vcl::B2DPolygonDrawable(rB2DPolygon));
}

void OutputDevice::DrawPolyPolygon( const tools::PolyPolygon& rPolyPoly )
{
    Draw(vcl::PolyPolygonDrawable(rPolyPoly));
}

// Caution: This method is nearly the same as
// OutputDevice::DrawTransparent( const basegfx::B2DPolyPolygon& rB2DPolyPoly, double fTransparency),
// so when changes are made here do not forget to make changes there, too

void OutputDevice::DrawPolyPolygon( const basegfx::B2DPolyPolygon& rB2DPolyPoly )
{
    assert(!is_double_buffered_window());

    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaPolyPolygonAction( tools::PolyPolygon( rB2DPolyPoly ) ) );

    // call helper
    Draw(vcl::B2DPolyPolyLineDrawable(rB2DPolyPoly));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
