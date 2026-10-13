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

#include <o3tl/string_view.hxx>

#include <string_view>

namespace comphelper
{
/// Whether rURL runs a script: its scheme is "macro" or "vnd.sun.star.script", in any case
inline bool isScriptURL(std::u16string_view rURL)
{
    const std::u16string_view aURL = o3tl::trim(rURL);
    return o3tl::matchIgnoreAsciiCase(aURL, u"macro:")
           || o3tl::matchIgnoreAsciiCase(aURL, u"vnd.sun.star.script:");
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
