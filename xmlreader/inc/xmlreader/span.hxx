/*************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2010 Oracle and/or its affiliates.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* This file is part of OpenOffice.org.
*
* OpenOffice.org is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License version 3
* only, as published by the Free Software Foundation.
*
* OpenOffice.org is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License version 3 for more details
* (a copy is included in the LICENSE file that accompanied this code).
*
* You should have received a copy of the GNU Lesser General Public License
* version 3 along with OpenOffice.org.  If not, see
* <http://www.openoffice.org/license.html>
* for a copy of the LGPLv3 License.
*
************************************************************************/

#ifndef INCLUDED_XMLREADER_SPAN_HXX
#define INCLUDED_XMLREADER_SPAN_HXX

#include "sal/config.h"

#include "rtl/string.h"
#include "sal/types.h"
#include "xmlreader/detail/xmlreaderdllapi.hxx"

namespace rtl { class OUString; }

namespace xmlreader {

struct OOO_DLLPUBLIC_XMLREADER Span {
    char const * begin;
    sal_Int32 length;

    inline Span(): begin(0), length(0) {}
        // init length to avoid compiler warnings

    inline Span(char const * theBegin, sal_Int32 theLength):
        begin(theBegin), length(theLength) {}

    inline void clear() throw() { begin = 0; }

    inline bool is() const { return begin != 0; }

    inline bool equals(Span const & text) const {
        return rtl_str_compare_WithLength(
            begin, length, text.begin, text.length) == 0;
    }

    inline bool equals(char const * textBegin, sal_Int32 textLength) const {
        return equals(Span(textBegin, textLength));
    }

    rtl::OUString convertFromUtf8() const;
};

}

#endif
