/*************************************************************************
 *
 *  $RCSfile: urp_marshal_decl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: willem.vandorp $ $Date: 2000-09-29 13:03:19 $
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

#ifndef _URP_MARSHAL_DECL_HXX_
#define _URP_MARSHAL_DECL_HXX_

#include <rtl/ustring.hxx>
#include <rtl/byteseq.hxx>

#include <com/sun/star/uno/Type.hxx>

namespace bridges_urp
{
    struct urp_BridgeImpl;

      typedef void
        ( SAL_CALL * urp_extractOidCallback )( remote_Interface *pRemoteI, rtl_uString **ppOid );

    extern char g_bMarshalSystemIsLittleEndian;
    class Marshal
    {
    public:
          Marshal( /* cache access */ struct urp_BridgeImpl *,
                   sal_Int32 m_nBufferSize,
                   urp_extractOidCallback callback = 0
              );
        ~Marshal( );

        inline void pack( void *pSource , typelib_TypeDescription *pType );

        void packRecursive( void *pSource, typelib_TypeDescription *pType );

        void packTid( const ::rtl::ByteSequence &id, sal_Bool bIgnoreCache = sal_False );
        void packOid( const ::rtl::OUString &oid );
        void packType( void *pSource );

        inline void packCompressedSize( sal_Int32 nSize );
        inline void packInt8( void *pSource );
        inline void packInt16( void *pSource );
        inline void packInt32( void *pSource );
        inline void packString( void *pSource );
        inline void packAny( void *pSource );
        inline void packByteSequence( sal_Int8 *pBuffer , sal_Int32 nSize );

        // can be called during marshaling, but not between
        // finish and restart
        // returns true, when nothing has been marshaled
        inline sal_Bool empty() const;

        // stops marshaling, inserts size in front of the buffer
        // getStart and getSize can now be called
        inline void finish( sal_Int32 nMessageCount );

        // must be called after finish. After calling restart,
        // a new marshalling session is started invalidating
        // the previous bufer
        inline void restart();

        // is only valid, after finish has been called.
        // valid until destructed.
        inline sal_Int8 *getBuffer();

        // is only valid, after finish has been called.
        // valid until destructed.
        inline sal_Int32 getSize();

        inline sal_Int32 getPos()
            { return m_pos - m_base; }

        inline sal_Bool isSystemLittleEndian()
            { return g_bMarshalSystemIsLittleEndian; }

    private:
        inline void ensureAdditionalMem( sal_Int32 nMemToAdd );
        sal_Int32 m_nBufferSize;
        sal_Int8 *m_base;
        sal_Int8 *m_pos;
        struct urp_BridgeImpl *m_pBridgeImpl;
        urp_extractOidCallback m_callback;
    };
}
#endif
