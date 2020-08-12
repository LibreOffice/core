/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#if !defined(VCL_DLLIMPLEMENTATION) && !defined(TOOLKIT_DLLIMPLEMENTATION)                         \
    && !defined(VCL_INTERNALS)
#error "don't use this in new code"
#endif

#include <vector>
#include <memory>

class SvTreeListEntry;
typedef std::vector<std::unique_ptr<SvTreeListEntry>> SvTreeListEntries;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
