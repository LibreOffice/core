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
    : mnOffsetOriginX(0)
    , mnOffsetOriginY(0)
{
}

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
