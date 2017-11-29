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
#ifndef INCLUDED_COMPHELPER_EXTRACT_HXX
#define INCLUDED_COMPHELPER_EXTRACT_HXX

#include <sal/config.h>

#include <cassert>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/TypeClass.hpp>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <cppu/unotype.hxx>

namespace cppu
{

/**
 * Sets enum from int32 value.  This function does NOT check for valid enum values!
 *
 * @param nEnum         int32 enum value
 * @param rType         enum type
 * @return enum or empty any.
 */
inline css::uno::Any int2enum(
    sal_Int32 nEnum, const css::uno::Type & rType )
{
    assert(rType.getTypeClass() == css::uno::TypeClass_ENUM);
    return css::uno::Any( &nEnum, rType );
}

/**
 * Sets int32 from enum or int in any.
 *
 * @param[out] rnEnum   int32 enum value
 * @param rAny          enum or int
 * @retval true if enum or int value was set
 * @retval false otherwise
 */
inline bool enum2int( sal_Int32 & rnEnum, const css::uno::Any & rAny )
{
    if (rAny.getValueTypeClass() == css::uno::TypeClass_ENUM)
    {
        rnEnum = * static_cast< const sal_Int32 * >( rAny.getValue() );
        return true;
    }

    return rAny >>= rnEnum;
}

/**
 * Sets int32 from enum or int in any with additional typecheck
 *
 * @param[out] eRet     the enum value as int. If there is no enum of the given type
 *                      a css::lang::IllegalArgumentException is thrown
 * @param rAny          enum or int
 * @throws css::lang::IllegalArgumentException
 */
template< typename E >
inline void any2enum( E & eRet, const css::uno::Any & rAny )
{
    // check for typesafe enum
    if (! (rAny >>= eRet))
    {
        // if not enum, maybe integer?
        sal_Int32 nValue = 0;
        if (! (rAny >>= nValue))
            throw css::lang::IllegalArgumentException();

        eRet = static_cast<E>(nValue);
    }
}

/**
 * Extracts a boolean either as a bool or an integer from
 * an any. If there is no bool or integer inside the any
 * a css::lang::IllegalArgumentException is thrown
 *
 * @throws css::lang::IllegalArgumentException
 */
inline bool any2bool( const css::uno::Any & rAny )
{
    bool b;
    if (rAny >>= b)
    {
        return b;
    }
    else
    {
        sal_Int32 nValue = 0;
        if (! (rAny >>= nValue))
            throw css::lang::IllegalArgumentException();
        return nValue != 0;
    }
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
