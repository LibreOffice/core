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
 * Stub missing symbols required for unit tests ...
 */

#include <config.h>

#include <kit/ChildSession.hpp>

void ChildSession::loKitCallback(const COKitCallbackType /* type */,
                                 const std::string& /* payload */) {}
void ChildSession::disconnect() {}
int ChildSession::getSpeed() { return 0; }
bool ChildSession::_handleInput(const char* /*buffer*/, int /*length*/) { return false; }
TilePrioritizer::Priority ChildSession::getTilePriority(const TileDesc &) const { return TilePrioritizer::Priority::NORMAL; }
ChildSession::~ChildSession() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
