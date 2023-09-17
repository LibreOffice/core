/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <o3tl/strong_int.hxx>
#include <sal/types.h>

#include <cstddef>
#include <string_view>

typedef o3tl::strong_int<sal_uInt16, struct ToolBoxItemIdTag> ToolBoxItemId;
namespace std
{
template <> struct hash<ToolBoxItemId>
{
    std::size_t operator()(ToolBoxItemId const& s) const { return std::size_t(sal_uInt16(s)); }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
