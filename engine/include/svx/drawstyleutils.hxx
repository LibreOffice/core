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

#include <svx/svxdllapi.h>

class SfxItemSet;
class SdrView;

namespace svx
{
/**
 * Convert slot-based drawing arguments in @p rArgs to draw-model items:
 * transparent fill/line color becomes "no fill/line", a bare fill color forces
 * a solid fill, and the line width and gradient JSON arguments are expanded.
 *
 * A bare line color is left alone; use applyBareLineColorToMarked() for that.
 */
SVXCORE_DLLPUBLIC void convertDrawStyleArguments(SfxItemSet& rArgs);

/**
 * Make a bare line color (non-transparent SID_ATTR_LINE_COLOR in @p rArgs with
 * no explicit non-None line style) visible on the objects marked in @p rView.
 *
 * Every marked object whose resolved line style (parents included) is None is
 * switched to a solid line; objects that already have a line keep their style.
 * Does nothing when @p rArgs carries no bare line color. Changes are registered
 * for undo when enabled; the caller owns any BegUndo()/EndUndo() grouping.
 */
SVXCORE_DLLPUBLIC void applyBareLineColorToMarked(SdrView& rView, const SfxItemSet& rArgs);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
