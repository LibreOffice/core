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
 * Global variables shared between Kit and WSD.
 * Functions: getKitPid()
 */

#include <config.h>

#include <common/Globals.hpp>
#include <common/Util.hpp>
#include <kit/KitGlobals.hpp>
#include <wsd/WSDGlobals.hpp>

// This is the globals set of the binary that holds both the server and the kit.
const bool Util::KitInProcess = true;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
