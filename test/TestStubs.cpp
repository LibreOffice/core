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

/*
 * The process-level entry points that each server executable defines for itself. The standalone
 * test programs run no forkit, so starting one reports failure.
 */

#include <config.h>

#include <string>

#include <common/StringVector.hpp>
#include <wsd/COOLWSD.hpp>

int createForkit(const std::string&, const StringVector&) { return -1; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
