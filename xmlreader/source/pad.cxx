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

#include <sal/config.h>

#include <cassert>

#include <sal/types.h>
#include <xmlreader/pad.hxx>
#include <xmlreader/span.hxx>

namespace xmlreader {

void Pad::add(char const * begin, sal_Int32 length) {
    assert(
        begin != nullptr && length >= 0 && !(span_.is() && buffer_.getLength() != 0));
    if (length != 0) {
        flushSpan();
        if (buffer_.isEmpty()) {
            span_ = Span(begin, length);
        } else {
            buffer_.append(begin, length);
        }
    }
}

void Pad::addEphemeral(char const * begin, sal_Int32 length) {
    assert(
        begin != nullptr && length >= 0 && !(span_.is() && buffer_.getLength() != 0));
    if (length != 0) {
        flushSpan();
        buffer_.append(begin, length);
    }
}

void Pad::clear() {
    assert(!(span_.is() && buffer_.getLength() != 0));
    span_.clear();
    buffer_.setLength(0);
}

Span Pad::get() const {
    assert(!(span_.is() && buffer_.getLength() != 0));
    if (span_.is()) {
        return span_;
    } else if (buffer_.isEmpty()) {
        return Span("");
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
