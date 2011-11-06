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
