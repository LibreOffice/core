/*************************************************************************
 *
 *  $RCSfile: urp_unmarshal.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jbu $ $Date: 2000-09-29 08:42:07 $
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
#ifndef _URP_UNMARSHAL_HXX_
#define _URP_UNMARSHAL_HXX_

#include <rtl/byteseq.hxx>

#include <bridges/remote/context.h>

#include <com/sun/star/uno/Type.hxx>

typedef struct _uno_Environment uno_Environment;
struct remote_Interface;

namespace bridges_urp
{

extern char g_bSystemIsLittleEndian;
class ThreadId;
struct urp_BridgeImpl;

class Unmarshal
{
public:
    Unmarshal(
        struct urp_BridgeImpl *,
        uno_Environment *pEnvRemote,
        remote_createStubFunc callback );
    ~Unmarshal();

    sal_Bool unpackRecursive( void *pDest , typelib_TypeDescription *pType );
    inline sal_Bool finished()
        { return m_base + m_nLength == m_pos; }
    inline sal_uInt32 getPos()
        { return (sal_uInt32 ) (m_pos - m_base); }

    inline sal_Bool setSize( sal_Int32 nSize );

    inline sal_Bool unpackCompressedSize( sal_Int32 *pData );
    inline sal_Bool unpack( void *pDest, typelib_TypeDescription *pType );
    inline sal_Bool unpackInt8( void *pDest );
    inline sal_Bool unpackString( void *pDest );
    inline sal_Bool unpackInt16( void *pDest );
    inline sal_Bool unpackInt32( void *pDest );
    sal_Bool unpackType( void *pDest );

    sal_Bool unpackAny( void *pDest );
    sal_Bool unpackOid( rtl_uString **ppOid );
    sal_Bool unpackTid( sal_Sequence **ppThreadId );

    sal_Int8 *getBuffer()
        { return m_base; }
    inline sal_Bool isSystemLittleEndian()
        { return g_bSystemIsLittleEndian; }

private:
    inline sal_Bool checkOverflow( sal_Int32 nNextMem );

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
        m_nBufferSize = nSize;
        m_base = (sal_Int8 * ) rtl_reallocateMemory( (sal_uInt8*) m_base , m_nBufferSize );
    }
    m_pos = m_base;
    m_nLength = nSize;
    return ( 0 != m_base );
}

inline sal_Bool Unmarshal::checkOverflow( sal_Int32 nNextMem )
{
    return nNextMem < 0 ||
           (((sal_uInt32)( m_pos - m_base )) + nNextMem ) > m_nLength;
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

inline sal_Bool Unmarshal::unpack( void *pDest , typelib_TypeDescription *pType )
{
    sal_Bool bReturn = sal_True;
    switch( pType->eTypeClass )
    {
    case typelib_TypeClass_VOID:
        // do nothing
        break;
    case typelib_TypeClass_BYTE:
    {
        bReturn = unpackInt8( pDest );
        break;
    }
    case typelib_TypeClass_BOOLEAN:
    {
        bReturn = ! checkOverflow( 1 );
        if( bReturn )
        {
            *((sal_Bool*)pDest) = (sal_Bool ) ( *m_pos);
            m_pos ++;
        }
        else
        {
            *((sal_Bool*)pDest) = 0;
        }
        break;
    }

    case typelib_TypeClass_CHAR:
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
    {
        unpackInt16( pDest );
        break;
    }
    case typelib_TypeClass_ENUM:
    case typelib_TypeClass_FLOAT:
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
    {
        bReturn = unpackInt32( pDest );
        break;
    }
    case typelib_TypeClass_DOUBLE:
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
    {
        sal_uInt64 *p = ( sal_uInt64 * ) pDest;
        *p = 0;
        bReturn = ! checkOverflow( 8 );
        if( bReturn )
        {
            if( isSystemLittleEndian() )
            {
                ((sal_Int8*) p )[7] = m_pos[0];
                ((sal_Int8*) p )[6] = m_pos[1];
                ((sal_Int8*) p )[5] = m_pos[2];
                ((sal_Int8*) p )[4] = m_pos[3];
                ((sal_Int8*) p )[3] = m_pos[4];
                ((sal_Int8*) p )[2] = m_pos[5];
                ((sal_Int8*) p )[1] = m_pos[6];
                ((sal_Int8*) p )[0] = m_pos[7];
            }
            else
            {
                ((sal_Int8*) p )[0] = m_pos[0];
                ((sal_Int8*) p )[1] = m_pos[1];
                ((sal_Int8*) p )[2] = m_pos[2];
                ((sal_Int8*) p )[3] = m_pos[3];
                ((sal_Int8*) p )[4] = m_pos[4];
                ((sal_Int8*) p )[5] = m_pos[5];
                ((sal_Int8*) p )[6] = m_pos[6];
                ((sal_Int8*) p )[7] = m_pos[7];
            }
            m_pos += 8;
        }
        break;
    }
    case typelib_TypeClass_STRING:
    {
        unpackString( pDest );
        break;
    }
    case typelib_TypeClass_ANY:
    {
        bReturn = unpackAny( pDest );
        break;
    }
    case typelib_TypeClass_INTERFACE:
    {
        *(remote_Interface**)pDest = 0;

        rtl_uString *pString = 0;
        bReturn = unpackOid( &pString ) && bReturn;

        if( bReturn && pString && pString->length )
        {
            m_callback( (remote_Interface**) pDest ,
                        pString,
                        pType->pWeakRef ,
                        m_pEnvRemote );
        }
        if( pString )
        {
            rtl_uString_release( pString );
        }
        break;
    }
    case typelib_TypeClass_TYPE:
    {
        bReturn = unpackType( pDest );
        break;
    }
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    case typelib_TypeClass_SEQUENCE:
    {
        bReturn = unpackRecursive( pDest, pType );
        break;
    }

    case typelib_TypeClass_UNION:
    case typelib_TypeClass_ARRAY:
    case typelib_TypeClass_SERVICE:
    case typelib_TypeClass_MODULE:
    case typelib_TypeClass_INTERFACE_METHOD:
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
    case typelib_TypeClass_UNKNOWN:
    default:
        OSL_ASSERT( 0 );
    }

    return bReturn;
}

}
#endif
