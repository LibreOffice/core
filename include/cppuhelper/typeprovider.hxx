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
#ifndef INCLUDED_CPPUHELPER_TYPEPROVIDER_HXX
#define INCLUDED_CPPUHELPER_TYPEPROVIDER_HXX

#include "sal/config.h"

#include <cstddef>

#include "rtl/alloc.h"
#include "com/sun/star/uno/Sequence.hxx"
#include "cppuhelper/cppuhelperdllapi.h"


namespace cppu
{

/** Helper class to implement css::lang::XTypeProvider.  Construct a static object
    of this class with your UNO object's supported types.
*/
class SAL_WARN_UNUSED CPPUHELPER_DLLPUBLIC OTypeCollection
{
    css::uno::Sequence< css::uno::Type > _aTypes;

public:
    /// @cond INTERNAL
    // these are here to force memory de/allocation to sal lib.
    static void * SAL_CALL operator new( size_t nSize )
        { return ::rtl_allocateMemory( nSize ); }
    static void SAL_CALL operator delete( void * pMem )
        { ::rtl_freeMemory( pMem ); }
    static void * SAL_CALL operator new( size_t, void * pMem )
        { return pMem; }
    static void SAL_CALL operator delete( void *, void * )
        {}
    /// @endcond

    OTypeCollection(
        const css::uno::Type & rType1,
        const css::uno::Sequence< css::uno::Type > & rAddTypes = css::uno::Sequence< css::uno::Type >() );
    OTypeCollection(
        const css::uno::Type & rType1,
        const css::uno::Type & rType2,
        const css::uno::Sequence< css::uno::Type > & rAddTypes = css::uno::Sequence< css::uno::Type >() );
    OTypeCollection(
        const css::uno::Type & rType1,
        const css::uno::Type & rType2,
        const css::uno::Type & rType3,
        const css::uno::Sequence< css::uno::Type > & rAddTypes = css::uno::Sequence< css::uno::Type >() );
    OTypeCollection(
        const css::uno::Type & rType1,
        const css::uno::Type & rType2,
        const css::uno::Type & rType3,
        const css::uno::Type & rType4,
        const css::uno::Sequence< css::uno::Type > & rAddTypes = css::uno::Sequence< css::uno::Type >() );
    OTypeCollection(
        const css::uno::Type & rType1,
        const css::uno::Type & rType2,
        const css::uno::Type & rType3,
        const css::uno::Type & rType4,
        const css::uno::Type & rType5,
        const css::uno::Sequence< css::uno::Type > & rAddTypes = css::uno::Sequence< css::uno::Type >() );
    OTypeCollection(
        const css::uno::Type & rType1,
        const css::uno::Type & rType2,
        const css::uno::Type & rType3,
        const css::uno::Type & rType4,
        const css::uno::Type & rType5,
        const css::uno::Type & rType6,
        const css::uno::Sequence< css::uno::Type > & rAddTypes = css::uno::Sequence< css::uno::Type >() );
    OTypeCollection(
        const css::uno::Type & rType1,
        const css::uno::Type & rType2,
        const css::uno::Type & rType3,
        const css::uno::Type & rType4,
        const css::uno::Type & rType5,
        const css::uno::Type & rType6,
        const css::uno::Type & rType7,
        const css::uno::Sequence< css::uno::Type > & rAddTypes = css::uno::Sequence< css::uno::Type >() );
    OTypeCollection(
        const css::uno::Type & rType1,
        const css::uno::Type & rType2,
        const css::uno::Type & rType3,
        const css::uno::Type & rType4,
        const css::uno::Type & rType5,
        const css::uno::Type & rType6,
        const css::uno::Type & rType7,
        const css::uno::Type & rType8,
        const css::uno::Sequence< css::uno::Type > & rAddTypes = css::uno::Sequence< css::uno::Type >() );
    OTypeCollection(
        const css::uno::Type & rType1,
        const css::uno::Type & rType2,
        const css::uno::Type & rType3,
        const css::uno::Type & rType4,
        const css::uno::Type & rType5,
        const css::uno::Type & rType6,
        const css::uno::Type & rType7,
        const css::uno::Type & rType8,
        const css::uno::Type & rType9,
        const css::uno::Sequence< css::uno::Type > & rAddTypes = css::uno::Sequence< css::uno::Type >() );
    OTypeCollection(
        const css::uno::Type & rType1,
        const css::uno::Type & rType2,
        const css::uno::Type & rType3,
        const css::uno::Type & rType4,
        const css::uno::Type & rType5,
        const css::uno::Type & rType6,
        const css::uno::Type & rType7,
        const css::uno::Type & rType8,
        const css::uno::Type & rType9,
        const css::uno::Type & rType10,
        const css::uno::Sequence< css::uno::Type > & rAddTypes = css::uno::Sequence< css::uno::Type >() );
    OTypeCollection(
        const css::uno::Type & rType1,
        const css::uno::Type & rType2,
        const css::uno::Type & rType3,
        const css::uno::Type & rType4,
        const css::uno::Type & rType5,
        const css::uno::Type & rType6,
        const css::uno::Type & rType7,
        const css::uno::Type & rType8,
        const css::uno::Type & rType9,
        const css::uno::Type & rType10,
        const css::uno::Type & rType11,
        const css::uno::Sequence< css::uno::Type > & rAddTypes = css::uno::Sequence< css::uno::Type >() );
    OTypeCollection(
        const css::uno::Type & rType1,
        const css::uno::Type & rType2,
        const css::uno::Type & rType3,
        const css::uno::Type & rType4,
        const css::uno::Type & rType5,
        const css::uno::Type & rType6,
        const css::uno::Type & rType7,
        const css::uno::Type & rType8,
        const css::uno::Type & rType9,
        const css::uno::Type & rType10,
        const css::uno::Type & rType11,
        const css::uno::Type & rType12,
        const css::uno::Sequence< css::uno::Type > & rAddTypes = css::uno::Sequence< css::uno::Type >() );

    /** Called upon XTypeProvider::getTypes().

        @return type collection
    */
    css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
        { return _aTypes; }
};

/** Helper class to implement IDs for XUnoTunnel.  Construct a static object
    of this class for your UNO object's implementation id.
*/
class SAL_WARN_UNUSED CPPUHELPER_DLLPUBLIC OImplementationId
{
    mutable css::uno::Sequence< sal_Int8 > * _pSeq;
    sal_Bool _bUseEthernetAddress;

public:
    /// @cond INTERNAL

    // these are here to force memory de/allocation to sal lib.
    static void * SAL_CALL operator new( size_t nSize )
        { return ::rtl_allocateMemory( nSize ); }
    static void SAL_CALL operator delete( void * pMem )
        { ::rtl_freeMemory( pMem ); }
    static void * SAL_CALL operator new( size_t, void * pMem )
        { return pMem; }
    static void SAL_CALL operator delete( void *, void * )
        {}

    ~OImplementationId();

    /// @endcond

    /** Constructor.

        @param bUseEthernetAddress whether an ethernet mac address should be taken into account
    */
    OImplementationId( bool bUseEthernetAddress = true )
        : _pSeq( NULL )
        , _bUseEthernetAddress( bUseEthernetAddress )
        {}
    /** Constructor giving implementation id.

        @param rSeq implementation id
    */
    OImplementationId( const css::uno::Sequence< sal_Int8 > & rSeq )
        : _pSeq( new css::uno::Sequence< sal_Int8 >( rSeq ) )
        , _bUseEthernetAddress( false )
        {}
    OImplementationId( const OImplementationId & rId )
        : _pSeq( new css::uno::Sequence< sal_Int8 >( rId.getImplementationId() ) )
        , _bUseEthernetAddress( false )
        {}

    /** Get implementation id.

        @return implementation id
    */
    css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
