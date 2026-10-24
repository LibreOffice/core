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

#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>

// The ids of the primitives that Calc defines for the content of its cells.

#define PRIMITIVE2D_ID_SC_CELLTEXTPRIMITIVE2D (PRIMITIVE2D_ID_RANGE_SC | 0)
#define PRIMITIVE2D_ID_SC_CELLMARKPRIMITIVE2D (PRIMITIVE2D_ID_RANGE_SC | 1)
#define PRIMITIVE2D_ID_SC_GRIDCOVERPRIMITIVE2D (PRIMITIVE2D_ID_RANGE_SC | 2)
#define PRIMITIVE2D_ID_SC_SPARKLINEPRIMITIVE2D (PRIMITIVE2D_ID_RANGE_SC | 3)

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
