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

#include <rtl/alloc.h>
#include <rtl/uuid.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/cppuhelperdllapi.h>


namespace cppu
{

/** Helper class to implement com::sun::star::lang::XTypeProvider.  Construct a static object
    of this class with your UNO object's supported types.
*/
class CPPUHELPER_DLLPUBLIC OTypeCollection
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > _aTypes;

public:
    /// @cond INTERNAL
    // these are here to force memory de/allocation to sal lib.
    inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW(())
        { return ::rtl_allocateMemory( nSize ); }
    inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW(())
        { ::rtl_freeMemory( pMem ); }
    inline static void * SAL_CALL operator new( size_t, void * pMem ) SAL_THROW(())
        { return pMem; }
    inline static void SAL_CALL operator delete( void *, void * ) SAL_THROW(())
        {}
    /// @endcond

    inline OTypeCollection( const OTypeCollection & rCollection )
        SAL_THROW(())
        : _aTypes( rCollection._aTypes )
        {}
    OTypeCollection(
        const ::com::sun::star::uno::Type & rType1,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > & rAddTypes = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >() )
        SAL_THROW(());
    OTypeCollection(
        const ::com::sun::star::uno::Type & rType1,
        const ::com::sun::star::uno::Type & rType2,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > & rAddTypes = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >() )
        SAL_THROW(());
    OTypeCollection(
        const ::com::sun::star::uno::Type & rType1,
        const ::com::sun::star::uno::Type & rType2,
        const ::com::sun::star::uno::Type & rType3,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > & rAddTypes = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >() )
        SAL_THROW(());
    OTypeCollection(
        const ::com::sun::star::uno::Type & rType1,
        const ::com::sun::star::uno::Type & rType2,
        const ::com::sun::star::uno::Type & rType3,
        const ::com::sun::star::uno::Type & rType4,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > & rAddTypes = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >() )
        SAL_THROW(());
    OTypeCollection(
        const ::com::sun::star::uno::Type & rType1,
        const ::com::sun::star::uno::Type & rType2,
        const ::com::sun::star::uno::Type & rType3,
        const ::com::sun::star::uno::Type & rType4,
        const ::com::sun::star::uno::Type & rType5,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > & rAddTypes = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >() )
        SAL_THROW(());
    OTypeCollection(
        const ::com::sun::star::uno::Type & rType1,
        const ::com::sun::star::uno::Type & rType2,
        const ::com::sun::star::uno::Type & rType3,
        const ::com::sun::star::uno::Type & rType4,
        const ::com::sun::star::uno::Type & rType5,
        const ::com::sun::star::uno::Type & rType6,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > & rAddTypes = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >() )
        SAL_THROW(());
    OTypeCollection(
        const ::com::sun::star::uno::Type & rType1,
        const ::com::sun::star::uno::Type & rType2,
        const ::com::sun::star::uno::Type & rType3,
        const ::com::sun::star::uno::Type & rType4,
        const ::com::sun::star::uno::Type & rType5,
        const ::com::sun::star::uno::Type & rType6,
        const ::com::sun::star::uno::Type & rType7,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > & rAddTypes = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >() )
        SAL_THROW(());
    OTypeCollection(
        const ::com::sun::star::uno::Type & rType1,
        const ::com::sun::star::uno::Type & rType2,
        const ::com::sun::star::uno::Type & rType3,
        const ::com::sun::star::uno::Type & rType4,
        const ::com::sun::star::uno::Type & rType5,
        const ::com::sun::star::uno::Type & rType6,
        const ::com::sun::star::uno::Type & rType7,
        const ::com::sun::star::uno::Type & rType8,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > & rAddTypes = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >() )
        SAL_THROW(());
    OTypeCollection(
        const ::com::sun::star::uno::Type & rType1,
        const ::com::sun::star::uno::Type & rType2,
        const ::com::sun::star::uno::Type & rType3,
        const ::com::sun::star::uno::Type & rType4,
        const ::com::sun::star::uno::Type & rType5,
        const ::com::sun::star::uno::Type & rType6,
        const ::com::sun::star::uno::Type & rType7,
        const ::com::sun::star::uno::Type & rType8,
        const ::com::sun::star::uno::Type & rType9,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > & rAddTypes = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >() )
        SAL_THROW(());
    OTypeCollection(
        const ::com::sun::star::uno::Type & rType1,
        const ::com::sun::star::uno::Type & rType2,
        const ::com::sun::star::uno::Type & rType3,
        const ::com::sun::star::uno::Type & rType4,
        const ::com::sun::star::uno::Type & rType5,
        const ::com::sun::star::uno::Type & rType6,
        const ::com::sun::star::uno::Type & rType7,
        const ::com::sun::star::uno::Type & rType8,
        const ::com::sun::star::uno::Type & rType9,
        const ::com::sun::star::uno::Type & rType10,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > & rAddTypes = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >() )
        SAL_THROW(());
    OTypeCollection(
        const ::com::sun::star::uno::Type & rType1,
        const ::com::sun::star::uno::Type & rType2,
        const ::com::sun::star::uno::Type & rType3,
        const ::com::sun::star::uno::Type & rType4,
        const ::com::sun::star::uno::Type & rType5,
        const ::com::sun::star::uno::Type & rType6,
        const ::com::sun::star::uno::Type & rType7,
        const ::com::sun::star::uno::Type & rType8,
        const ::com::sun::star::uno::Type & rType9,
        const ::com::sun::star::uno::Type & rType10,
        const ::com::sun::star::uno::Type & rType11,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > & rAddTypes = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >() )
        SAL_THROW(());
    OTypeCollection(
        const ::com::sun::star::uno::Type & rType1,
        const ::com::sun::star::uno::Type & rType2,
        const ::com::sun::star::uno::Type & rType3,
        const ::com::sun::star::uno::Type & rType4,
        const ::com::sun::star::uno::Type & rType5,
        const ::com::sun::star::uno::Type & rType6,
        const ::com::sun::star::uno::Type & rType7,
        const ::com::sun::star::uno::Type & rType8,
        const ::com::sun::star::uno::Type & rType9,
        const ::com::sun::star::uno::Type & rType10,
        const ::com::sun::star::uno::Type & rType11,
        const ::com::sun::star::uno::Type & rType12,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > & rAddTypes = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >() )
        SAL_THROW(());

    /** Called upon XTypeProvider::getTypes().

        @return type collection
    */
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() SAL_THROW(())
        { return _aTypes; }
};

/** Helper class to implement IDs for XUnoTunnel.  Construct a static object
    of this class for your UNO object's implementation id.
*/
class CPPUHELPER_DLLPUBLIC OImplementationId
{
    mutable ::com::sun::star::uno::Sequence< sal_Int8 > * _pSeq;
    sal_Bool _bUseEthernetAddress;

public:
    /// @cond INTERNAL

    // these are here to force memory de/allocation to sal lib.
    inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW(())
        { return ::rtl_allocateMemory( nSize ); }
    inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW(())
        { ::rtl_freeMemory( pMem ); }
    inline static void * SAL_CALL operator new( size_t, void * pMem ) SAL_THROW(())
        { return pMem; }
    inline static void SAL_CALL operator delete( void *, void * ) SAL_THROW(())
        {}

    ~OImplementationId() SAL_THROW(());

    /// @endcond

    /** Constructor.

        @param bUseEthernetAddress whether an ethernet mac address should be taken into account
    */
    inline OImplementationId( bool bUseEthernetAddress = true ) SAL_THROW(())
        : _pSeq( 0 )
        , _bUseEthernetAddress( bUseEthernetAddress )
        {}
    /** Constructor giving implementation id.

        @param rSeq implementation id
    */
    inline OImplementationId( const ::com::sun::star::uno::Sequence< sal_Int8 > & rSeq ) SAL_THROW(())
        : _pSeq( new ::com::sun::star::uno::Sequence< sal_Int8 >( rSeq ) )
        , _bUseEthernetAddress( false )
        {}
    inline OImplementationId( const OImplementationId & rId ) SAL_THROW(())
        : _pSeq( new ::com::sun::star::uno::Sequence< sal_Int8 >( rId.getImplementationId() ) )
        , _bUseEthernetAddress( false )
        {}

    /** Get implementation id.

        @return implementation id
    */
    ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() const SAL_THROW(());
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
