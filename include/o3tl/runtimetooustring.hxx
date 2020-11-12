/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_O3TL_RUNTIMETOOUSTRING_HXX
#define INCLUDED_O3TL_RUNTIMETOOUSTRING_HXX

#include <sal/config.h>

#include <cassert>
#include <cstring>

#include <rtl/textcvt.h>
#include <rtl/textenc.h>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>

namespace o3tl
{
/** Convert an NTBS from the C++ runtime to an OUString.

    This is used to convert an NTBS as provided by std::exception::what or
    std::type_info::name into an OUString in a "lossless" way.  The conversion
    is done using RTL_TEXTENCODING_ISO_8859_1, so each char in the input maps
    to one Unicode character in the output.
*/
inline OUString runtimeToOUString(char const* runtimeString)
{
    OUString s;
    bool ok = rtl_convertStringToUString(
        &s.pData, runtimeString, std::strlen(runtimeString), RTL_TEXTENCODING_ISO_8859_1,
        (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
         | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR));
    assert(ok);
    (void)ok;
    return s;
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
