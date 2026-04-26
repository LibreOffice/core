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
 * In-process mode entry point for COOLWSD.
 * Functions: main()
 */

#include <config.h>

#include <common/StringVector.hpp>
#include <common/Util.hpp>
#include <COOLWSD.hpp>
#include <Kit.hpp>

int createForkit(const std::string& forKitPath, const StringVector& args)
{
    // create forkit in a thread
    int argc = args.size() + 1;
    char** argv = new char*[argc];

    argv[0] = new char[forKitPath.size() + 1];
    std::strcpy(argv[0], forKitPath.c_str());
    for (size_t i = 0; i < args.size(); ++i)
    {
        argv[i + 1] = new char[args[i].size() + 1];
        std::strcpy(argv[i + 1], args[i].c_str());
    }

    std::thread([argc, argv] {
        ProcUtil::setThreadName("forkit");
        forkit_main(argc, argv);
    })
        .detach();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
