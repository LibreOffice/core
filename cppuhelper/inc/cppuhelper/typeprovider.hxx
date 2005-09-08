/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: typeprovider.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 09:22:03 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#define _CPPUHELPER_TYPEPROVIDER_HXX_

#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif


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
