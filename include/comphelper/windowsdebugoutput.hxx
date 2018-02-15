/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* Debug output operators for Windows-specific types. For use in SAL_INFO(), SAL_WARN(), and
 * friends. The exact format of the generated output is not guaranteed to be stable or contain
 * complete information.
 */

#ifndef INCLUDED_COMPHELPER_WINDOWSDEBUGOUTPUT_HXX
#define INCLUDED_COMPHELPER_WINDOWSDEBUGOUTPUT_HXX

#include <codecvt>
#include <ostream>
#include <string>

#ifdef LIBO_INTERNAL_ONLY
#include <prewin.h>
#include <postwin.h>
#else
#include <windows.h>
#endif

template <typename charT, typename traits>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& stream,
                                                     const IID& rIid)
{
    LPOLESTR pRiid;
    if (StringFromIID(rIid, &pRiid) != S_OK)
        return stream << "?";

    // TODO: Maybe look up a descriptive name for the service or interface, from HKCR\CLSID or
    // HKCR\Interface?

    stream << std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(std::wstring(pRiid));
    CoTaskMemFree(pRiid);
    return stream;
}

#endif // INCLUDED_COMPHELPER_WINDOWSDEBUGOUTPUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
