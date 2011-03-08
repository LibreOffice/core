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
#ifndef _URP_MARSHAL_HXX_
#define _URP_MARSHAL_HXX_
#include <rtl/ustrbuf.hxx>
#include <rtl/byteseq.hxx>
#include <com/sun/star/uno/Type.hxx>
#include "urp_bridgeimpl.hxx"
#include "urp_marshal_decl.hxx"

#include <string.h>

struct remote_Interface;

namespace bridges_urp
{
    // methods for accessing marshaling buffer
    inline void Marshal::finish( sal_Int32 nMessageCount )
    {
        sal_Int32 nSize = getSize() - 2*sizeof( sal_Int32 );

        // save the state
        sal_Int8 *pos = m_pos;
        m_pos = m_base;
        packInt32( &nSize );
        packInt32( &nMessageCount );

        // reset the state
        m_pos = pos;
    }

    inline void Marshal::restart()
    {
        m_pos = m_base + 2*sizeof( sal_Int32 );
    }

    inline sal_Int8 *Marshal::getBuffer()
    {
        return m_base;
    }

    inline sal_Bool Marshal::empty() const
    {
        return ( m_pos - m_base ) == 2*sizeof( sal_Int32 );
    }

    inline sal_Int32 Marshal::getSize()
    {
        return ((sal_Int32) (m_pos - m_base));
    }

    inline void Marshal::ensureAdditionalMem( sal_Int32 nMemToAdd )
    {
        sal_Int32 nDiff = m_pos - m_base;
        if( nDiff + nMemToAdd > m_nBufferSize )
        {
            m_nBufferSize = m_nBufferSize * 2 > nDiff + nMemToAdd ?
                m_nBufferSize* 2 :
                nDiff + nMemToAdd;

            m_base = ( sal_Int8 * ) rtl_reallocateMemory( m_base , m_nBufferSize );
            m_pos = m_base + nDiff;
        }
    }

    // marshaling methods
    inline void Marshal::packInt8( void *pSource )
    {
        ensureAdditionalMem( 1 );
        *m_pos = *((sal_Int8*) pSource );
        m_pos++;
    }

    inline void Marshal::packInt16( void *pSource )
    {
        ensureAdditionalMem( 2 );
        if( isSystemLittleEndian() )
        {
            m_pos[0] = ((unsigned char *)pSource)[1];
            m_pos[1] = ((unsigned char *)pSource)[0];
        }
        else
        {
            m_pos[1] = ((unsigned char *)pSource)[1];
            m_pos[0] = ((unsigned char *)pSource)[0];
        }
        m_pos +=2;
    }

    inline void Marshal::packByteSequence( sal_Int8 *pData , sal_Int32 nLength )
    {
        packCompressedSize( nLength );

        ensureAdditionalMem( nLength );
        memcpy( m_pos , pData , nLength );
        m_pos += nLength;
    }

    inline void Marshal::packString( void *pSource )
    {
        rtl_uString *p = *( rtl_uString ** ) pSource;

        // to be optimized !
        // static buffer in marshal
        ::rtl::OString o = ::rtl::OUStringToOString( p , RTL_TEXTENCODING_UTF8 );
        sal_Int32 nLength = o.pData->length;
        packCompressedSize( nLength );

        ensureAdditionalMem( nLength );

        memcpy( m_pos , o.pData->buffer , nLength  );
        m_pos += nLength;
    }

    inline sal_Bool Marshal::packAny( void *pSource )
    {
        sal_Bool bSuccess = sal_True;
        uno_Any *pAny = (uno_Any * ) pSource;

        // pack the type
        packType( &( pAny->pType ) );
        // pack the value
        typelib_TypeDescription *pType = 0;
        TYPELIB_DANGER_GET( &pType, pAny->pType );
        if( pType )
        {
            pack( pAny->pData , pType );
            TYPELIB_DANGER_RELEASE( pType );
        }
        else
        {
            rtl::OUStringBuffer buf( 128 );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("couldn't get typedescription for type " ) );
            buf.append( pAny->pType->pTypeName );
            m_pBridgeImpl->addError( buf.makeStringAndClear() );
            bSuccess = sal_False;
        }
        return bSuccess;
    }

    inline void Marshal::packInt32( void *pSource )
    {
        ensureAdditionalMem( 4 );
        if( isSystemLittleEndian() )
        {
            m_pos[0] = ((unsigned char *)pSource)[3];
            m_pos[1] = ((unsigned char *)pSource)[2];
            m_pos[2] = ((unsigned char *)pSource)[1];
            m_pos[3] = ((unsigned char *)pSource)[0];
        }
        else {
            m_pos[3] = ((unsigned char *)pSource)[3];
            m_pos[2] = ((unsigned char *)pSource)[2];
            m_pos[1] = ((unsigned char *)pSource)[1];
            m_pos[0] = ((unsigned char *)pSource)[0];
        }
        m_pos +=4;
    }

    inline void Marshal::packCompressedSize( sal_Int32 nSize )
    {
        ensureAdditionalMem( 5 );

        if( nSize < 0xff )
        {
            *((sal_uInt8*)m_pos) = (sal_uInt8) nSize;
            m_pos ++;
        }
        else
        {
            *((sal_uInt8*)m_pos) = 0xff;
            m_pos ++;
            packInt32( &nSize );
        }
    }

    inline sal_Bool Marshal::pack( void *pSource , typelib_TypeDescription *pType )
    {
        sal_Bool bSuccess = sal_True;
        switch( pType->eTypeClass )
        {
        case typelib_TypeClass_BYTE:
        {
            packInt8( pSource );
            break;
        }
        case typelib_TypeClass_BOOLEAN:
        {
            ensureAdditionalMem( 1 );
            *m_pos = ( *((sal_Bool*) pSource ) ) ? 1 : 0;
            m_pos++;
            break;
        }

        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
        {
            packInt16( pSource );
            break;
        }
        case typelib_TypeClass_ENUM:
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_FLOAT:
        {
            packInt32( pSource );
            break;
        }
        case typelib_TypeClass_DOUBLE:
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
        {
            ensureAdditionalMem( 8 );
            if( isSystemLittleEndian() )
            {
                m_pos[0] = ((unsigned char *)pSource)[7];
                m_pos[1] = ((unsigned char *)pSource)[6];
                m_pos[2] = ((unsigned char *)pSource)[5];
                m_pos[3] = ((unsigned char *)pSource)[4];
                m_pos[4] = ((unsigned char *)pSource)[3];
                m_pos[5] = ((unsigned char *)pSource)[2];
                m_pos[6] = ((unsigned char *)pSource)[1];
                m_pos[7] = ((unsigned char *)pSource)[0];
            }
            else
            {
                m_pos[7] = ((unsigned char *)pSource)[7];
                m_pos[6] = ((unsigned char *)pSource)[6];
                m_pos[5] = ((unsigned char *)pSource)[5];
                m_pos[4] = ((unsigned char *)pSource)[4];
                m_pos[3] = ((unsigned char *)pSource)[3];
                m_pos[2] = ((unsigned char *)pSource)[2];
                m_pos[1] = ((unsigned char *)pSource)[1];
                m_pos[0] = ((unsigned char *)pSource)[0];
            }
            m_pos += 8;
            break;
        }

        case typelib_TypeClass_STRING:
        {
            packString( pSource );
            break;
        }
        case typelib_TypeClass_TYPE:
        {
            packType( pSource );
            break;
        }
        case typelib_TypeClass_ANY:
        {
            bSuccess = packAny( pSource );
            break;
        }
        case typelib_TypeClass_TYPEDEF:
        {
            bSuccess = sal_False;
            m_pBridgeImpl->addError( "can't handle typedef typedescriptions" );
            break;
        }
        case typelib_TypeClass_INTERFACE:
        {
            remote_Interface *pRemoteI = *( remote_Interface ** )pSource;

            ::rtl::OUString sOid;
            sal_uInt16 nIndex = 0xffff;
            if( pRemoteI )
            {
                m_callback( pRemoteI , &(sOid.pData) );

                nIndex = m_pBridgeImpl->m_oidCacheOut.seek( sOid );
                if( 0xffff == nIndex )
                {
                    nIndex = m_pBridgeImpl->m_oidCacheOut.put( sOid );
                }
                else
                {
                    // cached !
                    sOid = ::rtl::OUString();
                }
            }
            packString( &sOid );
            packInt16( &nIndex );
            break;
        }
        case typelib_TypeClass_VOID:
        {
            // do nothing
            break;
        }
        case typelib_TypeClass_EXCEPTION:
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_SEQUENCE:
        {
            bSuccess = packRecursive( pSource, pType );
            break;
        }
        default:
        {
            bSuccess = sal_False;
            rtl::OUStringBuffer buf( 128 );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "can't handle values with typeclass " ) );
            buf.append( (sal_Int32 ) pType->eTypeClass , 10 );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( " (" ) );
            buf.append( pType->pTypeName );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( ")" ) );
            m_pBridgeImpl->addError( buf.makeStringAndClear() );
            break;
        }
        }
        return bSuccess;
    }
}



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
