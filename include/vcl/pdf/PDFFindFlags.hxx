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

#include <o3tl/typed_flags_set.hxx>
#include <sal/types.h>

namespace vcl::pdf
{
enum class PDFFindFlags : sal_uInt32
{
    MatchCase = 0x00000001,
    MatchWholeWord = 0x00000002,
    Consecutive = 0x00000004,
};

} // namespace vcl::pdf

namespace o3tl
{
template <>
struct typed_flags<vcl::pdf::PDFFindFlags> : is_typed_flags<vcl::pdf::PDFFindFlags, 0x00000007>
{
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
