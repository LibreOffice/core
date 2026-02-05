/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>
#include <i18nutil/i18nutildllapi.h>

namespace i18nutil
{
/**
 * Process Unicode escape sequences in a string.
 * Converts \uhhhh (4 hex digits) and \Uhhhhhhhh (8 hex digits) to their
 * corresponding Unicode characters.
 * Handles escaped backslashes: \\ becomes \
 * Filters out control characters < 0x20 except TAB, LF, and CR.
 *
 * @param input String potentially containing Unicode escape sequences
 * @return String with escape sequences converted to actual Unicode characters
 */
I18NUTIL_DLLPUBLIC OUString processUnicodeEscapes(std::u16string_view input);

} // namespace i18nutil

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
