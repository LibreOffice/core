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
 * Main entry point for the ForKit process.
 * Functions: main() - Initializes and runs ForKit
 */

#include <config.h>

#include <common/Common.hpp>
#include <kit/Kit.hpp>
#include <net/ServerSocket.hpp>

int ClientPortNumber = DEFAULT_CLIENT_PORT_NUMBER;
UnxSocketPath MasterLocation;

// Embed variant string to ensure different build-id for coolforkit-caps vs coolforkit-ns
#ifdef COOL_FORKIT_VARIANT
__attribute__((used)) const char* const ForkitVariant = COOL_FORKIT_VARIANT;
#endif

int main (int argc, char **argv)
{
    return forkit_main(argc, argv);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
