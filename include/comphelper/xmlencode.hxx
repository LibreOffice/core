/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

namespace comphelper::string
{
inline OUString encodeForXml(std::u16string_view rStr)
{
    // encode conforming xml:
    sal_Int32 len = rStr.length();
    OUStringBuffer buf(len + 16); // it's going to be at least len
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
