/*************************************************************************
 *
 *  $RCSfile: typeprovider.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-09 12:15:26 $
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

//==================================================================================================
class OTypeCollection
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > _aTypes;

public:
    // these are here to force memory de/allocation to sal lib.
    inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
        { return ::rtl_allocateMemory( nSize ); }
    inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
        { ::rtl_freeMemory( pMem ); }
    inline static void * SAL_CALL operator new( size_t, void * pMem ) SAL_THROW( () )
        { return pMem; }
    inline static void SAL_CALL operator delete( void *, void * ) SAL_THROW( () )
        {}

    OTypeCollection( const OTypeCollection & rCollection )
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

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() SAL_THROW( () )
        { return _aTypes; }
};

//==================================================================================================
class OImplementationId
{
    mutable ::com::sun::star::uno::Sequence< sal_Int8 > * _pSeq;
    sal_Bool _bUseEthernetAddress;

public:
    // these are here to force memory de/allocation to sal lib.
    inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
        { return ::rtl_allocateMemory( nSize ); }
    inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
        { ::rtl_freeMemory( pMem ); }
    inline static void * SAL_CALL operator new( size_t, void * pMem ) SAL_THROW( () )
        { return pMem; }
    inline static void SAL_CALL operator delete( void *, void * ) SAL_THROW( () )
        {}

    ~OImplementationId() SAL_THROW( () );
    OImplementationId( sal_Bool bUseEthernetAddress = sal_True ) SAL_THROW( () )
        : _pSeq( 0 )
        , _bUseEthernetAddress( bUseEthernetAddress )
        {}
    OImplementationId( const ::com::sun::star::uno::Sequence< sal_Int8 > & rSeq ) SAL_THROW( () )
        : _pSeq( new ::com::sun::star::uno::Sequence< sal_Int8 >( rSeq ) )
        {}
    OImplementationId( const OImplementationId & rId ) SAL_THROW( () )
        : _pSeq( new ::com::sun::star::uno::Sequence< sal_Int8 >( rId.getImplementationId() ) )
        {}

    ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() const SAL_THROW( () );
};

}

#endif
