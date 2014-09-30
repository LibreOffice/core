/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_O3TL_PTR_CONTAINER_HXX
#define INCLUDED_O3TL_PTR_CONTAINER_HXX

#include <sal/config.h>

#include <memory>

// Some glue for using std::unique_ptr with the Boost Pointer Container Library:

namespace o3tl { namespace ptr_container {

template<typename C, typename T>
inline void push_back(C & container, std::unique_ptr<T> && element) {
    container.push_back(element.get());
    element.release();
}

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
