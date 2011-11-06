/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _UNOTOOLS_QUERYDEEPINTERFACE_HXX
#define _UNOTOOLS_QUERYDEEPINTERFACE_HXX

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Type.hxx>

/** */ //for docpp
namespace utl
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
    return isDerivedFrom(
                rBaseType,
                ::getCppuType(static_cast<const ::com::sun::star::uno::Reference<Interface> *>(0)));
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
    if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface1 > *>(0))))
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
    if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface1 > *>(0))))
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface2 > *>(0))))
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
    if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface1 > *>(0))))
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface2 > *>(0))))
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface3 > *>(0))))
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
    if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface1 > *>(0))))
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface2 > *>(0))))
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface3 > *>(0))))
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface4 > *>(0))))
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
    if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface1 > *>(0))))
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface2 > *>(0))))
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface3 > *>(0))))
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface4 > *>(0))))
        return ::com::sun::star::uno::Any( &p4, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface5 > *>(0))))
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
    if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface1 > *>(0))))
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface2 > *>(0))))
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface3 > *>(0))))
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface4 > *>(0))))
        return ::com::sun::star::uno::Any( &p4, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface5 > *>(0))))
        return ::com::sun::star::uno::Any( &p5, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface6 > *>(0))))
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
    if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface1 > *>(0))))
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface2 > *>(0))))
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface3 > *>(0))))
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface4 > *>(0))))
        return ::com::sun::star::uno::Any( &p4, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface5 > *>(0))))
        return ::com::sun::star::uno::Any( &p5, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface6 > *>(0))))
        return ::com::sun::star::uno::Any( &p6, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface7 > *>(0))))
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
    if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface1 > *>(0))))
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface2 > *>(0))))
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface3 > *>(0))))
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface4 > *>(0))))
        return ::com::sun::star::uno::Any( &p4, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface5 > *>(0))))
        return ::com::sun::star::uno::Any( &p5, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface6 > *>(0))))
        return ::com::sun::star::uno::Any( &p6, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface7 > *>(0))))
        return ::com::sun::star::uno::Any( &p7, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface8 > *>(0))))
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
    if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface1 > *>(0))))
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface2 > *>(0))))
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface3 > *>(0))))
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface4 > *>(0))))
        return ::com::sun::star::uno::Any( &p4, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface5 > *>(0))))
        return ::com::sun::star::uno::Any( &p5, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface6 > *>(0))))
        return ::com::sun::star::uno::Any( &p6, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface7 > *>(0))))
        return ::com::sun::star::uno::Any( &p7, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface8 > *>(0))))
        return ::com::sun::star::uno::Any( &p8, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface9 > *>(0))))
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
    if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface1 > *>(0))))
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface2 > *>(0))))
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface3 > *>(0))))
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface4 > *>(0))))
        return ::com::sun::star::uno::Any( &p4, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface5 > *>(0))))
        return ::com::sun::star::uno::Any( &p5, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface6 > *>(0))))
        return ::com::sun::star::uno::Any( &p6, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface7 > *>(0))))
        return ::com::sun::star::uno::Any( &p7, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface8 > *>(0))))
        return ::com::sun::star::uno::Any( &p8, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface9 > *>(0))))
        return ::com::sun::star::uno::Any( &p9, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface10 > *>(0))))
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
    if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface1 > *>(0))))
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface2 > *>(0))))
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface3 > *>(0))))
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface4 > *>(0))))
        return ::com::sun::star::uno::Any( &p4, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface5 > *>(0))))
        return ::com::sun::star::uno::Any( &p5, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface6 > *>(0))))
        return ::com::sun::star::uno::Any( &p6, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface7 > *>(0))))
        return ::com::sun::star::uno::Any( &p7, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface8 > *>(0))))
        return ::com::sun::star::uno::Any( &p8, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface9 > *>(0))))
        return ::com::sun::star::uno::Any( &p9, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface10 > *>(0))))
        return ::com::sun::star::uno::Any( &p10, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface11 > *>(0))))
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
    if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface1 > *>(0))))
        return ::com::sun::star::uno::Any( &p1, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface2 > *>(0))))
        return ::com::sun::star::uno::Any( &p2, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface3 > *>(0))))
        return ::com::sun::star::uno::Any( &p3, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface4 > *>(0))))
        return ::com::sun::star::uno::Any( &p4, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface5 > *>(0))))
        return ::com::sun::star::uno::Any( &p5, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface6 > *>(0))))
        return ::com::sun::star::uno::Any( &p6, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface7 > *>(0))))
        return ::com::sun::star::uno::Any( &p7, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface8 > *>(0))))
        return ::com::sun::star::uno::Any( &p8, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface9 > *>(0))))
        return ::com::sun::star::uno::Any( &p9, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface10 > *>(0))))
        return ::com::sun::star::uno::Any( &p10, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface11 > *>(0))))
        return ::com::sun::star::uno::Any( &p11, rType );
    else if (isDerivedFrom(rType, ::getCppuType(static_cast<const ::com::sun::star::uno::Reference< Interface12 > *>(0))))
        return ::com::sun::star::uno::Any( &p12, rType );
    else
        return ::com::sun::star::uno::Any();
}

} // namespace utl

#endif // _UNOTOOLS_QUERYDEEPINTERFACE_HXX

