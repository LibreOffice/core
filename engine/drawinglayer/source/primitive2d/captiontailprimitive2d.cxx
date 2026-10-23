/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <drawinglayer/primitive2d/captiontailprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>

namespace drawinglayer::primitive2d
{
CaptionTailPrimitive2D::CaptionTailPrimitive2D(Primitive2DContainer&& aChildren)
    : GroupPrimitive2D(std::move(aChildren))
{
}

sal_uInt32 CaptionTailPrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_CAPTIONTAILPRIMITIVE2D;
}
} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
