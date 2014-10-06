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

namespace o3tl
{

/// The benefits of using this template are:
///  - no accidental mixing of bit flags from different types
///  - no accidental conversion to integer types
///  - no accidental assignment of integer values to bit-flag types
///
/// \param U the underlying integer type
/// \param M the all-bit-set value for the bit-flags
/// \param D the default value for the default constructor
///
template<typename U, U M, U D = 0>
struct typed_flags SAL_FINAL {

    typedef typed_flags<U,M,D> SELF;

    U mValue;

    typed_flags() : mValue(D) {}

    typed_flags(const SELF & v) : mValue(v.mValue) {}

    explicit typed_flags(U v) : mValue(v)
    {
        assert( v <= M );
    }

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

    inline bool operator==(const SELF& other) const
    {
        return mValue == other.mValue;
    }

    inline bool operator!=(const SELF& other) const
    {
        return mValue != other.mValue;
    }

    friend SELF operator| (SELF lhs, SELF rhs)
    {
        return SELF(lhs.mValue | rhs.mValue);
    }

    friend SELF operator& (SELF lhs, SELF rhs)
    {
        return SELF(lhs.mValue & rhs.mValue);
    }

    friend SELF operator~ (SELF rhs)
    {
        return SELF(M & ~(rhs.mValue));
    }

    friend SELF operator|= (SELF lhs, SELF rhs)
    {
        lhs = SELF(lhs.mValue | rhs.mValue);
        return lhs;
    }

    friend SELF operator&= (SELF lhs, SELF rhs)
    {
        lhs = SELF(lhs.mValue & rhs.mValue);
        return lhs;
    }
};

} /* namespace o3tl */

#endif /* INCLUDED_O3TL_TYPED_FLAGS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
