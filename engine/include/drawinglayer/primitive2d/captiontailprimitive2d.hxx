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

#pragma once

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>

namespace drawinglayer::primitive2d
{
/** The tail of a caption: its line with the line ends, or its hidden geometry where it has no
    line. The tail keeps its end when the rest of the caption is transformed and is laid out
    anew from it, so a reader that transforms the caption itself needs to find it. It
    decomposes to its children, so anyone not knowing it draws them as usual.
 */
class DRAWINGLAYER_DLLPUBLIC CaptionTailPrimitive2D final : public GroupPrimitive2D
{
public:
    explicit CaptionTailPrimitive2D(Primitive2DContainer&& aChildren);

    virtual sal_uInt32 getPrimitive2DID() const override;
};
} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
