/*************************************************************************
 *
 *  $RCSfile: urp_marshal.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jbu $ $Date: 2000-09-29 08:42:06 $
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
#ifndef _URP_MARSHAL_HXX_
#define _URP_MARSHAL_HXX_

#include <rtl/ustring.hxx>
#include <rtl/byteseq.hxx>

#include <com/sun/star/uno/Type.hxx>

#include "urp_bridgeimpl.hxx"
#include "urp_marshal_decl.hxx"

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
        sal_uInt16 *p = ( sal_uInt16 *  ) pSource;
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

    inline void Marshal::packAny( void *pSource )
    {
        uno_Any *pAny = (uno_Any * ) pSource;

        // pack the type
        packType( &( pAny->pType ) );
        // pack the value
        typelib_TypeDescription *pType = 0;
        TYPELIB_DANGER_GET( &pType, pAny->pType );
        pack( pAny->pData , pType );
        TYPELIB_DANGER_RELEASE( pType );
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

    inline void Marshal::pack( void *pSource , typelib_TypeDescription *pType )
    {
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
            packAny( pSource );
            break;
        }
        case typelib_TypeClass_TYPEDEF:
        {
            OSL_ASSERT( 0 ); // should never occur
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
            packRecursive( pSource, pType );
            break;
        }
        default:
            OSL_ASSERT( 0 );
        }
    }
}



#endif

