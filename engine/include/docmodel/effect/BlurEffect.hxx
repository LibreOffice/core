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

#include <docmodel/dllapi.h>
#include <basegfx/units/Length.hxx>

namespace model
{
/// A blur of the whole shape, with its fill, its line and its text, over the radius.
class DOCMODEL_DLLPUBLIC BlurEffect
{
public:
    gfx::Length maRadius = 0_emu;
    /// Whether the blur grows the shape by its radius.
    bool mbGrow = true;

    bool operator==(const BlurEffect&) const = default;
};

} // end of namespace model

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
