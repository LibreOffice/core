/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// A wrapper selecting either std::optional or boost::optional as a fallback for Xcode < 10.  To be
// removed once std::optional is available everywhere.

#ifndef INCLUDED_O3TL_OPTIONAL_HXX
#define INCLUDED_O3TL_OPTIONAL_HXX

#include <sal/config.h>

#if defined __APPLE__ && !__has_include(<optional>)

#include <boost/none.hpp>
#include <boost/optional.hpp>

namespace o3tl
{
using boost::make_optional;
using boost::optional;

inline constexpr auto nullopt = boost::none;
}

#else

#include <optional>

namespace o3tl
{
using std::make_optional;
using std::nullopt;
using std::optional;
}

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
