/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Fuzzer for HTTP response parsing.
 * Functions: LLVMFuzzerTestOneInput() - Tests http::Response parsing
 */

#include <config.h>

#include <common/Log.hpp>
#include <net/HttpRequest.hpp>
#include <fuzzer/Common.hpp>

#include <cstdlib>

extern "C" int LLVMFuzzerInitialize(int* /*argc*/, char*** /*argv*/)
{
    fuzzer::DoInitialization();

    // Shut down logging before static destructors run.
    std::atexit([] { Log::shutdown(); });

    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        http::Response response;
        response.readData(reinterpret_cast<const char*>(data), i);
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
