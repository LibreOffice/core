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
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#define _CPPUHELPER_TYPEPROVIDER_HXX_

#include <rtl/alloc.h>
#include <rtl/uuid.h>
#include <com/sun/star/uno/Sequence.hxx>


namespace cppu
{

/** Helper class to implement ::com::sun::star::lang::XTypeProvider.  Construct a static object
    of this class with your UNO object's supported types.
*/
class OTypeCollection
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > _aTypes;

public:
    // these are here to force memory de/allocation to sal lib.
    /** @internal */
    inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
        { return ::rtl_allocateMemory( nSize ); }
    /** @internal */
    inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
        { ::rtl_freeMemory( pMem ); }
    /** @internal */
    inline static void * SAL_CALL operator new( size_t, void * pMem ) SAL_THROW( () )
        { return pMem; }
    /** @internal */
    inline static void SAL_CALL operator delete( void *, void * ) SAL_THROW( () )
        {}

    inline OTypeCollection( const OTypeCollection & rCollection )
        SAL_THROW( () )
        : _aTypes( rCollection._aTypes )
        {}
    OTypeCollection(
        const ::com::sun::star::uno::Type & rType1,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > & rAddTypes = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >() )
        SAL_THROW( () );
    OTypeCollection(
        const ::com::sun::star::uno::Type & rType1,
        const ::com::sun::star::uno::Type & rType2,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > & rAddTypes = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >() )
        SAL_THROW( () );
    OTypeCollection(
        const ::com::sun::star::uno::Type & rType1,
        const ::com::sun::star::uno::Type & rType2,
        const ::com::sun::star::uno::Type & rType3,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > & rAddTypes = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >() )
        SAL_THROW( () );
    OTypeCollection(
        const ::com::sun::star::uno::Type & rType1,
        const ::com::sun::star::uno::Type & rType2,
        const ::com::sun::star::uno::Type & rType3,
        const ::com::sun::star::uno::Type & rType4,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > & rAddTypes = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >() )
        SAL_THROW( () );
    OTypeCollection(
        const ::com::sun::star::uno::Type & rType1,
        const ::com::sun::star::uno::Type & rType2,
        const ::com::sun::star::uno::Type & rType3,
        const ::com::sun::star::uno::Type & rType4,
        const ::com::sun::star::uno::Type & rType5,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > & rAddTypes = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >() )
        SAL_THROW( () );
    OTypeCollection(
        const ::com::sun::star::uno::Type & rType1,
        const ::com::sun::star::uno::Type & rType2,
        const ::com::sun::star::uno::Type & rType3,
        const ::com::sun::star::uno::Type & rType4,
        const ::com::sun::star::uno::Type & rType5,
        const ::com::sun::star::uno::Type & rType6,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > & rAddTypes = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >() )
        SAL_THROW( () );
    OTypeCollection(
        const ::com::sun::star::uno::Type & rType1,
        const ::com::sun::star::uno::Type & rType2,
        const ::com::sun::star::uno::Type & rType3,
        const ::com::sun::star::uno::Type & rType4,
        const ::com::sun::star::uno::Type & rType5,
        const ::com::sun::star::uno::Type & rType6,
        const ::com::sun::star::uno::Type & rType7,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > & rAddTypes = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >() )
        SAL_THROW( () );
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
        SAL_THROW( () );
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
        SAL_THROW( () );
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
        SAL_THROW( () );
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
        SAL_THROW( () );
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
        SAL_THROW( () );

    /** Called upon XTypeProvider::getTypes().

        @return type collection
    */
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() SAL_THROW( () )
        { return _aTypes; }
};

/** Helper class to implement ::com::sun::star::lang::XTypeProvider.  Construct a static object
    of this class for your UNO object's implementation id.
*/
class OImplementationId
{
    /** @internal */
    mutable ::com::sun::star::uno::Sequence< sal_Int8 > * _pSeq;
    /** @internal */
    sal_Bool _bUseEthernetAddress;

public:
    // these are here to force memory de/allocation to sal lib.
    /** @internal */
    inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
        { return ::rtl_allocateMemory( nSize ); }
    /** @internal */
    inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
        { ::rtl_freeMemory( pMem ); }
    /** @internal */
    inline static void * SAL_CALL operator new( size_t, void * pMem ) SAL_THROW( () )
        { return pMem; }
    /** @internal */
    inline static void SAL_CALL operator delete( void *, void * ) SAL_THROW( () )
        {}

    /** @internal */
    ~OImplementationId() SAL_THROW( () );
    /** Constructor.

        @param bUseEthernetAddress whether an ethernet mac address should be taken into account
    */
    inline OImplementationId( sal_Bool bUseEthernetAddress = sal_True ) SAL_THROW( () )
        : _pSeq( 0 )
        , _bUseEthernetAddress( bUseEthernetAddress )
        {}
    /** Constructor giving implementation id.

        @param rSeq implementation id
    */
    inline OImplementationId( const ::com::sun::star::uno::Sequence< sal_Int8 > & rSeq ) SAL_THROW( () )
        : _pSeq( new ::com::sun::star::uno::Sequence< sal_Int8 >( rSeq ) )
        {}
    inline OImplementationId( const OImplementationId & rId ) SAL_THROW( () )
        : _pSeq( new ::com::sun::star::uno::Sequence< sal_Int8 >( rId.getImplementationId() ) )
        {}

    /** Called upon XTypeProvider::getImplementationId().

        @return implementation id
    */
    ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() const SAL_THROW( () );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
