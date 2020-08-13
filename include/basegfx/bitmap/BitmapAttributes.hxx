/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <basegfx/basegfxdllapi.h>
#include <o3tl/typed_flags_set.hxx>

namespace basegfx
{
enum class MirrorDirectionFlags
{
    NONE = 0x00,
    Horizontal = 0x01,
    Vertical = 0x02,
};

} // end of namespace basegfx

namespace o3tl
{
template <>
struct typed_flags<basegfx::MirrorDirectionFlags>
    : is_typed_flags<basegfx::MirrorDirectionFlags, 0x03>
{
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
