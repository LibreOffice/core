/*************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2011 Oracle and/or its affiliates.
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
            rtl::OUString(
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
