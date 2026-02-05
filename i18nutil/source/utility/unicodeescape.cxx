/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <i18nutil/unicodeescape.hxx>
#include <rtl/character.hxx>
#include <rtl/ustrbuf.hxx>
#include <o3tl/numeric.hxx>
#include <cassert>

namespace i18nutil
{
namespace
{
// Convert hex string to codepoint
sal_uInt32 hexToCodepoint(std::u16string_view hexStr)
{
    assert(hexStr.size() <= 8);

    sal_uInt32 result = 0;
    for (sal_Unicode c : hexStr)
    {
        sal_Int32 hexValue = o3tl::convertToHex<sal_Int32>(c);
        if (hexValue < 0)
            return 0xFFFFFFFF; // Invalid
        result = (result << 4) | hexValue;
    }
    return result;
}

// Helper: Check if codepoint is allowed
bool isAllowedCodepoint(sal_uInt32 cp)
{
    if (cp < 0x20)
    {
        return cp == 0x09 || cp == 0x0A || cp == 0x0D; // TAB, LF, CR
    }
    return rtl::isUnicodeCodePoint(cp);
}

} // local namespace

OUString processUnicodeEscapes(std::u16string_view input)
{
    OUStringBuffer result;
    sal_Int32 len = input.size();

    for (sal_Int32 i = 0; i < len; ++i)
    {
        // Check for backslash and ensure we have at least one char after it
        if (input[i] == '\\' && i + 1 < len)
        {
            sal_Unicode next = input[i + 1];

            // Handle escaped backslash: (backslash)(backslash) -> (backslash)
            if (next == '\\')
            {
                result.append('\\');
                ++i; // Skip the second backslash
                continue;
            }

            // Handle \uhhhh (4 hex digits)
            else if (next == 'u' && i + 5 < len)
            {
                sal_uInt32 codepoint = hexToCodepoint(input.substr(i + 2, 4));
                if (isAllowedCodepoint(codepoint))
                {
                    result.append(sal_Unicode(codepoint));
                    i += 5; // Skip \uhhhh
                    continue;
                }
            }

            // Handle \Uhhhhhhhh (8 hex digits)
            else if (next == 'U' && i + 9 < len)
            {
                sal_uInt32 codepoint = hexToCodepoint(input.substr(i + 2, 8));
                if (isAllowedCodepoint(codepoint))
                {
                    result.appendUtf32(codepoint);
                    i += 9; // Skip \Uhhhhhhhh
                    continue;
                }
            }
        }

        // If not a valid escape, or just a regular character, append it as is
        result.append(input[i]);
    }

    return result.makeStringAndClear();
}

} // namespace i18nutil

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
