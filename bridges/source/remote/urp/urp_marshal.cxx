/*************************************************************************
 *
 *  $RCSfile: urp_marshal.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:28:50 $
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

#include <uno/any2.h>
#include <uno/sequence2.h>

#include "urp_marshal.hxx"

using namespace ::rtl;

using namespace ::com::sun::star::uno;

namespace bridges_urp {

static int g_nDetectLittleEndian = 1;
char g_bMarshalSystemIsLittleEndian = ((char*)&g_nDetectLittleEndian)[0];

Marshal::Marshal( urp_BridgeImpl *pBridgeImpl,
                  sal_Int32 nBufferSize,
                  urp_extractOidCallback callback) :
    m_callback( callback ),
    m_nBufferSize( nBufferSize ),
    m_base( (sal_Int8*)rtl_allocateMemory( nBufferSize ) ),
    m_pos( m_base + 5 ),
    m_nSizeOffset( 0 ),
    m_pBridgeImpl( pBridgeImpl )
{}

Marshal::~Marshal( )
{
    rtl_freeMemory( m_base );
}

void Marshal::packOid( const ::rtl::OUString & oid )
{
    sal_uInt16 nIndex;
    if( oid.getLength() )
    {
        nIndex = m_pBridgeImpl->m_oidCacheOut.seek( oid );
        if( 0xffff == nIndex )
        {
            nIndex = m_pBridgeImpl->m_oidCacheOut.put( oid );
            packString( (void*)(&oid.pData) );
        }
        else
        {
            OUString dummy;
            packString( &dummy );
        }
    }
    else
    {
        // null reference
        sal_uInt16 nIndex = 0xffff;
        OUString dummy;
        packString( &dummy );
    }
    packInt16( &nIndex );
}

void Marshal::packTid( const ByteSequence & threadId )
{
    sal_uInt16 nIndex = m_pBridgeImpl->m_tidCacheOut.seek( threadId );
    if( 0xffff == nIndex )
    {
        nIndex = m_pBridgeImpl->m_tidCacheOut.put( threadId );
        packByteSequence( (sal_Int8*) threadId.getConstArray() ,threadId.getLength());
    }
    else
    {
        packByteSequence( 0 , 0 );
    }
    packInt16( &nIndex );
}


void Marshal::packType( void *pSource )
{
    typelib_TypeDescriptionReference *pRef =
        *(typelib_TypeDescriptionReference ** ) pSource;

    OSL_ASSERT( pRef );
    Type type( pRef );

    sal_uInt8 nTypeClass = type.getTypeClass();

    if( nTypeClass <= /* any*/ 14 )
    {
        packInt8( (sal_Int8*)&nTypeClass );
    }
    else
    {
        OUString sTypeName;
        sal_uInt16 nIndex = 0xffff;

        nIndex = m_pBridgeImpl->m_typeCacheOut.seek( type );
        if( 0xffff == nIndex )
        {
            // put it into the cache
            nIndex = m_pBridgeImpl->m_typeCacheOut.put( type );
            sTypeName = type.getTypeName();
            nTypeClass = nTypeClass | 0x80;
        }
        packInt8( &nTypeClass  );
        packInt16( &nIndex );
        if( 0x80 & nTypeClass )
        {
            packString( &sTypeName );
        }
    }
}

void Marshal::packRecursive( void *pSource , typelib_TypeDescription *pType )
{
    switch( pType->eTypeClass )
    {
    case typelib_TypeClass_EXCEPTION:
    case typelib_TypeClass_STRUCT:
    {
        typelib_CompoundTypeDescription * pCompType = (typelib_CompoundTypeDescription*)pType;

        if (pCompType->pBaseTypeDescription)
        {
            pack( pSource , (typelib_TypeDescription*) pCompType->pBaseTypeDescription );
        }

        // then construct members
        typelib_TypeDescriptionReference ** ppTypeRefs = pCompType->ppTypeRefs;
        sal_Int32 * pMemberOffsets = pCompType->pMemberOffsets;
        sal_Int32 nDescr = pCompType->nMembers;

        for ( sal_Int32 nPos = 0; nPos < nDescr; ++nPos )
        {
              typelib_TypeDescription * pMemberType = 0;
              TYPELIB_DANGER_GET( &pMemberType, ppTypeRefs[nPos] );
            pack( (char*)pSource + pMemberOffsets[nPos] , pMemberType );
              TYPELIB_DANGER_RELEASE( pMemberType );
        }
        break;
    }
    case typelib_TypeClass_SEQUENCE:
    {
        typelib_IndirectTypeDescription *pIndirectType =
            ( typelib_IndirectTypeDescription* ) pType;

        const sal_Int32 nElements        = (*(uno_Sequence **)pSource)->nElements;
        char * pSourceElements   = (char *)(*(uno_Sequence **)pSource)->elements;

        if( typelib_TypeClass_BYTE == pIndirectType->pType->eTypeClass )
        {
            // Byte sequences are optimized
            packByteSequence( (sal_Int8*)pSourceElements , nElements );
        }
        else
        {
            typelib_TypeDescription *pElementType = 0;
            TYPELIB_DANGER_GET( &pElementType, pIndirectType->pType );
            if( pElementType )
            {
                const sal_Int32 nElementSize     = pElementType->nSize;

                packCompressedSize( nElements );
                for ( sal_Int32 i = 0 ; i < nElements; i++ )
                {
                    pack( pSourceElements + (nElementSize*i) , pElementType );
                }
                TYPELIB_DANGER_RELEASE( pElementType );
            }
        }

        break;
    }
    default:
        OSL_ASSERT( 0 );
    }
}

} // end namespace bridges
