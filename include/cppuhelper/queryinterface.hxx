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

#ifndef INCLUDED_CPPUHELPER_QUERYINTERFACE_HXX
#define INCLUDED_CPPUHELPER_QUERYINTERFACE_HXX

#include "sal/config.h"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Type.h"
#include "sal/types.h"

namespace cppu
{

/** Compares demanded type to given template argument types.

    @tparam Interface1 interface type
    @param rType demanded type
    @param p1 interface pointer
    @return acquired interface of demanded type or empty Any
*/
template< class Interface1 >
inline css::uno::Any SAL_CALL queryInterface(
    const css::uno::Type & rType,
    Interface1 * p1 )
{
    if (rType == Interface1::static_type())
        return css::uno::Any( &p1, rType );
    else
        return css::uno::Any();
}
/** Compares demanded type to given template argument types.

    @tparam Interface1 interface type
    @tparam Interface2 interface type
    @param rType demanded type
    @param p1 interface pointer
    @param p2 interface pointer
    @return acquired interface of demanded type or empty Any
*/
template< class Interface1, class Interface2 >
inline css::uno::Any SAL_CALL queryInterface(
    const css::uno::Type & rType,
    Interface1 * p1, Interface2 * p2 )
{
    if (rType == Interface1::static_type())
        return css::uno::Any( &p1, rType );
    else if (rType == Interface2::static_type())
        return css::uno::Any( &p2, rType );
    else
        return css::uno::Any();
}
/** Compares demanded type to given template argument types.

    @tparam Interface1 interface type
    @tparam Interface2 interface type
    @tparam Interface3 interface type
    @param rType demanded type
    @param p1 interface pointer
    @param p2 interface pointer
    @param p3 interface pointer
    @return acquired interface of demanded type or empty Any
*/
template< class Interface1, class Interface2, class Interface3 >
inline css::uno::Any SAL_CALL queryInterface(
    const css::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3 )
{
    if (rType == Interface1::static_type())
        return css::uno::Any( &p1, rType );
    else if (rType == Interface2::static_type())
        return css::uno::Any( &p2, rType );
    else if (rType == Interface3::static_type())
        return css::uno::Any( &p3, rType );
    else
        return css::uno::Any();
}
/** Compares demanded type to given template argument types.

    @tparam Interface1 interface type
    @tparam Interface2 interface type
    @tparam Interface3 interface type
    @tparam Interface4 interface type
    @param rType demanded type
    @param p1 interface pointer
    @param p2 interface pointer
    @param p3 interface pointer
    @param p4 interface pointer
    @return acquired interface of demanded type or empty Any
*/
template< class Interface1, class Interface2, class Interface3, class Interface4 >
inline css::uno::Any SAL_CALL queryInterface(
    const css::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4 )
{
    if (rType == Interface1::static_type())
        return css::uno::Any( &p1, rType );
    else if (rType == Interface2::static_type())
        return css::uno::Any( &p2, rType );
    else if (rType == Interface3::static_type())
        return css::uno::Any( &p3, rType );
    else if (rType == Interface4::static_type())
        return css::uno::Any( &p4, rType );
    else
        return css::uno::Any();
}
/** Compares demanded type to given template argument types.

    @tparam Interface1 interface type
    @tparam Interface2 interface type
    @tparam Interface3 interface type
    @tparam Interface4 interface type
    @tparam Interface5 interface type
    @param rType demanded type
    @param p1 interface pointer
    @param p2 interface pointer
    @param p3 interface pointer
    @param p4 interface pointer
    @param p5 interface pointer
    @return acquired interface of demanded type or empty Any
*/
template< class Interface1, class Interface2, class Interface3, class Interface4, class Interface5 >
inline css::uno::Any SAL_CALL queryInterface(
    const css::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4, Interface5 * p5 )
{
    if (rType == Interface1::static_type())
        return css::uno::Any( &p1, rType );
    else if (rType == Interface2::static_type())
        return css::uno::Any( &p2, rType );
    else if (rType == Interface3::static_type())
        return css::uno::Any( &p3, rType );
    else if (rType == Interface4::static_type())
        return css::uno::Any( &p4, rType );
    else if (rType == Interface5::static_type())
        return css::uno::Any( &p5, rType );
    else
        return css::uno::Any();
}
/** Compares demanded type to given template argument types.

    @tparam Interface1 interface type
    @tparam Interface2 interface type
    @tparam Interface3 interface type
    @tparam Interface4 interface type
    @tparam Interface5 interface type
    @tparam Interface6 interface type
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
inline css::uno::Any SAL_CALL queryInterface(
    const css::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4, Interface5 * p5,
    Interface6 * p6 )
{
    if (rType == Interface1::static_type())
        return css::uno::Any( &p1, rType );
    else if (rType == Interface2::static_type())
        return css::uno::Any( &p2, rType );
    else if (rType == Interface3::static_type())
        return css::uno::Any( &p3, rType );
    else if (rType == Interface4::static_type())
        return css::uno::Any( &p4, rType );
    else if (rType == Interface5::static_type())
        return css::uno::Any( &p5, rType );
    else if (rType == Interface6::static_type())
        return css::uno::Any( &p6, rType );
    else
        return css::uno::Any();
}
/** Compares demanded type to given template argument types.

    @tparam Interface1 interface type
    @tparam Interface2 interface type
    @tparam Interface3 interface type
    @tparam Interface4 interface type
    @tparam Interface5 interface type
    @tparam Interface6 interface type
    @tparam Interface7 interface type
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
inline css::uno::Any SAL_CALL queryInterface(
    const css::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4, Interface5 * p5,
    Interface6 * p6, Interface7 * p7 )
{
    if (rType == Interface1::static_type())
        return css::uno::Any( &p1, rType );
    else if (rType == Interface2::static_type())
        return css::uno::Any( &p2, rType );
    else if (rType == Interface3::static_type())
        return css::uno::Any( &p3, rType );
    else if (rType == Interface4::static_type())
        return css::uno::Any( &p4, rType );
    else if (rType == Interface5::static_type())
        return css::uno::Any( &p5, rType );
    else if (rType == Interface6::static_type())
        return css::uno::Any( &p6, rType );
    else if (rType == Interface7::static_type())
        return css::uno::Any( &p7, rType );
    else
        return css::uno::Any();
}
/** Compares demanded type to given template argument types.

    @tparam Interface1 interface type
    @tparam Interface2 interface type
    @tparam Interface3 interface type
    @tparam Interface4 interface type
    @tparam Interface5 interface type
    @tparam Interface6 interface type
    @tparam Interface7 interface type
    @tparam Interface8 interface type
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
inline css::uno::Any SAL_CALL queryInterface(
    const css::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4, Interface5 * p5,
    Interface6 * p6, Interface7 * p7, Interface8 * p8 )
{
    if (rType == Interface1::static_type())
        return css::uno::Any( &p1, rType );
    else if (rType == Interface2::static_type())
        return css::uno::Any( &p2, rType );
    else if (rType == Interface3::static_type())
        return css::uno::Any( &p3, rType );
    else if (rType == Interface4::static_type())
        return css::uno::Any( &p4, rType );
    else if (rType == Interface5::static_type())
        return css::uno::Any( &p5, rType );
    else if (rType == Interface6::static_type())
        return css::uno::Any( &p6, rType );
    else if (rType == Interface7::static_type())
        return css::uno::Any( &p7, rType );
    else if (rType == Interface8::static_type())
        return css::uno::Any( &p8, rType );
    else
        return css::uno::Any();
}
/** Compares demanded type to given template argument types.

    @tparam Interface1 interface type
    @tparam Interface2 interface type
    @tparam Interface3 interface type
    @tparam Interface4 interface type
    @tparam Interface5 interface type
    @tparam Interface6 interface type
    @tparam Interface7 interface type
    @tparam Interface8 interface type
    @tparam Interface9 interface type
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
inline css::uno::Any SAL_CALL queryInterface(
    const css::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4, Interface5 * p5,
    Interface6 * p6, Interface7 * p7, Interface8 * p8, Interface9 * p9 )
{
    if (rType == Interface1::static_type())
        return css::uno::Any( &p1, rType );
    else if (rType == Interface2::static_type())
        return css::uno::Any( &p2, rType );
    else if (rType == Interface3::static_type())
        return css::uno::Any( &p3, rType );
    else if (rType == Interface4::static_type())
        return css::uno::Any( &p4, rType );
    else if (rType == Interface5::static_type())
        return css::uno::Any( &p5, rType );
    else if (rType == Interface6::static_type())
        return css::uno::Any( &p6, rType );
    else if (rType == Interface7::static_type())
        return css::uno::Any( &p7, rType );
    else if (rType == Interface8::static_type())
        return css::uno::Any( &p8, rType );
    else if (rType == Interface9::static_type())
        return css::uno::Any( &p9, rType );
    else
        return css::uno::Any();
}
/** Compares demanded type to given template argument types.

    @tparam Interface1 interface type
    @tparam Interface2 interface type
    @tparam Interface3 interface type
    @tparam Interface4 interface type
    @tparam Interface5 interface type
    @tparam Interface6 interface type
    @tparam Interface7 interface type
    @tparam Interface8 interface type
    @tparam Interface9 interface type
    @tparam Interface10 interface type
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
inline css::uno::Any SAL_CALL queryInterface(
    const css::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4, Interface5 * p5,
    Interface6 * p6, Interface7 * p7, Interface8 * p8, Interface9 * p9, Interface10 * p10 )
{
    if (rType == Interface1::static_type())
        return css::uno::Any( &p1, rType );
    else if (rType == Interface2::static_type())
        return css::uno::Any( &p2, rType );
    else if (rType == Interface3::static_type())
        return css::uno::Any( &p3, rType );
    else if (rType == Interface4::static_type())
        return css::uno::Any( &p4, rType );
    else if (rType == Interface5::static_type())
        return css::uno::Any( &p5, rType );
    else if (rType == Interface6::static_type())
        return css::uno::Any( &p6, rType );
    else if (rType == Interface7::static_type())
        return css::uno::Any( &p7, rType );
    else if (rType == Interface8::static_type())
        return css::uno::Any( &p8, rType );
    else if (rType == Interface9::static_type())
        return css::uno::Any( &p9, rType );
    else if (rType == Interface10::static_type())
        return css::uno::Any( &p10, rType );
    else
        return css::uno::Any();
}
/** Compares demanded type to given template argument types.

    @tparam Interface1 interface type
    @tparam Interface2 interface type
    @tparam Interface3 interface type
    @tparam Interface4 interface type
    @tparam Interface5 interface type
    @tparam Interface6 interface type
    @tparam Interface7 interface type
    @tparam Interface8 interface type
    @tparam Interface9 interface type
    @tparam Interface10 interface type
    @tparam Interface11 interface type
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
inline css::uno::Any SAL_CALL queryInterface(
    const css::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4, Interface5 * p5,
    Interface6 * p6, Interface7 * p7, Interface8 * p8, Interface9 * p9, Interface10 * p10,
    Interface11 * p11 )
{
    if (rType == Interface1::static_type())
        return css::uno::Any( &p1, rType );
    else if (rType == Interface2::static_type())
        return css::uno::Any( &p2, rType );
    else if (rType == Interface3::static_type())
        return css::uno::Any( &p3, rType );
    else if (rType == Interface4::static_type())
        return css::uno::Any( &p4, rType );
    else if (rType == Interface5::static_type())
        return css::uno::Any( &p5, rType );
    else if (rType == Interface6::static_type())
        return css::uno::Any( &p6, rType );
    else if (rType == Interface7::static_type())
        return css::uno::Any( &p7, rType );
    else if (rType == Interface8::static_type())
        return css::uno::Any( &p8, rType );
    else if (rType == Interface9::static_type())
        return css::uno::Any( &p9, rType );
    else if (rType == Interface10::static_type())
        return css::uno::Any( &p10, rType );
    else if (rType == Interface11::static_type())
        return css::uno::Any( &p11, rType );
    else
        return css::uno::Any();
}
/** Compares demanded type to given template argument types.

    @tparam Interface1 interface type
    @tparam Interface2 interface type
    @tparam Interface3 interface type
    @tparam Interface4 interface type
    @tparam Interface5 interface type
    @tparam Interface6 interface type
    @tparam Interface7 interface type
    @tparam Interface8 interface type
    @tparam Interface9 interface type
    @tparam Interface10 interface type
    @tparam Interface11 interface type
    @tparam Interface12 interface type
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
inline css::uno::Any SAL_CALL queryInterface(
    const css::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4, Interface5 * p5,
    Interface6 * p6, Interface7 * p7, Interface8 * p8, Interface9 * p9, Interface10 * p10,
    Interface11 * p11, Interface12 * p12 )
{
    if (rType == Interface1::static_type())
        return css::uno::Any( &p1, rType );
    else if (rType == Interface2::static_type())
        return css::uno::Any( &p2, rType );
    else if (rType == Interface3::static_type())
        return css::uno::Any( &p3, rType );
    else if (rType == Interface4::static_type())
        return css::uno::Any( &p4, rType );
    else if (rType == Interface5::static_type())
        return css::uno::Any( &p5, rType );
    else if (rType == Interface6::static_type())
        return css::uno::Any( &p6, rType );
    else if (rType == Interface7::static_type())
        return css::uno::Any( &p7, rType );
    else if (rType == Interface8::static_type())
        return css::uno::Any( &p8, rType );
    else if (rType == Interface9::static_type())
        return css::uno::Any( &p9, rType );
    else if (rType == Interface10::static_type())
        return css::uno::Any( &p10, rType );
    else if (rType == Interface11::static_type())
        return css::uno::Any( &p11, rType );
    else if (rType == Interface12::static_type())
        return css::uno::Any( &p12, rType );
    else
        return css::uno::Any();
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
