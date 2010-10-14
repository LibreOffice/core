/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#define _CPPUHELPER_QUERYINTERFACE_HXX_

#include "sal/config.h"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Type.hxx"
#include "sal/types.h"

namespace cppu
{

/** Compares demanded type to given template argument types.

    @tplparam Interface1 interface type
    @param rType demanded type
    @param p1 interface pointer
    @return acquired interface of demanded type or empty Any
*/
template< class Interface1 >
inline ::com::sun::star::uno::Any SAL_CALL queryInterface(
    const ::com::sun::star::uno::Type & rType,
    Interface1 * p1 )
    SAL_THROW( () )
{
    if (rType == Interface1::static_type())
        return ::com::sun::star::uno::Any( &p1, rType );
    else
        return ::com::sun::star::uno::Any();
}
/** Compares demanded type to given template argument types.

    @tplparam Interface1 interface type
    @tplparam Interface2 interface type
    @param rType demanded type
    @param p1 interface pointer
    @param p2 interface pointer
    @return acquired interface of demanded type or empty Any
*/
template< class Interface1, class Interface2 >
inline ::com::sun::star::uno::Any SAL_CALL queryInterface(
    const ::com::sun::star::uno::Type & rType,
    Interface1 * p1, Interface2 * p2 )
    SAL_THROW( () )
{
    if (rType == Interface1::static_type())
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (rType == Interface2::static_type())
        return ::com::sun::star::uno::Any( &p2, rType );
    else
        return ::com::sun::star::uno::Any();
}
/** Compares demanded type to given template argument types.

    @tplparam Interface1 interface type
    @tplparam Interface2 interface type
    @tplparam Interface3 interface type
    @param rType demanded type
    @param p1 interface pointer
    @param p2 interface pointer
    @param p3 interface pointer
    @return acquired interface of demanded type or empty Any
*/
template< class Interface1, class Interface2, class Interface3 >
inline ::com::sun::star::uno::Any SAL_CALL queryInterface(
    const ::com::sun::star::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3 )
    SAL_THROW( () )
{
    if (rType == Interface1::static_type())
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (rType == Interface2::static_type())
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (rType == Interface3::static_type())
        return ::com::sun::star::uno::Any( &p3, rType );
    else
        return ::com::sun::star::uno::Any();
}
/** Compares demanded type to given template argument types.

    @tplparam Interface1 interface type
    @tplparam Interface2 interface type
    @tplparam Interface3 interface type
    @tplparam Interface4 interface type
    @param rType demanded type
    @param p1 interface pointer
    @param p2 interface pointer
    @param p3 interface pointer
    @param p4 interface pointer
    @return acquired interface of demanded type or empty Any
*/
template< class Interface1, class Interface2, class Interface3, class Interface4 >
inline ::com::sun::star::uno::Any SAL_CALL queryInterface(
    const ::com::sun::star::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4 )
    SAL_THROW( () )
{
    if (rType == Interface1::static_type())
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (rType == Interface2::static_type())
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (rType == Interface3::static_type())
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (rType == Interface4::static_type())
        return ::com::sun::star::uno::Any( &p4, rType );
    else
        return ::com::sun::star::uno::Any();
}
/** Compares demanded type to given template argument types.

    @tplparam Interface1 interface type
    @tplparam Interface2 interface type
    @tplparam Interface3 interface type
    @tplparam Interface4 interface type
    @tplparam Interface5 interface type
    @param rType demanded type
    @param p1 interface pointer
    @param p2 interface pointer
    @param p3 interface pointer
    @param p4 interface pointer
    @param p5 interface pointer
    @return acquired interface of demanded type or empty Any
*/
template< class Interface1, class Interface2, class Interface3, class Interface4, class Interface5 >
inline ::com::sun::star::uno::Any SAL_CALL queryInterface(
    const ::com::sun::star::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4, Interface5 * p5 )
    SAL_THROW( () )
{
    if (rType == Interface1::static_type())
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (rType == Interface2::static_type())
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (rType == Interface3::static_type())
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (rType == Interface4::static_type())
        return ::com::sun::star::uno::Any( &p4, rType );
    else if (rType == Interface5::static_type())
        return ::com::sun::star::uno::Any( &p5, rType );
    else
        return ::com::sun::star::uno::Any();
}
/** Compares demanded type to given template argument types.

    @tplparam Interface1 interface type
    @tplparam Interface2 interface type
    @tplparam Interface3 interface type
    @tplparam Interface4 interface type
    @tplparam Interface5 interface type
    @tplparam Interface6 interface type
    @param rType demanded type
    @param p1 interface pointer
    @param p2 interface pointer
    @param p3 interface pointer
    @param p4 interface pointer
    @param p5 interface pointer
    @param p6 interface pointer
    @return acquired interface of demanded type or empty Any
*/
template< class Interface1, class Interface2, class Interface3, class Interface4, class Interface5,
          class Interface6 >
inline ::com::sun::star::uno::Any SAL_CALL queryInterface(
    const ::com::sun::star::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4, Interface5 * p5,
    Interface6 * p6 )
    SAL_THROW( () )
{
    if (rType == Interface1::static_type())
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (rType == Interface2::static_type())
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (rType == Interface3::static_type())
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (rType == Interface4::static_type())
        return ::com::sun::star::uno::Any( &p4, rType );
    else if (rType == Interface5::static_type())
        return ::com::sun::star::uno::Any( &p5, rType );
    else if (rType == Interface6::static_type())
        return ::com::sun::star::uno::Any( &p6, rType );
    else
        return ::com::sun::star::uno::Any();
}
/** Compares demanded type to given template argument types.

    @tplparam Interface1 interface type
    @tplparam Interface2 interface type
    @tplparam Interface3 interface type
    @tplparam Interface4 interface type
    @tplparam Interface5 interface type
    @tplparam Interface6 interface type
    @tplparam Interface7 interface type
    @param rType demanded type
    @param p1 interface pointer
    @param p2 interface pointer
    @param p3 interface pointer
    @param p4 interface pointer
    @param p5 interface pointer
    @param p6 interface pointer
    @param p7 interface pointer
    @return acquired interface of demanded type or empty Any
*/
template< class Interface1, class Interface2, class Interface3, class Interface4, class Interface5,
          class Interface6, class Interface7 >
inline ::com::sun::star::uno::Any SAL_CALL queryInterface(
    const ::com::sun::star::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4, Interface5 * p5,
    Interface6 * p6, Interface7 * p7 )
    SAL_THROW( () )
{
    if (rType == Interface1::static_type())
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (rType == Interface2::static_type())
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (rType == Interface3::static_type())
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (rType == Interface4::static_type())
        return ::com::sun::star::uno::Any( &p4, rType );
    else if (rType == Interface5::static_type())
        return ::com::sun::star::uno::Any( &p5, rType );
    else if (rType == Interface6::static_type())
        return ::com::sun::star::uno::Any( &p6, rType );
    else if (rType == Interface7::static_type())
        return ::com::sun::star::uno::Any( &p7, rType );
    else
        return ::com::sun::star::uno::Any();
}
/** Compares demanded type to given template argument types.

    @tplparam Interface1 interface type
    @tplparam Interface2 interface type
    @tplparam Interface3 interface type
    @tplparam Interface4 interface type
    @tplparam Interface5 interface type
    @tplparam Interface6 interface type
    @tplparam Interface7 interface type
    @tplparam Interface8 interface type
    @param rType demanded type
    @param p1 interface pointer
    @param p2 interface pointer
    @param p3 interface pointer
    @param p4 interface pointer
    @param p5 interface pointer
    @param p6 interface pointer
    @param p7 interface pointer
    @param p8 interface pointer
    @return acquired interface of demanded type or empty Any
*/
template< class Interface1, class Interface2, class Interface3, class Interface4, class Interface5,
          class Interface6, class Interface7, class Interface8 >
inline ::com::sun::star::uno::Any SAL_CALL queryInterface(
    const ::com::sun::star::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4, Interface5 * p5,
    Interface6 * p6, Interface7 * p7, Interface8 * p8 )
    SAL_THROW( () )
{
    if (rType == Interface1::static_type())
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (rType == Interface2::static_type())
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (rType == Interface3::static_type())
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (rType == Interface4::static_type())
        return ::com::sun::star::uno::Any( &p4, rType );
    else if (rType == Interface5::static_type())
        return ::com::sun::star::uno::Any( &p5, rType );
    else if (rType == Interface6::static_type())
        return ::com::sun::star::uno::Any( &p6, rType );
    else if (rType == Interface7::static_type())
        return ::com::sun::star::uno::Any( &p7, rType );
    else if (rType == Interface8::static_type())
        return ::com::sun::star::uno::Any( &p8, rType );
    else
        return ::com::sun::star::uno::Any();
}
/** Compares demanded type to given template argument types.

    @tplparam Interface1 interface type
    @tplparam Interface2 interface type
    @tplparam Interface3 interface type
    @tplparam Interface4 interface type
    @tplparam Interface5 interface type
    @tplparam Interface6 interface type
    @tplparam Interface7 interface type
    @tplparam Interface8 interface type
    @tplparam Interface9 interface type
    @param rType demanded type
    @param p1 interface pointer
    @param p2 interface pointer
    @param p3 interface pointer
    @param p4 interface pointer
    @param p5 interface pointer
    @param p6 interface pointer
    @param p7 interface pointer
    @param p8 interface pointer
    @param p9 interface pointer
    @return acquired interface of demanded type or empty Any
*/
template< class Interface1, class Interface2, class Interface3, class Interface4, class Interface5,
          class Interface6, class Interface7, class Interface8, class Interface9 >
inline ::com::sun::star::uno::Any SAL_CALL queryInterface(
    const ::com::sun::star::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4, Interface5 * p5,
    Interface6 * p6, Interface7 * p7, Interface8 * p8, Interface9 * p9 )
    SAL_THROW( () )
{
    if (rType == Interface1::static_type())
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (rType == Interface2::static_type())
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (rType == Interface3::static_type())
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (rType == Interface4::static_type())
        return ::com::sun::star::uno::Any( &p4, rType );
    else if (rType == Interface5::static_type())
        return ::com::sun::star::uno::Any( &p5, rType );
    else if (rType == Interface6::static_type())
        return ::com::sun::star::uno::Any( &p6, rType );
    else if (rType == Interface7::static_type())
        return ::com::sun::star::uno::Any( &p7, rType );
    else if (rType == Interface8::static_type())
        return ::com::sun::star::uno::Any( &p8, rType );
    else if (rType == Interface9::static_type())
        return ::com::sun::star::uno::Any( &p9, rType );
    else
        return ::com::sun::star::uno::Any();
}
/** Compares demanded type to given template argument types.

    @tplparam Interface1 interface type
    @tplparam Interface2 interface type
    @tplparam Interface3 interface type
    @tplparam Interface4 interface type
    @tplparam Interface5 interface type
    @tplparam Interface6 interface type
    @tplparam Interface7 interface type
    @tplparam Interface8 interface type
    @tplparam Interface9 interface type
    @tplparam Interface10 interface type
    @param rType demanded type
    @param p1 interface pointer
    @param p2 interface pointer
    @param p3 interface pointer
    @param p4 interface pointer
    @param p5 interface pointer
    @param p6 interface pointer
    @param p7 interface pointer
    @param p8 interface pointer
    @param p9 interface pointer
    @param p10 interface pointer
    @return acquired interface of demanded type or empty Any
*/
template< class Interface1, class Interface2, class Interface3, class Interface4, class Interface5,
          class Interface6, class Interface7, class Interface8, class Interface9, class Interface10 >
inline ::com::sun::star::uno::Any SAL_CALL queryInterface(
    const ::com::sun::star::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4, Interface5 * p5,
    Interface6 * p6, Interface7 * p7, Interface8 * p8, Interface9 * p9, Interface10 * p10 )
    SAL_THROW( () )
{
    if (rType == Interface1::static_type())
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (rType == Interface2::static_type())
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (rType == Interface3::static_type())
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (rType == Interface4::static_type())
        return ::com::sun::star::uno::Any( &p4, rType );
    else if (rType == Interface5::static_type())
        return ::com::sun::star::uno::Any( &p5, rType );
    else if (rType == Interface6::static_type())
        return ::com::sun::star::uno::Any( &p6, rType );
    else if (rType == Interface7::static_type())
        return ::com::sun::star::uno::Any( &p7, rType );
    else if (rType == Interface8::static_type())
        return ::com::sun::star::uno::Any( &p8, rType );
    else if (rType == Interface9::static_type())
        return ::com::sun::star::uno::Any( &p9, rType );
    else if (rType == Interface10::static_type())
        return ::com::sun::star::uno::Any( &p10, rType );
    else
        return ::com::sun::star::uno::Any();
}
/** Compares demanded type to given template argument types.

    @tplparam Interface1 interface type
    @tplparam Interface2 interface type
    @tplparam Interface3 interface type
    @tplparam Interface4 interface type
    @tplparam Interface5 interface type
    @tplparam Interface6 interface type
    @tplparam Interface7 interface type
    @tplparam Interface8 interface type
    @tplparam Interface9 interface type
    @tplparam Interface10 interface type
    @tplparam Interface11 interface type
    @param rType demanded type
    @param p1 interface pointer
    @param p2 interface pointer
    @param p3 interface pointer
    @param p4 interface pointer
    @param p5 interface pointer
    @param p6 interface pointer
    @param p7 interface pointer
    @param p8 interface pointer
    @param p9 interface pointer
    @param p10 interface pointer
    @param p11 interface pointer
    @return acquired interface of demanded type or empty Any
*/
template< class Interface1, class Interface2, class Interface3, class Interface4, class Interface5,
          class Interface6, class Interface7, class Interface8, class Interface9, class Interface10,
          class Interface11 >
inline ::com::sun::star::uno::Any SAL_CALL queryInterface(
    const ::com::sun::star::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4, Interface5 * p5,
    Interface6 * p6, Interface7 * p7, Interface8 * p8, Interface9 * p9, Interface10 * p10,
    Interface11 * p11 )
    SAL_THROW( () )
{
    if (rType == Interface1::static_type())
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (rType == Interface2::static_type())
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (rType == Interface3::static_type())
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (rType == Interface4::static_type())
        return ::com::sun::star::uno::Any( &p4, rType );
    else if (rType == Interface5::static_type())
        return ::com::sun::star::uno::Any( &p5, rType );
    else if (rType == Interface6::static_type())
        return ::com::sun::star::uno::Any( &p6, rType );
    else if (rType == Interface7::static_type())
        return ::com::sun::star::uno::Any( &p7, rType );
    else if (rType == Interface8::static_type())
        return ::com::sun::star::uno::Any( &p8, rType );
    else if (rType == Interface9::static_type())
        return ::com::sun::star::uno::Any( &p9, rType );
    else if (rType == Interface10::static_type())
        return ::com::sun::star::uno::Any( &p10, rType );
    else if (rType == Interface11::static_type())
        return ::com::sun::star::uno::Any( &p11, rType );
    else
        return ::com::sun::star::uno::Any();
}
/** Compares demanded type to given template argument types.

    @tplparam Interface1 interface type
    @tplparam Interface2 interface type
    @tplparam Interface3 interface type
    @tplparam Interface4 interface type
    @tplparam Interface5 interface type
    @tplparam Interface6 interface type
    @tplparam Interface7 interface type
    @tplparam Interface8 interface type
    @tplparam Interface9 interface type
    @tplparam Interface10 interface type
    @tplparam Interface11 interface type
    @tplparam Interface12 interface type
    @param rType demanded type
    @param p1 interface pointer
    @param p2 interface pointer
    @param p3 interface pointer
    @param p4 interface pointer
    @param p5 interface pointer
    @param p6 interface pointer
    @param p7 interface pointer
    @param p8 interface pointer
    @param p9 interface pointer
    @param p10 interface pointer
    @param p11 interface pointer
    @param p12 interface pointer
    @return acquired interface of demanded type or empty Any
*/
template< class Interface1, class Interface2, class Interface3, class Interface4, class Interface5,
          class Interface6, class Interface7, class Interface8, class Interface9, class Interface10,
          class Interface11, class Interface12 >
inline ::com::sun::star::uno::Any SAL_CALL queryInterface(
    const ::com::sun::star::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4, Interface5 * p5,
    Interface6 * p6, Interface7 * p7, Interface8 * p8, Interface9 * p9, Interface10 * p10,
    Interface11 * p11, Interface12 * p12 )
    SAL_THROW( () )
{
    if (rType == Interface1::static_type())
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (rType == Interface2::static_type())
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (rType == Interface3::static_type())
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (rType == Interface4::static_type())
        return ::com::sun::star::uno::Any( &p4, rType );
    else if (rType == Interface5::static_type())
        return ::com::sun::star::uno::Any( &p5, rType );
    else if (rType == Interface6::static_type())
        return ::com::sun::star::uno::Any( &p6, rType );
    else if (rType == Interface7::static_type())
        return ::com::sun::star::uno::Any( &p7, rType );
    else if (rType == Interface8::static_type())
        return ::com::sun::star::uno::Any( &p8, rType );
    else if (rType == Interface9::static_type())
        return ::com::sun::star::uno::Any( &p9, rType );
    else if (rType == Interface10::static_type())
        return ::com::sun::star::uno::Any( &p10, rType );
    else if (rType == Interface11::static_type())
        return ::com::sun::star::uno::Any( &p11, rType );
    else if (rType == Interface12::static_type())
        return ::com::sun::star::uno::Any( &p12, rType );
    else
        return ::com::sun::star::uno::Any();
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
