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

#include <rtl/ustring.hxx>

#include <utility>
#include <vector>

namespace sd
{
/** The design templates available to apply to a deck, both the bundled set and
    any preset templates an integrator or user has supplied. Each pair is the
    template's display name (its file base name, in its original letter case) and
    the URL of its template document. The bundled set comes first; a later
    template whose name has already been seen is skipped, so a bundled template
    takes precedence over a preset that shares its name. */
std::vector<std::pair<OUString, OUString>> CollectDesignTemplates();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
