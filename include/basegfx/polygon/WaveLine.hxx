/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_BASEGFX_POLYGON_WAVELINE_HXX
#define INCLUDED_BASEGFX_POLYGON_WAVELINE_HXX

#include <basegfx/basegfxdllapi.h>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/range/b2drectangle.hxx>

namespace basegfx
{
// Creates a polygon of a wave line in the input rectangle.
//
// The polygon is created with points at the center of the rectangle,
// and the quadratic control points at the upper and lower side. See
// the diagram below.
//
// *----Q---------------Q------------*
// |                                 |
// |P-------P-------P-------P-------P|
// |                                 |
// *------------Q---------------Q----*
//
// P is the point
// Q is the quadratic bezier control point
//
BASEGFX_DLLPUBLIC B2DPolygon createWaveLinePolygon(basegfx::B2DRectangle const& rRectangle);

} // end of namespace basegfx

#endif // INCLUDED_BASEGFX_POLYGON_WAVELINE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
