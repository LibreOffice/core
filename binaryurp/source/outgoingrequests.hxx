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

#pragma once

#include <sal/config.h>

#include <map>
#include <vector>

#include <osl/mutex.hxx>

namespace binaryurp
{
struct OutgoingRequest;
}
namespace rtl
{
class ByteSequence;
}

namespace binaryurp
{
class OutgoingRequests
{
public:
    OutgoingRequests();

    ~OutgoingRequests();

    void push(rtl::ByteSequence const& tid, OutgoingRequest const& request);

    OutgoingRequest top(rtl::ByteSequence const& tid);

    void pop(rtl::ByteSequence const& tid) noexcept;

private:
    OutgoingRequests(const OutgoingRequests&) = delete;
    OutgoingRequests& operator=(const OutgoingRequests&) = delete;

    typedef std::map<rtl::ByteSequence, std::vector<OutgoingRequest>> Map;

    osl::Mutex mutex_;
    Map map_;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
