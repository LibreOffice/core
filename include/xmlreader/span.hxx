/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef INCLUDED_XMLREADER_SPAN_HXX
#define INCLUDED_XMLREADER_SPAN_HXX

#include <rtl/string.h>
#include <sal/types.h>
#include <cstddef>
#include <xmlreader/detail/xmlreaderdllapi.hxx>

namespace rtl { class OUString; }

namespace xmlreader {

struct OOO_DLLPUBLIC_XMLREADER Span {
    char const * begin;
    sal_Int32 length;

    inline Span(): begin(0), length(0) {}
        // init length to avoid compiler warnings

    inline Span(char const * theBegin, sal_Int32 theLength):
        begin(theBegin), length(theLength) {}

    template< std::size_t N > explicit inline Span(char const (& literal)[N]):
        begin(literal), length(N - 1)
    {}

    inline void clear() throw() { begin = 0; }

    inline bool is() const { return begin != 0; }

    inline bool equals(Span const & text) const {
        return length == text.length
            && (rtl_str_compare_WithLength(
                    begin, length, text.begin, text.length)
                == 0);
    }

    inline bool equals(char const * textBegin, sal_Int32 textLength) const {
        return equals(Span(textBegin, textLength));
    }

    template< std::size_t N > inline bool equals(char const (& literal)[N])
        const
    {
        return equals(Span(literal, N - 1));
    }

    rtl::OUString convertFromUtf8() const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
