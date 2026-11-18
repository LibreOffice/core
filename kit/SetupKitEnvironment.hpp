/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
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
#if defined(_WIN32)
        // Group Policy settings of the machine, read from the registry; see
        // winreg.cxx in the engine.
        "winreg:LOCAL_MACHINE "
#endif
        // The layer addOrganizationPath() files the shared AutoText, Dictionary
        // and Template paths into, ranking them above the shipped defaults. The
        // ini does not exist: the layer starts out empty and is filled at run
        // time. See addOrganizationPath() in desktop/source/lib/init.cxx.
#if defined(MACOS)
        "sharedext:${BRAND_BASE_DIR}/Resources/registry/sharedext.ini "
        // Managed preferences of the app's bundle identifier, deployed by an MDM
        // configuration profile; see "macOS Managed Preferences Layer" in
        // engine/configmgr/README.md.
        "macosprefs: "
#else
        "sharedext:${BRAND_BASE_DIR}/share/registry/sharedext.ini "
#endif
#if defined(_WIN32)
        // Group Policy settings of the user, ranking above the machine's.
        "winreg:CURRENT_USER "
#endif
#if MOBILEAPP && !WASMAPP
        // The user's own settings, which the engine writes as it does on the
        // desktop: the apps have a profile of their own, the one passed to
        // cok_init_2() or named in bootstraprc. The writable layer has to be
        // the last one. The server has no profile that outlives the kit and
        // keeps its configuration in memory instead; it persists an
        // allow-listed part through comphelper::COKit::persistUserSettings().
        // The WASM app's filesystem is in memory and goes with the page, so it
        // keeps the server's list too.
#if defined(MACOS)
        "user:!${$BRAND_BASE_DIR/Resources/bootstraprc:UserInstallation}"
        "/user/registrymodifications.xcu"
#elif defined(_WIN32)
        "user:!${$BRAND_BASE_DIR/program/bootstrap.ini:UserInstallation}"
        "/user/registrymodifications.xcu"
#else
        "user:!${$BRAND_BASE_DIR/program/bootstraprc:UserInstallation}"
        "/user/registrymodifications.xcu"
#endif
#endif
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
