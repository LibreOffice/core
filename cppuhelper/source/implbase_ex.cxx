/*************************************************************************
 *
 *  $RCSfile: implbase_ex.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 16:34:54 $
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

#ifdef SOLARIS
#include <alloca.h>
#elif defined MACOSX
#include <sys/types.h>
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif
#include <string.h>

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _RTL_BYTESEQ_HXX_
#include <rtl/byteseq.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif

#ifndef _CPPUHELPER_COMPBASE_EX_HXX_
#include <cppuhelper/compbase_ex.hxx>
#endif


using namespace ::cppu;
using namespace ::osl;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace cppu
{

/** Shared mutex for implementation helper initialization.
    Not for public use.
*/
::osl::Mutex & SAL_CALL getImplHelperInitMutex(void) SAL_THROW( () );

//--------------------------------------------------------------------------------------------------
static inline void checkInterface( Type const & rType )
    SAL_THROW( (RuntimeException) )
{
    if (TypeClass_INTERFACE != rType.getTypeClass())
    {
        OUStringBuffer buf( 64 );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("querying for interface \"") );
        buf.append( rType.getTypeName() );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\": no interface type!") );
        OUString msg( buf.makeStringAndClear() );
#if OSL_DEBUG_LEVEL > 0
        OString str( OUStringToOString( msg, RTL_TEXTENCODING_ASCII_US ) );
        OSL_ENSURE( 0, str.getStr() );
#endif
        throw RuntimeException( msg, Reference< XInterface >() );
    }
}
//--------------------------------------------------------------------------------------------------
static inline bool isXInterface( rtl_uString * pStr ) SAL_THROW( () )
{
    return (((OUString const *)&pStr)->equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.uno.XInterface") ) != sal_False);
}
//--------------------------------------------------------------------------------------------------
static inline void * makeInterface( sal_Int32 nOffset, void * that ) SAL_THROW( () )
{
    return (((char *)that) + nOffset);
}
//--------------------------------------------------------------------------------------------------
static inline bool __td_equals(
    typelib_TypeDescriptionReference * pTDR1, typelib_TypeDescriptionReference * pTDR2 )
    SAL_THROW( () )
{
    return ((pTDR1 == pTDR2) ||
            ((OUString const *)&pTDR1->pTypeName)->equals( *(OUString const *)&pTDR2->pTypeName ) != sal_False);
}
//--------------------------------------------------------------------------------------------------
static inline type_entry * __getTypeEntries( class_data * cd )
    SAL_THROW( (RuntimeException) )
{
    type_entry * pEntries = cd->m_typeEntries;
    if (! cd->m_storedTypeRefs) // not inited?
    {
        MutexGuard guard( getImplHelperInitMutex() );
        if (! cd->m_storedTypeRefs) // not inited?
        {
            // get all types
            for ( sal_Int32 n = cd->m_nTypes; n--; )
            {
                type_entry * pEntry = &pEntries[ n ];
                Type const & rType = (*pEntry->m_type.getCppuType)( 0 );
                OSL_ENSURE( rType.getTypeClass() == TypeClass_INTERFACE, "### wrong helper init: expected interface!" );
                OSL_ENSURE( ! isXInterface( rType.getTypeLibType()->pTypeName ), "### want to implement XInterface: template argument is XInterface?!?!?!" );
                if (rType.getTypeClass() != TypeClass_INTERFACE)
                {
                    OUStringBuffer buf( 48 );
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("type \"") );
                    buf.append( rType.getTypeName() );
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\" is no interface type!") );
                    OUString msg( buf.makeStringAndClear() );
#if OSL_DEBUG_LEVEL > 0
                    OString str( OUStringToOString( msg, RTL_TEXTENCODING_ASCII_US ) );
                    OSL_ENSURE( 0, str.getStr() );
#endif
                    throw RuntimeException( msg, Reference< XInterface >() );
                }
                // ref is statically held by getCppuType()
                pEntry->m_type.typeRef = rType.getTypeLibType();
            }
            cd->m_storedTypeRefs = sal_True;
        }
    }
    return pEntries;
}
//--------------------------------------------------------------------------------------------------
static inline void __fillTypes( Type * types, class_data * cd )
    SAL_THROW( (RuntimeException) )
{
    type_entry * pEntries = __getTypeEntries( cd );
    for ( sal_Int32 n = cd->m_nTypes; n--; )
    {
        types[ n ] = pEntries[ n ].m_type.typeRef;
    }
}
//--------------------------------------------------------------------------------------------------
static inline void * __queryDeepNoXInterface(
    typelib_TypeDescriptionReference * pDemandedTDR, class_data * cd, void * that )
    SAL_THROW( (RuntimeException) )
{
    type_entry * pEntries = __getTypeEntries( cd );
    sal_Int32 nTypes = cd->m_nTypes;
    sal_Int32 n;

    // try top interfaces without getting td
    for ( n = 0; n < nTypes; ++n )
    {
        if (__td_equals( pEntries[ n ].m_type.typeRef, pDemandedTDR ))
        {
            return makeInterface( pEntries[ n ].m_offset, that );
        }
    }
    // query deep getting td
    for ( n = 0; n < nTypes; ++n )
    {
        typelib_TypeDescription * pTD = 0;
        TYPELIB_DANGER_GET( &pTD, pEntries[ n ].m_type.typeRef );
        if (pTD)
        {
            // exclude top (already tested) and bottom (XInterface) interface
            typelib_InterfaceTypeDescription * pITD =
                ((typelib_InterfaceTypeDescription *)pTD)->pBaseTypeDescription;
            OSL_ENSURE( pITD, "### want to implement XInterface: template argument is XInterface?!?!?!" );
            while (pITD->pBaseTypeDescription)
            {
                if (__td_equals( (typelib_TypeDescriptionReference *)pITD, pDemandedTDR ))
                {
                    TYPELIB_DANGER_RELEASE( pTD );
                    return makeInterface( pEntries[ n ].m_offset, that );
                }
                pITD = pITD->pBaseTypeDescription;
            }
            TYPELIB_DANGER_RELEASE( pTD );
        }
        else
        {
            OUStringBuffer buf( 64 );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("cannot get type description for type \"") );
            buf.append( pEntries[ n ].m_type.typeRef->pTypeName );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\"!") );
            OUString msg( buf.makeStringAndClear() );
#if OSL_DEBUG_LEVEL > 0
            OString str( OUStringToOString( msg, RTL_TEXTENCODING_ASCII_US ) );
            OSL_ENSURE( 0, str.getStr() );
#endif
            throw RuntimeException( msg, Reference< XInterface >() );
        }
    }
    return 0;
}

// ImplHelper
//==================================================================================================
Any SAL_CALL ImplHelper_query(
    Type const & rType, class_data * cd, void * that )
    SAL_THROW( (RuntimeException) )
{
    checkInterface( rType );
    typelib_TypeDescriptionReference * pTDR = rType.getTypeLibType();

    void * p;
    // shortcut for XInterface
    if (isXInterface( pTDR->pTypeName ))
    {
        // take first one
        p = makeInterface( cd->m_typeEntries[ 0 ].m_offset, that );
    }
    else
    {
        p = __queryDeepNoXInterface( pTDR, cd, that );
        if (! p)
        {
            return Any();
        }
    }
    return Any( &p, pTDR );
}
//==================================================================================================
Any SAL_CALL ImplHelper_queryNoXInterface(
    Type const & rType, class_data * cd, void * that )
    SAL_THROW( (RuntimeException) )
{
    checkInterface( rType );
    typelib_TypeDescriptionReference * pTDR = rType.getTypeLibType();

    void * p = __queryDeepNoXInterface( pTDR, cd, that );
    if (p)
    {
        return Any( &p, pTDR );
    }
    else
    {
        return Any();
    }
}
//==================================================================================================
Sequence< sal_Int8 > SAL_CALL ImplHelper_getImplementationId( class_data * cd )
    SAL_THROW( (RuntimeException) )
{
    if (! cd->m_createdId)
    {
        sal_uInt8 * id = (sal_uInt8 *)alloca( 16 );
        ::rtl_createUuid( (sal_uInt8 *)id, 0, sal_True );

        MutexGuard guard( getImplHelperInitMutex() );
        if (! cd->m_createdId)
        {
            memcpy( cd->m_id, id, 16 );
            cd->m_createdId = sal_True;
        }
    }

    sal_Sequence * seq = 0;
    ::rtl_byte_sequence_constructFromArray( &seq, cd->m_id, 16 );
    return Sequence< sal_Int8 >( seq, SAL_NO_ACQUIRE );
}
//==================================================================================================
Sequence< Type > SAL_CALL ImplHelper_getTypes(
    class_data * cd )
    SAL_THROW( (RuntimeException) )
{
    Sequence< Type > types( cd->m_nTypes );
    Type * pTypes = types.getArray();
    __fillTypes( pTypes, cd );
    return types;
}
//==================================================================================================
Sequence< Type >  SAL_CALL ImplInhHelper_getTypes(
    class_data * cd, Sequence< Type > const & rAddTypes )
    SAL_THROW( (RuntimeException) )
{
    sal_Int32 nImplTypes = cd->m_nTypes;
    sal_Int32 nAddTypes = rAddTypes.getLength();
    Sequence< Type > types( nImplTypes + nAddTypes );
    Type * pTypes = types.getArray();
    __fillTypes( pTypes, cd );
    // append base types
    Type const * pAddTypes = rAddTypes.getConstArray();
    while (nAddTypes--)
    {
        pTypes[ nImplTypes + nAddTypes ] = pAddTypes[ nAddTypes ];
    }
    return types;
}

// WeakImplHelper
//==================================================================================================
Any SAL_CALL WeakImplHelper_query(
    Type const & rType, class_data * cd, void * that, OWeakObject * pBase )
    SAL_THROW( (RuntimeException) )
{
    checkInterface( rType );
    typelib_TypeDescriptionReference * pTDR = rType.getTypeLibType();

    // shortcut XInterface to OWeakObject
    if (! isXInterface( pTDR->pTypeName ))
    {
        void * p = __queryDeepNoXInterface( pTDR, cd, that );
        if (p)
        {
            return Any( &p, pTDR );
        }
    }
    return pBase->OWeakObject::queryInterface( rType );
}
//==================================================================================================
Sequence< Type > SAL_CALL WeakImplHelper_getTypes(
    class_data * cd )
    SAL_THROW( (RuntimeException) )
{
    sal_Int32 nTypes = cd->m_nTypes;
    Sequence< Type > types( nTypes +1 );
    Type * pTypes = types.getArray();
    __fillTypes( pTypes, cd );
    pTypes[ nTypes ] = ::getCppuType( (Reference< XWeak > const *)0 );
    return types;
}

// WeakAggImplHelper
//==================================================================================================
Any SAL_CALL WeakAggImplHelper_queryAgg(
    Type const & rType, class_data * cd, void * that, OWeakAggObject * pBase )
    SAL_THROW( (RuntimeException) )
{
    checkInterface( rType );
    typelib_TypeDescriptionReference * pTDR = rType.getTypeLibType();

    // shortcut XInterface to OWeakAggObject
    if (! isXInterface( pTDR->pTypeName ))
    {
        void * p = __queryDeepNoXInterface( pTDR, cd, that );
        if (p)
        {
            return Any( &p, pTDR );
        }
    }
    return pBase->OWeakAggObject::queryAggregation( rType );
}
//==================================================================================================
Sequence< Type > SAL_CALL WeakAggImplHelper_getTypes(
    class_data * cd )
    SAL_THROW( (RuntimeException) )
{
    sal_Int32 nTypes = cd->m_nTypes;
    Sequence< Type > types( nTypes +2 );
    Type * pTypes = types.getArray();
    __fillTypes( pTypes, cd );
    pTypes[ nTypes++ ] = ::getCppuType( (Reference< XWeak > const *)0 );
    pTypes[ nTypes ] = ::getCppuType( (const Reference< XAggregation > *)0 );
    return types;
}

// WeakComponentImplHelper
//==================================================================================================
Any SAL_CALL WeakComponentImplHelper_query(
    Type const & rType, class_data * cd, void * that, WeakComponentImplHelperBase * pBase )
    SAL_THROW( (RuntimeException) )
{
    checkInterface( rType );
    typelib_TypeDescriptionReference * pTDR = rType.getTypeLibType();

    // shortcut XInterface to WeakComponentImplHelperBase
    if (! isXInterface( pTDR->pTypeName ))
    {
        void * p = __queryDeepNoXInterface( pTDR, cd, that );
        if (p)
        {
            return Any( &p, pTDR );
        }
    }
    return pBase->WeakComponentImplHelperBase::queryInterface( rType );
}
//==================================================================================================
Sequence< Type > SAL_CALL WeakComponentImplHelper_getTypes(
    class_data * cd )
    SAL_THROW( (RuntimeException) )
{
    sal_Int32 nTypes = cd->m_nTypes;
    Sequence< Type > types( nTypes +2 );
    Type * pTypes = types.getArray();
    __fillTypes( pTypes, cd );
    pTypes[ nTypes++ ] = ::getCppuType( (Reference< XWeak > const *)0 );
    pTypes[ nTypes ] = ::getCppuType( (Reference< lang::XComponent > const *)0 );
    return types;
}

// WeakAggComponentImplHelper
//==================================================================================================
Any SAL_CALL WeakAggComponentImplHelper_queryAgg(
    Type const & rType, class_data * cd, void * that, WeakAggComponentImplHelperBase * pBase )
    SAL_THROW( (RuntimeException) )
{
    checkInterface( rType );
    typelib_TypeDescriptionReference * pTDR = rType.getTypeLibType();

    // shortcut XInterface to WeakAggComponentImplHelperBase
    if (! isXInterface( pTDR->pTypeName ))
    {
        void * p = __queryDeepNoXInterface( pTDR, cd, that );
        if (p)
        {
            return Any( &p, pTDR );
        }
    }
    return pBase->WeakAggComponentImplHelperBase::queryAggregation( rType );
}
//==================================================================================================
Sequence< Type > SAL_CALL WeakAggComponentImplHelper_getTypes(
    class_data * cd )
    SAL_THROW( (RuntimeException) )
{
    sal_Int32 nTypes = cd->m_nTypes;
    Sequence< Type > types( nTypes +3 );
    Type * pTypes = types.getArray();
    __fillTypes( pTypes, cd );
    pTypes[ nTypes++ ] = ::getCppuType( (Reference< XWeak > const *)0 );
    pTypes[ nTypes++ ] = ::getCppuType( (const Reference< XAggregation > *)0 );
    pTypes[ nTypes ] = ::getCppuType( (const Reference< lang::XComponent > *)0 );
    return types;
}

}
