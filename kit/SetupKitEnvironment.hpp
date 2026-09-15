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

#pragma once

#include <cstdlib>
#include <string>

#include <common/Log.hpp>

inline void setupKitEnvironment(const std::string& userInterface)
{
    // Setup & check environment
    std::string layers(
#if defined(MACOS)
        "xcsxcu:${BRAND_BASE_DIR}/Resources/registry "
        "res:${BRAND_BASE_DIR}/Resources/registry "
#else
        "xcsxcu:${BRAND_BASE_DIR}/share/registry "
        "res:${BRAND_BASE_DIR}/share/registry "
#endif
        "bundledext:${${BRAND_BASE_DIR}/program/lounorc:BUNDLED_EXTENSIONS_USER}/registry/com.sun.star.comp.deployment.configuration.PackageRegistryBackend/configmgr.ini "
        "sharedext:${${BRAND_BASE_DIR}/program/lounorc:SHARED_EXTENSIONS_USER}/registry/com.sun.star.comp.deployment.configuration.PackageRegistryBackend/configmgr.ini "
        "userext:${${BRAND_BASE_DIR}/program/lounorc:UNO_USER_PACKAGES_CACHE}/registry/com.sun.star.comp.deployment.configuration.PackageRegistryBackend/configmgr.ini "
        );
    ::setenv("CONFIGURATION_LAYERS", layers.c_str(),
             1 /* override */);

    // Turn off the shared library loader service in the engine. A
    // KIT_ALWAYS_ACTIVE engine ignores this and turns it off at compile time.
    ::setenv("KIT_DISABLE_SHARED_LIBRARY_LOADER", "1", 1 /* override */);

#if !MOBILEAPP
    // No-caps tracing can spawn eg. glxinfo & other oddness.
    unsetenv("DISPLAY");
#endif

    // Set various options we need.
    std::string options = "unipoll";
    if (userInterface == "notebookbar")
        options += ":notebookbar";

    options += ":sc_no_grid_bg";

    options += ":sc_print_twips_msgs";

    ::setenv("SAL_KIT_OPTIONS", options.c_str(), 0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
