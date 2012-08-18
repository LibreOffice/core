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

#include "sal/config.h"

#include "com/sun/star/uno/RuntimeException.hpp"
#include "rtl/byteseq.hxx"
#include "osl/mutex.hxx"

#include "lessoperators.hxx"
#include "outgoingrequest.hxx"
#include "outgoingrequests.hxx"

namespace binaryurp {

namespace {

namespace css = com::sun::star;

}

OutgoingRequests::OutgoingRequests() {}

OutgoingRequests::~OutgoingRequests() {}

void OutgoingRequests::push(
    rtl::ByteSequence const & tid, OutgoingRequest const & request)
{
    osl::MutexGuard g(mutex_);
    map_[tid].push_back(request);
}

OutgoingRequest OutgoingRequests::top(rtl::ByteSequence const & tid) {
    osl::MutexGuard g(mutex_);
    Map::iterator i(map_.find(tid));
    if (i == map_.end()) {
        throw css::uno::RuntimeException(
            OUString(
                RTL_CONSTASCII_USTRINGPARAM("URP: reply for unknown TID")),
            css::uno::Reference< css::uno::XInterface >());
    }
    OSL_ASSERT(!i->second.empty());
    return i->second.back();
}

void OutgoingRequests::pop(rtl::ByteSequence const & tid) throw () {
    osl::MutexGuard g(mutex_);
    Map::iterator i(map_.find(tid));
    OSL_ASSERT(i != map_.end());
    i->second.pop_back();
    if (i->second.empty()) {
        map_.erase(i);
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
