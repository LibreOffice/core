/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <vcl/alpha.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>

namespace drawinglayer::primitive2d
{
/* Returns 8-bit alpha mask created from passed mask.

   Negative fErodeDilateRadius values mean erode, positive - dilate.
   nTransparency defines minimal transparency level.
*/
AlphaMask ProcessAndBlurAlphaMask(const AlphaMask& rMask, double fErodeDilateRadius,
                                  double fBlurRadius, sal_uInt8 nTransparency,
                                  bool bConvertTo1Bit = true);

/* Returns the 8-bit alpha mask of a glow halo around the object in rMask.

   The object is grown by half of fGlowRadius, using a Euclidean distance transform so that the
   growth is the same in every direction, and the result is blurred over the other half. Corners
   therefore come out the way a blur makes them: a convex one thinner, a concave one filled in.

   fGlowRadius is the width of the halo in pixels. nTransparency defines minimal transparency level.
*/
AlphaMask CreateGlowAlphaMask(const AlphaMask& rMask, double fGlowRadius, sal_uInt8 nTransparency);

drawinglayer::geometry::ViewInformation2D
expandB2DRangeAtViewInformation2D(const drawinglayer::geometry::ViewInformation2D& rViewInfo,
                                  double nAmount);

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
