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

#include <rtl/ustring.hxx>

#include <comphelper/comphelperdllapi.h>

inline constexpr OUString AVMEDIA_MIMETYPE_COMMON = u"application/vnd.sun.star.media"_ustr;

namespace comphelper
{
COMPHELPER_DLLPUBLIC auto IsMediaMimeType(::std::string_view const rMimeType) -> bool;
COMPHELPER_DLLPUBLIC auto IsMediaMimeType(OUString const& rMimeType) -> bool;
COMPHELPER_DLLPUBLIC auto GuessMediaMimeType(::std::u16string_view rFileName) -> OUString;

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
