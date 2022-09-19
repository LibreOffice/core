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

#pragma once

#include <vcl/virdev.hxx>

// Helper class *exclusively* for VclProcessor2D. It should only
// be used internally, see current four usages. It is used to
// render something with mask or transparence (see MaskPrimitive2D,
// UnifiedTransparencePrimitive2D and TransparencePrimitive2D) or
// as tooling for preparing pixelized output in the renderer
// (see PatternFillPrimitive2D) if that is faster.
//
// To do so, initializing this instance takes over a lot of work
// from you:
// - It initializes a 'Content' VDev which is buffered (since it had
// shown that re-allocating all the time is slower). It checks
// visibility and all usages initializing this should check for
// isVisible() after construction.
// - It pre-initializes the 'Content' VDev with blitting the content
// of the target VDev.
// - It offers to get a 'Transparence' VDev (also from the buffer) if
// needed.
//
// If 'Transparence' is/was used, it combines as needed to paint
// all buffered stuff to target VDev when calling paint().
// Caution: It is designed to use *either* a fixed transparence in
// the paint()-call *or* a fill TransparenceChannel using a
// 'Transparence' VDev. It is *not* designed to use/combine
// both - it's simply not needed for it's intended purpose/usage.
//
// Painting transparent works based on a simple principle: It first
// blits the original content of the target VDev. Then the content
// is painted on top of that, plus a Transparence/Mask prepared.
// The combination always works since unchanged pixels will not
// change, independent of the transparence value [0..255] it gets
// mixed with. Or the other way around: Only pixels changed on the
// Content *can* be changed by transparence values.
//
// This is 2.5 times faster than first painting to a
// 'combined' VDev that supports transparency, as is used by the
// presentation engine.
// For the mentioned factor refer to:
//   Patch to demonstrate former and now repaint differences
//   https://gerrit.libreoffice.org/c/core/+/129301
//   git fetch https://git.libreoffice.org/core refs/changes/01/129301/3 && git cherry-pick FETCH_HEAD
//
// Note: This principle only works when the target is RGB, so
// useful for EditViews like for PrimitiveRenderers where this is
// the case. For usage with GBA targets the starting conditions
// would have to be modified to not blend against the initial color
// of 'Content' (usually COL_WHITE).
//
// After having finished the rework of ShadowPrimitive2D,
// SoftEdgePrimitive2D and GlowPrimitive2D (see commits:)
//      e735ad1c57cddaf17d6ffc0cf15b5e14fa63c4ad
//      707b0c328a282d993fa33b618083d20b6c521de6
//      c2d1458723c66c2fd717a112f89f773226adc841
// which used the impBufferDevice in such a mode combined with
// mentioned 'combined' transparence VDev it is now possible
// to return to this former, much faster method.
//
// Please do *not* hack/use this helper class, better create
// a new one fitting your/the intended purpose. I do not want
// to see losing performance by this getting modified again.
//
// Note: Using that 'combined' transparence VDev is not really
// recommended, it may vanish anytime. That it works with
// PrimitiveRenderers *at all* is neither designed nor tested
// or recommended - it's pure coincidence.
//
// I hope that for the future all this will vanish by getting to
// fully RGBA-capable devices - what is planned and makes sense.

namespace drawinglayer
{
class impBufferDevice
{
    OutputDevice& mrOutDev;
    VclPtr<VirtualDevice> mpContent;
    VclPtr<VirtualDevice> mpAlpha;
    tools::Rectangle maDestPixel;

public:
    impBufferDevice(OutputDevice& rOutDev, const basegfx::B2DRange& rRange);
    ~impBufferDevice();

    void paint(double fTrans = 0.0);
    bool isVisible() const { return !maDestPixel.IsEmpty(); }
    VirtualDevice& getContent();
    VirtualDevice& getTransparence();
};
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
