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

#include <sal/config.h>

#include <cassert>

#include <osl/file.h>

#include <acceptableaccessdenial.hxx>

namespace
{
// Above zero while the thread has said it handles a denied file access itself.
thread_local int acceptableAccessDenials;
}

void osl_pushAcceptableAccessDenial() { ++acceptableAccessDenials; }

void osl_popAcceptableAccessDenial()
{
    assert(acceptableAccessDenials > 0);
    --acceptableAccessDenials;
}

bool isAccessDenialAcceptable() { return acceptableAccessDenials > 0; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
