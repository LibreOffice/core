/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_O3TL_UNDERLYING_TYPE_HXX
#define INCLUDED_O3TL_UNDERLYING_TYPE_HXX

#include <sal/config.h>

#include <type_traits>

namespace o3tl {

template<typename T> struct underlying_type {
#if defined __GNUC__ && __GNUC__ == 4 && __GNUC_MINOR__ <= 6 && \
        !defined __clang__
    typedef int type;
#else
    typedef typename std::underlying_type<T>::type type;
#endif
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
