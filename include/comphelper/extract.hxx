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

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/TypeClass.hpp>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <cppu/unotype.hxx>

namespace cppu
{

/**
 * Sets enum from int32 value.  This function does NOT check for valid enum values!
 *<BR>
 * @param nEnum         int32 enum value
 * @param rType         enum type
 * @return enum or empty any.
 */
inline ::com::sun::star::uno::Any SAL_CALL int2enum(
    sal_Int32 nEnum, const ::com::sun::star::uno::Type & rType )
{
    if (rType.getTypeClass() == ::com::sun::star::uno::TypeClass_ENUM)
    {
        int nVal = nEnum;
        return ::com::sun::star::uno::Any( &nVal, rType );
    }
    return ::com::sun::star::uno::Any();
}

/**
 * Sets int32 from enum or int in any.
 *<BR>
 * @param rnEnum        [out] int32 enum value
 * @param rAny          enum or int
 * @param sal_True if enum or int value was set else sal_False.
 */
inline bool SAL_CALL enum2int( sal_Int32 & rnEnum, const ::com::sun::star::uno::Any & rAny )
{
    if (rAny.getValueTypeClass() == ::com::sun::star::uno::TypeClass_ENUM)
    {
        rnEnum = * static_cast< const int * >( rAny.getValue() );
        return true;
    }

    return rAny >>= rnEnum;
}

/**
 * Sets int32 from enum or int in any with additional typecheck
 * <BR>
 * @param rAny          enum or int
 * @param eRet          the enum value as int. If there is not enum of the given type or
 *                      a ::com::sun::star::lang::IllegalArgumentException is thrown
 */
template< typename E >
inline void SAL_CALL any2enum( E & eRet, const ::com::sun::star::uno::Any & rAny )
    throw( ::com::sun::star::lang::IllegalArgumentException )
{
    // check for typesafe enum
    if (! (rAny >>= eRet))
    {
        // if not enum, maybe integer?
        sal_Int32 nValue = 0;
        if (! (rAny >>= nValue))
            throw ::com::sun::star::lang::IllegalArgumentException();

        eRet = static_cast<E>(nValue);
    }
}

/**
 * Template function to create an uno::Any from an enum
 *
 * @DEPRECATED : use makeAny< E >()
 *
 */
template< typename E >
inline ::com::sun::star::uno::Any SAL_CALL enum2any( E eEnum )
{
    return ::com::sun::star::uno::Any( &eEnum, ::cppu::UnoType< E >::get() );
}

/**
 * extracts a boolean either as a sal_Bool or an integer from
 * an any. If there is no sal_Bool or integer inside the any
 * a ::com::sun::star::lang::IllegalArgumentException is thrown
 *
 */
inline bool SAL_CALL any2bool( const ::com::sun::star::uno::Any & rAny )
    throw( ::com::sun::star::lang::IllegalArgumentException )
{
    if (rAny.getValueTypeClass() == ::com::sun::star::uno::TypeClass_BOOLEAN)
    {
        return *static_cast<sal_Bool const *>(rAny.getValue());
    }
    else
    {
        sal_Int32 nValue = 0;
        if (! (rAny >>= nValue))
            throw ::com::sun::star::lang::IllegalArgumentException();
        return nValue != 0;
    }
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
