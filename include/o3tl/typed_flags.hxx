/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_O3TL_TYPED_FLAGS_HXX
#define INCLUDED_O3TL_TYPED_FLAGS_HXX

#include <sal/types.h>
#include <boost/static_assert.hpp>

namespace o3tl
{

/// The benefits of using this template are:
///  - no accidental mixing of bit flags from different types
///  - no accidental conversion to integer types
///  - no accidental assignment of integer values to bit-flag types
///
/// \param U the underlying integer type, must be unsigned
/// \param M the all-bit-set value for the bit-flags
/// \param D the default value for the default constructor
///
template<typename U, U M, U D = 0>
struct typed_flags SAL_FINAL {

private:
    typedef typed_flags<U,M,D> SELF;

    U mValue;

public:
    typed_flags() : mValue(D) {}

    typed_flags(const SELF & v) : mValue(v.mValue) {}

    explicit typed_flags(U v) : mValue(v)
    {
        // type U must be unsigned
        BOOST_STATIC_ASSERT( ! ::std::numeric_limits< U >::is_signed );
        assert( v <= M );
    }

    inline U getValue() const { return mValue; }

    /*
      Allow expressions like "if (mnPushFlags & PUSH_REFPOINT)"
      This cannot be implicit or the compiler will use this as a path
      to convert this type to integer values automatically, which is
      something we're trying to prevent.
    */
    explicit operator bool()
    {
        return mValue != 0;
    }

    bool operator== (SELF other) const
    {
        return mValue == other.mValue;
    }

    bool operator!= (SELF other) const
    {
        return mValue != other.mValue;
    }

    SELF operator| (SELF rhs) const
    {
        return SELF(mValue | rhs.mValue);
    }

    SELF operator& (SELF rhs) const
    {
        return SELF(mValue & rhs.mValue);
    }

    SELF operator~ () const
    {
        return SELF(M & ~(mValue));
    }

    void operator|= (SELF rhs)
    {
        mValue |= rhs.mValue;
    }

    void operator&= (SELF rhs)
    {
        mValue &= rhs.mValue;
    }
};

} /* namespace o3tl */

#endif /* INCLUDED_O3TL_TYPED_FLAGS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
