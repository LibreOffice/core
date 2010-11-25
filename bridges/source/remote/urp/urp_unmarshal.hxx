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
#ifndef _URP_UNMARSHAL_HXX_
#define _URP_UNMARSHAL_HXX_

#include <stack>
#include <vector>
#include <rtl/byteseq.hxx>
#include <rtl/ustrbuf.hxx>
#include <bridges/remote/context.h>

#include <bridges/remote/helper.hxx>
#include <com/sun/star/uno/Type.hxx>
#include "urp_bridgeimpl.hxx"

typedef struct _uno_Environment uno_Environment;
struct remote_Interface;

namespace bridges_urp
{

extern char g_bSystemIsLittleEndian;
class ThreadId;
struct urp_BridgeImpl;
void SAL_CALL urp_releaseRemoteCallback(
    remote_Interface *pRemoteI,rtl_uString *pOid,
    typelib_TypeDescriptionReference *pTypeRef,
    uno_Environment *pEnvRemote );

struct UnpackItem
{
    void * pDest;
    typelib_TypeDescription * pType;
    bool bMustBeConstructed;

    UnpackItem()
    : pDest( 0 ), pType( 0 ), bMustBeConstructed( false ) {}
    UnpackItem( void * d, typelib_TypeDescription * t, bool b = false )
    : pDest( d ), pType( t ), bMustBeConstructed( b ) {}
};

typedef std::stack< UnpackItem > UnpackItemStack;
typedef std::vector< typelib_TypeDescription * > TypeDescVector;

class Unmarshal
{
public:
    Unmarshal(
        struct urp_BridgeImpl *,
        uno_Environment *pEnvRemote,
        remote_createStubFunc callback );
    ~Unmarshal();

    inline sal_Bool finished() const
        { return m_base + m_nLength == m_pos; }
    inline sal_uInt32 getPos() const
        { return (sal_uInt32 ) (m_pos - m_base); }

    inline sal_Bool setSize( sal_Int32 nSize );

    sal_Bool unpack( void *pDest, typelib_TypeDescription *pType );
    inline sal_Bool unpackCompressedSize( sal_Int32 *pData );
    inline sal_Bool unpackInt8( void *pDest );
    inline sal_Bool unpackString( void *pDest );
    inline sal_Bool unpackInt16( void *pDest );
    inline sal_Bool unpackInt32( void *pDest );
    sal_Bool unpackType( void *pDest );

    inline sal_Bool unpackAny( void *pDest );
    sal_Bool unpackOid( rtl_uString **ppOid );
    sal_Bool unpackTid( sal_Sequence **ppThreadId );

    sal_Int8 *getBuffer()
        { return m_base; }
    inline sal_Bool isSystemLittleEndian() const
        { return g_bSystemIsLittleEndian; }

private:
    inline sal_Bool checkOverflow( sal_Int32 nNextMem );

    UnpackItemStack m_aItemsToUnpack;
    TypeDescVector  m_aTypesToRelease;

    sal_Int32 m_nBufferSize;
    sal_Int8 *m_base;
    sal_Int8 *m_pos;
    sal_Int32 m_nLength;

    remote_createStubFunc m_callback;
    uno_Environment *m_pEnvRemote;
    urp_BridgeImpl *m_pBridgeImpl;
};

inline sal_Bool Unmarshal::setSize( sal_Int32 nSize )
{
    if( nSize > m_nBufferSize )
    {
        // adjust buffer size and length.
        sal_Int8 * base =
            (sal_Int8*)rtl_reallocateMemory (m_base, sal_Size(nSize));
        if (base != 0)
        {
            m_base = base;
            m_nLength = m_nBufferSize = nSize;
        }
    }
    else
    {
        // adjust buffer length, only.
        m_nLength = nSize;
    }

    // reset buffer position, and leave.
    m_pos = m_base;
    return (m_nLength == nSize);
}

inline sal_Bool Unmarshal::checkOverflow( sal_Int32 nNextMem )
{
    sal_Bool bOverflow = nNextMem < 0 ||
           (((sal_Int32)( m_pos - m_base )) + nNextMem ) > m_nLength;
    if( bOverflow )
        m_pBridgeImpl->addError( "message too short" );
    return bOverflow;
}


inline sal_Bool Unmarshal::unpackInt8( void *pDest )
{
    sal_Bool bReturn = ! checkOverflow( 1 );
    if( bReturn )
    {
        *((sal_Int8*)pDest ) = *m_pos;
        m_pos++;
    }
    else
    {
        *((sal_Int8*)pDest ) = 0;
    }
    return bReturn;
}

inline sal_Bool Unmarshal::unpackInt32( void *pDest )
{
    sal_uInt32 *p = ( sal_uInt32 * ) pDest;
    sal_Bool bReturn = ! checkOverflow(4);
    if( bReturn )
    {
        if( isSystemLittleEndian() )
        {
            ((sal_Int8*) p )[3] = m_pos[0];
            ((sal_Int8*) p )[2] = m_pos[1];
            ((sal_Int8*) p )[1] = m_pos[2];
            ((sal_Int8*) p )[0] = m_pos[3];
            }
        else
        {
            ((sal_Int8*) p )[3] = m_pos[3];
            ((sal_Int8*) p )[2] = m_pos[2];
            ((sal_Int8*) p )[1] = m_pos[1];
            ((sal_Int8*) p )[0] = m_pos[0];
        }
        m_pos += 4;
    }
    else
    {
        *p = 0;
    }
    return bReturn;
}

inline sal_Bool Unmarshal::unpackInt16( void *pDest )
{
    sal_uInt16 *p = ( sal_uInt16 * ) pDest;

    sal_Bool bReturn = ! checkOverflow( 2 );
    if( bReturn )
    {
        if( isSystemLittleEndian() )
        {
            ((sal_Int8*) p )[1] = m_pos[0];
            ((sal_Int8*) p )[0] = m_pos[1];
        }
        else
        {
            ((sal_Int8*) p )[1] = m_pos[1];
            ((sal_Int8*) p )[0] = m_pos[0];
        }
        m_pos ++;
        m_pos ++;
    }
    else
    {
        *p = 0;
    }
    return bReturn;
}

inline sal_Bool Unmarshal::unpackString( void *pDest )
{
    sal_Int32 nLength;
    sal_Bool bReturn = unpackCompressedSize( &nLength );

    bReturn = bReturn && ! checkOverflow( nLength );
    if( bReturn )
    {
        *(rtl_uString **) pDest = 0;
        rtl_string2UString( (rtl_uString**) pDest, (const sal_Char * )m_pos , nLength,
                            RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS );
        m_pos += nLength;
    }
    else
    {
        *(rtl_uString ** ) pDest = 0;
        rtl_uString_new( (rtl_uString **) pDest );
    }
    return bReturn;
}

inline sal_Bool Unmarshal::unpackCompressedSize( sal_Int32 *pData )
{
    sal_uInt8 n8Size;
    sal_Bool bReturn = unpackInt8( &n8Size  );
    if( bReturn )
    {
        if( n8Size == 0xff )
        {
            unpackInt32( pData );
        }
        else
        {
            *pData = (sal_Int32 ) n8Size;
        }
    }
    return bReturn;
}

inline sal_Bool Unmarshal::unpackAny( void *pDest )
{
    typelib_TypeDescriptionReference *pTypeRef =
        * typelib_static_type_getByTypeClass( typelib_TypeClass_ANY );

    typelib_TypeDescription * pTD = 0;
    typelib_typedescriptionreference_getDescription( &pTD, pTypeRef );

    sal_Bool bReturn = unpack( pDest, pTD );

    typelib_typedescription_release( pTD );

    return bReturn;
}

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
