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

#include <cstddef>

/** Whole pages of memory taken directly from the system, outside the malloc heap, so that
    releasing them returns them to the system at once. */
namespace vcl::pagememory
{
/** The size of one page in bytes. */
std::size_t pageSize();

/** Returns rSize bytes rounded up to whole pages, and sets rSize to the size returned, or returns
    nullptr if the system has none to give. */
void* allocate(std::size_t& rSize);

/** Returns to the system pages that allocate returned, with the size it set. */
void release(void* pAddress, std::size_t nSize);

/** Makes pages that allocate returned inaccessible instead of returning them, so that any later
    access faults. They stay mapped for the life of the process. */
void makeInaccessible(void* pAddress, std::size_t nSize);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
