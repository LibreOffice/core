/*************************************************************************
 *
 *  $RCSfile: urp_unmarshal.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: jbu $ $Date: 2001-08-31 16:16:52 $
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
#include <string.h>

#include <osl/diagnose.h>

#include <rtl/alloc.h>
#include <rtl/ustrbuf.hxx>

#include <uno/data.h>
#include <uno/any2.h>
#include <uno/sequence2.h>

#include <com/sun/star/uno/Any.hxx>

#include "urp_unmarshal.hxx"
#include "urp_bridgeimpl.hxx"


using namespace ::rtl;
using namespace ::com::sun::star::uno;

namespace bridges_urp
{
static int g_nDetectLittleEndian = 1;
char g_bSystemIsLittleEndian = ((char*)&g_nDetectLittleEndian)[0];


Unmarshal::Unmarshal( struct urp_BridgeImpl *pBridgeImpl,
                      uno_Environment *pEnvRemote,
                      remote_createStubFunc callback ) :
    m_pBridgeImpl( pBridgeImpl ),
    m_pEnvRemote( pEnvRemote ),
    m_callback( callback ),
    m_nBufferSize( 4096 ),
    m_base( (sal_Int8*) rtl_allocateMemory( m_nBufferSize ) ),
    m_pos( m_base ),
    m_nLength( 0 )
{
}

Unmarshal::~Unmarshal()
{
    rtl_freeMemory( m_base );
}

// special unpacks
sal_Bool Unmarshal::unpackTid( sal_Sequence **ppThreadId )
{
    sal_Int32 nSize;
    sal_Bool bReturn = unpackCompressedSize( &nSize );
    if( bReturn )
    {
        if( nSize )
        {
            rtl_byte_sequence_constructFromArray( ppThreadId , m_pos , nSize );
            m_pos += nSize;
            sal_uInt16 nIndex;
            bReturn = unpackInt16( &nIndex );
            if( nIndex < m_pBridgeImpl->m_properties.nTidCacheSize )
            {
                m_pBridgeImpl->m_pTidIn[nIndex] = *(ByteSequence * )ppThreadId;
            }
            else if( 0xffff != nIndex )
            {
                bReturn = sal_False;
                rtl_byte_sequence_construct( ppThreadId , 0 );

                OUStringBuffer error( 128 );
                error.appendAscii( "cache index for tid " );
                OString o = byteSequence2HumanReadableString( *(ByteSequence * ) ppThreadId );
                error.appendAscii( o.getStr(), o.getLength() );
                error.appendAscii( "out of range(0x");
                error.append(  (sal_Int32) nIndex ,16 );
                error.appendAscii( ")" );
                m_pBridgeImpl->addError( error.makeStringAndClear() );
            }
        }
        else
        {
            sal_uInt16 nIndex;
            bReturn = unpackInt16( &nIndex );
            if( nIndex < m_pBridgeImpl->m_properties.nTidCacheSize )
            {
                *ppThreadId = m_pBridgeImpl->m_pTidIn[nIndex].getHandle();
                rtl_byte_sequence_acquire( *ppThreadId );
            }
            else
            {
                bReturn = sal_False;
                rtl_byte_sequence_construct( ppThreadId , 0 );
                OUStringBuffer error(128);
                error.appendAscii( "cache index for tids out of range(0x" );
                error.append( (sal_Int32) nIndex ,16 );
                error.appendAscii( ")" );
                m_pBridgeImpl->addError( error.makeStringAndClear() );
            }
        }
    }
    else
    {
        m_pBridgeImpl->addError( "couldn't unpack thread id because of previous errors" );
    }
    return bReturn;
}

sal_Bool Unmarshal::unpackOid( rtl_uString **ppOid )
{
    sal_Bool bReturn = sal_True;
    sal_uInt16 nCacheIndex;

    bReturn = bReturn && unpackString( ppOid );
    bReturn = bReturn && unpackInt16( &nCacheIndex );

    if( bReturn &&
        ! ( 0xffff == nCacheIndex && 0 == (*ppOid)->length ) /* null reference */  )
    {
        if( (*ppOid)->length )
        {
            // new unknown reference
            if( 0xffff != nCacheIndex )
            {
                // oid should be cached ?
                if( nCacheIndex < m_pBridgeImpl->m_properties.nOidCacheSize )
                {
                    m_pBridgeImpl->m_pOidIn[nCacheIndex] = *ppOid;
                }
                else
                {
                    OUStringBuffer error( 128 );
                    error.appendAscii( "new oid provided (" );
                    error.append( *ppOid );
                    error.appendAscii( "), but new cache index is out of range(0x");
                    error.append(  (sal_Int32) nCacheIndex ,16 );
                    error.appendAscii( ")" );
                    m_pBridgeImpl->addError( error.makeStringAndClear() );

                    bReturn = sal_False;
                    rtl_uString_new( ppOid );
                }
            }
        }
        else
        {
            // reference in cache !
            if( nCacheIndex < m_pBridgeImpl->m_properties.nOidCacheSize )
            {
                rtl_uString_assign( ppOid , m_pBridgeImpl->m_pOidIn[nCacheIndex].pData );
            }
            else
            {
                bReturn = sal_False;
                rtl_uString_new( ppOid );

                OUStringBuffer error( 128 );
                error.appendAscii( "cache index for oids out of range(0x");
                error.append(  (sal_Int32) nCacheIndex ,16 );
                error.appendAscii( ")" );
                m_pBridgeImpl->addError( error.makeStringAndClear() );
            }
        }
    }

    return bReturn;
}

sal_Bool Unmarshal::unpackType( void *pDest )
{
    *(typelib_TypeDescriptionReference **) pDest = 0;

    sal_uInt8 nTypeClass;
    sal_Bool bReturn = unpackInt8( &nTypeClass );

    typelib_TypeDescriptionReference *pTypeRef = 0;
    if( bReturn )
    {
        if( nTypeClass <= 14 /* any */ )
        {
            pTypeRef = * typelib_static_type_getByTypeClass((typelib_TypeClass )nTypeClass);
            typelib_typedescriptionreference_acquire( pTypeRef );
        }
        else
        {
            sal_uInt16 nCacheIndex;
            bReturn = bReturn && unpackInt16( &nCacheIndex  );

            if( bReturn )
            {
                if( nTypeClass & 0x80 )
                {
                    // new type
                    rtl_uString *pString = 0;
                    bReturn = bReturn && unpackString( &pString );
                    if( bReturn )
                    {
                        typelib_TypeDescription *pType = 0;
                        typelib_typedescription_getByName( &pType, pString );
                        if( pType )
                        {
                            // type is known in this process
                            if( (typelib_TypeClass )(nTypeClass & 0x7f) == pType->eTypeClass )
                            {
                                //typename and typeclass match, this is as it should be
                                pTypeRef = pType->pWeakRef;
                                typelib_typedescriptionreference_acquire( pTypeRef );
                            }
                            else
                            {
                                // typename and typeclass do not match, dispose the bridge
                                // as there must be inconsistent type base between both processes
                                // or trash comes over the wire ...
                                bReturn = sal_False;
                                OUStringBuffer error( 128 );
                                error.appendAscii( "it is tried to introduce type " );
                                error.append( pString );
                                error.appendAscii( "with typeclass " );
                                error.append( (sal_Int32)( nTypeClass & 0x7f ) , 10 );
                                error.appendAscii( " , which does not match with typeclass " );
                                error.append(  (sal_Int32) pType->eTypeClass ,10 );
                                m_pBridgeImpl->addError( error.makeStringAndClear() );
                            }
                            typelib_typedescription_release( pType );
                            pType = 0;
                        }
                        else
                        {
                            // a type by this name is not known in the process.
                            if( (nTypeClass & 0x7f) >= 0 &&
                                (nTypeClass & 0x7f) < typelib_TypeClass_UNKNOWN )
                            {
                                // typeclass is within a valid range, introduce the new
                                // type.
                                typelib_typedescriptionreference_new(
                                    &pTypeRef, (typelib_TypeClass )(nTypeClass & 0x7f), pString );
                            }
                            else
                            {
                                // typeclass is out of range !
                                bReturn = sal_False;
                                OUStringBuffer error( 128 );
                                error.appendAscii( "it is tried to introduce type " );
                                error.append( pString );
                                error.appendAscii( "with an out of range typeclass " );
                                error.append( (sal_Int32)( nTypeClass & 0x7f ) , 10 );
                                m_pBridgeImpl->addError( error.makeStringAndClear() );
                            }
                        }

                        if( bReturn && nCacheIndex != 0xffff )
                        {
                            if( nCacheIndex < m_pBridgeImpl->m_properties.nTypeCacheSize )
                            {
                                m_pBridgeImpl->m_pTypeIn[nCacheIndex] = *( Type * )&pTypeRef;
                            }
                            else
                            {
                                bReturn = sal_False;
                                OUStringBuffer error( 128 );
                                error.appendAscii( "cache index for type " );
                                error.append( pString );
                                error.appendAscii( "out of range(0x" );
                                error.append( (sal_Int32) nCacheIndex ,16 );
                                error.appendAscii( ")" );
                                m_pBridgeImpl->addError( error.makeStringAndClear() );
                            }
                        }
                    }
                    if( pString )
                    {
                        rtl_uString_release( pString );
                    }
                }
                else
                {
                    if( nCacheIndex < m_pBridgeImpl->m_properties.nTypeCacheSize )
                    {
                        pTypeRef = m_pBridgeImpl->m_pTypeIn[nCacheIndex].getTypeLibType();
                        typelib_typedescriptionreference_acquire( pTypeRef );
                    }
                    else
                    {
                        bReturn = sal_False;
                        OUStringBuffer error;
                        error.appendAscii( "cache index for types out of range(0x" );
                        error.append( (sal_Int32) nCacheIndex ,16 );
                        error.appendAscii( ")" );
                        m_pBridgeImpl->addError( error.makeStringAndClear() );
                    }
                }
            }
        }
    }
    if( ! pTypeRef )
    {
        pTypeRef = * typelib_static_type_getByTypeClass(typelib_TypeClass_VOID);
        typelib_typedescriptionreference_acquire( pTypeRef );
    }
    // pTypeRef is already acquired
    *(typelib_TypeDescriptionReference**)pDest = pTypeRef;
    return bReturn;
}

sal_Bool Unmarshal::unpackAny( void *pDest )
{
    uno_Any *pAny = ( uno_Any * )pDest;

    pAny->pType = 0;
    // Type is acquired with typelib_typedescription_acquire

    sal_Bool bReturn = unpackType( &(pAny->pType) );

    typelib_TypeDescription *pType = 0;
    if( bReturn && pAny->pType )
    {
        typelib_typedescriptionreference_getDescription( &pType , pAny->pType );

        if( pType )
        {
            switch (pType->eTypeClass)
            {
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
                if (sizeof(void *) < sizeof(sal_Int64))
                {
                    pAny->pData = rtl_allocateMemory( sizeof(sal_Int64) );
                }
                else
                {
                    pAny->pData = &pAny->pReserved;
                }
                break;
            case typelib_TypeClass_FLOAT:
                if (sizeof(void *) < sizeof(float))
                {
                    pAny->pData = rtl_allocateMemory( sizeof(float) );
                }
                else
                {
                    pAny->pData = &pAny->pReserved;
                }
                break;
            case typelib_TypeClass_DOUBLE:
                if (sizeof(void *) < sizeof(double))
                {
                    pAny->pData = rtl_allocateMemory( sizeof(double) );
                }
                else
                {
                    pAny->pData = &pAny->pReserved;
                }
                break;
            case typelib_TypeClass_STRUCT:
            case typelib_TypeClass_UNION:
            case typelib_TypeClass_EXCEPTION:
            case typelib_TypeClass_ARRAY:
                pAny->pData = rtl_allocateMemory( pType->nSize );
                break;
            default:
                pAny->pData = &pAny->pReserved;
            }

            bReturn = unpack( pAny->pData , pType );
        }
        else
        {
            OUStringBuffer error;
            error.appendAscii( "can't unmarshal any because typedescription for " );
            error.append( pAny->pType->pTypeName );
            error.appendAscii( " is missing" );
            m_pBridgeImpl->addError( error.makeStringAndClear() );
        }
    }

    if( pType )
    {
        typelib_typedescription_release( pType );
    }
    else
    {
        pAny->pData = 0;
        Type type; // void
        pAny->pType = type.getTypeLibType();
        typelib_typedescriptionreference_acquire( pAny->pType );

        bReturn = sal_False;
    }
    return bReturn;
}


sal_Bool Unmarshal::unpackRecursive( void *pDest , typelib_TypeDescription *pType )
{
    sal_Bool bReturn = sal_True;

    switch( pType->eTypeClass )
    {
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
        typelib_CompoundTypeDescription * pCompType =
            (typelib_CompoundTypeDescription *)pType;

        if (pCompType->pBaseTypeDescription)
        {
            bReturn =
                unpack( pDest , (typelib_TypeDescription * ) pCompType->pBaseTypeDescription );
        }

        // then construct members
        typelib_TypeDescriptionReference ** ppTypeRefs = pCompType->ppTypeRefs;
        sal_Int32 * pMemberOffsets = pCompType->pMemberOffsets;
        sal_Int32 nDescr = pCompType->nMembers;

        // at least assume 1 byte per member
        bReturn = bReturn && ! checkOverflow( nDescr * 1 );
        for ( sal_Int32 nPos = 0; nPos < nDescr; ++nPos )
        {
            typelib_TypeDescription * pMemberType = 0;
            TYPELIB_DANGER_GET( &pMemberType, ppTypeRefs[nPos] );
            // Even if bReturn is false, all values must be default constructed !
            if( bReturn )
            {
                bReturn = unpack( (char*)pDest + pMemberOffsets[nPos] , pMemberType ) && bReturn;
            }
            else
            {
                uno_constructData( (char*)pDest + pMemberOffsets[nPos]  , pMemberType );
            }
            TYPELIB_DANGER_RELEASE( pMemberType );
        }
        break;
    }
    case typelib_TypeClass_SEQUENCE:
    {
        sal_Int32 nLen;
        bReturn = unpackCompressedSize( &nLen );

        // urp protocol does not allow to use the elementsize as a guess, if enough data
        // is available. However, at least one byte per member must be within the message
        bReturn = bReturn && ! checkOverflow( 1 * nLen );
        uno_Sequence *pSequence = 0;
        if( nLen && bReturn )
        {
            typelib_TypeDescriptionReference * pETRef =
                ((typelib_IndirectTypeDescription *)pType)->pType;

            typelib_TypeDescription * pET = 0;
            typelib_typedescriptionreference_getDescription( &pET , pETRef );

            if( pET )
            {
                sal_Int32 nElementSize = pET->nSize;

                pSequence = (uno_Sequence *)rtl_allocateMemory(
                    SAL_SEQUENCE_HEADER_SIZE + nElementSize * nLen );
                pSequence->nRefCount = 1;
                pSequence->nElements = nLen;

                if( typelib_TypeClass_BYTE == pET->eTypeClass )
                {
                    memcpy( pSequence->elements , m_pos , nLen );
                    m_pos += nLen;
                }
                else
                {
                    for( sal_Int32 i = 0 ; i < nLen ; i ++ )
                    {
                        bReturn = unpack( ((char*)pSequence->elements)+ i*nElementSize,pET ) && bReturn;
                    }
                }
                typelib_typedescription_release( pET );
            }
            else
            {
                bReturn = sal_False;
                uno_constructData( &pSequence , pType );
                OUStringBuffer error;
                error.appendAscii( "can't unmarshal sequence, because there is no typedescription for element type " );
                error.append( pETRef->pTypeName );
                error.appendAscii( " available" );
                m_pBridgeImpl->addError( error.makeStringAndClear() );
            }
        }
        else
        {
            uno_constructData( &pSequence , pType );
        }

        *((uno_Sequence **)pDest) = pSequence;
        break;
    }
    default:
        OSL_ASSERT( 0 );
    }
    return bReturn;
}

}

