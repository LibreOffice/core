/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include "osl/diagnose.h"
#include "rtl/string.h"
#include "sal/types.h"

#include "pad.hxx"
#include "span.hxx"

namespace configmgr {

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

bool Pad::is() const {
    OSL_ASSERT(!(span_.is() && buffer_.getLength() != 0));
    return span_.is() || buffer_.getLength() != 0;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
