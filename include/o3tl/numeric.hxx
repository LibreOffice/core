/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_O3TL_NUMERIC_HXX
#define INCLUDED_O3TL_NUMERIC_HXX

#include <bit>
#include <stdexcept>

namespace o3tl
{
    struct divide_by_zero final : public std::runtime_error
    {
        explicit divide_by_zero()
            : std::runtime_error("divide by zero")
        {
        }
    };

    template<class T> constexpr unsigned int number_of_bits(T x)
    {
        return sizeof(T) * 8 - std::countl_zero(x);
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
