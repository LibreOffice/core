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
 * Fuzzer for Admin console message handling.
 * Functions: LLVMFuzzerTestOneInput() - Tests Admin::handleMessage()
 */

#include <config.h>

#include "Admin.hpp"

#include <fuzzer/Common.hpp>

#include <sstream>

static bool DoInitialization()
{
    fuzzer::DoInitialization();
    Admin::initialize();
    return true;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    static bool initialized = DoInitialization();
    (void)initialized;

    Admin& admin = Admin::instance();
    auto handler = std::make_shared<AdminSocketHandler>(&admin);

    std::string input(reinterpret_cast<const char*>(data), size);
    std::stringstream ss(input);
    std::string line;
    while (std::getline(ss, line, '\n'))
    {
        std::vector<char> lineVector(line.data(), line.data() + line.size());
        handler->handleMessage(lineVector);
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
