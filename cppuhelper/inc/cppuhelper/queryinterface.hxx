/*************************************************************************
 *
 *  $RCSfile: queryinterface.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 15:39:16 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#define _CPPUHELPER_QUERYINTERFACE_HXX_

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include "com/sun/star/uno/Any.hxx"
#endif
#ifndef _COM_SUN_STAR_UNO_TYPE_HXX_
#include "com/sun/star/uno/Type.hxx"
#endif
#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

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

