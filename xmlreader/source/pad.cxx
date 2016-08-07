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



#include "sal/config.h"
#include "osl/diagnose.h"
#include "rtl/string.h"
#include "sal/types.h"
#include "xmlreader/pad.hxx"
#include "xmlreader/span.hxx"

namespace xmlreader {

void Pad::add(char const * begin, sal_Int32 length) {
    OSL_ASSERT(
        begin != 0 && length >= 0 && !(span_.is() && buffer_.getLength() != 0));
    if (length != 0) {
        flushSpan();
        if (buffer_.getLength() == 0) {
            span_ = Span(begin, length);
        } else {
            buffer_.append(begin, length);
        }
    }
}

void Pad::addEphemeral(char const * begin, sal_Int32 length) {
    OSL_ASSERT(
        begin != 0 && length >= 0 && !(span_.is() && buffer_.getLength() != 0));
    if (length != 0) {
        flushSpan();
        buffer_.append(begin, length);
    }
}

void Pad::clear() {
    OSL_ASSERT(!(span_.is() && buffer_.getLength() != 0));
    span_.clear();
    buffer_.setLength(0);
}

Span Pad::get() const {
    OSL_ASSERT(!(span_.is() && buffer_.getLength() != 0));
    if (span_.is()) {
        return span_;
    } else if (buffer_.getLength() == 0) {
        return Span(RTL_CONSTASCII_STRINGPARAM(""));
    } else {
        return Span(buffer_.getStr(), buffer_.getLength());
    }
}

void Pad::flushSpan() {
    if (span_.is()) {
        buffer_.append(span_.begin, span_.length);
        span_.clear();
    }
}

}
