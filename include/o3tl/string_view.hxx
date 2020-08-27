/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <string_view>

#include <rtl/ustring.h>

namespace o3tl
{
// Like OUString::equalsIgnoreAsciiCase, but for two std::u16string_view:
inline bool equalsIgnoreAsciiCase(std::u16string_view s1, std::u16string_view s2)
{
    return rtl_ustr_compareIgnoreAsciiCase_WithLength(s1.data(), s1.size(), s2.data(), s2.size());
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
