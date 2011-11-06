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



#ifndef INCLUDED_XMLREADER_PAD_HXX
#define INCLUDED_XMLREADER_PAD_HXX

#include "sal/config.h"

#include "rtl/strbuf.hxx"
#include "sal/types.h"
#include "xmlreader/detail/xmlreaderdllapi.hxx"
#include "xmlreader/span.hxx"

namespace xmlreader {

class OOO_DLLPUBLIC_XMLREADER Pad {
public:
    void add(char const * begin, sal_Int32 length);

    void addEphemeral(char const * begin, sal_Int32 length);

    void clear();

    Span get() const;

private:
    SAL_DLLPRIVATE void flushSpan();

    Span span_;
    rtl::OStringBuffer buffer_;
};

}

#endif
