/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/Geometry.hxx>

Geometry::Geometry()
    : mbMap(false)
    , mnOffsetOriginX(0)
    , mnOffsetOriginY(0)
    , mnOffsetOriginLogicX(0)
    , mnOffsetOriginLogicY(0)
    , mnFrameOffsetX(0)
    , mnFrameOffsetY(0)
{
}

bool Geometry::IsMapModeEnabled() const { return mbMap; }

void Geometry::EnableMapMode(bool bEnable) { mbMap = bEnable; }

tools::Long Geometry::GetXOffsetFromOriginInPixels() const { return mnOffsetOriginX; }

void Geometry::SetXOffsetFromOriginInPixels(tools::Long nOffsetFromOriginXpx)
{
    mnOffsetOriginX = nOffsetFromOriginXpx;
}

tools::Long Geometry::GetYOffsetFromOriginInPixels() const { return mnOffsetOriginY; }

void Geometry::SetYOffsetFromOriginInPixels(tools::Long nOffsetFromOriginYpx)
{
    mnOffsetOriginY = nOffsetFromOriginYpx;
}

tools::Long Geometry::GetXOffsetFromOriginInLogicalUnits() const { return mnOffsetOriginLogicX; }

void Geometry::SetXOffsetFromOriginInLogicalUnits(tools::Long nOffsetFromOriginXInLogicalUnits)
{
    mnOffsetOriginLogicX = nOffsetFromOriginXInLogicalUnits;
}

tools::Long Geometry::GetYOffsetFromOriginInLogicalUnits() const { return mnOffsetOriginLogicX; }

void Geometry::SetYOffsetFromOriginInLogicalUnits(tools::Long nOffsetFromOriginYInLogicalUnits)
{
    mnOffsetOriginLogicY = nOffsetFromOriginYInLogicalUnits;
}

tools::Long Geometry::GetXFrameOffset() const { return mnFrameOffsetX; }

void Geometry::SetXFrameOffset(tools::Long nOffset) { mnFrameOffsetX = nOffset; }

tools::Long Geometry::GetYFrameOffset() const { return mnFrameOffsetY; }

void Geometry::SetYFrameOffset(tools::Long nOffset) { mnFrameOffsetY = nOffset; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
