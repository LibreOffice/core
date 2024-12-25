/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_O3TL_SOURCE_LOCATION_HXX
#define INCLUDED_O3TL_SOURCE_LOCATION_HXX

#if __has_include(<version>)
#include <version>
#endif

#if defined(__cpp_lib_source_location) && __cpp_lib_source_location >= 201907
#include <source_location>
#define LIBO_USE_SOURCE_LOCATION std
#elif __has_include(<experimental/source_location>)
#include <experimental/source_location>
#define LIBO_USE_SOURCE_LOCATION std::experimental
#endif

#if defined LIBO_USE_SOURCE_LOCATION
namespace o3tl
{
using source_location = LIBO_USE_SOURCE_LOCATION::source_location;
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
