/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: querydeep.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _COMPHELPER_QUERYDEEPINTERFACE_HXX
#define _COMPHELPER_QUERYDEEPINTERFACE_HXX

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Type.hxx>

/** */ //for docpp
namespace comphelper
{

//--------------------------------------------------------------------------------------------------------
/**
 * Inspect interfaces types whether they are related by inheritance.
 *<BR>
 * @return      true if rType is derived from rBaseType
 * @param       rBaseType   a <type>Type</type> of an interface.
 * @param       rType       a <type>Type</type> of an interface.
 */
sal_Bool isDerivedFrom(
    const ::com::sun::star::uno::Type & rBaseType,
    const ::com::sun::star::uno::Type & rType );

//--------------------------------------------------------------------------------------------------------
/**
 * Inspect interface types whether they are related by inheritance.
 *<BR>
 * @return      true if p is of a type derived from rBaseType
 * @param       rBaseType   a <type>Type</type> of an interface.
 * @param       p           a pointer to an interface.
 */
template <class Interface>
inline sal_Bool isDerivedFrom(
    const ::com::sun::star::uno::Type& rBaseType,
    Interface* /*p*/)
{
    return isDerivedFrom(rBaseType, Interface::static_type());
}

//--------------------------------------------------------------------------------------------------------
// possible optimization ?
//  Any aRet(::cppu::queryInterface(rType, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12));
//  if (aRet.hasValue())
//      return aRet;

/**
 * Inspect types and choose return proper interface.
 *<BR>
 * @param       p1      a pointer to an interface.
 */
template< class Interface1 >
inline ::com::sun::star::uno::Any queryDeepInterface(
    const ::com::sun::star::uno::Type & rType,
    Interface1 * p1 )
{
    if (isDerivedFrom(rType, Interface1::static_type()))
        return ::com::sun::star::uno::Any( &p1, rType );
    else
        return ::com::sun::star::uno::Any();
}

/**
 * Inspect types and choose return proper interface.
 *<BR>
 * @param       p1      a pointer to an interface.
 * @param       p2      a pointer to an interface.
 */
template< class Interface1, class Interface2 >
inline ::com::sun::star::uno::Any queryDeepInterface(
    const ::com::sun::star::uno::Type & rType,
    Interface1 * p1, Interface2 * p2 )
{
    if (isDerivedFrom(rType, Interface1::static_type()))
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (isDerivedFrom(rType, Interface2::static_type()))
        return ::com::sun::star::uno::Any( &p2, rType );
    else
        return ::com::sun::star::uno::Any();
}

/**
 * Inspect types and choose return proper interface.
 *<BR>
 * @param       p1      a pointer to an interface.
 * @param       p2      a pointer to an interface.
 * @param       p3      a pointer to an interface.
 */
template< class Interface1, class Interface2, class Interface3 >
inline ::com::sun::star::uno::Any queryDeepInterface(
    const ::com::sun::star::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3 )
{
    if (isDerivedFrom(rType, Interface1::static_type()))
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (isDerivedFrom(rType, Interface2::static_type()))
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (isDerivedFrom(rType, Interface3::static_type()))
        return ::com::sun::star::uno::Any( &p3, rType );
    else
        return ::com::sun::star::uno::Any();
}

/**
 * Inspect types and choose return proper interface.
 *<BR>
 * @param       p1      a pointer to an interface.
 * @param       p2      a pointer to an interface.
 * @param       p3      a pointer to an interface.
 * @param       p4      a pointer to an interface.
 */
template< class Interface1, class Interface2, class Interface3, class Interface4 >
inline ::com::sun::star::uno::Any queryDeepInterface(
    const ::com::sun::star::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4 )
{
    if (isDerivedFrom(rType, Interface1::static_type()))
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (isDerivedFrom(rType, Interface2::static_type()))
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (isDerivedFrom(rType, Interface3::static_type()))
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (isDerivedFrom(rType, Interface4::static_type()))
        return ::com::sun::star::uno::Any( &p4, rType );
    else
        return ::com::sun::star::uno::Any();
}

/**
 * Inspect types and choose return proper interface.
 *<BR>
 * @param       p1      a pointer to an interface.
 * @param       p2      a pointer to an interface.
 * @param       p3      a pointer to an interface.
 * @param       p4      a pointer to an interface.
 * @param       p5      a pointer to an interface.
 */
template< class Interface1, class Interface2, class Interface3, class Interface4, class Interface5 >
inline ::com::sun::star::uno::Any queryDeepInterface(
    const ::com::sun::star::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4, Interface5 * p5 )
{
    if (isDerivedFrom(rType, Interface1::static_type()))
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (isDerivedFrom(rType, Interface2::static_type()))
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (isDerivedFrom(rType, Interface3::static_type()))
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (isDerivedFrom(rType, Interface4::static_type()))
        return ::com::sun::star::uno::Any( &p4, rType );
    else if (isDerivedFrom(rType, Interface5::static_type()))
        return ::com::sun::star::uno::Any( &p5, rType );
    else
        return ::com::sun::star::uno::Any();
}

/**
 * Inspect types and choose return proper interface.
 *<BR>
 * @param       p1      a pointer to an interface.
 * @param       p2      a pointer to an interface.
 * @param       p3      a pointer to an interface.
 * @param       p4      a pointer to an interface.
 * @param       p5      a pointer to an interface.
 * @param       p6      a pointer to an interface.
 */
template< class Interface1, class Interface2, class Interface3, class Interface4, class Interface5,
          class Interface6 >
inline ::com::sun::star::uno::Any queryDeepInterface(
    const ::com::sun::star::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4, Interface5 * p5,
    Interface6 * p6 )
{
    if (isDerivedFrom(rType, Interface1::static_type()))
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (isDerivedFrom(rType, Interface2::static_type()))
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (isDerivedFrom(rType, Interface3::static_type()))
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (isDerivedFrom(rType, Interface4::static_type()))
        return ::com::sun::star::uno::Any( &p4, rType );
    else if (isDerivedFrom(rType, Interface5::static_type()))
        return ::com::sun::star::uno::Any( &p5, rType );
    else if (isDerivedFrom(rType, Interface6::static_type()))
        return ::com::sun::star::uno::Any( &p6, rType );
    else
        return ::com::sun::star::uno::Any();
}

/**
 * Inspect types and choose return proper interface.
 *<BR>
 * @param       p1      a pointer to an interface.
 * @param       p2      a pointer to an interface.
 * @param       p3      a pointer to an interface.
 * @param       p4      a pointer to an interface.
 * @param       p5      a pointer to an interface.
 * @param       p6      a pointer to an interface.
 * @param       p7      a pointer to an interface.
 */
template< class Interface1, class Interface2, class Interface3, class Interface4, class Interface5,
          class Interface6, class Interface7 >
inline ::com::sun::star::uno::Any queryDeepInterface(
    const ::com::sun::star::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4, Interface5 * p5,
    Interface6 * p6, Interface7 * p7 )
{
    if (isDerivedFrom(rType, Interface1::static_type()))
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (isDerivedFrom(rType, Interface2::static_type()))
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (isDerivedFrom(rType, Interface3::static_type()))
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (isDerivedFrom(rType, Interface4::static_type()))
        return ::com::sun::star::uno::Any( &p4, rType );
    else if (isDerivedFrom(rType, Interface5::static_type()))
        return ::com::sun::star::uno::Any( &p5, rType );
    else if (isDerivedFrom(rType, Interface6::static_type()))
        return ::com::sun::star::uno::Any( &p6, rType );
    else if (isDerivedFrom(rType, Interface7::static_type()))
        return ::com::sun::star::uno::Any( &p7, rType );
    else
        return ::com::sun::star::uno::Any();
}

/**
 * Inspect types and choose return proper interface.
 *<BR>
 * @param       p1      a pointer to an interface.
 * @param       p2      a pointer to an interface.
 * @param       p3      a pointer to an interface.
 * @param       p4      a pointer to an interface.
 * @param       p5      a pointer to an interface.
 * @param       p6      a pointer to an interface.
 * @param       p7      a pointer to an interface.
 * @param       p8      a pointer to an interface.
 */
template< class Interface1, class Interface2, class Interface3, class Interface4, class Interface5,
          class Interface6, class Interface7, class Interface8 >
inline ::com::sun::star::uno::Any queryDeepInterface(
    const ::com::sun::star::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4, Interface5 * p5,
    Interface6 * p6, Interface7 * p7, Interface8 * p8 )
{
    if (isDerivedFrom(rType, Interface1::static_type()))
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (isDerivedFrom(rType, Interface2::static_type()))
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (isDerivedFrom(rType, Interface3::static_type()))
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (isDerivedFrom(rType, Interface4::static_type()))
        return ::com::sun::star::uno::Any( &p4, rType );
    else if (isDerivedFrom(rType, Interface5::static_type()))
        return ::com::sun::star::uno::Any( &p5, rType );
    else if (isDerivedFrom(rType, Interface6::static_type()))
        return ::com::sun::star::uno::Any( &p6, rType );
    else if (isDerivedFrom(rType, Interface7::static_type()))
        return ::com::sun::star::uno::Any( &p7, rType );
    else if (isDerivedFrom(rType, Interface8::static_type()))
        return ::com::sun::star::uno::Any( &p8, rType );
    else
        return ::com::sun::star::uno::Any();
}

/**
 * Inspect types and choose return proper interface.
 *<BR>
 * @param       p1      a pointer to an interface.
 * @param       p2      a pointer to an interface.
 * @param       p3      a pointer to an interface.
 * @param       p4      a pointer to an interface.
 * @param       p5      a pointer to an interface.
 * @param       p6      a pointer to an interface.
 * @param       p7      a pointer to an interface.
 * @param       p8      a pointer to an interface.
 * @param       p9      a pointer to an interface.
 */
template< class Interface1, class Interface2, class Interface3, class Interface4, class Interface5,
          class Interface6, class Interface7, class Interface8, class Interface9 >
inline ::com::sun::star::uno::Any queryDeepInterface(
    const ::com::sun::star::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4, Interface5 * p5,
    Interface6 * p6, Interface7 * p7, Interface8 * p8, Interface9 * p9 )
{
    if (isDerivedFrom(rType, Interface1::static_type()))
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (isDerivedFrom(rType, Interface2::static_type()))
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (isDerivedFrom(rType, Interface3::static_type()))
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (isDerivedFrom(rType, Interface4::static_type()))
        return ::com::sun::star::uno::Any( &p4, rType );
    else if (isDerivedFrom(rType, Interface5::static_type()))
        return ::com::sun::star::uno::Any( &p5, rType );
    else if (isDerivedFrom(rType, Interface6::static_type()))
        return ::com::sun::star::uno::Any( &p6, rType );
    else if (isDerivedFrom(rType, Interface7::static_type()))
        return ::com::sun::star::uno::Any( &p7, rType );
    else if (isDerivedFrom(rType, Interface8::static_type()))
        return ::com::sun::star::uno::Any( &p8, rType );
    else if (isDerivedFrom(rType, Interface9::static_type()))
        return ::com::sun::star::uno::Any( &p9, rType );
    else
        return ::com::sun::star::uno::Any();
}

/**
 * Inspect types and choose return proper interface.
 *<BR>
 * @param       p1      a pointer to an interface.
 * @param       p2      a pointer to an interface.
 * @param       p3      a pointer to an interface.
 * @param       p4      a pointer to an interface.
 * @param       p5      a pointer to an interface.
 * @param       p6      a pointer to an interface.
 * @param       p7      a pointer to an interface.
 * @param       p8      a pointer to an interface.
 * @param       p9      a pointer to an interface.
 * @param       p10     a pointer to an interface.
 */
template< class Interface1, class Interface2, class Interface3, class Interface4, class Interface5,
          class Interface6, class Interface7, class Interface8, class Interface9, class Interface10 >
inline ::com::sun::star::uno::Any queryDeepInterface(
    const ::com::sun::star::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4, Interface5 * p5,
    Interface6 * p6, Interface7 * p7, Interface8 * p8, Interface9 * p9, Interface10 * p10 )
{
    if (isDerivedFrom(rType, Interface1::static_type()))
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (isDerivedFrom(rType, Interface2::static_type()))
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (isDerivedFrom(rType, Interface3::static_type()))
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (isDerivedFrom(rType, Interface4::static_type()))
        return ::com::sun::star::uno::Any( &p4, rType );
    else if (isDerivedFrom(rType, Interface5::static_type()))
        return ::com::sun::star::uno::Any( &p5, rType );
    else if (isDerivedFrom(rType, Interface6::static_type()))
        return ::com::sun::star::uno::Any( &p6, rType );
    else if (isDerivedFrom(rType, Interface7::static_type()))
        return ::com::sun::star::uno::Any( &p7, rType );
    else if (isDerivedFrom(rType, Interface8::static_type()))
        return ::com::sun::star::uno::Any( &p8, rType );
    else if (isDerivedFrom(rType, Interface9::static_type()))
        return ::com::sun::star::uno::Any( &p9, rType );
    else if (isDerivedFrom(rType, Interface10::static_type()))
        return ::com::sun::star::uno::Any( &p10, rType );
    else
        return ::com::sun::star::uno::Any();
}

/**
 * Inspect types and choose return proper interface.
 *<BR>
 * @param       p1      a pointer to an interface.
 * @param       p2      a pointer to an interface.
 * @param       p3      a pointer to an interface.
 * @param       p4      a pointer to an interface.
 * @param       p5      a pointer to an interface.
 * @param       p6      a pointer to an interface.
 * @param       p7      a pointer to an interface.
 * @param       p8      a pointer to an interface.
 * @param       p9      a pointer to an interface.
 * @param       p10     a pointer to an interface.
 * @param       p11     a pointer to an interface.
 */
template< class Interface1, class Interface2, class Interface3, class Interface4, class Interface5,
          class Interface6, class Interface7, class Interface8, class Interface9, class Interface10,
          class Interface11 >
inline ::com::sun::star::uno::Any queryDeepInterface(
    const ::com::sun::star::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4, Interface5 * p5,
    Interface6 * p6, Interface7 * p7, Interface8 * p8, Interface9 * p9, Interface10 * p10,
    Interface11 * p11 )
{
    if (isDerivedFrom(rType, Interface1::static_type()))
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (isDerivedFrom(rType, Interface2::static_type()))
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (isDerivedFrom(rType, Interface3::static_type()))
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (isDerivedFrom(rType, Interface4::static_type()))
        return ::com::sun::star::uno::Any( &p4, rType );
    else if (isDerivedFrom(rType, Interface5::static_type()))
        return ::com::sun::star::uno::Any( &p5, rType );
    else if (isDerivedFrom(rType, Interface6::static_type()))
        return ::com::sun::star::uno::Any( &p6, rType );
    else if (isDerivedFrom(rType, Interface7::static_type()))
        return ::com::sun::star::uno::Any( &p7, rType );
    else if (isDerivedFrom(rType, Interface8::static_type()))
        return ::com::sun::star::uno::Any( &p8, rType );
    else if (isDerivedFrom(rType, Interface9::static_type()))
        return ::com::sun::star::uno::Any( &p9, rType );
    else if (isDerivedFrom(rType, Interface10::static_type()))
        return ::com::sun::star::uno::Any( &p10, rType );
    else if (isDerivedFrom(rType, Interface11::static_type()))
        return ::com::sun::star::uno::Any( &p11, rType );
    else
        return ::com::sun::star::uno::Any();
}

/**
 * Inspect types and choose return proper interface.
 *<BR>
 * @param       p1      a pointer to an interface.
 * @param       p2      a pointer to an interface.
 * @param       p3      a pointer to an interface.
 * @param       p4      a pointer to an interface.
 * @param       p5      a pointer to an interface.
 * @param       p6      a pointer to an interface.
 * @param       p7      a pointer to an interface.
 * @param       p8      a pointer to an interface.
 * @param       p9      a pointer to an interface.
 * @param       p10     a pointer to an interface.
 * @param       p11     a pointer to an interface.
 * @param       p12     a pointer to an interface.
 */
template< class Interface1, class Interface2, class Interface3, class Interface4, class Interface5,
          class Interface6, class Interface7, class Interface8, class Interface9, class Interface10,
          class Interface11, class Interface12 >
inline ::com::sun::star::uno::Any queryDeepInterface(
    const ::com::sun::star::uno::Type & rType,
    Interface1 * p1, Interface2 * p2, Interface3 * p3, Interface4 * p4, Interface5 * p5,
    Interface6 * p6, Interface7 * p7, Interface8 * p8, Interface9 * p9, Interface10 * p10,
    Interface11 * p11, Interface12 * p12 )
{
    if (isDerivedFrom(rType, Interface1::static_type()))
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (isDerivedFrom(rType, Interface2::static_type()))
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (isDerivedFrom(rType, Interface3::static_type()))
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (isDerivedFrom(rType, Interface4::static_type()))
        return ::com::sun::star::uno::Any( &p4, rType );
    else if (isDerivedFrom(rType, Interface5::static_type()))
        return ::com::sun::star::uno::Any( &p5, rType );
    else if (isDerivedFrom(rType, Interface6::static_type()))
        return ::com::sun::star::uno::Any( &p6, rType );
    else if (isDerivedFrom(rType, Interface7::static_type()))
        return ::com::sun::star::uno::Any( &p7, rType );
    else if (isDerivedFrom(rType, Interface8::static_type()))
        return ::com::sun::star::uno::Any( &p8, rType );
    else if (isDerivedFrom(rType, Interface9::static_type()))
        return ::com::sun::star::uno::Any( &p9, rType );
    else if (isDerivedFrom(rType, Interface10::static_type()))
        return ::com::sun::star::uno::Any( &p10, rType );
    else if (isDerivedFrom(rType, Interface11::static_type()))
        return ::com::sun::star::uno::Any( &p11, rType );
    else if (isDerivedFrom(rType, Interface12::static_type()))
        return ::com::sun::star::uno::Any( &p12, rType );
    else
        return ::com::sun::star::uno::Any();
}

} // namespace comphelper

#endif // _COMPHELPER_QUERYDEEPINTERFACE_HXX

