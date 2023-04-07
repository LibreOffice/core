/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_RTL_XMLENCODE_HXX
#define INCLUDED_RTL_XMLENCODE_HXX

#include "rtl/ustring.hxx"

namespace rtl
{
inline OUString encodeForXml(std::u16string_view rStr)
{
    // encode conforming xml:
    sal_Int32 len = rStr.length();
    OUStringBuffer buf;
    for (sal_Int32 pos = 0; pos < len; ++pos)
    {
        sal_Unicode c = rStr[pos];
        switch (c)
        {
            case '<':
                buf.append("&lt;");
                break;
            case '>':
                buf.append("&gt;");
                break;
            case '&':
                buf.append("&amp;");
                break;
            case '\'':
                buf.append("&apos;");
                break;
            case '\"':
                buf.append("&quot;");
                break;
            default:
                buf.append(c);
                break;
        }
    }

    return buf.makeStringAndClear();
}

} /* Namespace */

#endif // INCLUDED_RTL_XMLENCODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */