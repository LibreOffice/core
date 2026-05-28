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

#include <config.h>

#include "AIHttpTransport.hpp"

#include <utility>

namespace ai
{
namespace
{
// Function-local static avoids any static-init-order dependency on the registrar.
HttpPostFn& store()
{
    static HttpPostFn fn;
    return fn;
}
} // namespace

void setHttpPostFn(HttpPostFn fn) { store() = std::move(fn); }

const HttpPostFn& httpPostFn() { return store(); }
} // namespace ai

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
