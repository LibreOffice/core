/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
